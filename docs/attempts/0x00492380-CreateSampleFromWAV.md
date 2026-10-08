# CreateSampleFromWAV (0x00492380, `src/legoland/sound_sfx.c`)

**Best: 97.49%** (start 97.49%)

## Tried
| Date | Model | Change | Result |
|---|---|---|---|
| 2026-10-07 | claude-haiku-5-5 | `if (SoundAvailable != 0) file = RES_OpenFile(...)` instead of early return | 97.49% (no change) |
| 2026-10-07 | claude-haiku-5-5 | `if (file == NULL) return NULL;` before the do/while | 97.49% (no change) |

## What still differs
- Original keeps the `SoundAvailable == 0` early return as its own inline epilogue (`jne` over 10 bytes of
  pops and `ret`). Ours jumps to the shared bottom `return NULL` epilogue, because MSVC merges the two
  identical `return NULL` blocks.
- Success path: ours duplicates `pop ebp; pop ebx; add esp; ret` after the `RES_CloseFile` tail.

## Ideas not tried yet
- Make the bottom `return NULL` differ from the early one so the compiler cannot merge them.
