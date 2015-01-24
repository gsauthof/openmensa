#!/usr/bin/python

# 2015-01-24, Georg Sauthoff <mail@georg.so>, GPLv3+

from   argparse   import ArgumentParser
from   subprocess import call, check_call

p = ArgumentParser(description='Compare outputs for golden testing')
p.add_argument('--inp' , help='Input file'      , required=True   ) 
p.add_argument('--out' , help='Output file'     , required=True   ) 
p.add_argument('--ref' , help='Reference file'  , required=True   ) 
p.add_argument('--exe' , help='Executable'      , required=True   ) 
p.add_argument('--ef'  , help='extra exe flags' , action='append' , default=[] )
p.add_argument('--diff', help='diff command'    , default='diff')
o = p.parse_args()

check_call([o.exe ] + o.ef + [  o.inp       ], stdout=open(o.out, 'w'))
check_call([o.diff, '-u', '-w', o.ref, o.out])

