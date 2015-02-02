#include <ostream>
static void help(const char *progname, std::ostream &o)
{
  o << progname /* fhrus2openmensa */ << " -"
    << R"( Convert the FH-Mensa Rüsselsheim plan to OpenMensa Feed v2

2015-01-18, Georg Sauthoff <mail@georg.so>

GPLv3+

Example:

    $ curl -o fhrus.html \
      'http://www.studentenwerkfrankfurt.de/index.php?id=585&no_cache=1&type=98'
    $ tidy -o fhrus.xml -bare -clean -indent --show-warnings no \
           --hide-comments yes -numeric \
           -q -asxml fhrus.html
    $ ./fhrus2openmensa fhrus.xml > fhrus_feed.xml
    $ xmllint -noout -schema open-mensa-v2.xsd fhrus_feed.xml


Options:

  --year YYYY    fake the current year

)";
}
#include "utility.h"

#include <libxml++/libxml++.h>
#include <boost/date_time/posix_time/posix_time.hpp>
//#include <boost/date_time/posix_time/posix_time_io.hpp>
#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/multiprecision/cpp_dec_float.hpp>
#include <boost/regex.hpp>
#include <cstdio>
#include <cstdlib>
#include <exception>
#include <iostream>
#include <locale>
#include <stdexcept>
#include <string>
using namespace xmlpp;
namespace mp = boost::multiprecision;
using namespace std;

class Today {
  private:
    string year_;
  public:
    Today();
    const string &year() const;
    void set_year(const string &y);
    static Today &instance();
};
Today::Today()
{
  auto t = boost::posix_time::second_clock::local_time();
  year_ = boost::lexical_cast<string>(t.date().year());
}
const string &Today::year() const { return year_; };
void Today::set_year(const string &y) { if (!y.empty()) year_ = y; }
Today &Today::instance()
{
  static Today t;
  return t;
}

static const Node::PrefixNsMap namespaces = {
  { "xhtml", "http://www.w3.org/1999/xhtml" }
};

static string date(const Node *node)
{
  string s(node->eval_to_string(
        "./xhtml:tr/xhtml:td/xhtml:strong/text()", namespaces));

  static const boost::regex re(R"(^[A-Za-z ,]+([0-9]{2})\.([0-9]{2})\.$)");
  boost::smatch m;
  if (boost::regex_match(s, m, re)) {
    return (boost::format("%1%-%2%-%3%") % Today::instance().year()
        % m[2] % m[1]).str();
  } else {
    throw runtime_error("Unexpected date string: " + s);
  }
}

static string normalize_price(const string &s)
{
  static const boost::regex re(R"(^([0-9]+),([0-9]+)[^0-9]+$)");
  boost::smatch m;
  if (boost::regex_match(s, m, re)) {
    return (boost::format("%1%.%2%") % m[1] % m[2]).str();
  } else {
    throw runtime_error("Unexpected price string: " + s);
  }
}

static NodeSet names(const Node *node)
{
  auto r = node->find("./xhtml:tr/xhtml:td/xhtml:div/xhtml:strong/text()",
      namespaces);
  return std::move(r);
}
static NodeSet notes(const Node *node)
{
  auto r = node->find("./xhtml:tr/xhtml:td/xhtml:div/xhtml:p/text()",
      namespaces);
  return std::move(r);
}
static NodeSet prices(const Node *node)
{
  auto r = node->find("./xhtml:tr/xhtml:td/xhtml:p[@class='price']/xhtml:strong/text()",
      namespaces);
  return std::move(r);
}

static mp::cpp_dec_float_50 guest_price(const string &s)
{
  mp::cpp_dec_float_50 r(s);
  mp::cpp_dec_float_50 x("1.6");
  return r + x;
}

static void gen_dow(const Node *node, ostream &o)
{
  string d(std::move(date(node)));
  o << "    <day date='" << d << "'>\n";
  NodeSet n(std::move(names(node)));
  NodeSet m(std::move(notes(node)));
  NodeSet p(std::move(prices(node)));
  auto i = n.begin();
  auto j = m.begin();
  auto k = p.begin();
  unsigned x = 1;
  for (; i != n.end() && j != m.end() && k != p.end(); ++i, ++j, ++k, ++x) {
    o << "      <category name='Essen " << x << "'>\n";
    o << "        <meal>\n";
    ContentNode *name = dynamic_cast<ContentNode*>(*i);
    o << "          <name>" << normalize(name->get_content()) << "</name>\n";
    ContentNode *note = dynamic_cast<ContentNode*>(*j);
    o << "          <note>" << normalize(note->get_content()) << "</note>\n";
    ContentNode *price = dynamic_cast<ContentNode*>(*k);
    string charge(std::move(normalize_price(price->get_content())));
    o << "          <price role='student'>" << charge << "</price>\n";
    o << "          <price role='employee'>" << guest_price(charge)
      << "</price>\n";
    o << "          <price role='other'>" << guest_price(charge)
      << "</price>\n";
    o << "        </meal>\n";
    o << "      </category>\n";
  }
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
  auto days = root->find("//xhtml:div[@class='dates']/xhtml:table",
      namespaces);
  for (auto day : days)
    gen_dow(day, o);
  o << R"(
  </canteen>
</openmensa>
)";
}

struct Option {
  string filename;
  string year;
  Option(int argc, char **argv);
};
Option::Option(int argc, char **argv)
{
  if (argc > 3 && !strcmp(argv[1], "--year")) {
    year = argv[2];
    filename = argv[3];
    return;
  }
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
  // std::locale::global(std::locale(""));
  // use the users local (otherwise glib throws on utf8 chars != ascii
  // + don't use braindead numpunct settings,
  //   e.g. 1,024 instead of 1024 ...
  std::locale::global(
      std::locale("").combine<std::numpunct<char> >(std::locale()) );
  try {
    Option opt(argc, argv);
    Today::instance().set_year(opt.year);
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
