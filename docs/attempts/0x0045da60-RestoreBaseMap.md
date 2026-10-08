# RestoreBaseMap (0x0045da60, `src/legoland/tilemap.c`)

**Best: 95.65%**. Kind: C.

## What still differs

- The original ends with `and dx, 0x20` before `ret`: the flag test on `TileSpriteInfo[id].sprite` is kept
  even though its result is discarded. Ours drops it.

## Tried (don't repeat)

| Date | Who | Change | Result |
|---|---|---|---|
| 2026-10-07 | Haiku agent | Original `(void)(flags & 0x20);` with volatile read (current best) | 95.65% |
| 2026-10-07 | Haiku agent | `flags &= 0x20;` after the store | 95.65% (no change, `and` dropped) |
| 2026-10-07 | Haiku agent | Non-volatile read plus `if (flags & 0x20) { tile->tile = id; }` | 72.73% (worse) |

## Ideas not tried yet

- Find a construct MSVC6 keeps as a dead `and` on a 16-bit register (for example a `volatile` store of a
  flag-derived value to a stack local whose slot is later reused).
