# PointToIsoPlane (0x0045bcd0, `src/legoland/tilemap.c`)

**Best: 25.70%** (round 2 version). Kind: C.

## What still differs

- The original's `sub esp, 0xc` frame is not reproduced.
- Screen-to-isometric maths: watch signed shifts/divisions and float vs int.

## Tried (don't repeat)

| Date | Who | Change | Result |
|---|---|---|---|
| 2026-10-07 r1 | Haiku agent | Changed a type to `char` | 23.67% -> 22.04% (worse, reverted) |
| 2026-10-07 r2 | Haiku agent | Load `*param_1` and `param_1[1]` once into `int x`, `int y` (the original reads each once) | 23.67% -> 25.70% (kept) |

## Ideas not tried yet

- Find the three locals behind the 0xc frame.
