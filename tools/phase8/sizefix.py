"""sizefix.py [--dry] [files...]: constant sizes that are the size of a pointer-holding struct become sizeof.

In malloc(N), memset(p, c, N), memcpy(p, q, N), SaveGameRead(p, N) and SaveGameWrite(p, N), when N is a literal
equal to sizeof(T) on x86 and p (or the variable malloc's result goes to) points to T, and T holds pointers (so its
size differs on 64-bit), N becomes sizeof(T) (spelled with T's name). MSVC6 folds sizeof to the same constant.
RES_ReadFile is left alone (the game's files keep the 32-bit layout)."""
import os
import re
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import views_scan as vs  # noqa: E402

ci = vs.ci
os.chdir(vs.SRC)
FUNCS = {'malloc': (0, None), 'memset': (2, 0), 'memcpy': (2, 0), 'SaveGameRead': (1, 0), 'SaveGameWrite': (1, 0),
         'calloc': None}


def holds_pointer(t, seen=None):
    t = t.get_canonical()
    if t.kind == ci.TypeKind.POINTER:
        return True
    if t.kind == ci.TypeKind.CONSTANTARRAY:
        return holds_pointer(t.get_array_element_type())
    if t.kind == ci.TypeKind.RECORD:
        for f in t.get_declaration().type.get_fields():
            if holds_pointer(f.type):
                return True
    return False


def type_name(t):
    """a spelling of t usable in sizeof()"""
    c = t.get_canonical()
    if c.kind == ci.TypeKind.RECORD:
        d = c.get_declaration()
        if d.spelling and '(anonymous' not in d.spelling:
            return ('union ' if d.kind == ci.CursorKind.UNION_DECL else 'struct ') + d.spelling
        return None
    return None


def target_type(arg):
    """the pointee type of a pointer argument expression (through casts and &)"""
    e = arg
    while e.kind in (ci.CursorKind.UNEXPOSED_EXPR, ci.CursorKind.CSTYLE_CAST_EXPR, ci.CursorKind.PAREN_EXPR) and \
            e.type.get_canonical().kind == ci.TypeKind.POINTER and \
            e.type.get_canonical().get_pointee().kind in (ci.TypeKind.VOID, ci.TypeKind.CHAR_S, ci.TypeKind.UCHAR,
                                                          ci.TypeKind.CHAR_U, ci.TypeKind.UINT, ci.TypeKind.INT):
        kids = list(e.get_children())
        if not kids:
            break
        e = kids[-1]
    t = e.type.get_canonical()
    if t.kind == ci.TypeKind.POINTER:
        return t.get_pointee()
    return None


def literal(e):
    while e.kind in (ci.CursorKind.UNEXPOSED_EXPR, ci.CursorKind.PAREN_EXPR):
        kids = list(e.get_children())
        if not kids:
            return None
        e = kids[0]
    if e.kind == ci.CursorKind.INTEGER_LITERAL:
        toks = [t.spelling for t in e.get_tokens()]
        return toks[0] if toks else None
    return None


def main():
    dry = '--dry' in sys.argv
    files = [a for a in sys.argv[1:] if a.endswith('.c')] or sorted(
        f for f in os.listdir('.') if f.endswith('.c') and f not in ('imports.c', 'crt.c', 'bootstrap.c'))
    total = 0
    for path in files:
        text = open(path, errors='replace').read()
        clean = re.sub(r'\b_?_asm\b\s*\{[^}]*\}', lambda m: ';' + '\n' * m.group(0).count('\n'), text)
        clean = re.sub(r'\b_?_asm\b[^\n{]*$', ';', clean, flags=re.M)
        full = os.path.join(vs.SRC, path)
        tu = ci.Index.create().parse(full, args=vs.ARGS, unsaved_files=[(full, clean)])
        edits = []

        def visit(c, assign_to=None):
            if c.kind == ci.CursorKind.CALL_EXPR and c.spelling in FUNCS and FUNCS[c.spelling] is not None:
                args = list(c.get_arguments())
                ni, pi = FUNCS[c.spelling]
                if ni < len(args):
                    lit = literal(args[ni])
                    t = assign_to if c.spelling == 'malloc' else (target_type(args[pi]) if pi < len(args) else None)
                    if lit and t is not None:
                        n = int(lit, 0) if not lit.lower().endswith(('u', 'l')) else int(lit.rstrip('uUlL'), 0)
                        name = type_name(t)
                        if name and t.get_canonical().get_size() == n and holds_pointer(t):
                            ext = args[ni].extent
                            if ext.start.file and os.path.basename(ext.start.file.name) == path:
                                edits.append((ext.start.line, ext.start.column, ext.end.column, lit, 'sizeof(%s)' % name))
            if c.kind == ci.CursorKind.BINARY_OPERATOR:
                kids = list(c.get_children())
                toks = [t.spelling for t in c.get_tokens()]
                if len(kids) == 2 and '=' in toks and kids[0].type.get_canonical().kind == ci.TypeKind.POINTER:
                    visit(kids[0])
                    visit(kids[1], kids[0].type.get_canonical().get_pointee())
                    return
            if c.kind == ci.CursorKind.VAR_DECL and c.type.get_canonical().kind == ci.TypeKind.POINTER:
                for k in c.get_children():
                    visit(k, c.type.get_canonical().get_pointee())
                return
            for k in c.get_children():
                visit(k, assign_to if c.kind in (ci.CursorKind.UNEXPOSED_EXPR, ci.CursorKind.CSTYLE_CAST_EXPR,
                                                  ci.CursorKind.PAREN_EXPR) else None)

        for c in tu.cursor.get_children():
            if c.location.file and os.path.basename(c.location.file.name) == path:
                visit(c)
        if not edits:
            continue
        L = text.split('\n')
        for line, a, b, lit, new in sorted(set(edits), reverse=True):
            s = L[line - 1]
            if s[a - 1:b - 1] == lit:
                L[line - 1] = s[:a - 1] + new + s[b - 1:]
                total += 1
                if dry:
                    print('%s:%d: %s' % (path, line, L[line - 1].strip()[:140]))
        if not dry:
            open(path, 'w').write('\n'.join(L))
    print(total, 'sizes')


if __name__ == '__main__':
    main()
