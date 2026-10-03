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
| (next) | 10 script keyword tables (`DAT_004bb5b4`, `DAT_004bb5c4`, `DAT_004bb5d8`, `DAT_004bb5e0`, `DAT_004bb5f4`, `DAT_004bb624`, `DAT_004bb688`, `DAT_004bb6bc`, `DAT_004bb6d4`, `DAT_004b7e9c`) | Declared `unsigned int`, but `FindStringNoCase` reads N `char *` entries from each | `char *NAME[N]` (4, 5, 2, 5, 12, 25, 13, 6, 9, 22 entries) |

Also still open from the coordinator's survey: the undersized globals in
`/mnt/project-files/notes/undersized-globals.md` (script keyword tables declared as single `uint`s, the visitor action
table `PTR_Bloke_DoNothing_004b8368`, the park entrance points `DAT_004b8318/8320/8328`).
