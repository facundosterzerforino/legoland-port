# fdef.py name... : each function's definition (signature and body, up to N lines) in the scratch decomp, and its
# prototype in the headers
import glob
import os
import re
import sys

os.chdir(os.path.expanduser('~/scratch/dv64/src/legoland'))
maxlines = int(os.environ.get('N', '25'))
files = {f: open(f, errors='replace').read() for f in glob.glob('*.c') + glob.glob('*.h')}
for name in sys.argv[1:]:
    for f, s in files.items():
        if f.endswith('.h'):
            for m in re.finditer(r'^[^\n;]*\b%s\s*\([^;{]*\);' % name, s, re.M):
                print('%s: %s' % (f, m.group(0).strip()))
            continue
        m = re.search(r'^[^\n;]*\b%s\s*\([^;{]*\)\s*\{' % name, s, re.M)
        if not m:
            continue
        depth, i = 1, m.end()
        while depth and i < len(s):
            depth += {'{': 1, '}': -1}.get(s[i], 0)
            i += 1
        body = s[m.start():i].split('\n')
        line = s[:m.start()].count('\n') + 1
        print('== %s:%d (%d lines)' % (f, line, len(body)))
        print('\n'.join(body[:maxlines]))
        if len(body) > maxlines:
            print('    ...')
