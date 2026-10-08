# GetLegColourOfBloke (0x004431f0, `src/legoland/render3d.c`)

**Best: 25.00%**. Kind: C.

## What still differs

- Only a register swap: the original builds R/G in `ecx` (`mov ch,R` / `mov cl,G`) and B in `edx`, zeroing both before dereferencing `param_1->field_4` (loaded via `eax`); ours builds R/G in `edx`, B in `cl`, and loads `field_4` via `ecx`. Every instruction shifts with it, hence 25%.

## Tried (don't repeat)

| Date | Who | Change | Result |
|---|---|---|---|
| 2026-10-07 r2 | Haiku agent | About eight variants, including byte-combining `|=` / `<<=` | no change |
| 2026-10-07 | Claude | Return the expression directly, no r/g/b locals | no change (25%) |
| 2026-10-07 | Claude | `unsigned char` r/g/b locals | no change (25%) |
| 2026-10-07 | Claude | r, g, b declared and loaded in that order | no change (25%) |
| 2026-10-07 | Claude | One accumulator: `c = r; c = c << 8 | g; c = c << 8 | b` | no change (25%) |
| 2026-10-08 | permuter + Opus 5.5 | pointer to the 3-byte entry (with/without r/b/g locals), int locals, single |-expression with << 16, rg accumulator, [idx][k] row cast (2 forms) - all 25%; bytes stored into a uint 23.08% | 25 x8 / 23.08 |
| 2026-10-08 | permuter + Opus 5.5 | (see above) register swap unaffected by any addressing form | - |

## Ideas not tried yet

- Two accumulators initialised to 0 at declaration, before the pointer is read.
- A union or struct of bytes for the colour instead of shifts.
- Whatever matches here likely fixes GetArmColourOfBloke too.
