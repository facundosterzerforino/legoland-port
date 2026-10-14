"""dupchk.py [dir]: globals declared (globals.h) or defined (globals.c) more than once"""
import collections
import os
import re
import sys

os.chdir(os.path.expanduser(sys.argv[1] if len(sys.argv) > 1 else '~/wt/port/src/legoland'))
for fn, pat in (('globals.h', r'^extern [^;(]*?\**\b(\w+)(?:\[[^\]]*\])*;'),
                ('globals.c', r'^(?!extern)[A-Za-z][^;(=]*?\**\b(\w+)(?:\[[^\]]*\])*(?: = [^;]*)?;')):
    cnt = collections.Counter(re.findall(pat, open(fn).read(), re.M))
    print(fn, 'duplicates:', [k for k, v in cnt.items() if v > 1])
