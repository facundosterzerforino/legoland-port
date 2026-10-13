#ifndef MINIWIN_AVIFILE_H
#define MINIWIN_AVIFILE_H

/* [library:movie] The AVIFile calls the game makes, without Video for Windows (miniwin/src/avifile.c): AVI files
 * with an idx1 index, read as AVIFile reads them (file and stream info, formats, samples, key frames). Frames are
 * never decompressed here: AVIStreamGetFrameOpen returns NULL, as on Windows 11 for the game's Indeo movies, and
 * the port decodes them itself (port/port_movie.c). miniwin/redirect.h sends the game's calls here under their
 * Windows names.
 *
 * The handles are a PAVIFILE / PAVISTREAM, the info structures an AVIFILEINFOA / AVISTREAMINFOA, and the results
 * HRESULTs (AVIERR_*) or reference counts. This header isn't included into the game's sources:
 * src/legoland/imports.h declares some of these with no result, which the calls ignore. */

void __stdcall MiniwinAVIFileInit(void);
void __stdcall MiniwinAVIFileExit(void);
long __stdcall MiniwinAVIFileOpenA(void **file, const char *name, int mode, void *handler);
long __stdcall MiniwinAVIFileInfoA(void *file, void *info, int size);
long __stdcall MiniwinAVIFileGetStream(void *file, void **stream, unsigned int type, int n);
long __stdcall MiniwinAVIFileRelease(void *file);
long __stdcall MiniwinAVIStreamInfoA(void *stream, void *info, int size);
long __stdcall MiniwinAVIStreamAddRef(void *stream);
long __stdcall MiniwinAVIStreamRelease(void *stream);
long __stdcall MiniwinAVIStreamReadFormat(void *stream, int pos, void *format, int *size);
long __stdcall MiniwinAVIStreamStart(void *stream);
long __stdcall MiniwinAVIStreamLength(void *stream);
long __stdcall MiniwinAVIStreamRead(void *stream, int start, int samples, void *buffer, int buffer_size, int *bytes,
    int *samples_out);
long __stdcall MiniwinAVIStreamFindSample(void *stream, long pos, long flags);
void *__stdcall MiniwinAVIStreamGetFrameOpen(void *stream, void *format);
void *__stdcall MiniwinAVIStreamGetFrame(void *getframe, int position);
long __stdcall MiniwinAVIStreamGetFrameClose(void *getframe);

#endif
