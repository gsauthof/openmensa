#!/usr/bin/python

# Update an openmensa feed and check for errors
#
# 2015-01-23, Georg Sauthoff <mail@georg.so>, GPLv3+

import argparse
from   os         import chdir
from   random     import expovariate
from   subprocess import call, check_call
from   time       import sleep

parser = argparse.ArgumentParser(description='Update openmensa feed')
parser.add_argument('--url',       help='URL to fetch',
                    required=True)
parser.add_argument('--output',    help='output directory',
                    required=True)
parser.add_argument('--work',      help='work directory',
                    required=True)
parser.add_argument('--exe',       help='converter to call',
                    required=True)
parser.add_argument('--path',      help='where to look for EXE and xsd',
                    required=True)
parser.add_argument('--name',      help='used for feed name NAME.xml ...',
                    required=True)
parser.add_argument('--tf',        help='extra tidy flags',
                    action='append', default=[])
parser.add_argument('--cf',        help='extra curl flags',
                    action='append', default=[])
parser.add_argument('--xsd',       help='open mensa XSD file',
                    default='open-mensa-v2.xsd')
parser.add_argument('--wait',      help='wait rate',
                    default=0.1,   type=float)
parser.add_argument('--agent',     help='user agent')
opt = parser.parse_args()

sleep(expovariate(1.0/opt.wait))

if opt.agent is not None:
  opt.cf += [ '--user-agent', opt.agent ]

html = opt.name   + '_inp.html'
xml  = opt.name   + '_inp.xml'
feed = opt.name   + '.xml'
exe  = opt.path   + '/' + opt.exe
if opt.xsd.startswith('/'):
  xsd  = opt.xsd
else:
  xsd  = opt.path   + '/' + opt.xsd

chdir(opt.work)

check_call(['curl', '-o', html] + opt.cf + ['--silent', opt.url])
r =   call(['tidy', '-o', xml ] + opt.tf + ['-bare', '-clean', '-indent',
                    '--show-warnings', 'no', '--hide-comments', 'yes',
                    '-numeric', '-q', '-asxml', html])
if (r != 0 and r != 1):
  raise Exception('tidy returned: {}'.format(r))
check_call([exe,       xml], stdout=open(feed, 'w'))
check_call(['xmllint', '-noout', '-schema', xsd, feed],
    stderr=open('xmllint.log', 'w'))
check_call(['cp',      feed, opt.output])
