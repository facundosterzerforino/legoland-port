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
