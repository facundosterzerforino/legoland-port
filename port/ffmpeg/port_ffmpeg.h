/*
 * [library:movie] The port's entry points into FFmpeg's Indeo 3 and Indeo 5 decoders (this folder).
 *
 * This file is part of the port's copy of FFmpeg's Indeo decoders and, like them, is licensed under the
 * GNU Lesser General Public License, version 2.1 or later (see COPYING.LGPLv2.1).
 */

#ifndef PORT_FFMPEG_H
#define PORT_FFMPEG_H

/* Bytes of zeros the caller must place after each frame's data: the bit readers look a few bytes ahead. */
#define PORT_FFMPEG_PADDING 64

typedef struct PortVideoDecoder PortVideoDecoder;

/* fourcc is the AVI stream's biCompression: IV31/IV32 (Indeo 3) or IV50 (Indeo 5), any case. Returns NULL
 * for other codecs. */
PortVideoDecoder *PortVideoDecoder_Open(unsigned int fourcc, int width, int height);

/* Decodes one frame (one AVI sample). Returns 1 with the YUV 4:1:0 planes (Y, U, V; chroma a quarter of the
 * width and height) when it produced a picture, 0 for a frame without one (a null frame: the previous picture
 * stays), or a negative value on an error. The planes stay valid until the next call. */
int PortVideoDecoder_Decode(PortVideoDecoder *decoder, const unsigned char *data, int size,
    const unsigned char *planes[3], int linesize[3]);

void PortVideoDecoder_Close(PortVideoDecoder *decoder);

#endif
