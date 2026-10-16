"""tools/linux/gen_win32.py: generates miniwin/include/win32/, the Windows declarations the port's code uses, for
non-Windows builds (ROADMAP.md phase 8, Linux). Run from the repo with libclang (uv run --with libclang==18.1.1)
after tools/linux/sdkuse.py; needs the Windows SDK the x64 build uses (~/xwin). Then
tools/linux/check_layouts.py compares every generated record's layout with the SDK's, on 64 and 32 bits.

It parses that SDK with libclang, starts from what the code uses (tools/linux/sdkuse.json, and every identifier in
the sources that the SDK declares) minus what the platform's own C library declares, follows the dependencies (field, parameter
and return types, the identifiers inside macros), and writes each declaration as C: the facts of the SDK (names,
values, layouts) in the port's own formatting. Types keep Windows' sizes: the SDK's long (32 bits on Windows)
becomes __LONG32 (win32_prelude.h: long or int, whichever is 32 bits); WCHAR stays 16-bit; calling conventions go."""
import json
import os
import re
import subprocess
import sys

import clang.cindex as ci

ATTRIBUTED = getattr(ci.TypeKind, 'ATTRIBUTED', None)

ROOT = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
XWIN = os.environ.get('XWIN', os.path.expanduser('~/xwin'))
OUT = os.path.join(ROOT, 'miniwin/include/win32/win32.h')
HEADERS = ['windows.h', 'ddraw.h', 'dsound.h', 'dinput.h', 'mmsystem.h', 'mmreg.h', 'msacm.h', 'vfw.h', 'commctrl.h',
           'commdlg.h', 'shlobj.h', 'dbghelp.h', 'winspool.h', 'basetsd.h', 'io.h', 'direct.h', 'intrin.h',
           'fcntl.h', 'sys/stat.h', 'stdio.h', 'wchar.h', 'stdlib.h', 'string.h', 'malloc.h', 'float.h']
ARGS = ['-x', 'c', '--target=x86_64-pc-windows-msvc', '-fms-extensions', '-fms-compatibility',
        '-fms-compatibility-version=19.40', '-D_WIN32', '-D_WIN64', '-D_M_X64', '-D_AMD64_', '-DWIN32', '-D_WINDOWS',
        '-DDIRECTINPUT_VERSION=0x0500', '-D_CRT_SECURE_NO_WARNINGS', '-w',
        '-isystem', XWIN + '/crt/include', '-isystem', XWIN + '/sdk/include/ucrt', '-isystem',
        XWIN + '/sdk/include/um', '-isystem', XWIN + '/sdk/include/shared']
# defined by the prelude (or meaningless off Windows): not generated
PRELUDE_NAMES = set('''__va_start wchar_t size_t ptrdiff_t intptr_t uintptr_t va_list
WINAPI WINAPIV APIENTRY CALLBACK PASCAL STDMETHODCALLTYPE STDAPICALLTYPE CDECL FAR NEAR far near
CONST IN OUT OPTIONAL DECLSPEC_IMPORT DECLSPEC_NORETURN DECLSPEC_ALIGN DECLSPEC_NOTHROW DECLSPEC_SELECTANY
WINBASEAPI WINUSERAPI WINGDIAPI WINMMAPI WINADVAPI WINSPOOLAPI SHSTDAPI SHFOLDERAPI COMDLGAPI WINCOMMCTRLAPI
WINOLEAPI WINOLEAUTAPI DECLSPEC_GUID DECLSPEC_UUID EXTERN_C STDAPI STDAPI_ STDMETHOD STDMETHOD_ PURE THIS THIS_
interface DECLARE_INTERFACE DECLARE_INTERFACE_ BEGIN_INTERFACE END_INTERFACE _Null_terminated_ __int64
__declspec __stdcall __cdecl __fastcall __unaligned __w64 UNALIGNED POINTER_64 POINTER_32 POINTER_SIGNED
POINTER_UNSIGNED FORCEINLINE NTAPI WINBOOL bool true false _DRVRESERVED
_byteswap_ushort _byteswap_ulong _byteswap_uint64 GetExceptionInformation GetExceptionCode'''.split())

# Windows' integers the size of a pointer: the target's, whatever the SDK parsed was built for
POINTER_SIZED = {'INT_PTR': 'intptr_t', 'UINT_PTR': 'uintptr_t', 'LONG_PTR': 'intptr_t', 'ULONG_PTR': 'uintptr_t',
                 'DWORD_PTR': 'uintptr_t', 'SIZE_T': 'size_t', 'SSIZE_T': 'intptr_t', 'HALF_PTR': 'short',
                 'UHALF_PTR': 'unsigned short'}

# ---- what the platform's C library already has (parse its headers) ----


def linux_names():
    src = '\n'.join('#include <%s>' % h for h in (
        'stdio.h stdlib.h string.h strings.h math.h ctype.h time.h stdarg.h stddef.h stdint.h errno.h limits.h '
        'fcntl.h sys/stat.h sys/types.h unistd.h float.h malloc.h wchar.h signal.h setjmp.h inttypes.h '
        'stdbool.h locale.h').split()) + '\n'
    # the pip libclang has no compiler headers (stddef.h, stdarg.h...): the system clang's
    res = subprocess.run(['clang', '-print-resource-dir'], capture_output=True, text=True).stdout.strip()
    tu = ci.Index.create().parse('linux.c', args=['-x', 'c', '-w', '-resource-dir', res], unsaved_files=[('linux.c', src)],
                                 options=ci.TranslationUnit.PARSE_DETAILED_PROCESSING_RECORD)
    names = set()
    for c in tu.cursor.walk_preorder():
        if c.spelling:
            names.add(c.spelling)
    return names


# ---- the SDK ----
sdk_src = ''.join('#include <%s>\n' % h for h in HEADERS)
tu = ci.Index.create().parse('sdk.c', args=ARGS, unsaved_files=[('sdk.c', sdk_src)],
                             options=ci.TranslationUnit.PARSE_DETAILED_PROCESSING_RECORD)
errs = [d for d in tu.diagnostics if d.severity >= 3]
if errs:
    for d in errs[:10]:
        print('SDK parse:', d, file=sys.stderr)
decls = {}  # name -> cursor (first definition wins; typedefs, records, enums, functions, vars, macros)
enum_of = {}  # enum constant -> enum cursor
for c in tu.cursor.get_children():
    k = c.kind
    if k in (ci.CursorKind.TYPEDEF_DECL, ci.CursorKind.FUNCTION_DECL, ci.CursorKind.VAR_DECL,
             ci.CursorKind.MACRO_DEFINITION):
        if c.spelling and (c.spelling not in decls or k == ci.CursorKind.MACRO_DEFINITION):
            decls.setdefault(c.spelling, c) if k != ci.CursorKind.MACRO_DEFINITION else decls.__setitem__(c.spelling, c)
    elif k in (ci.CursorKind.STRUCT_DECL, ci.CursorKind.UNION_DECL, ci.CursorKind.ENUM_DECL):
        if c.spelling and not c.spelling.startswith('(') and '(' not in c.spelling:
            key = ('struct ' if k == ci.CursorKind.STRUCT_DECL else 'union ' if k == ci.CursorKind.UNION_DECL
                   else 'enum ') + c.spelling
            if c.is_definition() or key not in decls:
                decls[key] = c
        if k == ci.CursorKind.ENUM_DECL:
            for e in c.get_children():
                enum_of[e.spelling] = c
print('SDK declarations:', len(decls), file=sys.stderr)

use = json.load(open(os.path.join(ROOT, 'tools/linux/sdkuse.json')))
have = linux_names()
seeds = set()
# and every identifier in our sources that the SDK declares (code under #ifdef the scan didn't see)
for top in ('src/legoland', 'port', 'miniwin/src', 'miniwin/include/miniwin', 'extensions', 'tools/test_miniwin'):
    for dirpath, dirs, files in os.walk(os.path.join(ROOT, top)):
        dirs[:] = [d for d in dirs if d not in ('dmusic', 'ffmpeg', 'blast', 'puff', 'cmake')]
        for f in files:
            if f.endswith(('.c', '.h')):
                for w in set(re.findall(r'[A-Za-z_]\w*', open(os.path.join(dirpath, f), errors='replace').read())):
                    if w in decls or w in enum_of:
                        use.setdefault('source', []).append(w)
print('source identifiers the SDK declares: %d; the C library\'s names: %d' % (len(set(use.get('source', []))), len(have)),
      file=sys.stderr)
for kind, names in use.items():
    for n in names:
        if n in have or n in PRELUDE_NAMES:
            continue
        for kw in ('', 'struct ', 'union ', 'enum '):
            if kw + n in decls or (not kw and n in enum_of):
                seeds.add(kw + n)
                break
        else:
            seeds.add(n)

# ---- dependencies ----
needed = []
seen = set()


via_pointer = set()  # of the last deps() call: the names reached only through a pointer


def type_deps(t, out, ptr=False):
    while True:
        k = t.kind
        if k == ci.TypeKind.POINTER:
            t = t.get_pointee()
            ptr = True
        elif k in (ci.TypeKind.CONSTANTARRAY, ci.TypeKind.INCOMPLETEARRAY):
            t = t.element_type
        elif k == ci.TypeKind.ELABORATED:
            t = t.get_named_type()
        elif k == ATTRIBUTED:
            t = t.get_modified_type() if hasattr(t, 'get_modified_type') else t.get_canonical()
            if t.kind == ATTRIBUTED:
                break
        else:
            break
    if t.kind == ci.TypeKind.TYPEDEF:
        add_dep(out, t.get_declaration().spelling, ptr)
    elif t.kind in (ci.TypeKind.RECORD, ci.TypeKind.ENUM):
        d = t.get_declaration()
        if d.spelling and '(' not in d.spelling and not d.spelling.startswith('('):
            kw = 'struct ' if d.kind == ci.CursorKind.STRUCT_DECL else 'union ' if d.kind == ci.CursorKind.UNION_DECL \
                else 'enum '
            add_dep(out, kw + d.spelling, ptr)
        else:
            # an anonymous record: its fields' types
            record_deps(d, out, ptr)
    elif t.kind == ci.TypeKind.FUNCTIONPROTO:
        type_deps(t.get_result(), out, ptr)
        for a in t.argument_types():
            type_deps(a, out, ptr)


def add_dep(out, name, ptr):
    if ptr and name not in out:
        via_pointer.add(name)
    elif not ptr:
        via_pointer.discard(name)
    out.add(name)


def record_deps(c, out, ptr=False):
    for f in c.get_children():
        if f.kind == ci.CursorKind.FIELD_DECL:
            type_deps(f.type, out, ptr)
        elif f.kind in (ci.CursorKind.STRUCT_DECL, ci.CursorKind.UNION_DECL):
            record_deps(f, out, ptr)


def deps(name):
    via_pointer.clear()
    c = decls.get(name)
    out = set()
    if c is None:
        if name in enum_of:
            e = enum_of[name]
            out.add('enum ' + e.spelling if e.spelling and '(' not in e.spelling else '@enum:' + name)
        return out
    k = c.kind
    if k == ci.CursorKind.TYPEDEF_DECL:
        type_deps(c.underlying_typedef_type, out)
    elif k in (ci.CursorKind.STRUCT_DECL, ci.CursorKind.UNION_DECL):
        record_deps(c, out)
    elif k == ci.CursorKind.FUNCTION_DECL:
        type_deps(c.type, out)
    elif k == ci.CursorKind.VAR_DECL:
        type_deps(c.type, out)
    elif k == ci.CursorKind.MACRO_DEFINITION:
        toks = [t.spelling for t in c.get_tokens()][1:]
        for t in toks:
            if re.match(r'^[A-Za-z_]\w*$', t) and t != name and (t in decls or t in enum_of) and t not in PRELUDE_NAMES:
                out.add(t)
            if t in ('struct', 'union', 'enum'):
                pass
    return out


todo = sorted(seeds)
while todo:
    n = todo.pop()
    if n in seen:
        continue
    seen.add(n)
    if n.startswith('@enum:'):
        continue
    needed.append(n)
    for d in deps(n):
        if d not in seen and d not in PRELUDE_NAMES and not (d in have and not d.startswith(('struct ', 'union ', 'enum '))):
            todo.append(d)
missing = [n for n in needed if n not in decls and n not in enum_of]
print('needed %d, not found in the SDK %d: %s' % (len(needed), len(missing), ' '.join(sorted(missing)[:40])),
      file=sys.stderr)


# ---- writing ----


def fix(s):
    """an SDK type spelling in the port's terms: Windows' 32-bit long, 16-bit wchar_t, no calling conventions"""
    s = re.sub(r'__attribute__\(\((stdcall|cdecl|fastcall|thiscall|ms_abi)\)\)\s*', '', s)
    s = re.sub(r'\b(__stdcall|__cdecl|__fastcall|__unaligned|__ptr64|__ptr32)\b\s*', '', s)
    s = re.sub(r'(?<!long )\blong\b(?! long)', '__LONG32', s)
    s = s.replace('unsigned __LONG32 int', 'unsigned __LONG32').replace('__LONG32 int', '__LONG32')
    s = re.sub(r'\bwchar_t\b', 'unsigned short', s)
    s = s.replace('__int64', 'long long')
    return s.strip()


def strip_type(t):
    while t.kind in (ci.TypeKind.ELABORATED, ATTRIBUTED) and t.kind is not None:
        if t.kind == ci.TypeKind.ELABORATED:
            t = t.get_named_type()
        else:
            m = t.get_modified_type() if hasattr(t, 'get_modified_type') else None
            if m is None:
                break
            t = m
    return t


def declarator(t, name):
    """C text declaring name with type t (function pointers and arrays written out)"""
    t0 = t
    t = strip_type(t)
    if t.kind == ci.TypeKind.CONSTANTARRAY:
        return declarator(t.element_type, '%s[%d]' % (name, t.element_count))
    if t.kind == ci.TypeKind.INCOMPLETEARRAY:
        return declarator(t.element_type, '%s[]' % name)
    if t.kind == ci.TypeKind.POINTER:
        p = strip_type(t.get_pointee())
        if p.kind in (ci.TypeKind.FUNCTIONPROTO, ci.TypeKind.FUNCTIONNOPROTO):
            args = ', '.join(fix(a.spelling) for a in p.argument_types()) if p.kind == ci.TypeKind.FUNCTIONPROTO \
                else ''
            if p.kind == ci.TypeKind.FUNCTIONPROTO and p.is_function_variadic():
                args += ', ...' if args else '...'
            return '%s (*%s)(%s)' % (fix(p.get_result().spelling), name, args or 'void')
        if p.kind in (ci.TypeKind.CONSTANTARRAY,):
            return declarator(p.element_type, '(*%s)[%d]' % (name, p.element_count))
    return ('%s %s' % (fix(t0.spelling), name)).strip()


# Array sizes the SDK sets per target (#ifdef _WIN64): written by their macro's name, which win32_prelude.h defines
# for the target; as numbers they would be the 64-bit SDK's. (Generating for 32-bit Windows and comparing found these;
# the other differences, CONTEXT and IMAGE_NT_HEADERS, are x64's on every target on purpose: the crash report.)
TARGET_SIZES = {'_DRVRESERVED'}
# Members the SDK has on 64-bit Windows only (#if defined(_WIN64)): kept to 64-bit targets
WIN64_FIELDS = {('_MEMORY_BASIC_INFORMATION', 'PartitionId')}


def natural_align(c):
    """the alignment the record's members would give it without #pragma pack"""
    a = 1
    for f in c.get_children():
        if f.kind == ci.CursorKind.FIELD_DECL:
            a = max(a, f.type.get_align())
        elif f.kind in (ci.CursorKind.STRUCT_DECL, ci.CursorKind.UNION_DECL) and (
                not f.spelling or '(' in f.spelling or f.spelling.startswith('(')):
            a = max(a, natural_align(f))
    return a


def packed(c, text):
    """The record's text, inside the SDK's #pragma pack when it was declared under one (WAVEFORMATEX: pack(1), 18 bytes
    and not 20; BITMAPFILEHEADER: pack(2)): the record's alignment is then smaller than its members'."""
    a = c.type.get_align()
    if 0 < a < natural_align(c):
        return '#pragma pack(push, %d)\n%s\n#pragma pack(pop)' % (a, text)
    return text


def record_body(c, indent='    '):
    lines = []
    for f in c.get_children():
        if f.kind == ci.CursorKind.FIELD_DECL:
            ft = strip_type(f.type)
            decl = ft.get_declaration()
            if ft.kind == ci.TypeKind.RECORD and (not decl.spelling or '(' in decl.spelling or
                                                  decl.spelling.startswith('(')):
                kw = 'struct' if decl.kind == ci.CursorKind.STRUCT_DECL else 'union'
                lines.append('%s%s {\n%s%s} %s;' % (indent, kw, record_body(decl, indent + '    '), indent, f.spelling))
            else:
                bits = ' : %d' % f.get_bitfield_width() if f.is_bitfield() else ''
                size = next((t.spelling for t in f.get_tokens() if t.spelling in TARGET_SIZES), None)
                if size is not None and ft.kind == ci.TypeKind.CONSTANTARRAY:
                    lines.append('%s%s;' % (indent, declarator(ft.element_type, '%s[%s]' % (f.spelling, size))))
                elif (c.spelling, f.spelling) in WIN64_FIELDS:
                    lines.append('#if defined(_WIN64) || defined(__LP64__)\n%s%s%s;\n#endif' % (
                        indent, declarator(f.type, f.spelling), bits))
                else:
                    lines.append('%s%s%s;' % (indent, declarator(f.type, f.spelling), bits))
        elif f.kind in (ci.CursorKind.STRUCT_DECL, ci.CursorKind.UNION_DECL) and (
                not f.spelling or '(' in f.spelling or f.spelling.startswith('(')):
            # an anonymous member record (no field of its own): MS extension, members visible directly
            if not any(g.kind == ci.CursorKind.FIELD_DECL and strip_type(g.type).get_declaration() == f
                       for g in c.get_children()):
                kw = 'struct' if f.kind == ci.CursorKind.STRUCT_DECL else 'union'
                lines.append('%s%s {\n%s%s};' % (indent, kw, record_body(f, indent + '    '), indent))
    return '\n'.join(lines) + '\n'


def macro_text(c):
    toks = list(c.get_tokens())
    if not toks:
        return None
    # rebuild with the original spacing between tokens of the definition
    src = []
    prev_end = None
    for t in toks:
        if prev_end is not None:
            gap = t.extent.start.offset - prev_end
            src.append(' ' if gap > 0 else '')
        src.append(t.spelling)
        prev_end = t.extent.end.offset
    text = ''.join(src)
    name = c.spelling
    body = text[len(name):]
    if len(toks) > 1 and toks[1].spelling == '(' and toks[1].extent.start.offset == toks[0].extent.end.offset:
        # name(args) body: the parameter list follows the name with no space
        return '#define %s%s' % (name, fix_macro(body))
    return '#define %s %s' % (name, fix_macro(body.strip()))


def fix_macro(s):
    s = re.sub(r'\b__int64\b', 'long long', s)
    s = re.sub(r'(?<!long )\blong\b(?! long)', '__LONG32', s)
    s = re.sub(r'\b(__stdcall|__cdecl|WINAPI|CALLBACK|STDMETHODCALLTYPE)\b', '', s)
    s = re.sub(r'(\b0x[0-9A-Fa-f]+|\b\d+)L\b', r'\1', s)
    s = re.sub(r'(\b0x[0-9A-Fa-f]+|\b\d+)UL\b', r'\1U', s)
    return s



# ---- which header each declaration comes with ----
# Each SDK header gets what it brings on Windows, no more: the game declares a few imports itself (imports.h), with
# its own types, next to <windows.h> but without <vfw.h> or <msacm.h>. The C runtime's extensions go with windows.h
# (the platform's own <stdio.h>... are used, so they have nowhere else to go).
BASE = ('windows.h', 'basetsd.h', 'stdio.h', 'stdlib.h', 'string.h', 'wchar.h', 'malloc.h', 'sys/stat.h', 'fcntl.h',
        'float.h')


def reach(h):
    t = ci.Index.create().parse('r.c', args=ARGS, unsaved_files=[('r.c', '#include <%s>\n' % h)])
    return {i.include.name for i in t.get_includes()} | {i.source.name for i in t.get_includes() if i.depth == 1}


reach_of = {h: reach(h) for h in HEADERS}
GROUP_ORDER = list(BASE) + [h for h in HEADERS if h not in BASE]


def group_of_file(f):
    for h in GROUP_ORDER:
        if f in reach_of[h]:
            return 'base' if h in BASE else h
    return 'base'


def cursor_of(n):
    if n in decls:
        return decls[n]
    if n in enum_of:
        return enum_of[n]
    return None


def group_of(n):
    c = cursor_of(n)
    if c is None or c.location.file is None:
        return 'base'
    return group_of_file(c.location.file.name)


def group_file(g):
    return 'win32.h' if g == 'base' else 'win32_%s' % g.replace('/', '_')


def record_alias(c):
    # a typedef of a named record or a pointer to one: valid as soon as the record is forward-declared
    t = c.underlying_typedef_type.get_canonical()
    while t.kind == ci.TypeKind.POINTER:
        t = t.get_pointee()
    if t.kind != ci.TypeKind.RECORD:
        return None
    d = t.get_declaration()
    if not d.spelling or '(' in d.spelling:
        return None
    kw = 'struct ' if d.kind == ci.CursorKind.STRUCT_DECL else 'union '
    if kw + d.spelling not in seen:
        return None
    return 'typedef %s;' % declarator(c.underlying_typedef_type.get_canonical(), c.spelling)


early = {}
for n in sorted(needed):
    c = decls.get(n)
    if c is not None and c.kind == ci.CursorKind.TYPEDEF_DECL:
        t = record_alias(c)
        if t:
            early[n] = t
order = []
placed = set()


def graph_deps(n):
    ds = deps(n)
    ptr = set(via_pointer)
    # through a pointer, a record (forward-declared) or its typedef name (up front) needs nothing before it
    return sorted(d for d in ds if d in seen and d != n and not (d in ptr and (d.startswith(('struct ', 'union ')) or d in early)))


graph = {n: graph_deps(n) for n in needed}
all_deps = {n: [d for d in deps(n) if d in seen and d != n] for n in needed}


def place(n, stack=()):
    if n in placed or n in stack:
        return
    for d in graph.get(n, []):
        place(d, stack + (n,))
    placed.add(n)
    order.append(n)


for n in sorted(needed):
    place(n)


def emit(n, enums_done):
    """the C text of one declaration, or None"""
    c = decls.get(n)
    if c is None and n in enum_of:
        e = enum_of[n]
        if e.hash in enums_done or (e.spelling and '(' not in e.spelling and 'enum ' + e.spelling in seen):
            return None
        enums_done.add(e.hash)
        vals = ', '.join('%s = %d' % (x.spelling, x.enum_value) for x in e.get_children()
                         if x.kind == ci.CursorKind.ENUM_CONSTANT_DECL)
        return 'enum { %s };' % vals
    if c is None or n in early:
        return None
    k = c.kind
    if k == ci.CursorKind.MACRO_DEFINITION:
        return macro_text(c) if c.location.file is not None else None
    if k == ci.CursorKind.TYPEDEF_DECL:
        if n in POINTER_SIZED:
            # the SDK parsed is 64-bit Windows': these follow the target's pointer size instead (wasm32 is 32-bit)
            return 'typedef %s %s;' % (POINTER_SIZED[n], n)
        u = c.underlying_typedef_type
        us = strip_type(u)
        d = us.get_declaration()
        if us.kind in (ci.TypeKind.RECORD, ci.TypeKind.ENUM) and (not d.spelling or '(' in d.spelling):
            if us.kind == ci.TypeKind.ENUM:
                vals = ', '.join('%s = %d' % (x.spelling, x.enum_value) for x in d.get_children())
                return 'typedef enum { %s } %s;' % (vals, n)
            kw = 'struct' if d.kind == ci.CursorKind.STRUCT_DECL else 'union'
            return packed(d, 'typedef %s {\n%s} %s;' % (kw, record_body(d), n))
        if us.kind == ci.TypeKind.FUNCTIONPROTO:
            return 'typedef %s %s(%s);' % (fix(us.get_result().spelling), n,
                                           ', '.join(fix(a.spelling) for a in us.argument_types()) or 'void')
        return 'typedef %s;' % declarator(u, n)
    if k in (ci.CursorKind.STRUCT_DECL, ci.CursorKind.UNION_DECL):
        return packed(c, '%s {\n%s};' % (n, record_body(c))) if c.is_definition() else None
    if k == ci.CursorKind.ENUM_DECL:
        if c.hash in enums_done:
            return None
        enums_done.add(c.hash)
        vals = ', '.join('%s = %d' % (x.spelling, x.enum_value) for x in c.get_children())
        return '%s { %s };' % (n, vals)
    if k == ci.CursorKind.FUNCTION_DECL:
        t = c.type
        if t.kind != ci.TypeKind.FUNCTIONPROTO:
            return '%s %s();' % (fix(t.get_result().spelling), n)
        args = [declarator(a.type, a.spelling or 'a%d' % i) for i, a in enumerate(c.get_arguments())]
        if t.is_function_variadic():
            args.append('...')
        return '%s %s(%s);' % (fix(t.get_result().spelling), n, ', '.join(args) or 'void')
    if k == ci.CursorKind.VAR_DECL:
        return 'extern %s;' % declarator(c.type, n)
    return None


groups = {}
for n in needed:
    groups.setdefault(group_of(n), None)
body = {g: [] for g in groups}
fwd = {g: [] for g in groups}
alias = {g: [] for g in groups}
uses = {g: set() for g in groups}
for n in sorted(needed):
    g = group_of(n)
    if n.startswith(('struct ', 'union ')):
        fwd[g].append('%s;' % n)
    if n in early:
        alias[g].append(early[n])
    for d in all_deps[n]:
        if group_of(d) != g:
            uses[g].add(group_of(d))
enums_done = set()
emitted = 0
skipped = []
for n in order:
    if n.startswith('@'):
        continue
    try:
        t = emit(n, enums_done)
    except Exception as ex:
        skipped.append('%s (%s)' % (n, ex))
        continue
    if t:
        body[group_of(n)].append(t)
        emitted += 1
    elif cursor_of(n) is None:
        skipped.append(n)

D = os.path.dirname(OUT)
os.makedirs(D, exist_ok=True)
for g in groups:
    f = group_file(g)
    guard = 'MINIWIN_' + re.sub(r'\W', '_', f).upper()
    what = 'windows.h and the C runtime\'s extensions' if g == 'base' else '<%s>' % g
    out = ['''/* GENERATED by tools/linux/gen_win32.py: the Windows SDK's declarations (names, values, layouts) that come with %s
 * and that the port's code uses. Do not edit. For non-Windows builds only (miniwin/include/win32/ is on their include
 * path). Windows' sizes are kept: the SDK's long is __LONG32 (32 bits, as on Windows: the target's long where that
 * is 32 bits, else int), WCHAR is 16-bit; the pointer-sized integers (INT_PTR, SIZE_T...) are the target's. */
#ifndef %s
#define %s
''' % (what, guard, guard)]
    out.append('#include "win32_prelude.h"' if g == 'base' else '#include "win32.h"')
    for u in sorted(uses[g] - {'base', g}):
        out.append('#include "%s"' % group_file(u))
    out.append('')
    if fwd[g]:
        out.append('/* the records, forward-declared */')
        out += fwd[g] + ['']
    if alias[g]:
        out.append('/* their typedef names, up front too */')
        out += alias[g] + ['']
    out += body[g]
    out.append('\n#endif')
    open(os.path.join(D, f), 'w').write('\n'.join(out) + '\n')
    print('  %-22s %4d declarations, uses %s' % (f, len(body[g]), ' '.join(sorted(uses[g] - {g})) or '-'),
          file=sys.stderr)

# the headers the code includes, each with its group (and what Windows' one brings with it)
THIN = {'windows.h': 'Windows', 'ddraw.h': 'DirectDraw', 'dsound.h': 'DirectSound', 'dinput.h': 'DirectInput',
        'mmsystem.h': 'the multimedia system', 'mmreg.h': 'the multimedia formats',
        'msacm.h': 'the audio compression manager', 'vfw.h': 'Video for Windows', 'commctrl.h': 'the common controls',
        'commdlg.h': 'the common dialogs', 'shlobj.h': 'the shell', 'dbghelp.h': 'the debug helper',
        'basetsd.h': 'the sized types', 'io.h': 'the C runtime\'s low-level files',
        'direct.h': 'the C runtime\'s directories', 'intrin.h': 'the compiler intrinsics',
        'winspool.h': 'the print spooler', 'objbase.h': 'COM', 'winbase.h': 'Windows', 'winuser.h': 'Windows',
        'wingdi.h': 'Windows'}
for h, what in THIN.items():
    g = 'MINIWIN_WIN32_THIN_%s' % re.sub(r'\W', '_', h).upper()
    inc = ['#include "win32.h"']
    if h in groups and h not in BASE:
        inc.append('#include "%s"' % group_file(h))
    open(os.path.join(D, h), 'w').write(
        '/* <%s> off Windows: %s, as declared by the generated win32*.h (tools/linux/gen_win32.py). */\n'
        '#ifndef %s\n#define %s\n%s\n#endif\n' % (h, what, g, g, '\n'.join(inc)))
print('wrote %d declarations in %d headers; skipped %d: %s' % (emitted, len(groups), len(skipped),
                                                               ' '.join(skipped[:30])), file=sys.stderr)

# ---- the GUIDs' values (the SDK's DEFINE_GUID lines; Windows has them in uuid.lib and dxguid.lib) ----
guid_values = {}
for c in tu.cursor.walk_preorder():
    if c.kind == ci.CursorKind.MACRO_INSTANTIATION and c.spelling == 'DEFINE_GUID':
        toks = [t.spelling for t in c.get_tokens()]
        args = [t for t in toks[2:] if t not in (',', ')', '(')]
        if len(args) >= 12:
            guid_values[args[0]] = args[1:12]
# defined in the SDK's headers only as declarations
guid_values.setdefault('GUID_NULL', ['0x00000000', '0x0000', '0x0000'] + ['0x00'] * 8)
guid_values.setdefault('IID_IUnknown', ['0x00000000', '0x0000', '0x0000', '0xC0', '0x00', '0x00', '0x00', '0x00', '0x00',
                                        '0x00', '0x46'])
guids = []
for n in sorted(needed):
    c = decls.get(n)
    if c is not None and c.kind == ci.CursorKind.VAR_DECL and n in guid_values:
        guids.append(n)
G = os.path.join(ROOT, 'miniwin/src/posix/guids.c')
os.makedirs(os.path.dirname(G), exist_ok=True)
lines = ['''/* GENERATED by tools/linux/gen_win32.py: the values of the GUIDs the port's code uses, from the Windows SDK's
 * DEFINE_GUID lines (on Windows they come from uuid.lib and dxguid.lib). Do not edit. */
#include <windows.h>
#include <ddraw.h>
#include <dinput.h>
#include <dsound.h>
''']
for n in guids:
    v = guid_values[n]
    lines.append('const GUID %s = {%s, %s, %s, {%s}};' % (n, v[0], v[1], v[2], ', '.join(v[3:11])))
open(G, 'w').write('\n'.join(lines) + '\n')
missing_guids = [n for n in sorted(needed) if decls.get(n) is not None and decls[n].kind == ci.CursorKind.VAR_DECL
                 and n not in guid_values]
print('GUIDs: %d; variables without a value: %s' % (len(guids), ' '.join(missing_guids) or '-'), file=sys.stderr)
