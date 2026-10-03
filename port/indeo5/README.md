# Indeo 5 decoder (from FFmpeg)

Windows 11 has no Indeo codec, so the port decodes LEGOLAND's Indeo Video 5 (`IV50`) AVIs itself
(`[library:movie]`, see `PORTING.md`). This folder is FFmpeg's Indeo 5 decoder, taken from
[FFmpeg](https://github.com/FFmpeg/FFmpeg) `libavcodec` at commit `91fcca692fc96a8213b341de88d05a7c0866152c`.

**License:** LGPL-2.1-or-later (`COPYING.LGPLv2.1`). Keep the license headers.

| File | Origin |
|---|---|
| `indeo5.c`, `ivi.c`, `ivi.h`, `ivi_dsp.c`, `ivi_dsp.h`, `indeo5data.h` | FFmpeg, unchanged except their `#include` lines (now `ivi_compat.h`) and, in `indeo5.c`, the `FFCodec` registration replaced by `ff_indeo5_decode_init` |
| `ivi_compat.h` | Written for the port: the libavutil/libavcodec definitions those files use (little-endian bit reader, VLC tables, frame and memory helpers) |
| `port_indeo5.c`, `port_indeo5.h` | Written for the port: the decoder's entry points, the VLC table builder and the zigzag table |

`port/port_movie.c` reads the AVI samples through Video for Windows and turns the decoded YUV 4:1:0 pictures
into the 16-bit frames the game draws.

To update from a newer FFmpeg: copy the six FFmpeg files again and redo the include edits.
