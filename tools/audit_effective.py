#!/usr/bin/env python3
"""List reccmp "effective" matches whose differences move an unconditional jump.

reccmp reports a function as a 100% effective match when the only differences are register choice or
instruction order. Swapped if/else arms look the same to it but change behaviour: FUN_0045d560 kept the
smaller start of two rectangles instead of the larger, and placing any building ran off the end of memory.
This reads a reccmp JSON report and lists every effective match whose differing lines include a `jmp`, for a
person to compare against the original asm. Exits 1 if any are found.

Usage (in the decomp checkout, after a build):
    ./tools/verify --silent --json report.json
    python3 tools/audit_effective.py report.json
"""
import json
import sys


def main():
    rows = json.load(open(sys.argv[1]))["data"]
    found = 0
    for r in rows:
        if not r.get("effective"):
            continue
        moved = []
        for hunk in r.get("diff") or []:
            for block in hunk[1]:
                if "both" in block:
                    continue
                orig = [l[1].split("\t")[0].strip() for l in block.get("orig", [])]
                recomp = [l[1].split("\t")[0].strip() for l in block.get("recomp", [])]
                if any(x.startswith("jmp") for x in orig + recomp):
                    moved.append((orig, recomp))
        if moved:
            found += 1
            print(f"{r['address']} {r['name']}: check the branch arms against the original")
            for orig, recomp in moved[:3]:
                print("    original:", " | ".join(orig)[:140])
                print("    decomp  :", " | ".join(recomp)[:140])
    print(f"{found} effective match(es) with moved jumps")
    return 1 if found else 0


if __name__ == "__main__":
    sys.exit(main())
