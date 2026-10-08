# RenderTempleSlide (0x00416fa0, `src/legoland/temple_slide.c`)

**Best: 95.00%**. Kind: C.

## What still differs

- Stack slots of the two `struct Point` locals (`off1`, `off2`) differ. The original puts the `&off1` temp at `[esp+0x18]`
  and the `&off2` temp at `[esp+0x14]` before the call pushes. Ours puts them at `[esp+0x10]` and `[esp+0x1c]`, so every
  store and reload in the block shifts. The diff is 31 lines starting at 0x417027.

## Tried (don't repeat)

| Date | Who | Change | Result |
|---|---|---|---|
| 2026-10-07 | Haiku agent | Original (current best): `off1` at function scope, `off2` inside the loop block | 95.00% |
| 2026-10-07 | Haiku agent | Move `off2` to function scope, after `off1` | 95.00% (no change in the slot layout) |
| 2026-10-07 | Haiku agent | Declare `off2` then `off1` at function scope | 95.00% (no change) |
| 2026-10-07 | Haiku agent | Move both `off1` and `off2` into the loop block | 95.00% (no change) |

## Ideas not tried yet

- A single `struct Point off[2]` array (changes the addressing, so it may give a different slot order).
- Reading `bloke->screen_x` and the `DAT_004cbf8*` globals into locals before the first `AdjustOffsetForViewMode` call.
