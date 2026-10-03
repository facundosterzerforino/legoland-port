# Decomp fixes to carry back

Bugs found while making the port run that are really mistakes in the decompiled C, so they belong in the decomp
repo ([facundosterzerforino/legoland](https://github.com/facundosterzerforino/legoland)) too. Each was checked
against the original `legoland.exe`. Once a fix lands in the decomp and is merged here, remove its row.

| Port commit | Function (file) | What's wrong in the decomp | Fix |
|---|---|---|---|
| 0b31037 | `FUN_00458ee0` 0x00458ee0 (`screens.c`) | `PrintSprite(InterfaceBgSprite, Hover.type, saved_value, saved_action, ...)`: the original pushes `0, 0, 0` (asm at 0x458fa4) | `PrintSprite(InterfaceBgSprite, 0, 0, 0, frame.outgoing)` |
| edac58e | `__BMPLoader` 0x0044e010 (`gfx.c`) | 24-bit path: each row aligns the buffer start instead of continuing from the previous row's end, so every row repeats the first; then frees the moved pointer | Keep a running `src`, align it per row, free the original buffer |
| edac58e | `__BMPLoader` 0x0044e010 (`gfx.c`) | 8-bit path: `free(pixels)` after `image->data = pixels` (use after free); the original never frees it | Remove that `free` |
| 0dcd546 | `DAT_004bed40` (`globals.c/.h`) | Typed `unsigned int`, but 0x4bed40..0x4bef9c is a pool of 28 sprite-name strings that `ProgressScreenTables` points into | Type it as `char[0x25c]` (the port also drops `DAT_004bed44`) |
| 724df01 | `SpeechAcmHeader` 0x007aac40, `struct AcmHdr` (`globals.h`) | Declared 0x30 bytes, but it is an `ACMSTREAMHEADER` (0x54 bytes; the ACM driver writes its state into `dwReservedDriver`) and the original reserves 0x60 | Add `dwDstUser`, `dwReservedDriver[10]` and pad to 0x60 |
| 961801b | 10 script keyword tables (`DAT_004bb5b4`, `DAT_004bb5c4`, `DAT_004bb5d8`, `DAT_004bb5e0`, `DAT_004bb5f4`, `DAT_004bb624`, `DAT_004bb688`, `DAT_004bb6bc`, `DAT_004bb6d4`, `DAT_004b7e9c`) | Declared `unsigned int`, but `FindStringNoCase` reads N `char *` entries from each | `char *NAME[N]` (4, 5, 2, 5, 12, 25, 13, 6, 9, 22 entries) |
| 04641db | Ride sprite-info blocks returned by the `cb_a0` callbacks: `DAT_004c1100` (catapult), `DAT_004c1170` (copters), `DAT_004c1228` (joust), `DAT_0062fe60` (plane ride), `DAT_004cbed0` (safari), `DAT_004cbf40` (spider), `DAT_0062fdb0` (spinning barrels), `DAT_004cbf98` (temple slide), `DAT_004cbff0` + `DAT_004cc000` (water works), `DAT_00616120` (eatery), `DAT_0062fd48` (space tower) | Each declared as four separate scalars, but the callback returns the first one's address and the caller reads it as a `struct RideSpriteInfo` (0x14 bytes); `field_10` is the draw tint, so in the port it read the next global and built rides drew dark/magenta | Type each as `struct RideSpriteInfo` |
| cfc898b | `DAT_007cb3e2` (`globals.c/.h`), loops in `obj_instance.c` | `DAT_007cb3e2[128]` is a second view of `ObjInstanceTable` 2 bytes in (the `value` half of each entry), and the loops end at `&DAT_007cb5e0`, the next global | Drop `DAT_007cb3e2`, use `ObjInstanceTable[i].value`, end the loops at `ObjInstanceTable + 128` |
| 1fbb146 | `PTR_Bloke_DoNothing_004b8368` (`globals.c/.h`) | Declared with 16 entries, but the original table has 26 (0x4b8368..0x4b83d0); actions 16..25 are the gardener and mechanic handlers (`Gardener_Idle`, `Gardener_Build`, `Garderner_Repair`, ...), which `DoHighLevelAI` read past the end of the array | Declare it `[26]` |
| a12b5b3 | `TMNegParity` (`render3d.c`) | Not a decomp typing bug: the original's x87 `fcomp; test ah,1` treats NaN as less, and `FUN_00440a30` passes ints read as floats (a NaN), so it returns 1. A C `<` compiled for SSE returns 0 | Port-only; nothing to change in the matching decomp |
| c4ed764 | `DAT_004b9d44` objective event handler table (`nerps.c`) | Declared with 68 entries, but the original has 70 (up to 0x4b9e5c); type 68 (`FUN_0046b1e0`, unimplemented objective, returns 1) and 69 (`FUN_0046b1f0`, returns 0) read past the array, so those level objectives sometimes never completed | Declare it `[70]` with the two handlers |

Also still open from the coordinator's survey: the undersized globals in
`/mnt/project-files/notes/undersized-globals.md` (script keyword tables declared as single `uint`s, the visitor action
table `PTR_Bloke_DoNothing_004b8368`, the park entrance points `DAT_004b8318/8320/8328`).
