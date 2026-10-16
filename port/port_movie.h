#ifndef PORT_MOVIE_H
#define PORT_MOVIE_H

#include <stdio.h>

/* [library:movie] Video decoding for AVI streams that Windows can no longer decompress (Indeo 3 and Indeo 5).
 *
 * PortMovieGetFrame stands in for AVIStreamGetFrame: it returns a packed DIB (a BITMAPINFOHEADER followed by
 * bottom-up 16-bit RGB555 pixels), the format the game asks the Video for Windows decompressor for. The frame
 * stays valid until the next call. Returns NULL when the stream isn't Indeo or the frame can't be decoded. */
void *PortMovieGetFrame(void *stream, long position);

/* Drops the decoder state kept for a stream; call before the stream is released. */
void PortMovieRelease(void *stream);

/* -port-selftest: every movie in FMV\ decoded frame by frame, as the game shows them, and timed. Writes each
 * movie's frames, its frame rate and the decoding time per frame (average and worst) to the report; the times say
 * whether this build decodes fast enough (they differ between machines and aren't compared). Returns the number of
 * problems: movies that don't open or don't decode. */
int PortMovieSelfTest(FILE *report);

#endif
