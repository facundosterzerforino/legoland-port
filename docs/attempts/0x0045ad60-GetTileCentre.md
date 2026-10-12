# GetTileCentre (0x0045ad60, `src/legoland/tilemap.c`)

**Best: 93.02%**. Kind: C.

## What still differs

- `out[1]`: the original computes `imul`, then adds `lpConfig->view_y`, then subtracts `ScrollY >> 8` (loaded last).
  Ours loads `ScrollY` and subtracts it right after the `imul`, and adds `view_y` after.

## Tried (don't repeat)

| Date | Who | Change | Result |
|---|---|---|---|
| 2026-10-07 p6 | Haiku agent | `out[1] = (...) * (...) + lpConfig->view_y - (ScrollY >> 8);` without the temp `y` | 93.02% (no change) |
| 2026-10-07 p6 | Haiku agent | `out[1] = lpConfig->view_y + (...) * (...) - (ScrollY >> 8);` | 93.02% (no change) |
| 2026-10-07 p6 | Haiku agent | `int y`, `y += lpConfig->view_y;`, then `out[1] = y - (ScrollY >> 8);` | 93.02% (no change) |
| 2026-10-09 | Opus 5.5 | `y +=` / `y -=` statements, `(int)` view_y, `(unsigned int)(ScrollY >> 8)`, `+ (view_y - (ScrollY >> 8))`, `- (ScrollY >> 8) + view_y` | 93.02 (no change, the ScrollY load is always scheduled first) |

## Ideas not tried yet

- Pass the sum through a helper or a differently typed intermediate to stop MSVC from hoisting the `ScrollY` load.
