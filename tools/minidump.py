#!/usr/bin/env python3
"""Read the port's crash/freeze minidumps (run\\crash-*.dmp, freeze-*.dmp) from WSL, by global name.

The watchdog writes dumps with the data segments (MiniDumpWithDataSegs), so every global's value at the time of the
crash is in them; heap memory mostly is not. Globals are found through the .pdb of the exe that crashed (llvm-pdbutil),
and the exe's load address comes from the dump's module list.

  python3 tools/minidump.py DUMP --pdb PDB globals NAME [NAME ...] [--len N]   value of each global (hex dwords)
  python3 tools/minidump.py DUMP --pdb PDB around NAME [--span N]             the globals laid out around NAME
  python3 tools/minidump.py DUMP --pdb PDB zerorun NAME                       the run of zero bytes containing NAME
                                                                              and the globals at its ends (a wipe?)
  python3 tools/minidump.py DUMP --pdb PDB find HEXVALUE                      every place the dword occurs
  python3 tools/minidump.py DUMP read ADDRESS [--len N]                       raw memory (hex dwords)

The PDB must belong to the exe that crashed: the run folder's legoland-port.pdb (or build-clang-x86/legoland.pdb if
its legoland.exe has the same sha256 as run\\legoland-port.exe). Needs llvm-pdbutil for anything by name.
"""
import argparse
import bisect
import re
import struct
import subprocess
import sys


class Dump:
    def __init__(self, path):
        self.d = open(path, "rb").read()
        sig, _ver, nstreams, dir_rva = struct.unpack_from("<IIII", self.d, 0)
        if sig != 0x504D444D:
            raise SystemExit("%s is not a minidump" % path)
        self.ranges = []
        self.modules = []
        for i in range(nstreams):
            kind, _size, rva = struct.unpack_from("<III", self.d, dir_rva + 12 * i)
            if kind == 9:  # Memory64ListStream
                count, base = struct.unpack_from("<QQ", self.d, rva)
                off = base
                for j in range(count):
                    addr, size = struct.unpack_from("<QQ", self.d, rva + 16 + 16 * j)
                    self.ranges.append((addr, size, off))
                    off += size
            elif kind == 5:  # MemoryListStream
                count = struct.unpack_from("<I", self.d, rva)[0]
                for j in range(count):
                    addr, size, off = struct.unpack_from("<QII", self.d, rva + 4 + 16 * j)
                    self.ranges.append((addr, size, off))
            elif kind == 4:  # ModuleListStream
                count = struct.unpack_from("<I", self.d, rva)[0]
                for j in range(count):
                    m = rva + 4 + 108 * j
                    base, size = struct.unpack_from("<QI", self.d, m)
                    name_rva = struct.unpack_from("<I", self.d, m + 20)[0]
                    nlen = struct.unpack_from("<I", self.d, name_rva)[0]
                    name = self.d[name_rva + 4:name_rva + 4 + nlen].decode("utf-16-le", "replace")
                    self.modules.append((base, size, name))
        self.ranges.sort()
        self.starts = [r[0] for r in self.ranges]

    def exe_base(self):
        for base, _size, name in self.modules:
            if name.lower().endswith(".exe"):
                return base
        raise SystemExit("no .exe in the dump's module list")

    def read(self, addr, n):
        i = bisect.bisect_right(self.starts, addr) - 1
        if i >= 0:
            base, size, off = self.ranges[i]
            if addr + n <= base + size:
                return self.d[off + addr - base:off + addr - base + n]
        return None

    def u32(self, addr):
        b = self.read(addr, 4)
        return None if b is None else struct.unpack("<I", b)[0]


def pdb_globals(pdb):
    """[(rva, name, type)] for every S_GDATA32/S_LDATA32 in the pdb, sorted by rva."""
    sections = subprocess.run(["llvm-pdbutil", "dump", "-section-headers", pdb], capture_output=True, text=True).stdout
    vas = [int(v, 16) for v in re.findall(r"^\s*([0-9A-F]+) virtual address", sections, re.M)]
    text = subprocess.run(["llvm-pdbutil", "dump", "-globals", pdb], capture_output=True, text=True).stdout
    rows = []
    for name, ty, sec, off in re.findall(
            r"S_[GL]DATA32 \[size = \d+\] `([^`]+)`\s+type = 0x\w+ \(([^)]*)\), addr = (\d+):(\d+)", text):
        sec = int(sec)
        if 1 <= sec <= len(vas):
            rows.append((vas[sec - 1] + int(off), name, ty))
    rows.sort()
    return rows


def dwords(b):
    return " ".join("%08x" % x for x in struct.unpack("<%dI" % (len(b) // 4), b[:len(b) // 4 * 4]))


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("dump")
    ap.add_argument("--pdb", help="pdb of the exe that crashed")
    sub = ap.add_subparsers(dest="cmd", required=True)
    g = sub.add_parser("globals")
    g.add_argument("names", nargs="+")
    g.add_argument("--len", type=lambda s: int(s, 0), default=16)
    a = sub.add_parser("around")
    a.add_argument("name")
    a.add_argument("--span", type=lambda s: int(s, 0), default=0x80)
    z = sub.add_parser("zerorun")
    z.add_argument("name")
    f = sub.add_parser("find")
    f.add_argument("value", type=lambda s: int(s, 16))
    r = sub.add_parser("read")
    r.add_argument("address", type=lambda s: int(s, 16))
    r.add_argument("--len", type=lambda s: int(s, 0), default=64)
    args = ap.parse_args()

    dump = Dump(args.dump)
    base = dump.exe_base()
    if args.cmd == "read":
        b = dump.read(args.address, args.len)
        print(dwords(b) if b is not None else "not in the dump")
        return 0
    if args.cmd == "find":
        pat = struct.pack("<I", args.value)
        for addr, size, off in dump.ranges:
            seg = dump.d[off:off + size]
            i = seg.find(pat)
            while i != -1:
                where = addr + i
                tag = " (exe rva %08x)" % (where - base) if base <= where < base + 0x2000000 else ""
                print("%08x%s" % (where, tag))
                i = seg.find(pat, i + 4)
        return 0

    if not args.pdb:
        raise SystemExit("--pdb is needed to look up globals by name")
    rows = pdb_globals(args.pdb)
    by_name = {n: (rva, ty) for rva, n, ty in rows}
    rvas = [r[0] for r in rows]

    def owner(addr):
        i = bisect.bisect_right(rvas, addr - base) - 1
        return rows[i] if i >= 0 else None

    def lookup(name):
        if name not in by_name:
            raise SystemExit("no global %s in %s" % (name, args.pdb))
        return base + by_name[name][0], by_name[name][1]

    print("exe loaded at %08x" % base)
    if args.cmd == "globals":
        for name in args.names:
            addr, ty = lookup(name)
            b = dump.read(addr, args.len)
            print("%-32s %-24s %08x  %s" % (name, ty, addr, dwords(b) if b is not None else "not in the dump"))
    elif args.cmd == "around":
        addr, _ = lookup(args.name)
        for rva, name, ty in rows:
            if addr - args.span <= base + rva <= addr + args.span:
                b = dump.read(base + rva, 8)
                mark = "  <--" if name == args.name else ""
                print("%08x %-32s %-24s %s%s" % (base + rva, name, ty, dwords(b) if b else "-", mark))
    elif args.cmd == "zerorun":
        addr, _ = lookup(args.name)
        if dump.u32(addr) != 0:
            print("%s is not zero" % args.name)
            return 0
        lo = addr
        while dump.u32(lo - 4) == 0:
            lo -= 4
        hi = addr
        while dump.u32(hi) == 0:
            hi += 4
        print("zero bytes %08x..%08x (0x%x bytes)" % (lo, hi, hi - lo))
        for label, at in (("starts in", lo), ("ends before", hi)):
            o = owner(at)
            if o:
                print("  %s %s (%s) at %08x +0x%x" % (label, o[1], o[2], base + o[0], at - base - o[0]))
    return 0


if __name__ == "__main__":
    sys.exit(main())
