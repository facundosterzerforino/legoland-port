"""x64fn.py file.c [function...] [--code]: the 64-bit warnings of /tmp/x64check.txt for one file, by function (the
overlay's lines). With function names: only those, with the warning lines' code."""
import collections
import re
import sys

f = sys.argv[1]
want = [a for a in sys.argv[2:] if not a.startswith('--')]
code = '--code' in sys.argv or want
src = open('/tmp/x64overlay/src/legoland/' + f, errors='replace').read().split('\n')
owner = []
cur = '(top)'
for i, l in enumerate(src):
    m = re.match(r'^\w[\w\s\*]*?\b(\w+)\s*\([^;]*$', l)
    if m and not l.startswith(('struct', 'typedef', 'extern', 'static const', 'union')) or \
            (m and l.startswith('struct') and l.rstrip().endswith('{') and '(' in l):
        cur = m.group(1)
    owner.append(cur)
by = collections.OrderedDict()
for l in open('/tmp/x64check.txt'):
    m = re.match(r'([^()]+)\((\d+),(\d+)\): warning: (.*) \[(-W[a-z-]+)\]', l)
    if not m or not m.group(1).endswith('/' + f) and m.group(1) != f:
        continue
    ln = int(m.group(2))
    fn = owner[ln - 1]
    by.setdefault(fn, []).append((ln, m.group(4), m.group(5)))
if not want:
    for fn, ws in sorted(by.items(), key=lambda x: x[1][0][0]):
        print('%4d  %-32s %s' % (len(ws), fn, ', '.join(sorted({w[2][2:] for w in ws}))))
    print(sum(len(w) for w in by.values()), 'warnings in', len(by), 'functions')
else:
    for fn in want:
        for ln, msg, kind in sorted(set(by.get(fn, []))):
            print('%s:%d %s\n    %s' % (fn, ln, msg[:110], src[ln - 1].strip()[:150]))
