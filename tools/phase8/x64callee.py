"""x64callee.py [N]: the functions whose calls cause the most 64-bit warnings (the warning's column inside a call's
argument list, or a cast of the call's result), across /tmp/x64check.txt"""
import collections
import re
import sys

src = {}
count = collections.Counter()
files = collections.defaultdict(set)
for l in open('/tmp/x64check.txt'):
    m = re.match(r'([^()]+)\((\d+),(\d+)\): warning: (.*) \[(-W[a-z-]+)\]', l)
    if not m:
        continue
    f, ln, col = m.group(1), int(m.group(2)), int(m.group(3))
    if f not in src:
        try:
            src[f] = open('/tmp/x64overlay/src/legoland/' + f.split('/')[-1], errors='replace').read().split('\n')
        except OSError:
            continue
    line = src[f][ln - 1]
    # innermost call whose argument list contains the column
    depth = 0
    callee = None
    for i in range(min(col - 1, len(line) - 1), -1, -1):
        c = line[i]
        if c == ')':
            depth += 1
        elif c == '(':
            if depth == 0:
                mm = re.search(r'([A-Za-z_]\w*)\s*$', line[:i])
                if mm and mm.group(1) not in ('if', 'while', 'for', 'return', 'sizeof', 'switch'):
                    callee = mm.group(1)
                    break
            else:
                depth -= 1
    if callee is None:
        mm = re.search(r'=\s*\([^)]*\)\s*([A-Za-z_]\w*)\(', line)
        if mm:
            callee = mm.group(1) + ' (result)'
    if callee:
        count[callee] += 1
        files[callee].add(f.split('/')[-1])
for name, n in count.most_common(int(sys.argv[1]) if len(sys.argv) > 1 else 40):
    print('%4d  %-34s %d files' % (n, name, len(files[name])))
