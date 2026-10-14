# vcompare.py baseline.json new.json: every function whose match changed (worse first), and the totals
import json
import sys


def load(p):
    out = {}
    for f in json.load(open(p))['data']:
        out[f['address']] = (f.get('name'), f.get('matching') or 0.0, bool(f.get('effective')))
    return out


a, b = load(sys.argv[1]), load(sys.argv[2])
worse, better = [], []
for addr in sorted(set(a) | set(b)):
    na, ma, ea = a.get(addr, (None, 0.0, False))
    nb, mb, eb = b.get(addr, (None, 0.0, False))
    # "effective" counts as a full match
    sa = 1.0 if ea else ma
    sb = 1.0 if eb else mb
    if abs(sa - sb) > 1e-9:
        (worse if sb < sa else better).append((sb - sa, addr, nb or na, sa, sb))
for d, addr, name, sa, sb in sorted(worse):
    print('WORSE  %s %-36s %6.2f%% -> %6.2f%%' % (addr, name, sa * 100, sb * 100))
for d, addr, name, sa, sb in sorted(better, reverse=True):
    print('better %s %-36s %6.2f%% -> %6.2f%%' % (addr, name, sa * 100, sb * 100))
print('%d worse, %d better, %d functions' % (len(worse), len(better), len(b)))
sys.exit(1 if worse else 0)
