# LEGOLAND, portable

[Play in your browser](https://facundosterzerforino.github.io/legoland-port/) | [Releases](https://github.com/facundosterzerforino/legoland-port/releases/latest) | [Decompilation](https://github.com/facundosterzerforino/legoland) | [Porting rules](PORTING.md) | [Roadmap](ROADMAP.md)

This project is a portable version of **LEGOLAND** (LEGO Media, 1999), built on the
[matching decompilation](https://github.com/facundosterzerforino/legoland) of its `legoland.exe`. Its goal is to make the
game run natively on today's systems (Windows 10/11, Linux, Android and web browsers) while keeping the original
game's experience as faithful as possible.

The decompilation recreates the game as C that the original Microsoft Visual C++ 6.0 compiler turns into
byte-identical code, checked function by function with [`reccmp`](https://github.com/isledecomp/reccmp); its `main` is
the source of truth for what the game does. This repository adds what it takes to run it: a real program entry, the
game's startup data, plain-C replacements for its hand-written assembly, and the Windows APIs it calls reimplemented on
[SDL3](https://www.libsdl.org/).

Please note: the port is about platform independence, not about changing the game. Optional changes live in
[`extensions/`](extensions), each with a switch to turn it off. The approach follows
[isle-portable](https://github.com/isledecomp/isle-portable), which did the same for LEGO Island.

## Status

| Platform | Status |
| - | - |
| Windows 10/11 (x86, x64) | Played from start to finish; released |
| Linux (x64) | Builds and passes its self-test with the Windows build's results; play testing in progress |
| Android (arm64, x64) | Builds and passes its self-test on an emulator; play testing in progress |
| [Web](https://facundosterzerforino.github.io/legoland-port/) | Live; passes its self-test in Chromium; play testing in progress |

The decompilation is at 97% progress (99.2% average match). All 44 functions the original wrote in assembly have
plain-C versions here, checked against the original in a CPU emulator where it mattered (`tools/emu/`).

## Usage

**An existing copy of LEGOLAND (the CD, or its image: an `.iso` or `.bin`) is required.** No game data is included:
the first start installs the game from your CD, in any of its languages (English, German, Spanish, French, Italian,
Dutch, Danish, Swedish, Norwegian, Czech...).

| Platform | Download | How |
| - | - | - |
| Web | [Play in your browser](https://facundosterzerforino.github.io/legoland-port/) | Choose your CD image; it's installed in the browser's storage for the site, nothing is uploaded. Recent Chrome, Edge, Firefox, Safari 17+. Saved games can be exported and imported. |
| Windows | [`LEGOLAND-port-windows-<version>.zip`](https://github.com/facundosterzerforino/legoland-port/releases/latest) | Unzip and start `LEGOLAND-port.exe`. A launcher offers full screen or a window. |
| Linux | [`LEGOLAND-port-linux-x86_64-<version>.tar.gz`](https://github.com/facundosterzerforino/legoland-port/releases/latest) | glibc 2.31 or later (Ubuntu 20.04, Debian 11, Fedora 32...), X11 or Wayland. Unpack and start `legoland` (`-windowed` for a window). |
| Android | [`LEGOLAND-port-android-<version>.apk`](https://github.com/facundosterzerforino/legoland-port/releases/latest) | Android 7.0 or later. Install the APK, then choose the CD image on the first start. Touch is the mouse; two fingers the right button; three fingers show the keyboard. |

On Windows the setup finds the CD in a drive (a mounted ISO counts), or takes an ISO, a folder with a copy of the CD or
an installed LEGOLAND, and copies about 400 MB next to the program. The music plays through the port's own DirectMusic;
off Windows its instruments are [GeneralUser GS](https://www.schristiancollins.com/generaluser) (free, included), and
Windows' own `gm.dls` is used instead when you copy it into the game's folder (the web page has a button for it).

The game still draws its 640x480 picture; the port scales it up with sharp pixels, 4:3 with black bars, in full
screen or a window (`-fullscreen`, `-res 1024x768`, `-windowed`, or the launcher on Windows).

### Optional features (`extensions/`)

| Feature | Default | Switch |
| - | - | - |
| **Music follows the level**: each level plays the theme of its story (Western, Castle, Egypt, Inca); the original only ever played the main theme, the others were cheat codes | on | launcher box, web page checkbox, `-no-level-music` |
| **Movies pause in the background** instead of running on and skipping ahead | on | `-no-pause-movies` |
| **Portable music**: the port's DirectMusic (GothicKit dmusic + FluidSynth) instead of Windows' | on | `-windows-music` |
| **Skip the logo movie** with a click | on | `-no-skip-logo` |
| **Action log**: what you do, in `legoland-port-trace.txt`, for bug reports | on | `-no-action-log` |
| **Debug console** for testing ([commands](docs/debug-console.md)) | off | `-console` |

## Library substitutions

To run anywhere, every Windows-only library is replaced. Each changed call in the decompiled code is tagged with its
subsystem (`// [library:video]` and so on), so the "Remarks" link lists every place it touched.

| Library/subsystem | Substitution | Status | |
| - | - | - | - |
| Window, events | [SDL3](https://www.libsdl.org/) | ✅ | [Remarks](https://github.com/search?q=repo%3Afacundosterzerforino%2Flegoland-port+%22%5Blibrary%3Awindow%5D%22&type=code) |
| DirectDraw (2D video) | `miniwin` surfaces in memory, shown through SDL3 | ✅ | [Remarks](https://github.com/search?q=repo%3Afacundosterzerforino%2Flegoland-port+%22%5Blibrary%3Avideo%5D%22&type=code) |
| Software 3D renderer (hand-written assembly) | Plain C | ✅ | [Remarks](https://github.com/search?q=repo%3Afacundosterzerforino%2Flegoland-port+%22%5Blibrary%3Aasm%5D%22&type=code) |
| DirectInput (keyboard, mouse) | [SDL3](https://www.libsdl.org/); touch on Android | ✅ | [Remarks](https://github.com/search?q=repo%3Afacundosterzerforino%2Flegoland-port+%22%5Blibrary%3Ainput%5D%22&type=code) |
| DirectSound, ACM (sound effects) | [SDL3](https://www.libsdl.org/) audio, `miniwin` ADPCM decoding | ✅ | [Remarks](https://github.com/search?q=repo%3Afacundosterzerforino%2Flegoland-port+%22%5Blibrary%3Aaudio%5D%22&type=code) |
| DirectMusic (segments and styles) | [GothicKit dmusic](https://github.com/GothicKit/dmusic) + a style engine, [FluidSynth](https://www.fluidsynth.org/) | ✅ | [Remarks](https://github.com/search?q=repo%3Afacundosterzerforino%2Flegoland-port+%22%5Blibrary%3Amusic%5D%22&type=code) |
| Video for Windows (Indeo 5/3.2, Cinepak movies) | FFmpeg's decoders, our own AVI reader | ✅ | [Remarks](https://github.com/search?q=repo%3Afacundosterzerforino%2Flegoland-port+%22%5Blibrary%3Amovie%5D%22&type=code) |
| Files, folders, CD checks | `miniwin`: case-insensitive paths, the installed data folder counts as the CD | ✅ | [Remarks](https://github.com/search?q=repo%3Afacundosterzerforino%2Flegoland-port+%22%5Blibrary%3Afilesystem%5D%22&type=code) |
| Registry (settings) | An `.ini` file | ✅ | [Remarks](https://github.com/search?q=repo%3Afacundosterzerforino%2Flegoland-port+%22%5Blibrary%3Aconfig%5D%22&type=code) |
| Timers | [SDL3](https://www.libsdl.org/) | ✅ | [Remarks](https://github.com/search?q=repo%3Afacundosterzerforino%2Flegoland-port+%22%5Blibrary%3Atimer%5D%22&type=code) |
| Threads, events, critical sections | `miniwin` (Win32 on Windows, pthreads elsewhere) | ✅ | [Remarks](https://github.com/search?q=repo%3Afacundosterzerforino%2Flegoland-port+%22%5Blibrary%3Athread%5D%22&type=code) |
| GDI text and bitmaps | `miniwin`; off Windows the game's TrueType font through [stb_truetype](https://github.com/nothings/stb) | ✅ | [Remarks](https://github.com/search?q=repo%3Afacundosterzerforino%2Flegoland-port+%22%5Blibrary%3Agdi%5D%22&type=code) |
| Certificate printing | The default printer on Windows; off Windows, saved as a picture | ✅ | [Remarks](https://github.com/search?q=repo%3Afacundosterzerforino%2Flegoland-port+%22%5Blibrary%3Aprint%5D%22&type=code) |

## Building

The project uses [CMake](https://cmake.org/) presets and cross-compiles everything from Linux (WSL works):

```sh
cmake --preset clang-cl-x86 && cmake --build build-clang-x86        # Windows, 32-bit (clang-cl + xwin's SDK)
cmake --preset clang-cl-x64 && cmake --build build-clang-x64        # Windows, 64-bit
cmake --preset linux && cmake --build build-linux                   # Linux, with the host's clang
cmake --preset linux-portable && cmake --build build-linux-portable # the Linux release (glibc 2.31+, Zig's clang)
tools/android/build_apk.sh                                          # Android (SDK + NDK, no Gradle)
cmake --preset web && cmake --build build-web                       # the web (Emscripten)
tools/release/make_release.py all                                   # the release packages, in dist/
tools/release/github_release.py                                     # dist/ as a GitHub Release (needs GITHUB_TOKEN)
```

<details>
<summary>Requirements and details</summary>

- **Windows builds** need `clang`, `lld`, `llvm`, `cmake`, `ninja` and the Windows SDK and CRT from
  [xwin](https://github.com/Jake-Shadle/xwin): `xwin --accept-license --arch x86,x86_64 splat --output ~/xwin`
  (`XWIN_DIR` to use another folder). The first configure downloads SDL3 and FluidSynth (pinned, checked by SHA-256).
- **Linux** needs SDL3's dependencies for its video and sound backends (Debian/Ubuntu: `libx11-dev libxext-dev
  libxrandr-dev libxcursor-dev libxi-dev libxss-dev libxtst-dev libwayland-dev libxkbcommon-dev wayland-protocols
  libasound2-dev libpulse-dev libpipewire-0.3-dev libdecor-0-dev`); the release uses [Zig](https://ziglang.org)
  (`~/zig/zig`) to link against an old glibc. Off Windows, `<windows.h>` and the DirectX headers are generated from the
  SDK (`tools/linux/gen_win32.py`).
- **Android** needs the Android SDK in `~/android-sdk` (build-tools 35, platform 35, NDK r27) and JDK 17.
- **The web** needs the [Emscripten SDK](https://emscripten.org) in `~/emsdk`; see `port/web/README.md`.
- The build turns the game's initialized data into C (`tools/gen_data.py`) from the original `legoland.exe`, the one
  file of the game in this repository; the program doesn't need it at runtime.
- **Checks:** every build has `-port-selftest`, which reads the game's resources and writes dumps that must be
  identical to the Windows build's (`tools/android/selftest.sh`, `tools/web/pagetest.py` run it on Android and in
  Chromium); `tools/test_miniwin/` compares `miniwin` with Windows' own DirectDraw, DirectSound and GDI; `tools/emu/`
  runs original and port functions side by side in a CPU emulator.
- **The matching build** (MSVC6 through [wibo](https://github.com/decompals/wibo)) still works:
  `uv run setup.py && cmake --preset msvc6 && cmake --build build && ./tools/verify`.

</details>

## Contributing

Bug reports are welcome in the [issues](https://github.com/facundosterzerforino/legoland-port/issues); the game's log
(`legoland-port-trace.txt`, or "Save the game's log" on the web page) helps a lot. The decompiled code's fixes and
names go to the [decompilation](https://github.com/facundosterzerforino/legoland) first and are merged in here. The
working rules are in [`PORTING.md`](PORTING.md) and [`CLAUDE.md`](CLAUDE.md); the plan is in [`ROADMAP.md`](ROADMAP.md).

## Third-party code

Built into the port, each under its own license: [SDL3](https://www.libsdl.org/) (zlib), FFmpeg's Indeo and Cinepak
decoders (`port/ffmpeg/`, LGPL-2.1+), [GothicKit dmusic](https://github.com/GothicKit/dmusic) with TinySoundFont
(`port/dmusic/`, MIT), [FluidSynth](https://www.fluidsynth.org/) (LGPL-2.1+, built from its pinned release), zlib's
blast and puff (`port/blast/`, `port/puff/`, zlib); off Windows [stb_truetype](https://github.com/nothings/stb)
(`port/stb/`, public domain or MIT); on the web [coi-serviceworker](https://github.com/gzuidhof/coi-serviceworker)
(`port/web/`, MIT). The LGPL parts can be replaced: this repository has everything needed to rebuild the program with
another version of them.

LEGOLAND is a trademark of the LEGO Group. This is a fan project, not made or endorsed by the LEGO Group; you need
your own copy of the game.
