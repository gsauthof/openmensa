Open Mensa Feed v2 Generators

2015-01-23, Georg Sauthoff <mail@georg.so>

This repository contains a few programs for generating menus in the
[Open Mensa Feed v2][feed] format:

- fhrus2openmena.cc  - FH-Rüsselsheim Mensa page parser, also supports
                       several other Mensas of the Studentenwerk
                       Frankfurt
- unibi2openmensa.cc - Bielefeld University Mensa page parser
                       (outdated)
- unibi2openmensa.py - Bielefeld University Mensa page parser,
                       also supports other Mensas of the operator
- update-mensa.py    - Feed updater

## Compile

    $ mkdir build
    $ cd build
    $ cmake -G Ninja ..
    $ ninja-build

If you don't have the ninja build system, you can omit the `-G Ninja` part
and use `make` instead.

The repository contains a few golden tests, they can be executed via:

    $ ninja-build check

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

The second generation, i.e. the latest parser for Bielefeld
University, is written in Python.

Main motivating factor for choosing Python over a C++ solution:
The html5lib Python package, which does a very good job
normalizing real-world html into an XML tree. It certainly does a
better job than libxml (via its html API) and HTML tidy. Also, in
contrast to HTML tidy, it is actively maintained and obviously
easier to integrate than an external program.

Besides that, the rich Python standard library doesn't leave much
to be desired: high-level string processing, XML tree
construction, XPath all is available without extra dependencies.

### Dependencies

- C++11 Compiler (e.g. [GCC][gcc] >= 4.8)
- POSIX like system
- [CMake][cmake] (>= 2.8)
- [Boost][boost] (>= 1.55)
- [libxml++][libxml++] (>= 2.6, C++ wrapper around [libxml2][libxml2])
- [HTML tidy][tidy]
- [xmllint][xmllint] (part of libxml2)
- [html5lib][html5lib]
- [requests][requests]

## Licence

[GPLv3+][gpl3]

[boost]:    http://www.boost.org/
[cmake]:    http://www.cmake.org/
[feed]:     http://doc.openmensa.org/feed/v2/
[gcc]:      http://gcc.gnu.org/
[gpl3]:     http://www.gnu.org/copyleft/gpl.html
[libxml++]: http://library.gnome.org/devel/libxml++-tutorial/stable/
[libxml2]:  http://www.xmlsoft.org/
[python]:   http://www.python.org/
[tidy]:     http://tidy.sourceforge.net/
[xmllint]:  http://xmlsoft.org/xmllint.html
[xpath]:    http://en.wikipedia.org/wiki/XPath
[html5lib]: https://github.com/html5lib/html5lib-python
[requests]: http://docs.python-requests.org/en/master/
