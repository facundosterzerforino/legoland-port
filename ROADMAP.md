# Port roadmap

The step-by-step plan from today's state to a native Windows 11 build, then 64-bit, Linux and Android. The
rules every step follows are in [`PORTING.md`](PORTING.md). Each phase lists what to do, what "done" means, and
the decisions to make on the way.

## Where we are (start of phase 1)

- **Game code:** 102 C files, about 100,000 lines, from the decomp (94% reccmp progress; every pure-C function
  decompiled).
- **Inline assembly:** 22 of the original 44 asm functions still need C versions.
- **Build:** MSVC6 through `wibo`, linked with no C runtime (`/NODEFAULTLIB`) and a dummy entry point
  (`legoland_entry`). The C runtime and Windows imports are stub symbols. The program links but can't run.
- **Data:** only 93 of the 1,916 globals have their original values; the rest of the original `.data` (tables,
  strings, constants) isn't in the source yet.
- **Game files:** a complete install in `C:\Users\fsterzer\Dropbox\legoland pc port\installed` (see `PORTING.md`),
  and the original `legoland.exe` in `external/`.

## Phase 1: plain C everywhere (done)

**Status:** done. All 44 inline-asm functions are plain C, tagged `[library:asm]`; the helpers are in
`port/port_asm.h`. None of it has run yet, so expect bugs to show up in phases 4 and 5.


Finish replacing the inline assembly, so every function is C a modern compiler can build for any CPU.

1. Convert the 22 remaining asm functions:
   - `draw.c` (13): the 2D blitters: sprite and transparent blits, `ZBufferHelper`, `SoftPrint_XBltFast`
     and others.
   - `render.c` (4) and `man3d.c` (2): the character renderer.
   - `castle.c` (2): the track tube mesh (`FUN_00428cb0`) and the 3-part marker (`FUN_004292f0`).
   - `copters.c` (1).
2. Tag every converted function `// [library:asm]`, the 22 already done included.
3. Move `port_asm.h` (`PortRound`, `PortFixMul`, `PortTimestamp`) into `port/`.

**Done when:** no inline-asm `STUB()` is left in the game files. **Size:** a few sessions.

## Phase 2: a modern build (done)

**Status:** done with clang-cl + lld-link from WSL (preset `clang-cl-x86`, SDK from `xwin`). Fixed on the
way: prototype-scope structs (forward declarations), prototypes out of date with their definitions, the
game's `wWinMain` colliding with the SDK's (renamed `LegolandMain` in the port build), `DIRECTINPUT_VERSION`
0x0300, and a few calls with stray arguments. Still warnings, on purpose: about 650 pointer/integer mixes
(phase 8) and 17 functions that can end without returning a value (look at them when they misbehave in
phase 4). Not yet checked: that the exe actually reaches `WinMain`; that needs phase 3's data first.


Build with a current compiler and the real C runtime, still as 32-bit Windows. 32-bit comes first because the
code still stores pointers in `unsigned int`, which only works when pointers are 32 bits (phase 8 fixes that).

1. Add a CMake preset, e.g. `windows-x86`, for MSVC 2022 (or clang-cl). Keep the MSVC6 preset for reference
   builds.
2. Drop the matching-only scaffolding from that preset:
   - the CRT and import stub files, linking the real C runtime and Windows import libraries instead;
   - `/NODEFAULTLIB` and the dummy entry, using the game's own `WinMain` in `main.c`.
3. Fix what the modern compiler rejects or warns about. Expect old-style declarations, MSVC6-only keywords and
   the matching tricks (`volatile` temporaries, `#pragma optimize`); `PORTING.md` allows cleaning those up.
4. Turn warnings on (`/W4`); in port-only code, warnings are errors.

**Done when:** `cmake --preset windows-x86 && cmake --build` produces a `legoland.exe` that starts and reaches
`WinMain`, even if it fails right after. **Decision:** MSVC 2022 or clang-cl. clang-cl is closer to the
compilers used for Linux and Android.

## Phase 3: the game's initialized data (done)

**Status:** done. `tools/gen_data.py` embeds the original `.rdata` + `.data` (94 KB) in the generated
`port/port_data.c`; `PortLoadData()` runs first in `WinMain`. It patches 1,370 pointer slots (to port globals,
functions, or the embedded copy for strings and unnamed data) and fills 410 globals. The exe has no relocation
table, so pointers are found heuristically, with text filtered out (string literals, text runs, short strings);
the generator's `--report` lists everything it skipped or left unchanged. `legoland.exe -port-selftest` checks
the result and writes `port-selftest.txt`: 0 problems. It also lists 140 globals the port declares smaller than
the original data around them (an upper bound: much of it is constants the code uses as literals). Those are the
first suspects when something misbehaves in phase 4. Rerun the generator whenever globals are added or retyped.
Different from the plan above: the data is embedded as bytes and copied at startup rather than written as typed
C initializers, because many globals' decomp types are still placeholders; typed initializers can replace it in
phase 8.


The original program starts with about 92 KB of tables, strings and constants already in memory. The port
needs them too.

1. Write `tools/gen_data.py`. It reads the original `.data`/`.rdata` from `external/legoland.exe` and writes
   real C initializers for every global, into a generated `globals_data.c` or into `globals.c` itself:
   - the address annotations (`// GLOBAL: LEGOLAND 0x...`) say where each global's bytes are;
   - the PE relocation table says which 4-byte values are pointers. Those become `&symbol` or `"string"`
     expressions, not raw numbers, so the data stays correct at any load address and later on 64-bit.
2. Data the decomp hasn't named yet (gaps between known globals) gets placeholder globals with a warning, so
   nothing is silently zero.
3. Regenerate when the decomp adds or retypes globals. The script, not hand edits, owns the generated values.

**Done when:** every global has its original value, and pointers point at port symbols.
**Why generate source instead of loading the exe at runtime:** `legoland.exe` may be committed (see
`PORTING.md`), generated initializers have no load-time patching, and they keep working on 64-bit and Android.

## Phase 4: first boot, with the original APIs

Windows 11 still ships DirectDraw, DirectSound and DirectInput as compatibility layers. Running once with the
original API calls gives a baseline that later phases are compared against.

1. Point the game at the data folder: working directory = `installed\`. The code looks in `.\volumes`,
   `.\speech` and `.\FMV` first.
2. Make the CD check (`cdcheck.c`) pass when the data folder is complete. Tag it `[library:filesystem]`.
3. Start with music off (the game's `-nomusic` switch) and movies skipped if needed: DirectMusic and the Indeo
   codec usually aren't on Windows 11.
4. Debug until the title screen shows, then click through the menus.

**Done when:** the title screen and main menu work on Windows 11, natively, with no emulator.
**Reference for later phases (decided 2026-10-08):** no Windows 98 VM or DirectDraw wrapper. This port, running
the original DirectX APIs on Windows 11, is the reference that phases 5+ are compared against (the whole
campaign has been played on it: levels, movies, music, sound, saves, the certificate).

**Status (2026-10-08):** everything above works, well past the title screen. Phase 4 is marked done after
the maintainer's full playthrough.

## Phase 5: miniwin + SDL3 (window, video, input, files)

Replace the Windows-only layers with SDL3 behind `miniwin/`, one subsystem at a time, checking against phase 4's
build after each.

1. Add SDL3 in `3rdparty/` (submodule or `FetchContent`), and create `miniwin/include/miniwin/` and
   `miniwin/src/`.
2. **Window and events** `[library:window]`: the window, message loop and `WndProc` (`wndenv.c`, `screens.c`,
   `draw.c`) on SDL events.
3. **2D video** `[library:video]`: the DirectDraw surfaces in `draw.c`/`render.c` become a 640x480 16-bit memory
   framebuffer, uploaded to an SDL texture once per frame. Scaling and windowed mode come for free.
4. **Input** `[library:input]`: DirectInput (`input.c`) and the cursor on SDL keyboard and mouse.
5. **Timers and threads** `[library:timer]`, `[library:thread]`.
6. **Files** `[library:filesystem]`: SDL paths, `/` separators, case-insensitive lookup. Test it by renaming a
   data folder to different case.
7. **Settings** `[library:config]`: registry calls on iniparser, an `.ini` in the user's folder.

**Done when:** the title screen and menus run with SDL3 only, with no DirectX or registry calls left in those paths
(`grep` the tags to check), and screenshots match phase 4.

## Phase 6: sound, movies and music

1. **Sound effects and speech** `[library:audio]`: DirectSound and ACM (`sound_sfx.c`, `stream.c`,
   `profile_io.c`, `interface.c`) on miniaudio. Its WAV decoder covers the MS and IMA ADPCM speech files.
2. **Movies** `[library:movie]`: a small AVI reader plus `libavcodec` built with only `indeo5`, `indeo3`,
   `adpcm_ms` and `adpcm_ima_wav` (`interface.c`, `challenge.c`). Test on all 40 AVIs: the 14 in `FMV\` and the
   26 animations.
3. **Music** `[library:music]`: GothicKit/dmusic for the `IMusic\` segments and styles, with the user's `gm.dls`
   on Windows. Compare by ear against the original.

**Done when:** the intro movie, speech, sound effects and music all play, compared against the original.
**Decisions:** FFmpeg dependency or a ported Indeo 5 decoder; which free General MIDI bank to use where
`gm.dls` isn't available.

## Phase 7: playable on Windows 11

1. Play through: every park area, every building and ride, the challenges and tutorials, saving and loading,
   and the certificate (printing becomes "save as image" `[library:print]`).
2. Keep a checklist in `docs/` of screens and features, with their status and known differences.
3. Fix crashes and undefined behavior as they show up. Gameplay changes go in `extensions/` (see `PORTING.md`).
4. Package it: a release zip, and a small setup tool that builds the data folder from the user's CD or ISO
   (unpacks `main.z`, copies `volumes\`, `speech\`, `FMV\`).

**Done when:** the whole game can be played start to finish on Windows 11. This is the first release.

## Phase 8: 64-bit and Linux

1. Turn on pointer-truncation warnings and fix every pointer stored in an `int`. Many decompiled signatures pass
   pointers as `unsigned int`, for example `FUN_00420e90(unsigned int mesh, ...)`.
2. Check every struct that is read from a game file: fixed-size fields and explicit little-endian loads, no
   pointers inside.
3. Build x64 Windows and play-test it against the 32-bit build.
4. Build on Linux. It's the cheapest way to find anything Windows-only that's left, plus case-sensitive paths
   and the `char` signedness problems ARM will have.

**Done when:** x64 Windows and Linux builds play the same as the 32-bit one.

## Phase 9: Android

1. An `android-project/` based on SDL3's Android template: Gradle plus the NDK building the same CMake project.
2. **Data:** a first-run screen where the user picks their LEGOLAND folder (or CD/ISO), which is copied into
   app storage.
3. **Touch:** a tap is a left click, with a drag cursor or long-press for right click; the game is mouse-driven.
4. **Screen:** scale 640x480 to the device, letterboxed.
5. **App lifecycle:** pause audio and the game loop when the app goes to the background, and save safely.
6. Audio latency and performance checks on a real device.

**Done when:** the game installs as an APK and plays on an Android phone or tablet.

## Throughout

- **Merge the decomp regularly** (`git merge main` in `~/wt/port`). Better matches and names flow in. Resolve
  conflicts in `[port:rewrite]` functions by keeping the port's version.
- **Small commits, one subsystem each**, every one building cleanly.
- **Never commit game data** (see `PORTING.md`).
- **Compare against the original** whenever behavior could have changed.

## Decisions still open

| Decision | When | Options |
|---|---|---|
| Compiler | phase 2 | MSVC 2022, or clang-cl (closer to the Linux and Android compilers) |
| Data initializers | phase 3 | generated C source (recommended above), or loading `.data` from the exe at runtime |
| Reference setup | phase 4 | Windows 98 VM, or the original under a DirectDraw wrapper |
| Indeo decoding | phase 6 | FFmpeg `libavcodec` subset, or a port of FFmpeg's `indeo5` decoder |
| Music instruments | phase 6 | the user's `gm.dls` on Windows; a free General MIDI bank elsewhere |
