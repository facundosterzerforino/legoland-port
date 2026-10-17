"""Compares the 16-bit animation drawer DrawLLSSpriteMousePick (0x466770, with the hand-written blitters it calls) in
the original legoland.exe and in the port's x86 build, both run in a CPU emulator (Unicorn): nothing runs natively.
The sprites are the game's real ones (the park's border scenery, from the .res volumes); each is drawn into a
software screen the way the map draws it (whole sprite at 0,0, as FUN_00488c80 does) and with random clips and
positions, with the mouse away from and over the sprite. The two screens must come out the same.

  python llsemu.py <installed folder> <original exe> <port exe> <port pdb> [random trials] [seed]

(2026-10-17: the 78 border scenery sprites and 40 random clips, 196 cases: 0 differ.)
"""
import os
import random
import re
import struct
import subprocess
import sys

import numpy

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from drawemu import load_pe  # noqa: E402
from resvol import Volumes  # noqa: E402
from unicorn import Uc, UC_ARCH_X86, UC_MODE_32, UcError  # noqa: E402
from unicorn.x86_const import UC_X86_REG_EIP, UC_X86_REG_ESP, UC_X86_REG_CR0, UC_X86_REG_CR4  # noqa: E402

STACK = 0x00100000
STACK_SIZE = 0x00100000
SENTINEL = 0x30000000
DATA = 0x40000000
SPRITE = DATA  # the LLS file as loaded (gfx.c)
ARGS = DATA + 0x00100000  # clip RECT, pos
SCREEN = DATA + 0x00200000
SCREEN_W = 1280
SCREEN_H = 960
PITCH = SCREEN_W * 2
DATA_SIZE = 0x00200000 + PITCH * SCREEN_H + 0x10000
BACKGROUND = 0xa5a5  # what the screen holds before the draw (the map's scratch is cleared to the transparent colour)

ORIGINAL = {'DrawLLSSpriteMousePick': 0x466770, 'CurrentSurfaceDesc': 0x0066809c, 'OverrideFrame': 0x004b9ca8,
            'MousePos': 0x00813a44, 'DAT_007feb14': 0x007feb14}


def port_symbols(pdb, exe, names):
    pub = subprocess.run(['llvm-pdbutil', 'dump', '--publics', pdb], capture_output=True, text=True).stdout
    hdr = subprocess.run(['llvm-readobj', '--sections', exe], capture_output=True, text=True).stdout
    secva = [int(m, 16) for m in re.findall(r'VirtualAddress: (0x[0-9A-Fa-f]+)', hdr)]
    out = {}
    for n in names:
        m = re.search(r'`_' + n + r'`\s*\n\s*flags = [a-z]+, addr = (\d+):(\d+)', pub)
        out[n] = 0x400000 + secva[int(m.group(1)) - 1] + int(m.group(2))
    return out


class Machine:
    def __init__(self, exe, sym):
        self.uc = Uc(UC_ARCH_X86, UC_MODE_32)
        load_pe(self.uc, exe)
        self.uc.mem_map(STACK, STACK_SIZE)
        self.uc.mem_map(DATA, (DATA_SIZE + 0xfff) & ~0xfff)
        self.uc.mem_map(SENTINEL, 0x1000)
        self.uc.reg_write(UC_X86_REG_CR0, 0x00000033)
        self.uc.reg_write(UC_X86_REG_CR4, 0x00000600)
        self.sym = sym

    def w32(self, a, v):
        self.uc.mem_write(a, struct.pack('<I', v & 0xffffffff))

    def draw(self, lls, clip, pos, mouse, override=-1):
        uc = self.uc
        uc.mem_write(SPRITE, lls)
        uc.mem_write(SCREEN, struct.pack('<H', BACKGROUND) * (SCREEN_W * SCREEN_H))
        uc.mem_write(ARGS, struct.pack('<4i2i', *clip, *pos))
        desc = self.sym['CurrentSurfaceDesc']
        self.w32(desc + 0x08, SCREEN_H)  # dwHeight
        self.w32(desc + 0x0c, SCREEN_W)  # dwWidth
        self.w32(desc + 0x10, PITCH)  # lPitch
        self.w32(desc + 0x24, SCREEN)  # lpSurface
        self.w32(self.sym['OverrideFrame'], override)
        uc.mem_write(self.sym['MousePos'], struct.pack('<2i', *mouse))
        self.w32(self.sym['DAT_007feb14'], 0)
        esp = STACK + STACK_SIZE - 0x100
        for v in (ARGS + 16, ARGS, SPRITE, SENTINEL):  # cdecl (lls, clip, pos), then the return address
            esp -= 4
            self.w32(esp, v)
        uc.reg_write(UC_X86_REG_ESP, esp)
        uc.emu_start(self.sym['DrawLLSSpriteMousePick'], SENTINEL, count=200_000_000)
        return bytes(uc.mem_read(SCREEN, PITCH * SCREEN_H)), struct.unpack('<I', uc.mem_read(self.sym['DAT_007feb14'], 4))[0]


def sprites(installed):
    v = Volumes(installed)
    out = []
    for name in sorted(v.files):
        if not name.endswith('.ilf') or 'terrain' not in name:
            continue
        path, base, size = v.files[name]
        with open(path, 'rb') as f:
            f.seek(base)
            text = f.read(size).decode('latin-1')
        for m in re.findall(r'[\w .\-]+\.lls', text, re.I):
            m = m.strip().lower()
            hit = [n for n in v.files if n.endswith('\\' + m)]
            if not hit:
                continue
            p, b, s = v.files[hit[0]]
            with open(p, 'rb') as f:
                f.seek(b)
                d = bytearray(f.read(s))
            if struct.unpack_from('<I', d, 0xc)[0] == 8:
                continue  # indexed-colour: another drawer (FUN_00464480)
            struct.pack_into('<hH', d, 0, 0, 4)  # as gfx.c loads it: frame 0, delay 4, loop delay 4
            struct.pack_into('<h', d, 0x12, 4)
            out.append((hit[0], bytes(d)))
    seen = set()
    unique = []
    for n, d in out:
        if n not in seen:
            seen.add(n)
            unique.append((n, d))
    return unique


def screen(data):
    return numpy.frombuffer(data, dtype='<u2').reshape(SCREEN_H, SCREEN_W)


def compare(a, b, w, h, pos):
    """(differing pixels inside the sprite's rectangle, outside it)"""
    diff = screen(a) != screen(b)
    inside = int(diff[pos[1]:pos[1] + h, pos[0]:pos[0] + w].sum())
    return inside, int(diff.sum()) - inside


def main():
    installed, orig_exe, port_exe, port_pdb = sys.argv[1:5]
    trials = int(sys.argv[5]) if len(sys.argv) > 5 else 40
    seed = int(sys.argv[6]) if len(sys.argv) > 6 else 1
    port = Machine(port_exe, port_symbols(port_pdb, port_exe, list(ORIGINAL)))
    orig = Machine(orig_exe, ORIGINAL)
    rng = random.Random(seed)
    cases = []
    for name, lls in sprites(installed):
        w, h = struct.unpack_from('<ii', lls, 4)
        cases.append((name, lls, (0, 0, w, h), (0, 0), (5000, 5000), 'map, mouse away'))
        cases.append((name, lls, (0, 0, w, h), (0, 0), (w // 2, h // 2), 'map, mouse over'))
    allspr = [(n, d) for n, d, *_ in cases[::2]]
    for t in range(trials):
        name, lls = rng.choice(allspr)
        w, h = struct.unpack_from('<ii', lls, 4)
        left, top = rng.randrange(w), rng.randrange(h)
        right, bottom = rng.randint(left + 1, w), rng.randint(top + 1, h)
        pos = (rng.randrange(SCREEN_W - w), rng.randrange(SCREEN_H - h))
        mouse = (pos[0] + rng.randrange(w), pos[1] + rng.randrange(h)) if rng.random() < 0.5 else (5000, 5000)
        cases.append((name, lls, (left, top, right, bottom), pos, mouse, 'random clip'))
    bad = 0
    for name, lls, clip, pos, mouse, kind in cases:
        w, h = struct.unpack_from('<ii', lls, 4)
        res = []
        for mach in (orig, port):
            try:
                res.append(mach.draw(lls, clip, pos, mouse))
            except UcError as e:
                res.append(None)
                print('%s (%s, clip %s pos %s): %s stopped: %s at %08x' % (name, kind, clip, pos,
                      'original' if mach is orig else 'port', e, mach.uc.reg_read(UC_X86_REG_EIP)))
        if None in res:
            bad += 1
            continue
        (sa, ha), (sb, hb) = res
        drawn = int((screen(sa) != BACKGROUND).sum())
        if sa == sb and ha == hb:
            if kind.startswith('map'):
                print('same   %-40s %-17s %4dx%-4d %6d pixels drawn' % (name, kind, w, h, drawn))
            continue
        bad += 1
        inside, outside = compare(sa, sb, w, h, pos)
        print('DIFFER %-40s %-17s clip %s pos %s: %d pixels differ in the sprite, %d outside; cursor hit %d vs %d'
              % (name, kind, clip, pos, inside, outside, ha, hb))
    print('%d of %d cases differ' % (bad, len(cases)))


if __name__ == '__main__':
    main()
