#!/usr/bin/env python3
"""Compare every initialized pointer table in src/legoland/ with the original exe.

For each annotated array whose initializer names functions or globals, check each entry against the dword
at the same place in the original, out to the table's extent there (the gap to the next annotated global):
an entry that points elsewhere, or a slot the port doesn't have while the original holds a code pointer
(the objective event table had 68 of 70 handlers), is reported. Exits 1 if anything differs.

Usage: python3 tools/audit_tables.py
"""
import re
import struct
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
SRC = ROOT / "src/legoland"
EXE = ROOT / "external/legoland.exe"


class Exe:
    def __init__(self, path):
        d = self.d = path.read_bytes()
        pe = struct.unpack_from("<I", d, 0x3c)[0]
        nsec = struct.unpack_from("<H", d, pe + 6)[0]
        optsz = struct.unpack_from("<H", d, pe + 20)[0]
        base = struct.unpack_from("<I", d, pe + 24 + 28)[0]
        self.secs, self.text = [], (0, 0)
        o = pe + 24 + optsz
        for _ in range(nsec):
            name = d[o:o + 8].rstrip(b"\0")
            vsz, va, rsz, rptr = struct.unpack_from("<IIII", d, o + 8)
            self.secs.append((base + va, rsz, rptr))
            if name == b".text":
                self.text = (base + va, base + va + vsz)
            o += 40

    def dword(self, v):
        for start, size, ptr in self.secs:
            if start <= v < start + size:
                return struct.unpack_from("<I", self.d, v - start + ptr)[0]
        return None


def split_top(body):
    """Split an initializer on commas that aren't inside parentheses (casts like (void (*)(int, int)))."""
    out, depth, cur = [], 0, ""
    for ch in body:
        if ch == "(":
            depth += 1
        elif ch == ")":
            depth -= 1
        if ch == "," and depth == 0:
            out.append(cur)
            cur = ""
        else:
            cur += ch
    out.append(cur)
    return [e.strip() for e in out if e.strip()]


def strip_cast(e):
    e = e.strip()
    while e.startswith("("):
        depth = 0
        for k, ch in enumerate(e):
            depth += {"(": 1, ")": -1}.get(ch, 0)
            if depth == 0:
                break
        e = e[k + 1:].strip()
    return e.lstrip("&").strip()


def main():
    exe = Exe(EXE)
    files = {p: p.read_text(encoding="latin-1") for p in SRC.glob("*.c")}
    sym = {}
    for s in files.values():
        for m in re.finditer(r"// (?:FUNCTION|GLOBAL|STUB): LEGOLAND (0x[0-9a-f]+)\n(?:\s*//[^\n]*\n)*"
                             r"[^\n(=;]*?\(?\s*\*?\s*\b(\w+)\s*[\[(=;]", s):
            sym.setdefault(m.group(2), int(m.group(1), 16))
    addrs = sorted({int(a, 16) for s in files.values() for a in re.findall(r"// GLOBAL: LEGOLAND (0x[0-9a-f]+)", s)})
    bad = checked = 0
    decl = re.compile(r"// GLOBAL: LEGOLAND (0x[0-9a-f]+)\n(?:\s*//[^\n]*\n)*[^\n]*?\(?\s*\*?\s*(\w+)\s*\[[^\]]*\]"
                      r"\s*\)?\s*(?:\([^)]*\))?\s*=\s*\{")
    for p, s in sorted(files.items()):
        for m in decl.finditer(s):
            addr, name = int(m.group(1), 16), m.group(2)
            depth, k = 1, m.end()
            while depth and k < len(s):
                depth += {"{": 1, "}": -1}.get(s[k], 0)
                k += 1
            body = re.sub(r"/\*.*?\*/|//[^\n]*", "", s[m.end():k - 1], flags=re.S)
            if "{" in body or '"' in body:
                continue  # struct entries or strings: not a flat pointer table
            entries = [strip_cast(e) for e in split_top(body)]
            if not any(re.fullmatch(r"[A-Za-z_]\w*", e) and e != "NULL" for e in entries):
                continue
            i = addrs.index(addr)
            slots = (addrs[i + 1] - addr) // 4 if i + 1 < len(addrs) else len(entries)
            issues = []
            for j in range(slots):
                orig = exe.dword(addr + 4 * j)
                if orig is None:
                    break
                if j >= len(entries):
                    if exe.text[0] <= orig < exe.text[1]:
                        issues.append(f"[{j}] missing in the port; the original has a code pointer {orig:#x}")
                    continue
                e = entries[j]
                port = 0 if e in ("NULL", "0") else sym.get(e)
                if port is None and re.fullmatch(r"0x[0-9a-fA-F]+|\d+", e):
                    port = int(e, 0)
                if port is not None and port != orig:
                    issues.append(f"[{j}] port {e} ({port:#x}), original {orig:#x}")
            checked += 1
            if "-v" in sys.argv:
                print(f"checked {name} ({p.name}): {len(entries)} entries, {slots} slots")
            if issues:
                bad += 1
                print(f"{addr:#010x} {name} ({p.name}): {len(entries)} entries in the port, {slots} slots in the original")
                for x in issues[:10]:
                    print("    " + x)
    print(f"{checked} pointer tables checked, {bad} differ from the original")
    return 1 if bad else 0


if __name__ == "__main__":
    sys.exit(main())
