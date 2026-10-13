#ifndef MINIWIN_FILES_H
#define MINIWIN_FILES_H

/* [library:filesystem] miniwin's file paths (miniwin/src/files.c). The game names its files Windows-style:
 * backslashes, relative to the data folder, and not always in the case they have on disk. Every path it opens goes
 * through MiniwinPath first, by way of the wrappers below (miniwin/redirect.h sends the game's calls to them). */

#include <windows.h>
#include <io.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

/* Translates a game path into buf (size bytes) and returns buf: backslashes become '/', and each part of the path
 * that exists on disk under another case is spelled as it is there (systems with case-sensitive files: Linux,
 * Android). A path that exists as written is returned as is; parts that don't exist yet (a file about to be
 * created) and wildcards are kept as written. Device paths (\\.\...) are left alone. */
const char *MiniwinPath(const char *path, char *buf, size_t size);

/* The C runtime and Windows file functions the game calls, with the path translated first. */
FILE *MiniwinFopen(const char *path, const char *mode);
int MiniwinOpen(const char *path, int flags, ...);
int MiniwinRemove(const char *path);
int MiniwinMkdir(const char *path);
intptr_t MiniwinFindFirst(const char *spec, struct _finddata_t *data);
HANDLE MiniwinCreateFileA(const char *path, DWORD access, DWORD share, LPSECURITY_ATTRIBUTES security,
    DWORD disposition, DWORD flags, HANDLE template_file);
DWORD MiniwinGetFileAttributesA(const char *path);
BOOL MiniwinSetCurrentDirectoryA(const char *path);

#endif
