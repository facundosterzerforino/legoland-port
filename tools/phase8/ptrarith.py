"""ptrarith.py: in the scratch decomp's uncommitted changes, variables retyped from an integer to a pointer that the
same file still uses with integer arithmetic (name + N, name - N, name[...] on a struct pointer, (T)(name + N))"""
import os
import re
import subprocess

D = os.path.expanduser('~/scratch/dv64')
diff = subprocess.run(['git', '-C', D, 'diff', '-U0', '--', 'src/legoland'], capture_output=True, text=True).stdout
cur = None
retyped = {}
for l in diff.split('\n'):
    if l.startswith('+++ b/'):
        cur = l[6:]
        retyped.setdefault(cur, set())
    elif l.startswith('-') and not l.startswith('---'):
        pass
    elif l.startswith('+') and cur:
        for m in re.finditer(r'struct \w+ \*(\w+)\s*[;,)=]', l):
            retyped[cur].add(m.group(1))
for f, names in retyped.items():
    if not names:
        continue
    lines = open(os.path.join(D, f), errors='replace').read().split('\n')
    for i, line in enumerate(lines):
        for n in names:
            if re.search(r'\(\s*(?:\([^()]*\)\s*)?%s\s*[+-]\s*(?:0x[0-9a-fA-F]+|\d+)\s*\)' % re.escape(n), line):
                print('%s:%d  [%s]  %s' % (os.path.basename(f), i + 1, n, line.strip()[:130]))
