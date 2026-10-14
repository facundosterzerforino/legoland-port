"""newwarn.py old.log new.log: MSVC warnings (C40xx) in new.log that old.log doesn't have, by file and message
(line numbers ignored for the comparison, shown for the new ones)"""
import collections
import re
import sys


def load(p):
    out = []
    for line in open(p, errors='replace'):
        m = re.search(r'legoland\\(\w+\.[ch])\((\d+)\) : warning (C4\d+): (.*)$', line.strip())
        if m:
            out.append((m.group(1), m.group(3), m.group(4).strip(), int(m.group(2))))
    return out


old = collections.Counter((f, c, msg) for f, c, msg, _ in set(load(sys.argv[1])))
seen = collections.Counter()
for f, c, msg, ln in sorted(set(load(sys.argv[2]))):
    k = (f, c, msg)
    seen[k] += 1
    if seen[k] > old[k] and c not in ('C4028',):
        print('%s(%d): %s %s' % (f, ln, c, msg[:150]))
