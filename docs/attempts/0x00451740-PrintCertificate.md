# PrintCertificate (0x00451740, `src/legoland/certificate.c`)

**Best: 81.20%** (start value, no change yet). Kind: C.

## What still differs

- Frame slots: the original keeps `returned` at `esp+0x28` (after the 4 pushes) and `needed` at `esp+0x30`. Ours puts `returned` at `esp+0x2c`, so the zero stores before `EnumPrintersA` use `[esp+0x44]`/`[esp+0x4c]` in the original and `[esp+0x48]`/`[esp+0x4c]` in ours. Declaration order does not move it.
- Register choice: the original uses `ebp` as the zero register and `ebx` for the `_open` result `fd`; ours has them swapped (`xor ebx, ebx`, `fd` in `ebp`).
- Error exits: the original repeats the full cleanup (`GlobalFree`/`DeleteDC`/`pop`s/`ret`) at each failure site. Ours jumps back to a shared tail (`jmp -0xc5`, `jmp -0xf1`), so the layout and the `pInfo == NULL` path differ.
- Cleanup order around `GetDeviceCaps(RASTERCAPS)` / `DeleteObject(hBitmap)` / `je` differs: the original calls `DeleteObject` and `DeleteDC` in the tail after a `test al,1` and a separate `push 0x26` call.

## Tried (don't repeat)

| Date | Who | Change | Result |
|---|---|---|---|
| 2026-10-08 | Haiku 5.5 (a2) | Declaration order: `printers` moved to the end of the locals | 81.20% (no change) |
| 2026-10-08 | Haiku 5.5 (a2) | Declaration order: `returned` declared before `needed` and `bmfh`/`bmih` first | 81.20% (no change) |
| 2026-10-08 | permuter + Opus 5.5 | permuter 84.1% came from a temp landing as the body of the braceless `if (biBitCount < 9)` so `_read` always ran (behaviour change); without it no gain | rejected |

## Best

81.20%, the version currently in the file (unchanged from the start).

## Ideas not tried yet

- Find the 4-byte hole the original leaves at `esp+0x2c`. Something the original declares is either in a block or not spilled at all; try block scope for `needed`/`returned` (for example a `{ }` around the `EnumPrintersA` check).
- Write the first failure path as the original does (a full copy of the cleanup and `return 0`), so the tail is not merged.
- Swap `fd` and the zero register by changing which of `_open`'s result or `needed=0` is assigned first.
