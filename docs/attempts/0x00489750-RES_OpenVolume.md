# RES_OpenVolume (0x00489750, `src/legoland/resource.c`)

**Best: 92.37%**. Kind: C.

## What still differs

- Directory read block (`SetFilePointer(handle, dir_offset, 0, 0)` / `ReadFile`): register choice differs. Original
  keeps `dir_offset` in eax and the handle in ecx; ours uses ecx and edx.
- `file_size - dir_offset` is computed inline in the original, in eax, and reused in ecx for the `ReadFile` size.

## Tried (don't repeat)

| Date | Who | Change | Result |
|---|---|---|---|
| 2026-10-07 p7 | Haiku agent | Hoist `file_size - dir_offset` into a `dir_size` local, used for malloc/ReadFile/compare | 88.39% |

## Ideas not tried yet

- Swap the order of the two `SetFilePointer` arguments into a temp so the handle lands in ecx.
