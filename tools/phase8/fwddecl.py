"""fwddecl.py root [--dry]: in every header of root/src/legoland, a struct first named inside a function prototype's
parameter list (C gives it prototype scope: a different type from the real one) gets a file-scope
`struct X;` before its first use"""
import os
import re
import sys

os.chdir(os.path.expanduser(sys.argv[1]) + '/src/legoland')
total = 0
for f in sorted(os.listdir('.')):
    if not f.endswith('.h'):
        continue
    s = open(f, encoding='latin-1').read()
    lines = s.split('\n')
    declared = set()
    need = {}
    depth = 0
    for i, l in enumerate(lines):
        code = re.sub(r'/\*.*?\*/|//.*', '', l)
        for m in re.finditer(r'\bstruct (\w+)\s*(;|\{)', code):
            if depth == 0:
                declared.add(m.group(1))
        is_proto = depth == 0 and re.match(r'^[A-Za-z_].*\w\s*\(.*\)\s*;\s*$', code) and 'typedef' not in code
        if is_proto:
            params = code[code.index('(') + 1:code.rindex(')')]
            for m in re.finditer(r'\bstruct (\w+)', params):
                if m.group(1) not in declared and m.group(1) not in need:
                    need[m.group(1)] = i
        elif depth == 0:
            for m in re.finditer(r'\bstruct (\w+)', code):
                declared.add(m.group(1))
        depth += code.count('{') - code.count('}')
    if not need:
        continue
    for name, i in sorted(need.items(), key=lambda x: -x[1]):
        lines.insert(i, 'struct %s;' % name)
    total += len(need)
    print('%s: %s' % (f, ' '.join(sorted(need))))
    if '--dry' not in sys.argv:
        open(f, 'w', encoding='latin-1').write('\n'.join(lines))
print(total, 'forward declarations')
