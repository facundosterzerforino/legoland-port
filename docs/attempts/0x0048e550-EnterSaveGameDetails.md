# EnterSaveGameDetails (0x0048e550, `src/legoland/savegame_ui.c`)

**Best: 84.10%**

## What still differs

(not analysed yet)

## Tried (don't repeat)

| Date | Who | Change | Result |
|---|---|---|---|
| 2026-10-08 | permuter + Opus 5.5 | nested `count < 0x1f` / `0xcb > DAT_00798738` ifs, `register int right`, `unsigned int bottom`, `register RECT rc`, name / sprite_y temps, fy before fx | 76.70 -> 84.10 |
| 2026-10-09 | Opus 5.5 | one function-scope `RECT rc` for both by-value calls (no `register` RECTs) | 89.51; the original frame (0x18) reserves 16 bytes it never touches, ours is 8 |

## Ideas not tried yet

- 
