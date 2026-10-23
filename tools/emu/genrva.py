import re, subprocess, os, sys
P = os.path.expanduser('~/wt/port')
pdb = subprocess.run(['llvm-pdbutil', 'dump', '--publics', P + '/build-clang-x86/legoland.pdb'], capture_output=True, text=True).stdout
hdr = subprocess.run(['llvm-readobj', '--sections', P + '/build-clang-x86/legoland.exe'], capture_output=True, text=True).stdout
secva = [int(m, 16) for m in re.findall(r'VirtualAddress: (0x[0-9A-Fa-f]+)', hdr)]
names = ['FUN_00440a30', 'FUN_00486590', 'FUN_00486c70', 'FUN_004877b0', 'FUN_00487d40', 'FUN_004864e0', 'FUN_004886e0',
         'GetBlokeAnim3DFromPerson', 'FUN_00485fe0', 'ZBufferHelper', 'OverrideFrame', 'SoftPrint_IndexedLLS',
         'OverridePalette', 'DAT_007fe9a8', 'DAT_007feb14', 'DAT_00797e68', 'DAT_00701e58',
         'RemapTexCoordsToCell', 'TextureSizes', 'FLOAT_004ab390', 'Load3DMesh', 'RES_OpenFile', 'RES_ReadFile',
         'RES_CloseFile', 'malloc', 'calloc', 'sprintf', 'FUN_00486280', 'GetGFXFName', 'RES_SetFilePointer',
         'RES_GetFilePointer', 'LoadBmpIntoImage', 'DAT_0081c4c0', 'free']
out = {}
for n in names:
    m = re.search(r'`_' + n + r'`\s*\n\s*flags = [a-z]+, addr = (\d+):(\d+)', pdb)
    sec, off = int(m.group(1)), int(m.group(2))
    out[n] = secva[sec - 1] + off
src = open(P + '/src/legoland/man3d.c').read()
m = re.search(r'// FUNCTION: LEGOLAND (0x[0-9a-f]+)\s*\n[^\n]*GetBlokeAnim3DFromPerson\(', src)
out['ORIG_GetBlokeAnim3DFromPerson'] = int(m.group(1), 16)
out['ORIG_GetGFXFName'] = 0x44de90
out['ORIG_free'] = 0x49e4d0
import json
json.dump(out, open(sys.argv[1], 'w'), indent=1)
print(out)
