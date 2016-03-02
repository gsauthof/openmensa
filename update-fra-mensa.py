#!/usr/bin/python3

import subprocess
import os
import sys
import re

home = os.environ['HOME']

items = [
  ( 'fhrus'            , 'mensa-ruesselsheim'                           ),
  ( 'wiesbaden_accent' , 'mensa-accent'                                 ),
  ( 'fra_pi'           , 'mensa-pi-x-gaumen'                            ),
  ( 'fra_casino'       , 'cafeteria-casino'                             ),
  ( 'fra_level'        , 'cafeteria-level'                              ),
  ( 'fra_darwins'      , 'cafeteria-darwins'                            ),
  ( 'fra_casino_anbau' , 'mensa-anbau-casino'                           ),
  ( 'fra_dasein'       , 'dasein'                                       ),
  ( 'fra_bockenheim'   , 'cafeteria-bockenheim'                         ),
  ( 'fra_hochform'     , 'cafe-hochform'                                ),
  ( 'fra_esswerk'      , 'mensa-esswerk'                                ),
  ( 'fra_hfmdk'        , 'hochschule-fuer-musik-und-darstellende-kunst' ),
  ( 'fra_offenbach'    , 'cafeteria-offenbach'                          ),
  ( 'fra_point'        , 'mensa-point'                                  )
  ]

exit_code = 0
for item in items:
  argv = [ '/usr/local/bin/update-mensa.py',
           '--url',    'http://www.studentenwerkfrankfurt.de/essen-trinken/speiseplaene/{}/'.format(item[1]),
           '--output', '/srv/mensa/feed',
           '--work'  , home + '/work/openmensa',
           '--exe'   , 'fhrus2openmensa',
           '--path'  , '/usr/local/bin',
           '--name'  , item[0],
           '--wait'  , '60',
           '--agent' , 'Mozilla/5.0 (X11; Linux x86_64; rv:34.0)'
                        + ' Gecko/20100101 Firefox/34.0',
           '--xsd'   , '/usr/local/share/mensa/open-mensa-v2.xsd',
	   '--html2xml', 'xmllint'
         ]
  try:
    subprocess.check_output(argv, stderr=subprocess.STDOUT)
  except subprocess.CalledProcessError as e:
    exit_code = 1
    print('Command {} failed with exit status {}:\n{}'.format(
      re.sub(',', '', str(e.cmd)), e.returncode, e.output),
      file=sys.stderr)

sys.exit(exit_code)
