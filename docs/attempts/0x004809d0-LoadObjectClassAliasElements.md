# LoadObjectClassAliasElements (0x004809d0, `src/legoland/objclass.c`)

**Best: 73.10%**

## What still differs

Registers: the original keeps `token` (ebp) and `elem` (edi) in separate registers and uses immediates for the flag
test/update (`test byte [edi+8],4` and `mov eax,[edi+8]; or al,4; mov [edi+8],eax`). Ours shares edi between token and
elem, so ebx is free and MSVC parks the constant 4 in it (`test [edi+8],bl`, `or [edi+8],ebx`).

## Tried (don't repeat)

| Date | Who | Change | Result |
|---|---|---|---|
| 2026-10-09 | Opus 5.5 | `_strcmpi` (the original calls strcmpi, not stricmp) + `GLOBAL` annotation on ObjClassAliasTable (0x4bcdcc) | 73.10 (call and table now match; kept) |
| 2026-10-09 | Opus 5.5 | index loop `for (i = 0; i < 5; i++)` over the table | 73.10 |

## Ideas not tried yet

- 
