#ifndef PORT_MOVIE_H
#define PORT_MOVIE_H

/* [library:movie] Video decoding for AVI streams that Windows can no longer decompress (Indeo 5, IV50).
 *
 * PortMovieGetFrame stands in for AVIStreamGetFrame: it returns a packed DIB (a BITMAPINFOHEADER followed by
 * bottom-up 16-bit RGB555 pixels), the format the game asks the Video for Windows decompressor for. The frame
 * stays valid until the next call. Returns NULL when the stream is not IV50 or the frame can't be decoded. */
void *PortMovieGetFrame(void *stream, long position);

/* Drops the decoder state kept for a stream; call before the stream is released. */
void PortMovieRelease(void *stream);

#endif
