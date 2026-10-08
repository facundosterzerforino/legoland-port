# CoptersPlaceRider (0x00404630, `src/legoland/copters.c`)

**Best: 54.39%** (`volatile int index`, `index` named in the `fistp` asm block). Kind: C + __asm.

## What still differs

- Hand-written asm in the original (the `fld/fmul/fistp` macro) inside compiled C, and a `switch` with a jump table, so it cannot be transcribed as naked asm.
- What is left is a register permutation: the original keeps `entry`/`layer`/`s` in `edi`/`ebx`/`esi` and spills `entry` into `node`'s parameter slot `[ebp+8]`; ours uses `esi`/`edi`/`ebx`.
- `index` doubles as the y offset (the default case keeps the original index) and as the asm's float/int scratch.

## Tried (don't repeat)

| Date | Who | Change | Result |
|---|---|---|---|
| 2026-10-07 | Claude | First C version with a separate `yoff` | 29% (frame 0x2c, original 0x28) |
| 2026-10-07 | Claude | `index = 0xeb` etc. instead of `yoff` | no frame change |
| 2026-10-07 | Claude | Drop the `frame` pointer local (repeat the expression) | no frame change |
| 2026-10-07 | Claude | `entry` computed before the GetScreenCoordsForObject call; body inside `if (rider != NULL) {}` | structure closer |
| 2026-10-07 | Claude | `fld dword ptr index` / `fistp index` in the asm (index stays in memory) | 36.7%, frame now 0x28 (kept) |
| 2026-10-07 | Claude | `volatile int index` (reloaded at every use, as in the original) | 54.39% (kept) |
| 2026-10-07 | Claude | Reassign `node` to the entry (`node = (struct CopterNode *)&node->layer[index]`) | 44.77% (worse) |

## Ideas not tried yet

- Get `entry` spilled to `[ebp+8]` without reusing `node` (maybe an inner block, or a second pointer local).
