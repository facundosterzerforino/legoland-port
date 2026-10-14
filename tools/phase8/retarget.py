"""retarget.py View=Real ... [View.field=realfield ...] [--dry]: replaces view structs with the real struct, in
~/scratch/dv64/src/legoland.

Each View=Real pair (names, or Name:file.c when a name is defined in several files): every named field of View is
mapped to Real's field at the same offset (same name first, then same size and type, then same size; an explicit
View.field=realfield wins; realfield may be a path like head.next). Uses of the view's fields are rewritten,
`struct View` becomes `struct Real` where the view was visible and the view's definition is deleted. Fields whose type
changes are listed: MSVC6 + reccmp (dvcheck) prove the result.
"""
import collections
import os
import re
import subprocess
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import views_scan as vs  # noqa: E402

ci = vs.ci
SRC = vs.SRC
os.chdir(SRC)


def parse(path):
    text = open(path, errors='replace').read()
    text = re.sub(r'\b_?_asm\b\s*\{[^}]*\}', lambda m: ';' + '\n' * m.group(0).count('\n'), text)
    text = re.sub(r'\b_?_asm\b[^\n{]*$', ';', text, flags=re.M)
    full = os.path.join(SRC, path)
    return ci.Index.create().parse(full, args=vs.ARGS, unsaved_files=[(full, text)])


def all_c():
    return sorted(f for f in os.listdir('.') if f.endswith('.c') and f not in ('imports.c', 'crt.c', 'bootstrap.c'))


def key(c):
    c = c.get_definition() or c
    return (c.spelling, os.path.basename(c.location.file.name) if c.location.file else '', c.location.line)


def fields_of(c, top=None, out=None, nested=False):
    """flattened fields of struct/union c, offsets relative to c: (path, offset, size, canonical type, cursor).
    Anonymous unions/structs and MS-embedded `struct Tag;` members are looked through; with nested, named struct
    fields are also expanded as name.sub."""
    out = []
    end = 0
    kids = list(c.get_children())
    field_types = set()
    for k in kids:
        if k.kind == ci.CursorKind.FIELD_DECL:
            t = k.type.get_canonical()
            while t.kind in (ci.TypeKind.POINTER, ci.TypeKind.CONSTANTARRAY, ci.TypeKind.INCOMPLETEARRAY):
                t = t.get_pointee() if t.kind == ci.TypeKind.POINTER else t.element_type
            d = t.get_declaration()
            field_types.add(d.spelling)
    for k in kids:
        if k.kind == ci.CursorKind.FIELD_DECL and k.spelling:
            off = c.type.get_offset(k.spelling)
            off = off // 8 if off >= 0 else -1
            rec = k.type.get_canonical().get_declaration()
            out.append((k.spelling, off, k.type.get_size(), k.type.get_canonical().spelling, k))
            end = max(end, off + max(k.type.get_size(), 0))
            if nested and rec.kind == ci.CursorKind.STRUCT_DECL and k.type.get_canonical().kind == ci.TypeKind.RECORD:
                for path, o, sz, ty, cur in fields_of(rec.get_definition() or rec, nested=True):
                    out.append((k.spelling + '.' + path, off + o, sz, ty, cur))
            continue
        if k.kind not in (ci.CursorKind.STRUCT_DECL, ci.CursorKind.UNION_DECL):
            continue
        if k.is_anonymous():
            sub = fields_of(k, nested=nested)
            named = [x for x in sub if '.' not in x[0]]
            if not named:
                continue
            base = c.type.get_offset(named[0][0])
            base = base // 8 - named[0][1] if base >= 0 else end
        elif not k.is_definition() and k.spelling not in field_types and (k.get_definition() is not None):
            # MS extension: `struct Tag;` embeds Tag's fields
            d = k.get_definition()
            al = max(d.type.get_align(), 1)
            base = (end + al - 1) // al * al
            sub = fields_of(d, nested=nested)
        else:
            continue
        for path, o, sz, ty, cur in sub:
            out.append((path, base + o, sz, ty, cur))
            end = max(end, base + o + max(sz, 0))
    return out


def is_pad(name):
    return re.match(r'(pad|unk)(_|\d|$)', name.split('.')[-1]) is not None


def main():
    pairs, maps = [], {}
    for a in sys.argv[1:]:
        if a.startswith('--'):
            continue
        l, r = a.split('=', 1)
        if '.' in l.split(':')[0]:
            v, f = l.split('.', 1)
            maps[(v, f)] = r
        else:
            pairs.append((l, r))
    names = {n for p in pairs for n in p}
    defs = {}
    for f in all_c():
        s = open(f, errors='replace').read()
        if not any(re.search(r'\bstruct %s\b' % n.split(':')[0], s) for n in names):
            continue
        tu = parse(f)
        for c in tu.cursor.walk_preorder():
            if c.kind == ci.CursorKind.STRUCT_DECL and c.is_definition() and c.location.file and \
                    c.location.file.name.startswith(SRC):
                base = os.path.basename(c.location.file.name)
                for n in names:
                    nm, _, fl = n.partition(':')
                    if n not in defs and c.spelling == nm and (not fl or fl == base):
                        defs[n] = c
        if len(defs) == len(names):
            break
    missing = names - set(defs)
    if missing:
        sys.exit('not found: %s' % sorted(missing))
    renames = {}  # view key -> {field: new path}
    problems = []
    for v, r in pairs:
        vd, rd = defs[v], defs[r]
        rf = fields_of(rd, nested=True)
        m = {}
        for path, o, s, ty, cur in fields_of(vd):
            if is_pad(path):
                continue
            want = maps.get((v.split(':')[0], path))
            if want is None:
                cands = [x for x in rf if x[1] == o and not is_pad(x[0])]
                pick = [x for x in cands if x[0] == path and x[2] == s] or \
                       [x for x in cands if x[2] == s and x[3] == ty] or \
                       [x for x in cands if x[2] == s]
                if len(pick) > 1:
                    pick = [x for x in pick if x[0].split('.')[-1] == path.split('.')[-1]] or pick
                if len(pick) > 1:
                    pick = [x for x in pick if '.' not in x[0]] or pick
                if len(pick) > 1:
                    pick = [x for x in pick if x[3] == ty] or pick
                if len(pick) != 1:
                    problems.append('%s.%s (%#x, %d bytes, %s): %s' % (
                        v, path, o, s, ty, ', '.join('%s %s' % (x[0], x[3]) for x in cands) or 'nothing there'))
                    continue
                want = pick[0][0]
                if pick[0][3] != ty:
                    print('type change %s.%s %s -> %s.%s %s' % (v, path, ty, r, want, pick[0][3]))
            m[path] = want
        renames[key(vd)] = (vd, rd, m)
    if problems:
        print('unmapped fields:\n  ' + '\n  '.join(problems))
        sys.exit(1)
    for k, (vd, rd, m) in renames.items():
        print('%s -> %s: %s' % (vd.spelling, rd.spelling, ', '.join('%s=%s' % (a, b) for a, b in m.items() if a != b)
                                or 'same names'))
    if '--dry' in sys.argv:
        return
    # member uses of the views' fields
    vnames = {vd.spelling for vd, rd, m in renames.values()}
    edits = collections.defaultdict(set)
    for f in all_c():
        s = open(f, errors='replace').read()
        if not any(re.search(r'\bstruct %s\b' % n, s) for n in vnames):
            continue
        tu = parse(f)
        for c in tu.cursor.walk_preorder():
            if c.kind != ci.CursorKind.MEMBER_REF_EXPR or c.referenced is None:
                continue
            fld = c.referenced
            par = fld.semantic_parent
            path = fld.spelling
            while par is not None and par.is_anonymous() and par.semantic_parent is not None:
                par = par.semantic_parent
            if par is None or par.kind != ci.CursorKind.STRUCT_DECL:
                continue
            ent = renames.get(key(par))
            if ent is None or ent[2].get(path, path) == path:
                continue
            loc = c.extent.end
            fn = os.path.basename(loc.file.name)
            line = open(fn, errors='replace').read().split('\n')[loc.line - 1]
            a = loc.column - 1 - len(path)
            if line[a:loc.column - 1] != path:
                sys.exit('%s:%d: expected %s at column %d: %s' % (fn, loc.line, path, a + 1, line))
            edits[fn].add((loc.line, a, loc.column - 1, ent[2][path]))
    # view definitions
    dels = collections.defaultdict(list)
    for vd, rd, m in renames.values():
        e = vd.extent
        dels[os.path.basename(e.start.file.name)].append((e.start.line, e.start.column, e.end.line, e.end.column))
    touched = set(edits) | set(dels)
    for f in touched:
        L = open(f).read().split('\n')
        for line, a, b, new in sorted(edits[f], reverse=True):
            L[line - 1] = L[line - 1][:a] + new + L[line - 1][b:]
        for l1, c1, l2, c2 in sorted(dels[f], reverse=True):
            head, tail = L[l1 - 1][:c1 - 1], L[l2 - 1][c2 - 1:]
            tail = tail[1:] if tail.startswith(';') else tail
            rest = (head + tail).strip()
            L[l1 - 1:l2] = [head + tail] if rest else []
            if not rest and l1 - 1 < len(L) and not L[l1 - 1].strip() and l1 >= 2 and not L[l1 - 2].strip():
                del L[l1 - 1]
        open(f, 'w').write('\n'.join(L))
    # type names
    for vd, rd, m in renames.values():
        vf = os.path.basename(vd.location.file.name)
        files = [vf] if vf.endswith('.c') else [f for f in sorted(os.listdir('.')) if f.endswith(('.c', '.h'))]
        for f in files:
            s = open(f, errors='replace').read()
            s2 = re.sub(r'^struct %s;\n' % vd.spelling, '', s, flags=re.M)
            s2 = re.sub(r'\bstruct %s\b' % vd.spelling, 'struct %s' % rd.spelling, s2)
            if s2 != s:
                open(f, 'w').write(s2)
                touched.add(f)
    subprocess.run(['clang-format', '-i'] + sorted(touched))
    print('changed: %s' % ' '.join(sorted(touched)))


if __name__ == '__main__':
    main()
