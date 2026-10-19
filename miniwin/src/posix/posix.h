/* miniwin/src/posix/: the Win32 functions the game, the port and miniwin call, for non-Windows builds (ROADMAP.md
 * phase 8, step 4). Each does what Windows' does, for what the code asks of it; what has no counterpart here (CD
 * drives, printers, DLLs) reports failure the way Windows does when the thing isn't there. */
#ifndef MINIWIN_POSIX_H
#define MINIWIN_POSIX_H

#include <windows.h>
#include <pthread.h>
#include <stdio.h>

/* Kernel objects: every HANDLE these functions return points to one. One lock and one condition for all of them,
 * which every change that can end a wait broadcasts. */
enum PosixKind {
    POSIX_FILE = 0x46494c45,
    POSIX_EVENT,
    POSIX_MUTEX,
    POSIX_THREAD,
    POSIX_FIND,
};

struct PosixObject {
    enum PosixKind kind;
    int closed; /* CloseHandle'd: freed once nothing else uses it (a thread still running) */
};

extern pthread_mutex_t posix_objects_lock;
extern pthread_cond_t posix_objects_changed;

/* [library:filesystem] Read-only files through a cache of their blocks (readcache.c): ReadFile's read-only handles
 * and MiniwinFopen's "r" files. PosixReadCacheRead reads size bytes from pos (fewer at the end of the file; -1 and
 * errno on an error). PosixFopenRead is fopen(path, mode) for reading, through such a cache. */
struct PosixReadCache;
struct PosixReadCache *PosixReadCacheNew(int fd);
long long PosixReadCacheRead(struct PosixReadCache *c, long long pos, void *buffer, size_t size);
void PosixReadCacheFree(struct PosixReadCache *c);
FILE *PosixFopenRead(const char *path, const char *mode);
int PosixOpenRead(const char *path); /* open(path, O_RDONLY), in a browser the faster way (readcache.c) */

void *PosixNewObject(enum PosixKind kind, size_t size);
void *PosixObjectOf(HANDLE handle, enum PosixKind kind); /* NULL if handle isn't one of that kind */
BOOL PosixCloseObject(struct PosixObject *o); /* an event, mutex or thread's CloseHandle */

/* a Windows search spec ("data\*.lls") as the folder on disk and the pattern */
int PosixSplitSpec(const char *spec, char *folder, size_t folder_size, char *pattern, size_t pattern_size);

/* A folder's names that match a Windows wildcard pattern, in the order Windows' file systems list them (NTFS and
 * the CD's: by their characters in upper case), malloc'd; PosixFreeList frees them. -1 if the folder isn't there. */
int PosixListFolder(const char *folder, const char *pattern, char ***names);
void PosixFreeList(char **names, int count);

/* a GDI device context drawing on width x height RGB565 pixels, pitch bytes a row (DirectDraw's GetDC); deleted with
 * DeleteDC */
HDC PosixSurfaceDc(void *bits, int width, int height, int pitch);

/* an alternate signal stack for this thread, so that a fault on a full stack is still reported (seh.c) */
void PosixThreadSignalStack(void);

/* GetLastError's value, per thread; from errno for the file functions */
void PosixSetError(DWORD error);
DWORD PosixErrorFromErrno(int error);

/* a Windows path (backslashes, any case) as it is on disk: miniwin's MiniwinPath */
const char *PosixPath(const char *path, char *buf, size_t size);

/* The current folder: chdir that remembers it (absolute), and getcwd from what it remembered. In the browser getcwd
 * in a folder of the page's storage (WASMFS on OPFS) gives "//LEGOLAND" for "/opfs/LEGOLAND", which chdir refuses:
 * LoadStringTable went back to it and gave up before reading a string (every text from GetString missing). */
int PosixChdir(const char *path);
char *PosixGetcwd(char *buffer, size_t size);

/* the monotonic clock, in milliseconds */
unsigned long long PosixMilliseconds(void);

/* Windows' FILETIME (100 ns since 1601) from Unix time */
void PosixFileTime(long long seconds, long nanoseconds, FILETIME *out);

#endif
