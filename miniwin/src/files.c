/* [library:filesystem] miniwin's file paths. See miniwin/files.h. */
#include <SDL3/SDL.h>
#include <direct.h>
#include <fcntl.h>
#include <stdarg.h>
#include <string.h>
#include <sys/stat.h>

#include "miniwin/files.h"
#ifndef _WIN32
#include "posix/posix.h"
#endif

static int Exists(const char *path) {
    return SDL_GetPathInfo(path, NULL);
}

/* The folders' listings, kept: the game names its files in another case than the disk's ("imusic\\segtheme1.sgt" for
 * IMusic/SegTheme1.sgt), and each such name was matched by listing its folders again. Cheap on a desktop; in a
 * browser a listing is many round trips to the page's storage, and the music thread's 243 files took seconds. A name
 * the listing doesn't have (a file made since) is looked up on disk again, and the listing redone. */
struct Listing {
    char dir[MAX_PATH];
    char **names;
    int count;
    int capacity;
    struct Listing *next;
};

static struct Listing *listings;
static SDL_Mutex *listings_lock;
static SDL_SpinLock listings_lock_made;

static SDL_EnumerationResult SDLCALL AddName(void *userdata, const char *dirname, const char *fname) {
    struct Listing *l = (struct Listing *)userdata;

    (void)dirname;
    if (l->count == l->capacity) {
        int capacity = l->capacity != 0 ? l->capacity * 2 : 64;
        char **names = (char **)SDL_realloc(l->names, sizeof(char *) * (size_t)capacity);

        if (names == NULL) {
            return SDL_ENUM_FAILURE;
        }
        l->names = names;
        l->capacity = capacity;
    }
    if ((l->names[l->count] = SDL_strdup(fname)) != NULL) {
        l->count++;
    }
    return SDL_ENUM_CONTINUE;
}

static void LockListings(void) {
    SDL_LockSpinlock(&listings_lock_made);
    if (listings_lock == NULL) {
        listings_lock = SDL_CreateMutex();
    }
    SDL_UnlockSpinlock(&listings_lock_made);
    SDL_LockMutex(listings_lock);
}

/* the name in dir, in the disk's case, into found: from the kept listing, or (refresh) a new one */
static int MatchKept(const char *dir, const char *want, char *found, size_t size, int refresh) {
    struct Listing *l;
    int i;
    int ok = 0;

    LockListings();
    for (l = listings; l != NULL && SDL_strcmp(l->dir, dir) != 0; l = l->next) {
    }
    if (l == NULL && refresh) {
        l = (struct Listing *)SDL_calloc(1, sizeof(*l));
        if (l != NULL) {
            SDL_strlcpy(l->dir, dir, sizeof(l->dir));
            l->next = listings;
            listings = l;
        }
    }
    if (l != NULL && refresh) {
        for (i = 0; i < l->count; i++) {
            SDL_free(l->names[i]);
        }
        l->count = 0;
        SDL_EnumerateDirectory(dir, AddName, l);
    }
    if (l != NULL) {
        for (i = 0; i < l->count && !ok; i++) {
            if (SDL_strcmp(l->names[i], want) == 0) {
                ok = 1;
                SDL_strlcpy(found, l->names[i], size);
            }
        }
        for (i = 0; i < l->count && !ok; i++) {
            if (SDL_strcasecmp(l->names[i], want) == 0) {
                ok = 1;
                SDL_strlcpy(found, l->names[i], size);
            }
        }
    }
    SDL_UnlockMutex(listings_lock);
    return ok;
}

static void ForgetListings(void) {
    LockListings();
    while (listings != NULL) {
        struct Listing *l = listings;
        int i;

        listings = l->next;
        for (i = 0; i < l->count; i++) {
            SDL_free(l->names[i]);
        }
        SDL_free(l->names);
        SDL_free(l);
    }
    SDL_UnlockMutex(listings_lock);
}

const char *MiniwinPath(const char *path, char *buf, size_t size) {
    char out[MAX_PATH];
    char part[MAX_PATH];
    size_t len = 0;
    const char *p;
    char *s;

    if (size == 0) {
        return buf;
    }
    if (path == NULL || strncmp(path, "\\\\", 2) == 0 || strlen(path) >= sizeof(out)) {
        SDL_strlcpy(buf, path != NULL ? path : "", size);
        return buf;
    }
    SDL_strlcpy(out, path, sizeof(out));
    for (s = out; *s != '\0'; s++) {
        if (*s == '\\') {
            *s = '/';
        }
    }
#ifdef LEGOLAND_WEB_BROWSER
    {
        /* the music's files: in the page's memory, when the page copied them there (port/web/shell.html) */
        static int in_memory = -1;

        if (in_memory < 0) {
            in_memory = Exists("/imusic-mem");
        }
        if (in_memory && SDL_strncasecmp(out, "imusic", 6) == 0 && (out[6] == '/' || out[6] == '\0')) {
            char moved[MAX_PATH];

            SDL_snprintf(moved, sizeof(moved), "/imusic-mem%s", out + 6);
            SDL_strlcpy(out, moved, sizeof(out));
        }
    }
#endif
    if (out[0] == '\0' || Exists(out)) {
        SDL_strlcpy(buf, out, size);
        return buf;
    }
    /* walk the parts, keeping the prefix that has been matched so far in buf */
    buf[0] = '\0';
    p = out;
    if (*p == '/') {
        SDL_strlcpy(buf, "/", size);
        p++;
    } else if (p[0] != '\0' && p[1] == ':') {
        SDL_strlcpy(buf, out, size < 4 ? size : 4); /* "C:/" */
        p += p[2] == '/' ? 3 : 2;
    }
    len = strlen(buf);
    while (*p != '\0') {
        const char *end = strchr(p, '/');
        size_t n = end != NULL ? (size_t)(end - p) : strlen(p);

        if (n >= sizeof(part)) {
            break;
        }
        memcpy(part, p, n);
        part[n] = '\0';
        if (n > 0 && strcmp(part, ".") != 0 && strcmp(part, "..") != 0 && strpbrk(part, "*?") == NULL) {
            char candidate[MAX_PATH];
            char found[MAX_PATH];
            const char *dir = len > 0 ? buf : ".";

            if (MatchKept(dir, part, found, sizeof(found), 0)) {
                SDL_strlcpy(part, found, sizeof(part));
            } else {
                SDL_snprintf(candidate, sizeof(candidate), "%s%s", buf, part);
                if (!Exists(candidate)) {
                    if (!MatchKept(dir, part, found, sizeof(found), 1)) {
                        break; /* not on disk (yet): the rest as written */
                    }
                    SDL_strlcpy(part, found, sizeof(part));
                }
            }
        }
        SDL_snprintf(buf + len, size - len, "%s%s", part, end != NULL ? "/" : "");
        len = strlen(buf);
        p = end != NULL ? end + 1 : p + n;
    }
    if (*p != '\0') {
        SDL_snprintf(buf + len, size - len, "%s", p);
    }
    return buf;
}

FILE *MiniwinFopen(const char *path, const char *mode) {
    char buf[MAX_PATH];

#if defined(__ANDROID__)
    FILE *f = fopen(MiniwinPath(path, buf, sizeof(buf)), mode);

    /* read in 64 KB blocks */
    if (f != NULL && mode[0] == 'r' && strchr(mode, '+') == NULL) {
        setvbuf(f, NULL, _IOFBF, 65536);
    }
    return f;
#else
#ifndef _WIN32
    /* read through a cache of the file's blocks (posix/readcache.c): in a browser each read is a round trip to the
     * page's storage, and Emscripten's stdio reads 1 KB at a time (setvbuf can't make it more without a buffer of
     * the caller's, which fclose wouldn't free) */
    if (mode[0] == 'r' && strchr(mode, '+') == NULL) {
        return PosixFopenRead(MiniwinPath(path, buf, sizeof(buf)), mode);
    }
#endif
    return fopen(MiniwinPath(path, buf, sizeof(buf)), mode);
#endif
}

int MiniwinOpen(const char *path, int flags, ...) {
    char buf[MAX_PATH];
    int pmode = 0;

    if ((flags & _O_CREAT) != 0) {
        va_list args;
        va_start(args, flags);
        pmode = va_arg(args, int);
        va_end(args);
    }
    return _open(MiniwinPath(path, buf, sizeof(buf)), flags, pmode);
}

int MiniwinRemove(const char *path) {
    char buf[MAX_PATH];

    return remove(MiniwinPath(path, buf, sizeof(buf)));
}

int MiniwinMkdir(const char *path) {
    char buf[MAX_PATH];

    return _mkdir(MiniwinPath(path, buf, sizeof(buf)));
}

intptr_t MiniwinFindFirst(const char *spec, struct _finddata_t *data) {
    char buf[MAX_PATH];

    return _findfirst(MiniwinPath(spec, buf, sizeof(buf)), data);
}

HANDLE MiniwinCreateFileA(const char *path, DWORD access, DWORD share, LPSECURITY_ATTRIBUTES security,
    DWORD disposition, DWORD flags, HANDLE template_file) {
    char buf[MAX_PATH];

    return CreateFileA(
        MiniwinPath(path, buf, sizeof(buf)), access, share, security, disposition, flags, template_file);
}

DWORD MiniwinGetFileAttributesA(const char *path) {
    char buf[MAX_PATH];

    return GetFileAttributesA(MiniwinPath(path, buf, sizeof(buf)));
}

BOOL MiniwinSetCurrentDirectoryA(const char *path) {
    char buf[MAX_PATH];

    ForgetListings(); /* kept by relative names */
    return SetCurrentDirectoryA(MiniwinPath(path, buf, sizeof(buf)));
}
