#!/usr/bin/env python3

# 2017, Georg Sauthoff <mail@gms.tf>

import argparse
import html5lib
import random
import requests
import sys
import time
import xml.etree.ElementTree as ET

xns = '{http://www.w3.org/1999/xhtml}'
ons = '{http://openmensa.org/open-mensa-v2}'

def mk_arg_parser():
  p = argparse.ArgumentParser(
        formatter_class=argparse.RawDescriptionHelpFormatter,
        description='Create OpenMensa feed for Bielefeld University',
        epilog='...')
  p.add_argument('--output', '-o', metavar='FILE', required=True,
      help='output feed XML file')
  p.add_argument('--input', '-i', metavar='FILE',
      help='Use local file instead of HTTP get')
  p.add_argument('--url', metavar='URL', default='http://www.studierendenwerk-bielefeld.de/essen-trinken/essen-und-trinken-in-mensen/bielefeld/mensa-gebaeude-x.html',
      help='HTTP url to fetch')
  p.add_argument('--wait', nargs='?', const=60, type=float,
      help='wait some random amount of time before doing any work')
  p.add_argument('--agent', default='Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/59.0.3071.115 Safari/537.36',
      help='HTTP user agent to use')
  return p

def parse_args(*a):
  arg_parser = mk_arg_parser()
  args = arg_parser.parse_args(*a)
  if args.wait:
    time.sleep(random.expovariate(1.0/args.wait))
  return args

def get(url, agent):
  s = requests.Session()
  s.headers.update({'User-Agent': agent})
  r = s.get(url)
  r.raise_for_status()
  return r.text

def get_tables(inp):
  d = html5lib.parse(inp)
  l = []
  for i in d.iter():
      if i.tag == xns + 'h2':
          last_h2 = i
      elif i.tag == xns+'div' and i.get('class') == 'mensa plan':
          l.append( (last_h2, i.find(xns+'div').find(xns+'table') ) )
  return l

def parse_date(s):
  x = s.strip()
  x = x.split()[1]
  es = x.split('.')
  es.reverse()
  if len(es) != 3:
      return None
  r = '-'.join(es)
  return r

def parse_name(ps):
  ts = [ y for y in  ( x.strip() for p in ps for x in p.itertext() ) if y ]
  ts = [ x for x in ts if 'Details' not in x ]
  if not ts:
    return ('', '')
  notes = ' '.join(ts[1:])[0:250] if ts.__len__() > 1 else ''
  return ( ts[0], notes )

def parse_price(s):
  ls = s.split('|')
  class Only_Num:
    def __getitem__(self, c):
      if chr(c).isnumeric():
        return c
      if chr(c) == ',':
        return '.'
  rs = [ x.translate(Only_Num()) for x in ls ]
  def f(x):
    i = x.find('.')
    if i == -1:
      return x
    else:
      return x[0:i+3]
  rs = [ f(x) for x in rs ]
  rs = [ x for x in rs if x ]
  return rs

class Skip_This(Exception):
  pass

def mk_category(tr, old_c):
  tds = tr.findall(xns+'td')
  name = ''.join(tds[0].find(xns+'h3').itertext()).strip()
  ls = parse_name(tds[0].findall(xns+'p'))
  if not name or not ls[0]:
    raise Skip_This()
  if old_c and old_c.get('name') == name:
    c = old_c
  else:
    c = ET.Element(ons+'category', name=name)
  m = ET.SubElement(c, ons+'meal')
  ls = parse_name(tds[0].findall(xns+'p'))
  ET.SubElement(m, ons+'name').text = ls[0]
  if ls[1]:
    ET.SubElement(m, ons+'note').text = ls[1]
  ps = parse_price(' '.join(tds[1].itertext()))
  for role, price in zip(['student', 'employee', 'other'], ps):
    ET.SubElement(m, ons+'price', role=role).text = price
  return c

def mk_feed(ts):
  feed = ET.Element(ons + 'openmensa', version='2.0')
  canteen = ET.SubElement(feed, ons+'canteen')
  for h2, table in ts:
    dt = parse_date(h2.text)
    if dt is None:
        continue
    day = ET.SubElement(canteen, ons+'day', date=dt)
    old_c = None
    for tr in table.find(xns+'tbody').iter(xns+'tr'):
      try:
        c = mk_category(tr, old_c)
      except Skip_This:
        continue
      day.append(c)
      old_c = c

  return ET.ElementTree(feed)

def run(args):
  ET.register_namespace('', ons[1:-1])
  if args.input:
    with open(args.input) as f:
      inp = f.read()
  else:
    inp = get(args.url, args.agent)
  ts = get_tables(inp)
  feed = mk_feed(ts)
  feed.write(args.output, encoding='UTF-8')
  return 0

def main(*a):
  args = parse_args(*a)
  return run(args)

if __name__ == '__main__':
  sys.exit(main())
