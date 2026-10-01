# Porting rules

These are the rules for turning the LEGOLAND decompilation into a portable game. They follow the model of
[isle-portable](https://github.com/isledecomp/isle-portable), the portable version of LEGO Island built on the
[isle](https://github.com/isledecomp/isle) decompilation, which now runs on Windows, macOS, Linux, Android,
iOS, the web and consoles from one codebase.

The first target is Windows 11. Android is a planned target, so every rule below is written so that the same
code can also build for Android (64-bit ARM), Linux and macOS later, without a rewrite.

## Scope

- **The goal is platform independence.** The game must look, sound and play the same as the original: same
  gameplay, timing, visuals and saves.
- **Rewriting is allowed when it breaks nothing.** Unlike isle-portable, code may be rewritten to be clearer,
  safer or faster, as long as the behavior stays the same. "The same" means the same pixels, sounds, timing
  and saved data. Check it before committing, for example by comparing the old and new function on the same
  inputs or on screenshots. Fixing crashes and undefined behavior is welcome. Fixes that change gameplay
  (including original game bugs players may rely on) go in `extensions/` as an option.
- **The decompilation is the source of truth for what the game does.** Game logic comes from
  [facundosterzerforino/legoland](https://github.com/facundosterzerforino/legoland) and is merged in regularly.
  A rewritten function no longer merges cleanly, so:
  - tag it `// [port:rewrite]` (with a line on what changed), so a merge conflict there is resolved by keeping
    the port's version;
  - prefer rewriting functions that are already finished in the decomp (100% matched), since they won't
    change there any more;
  - keep rewrites in their own commits, separate from portability changes.
- **Optional features** such as widescreen, higher resolutions, mods or cheats live in `extensions/` and are
  off by default. They never change the default game.

## Repository layout

| Directory | Contents | Rules |
|---|---|---|
| `src/legoland/` | The decompiled game (like isle-portable's `LEGO1/`). | Change only to make it portable. Keep its structure, names and address annotations, so merges from the decomp stay clean. |
| `miniwin/` | A small re-implementation of the Windows APIs the game calls, built on SDL3: `include/miniwin/windows.h`, `ddraw.h`, `dsound.h`, `dinput.h`, `vfw.h`, `mmsystem.h` and so on, plus `src/` grouped by API. | The game includes these headers instead of the Windows SDK on every platform except a "native Windows" build. Implement only what the game uses. |
| `port/` | Port-only code that isn't a Windows API: the real entry point and main loop, config, loading the game's data, plain-C replacements for inline assembly (today's `port_asm.h` moves here), path handling. | |
| `extensions/` | Optional features. | Each one has its own switch, is off by default and hooks in at as few points as possible. |
| `3rdparty/` | SDL3, miniaudio, iniparser and any other libraries, unmodified. | Don't edit them; style rules don't apply. Prefer git submodules or CMake `FetchContent` over copied sources. |
| `android-project/`, `packaging/` | Platform build and packaging files (Gradle project, installers). | Added when that platform is started. |

## Library substitutions

Every place in the code that touches one of these subsystems carries a searchable tag comment, as
isle-portable does (`// [library:window]` and so on). That way `grep -rn "\[library:audio\]"` lists everything
that still needs work, or everything that changed, for one subsystem.

| Subsystem (what the original uses) | Replacement | Tag |
|---|---|---|
| Window, message loop, events (`CreateWindow`, `PeekMessage`, `WndProc`) | SDL3 | `[library:window]` |
| 2D video (DirectDraw: 640x480 16-bit surfaces, blits, locking) | SDL3 (a 16-bit framebuffer uploaded to a texture each frame) | `[library:video]` |
| Software 3D renderer (inline asm in `castle.c`, `draw.c`, `render.c`, `man3d.c`) | Plain C; it already draws into a memory buffer, so it needs no GPU API | `[library:asm]` |
| Keyboard, mouse, cursor (DirectInput, `GetCursorPos`, `ShowCursor`) | SDL3; touch on Android acts as the mouse | `[library:input]` |
| Sound effects (DirectSound, `waveOut`, ACM decompression `acmStream*`) | SDL3 audio + miniaudio (its WAV decoder also handles MS and IMA ADPCM, which ACM decoded) | `[library:audio]` |
| Music (DirectMusic through COM `CoCreateInstance`: 30 segments `.sgt` and 212 styles `.sty`) | [GothicKit/dmusic](https://github.com/GothicKit/dmusic), a C re-implementation of DirectMusic's segment and style playback (see the notes below) | `[library:music]` |
| Movies (`AVIFile*`/`AVIStream*` from vfw32): Indeo Video 5 (`IV50`) and one Indeo 3.2 (`IV32`), with PCM, MS ADPCM or IMA ADPCM sound | FFmpeg's `libavcodec`, built with only the `indeo5`, `indeo3`, `adpcm_ms` and `adpcm_ima_wav` decoders (LGPL), plus a small AVI reader of our own | `[library:movie]` |
| Settings (Windows registry `Reg*`) | iniparser (an `.ini` file in the user's data folder) | `[library:config]` |
| Files and folders (`CreateFile`, `fopen`, `SetCurrentDirectory`, drive/volume and CD checks) | SDL3 filesystem plus a case-insensitive path lookup; the CD check passes when the data is present | `[library:filesystem]` |
| Timers (`timeGetTime`, `GetTickCount`, `QueryPerformanceCounter`, `rdtsc` profiling) | SDL3 timers | `[library:timer]` |
| Threads and sync (music thread, events, critical sections) | SDL3 | `[library:thread]` |
| Message boxes (`MessageBox`) | `SDL_ShowSimpleMessageBox` | `[library:dialog]` |
| GDI bitmaps (`CreateDIB*`, `BitBlt`, `StretchBlt`) | Plain C on memory buffers | `[library:gdi]` |
| Certificate printing (winspool, `StartDoc`) | Save the certificate as an image instead | `[library:print]` |

Notes on the two harder rows:

- **Music.** LEGOLAND uses DirectMusic's adaptive composition (styles and segments), not plain MIDI files, so
  a MIDI player isn't enough. GothicKit/dmusic plays exactly these formats and is used by the Gothic remakes on
  several systems; it is still incomplete, so LEGOLAND's soundtrack must be checked by ear. The game ships no
  instrument file (`.dls`): it relies on the General MIDI set built into Windows (`gm.dls`), which can't be
  redistributed. On Windows the port can load the user's own `gm.dls`; other systems need a free General MIDI
  sound bank instead.
- **Movies.** Indeo was never open, and FFmpeg's decoders are the only maintained open implementations.
  Building `libavcodec` with just these four decoders keeps it small. The fallback, if a dependency on FFmpeg
  is unwanted, is to port its `indeo5` decoder (a few thousand lines) into `port/`.

Unlike LEGO Island, LEGOLAND has **no Direct3D**: its 3D is drawn by its own software renderer. Once the
inline assembly is plain C, the whole picture is one 16-bit buffer. Showing it on any platform is a single
texture upload, so the port doesn't need renderer backends.

## How to change decompiled code

1. **Prefer `miniwin/`.** If an API can be reimplemented behind the same name and signature, do that, and the
   decompiled code stays untouched.
2. **If a call has to change, change it in place** and tag it: `// [library:audio] was DirectSoundCreate`.
   Don't wrap whole functions in `#ifdef PORT`; one build of `src/legoland/` serves every platform.
3. **Keep each change small and in one subsystem**: one subsystem per commit, about 10 files at most, so a
   merge conflict with the decomp is easy to resolve.
4. **Names belong in the decomp.** A better name for a decompiled function, global or struct field should go
   into the decomp first and be merged in from there, so both repos agree. Don't reorder decompiled functions;
   address order keeps merges readable.
5. **Inline assembly** becomes plain C that does the same thing. Add a comment saying the original was inline
   asm, plus the `[library:asm]` tag. Shared helpers (`PortRound`, `PortFixMul`, `PortTimestamp`) live in
   `port/`.

## Portable C rules

These are what make an Android (64-bit ARM) build possible later. They apply to all new and changed code.

- **Pointers are pointers.** Never store a pointer in `int` or `unsigned int`: on 64-bit platforms it doesn't
  fit. Use real pointer types, or `uintptr_t` when an integer is unavoidable. Many decompiled signatures still
  use `unsigned int` for pointers (for example `FUN_00420e90(unsigned int mesh, ...)`); fix them as you touch
  them.
- **File formats use exact sizes.** Read the game's files into structs made of `int32_t`, `uint16_t` and so on,
  with explicit little-endian loads. Never `fread` straight into a struct that contains pointers, and don't
  rely on MSVC padding.
- **No unaligned or type-punned access** through casted pointers (`*(int *)(bytes + 3)`). Use `memcpy` or
  byte-wise loads; ARM and optimizing compilers can break the cast.
- **`char` signedness.** `char` is unsigned on ARM. Where the sign matters, write `signed char` or
  `unsigned char`.
- **No compiler-specific code** in shared files: no `__stdcall`/`__cdecl` (miniwin defines them as empty),
  `__int64` (use `int64_t`), `__declspec`, `_ftol`, inline asm or `#pragma` packing tricks.
- **Floating point.** Don't depend on x87 80-bit precision or `_control87`. Where the original rounds with
  `fistp` (round to nearest), use `PortRound`; a plain C cast truncates.
- **Paths.** Use `/` and compare names case-insensitively through the filesystem layer. Never hard-code drive
  letters or `C:\`.
- **The data folder is configurable.** On Android it is app storage the user copies the game into; on Windows
  it is the game's install folder.

## Game data

- **Never commit game data**: no `.res` volumes, movies, sounds, music, level files, installer archives or
  CD images. They are copyrighted. `.gitignore` blocks these file types.
- **The one exception is `legoland.exe`**, the program itself, which stays committed as `external/legoland.exe`
  (the decomp needs it to match against, and the port reads its initialized data).
- On the maintainer's machine the game files live outside the repo in `C:\Users\fsterzer\Dropbox\legoland pc port`:
  `cd/` is a copy of the CD and `installed/` is the unpacked `main.z`.
- The reference CD (`LEGOLAND.iso`, volume `LEGOLAND`, readme dated 10 April 2000) has these loose files: the
  `.res` volumes (`Legoland.res`, `Graphics1.res`, `Graphics2.res`), 14 `.avi` movies, 1,266 speech `.wav`
  files, an Indeo 5 codec installer and DirectX 7 setup. The game itself is inside the InstallShield 3 archive
  `main.z` (PKWARE DCL compression): `legoland.exe`, 30 `.sgt` and 212 `.sty` music files, 26 small silent
  `.avi` animations (Indeo 5, 112x96),
  23 `.bnv` and other level files. Its `legoland.exe` is byte-identical to the decomp's target (sha256
  `c50865b6...e2bd9`). The port will need a small installer step that unpacks `main.z`.
- The port reads everything from the user's own installation, including the initialized globals from
  `legoland.exe` (the `.data` tables that `globals.c` declares without values). That loader lives in `port/`.

## Build

- CMake with presets per platform. The port moves to modern compilers (MSVC 2022, clang, gcc, and the Android
  NDK later). The MSVC6/`wibo` build stays in the decomp repo, where matching happens.
- Builds must have no warnings in `port/`, `miniwin/` and `extensions/`. Turn on `-Wall` and the 64-bit
  pointer-truncation warnings early, because they find the "pointer in an int" bugs.
- Formatting: `clang-format` with the repo's `.clang-format`, on everything except `3rdparty/`.

## Order of work

1. **Finish the inline asm** in plain C (22 functions left) and make the code build with a modern compiler
   as 32-bit x86 Windows.
2. **Startup:** a real entry point in `port/`, the C runtime, and the data loader for initialized globals.
3. **miniwin + SDL3 on Windows:** window, 2D video, input, timers. Goal: the title screen.
4. **Sound, then movies and music**, then every screen of the game: playable on Windows 11.
5. **64-bit clean:** build and run as x64 Windows with no pointer-size warnings.
6. **Linux** (a cheap test that nothing Windows-only is left), **then Android**: Gradle project, touch input,
   data-folder picker.
