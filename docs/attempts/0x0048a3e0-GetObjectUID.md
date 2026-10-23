# GetObjectUID (0x0048a3e0)

## 2026-10-19: back to C

The rule that allowed a documented naked `__asm` transcription for compiler choices C cannot reach was withdrawn.
The function is the plain C that was the `#else` branch again: 77.35%. Matching has to come from C now.

## 2026-10-19 Opus 5.5 (plain C, no change: 77.35%)

Scalar x/y instead of the `tile` / `probe` Points (38.52 and 30.61), int `px`/`py` probes (34.30), field-wise probe
copies (34.30-55.01), `tile.x = pos->x >> 8` field-wise (77.95 harness, 97.44 ignoring structure, but no better in
verify). The structure copies `probe = tile` stay.
