/* [library:filesystem] Read-only files through a cache of their blocks, off Windows. See posix.h.
 *
 * The game reads its files a few bytes at a time (RES_ReadFile: a length, a chunk header, a palette; the string
 * table, the paths and meshes through stdio), and jumps back and forth inside its volumes. On a desktop each read is
 * a cheap system call. In a browser it's a round trip to the page's storage (OPFS) through another thread: about
 * 1.5 ms however small, plus about 4 ms a MB. A single 64 KB window per file made 449 of those round trips for the
 * selftest's resources, mostly to read again what had just been dropped; 16 aligned blocks of 256 KB make 62. A run
 * of blocks one request needs is read in one go. The same code runs on every system, so the desktop tests check it. */
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include "posix.h"

#define BLOCK_SIZE (256 * 1024)
#define BLOCKS 16 /* 4 MB a file at most */
#define RUN_MAX 8 /* blocks read at once */

struct Block {
    long long at; /* where in the file */
    size_t len; /* less than BLOCK_SIZE: the file's last block */
    size_t cap;
    unsigned char *data;
    unsigned long long used;
};

struct PosixReadCache {
    int fd;
    int count;
    unsigned long long tick;
    struct Block block[BLOCKS];
};

struct PosixReadCache *PosixReadCacheNew(int fd) {
    struct PosixReadCache *c = (struct PosixReadCache *)calloc(1, sizeof(struct PosixReadCache));

    if (c != NULL) {
        c->fd = fd;
    }
    return c;
}

void PosixReadCacheFree(struct PosixReadCache *c) {
    int i;

    if (c == NULL) {
        return;
    }
    for (i = 0; i < c->count; i++) {
        free(c->block[i].data);
    }
    free(c);
}

static struct Block *Find(struct PosixReadCache *c, long long at) {
    int i;

    for (i = 0; i < c->count; i++) {
        if (c->block[i].at == at) {
            return &c->block[i];
        }
    }
    return NULL;
}

/* a free block, or the one used longest ago */
static struct Block *Slot(struct PosixReadCache *c) {
    struct Block *oldest;
    int i;

    if (c->count < BLOCKS) {
        return &c->block[c->count++];
    }
    oldest = &c->block[0];
    for (i = 1; i < BLOCKS; i++) {
        if (c->block[i].used < oldest->used) {
            oldest = &c->block[i];
        }
    }
    return oldest;
}

/* all of size bytes from at, unless the file ends first: how many, or -1 */
static long long ReadFully(int fd, unsigned char *buffer, size_t size, long long at) {
    size_t done = 0;

    while (done < size) {
        ssize_t n = pread(fd, buffer + done, size - done, (off_t)(at + (long long)done));

        if (n < 0 && errno == EINTR) {
            continue;
        }
        if (n < 0) {
            return -1;
        }
        if (n == 0) {
            break;
        }
        done += (size_t)n;
    }
    return (long long)done;
}

/* run blocks from at (a block's start), in one read; the ones past the end of the file aren't kept */
static int Fill(struct PosixReadCache *c, long long at, int run) {
    unsigned char *all = (unsigned char *)malloc((size_t)run * BLOCK_SIZE);
    long long n;
    int k;

    if (all == NULL) {
        errno = ENOMEM;
        return -1;
    }
    n = ReadFully(c->fd, all, (size_t)run * BLOCK_SIZE, at);
    if (n < 0) {
        free(all);
        return -1;
    }
    for (k = 0; k < run && (long long)k * BLOCK_SIZE < n; k++) {
        long long left = n - (long long)k * BLOCK_SIZE;
        size_t len = left < BLOCK_SIZE ? (size_t)left : BLOCK_SIZE;
        struct Block *b = Slot(c);

        if (b->cap < len) {
            unsigned char *data = (unsigned char *)realloc(b->data, len);

            if (data == NULL) {
                b->len = 0;
                b->at = -1;
                free(all);
                errno = ENOMEM;
                return -1;
            }
            b->data = data;
            b->cap = len;
        }
        memcpy(b->data, all + (size_t)k * BLOCK_SIZE, len);
        b->at = at + (long long)k * BLOCK_SIZE;
        b->len = len;
        b->used = ++c->tick;
    }
    free(all);
    return 0;
}

long long PosixReadCacheRead(struct PosixReadCache *c, long long pos, void *buffer, size_t size) {
    size_t done = 0;

    if (pos < 0) {
        errno = EINVAL;
        return -1;
    }
    while (done < size) {
        long long at = pos - pos % BLOCK_SIZE;
        struct Block *b = Find(c, at);
        size_t from;
        size_t take;

        if (b == NULL) {
            long long end = pos + (long long)(size - done);
            int run = 1;

            while (run < RUN_MAX && at + (long long)run * BLOCK_SIZE < end &&
                Find(c, at + (long long)run * BLOCK_SIZE) == NULL) {
                run++;
            }
            if (Fill(c, at, run) != 0) {
                return done > 0 ? (long long)done : -1;
            }
            b = Find(c, at);
            if (b == NULL) {
                break; /* at or past the end of the file */
            }
        }
        b->used = ++c->tick;
        from = (size_t)(pos - at);
        if (from >= b->len) {
            break; /* the end of the file */
        }
        take = b->len - from;
        if (take > size - done) {
            take = size - done;
        }
        memcpy((unsigned char *)buffer + done, b->data + from, take);
        done += take;
        pos += (long long)take;
        if (b->len < BLOCK_SIZE && from + take == b->len) {
            break; /* the last block: the end */
        }
    }
    return (long long)done;
}

int PosixOpenRead(const char *path) {
#ifdef LEGOLAND_WEB_BROWSER
    /* Emscripten's file system opens a read-only file in the page's storage as a Blob, and each read of it is an
     * asynchronous slice of the Blob, about 1.5 ms. Opened read-write it's an access handle, which the storage
     * thread reads at once: about 0.1 ms. Nothing is ever written through it. Another tab with the game holds the
     * file's access handle (they're exclusive): then read-only, as before. */
    int fd = open(path, O_RDWR | O_CLOEXEC);

    if (fd >= 0 || errno == ENOENT || errno == ENOTDIR) {
        return fd;
    }
#endif
    return open(path, O_RDONLY | O_CLOEXEC);
}

/* ---- stdio on top: fopen(path, "r"/"rb") (miniwin's MiniwinFopen) ---- */

#ifndef __ANDROID__ /* Android's C library has no fopencookie; its stdio buffers files itself (files.c) */

#if defined(__GLIBC__) || defined(__BIONIC__)
typedef off64_t CookieOffset;
#else
typedef off_t CookieOffset;
#endif

struct Cookie {
    int fd;
    long long pos;
    struct PosixReadCache *cache;
};

static ssize_t CookieRead(void *p, char *buffer, size_t size) {
    struct Cookie *c = (struct Cookie *)p;
    long long n = PosixReadCacheRead(c->cache, c->pos, buffer, size);

    if (n > 0) {
        c->pos += n;
    }
    return (ssize_t)n;
}

static int CookieSeek(void *p, CookieOffset *offset, int whence) {
    struct Cookie *c = (struct Cookie *)p;
    long long base = 0;
    struct stat st;

    if (whence == SEEK_CUR) {
        base = c->pos;
    } else if (whence == SEEK_END) {
        if (fstat(c->fd, &st) != 0) {
            return -1;
        }
        base = (long long)st.st_size;
    } else if (whence != SEEK_SET) {
        errno = EINVAL;
        return -1;
    }
    if (base + (long long)*offset < 0) {
        errno = EINVAL;
        return -1;
    }
    c->pos = base + (long long)*offset;
    *offset = (CookieOffset)c->pos;
    return 0;
}

static int CookieClose(void *p) {
    struct Cookie *c = (struct Cookie *)p;

    PosixReadCacheFree(c->cache);
    close(c->fd);
    free(c);
    return 0;
}

FILE *PosixFopenRead(const char *path, const char *mode) {
    cookie_io_functions_t io = {CookieRead, NULL, CookieSeek, CookieClose};
    struct Cookie *c;
    FILE *f;
    int fd = PosixOpenRead(path);

    if (fd < 0) {
        return NULL;
    }
    c = (struct Cookie *)calloc(1, sizeof(struct Cookie));
    if (c != NULL) {
        c->fd = fd;
        c->cache = PosixReadCacheNew(fd);
    }
    if (c == NULL || c->cache == NULL) {
        free(c);
        close(fd);
        errno = ENOMEM;
        return NULL;
    }
    f = fopencookie(c, mode, io);
    if (f == NULL) {
        CookieClose(c);
        return NULL;
    }
    return f;
}
#endif
