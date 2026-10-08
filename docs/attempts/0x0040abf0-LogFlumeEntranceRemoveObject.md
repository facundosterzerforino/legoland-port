# LogFlumeEntranceRemoveObject (0x0040abf0, `src/legoland/log_flume.c`)

**Best: 95.10%**. Kind: C.

## What still differs

- Store order inside the loop body. The original computes `cursor.tile_x` (edx) first, then stores
  `cursor.tile_y` (at `[esp+0x141c]`), then stores `cursor.tile_x` (at `[esp+0x1418]`), and only then runs the
  20-byte memcpy into `cursor.footprint.v`. Ours stores `tile_x` before `tile_y`.
- Load order: the original loads `LogFlumeFootprint+8` (x1) first, then `cur->tile.pos.x`, then `LogFlumeFootprint`.
- Everything else (the memcpy into `LogFlumeTrackRide->footprint`, the two decrements, the `StandardRemoveObject`
  call, the `flags10 & 2` check) already matches.

## Tried (don't repeat)

| Date | Who | Change | Result |
|---|---|---|---|
| 2026-10-07 | Haiku 5.5 (flumea) | Swap the `cursor.tile_x` / `cursor.tile_y` assignment lines | 91.18% (worse) |
| 2026-10-07 | Haiku 5.5 (flumea) | `int tx` local holding tile_x, stored after tile_y | 82.35% (worse) |

## Ideas not tried yet

- Hold `tile_x` and `tile_y` in locals computed in the original's order, then store tile_y before tile_x without
  reordering the expression that loads `cur->tile` (the two experiments above both changed the load order too).
- Read `cur->tile` through a `TileId *` local to see whether that gives the original's `dl` / `al` load pattern.
