# RenderFullMap (0x004567a0, `src/legoland/mapscreen.c`)

**Best: 81.85%**. Kind: C.

## What still differs

- Stack-slot assignment of the locals. The six `ElemID` results and the three LoadSprite results sit in different frame slots than in the original (e.g. square_track at esp+0x88 in ours, esp+0x8c in the original; square_track_height at esp+0x9c in ours, esp+0xb4 in the original; stick at esp+0x98 vs esp+0x9c). Frame size (`sub esp, 0xf8`) already matches.
- The 15 diff hunks run through the whole function (about 0x4567a0 to 0x457850); the first ones are the stack-slot differences, the later ones are the rendering loop and the terrain pass.

## Tried (don't repeat)

No attempts recorded yet (first session: baseline measured at 81.60%).

## Ideas not tried yet

- Declare the ElemID result locals in a different order (or in nested blocks) to see which order reproduces the original slot assignment.
- Check which locals are only live in the first block, since the original shares slot 0x8c between square_track and square_track_height_path.

## Notes

No earlier round notes for this function.

| 2026-10-08 | permuter + Opus 5.5 | many temps (bounds_y1/y2, dest_x, uid_x, ...), `register` pen/base_x/x1, loops as while, nested ifs, unsigned py/x2 (permuter 84.5% included a y1/y2 bug: expression dropped) | 81.60 -> 81.85 |