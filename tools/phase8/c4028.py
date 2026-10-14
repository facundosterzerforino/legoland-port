"""c4028.py: for each C4028 site in the last dvcheck build, the definition line and the header prototype(s)"""
import glob
import os
import re

os.chdir(os.path.expanduser('~/scratch/dv64/src/legoland'))
sites = sorted(set(re.findall(r'legoland\\(\w+\.c)\((\d+)\) : warning C4028', open('/tmp/dv_build.log', errors='replace').read())))
hdr = {f: open(f, errors='replace').read() for f in glob.glob('*.h')}
seen = set()
for f, ln in sites:
    if (f, ln) in seen:
        continue
    seen.add((f, ln))
    L = open(f, errors='replace').read().split('\n')
    line = L[int(ln) - 1]
    print('%s:%s: %s' % (f, ln, line.strip()[:160]))
    m = re.search(r'(\w+)\(', line)
    if m:
        for h, s in hdr.items():
            for pm in re.finditer(r'^[^\n]*\b%s\([^;]*;' % m.group(1), s, re.M):
                print('    %s: %s' % (h, pm.group(0)[:160]))
