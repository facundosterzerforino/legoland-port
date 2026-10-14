# unused_structs.py file... [--delete]: struct definitions in each file that nothing names any more (in any .c/.h)
import glob
import os
import re
import sys

os.chdir(os.path.expanduser('~/scratch/dv64/src/legoland'))
allsrc = {f: open(f, errors='replace').read() for f in glob.glob('*.c') + glob.glob('*.h')}
for f in [a for a in sys.argv[1:] if not a.startswith('--')]:
    s = allsrc[f]
    dead = []
    for m in re.finditer(r'^struct (\w+) \{', s, re.M):
        n = m.group(1)
        uses = sum(len(re.findall(r'\bstruct %s\b' % n, t)) for t in allsrc.values())
        if uses == 1:
            dead.append(n)
    print(f, 'unused:', ' '.join(dead))
    if '--delete' in sys.argv:
        for n in dead:
            s = re.sub(r'^struct %s \{.*?^\};\n\n?' % n, '', s, count=1, flags=re.M | re.S)
        open(f, 'w').write(s)
