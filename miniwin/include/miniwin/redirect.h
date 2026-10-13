#ifndef MINIWIN_REDIRECT_H
#define MINIWIN_REDIRECT_H

/* miniwin's versions of Windows and C runtime functions, under the names the game calls. In the port build this
 * header is included first into every game source (CMakeLists.txt, /FI), so the decompiled calls stay unchanged.
 * The system headers come first, so that including them again later changes nothing. */

#include <windows.h>
#include <direct.h>
#include <fcntl.h>
#include <io.h>
#include <stdio.h>
#include <stdlib.h>

#include "miniwin/acm.h"
#include "miniwin/dmusic.h"
#include "miniwin/files.h"
#include "miniwin/timer.h"

/* [library:timer] */
#define GetTickCount MiniwinGetTickCount
#define QueryPerformanceCounter MiniwinQueryPerformanceCounter
#define QueryPerformanceFrequency MiniwinQueryPerformanceFrequency
#define Sleep MiniwinSleep

/* [library:filesystem] */
#define fopen(path, mode) MiniwinFopen(path, mode)
#define remove(path) MiniwinRemove(path)
#define _open MiniwinOpen
#define _mkdir MiniwinMkdir
#undef _findfirst /* the C runtime's own macro (for the 64-bit time variant) */
#define _findfirst MiniwinFindFirst
#define CreateFileA MiniwinCreateFileA
#define GetFileAttributesA MiniwinGetFileAttributesA
#define SetCurrentDirectoryA MiniwinSetCurrentDirectoryA

/* [library:audio] */
#define acmStreamOpen MiniwinAcmStreamOpen
#define acmStreamSize MiniwinAcmStreamSize
#define acmStreamClose MiniwinAcmStreamClose
#define acmStreamPrepareHeader MiniwinAcmStreamPrepareHeader
#define acmStreamConvert MiniwinAcmStreamConvert
#define acmStreamUnprepareHeader MiniwinAcmStreamUnprepareHeader

/* [library:music] the DirectMusic objects: miniwin's with -portable-music, else Windows' */
#define CoInitialize MiniwinCoInitialize
#define CoCreateInstance MiniwinCoCreateInstance

/* [library:movie] (miniwin/avifile.h isn't included: src/legoland/imports.h declares these with no result for
 * some, which the calls ignore) */
#define AVIFileInit MiniwinAVIFileInit
#define AVIFileExit MiniwinAVIFileExit
#define AVIFileOpenA MiniwinAVIFileOpenA
#define AVIFileInfoA MiniwinAVIFileInfoA
#define AVIFileGetStream MiniwinAVIFileGetStream
#define AVIFileRelease MiniwinAVIFileRelease
#define AVIStreamInfoA MiniwinAVIStreamInfoA
#define AVIStreamAddRef MiniwinAVIStreamAddRef
#define AVIStreamRelease MiniwinAVIStreamRelease
#define AVIStreamReadFormat MiniwinAVIStreamReadFormat
#define AVIStreamStart MiniwinAVIStreamStart
#define AVIStreamLength MiniwinAVIStreamLength
#define AVIStreamRead MiniwinAVIStreamRead
#define AVIStreamGetFrameOpen MiniwinAVIStreamGetFrameOpen
#define AVIStreamGetFrame MiniwinAVIStreamGetFrame
#define AVIStreamGetFrameClose MiniwinAVIStreamGetFrameClose

#endif
