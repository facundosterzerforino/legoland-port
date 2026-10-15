"""voidscan.py: views reached through void * (port tree), the casts castscan can't see.

For every void * slot -- a struct field (Struct.field), a global, a function parameter (func#n) -- it records the
struct types stored into it (assignments and call arguments, under casts to void *), then checks every
(struct V *)slot cast: V's scalar fields against each stored type's field at the same i686 offset; where the
x86_64 offsets differ it reports a hazard (a view that only lines up on 32 bits)."""
import collections
import os
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parents[1]) if Path(__file__).resolve().parent.name == 'phase8'
                else os.path.expanduser('~/wt/port/tools'))
import clang.cindex as ci  # noqa: E402
import port_layout as pl  # noqa: E402

D = (Path(__file__).resolve().parents[2] / 'src/legoland') if Path(__file__).resolve().parent.name == 'phase8' \
    else Path(os.path.expanduser('~/wt/port/src/legoland'))
files = [a for a in sys.argv[1:] if a.endswith('.c')] or sorted(p.name for p in D.glob('*.c'))


def strip(c):
    while c.kind in (ci.CursorKind.UNEXPOSED_EXPR, ci.CursorKind.PAREN_EXPR, ci.CursorKind.CSTYLE_CAST_EXPR):
        ch = list(c.get_children())
        if not ch:
            break
        c = ch[-1]
    return c


def strip_paren(c):
    while c.kind in (ci.CursorKind.UNEXPOSED_EXPR, ci.CursorKind.PAREN_EXPR):
        ch = list(c.get_children())
        if len(ch) != 1:
            break
        c = ch[0]
    return c


def is_void_ptr(t):
    t = t.get_canonical()
    return t.kind == ci.TypeKind.POINTER and t.get_pointee().get_canonical().kind == ci.TypeKind.VOID


def rec_of(t):
    t = t.get_canonical()
    if t.kind == ci.TypeKind.POINTER:
        p = t.get_pointee().get_canonical()
        if p.kind == ci.TypeKind.RECORD and p.get_size() > 0:
            return p
    return None


def slot_key(c, func):
    """the void * slot an expression names, or None"""
    c = strip_paren(c)
    if c.kind == ci.CursorKind.MEMBER_REF_EXPR:
        ch = list(c.get_children())
        if ch:
            base = ch[0].type.get_canonical()
            if base.kind == ci.TypeKind.POINTER:
                base = base.get_pointee().get_canonical()
            return '%s.%s' % (base.spelling, c.spelling)
    if c.kind == ci.CursorKind.DECL_REF_EXPR:
        d = c.referenced
        if d is not None:
            if d.kind == ci.CursorKind.PARM_DECL and func is not None:
                return '%s#%s' % (func, d.spelling)
            if d.kind == ci.CursorKind.VAR_DECL and d.semantic_parent is not None and \
                    d.semantic_parent.kind == ci.CursorKind.TRANSLATION_UNIT:
                return 'global %s' % d.spelling
    return None


def leaves(t, base=0, path=''):
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
        if e.get_canonical().kind in (ci.TypeKind.CHAR_S, ci.TypeKind.UCHAR, ci.TypeKind.SCHAR, ci.TypeKind.CHAR_U):
            return [(base, t.get_size(), path, 'pad')]
        for i in range(min(t.get_array_size(), 64)):
            out += leaves(e, base + i * e.get_size(), '%s[%d]' % (path, i))
    else:
        out.append((base, t.get_size(), path, 'x'))
    return out


def scan(path, target):
    args = [a if not a.startswith('--target=') else '--target=' + target for a in pl.ARGS]
    tu = ci.Index.create().parse(str(path), args=args + ['-I%s' % D])
    stores = collections.defaultdict(dict)   # key -> {type spelling: leaves}
    casts = []                               # (line, key, view spelling, view leaves)
    params = {}                              # func -> [param names]

    def walk(c, func):
        if c.location.file is not None and Path(c.location.file.name).resolve() != path.resolve():
            return
        if c.kind == ci.CursorKind.FUNCTION_DECL and c.is_definition():
            func = c.spelling
        if c.kind == ci.CursorKind.BINARY_OPERATOR:
            ch = list(c.get_children())
            toks = [t.spelling for t in c.get_tokens()]
            if len(ch) == 2 and is_void_ptr(ch[0].type):
                # '=' : the operator token right after the left side
                op = None
                for t in c.get_tokens():
                    if t.extent.start.offset >= ch[0].extent.end.offset:
                        op = t.spelling
                        break
                if op == '=':
                    k = slot_key(ch[0], func)
                    r = rec_of(strip(ch[1]).type)
                    if k and r is not None:
                        stores[k][r.spelling] = leaves(r)
        if c.kind == ci.CursorKind.CALL_EXPR and c.referenced is not None:
            callee = c.referenced
            pnames = [p.spelling for p in callee.get_arguments()] if callee.kind == ci.CursorKind.FUNCTION_DECL else []
            for i, a in enumerate(c.get_arguments()):
                if i < len(pnames) and is_void_ptr(a.type if a.kind != ci.CursorKind.UNEXPOSED_EXPR else a.type):
                    r = rec_of(strip(a).type)
                    if r is not None:
                        stores['%s#%s' % (callee.spelling, pnames[i])][r.spelling] = leaves(r)
        if c.kind == ci.CursorKind.CSTYLE_CAST_EXPR:
            v = rec_of(c.type)
            src = strip_paren(list(c.get_children())[-1])
            if v is not None and is_void_ptr(src.type):
                k = slot_key(src, func)
                if k:
                    casts.append((c.location.line, k, v.spelling, leaves(v)))
        for x in c.get_children():
            walk(x, func)
    walk(tu.cursor, None)
    return stores, casts


def compare(lv32, lv64, lr32, lr64):
    for (oa, sa, pa, ka), (oa64, _, _, _) in zip(lv32, lv64):
        if ka == 'pad':
            continue
        for i, (ob, sb, pb, kb) in enumerate(lr32):
            if ob == oa and kb != 'pad':
                if lr64[i][0] != oa64:
                    return (pa, oa, oa64, pb, lr64[i][0])
                break
    return None


all32 = collections.defaultdict(dict)
all64 = collections.defaultdict(dict)
casts32 = []
casts64 = {}
for f in files:
    p = D / f
    s32, c32 = scan(p, 'i686-pc-windows-msvc')
    s64, c64 = scan(p, 'x86_64-pc-windows-msvc')
    for k, v in s32.items():
        all32[k].update(v)
    for k, v in s64.items():
        all64[k].update(v)
    for x in c32:
        casts32.append((f,) + x)
    for x in c64:
        casts64[(f, x[0], x[1], x[2])] = x[3]
n = 0
seen = set()
for f, line, k, vname, lv32 in casts32:
    lv64 = casts64.get((f, line, k, vname))
    if lv64 is None or k not in all32:
        continue
    if vname in all32[k]:
        continue  # the slot holds several kinds of object and this is one of them
    for rname, lr32 in all32[k].items():
        if rname == vname or (f, vname, rname) in seen:
            continue
        lr64 = all64.get(k, {}).get(rname)
        if lr64 is None:
            continue
        bad = compare(lv32, lv64, lr32, lr64)
        if bad:
            seen.add((f, vname, rname))
            n += 1
            print('%s:%d (%s *)%s, which holds %s: %s at 0x%x, x64 0x%x; %s there is at x64 0x%x' % (
                f, line, vname, k, rname, bad[0], bad[1], bad[2], bad[3], bad[4]))
print(n, 'hazards')
