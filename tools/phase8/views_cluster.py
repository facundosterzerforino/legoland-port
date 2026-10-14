"""views_cluster.py [filter]: groups /tmp/views.json's structs into clusters (structs cast into each other) and shows
each cluster: members, sizes, pad fields, where the casts are, and whether the members' named fields agree."""
import collections
import json
import sys

v = json.load(open('/tmp/views.json'))
R = v['records']
parent = {}


def find(x):
    parent.setdefault(x, x)
    while parent[x] != x:
        parent[x] = parent[parent[x]]
        x = parent[x]
    return x


for a, b, loc in v['edges']:
    parent[find(a)] = find(b)
clusters = collections.defaultdict(set)
for x in list(parent):
    clusters[find(x)].add(x)
casts = collections.defaultdict(list)
for a, b, loc in v['edges']:
    casts[find(a)].append(loc)


def conflicts(members):
    """named, non-pad fields at the same offset with different sizes or that partly overlap"""
    spans = []
    for m in members:
        for f in R.get(m, {}).get('fields', []):
            if f['offset'] is not None and not f['pad'] and f['name']:
                spans.append((f['offset'], f['offset'] + max(f['size'], 0), m.split(':')[0], f['name'], f['type']))
    spans.sort()
    bad = 0
    for i, (s, e, m, n, t) in enumerate(spans):
        for s2, e2, m2, n2, t2 in spans[i + 1:]:
            if s2 >= e:
                break
            if (s2, e2) != (s, e):
                bad += 1
    return bad


flt = sys.argv[1] if len(sys.argv) > 1 else ''
rows = sorted(clusters.values(), key=lambda c: -len(c))
print('%d clusters, %d structs in them' % (len(rows), sum(len(c) for c in rows)))
for c in rows:
    files = collections.Counter(l.split(':')[0] for l in casts[find(next(iter(c)))])
    if flt and not any(flt in m for m in c) and flt not in files:
        continue
    print('== %d members, %d casts in %s, partial overlaps %d' % (
        len(c), sum(files.values()), dict(files.most_common(4)), conflicts(c)))
    for m in sorted(c, key=lambda m: -R.get(m, {}).get('size', 0)):
        r = R.get(m)
        if r is None:
            print('   %s (no definition)' % m)
            continue
        pads = sum(1 for f in r['fields'] if f['pad'])
        print('   %-45s size %-6s %d fields, %d pads' % (m, hex(r['size']) if r['size'] > 0 else r['size'],
                                                       len(r['fields']), pads))
