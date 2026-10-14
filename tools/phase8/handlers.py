"""handlers.py [slot...]: the functions assigned to each Ride callback slot (->slot = Func) and their signatures"""
import collections
import glob
import os
import re
import sys

os.chdir(os.path.expanduser('~/scratch/dv64/src/legoland'))
src = {f: open(f, errors='replace').read() for f in glob.glob('*.c')}
defs = {}
for f, s in src.items():
    for m in re.finditer(r'^[A-Za-z_][^;\n]*?\b(\w+)\(([^;{]*)\)\s*\{', s, re.M):
        defs.setdefault(m.group(1), (f, m.group(0)[:-1].strip()))
slots = collections.defaultdict(set)
for f, s in src.items():
    for m in re.finditer(r'->(cb_\w+|cb_sprite|cb_pre_render|load_hook|save_hook) = (?:\([^)]*\))?\s*(\w+);', s):
        slots[m.group(1)].add(m.group(2))
want = sys.argv[1:] or sorted(slots)
for slot in want:
    print('== %s (%d handlers)' % (slot, len(slots[slot])))
    for fn in sorted(slots[slot]):
        d = defs.get(fn)
        print('   %-40s %s' % (fn, d[1][:110] if d else '(no definition)'))
