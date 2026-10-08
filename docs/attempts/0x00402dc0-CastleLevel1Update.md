# CastleLevel1Update (0x00402dc0, `src/legoland/castle_level.c`)

**Best: 85.88%**. Kind: C.

## What still differs

- Switch-case tail sharing: the original's case 0 and case 4 both branch into one shared block that calls
  `CalcMoveLine` from `bloke->dest` (`0x402e43`). Ours keeps the cases separate.
- Store order inside case 0 and case 4 (`shl edi,8` before `mov [esi+0x24],ebp`).
- The jump-table entries and the default branch layout differ (`ja` target).

## Tried (don't repeat)

| Date | Who | Change | Result |
|---|---|---|---|
| 2026-10-08 | Haiku agent (a1) | Start: case 4 uses a local `to` then `bloke->dest = to` | 85.88% |
| 2026-10-08 | Haiku agent (a1) | Case 4 writes `bloke->dest.x/.y` directly and calls `CalcMoveLine(bloke->pos, bloke->dest, ...)` | 82.55% (worse) |

## Ideas not tried yet

- Write case 4 with the same statement order as case 0 (`dest.x` then `dest.y` then the call) only after case 0's order
  is matched first, so the shared tail is reached by the same instructions.
- Check the `(unsigned char)(dir + 0x10) >> 5` expression in the cases for a store-order difference.
