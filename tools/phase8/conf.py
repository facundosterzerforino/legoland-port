"""conf.py [file...]: the port's conflict hunks (diff3) with line numbers; default all unmerged files"""
import os
import re
import subprocess
import sys

os.chdir(os.path.expanduser('~/wt/port'))
files = sys.argv[1:] or subprocess.run(['git', 'diff', '--name-only', '--diff-filter=U'], capture_output=True,
                                       text=True).stdout.split()
for f in files:
    L = open(f).read().split('\n')
    i = 0
    while i < len(L):
        if L[i].startswith('<<<<<<<'):
            j = i
            while not L[j].startswith('>>>>>>>'):
                j += 1
            print('==== %s:%d-%d' % (f, i + 1, j + 1))
            print('\n'.join(L[i:j + 1]))
            i = j
        i += 1
