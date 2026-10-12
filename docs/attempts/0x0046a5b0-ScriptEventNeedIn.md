# ScriptEventNeedIn (0x0046a5b0, `src/legoland/nerps.c`)

**Best: 74.32%**

## What still differs

(not analysed yet)

## Tried (don't repeat)

| Date | Who | Change | Result |
|---|---|---|---|
| 2026-10-08 | permuter + Opus 5.5 | permuter 76.4% made y unsigned (then `y >= 0` is always true) - not kept | rejected |
| 2026-10-09 | Opus 5.5 | `int y` (the original compares y signed, `jg`); + `volatile int count` / `int count[1]` | verify 74.32 (was 75.14), not kept; the GameMap/lpConfig hoisting differs like ScriptEventClearArea |

## Ideas not tried yet

- 
