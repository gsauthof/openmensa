#include <ostream>
static void help(const char *progname, std::ostream &o)
{
  o << progname /* fhrus2openmensa */ << " -"
    << R"( unibi2openmensa - Convert the Bielefeld University Mensa plan
                  to OpenMensa Feed v2

2015-01-20, Georg Sauthoff <mail@georg.so>

GPLv3+

Example:

    $ curl -o unibi.html \
      'http://www.studentenwerkbielefeld.de/index.php?id=129'
    $ tidy -o unibi.xml -utf8 -bare -clean -indent --show-warnings no \
           --hide-comments yes -numeric -q -asxml unibi.html
    $ ./unibi2openmensa unibi.xml > unibi_feed.xml
    $ xmllint -noout -schema open-mensa-v2.xsd unibi_feed.xml


)";
}

#include "utility.h"

#include <libxml++/libxml++.h>
#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/regex.hpp>
#include <exception>
#include <iostream>
#include <locale>
#include <stdexcept>
#include <string>
using namespace xmlpp;
using namespace std;

static const Node::PrefixNsMap namespaces = {
  { "xhtml", "http://www.w3.org/1999/xhtml" }
};

static unsigned moy(const string &name)
{
  static const map<string, unsigned> m = {
     { "Januar"    , 1  }  , 
     { "Februar"   , 2  }  , 
     { "März"      , 3  }  , 
     { "April"     , 4  }  , 
     { "Mai"       , 5  }  , 
     { "Juni"      , 6  }  , 
     { "Juli"      , 7  }  , 
     { "August"    , 8  }  , 
     { "September" , 9  }  , 
     { "Oktober"   , 10 }  , 
     { "November"  , 11 }  , 
     { "Dezember"  , 12 } 
  };
  return m.at(name);
}

static string date(const Node *day)
{
  string s(day->eval_to_string(
        "normalize-space(./xhtml:a[@class='day-information']/text())",
        namespaces));
  static const boost::regex re(
      R"(^[^,]+, ([0-9]{1,2})\. ([^ ]+) ([0-9]{4})$)");
  boost::smatch m;
  if (boost::regex_match(s, m, re)) {
    return (boost::format("%s-%02u-%02u") % m[3] % moy(m[2])
        % boost::lexical_cast<string>(m[1])).str();
  } else {
    throw runtime_error("Unexpected date string: " + s);
  }
  return std::move(s);
}

static string cat_name(const Node *cat)
{
  string s(cat->eval_to_string("normalize-space(./xhtml:th/text())",
        namespaces));
  return std::move(s);
}

static void gen_note(const Node *cat, ostream &o)
{
  string s(cat->eval_to_string("./xhtml:td[2]//xhtml:div/xhtml:img/@alt",
        namespaces));
  if (s.empty())
    return;
  o << "          <note>" << s << "</note>\n";
}

static void gen_name(const Node *cat, ostream &o)
{
  auto ts = cat->find("./xhtml:td[1]//text()", namespaces);
  for (auto t : ts) {
    string s(t->eval_to_string("normalize-space(.)"));
    if (!s.empty()) {
      o << "          <name>" << s << "</name>\n";
      return;
    }
  }
}

static void gen_long_note(const Node *cat, ostream &o)
{
  auto ts = cat->find("./xhtml:td[1]//text()", namespaces);
  auto i = ts.begin();
  for (; i != ts.end(); ++i) {
    string s((*i)->eval_to_string("normalize-space(.)"));
    if (!s.empty()) {
      ++i;
      break;
    }
  }
  ostringstream m;
  for (; i != ts.end(); ++i) {
    string s((*i)->eval_to_string("normalize-space(.)"));
    if (s.empty())
      continue;
    m << s << ' ';
  }
  static const boost::regex re(R"(\([^)]+\))");
  string s(boost::regex_replace(m.str(), re, ""));
  static const boost::regex re_comma(R"(^ *,)");
  s = boost::regex_replace(s, re_comma, "");
  static const boost::regex re_plenk(R"( +,)");
  s = boost::regex_replace(s, re_plenk, ", ");
  s = normalize(s);
  if (s.empty())
    return;
  if (s.size() > 250)
    s = s.substr(0, 250);
  o << "          <note>" << s << "</note>\n";
}

static const string &to_role(const string &s)
{
  static const map<string, string> m = {
     { "Studierende" , "student"  }  , 
     { "Bedienstete" , "employee" }  , 
     { "Gast"        , "other"    } 
  };
  static string other{"other"};
  auto i = m.find(s);
  if (i == m.end())
    return other;
  return i->second;
}

static void gen_prices(const Node *cat, ostream &o)
{
  auto ts = cat->find("./xhtml:td[3]/xhtml:p/text()", namespaces);
  for (auto t : ts) {
    string s(t->eval_to_string("normalize-space(.)"));
    static const boost::regex re(
        R"(^([A-Za-z]*):? *([0-9]+),([0-9]+) [^ ].*$)");
    boost::smatch m;
    if (boost::regex_match(s, m, re)) {
      o << "          <price role='" << to_role(m[1]) << "'>"
        << m[2] << '.' << m[3]
        << "</price>\n";
    }
  }
}

static bool has_prices(const Node *cat)
{
  auto ts = cat->find("./xhtml:td[3]/xhtml:p/text()", namespaces);
  for (auto t : ts) {
    string s(t->eval_to_string("normalize-space(.)"));
    if (!s.empty())
      return true;
  }
  return false;
}

static void gen_meal(const Node *cat, ostream &o)
{
  o << "        <meal>\n";
  gen_name(cat, o);
  gen_note(cat, o);
  gen_long_note(cat, o);
  gen_prices(cat, o);
  o << "        </meal>\n";
}

static void gen_cat(const Node *cat, string &last_cat_name, ostream &o)
{
  if (!has_prices(cat))
    return;
  string cname(cat_name(cat));
  if (cname != last_cat_name) {
    o << "      </category>\n";
    o << "      <category name='" << cname << "'>\n";
  }
  gen_meal(cat, o);
  last_cat_name = cname;
}

static void gen_dow(const Node *day, ostream &o)
{
  auto cats = std::move(day->find("./xhtml:table/xhtml:tr", namespaces));
  string last_cat_name;
  auto i = cats.begin();
  if (i != cats.end()) {
    if (!has_prices(*i))
      return;
    string d(std::move(date(day)));
    o << "    <day date='" << d << "'>\n";
    o << "      <category name='" << cat_name(*i) << "'>\n";
    gen_meal(*i, o);
    ++i;
  }
  for (; i != cats.end(); ++i)
    gen_cat(*i, last_cat_name, o);
  o << "      </category>\n";
  o << "    </day>\n";
}

static void generate_openmensa(const Node *root, ostream &o)
{
  o << R"(<?xml version="1.0" encoding="UTF-8"?>
<openmensa version="2.0"
           xmlns="http://openmensa.org/open-mensa-v2"
           xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
           xsi:schemaLocation="http://openmensa.org/open-mensa-v2 http://openmensa.org/open-mensa-v2.xsd">
  <canteen>
)";
  auto days = root->find("//xhtml:div[@class='day-block']", namespaces);
  for (auto day : days)
    gen_dow(day, o);
  o << R"(
  </canteen>
</openmensa>
)";
}

struct Option {
  string filename;
  Option(int argc, char **argv);
};
Option::Option(int argc, char **argv)
{
  if (argc > 1) {
    if (!strcmp(argv[1], "-h") || !strcmp(argv[1], "--help")) {
      help(argv[0], cout);
      exit(0);
    }
    filename = argv[1];
    return;
  }
  help(argv[0], cerr);
  exit(1);
}

int main(int argc, char **argv)
{
  std::locale::global(
      std::locale("").combine<std::numpunct<char> >(std::locale()) );
  try {
    Option opt(argc, argv);
    DomParser parser;
    parser.set_substitute_entities(true);
    parser.parse_file(opt.filename);
    generate_openmensa(parser.get_document()->get_root_node(), cout);
  } catch (const std::exception &e) {
    cerr << "Fail: " << e.what() << '\n';
    return 1;
  }
  return 0;
}
