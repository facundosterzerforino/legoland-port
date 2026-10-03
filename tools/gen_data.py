#!/usr/bin/env python3
"""Generate port/port_data.c: the original program's initialized data, for the port.

The original legoland.exe starts with its .rdata and .data sections already filled in (tables, strings,
constants). Most of the port's globals are declared without those values, so this script embeds the
original bytes and writes PortLoadData(), which the port calls first thing in WinMain:

  1. Pointer slots in the embedded copy are patched to their port equivalents: a pointer into an annotated
     global becomes &global + offset, a pointer to an annotated function becomes that function, and any
     other pointer into the initialized data (string literals, unnamed tables) points into the copy itself.
  2. Every annotated, writable global in the initialized range gets its original bytes from the patched
     copy (as many as both its port size and the gap to the next annotated global allow).

The exe has no relocation table, so pointer slots are found heuristically: aligned dwords whose value lies
inside the image (0x401000..0x836000). Values that can't be resolved to a global, a function or the
initialized data are left untouched and listed in the report.

Usage: python3 tools/gen_data.py [--report]   (run from the repo root; rerun after globals change)
"""
import re
import struct
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
SRC = ROOT / "src/legoland"
OUT = ROOT / "port/port_data.c"
EXE = ROOT / "external/legoland.exe"
IMAGE_LO, IMAGE_HI = 0x401000, 0x836000

exe = EXE.read_bytes()
pe = struct.unpack_from("<I", exe, 0x3C)[0]
nsec = struct.unpack_from("<H", exe, pe + 6)[0]
optsz = struct.unpack_from("<H", exe, pe + 20)[0]
base = struct.unpack_from("<I", exe, pe + 24 + 28)[0]
sections = {}
for i in range(nsec):
    o = pe + 24 + optsz + 40 * i
    name = exe[o:o + 8].rstrip(b"\0").decode()
    vsz, va, rsz, raw = struct.unpack_from("<IIII", exe, o + 8)
    sections[name] = (base + va, vsz, rsz, raw)

# the embedded copy: .rdata and .data raw bytes, which are contiguous in memory
rdata, data = sections[".rdata"], sections[".data"]
assert rdata[0] + rdata[2] == data[0], "expected .rdata raw data to end where .data starts"
DATA_LO = rdata[0]
DATA_HI = data[0] + data[2]
image = bytearray(exe[rdata[3]:rdata[3] + rdata[2]] + exe[data[3]:data[3] + data[2]])
assert len(image) == DATA_HI - DATA_LO

# ---- annotations -----------------------------------------------------------------------------------------
NAME_RE = re.compile(r"\(\s*\*\s*(\w+)\s*\)|(\w+)\s*(?:\[|=|;|$)")


def def_name(line):
    line = re.sub(r"/\*.*?\*/|//.*$", "", line)  # a trailing comment would hide the name
    head = line.split("=")[0].strip()
    m = re.search(r"\(\s*\*\s*(\w+)\s*(\[[^\]]*\])?\s*\)", head)  # (*name)(...) and (*name[N])(...)
    if m:
        return m.group(1)
    m = re.search(r"(\w+)\s*(\[[^=]*\])?\s*;?\s*$", head)
    return m.group(1) if m else None


globals_ = {}  # addr -> dict
functions = {}  # addr -> dict
for p in sorted(SRC.glob("*.c")):
    lines = p.read_text(encoding="latin-1").split("\n")
    for i, l in enumerate(lines):
        m = re.match(r"\s*// GLOBAL: LEGOLAND (0x[0-9a-f]+)", l)
        if m:
            j = i + 1
            while j < len(lines) and lines[j].strip().startswith("//"):
                j += 1  # other annotations (// STRING: ...) between the GLOBAL line and the definition
            d = lines[j].strip()
            name = def_name(d)
            addr = int(m.group(1), 16)
            if name and addr not in globals_:
                globals_[addr] = dict(name=name, file=p.name, decl=d, static=d.startswith("static"),
                                      const=bool(re.match(r"(static\s+)?(const\s|.*\bconst\s+\w+\s*\[)", d.split("=")[0])))
        m = re.match(r"\s*// (FUNCTION|STUB|LIBRARY): LEGOLAND (0x[0-9a-f]+)", l)
        if m:
            j = i + 1
            while j < len(lines) and (lines[j].startswith("//") or not lines[j].strip()):
                j += 1
            sig = lines[j]
            m2 = re.search(r"(\w+)\s*\(", sig)
            if m2:
                functions[int(m.group(2), 16)] = dict(name=m2.group(1), file=p.name, sig=sig,
                                                      static=sig.startswith("static"), kind=m.group(1))

headers = "\n".join(h.read_text(encoding="latin-1") for h in SRC.glob("*.h"))
headers = re.sub(r"/\*.*?\*/", " ", headers, flags=re.S)  # mentions in comments don't declare anything
headers = re.sub(r"//[^\n]*", " ", headers)


def declared_in_headers(name):
    return re.search(r"\b%s\b" % re.escape(name), headers) is not None


gaddrs = sorted(globals_)


def global_containing(t):
    """The annotated global whose range [addr, next annotated addr) contains t."""
    lo, hi = 0, len(gaddrs)
    while lo < hi:
        mid = (lo + hi) // 2
        if gaddrs[mid] <= t:
            lo = mid + 1
        else:
            hi = mid
    if lo == 0:
        return None
    a = gaddrs[lo - 1]
    nxt = gaddrs[lo] if lo < len(gaddrs) else IMAGE_HI
    return a if t < nxt else None


NON_CDECL = re.compile(r"\b(__stdcall|WINAPI|CALLBACK|APIENTRY|__fastcall|__thiscall)\b")
extra_decls = {}  # name -> declaration this file adds for symbols no header declares


def gap_of(a):
    i = gaddrs.index(a)
    nxt = gaddrs[i + 1] if i + 1 < len(gaddrs) else DATA_HI
    return max(0, min(nxt, max(DATA_HI, a + 1)) - a)


def has_initializer(g):
    """True if the source already gives the global a real (non-zero) initial value."""
    if "=" not in g["decl"]:
        return False
    init = g["decl"].split("=", 1)[1].strip().rstrip(";").strip()
    return init not in ("0", "{0}", "{ 0 }", "{}", "NULL")


def is_opaque(g):
    """The port can't see this global's full type here: no header declares it (its type may be local to its
    .c file), or a header declares it as an unsized array. Such globals are only referenced by address."""
    return not declared_in_headers(g["name"]) or re.search(r"\b%s\s*\[\s*\]" % re.escape(g["name"]), headers)


def size_expr(a):
    g = globals_[a]
    return ("0x%xu" % gap_of(a)) if is_opaque(g) else "sizeof(%s)" % g["name"]


def usable_global(a):
    g = globals_[a]
    if g["static"]:
        return False
    if not declared_in_headers(g["name"]) and g["name"] not in extra_decls:
        # only its address is needed: declare it as an opaque byte array
        extra_decls[g["name"]] = "extern char %s[];" % g["name"]
    return True


def usable_function(a):
    f = functions[a]
    if f["kind"] != "FUNCTION" or f["static"]:
        return False
    if not declared_in_headers(f["name"]) and f["name"] not in extra_decls:
        sig = f["sig"].strip()
        if NON_CDECL.search(sig):
            # the calling convention changes the symbol name, so use the exact signature
            extra_decls[f["name"]] = sig.rstrip("{").rstrip() + ";"
        else:
            # cdecl: an unprototyped declaration is enough to take its address
            extra_decls[f["name"]] = "void %s();" % f["name"]
    return True


# ---- text that looks like a pointer ----------------------------------------------------------------------
# The tail of a string, e.g. "abc\0", reads as the dword 0x00636261, which lies inside the image. Such words
# must not be patched: skip words inside annotated string literals, and words that continue a run of text.
string_ranges = []
for p in SRC.glob("*.c"):
    for a in re.findall(r"// STRING: LEGOLAND (0x[0-9a-f]+)", p.read_text(encoding="latin-1")):
        a = int(a, 16)
        if DATA_LO <= a < DATA_HI:
            o = a - DATA_LO
            string_ranges.append((o, image.index(0, o) + 1))
string_ranges.sort()
string_starts = [s for s, e in string_ranges]


def printable(b):
    return 0x20 <= b < 0x7F


def looks_like_text(off):
    import bisect
    i = bisect.bisect_right(string_starts, off + 3) - 1
    if i >= 0 and off < string_ranges[i][1]:
        return "annotated string"
    w = image[off:off + 4]
    if all(printable(b) for b in w[:3]) and w[3] == 0:
        if off > 0 and printable(image[off - 1]):
            return "text run"
        # a short string starting on the word ("cos\0"): as a pointer it would land inside an uninitialized
        # global, not at its start, which real pointers into uninitialized data practically never do
        v = struct.unpack_from("<I", image, off)[0]
        ga = global_containing(v)
        if v >= DATA_HI and (ga is None or ga != v):
            return "short string"
    return None


# ---- pointer slots ---------------------------------------------------------------------------------------
fixups = []  # (slot offset in image, kind, target expr, extra)
unresolved = []
text_skipped = {}
for off in range(0, len(image), 4):
    v = struct.unpack_from("<I", image, off)[0]
    if not IMAGE_LO <= v < IMAGE_HI:
        continue
    why_text = looks_like_text(off)
    if why_text:
        text_skipped[why_text] = text_skipped.get(why_text, 0) + 1
        continue
    slot = DATA_LO + off
    ga = global_containing(v)
    if v in functions:
        if usable_function(v):
            fixups.append((off, "F", functions[v]["name"], 0))
        else:
            unresolved.append((slot, v, "function %s (not referenceable)" % functions[v]["name"]))
    elif ga is not None and usable_global(ga) and not globals_[ga]["const"]:
        fixups.append((off, "G", globals_[ga]["name"], v - ga))
    elif DATA_LO <= v < DATA_HI:
        fixups.append((off, "I", None, v - DATA_LO))
    else:
        where = "code" if v < rdata[0] else ("global %s" % globals_[ga]["name"] if ga else "uninitialized data")
        unresolved.append((slot, v, where))

# ---- globals to fill -------------------------------------------------------------------------------------
fill = []
skipped = []
for i, a in enumerate(gaddrs):
    if not DATA_LO <= a < DATA_HI:
        continue
    g = globals_[a]
    gap = (gaddrs[i + 1] if i + 1 < len(gaddrs) else DATA_HI) - a
    gap = min(gap, DATA_HI - a)
    if g["const"]:
        skipped.append((a, g["name"], "const (initialized in its source)"))
    elif g["static"]:
        skipped.append((a, g["name"], "static in %s%s" % (g["file"], "" if has_initializer(g) else ", NOT INITIALIZED")))
    elif is_opaque(g):
        usable_global(a)
        skipped.append((a, g["name"], "type not visible here; %s" % (
            "initialized in its source" if has_initializer(g) else "NOT INITIALIZED (declare it in a header)")))
    else:
        fill.append((a, g["name"], gap))

# ---- report ----------------------------------------------------------------------------------------------
kinds = {}
for f in fixups:
    kinds[f[1]] = kinds.get(f[1], 0) + 1
print("initialized data: 0x%x..0x%x (%d bytes)" % (DATA_LO, DATA_HI, len(image)))
print("globals filled: %d, skipped: %d" % (len(fill), len(skipped)))
print("pointer slots patched: %d (globals %d, functions %d, into the copy %d), left unchanged: %d"
      % (len(fixups), kinds.get("G", 0), kinds.get("F", 0), kinds.get("I", 0), len(unresolved)))
print("words skipped as text: %s" % ", ".join("%d (%s)" % (n, k) for k, n in sorted(text_skipped.items())))
if "--report" in sys.argv:
    for a, n, why in skipped:
        print("  skipped 0x%08x %-28s %s" % (a, n, why))
    for slot, v, where in unresolved:
        g = global_containing(slot)
        print("  unchanged at 0x%08x (in %s): 0x%08x -> %s" % (slot, globals_[g]["name"] if g else "unnamed data", v, where))

# ---- output ----------------------------------------------------------------------------------------------
out = []
out.append("/* GENERATED by tools/gen_data.py from external/legoland.exe -- do not edit. */\n")
out.append("/* [port] The original program's initialized data (.rdata + .data, 0x%08x..0x%08x) and the code that\n"
           "   puts it into the port's globals at startup. See tools/gen_data.py. */\n" % (DATA_LO, DATA_HI))
out.append("#include <windows.h>\n#include <stdio.h>\n#include <string.h>\n")
for h in sorted(SRC.glob("*.h")):
    out.append('#include "../src/legoland/%s"\n' % h.name)
out.append('#include "port_data.h"\n\n')
if extra_decls:
    out.append("/* Symbols no header declares (callbacks only reached through these tables, TU-level globals). */\n")
    for name in sorted(extra_decls):
        out.append(extra_decls[name] + "\n")
    out.append("\n")
out.append("#define PORT_DATA_VA 0x%08xu\n#define PORT_DATA_SIZE 0x%xu\n\n" % (DATA_LO, len(image)))
out.append("static unsigned char port_data[PORT_DATA_SIZE] = {\n")
for i in range(0, len(image), 24):
    out.append("    " + ",".join("0x%02x" % b for b in image[i:i + 24]) + ",\n")
out.append("};\n\n")
out.append("struct PortFixup {\n    unsigned int slot;   /* offset of the pointer slot in port_data */\n"
           "    unsigned char kind;  /* 'G' global + offset, 'F' function, 'I' into port_data */\n"
           "    void *target;        /* the global or function (NULL for 'I') */\n"
           "    unsigned int size;   /* sizeof the global ('G') */\n"
           "    unsigned int offset; /* offset into the global ('G') or into port_data ('I') */\n"
           "    unsigned int data;   /* the original target's offset in port_data, or ~0u if outside it */\n};\n\n")
out.append("static const struct PortFixup port_fixups[] = {\n")
for off, kind, name, x in fixups:
    v = struct.unpack_from("<I", image, off)[0]
    dat = ("0x%xu" % (v - DATA_LO)) if DATA_LO <= v < DATA_HI else "~0u"
    if kind == "G":
        ga = global_containing(v)
        out.append("    {0x%x, 'G', (void *)&%s, %s, 0x%x, %s},\n" % (off, name, size_expr(ga), x, dat))
    elif kind == "F":
        out.append("    {0x%x, 'F', (void *)%s, 0, 0, %s},\n" % (off, name, dat))
    else:
        out.append("    {0x%x, 'I', NULL, 0, 0x%x, %s},\n" % (off, x, dat))
out.append("};\n\n")
out.append("struct PortGlobal {\n    void *addr;\n    unsigned int size;   /* sizeof in the port */\n"
           "    unsigned int offset; /* original address - PORT_DATA_VA */\n"
           "    unsigned int gap;    /* bytes up to the next annotated global */\n};\n\n")
out.append("static const struct PortGlobal port_globals[] = {\n")
for a, name, gap in fill:
    out.append("    {(void *)&%s, sizeof(%s), 0x%x, 0x%x},\n" % (name, name, a - DATA_LO, gap))
out.append("};\n\n")
out.append(r"""unsigned int PortDataOutsideTargets = 0; /* 'G' targets past the end of the port's (smaller) global */

void PortLoadData(void) {
    unsigned int i;

    /* 1. patch the pointer slots in the copy */
    for (i = 0; i < sizeof(port_fixups) / sizeof(port_fixups[0]); i++) {
        const struct PortFixup *f = &port_fixups[i];
        void *p;

        if (f->kind == 'G' && f->offset < f->size) {
            p = (char *)f->target + f->offset;
        } else if (f->kind == 'G') {
            /* the port declares this global smaller than the original's gap: point into the copy instead */
            PortDataOutsideTargets++;
            p = f->data != ~0u ? (void *)(port_data + f->data) : (char *)f->target + f->offset;
        } else if (f->kind == 'F') {
            p = f->target;
        } else {
            p = port_data + f->offset;
        }
        memcpy(port_data + f->slot, &p, sizeof(p));
    }

    /* 2. fill the globals */
    for (i = 0; i < sizeof(port_globals) / sizeof(port_globals[0]); i++) {
        const struct PortGlobal *g = &port_globals[i];
        unsigned int n = g->size < g->gap ? g->size : g->gap;

        memcpy(g->addr, port_data + g->offset, n);
    }
}

/* Checks PortLoadData's work and writes a report to path: every filled global holds its original bytes, and
   every patched pointer inside a filled global points where the table says. Returns the number of problems. */
int PortDataSelfTest(const char *path) {
    FILE *f = fopen(path, "w");
    unsigned int i;
    unsigned int j;
    unsigned int bytes = 0;
    unsigned int pointers = 0;
    int bad = 0;

    if (f == NULL) {
        return -1;
    }
    for (i = 0; i < sizeof(port_globals) / sizeof(port_globals[0]); i++) {
        const struct PortGlobal *g = &port_globals[i];
        unsigned int n = g->size < g->gap ? g->size : g->gap;

        if (memcmp(g->addr, port_data + g->offset, n) != 0) {
            fprintf(f, "global at original 0x%08x: bytes differ\n", PORT_DATA_VA + g->offset);
            bad++;
        }
        bytes += n;
        for (j = 0; j < sizeof(port_fixups) / sizeof(port_fixups[0]); j++) {
            const struct PortFixup *x = &port_fixups[j];
            void *want;
            void *have;

            if (x->slot < g->offset || x->slot + 4 > g->offset + n) {
                continue;
            }
            memcpy(&have, (char *)g->addr + (x->slot - g->offset), sizeof(have));
            if (x->kind == 'F') {
                want = x->target;
            } else if (x->kind == 'I') {
                want = port_data + x->offset;
            } else if (x->offset < x->size) {
                want = (char *)x->target + x->offset;
            } else {
                continue; /* fell back to the copy; checked by value above */
            }
            if (have != want) {
                fprintf(f, "pointer at original 0x%08x: %p, expected %p\n", PORT_DATA_VA + x->slot, have, want);
                bad++;
            }
            pointers++;
        }
    }
    /* globals the port declares smaller than the original's data: the bytes past their end aren't loaded */
    {
        unsigned int truncated = 0;
        unsigned int lost = 0;

        for (i = 0; i < sizeof(port_globals) / sizeof(port_globals[0]); i++) {
            const struct PortGlobal *g = &port_globals[i];
            unsigned int k;
            unsigned int nonzero = 0;

            for (k = g->size; k < g->gap; k++) {
                nonzero += port_data[g->offset + k] != 0;
            }
            if (nonzero) {
                fprintf(f, "undersized: global at original 0x%08x is %u bytes in the port; the original has %u more "
                           "non-zero bytes before the next known global\n",
                        PORT_DATA_VA + g->offset, g->size, nonzero);
                truncated++;
                lost += nonzero;
            }
        }
        fprintf(f, "undersized globals: %u (%u non-zero original bytes not loaded into them)\n", truncated, lost);
    }
    fprintf(f, "globals: %u (%u bytes), pointers inside them checked: %u, fixups: %u, targets past a global's end: %u\n",
            (unsigned int)(sizeof(port_globals) / sizeof(port_globals[0])), bytes, pointers,
            (unsigned int)(sizeof(port_fixups) / sizeof(port_fixups[0])), PortDataOutsideTargets);
    fprintf(f, "%s: %d problem(s)\n", bad ? "FAILED" : "OK", bad);
    fclose(f);
    return bad;
}

const void *PortOriginalData(unsigned int va) {
    if (va < PORT_DATA_VA || va >= PORT_DATA_VA + PORT_DATA_SIZE) {
        return NULL;
    }
    return port_data + (va - PORT_DATA_VA);
}
""")
if "--report" not in sys.argv or "--write" in sys.argv:
    OUT.parent.mkdir(exist_ok=True)
    OUT.write_text("".join(out), encoding="latin-1")
    print("wrote", OUT.relative_to(ROOT), "(%d KB)" % (OUT.stat().st_size // 1024))
