# UpdateQueuePathShape (0x00413650, `src/legoland/roads.c`)

**Best: 97.18%** (unchanged from start). Kind: C.

## What still differs

- The original reloads the `id` parameter from its stack slot inside each branch (`mov dx, word ptr [esp+0x3c]`); ours hoists the load of `param_1`/`id` into `dx` before the first test.
- Register allocation for the `mask`/`count` join: the original reloads `count` from `[esp+0x10]` on the else path.
- The inner `(m & 0x38) == 0x38` else-branch (`flag | 1, 2` vs `flag, 0`) is placed far away in ours (`jne 0x97`) and right after the then-block in the original.

## Tried (don't repeat)

| Date | Who | Change | Result |
|---|---|---|---|
| 2026-10-08 | Claude Haiku 5.5 | Drop the `id` local and use `param_1` directly in every compare | 88.64% (same, load hoisted instead) |
| 2026-10-08 | Claude Haiku 5.5 | `if (r.field_0 != NULL && r.field_0->field_8 == id)` instead of nested if | 88.64% (same) |
| 2026-10-08 | permuter + Opus 5.5 | `int m` (was unsigned); `if (r.field_18 != NULL) { if (... == id)` nested | 88.64 -> 97.18 |

## Ideas not tried yet

- Restructure the `(m & 0x11)` block as a flat chain of `if / else if` returning per branch to change where the else blocks are placed.
- Declare `count` after the `FUN_00413520` call so it lives in memory like the original.
