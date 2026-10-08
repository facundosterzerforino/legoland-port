# ScriptEventClear (0x00469c80, `src/legoland/objectives.c`)

**Best: 94.32%**. Kind: C.

## What still differs

- Per-object loop body (around 0x469d57): the original reads `current->tile_y` / `tile_x` (unsigned char fields at +4 / +5 of `SweepInstance`) as `xor edx,edx; mov dl,[edi+4]`, zero-extended into a 32-bit register kept as `ebp`. Ours reads them as `movzx si, byte` / `movzx cx, byte` + `movsx edx, cx` with `short` locals.
- Register choice for the tile values and the sums (`add esi, edx` in the original vs `add ebp, edx` in ours) follows from the load type.

## Tried (don't repeat)

| Date | Who | Change | Result |
|---|---|---|---|
| 2026-10-07 | Haiku agent | `tile_x` and `tile_y` locals as `unsigned char` | 80.86% (frame 0x185c, worse) |
| 2026-10-07 | Haiku agent | Only `tile_y` as `unsigned char` (`tile_x` stays `short`) | 84.30% (frame 0x1858, worse) |
| 2026-10-07 | Haiku agent | `tile_x` and `tile_y` locals as `int` | 65.33% (worse) |
| 2026-10-08 | permuter + Opus 5.5 | `right` temp for footprint.v[2]; tile_x read before tile_y; decl order | 93.18 -> 94.32 |

## Ideas not tried yet

- Keep `short` locals but read the byte fields straight into `point.y` / `point.x` (int) before the `rect` block, so the zero-extend lands in a 32-bit register.
- Split the `rect` computation so the footprint adds use the same register as the tile loads.
