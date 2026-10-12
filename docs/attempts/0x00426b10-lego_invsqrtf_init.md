# lego_invsqrtf_init (0x00426b10, `src/legoland/castle.c`)

**Best: 97.83%** (the code is byte-identical)

## What still differs

Only reccmp's symbolization of the first loop's end pointer: `cmp eax, &invsqrtf_table[0] + 0x204`. In the original
that address is `invsqrtf_exp_table` (0x610e44), because there are 4 unexplained bytes at 0x610e40 between the two
tables. In our image nothing lies there, so it prints as `invsqrtf_table+0x204`.

## Tried (don't repeat)

| Date | Who | Change | Result |
|---|---|---|---|
| 2026-10-09 | Opus 5.5 | analysis only | - |

## Ideas not tried yet

- Fixing it needs something 4 bytes long at 0x610e40 followed by `invsqrtf_exp_table` in our .bss (a `[129]` table
  or a filler global). That is padding, which the project rules don't allow, unless the real variable at 0x610e40 is found.
