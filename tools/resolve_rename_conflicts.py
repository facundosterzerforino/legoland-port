#!/usr/bin/env python3
# resolve_rename_conflicts.py RENAMES.json [--apply-only]
# --apply-only: apply the rename map to src/legoland and port/ (before merging).
# Otherwise: resolve merge conflicts (diff3 style) where the port's side only applied the rename map to the
# base: then the decomp's side ("theirs") is the base with renames plus more (field names), so take it.
# Hunks where the port made a change of its own are left as conflicts and listed.
import json
import re
import subprocess
from pathlib import Path

import sys
W = Path(".").resolve()
ren = json.load(open(sys.argv[1]))
pat = re.compile(r"\b(" + "|".join(sorted(map(re.escape, ren), key=len, reverse=True)) + r")\b")


def renamed(s):
    return pat.sub(lambda m: ren[m.group(1)], s)


if "--apply-only" in sys.argv:
    n = 0
    for p in list((W / "src/legoland").glob("*.[ch]")) + [q for q in (W / "port").glob("*.[ch]") if q.name != "port_data.c"]:
        t = p.read_text(encoding="latin-1")
        k = len(pat.findall(t))
        if k:
            p.write_text(renamed(t), encoding="latin-1")
            n += k
    print("replaced", n, "occurrences")
    sys.exit(0)
files = subprocess.run(["git", "-C", str(W), "diff", "--name-only", "--diff-filter=U"], capture_output=True, text=True).stdout.split()
auto = manual = 0
left = []
for f in files:
    p = W / f
    lines = p.read_text(encoding="latin-1").split("\n")
    out = []
    i = 0
    while i < len(lines):
        if not lines[i].startswith("<<<<<<< "):
            out.append(lines[i])
            i += 1
            continue
        j = i + 1
        ours, base, theirs = [], [], []
        cur = ours
        while not lines[j].startswith(">>>>>>> "):
            if lines[j].startswith("||||||| "):
                cur = base
            elif lines[j] == "=======":
                cur = theirs
            else:
                cur.append(lines[j])
            j += 1
        if renamed("\n".join(base)) == "\n".join(ours):
            out.extend(theirs)
            auto += 1
        elif "\n".join(ours) == "\n".join(theirs):
            out.extend(ours)
            auto += 1
        else:
            out.extend(lines[i:j + 1])
            manual += 1
            left.append(f)
        i = j + 1
    p.write_text("\n".join(out), encoding="latin-1")
print("resolved automatically:", auto, "left for review:", manual)
for f in sorted(set(left)):
    print("  ", f, left.count(f))
