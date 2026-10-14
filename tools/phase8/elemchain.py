"""elemchain.py file.c [--dry]: retargets a file's views of the element -> class -> layer/riders chain:
a struct {pad[0xc]; T *f} (an Element) -> Element (f -> ride or data), T if it is laid out like struct Ride -> Ride
(its 0x64 field -> layer, 0xcc -> riders), the 0x64 field's struct -> Sprite and the 0xcc field's -> RideNode.
Prints and runs the retarget.py command."""
import json
import os
import re
import subprocess
import sys

R = json.load(open('/tmp/views.json'))['records']
f = sys.argv[1]
recs = {r['name']: r for r in R.values() if r['file'] == f}


def named(r):
    return [x for x in r['fields'] if not x['pad'] and x['offset'] is not None]


def pointee(t):
    m = re.match(r'struct (\w+) \*$', t)
    return m.group(1) if m else None


pairs, maps = [], []
done = set()
for name, r in sorted(recs.items()):
    fs = named(r)
    if r['size'] != 0x10 or len(fs) != 1 or fs[0]['offset'] != 0xc:
        continue
    fld = fs[0]
    p = pointee(fld['type'])
    if p is None and fld['type'] not in ('void *',):
        continue
    pairs.append('%s:%s=Element' % (name, f))
    maps.append('%s.%s=%s' % (name, fld['name'], 'ride' if p else 'data'))
    done.add(name)
    if p and p in recs and p not in done:
        cr = recs[p]
        cfs = named(cr)
        if cr['size'] <= 0xd0 and all(x['offset'] in (0, 4, 8, 0xc, 0x10, 0x14, 0x18, 0x1c, 0x20, 0x24, 0x25, 0x26, 0x2a, 0x2c,
                                                      0x2e, 0x32, 0x33, 0x36, 0x38, 0x3a, 0x3c, 0x40, 0x44, 0x48, 0x4c,
                                                      0x64, 0x68, 0x6c, 0x78, 0xc4, 0xc8, 0xcc) for x in cfs):
            pairs.append('%s:%s=Ride' % (p, f))
            done.add(p)
            for x in cfs:
                o = x['offset']
                if o == 0x64:
                    maps.append('%s.%s=layer' % (p, x['name']))
                    sp = pointee(x['type'])
                    if sp and sp in recs and sp not in done and recs[sp]['size'] <= 0x20:
                        pairs.append('%s:%s=Sprite' % (sp, f))
                        done.add(sp)
                elif o == 0xcc:
                    maps.append('%s.%s=riders' % (p, x['name']))
                    np_ = pointee(x['type'])
                    if np_ and np_ in recs and np_ not in done and recs[np_]['size'] <= 0x14:
                        pairs.append('%s:%s=RideNode' % (np_, f))
                        done.add(np_)
                elif o == 0x3c and x['type'] in ('int', 'unsigned int'):
                    maps.append('%s.%s=footprint.x0' % (p, x['name']))
                elif o == 0x40:
                    maps.append('%s.%s=footprint.y0' % (p, x['name']))
                elif o == 0x44:
                    maps.append('%s.%s=footprint.x1' % (p, x['name']))
                elif o == 0x48:
                    maps.append('%s.%s=footprint.y1' % (p, x['name']))
                elif o == 0x1c:
                    maps.append('%s.%s=flags' % (p, x['name']))
                elif o == 0x14:
                    maps.append('%s.%s=field_14' % (p, x['name']))
                elif o == 0x18:
                    maps.append('%s.%s=field_18' % (p, x['name']))
                elif o == 0x10:
                    maps.append('%s.%s=y' % (p, x['name']))
                elif o == 0xc:
                    maps.append('%s.%s=x' % (p, x['name']))
        else:
            print('# %s: %s is not laid out like a Ride: %s' % (f, p, ' '.join('%x:%s' % (x['offset'], x['name']) for x in cfs)))
args = pairs + maps
print(' '.join(args))
if args:
    cmd = ['bash', os.path.join(os.path.dirname(os.path.abspath(__file__)), 'rt.sh')] + args + (['--dry'] if '--dry' in sys.argv else [])
    subprocess.run(cmd)
