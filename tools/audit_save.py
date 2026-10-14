#!/usr/bin/env python3
"""Audit the saved games for 64 bits: every SaveGameRead/SaveGameWrite call in src/legoland.

A saved game holds the game's structs in the original's 32-bit layout; on a 64-bit build port/port_save.c
converts each struct by the type of the buffer the call passes (PORT_X86_TYPE, the types in tools/gen_x86.py).
That works when the buffer's static type is the struct being saved and the size is a whole number of them. This
parses every source for i686 and x86_64 and reports the calls where it doesn't:
  - the buffer's type has a different layout on 64 bits and no converter (add it to gen_x86.py's TYPES);
  - the size isn't a sizeof of the buffer's type (or a multiple): a literal x86 size, or part of a struct;
  - the buffer is untyped (void *, a cast to an integer and back) and the size isn't a plain byte count.
Scalars (int, short, char arrays) and structs with the same layout on both are fine as they are.

Usage: uv run --no-project --with libclang==18.1.1 python tools/audit_save.py
"""
import os
import re
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / "tools"))
import clang.cindex as ci  # noqa: E402
import port_layout as pl  # noqa: E402

SRC = ROOT / "src/legoland"
FN = ("SaveGameRead", "SaveGameWrite")


def converted_types():
    text = (ROOT / "tools/gen_x86.py").read_text()
    m = re.search(r"^TYPES = \[(.*?)\]", text, re.S | re.M)
    return set(re.findall(r'"([^"]+)"', m.group(1)))


def strip(c):
    while c.kind in (ci.CursorKind.UNEXPOSED_EXPR, ci.CursorKind.PAREN_EXPR):
        ch = list(c.get_children())
        if len(ch) != 1:
            break
        c = ch[0]
    return c


def leaves(t):
    out = []

    def rec(t, base):
        t = t.get_canonical()
        if t.kind == ci.TypeKind.RECORD:
            for f in t.get_fields():
                off = t.get_offset(f.spelling)
                if off >= 0:
                    rec(f.type, base + off // 8)
        elif t.kind == ci.TypeKind.CONSTANTARRAY:
            e = t.get_array_element_type()
            for i in range(min(t.get_array_size(), 64)):
                rec(e, base + i * e.get_size())
        else:
            out.append((base, t.get_size()))
    rec(t, 0)
    return t.get_canonical().get_size(), tuple(out)


def text(c):
    return " ".join(t.spelling for t in c.get_tokens())


def pointee(t):
    t = t.get_canonical()
    if t.kind == ci.TypeKind.POINTER:
        return t.get_pointee()
    if t.kind in (ci.TypeKind.CONSTANTARRAY, ci.TypeKind.INCOMPLETEARRAY):
        return t.get_array_element_type()
    return None


def call_args(lines, line, col):
    """the two arguments of the call written at (line, col) in the source (the macro hides them from libclang)"""
    s = "\n".join(lines[line - 1:line + 4])[col - 1:]
    i = s.find("(")
    depth = 0
    args = [""]
    for ch in s[i + 1:]:
        if ch in "([":
            depth += 1
        elif ch in ")]":
            if depth == 0:
                break
            depth -= 1
        if ch == "," and depth == 0:
            args.append("")
            continue
        args[-1] += ch
    return [" ".join(a.split()) for a in args]


def scan(path, target):
    args = [a if not a.startswith("--target=") else "--target=" + target for a in pl.ARGS]
    tu = ci.Index.create().parse(str(path), args=args + ["-I%s" % SRC])
    lines = path.read_text(errors="replace").split("\n")
    out = {}

    def walk(c):
        if c.location.file is not None and Path(c.location.file.name).resolve() != path.resolve():
            return
        if c.kind == ci.CursorKind.CALL_EXPR and c.spelling in FN + ("PortSaveRead", "PortSaveWrite"):
            a = list(c.get_arguments())
            if len(a) >= 2:
                buf = strip(a[0])
                pt = pointee(buf.type)
                lay = leaves(pt) if pt is not None and pt.get_canonical().kind != ci.TypeKind.VOID else None
                name = pt.get_canonical().spelling if pt is not None else "?"
                src = call_args(lines, c.location.line, c.location.column)
                if len(src) < 2:
                    src = [text(a[0]), text(a[1])]
                out[(c.location.line, c.location.column)] = (c.spelling.replace("PortSave", "SaveGame"), src[0],
                                                              src[1], name, lay)
        for ch in c.get_children():
            walk(ch)
    walk(tu.cursor)
    return out


def main():
    conv = converted_types()
    findings = []
    calls = 0
    for f in sorted(SRC.glob("*.c")):
        a = scan(f, "i686-pc-windows-msvc")
        b = scan(f, "x86_64-pc-windows-msvc")
        for key in sorted(a):
            calls += 1
            fn, buf, size, name, lay32 = a[key]
            lay64 = b.get(key, (None,) * 5)[4]
            where = "%s:%d %s(%s, %s)" % (f.name, key[0], fn, buf, size)
            if lay32 is None:
                if name in ("?", "void") and not re.fullmatch(r"[\w.>\- ]+", size):
                    findings.append("%s: untyped buffer" % where)
                elif name in ("?", "void"):
                    findings.append("%s: untyped buffer (check it holds no pointers)" % where)
                continue
            if lay32 == lay64:
                continue
            if name not in conv:
                findings.append("%s: %s differs on 64 bits and has no converter" % (where, name))
                continue
            if "sizeof" not in size:
                findings.append("%s: %s with a size that isn't a sizeof" % (where, name))
    for x in findings:
        print(x)
    print("%d save calls, %d finding(s)" % (calls, len(findings)))


if __name__ == "__main__":
    main()
