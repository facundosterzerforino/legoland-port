"""tools/android/exe_icon.py <legoland.exe> <out.png> [size]: the exe's window icon (resource group 0x65, its
largest image) as a PNG, scaled up to size (192) without smoothing, for the Android app's launcher icon. The
icon is the original game's, so it's made at build time from external/legoland.exe, not committed."""
import struct
import sys
import zlib


def resources(data):
    pe = struct.unpack_from('<I', data, 0x3c)[0]
    sections = struct.unpack_from('<H', data, pe + 6)[0]
    opt_size = struct.unpack_from('<H', data, pe + 0x14)[0]
    opt = pe + 0x18
    magic = struct.unpack_from('<H', data, opt)[0]
    rsrc_rva = struct.unpack_from('<I', data, opt + (0x70 if magic == 0x20b else 0x60) + 2 * 8)[0]
    for s in range(sections):
        h = opt + opt_size + s * 40
        vsize, va, rawsize, raw = struct.unpack_from('<IIII', data, h + 8)
        if va <= rsrc_rva < va + max(vsize, rawsize):
            return raw + (rsrc_rva - va), va, raw
    raise SystemExit('no resources')


def entries(data, base, at):
    named, ids = struct.unpack_from('<HH', data, at + 12)
    for i in range(named + ids):
        name, off = struct.unpack_from('<II', data, at + 16 + i * 8)
        yield name, off


def find(data, base, va, raw, rtype, rid):
    """the bytes of resource rtype/rid (any language)"""
    for name, off in entries(data, base, base):
        if name != rtype:
            continue
        for n2, off2 in entries(data, base, base + (off & 0x7fffffff)):
            if rid is not None and n2 != rid:
                continue
            for n3, off3 in entries(data, base, base + (off2 & 0x7fffffff)):
                rva, size = struct.unpack_from('<II', data, base + off3)
                return data[raw + rva - va:raw + rva - va + size]
    return None


def decode(dib):
    """a DIB icon image (header, palette, XOR bits, AND mask) as width, height, RGBA rows top-down"""
    size, w, h2, planes, bpp = struct.unpack_from('<IiiHH', dib, 0)
    h = h2 // 2
    colors = struct.unpack_from('<I', dib, 32)[0] or (1 << bpp if bpp <= 8 else 0)
    pal = [dib[size + 4 * i:size + 4 * i + 4] for i in range(colors)]
    xor = size + 4 * colors
    xstride = ((w * bpp + 31) // 32) * 4
    astride = ((w + 31) // 32) * 4
    andm = xor + xstride * h
    rows = []
    for y in range(h):
        sy = h - 1 - y
        row = bytearray()
        for x in range(w):
            p = xor + sy * xstride
            if bpp == 32:
                b, g, r, a = dib[p + 4 * x:p + 4 * x + 4]
            elif bpp == 24:
                b, g, r = dib[p + 3 * x:p + 3 * x + 3]
                a = 255
            else:
                if bpp == 8:
                    k = dib[p + x]
                elif bpp == 4:
                    k = (dib[p + x // 2] >> (4 if x % 2 == 0 else 0)) & 15
                else:
                    k = (dib[p + x // 8] >> (7 - x % 8)) & 1
                b, g, r = pal[k][0], pal[k][1], pal[k][2]
                a = 255
            if bpp != 32 and (dib[andm + sy * astride + x // 8] >> (7 - x % 8)) & 1:
                a = 0
            row += bytes((r, g, b, a))
        rows.append(bytes(row))
    return w, h, rows


def png(w, h, rows):
    raw = b''.join(b'\0' + r for r in rows)

    def chunk(t, d):
        return struct.pack('>I', len(d)) + t + d + struct.pack('>I', zlib.crc32(t + d) & 0xffffffff)
    return b'\x89PNG\r\n\x1a\n' + chunk(b'IHDR', struct.pack('>IIBBBBB', w, h, 8, 6, 0, 0, 0)) + \
        chunk(b'IDAT', zlib.compress(raw, 9)) + chunk(b'IEND', b'')


def main():
    data = open(sys.argv[1], 'rb').read()
    size = int(sys.argv[3]) if len(sys.argv) > 3 else 192
    base, va, raw = resources(data)
    group = find(data, base, va, raw, 14, 0x65)  # RT_GROUP_ICON 101
    if group is None:
        group = find(data, base, va, raw, 14, None)
    count = struct.unpack_from('<H', group, 4)[0]
    best = None
    for i in range(count):
        w, h, cc, _, planes, bpp, nbytes, iid = struct.unpack_from('<BBBBHHIH', group, 6 + i * 14)
        key = ((w or 256) * (h or 256), bpp)
        if best is None or key > best[0]:
            best = (key, iid)
    w, h, rows = decode(find(data, base, va, raw, 3, best[1]))  # RT_ICON
    out = []
    for y in range(size):
        r = rows[y * h // size]
        out.append(b''.join(r[(x * w // size) * 4:(x * w // size) * 4 + 4] for x in range(size)))
    open(sys.argv[2], 'wb').write(png(size, size, out))
    print('icon %dx%d -> %s (%dx%d)' % (w, h, sys.argv[2], size, size))


main()
