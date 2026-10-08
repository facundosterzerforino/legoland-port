# SearchJunglePathConnected (0x00437260, `src/legoland/jungle_cruise.c`)

**Best: 38.41%**. Kind: C.

## What still differs

- The original turns the last recursive call (the `x - 5` branch) into a jump back to the top; MSVC already does that from the plain recursive source.
- The gap is register allocation: which of `x`/`y`/`tx`/`ty` land in `esi`/`edi`/`ebp`/`ebx`.

## Tried (don't repeat)

| Date | Who | Change | Result |
|---|---|---|---|
| 2026-10-07 r2 | Haiku agent | Rewrote as `for (;;)` with the tail call as `continue` (two variants) | 19.86% (much worse) |

## Ideas not tried yet

- None recorded.
