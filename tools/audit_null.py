#!/usr/bin/env python3
"""List where the decompiled code sets a pointer to NULL on purpose and then uses it.

The usual shape is a bounds check written out by the original programmers (or the compiler's inlining):

    if (x >= 0 && x < lpConfig->width && ...) elem = &GameMap[y][x]; else elem = NULL;
    elem->flags = 8;

MSVC6 kept the check and the NULL access crashed the original game. Clang treats a NULL access as impossible,
so it deletes the check and the port quietly uses GameMap[y][x] out of bounds. CMakeLists.txt builds the port
with -fno-delete-null-pointer-checks so the access faults as in the original (and the watchdog logs it). This
lists every such place, from clang's static analyzer (core.NullDereference, path-sensitive): each one is a spot
where the original crashes if it is ever reached. Most never are; one that is reached in play (a crash in the
trace at one of these lines) is a bug of the original game, to fix as an option in extensions/.

Only the analyzer's findings whose NULL comes from a literal `p = NULL;` / `p = 0;` are kept (the rest are
list walks on globals the analyzer can't know are non-empty). Reviewed findings go in tools/audit_null_ok.txt
as "file:function" lines; exits 1 if anything is new.

Usage: python3 tools/audit_null.py [--all] [file.c ...]   (needs clang-cl and the xwin SDK, as the clang-cl-x86 build)
"""
import concurrent.futures
import os
import re
import subprocess
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
import audit_globals  # noqa: E402  (shares the clang-cl command line)

ROOT = audit_globals.ROOT
SRC = audit_globals.SRC
OK = ROOT / "tools/audit_null_ok.txt"
CC = [c for c in audit_globals.CC if c not in ("/c", "/FA", "-w")]
NULL_STORE = re.compile(r"^\s*(?:\}\s*else\s*\{?\s*)?\w+(?:\.\w+|->\w+)*\s*=\s*(?:NULL|0|\(\s*[\w\s]+\*\s*\)\s*0)\s*;")
FUNC = re.compile(r"^[A-Za-z_][\w\s\*]*?\b(\w+)\s*\([^;]*\)\s*\{\s*$")


def analyze(path):
    r = subprocess.run(CC + ["--analyze", "-Xclang", "-analyzer-checker=core.NullDereference",
                             "-Xclang", "-analyzer-output=text", "-o", os.devnull, str(path)],
                       capture_output=True, text=True)
    return path, r.stdout + r.stderr


def functions(lines):
    """line number -> name of the function it is in."""
    out, cur = {}, None
    for i, l in enumerate(lines, 1):
        m = FUNC.match(l)
        if m:
            cur = m.group(1)
        out[i] = cur
    return out


def findings(path, text):
    lines = path.read_text(encoding="latin-1").split("\n")
    func = functions(lines)
    out, block = [], []
    for l in text.split("\n") + ["x: warning:"]:
        if ": warning:" in l and block:
            m = re.match(r".*\((\d+),\d+\): warning: (.*?) \[core", block[0])
            stores = [int(s) for s in re.findall(r"\((\d+),\d+\): note: Null pointer value stored to", "\n".join(block))]
            if m and any(NULL_STORE.match(lines[s - 1]) for s in stores):
                use = int(m.group(1))
                s = next(s for s in stores if NULL_STORE.match(lines[s - 1]))
                out.append((func.get(use) or "?", use, s, m.group(2), lines[s - 1].strip(), lines[use - 1].strip()))
            block = []
        if Path(path).name in l and (": warning:" in l or ": note:" in l):
            block.append(l)
    return out


def main():
    args = [a for a in sys.argv[1:] if not a.startswith("--")]
    files = [SRC / a for a in args] if args else sorted(SRC.glob("*.c"))
    reviewed = set()
    if OK.exists():
        reviewed = {l.split()[0] for l in OK.read_text().splitlines() if l.strip() and not l.startswith("#")}
    total = new = 0
    with concurrent.futures.ThreadPoolExecutor(os.cpu_count() or 4) as pool:
        for path, text in pool.map(analyze, files):
            seen = set()
            for fn, use, store, what, store_src, use_src in findings(path, text):
                key = f"{path.name}:{fn}"
                if (key, use) in seen:
                    continue
                seen.add((key, use))
                total += 1
                if key in reviewed and "--all" not in sys.argv:
                    continue
                new += key not in reviewed
                flag = "" if key in reviewed else "NEW "
                print(f"{flag}{path.name}:{use} in {fn}: {what}\n      set at {store}: {store_src}\n      used:     {use_src}")
    print(f"{total} use(s) of a pointer set to NULL on purpose; {new} new")
    return 1 if new else 0


if __name__ == "__main__":
    sys.exit(main())
