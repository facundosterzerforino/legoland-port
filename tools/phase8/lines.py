# lines.py file ctx line...: shows each line with ctx lines around it (in the scratch decomp)
import os, sys
os.chdir(os.path.expanduser('~/scratch/dv64/src/legoland'))
f, ctx = sys.argv[1], int(sys.argv[2])
L = open(f, errors='replace').read().split('\n')
for a in sys.argv[3:]:
    if '-' in a:
        s, e = map(int, a.split('-'))
    else:
        s, e = int(a) - ctx, int(a) + ctx
    print('--- %s:%s' % (f, a))
    for i in range(max(s, 1), min(e, len(L)) + 1):
        print('%5d %s' % (i, L[i - 1]))
