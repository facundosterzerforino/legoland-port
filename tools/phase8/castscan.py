"""castscan.py: casts between struct pointer types whose fields don't line up on 64 bits (port tree).

For each `(struct A *)e` where e has type `struct B *` (A != B, both complete), every scalar field of A is matched
with B's scalar field at the same i686 offset; if B has one there and its x86_64 offset differs from A's, the cast
is a 64-bit hazard (a view that only lines up on 32 bits). One line per (file, A, B) with the first such field."""
import os
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
import clang.cindex as ci  # noqa: E402
import port_layout as pl  # noqa: E402

D = Path(__file__).resolve().parents[2] / 'src/legoland'
files = [a for a in sys.argv[1:] if a.endswith('.c')] or sorted(p.name for p in D.glob('*.c'))


def strip(c):
    while c.kind in (ci.CursorKind.UNEXPOSED_EXPR, ci.CursorKind.PAREN_EXPR):
        ch = list(c.get_children())
        if len(ch) != 1:
            break
        c = ch[0]
    return c


def leaves(t, base=0, path=''):
    """[(offset, size, path)] of the scalar fields of record type t"""
    t = t.get_canonical()
    out = []
    if t.kind == ci.TypeKind.RECORD:
        for f in t.get_fields():
            o = t.get_offset(f.spelling)
            if o < 0:
                continue
            out += leaves(f.type, base + o // 8, path + '.' + (f.spelling or '(anon)'))
    elif t.kind == ci.TypeKind.CONSTANTARRAY:
        e = t.get_array_element_type()
        n = t.get_array_size()
        if e.get_canonical().kind in (ci.TypeKind.CHAR_S, ci.TypeKind.UCHAR, ci.TypeKind.SCHAR, ci.TypeKind.CHAR_U):
            return [(base, t.get_size(), path, 'pad')]
        for i in range(min(n, 64)):
            out += leaves(e, base + i * e.get_size(), '%s[%d]' % (path, i))
    else:
        out.append((base, t.get_size(), path, 'ptr' if t.kind == ci.TypeKind.POINTER else 'scalar'))
    return out


def scan(path, target):
    args = [a if not a.startswith('--target=') else '--target=' + target for a in pl.ARGS]
    tu = ci.Index.create().parse(str(path), args=args + ['-I%s' % D])
    out = {}

    def walk(c):
        if c.location.file is not None and Path(c.location.file.name).resolve() != path.resolve():
            return
        if c.kind == ci.CursorKind.CSTYLE_CAST_EXPR:
            to = c.type.get_canonical()
            src = strip(list(c.get_children())[-1])
            st = src.type.get_canonical()
            if to.kind == ci.TypeKind.POINTER and st.kind == ci.TypeKind.POINTER:
                a = to.get_pointee().get_canonical()
                b = st.get_pointee().get_canonical()
                if (a.kind == ci.TypeKind.RECORD and b.kind == ci.TypeKind.RECORD and a.spelling != b.spelling
                        and a.get_size() > 0 and b.get_size() > 0):
                    out.setdefault((a.spelling, b.spelling), (c.location.line, leaves(a), leaves(b)))
        for x in c.get_children():
            walk(x)
    walk(tu.cursor)
    return out


OK = set()
for line in open(Path(__file__).resolve().parent / 'castscan_ok.txt'):
    line = line.strip()
    if line and not line.startswith('#'):
        f_, rest = line.split(' ', 1)
        a_, b_ = rest.split(' struct ', 1) if rest.count('struct ') == 2 else (rest, '')
        OK.add((f_, a_.strip(), 'struct ' + b_.strip()))
total = 0
reviewed = 0
for f in files:
    p = D / f
    a32 = scan(p, 'i686-pc-windows-msvc')
    a64 = scan(p, 'x86_64-pc-windows-msvc')
    for key, (line, la32, lb32) in sorted(a32.items(), key=lambda kv: kv[1][0]):
        if key not in a64:
            continue
        _, la64, lb64 = a64[key]
        bad = None
        for (oa, sa, pa, ka), (oa64, _, pa64, _) in zip(la32, la64):
            if ka == 'pad':
                continue
            # B's field at A's x86 offset
            match = [i for i, (ob, sb, pb, kb) in enumerate(lb32) if ob == oa and kb != 'pad']
            if not match:
                continue
            ob64 = lb64[match[0]][0]
            if ob64 != oa64:
                bad = (pa, oa, oa64, lb32[match[0]][2], ob64)
                break
        if bad and (f, key[0], key[1]) in OK:
            reviewed += 1
            continue
        if bad:
            total += 1
            print('%s:%d (%s *) from %s *: %s at 0x%x, x64 0x%x; %s there is at x64 0x%x' % (
                f, line, key[0], key[1], bad[0], bad[1], bad[2], bad[3], bad[4]))
print(total, 'hazards,', reviewed, 'reviewed (castscan_ok.txt)')
