# GetArmColourOfBloke (0x00443220, `src/legoland/render3d.c`)

**Best: 30.30%**. Kind: C.

## What still differs

- Same pattern as GetLegColourOfBloke (0x004431f0), on `field_90`.

## Tried (don't repeat)

| Date | Who | Change | Result |
|---|---|---|---|
| 2026-10-07 r2 | Haiku agent | The same variants as GetLegColourOfBloke | no change |
| 2026-10-08 | permuter + Opus 5.5 | new permuter: volatile r assigned last (register swap not found) | 25.00 -> 30.30 |
| 2026-10-09 | Opus 5.5 | MAKEWORD form, see GetLegColourOfBloke (same code, field_90) | original shape, ecx/edx swapped |

## Ideas not tried yet

- Solve GetLegColourOfBloke first and apply the same form.
