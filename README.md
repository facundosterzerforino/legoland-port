# legoland — the LEGOLAND port

This repository turns the LEGOLAND decompilation into a **game that runs natively on Windows 11, Linux, Android and
in web browsers**. You supply your own copy of the 1999 game; the port uses its files and does not ship any game
data.

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
| Runs on Windows 11 | **yes**: played start to finish (phase 4); on SDL3 with its own sound, movies and music (phases 5-6); released since phase 7; 32- and 64-bit |
| Runs on Linux | **builds and passes its checks** (phase 8): `-port-selftest` with the Windows build's results, the setup, the music, the text; not play-tested yet |
| Runs on Android | **builds and passes its checks** (phase 9): `-port-selftest` on an emulator with the Windows build's results; not play-tested on a device yet |
| Runs in a browser | **builds and passes its checks** (phase 10): `-port-selftest` in Chromium with the 32-bit Windows build's results; not play-tested yet |
| Hand-written assembly | **done**: all 44 inline-asm functions are plain C (phase 1 of `ROADMAP.md`) |

## Releases

Each release is the program only; the first start installs the game from your LEGOLAND CD (or its image: an .iso or
.bin), in any of its languages. `tools/release/make_release.py all` makes them in `dist/`, each with a README:

| Platform | File | Runs on |
|---|---|---|
| Windows | `LEGOLAND-port-windows-<version>.zip` | Windows 10 and 11. Unzip, start `LEGOLAND-port.exe`. |
| Linux | `LEGOLAND-port-linux-x86_64-<version>.tar.gz` | 64-bit Linux with glibc 2.31 or later (Ubuntu 20.04, Debian 11, Fedora 32 and later), X11 or Wayland. Unpack, start `legoland`. |
| Android | `LEGOLAND-port-android-<version>.apk` | Android 7.0 and later, ARM or x86 64-bit. Install the APK, choose the CD image on the first start. |
| Web | `LEGOLAND-port-web-<version>.zip` | A website: put its files on any web server (HTTPS) and open `index.html`; recent Chrome, Edge, Firefox, Safari 17+. The game's files stay in the browser's storage for the site. |

Off Windows the music needs General MIDI instruments, which Windows has and the others don't: copy `gm.dls` from a
Windows PC (`C:\Windows\System32\drivers\gm.dls`) into the game's folder for the original sound, or use a General MIDI
SoundFont (an `.sf2` file; on Linux, your distribution's `fluid-soundfont-gm`). The web page and the Android app take
it as a file in the game's folder too.

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
   input, choosing the data folder), then the web. All four build and pass their checks; Linux, Android and the web
   wait for their first play tests.

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

The first configure downloads SDL3 and FluidSynth (pinned releases, checked by their SHA-256) and builds them with
the same toolchain; to build offline, unpack those releases somewhere and add `-DFETCHCONTENT_SOURCE_DIR_SDL3=<folder>`
and `-DFETCHCONTENT_SOURCE_DIR_FLUIDSYNTH=<folder>`.
After the toolchain file changes, reconfigure with `cmake --fresh --preset clang-cl-x86`.
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

### The other platforms

All from Linux (WSL works):

```sh
cmake --preset clang-cl-x64 && cmake --build build-clang-x64        # 64-bit Windows
cmake --preset linux && cmake --build build-linux                   # Linux, with the host's clang
cmake --preset linux-portable && cmake --build build-linux-portable # the Linux release: glibc 2.31+, Zig's clang
tools/android/build_apk.sh                                          # Android: build-android/LEGOLAND-port.apk
cmake --preset web && cmake --build build-web                       # the web: legoland.html/.js/.wasm (Emscripten)
cmake --preset web-node && cmake --build build-web-node             # the same for Node.js (-port-selftest)
```

- **Linux** needs SDL3's build dependencies for its video and sound backends (Debian/Ubuntu: `libx11-dev
  libxext-dev libxrandr-dev libxcursor-dev libxi-dev libxss-dev libxtst-dev libwayland-dev libxkbcommon-dev
  wayland-protocols libasound2-dev libpulse-dev libpipewire-0.3-dev libdecor-0-dev`). Off Windows, `<windows.h>` and
  the DirectX headers are `miniwin/include/win32/`, generated from the SDK (`tools/linux/gen_win32.py`), and the
  Win32 functions the game calls are `miniwin/src/posix/`. The release build uses [Zig](https://ziglang.org)'s clang
  (`~/zig/zig`, or `ZIG`) to link against an old glibc.
- **Android** needs the Android SDK in `~/android-sdk` (`ANDROID_HOME`): build-tools 35.0.0, platform 35, an NDK
  (r27), and a JDK 17. No Gradle: `tools/android/build_apk.sh` compiles SDL's Java side and packs the APK, signed
  with a key of the machine's (`~/.android/legoland-port.keystore`, made on the first build). `tools/android/selftest.sh`
  runs `-port-selftest` on an emulator.
- **The web** needs the [Emscripten SDK](https://emscripten.org) in `~/emsdk` (`EMSDK`). `tools/web/pagetest.py`
  checks the page in headless Chromium and runs `-port-selftest` there. See `port/web/README.md`.

Each platform's `-port-selftest` reads the game's resources, files and meshes and writes dumps that must be identical
to the Windows build's (on Android in the app's folder, on the web in the page's storage).

## You need your own game

The port plays the original game's data: the `.res` volumes, speech, movies and music from the LEGOLAND CD. No
proprietary files are committed to this repository (the original `legoland.exe` the build generates the game's
startup data from is the one exception, and the port doesn't need it at runtime).

The data folder is the folder the game starts in, or the exe's own folder. When neither has the data, the first
start shows a setup window: it finds the LEGOLAND CD in a CD drive (a mounted ISO image counts: double-click it), or
takes an ISO image, a folder with a copy of the CD or an installed LEGOLAND, and copies the files next to the exe
(about 400 MB: it unpacks the CD's `main.z` and copies `FMV`, `speech` and `volumes`; from an install, everything,
saves included). See `port/port_setup.h`.

On Linux the setup finds an image next to the program or a mounted CD by itself, or asks with the desktop's file
dialog (`port/port_setup_sdl.c`); on Android the data folder is the app's own (`Android/data/<package>/files`,
reachable over USB), filled from a CD image chosen with Android's file picker; on the web it's the browser's private
storage for the site, filled from the image the page asks for.

## Resolution and window mode

A small launcher opens before the game with three choices:

- **Full screen (keep the desktop's resolution)**, the default: the picture fills the screen, 4:3 and centred, and
  the display mode is never changed (Alt+Tab is instant).
- **Full screen (change the resolution)**: a 4:3 display mode (640x480 is the original; 800x600, 1024x768,
  1280x960 and more, as far as the display allows).
- **In a window** of a chosen 4:3 size; the window can be resized.

The game still draws its 640x480, 16-bit picture and the port scales it up with sharp pixels (1280x960 and
1920x1440 are exact 2x and 3x) and converts it to the screen's colours, with black bars on screens that aren't 4:3.
Nothing about the game changes.

The choice is saved next to the exe (`LEGOLAND-port.ini`). The launcher is a Windows window: on Linux, Android
and the web the game starts full screen at the screen's resolution, and the command line (or the .ini) chooses
otherwise. "Don't show this again" skips the launcher; hold
Shift while starting the game to see it again. Command line: `-fullscreen` (desktop resolution), `-res 1024x768`
(full screen in that mode), `-windowed` (or the original `WINDEBUG`) choose without the launcher, `-launcher`
always shows it.

The window and the picture come from SDL3, through the port's platform layer (`miniwin/`, see `ROADMAP.md`
phase 5); the game no longer uses DirectDraw.

## Optional features

These are new in the port and off unless you turn them on (`extensions/`):

- **Music follows the level** (a box in the launcher, or `-level-music` / `-no-level-music`): the game has
  five music themes, but the original only plays the main one; the other four were only reachable with cheat
  codes. With this on, each level plays the theme that fits its story (Western, Castle, Egypt or Inca; the
  tutorials and some levels keep the main theme), and the menus go back to the main theme. The table is in
  `extensions/ext_music.c`.
- **Movies pause in the background** (on by default; `-no-pause-movies` turns it off): the original keeps a
  movie running while the game is in the background (Alt+Tab, a click elsewhere), so when you come back it jumps
  ahead and skips what it "played" meanwhile. In the port the movie stops while you are away and goes on where it
  was, picture and sound together. It also fixes the original's catch-up after a longer stall, which
  left the rest of most movies silent or out of step.
- **Portable music** (on by default; `-windows-music` uses Windows' DirectMusic instead): the music plays through
  the port's own DirectMusic
  (GothicKit dmusic and a style engine checked against Windows', with FluidSynth playing Windows' gm.dls) instead of
  Windows'. The notes are the same, and the instruments sound very close (compared one by one with Windows'
  synthesizer: `tools/music/README.md`).
- **Debug console** (`-console`): see `docs/debug-console.md`.
- The LEGO Media logo at startup can be skipped with a click (on by default; `-no-skip-logo` turns it off).

## More documentation

- `ROADMAP.md`: the phase-by-phase plan, from finishing the assembly replacements to Android.
- `PORTING.md`: the porting rules (layout, library substitutions, portable C, order of work).
- `CLAUDE.md`: working rules for AI-assisted sessions (port rules first, then the matching rules from `main`).
- `DECOMPILING.md`, `HEADERS.md`, `docs/decomp-tips.md`: how the matching decompilation works.
- `docs/title-screen-port.md`: an earlier scoping study of the boot-to-title-screen path. Its match numbers
  date from when the decompilation was at 52.71%; most functions it lists as missing are done now.

## Third-party code

Built into the port, each under its own license: [SDL3](https://www.libsdl.org/) (zlib), FFmpeg's Indeo and Cinepak
decoders (`port/ffmpeg/`, LGPL-2.1+), [GothicKit dmusic](https://github.com/GothicKit/dmusic) with TinySoundFont
(`port/dmusic/`, MIT), [FluidSynth](https://www.fluidsynth.org/) (LGPL-2.1+, downloaded and built from its pinned
release), zlib's blast and puff (`port/blast/`, `port/puff/`, zlib); off Windows
[stb_truetype](https://github.com/nothings/stb) (`port/stb/`, public domain or MIT), and on the web
[coi-serviceworker](https://github.com/gzuidhof/coi-serviceworker) (`port/web/`, MIT). The LGPL parts can be replaced:
this repository has everything needed to rebuild the program with another version of them.
