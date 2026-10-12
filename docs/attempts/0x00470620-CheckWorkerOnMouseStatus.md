# CheckWorkerOnMouseStatus (0x00470620, `src/legoland/worker_mouse.c`)

**Best: ?**

## What still differs

(see the rows below)

## Tried (don't repeat)

| Date | Who | Change | Result |
|---|---|---|---|
| 2026-10-09 | Opus 5.5 | the never-looping `for (;;)` replaced by a shared `goto fail` exit (`DAT_00668954 = 1; SetWorkersPositionAtMouse();`) | harness 80.11, verify 77.36 (reverted). The original keeps the constant 1 in ebp (`mov esi, ebp` for isOrder); ours uses immediates |

## Ideas not tried yet

- 
