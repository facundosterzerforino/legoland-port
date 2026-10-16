"""tools/linux/check_layouts.py: the layouts of the generated Windows declarations (miniwin/include/win32/) against
the Windows SDK's own, field by field. Run from the repo with libclang (uv run --with libclang==18.1.1), after
tools/linux/gen_win32.py; needs the Windows SDK (~/xwin) and, for wasm32, Emscripten's sysroot (~/emsdk).

Every struct and union the generated headers define is compared, by name, with the SDK's: its size and every
member's offset, on 64 bits (the SDK for x64 Windows against x86-64 Linux, as Linux and Android build) and on 32 bits
(the SDK for x86 Windows against wasm32, as the web builds). A difference means the code and Windows' layouts
disagree: data read from files or handed between the game and miniwin lands in the wrong place. The generator
caught two this way: #pragma pack (WAVEFORMATEX 18 bytes, not 20) and msacm.h's driver area, sized by _WIN64.

Known and deliberate: the CPU context and the PE headers are x64's on every target (the crash report); the
minidump records are the Windows watchdog's (port/port_watchdog.c isn't built elsewhere), and some contain the
context."""
import os
import re
import subprocess
import sys

import clang.cindex as ci

ROOT = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
XWIN = os.environ.get('XWIN', os.path.expanduser('~/xwin'))
EMSYSROOT = os.path.expanduser('~/emsdk/upstream/emscripten/cache/sysroot')
GEN = os.path.join(ROOT, 'miniwin/include/win32')
src = open(os.path.join(ROOT, 'tools/linux/gen_win32.py')).read()
HEADERS = eval(re.search(r'^HEADERS = (\[.*?\])', src, re.M | re.S).group(1))
RES = subprocess.run(['clang', '-print-resource-dir'], capture_output=True, text=True).stdout.strip()
# deliberate: x64's on every target (exceptlog.c, port_watchdog.c)
DELIBERATE = re.compile(r'^_?(CONTEXT|IMAGE_NT_HEADERS|IMAGE_OPTIONAL_HEADER|XSAVE_FORMAT|XMM_SAVE_AREA32|M128A|'
                        r'FLOATING_SAVE_AREA|EXCEPTION_POINTERS|MINIDUMP_)')

SDK_COMMON = ['-x', 'c', '-fms-extensions', '-fms-compatibility', '-fms-compatibility-version=19.40', '-D_WIN32',
              '-DWIN32', '-D_WINDOWS', '-DDIRECTINPUT_VERSION=0x0500', '-D_CRT_SECURE_NO_WARNINGS', '-w',
              '-isystem', XWIN + '/crt/include', '-isystem', XWIN + '/sdk/include/ucrt', '-isystem',
              XWIN + '/sdk/include/um', '-isystem', XWIN + '/sdk/include/shared', '-resource-dir', RES]
SDK = {64: ['--target=x86_64-pc-windows-msvc', '-D_WIN64', '-D_M_X64', '-D_AMD64_'] + SDK_COMMON,
       32: ['--target=i686-pc-windows-msvc', '-D_M_IX86=600', '-D_X86_'] + SDK_COMMON}
GEN_COMMON = ['-x', 'c', '-fms-extensions', '-fshort-wchar', '-fsigned-char', '-w', '-I', GEN, '-resource-dir', RES,
              '-D_GNU_SOURCE']
PORT = {64: ['--target=x86_64-linux-gnu'] + GEN_COMMON,
        32: ['--target=wasm32-unknown-emscripten', '--sysroot=' + EMSYSROOT, '-D__EMSCRIPTEN__'] + GEN_COMMON}


def records(args, only_files=None):
    """name -> (size, {field: offset in bytes}) for every struct/union defined (by tag, or by typedef name)"""
    text = ''.join('#include <%s>\n' % h for h in HEADERS)
    tu = ci.Index.create().parse('check.c', args=args, unsaved_files=[('check.c', text)])
    errors = [d for d in tu.diagnostics if d.severity >= ci.Diagnostic.Error]
    if errors:
        for d in errors[:10]:
            print('  parse error:', d)
    out = {}

    def fields(t, prefix=''):
        res = {}
        decl = t.get_declaration()
        for f in decl.get_children():
            if f.kind == ci.CursorKind.FIELD_DECL and f.spelling:
                off = t.get_offset(f.spelling)
                if off >= 0:
                    res[prefix + f.spelling] = off // 8
        return res

    for c in tu.cursor.walk_preorder():
        if c.location.file is None:
            continue
        if only_files is not None and not str(c.location.file).startswith(only_files):
            continue
        name = None
        t = None
        if c.kind in (ci.CursorKind.STRUCT_DECL, ci.CursorKind.UNION_DECL) and c.is_definition() and c.spelling \
                and '(' not in c.spelling:
            name, t = c.spelling, c.type
        elif c.kind == ci.CursorKind.TYPEDEF_DECL:
            u = c.underlying_typedef_type.get_canonical()
            if u.kind == ci.TypeKind.RECORD and u.get_declaration().is_definition():
                name, t = c.spelling, u
        if name is None or name in out:
            continue
        size = t.get_size()
        if size < 0:
            continue
        out[name] = (size, fields(t))
    return out


def main():
    problems = 0
    for bits in (64, 32):
        sdk = records(SDK[bits])
        port = records(PORT[bits], only_files=GEN)
        compared = 0
        for name, (size, fl) in sorted(port.items()):
            if name not in sdk:
                continue
            compared += 1
            ssize, sfl = sdk[name]
            diffs = []
            if size != ssize:
                diffs.append('size %d, Windows %d' % (size, ssize))
            for f, off in fl.items():
                if f in sfl and sfl[f] != off:
                    diffs.append('%s at %d, Windows %d' % (f, off, sfl[f]))
            if diffs:
                tag = 'deliberate' if DELIBERATE.match(name) else 'DIFFERS'
                if tag == 'DIFFERS':
                    problems += 1
                print('%d-bit %s %s: %s' % (bits, tag, name, '; '.join(diffs[:4])))
        print('%d-bit: %d records compared with the SDK\'s' % (bits, compared))
    print('layouts: %s' % ('the same as Windows\'' if problems == 0 else '%d differ' % problems))
    return 1 if problems else 0


if __name__ == '__main__':
    sys.exit(main())
