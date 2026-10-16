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
#define SECTOR 2048

/* ---- a source: a folder (a CD drive, a copy of the CD, an install) or a CD image read directly ---- */

typedef struct Source {
    SDL_IOStream *iso; /* NULL for a folder */
} Source;

/* a folder or a file of a source */
typedef struct Node {
    char name[256];
    int dir;
    unsigned long long size;
    char path[PATH_SIZE]; /* folder sources: its path */
    unsigned long long offset; /* CD images: where its data (a file) or its records (a folder) start */
} Node;

typedef int (*EachFn)(void *ctx, const Node *child); /* 0 stops */

static int IsFolder(const char *path) {
    SDL_PathInfo info;

    return SDL_GetPathInfo(path, &info) && info.type == SDL_PATHTYPE_DIRECTORY;
}

static int IsFile(const char *path) {
    SDL_PathInfo info;

    return SDL_GetPathInfo(path, &info) && info.type == SDL_PATHTYPE_FILE;
}

static int ReadAt(SDL_IOStream *io, unsigned long long offset, void *buf, size_t n) {
    return SDL_SeekIO(io, (Sint64)offset, SDL_IO_SEEK_SET) == (Sint64)offset && SDL_ReadIO(io, buf, n) == n;
}

/* An ISO 9660 image: the Joliet descriptor when there is one (the names as Windows shows them), else the primary
 * one (8.3 upper-case names). Fills the root folder. */
static int OpenImage(Source *s, const char *path, Node *root) {
    unsigned char d[SECTOR];
    unsigned long long primary = 0, joliet = 0;
    unsigned long sector;
    const unsigned char *r;

    s->iso = SDL_IOFromFile(path, "rb");
    if (s->iso == NULL) {
        return 0;
    }
    for (sector = 16; sector < 64; sector++) {
        if (!ReadAt(s->iso, (unsigned long long)sector * SECTOR, d, sizeof(d)) || memcmp(d + 1, "CD001", 5) != 0) {
            break;
        }
        if (d[0] == 1 && primary == 0) {
            primary = (unsigned long long)sector * SECTOR;
        } else if (d[0] == 2 && d[88] == '%' && d[89] == '/' && (d[90] == '@' || d[90] == 'C' || d[90] == 'E')) {
            joliet = (unsigned long long)sector * SECTOR;
        } else if (d[0] == 255) {
            break;
        }
    }
    if (primary == 0 || !ReadAt(s->iso, joliet != 0 ? joliet : primary, d, sizeof(d))) {
        SDL_CloseIO(s->iso);
        s->iso = NULL;
        return 0;
    }
    r = d + 156; /* the root folder's record */
    memset(root, 0, sizeof(*root));
    root->dir = 1;
    root->offset = (unsigned long long)(r[2] | r[3] << 8 | r[4] << 16 | (unsigned long)r[5] << 24) * SECTOR;
    root->size = r[10] | r[11] << 8 | r[12] << 16 | (unsigned long)r[13] << 24;
    root->path[0] = joliet != 0 ? 'J' : 'P'; /* which names the records have */
    return 1;
}

static int OpenSource(Source *s, const char *path, Node *root) {
    memset(s, 0, sizeof(*s));
    if (IsFile(path)) {
        return OpenImage(s, path, root);
    }
    if (!IsFolder(path)) {
        return 0;
    }
    memset(root, 0, sizeof(*root));
    root->dir = 1;
    SDL_strlcpy(root->path, path, sizeof(root->path));
    return 1;
}

static void CloseSource(Source *s) {
    if (s->iso != NULL) {
        SDL_CloseIO(s->iso);
        s->iso = NULL;
    }
}

typedef struct FolderWalk {
    const Node *dir;
    EachFn fn;
    void *ctx;
    int stopped;
} FolderWalk;

static SDL_EnumerationResult SDLCALL FolderEntry(void *userdata, const char *dirname, const char *fname) {
    FolderWalk *w = (FolderWalk *)userdata;
    SDL_PathInfo info;
    Node n;

    memset(&n, 0, sizeof(n));
    SDL_strlcpy(n.name, fname, sizeof(n.name));
    SDL_snprintf(n.path, sizeof(n.path), "%s%s", dirname, fname);
    if (!SDL_GetPathInfo(n.path, &info) ||
        (info.type != SDL_PATHTYPE_DIRECTORY && info.type != SDL_PATHTYPE_FILE)) {
        return SDL_ENUM_CONTINUE;
    }
    n.dir = info.type == SDL_PATHTYPE_DIRECTORY;
    n.size = info.size;
    if (!w->fn(w->ctx, &n)) {
        w->stopped = 1;
        return SDL_ENUM_FAILURE;
    }
    return SDL_ENUM_CONTINUE;
}

/* the records of a CD image folder */
static int ImageChildren(Source *s, const Node *dir, EachFn fn, void *ctx) {
    unsigned char *buf;
    unsigned long long size = dir->size, i = 0;
    int joliet = dir->path[0] == 'J';
    int ok = 1;

    if (size == 0 || size > (64u << 20)) {
        return 0;
    }
    buf = (unsigned char *)malloc((size_t)size);
    if (buf == NULL || !ReadAt(s->iso, dir->offset, buf, (size_t)size)) {
        free(buf);
        return 0;
    }
    while (ok && i < size) {
        const unsigned char *r = buf + i;
        unsigned len = r[0], nlen, k, o = 0;
        Node n;

        if (len == 0) {
            i = (i / SECTOR + 1) * SECTOR; /* records don't cross sectors */
            continue;
        }
        if (len < 34 || i + len > size || 33u + r[32] > len) {
            ok = 0;
            break;
        }
        nlen = r[32];
        i += len;
        if (nlen == 1 && (r[33] == 0 || r[33] == 1)) {
            continue; /* . and .. */
        }
        memset(&n, 0, sizeof(n));
        n.dir = (r[25] & 2) != 0;
        n.offset = (unsigned long long)(r[2] | r[3] << 8 | r[4] << 16 | (unsigned long)r[5] << 24) * SECTOR;
        n.size = r[10] | r[11] << 8 | r[12] << 16 | (unsigned long)r[13] << 24;
        n.path[0] = joliet ? 'J' : 'P';
        for (k = 0; k + (joliet ? 1 : 0) < nlen && o + 4 < sizeof(n.name); k += joliet ? 2 : 1) {
            unsigned c = joliet ? (unsigned)r[33 + k] << 8 | r[34 + k] : r[33 + k];

            if (c < 0x80) {
                n.name[o++] = (char)c;
            } else if (c < 0x800) {
                n.name[o++] = (char)(0xc0 | c >> 6);
                n.name[o++] = (char)(0x80 | (c & 0x3f));
            } else {
                n.name[o++] = (char)(0xe0 | c >> 12);
                n.name[o++] = (char)(0x80 | (c >> 6 & 0x3f));
                n.name[o++] = (char)(0x80 | (c & 0x3f));
            }
        }
        n.name[o] = '\0';
        if (!n.dir) {
            char *semi = SDL_strrchr(n.name, ';'); /* the version, ";1" */

            if (semi != NULL) {
                *semi = '\0';
            }
            o = (unsigned)SDL_strlen(n.name);
            if (o > 0 && n.name[o - 1] == '.') {
                n.name[o - 1] = '\0'; /* "README." for a name without an extension */
            }
        }
        if (n.name[0] != '\0') {
            ok = fn(ctx, &n);
        }
    }
    free(buf);
    return ok;
}

/* calls fn for each entry of folder dir; 0 when it can't be read or fn stopped */
static int EachChild(Source *s, const Node *dir, EachFn fn, void *ctx) {
    FolderWalk w;

    if (s->iso != NULL) {
        return ImageChildren(s, dir, fn, ctx);
    }
    w.dir = dir;
    w.fn = fn;
    w.ctx = ctx;
    w.stopped = 0;
    return SDL_EnumerateDirectory(dir->path, FolderEntry, &w) && !w.stopped;
}

typedef struct Lookup {
    const char *want;
    Node *out;
    int found;
} Lookup;

static int LookupEntry(void *ctx, const Node *child) {
    Lookup *l = (Lookup *)ctx;

    if (SDL_strcasecmp(child->name, l->want) == 0) {
        *l->out = *child;
        l->found = 1;
        return 0;
    }
    return 1;
}

/* dir's entry called name, in any case (a CD's names may be in either) */
static int Child(Source *s, const Node *dir, const char *name, Node *out) {
    Lookup l;

    l.want = name;
    l.out = out;
    l.found = 0;
    EachChild(s, dir, LookupEntry, &l);
    return l.found;
}

static int ChildFolder(Source *s, const Node *dir, const char *name, Node *out) {
    return Child(s, dir, name, out) && out->dir;
}

static int ChildFile(Source *s, const Node *dir, const char *name, Node *out) {
    return Child(s, dir, name, out) && !out->dir;
}

/* What root is. The CD has main.z, the speech folder and the game's .res files (Legoland.res) either beside it or
 * in volumes\, and the movies either beside it or in FMV\ (both layouts are handled). */
static int Kind(Source *s, const Node *root) {
    Node a, b;
    int res = ChildFile(s, root, "Legoland.res", &a) ||
        (ChildFolder(s, root, "volumes", &a) && ChildFile(s, &a, "Legoland.res", &b));

    if (!res || !ChildFolder(s, root, "speech", &a)) {
        return PORT_SETUP_NONE;
    }
    if (s->iso == NULL && ChildFolder(s, root, "IMusic", &a) && ChildFolder(s, root, "zbuffers", &a) &&
        ChildFolder(s, root, "FMV", &a)) {
        return PORT_SETUP_INSTALLED;
    }
    if (ChildFile(s, root, "main.z", &a)) {
        return s->iso != NULL ? PORT_SETUP_IMAGE : PORT_SETUP_CD;
    }
    return PORT_SETUP_NONE;
}

int PortSetupKind(const char *path) {
    Source s;
    Node root;
    int kind;

    if (!OpenSource(&s, path, &root)) {
        return PORT_SETUP_NONE;
    }
    kind = Kind(&s, &root);
    CloseSource(&s);
    return kind;
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
    char *source; /* a file to copy from a folder, or NULL for one of main.z's or the image's */
    char *target; /* relative to the target folder, '/'-separated */
    unsigned long long size;
    int from_image; /* 1: the image's bytes at image_offset */
    unsigned long long image_offset;
    unsigned long z_offset, z_size; /* in main.z */
} Job;

typedef struct Work {
    Source *src;
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

/* Files the data folder doesn't take: the original's exe (the port doesn't need it, and the port's own exe may have
 * its name) and anything named like the running exe, which can't be overwritten while it runs. */
static int Skipped(const char *target) {
#ifdef _WIN32
    char exe[MAX_PATH];
    const char *name;

    if (GetModuleFileNameA(NULL, exe, sizeof(exe)) > 0) {
        name = SDL_strrchr(exe, '\\');
        if (SDL_strcasecmp(target, name != NULL ? name + 1 : exe) == 0) {
            return 1;
        }
    }
#endif
    return SDL_strcasecmp(target, "legoland.exe") == 0;
}

static Job *NewJob(Work *w, const char *target, unsigned long long size) {
    Job *j;

    if (w->count == w->cap) {
        size_t cap = w->cap ? w->cap * 2 : 256;
        Job *grown = (Job *)realloc(w->jobs, cap * sizeof(Job));

        if (grown == NULL) {
            Fail(w, "Out of memory", target);
            return NULL;
        }
        w->jobs = grown;
        w->cap = cap;
    }
    j = &w->jobs[w->count++];
    memset(j, 0, sizeof(*j));
    j->target = SDL_strdup(target);
    j->size = size;
    w->total += size;
    return j;
}

/* a file of the source */
static int AddNode(Work *w, const Node *n, const char *target) {
    Job *j;

    if (Skipped(target)) {
        return 1;
    }
    j = NewJob(w, target, n->size);
    if (j == NULL) {
        return 0;
    }
    if (w->src->iso != NULL) {
        j->from_image = 1;
        j->image_offset = n->offset;
    } else {
        j->source = SDL_strdup(n->path);
    }
    return 1;
}

/* one of main.z's files */
static int AddPacked(Work *w, const char *target, unsigned long long size, unsigned long z_offset,
    unsigned long z_size) {
    Job *j;

    if (Skipped(target)) {
        return 1;
    }
    j = NewJob(w, target, size);
    if (j == NULL) {
        return 0;
    }
    j->z_offset = z_offset;
    j->z_size = z_size;
    return 1;
}

typedef struct Walk {
    Work *work;
    const char *rel; /* the target path of this folder */
} Walk;

static int AddTree(Work *w, const Node *folder, const char *rel);

static int WalkEntry(void *ctx, const Node *child) {
    Walk *k = (Walk *)ctx;
    char rel[PATH_SIZE];

    SDL_snprintf(rel, sizeof(rel), "%s%s%s", k->rel, k->rel[0] != '\0' ? "/" : "", child->name);
    return child->dir ? AddTree(k->work, child, rel) : AddNode(k->work, child, rel);
}

static int AddTree(Work *w, const Node *folder, const char *rel) {
    Walk k;

    k.work = w;
    k.rel = rel;
    if (!EachChild(w->src, folder, WalkEntry, &k)) {
        if (w->error[0] == '\0') {
            Fail(w, "Can't read the folder", folder->path[0] != '\0' && w->src->iso == NULL ? folder->path : rel);
        }
        return 0;
    }
    return 1;
}

static int EndsWith(const char *name, const char *ext) {
    size_t n = SDL_strlen(name), e = SDL_strlen(ext);

    return n > e && SDL_strcasecmp(name + n - e, ext) == 0;
}

/* the CD's own files beside main.z: the movies (.avi) go into FMV\, the game's resources (.res) into volumes\ */
static int LooseEntry(void *ctx, const Node *child) {
    Work *w = (Work *)ctx;
    char rel[PATH_SIZE];

    if (child->dir) {
        return 1;
    }
    if (EndsWith(child->name, ".avi")) {
        SDL_snprintf(rel, sizeof(rel), "FMV/%s", child->name);
        return AddNode(w, child, rel);
    }
    if (EndsWith(child->name, ".res")) {
        SDL_snprintf(rel, sizeof(rel), "volumes/%s", child->name);
        return AddNode(w, child, rel);
    }
    return 1;
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
        if (!AddPacked(w, rel, orig, off, comp)) {
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
    static unsigned char buf[1 << 20];
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
    if (j->source == NULL && !j->from_image) {
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
    } else if (j->from_image) {
        unsigned long long left = j->size, at = j->image_offset;

        while (ok && left > 0) {
            size_t n = left > sizeof(buf) ? sizeof(buf) : (size_t)left;

            if (!ReadAt(w->src->iso, at, buf, n)) {
                ok = Fail(w, "Can't read the CD image (is it complete?)", j->target);
                break;
            }
            if (fwrite(buf, 1, n, out) != n) {
                ok = Fail(w, "Can't write (is the disk full?)", path);
                break;
            }
            at += n;
            left -= n;
            w->done += n;
            Report(w, j->target);
            if (Cancelled(w)) {
                ok = Fail(w, "Cancelled", "");
            }
        }
    } else {
        FILE *in = fopen(j->source, "rb");
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

/* main.z's bytes */
static unsigned char *LoadArchive(Source *s, const Node *z, size_t *size) {
    unsigned char *data;

    if (s->iso == NULL) {
        return (unsigned char *)SDL_LoadFile(z->path, size);
    }
    data = (unsigned char *)SDL_malloc((size_t)z->size + 1);
    if (data != NULL && !ReadAt(s->iso, z->offset, data, (size_t)z->size)) {
        SDL_free(data);
        return NULL;
    }
    *size = (size_t)z->size;
    return data;
}

int PortSetupInstall(const char *source, const char *target, const PortSetupProgress *progress, char *error,
    unsigned int error_size) {
    Work w;
    Source s;
    Node root;
    unsigned char *z = NULL;
    size_t zsize = 0;
    int kind;
    int ok = 1;
    size_t i;

    memset(&w, 0, sizeof(w));
    w.src = &s;
    w.progress = progress;
    w.error = error;
    w.error_size = error_size;
    error[0] = '\0';
    if (!OpenSource(&s, source, &root)) {
        return Fail(&w, "Not the LEGOLAND CD, its image or a LEGOLAND folder", source);
    }
    kind = Kind(&s, &root);
    if (kind == PORT_SETUP_NONE) {
        CloseSource(&s);
        return Fail(&w, "Not the LEGOLAND CD, its image or a LEGOLAND folder", source);
    }
    if (!IsFolder(target) && !SDL_CreateDirectory(target)) {
        CloseSource(&s);
        return Fail(&w, "Can't create the folder", target);
    }
    if (kind == PORT_SETUP_CD || kind == PORT_SETUP_IMAGE) {
        Node n;

        ChildFile(&s, &root, "main.z", &n);
        z = LoadArchive(&s, &n, &zsize);
        if (z == NULL) {
            ok = Fail(&w, "Can't read", "main.z");
        }
        ok = ok && AddArchive(&w, z, zsize, "main.z");
        ok = ok && ChildFolder(&s, &root, "speech", &n) && AddTree(&w, &n, "speech");
        if (ok && ChildFolder(&s, &root, "FMV", &n)) {
            ok = AddTree(&w, &n, "FMV");
        }
        if (ok && ChildFolder(&s, &root, "volumes", &n)) {
            ok = AddTree(&w, &n, "volumes");
        }
        ok = ok && EachChild(&s, &root, LooseEntry, &w);
    } else {
        ok = AddTree(&w, &root, "");
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
    CloseSource(&s);
    return ok;
}
