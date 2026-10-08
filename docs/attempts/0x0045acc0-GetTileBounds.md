# GetTileBounds (0x0045acc0, `src/legoland/tilemap.c`)

**Best: 28.30%**. Kind: C.

## What still differs

- Register assignment: the original loads `size` into `edx` and keeps `ref` in `ecx`.

## Tried (don't repeat)

| Date | Who | Change | Result |
|---|---|---|---|
| 2026-10-07 r2 | Haiku agent | Inlined the sprite lookup | no change or worse |
| 2026-10-07 r2 | Haiku agent | `int` vs `short` for the size | no change or worse |
| 2026-10-07 r2 | Haiku agent | A `dia` local | no change or worse |
| 2026-10-07 r2 | Haiku agent | Store into a `left` local, then `out[0]` | 14.43% (much worse) |

## Ideas not tried yet

- None recorded.
