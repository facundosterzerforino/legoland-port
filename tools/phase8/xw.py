"""xw.py file.c [kind-regex]: the file's x64check warnings with the overlay source line and enclosing function"""
import re
import sys

f = sys.argv[1]
kind = sys.argv[2] if len(sys.argv) > 2 else '.'
L = open('/tmp/x64overlay/src/legoland/' + f, errors='replace').read().split('\n')
func_at = {}
cur = '?'
for i, line in enumerate(L):
    m = re.match(r'^[A-Za-z_][^;=(]*?\b(\w+)\([^;]*$', line)
    if m and not line.startswith(('if', 'for', 'while', 'return', 'typedef', '#')):
        cur = m.group(1)
    func_at[i + 1] = cur
for w in open('/tmp/x64check.txt'):
    m = re.match(r'%s\((\d+),(\d+)\): warning: (.*)$' % re.escape(f), w.strip())
    if not m or not re.search(kind, m.group(3)):
        continue
    ln = int(m.group(1))
    msg = re.sub(r" \(aka '[^']*'\)", '', m.group(3))
    print('%d %s: %s\n      %s' % (ln, func_at.get(ln, '?'), msg[:130], L[ln - 1].strip()[:150]))
