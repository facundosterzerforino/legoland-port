# sdef.py Name... : every definition of each struct in the scratch decomp, with file:line
import glob
import os
import re
import sys

os.chdir(os.path.expanduser('~/scratch/dv64/src/legoland'))
files = {f: open(f, errors='replace').read() for f in sorted(glob.glob('*.h')) + sorted(glob.glob('*.c'))}
for name in sys.argv[1:]:
    for f, s in files.items():
        for m in re.finditer(r'^(typedef )?struct %s \{.*?^\}[^;]*;' % re.escape(name), s, re.M | re.S):
            print('== %s:%d' % (f, s[:m.start()].count('\n') + 1))
            print(m.group(0))
