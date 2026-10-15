"""memscan.py: memcpy/memmove/memset/memcmp (and SaveGame-like byte copies) with a literal size, where the size is
the x86 size of the source or destination struct (or a multiple) and that struct is bigger on 64 bits."""
import os
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parents[1]) if Path(__file__).resolve().parent.name == 'phase8'
                else os.path.expanduser('~/wt/port/tools'))
import clang.cindex as ci  # noqa: E402
import port_layout as pl  # noqa: E402

D = (Path(__file__).resolve().parents[2] / 'src/legoland') if Path(__file__).resolve().parent.name == 'phase8' \
    else Path(os.path.expanduser('~/wt/port/src/legoland'))
FN = {'memcpy': (0, 1, 2), 'memmove': (0, 1, 2), 'memcmp': (0, 1, 2), 'memset': (0, None, 2)}


def strip(c):
    while c.kind in (ci.CursorKind.UNEXPOSED_EXPR, ci.CursorKind.PAREN_EXPR, ci.CursorKind.CSTYLE_CAST_EXPR):
        ch = list(c.get_children())
        if not ch:
            break
        c = ch[-1]
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
    return None


def pointee(c):
    """the struct (or array of structs) an argument points to, as (spelling, size)"""
    c = strip(c)
    t = c.type.get_canonical()
    if t.kind == ci.TypeKind.POINTER:
        p = t.get_pointee().get_canonical()
    elif t.kind in (ci.TypeKind.CONSTANTARRAY,):
        p = t.get_array_element_type().get_canonical()
    else:
        return None
    if p.kind == ci.TypeKind.CONSTANTARRAY:
        p = p.get_array_element_type().get_canonical()
    if p.kind == ci.TypeKind.RECORD and p.get_size() > 0:
        return p.spelling, p.get_size()
    return None


def scan(path, target):
    args = [a if not a.startswith('--target=') else '--target=' + target for a in pl.ARGS]
    tu = ci.Index.create().parse(str(path), args=args + ['-I%s' % D])
    out = {}

    def walk(c):
        if c.location.file is not None and Path(c.location.file.name).resolve() != path.resolve():
            return
        if c.kind == ci.CursorKind.CALL_EXPR and c.spelling in FN:
            a = list(c.get_arguments())
            di, si, ni = FN[c.spelling]
            if len(a) > ni:
                n = lit(a[ni])
                if n is not None:
                    types = []
                    for i in (di, si):
                        if i is not None:
                            p = pointee(a[i])
                            if p:
                                types.append(p)
                    out[(c.location.line, c.location.column)] = (c.spelling, n, types)
        for x in c.get_children():
            walk(x)
    walk(tu.cursor)
    return out


n_found = 0
for f in sorted(p.name for p in D.glob('*.c')):
    p = D / f
    a = scan(p, 'i686-pc-windows-msvc')
    b = scan(p, 'x86_64-pc-windows-msvc')
    lines = p.read_text(errors='replace').split('\n')
    for key, (fn, n, t32) in sorted(a.items()):
        t64 = b.get(key, (None, None, []))[2]
        for (name, s32), (_, s64) in zip(t32, t64):
            if s32 != s64 and s32 and n % s32 == 0 or (s32 != s64 and n == s32):
                n_found += 1
                print('%s:%d %s(.., %d) on %s: x86 0x%x, x64 0x%x\n      %s' % (f, key[0], fn, n, name, s32, s64,
                                                                           lines[key[0] - 1].strip()[:150]))
                break
print(n_found, 'found')
