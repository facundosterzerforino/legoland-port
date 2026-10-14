#!/usr/bin/env python3
"""The original (x86) layout of the port's globals, for tools/gen_data.py.

Parses the game headers as the 32-bit build sees them (libclang, i686-pc-windows-msvc, the clang-cl-x86 build's
include paths) and prints, as JSON, each requested global's type as a tree:

  {"k": "B", "o": off, "n": size}                     plain bytes (no pointers inside: same layout everywhere)
  {"k": "P", "o": off} / {"k": "Q", "o": off}         a pointer / a pointer-sized integer (UINT_PTR, ...)
  {"k": "R", "o": off, "f": [[designator, tree]...]}   a struct or union with pointers somewhere inside
  {"k": "A", "o": off, "n": count, "s": elem size, "e": tree}   an array whose elements hold pointers

Offsets are relative to the enclosing record/array element. For a union, the first member is loaded, except
that a pointer member is used where the original holds a pointer (the slot is listed in --pointers).

Usage: uv run --no-project --with libclang==18.1.1 python tools/port_layout.py names... (or --stdin)
       ... port_layout.py --types file   (the type spellings in file, one per line, e.g. "struct Ride")
"""
import json
import os
import sys
from pathlib import Path

import clang.cindex as ci

for lib in ('/usr/lib/llvm-21/lib/libclang-21.so.1', '/usr/lib/llvm-18/lib/libclang-18.so.1'):
    if os.path.exists(lib):
        ci.Config.set_library_file(lib)
        break

ROOT = Path(__file__).resolve().parents[1]
SRC = ROOT / "src/legoland"
X = Path(os.environ.get("XWIN", Path.home() / "xwin"))
B = ROOT / "build-clang-x86"
PTR_INTS = {"UINT_PTR", "INT_PTR", "LONG_PTR", "ULONG_PTR", "DWORD_PTR", "size_t", "ptrdiff_t", "intptr_t",
            "uintptr_t", "WPARAM", "LPARAM", "LRESULT", "SIZE_T", "SSIZE_T", "HANDLE_PTR"}
ARGS = ["-x", "c", "--target=i686-pc-windows-msvc", "-fms-extensions", "-fms-compatibility", "-w",
        "-DDIRECTINPUT_VERSION=0x0500", "-DLEGOLAND_PORT=1", "-D_CRT_SECURE_NO_WARNINGS", "-DWIN32", "-D_WINDOWS",
        "-I%s" % (ROOT / "port"), "-I%s" % (ROOT / "extensions"), "-I%s" % (ROOT / "miniwin/include"),
        "-I%s" % (B / "_deps/sdl3-build/include-revision"), "-I%s" % (B / "_deps/sdl3-src/include"),
        "-isystem", str(X / "crt/include"), "-isystem", str(X / "sdk/include/ucrt"),
        "-isystem", str(X / "sdk/include/um"), "-isystem", str(X / "sdk/include/shared"),
        "-include", str(ROOT / "miniwin/include/miniwin/redirect.h"), "-ferror-limit=0"]


PREFER_POINTERS = False  # set by --types


def is_ptr_int(t):
    """a typedef chain that names a pointer-sized integer"""
    while t.kind == ci.TypeKind.TYPEDEF:
        name = t.get_declaration().spelling
        if name in PTR_INTS:
            return True
        t = t.get_declaration().underlying_typedef_type
    return False


def tree(t, pointer_slots, base):
    """the layout tree of type t at absolute original offset base (for the pointer-slot test)"""
    if is_ptr_int(t):
        return {"k": "Q", "o": 0}
    c = t.get_canonical()
    k = c.kind
    if k in (ci.TypeKind.POINTER, ci.TypeKind.BLOCKPOINTER):
        return {"k": "P", "o": 0}
    if k == ci.TypeKind.CONSTANTARRAY:
        e = c.get_array_element_type()
        n = c.get_array_size()
        es = e.get_size()
        sub = tree(e, pointer_slots, base)
        if sub["k"] == "B":
            return {"k": "B", "o": 0, "n": n * es}
        # each element at its own base (pointer slots differ per element for unions)
        elems = [tree(e, pointer_slots, base + i * es) for i in range(n)]
        if all(json.dumps(x, sort_keys=True) == json.dumps(elems[0], sort_keys=True) for x in elems):
            return {"k": "A", "o": 0, "n": n, "s": es, "e": elems[0]}
        return {"k": "R", "o": 0, "f": [["[%d]" % i, dict(x, o=i * es)] for i, x in enumerate(elems)]}
    if k == ci.TypeKind.RECORD:
        decl = c.get_declaration()
        size = c.get_size()
        if size <= 0:
            return {"k": "B", "o": 0, "n": 0}
        is_union = decl.kind == ci.CursorKind.UNION_DECL
        members = []
        for f in decl.type.get_fields():
            if f.is_bitfield():
                return {"k": "B", "o": 0, "n": size}
            anon = is_anon(f)
            if is_union:
                off = 0
            elif not anon:
                off = c.get_offset(f.spelling) // 8
            else:
                off = anon_offset(c, f)
            sub = tree(f.type, pointer_slots, base + off)
            members.append(["" if anon else f.spelling, off, sub, f])
        if all(m[2]["k"] == "B" for m in members):
            return {"k": "B", "o": 0, "n": size}
        if is_union and PREFER_POINTERS:
            # --types: no original image to say what a slot holds; the member with pointers (the largest) lays the
            # union out, so the port's struct gets its pointer-sized slots (an integer member reads their low half)
            with_ptrs = [m for m in members if any(True for _ in ptr_offsets(m[2], 0))]
            if with_ptrs:
                choice = max(with_ptrs, key=lambda m: m[3].type.get_size())
                return {"k": "R", "o": 0, "f": [[desig(choice), dict(choice[2], o=0)]]}
        if is_union:
            choice = members[0]
            for m in members:
                if m[2]["k"] in ("P", "Q") and base in pointer_slots:
                    choice = m
                    break
            else:
                if choice[2]["k"] in ("P", "Q") and base not in pointer_slots:
                    # an integer view of the slot if the original holds no pointer there
                    for m in members:
                        if m[2]["k"] == "B" and m[2]["n"] >= 4:
                            choice = m
                            break
            fields = [[desig(choice), dict(choice[2], o=0)]]
            if choice[2]["k"] == "B":
                return {"k": "B", "o": 0, "n": size}
            return {"k": "R", "o": 0, "f": fields}
        fields = []
        for m in members:
            fields.append([desig(m), dict(m[2], o=m[1])])
        return {"k": "R", "o": 0, "f": fields}
    size = c.get_size()
    return {"k": "B", "o": 0, "n": max(size, 0)}


def has_pointer(t):
    return False


def is_anon(f):
    return not f.spelling or "(anonymous" in f.spelling


def first_named(t):
    """the path to the first named field of an anonymous member (its address is the member's)"""
    for g in t.get_canonical().get_declaration().type.get_fields():
        if is_anon(g):
            return first_named(g.type)
        return "." + g.spelling
    return ""


def desig_name(name):
    return "." + name if name else ""


def desig(m):
    """the designator of member m: an anonymous one has none, except that a leaf is addressed by its first field"""
    if m[0]:
        return "." + m[0]
    return "" if m[2]["k"] == "R" else first_named(m[3].type)


def anon_offset(rec, f):
    """offset of an anonymous struct/union member: the offset of its first named field"""
    for g in f.type.get_canonical().get_declaration().type.get_fields():
        if not is_anon(g):
            o = rec.get_offset(g.spelling)
            if o >= 0:
                return o // 8
        else:
            return anon_offset(rec, g)
    return 0


def ptr_offsets(t, base):
    """the original offsets of the pointer slots in tree t placed at base"""
    k = t["k"]
    if k in ("P", "Q"):
        yield base
    elif k == "R":
        for d, sub in t["f"]:
            yield from ptr_offsets(sub, base + sub["o"])
    elif k == "A":
        for i in range(t["n"]):
            yield from ptr_offsets(t["e"], base + i * t["s"])


def main():
    names = sys.stdin.read().split() if "--stdin" in sys.argv else [a for a in sys.argv[1:] if not a.startswith("--") and not os.path.exists(a)]
    slots = set()
    if "--pointers" in sys.argv:
        slots = set(int(x, 16) for x in open(sys.argv[sys.argv.index("--pointers") + 1]).read().split())
    want = {}  # name -> original address (for the pointer-slot test)
    for n in names:
        if "@" in n:
            nm, a = n.split("@")
            want[nm] = int(a, 16)
        else:
            want[n] = 0
    text = "#include <windows.h>\n" + "".join('#include "%s"\n' % h.name for h in sorted(SRC.glob("*.h")))
    types = {}  # probe variable -> type spelling (--types: lay out these types instead of globals)
    if "--types" in sys.argv:
        global PREFER_POINTERS
        PREFER_POINTERS = True
        spellings = [x.strip() for x in open(sys.argv[sys.argv.index("--types") + 1]).read().split("\n") if x.strip()]
        for i, t in enumerate(spellings):
            types["__port_x86_%d" % i] = t
            text += "extern %s __port_x86_%d;\n" % (t, i)
            want["__port_x86_%d" % i] = 0
    path = str(ROOT / "port" / "_layout_probe.c")
    tu = ci.Index.create().parse(path, args=ARGS + ["-I%s" % SRC], unsaved_files=[(path, text)])
    errs = [d for d in tu.diagnostics if d.severity >= ci.Diagnostic.Error]
    out = {"errors": [str(d) for d in errs[:20]], "globals": {}}
    for c in tu.cursor.get_children():
        if c.kind == ci.CursorKind.VAR_DECL and c.spelling in want and c.spelling not in out["globals"]:
            t = tree(c.type, slots, want[c.spelling])
            key = types.get(c.spelling, c.spelling)
            out["globals"][key] = {"size": c.type.get_size(), "t": t, "ptrs": sorted(set(ptr_offsets(t, 0)))}
    json.dump(out, sys.stdout)


if __name__ == "__main__":
    main()
