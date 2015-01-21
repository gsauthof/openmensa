/* Utility functions.
 *
 * 2015-01-20, Georg Sauthoff <mail@georg.so>
 *
 * GPLv3+
 */
#include <boost/regex.hpp>
#include <boost/algorithm/string/trim.hpp>

using namespace std;

string normalize(const string &s)
{
  static const boost::regex re(R"([ \t\n]+)");
  return boost::algorithm::trim_copy(boost::regex_replace(s, re, " "));
}
