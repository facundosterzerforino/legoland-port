/* [library:filesystem] miniwin's file paths. See miniwin/files.h. */
#include <SDL3/SDL.h>
#include <direct.h>
#include <fcntl.h>
#include <stdarg.h>
#include <string.h>
#include <sys/stat.h>

#include "miniwin/files.h"

struct Match {
    const char *want;
    char found[MAX_PATH];
    int ok;
};

static SDL_EnumerationResult SDLCALL MatchEntry(void *userdata, const char *dirname, const char *fname) {
    struct Match *m = (struct Match *)userdata;

    (void)dirname;
    if (SDL_strcasecmp(fname, m->want) == 0) {
        SDL_strlcpy(m->found, fname, sizeof(m->found));
        m->ok = 1;
        return SDL_ENUM_SUCCESS;
    }
    return SDL_ENUM_CONTINUE;
}

static int Exists(const char *path) {
    return SDL_GetPathInfo(path, NULL);
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
            struct Match m;
            char candidate[MAX_PATH];

            SDL_snprintf(candidate, sizeof(candidate), "%s%s", buf, part);
            if (!Exists(candidate)) {
                m.want = part;
                m.ok = 0;
                SDL_EnumerateDirectory(len > 0 ? buf : ".", MatchEntry, &m);
                if (!m.ok) {
                    break; /* not on disk (yet): the rest as written */
                }
                SDL_strlcpy(part, m.found, sizeof(part));
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

    return fopen(MiniwinPath(path, buf, sizeof(buf)), mode);
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

    return SetCurrentDirectoryA(MiniwinPath(path, buf, sizeof(buf)));
}
