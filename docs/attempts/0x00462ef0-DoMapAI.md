# DoMapAI (0x00462ef0, `src/legoland/map_object.c`)

**Best: 93.43%**. Kind: C.

## What still differs

- Frame: the original is `sub esp, 8` (`i` and the `TileId id`). Ours is `sub esp, 0x10`: the
  `unsigned char tx` / `ty` locals get spilled. Removing them (reading `tile->field_4/5` directly into
  `id.pos`) matches the frame but makes MSVC swap `lpConfig` (ebx in the original) and the zero
  constant (edi in the original), which drops the score to 81.24%.
- Hence the remaining diffs are the `je`/`jl` displacements that follow the spilled-local layout.

## Tried (don't repeat)

| Date | Who | Change | Result |
|---|---|---|---|
| 2026-10-07 | Haiku agent | Baseline (start of this session) | 93.43% |
| 2026-10-07 | Haiku agent | `tx`/`ty` locals removed, `tile->field_4/5` read directly into `id.pos` | 81.24% (worse: lpConfig/zero register swap) |
| 2026-10-07 | Haiku agent | Same as above plus unused `cls` local removed (`cls` is used at the end, so this failed to build; restored) | 81.24% (same) |

## Ideas not tried yet

- Keep the direct `tile->field_4/5` reads but move the `lpConfig` load so its register is pinned to ebx
  (e.g. read `lpConfig->height` / `width` before the switch).
- Reorder the case 0/1/2 bodies only in the source (not the switch) to influence coloring.
