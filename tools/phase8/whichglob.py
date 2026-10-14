"""whichglob.py [--decomp] addr...: the annotated global (port sources by default) containing each address"""
import glob
import os
import re
import sys

d = '~/scratch/dv64/src/legoland' if '--decomp' in sys.argv else '~/wt/port/src/legoland'
os.chdir(os.path.expanduser(d))
ann = []
for f in glob.glob('*.c'):
    L = open(f, encoding='latin-1').read().split('\n')
    for i, l in enumerate(L):
        m = re.match(r'\s*// GLOBAL: LEGOLAND 0x([0-9a-f]+)', l)
        if m:
            j = i + 1
            while j < len(L) and L[j].strip().startswith('//'):
                j += 1
            ann.append((int(m.group(1), 16), f, L[j].strip()[:120] if j < len(L) else ''))
ann.sort()
for a in sys.argv[1:]:
    if a.startswith('--'):
        continue
    t = int(a, 16)
    best = None
    for g in ann:
        if g[0] <= t:
            best = g
    print('0x%08x in 0x%08x +0x%x  %s: %s' % (t, best[0], t - best[0], best[1], best[2]))
