# UnlinkGardenerOrder (0x00499d60, `src/legoland/worker.c`)

**Best: 76.73%**

## What still differs

The original has one "Work orders START" DBPrintf block placed right after the head-of-list branch; the search path
jumps back to it (tail merge). Ours emits the print twice: the first copy loads Tail into ecx (Head/Tail registers
differ between the copies), so MSVC does not merge them.

## Tried (don't repeat)

| Date | Who | Change | Result |
|---|---|---|---|
| 2026-10-09 | Opus 5.5 | one shared print after an if/else (not-found path returns early) | 71.23 (else body laid out before the print) |

## Ideas not tried yet

- make both print copies load Head/Tail the same way so the tail merge happens
