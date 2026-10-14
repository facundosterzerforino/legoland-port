"""danglefix.py file.c...: renames the dangling struct references (dangling.py) to their retarget type, from the
elemchain mappings of the given files; forward declarations of the old names are dropped"""
import glob
import os
import re
import subprocess
import sys

S = os.path.dirname(os.path.abspath(__file__))
mp = {}
for f in sys.argv[1:]:
    out = subprocess.run(['python3', '/tmp/ec_args.py', f], capture_output=True, text=True).stdout.split()
    for a in out:
        m = re.match(r'(\w+):[\w.]+=(\w+)$', a)
        if m:
            mp[m.group(1)] = m.group(2)
os.chdir(os.path.expanduser('~/scratch/dv64'))
d = subprocess.run(['git', 'diff', 'HEAD', '--', 'src/legoland'], capture_output=True, text=True).stdout
removed = set(re.findall(r'^-struct (\w+) \{', d, re.M))
files = glob.glob('src/legoland/*.[ch]')
defined = set()
for f in files:
    defined |= set(re.findall(r'^(?:typedef )?struct (\w+) \{', open(f, errors='replace').read(), re.M))
for n in sorted(removed - defined):
    if n not in mp:
        print('no mapping for', n)
        continue
    for f in files:
        s = open(f, errors='replace').read()
        s2 = re.sub(r'^struct %s;\n' % n, '', s, flags=re.M)
        s2 = re.sub(r'\bstruct %s\b' % n, 'struct ' + mp[n], s2)
        if s2 != s:
            open(f, 'w').write(s2)
            print('%s: %s -> %s' % (os.path.basename(f), n, mp[n]))
