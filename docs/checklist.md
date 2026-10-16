# Windows 11 checklist (phase 7)

What has to work for the first release, and what has been checked. Phase 4's full play-through ran the game on
Windows' own DirectDraw, DirectSound, DirectInput, DirectMusic and Video for Windows; since then the port replaced
all of them (phases 5 and 6: SDL3, miniwin, FluidSynth, FFmpeg's Indeo decoders). Each line is re-checked on the
current build.

Status: **OK** checked and fine, **diff** works with a known difference (see the last section), **bug** broken
(with a note), **?** not checked yet. "Tests" means checked by the port's own tools, without playing.

## Installing and starting

| | Phase 4 | Now | Notes |
|---|---|---|---|
| Release zip unpacked to a new folder, first start shows the setup window | - | ? | `port/port_setup_win.c` |
| Setup from the CD in a drive | - | ? | |
| Setup from a mounted ISO | - | ? | |
| Setup from an ISO file (Choose ISO...) | - | Tests | `test_setup.exe` on the maintainer's ISO: identical to the install |
| Setup from a chosen folder (a copy of the CD; an old install, saves included) | - | Tests | `test_setup.exe`: identical to the install |
| Setup: Cancel, a read-only folder, a full disk give a clear message | - | ? | |
| Second start goes straight to the launcher | - | ? | |
| Starting from a shortcut whose "Start in" is the data folder | OK | ? | |
| A second start while the game runs brings the window to the front | OK | ? | |

## Display and input

| | Phase 4 | Now | Notes |
|---|---|---|---|
| Launcher: full screen at the desktop's resolution | OK | ? | |
| Launcher: full screen changing the resolution (640x480 and larger) | OK | ? | |
| Launcher: in a window, several sizes; resizing the window | OK | ? | |
| "Don't show this again", and Shift to see the launcher again | OK | ? | |
| Alt+Tab away and back, minimize and restore, in each mode | OK | ? | |
| Display scaling above 100% (DPI) | OK | ? | |
| Mouse: cursor, clicks, drags, the wheel in windowed mode | OK | ? | |
| Keyboard shortcuts and cheat codes | OK | ? | |

## Front end

| | Phase 4 | Now | Notes |
|---|---|---|---|
| LEGO Media logo (a click skips it), intro movie | OK | ? | |
| Title screen and main menu | OK | ? | |
| Profiles: new, load, delete | OK | ? | |
| Options: sound, music and speech volumes | OK | ? | |
| Credits, quitting | OK | ? | |

## The game

| | Phase 4 | Now | Notes |
|---|---|---|---|
| The tutorials | OK | ? | |
| Levels 1 to 10, each to its end | OK | ? | |
| Building: paths, rides, shops, scenery, demolishing | OK | ? | |
| Staff: gardeners and mechanics, repairs | OK | ? | |
| Visitors: crowds, ride queues, thoughts | OK | ? | |
| Advisor: animations, speech, reminders | OK | ? | |
| Appraisals | OK | ? | |
| Saving and loading during a level; saves kept between runs | OK | ? | |
| End movies | OK | ? | |
| Certificate printing (a printer, and "Microsoft Print to PDF") | OK | ? | |

## Sound, movies and music

| | Phase 4 | Now | Notes |
|---|---|---|---|
| Sound effects | OK | Tests | `test_dsound.exe` against Windows' DirectSound: identical |
| Speech (MS and IMA ADPCM) | OK | Tests | `test_acm.exe` on all 1266 speech files: identical to Windows' ACM |
| The 14 movies and the 26 advisor animations | OK | Tests | `test_avifile.exe` on all 40: identical to Windows' AVIFile |
| Movies pause in the background (on by default) | - | OK | the maintainer, in game |
| Music: main theme, the four world themes, the transitions | OK | Tests, ? | the notes checked against Windows' DirectMusic; FluidSynth by ear on recordings; in game: ? |
| `-windows-music` (Windows' DirectMusic) | OK | ? | |
| "Music follows the level" (launcher box) | OK | ? | |
| Sound muted while another window has the focus | OK | ? | |

## Robustness

| | Phase 4 | Now | Notes |
|---|---|---|---|
| A long session (an hour or more) without a freeze or crash | OK | ? | crash and freeze reports go next to the exe |
| `-port-selftest` | OK | OK | run with every build |

## Known differences from the original

- The music plays through FluidSynth on Windows' gm.dls instead of Microsoft's synthesizer: the same notes, the
  instruments very close (`tools/music/README.md`). `-windows-music` plays it the original way.
- Movies pause while the game is in the background (`-no-pause-movies` for the original's behaviour).
- The picture is scaled to the screen with sharp pixels; the game itself still draws 640x480.
- The original's own bugs are left as they are: `docs/original-bugs.md`.
