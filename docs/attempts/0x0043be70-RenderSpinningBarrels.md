# RenderSpinningBarrels (0x0043be70, `src/legoland/spinning_barrels.c`)

**Best: 99.20%** (starting version, unchanged). Kind: C.

## What still differs

- The `seat = DAT_0062fdd8` copy (rider loop, before the `person->offset` stores). The original loads
  `DAT_0062fdd8+4` into `eax` before `person` is loaded, and loads `DAT_0062fdd8` (x) only after
  `bloke->screen_x` is read. Ours loads both at the copy site, which costs 3 bytes and shifts later jumps.

## Tried (don't repeat)

| Date | Who | Change | Result |
|---|---|---|---|
| 2026-10-07 | Haiku agent (barrels) | Seat copy moved after `person->offset.x` store | 95.31% (worse) |
| 2026-10-07 | Haiku agent (barrels) | Seat copy moved to before `bloke = riders->rider` | 92.00% (worse) |
| 2026-10-07 | Haiku agent (barrels) | Seat copy split into `seat.x = DAT.x; seat.y = DAT.y;` at original spot | 99.20% (no change) |
| 2026-10-07 | Haiku agent (barrels) | Seat copy moved after both `person->offset` stores | 95.05% (worse) |

## Ideas not tried yet

- Reading `DAT_0062fdd8` through a pointer or temporary `Point *` instead of a struct copy.
- Changing `AdjustOffsetForViewMode(&seat)` to operate on a `DAT_0062fdd8` copy directly (not tried).
