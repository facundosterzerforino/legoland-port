#!/usr/bin/env python3
"""Audit: a local scalar whose address is passed where more than one element is written.

MSVC6 placed locals next to each other, so decompiled code like

    int x, y;
    GetPoint(&x);   /* writes p[0] and p[1] */
    use(y);

matched the original: GetPoint's p[1] landed in y. A modern compiler doesn't keep the two adjacent, so y is
garbage (the earth slide's visitors walked to a garbage y because of this). The same happens with
(&param_4)[1] on parameters.

Lists every call that passes &scalar_local to a parameter the callee writes past element 0 (p[1] = ...), and
every (struct T *)&scalar_local. Reviewed harmless cases go in tools/audit_locals_ok.txt as
"file.c:Caller->Callee". Exits 1 when there is a new one.

Usage: python3 tools/audit_locals.py
"""
import re
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
SRC = ROOT / "src/legoland"
OK = ROOT / "tools/audit_locals_ok.txt"
FUNC = re.compile(r"^(?:LEGO_EXPORT\s+)?(?:static\s+)?[\w\s\*]+?\b(\w+)\s*\(([^)]*)\)\s*\{\s*$", re.M)
SCALAR = re.compile(r"^\s+(?:unsigned\s+|signed\s+)?(?:int|short|char|long|float|double|unsigned)\s+(\w+)\s*;", re.M)
CALL = re.compile(r"\b(\w+)\s*\(([^;]*?)\);")


def main():
    src = {f.name: f.read_text(encoding="latin-1") for f in sorted(SRC.glob("*.c"))}
    wide, bodies = {}, {}
    for f, s in src.items():
        for m in FUNC.finditer(s):
            name, params = m.group(1), [p.strip() for p in m.group(2).split(",")]
            end = s.find("\n}\n", m.end())
            body = s[m.end():end]
            bodies[(f, name)] = body
            for i, p in enumerate(params):
                pm = re.match(r".*?\*\s*(\w+)$", p)
                if not pm:
                    continue
                pn = pm.group(1)
                if (re.search(r"\b%s\[[1-9]\]\s*(=[^=]|\+=|-=|\|=|&=)" % pn, body)
                        or re.search(r"\*\(\s*%s\s*\+\s*[1-9]\s*\)\s*=" % pn, body)
                        or re.search(r"\(\(\w+\s*\*\)\s*%s\)\[[1-9]\]\s*=" % pn, body)):
                    wide.setdefault(name, set()).add(i)
    ok = set()
    if OK.exists():
        ok = {l.split("#")[0].strip() for l in OK.read_text().splitlines() if l.split("#")[0].strip()}
    new = 0
    for (f, name), body in bodies.items():
        scalars = set(SCALAR.findall(body))
        for cm in CALL.finditer(body):
            callee = cm.group(1)
            args = [a.strip() for a in re.split(r",(?![^(]*\))", cm.group(2))]
            for i, a in enumerate(args):
                am = re.match(r"^(?:\([^)]*\)\s*)?&(\w+)$", a)
                if not am or am.group(1) not in scalars:
                    continue
                why = None
                if callee in wide and i in wide[callee]:
                    why = "%s writes past element 0 of argument %d" % (callee, i + 1)
                elif a.startswith("(struct"):
                    why = "scalar local cast to a struct pointer"
                if why:
                    key = "%s:%s->%s" % (f, name, callee)
                    tag = "known" if key in ok else "NEW"
                    new += tag == "NEW"
                    print("%s %s: %s passes %s (%s)" % (tag, key, name, a, why))
    print("%d new finding(s)" % new)
    return 1 if new else 0


if __name__ == "__main__":
    sys.exit(main())
