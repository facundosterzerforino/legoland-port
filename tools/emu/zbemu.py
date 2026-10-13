"""Compares ZBufferHelper (0x464a90: an indexed-colour animation frame decoded into the 3D people's z-buffer) in the
original exe and the port, both emulated. Random frames (random pixel bytes and code stream), random clip rectangles
and positions; the whole z-buffer plus a guard area after it must come out the same.

  python zbemu.py <original exe> <port exe> <rvas.json> [trials] [seed]
"""
import json
import os
import random
import struct
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from drawemu import Machine, DATA, DATA_SIZE, STACK, STACK_SIZE, SENTINEL  # noqa: E402
from unicorn import UcError  # noqa: E402
from unicorn.x86_const import UC_X86_REG_ESP, UC_X86_REG_EIP, UC_X86_REG_FPCW  # noqa: E402

LLS = DATA
RECT = DATA + 0x80000
POS = RECT + 0x10
ZBUF = DATA + 0x90000
ZBUF_SIZE = 0xF000 + 0x4000  # the z-buffer and a guard area


class CodeWriter:
    """Writes the 2-bit code stream the way PortIdxCode / PortIdxCount read it: 16 codes per 32-bit word, low bits
    first; a count takes 4 slots and starts a new word if fewer than 4 are left."""

    def __init__(self):
        self.words = []
        self.slot = 16  # no word yet

    def _new_word(self):
        self.words.append(0)
        self.slot = 0

    def code(self, c):
        if self.slot >= 16:
            self._new_word()
        self.words[-1] |= c << (2 * self.slot)
        self.slot += 1

    def count(self, n):
        if 16 - self.slot < 4:
            self._new_word()
        self.words[-1] |= n << (2 * self.slot)
        self.slot += 4

    def data(self):
        return struct.pack('<%dI' % len(self.words), *self.words)


def make_frame(rng, width, height):
    """A well-formed frame: per row, items whose widths add up to at most the row's width, then the row end.
    Returns (number of source bytes used, code stream bytes)."""
    w = CodeWriter()
    src = 0
    for _ in range(height):
        left = rng.choice([width, width, rng.randint(0, width)])
        while left > 0:
            kind = rng.random()
            if kind < 0.3:
                w.code(rng.choice([0, 1]))  # one source pixel
                src += 1
                left -= 1
            elif kind < 0.45:
                w.code(2)  # one transparent pixel
                left -= 1
            else:
                n = rng.randint(1, min(left, 255))
                w.code(3)
                w.count(n)
                k = rng.choice([0, 1, 2, 3])
                w.code(k)
                if k == 0:
                    src += n  # n source pixels
                elif k == 1:
                    src += 1  # one source pixel, repeated
                left -= n
        w.code(3)
        w.count(0)  # end of the row
    return src, w.data()


def call(m, func, args):
    esp = STACK + STACK_SIZE - 0x100
    for a in reversed(args):
        esp -= 4
        m.uc.mem_write(esp, struct.pack('<I', a & 0xffffffff))
    esp -= 4
    m.uc.mem_write(esp, struct.pack('<I', SENTINEL))
    m.uc.reg_write(UC_X86_REG_ESP, esp)
    m.uc.emu_start(func, SENTINEL, count=20_000_000)


def main():
    orig_exe, port_exe, rva_json = sys.argv[1:4]
    trials = int(sys.argv[4]) if len(sys.argv) > 4 else 500
    rng = random.Random(int(sys.argv[5]) if len(sys.argv) > 5 else 1)
    rva = json.load(open(rva_json))
    orig = Machine(orig_exe, {})
    port = Machine(port_exe, {})
    pb = 0x400000
    sides = ((orig, 0x464a90, 0x4b9ca8), (port, pb + rva['ZBufferHelper'], pb + rva['OverrideFrame']))
    bad = 0
    for t in range(trials):
        frames = rng.randint(1, 3)
        width = rng.randint(1, 160)
        height = rng.randint(1, 120)
        frame_now = rng.randrange(frames + 1)  # sometimes past the end (clamped)
        override = rng.choice([0xffffffff, 0xffffffff, rng.randrange(frames + 1)])
        lls = bytearray(struct.pack('<hHiiIhhI', frame_now, 0, width, height, 0, frames, 0, 0))
        for f in range(frames):
            npix, codes = make_frame(rng, width, height)
            body = struct.pack('<II', 0, npix) + bytes(rng.randrange(256) for _ in range(npix))
            if f == 0:
                body += bytes(rng.randrange(256) for _ in range(0x200))
            body += codes + bytes(64)
            body = bytearray(body)
            struct.pack_into('<I', body, 0, len(body))
            lls += body
        rl = rng.randint(0, width - 1)
        rt = rng.randint(0, height - 1)
        rect = struct.pack('<4i', rl, rt, rng.randint(rl + 1, width), rng.randint(rt + 1, height))
        pos = struct.pack('<2i', rng.randint(0, 20), rng.randint(0, 20))
        zinit = bytes(rng.randrange(256) for _ in range(64)) * (ZBUF_SIZE // 64)
        out = []
        for m, func, override_addr in sides:
            m.uc.mem_write(DATA, bytes(DATA_SIZE))
            m.uc.mem_write(LLS, bytes(lls))
            m.uc.mem_write(RECT, rect)
            m.uc.mem_write(POS, pos)
            m.uc.mem_write(ZBUF, zinit)
            m.uc.mem_write(override_addr, struct.pack('<I', override))
            try:
                call(m, func, [LLS, RECT, POS, ZBUF])
                out.append(bytes(m.uc.mem_read(ZBUF, ZBUF_SIZE)))
            except UcError as e:
                out.append('stopped: %s at %08x' % (e, m.uc.reg_read(UC_X86_REG_EIP)))
        if out[0] == out[1]:
            continue
        bad += 1
        if bad <= 6:
            desc = 'frames %d size %dx%d frame %d override %d rect %s pos %s' % (
                frames, width, height, frame_now, override if override < 0x80000000 else -1,
                struct.unpack('<4i', rect), struct.unpack('<2i', pos))
            if isinstance(out[0], str) or isinstance(out[1], str):
                print('trial %d (%s): original %s, port %s' % (t, desc, out[0] if isinstance(out[0], str) else 'ok',
                                                             out[1] if isinstance(out[1], str) else 'ok'))
                continue
            diffs = [i for i in range(0, ZBUF_SIZE, 4) if out[0][i:i + 4] != out[1][i:i + 4]]
            i = diffs[0]
            print('trial %d (%s): %d words differ, first at offset %#x (row %d, x %d): original %08x port %08x' % (
                t, desc, len(diffs), i, i // 0x200, (i % 0x200) // 4, struct.unpack_from('<I', out[0], i)[0],
                struct.unpack_from('<I', out[1], i)[0]))
    print('%d of %d trials differ' % (bad, trials))


if __name__ == '__main__':
    main()
