# RemapTexCoordsToCell (0x00442040, `src/legoland/render3d.c`)

**Best: 41.17%**. Kind: C.

## What still differs

- Frame: original `sub esp, 0x30`, ours `0x2c` (one slot short).
- The original stores `lo_x`, `hi_x`, `lo_y`, `hi_y` at `[esp+0x2c..0x38]` in that order; the float temporaries are at `[esp+0x40..0x54]`.

## Tried (don't repeat)

| Date | Who | Change | Result |
|---|---|---|---|
| 2026-10-07 r2 | Haiku agent | Reordered the locals | no change |

## Ideas not tried yet

- Find the missing 4-byte local.
