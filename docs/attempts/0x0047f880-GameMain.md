# GameMain (0x0047f880, `src/legoland/debug.c`)

**Best: 99.62%**. Kind: C.

## What still differs

- One instruction only: the first resource-volume loop (`for (i = 0; i < 3; i++)`) ends with `jb` in the original (unsigned compare on `i`) and `jl` in ours. The error-path loop after it (`for (j = 0; j < i; j++)`) needs a signed `i` (`jle`), so the two loops disagree on signedness.

## Tried (don't repeat)

| Date | Who | Change | Result |
|---|---|---|---|
| 2026-10-07 | Haiku 5.5 | Baseline (previous round's C) | 99.62% |
| 2026-10-07 | Haiku 5.5 | `unsigned int i` for the whole function | 99.24% (jb/jl still differs, error loop becomes `jbe`) |
| 2026-10-07 | Haiku 5.5 | `(unsigned int)i < 3` in the loop condition | 99.62% (MSVC still emits `jl`) |
| 2026-10-07 | Haiku 5.5 | `i < sizeof(ResourceFileNames) / sizeof(ResourceFileNames[0])` | 99.62% (still `jl`) |
| 2026-10-07 | Haiku 5.5 | `unsigned int i` plus `j < (int)i` in the error loop | 99.62% (still `jl`) |
| 2026-10-07 | Haiku 5.5 | First loop rewritten as `do { } while (i < 3)` | 82.07% (worse, codegen restructured) |
| 2026-10-07 | Claude Opus 5.5 | `i < 3U` (unsigned constant) | 99.62% (still `jl`) |
| 2026-10-07 | Claude Opus 5.5 | `(unsigned)i < 3U` | 99.62% (still `jl`: MSVC6 picks the signed jump even for an explicitly unsigned compare here) |
| 2026-10-07 | Claude Opus 5.5 | `i < (int)3U` (control) | 99.62% (`jl`) |

## Ideas not tried yet

- A loop shape where MSVC cannot range-analyse `i` (so it keeps the unsigned compare), while the error-path loop still sees a signed `i`.
- Check whether the `jb` comes from a different bound expression (e.g. a size_t-typed count) that is not folded to a constant.
