# addinc.py header struct: adds #include "header" to every .c whose last build says struct `struct` is undefined
import os
import re
import sys

os.chdir(os.path.expanduser('~/scratch/dv64/src/legoland'))
hdr, name = sys.argv[1], sys.argv[2]
log = open('/tmp/dvb.log', errors='replace').read()
pat = r'(\w+\.c)\(\d+\) : error C2\d+:[^\n]*undefined struct/union \'' + name + r'\''
files = sorted(set(re.findall(pat, log)))
for f in files:
    s = open(f).read()
    if '#include "%s"' % hdr in s:
        print('already includes', f)
        continue
    incs = list(re.finditer(r'^#include "[^"]+"\n', s, re.M))
    pos = incs[-1].end() if incs else 0
    s = s[:pos] + '#include "%s"\n' % hdr + s[pos:]
    open(f, 'w').write(s)
    print('included in', f)
