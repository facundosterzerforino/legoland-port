#!/usr/bin/env python3
"""Copy the original legoland.exe's Win32 resources into a .res file for the port's link.

The game loads its window icon (group 0x65), window cursor (0x7d) and the element-picker dialog (0x75) from its
own resources. The port builds them from the original exe at build time instead of committing them: every
resource (icons, cursors and their groups, dialog, version) is copied byte for byte into a standard .res file,
which lld-link links in.

Usage: python3 tools/extract_resources.py <legoland.exe> <out.res>
"""
import struct
import sys


def resources(exe):
    pe = struct.unpack_from("<I", exe, 0x3C)[0]
    nsec = struct.unpack_from("<H", exe, pe + 6)[0]
    optsz = struct.unpack_from("<H", exe, pe + 20)[0]
    for i in range(nsec):
        o = pe + 24 + optsz + 40 * i
        if exe[o:o + 8].rstrip(b"\0") == b".rsrc":
            va, raw = struct.unpack_from("<I", exe, o + 12)[0], struct.unpack_from("<I", exe, o + 20)[0]
            break
    else:
        return []
    out = []

    def walk(off, path):
        named, ids = struct.unpack_from("<HH", exe, raw + off + 12)
        for k in range(named + ids):
            name, ptr = struct.unpack_from("<II", exe, raw + off + 16 + 8 * k)
            if name & 0x80000000:
                raise SystemExit("named resources are not supported")
            if ptr & 0x80000000:
                walk(ptr & 0x7FFFFFFF, path + [name])
            else:
                rva, size = struct.unpack_from("<II", exe, raw + ptr)
                out.append((path[0], path[1], name, exe[raw + rva - va:raw + rva - va + size]))

    walk(0, [])
    return out


def entry(rtype, name, lang, data):
    # DataSize, HeaderSize, TYPE and NAME as ordinals, DataVersion, MemoryFlags, LanguageId, Version, Characteristics
    flags = 0x1030 if rtype in (1, 3) else 0x1010
    header = struct.pack("<IIHHHHIHHII", len(data), 32, 0xFFFF, rtype, 0xFFFF, name, 0, flags, lang, 0, 0)
    return header + data + b"\0" * (-len(data) % 4)


def main():
    exe = open(sys.argv[1], "rb").read()
    res = struct.pack("<IIHHHHIHHII", 0, 32, 0xFFFF, 0, 0xFFFF, 0, 0, 0, 0, 0, 0)
    found = resources(exe)
    for rtype, name, lang, data in found:
        res += entry(rtype, name, lang, data)
    open(sys.argv[2], "wb").write(res)
    print(f"{len(found)} resources from {sys.argv[1]} -> {sys.argv[2]}")


if __name__ == "__main__":
    main()
