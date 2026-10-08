# RunAppraisal (0x004453a0, `src/legoland/challenge.c`)

**Best: 15.39%**. Kind: C.

## What still differs

- Stack frame: original `sub esp, 0x23d4`, ours `0x23c0` (20 bytes short). `rep` starts at `[esp+0x70]` in the original and `[esp+0x68]` in ours.
- `out60`, `out64`, `out68`, `out6c` are declared but never used, so MSVC drops them. The original uses those slots: it zeroes `[esp+0x60]` and `[esp+0x5c]` on entry, reads `[esp+0x68]` around the `ReportFlags & 0x80` block and passes `lea ecx,[esp+0x6c]` to FUN_00444bf0.
- The original keeps `xbase`, the zero counters and `[esp+0x58]` (a pointer into `rep`) in memory; ours keeps them in registers.
- About 3,600 lines with many `goto LAB_...`: too big for one agent pass.

## Tried (don't repeat)

| Date | Who | Change | Result |
|---|---|---|---|
| 2026-10-07 r1 | Haiku agent | Added a padding local to make up the 20 bytes | rejected: fake padding is not allowed |
| 2026-10-07 r2 | Haiku agent | Analysis only (the frame findings above) | no change |

## Ideas not tried yet

- Find which real variables live at `[esp+0x58..0x6c]` (the FUN_00444bf0 out-parameter at 0x6c first) and use them, so the frame grows naturally.
- Split the work: fix the frame first, then go block by block.
