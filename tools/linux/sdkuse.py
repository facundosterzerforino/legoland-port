"""tools/linux/sdkuse.py: every Windows SDK / C runtime declaration (function, type, macro) the port's own code uses
(game, port, miniwin, extensions), parsed as the x64 Windows build compiles it (ninja's commands, so build
build-clang-x64 first). Writes tools/linux/sdkuse.json (the input of gen_win32.py) and, grouped by kind and SDK
header with how many of our files use each, /tmp/sdkuse.txt."""
import collections
import os
import re
import shlex
import subprocess
import sys

import clang.cindex as ci

ROOT = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
B = ROOT + '/build-clang-x64'
XWIN = os.environ.get('XWIN', os.path.expanduser('~/xwin'))
cmds = subprocess.run(['ninja', '-C', B, '-t', 'commands'], capture_output=True, text=True).stdout.splitlines()
files = {}
for c in cmds:
    m = re.search(r' -- (\S+\.c)$', c)
    if not m or 'clang-cl' not in c:
        continue
    src = m.group(1)
    if not re.search(r'/(src/legoland|port|miniwin/src|extensions)/', src) or '/_deps/' in src or '/dmusic/' in src \
            or '/ffmpeg/' in src or '/blast/' in src or '/puff/' in src:
        continue
    if src.endswith(('/imports.c', '/bootstrap.c', '/crt.c')):
        continue
    args = shlex.split(c.split(' -- ')[0])[1:]
    # clang-cl flags to clang driver flags
    out = ['--driver-mode=cl']
    skip = False
    for a in args:
        if skip:
            skip = False
            continue
        if a.startswith(('/Fo', '/Fd', '/showIncludes', '/FS', '-c', '/c')):
            continue
        out.append(a)
    files[src] = out

idx = ci.Index.create()
use = {'function': collections.defaultdict(set), 'type': collections.defaultdict(set),
       'macro': collections.defaultdict(set), 'var': collections.defaultdict(set)}
header_of = {}


def sdk(path):
    return path is not None and (path.startswith(XWIN) or '/crt/include' in path or '/sdk/include' in path)


for n, (src, args) in enumerate(sorted(files.items())):
    tu = idx.parse(src, args=args, options=ci.TranslationUnit.PARSE_DETAILED_PROCESSING_RECORD)
    me = os.path.relpath(src, ROOT)
    for c in tu.cursor.walk_preorder():
        if c.location.file is None or sdk(c.location.file.name):
            continue
        kind = None
        ref = None
        if c.kind == ci.CursorKind.MACRO_INSTANTIATION:
            ref = c.referenced
            kind = 'macro'
        elif c.kind in (ci.CursorKind.DECL_REF_EXPR, ci.CursorKind.TYPE_REF):
            ref = c.referenced
            if ref is not None:
                if ref.kind == ci.CursorKind.FUNCTION_DECL:
                    kind = 'function'
                elif ref.kind in (ci.CursorKind.VAR_DECL,):
                    kind = 'var'
                elif ref.kind in (ci.CursorKind.ENUM_CONSTANT_DECL,):
                    kind = 'macro'
                else:
                    kind = 'type'
        if kind is None or ref is None or ref.location.file is None or not sdk(ref.location.file.name):
            continue
        name = ref.spelling
        if not name:
            continue
        use[kind][name].add(me)
        header_of[(kind, name)] = os.path.basename(ref.location.file.name)
    print('%d/%d %s' % (n + 1, len(files), me), file=sys.stderr)

with open('/tmp/sdkuse.txt', 'w') as f:
    for kind in ('function', 'type', 'macro', 'var'):
        by_header = collections.defaultdict(list)
        for name, users in use[kind].items():
            by_header[header_of[(kind, name)]].append((name, len(users)))
        total = len(use[kind])
        f.write('== %s: %d\n' % (kind, total))
        for h, names in sorted(by_header.items(), key=lambda kv: -len(kv[1])):
            f.write('  %s (%d): %s\n' % (h, len(names), ' '.join('%s/%d' % nu for nu in sorted(names))))
import json
json.dump({k: {n: [header_of[(k, n)], sorted(u)] for n, u in sorted(v.items())} for k, v in use.items()},
          open(os.path.join(ROOT, 'tools/linux/sdkuse.json'), 'w'), indent=0)
print('files', len(files))
for kind in ('function', 'type', 'macro', 'var'):
    print(kind, len(use[kind]))
