#!/usr/bin/env python3

# 2020, Georg Sauthoff <mail@gms.tf>

import argparse
import datetime
import decimal
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
        description='Create OpenMensa feed for Frankfurt University Mensas',
        epilog='...')
    p.add_argument('--output', '-o', metavar='FILE', required=True,
            help='output feed XML file')
    p.add_argument('--input', '-i', metavar='FILE',
            help='Use local file instead of HTTP get')
    p.add_argument('--url', metavar='URL',
            default='https://www.studentenwerkfrankfurt.de/essen-trinken/speiseplaene/cafeteria-level',
            help='HTTP url to fetch (default: %(default)s)')
    p.add_argument('--wait', nargs='?', const=60, type=float,
            help='wait some random amount of time before doing any work')
    p.add_argument('--agent', default=('Mozilla/5.0 (Windows NT 10.0; Win64; x64) '
            'AppleWebKit/537.36 (KHTML, like Gecko) Chrome/59.0.3071.115 Safari/537.36'),
            help='HTTP user agent to use')
    p.add_argument('--year', type=int,
            help=f"fake the current year (default: today's year , i.e. {year})")
    return p

def parse_args(*a):
    global year
    arg_parser = mk_arg_parser()
    args = arg_parser.parse_args(*a)
    if args.wait:
        time.sleep(random.expovariate(1.0/args.wait))
    if args.year:
        year = args.year
    return args


def get(url, agent):
    s = requests.Session()
    s.headers.update({'User-Agent': agent})
    r = s.get(url)
    r.raise_for_status()
    return r.text


months = [ "Januar", "Februar", "März", "April", "Mai", "Juni",
      "Juli", "August", "September", "Oktober", "November", "Dezember" ]

year = datetime.date.today().year

def parse_date(d):
    for e in d.iterfind(f'./{xns}div[@class="panel-heading"]/{xns}strong'):
        xs = e.text.strip().split()
        if len(xs) != 3:
            break
        i = months.index(xs[2]) + 1
        if i == -1:
            break
        j = int(xs[1].strip('.'))
        return f'{year}-{i:02}-{j:02}'
    return None

def parse_name(d):
    for e in d.iterfind(f'./{xns}td//{xns}strong'):
        return e.text
    raise RuntimeError("Couldn't find meal name")

def parse_note(d):
    for e in d.iterfind(f'./{xns}td//{xns}p'):
        if e.text is None:
            continue
        return ' '.join(e.text.split())
    return ''

def parse_tags(d):
    return list(e.get('title').strip() for e in d.iterfind(f'.//{xns}img[@title]'))

def parse_price(d):
    try:
        itr = d.iterfind(f'./{xns}td//{xns}strong')
        next(itr)
        e = next(itr)
        if '€' not in e.text:
            return None
        s = e.text.split()[0]
        s = s.replace(',', '.')
        return decimal.Decimal(s)
    except StopIteration:
        return None


def doc2feed(doc):
    feed = ET.Element(ons + 'openmensa', version='2.0')
    feed.text = '\n  '
    feed.tail = '\n'
    canteen = ET.SubElement(feed, ons+'canteen')
    canteen.text = '\n    '
    canteen.tail = '\n'

    day = ET.Element('dummy')
    for day_e in doc.iterfind(f'.//{xns}div[@class="panel-heading"]/..'):
        dt = parse_date(day_e)
        if dt is None:
            continue
        day = ET.SubElement(canteen, ons+'day', date=dt)
        day.text = '\n      '
        day.tail = '\n    '
        category = ET.Element('dummy')
        for i, meal_e in enumerate(day_e.iterfind(f'.//{xns}div[@class="panel-body"]/{xns}table/*/{xns}tr'), 1):
            category.tail = '\n      '
            category = ET.SubElement(day, ons+'category', name=f'Essen {i}')
            category.text = '\n        '
            category.tail = '\n    '
            meal     = ET.SubElement(category, ons+'meal')
            meal.text = '\n          '
            meal.tail = '\n      '
            name = ET.SubElement(meal, ons+'name')
            name.text = parse_name(meal_e)
            name.tail = '\n          '
            note_str = parse_note(meal_e)
            if note_str:
                note = ET.SubElement(meal, ons+'note')
                note.text = note_str
                note.tail = '\n          '
            for t in parse_tags(meal_e):
                note = ET.SubElement(meal, ons+'note')
                note.text = t
                note.tail = '\n          '
            p, markup = parse_price(meal_e), decimal.Decimal('1.60')
            if p:
                for r in ('employee', 'other'):
                    price = ET.SubElement(meal, ons+'price', role='employee')
                    price.text = str(p + markup)
                    price.tail = '\n          '
                price = ET.SubElement(meal, ons+'price', role='student' )
                price.text = str(p)
                price.tail = '\n        '
    day.tail = '\n  '

    return ET.ElementTree(feed)


def run(args):
    ET.register_namespace('', ons[1:-1])
    if args.input:
        with open(args.input) as f:
            inp = f.read()
    else:
        inp = get(args.url, args.agent)

    doc  = html5lib.parse(inp)
    feed = doc2feed(doc)
    feed.write(args.output, encoding='UTF-8')


def main(*a):
    args = parse_args(*a)
    return run(args)


if __name__ == '__main__':
    sys.exit(main())
