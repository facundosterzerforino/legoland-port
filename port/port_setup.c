/* [library:filesystem] The game's data folder: finding it, and building it from the LEGOLAND CD. See port_setup.h. */
#include "port_setup.h"

#include <SDL3/SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

#include "blast/blast.h"

#define PATH_SIZE 1024

/* ---- folders, matched case-insensitively (a CD's names may be in either case) ---- */

typedef struct Lookup {
    const char *want;
    char found[256];
} Lookup;

static SDL_EnumerationResult SDLCALL LookupEntry(void *userdata, const char *dirname, const char *fname) {
    Lookup *l = (Lookup *)userdata;

    (void)dirname;
    if (SDL_strcasecmp(fname, l->want) == 0) {
        SDL_strlcpy(l->found, fname, sizeof(l->found));
        return SDL_ENUM_SUCCESS;
    }
    return SDL_ENUM_CONTINUE;
}

/* folder/name with name's actual case; 0 when there is no such entry */
static int Child(const char *folder, const char *name, char *out, size_t size) {
    Lookup l;

    l.want = name;
    l.found[0] = '\0';
    SDL_EnumerateDirectory(folder, LookupEntry, &l);
    if (l.found[0] == '\0') {
        return 0;
    }
    SDL_snprintf(out, size, "%s%s%s", folder,
        folder[0] != '\0' && folder[SDL_strlen(folder) - 1] != '/' && folder[SDL_strlen(folder) - 1] != '\\' ? "/"
                                                                                                             : "",
        l.found);
    return 1;
}

static int IsFolder(const char *path) {
    SDL_PathInfo info;

    return SDL_GetPathInfo(path, &info) && info.type == SDL_PATHTYPE_DIRECTORY;
}

static int IsFile(const char *path) {
    SDL_PathInfo info;

    return SDL_GetPathInfo(path, &info) && info.type == SDL_PATHTYPE_FILE;
}

/* folder has a/b (a folder, then a file in it) */
static int HasFileIn(const char *folder, const char *sub, const char *file) {
    char a[PATH_SIZE];
    char b[PATH_SIZE];

    return Child(folder, sub, a, sizeof(a)) && IsFolder(a) && Child(a, file, b, sizeof(b)) && IsFile(b);
}

static int HasFolder(const char *folder, const char *sub) {
    char a[PATH_SIZE];

    return Child(folder, sub, a, sizeof(a)) && IsFolder(a);
}

int PortSetupKind(const char *folder) {
    char z[PATH_SIZE];
    int big = HasFileIn(folder, "volumes", "Legoland.res") && HasFolder(folder, "speech") && HasFolder(folder, "FMV");

    if (!big) {
        return PORT_SETUP_NONE;
    }
    if (HasFolder(folder, "IMusic") && HasFolder(folder, "zbuffers")) {
        return PORT_SETUP_INSTALLED;
    }
    if (Child(folder, "main.z", z, sizeof(z)) && IsFile(z)) {
        return PORT_SETUP_CD;
    }
    return PORT_SETUP_NONE;
}

const char *PortSetupExeFolder(void) {
    const char *base = SDL_GetBasePath();

    return base != NULL ? base : "./";
}

int PortSetupFindData(void) {
    if (PortSetupKind(".") == PORT_SETUP_INSTALLED) {
        return 1;
    }
    if (PortSetupKind(PortSetupExeFolder()) == PORT_SETUP_INSTALLED) {
#ifdef _WIN32
        return SetCurrentDirectoryA(PortSetupExeFolder()) != 0;
#else
        return chdir(PortSetupExeFolder()) == 0;
#endif
    }
    return 0;
}

/* ---- the work: a list of files to write, then writing them ---- */

typedef struct Job {
    char *source; /* a file to copy, or NULL for one of main.z's */
    char *target; /* relative to the target folder, '/'-separated */
    unsigned long long size;
    unsigned long z_offset, z_size; /* in main.z */
} Job;

typedef struct Work {
    Job *jobs;
    size_t count, cap;
    unsigned long long total, done;
    const PortSetupProgress *progress;
    char *error;
    unsigned int error_size;
} Work;

static int Fail(Work *w, const char *what, const char *path) {
    SDL_snprintf(w->error, w->error_size, "%s: %s", what, path);
    return 0;
}

static int AddFile(Work *w, const char *source, const char *target, unsigned long long size, unsigned long z_offset,
    unsigned long z_size) {
    Job *j;

    if (w->count == w->cap) {
        size_t cap = w->cap ? w->cap * 2 : 256;
        Job *grown = (Job *)realloc(w->jobs, cap * sizeof(Job));

        if (grown == NULL) {
            return Fail(w, "Out of memory", target);
        }
        w->jobs = grown;
        w->cap = cap;
    }
    j = &w->jobs[w->count++];
    j->source = source != NULL ? SDL_strdup(source) : NULL;
    j->target = SDL_strdup(target);
    j->size = size;
    j->z_offset = z_offset;
    j->z_size = z_size;
    w->total += size;
    return 1;
}

typedef struct Walk {
    Work *work;
    const char *rel; /* the target path of this folder */
    int ok;
} Walk;

static int AddTree(Work *w, const char *folder, const char *rel);

static SDL_EnumerationResult SDLCALL WalkEntry(void *userdata, const char *dirname, const char *fname) {
    Walk *k = (Walk *)userdata;
    char path[PATH_SIZE];
    char rel[PATH_SIZE];
    SDL_PathInfo info;

    SDL_snprintf(path, sizeof(path), "%s%s", dirname, fname);
    SDL_snprintf(rel, sizeof(rel), "%s%s%s", k->rel, k->rel[0] != '\0' ? "/" : "", fname);
    if (!SDL_GetPathInfo(path, &info)) {
        return SDL_ENUM_CONTINUE;
    }
    if (info.type == SDL_PATHTYPE_DIRECTORY) {
        if (!AddTree(k->work, path, rel)) {
            k->ok = 0;
            return SDL_ENUM_FAILURE;
        }
    } else if (info.type == SDL_PATHTYPE_FILE) {
        if (!AddFile(k->work, path, rel, info.size, 0, 0)) {
            k->ok = 0;
            return SDL_ENUM_FAILURE;
        }
    }
    return SDL_ENUM_CONTINUE;
}

static int AddTree(Work *w, const char *folder, const char *rel) {
    Walk k;

    k.work = w;
    k.rel = rel;
    k.ok = 1;
    if (!SDL_EnumerateDirectory(folder, WalkEntry, &k) && k.ok) {
        return Fail(w, "Can't read the folder", folder);
    }
    return k.ok;
}

/* main.z (InstallShield 3): a header, the data, then a table of folders and a table of files. */
static int AddArchive(Work *w, const unsigned char *z, size_t size, const char *path) {
    unsigned long files, dirs, dir_off, file_off, p, i;
    char names[64][256];

#define U16(o) ((unsigned long)z[(o)] | (unsigned long)z[(o) + 1] << 8)
#define U32(o) (U16(o) | U16((o) + 2) << 16)
    if (size < 0xff || U32(0) != 0x8C655D13UL) {
        return Fail(w, "Not an InstallShield 3 archive", path);
    }
    files = U16(0x0C);
    dir_off = U32(0x29);
    dirs = U16(0x31);
    file_off = U32(0x33);
    if (dirs > 64 || dir_off >= size || file_off >= size) {
        return Fail(w, "Damaged archive", path);
    }
    p = dir_off;
    for (i = 0; i < dirs; i++) {
        unsigned long entry, len;
        char *s;

        if (p + 6 > size) {
            return Fail(w, "Damaged archive", path);
        }
        entry = U16(p + 2);
        len = U16(p + 4);
        if (len >= sizeof(names[0]) || p + 6 + len > size || entry == 0) {
            return Fail(w, "Damaged archive", path);
        }
        memcpy(names[i], z + p + 6, len);
        names[i][len] = '\0';
        for (s = names[i]; *s != '\0'; s++) {
            *s = *s == '\\' ? '/' : *s;
        }
        p += entry;
    }
    p = file_off;
    for (i = 0; i < files; i++) {
        unsigned long dir, orig, comp, off, entry, len;
        char name[256];
        char rel[PATH_SIZE];

        if (p + 30 > size) {
            return Fail(w, "Damaged archive", path);
        }
        dir = U16(p + 1);
        orig = U32(p + 3);
        comp = U32(p + 7);
        off = U32(p + 11);
        entry = U16(p + 23);
        len = z[p + 29];
        if (dir >= dirs || entry == 0 || p + 30 + len > size || off + comp > size || len >= sizeof(name)) {
            return Fail(w, "Damaged archive", path);
        }
        memcpy(name, z + p + 30, len);
        name[len] = '\0';
        SDL_snprintf(rel, sizeof(rel), "%s%s%s", names[dir], names[dir][0] != '\0' ? "/" : "", name);
        if (!AddFile(w, NULL, rel, orig, off, comp)) {
            return 0;
        }
        p += entry;
    }
#undef U32
#undef U16
    return 1;
}

/* creates the folders of target/rel */
static int MakeParents(Work *w, const char *target, const char *rel) {
    char path[PATH_SIZE];
    char *s;

    SDL_snprintf(path, sizeof(path), "%s/%s", target, rel);
    for (s = path + SDL_strlen(target) + 1; *s != '\0'; s++) {
        if (*s == '/') {
            *s = '\0';
            if (!IsFolder(path) && !SDL_CreateDirectory(path)) {
                return Fail(w, "Can't create the folder", path);
            }
            *s = '/';
        }
    }
    return 1;
}

typedef struct Blast {
    const unsigned char *in;
    unsigned long left;
    FILE *out;
    Work *work;
    unsigned long long written;
} Blast;

static unsigned BlastIn(void *how, unsigned char **buf) {
    Blast *b = (Blast *)how;
    unsigned n = b->left > 0x10000 ? 0x10000 : (unsigned)b->left;

    *buf = (unsigned char *)b->in;
    b->in += n;
    b->left -= n;
    return n;
}

static int BlastOut(void *how, unsigned char *buf, unsigned len) {
    Blast *b = (Blast *)how;

    b->written += len;
    return fwrite(buf, 1, len, b->out) != len;
}

static void Report(Work *w, const char *file) {
    if (w->progress != NULL && w->progress->report != NULL) {
        w->progress->report(w->progress->ctx, w->done, w->total, file);
    }
}

static int Cancelled(Work *w) {
    return w->progress != NULL && w->progress->cancelled != NULL && w->progress->cancelled(w->progress->ctx);
}

static int RunJob(Work *w, const Job *j, const char *target, const unsigned char *z) {
    char path[PATH_SIZE];
    FILE *out;
    int ok = 1;

    if (!MakeParents(w, target, j->target)) {
        return 0;
    }
    SDL_snprintf(path, sizeof(path), "%s/%s", target, j->target);
    Report(w, j->target);
    out = fopen(path, "wb");
    if (out == NULL) {
        return Fail(w, "Can't write", path);
    }
    if (j->source == NULL) {
        Blast b;

        b.in = z + j->z_offset;
        b.left = j->z_size;
        b.out = out;
        b.work = w;
        b.written = 0;
        if (blast(BlastIn, &b, BlastOut, &b, NULL, NULL) != 0 || b.written != j->size) {
            ok = Fail(w, "Damaged file in main.z", j->target);
        }
        w->done += j->size;
    } else {
        FILE *in = fopen(j->source, "rb");
        static unsigned char buf[1 << 20];
        size_t n;

        if (in == NULL) {
            fclose(out);
            return Fail(w, "Can't read", j->source);
        }
        while ((n = fread(buf, 1, sizeof(buf), in)) > 0) {
            if (fwrite(buf, 1, n, out) != n) {
                ok = Fail(w, "Can't write (is the disk full?)", path);
                break;
            }
            w->done += n;
            Report(w, j->target);
            if (Cancelled(w)) {
                ok = Fail(w, "Cancelled", "");
                break;
            }
        }
        if (ok && ferror(in)) {
            ok = Fail(w, "Can't read (is the CD readable?)", j->source);
        }
        fclose(in);
    }
    if (fclose(out) != 0 && ok) {
        ok = Fail(w, "Can't write (is the disk full?)", path);
    }
    return ok;
}

int PortSetupInstall(const char *source, const char *target, const PortSetupProgress *progress, char *error,
    unsigned int error_size) {
    Work w;
    unsigned char *z = NULL;
    size_t zsize = 0;
    int kind = PortSetupKind(source);
    int ok = 1;
    size_t i;

    memset(&w, 0, sizeof(w));
    w.progress = progress;
    w.error = error;
    w.error_size = error_size;
    error[0] = '\0';
    if (kind == PORT_SETUP_NONE) {
        return Fail(&w, "Not the LEGOLAND CD or a LEGOLAND folder", source);
    }
    if (!IsFolder(target) && !SDL_CreateDirectory(target)) {
        return Fail(&w, "Can't create the folder", target);
    }
    if (kind == PORT_SETUP_CD) {
        static const char *const folders[] = {"FMV", "speech", "volumes"};
        char path[PATH_SIZE];

        Child(source, "main.z", path, sizeof(path));
        z = (unsigned char *)SDL_LoadFile(path, &zsize);
        if (z == NULL) {
            return Fail(&w, "Can't read", path);
        }
        ok = AddArchive(&w, z, zsize, path);
        for (i = 0; ok && i < sizeof(folders) / sizeof(folders[0]); i++) {
            ok = Child(source, folders[i], path, sizeof(path)) && AddTree(&w, path, folders[i]);
        }
    } else {
        ok = AddTree(&w, source, "");
    }
    for (i = 0; ok && i < w.count; i++) {
        ok = RunJob(&w, &w.jobs[i], target, z) && !Cancelled(&w);
        if (ok == 0 && error[0] == '\0') {
            Fail(&w, "Cancelled", "");
        }
    }
    if (ok) {
        w.done = w.total;
        Report(&w, "");
    }
    for (i = 0; i < w.count; i++) {
        SDL_free(w.jobs[i].source);
        SDL_free(w.jobs[i].target);
    }
    free(w.jobs);
    SDL_free(z);
    return ok;
}
