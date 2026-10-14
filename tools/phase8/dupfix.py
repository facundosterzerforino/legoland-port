"""dupfix.py: for the port headers' duplicated prototypes, keeps the decomp's line (when exactly one of them is
the decomp's) and drops the others; reports the rest"""
import collections
import glob
import os
import re

P = os.path.expanduser('~/wt/port/src/legoland')
D = os.path.expanduser('~/scratch/dv64/src/legoland')
for h in sorted(glob.glob(P + '/*.h')):
    L = open(h).read().split('\n')
    dl = set(x.strip() for x in open(os.path.join(D, os.path.basename(h))).read().split('\n')) \
        if os.path.exists(os.path.join(D, os.path.basename(h))) else set()
    cnt = collections.defaultdict(list)
    for i, line in enumerate(L):
        m = re.match(r'^(?!typedef|#|\s|//|/\*)[^;(]*?\b(\w+)\(.*\);\s*$', line)
        if m:
            cnt[m.group(1)].append(i)
    drop = set()
    for k, idx in cnt.items():
        if len(idx) < 2:
            continue
        mine = [i for i in idx if L[i].strip() in dl]
        if len(mine) == 1:
            drop |= set(idx) - set(mine)
            print('%s: %s kept the decomp\'s' % (os.path.basename(h), k))
        else:
            print('%s: %s: %d of %d lines are the decomp\'s; left' % (os.path.basename(h), k, len(mine), len(idx)))
    if drop:
        open(h, 'w').write('\n'.join(x for i, x in enumerate(L) if i not in drop))
