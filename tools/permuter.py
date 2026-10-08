# /// script
# requires-python = ">=3.11"
# dependencies = ["reccmp>=0.1.7"]
# ///
"""Random-search permuter for near-miss functions (the idea of decomp-permuter, for MSVC6).

    uv run tools/permuter.py 0x0045ff00                 # 10 minutes on RenderCursor
    uv run tools/permuter.py 0x0045ff00 --minutes 60 --seed 3
    uv run tools/permuter.py 0x0045ff00 --score-only    # just score the current source

It compiles only the function's translation unit (a temporary copy outside the repo, same /O2 flags
as the build), cuts the function out of the .obj, and scores it against the original with a
reccmp-like instruction diff: calls and data references become symbol names (from the source
annotations, the exe's imports and the .obj's relocations), float constants become their value and
strings their text, and jumps become displacements. Each round applies 1-3 random source mutations
that keep the meaning (operand swaps, comparison flips, ++/+= forms, if/else swaps, statement and
declaration reordering, ...) to the best version so far and keeps whatever scores at least as well.

Improvements are written to permuter_out/<addr>/ (gitignored). Nothing in src/ is modified: confirm a
candidate by putting it in the source, building, and running `./tools/verify -v <addr>`. The scorer
is close to reccmp's but not identical, and some mutations (statement reordering) can change
behaviour, so read every candidate before keeping it.
"""

from __future__ import annotations

import argparse
import bisect
import difflib
import hashlib
import os
import random
import re
import struct
import subprocess
import sys
import tempfile
import time
from pathlib import Path

TOOLS = Path(__file__).resolve().parent
ROOT = TOOLS.parent
SRC = ROOT / "src" / "legoland"
CL = ROOT / "cmake" / "wrappers" / "cl"
sys.path.insert(0, str(TOOLS))

import capstone  # noqa: E402
from capstone import x86  # noqa: E402
from reccmp.formats.detect import detect_image  # noqa: E402

from progress import parse_annotations  # noqa: E402

MD = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_32)
MD.detail = True
JUMPS = {
    "jmp", "loop", "jecxz", "ja", "jae", "jb", "jbe", "je", "jg", "jge", "jl", "jle", "jne", "jno", "jnp",
    "jns", "jo", "jp", "js",
}  # fmt: skip


def bare(name: str) -> str:
    """Compare symbol names without C decoration: _f, _f@8, @f@8, __imp__f@8 -> f."""
    if name.startswith("__imp_"):
        name = name[6:]
    name = name.lstrip("_@")
    return re.sub(r"@\d+$", "", name)


# ---------------------------------------------------------------- shared listing


SENTINEL = 0x7EADBEEF


def render(raw: bytes, address: int, field: int | None, name: str | None) -> str:
    """Disassemble one instruction; if `field` is given, print the 4-byte value at that offset as `name`."""
    if field is not None:
        b = bytearray(raw)
        b[field : field + 4] = struct.pack("<I", SENTINEL)
        ins = next(MD.disasm(bytes(b), address))
        text = f"{ins.mnemonic} {ins.op_str}".strip()
        return text.replace(f"{SENTINEL:#x}", name)
    ins = next(MD.disasm(raw, address))
    return f"{ins.mnemonic} {ins.op_str}".strip()


def fp_size(ins) -> int:
    if not ins.mnemonic.startswith("f"):
        return 0
    return 8 if "qword" in ins.op_str else 4 if "dword" in ins.op_str else 0


def listing(code: bytes, names: dict, tables: dict, entry_bias: int) -> list[str]:
    """Instruction strings for `code` (disassembled at address 0).

    names:  instruction offset -> (field offset in instruction, name) for operands that are addresses,
            or (None, name) to replace the whole operand of a call/jmp
    tables: data offset inside the function -> "dword" (jump table) or "byte" (index table)
    entry_bias: subtracted from jump-table entries to make them function-relative
    """
    code_end = min(tables) if tables else len(code)
    out = []
    for ins in MD.disasm(code[:code_end], 0):
        raw = code[ins.address : ins.address + ins.size]
        if ins.address in names:
            field, name = names[ins.address]
            if field is None:
                out.append(f"{ins.mnemonic} {name}")
            else:
                out.append(render(raw, ins.address, field, name))
        elif ins.mnemonic in JUMPS or ins.mnemonic == "call":
            op = ins.operands[0] if ins.operands else None
            if op is not None and op.type == x86.X86_OP_IMM:
                out.append(f"{ins.mnemonic} {op.imm - (ins.address + ins.size):+#x}")
            else:
                out.append(render(raw, ins.address, None, None))
        else:
            out.append(render(raw, ins.address, None, None))
    while out and out[-1] in ("nop", "int3"):
        out.pop()
    starts = sorted(tables) + [len(code)]
    for k, t in enumerate(starts[:-1]):
        nxt = starts[k + 1]
        if tables[t] == "dword":
            for o in range(t, nxt - 3, 4):
                v = struct.unpack_from("<I", code, o)[0] - entry_bias
                if not (0 <= v < len(code)):
                    break
                out.append(f"case {v:#x}")
        else:
            data = code[t:nxt].rstrip(b"\x90\xcc")
            out.append("bytes " + data.hex())
    return out


# ---------------------------------------------------------------- original side


def annotations():
    funcs, globals_, strings = {}, {}, set()
    for c in sorted(SRC.glob("*.c")):
        lines = c.read_text(encoding="latin-1").split("\n")
        for i, line in enumerate(lines):
            m = re.match(r"\s*// (FUNCTION|STUB|LIBRARY|GLOBAL|STRING): LEGOLAND (0x[0-9a-f]+)", line)
            if not m:
                continue
            kind, addr = m.group(1), int(m.group(2), 16)
            nxt = lines[i + 1] if i + 1 < len(lines) else ""
            if kind == "STRING":
                strings.add(addr)
            elif kind == "GLOBAL":
                decl = nxt.split("=")[0]
                n = re.search(r"\(\s*\*\s*(\w+)", decl) or re.search(r"(\w+)\s*(\[[^\]]*\]\s*)*;?\s*$", decl.strip())
                if n:
                    globals_[addr] = n.group(1)
            else:
                n = re.search(r"([A-Za-z_]\w*)\s*\(", re.sub(r"__declspec\(\w+\)", "", nxt))
                if n:
                    funcs[addr] = n.group(1)
    return funcs, globals_, strings


def reccmp_symbols() -> dict[int, tuple[str, int, int]]:
    """Original address -> (name, entity type, size) from reccmp's database, cached until the PDB changes.
    This names everything the way reccmp does (CRT functions, floats, strings, imports)."""
    import json

    cache = ROOT / "permuter_out" / ".orig_symbols.json"
    pdb = ROOT / "build" / "legoland.pdb"
    if cache.exists() and pdb.exists() and cache.stat().st_mtime > pdb.stat().st_mtime:
        return {int(k): tuple(v) for k, v in json.loads(cache.read_text()).items()}
    print("building the original-symbol cache from reccmp (once per build)...", flush=True)
    import logging

    logging.disable(logging.CRITICAL)
    os.environ["PATH"] = str(TOOLS) + os.pathsep + os.environ.get("PATH", "")
    from reccmp.compare.core import Compare
    from reccmp.project.detect import RecCmpProject

    compare = Compare.from_target(RecCmpProject.from_directory(ROOT).get("LEGOLAND"))
    syms = {}
    for e in compare._db.get_all():
        if e.orig_addr is not None and e.name:
            syms[e.orig_addr] = (e.name, int(e.entity_type or 0), int(e.size(0) or e.any_size() or 0))
    cache.parent.mkdir(parents=True, exist_ok=True)
    cache.write_text(json.dumps({str(k): v for k, v in syms.items()}))
    return syms


FLOAT_TYPE, STRING_TYPE, IMPORT_TYPE = 6, 4, 7


class Original:
    def __init__(self):
        self.image = detect_image(str(ROOT / "external" / "legoland.exe"))
        self.syms = reccmp_symbols()
        funcs, globals_, strings = annotations()  # fallback for anything reccmp has no name for
        self.c_globals = set(globals_)  # addresses the source declares as globals
        for a, n in {**globals_, **funcs}.items():
            self.syms.setdefault(a, (n, 0, 0))
        for a in strings:
            self.syms.setdefault(a, ("", STRING_TYPE, 0))
        self.addrs = sorted(self.syms)
        self.imports = {imp.addr: imp.name for imp in getattr(self.image, "imports", []) if imp.name}
        _, self.starts = parse_annotations(SRC)
        self._cache = {}

    def read(self, addr, n):
        try:
            return bytes(self.image.read(addr, n))
        except Exception:
            return b""

    def string_at(self, addr):
        return '"' + self.read(addr, 256).split(b"\0")[0].decode("latin-1") + '"'

    def float_at(self, addr, size):
        raw = self.read(addr, size)
        return f"<f:{struct.unpack('<f' if size == 4 else '<d', raw)[0]!r}>" if len(raw) == size else "<addr>"

    def name(self, addr, fp):
        if addr in self.imports:
            return bare(self.imports[addr])
        e = self.syms.get(addr)
        if e is not None:
            name, typ, size = e
            if typ == FLOAT_TYPE and addr not in self.c_globals:
                return self.float_at(addr, fp or size or 4)
            if typ == STRING_TYPE:
                return self.string_at(addr)
            if typ == IMPORT_TYPE or "::" in name:
                return bare(name.split("::")[-1])
            return bare(name)
        if fp:
            return self.float_at(addr, fp)
        s = self.read(addr, 256).split(b"\0")[0]
        if len(s) >= 2 and all(32 <= c < 127 or c in (9, 10, 13) for c in s):
            return '"' + s.decode("latin-1") + '"'
        i = bisect.bisect_right(self.addrs, addr) - 1
        if i >= 0:
            base = self.addrs[i]
            name, typ, size = self.syms[base]
            if addr - base < max(size, 1) and typ not in (FLOAT_TYPE, STRING_TYPE):
                return f"{bare(name)}+{addr - base:#x}"
        return "<addr>"

    def function_listing(self, addr, size):
        """Listing of `size` bytes at `addr` (reccmp also reads the original with our function's size),
        never past the next annotated function."""
        nxt = self.starts[self.starts.index(addr) + 1]
        size = min(size, nxt - addr)
        if size in self._cache:
            return self._cache[size]
        end = addr + size
        code = self.read(addr, size)
        names, tables = {}, {}
        for ins in MD.disasm(code, addr):
            off = ins.address - addr
            raw = code[off : off + ins.size]
            if ins.mnemonic == "call" or ins.mnemonic in JUMPS:
                op = ins.operands[0] if ins.operands else None
                if op is not None and op.type == x86.X86_OP_IMM:
                    if ins.mnemonic == "call" or not (addr <= op.imm < nxt):
                        names[off] = (None, self.name(op.imm, 0))  # calls are always named (even recursion)
                    continue  # jumps inside the function are printed as displacements
            for op in ins.operands:
                v = None
                if op.type == x86.X86_OP_MEM and 0x400000 <= op.mem.disp < 0x900000:
                    v = op.mem.disp
                elif op.type == x86.X86_OP_IMM and 0x400000 <= op.imm < 0x900000:
                    # a plain constant can fall inside a symbol's range by chance (the reccmp false positive
                    # tools/reccmp_fixes.py handles): only an exact symbol address counts as an address
                    if op.imm in self.syms or op.imm in self.imports or (addr <= op.imm < nxt):
                        v = op.imm
                if v is None:
                    continue
                field = raw.find(struct.pack("<I", v))
                if addr <= v < end:  # data inside the function: jump/index table
                    tables[v - addr] = "dword" if ins.mnemonic == "jmp" else "byte"
                    names[off] = (field, "<table>")
                elif field >= 0:
                    names[off] = (field, self.name(v, fp_size(ins)))
        self._cache[size] = listing(code, names, tables, entry_bias=addr)
        return self._cache[size]


# ---------------------------------------------------------------- candidate side (COFF .obj)


class Obj:
    def __init__(self, data: bytes):
        self.data = data
        nsec = struct.unpack_from("<H", data, 2)[0]
        symptr, nsym = struct.unpack_from("<II", data, 8)
        optsize = struct.unpack_from("<H", data, 16)[0]
        strtab = symptr + nsym * 18
        self.sections = []
        for k in range(nsec):
            o = 20 + optsize + 40 * k
            name = data[o : o + 8].rstrip(b"\0").decode("latin-1")
            size, rawptr, relptr = struct.unpack_from("<III", data, o + 16)
            nrel = struct.unpack_from("<H", data, o + 32)[0]
            relocs = [struct.unpack_from("<IIH", data, relptr + 10 * r) for r in range(nrel)]
            self.sections.append((name, size, rawptr, relocs))
        self.symbols = {}
        k = 0
        while k < nsym:
            o = symptr + 18 * k
            if data[o : o + 4] == b"\0\0\0\0":
                off = struct.unpack_from("<I", data, o + 4)[0]
                name = data[strtab + off : data.index(b"\0", strtab + off)].decode("latin-1")
            else:
                name = data[o : o + 8].rstrip(b"\0").decode("latin-1")
            value, secnum, typ, cls, naux = struct.unpack_from("<IhHBB", data, o + 8)
            self.symbols[k] = (name, value, secnum, typ, cls)
            k += 1 + naux

    def section_bytes(self, secnum):
        _, size, rawptr, _ = self.sections[secnum - 1]
        return self.data[rawptr : rawptr + size]

    def function(self, cname):
        want = bare(cname)
        for name, value, secnum, typ, cls in self.symbols.values():
            if secnum > 0 and typ & 0x20 and bare(name) == want:
                others = sorted(v for n, v, s, t, c in self.symbols.values() if s == secnum and v > value and t & 0x20)
                size = others[0] - value if others else self.sections[secnum - 1][1] - value
                return secnum, value, size
        raise KeyError(cname)

    def data_name(self, secnum, offset, fp):
        """Name for a reference into one of this object's own data sections."""
        named = [(v, n) for n, v, s, t, c in self.symbols.values() if s == secnum and not n.startswith(".") and v <= offset]
        v, n = max(named) if named else (None, None)
        raw = self.section_bytes(secnum)
        if n is not None and n.startswith("__real@"):
            size = 4 if n.startswith("__real@4") else 8
            return f"<f:{struct.unpack('<f' if size == 4 else '<d', raw[v : v + size])[0]!r}>"
        if n is None or n.startswith(("??_C@", "$SG")):
            return '"' + raw[offset:].split(b"\0")[0].decode("latin-1") + '"'
        return bare(n) + (f"+{offset - v:#x}" if offset != v else "")

    def symbol_name(self, idx, addend, fp):
        name, value, secnum, typ, cls = self.symbols[idx]
        if secnum > 0 and (name.startswith((".", "$SG", "??_C@", "__real@"))):
            return self.data_name(secnum, value + addend, fp)
        return bare(name) + (f"+{addend:#x}" if addend else "")


def candidate_listing(obj: Obj, cname: str):
    secnum, start, size = obj.function(cname)
    section = obj.sections[secnum - 1]
    raw = obj.section_bytes(secnum)
    code = raw[start : start + size]
    insns = list(MD.disasm(code, 0))
    starts = [i.address for i in insns]
    names, tables = {}, {}
    for va, symidx, rtype in section[3]:
        if not (start <= va < start + size):
            continue
        off = va - start
        addend = struct.unpack_from("<i", raw, va)[0]
        sname, svalue, ssec, _, _ = obj.symbols[symidx]
        k = bisect.bisect_right(starts, off) - 1
        ins = insns[k] if k >= 0 and off < starts[k] + insns[k].size else None
        if ssec == secnum and rtype == 6 and start <= svalue + addend < start + size:
            # a reference to data inside this function (a table), or a table entry
            if ins is not None and ins.mnemonic in ("jmp", "mov", "movzx", "movsx", "lea", "cmp", "add") and ins.address + ins.size <= min(tables, default=size):
                tables[svalue + addend - start] = "dword" if ins.mnemonic == "jmp" else "byte"
                names[ins.address] = (off - ins.address, "<table>")
            continue
        if ins is None:
            continue
        if rtype == 0x14:  # REL32 call/jmp to a symbol
            names[ins.address] = (None, obj.symbol_name(symidx, 0, 0))
        else:
            names[ins.address] = (off - ins.address, obj.symbol_name(symidx, addend, fp_size(ins)))
    # jump-table entries are section offsets: make them function-relative
    return listing(code, names, tables, entry_bias=start), size

# ---------------------------------------------------------------- compiling


def function_region(lines, addr):
    marker = f"// FUNCTION: LEGOLAND 0x{addr:08x}"
    i = lines.index(marker)
    end = i + 1
    if not lines[end].rstrip().endswith("}"):
        while lines[end] != "}":
            end += 1
    return i + 1, end + 1  # signature line .. closing brace (inclusive), excluding the marker


def compile_tu(text: str, workdir: Path) -> bytes | None:
    src = workdir / "perm.c"
    obj = workdir / "perm.obj"
    src.write_text(text, encoding="latin-1")
    if obj.exists():
        obj.unlink()
    r = subprocess.run(
        [str(CL), f"/I{SRC}", "/nologo", "/W3", "/O2", "/Z7", "/c", f"/Fo{obj}", str(src)],
        capture_output=True,
        text=True,
        cwd=ROOT,
    )
    if r.returncode != 0 or not obj.exists():
        return None
    return obj.read_bytes()


def score(orig: list[str], cand: list[str]) -> float:
    if orig == cand:
        return 1.0
    return difflib.SequenceMatcher(None, orig, cand, autojunk=False).ratio()


# ---------------------------------------------------------------- mutations

OPERAND = r"(?:\(\w+\s*\*?\)\s*)?[A-Za-z_][\w]*(?:(?:->|\.)\w+|\[[^\[\]]+\])*|\d+[uUlL]*|0x[0-9a-fA-F]+[uUlL]*"
COMMUTATIVE = r"\+|\*|&(?!&)|\|(?!\|)|\^|==|!="
FLIP = {"<": ">", ">": "<", "<=": ">=", ">=": "<="}


def _sub_random(text, pattern, repl, rng, flags=0):
    matches = list(re.finditer(pattern, text, flags))
    if not matches:
        return None
    m = rng.choice(matches)
    new = repl(m)
    if new is None or new == m.group(0):
        return None
    return text[: m.start()] + new + text[m.end() :]


def m_swap_commutative(body, rng):
    pat = rf"(?<![\w\]\)])({OPERAND})\s*({COMMUTATIVE})\s*({OPERAND})(?![\w\[(])"
    return _sub_random(body, pat, lambda m: f"{m.group(3)} {m.group(2)} {m.group(1)}", rng)


def m_flip_compare(body, rng):
    pat = rf"(?<![\w\]\)])({OPERAND})\s*(<=|>=|<(?!<)|>(?!>))\s*({OPERAND})(?![\w\[(])"
    return _sub_random(body, pat, lambda m: f"{m.group(3)} {FLIP[m.group(2)]} {m.group(1)}", rng)


def m_incdec(body, rng):
    def repl(m):
        ind, a, b, op, var = m.group(1), m.group(2), m.group(3), m.group(4), m.group(5)
        v = a or var
        o = (op or b)[0]
        forms = [f"{v}{o}{o};", f"{o}{o}{v};", f"{v} {o}= 1;", f"{v} = {v} {o} 1;"]
        return ind + rng.choice(forms)

    pat = r"(?m)^(\s*)(?:(\w+)(\+\+|--)|(\+\+|--)(\w+));"
    return _sub_random(body, pat, repl, rng)


def m_compound(body, rng):
    def repl(m):
        ind, v, rhs_v, op, rest = m.groups()
        return f"{ind}{v} {op}= {rest};" if v == rhs_v else None

    pat = r"(?m)^(\s*)(\w+(?:->\w+|\.\w+)?) = (\w+(?:->\w+|\.\w+)?) ([-+*&|^]) ([^;]+);"
    out = _sub_random(body, pat, repl, rng)
    if out is not None:
        return out
    pat2 = r"(?m)^(\s*)(\w+(?:->\w+|\.\w+)?) ([-+*&|^])= ([^;]+);"
    return _sub_random(body, pat2, lambda m: f"{m.group(1)}{m.group(2)} = {m.group(2)} {m.group(3)} {m.group(4)};", rng)


def m_zero_test(body, rng):
    choices = [
        (r"\b(\w+(?:->\w+|\.\w+)?) != 0\b", lambda m: m.group(1)),
        (r"\b(\w+(?:->\w+|\.\w+)?) == 0\b", lambda m: f"!{m.group(1)}"),
        (r"\b(\w+(?:->\w+|\.\w+)?) != NULL\b", lambda m: m.group(1)),
        (r"\b(\w+(?:->\w+|\.\w+)?) == NULL\b", lambda m: f"!{m.group(1)}"),
        (r"\b0 != (\w+)", lambda m: f"{m.group(1)} != 0"),
    ]
    pat, repl = rng.choice(choices)
    return _sub_random(body, pat, repl, rng)


def _statements(lines):
    """Indices of simple one-line statements (not declarations, control flow or braces)."""
    out = []
    for k, l in enumerate(lines):
        s = l.strip()
        if not s.endswith(";") or s.startswith(("return", "break", "continue", "goto", "case", "default", "//")):
            continue
        if "{" in s or "}" in s or s.startswith(("if", "for", "while", "do", "switch", "else")):
            continue
        out.append(k)
    return out


def m_swap_statements(body, rng):
    lines = body.split("\n")
    st = set(_statements(lines))
    pairs = [k for k in st if k + 1 in st and (len(lines[k]) - len(lines[k].lstrip())) == (len(lines[k + 1]) - len(lines[k + 1].lstrip()))]
    pairs = [k for k in pairs if not re.match(r"\s*(?:const\s+|unsigned\s+|signed\s+|struct\s+\w+\s*\*?|int|char|short|long|float|double|void)\b[^=]*;", lines[k])]
    if not pairs:
        return None
    k = rng.choice(pairs)
    a, b = lines[k], lines[k + 1]
    # don't swap if the second reads what the first writes (simple check)
    w = re.match(r"\s*\*?(\w+)", a)
    if w and re.search(rf"\b{w.group(1)}\b", b.split("=", 1)[-1]):
        return None
    lines[k], lines[k + 1] = b, a
    return "\n".join(lines)


DECL = re.compile(r"^\s+(?:const\s+|volatile\s+|register\s+)?(?:unsigned\s+|signed\s+)?(?:struct\s+\w+|\w+)\s*\**\s*\w+(\[[^\]]*\])*\s*;\s*$")


def m_swap_decls(body, rng):
    lines = body.split("\n")
    ds = [k for k, l in enumerate(lines) if DECL.match(l)]
    pairs = [k for k in ds if k + 1 in ds]
    if not pairs:
        return None
    k = rng.choice(pairs)
    lines[k], lines[k + 1] = lines[k + 1], lines[k]
    return "\n".join(lines)


def m_register(body, rng):
    lines = body.split("\n")
    ds = [k for k, l in enumerate(lines) if DECL.match(l) and "[" not in l]
    if not ds:
        return None
    k = rng.choice(ds)
    l = lines[k]
    lines[k] = l.replace("register ", "") if "register " in l else re.sub(r"^(\s+)", r"\1register ", l, count=1)
    return "\n".join(lines)


def _block_end(lines, k):
    """Index of the line that closes the brace opened at the end of lines[k]."""
    depth = 0
    for j in range(k, len(lines)):
        depth += lines[j].count("{") - lines[j].count("}")
        if depth == 0 and j > k:
            return j
    return None


def m_swap_if_else(body, rng):
    lines = body.split("\n")
    cands = []
    for k, l in enumerate(lines):
        m = re.match(r"^(\s*)if \((.*)\) \{$", l)
        if not m:
            continue
        e = _block_end(lines, k)
        if e is None or not re.match(r"^\s*\} else \{$", lines[e]):
            continue
        e2 = _block_end(lines, e)
        if e2 is None or lines[e2].strip() != "}":
            continue
        cands.append((k, e, e2, m.group(1), m.group(2)))
    if not cands:
        return None
    k, e, e2, ind, cond = rng.choice(cands)
    then_body, else_body = lines[k + 1 : e], lines[e + 1 : e2]
    new = [f"{ind}if (!({cond})) {{"] + else_body + [f"{ind}}} else {{"] + then_body + [f"{ind}}}"]
    return "\n".join(lines[:k] + new + lines[e2 + 1 :])


def m_split_and(body, rng):
    lines = body.split("\n")
    cands = []
    for k, l in enumerate(lines):
        m = re.match(r"^(\s*)if \(([^()]*(?:\([^()]*\)[^()]*)*) && (.*)\) \{$", l)
        if not m:
            continue
        e = _block_end(lines, k)
        if e is None or lines[e].strip() != "}":
            continue
        cands.append((k, e, m.group(1), m.group(2), m.group(3)))
    if not cands:
        return None
    k, e, ind, a, b = rng.choice(cands)
    inner = ["    " + x if x else x for x in lines[k + 1 : e]]
    new = [f"{ind}if ({a}) {{", f"{ind}    if ({b}) {{"] + inner + [f"{ind}    }}", f"{ind}}}"]
    return "\n".join(lines[:k] + new + lines[e + 1 :])


def m_brace_statement(body, rng):
    lines = body.split("\n")
    st = _statements(lines)
    if not st:
        return None
    k = rng.choice(st)
    ind = lines[k][: len(lines[k]) - len(lines[k].lstrip())]
    lines[k] = f"{ind}{{ {lines[k].strip()} }}"
    return "\n".join(lines)


def _decl_insert_at(lines):
    """Index after the function's local declarations (first line inside the body that isn't one)."""
    k = 1  # lines[0] is the signature with the opening brace
    while k < len(lines) and (DECL.match(lines[k]) or not lines[k].strip()):
        k += 1
    return k


_temp_counter = [0]


def m_temp(body, rng):
    """Move a member/array access into a new int temporary on the line before its statement."""
    lines = body.split("\n")
    st = _statements(lines)
    cands = []
    for k in st:
        rhs = lines[k].split("=", 1)[1] if "=" in lines[k] else lines[k]
        for m in re.finditer(r"[A-Za-z_]\w*(?:->\w+|\.\w+|\[[^\[\]]+\])+", rhs):
            cands.append((k, m.group(0)))
    if not cands:
        return None
    k, expr = rng.choice(cands)
    _temp_counter[0] += 1
    t = f"ptmp{_temp_counter[0]}"
    ind = lines[k][: len(lines[k]) - len(lines[k].lstrip())]
    lines[k] = lines[k].replace(expr, t, 1) if "=" not in lines[k] else lines[k].split("=", 1)[0] + "=" + lines[k].split("=", 1)[1].replace(expr, t, 1)
    lines.insert(k, f"{ind}{t} = {expr};")
    lines.insert(_decl_insert_at(lines), f"    int {t};")
    return "\n".join(lines)


def m_unsigned(body, rng):
    lines = body.split("\n")
    ds = [k for k, l in enumerate(lines) if re.match(r"^\s+(?:register\s+)?(?:unsigned\s+)?(?:int|char|short|long)\s+\w+\s*;", l)]
    if not ds:
        return None
    k = rng.choice(ds)
    l = lines[k]
    lines[k] = l.replace("unsigned ", "", 1) if "unsigned " in l else re.sub(r"\b(int|char|short|long)\b", r"unsigned \1", l, count=1)
    return "\n".join(lines)


def m_for_to_while(body, rng):
    lines = body.split("\n")
    cands = []
    for k, l in enumerate(lines):
        m = re.match(r"^(\s*)for \(([^;]*); ([^;]*); ([^)]*)\) \{$", l)
        if not m:
            continue
        e = _block_end(lines, k)
        if e is None or lines[e].strip() != "}" or any("continue" in x for x in lines[k:e]):
            continue
        cands.append((k, e, m.groups()))
    if not cands:
        return None
    k, e, (ind, init, cond, inc) = rng.choice(cands)
    new = ([f"{ind}{init};"] if init.strip() else []) + [f"{ind}while ({cond or '1'}) {{"] + lines[k + 1 : e]
    new += ([f"{ind}    {inc};"] if inc.strip() else []) + [f"{ind}}}"]
    return "\n".join(lines[:k] + new + lines[e + 1 :])


def m_cast_compare(body, rng):
    pat = rf"(?<![\w\]\)])({OPERAND})(\s*(?:<=|>=|<(?!<)|>(?!>)|==|!=)\s*)({OPERAND})(?![\w\[(])"
    cast = rng.choice(["(int)", "(unsigned int)", "(unsigned)"])
    side = rng.randint(1, 2)
    return _sub_random(body, pat, lambda m: f"{cast}{m.group(1)}{m.group(2)}{m.group(3)}" if side == 1 else f"{m.group(1)}{m.group(2)}{cast}{m.group(3)}", rng)


MUTATIONS = [
    (m_swap_commutative, 5),
    (m_flip_compare, 3),
    (m_incdec, 2),
    (m_compound, 2),
    (m_zero_test, 2),
    (m_swap_statements, 4),
    (m_swap_decls, 1),
    (m_register, 2),
    (m_swap_if_else, 2),
    (m_split_and, 1),
    (m_brace_statement, 1),
    (m_temp, 3),
    (m_unsigned, 2),
    (m_for_to_while, 1),
    (m_cast_compare, 2),
]


def mutate(body, rng):
    funcs = [f for f, w in MUTATIONS for _ in range(w)]
    for _ in range(rng.randint(1, 3)):
        for _attempt in range(10):
            new = rng.choice(funcs)(body, rng)
            if new is not None:
                body = new
                break
    return body


# ---------------------------------------------------------------- main


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("addr", type=lambda s: int(s, 16))
    ap.add_argument("--minutes", type=float, default=10)
    ap.add_argument("--seed", type=int, default=None)
    ap.add_argument("--score-only", action="store_true", help="score the current source and show the diff")
    args = ap.parse_args()

    orig = Original()
    marker = f"// FUNCTION: LEGOLAND 0x{args.addr:08x}"
    src = next(c for c in SRC.glob("*.c") if marker in c.read_text(encoding="latin-1"))
    lines = src.read_text(encoding="latin-1").split("\n")
    a, b = function_region(lines, args.addr)
    cname = re.search(r"([A-Za-z_]\w*)\s*\(", re.sub(r"__declspec\(\w+\)", "", lines[a])).group(1)
    head, tail = "\n".join(lines[:a]) + "\n", "\n" + "\n".join(lines[b:])
    base_body = "\n".join(lines[a:b])

    work = Path(tempfile.mkdtemp(prefix="permuter-"))
    outdir = ROOT / "permuter_out" / f"0x{args.addr:08x}"

    def evaluate(body):
        data = compile_tu(head + body + tail, work)
        if data is None:
            return None, None, None
        try:
            cand, size = candidate_listing(Obj(data), cname)
        except KeyError:
            return None, None, None
        target = orig.function_listing(args.addr, size)
        return score(target, cand), cand, target

    best, cand, target = evaluate(base_body)
    if best is None:
        sys.exit(f"{cname}: the current source does not compile on its own")
    print(f"{cname} ({src.name}) base score {best * 100:.2f}% ({len(target)} instructions)", flush=True)
    if args.score_only:
        for line in difflib.unified_diff(target, cand, "original", "ours", lineterm="", n=1):
            print(line)
        return

    rng = random.Random(args.seed)
    best_body, current = base_body, base_body
    seen = {hashlib.sha1(base_body.encode()).hexdigest()}
    deadline = time.time() + args.minutes * 60
    tries = compiled = failed = 0
    outdir.mkdir(parents=True, exist_ok=True)
    while time.time() < deadline and best < 1.0:
        tries += 1
        start = current if rng.random() < 0.5 else best_body
        body = mutate(start, rng)
        h = hashlib.sha1(body.encode()).hexdigest()
        if h in seen:
            continue
        seen.add(h)
        s, _, _ = evaluate(body)
        compiled += 1
        if s is None:
            failed += 1
            continue
        if s > best:
            best, best_body, current = s, body, body
            path = outdir / f"{s * 100:07.3f}.c"
            path.write_text(marker + "\n" + body + "\n", encoding="latin-1")
            print(f"[{compiled}] {s * 100:.2f}%  -> {path.relative_to(ROOT)}", flush=True)
        elif s == best:
            current = body  # walk along the plateau
    print(f"done: best {best * 100:.2f}% after {compiled} compiles ({failed} did not compile, {tries} tries)")


if __name__ == "__main__":
    main()
