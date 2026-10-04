#!/usr/bin/env python3
"""Compare the C initializers of globals the startup loader doesn't fill with the original exe's bytes.

port_data.c (tools/gen_data.py) copies the original initialized data into the writable globals it can see,
so their C initializers don't matter. Globals it skips keep whatever the decomp wrote: `static` ones, `const`
ones, and ones whose type isn't visible in a header. Gold_Slots (static) had 3 entries with the wrong values
where the original has 6. This compiles each source file (nothing runs), reads every such global's bytes
from clang's assembly output and compares them with the exe over the global's extent in the original.
Pointer slots are compared by symbol: a function or annotated global must have the same address as the
original's dword; string literals are compared as text. Exits 1 if anything differs.

Usage: python3 tools/audit_values.py        (needs clang-cl and the xwin SDK, as the clang-cl-x86 build)
"""
import re
import struct
import subprocess
import sys
import tempfile
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
from audit_globals import CC, EXE, SRC, annotated_globals  # noqa: E402


class Exe:
    def __init__(self, path):
        d = self.d = path.read_bytes()
        pe = struct.unpack_from("<I", d, 0x3c)[0]
        nsec = struct.unpack_from("<H", d, pe + 6)[0]
        optsz = struct.unpack_from("<H", d, pe + 20)[0]
        base = struct.unpack_from("<I", d, pe + 24 + 28)[0]
        self.secs = []
        o = pe + 24 + optsz
        for _ in range(nsec):
            name = d[o:o + 8].rstrip(b"\0")
            vsz, va, rsz, rptr = struct.unpack_from("<IIII", d, o + 8)
            if not name.startswith(b".text"):
                self.secs.append((base + va, rsz, rptr))
            o += 40

    def read(self, addr, n):
        for start, size, ptr in self.secs:
            if start <= addr < start + size:
                n = min(n, start + size - addr)
                return self.d[ptr + addr - start:ptr + addr - start + n]
        return None


def parse_data(asm, label):
    """Bytes (and symbolic pointer slots) of one data label in clang's assembly."""
    m = re.search(r"^_?%s:\s*(#.*)?\n" % re.escape(label), asm, re.M)
    if not m:
        return None
    out, syms = bytearray(), {}
    for line in asm[m.end():].split("\n"):
        line = line.split("#")[0].strip()
        if not line:
            continue
        if line.endswith(":") or line.startswith((".section", ".globl", ".def", ".scl", ".type", ".endef", ".comm", ".lcomm")):
            break
        mm = re.match(r"\.(byte|short|long|quad|zero|ascii|asciz|space)\s+(.*)", line)
        if not mm:
            if line.startswith(".p2align"):
                continue
            break
        kind, arg = mm.groups()
        if kind in ("zero", "space"):
            parts = [x.strip() for x in arg.split(",")]
            out += bytes([int(parts[1], 0) & 0xff if len(parts) > 1 else 0]) * int(parts[0], 0)
        elif kind in ("ascii", "asciz"):
            s = bytes(arg.strip()[1:-1], "latin-1").decode("unicode_escape").encode("latin-1")
            out += s + (b"\0" if kind == "asciz" else b"")
        else:
            size = {"byte": 1, "short": 2, "long": 4, "quad": 8}[kind]
            try:
                out += (int(arg, 0) & ((1 << (8 * size)) - 1)).to_bytes(size, "little")
            except ValueError:
                syms[len(out)] = arg.strip()
                out += b"\0" * size
    return bytes(out), syms


def main():
    exe = Exe(EXE)
    found, per_file = annotated_globals()
    addrs = sorted(found)
    port_data = (SRC.parent.parent / "port/port_data.c").read_text(encoding="latin-1")
    filled = set(re.findall(r"\(void \*\)&?(\w+), (?:sizeof|0x)", port_data))
    sym_addr = {g["name"]: a for a, g in found.items()}
    for p in SRC.glob("*.c"):
        for m in re.finditer(r"// (?:FUNCTION|STUB): LEGOLAND (0x[0-9a-f]+)\n[^\n(]*?\b(\w+)\s*\(", p.read_text(encoding="latin-1")):
            sym_addr.setdefault(m.group(2), int(m.group(1), 16))
    bad = checked = 0
    with tempfile.TemporaryDirectory() as tmp:
        for f, names in sorted(per_file.items()):
            todo = [a for a, g in found.items() if g["file"] == f and g["name"] not in filled and "=" in g["decl"]]
            if not todo:
                continue
            asm_path = Path(tmp) / f"{f}.asm"
            subprocess.run(CC + [f"/Fa{asm_path}", f"/Fo{tmp}/v.obj", str(SRC / f)], capture_output=True, text=True)
            if not asm_path.exists():
                print(f"warning: could not compile {f}", file=sys.stderr)
                continue
            asm = asm_path.read_text(encoding="latin-1")
            for a in sorted(todo):
                g = found[a]
                parsed = parse_data(asm, g["name"])
                if parsed is None:
                    continue
                data, syms = parsed
                i = addrs.index(a)
                extent = addrs[i + 1] - a if i + 1 < len(addrs) else len(data)
                orig = exe.read(a, max(len(data), extent))
                if orig is None:
                    continue
                checked += 1
                issues = []
                if len(data) < extent and orig[len(data):extent].strip(b"\0") and "-v" in sys.argv:
                    print(f"note: {g['name']}: "
                          f"port has {len(data):#x} bytes, more data follows in the original up to {extent:#x} "
                          f"(check with tools/audit_globals.py; often an unnamed string or table)")
                k = 0
                while k < min(len(data), len(orig)):
                    if k in syms:
                        target = syms[k]
                        ov = struct.unpack_from("<I", orig, k)[0]
                        want = sym_addr.get(target.lstrip("_"))
                        if want is not None and want != ov:
                            issues.append(f"+{k:#x}: pointer {target} ({want:#x}), original {ov:#x}")
                        k += 4
                        continue
                    if data[k] != orig[k]:
                        j = k
                        while j < min(len(data), len(orig)) and data[j] != orig[j] and j not in syms:
                            j += 1
                        issues.append(f"+{k:#x}: port {data[k:j][:16].hex()} original {orig[k:j][:16].hex()}")
                        k = j
                        continue
                    k += 1
                if issues:
                    bad += 1
                    print(f"{a:#010x} {g['name']} ({f}): {g['decl'][:90]}")
                    for x in issues[:6]:
                        print("    " + x)
    print(f"{checked} initialized globals not filled by port_data.c checked, {bad} differ from the original")
    return 1 if bad else 0


if __name__ == "__main__":
    sys.exit(main())
