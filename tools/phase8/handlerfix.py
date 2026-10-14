"""handlerfix.py [--dry]: the functions assigned to struct Ride's callback slots get pointer parameters where the slot
passes a pointer (an integer parameter there is retyped, in the definition and in the headers' prototypes);
parameters that point to another struct than the slot's are listed."""
import collections
import os
import re
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import retarget as rt  # noqa: E402

ci = rt.ci
os.chdir(rt.SRC)
INTS = (ci.TypeKind.INT, ci.TypeKind.UINT, ci.TypeKind.LONG, ci.TypeKind.ULONG, ci.TypeKind.SHORT, ci.TypeKind.USHORT)
E = 'struct Element *'
SLOTS = {
    'cb_8c': [],
    'cb_90': [E, 'struct Point *', None],
    'cb_94': [E, 'struct Point *'],
    'cb_98': [E, 'int *'],
    'cb_9c': [E, None, 'struct Cursor *'],
    'cb_sprite': [E, None],
    'cb_a4': [E],
    'cb_pre_render': [E],
    'cb_ac': [E],
    'cb_b0': [E, None, None, 'int *', 'void *', None],
    'load_hook': [E],
    'save_hook': [E],
    'cb_c0': [E, None],
}


def main():
    handlers = collections.defaultdict(set)
    for f in rt.all_c():
        s = open(f, errors='replace').read()
        for m in re.finditer(r'->(cb_\w+|cb_sprite|cb_pre_render|load_hook|save_hook) = (?:\([^)]*\))?\s*(\w+);', s):
            if m.group(1) in SLOTS:
                handlers[m.group(2)].add(m.group(1))
    names = set(handlers)
    edits = collections.defaultdict(set)
    protos = collections.defaultdict(list)
    views = []
    for f in rt.all_c():
        s = open(f, errors='replace').read()
        if not any(re.search(r'^[^\n;]*\b%s\(' % n, s, re.M) for n in names):
            continue
        tu = rt.parse(f)
        for c in tu.cursor.get_children():
            if c.kind != ci.CursorKind.FUNCTION_DECL or not c.is_definition() or c.spelling not in names:
                continue
            if os.path.basename(c.location.file.name) != f:
                continue
            for slot in handlers[c.spelling]:
                canon = SLOTS[slot]
                for i, p in enumerate(c.get_arguments()):
                    if i >= len(canon) or canon[i] is None:
                        continue
                    t = p.type.get_canonical()
                    if t.kind in INTS:
                        line = open(f, errors='replace').read().split('\n')[p.location.line - 1]
                        m = None
                        for mm in re.finditer(r'((?:unsigned |signed )?(?:int|long|short)\s+)%s\b' % re.escape(p.spelling), line):
                            m = mm
                        if m is None:
                            print('cannot retype %s.%s at %s:%d' % (c.spelling, p.spelling, f, p.location.line))
                            continue
                        nt = canon[i]
                        edits[f].add((p.location.line, m.start(), m.start(1) + len(m.group(1)), nt.rstrip() + ('' if nt.endswith('*') else ' ')))
                        protos[c.spelling].append((i, nt))
                    elif t.kind == ci.TypeKind.POINTER and canon[i].startswith('struct '):
                        want = canon[i].split()[1]
                        pt = t.get_pointee().get_canonical()
                        if pt.kind == ci.TypeKind.RECORD and pt.get_declaration().spelling != want:
                            views.append('%s(%s) param %d: %s (slot %s wants %s)' % (c.spelling, f, i, p.type.spelling, slot,
                                                                                     canon[i]))
    for f, es in edits.items():
        L = open(f).read().split('\n')
        for line, a, b, t in sorted(es, reverse=True):
            L[line - 1] = L[line - 1][:a] + t + L[line - 1][b:]
        if '--dry' not in sys.argv:
            open(f, 'w').write('\n'.join(L))
    n = 0
    for fn, changes in protos.items():
        for hf in sorted(os.listdir('.')):
            if not hf.endswith('.h'):
                continue
            s = open(hf).read()
            m = re.search(r'^([^\n;]*\b%s\()([^;]*)(\);)$' % fn, s, re.M)
            if not m:
                continue
            params = [p.strip() for p in m.group(2).split(',')]
            for idx, nt in changes:
                if idx < len(params):
                    pm = re.match(r'(?:const )?(?:unsigned |signed )?(?:int|long|short)\s*(\w*)$', params[idx])
                    if pm:
                        params[idx] = (nt.rstrip() + ('' if nt.endswith('*') else ' ') + pm.group(1)).strip()
            s2 = s[:m.start(2)] + ', '.join(params) + s[m.end(2):]
            if s2 != s and '--dry' not in sys.argv:
                open(hf, 'w').write(s2)
                n += 1
    print('%d parameter(s) retyped in %d file(s), %d prototype(s); pointer views to review:' % (
        sum(len(e) for e in edits.values()), len(edits), n))
    for v in sorted(set(views)):
        print('   ', v)


if __name__ == '__main__':
    main()
