# TempleSlideUpdate (0x00417430, `src/legoland/temple_slide.c`)

**Best: 79.39%**

## What still differs

(not analysed yet)

## Tried (don't repeat)

| Date | Who | Change | Result |
|---|---|---|---|
| 2026-10-08 | permuter + Opus 5.5 | `register unsigned int cy` (cy >>= 9 becomes shr - suggests the original is unsigned), `unsigned short sx`, tile before bloke, field_73 temp. permuter's `cx -= a / 2 - b` (wrong) not kept | 75.35 -> 79.39 |

## Ideas not tried yet

- 
