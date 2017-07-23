#!/usr/bin/python

# 2015-01-24, Georg Sauthoff <mail@georg.so>, GPLv3+

from   argparse           import ArgumentParser
from   distutils.dir_util import mkpath
from   os                 import remove
from   os.path            import dirname, exists
from   subprocess         import call, check_call
import tempfile

p = ArgumentParser(description='Compare outputs for golden testing')
p.add_argument('--inp' , help='Input file'      , required=True   ) 
p.add_argument('--out' , help='Output file'     , required=True   ) 
p.add_argument('--ref' , help='Reference file'  , required=True   ) 
p.add_argument('--exe' , help='Executable'      , required=True   ) 
p.add_argument('--ef'  , help='extra exe flags' , action='append' , default=[] )
p.add_argument('--diff', help='diff command'    , default='diff')
p.add_argument('--new', action='store_true', help='new style call')
p.add_argument('--xmllint', default='xmllint', help='xmllint command')
p.add_argument('--schema', default='open-mensa-v2.xsd',
    help='OpenMensa schema')
o = p.parse_args()

mkpath(dirname(o.out))
if exists(o.out):
  remove(o.out)
if o.new:
  check_call([o.exe ] + o.ef + [ '-i', o.inp, '-o', o.out ])
else:
  with open(o.out, 'w') as f:
    check_call([o.exe ] + o.ef + [  o.inp       ], stdout=f)
check_call([o.xmllint, '-noout', '-schema', o.schema, o.ref ])
check_call([o.xmllint, '-noout', '-schema', o.schema, o.out ])
with tempfile.NamedTemporaryFile() as f, tempfile.NamedTemporaryFile() as g:
  check_call([o.xmllint, '--format', o.ref, '--output', f.name])
  check_call([o.xmllint, '--format', o.out, '--output', g.name])
  check_call([o.diff, '-u', '-w', f.name, g.name])
