"""offscan.py [file.c...]: constant byte/word offsets into structs through casts, in the decomp tree (x64 hazards).

Finds `(T *)p + k`, `((T *)p)[k]` and `(T *)((char *)p + k)` where p points to a struct and T is a scalar (char,
short, int, ...). For each it works out the x86 byte offset, the field of the struct there, and that field's
offset on x86_64; it prints the ones whose offset differs (DIFF) and, with -a, all of them."""
import os
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
import clang.cindex as ci  # noqa: E402
import port_layout as pl  # noqa: E402

D = Path(__file__).resolve().parents[2] / 'src/legoland'
ALL = '-a' in sys.argv
files = [a for a in sys.argv[1:] if a.endswith('.c')] or sorted(p.name for p in D.glob('*.c')
                                                                 if p.name not in ('crt.c', 'imports.c', 'bootstrap.c'))
INC = ['-I%s' % D]


def strip(c):
    while c.kind in (ci.CursorKind.UNEXPOSED_EXPR, ci.CursorKind.PAREN_EXPR):
        ch = list(c.get_children())
        if len(ch) != 1:
            break
        c = ch[0]
    return c


def lit(c):
    c = strip(c)
    if c.kind == ci.CursorKind.INTEGER_LITERAL:
        t = [x.spelling for x in c.get_tokens()]
        if t:
            try:
                return int(t[0].rstrip('uUlL'), 0)
            except ValueError:
                return None
    if c.kind == ci.CursorKind.UNARY_OPERATOR:
        t = [x.spelling for x in c.get_tokens()]
        if t and t[0] == '-':
            v = lit(list(c.get_children())[0])
            return -v if v is not None else None
    return None


def leaf_at(t, off):
    """(path, offset) of the scalar field of record t that covers byte off"""
    t = t.get_canonical()
    if t.kind == ci.TypeKind.RECORD:
        best = None
        for f in t.get_fields():
            o = t.get_offset(f.spelling)
            if o < 0:
                continue
            o //= 8
            sz = f.type.get_size()
            if o <= off < o + max(sz, 1):
                sub = leaf_at(f.type, off - o)
                p = (f.spelling or '(anon)') + ('.' + sub[0] if sub[0] else '')
                cand = (p, o + sub[1])
                if best is None:
                    best = cand
        return best or ('?', off)
    if t.kind == ci.TypeKind.CONSTANTARRAY:
        e = t.get_array_element_type()
        es = e.get_size()
        i = off // es
        sub = leaf_at(e, off - i * es)
        return ('[%d]' % i + ('.' + sub[0] if sub[0] else ''), i * es + sub[1])
    return ('', 0)


def offset_of(t, path):
    """offset of path (as leaf_at prints it) in record t, on this parse's target"""
    t = t.get_canonical()
    off = 0
    for part in path.replace('[', '.[').split('.'):
        if not part:
            continue
        if part.startswith('['):
            e = t.get_array_element_type()
            off += int(part[1:-1]) * e.get_size()
            t = e.get_canonical()
            continue
        for f in t.get_fields():
            if (f.spelling or '(anon)') == part:
                off += t.get_offset(f.spelling) // 8
                t = f.type.get_canonical()
                break
        else:
            return None
    return off


def scan(path, target):
    args = [a if not a.startswith('--target=') else '--target=' + target for a in pl.ARGS]
    tu = ci.Index.create().parse(str(path), args=args + INC)
    out = {}

    def record(where, base_t, k):
        rec = base_t.get_canonical()
        if rec.kind != ci.TypeKind.RECORD or rec.get_size() <= 0:
            return
        out[where] = (rec, k)

    def cast_info(c):
        """(record type, element size) for a cast (T *)p with p a struct pointer and T scalar"""
        c = strip(c)
        if c.kind != ci.CursorKind.CSTYLE_CAST_EXPR:
            return None
        to = c.type.get_canonical()
        if to.kind != ci.TypeKind.POINTER:
            return None
        el = to.get_pointee().get_canonical()
        if el.kind == ci.TypeKind.RECORD:
            return None
        src = strip(list(c.get_children())[-1])
        st = src.type.get_canonical()
        if st.kind != ci.TypeKind.POINTER:
            return None
        rec = st.get_pointee().get_canonical()
        if rec.kind != ci.TypeKind.RECORD:
            return None
        es = el.get_size() if el.kind != ci.TypeKind.VOID else 1
        return rec, max(es, 1)

    def walk(c):
        if c.location.file is not None and Path(c.location.file.name).resolve() != path.resolve():
            return
        ch = list(c.get_children())
        where = (c.location.line, c.location.column)
        if c.kind == ci.CursorKind.BINARY_OPERATOR and len(ch) == 2:
            toks = [t.spelling for t in c.get_tokens()]
            ci_ = cast_info(ch[0])
            k = lit(ch[1])
            if ci_ and k is not None and '+' in toks:
                record(where, ci_[0], k * ci_[1])
        if c.kind == ci.CursorKind.ARRAY_SUBSCRIPT_EXPR and len(ch) == 2:
            ci_ = cast_info(ch[0])
            k = lit(ch[1])
            if ci_ and k is not None and k != 0:
                record(where, ci_[0], k * ci_[1])
        for x in ch:
            walk(x)
    walk(tu.cursor)
    return out


n = 0
for f in files:
    p = D / f
    a = scan(p, 'i686-pc-windows-msvc')
    b = scan(p, 'x86_64-pc-windows-msvc')
    lines = p.read_text(errors='replace').split('\n')
    for where in sorted(a):
        rec, k = a[where]
        fld, fo = leaf_at(rec, k)
        rest = k - fo
        rec64 = b.get(where, (None, None))[0]
        o64 = offset_of(rec64, fld) if rec64 is not None and fld != '?' else None
        diff = o64 is None or o64 != fo
        if diff or ALL:
            n += 1
            print('%s %s:%d %s +0x%x = %s%s (x64 %s)\n      %s' % ('DIFF' if diff else 'same', f, where[0],
                  rec.spelling, k, fld, '+%d' % rest if rest else '', '0x%x' % o64 if o64 is not None else '?',
                  lines[where[0] - 1].strip()[:150]))
print(n, 'found')
