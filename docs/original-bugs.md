# Bugs in the original game (left as they are)

The port plays like the original, so these stay unfixed by default. Each one is behaviour of the original
`legoland.exe`, checked against its machine code, not a decomp or port mistake. Fixes that change what the
player sees belong in `extensions/`, as an option that's off by default (see PORTING.md). Found on 2026-10-05
with clang's warnings and static analyzer (see "How these were found" at the end).

## Fixed in the port (nothing visible changes)

### The info popup leaks two device contexts each time it measures its text
`FUN_00471840` and `FUN_004717a0` (`popupinfo.c`, 0x471840 and 0x4717a0) measure the popup's name and info text
with `CreateCompatibleDC(NULL)` and `DrawTextA`, and never call `DeleteDC` (checked in the machine code: the only
calls are CreateCompatibleDC, SelectFont and DrawTextA). `DrawPopUpInfo` calls both whenever the popup's size may
change, so every ride, shop or visitor popup leaks two GDI objects. Windows refuses new GDI objects at 10000 per
process; after a long session `CreateCompatibleDC` fails everywhere and every measured text box comes out empty:
the advisor's and the interval screen's speech bubbles shrank to a thin strip (`BubbleHelp`, seen 2026-10-07).
The port deletes the DC (tagged `[library:gdi]`); the watchdog traces the GDI object count every 5 minutes.

## Confirmed and visible

### Castle ride visitors always wear the "chest girly2" texture
`GetChestTextureNameOfBloke` (`render3d.c`, 0x4431a0, 100% match). It reads the visitor's chest texture name
from `visitor\altman.txt` / `altwoman.txt`, compares it with "chest girly1" with `_stricmp`, throws the
result away and always returns "chest girly2". The only caller fills the castle ride's visitor model
(`castle.c`, `strcpy(DAT_004b5988.chest, ...)`). Probably meant: "if it's chest girly1 use chest girly2,
otherwise the name". Possible extension: return the looked-up name, with that substitution.

### Appraisal: "Happy" and "Well Fed" share one threshold
`RunAppraisal` (`challenge.c`) compares both the happy-visitor count (row "Happy") and the hunger total (row
"Well Fed") with the same global, `0x666050`, and shows the same bar limits (`0x666050`/`0x666054`). The
scripts' `REPORT HAPPPY_VIS` / `HUNGRY_VIS` settings can't be set apart. None of the 15 level scripts uses
either, so it doesn't affect the shipped levels.

### Driving school blue cars are missing
Every level load logs `Failed to load (.\graphics\small\DS_carblu1.BMP)` up to `DS_carblu16`. The names are
listed in `Legoland.res` (with the red and yellow ones), but no blue car image ships in any volume or on the
CD; the cars that are drawn come from `ds_car&m.lls`. Nothing to fix unless the images turn up.

## Unset values the game's data never reaches

These read a variable that isn't set on some path. With the shipped data and scripts the path isn't taken,
so nothing happens; a mod or broken data could reach it. A fix is a defined default (and, for scripts, an
error message).

| Where | What is unset | When |
|---|---|---|
| `Get3DDataListString` (`render3d.c`, 0x4428f0) | `values`, the chest list | the file's face section has no strings (the original then uses the file pointer itself); altman/altwoman always have strings |
| `ScriptCmdSelectMode` (`game_util.c`, 0x47a3d0) | `index`, the mode | a script writes `SELECTMODE` with no argument |
| `FUN_00411680` (`log_flume.c`, log boat movement) | `dir`; the boat position `r` | two consecutive track pieces on the same square; a piece whose mode/submode has no shape |
| `RenderLogFlumeTunnel` (0x40f450), `RenderLogFlumeCsaw` (`log_flume.c`) | `frame` passed to `LLSSetFrame` | the ride's layer sprite has no animation |
| `FUN_00465850` (`draw.c`, movie frame blit) | `next` | a 0-pixel-high movie frame (the port plays movies through `port_movie.c` anyway) |
| `CoptersUpdate` (`copters.c`) | the queue table `spr` | a queue index outside 0-4 |

## Missing return values nobody reads

These functions end without a `return` on some path. Checked on 2026-10-05: in the original every `ret` follows a
call, so they return whatever that call left in `eax`, as the decomp does; and every caller (or callback slot)
ignores the value. A cleanup would add the return.

| Function | Match | Who ignores the value |
|---|---|---|
| `FUN_0041eaf0`, `FUN_0041d1d0`, `FUN_00426750`, `FUN_0042a640` (`castle.c`) | 100% | `ForEachRingNode` (void visitor) and plain statement calls |
| `Catapult_AddNode`, `InitGameInterface`, `FUN_00455a50` (`text.c`) | 100% | no caller uses it |
| `UnloadPopUpSprites` | 100% | `UnLoad_PopUpInfo`, called as a statement |
| `AcquireWaterWorksSfx` | 100% | `WaterWorksEntranceLoad`, a `cb_a4` "load resources" callback; that slot is never called with its result used (it isn't the save-load hook, see below) |
| `RenderLogFlumeCorner`, `RenderLogFlumeTrack` (`log_flume.c`) | 53% / 100% effective | the 0xb0 render callback, called through a void function pointer (`print_sprite.c`) |
| `FUN_0040d6f0` (`log_flume.c`) | 97.7% (scheduling only) | the 0x90 callback, result unused (`map_object.c`) |
| `AddBasicObject` | 99.0% (reccmp shows the constant 0x800000 as `EditCursor+5184`) | the 0x98 add-object callbacks, result unused (`gamemap.c`) |
| `FUN_00415a90` (`spider_ride.c`) | 96.2% (two stores swapped) | `AddSpiderNode`, called as a statement |

Checking the five that don't match 100% found one real decomp mistake, fixed: `RenderLogFlumeCorner` (log flume
curves) passed a track entry as `PrintSprite`'s clip argument in two of four branches; the original passes the
caller's clip (`arg`) or 0.

The save-load hook is different: `LoadGame` fails if a ride's `load_hook` (ride +0xb8) returns 0. All 15
functions the original installs there (`Catapult_Load`, `Copters_Load`, `LoadGoldWash`, `LoadJoust`,
`LogFlumeEntrance_Load`, `LoadSafariRide`, `LoadSpider`, `LoadTempleSlide`, `Load_WaterBlock`,
`Load_ElephantF`, `CastleObj_Load`, `LoadJailCells`, `SpaceTower_Load`, `LoadSBarrel`, `LoadZoomer`) return 1 or
0 explicitly on every path.

The ones whose value *is* used were fixed in the decomp: `AddRepairOrder` / `AddRepairOrderForObject`,
`FUN_004723f0` (the info popup's Delete button), `CheckHostSystemGPU`.

## Analyzer findings not reviewed one by one

Functions that match the original 100%, where clang's analyzer reports a value that may be unset. Matching
100% means the original does exactly the same; these were not traced to see whether the game can reach them.

- `log_flume.c`: `RenderLogFlumeEntrance` (`ok`, when the entrance has no flume entry), `LogFlumeTrackCalcCursor` and
  `LogFlumeTrackAddObject` (`key`), `FUN_0040cca0` (array index `idx`), `FUN_0040ce20`, `FUN_0040d420`
- `sound_music.c`: `KillAllSamplesFromSource`, `UnSourceAndFadeAllSamplesFromSource` (`matched`, for a
  sample source type outside the switch)
- `render3d.c`: `FUN_00442580` (`a`, `b`)
- `man3d.c`: `Load3DDataFile` (`buffer` returned when the file isn't found)
- `roads.c`: `DrivingSchoolRoadsAddObject` (`id`)
- `castle.c`: `FUN_0041f880`, `SubdivideCurveAtMaxError`, `FUN_00429c60`
- `jungle_cruise.c`: `FUN_004367b0`, `JungleCruiseWaterRemoveObject`, `FUN_00436dc0`
- `bloke_ai.c`: `FUN_0044e790`, `FUN_0044e890` (return value)
- `boating_school.c`: `BoatingSchoolWaterRemoveObject` (`owner`)
- `image_sprite.c`: `LoadCompSprite`; `string.c`: `LoadStringTable`; `ride_bloke.c`: `RotateOffset` (default case)

Not bugs, but reported: `FUN_0041db90` (`castle.c`, fills its array through function pointers the analyzer
can't follow).

## How these were found

- clang with `-Wuninitialized -Wsometimes-uninitialized -Wconditional-uninitialized -Wreturn-type` over every
  file in `src/legoland/`.
- clang's static analyzer (`--analyze`, core and `core.uninitialized.*` checkers).
- `tools/audit_locals.py` (a callee writing past one local, which relied on MSVC6's stack layout).
- Each finding compared with the original's machine code (`objdump -d -M intel` on `legoland.exe`).
