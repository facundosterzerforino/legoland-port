# RemoveNewObject (0x00471ca0, `src/legoland/popupinfo.c`)

**Best: 63.16%** (round 2 version). Kind: C.

## What still differs

- Register allocation: the original keeps `n` in `edx` throughout; ours copies it through `eax` at the entry test, after `n--` and at the loop bound.
- The `count = n` store and the `current` reset are ordered differently.

## Tried (don't repeat)

| Date | Who | Change | Result |
|---|---|---|---|
| 2026-10-07 r2 | Haiku agent | Locals `n`, `k`, `j`; the kill branch doesn't reload `count`; the shift loop reads `NewObjects.count` in its condition | 48.60% -> 63.16% (kept) |
| 2026-10-07 r2 | Haiku agent | `#pragma optimize("y", off)` | 46.02% (worse: the original has no ebp frame) |
| 2026-10-07 r2 | Haiku agent | `while` outer loop; `n > 0` instead of `0 < n` | no change |
| 2026-10-07 r2 | Haiku agent | `count` store before the `current` check; `--n` | no change |

## Ideas not tried yet

- None recorded.
