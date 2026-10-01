# legoland — Windows 11 port

This repository turns the LEGOLAND decompilation into a **game that runs natively on modern Windows 11**.
You supply your own copy of the 1999 game; the port uses its files and does not ship any game data.

It is built on the **matching decompilation**
([facundosterzerforino/legoland](https://github.com/facundosterzerforino/legoland)), which recreates `legoland.exe` as C that
the original Microsoft Visual C++ 6.0 compiler turns into byte-identical code (verified per function with
[`reccmp`](https://github.com/isledecomp/reccmp)). The decomp's `main` is the source of truth for the game's code. This
repository adds what it takes to run it: a real program entry, the game's data, replacements for hand-written assembly, and
modern Windows support.

## Status

| | |
|---|---|
| Decompiled code (from `main`) | 94.0% reccmp progress; every pure-C function decompiled, 97% average similarity |
| Runs on Windows 11 | **not yet**: builds and links with clang-cl, but most globals still lack their original values (phase 3) |
| Hand-written assembly | **done**: all 44 inline-asm functions are plain C (phase 1 of `ROADMAP.md`) |

## Goal and approach

The port follows [isle-portable](https://github.com/isledecomp/isle-portable), which took the LEGO Island
decompilation to Windows, macOS, Linux, Android, the web and consoles. Here too, the decompiled game keeps behaving
exactly like the original. Unlike isle-portable, code may also be rewritten to be clearer or faster when that
breaks nothing. Windows APIs are reimplemented in a small `miniwin/` layer on
[SDL3](https://www.libsdl.org/), and every changed call is tagged by subsystem (`// [library:video]` and so on).
Windows 11 comes first; the rules keep the code ready for Android and other systems later. See
[`PORTING.md`](PORTING.md) for the rules and the library substitution table.

LEGOLAND helps here: it has no Direct3D. Its 3D is drawn by its own software renderer into a 16-bit buffer, so
once the hand-written assembly is plain C, showing a frame on any platform is a single texture upload.

## Milestones

1. **Plain C everywhere.** Finish the assembly replacements and build with a modern compiler (32-bit Windows).
2. **A program that starts.** A real entry point and C runtime, and the game's initialized data (about 92 KB of
   tables and constants) loaded from your own `legoland.exe`, since most globals in `globals.c` are declared
   without their original values.
3. **Title screen.** `miniwin` + SDL3: window, 2D video, input and timers on Windows 11.
4. **Playable.** Sound, then movies and music, then every screen, saving and loading.
5. **64-bit and other systems.** x64 Windows with no pointer-size warnings, then Linux, then Android (touch
   input, choosing the data folder).

## Building

### The port build (clang-cl, 32-bit Windows)

The runnable build cross-compiles from Linux or WSL with clang-cl and lld-link, against the Windows SDK and
MSVC runtime fetched by [`xwin`](https://github.com/Jake-Shadle/xwin):

```sh
sudo apt install clang lld llvm cmake ninja-build
# xwin: download a release binary from GitHub, then (this accepts Microsoft's license for the SDK/CRT):
xwin --accept-license --arch x86 splat --output ~/xwin
cmake --preset clang-cl-x86
cmake --build build-clang-x86    # -> build-clang-x86/legoland.exe + .pdb
```

Set `XWIN_DIR` if the SDK isn't in `~/xwin`. The preset defines `LEGOLAND_PORT`, leaves out the matching-only
files (`bootstrap.c`, `imports.c`) and links the real C runtime (static) and Windows import libraries.
Pointer/integer mixing from the decompiled code is reported as warnings, not errors, until phase 8 (64-bit).

### The matching build (MSVC6)

The original MSVC6 toolchain, run through [`wibo`](https://github.com/decompals/wibo) from WSL, still works:

```sh
uv run setup.py          # download MSVC6 + DLLs into toolchain/
cmake --preset msvc6
cmake --build build      # -> build/legoland.exe
./tools/verify           # still useful: shows which functions a port change touched
```

More presets (x64, Linux, Android) come with later milestones.

## You need your own game

The port reads data from an original LEGOLAND installation (the `.res` volumes, sounds, movies, and
`legoland.exe` itself for the initialized data). No proprietary files are committed to this repository.

## More documentation

- `ROADMAP.md`: the phase-by-phase plan, from finishing the assembly replacements to Android.
- `PORTING.md`: the porting rules (layout, library substitutions, portable C, order of work).
- `CLAUDE.md`: working rules for AI-assisted sessions (port rules first, then the matching rules from `main`).
- `DECOMPILING.md`, `HEADERS.md`, `docs/decomp-tips.md`: how the matching decompilation works.
- `docs/title-screen-port.md`: an earlier scoping study of the boot-to-title-screen path. Its match numbers
  date from when the decompilation was at 52.71%; most functions it lists as missing are done now.
