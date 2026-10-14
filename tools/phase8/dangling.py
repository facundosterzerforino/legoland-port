"""dangling.py: struct names whose definition the uncommitted diff removed but that are still referenced somewhere"""
import glob
import os
import re
import subprocess

os.chdir(os.path.expanduser('~/scratch/dv64'))
d = subprocess.run(['git', 'diff', 'HEAD', '--', 'src/legoland'], capture_output=True, text=True).stdout
removed = set(re.findall(r'^-struct (\w+) \{', d, re.M))
src = {f: open(f, errors='replace').read() for f in glob.glob('src/legoland/*.[ch]')}
defined = set()
for s in src.values():
    defined |= set(re.findall(r'^(?:typedef )?struct (\w+) \{', s, re.M))
for n in sorted(removed - defined):
    for f, s in src.items():
        for i, line in enumerate(s.split('\n')):
            if re.search(r'\bstruct %s\b' % n, line):
                print('%s:%d: %s' % (os.path.basename(f), i + 1, line.strip()))
