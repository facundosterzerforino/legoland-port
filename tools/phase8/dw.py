"""dw.py file.c... : /tmp/dx64.txt warnings (the decomp's own lines) with the source line and the function"""
import os
import re
import sys

os.chdir(os.path.expanduser('~/scratch/dv64/src/legoland'))
W = [l.strip() for l in open('/tmp/dx64.txt')]
for f in sys.argv[1:]:
    L = open(f, errors='replace').read().split('\n')
    fn = {}
    cur = '?'
    for i, line in enumerate(L):
        m = re.match(r'^[A-Za-z_][^;=(]*?\b(\w+)\([^;]*$', line)
        if m and not line.startswith(('if', 'for', 'while', 'return', 'typedef', '#')):
            cur = m.group(1)
        fn[i + 1] = cur
    seen = set()
    for w in sorted(W, key=lambda x: int(re.search(r'\((\d+),', x).group(1)) if x.startswith(f + '(') else 0):
        m = re.match(r'%s\((\d+),(\d+)\): warning: (.*)$' % re.escape(f), w)
        if not m:
            continue
        ln = int(m.group(1))
        msg = re.sub(r" \(aka '[^']*'\)", '', m.group(3))
        msg = re.sub(r' \[-W[\w-]+\]$', '', msg)
        print('%s:%d %s: %s' % (f, ln, fn.get(ln, '?'), msg[:110]))
        if ln not in seen:
            print('      ' + L[ln - 1].strip()[:160])
            seen.add(ln)
