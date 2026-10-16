/*
 * [library:movie] The port's entry points into FFmpeg's Indeo 3, Indeo 5 and Cinepak decoders (this folder).
 *
 * This file is part of the port's copy of FFmpeg's Indeo decoders and, like them, is licensed under the
 * GNU Lesser General Public License, version 2.1 or later (see COPYING.LGPLv2.1).
 */

#ifndef PORT_FFMPEG_H
#define PORT_FFMPEG_H

/* Bytes of zeros the caller must place after each frame's data: the bit readers look a few bytes ahead. */
#define PORT_FFMPEG_PADDING 64

typedef struct PortVideoDecoder PortVideoDecoder;

/* fourcc is the AVI stream's biCompression: IV31/IV32 (Indeo 3), IV50 (Indeo 5) or cvid (Cinepak, the intro of
 * the European CDs), any case; bit_count its biBitCount. Returns NULL for other codecs. */
PortVideoDecoder *PortVideoDecoder_Open(unsigned int fourcc, int width, int height, int bit_count);

/* Decodes one frame (one AVI sample). Returns 1 with the picture when it produced one: the YUV 4:1:0 planes (Y,
 * U, V; chroma a quarter of the width and height), or for an RGB decoder (PortVideoDecoder_IsRgb) one top-down
 * RGB24 plane in planes[0]; 0 for a frame without one (a null frame: the previous picture stays), or a negative
 * value on an error. The planes stay valid until the next call. */
int PortVideoDecoder_Decode(PortVideoDecoder *decoder, const unsigned char *data, int size,
    const unsigned char *planes[3], int linesize[3]);

/* Non-zero when the decoder gives RGB24 pictures (Cinepak) instead of YUV 4:1:0 planes. */
int PortVideoDecoder_IsRgb(const PortVideoDecoder *decoder);

void PortVideoDecoder_Close(PortVideoDecoder *decoder);

#endif
