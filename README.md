Open Mensa Feed v2 Generators

2015-2020, Georg Sauthoff <mail@georg.so>

This repository contains a few programs for generating menus in the
[Open Mensa Feed v2][feed] format:

- fra2openmensa.py   - Frankfurt University Mensa page parser,
                       also supports other Mensas of the operator
- unibi2openmensa.py - Bielefeld University Mensa page parser,
                       also supports other Mensas of the operator

- fhrus2openmena.cc  - FH-Rüsselsheim Mensa page parser, also supports
                       several other Mensas of the Studentenwerk
                       Frankfurt (outdated)
- unibi2openmensa.cc - Bielefeld University Mensa page parser
                       (outdated)


## Design Choices

### First Generation

For converting the HTML pages into the XML feed, they are first cleaned
up into well-formed and conforming XHTML via [HTML Tidy][tidy].

The generators are written in C++11 and heavily use [XPath][xpath]
expressions.

[Python][python] is used for the update and test scripts instead of Shell,
because of the library support. In that sense, Python is used as a
replacement for shell scripting.

### Second Generation

The second generation is written in [Python][python].

Main motivating factor for choosing Python over a C++ solution:
The [html5lib][html5lib] Python package - which does a very good
job normalizing real-world html into an XML tree. It certainly
does a better job than [libxml2][libxml2] (via its html API) and
HTML [tidy][tidy]. Also, in contrast to HTML tidy, it is actively
maintained and obviously easier to integrate than an external
program.

Besides that, the rich Python standard library doesn't leave much
to be desired: high-level string processing, XML tree
construction, [XPath][xpath] all is available without extra dependencies.

### Dependencies

- [html5lib][html5lib]
- [requests][requests]

## Licence

[GPLv3+][gpl3]

[feed]:     http://doc.openmensa.org/feed/v2/
[gpl3]:     http://www.gnu.org/copyleft/gpl.html
[libxml2]:  http://www.xmlsoft.org/
[python]:   http://www.python.org/
[tidy]:     http://tidy.sourceforge.net/
[xpath]:    http://en.wikipedia.org/wiki/XPath
[html5lib]: https://github.com/html5lib/html5lib-python
[requests]: http://docs.python-requests.org/en/master/
