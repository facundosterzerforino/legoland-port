# take_theirs.py file...: resolves every diff3 conflict block in the files with the "theirs" side
import re, sys
for f in sys.argv[1:]:
    s = open(f, encoding='latin-1').read()
    s2, n = re.subn(r'^<<<<<<< [^\n]*\n.*?^=======\n(.*?)^>>>>>>> [^\n]*\n', r'\1', s, flags=re.M | re.S)
    open(f, 'w', encoding='latin-1').write(s2)
    print(f, n, 'blocks resolved with theirs')
