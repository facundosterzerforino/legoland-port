# Phase 8 tools: the decomp's types, for 64-bit

The port's 64-bit build (preset `clang-cl-x64`) needs every pointer in the game code to have a pointer type, and no
struct that is only a partial "view" of another (fixed offsets that line up on 32-bit only). These fixes go in the
decomp (`~/legoland`, `main`), checked with reccmp to keep the match, then merge into the port (ROADMAP.md phase 8).

The work is done in a scratch worktree of the decomp, `~/scratch/dv64`, so the shared `~/legoland` is only touched
when a commit is moved onto `main`.

- `dvsetup.sh`: creates `~/scratch/dv64` (a detached worktree of `main`, the toolchain linked, the reccmp configs
  pointed at it), builds it and writes the baseline match report, `/tmp/dv_baseline.json`.
- `x64check.sh [file.c...]`: the 64-bit pointer warnings the port will have after merging the scratch decomp's
  edits. It copies the port's `src/legoland` to `/tmp/x64overlay`, merges each changed file in as `git merge` would
  (base: the last decomp commit the port merged; where both sides changed the same lines it takes the decomp's and lists the file in
  `/tmp/x64check.conflicts`, to resolve by hand in the real merge), and compiles every file as 64-bit (syntax
  only, the port's x64 flags). Every warning goes to `/tmp/x64check.txt`. About 8 s for all files.
- `wshow.py <warnings> <file filter> [max]`: those warnings with their code lines.
- `dvcheck.sh`: after an edit: the MSVC6 build (clean when a header changed), `verify`, the comparison with the
  baseline (`vcompare.py`: any function that got worse is listed and fails the check), then `x64check.sh`.
  About 20 s.
- `dvcommit.sh <message file>`: clang-format on the changed files, `needsdecl.py`, `dvcheck.sh` again; then commits
  in the scratch worktree and moves `main` to it with `git update-ref` (refused if `main` moved meanwhile),
  updating `~/legoland`'s index and working tree for those files only. The new report becomes the baseline.

Afterwards, merge `main` into the port as usual (CLAUDE.md, merge steps), resolving the conflicts `x64check.sh`
listed, and check the full 64-bit build: `cmake --build build-clang-x64`.

What to fix, roughly in order:
1. Globals and struct fields that hold pointers as `unsigned int` (each one causes many warnings).
2. View structs (`pad_` arrays then a few fields, used through casts) that are really another struct: use the
   real one (for example eatery's `EateryFX` was `struct Ride`).
3. Callback slots typed `void (*)()`: give each its real signature, and every handler the same one.
4. Address arithmetic on integers (`obj + 0x40`) and tables read at byte offsets: struct fields instead.

## Struct tools (libclang)

They parse the scratch decomp with clang as MSVC6 sees it (32-bit layouts). Once: `bash clangsetup.sh`. Run them
from `/tmp` with `uv run -q --no-project --with "libclang==18.1.1" python ~/wt/port/tools/phase8/<tool>.py`
(the Python bindings load the system's libclang 21; inline `__asm` blocks are blanked first, clang crashes on some).

- `views_scan.py`: every struct (fields, offsets, pads), every cast between struct pointers and every int cast to a
  struct pointer, to `/tmp/views.json` (5 s for the whole decomp). `views_cluster.py [filter]` groups structs cast
  into each other; `views_layout.py Name...` prints a group side by side by offset (`--only`: just those structs).
- `retarget.py View=Real ... [View.field=real.path ...] [--dry]`: replaces view structs with the real struct. Each
  view field maps to the real field at its offset (same name, then same size and type, then same size; nested
  fields like `piece.next` are candidates; an explicit `View.field=path` wins). It rewrites the uses (libclang's
  member references), renames `struct View` where the view was visible (`Name:file.c` when names repeat) and
  deletes the view. Type changes are listed: build, then `dvcheck.sh` proves the match. Prototypes in other
  headers: `hdr_rename.py header Old=New ...`.
- `x64fn.py file.c [function...]`: the file's 64-bit warnings by function, or those functions' warnings with code.
- `newwarn.py save|show [file.c]`: MSVC6 warnings of the last build (`/tmp/dvb.log`) not in the saved reference.
- `unused_structs.py file... [--delete]`: struct definitions nothing names any more.
- Small helpers: `sdef.py Name...` (definitions), `fdef.py fn...` (function bodies), `lines.py file ctx line|a-b`.

Typing patterns that keep the match: `INT_PTR`/`UINT_PTR` (MSVC6: `long`) for pointers compared or computed as
integers; `idx * sizeof(struct T) + offsetof(struct T, f)` where the original indexes a table by bytes; unions
of same-size aliases (`float start` / `unsigned int start_bits`) where code passes the raw bits; anonymous
embedded structs (`struct PieceHead;`, MS extension, MSVC6 and clang accept it) for a common header.

## Later tools (2026-10-14)

- `dx64.sh [file.c...]`: the scratch decomp's own files compiled as 64-bit (clang, xwin SDK, syntax only; the
  MSVC6 math.h maps to the SDK's): warnings in `/tmp/dx64.txt` with the decomp's line numbers. `dw.py file.c...`
  lists them with the source line and function.
- `cmpfix.py [--dry]`: pointer casts compared as integers (`(int)p < (int)&X`) become INT_PTR/UINT_PTR.
- `retglob.py NAME=TYPE...`: retypes globals (globals.h and globals.c) and drops the integer casts assigned to them.
- `sizefix.py` (run through `sf.sh`): constant sizes of pointer-holding structs in malloc/memset/memcpy/
  SaveGameRead/SaveGameWrite become `sizeof(struct ...)`.
- `elemchain.py file.c`: retargets a ride file's Element/Ride/RideNode/Sprite views in one go;
  `dangling.py`/`danglefix.py` find and rename references to view structs a retarget removed.
- Merging into the port: `conf.py` (conflict hunks), `resolve.py file:spec` (o/t per hunk), `dupchk.py`
  (globals declared twice), `dupproto.py`/`dupfix.py` (prototypes duplicated by the merge; keeps the decomp's).
- `whichglob.py [--decomp] addr...` (the annotated global containing an address), `dumpva.py va len` (the
  original exe's bytes there), `vf.sh addr...` (incremental build + reccmp for those functions).
- `dvcommit.sh` refuses a commit when any function got worse (except MusicThreadProc and FUN_00425e20, which
  move with unrelated edits) and pushes the decomp with `~/push-decomp.sh`.

The data loader (`tools/gen_data.py` with `tools/port_layout.py`) loads every global that holds pointers field by
field, so the 64-bit exe's `-port-selftest` passes; `gen_data.py --report` lists words that only look like
pointers (left unpatched) and pointer slots outside the filled globals.

64-bit hazards the compiler doesn't warn about (libclang scans of the port tree, i686 against x86_64; run with
`uv run --no-project --with libclang==18.1.1 python tools/phase8/<scan>.py`):
- `allocscan.py`: allocations sized with a literal (or `n * literal`) smaller than the type on 64 bits.
- `offscan.py`: `(T *)p + k` / `((T *)p)[k]` into a struct whose field at x86 offset k moves on 64 bits.
- `castscan.py`: casts between struct pointer types whose fields only line up on 32 bits (views). The reviewed
  false positives (generic pointers like Hover.ptr, a tile key read through a node) are in castscan_ok.txt.
- `voidscan.py`: the same for views reached through void * (struct fields, globals, parameters): what is stored
  in each slot against how it is cast back.
- `memscan.py`: memcpy/memset/memmove/memcmp with a literal size that is the x86 size of the struct they copy.
- `tools/audit_save.py` (ptest runs it): saved-game calls the save layer (port/port_save.c) can't convert.
