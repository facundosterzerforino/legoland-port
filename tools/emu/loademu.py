"""Compares Load3DMesh (0x43fa80: reads a visitor mesh file and converts it to fixed point) in the original exe and
the port, both emulated. The file functions read a synthetic mesh file, malloc hands out the same addresses on both
sides, and the colour sets the solid faces ask for are recorded; the whole loaded mesh must come out the same.

  python loademu.py <original exe> <port exe> <rvas.json> [trials] [seed]
"""
import json
import os
import random
import struct
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from drawemu import Machine, DATA, DATA_SIZE, STACK, STACK_SIZE, SENTINEL  # noqa: E402
from unicorn import UC_HOOK_CODE, UcError  # noqa: E402
from unicorn.x86_const import UC_X86_REG_EAX, UC_X86_REG_EIP, UC_X86_REG_ESP, UC_X86_REG_FPCW  # noqa: E402

HEAP = DATA + 0x10000


class Loader:
    """The hooked helpers for one machine."""

    def __init__(self, machine, addrs):
        self.m = machine
        self.file = b''
        self.pos = 0
        self.heap = HEAP
        self.colours = []
        self.allocs = []
        for name, addr in addrs.items():
            machine.uc.hook_add(UC_HOOK_CODE, self.hook, name, begin=addr, end=addr)

    def reset(self, file):
        self.file = file
        self.pos = 0
        self.heap = HEAP
        self.colours = []
        self.allocs = []

    def hook(self, uc, address, size, name):
        esp = uc.reg_read(UC_X86_REG_ESP)
        ret = struct.unpack('<I', uc.mem_read(esp, 4))[0]
        a = struct.unpack('<4I', uc.mem_read(esp + 4, 16))
        result = 0
        if name == 'open':
            self.pos = 0
            result = 0x5000
        elif name == 'read':
            data = self.file[self.pos:self.pos + a[2]]
            self.pos += a[2]
            uc.mem_write(a[1], data + bytes(a[2] - len(data)))
            result = len(data)
        elif name == 'malloc':
            result = self.heap
            self.allocs.append(('malloc', a[0]))
            self.heap = (self.heap + a[0] + 63) & ~63
        elif name == 'calloc':  # the port's compiler turned malloc + memset into calloc (the heap is zeroed)
            result = self.heap
            self.allocs.append(('calloc', a[0] * a[1]))
            self.heap = (self.heap + a[0] * a[1] + 63) & ~63
        elif name == 'colour':
            rgb = bytes(uc.mem_read(a[1], 3))
            self.colours.append((a[0], rgb))
            result = 0x07000000 | int.from_bytes(rgb, 'big')
        uc.reg_write(UC_X86_REG_EAX, result)
        uc.reg_write(UC_X86_REG_ESP, esp + 4)
        uc.reg_write(UC_X86_REG_EIP, ret)


def make_file(rng):
    def coord():
        r = rng.random()
        if r < 0.2:  # exactly halfway between two 1/65536 steps: the rounding mode decides
            return (rng.randint(-2000000, 2000000) + 0.5) / 65536
        if r < 0.3:
            return float(rng.randint(-40, 40))
        return rng.uniform(-40, 40)

    elems = rng.randint(1, 4)
    out = struct.pack('<i', elems)
    for _ in range(elems):
        n = rng.randint(1, 50)
        out += struct.pack('<i', n) + struct.pack('<%df' % (n * 3), *[coord() for _ in range(n * 3)])
        nn = rng.randint(1, 60)
        norms = []
        for _ in range(nn):
            v = [rng.uniform(-1, 1) for _ in range(3)]
            if rng.random() < 0.05:
                v = [0.0, 0.0, 0.0]
            norms += v
        out += struct.pack('<i', nn) + struct.pack('<%df' % (nn * 3), *norms)
    tris = rng.randint(1, 80)
    out += struct.pack('<ii', tris, rng.randint(0, tris))
    out += struct.pack('<%di' % (tris * 3), *[rng.randint(0, 49) for _ in range(tris * 3)])
    for _ in range(tris):
        flags = 0x2000 if rng.random() < 0.5 else rng.choice([0, 0x10])
        uv = [rng.choice([rng.uniform(0, 1), rng.uniform(-0.5, 1.5), 0.0, 1.0]) for _ in range(6)]
        out += struct.pack('<I3BxI6f', flags, rng.randrange(256), rng.randrange(256), rng.randrange(256),
                           rng.randrange(500), *uv)
    return out


def main():
    orig_exe, port_exe, rva_json = sys.argv[1:4]
    trials = int(sys.argv[4]) if len(sys.argv) > 4 else 500
    rng = random.Random(int(sys.argv[5]) if len(sys.argv) > 5 else 1)
    rva = json.load(open(rva_json))
    pb = 0x400000
    orig = Machine(orig_exe, {})
    port = Machine(port_exe, {})
    sides = (
        (orig, Loader(orig, {'sprintf': 0x49e573, 'open': 0x489b60, 'read': 0x489cf0, 'close': 0x489de0,
                             'malloc': 0x49e4ff, 'colour': 0x486280}), 0x43fa80),
        (port, Loader(port, {'sprintf': pb + rva['sprintf'], 'open': pb + rva['RES_OpenFile'],
                             'read': pb + rva['RES_ReadFile'], 'close': pb + rva['RES_CloseFile'],
                             'malloc': pb + rva['malloc'], 'calloc': pb + rva['calloc'],
                             'colour': pb + rva['FUN_00486280']}),
         pb + rva['Load3DMesh']),
    )
    bad = 0
    for t in range(trials):
        f = make_file(rng)
        ctx = rng.randint(0, 50)
        res = []
        for m, ld, func in sides:
            m.uc.mem_write(DATA, bytes(DATA_SIZE))
            m.uc.mem_write(DATA, b'Name.3d\0dir\0')
            ld.reset(f)
            m.uc.reg_write(UC_X86_REG_FPCW, 0x027f)  # the C runtime's default
            esp = STACK + STACK_SIZE - 0x100
            for v in (ctx, DATA + 8, DATA):
                esp -= 4
                m.uc.mem_write(esp, struct.pack('<I', v))
            esp -= 4
            m.uc.mem_write(esp, struct.pack('<I', SENTINEL))
            m.uc.reg_write(UC_X86_REG_ESP, esp)
            try:
                m.uc.emu_start(func, SENTINEL, count=20_000_000)
                res.append((m.uc.reg_read(UC_X86_REG_EAX), bytes(m.uc.mem_read(HEAP, ld.heap - HEAP)), ld.colours))
            except UcError as e:
                res.append('stopped: %s at %08x' % (e, m.uc.reg_read(UC_X86_REG_EIP)))
        if res[0] == res[1]:
            continue
        bad += 1
        if bad > 6:
            continue
        if isinstance(res[0], str) or isinstance(res[1], str):
            print('trial %d: original %s / port %s' % (t, res[0] if isinstance(res[0], str) else 'ok',
                                                       res[1] if isinstance(res[1], str) else 'ok'))
            continue
        (ra, ha, ca), (rb, hb, cb) = res
        if ra != rb or len(ha) != len(hb):
            print('trial %d: mesh %08x vs %08x, heap %d vs %d bytes' % (t, ra, rb, len(ha), len(hb)))
            print('   original allocations', sides[0][1].allocs[:12])
            print('   port allocations    ', sides[1][1].allocs[:12])
        elif ca != cb:
            print('trial %d: colour requests differ: %s vs %s' % (t, ca[:4], cb[:4]))
        else:
            diffs = [i for i in range(0, len(ha), 4) if ha[i:i + 4] != hb[i:i + 4]]
            i = diffs[0]
            starts = []
            off = 0
            for kind, n in sides[0][1].allocs:
                starts.append((off, kind, n))
                off = (off + n + 63) & ~63
            blk = [k for k, st in enumerate(starts) if st[0] <= i < st[0] + max(st[2], 1)]
            # allocation order per element: verts, norms (after the header, the shared header and the elements)
            if blk:
                k = blk[0]
                what = 'header' if k == 0 else 'shared' if k == 1 else 'elements' if k == 2 else (
                    'vertices' if (k - 3) % 2 == 0 else 'normals') if k < len(starts) - 2 else (
                    'triangles' if k == len(starts) - 2 else 'faces')
                print('   in allocation %d (%s), word %d' % (k, what, (i - starts[k][0]) // 4))
            print('trial %d: %d words differ, first at heap+%#x: original %08x port %08x' % (
                t, len(diffs), i, struct.unpack_from('<I', ha, i)[0], struct.unpack_from('<I', hb, i)[0]))
    print('%d of %d trials differ' % (bad, trials))


if __name__ == '__main__':
    main()
