"""Runs LoadBmpIntoImage (0x4434d0) from the original exe and the port on a real visitor texture, both emulated, and
compares the image they produce. The file functions read the game's volumes (resvol.py).

  python bmpemu.py <original exe> <port exe> <rvas.json> <installed folder> <file in the volumes>
"""
import json
import os
import struct
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from drawemu import Machine, DATA, DATA_SIZE, STACK, STACK_SIZE, SENTINEL  # noqa: E402
from resvol import Volumes  # noqa: E402
from unicorn import UC_HOOK_CODE, UcError  # noqa: E402
from unicorn.x86_const import UC_X86_REG_EAX, UC_X86_REG_EIP, UC_X86_REG_ESP  # noqa: E402

IMAGE = DATA
NAME = DATA + 0x100
HEAP = DATA + 0x1000


class Hooks:
    def __init__(self, m, addrs, filedata):
        self.m = m
        self.data = filedata
        self.pos = 0
        self.heap = HEAP
        for name, addr in addrs.items():
            m.uc.hook_add(UC_HOOK_CODE, self.hook, name, begin=addr, end=addr)

    def hook(self, uc, address, size, name):
        esp = uc.reg_read(UC_X86_REG_ESP)
        ret = struct.unpack('<I', uc.mem_read(esp, 4))[0]
        a = struct.unpack('<4I', uc.mem_read(esp + 4, 16))
        result = 0
        if name == 'gfxname':
            result = NAME
        elif name == 'open':
            self.pos = 0
            result = 0x5000
        elif name == 'read':
            d = self.data[self.pos:self.pos + a[2]]
            self.pos += len(d)
            uc.mem_write(a[1], d)
            result = len(d)
        elif name == 'setpos':
            self.pos = a[1]
        elif name == 'getpos':
            result = self.pos
        elif name in ('malloc', 'calloc'):
            n = a[0] * (a[1] if name == 'calloc' else 1)
            result = self.heap
            self.heap = (self.heap + n + 63) & ~63
        uc.reg_write(UC_X86_REG_EAX, result)
        uc.reg_write(UC_X86_REG_ESP, esp + 4)
        uc.reg_write(UC_X86_REG_EIP, ret)


def main():
    orig_exe, port_exe, rva_json, installed, path = sys.argv[1:6]
    rva = json.load(open(rva_json))
    data = Volumes(installed).read(path)
    pb = 0x400000
    o = {'gfxname': rva['ORIG_GetGFXFName'], 'open': 0x489b60, 'read': 0x489cf0, 'close': 0x489de0,
         'setpos': 0x489d70, 'getpos': 0x489db0, 'malloc': 0x49e4ff, 'free': rva['ORIG_free']}
    p = {'gfxname': pb + rva['GetGFXFName'], 'open': pb + rva['RES_OpenFile'], 'read': pb + rva['RES_ReadFile'],
         'close': pb + rva['RES_CloseFile'], 'setpos': pb + rva['RES_SetFilePointer'],
         'getpos': pb + rva['RES_GetFilePointer'], 'malloc': pb + rva['malloc'], 'calloc': pb + rva['calloc'],
         'free': pb + rva['free']}
    results = []
    for exe, addrs, func, palette in ((orig_exe, o, 0x4434d0, 0x81c4c0), (port_exe, p, pb + rva['LoadBmpIntoImage'],
                                                                         pb + rva['DAT_0081c4c0'])):
        m = Machine(exe, {})
        Hooks(m, addrs, data)
        m.uc.mem_write(DATA, bytes(DATA_SIZE))
        esp = STACK + STACK_SIZE - 0x100
        esp -= 4
        m.uc.mem_write(esp, struct.pack('<I', IMAGE))
        esp -= 4
        m.uc.mem_write(esp, struct.pack('<I', SENTINEL))
        m.uc.reg_write(UC_X86_REG_ESP, esp)
        try:
            m.uc.emu_start(func, SENTINEL, count=50_000_000)
        except UcError as e:
            print('stopped: %s at %08x' % (e, m.uc.reg_read(UC_X86_REG_EIP)))
            return
        img = bytes(m.uc.mem_read(IMAGE, 0x20))
        w, h = struct.unpack_from('<hh', img, 8)
        datap = struct.unpack_from('<I', img, 0)[0]
        pixels = bytes(m.uc.mem_read(datap, w * h)) if datap else b''
        results.append((m.uc.reg_read(UC_X86_REG_EAX), w, h, pixels, bytes(m.uc.mem_read(palette, 0x400))))
        print('%s: returned %d, %dx%d, first row %s, last row %s' % (
            'original' if exe == orig_exe else 'port', results[-1][0], w, h, pixels[:12].hex(), pixels[-w:][:12].hex()))
    a, b = results
    print('pixels identical:', a[3] == b[3], ' palette identical:', a[4] == b[4])
    if a[3] != b[3] and a[1] == b[1] and a[2] == b[2]:
        w, h = a[1], a[2]
        flipped = all(a[3][y * w:(y + 1) * w] == b[3][(h - 1 - y) * w:(h - y) * w] for y in range(h))
        print('the port image is the original flipped upside down:', flipped)
        m = row_map(a, b)
        for y in list(range(0, 8)) + list(range(100, 110)) + list(range(248, 256)):
            print('  port row %3d = original rows %s' % (y, m[y][:6]))


def row_map(a, b):
    """For each row of image b, which rows of image a are identical (w, h, pixels from main's results)."""
    w, h = a[1], a[2]
    rows_a = {}
    for y in range(h):
        rows_a.setdefault(a[3][y * w:(y + 1) * w], []).append(y)
    return [rows_a.get(b[3][y * w:(y + 1) * w], []) for y in range(h)]


if __name__ == '__main__':
    main()

