"""allocscan.py [srcdir]: every allocation (malloc/calloc/realloc/FUN_004775b0) whose size is a literal, or a
product with a literal, in the port tree, parsed for i686 and x86_64. Reports the allocation's destination type
(the cast, or the assigned lvalue's type), the pointee's size on both targets and whether the literal covers it.
Prints only the allocations that may be short on x64."""
import os
import sys
from pathlib import Path

sys.path.insert(0, os.path.expanduser('~/wt/port/tools'))
import clang.cindex as ci  # noqa: E402
import port_layout as pl  # noqa: E402

ROOT = Path(os.path.expanduser('~/wt/port'))
SRC = ROOT / 'src/legoland'
ALLOC = {'malloc': 0, 'calloc': None, 'realloc': 1, 'FUN_004775b0': 0}


def lit(c):
    """the integer value of an integer literal cursor (through parens/casts), or None"""
    while c.kind in (ci.CursorKind.PAREN_EXPR, ci.CursorKind.UNEXPOSED_EXPR, ci.CursorKind.CSTYLE_CAST_EXPR):
        ch = list(c.get_children())
        if not ch:
            return None
        c = ch[-1]
    if c.kind == ci.CursorKind.INTEGER_LITERAL:
        t = [x.spelling for x in c.get_tokens()]
        if t:
            try:
                return int(t[0].rstrip('uUlL'), 0)
            except ValueError:
                return None
    return None


def size_kind(arg):
    """('lit', n) or ('mul', n) for 'x * n' / 'n * x', or None"""
    v = lit(arg)
    if v is not None:
        return ('lit', v)
    c = arg
    while c.kind in (ci.CursorKind.PAREN_EXPR, ci.CursorKind.UNEXPOSED_EXPR):
        ch = list(c.get_children())
        if len(ch) != 1:
            break
        c = ch[0]
    if c.kind == ci.CursorKind.BINARY_OPERATOR:
        toks = [t.spelling for t in c.get_tokens()]
        a, b = list(c.get_children())
        op = None
        # the operator token: the first token after a's extent
        for t in c.get_tokens():
            if t.extent.start.offset >= a.extent.end.offset:
                op = t.spelling
                break
        if op == '*':
            for x in (a, b):
                v = lit(x)
                if v is not None:
                    return ('mul', v)
    return None


def dest_type(call, parents):
    """the pointer type the allocation is converted to: a cast around it, or the assignment/initialisation target"""
    for p in reversed(parents):
        if p.kind == ci.CursorKind.CSTYLE_CAST_EXPR:
            return p.type
        if p.kind in (ci.CursorKind.PAREN_EXPR, ci.CursorKind.UNEXPOSED_EXPR):
            continue
        if p.kind == ci.CursorKind.BINARY_OPERATOR:
            ch = list(p.get_children())
            if ch and ch[0].type.get_canonical().kind == ci.TypeKind.POINTER:
                return ch[0].type
            return None
        if p.kind == ci.CursorKind.VAR_DECL:
            return p.type
        return None
    return None


def scan(path, target):
    args = [a if not a.startswith('--target=') else '--target=' + target for a in pl.ARGS]
    tu = ci.Index.create().parse(str(path), args=args)
    out = {}

    def walk(c, parents):
        if c.location.file is not None and Path(c.location.file.name).resolve() != path.resolve():
            return
        if c.kind == ci.CursorKind.CALL_EXPR and c.spelling in ALLOC:
            args = list(c.get_arguments())
            idx = ALLOC[c.spelling]
            cand = []
            if idx is None:
                cand = args[:2]
            elif idx < len(args):
                cand = [args[idx]]
            for a in cand:
                sk = size_kind(a)
                if sk is None:
                    continue
                t = dest_type(c, parents)
                pointee = None
                if t is not None and t.get_canonical().kind == ci.TypeKind.POINTER:
                    pointee = t.get_canonical().get_pointee()
                key = (c.location.line, c.location.column)
                out[key] = (sk, t.spelling if t is not None else '?',
                            pointee.get_size() if pointee is not None else None,
                            pointee.spelling if pointee is not None else '?')
        for ch in c.get_children():
            walk(ch, parents + [c])
    walk(tu.cursor, [])
    return out


files = sorted(SRC.glob('*.c'))
for f in files:
    a = scan(f, 'i686-pc-windows-msvc')
    b = scan(f, 'x86_64-pc-windows-msvc')
    for key in sorted(a):
        (kind, n), tname, s32, pname = a[key]
        s64 = b.get(key, (None, None, None, None))[2]
        if s32 is None or s32 <= 0 or s64 is None or s64 <= 0:
            if kind == 'lit' or n == 4:
                print('%s:%d %s %s(0x%x) -> %s (unknown size)' % (f.name, key[0], 'size' if kind == 'lit' else 'n *',
                                                                     kind, n, tname))
            continue
        if kind == 'lit':
            bad = n < s64 or n != s32
            if bad:
                print('%s:%d literal 0x%x -> %s: x86 0x%x x64 0x%x%s' % (f.name, key[0], n, pname, s32, s64,
                                                                          '' if n < s64 else ' (x86 mismatch only)'))
        else:
            if n != s64:
                print('%s:%d n * %d -> %s: element x86 %d x64 %d%s' % (f.name, key[0], n, pname, s32, s64,
                                                                        '  SHORT ON x64' if n < s64 else ''))
