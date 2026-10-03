/*
 * [library:movie] The port's entry points into FFmpeg's Indeo 5 decoder (this folder).
 *
 * This file is part of the port's copy of FFmpeg's Indeo 5 decoder and, like it, is licensed under the
 * GNU Lesser General Public License, version 2.1 or later (see COPYING.LGPLv2.1).
 */

#ifndef PORT_INDEO5_H
#define PORT_INDEO5_H

/* Bytes of zeros the caller must place after each frame's data: the bit reader looks a few bytes ahead. */
#define PORT_INDEO5_PADDING 64

typedef struct PortIndeo5 PortIndeo5;

PortIndeo5 *PortIndeo5_Open(int width, int height);

/* Decodes one frame (one AVI sample). Returns 1 with the YUV 4:1:0 planes (Y, U, V; chroma a quarter of the
 * width and height) when it produced a picture, 0 for a frame without one (a null frame: the previous picture
 * stays), or a negative value on an error. The planes stay valid until the next call. */
int PortIndeo5_Decode(PortIndeo5 *decoder, const unsigned char *data, int size, const unsigned char *planes[3],
    int linesize[3]);

void PortIndeo5_Close(PortIndeo5 *decoder);

#endif
