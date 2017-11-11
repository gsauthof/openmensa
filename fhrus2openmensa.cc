#include <ostream>
static void help(const char *progname, std::ostream &o)
{
  o << progname /* fhrus2openmensa */ << " -"
    << R"( Convert the FH-Mensa Rüsselsheim plan to OpenMensa Feed v2

2015-01-18, Georg Sauthoff <mail@georg.so>

GPLv3+

Example:

    $ curl 'http://www.studentenwerkfrankfurt.de/essen-trinken/speiseplaene/mensa-ruesselsheim/' -o fhrus_20151124.html
    $ xmllint --html fhrus_20151124.html --format --xmlout --nonet \
        --output ../test/in/fhrus_20151124.xml
    $ ./fhrus2openmensa --year 2015 ../test/in/fhrus_20151124.xml \
        > ../test/ref/fhrus_20151124.xml
    $ xmllint -noout -schema ../open-mensa-v2.xsd ../test/ref/fhrus_20151124.xml


Options:

  --year YYYY    fake the current year

)";
}

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

static string translate_month(const string &s)
{
  if (s.size() == 3 && s[2] == '.') {
    return s.substr(0, 2);
  } else {
    static const array<const char *, 12> months = {
      "Januar", "Februar", "März", "April", "Mai", "Juni",
      "Juli", "August", "September", "Oktober", "November", "Dezember"
    };
    size_t i = 1;
    for (auto m : months) {
      if (m == s)
        return (boost::format("%02u") % i).str();
      ++i;
    }
    throw runtime_error("Unexpected month string: " + s);
  }
}

static string date(const Node *node)
{
  string s(node->eval_to_string(
        "normalize-space(string(.//div[@class='panel-heading']))", namespaces));

  static const boost::regex re(R"(^[A-Za-z ,]+([0-9]{2})\. *([^ ]+)$)");
  boost::smatch m;
  if (boost::regex_match(s, m, re)) {
    return (boost::format("%1%-%2%-%3%") % Today::instance().year()
        % translate_month(m[2]) % m[1]).str();
  } else {
    throw runtime_error("Unexpected date string: |" + s + "|");
  }
}

static string normalize_price(const string &s)
{
  static const boost::regex re(R"(^([0-9]+),([0-9]+)[^0-9]+$)");
  boost::smatch m;
  if (boost::regex_match(s, m, re)) {
    return (boost::format("%1%.%2%") % m[1] % m[2]).str();
  } else {
    throw underflow_error("Unexpected price string: " + s);
  }
}

static mp::cpp_dec_float_50 guest_price(const string &s)
{
  mp::cpp_dec_float_50 r(s);
  mp::cpp_dec_float_50 x("1.6");
  return r + x;
}

static void gen_name(const Node *menue, ostream &o)
{
  string s(menue->eval_to_string("normalize-space(string(./td[1]//strong))",
      namespaces));
  o << "          <name>" << s << "</name>\n";
}

static void gen_note(const Node *menue, ostream &o)
{
  string s(menue->eval_to_string("normalize-space(string(./td[1]//p))",
      namespaces));
  if (s.empty())
    return;
  o << "          <note>" << s << "</note>\n";
}

static void gen_tags(const Node *menue, ostream &o)
{
  auto tags = menue->find(".//img[@title]", namespaces);
  for (auto tag : tags) {
    auto e = dynamic_cast<const xmlpp::Element*>(tag);
    string s(e->get_attribute_value("title"));
    o << "          <note>" << s << "</note>\n";
  }
}

static void gen_price(const Node *menue, ostream &o)
{
  string charge(normalize_price(menue->eval_to_string(
          "normalize-space(string((./td[2]//p)[1]))",
      namespaces)));
  o << "          <price role='student'>" << charge << "</price>\n";
  o << "          <price role='employee'>" << guest_price(charge)
    << "</price>\n";
  o << "          <price role='other'>" << guest_price(charge)
    << "</price>\n";
}

static void gen_dow(const Node *node, ostream &o)
{
  string d(date(node));
  o << "    <day date='" << d << "'>\n";

  unsigned x = 1;
  auto menues = node->find(".//div[@class='panel-body']/*/tr", namespaces);
  for (auto menue : menues) {
    o << "      <category name='Essen " << x << "'>\n";
    o << "        <meal>\n";
    gen_name(menue, o);
    gen_note(menue, o);
    gen_tags(menue, o);
    try {
      gen_price(menue, o);
    } catch (const underflow_error &) {
      // the price is optional in the XSD - and often enough it
      // is only added later, online
    }
    o << "        </meal>\n";
    o << "      </category>\n";
    ++x;
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
  auto days = root->find("//div[@class='panel panel-default']",
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
