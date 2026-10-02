#!/usr/bin/env python3
"""Old -> new names of annotated functions and globals between two decomp commits."""
# decomp_renames.py OLD NEW [OUT.json]: old->new names of annotated functions and globals between two decomp commits,
# read from the address annotations (// FUNCTION:, // GLOBAL:, // STUB:, // LIBRARY:) at each commit.
import json
import re
import subprocess
import sys

old_rev, new_rev = sys.argv[1], sys.argv[2]
REPO = "."


def git(*a):
    return subprocess.run(["git", "-C", REPO] + list(a), capture_output=True, text=True, errors="replace").stdout


def names_at(rev):
    out = {}
    files = [f for f in git("ls-tree", "-r", "--name-only", rev, "src/legoland").split() if f.endswith(".c")]
    for f in files:
        lines = git("show", "%s:%s" % (rev, f)).split("\n")
        for i, l in enumerate(lines):
            m = re.match(r"\s*// (FUNCTION|GLOBAL|STUB|LIBRARY): LEGOLAND (0x[0-9a-f]+)", l)
            if not m:
                continue
            j = i + 1
            while j < len(lines) and lines[j].strip().startswith("//"):
                j += 1
            d = lines[j]
            if m.group(1) == "GLOBAL":
                head = d.split("=")[0]
                n = re.search(r"\(\s*\*\s*(\w+)\s*\)", head) or re.search(r"(\w+)\s*(\[[^=]*\])?\s*;?\s*$", head.strip())
            else:
                n = re.search(r"(\w+)\s*\(", d)
            if n:
                out[(m.group(1) == "GLOBAL", int(m.group(2), 16))] = n.group(1)
    return out


a, b = names_at(old_rev), names_at(new_rev)
renames = {}
for k, old in a.items():
    new = b.get(k)
    if new and new != old:
        renames[old] = new
# drop chains/collisions: an old name that is itself someone's new name would be ambiguous
clash = set(renames) & set(renames.values())
for c in clash:
    print("chain (left alone):", c, "->", renames[c], file=sys.stderr)
json.dump(renames, open(sys.argv[3] if len(sys.argv) > 3 else "decomp_renames.json", "w"), indent=0, sort_keys=True)
print("renames:", len(renames), "functions:", sum(1 for (g, _), o in a.items() if not g and o in renames),
      "globals:", sum(1 for (g, _), o in a.items() if g and o in renames))
for o in list(renames)[:12]:
    print("  %s -> %s" % (o, renames[o]))
