# legoland — Windows 11 port

This branch (`port`) turns the LEGOLAND decompilation into a **game that runs natively on modern Windows 11**.
You supply your own copy of the 1999 game; the port uses its files and does not ship any game data.

It is built on the **matching decompilation** on the `main` branch, which recreates `legoland.exe` as C that
the original Microsoft Visual C++ 6.0 compiler turns into byte-identical code (verified per function with
[`reccmp`](https://github.com/isledecomp/reccmp)). `main` is the source of truth for the game's code. This branch
adds what it takes to run it: a real program entry, the game's data, replacements for hand-written assembly, and
modern Windows support.

## Status

| | |
|---|---|
| Decompiled code (from `main`) | 94.0% reccmp progress; every pure-C function decompiled, 97% average similarity |
| Runs on Windows 11 | **not yet**: the build links but does not start (see milestone 1) |
| Hand-written assembly still missing | 44 functions (software renderer, 3D math, character renderer) |

## Milestones

1. **A program that starts.** Today the build links with no C runtime and a dummy entry point, purely so
   `reccmp` can compare functions. The port needs:
   - the real C runtime startup and libraries;
   - the game's initialized data (about 92 KB of tables and constants), loaded at startup from your own
     `legoland.exe`, since most globals in `globals.c` are declared without their original values;
   - plain-C versions of the 44 functions the original wrote in inline assembly.
2. **Title screen on Windows 11.** Boot, load the title-screen UI, and run the frame loop with input. Windows 11
   still ships DirectDraw, DirectSound and DirectInput, so the original API calls are the first thing to try.
   Movies (FLC/AVI) and music are out of scope at this stage; the game already has a `-nomusic` switch, and
   DirectMusic may not be available on Windows 11.
3. **Playable.** All game screens, sound effects, saving and loading, the parks themselves.
4. **Modern niceties.** Windowed mode, higher resolutions, and, where the 1999 APIs misbehave, replacing them
   (for example DirectDraw/DirectSound with SDL), plus music without DirectMusic.

## How this branch relates to `main`

- Game logic stays as close to `main` as possible, so new matches can flow in: **merge `main` into `port`
  regularly**.
- Port-only work (startup, data loading, assembly replacements, platform code) is kept separate where possible,
  for example in new files, so those merges stay easy.
- Matching-only tricks on `main` (`volatile` temporaries, `#pragma optimize`, duplicated inline helpers) can be
  cleaned up here when they get in the way.

## Building

For now this branch builds exactly like `main`, with the original MSVC6 toolchain run through
[`wibo`](https://github.com/decompals/wibo) from WSL:

```sh
uv run setup.py          # download MSVC6 + DLLs into toolchain/
cmake --preset msvc6
cmake --build build      # -> build/legoland.exe
./tools/verify           # still useful: shows which functions a port change touched
```

Whether to move to a modern compiler (MSVC 2022 or clang-cl, still 32-bit x86) is an open decision for
milestone 1.

## You need your own game

The port reads data from an original LEGOLAND installation (the `.res` volumes, sounds, movies, and
`legoland.exe` itself for the initialized data). No proprietary files are committed to this repository.

## More documentation

- `CLAUDE.md`: working rules for AI-assisted sessions (port rules first, then the matching rules from `main`).
- `DECOMPILING.md`, `HEADERS.md`, `docs/decomp-tips.md`: how the matching decompilation works.
- `docs/title-screen-port.md`: an earlier scoping study of the boot-to-title-screen path. Its match numbers
  date from when the decompilation was at 52.71%; most functions it lists as missing are done now.
