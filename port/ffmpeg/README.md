# Indeo 3 and Indeo 5 decoders (from FFmpeg)

Windows 11 has no Indeo codecs, so the port decodes LEGOLAND's Indeo AVIs itself (`[library:movie]`, see
`PORTING.md`): Indeo Video 5 (`IV50`) for the advisor animations and most movies, Indeo 3 (`IV31`/`IV32`) for
one movie. This folder holds FFmpeg's decoders for both, taken from [FFmpeg](https://github.com/FFmpeg/FFmpeg)
`libavcodec` at commit `91fcca692fc96a8213b341de88d05a7c0866152c`.

**License:** LGPL-2.1-or-later (`COPYING.LGPLv2.1`). Keep the license headers.

| File | Origin |
|---|---|
| `indeo5.c`, `ivi.c`, `ivi.h`, `ivi_dsp.c`, `ivi_dsp.h`, `indeo5data.h` | FFmpeg, unchanged except their `#include` lines (now `ffmpeg_compat.h`) and, in `indeo5.c`, the `FFCodec` registration replaced by `ff_indeo5_decode_init` |
| `indeo3.c`, `indeo3data.h` | FFmpeg, unchanged except their `#include` lines and, in `indeo3.c`, the `FFCodec` registration replaced by `ff_indeo3_decode_*` wrappers |
| `ffmpeg_compat.h` | Written for the port: the libavutil/libavcodec definitions those files use (bit and byte readers, VLC tables, block copies, frame and memory helpers) |
| `port_ffmpeg.c`, `port_ffmpeg.h` | Written for the port: the decoders' entry points, the VLC table builder and the zigzag table |

`port/port_movie.c` reads the AVI samples through Video for Windows and turns the decoded YUV 4:1:0 pictures
into the 16-bit frames the game draws.

To update from a newer FFmpeg: copy the FFmpeg files again and redo the include edits.
