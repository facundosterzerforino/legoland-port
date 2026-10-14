"""resolve.py file:spec ...: resolves the port's diff3 conflict hunks; spec is one letter per hunk (o = ours,
t = theirs) or a single letter for all hunks. The file is git-added when no conflict is left."""
import os
import subprocess
import sys

os.chdir(os.path.expanduser('~/wt/port'))
for arg in sys.argv[1:]:
    f, spec = arg.rsplit(':', 1)
    f = f if '/' in f else 'src/legoland/' + f
    L = open(f).read().split('\n')
    out = []
    k = 0
    i = 0
    while i < len(L):
        if L[i].startswith('<<<<<<<'):
            j = i
            while not L[j].startswith('|||||||'):
                j += 1
            b = j
            while not L[b].startswith('======='):
                b += 1
            e = b
            while not L[e].startswith('>>>>>>>'):
                e += 1
            c = spec[k] if len(spec) > 1 else spec
            out += L[i + 1:j] if c == 'o' else L[b + 1:e]
            k += 1
            i = e + 1
            continue
        out.append(L[i])
        i += 1
    s = '\n'.join(out).replace('(unsigned int *)(unsigned int *)&', '(unsigned int *)&')
    open(f, 'w').write(s)
    left = s.count('\n<<<<<<<')
    if left == 0:
        subprocess.run(['git', 'add', f])
    print('%s: %d hunk(s) resolved, %d left' % (f, k, left))
