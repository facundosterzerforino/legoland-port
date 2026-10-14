"""cmpfix.py [--dry]: from /tmp/dx64.txt (dx64.sh), pointer-to-integer casts whose operand is compared (< > <= >= == !=)
become INT_PTR/UINT_PTR casts (same code on x86, no truncation on x64)"""
import collections
import os
import re
import sys

os.chdir(os.path.expanduser('~/scratch/dv64/src/legoland'))
SIGNED = {'int', 'long', 'signed int', 'short'}
UNSIGNED = {'unsigned int', 'unsigned long', 'unsigned', 'unsigned short'}
hits = collections.defaultdict(set)
for w in open('/tmp/dx64.txt'):
    m = re.match(r'(\w+\.c)\((\d+),(\d+)\): warning: cast to smaller integer type \'([^\']+)\' from', w)
    if m:
        hits[m.group(1)].add((int(m.group(2)), int(m.group(3)), m.group(4)))
CMP = r'(<=|>=|==|!=|<|>)'
OPERAND = r'&?\(?[\w.\->\[\]]+\)?(?:\[[^\]]*\])*'
total = 0
for f, hs in sorted(hits.items()):
    L = open(f).read().split('\n')
    for ln, col, typ in sorted(hs, key=lambda x: (x[0], -x[1])):
        line = L[ln - 1]
        c = col - 1
        cast = '(%s)' % typ
        if not line.startswith(cast, c):
            continue
        rest = line[c + len(cast):]
        om = re.match(r'\s*' + OPERAND, rest)
        if not om:
            continue
        after = rest[om.end():]
        before = line[:c]
        if not (re.match(r'\s*' + CMP + r'(?!=)', after) or re.search(CMP + r'\s*$', before)):
            continue
        if typ in SIGNED:
            new = '(INT_PTR)'
        elif typ in UNSIGNED:
            new = '(UINT_PTR)'
        else:
            continue
        L[ln - 1] = line[:c] + new + line[c + len(cast):]
        total += 1
        if '--dry' in sys.argv:
            print('%s:%d: %s' % (f, ln, L[ln - 1].strip()[:140]))
    if '--dry' not in sys.argv:
        open(f, 'w').write('\n'.join(L))
print(total, 'casts')
