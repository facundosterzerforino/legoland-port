"""hdr_rename.py header Old=New ...: renames `struct Old` to `struct New` in one header (drops `struct Old;` lines);
first lists every other file that mentions each Old name."""
import os
import re
import sys

os.chdir(os.path.expanduser('~/scratch/dv64/src/legoland'))
hdr = sys.argv[1]
pairs = [a.split('=') for a in sys.argv[2:]]
for old, new in pairs:
    others = [f for f in sorted(os.listdir('.')) if f.endswith(('.c', '.h')) and f != hdr and
              re.search(r'\bstruct %s\b' % old, open(f, errors='replace').read())]
    if others:
        print('%s also in: %s' % (old, ' '.join(others)))
s = open(hdr).read()
for old, new in pairs:
    s = re.sub(r'^struct %s;\n' % old, '', s, flags=re.M)
    s = re.sub(r'\bstruct %s\b' % old, 'struct %s' % new, s)
open(hdr, 'w').write(s)
