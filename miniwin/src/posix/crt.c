/* [library:filesystem] The Microsoft C runtime's own functions (low-level files, searches, folders, string
 * helpers, the FPU control word) on the platform's C library. See posix.h. */
#include <ctype.h>
#include <direct.h>
#include <errno.h>
#include <fcntl.h>
#include <fenv.h>
#include <io.h>
#include <malloc.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/stat.h>
#include <unistd.h>

#include "posix.h"

/* ---- low-level files: the CRT's _O_ flags are Windows' values ---- */

int _open(const char *name, int flags, ...) {
    char path[MAX_PATH];
    int oflags;
    int mode = 0644;

    switch (flags & 3) {
    case 1: /* _O_WRONLY */
        oflags = O_WRONLY;
        break;
    case 2: /* _O_RDWR */
        oflags = O_RDWR;
        break;
    default:
        oflags = O_RDONLY;
        break;
    }
    if ((flags & 0x0008) != 0) { /* _O_APPEND */
        oflags |= O_APPEND;
    }
    if ((flags & 0x0100) != 0) { /* _O_CREAT */
        va_list args;
        int pmode;

        oflags |= O_CREAT;
        va_start(args, flags);
        pmode = va_arg(args, int);
        va_end(args);
        mode = (pmode & 0x0080) != 0 ? 0644 : 0444; /* _S_IWRITE */
    }
    if ((flags & 0x0200) != 0) { /* _O_TRUNC */
        oflags |= O_TRUNC;
    }
    if ((flags & 0x0400) != 0) { /* _O_EXCL */
        oflags |= O_EXCL;
    }
    /* _O_BINARY and _O_TEXT: every file is binary here (the code opens its data files as binary) */
    return open(PosixPath(name, path, sizeof(path)), oflags | O_CLOEXEC, mode);
}

int _close(int fd) {
    return close(fd);
}

int _read(int fd, void *buffer, unsigned int count) {
    return (int)read(fd, buffer, count);
}

int _write(int fd, const void *buffer, unsigned int count) {
    return (int)write(fd, buffer, count);
}

__LONG32 _lseek(int fd, __LONG32 offset, int origin) {
    return (__LONG32)lseek(fd, offset, origin);
}

__LONG32 _tell(int fd) {
    return (__LONG32)lseek(fd, 0, SEEK_CUR);
}

__LONG32 _filelength(int fd) {
    struct stat st;

    return fstat(fd, &st) == 0 ? (__LONG32)st.st_size : -1;
}

/* ---- searches ---- */

struct CrtFind {
    char folder[MAX_PATH];
    char **names;
    int count;
    int next;
};

static int NextMatch(struct CrtFind *find, struct _finddata64i32_t *data) {
    while (find->next < find->count) {
        const char *name = find->names[find->next++];
        char full[MAX_PATH * 2];
        struct stat st;

        snprintf(full, sizeof(full), "%s/%s", find->folder, name);
        if (strlen(name) >= sizeof(data->name) || stat(full, &st) != 0) {
            continue;
        }
        memset(data, 0, sizeof(*data));
        data->attrib = S_ISDIR(st.st_mode) ? 0x10 /* _A_SUBDIR */ : 0;
        if ((st.st_mode & S_IWUSR) == 0) {
            data->attrib |= 0x01; /* _A_RDONLY */
        }
        data->time_create = st.st_ctime;
        data->time_access = st.st_atime;
        data->time_write = st.st_mtime;
        data->size = (_fsize_t)st.st_size;
        strcpy(data->name, name);
        return 1;
    }
    return 0;
}

intptr_t _findfirst64i32(const char *spec, struct _finddata64i32_t *data) {
    struct CrtFind *find = (struct CrtFind *)calloc(1, sizeof(struct CrtFind));
    char pattern[MAX_PATH];

    if (find == NULL) {
        errno = ENOMEM;
        return -1;
    }
    if (!PosixSplitSpec(spec, find->folder, sizeof(find->folder), pattern, sizeof(pattern)) ||
        (find->count = PosixListFolder(find->folder, pattern, &find->names)) < 0) {
        free(find);
        errno = ENOENT;
        return -1;
    }
    if (!NextMatch(find, data)) {
        PosixFreeList(find->names, find->count);
        free(find);
        errno = ENOENT;
        return -1;
    }
    return (intptr_t)find;
}

int _findnext64i32(intptr_t handle, struct _finddata64i32_t *data) {
    struct CrtFind *find = (struct CrtFind *)handle;

    if (handle == -1 || find == NULL) {
        errno = EINVAL;
        return -1;
    }
    if (!NextMatch(find, data)) {
        errno = ENOENT;
        return -1;
    }
    return 0;
}

int _findclose(intptr_t handle) {
    struct CrtFind *find = (struct CrtFind *)handle;

    if (handle == -1 || find == NULL) {
        errno = EINVAL;
        return -1;
    }
    PosixFreeList(find->names, find->count);
    free(find);
    return 0;
}

/* ---- folders ---- */

int _chdir(const char *name) {
    char path[MAX_PATH];

    return PosixChdir(PosixPath(name, path, sizeof(path)));
}

char *_getcwd(char *buffer, int size) {
    return PosixGetcwd(buffer, (size_t)size);
}

int _mkdir(const char *name) {
    char path[MAX_PATH];

    return mkdir(PosixPath(name, path, sizeof(path)), 0755);
}

/* ---- strings ---- */

int _stricmp(const char *a, const char *b) {
    return strcasecmp(a, b);
}

int _strcmpi(const char *a, const char *b) {
    return strcasecmp(a, b);
}

int _memicmp(const void *a, const void *b, size_t size) {
    const unsigned char *p = (const unsigned char *)a;
    const unsigned char *q = (const unsigned char *)b;
    size_t i;

    for (i = 0; i < size; i++) {
        int d = tolower(p[i]) - tolower(q[i]);
        if (d != 0) {
            return d;
        }
    }
    return 0;
}

char *_strupr(char *s) {
    char *p;

    for (p = s; *p != '\0'; p++) {
        *p = (char)toupper((unsigned char)*p);
    }
    return s;
}

/* Microsoft's: -1 when it doesn't fit, and then no terminating NUL */
int _snprintf(char *const buffer, const size_t count, const char *const format, ...) {
    va_list args;
    char *tmp;
    int n;

    va_start(args, format);
    n = vsnprintf(NULL, 0, format, args);
    va_end(args);
    if (n < 0) {
        return -1;
    }
    tmp = (char *)malloc((size_t)n + 1);
    if (tmp == NULL) {
        return -1;
    }
    va_start(args, format);
    vsnprintf(tmp, (size_t)n + 1, format, args);
    va_end(args);
    memcpy(buffer, tmp, (size_t)n < count ? (size_t)n + 1 : count);
    free(tmp);
    return (size_t)n < count ? n : -1;
}

void _splitpath(const char *path, char *drive, char *dir, char *fname, char *ext) {
    const char *p = path;
    const char *last_slash = NULL;
    const char *dot = NULL;
    const char *s;

    if (p[0] != '\0' && p[1] == ':') {
        if (drive != NULL) {
            drive[0] = p[0];
            drive[1] = ':';
            drive[2] = '\0';
        }
        p += 2;
    } else if (drive != NULL) {
        drive[0] = '\0';
    }
    for (s = p; *s != '\0'; s++) {
        if (*s == '\\' || *s == '/') {
            last_slash = s;
        }
    }
    for (s = last_slash != NULL ? last_slash + 1 : p; *s != '\0'; s++) {
        if (*s == '.') {
            dot = s;
        }
    }
    if (dir != NULL) {
        size_t n = last_slash != NULL ? (size_t)(last_slash - p + 1) : 0;
        memcpy(dir, p, n);
        dir[n] = '\0';
    }
    s = last_slash != NULL ? last_slash + 1 : p;
    if (fname != NULL) {
        size_t n = dot != NULL ? (size_t)(dot - s) : strlen(s);
        memcpy(fname, s, n);
        fname[n] = '\0';
    }
    if (ext != NULL) {
        strcpy(ext, dot != NULL ? dot : "");
    }
}

/* ---- memory ---- */

size_t _msize(void *block) {
    /* the block's usable size: at least what was asked for (the code only counts allocated bytes with it) */
    return malloc_usable_size(block);
}

/* ---- the FPU control word: precision has no SSE counterpart; the rounding mode is applied ---- */

unsigned int _control87(unsigned int value, unsigned int mask) {
    static unsigned int word = 0x0009001f; /* the default: exceptions masked, 53-bit precision, round to nearest */

    word = (word & ~mask) | (value & mask);
    if ((mask & _MCW_RC) != 0) {
        switch (word & _MCW_RC) {
        case 0x100: /* _RC_DOWN */
            fesetround(FE_DOWNWARD);
            break;
        case 0x200: /* _RC_UP */
            fesetround(FE_UPWARD);
            break;
        case 0x300: /* _RC_CHOP */
            fesetround(FE_TOWARDZERO);
            break;
        default:
            fesetround(FE_TONEAREST);
            break;
        }
    }
    return word;
}
