#!/usr/bin/env python3
"""Find globals declared smaller than the space the original exe gives them.

The decomp is checked only on each function's code, so a guessed type (`unsigned int`, `[1]`, a short table)
never fails to match. In the original every global sat at a fixed address and a table that outgrew its guess
just used its own space; in the port the compiler lays globals out in its own order, so the same writes land
on an unrelated global. This lists each annotated global whose port size (sizeof, read back from clang, nothing
runs) is smaller than its extent in the original (the gap to the next annotated global), when

  - the code uses it as a table or buffer (variable index, pointer arithmetic, memcpy/strcpy/..., a cast of
    its address), and
  - nothing in the original refers into the rest of that gap (absolute operands in the code, image-range
    dwords in the data): if something did, the gap holds another, unnamed variable.

It also lists the opposite mistake, one object of the original split into several globals:

  - overlap: a global whose port size reaches past the next annotated global (the next one is really a field
    or element of it, e.g. Footprint.next declared as its own pointer), and
  - cast: `(struct T *)&G` (or an array G) where T is bigger than G's space in the original, so the code
    reaches the following globals through G, which in the port lie elsewhere.

Findings already reviewed as harmless (alignment padding, bounds-checked tables, plain scalars) are listed in
tools/audit_globals_ok.txt and not reported again. Run after merging the decomp; exits 1 if anything is new.

Usage: python3 tools/audit_globals.py [--all]      (needs clang-cl and the xwin SDK, as the clang-cl-x86 build)
"""
import collections
import os
import re
import struct
import subprocess
import sys
import tempfile
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
SRC = ROOT / "src/legoland"
EXE = ROOT / "external/legoland.exe"
OK = ROOT / "tools/audit_globals_ok.txt"
XWIN = Path(os.environ.get("XWIN_DIR", Path.home() / "xwin"))
CC = ["clang-cl", "/nologo", "-DDIRECTINPUT_VERSION=0x0500", "-DLEGOLAND_PORT=1", f"-I{ROOT / 'port'}",
      f"-I{ROOT / 'extensions'}", "--target=i686-pc-windows-msvc", "-fms-compatibility",
      "/imsvc", str(XWIN / "crt/include"), "/imsvc", str(XWIN / "sdk/include/ucrt"),
      "/imsvc", str(XWIN / "sdk/include/um"), "/imsvc", str(XWIN / "sdk/include/shared"),
      "/DWIN32", "/D_WINDOWS", "/O1", "-w", "-Wno-error=int-conversion",
      "-Wno-error=incompatible-function-pointer-types", "-Wno-error=return-mismatch", "/c", "/FA"]


def def_name(line):
    line = re.sub(r"/\*.*?\*/|//.*$", "", line)
    head = line.split("=")[0].strip()
    m = re.search(r"\(\s*\*\s*(\w+)\s*(\[[^\]]*\])?\s*\)", head)
    if m:
        return m.group(1)
    m = re.search(r"(\w+)\s*(\[[^=]*\])?\s*;?\s*$", head)
    return m.group(1) if m else None


def annotated_globals():
    found, per_file = {}, collections.defaultdict(list)
    for p in sorted(SRC.glob("*.c")):
        lines = p.read_text(encoding="latin-1").split("\n")
        for i, l in enumerate(lines):
            m = re.match(r"\s*// GLOBAL: LEGOLAND (0x[0-9a-f]+)", l)
            if not m:
                continue
            j = i + 1
            while j < len(lines) and lines[j].strip().startswith("//"):
                j += 1
            d = lines[j].strip()
            name = def_name(d)
            a = int(m.group(1), 16)
            if name and a not in found and not d.startswith("#"):
                found[a] = dict(name=name, file=p.name, decl=d)
                per_file[p.name].append(name)
    return found, per_file


def port_sizes(per_file):
    """sizeof every global, from a helper per source file that #includes it (its statics are visible)."""
    sizes = {}
    with tempfile.TemporaryDirectory() as tmp:
        for f, names in per_file.items():
            names = sorted(set(names))
            helper = Path(tmp) / f"sz_{f}"
            helper.write_text(f'#include "{SRC / f}"\nconst unsigned int audit_sizes[] = {{\n'
                              + "".join(f"    sizeof({n}),\n" for n in names) + "};\n")
            asm = Path(tmp) / f"sz_{f}.asm"
            subprocess.run(CC + [f"/Fa{asm}", f"/Fo{tmp}/sz.obj", str(helper)], capture_output=True, text=True)
            text = asm.read_text() if asm.exists() else ""
            m = re.search(r"_audit_sizes:\s*\n((?:\s*\.long\s+\d+.*\n)+)", text)
            vals = [int(v) for v in re.findall(r"\.long\s+(\d+)", m.group(1))] if m else []
            if len(vals) != len(names):
                print(f"warning: could not size the globals of {f}", file=sys.stderr)
                continue
            sizes.update(zip(names, vals))
    return sizes


def original_references():
    """Every image address the original refers to: code operands and image-range dwords in its data."""
    d = EXE.read_bytes()
    pe = struct.unpack_from("<I", d, 0x3c)[0]
    nsec = struct.unpack_from("<H", d, pe + 6)[0]
    optsz = struct.unpack_from("<H", d, pe + 20)[0]
    base = struct.unpack_from("<I", d, pe + 24 + 28)[0]
    lo, hi = base, base + struct.unpack_from("<I", d, pe + 24 + 56)[0]
    refs = set()
    asm = subprocess.run(["llvm-objdump", "-d", "--no-show-raw-insn", str(EXE)], capture_output=True, text=True).stdout
    refs.update(v for v in (int(h, 16) for h in re.findall(r"0x([0-9a-f]{6,8})\b", asm)) if lo <= v < hi)
    o = pe + 24 + optsz
    for _ in range(nsec):
        name = d[o:o + 8].rstrip(b"\0")
        rsz, rptr = struct.unpack_from("<II", d, o + 16)
        if not name.startswith(b".text"):
            for k in range(rptr, rptr + rsz - 3, 4):
                v = struct.unpack_from("<I", d, k)[0]
                if lo <= v < hi:
                    refs.add(v)
        o += 40
    return refs


def size_types(f, types):
    """sizeof each type as seen in source file f; types that don't compile there (incomplete) are left out."""
    types, out = sorted(types), {}
    with tempfile.TemporaryDirectory() as tmp:
        for _ in range(8):
            helper = Path(tmp) / f"ty_{f}"
            helper.write_text(f'#include "{SRC / f}"\nconst unsigned int audit_tsizes[] = {{\n'
                              + "".join(f"    sizeof({t}),\n" for t in types) + "};\n")
            asm = Path(tmp) / "ty.asm"
            if asm.exists():
                asm.unlink()
            r = subprocess.run(CC + [f"/Fa{asm}", f"/Fo{tmp}/ty.obj", str(helper)], capture_output=True, text=True)
            text = asm.read_text() if asm.exists() else ""
            m = re.search(r"_audit_tsizes:\s*\n((?:\s*\.long\s+\d+.*\n)+)", text)
            vals = [int(v) for v in re.findall(r"\.long\s+(\d+)", m.group(1))] if m else []
            if len(vals) == len(types):
                out.update(zip(types, vals))
                break
            bad = {int(x) - 3 for x in re.findall(re.escape(helper.name) + r"\((\d+)", r.stdout + r.stderr)}
            if not bad:
                print(f"warning: could not size the cast types of {f}", file=sys.stderr)
                break
            types = [t for i, t in enumerate(types) if i not in bad]
    return out


CAST = re.compile(r"\(\s*((?:const\s+)?(?:struct\s+|union\s+)?\w+)\s*\*\s*\)\s*\(?\s*(&?)\s*(\w+)\s*\)?(?!\s*[\[.]|\s*->)")
SKIP_TYPES = {"void", "char", "unsigned char", "signed char", "const char", "const void", "const unsigned char"}


def split_objects(found, sizes, gaps):
    """(key, text) for globals that overlap the next one, and casts of a global to a type bigger than its space."""
    out = []
    addrs = sorted(found)
    for i, a in enumerate(addrs[:-1]):
        g, size = found[a], sizes.get(found[a]["name"])
        if size is None or size <= gaps[a]:
            continue
        inside = ", ".join(f"{found[b]['name']} (+{b - a:#x})" for b in addrs[i + 1:] if b < a + size)
        out.append((f"overlap:{a:#010x}", f"{a:#010x} {g['name']}: {size:#x} bytes in the port, the original has "
                    f"{gaps[a]:#x} and then {inside} ({g['file']})"))
    by_name = {g["name"]: a for a, g in found.items()}
    arrays = {g["name"] for g in found.values() if re.search(r"\w\s*\[", g["decl"].split("=")[0])}
    for p in sorted(SRC.glob("*.c")):
        hits = []
        for ln, l in enumerate(p.read_text(encoding="latin-1").split("\n"), 1):
            for m in CAST.finditer(l):
                t, amp, name = m.groups()
                if name in by_name and t not in SKIP_TYPES and (amp or name in arrays):
                    hits.append((t, name, ln))
        if not hits:
            continue
        tsz = size_types(p.name, {h[0] for h in hits})
        seen = set()
        for t, name, ln in hits:
            a = by_name[name]
            if a not in gaps or tsz.get(t, 0) <= gaps[a] or (t, name) in seen:
                continue
            seen.add((t, name))
            out.append((f"cast:{name}:{t.replace(' ', '_')}", f"{p.name}:{ln} ({t} *)&{name}: {tsz[t]:#x} bytes, "
                        f"but {name} has {gaps[a]:#x} in the original before {found[a + gaps[a]]['name']}"))
    return out


def usage(name, code):
    n = re.escape(name)
    pats = {
        "index": r"\b%s\s*\[\s*[^\]0-9\s][^\]]*\]" % n,
        "&+": r"&\s*%s\s*(\)\s*)?\+" % n,
        "cast&": r"\(\s*[\w\s]+\*+\s*\)\s*&\s*%s\b" % n,
        "+": r"\b%s\s*\+\s*[\w(]" % n,
        "memfn": r"\b(memcpy|memset|memmove|strcpy|strncpy|sprintf|strcat|fread|ReadFile|RES_ReadFile|SaveGameRead)\s*\([^;]*\b%s\b" % n,
    }
    return [k for k, p in pats.items() if re.search(p, code)]


def main():
    show_all = "--all" in sys.argv
    found, per_file = annotated_globals()
    sizes = port_sizes(per_file)
    refs = original_references()
    code = "\n".join(p.read_text(encoding="latin-1") for p in SRC.glob("*.c") if p.name != "globals.c")
    reviewed = set()
    if OK.exists():
        reviewed = {l.split()[0] for l in OK.read_text().splitlines() if l.strip() and not l.startswith("#")}
    addrs = sorted(found)
    new = 0
    print(f"{len(sizes)} of {len(found)} annotated globals sized")
    for i, a in enumerate(addrs[:-1]):
        g = found[a]
        size, gap = sizes.get(g["name"]), addrs[i + 1] - a
        if size is None or size >= gap:
            continue
        use = usage(g["name"], code)
        if not use or any(a + size <= r < a + gap for r in refs):
            continue
        key = f"{a:#010x}"
        if key in reviewed and not show_all:
            continue
        new += key not in reviewed
        flag = "" if key in reviewed else "NEW "
        print(f"{flag}{key} {g['name']}: {size:#x} bytes in the port, {gap:#x} in the original "
              f"({','.join(use)}; {g['file']}): {g['decl'][:100]}")
    gaps = {a: addrs[i + 1] - a for i, a in enumerate(addrs[:-1])}
    for key, text in split_objects(found, sizes, gaps):
        if key in reviewed and not show_all:
            continue
        new += key not in reviewed
        print(f"{'' if key in reviewed else 'NEW '}{key.split(':')[0]} {text}")
    print(f"{new} new finding(s)" + ("" if new else " - all known findings are reviewed in " + OK.name))
    return 1 if new else 0


if __name__ == "__main__":
    sys.exit(main())
