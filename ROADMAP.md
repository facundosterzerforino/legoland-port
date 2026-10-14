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

## Phase 4: first boot, with the original APIs (done)

**Status:** done 2026-10-13, after the maintainer's full playthrough on Windows 11 (all ten game levels, the
tutorials, movies, music, sound, speech, saves, the appraisals, the end movies and the certificate printout).
Bugs found on the way, nearly all data the modern compiler lays out differently from MSVC6: globals declared
smaller than the original (split tables, a name buffer written past its end), locals that MSVC6 happened to
place next to each other (the sensory coaster's z-buffer fill), and one wrong string index that emptied every
advisor reminder (`PercentSFormat`). The fixes that also apply to the decomp were made there and merged.
Port additions made during this phase, all off by default or invisible: the debug console's `variety` and
appraisal targets, the "music follows the level" launcher option, the mouse wheel in windowed mode, and the
certificate printing to the default printer.

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


## Phase 5: miniwin + SDL3 (window, video, input, files) (done)

**Status:** done 2026-10-13, every step confirmed in game by the maintainer. The exe no longer imports ddraw.dll or
dinput.dll.
- Step 1 is done: SDL3 3.4.18 is fetched at configure time and linked statically into the port build
  (`-port-selftest` reports the version it linked).
- Steps 2 and 3 are done (confirmed in game by the maintainer, 2026-10-13): the window and the picture are SDL3's,
  and DirectDraw is gone (no `ddraw.dll` import). `miniwin/src/ddraw.c` is DirectDraw on memory surfaces: the game's calls reach it through the same COM
  interfaces, so the decompiled drawing code is unchanged. `tools/test_miniwin` compares it with Windows' own
  DirectDraw pixel by pixel (copies, colour keys, stretches with and without clip lists, fills, GDI text, error
  codes): identical. DirectDraw's stretch rounding was measured for every pair of sizes up to 640 and is
  reproduced exactly. `miniwin/src/window.c` is the SDL3 window, renderer and events; the game takes its
  windowed code path in all three display modes.
- Step 4 is done (confirmed in game): the keyboard and mouse come from SDL3. `miniwin/src/dinput.c` is
  DirectInput's keyboard and mouse devices on SDL (the keyboard as DirectInput's 256 bytes by DIK code, through a
  table generated from SDL's own Windows scancode table); the windowed mouse code, the focus checks and Caps Lock
  use SDL too. No `dinput.dll` import any more.
- Step 5 is done (confirmed in game): `miniwin/redirect.h` is included first into every game source and sends
  GetTickCount, QueryPerformanceCounter/Frequency and Sleep to SDL3 (`miniwin/src/timer.c`).
  GetTickCount keeps Windows' 15.625 ms steps: the frame limiter waits 28 ms on it, so frames last 31.25 ms, and a
  precise clock would make the game run about 10% faster. Threads stay as they are until phase 6: the only game
  threads are the sound and music ones (and the debug console's).
- Step 6 is done (confirmed in game): through `miniwin/redirect.h` too, every file the game opens
  (fopen, _open, CreateFileA, remove, _mkdir, _findfirst, GetFileAttributesA, SetCurrentDirectoryA) goes through
  `MiniwinPath` (`miniwin/src/files.c`): backslashes become '/', and a path that isn't on disk as written is matched
  part by part without regard to case. On Windows nothing changes (its folders ignore case);
  `tools/test_miniwin/test_paths.exe` checks the matching on a case-sensitive folder. The CD check already passes
  whenever the data folder is complete (`cdcheck.c`). Movies (AVIFile) come with phase 6.
- Step 7, settings: nothing to do. The game itself never touches the registry; its settings and profiles are files
  in the data folder, and the launcher's choices are in `legoland-port.ini`.
- Still Windows-only underneath: GDI text on the surfaces (GetDC), DirectSound and the movies (phase 6), and the
  launcher window (a Windows dialog; other platforms will need their own).

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

**Status (2026-10-13):** started.
- Step 1, sound effects, speech and movie sound, waiting for the maintainer's test: `miniwin/src/dsound.c` is
  DirectSound on SDL3 audio (the game's buffers mixed on SDL's audio thread with DirectSound's volume, pan,
  frequency, looping and cursors; muted, still moving, while the window doesn't have the focus, as DirectSound does). `tools/test_miniwin/test_dsound.exe` compares it with Windows' DirectSound:
  identical. DirectMusic still renders through a real Windows DirectSound of its own (`port/port_audio.c`) until
  the music step.
- ADPCM decoding, waiting for the maintainer's test: `miniwin/src/acm.c` replaces Windows' ACM for the speech
  files and the movies' sound (MS ADPCM, IMA ADPCM and PCM, to 16- or 8-bit PCM). `tools/test_miniwin/test_acm.exe`
  compares it with Windows' ACM, sizes, header fields and decoded bytes, on made-up blocks and, given the data
  folder, on all 1266 speech files and the 9 ADPCM movie soundtracks: identical. The port no longer links msacm32.
  (Confirmed in game, with the movies paused in the background: `extensions/ext_movie.h`, on by default.)
- Step 2, movies, waiting for the maintainer's test: the frames were already decoded by the port (FFmpeg's Indeo 3
  and Indeo 5 decoders, `port/port_movie.c`); now the files are read by `miniwin/src/avifile.c` instead of Video for
  Windows' AVIFile. `tools/test_miniwin/test_avifile.exe` compares the two on all 40 AVIs (the 14 movies and the 26
  advisor animations): file and stream info, formats, key frames, every video frame and the sound in the game's own
  read sizes, about 25 million checks: identical. The port no longer links vfw32. (Confirmed in game.)
- Step 3, music, part 1, waiting for the maintainer's test: Windows' DirectMusic now renders into miniwin's
  DirectSound like the game's other sounds (one mixer, one volume and focus behaviour); `port/port_audio.c`, the
  separate Windows DirectSound, is gone. `tools/test_miniwin/test_dmusic.exe` sets DirectMusic up the way the game
  does on miniwin's DirectSound and checks that the music reaches it. Part 2, the portable replacement: GothicKit
  dmusic, checked note by note against a recording of what Windows' DirectMusic plays.
- Step 3, part 2, waiting for the maintainer's listening test: miniwin's DirectMusic (`miniwin/src/dmusic.c`, the
  objects the game's music thread uses) on a style engine (`miniwin/src/dmengine.c`) built from what the
  recordings showed (`tools/music/README.md`), with GothicKit dmusic (`port/dmusic`) for the files and the
  synthesizer. After the first listening test (the drums played the piano, 16 dB too quiet): the drum kits and
  variation banks fixed in dmusic, DLS's velocity curve, and the level measured against recordings of Windows'
  DirectMusic in the game's setup (`tools/test_miniwin/capture_dmusic.exe`), now within 0.6 dB on every theme.
  After the second (bad after the first bars): compared instrument by instrument with Windows' synthesizer
  (`tools/test_miniwin/instruments_dmusic.exe`), notes decayed 2-4 times too slowly (gm.dls's key-scaled decay was
  lost in the conversion) and the synthesizer's default reverb was missing; both fixed, the envelopes now within
  3 dB on average over the 53 instruments. After the third (the notes right, the instruments still a little
  different): FluidSynth (LGPL), which reads DLS by Microsoft's rules itself, is now the synthesizer, built into the
  exe from a pinned release; 2.7 dB on average, envelope correlation 0.97-1.0, and the maintainer, comparing the
  recordings by ear: it sounds right.
  Its output passes the same checks as Windows' DirectMusic: every pitch, every bar, the same spread
  of random picks. Off for now: `-portable-music` turns it on, to compare by ear.

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
