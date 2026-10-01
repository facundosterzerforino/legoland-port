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
| Runs on Windows 11 | **not yet**: the build links but does not start (see milestone 2) |
| Hand-written assembly still missing | 22 of 44 functions (2D blitters, character renderer, 3D math) |

## Goal and approach

The port follows [isle-portable](https://github.com/isledecomp/isle-portable), which took the LEGO Island
decompilation to Windows, macOS, Linux, Android, the web and consoles. Here too, the decompiled game stays as
close to the decomp as possible. Windows APIs are reimplemented in a small `miniwin/` layer on
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

For now this repository builds exactly like the decomp, with the original MSVC6 toolchain run through
[`wibo`](https://github.com/decompals/wibo) from WSL:

```sh
uv run setup.py          # download MSVC6 + DLLs into toolchain/
cmake --preset msvc6
cmake --build build      # -> build/legoland.exe
./tools/verify           # still useful: shows which functions a port change touched
```

Milestone 1 moves the port to modern compilers (MSVC 2022, clang, gcc, later the Android NDK) with CMake
presets per platform; the MSVC6 build stays in the decomp repo, where matching happens.

## You need your own game

The port reads data from an original LEGOLAND installation (the `.res` volumes, sounds, movies, and
`legoland.exe` itself for the initialized data). No proprietary files are committed to this repository.

## More documentation

- `PORTING.md`: the porting rules (layout, library substitutions, portable C, order of work).
- `CLAUDE.md`: working rules for AI-assisted sessions (port rules first, then the matching rules from `main`).
- `DECOMPILING.md`, `HEADERS.md`, `docs/decomp-tips.md`: how the matching decompilation works.
- `docs/title-screen-port.md`: an earlier scoping study of the boot-to-title-screen path. Its match numbers
  date from when the decompilation was at 52.71%; most functions it lists as missing are done now.
