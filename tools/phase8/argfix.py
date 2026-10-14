"""argfix.py Func:index ... [--dry]: makes the callers of functions whose parameter became a pointer pass pointers,
in ~/scratch/dv64/src/legoland. For each call's argument `index` (0-based), where the parameter's type is a pointer:
  (unsigned int)E / (int)E / (void *)E with E a pointer            -> E (the cast removed)
  V or &V, V a local variable or parameter of an integer type      -> V's declaration retyped (the parameter's
                                                                      pointee type for &V, its type for V); a
                                                                      parameter is retyped in its function's
                                                                      definition and in the headers' prototypes
Everything else is listed for a manual fix."""
import collections
import os
import re
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import retarget as rt  # noqa: E402

ci = rt.ci
os.chdir(rt.SRC)

INTS = (ci.TypeKind.INT, ci.TypeKind.UINT, ci.TypeKind.LONG, ci.TypeKind.ULONG)


def strip(e):
    while e.kind in (ci.CursorKind.UNEXPOSED_EXPR, ci.CursorKind.PAREN_EXPR) and len(list(e.get_children())) == 1:
        e = list(e.get_children())[0]
    return e


def text_of(ext):
    lines = open(ext.start.file.name, errors='replace').read().split('\n')
    if ext.start.line == ext.end.line:
        return lines[ext.start.line - 1][ext.start.column - 1:ext.end.column - 1]
    return None


def type_text(t):
    s = t.spelling
    return s.replace('Element *', 'struct Element *') if s.startswith('Element') else s


def main():
    wants = {}
    for a in sys.argv[1:]:
        if a.startswith('--'):
            continue
        fn, idx = a.split(':')
        wants[fn] = [int(i) for i in idx.split(',')]
    edits = collections.defaultdict(set)  # file -> (line, col0, col1, text)
    decls = {}  # decl cursor hash -> (cursor, new type text)
    manual = []
    globals_ = {}
    for f in rt.all_c():
        s = open(f, errors='replace').read()
        if not any(re.search(r'\b%s\s*\(' % fn, s) for fn in wants):
            continue
        tu = rt.parse(f)
        for c in tu.cursor.walk_preorder():
            if c.kind != ci.CursorKind.CALL_EXPR or c.spelling not in wants:
                continue
            if c.location.file is None or os.path.basename(c.location.file.name) != f:
                continue
            fdecl = c.referenced
            args = list(c.get_arguments())
            for i in wants[c.spelling]:
                if i >= len(args):
                    continue
                ptype = None
                if fdecl is not None:
                    ps = list(fdecl.get_arguments())
                    if i < len(ps):
                        ptype = ps[i].type
                if ptype is None or ptype.get_canonical().kind != ci.TypeKind.POINTER:
                    continue
                arg = strip(args[i])
                where = '%s:%d %s' % (f, c.location.line, text_of(c.extent) or c.spelling)
                if arg.kind == ci.CursorKind.CSTYLE_CAST_EXPR:
                    inner = strip(list(arg.get_children())[-1])
                    if inner.type.get_canonical().kind in (ci.TypeKind.POINTER, ci.TypeKind.CONSTANTARRAY):
                        t = text_of(inner.extent)
                        if t is not None and arg.extent.start.line == arg.extent.end.line:
                            edits[f].add((arg.extent.start.line, arg.extent.start.column - 1, arg.extent.end.column - 1, t))
                            continue
                    arg = inner
                target = None
                newtype = None
                refs = (ci.CursorKind.DECL_REF_EXPR, ci.CursorKind.MEMBER_REF_EXPR)
                if arg.kind == ci.CursorKind.UNARY_OPERATOR and text_of(arg.extent or '') and text_of(arg.extent).startswith('&'):
                    v = strip(list(arg.get_children())[0])
                    if v.kind in refs and v.referenced is not None and \
                            v.referenced.type.get_canonical().kind in INTS:
                        target = v.referenced
                        newtype = ptype.get_pointee().spelling
                        if ptype.get_pointee().get_canonical().kind == ci.TypeKind.VOID:
                            target = None
                elif arg.kind in refs and arg.referenced is not None and \
                        arg.referenced.type.get_canonical().kind in INTS:
                    target = arg.referenced
                    newtype = ptype.spelling
                if target is not None and target.kind == ci.CursorKind.VAR_DECL and (
                        target.semantic_parent is None or target.semantic_parent.kind != ci.CursorKind.FUNCTION_DECL):
                    globals_.setdefault(target.spelling, newtype)
                    continue
                if target is not None and target.kind in (ci.CursorKind.VAR_DECL, ci.CursorKind.PARM_DECL,
                                                          ci.CursorKind.FIELD_DECL):
                    decls[target.hash] = (target, newtype)
                    continue
                if strip(args[i]).type.get_canonical().kind in INTS:
                    manual.append(where)
    # globals: extern in globals.h, definition in globals.c
    for name, newtype in globals_.items():
        nt = newtype.rstrip()
        for gf in ('globals.h', 'globals.c'):
            g = open(gf).read()
            pat = r'^((?:extern )?)(?:unsigned |signed )?(?:int|long)\s+' + re.escape(name) + r'\b'
            rep = (lambda m, nt=nt: m.group(1) + nt + ('' if nt.endswith('*') else ' ') + name)
            g2, n = re.subn(pat, rep, g, count=1, flags=re.M)
            if n and '--dry' not in sys.argv:
                open(gf, 'w').write(g2)
            print('retype global %s -> %s in %s: %s' % (name, newtype, gf, 'yes' if n else 'NOT FOUND'))
    # declarations: replace the type before the name
    protos = collections.defaultdict(list)
    for h, (d, newtype) in decls.items():
        f = os.path.basename(d.location.file.name)
        line = open(f, errors='replace').read().split('\n')[d.location.line - 1]
        name = d.spelling
        nt = newtype.replace('const ', 'const ') if newtype else None
        nt = nt if nt.endswith('*') else nt + ' '
        m = None
        for mm in re.finditer(r'((?:unsigned |signed )?(?:int|long|short)\s+)%s\b' % re.escape(name), line):
            if mm.start() <= d.location.column - 1 + len(name):
                m = mm
        if m is None:
            print('cannot retype %s at %s:%d: %s' % (name, f, d.location.line, line.strip()))
            continue
        edits[f].add((d.location.line, m.start(), m.start(1) + len(m.group(1)), nt.rstrip() + (' ' if not nt.endswith('*') else '')))
        if d.kind == ci.CursorKind.PARM_DECL:
            fn = d.semantic_parent
            idx = [p.spelling for p in fn.get_arguments()].index(name)
            protos[fn.spelling].append((idx, nt))
        print('retype %s %s -> %s (%s:%d)' % ('param' if d.kind == ci.CursorKind.PARM_DECL else 'local', name, newtype, f,
                                              d.location.line))
    for f, es in edits.items():
        L = open(f).read().split('\n')
        for line, a, b, t in sorted(es, reverse=True):
            L[line - 1] = L[line - 1][:a] + t + L[line - 1][b:]
        if '--dry' not in sys.argv:
            open(f, 'w').write('\n'.join(L))
    # prototypes in headers: same parameter position
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
                    p = params[idx]
                    pm = re.match(r'(?:const )?(?:unsigned |signed )?(?:int|long|short)\s*(\w*)$', p)
                    if pm:
                        params[idx] = (nt.rstrip() + (' ' if not nt.rstrip().endswith('*') else '') + pm.group(1)).strip()
            s2 = s[:m.start(2)] + ', '.join(params) + s[m.end(2):]
            if s2 != s and '--dry' not in sys.argv:
                open(hf, 'w').write(s2)
                print('prototype of %s in %s' % (fn, hf))
    print('%d cast(s) removed / declaration(s) edited in %d file(s); manual:' % (
        sum(len(e) for e in edits.values()), len(edits)))
    for m in manual:
        print('   ', m)


if __name__ == '__main__':
    main()
