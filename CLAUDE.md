# legoland — Windows 11 port (instructions for Claude)

**This is the Windows 11 port** ([facundosterzerforino/legoland-port](https://github.com/facundosterzerforino/legoland-port)).
Its goal is a game that runs natively on modern Windows 11, built from the matching decompilation (`main` of the
decomp repo, [facundosterzerforino/legoland](https://github.com/facundosterzerforino/legoland)). The rules in this first section override the matching rules further down, which still
describe how `main` works.

## Port rules

**`PORTING.md` holds the full rules; read it before changing code.** It follows
[isle-portable](https://github.com/isledecomp/isle-portable), so that the game can later run on other systems
(Android is a planned target). The short version:

- **Goal:** platform independence. The game must look, sound and play the same as the original. A function
  does not need to match the original byte for byte here.
- **Rewrites and optimizations are allowed when they break nothing**: same pixels, sounds, timing and saves.
  Verify before committing (old vs new on the same inputs, or screenshots). Tag rewritten functions
  `// [port:rewrite]` with what changed, prefer functions already 100% matched in the decomp, and keep
  rewrites in their own commits. Gameplay-changing fixes go in `extensions/` as an option.
- **The decomp is the source of truth for what the game does.** Merge its `main` in regularly. Portability
  changes to `src/legoland/` go in small single-subsystem commits; better names go into the decomp first; don't
  reorder decompiled functions.
- **Layout:** `src/legoland/` is the decompiled game; `miniwin/` reimplements the Windows APIs it calls on
  SDL3; `port/` holds the entry point, config, data loading and asm replacements; `extensions/` holds optional
  features (off by default); `3rdparty/` holds unmodified libraries.
- **Prefer a `miniwin/` shim** over editing a call. When a call must change, change it in place and tag it with
  the subsystem: `// [library:video]`, `[library:audio]`, `[library:input]`, `[library:config]`, and the rest of
  the table in `PORTING.md`. No `#ifdef PORT` blocks around whole functions.
- **Inline-assembly functions:** write plain-C equivalents that do the same thing. Comment that the original
  was inline asm and tag them `// [library:asm]`.
- **Portable C:** no pointers stored in `int`/`unsigned int` (use pointer types or `uintptr_t`), exact-size
  types and explicit little-endian loads for file formats, no unaligned or type-punned access, explicit
  `signed`/`unsigned char`, no compiler-specific keywords or x87 assumptions in shared code, `/` paths with
  case-insensitive lookup.
- **Game data:** never commit proprietary files. The port reads the user's own `legoland.exe` and game data at
  runtime.
- **Matching-only workarounds** from the decomp (`volatile` temporaries, `#pragma optimize`, duplicated
  `static __inline` helpers) may be cleaned up here when they get in the way.
- `./tools/verify` still works and is useful to see which functions a change touched, but a lower score is not
  a failure in this repo.
- See `README.md` for status and `PORTING.md` for the order of work.
- **Merging the decomp** (it renames functions, globals and struct fields all the time):
  1. `git -C ~/legoland log port..main` to see what's new; pick the commit to merge (`NEW`).
  2. In `~/legoland`: `python3 ~/wt/port/tools/decomp_renames.py <last merged decomp commit> NEW /tmp/ren.json`.
  3. In `~/wt/port`: `python3 tools/resolve_rename_conflicts.py /tmp/ren.json --apply-only`, commit, then
     `git merge NEW`. For conflicts: `git checkout --conflict=diff3 -- <file>` for each conflicted file, then
     `python3 tools/resolve_rename_conflicts.py /tmp/ren.json` takes the decomp's side wherever the port only
     renamed; anything left is a real port change to resolve by hand.
  4. Fix what port-only code still calls by an old struct-field name (the build says where), regenerate
     `python3 tools/gen_data.py`, build both presets, run `legoland.exe -port-selftest`.
  5. Run the data audits; each exits 1 and lists what's new. The decomp is checked only on its code, so a
     guessed variable size or a wrong table entry never fails to match, but in the port it overwrites
     neighbouring globals (most crashes found while making the game playable were this):
     - `python3 tools/audit_globals.py`: globals declared smaller than their space in the original exe and
       used as tables/buffers, and the reverse: one object of the original split into several globals
       (`overlap`: a global reaching past the next one; `cast`: `(struct T *)&G` bigger than G's space).
       The boating-school erase crash and the castle state block were this. Fix the declaration (in the
       decomp), or add a reviewed harmless case to `tools/audit_globals_ok.txt`.
     - `python3 tools/audit_tables.py`: every pointer table written out in C, entry by entry against the exe.
     - `python3 tools/audit_values.py`: C initializers the startup loader doesn't replace (`static`,
       `const`, opaque types), byte by byte against the exe.
     - `python3 tools/audit_null.py`: places that set a pointer to NULL and then use it. The original crashed
       there; clang would delete the check and write out of bounds, hence `-fno-delete-null-pointer-checks`.
     - `python3 tools/audit_locals.py`: `&local` passed where the callee writes past element 0 (`p[1] = ...`),
       or a scalar local cast to a struct pointer. MSVC6 kept the locals adjacent, so it matched; modern
       compilers don't (the earth slide's visitor target was this).
     - In the decomp: `./tools/verify --silent --json r.json && python3 ~/wt/port/tools/audit_effective.py
       r.json` lists "100% effective" matches whose jumps moved (swapped if/else arms change behaviour).
- The real repo and this worktree live in WSL Ubuntu (`~/wt/port`, local branch `port`, a worktree of
  `~/legoland`); the Windows folder `C:\Users\fsterzer\Dropbox\Decomp\legoland` is a stale clone.
- **Publishing:** the local `port` branch is pushed to `main` of the `legoland-port` repo with `~/push-port.sh`
  (remote `port-repo` in `~/legoland`). Decomp work still goes to the `legoland` repo; never push port changes
  there.

## Working with the maintainer (handoff, 2026-10-04)

- **Never launch, click or type into the game.** The maintainer does all in-game testing. You may run only
  `legoland-port.exe -port-selftest` (no window).
- **After every fix:** build both presets, run the selftest, copy `build-clang-x86/legoland.exe` (+ `.pdb`)
  into `C:\Users\fsterzer\Dropbox\legoland pc port\run\` as both `legoland-port.exe` and
  `legoland-windowed.exe` (only if no `legoland*` process is running), push with `~/push-port.sh`, and report
  the commit and the exe's sha256. Push validated fixes without asking.
- **Decomp-side bugs:** fix them in `~/legoland` (commit on `main`, check `./tools/verify`), then merge `main`
  into `port`. The maintainer pushes the decomp fork himself. Port-only fixes are listed in
  `docs/decomp-fixes-todo.md` when they also belong in the decomp.
- **Debugging a crash/freeze:** read `run\legoland-port-trace.txt` (the previous run is
  `legoland-port-trace-prev.txt`). The watchdog (`port/port_watchdog.c`) writes `CRASH:`/`FREEZE detected` with a
  symbolized stack and saves `run\crash-*.dmp` / `freeze-*.dmp`. For crashes the in-game handler can't catch,
  `tools/dbgrun` is a small debugger (`dbgrun.exe <log> <exe> args`). `tools/minidump.py` reads a dump from WSL by
  global name (values, neighbours, zeroed runs, value search; pass the .pdb of the exe that crashed). The watchdog
  arms hardware write watchpoints (`watch:` lines in the trace, `ArmWatches` in `port/port_watchdog.c`) to catch
  code that overwrites globals it doesn't own. Work from the code; ask the maintainer to reproduce.
- **Windowed mode:** `run\legoland-windowed.exe WINDEBUG`, working directory `installed\`. (Windows keeps
  "16-bit colour, 640x480" compatibility flags on `legoland-port.exe`, which change the desktop resolution.)
- **Most bugs so far were data, not code:** globals declared smaller than in the original exe, tables with
  missing or wrong entries, unfinished functions, and MSVC6 tricks that are undefined behaviour in modern C (e.g.
  `(&param_4)[1]`). Run the audits in step 5 of the merge steps above after every decomp merge.

Status (phase 4, `ROADMAP.md`): boot, menus, tutorial, building, music, sound, speech, movies (Indeo via FFmpeg),
windowed mode all work on Windows 11. Fixed on 2026-10-04: lesson-2 tree objective, greenhouse crash, copters
crash (NULL sample). Open:
- Sensory coaster crash (GetTileBounds, map globals zeroed): fixed 2026-10-07. The watchpoints caught the
  z-buffer clear (`FUN_00423140` -> `FUN_004232b0` -> `PortFillPolygon`): `FUN_004232b0` stored the end line
  through an offset from `idx` that only reaches `src[n - 1]` when the two locals are adjacent (MSVC6), so
  clang left it uninitialized and the fill ran ~2.4 MB past the buffer. Fixed in the decomp (`08df14b`);
  confirm in game. The watchpoints in `ArmWatches` can come out once it is confirmed.
- Copters sounds: fixed 2026-10-07 (gen_data aimed their name pointers past the 8-byte `CopterPathTable4`);
  confirm in game that the helicopter sounds and copter models load ("Loaded SFX Helicopter ..." in the trace).
- Phase 4 is done once the maintainer's full playthrough (all levels, the three parks, gallery, save/load, end
  screens, certificate printing) finds nothing new; then mark it done in `ROADMAP.md`. No reference VM: this
  port, on the original DirectX APIs, is the reference for phases 5+ (decided 2026-10-08).
- Certificate printing: fixed 2026-10-08 (default printer by name, full landscape DEVMODE); confirm in game.
- Temporary diagnostics to remove after the playthrough: the `build:` and `BubbleHelp:` trace lines
  (map_object.c, text.c) and the coaster watchpoints (`ArmWatches`).

---

# Matching rules (from `main`)

This is a matching decomp of `legoland.exe` (MSVC6, x86) in the isledecomp/reccmp style. Goal: write C
that the *original* MSVC6 compiler turns into bytes identical to the original binary, checked
per-function by `reccmp`. We do **not** reproduce the binary's layout — reccmp normalizes addresses.

## Toolchain

- Everything is downloaded by `uv run setup.py` into `toolchain/` (gitignored): MSVC6 (`toolchain/msvc6`),
  msvcrt DLLs (`toolchain/dlls`). Do not commit `toolchain/`.
- `wibo` must be on `PATH`. Both compile and link run through it (`/O2 /Z7`).
- System import libs (`kernel32`, `ddraw`, …) are committed in `libs/` (not downloadable); the link
  wrapper adds them automatically and LINK pulls only what's referenced.

## Build & verify

```sh
cmake --preset msvc6 && cmake --build build   # -> build/legoland.exe + PDB
cmake --build build --clean-first             # full rebuild (needed after header changes — no dep tracking)
./tools/verify                                 # per-function + total match %
./tools/verify -v 0x004015c0                   # asm diff for one function
uv run tools/progress.py                       # per-TU progress table
uv run tools/progress.py --worst 30            # the 30 lowest-scoring functions
uv run tools/asm2naked.py 0x00466d80 --apply   # transcribe a hand-written-asm function as naked __asm
```

**Header changes require `--clean-first`** — the MSVC6 wrapper does not track header dependencies.

`tools/verify` wraps `reccmp --target LEGOLAND`. reccmp's `cvdump.exe` (PDB reader) runs
through wibo via `tools/wine`; `tools/winepath` maps wibo's `Z:`-root PDB paths.

When you decompile a function: replace its `void name(void) { STUB(); }` body with
real C and the correct signature, rebuild, and run `./tools/verify -v <addr>` to
iterate the asm diff to 100%. A few empty/trivial functions already match the bare
stub. The image links with `/NODEFAULTLIB` and a dummy `/ENTRY` (`legoland_entry` in
`bootstrap.c`); imported APIs resolve from the committed `libs/` import libs.

## decomp.me workflow

Target asm for each function is in `../port2/project/asm/0x<ADDR>.asm`. Use the decomp-match
agent type to iterate scratches on decomp.me to a byte-match, then integrate the result into
the codebase. Scratches can be created via the MCP tools or the CLI in `../mcp/`.

See `docs/decomp-tips.md` for MSVC6 /O2 codegen patterns (register allocation, switch vs if-else,
struct assignment, etc.).

## Conventions

- One `.c` per TU (translation unit) under `src/legoland/`, named from `ghidra/functions.csv`'s `tu`
  column (`TU_RIDE_BLOKE` → `ride_bloke.c`). Functions appear in **address order**.
- Every function is tagged `// FUNCTION: LEGOLAND 0x<addr>` immediately above it.
- Unmatched functions have a `STUB();` body (macro in `legoland.h`). No game function is at `STUB()` any more:
  the last ones (hand-written assembly) were decompiled on 2026-10-07, and all but one match.
  `docs/remaining-work.md` lists what is left.
- When you decompile a function: replace its `STUB()` body with real C, build, run reccmp, iterate to 100%.
- **Before retrying a partial match, read its file in `docs/attempts/`** (what was tried, with scores) and don't
  repeat a listed attempt; afterwards add your attempts there, failed ones included. `docs/attempts/README.md`
  has the lessons that apply to most of them.
- No `ctx.h` — include real MSVC6 headers; shared decls go in `src/legoland/legoland.h`.
- **No forward declarations in `.c` files** — put all declarations in the TU's `.h` header.
  Run `uv run tools/needsdecl.py` to check.
- **Inline `__asm` and `__declspec(naked)` only where the original is hand-written assembly** — i.e. it
  contains instructions MSVC6 never emits from C (`rdtsc`, `xchg`, `pusha`, `shrd`, `fistp` after `fstp/fld`, …;
  see "Functions With an ebp Frame" in `docs/decomp-tips.md`). `uv run tools/progress.py <tu>` flags them.
  Everything else stays pure C. First try C with `__asm` only for the parts C can't produce. If MSVC6's
  register or stack-slot choices around the asm won't converge, transcribe the whole function as
  `__declspec(naked)` (`tools/asm2naked.py` does it) and say so in the comment above `// FUNCTION:`. The port
  replaces these functions with plain-C equivalents tagged `// [library:asm]`.
- **`goto` only with named labels** — cleanup/error paths like `goto fail;` are fine (the original code
  used them, and they often match better than duplicated cleanup). No Ghidra-style `goto LAB_00446b71;`:
  give the label a meaningful name or restructure with `if`/loops.
- **Run `clang-format -i`** on all modified `.c`/`.h` files before committing.
- **Retype globals** when casts can be removed. If a global is always cast to `struct Foo *`,
  change its type in `globals.h`/`globals.c`. Consecutive globals that form a struct
  (e.g., `EditMode` + `DAT_008119b4` + `DAT_008119b8`) should be merged into one struct global.
- **`TU_CRT` and `TU_IMPORTS` are NOT decompiled** — they exist as stub symbol files
  (`src/legoland/crt.c`, `imports.c`) using `// STUB: LEGOLAND 0x<addr>` annotations. Game functions
  call into them (CRT helpers like `memcpy`/`__ftol`/heap routines, and import thunks), so the symbols
  must be present for callers to *link* and for reccmp to *match* the call. **Leave every function in
  them as `STUB()` — never fill them in.** `// STUB:` keeps them out of the match % (so it reflects
  game functions only). One day they could be satisfied by a real CRT lib instead.

## Don't

- Don't edit `external/legoland.exe` (the match target) or anything in `toolchain/`.
- Don't try to make the binary *runnable* — that's a non-goal; it only needs to link + carry symbols.
- Don't commit downloaded toolchain binaries.
- Don't integrate/decompile `crt.c` / `imports.c` functions — they stay `STUB()` (symbols only; see above).
