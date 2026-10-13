"""Compares the original person drawer FUN_00440a30 (asm in the original legoland.exe) with the port's C version
(the port's legoland.exe), both run in a CPU emulator (Unicorn): nothing runs natively. The helpers they call are
intercepted: the triangle fillers and the colour/texture selection are recorded instead of drawn, and the mesh lookup
returns a synthetic test mesh. For the same person and mesh the two recordings must match.

  python drawemu.py <original exe> <port exe> <port pdb rvas.json> [trials] [seed]
"""
import json
import math
import random
import struct
import sys

from unicorn import Uc, UC_ARCH_X86, UC_MODE_32, UC_HOOK_CODE, UcError
from unicorn.x86_const import (UC_X86_REG_EAX, UC_X86_REG_EIP, UC_X86_REG_ESP, UC_X86_REG_FPCW, UC_X86_REG_CR0,
                               UC_X86_REG_CR4)

STACK = 0x00100000
STACK_SIZE = 0x00100000
DATA = 0x20000000
DATA_SIZE = 0x00100000
SENTINEL = 0x30000000


def load_pe(uc, path):
    d = open(path, 'rb').read()
    pe = struct.unpack_from('<I', d, 0x3c)[0]
    nsec = struct.unpack_from('<H', d, pe + 6)[0]
    opt = struct.unpack_from('<H', d, pe + 20)[0]
    base = struct.unpack_from('<I', d, pe + 24 + 28)[0]
    size = struct.unpack_from('<I', d, pe + 24 + 56)[0]
    uc.mem_map(base, (size + 0xfff) & ~0xfff)
    uc.mem_write(base, d[:0x1000])
    for i in range(nsec):
        o = pe + 24 + opt + 40 * i
        vs, va, rs, ro = struct.unpack_from('<IIII', d, o + 8)
        uc.mem_write(base + va, d[ro:ro + min(rs, vs if vs else rs)])
    return base


class Machine:
    def __init__(self, exe, helpers):
        self.uc = Uc(UC_ARCH_X86, UC_MODE_32)
        self.base = load_pe(self.uc, exe)
        self.uc.mem_map(STACK, STACK_SIZE)
        self.uc.mem_map(DATA, DATA_SIZE)
        self.uc.mem_map(SENTINEL, 0x1000)
        self.uc.reg_write(UC_X86_REG_CR0, 0x00000033)  # protected mode, FPU present (MP, ET), no emulation
        self.uc.reg_write(UC_X86_REG_CR4, 0x00000600)  # SSE enabled (OSFXSR, OSXMMEXCPT)
        self.calls = []
        self.colour = 0
        self.texture = 0
        self.anim = 0
        for name, addr in helpers.items():
            self.uc.hook_add(UC_HOOK_CODE, self.hook, name, begin=addr, end=addr)

    def u32(self, a):
        return struct.unpack('<I', self.uc.mem_read(a, 4))[0]

    def hook(self, uc, address, size, name):
        esp = uc.reg_read(UC_X86_REG_ESP)
        ret = self.u32(esp)
        args = struct.unpack('<4I', uc.mem_read(esp + 4, 16))
        if name.startswith('fill'):
            verts = [bytes(uc.mem_read(args[k], 0x1c)) for k in range(3)]
            self.calls.append((name, self.colour, self.texture, verts))
            if name == 'fill2':  # the flat filler writes the first vertex's shade * 64 back (both versions)
                shade = struct.unpack_from('<i', verts[0], 0x14)[0]
                uc.mem_write(args[0] + 0x14, struct.pack('<i', (shade << 6) & 0xffffffff if shade >= 0 else
                                                         ((shade << 6) & 0xffffffff)))
        elif name == 'colour':
            self.colour = args[0]
        elif name == 'texture':
            self.texture = args[0]
        uc.reg_write(UC_X86_REG_EAX, self.anim if name == 'anim' else 0)
        uc.reg_write(UC_X86_REG_ESP, esp + 4)  # cdecl: the caller removes the arguments
        uc.reg_write(UC_X86_REG_EIP, ret)

    def call(self, func, person):
        self.calls = []
        esp = STACK + STACK_SIZE - 0x100
        esp -= 4
        self.uc.mem_write(esp, struct.pack('<I', person))
        esp -= 4
        self.uc.mem_write(esp, struct.pack('<I', SENTINEL))
        self.uc.reg_write(UC_X86_REG_ESP, esp)
        self.uc.reg_write(UC_X86_REG_FPCW, 0x007f)  # DAT_004b7abc, as Render3DPerson sets it
        self.uc.emu_start(func, SENTINEL, count=50_000_000)
        return self.calls


def build(rng, mem):
    """Writes a random person and mesh into the data region (same bytes for both machines); returns
    (person address, anim address, description)."""
    nv = rng.randint(4, 60)
    nt = rng.randint(1, 120)
    ns = rng.randint(0, nt)
    out = bytearray(DATA_SIZE)
    addr = {}
    pos = 0

    def alloc(name, data):
        nonlocal pos
        addr[name] = DATA + pos
        out[pos:pos + len(data)] = data
        pos = (pos + len(data) + 15) & ~15

    verts = [rng.randint(-(30 << 16), 30 << 16) for _ in range(nv * 3)]
    norms = []
    for _ in range(200 * 3):
        x, y, z = (rng.randint(-1000, 1000) for _ in range(3))
        n = math.sqrt(x * x + y * y + z * z) or 1.0
        norms += [int(x / n * 65536), int(y / n * 65536), int(z / n * 65536)]
    tris = [rng.randrange(nv) for _ in range(nt * 3)]
    faces = b''
    for _ in range(nt):
        flags = 0x2000 if rng.random() < 0.5 else 0
        faces += struct.pack('<I4BI6f', flags, 0, 0, 0, 0, rng.randrange(1000), *[rng.randrange(1000) / 1000 for _ in range(6)])
    alloc('verts', struct.pack('<%di' % len(verts), *verts))
    alloc('norms', struct.pack('<%di' % len(norms), *norms))
    alloc('tris', struct.pack('<%di' % len(tris), *tris))
    alloc('faces', faces)
    alloc('shared', struct.pack('<iiI', nt, ns, addr['tris']))
    mins = [min(verts[i::3]) for i in range(3)]
    maxs = [max(verts[i::3]) for i in range(3)]
    elem = struct.pack('<6iiIIiI', *mins, *maxs, nv, addr['verts'], addr['shared'], 600, addr['norms']) + b'\0' * 12
    alloc('elems', b'\0' * 0x38 + elem)
    alloc('anim', struct.pack('<iII', 0, addr['elems'], 0))
    angle = rng.randrange(6283) / 1000
    s = int(math.floor(math.sin(angle) * 65536 + 0.5))
    c = int(math.floor(math.cos(angle) * 65536 + 0.5))
    m = [c, 0, s, 0, -0x10000, 0, -s, 0, c]
    sprite = 0x1234 if rng.random() < 0.75 else 0
    field_38 = rng.randrange(1000) / 1000 if rng.random() < 0.67 else 0.0
    person = struct.pack('<IIII3fii ii I II ff 3f i I I 9i 6I',
                         0, 0, 1, 0, 1.0, 1.0, 1.0, 0, 0, 0, 0, sprite, 0, rng.randrange(8), field_38,
                         rng.randrange(2000) / 1000, 0.0, angle, 0.0, 1, addr['faces'], 0, *m, 0, 0, 0, 0, 0, 0)
    alloc('person', person)
    mem[:] = out
    return addr['person'], addr['anim'], 'nv %d nt %d smooth %d sprite %d light %.3f angle %.3f' % (
        nv, nt, ns, sprite != 0, field_38, angle)


def describe(v):
    x, y, d, u, vv, sh, _ = struct.unpack('<iiIffii', v)
    return 'x %08x y %08x d %08x u %.4f v %.4f sh %08x' % (x & 0xffffffff, y & 0xffffffff, d, u, vv, sh & 0xffffffff)


def main():
    orig_exe, port_exe, rva_json = sys.argv[1:4]
    trials = int(sys.argv[4]) if len(sys.argv) > 4 else 300
    seed = int(sys.argv[5]) if len(sys.argv) > 5 else 1
    rva = json.load(open(rva_json))
    orig = Machine(orig_exe, {'fill0': 0x486590, 'fill1': 0x486c70, 'fill2': 0x4877b0, 'fill3': 0x487d40,
                              'colour': 0x4864e0, 'texture': 0x4886e0, 'anim': rva['ORIG_GetBlokeAnim3DFromPerson'],
                              'target': 0x485fe0})
    pb = 0x400000
    port = Machine(port_exe, {'fill0': pb + rva['FUN_00486590'], 'fill1': pb + rva['FUN_00486c70'],
                              'fill2': pb + rva['FUN_004877b0'], 'fill3': pb + rva['FUN_00487d40'],
                              'colour': pb + rva['FUN_004864e0'], 'texture': pb + rva['FUN_004886e0'],
                              'anim': pb + rva['GetBlokeAnim3DFromPerson'], 'target': pb + rva['FUN_00485fe0']})
    rng = random.Random(seed)
    bad = 0
    drawn = 0
    for t in range(trials):
        mem = bytearray(DATA_SIZE)
        person, anim, desc = build(rng, mem)
        results = []
        for mach, func in ((orig, 0x440a30), (port, pb + rva['FUN_00440a30'])):
            mach.uc.mem_write(DATA, bytes(mem))
            mach.anim = anim
            try:
                results.append(mach.call(func, person))
            except UcError as e:
                eip = mach.uc.reg_read(UC_X86_REG_EIP)
                results.append(None)
                print('trial %d: %s machine stopped: %s at %08x (%s)' % (t, 'original' if mach is orig else 'port', e,
                                                                        eip, desc))
        a, b = results
        if a is None or b is None:
            bad += 1
            continue
        drawn += len(a)
        if a == b:
            continue
        bad += 1
        if bad > 6:
            continue
        print('trial %d (%s): original drew %d triangles, port %d' % (t, desc, len(a), len(b)))
        for i, (ca, cb) in enumerate(zip(a, b)):
            if ca != cb:
                print('  first difference at call %d: original %s colour %d texture %d | port %s colour %d texture %d'
                      % (i, ca[0], ca[1], ca[2], cb[0], cb[1], cb[2]))
                for k in range(3):
                    print('    v%d original %s\n       port     %s' % (k, describe(ca[3][k]), describe(cb[3][k])))
                break
    print('%d of %d trials differ (%d triangles drawn by the original)' % (bad, trials, drawn))


if __name__ == '__main__':
    main()
