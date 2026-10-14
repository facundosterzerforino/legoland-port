"""dupproto.py [dir]: function prototypes that appear more than once in one header (merge leftovers)"""
import collections
import glob
import os
import re
import sys

os.chdir(os.path.expanduser(sys.argv[1] if len(sys.argv) > 1 else '~/wt/port/src/legoland'))
for h in sorted(glob.glob('*.h')):
    cnt = collections.defaultdict(list)
    for i, line in enumerate(open(h, errors='replace').read().split('\n')):
        m = re.match(r'^(?!typedef|#|\s|//|/\*)[^;(]*?\b(\w+)\(.*\);\s*$', line)
        if m:
            cnt[m.group(1)].append((i + 1, line.strip()))
    for k, v in cnt.items():
        if len(v) > 1:
            for ln, t in v:
                print('%s:%d: %s' % (h, ln, t[:140]))
