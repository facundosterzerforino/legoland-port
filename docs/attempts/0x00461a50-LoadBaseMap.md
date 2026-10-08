# LoadBaseMap (0x00461a50, `src/legoland/map_object.c`)

**Best: 84.15%** (unchanged from start). Kind: C.

## What still differs

- Four diff hunks (at 0x461a62, 0x461ad7, 0x461c84, 0x461dc3). Most of the function matches.
- Stack layout: small scalar locals sit 4 bytes lower in ours than in the original (for example `len`
  is `[esp+0x14]` in ours and `[esp+0x18]` in the original; `elem` is `[esp+0x18]` vs `[esp+0x1c]`).
  The large buffers (`[esp+0x134]`, `[esp+0x340]`, `[esp+0x350]`) and one slot at `[esp+0x10]` match, and the
  frame size (`0x524`) is the same. The cause is not yet found.

## Tried (don't repeat)

| Date | Who | Change | Result |
|---|---|---|---|
| 2026-10-08 | Haiku agent | `elem` declared before `file` | 81.95% (same) |
| 2026-10-08 | Haiku agent | `len` declared first in the locals list | 81.95% (same) |
| 2026-10-08 | permuter + Opus 5.5 | `register int off`/`runlen`, reordered tile/v and prev/phase stores, tile_flags / rf temps, nested `(len & 8) == 0` if, constant-first operands | 81.95 -> 84.15 |

MSVC does not seem to care about declaration order for these slots (the `file`/`elem` and `len` swaps changed nothing).

## Ideas not tried yet

- Check whether the nested blocks (`n/idx/objpos` and `prev/carry/hibit/runlen/rle`) take slots that
  overlap in the original; hoisting them to the top changes the frame size, so compare the frame first.
- Check whether `hdr[200]` / `strbuf` / `namebuf` ordering or size (512 vs another size) moves the scalars.
