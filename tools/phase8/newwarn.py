"""newwarn.py save|show [file.c]: MSVC6 warnings of /tmp/dvb.log keyed by (file, code, message, source line text);
save stores them as the reference, show prints the ones not in the reference (with line numbers)."""
import json
import os
import re
import sys

SRC = os.path.expanduser('~/scratch/dv64/src/legoland/')
REF = '/tmp/dvb_ref.json'
cache = {}


def warnings():
    out = []
    for l in open('/tmp/dvb.log', errors='replace'):
        m = re.search(r'src\\legoland\\(\w+\.[ch])\((\d+)\) : (warning C\d+|error C\d+): (.*)', l)
        if not m:
            continue
        f, ln = m.group(1), int(m.group(2))
        if f not in cache:
            cache[f] = open(SRC + f, errors='replace').read().split('\n')
        text = cache[f][ln - 1].strip() if ln <= len(cache[f]) else ''
        out.append((f, ln, m.group(3), m.group(4).strip(), text))
    return out


w = warnings()
if sys.argv[1] == 'save':
    json.dump(sorted({(f, c, msg, t) for f, ln, c, msg, t in w}), open(REF, 'w'))
    print(len(w), 'warnings saved')
else:
    ref = {tuple(x) for x in json.load(open(REF))}
    only = sys.argv[2] if len(sys.argv) > 2 else None
    seen = set()
    for f, ln, c, msg, t in sorted(w, key=lambda x: (x[0], x[1])):
        if (f, c, msg, t) in ref or (only and f != only) or (f, ln, c, msg) in seen:
            continue
        seen.add((f, ln, c, msg))
        print('%s:%d %s: %s\n      %s' % (f, ln, c, msg[:100], t[:140]))
