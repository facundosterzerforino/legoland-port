# AdvanceFlumeMover (0x00411680, `src/legoland/log_flume.c`)

**Best: 40.52%**. Kind: C.

## What still differs

- Stack frame: the original reserves 12 bytes of locals, with `flag` in memory at `[esp+0xc]`, `rev` in `ebx` and `shape` in `edi`. Ours reserves 8 bytes, keeps `flag` in `ebx`, `rev` in `edi` and `shape` in `eax`.
- The original's fallback path reads an uninitialised `r` from `[esp+0x14]` and `[esp+0x18]`; ours doesn't.

## Tried (don't repeat)

| Date | Who | Change | Result |
|---|---|---|---|
| 2026-10-07 r2 | Haiku agent | Reordered the declarations | no change |
| 2026-10-07 r2 | Haiku agent | Took the address of `flag` to force it into memory | no change |

## Ideas not tried yet

- Make `r` a real (uninitialised) local struct/pair that the fallback reads.
