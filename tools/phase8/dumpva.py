"""dumpva.py va len [va len ...]: the original exe's initialized bytes at a virtual address, as dwords"""
import struct
import sys
from pathlib import Path

exe = Path('~/legoland/external/legoland.exe').expanduser().read_bytes()
pe = struct.unpack_from('<I', exe, 0x3C)[0]
nsec = struct.unpack_from('<H', exe, pe + 6)[0]
optsz = struct.unpack_from('<H', exe, pe + 20)[0]
base = struct.unpack_from('<I', exe, pe + 24 + 28)[0]
secs = []
for i in range(nsec):
    o = pe + 24 + optsz + 40 * i
    vsz, va, rsz, raw = struct.unpack_from('<IIII', exe, o + 8)
    secs.append((base + va, rsz, raw))
args = sys.argv[1:]
for k in range(0, len(args), 2):
    va, n = int(args[k], 16), int(args[k + 1], 16)
    for sva, rsz, raw in secs:
        if sva <= va < sva + rsz:
            b = exe[raw + va - sva: raw + va - sva + n]
            print('0x%08x:' % va)
            for i in range(0, len(b), 16):
                ws = [struct.unpack_from('<I', b, i + j)[0] for j in range(0, min(16, len(b) - i), 4)]
                print('  +0x%02x  %s' % (i, ' '.join('%08x' % w for w in ws)))
            break
