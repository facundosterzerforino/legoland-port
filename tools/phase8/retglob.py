"""retglob.py NAME=TYPE ...: retypes globals in the scratch decomp (globals.h extern + globals.c definition, keeping
array suffixes and initializers) and drops the integer casts on values assigned to them
(NAME = (unsigned int)x; -> NAME = x;). TYPE is the full pointer type, e.g. 'struct IconNode *'."""
import glob
import os
import re
import sys

os.chdir(os.path.expanduser('~/scratch/dv64/src/legoland'))
ICAST = r'\((?:unsigned int|int|unsigned long|long|void \*|UINT_PTR|INT_PTR)\)\s*'
for arg in sys.argv[1:]:
    name, typ = arg.split('=', 1)
    typ = typ.strip()
    sep = '' if typ.endswith('*') else ' '
    for fn, pat, rep in (('globals.h', r'^extern (LEGO_EXPORT )?[^;()]*?\b%s(\[[^\]]*\])?;' % name,
                          r'extern \g<1>%s%s%s\g<2>;' % (typ, sep, name)),
                         ('globals.c', r'^(LEGO_EXPORT )?(?!extern)[A-Za-z][^;()=]*?\b%s(\[[^\]]*\])?( = [^;]*)?;' % name,
                          r'\g<1>%s%s%s\g<2>\g<3>;' % (typ, sep, name))):
        s = open(fn).read()
        s2, n = re.subn(pat, rep, s, flags=re.M)
        if n != 1:
            print('%s: %s matched %d times' % (fn, name, n))
            continue
        open(fn, 'w').write(s2)
    tot = 0
    for f in glob.glob('*.c'):
        s = open(f).read()
        s2 = re.sub(r'\b(%s(?:\[[^\]]*\])?) = %s' % (name, ICAST), r'\1 = ', s)
        if s2 != s:
            tot += sum(1 for a, b in zip(s.split('\n'), s2.split('\n')) if a != b)
            open(f, 'w').write(s2)
    print('%s -> %s (%d assignment casts dropped)' % (name, typ, tot))
