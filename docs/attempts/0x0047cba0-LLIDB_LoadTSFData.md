# LLIDB_LoadTSFData (0x0047cba0, `src/legoland/llidb.c`)

**Best: 90.58%** (unchanged C from the start of this round). Kind: C.

## What still differs

- Register permutation only: the original takes `sprites[i]` in `eax` (reloaded from `si->sprites`), ours uses `ecx`.
- After `RES_ReadFile(file, &len, 4)` the original takes `&len` in `ecx` and ours in `edx`.
- `element` is loaded into `eax` before the `head->data` store in the original, into `ecx` in ours; the `*(element + 0xc)` temp is `edx` vs `eax`.

## Tried (don't repeat)

| Date | Who | Change | Result |
|---|---|---|---|
| 2026-10-07 | Claude Haiku 5.5 | Duplicate `anim = si->sprites[i]->image;` line | no change (90.58%) |
| 2026-10-07 | Claude Haiku 5.5 | Read the sprite through a `struct Sprite *sp` local | no change (90.58%) |

## Ideas not tried yet

- Reorder the `element` use so the original's `eax`/`edx` split comes out (e.g. read `*(int *)(element + 0xc)` into a local before the `head->data` store).
