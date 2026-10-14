# wshow.py <warnings file> <file filter> [max] [--all]: each 64-bit pointer warning with its code line
import os
import re
import sys

os.chdir(os.path.expanduser('~/wt/port'))
lines = open(sys.argv[1]).read().splitlines()
want = sys.argv[2]
limit = int(sys.argv[3]) if len(sys.argv) > 3 else 40
keep_all = '--all' in sys.argv
cache = {}
shown = 0
for l in lines:
    m = re.match(r'([^()]+)\((\d+),(\d+)\): warning: (.*) \[(-W[a-z-]+)\]', l)
    if not m or want not in m.group(1):
        continue
    if not keep_all and m.group(5) == '-Wincompatible-pointer-types':
        continue
    f, ln = m.group(1), int(m.group(2))
    if f not in cache:
        path = f if os.path.exists(f) else '/tmp/x64overlay/src/legoland/' + os.path.basename(f)
        cache[f] = open(path, errors='replace').read().split('\n')
    print('%s:%d  %s\n    %s' % (f.split('/')[-1], ln, m.group(4)[:110], cache[f][ln - 1].strip()[:150]))
    shown += 1
    if shown >= limit:
        break
