"""views_scan.py [files...]: parses the decomp's game files with libclang as MSVC6 sees them (32-bit) and writes
/tmp/views.json:
  records: every struct definition (name, file, line, size, fields: name, offset, size, type, is_pad)
  edges:   casts between pointers to two different structs (the two are views of one object)
  holders: casts from an integer (a variable, parameter, field or global) to a struct pointer (what it holds)
Run with: uv run --with libclang python views_scan.py
"""
import glob
import json
import multiprocessing
import os
import re
import sys

import clang.cindex as ci

ci.Config.set_library_file('/usr/lib/llvm-21/lib/libclang-21.so.1')

D = os.path.expanduser('~/scratch/dv64')
SRC = D + '/src/legoland'
ARGS = ['-x', 'c', '--target=i686-pc-windows-msvc', '-fms-extensions', '-fms-compatibility',
        '-fms-compatibility-version=12.00', '-I' + SRC, '-isystem', os.path.expanduser('~/scratch/msvc6inc'),
        '-ivfsoverlay', os.path.expanduser('~/scratch/msvc6vfs.yaml'), '-std=gnu89',
        '-Wno-error=int-conversion', '-Wno-error=incompatible-function-pointer-types',
        '-D_M_IX86=500', '-D_WIN32', '-DWIN32', '-D_MSC_VER=1200', '-w', '-ferror-limit=0']


def record_of(t):
    """the struct declaration a (possibly pointer, typedef'd, const) type is, or None"""
    t = t.get_canonical()
    if t.kind == ci.TypeKind.RECORD:
        d = t.get_declaration()
        if d.kind == ci.CursorKind.STRUCT_DECL:
            return d
    return None


def pointee_record(t):
    t = t.get_canonical()
    if t.kind != ci.TypeKind.POINTER:
        return None
    return record_of(t.get_pointee())


def rid(d):
    """a struct's identity: its definition's file and line (names repeat across files)"""
    d = d.get_definition() or d
    loc = d.location
    return '%s:%s:%d' % (d.spelling or '(anon)', os.path.basename(loc.file.name) if loc.file else '?', loc.line)


def entity(e):
    """what an expression names: a variable/parameter/global or a field (struct id + field name)"""
    while e.kind in (ci.CursorKind.UNEXPOSED_EXPR, ci.CursorKind.PAREN_EXPR):
        kids = list(e.get_children())
        if len(kids) != 1:
            return None
        e = kids[0]
    if e.kind == ci.CursorKind.DECL_REF_EXPR and e.referenced is not None:
        r = e.referenced
        if r.kind == ci.CursorKind.PARM_DECL:
            fn = r.semantic_parent
            return 'param:%s:%s' % (fn.spelling if fn else '?', r.spelling)
        if r.kind == ci.CursorKind.VAR_DECL:
            if r.semantic_parent is not None and r.semantic_parent.kind == ci.CursorKind.FUNCTION_DECL:
                return 'local:%s:%s' % (r.semantic_parent.spelling, r.spelling)
            return 'global:%s' % r.spelling
    if e.kind == ci.CursorKind.MEMBER_REF_EXPR and e.referenced is not None:
        f = e.referenced
        parent = f.semantic_parent
        # anonymous unions/structs: up to the named struct
        while parent is not None and not parent.spelling and parent.semantic_parent is not None and \
                parent.semantic_parent.kind in (ci.CursorKind.STRUCT_DECL, ci.CursorKind.UNION_DECL):
            parent = parent.semantic_parent
        if parent is not None and parent.kind == ci.CursorKind.STRUCT_DECL:
            return 'field:%s.%s' % (rid(parent), f.spelling)
    if e.kind == ci.CursorKind.ARRAY_SUBSCRIPT_EXPR:
        kids = list(e.get_children())
        if kids:
            inner = entity(kids[0])
            return inner + '[]' if inner else None
    return None


def scan(path):
    # clang crashes on some MSVC inline asm blocks: blank them, keeping the line numbers
    text = re.sub(r'\b_?_asm\b\s*\{[^}]*\}', lambda m: ';' + '\n' * m.group(0).count('\n'),
                  open(path, errors='replace').read())
    text = re.sub(r'\b_?_asm\b[^\n{]*$', ';', text, flags=re.M)
    tu = ci.Index.create().parse(path, args=ARGS, unsaved_files=[(path, text)])
    records, edges, holders = {}, [], []
    me = os.path.basename(path)

    def visit(c):
        for k in c.get_children():
            if k.location.file is None or not k.location.file.name.startswith(SRC):
                continue
            if k.kind == ci.CursorKind.STRUCT_DECL and k.is_definition():
                key = rid(k)
                if key not in records:
                    t = k.type
                    fields = []
                    for f in k.type.get_fields():
                        off = t.get_offset(f.spelling) if f.spelling else -1
                        fields.append({'name': f.spelling, 'offset': off // 8 if off >= 0 else None,
                                       'size': f.type.get_size(), 'type': f.type.spelling,
                                       'pad': f.spelling.startswith('pad_') or f.spelling.startswith('unk_')})
                    records[key] = {'name': k.spelling, 'file': os.path.basename(k.location.file.name),
                                    'line': k.location.line, 'size': t.get_size(), 'fields': fields}
            if k.kind == ci.CursorKind.CSTYLE_CAST_EXPR:
                kids = list(k.get_children())
                src = kids[-1] if kids else None
                to = pointee_record(k.type)
                if src is not None and to is not None:
                    frm = pointee_record(src.type)
                    loc = '%s:%d' % (os.path.basename(k.location.file.name), k.location.line)
                    if frm is not None and rid(frm) != rid(to):
                        edges.append([rid(frm), rid(to), loc])
                    elif frm is None and src.type.get_canonical().kind in (
                            ci.TypeKind.INT, ci.TypeKind.UINT, ci.TypeKind.LONG, ci.TypeKind.ULONG):
                        ent = entity(src)
                        if ent:
                            holders.append([ent, rid(to), loc])
            visit(k)

    visit(tu.cursor)
    return me, records, edges, holders


def main():
    files = sys.argv[1:] or sorted(f for f in glob.glob(SRC + '/*.c')
                                   if os.path.basename(f) not in ('imports.c', 'crt.c', 'bootstrap.c'))
    out = {'records': {}, 'edges': [], 'holders': []}
    with multiprocessing.Pool(8) as pool:
        for me, records, edges, holders in pool.imap_unordered(scan, files):
            out['records'].update(records)
            out['edges'] += edges
            out['holders'] += holders
    json.dump(out, open('/tmp/views.json', 'w'))
    print('%d structs (%d with pads), %d struct-to-struct casts, %d int-to-struct casts' % (
        len(out['records']), sum(1 for r in out['records'].values() if any(f['pad'] for f in r['fields'])),
        len(out['edges']), len(out['holders'])))


if __name__ == '__main__':
    main()
