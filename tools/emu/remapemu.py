"""Compares RemapTexCoordsToCell (0x442040: moves a visitor's face / shirt texture coordinates from the template cell
to the chosen one) in the original exe and the port, both emulated. Random cells and faces, with texture coordinates
often exactly on a cell's edges; the faces must come out byte for byte the same.

  python remapemu.py <original exe> <port exe> <rvas.json> [trials] [seed]
"""
import json
import os
import random
import struct
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from drawemu import Machine, DATA, DATA_SIZE  # noqa: E402
from zbemu import call  # noqa: E402
from unicorn import UcError  # noqa: E402
from unicorn.x86_const import UC_X86_REG_EIP, UC_X86_REG_FPCW  # noqa: E402

CONT = DATA
ULPS = []
ENTRIES = DATA + 0x1000
FACES = DATA + 0x2000


def main():
    orig_exe, port_exe, rva_json = sys.argv[1:4]
    trials = int(sys.argv[4]) if len(sys.argv) > 4 else 2000
    rng = random.Random(int(sys.argv[5]) if len(sys.argv) > 5 else 1)
    rva = json.load(open(rva_json))
    pb = 0x400000
    sides = ((Machine(orig_exe, {}), 0x442040, 0x81c0c0), (Machine(port_exe, {}), pb + rva['RemapTexCoordsToCell'],
                                                            pb + rva['DAT_0081c0c0']))
    bad = 0
    changed_total = 0
    for t in range(trials):
        sizes = [rng.choice([16, 32, 64, 128, 256]) for _ in range(16)]
        base = rng.randint(0, 4)
        entries = []
        for _ in range(8):
            tex = rng.randint(0, 6)
            w, h = sizes[(tex + base) * 2 % 16], sizes[((tex + base) * 2 + 1) % 16]
            cw, ch = rng.randint(1, 40), rng.randint(1, 40)
            entries.append((tex, rng.randint(0, max(0, min(255, w - 1))), rng.randint(0, max(0, min(255, h - 1))),
                            cw, ch))
        src_i, dst_i = rng.randrange(8), rng.randrange(8)
        tex1 = entries[src_i][0] + base
        w1, h1 = sizes[tex1 * 2 % 16], sizes[(tex1 * 2 + 1) % 16]
        e1 = entries[src_i]
        faces = b''
        count = rng.randint(1, 40)
        for _ in range(count):
            flags = 0x2000 if rng.random() < 0.15 else rng.choice([0, 0x10, 0x1000])
            cell = tex1 if rng.random() < 0.85 else rng.randint(0, 10)
            uv = []
            for k in range(6):
                lo, span, size = (e1[1], e1[3], w1) if k % 2 == 0 else (e1[2], e1[4], h1)
                r = rng.random()
                if r < 0.4:  # exactly on a pixel boundary inside the cell, edges included
                    v = (lo + rng.randint(0, span + 1)) / size
                elif r < 0.6:  # just outside the cell
                    v = (lo + rng.choice([-1, span + 2])) / size
                elif r < 0.65:
                    v = rng.choice([-0.25, 1.25, 1.0, 0.0])
                else:
                    v = (lo + rng.random() * (span + 1)) / size
                uv.append(v)
            faces += struct.pack('<Iii6f', flags, rng.randint(0, 99), cell, *uv)
        tsize = struct.pack('<16i', *sizes)
        cont = struct.pack('<Ii', 0, base) + bytes(0x30 - 8) + struct.pack('<I', ENTRIES)
        ent = b''.join(struct.pack('<hBBBB', *e) for e in entries)
        out = []
        for m, func, tsize_addr in sides:
            m.uc.mem_write(DATA, bytes(DATA_SIZE))
            m.uc.mem_write(CONT, cont)
            m.uc.mem_write(ENTRIES, ent)
            m.uc.mem_write(FACES, faces)
            m.uc.mem_write(tsize_addr, tsize)
            m.uc.reg_write(UC_X86_REG_FPCW, 0x027f)  # the C runtime's default: 53-bit precision, round to nearest
            try:
                call(m, func, [CONT, src_i, dst_i, FACES, count])
                out.append(bytes(m.uc.mem_read(FACES, len(faces))))
            except UcError as e:
                out.append('stopped: %s at %08x' % (e, m.uc.reg_read(UC_X86_REG_EIP)))
        if not isinstance(out[0], str):
            changed_total += sum(1 for i in range(count) if out[0][i * 36:i * 36 + 36] != faces[i * 36:i * 36 + 36])
        if out[0] == out[1]:
            continue
        if not isinstance(out[0], str) and not isinstance(out[1], str):
            decision = 0
            max_ulp = 0
            for i in range(count):
                fa = struct.unpack_from('<Iii6f', out[0], i * 36)
                fb = struct.unpack_from('<Iii6f', out[1], i * 36)
                if fa[2] != fb[2]:
                    decision += 1
                for k in range(6):
                    ia = struct.unpack_from('<i', out[0], i * 36 + 12 + 4 * k)[0]
                    ib = struct.unpack_from('<i', out[1], i * 36 + 12 + 4 * k)[0]
                    max_ulp = max(max_ulp, abs(ia - ib))
            ULPS.append(max_ulp)
            if decision == 0:
                continue  # only rounding in the last bits; counted below
            print('trial %d: %d faces remapped by one version only' % (t, decision))
        bad += 1
        if bad <= 8:
            if isinstance(out[0], str) or isinstance(out[1], str):
                print('trial %d: original %s / port %s' % (t, out[0] if isinstance(out[0], str) else 'ok',
                                                           out[1] if isinstance(out[1], str) else 'ok'))
                continue
            for i in range(count):
                a, b = out[0][i * 36:i * 36 + 36], out[1][i * 36:i * 36 + 36]
                if a != b:
                    fa, fb, f0 = struct.unpack('<Iii6f', a), struct.unpack('<Iii6f', b), struct.unpack('<Iii6f',
                                                                                                    faces[i * 36:i * 36 + 36])
                    print('trial %d face %d (cell %s %dx%d): input cell %d uv %s' % (
                        t, i, e1, w1, h1, f0[2], ['%.6f' % v for v in f0[3:]]))
                    print('    original cell %d uv %s' % (fa[2], ['%.6f' % v for v in fa[3:]]))
                    print('    port     cell %d uv %s' % (fb[2], ['%.6f' % v for v in fb[3:]]))
                    break
    print('%d of %d trials differ in a remap decision (%d faces remapped by the original)' % (bad, trials,
                                                                                         changed_total))
    print('%d trials differ only in rounding; largest difference %d ulp' % (len(ULPS), max(ULPS) if ULPS else 0))


if __name__ == '__main__':
    main()
