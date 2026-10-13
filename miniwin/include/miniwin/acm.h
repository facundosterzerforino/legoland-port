#ifndef MINIWIN_ACM_H
#define MINIWIN_ACM_H

/* [library:audio] The Audio Compression Manager calls the game makes, without Windows' ACM (miniwin/src/acm.c):
 * MS ADPCM and IMA ADPCM (the speech files and the movies' sound) and plain PCM, decoded to 16-bit PCM.
 * miniwin/redirect.h sends the game's calls here under their Windows names.
 *
 * The types are the game's own (src/legoland/imports.h declares the same functions under the redirected names, and
 * the two must agree); the handle is a HACMSTREAM, the header an ACMSTREAMHEADER, the formats WAVEFORMATEXs, and
 * the results MMRESULTs. */

int __stdcall MiniwinAcmStreamOpen(void **stream, void *driver, void *src_format, void *dst_format, void *filter,
    unsigned int callback, unsigned int instance, unsigned int flags);
int __stdcall MiniwinAcmStreamSize(void *stream, unsigned int input_size, unsigned int *output_size,
    unsigned int flags);
void __stdcall MiniwinAcmStreamClose(void *stream, unsigned int flags);
int __stdcall MiniwinAcmStreamPrepareHeader(void *stream, void *header, unsigned int flags);
int __stdcall MiniwinAcmStreamConvert(void *stream, void *header, unsigned int flags);
void __stdcall MiniwinAcmStreamUnprepareHeader(void *stream, void *header, unsigned int flags);

#endif
