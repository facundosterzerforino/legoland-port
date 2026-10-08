# SpeechParseWavHeader (0x00498420, `src/legoland/stream.c`)

**Best: 93.19%**. Kind: C.

## What still differs

- The original reads the first chunk tag before the loop, then tests `tag == 'data'` at the top of the loop, with the
  next tag read at the bottom (`jne` back to the top). Ours puts the read in the loop condition, so the bottom test and
  the early-return layout differ.
- Original has the `tag != 'data'` return only as a shared fallthrough; ours has an explicit final check.

## Tried (don't repeat)

| Date | Who | Change | Result |
|---|---|---|---|
| 2026-10-07 p7 | Haiku agent | Read tag before loop; `while (tag != 'data')` with the tag read at the bottom | 87.43% |
| 2026-10-07 p7 | Haiku agent | `for (;;)` with `if (tag == 'data') break;` at top and the initial read outside the loop | 68.69% (early returns stop being inlined) |

## Ideas not tried yet

- Original's early returns are inline and only the loop-end return is shared; try moving the `tag != 'data'` check
  into the data path.
