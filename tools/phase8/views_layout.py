"""views_layout.py Name[:file] ...: the cluster each named struct is in (or just the named structs with --only), as one
table by offset: each member's field there (name type size), plus the casts that link them."""
import collections
import json
import sys

v = json.load(open('/tmp/views.json'))
R = v['records']
only = '--only' in sys.argv
names = [a for a in sys.argv[1:] if not a.startswith('--')]
parent = {}


def find(x):
    parent.setdefault(x, x)
    while parent[x] != x:
        parent[x] = parent[parent[x]]
        x = parent[x]
    return x


for a, b, loc in v['edges']:
    parent[find(a)] = find(b)


def match(key, n):
    return key.startswith(n + ':') if ':' not in n else key.startswith(n.split(':')[0] + ':') and (':' + n.split(':')[1] + ':') in key


want = set()
for n in names:
    for k in set(R) | set(parent):
        if match(k, n):
            want |= {k} if only else {x for x in set(parent) | {k} if find(x) == find(k)}
members = sorted((m for m in want if m in R), key=lambda m: -R[m]['size'])
offs = collections.defaultdict(dict)
for m in members:
    for f in R[m]['fields']:
        if f['offset'] is not None:
            offs[f['offset']][m] = f
short = {m: '%s@%d' % (m.split(':')[0], R[m]['line']) for m in members}
for m in members:
    print('%-28s size %s  %s:%d' % (short[m], hex(R[m]['size']), R[m]['file'], R[m]['line']))
print()
for o in sorted(offs):
    cells = []
    for m in members:
        f = offs[o].get(m)
        if f:
            cells.append('%s: %s%s %s' % (short[m], '~' if f['pad'] else '', f['name'], f['type'] if not f['pad']
                                          else '[%#x]' % f['size']))
    print('%#6x  %s' % (o, ' | '.join(cells)))
print()
for a, b, loc in v['edges']:
    if a in want and b in want:
        print('cast %s -> %s at %s' % (a.split(':')[0], b.split(':')[0], loc))
