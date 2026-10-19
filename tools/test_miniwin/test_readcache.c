/* test_readcache: miniwin's read cache off Windows (miniwin/src/posix/readcache.c), against plain reads of the same
 * file: PosixReadCacheRead, ReadFile on a read-only handle (with SetFilePointer), and MiniwinFopen's stdio (fread,
 * fseek, ftell, fgetc, ungetc, fgets, feof) against the C library's own fopen. Random reads of every size, inside a
 * block, across many, and at and past the end of a file whose size isn't a multiple of the blocks'. Built by
 * build_linux.sh. */
#include <windows.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "miniwin/files.h"
#include "posix/posix.h"

#define FILE_SIZE (3 * 1024 * 1024 + 12345)

static int checks;
static int failed;

static void Check(int ok, const char *what, long long a, long long b) {
    checks++;
    if (!ok) {
        failed++;
        if (failed <= 20) {
            printf("FAILED: %s (%lld, %lld)\n", what, a, b);
        }
    }
}

static unsigned int seed = 12345;

static unsigned int Random(void) {
    seed = seed * 1103515245u + 12345u;
    return seed >> 8;
}

/* sizes as the game asks for them: mostly a few bytes, some KB, now and then MBs */
static size_t RandomSize(void) {
    unsigned int r = Random() % 100;

    if (r < 60) {
        return 1 + Random() % 64;
    }
    if (r < 90) {
        return 1 + Random() % (300 * 1024);
    }
    return 1 + Random() % (3 * 1024 * 1024);
}

static long long RandomPos(long long pos) {
    unsigned int r = Random() % 100;

    if (r < 50) {
        return pos; /* on from the last read */
    }
    if (r < 60) {
        return FILE_SIZE - (long long)(Random() % 300); /* near the end */
    }
    if (r < 65) {
        return FILE_SIZE + (long long)(Random() % 300); /* past it */
    }
    return (long long)(Random() % FILE_SIZE);
}

static void TestCache(const char *path) {
    static unsigned char want[4 * 1024 * 1024];
    static unsigned char got[4 * 1024 * 1024];
    int fd = open(path, O_RDONLY);
    struct PosixReadCache *c = PosixReadCacheNew(fd);
    long long pos = 0;
    int i;

    for (i = 0; i < 3000; i++) {
        size_t size = RandomSize();
        long long n;
        ssize_t m;

        pos = RandomPos(pos);
        m = pread(fd, want, size, pos);
        n = PosixReadCacheRead(c, pos, got, size);
        Check(n == (long long)m, "PosixReadCacheRead: as many bytes as pread", n, (long long)m);
        Check(n <= 0 || memcmp(want, got, (size_t)n) == 0, "PosixReadCacheRead: the same bytes", pos, (long long)size);
        if (n > 0) {
            pos += n;
        }
    }
    PosixReadCacheFree(c);
    close(fd);
}

static void TestReadFile(const char *path) {
    static unsigned char want[4 * 1024 * 1024];
    static unsigned char got[4 * 1024 * 1024];
    int fd = open(path, O_RDONLY);
    HANDLE h = CreateFileA(path, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
    long long pos = 0;
    int i;

    Check(h != INVALID_HANDLE_VALUE, "CreateFileA", 0, 0);
    for (i = 0; i < 2000; i++) {
        size_t size = RandomSize();
        DWORD n = 0;
        ssize_t m;
        BOOL ok;

        if (Random() % 3 == 0) {
            pos = RandomPos(pos);
            SetFilePointer(h, (LONG)pos, NULL, FILE_BEGIN);
        }
        m = pread(fd, want, size, pos);
        ok = ReadFile(h, got, (DWORD)size, &n, NULL);
        Check(ok && (long long)n == (long long)m, "ReadFile: as many bytes as pread", (long long)n, (long long)m);
        Check(memcmp(want, got, n) == 0, "ReadFile: the same bytes", pos, (long long)size);
        pos += n;
        Check(SetFilePointer(h, 0, NULL, FILE_CURRENT) == (DWORD)pos, "SetFilePointer: the position", pos, 0);
    }
    Check(SetFilePointer(h, 0, NULL, FILE_END) == FILE_SIZE, "SetFilePointer: the end", FILE_SIZE, 0);
    CloseHandle(h);
    close(fd);
}

static void TestStdio(const char *path) {
    static unsigned char want[4 * 1024 * 1024];
    static unsigned char got[4 * 1024 * 1024];
    FILE *plain = fopen(path, "rb");
    FILE *cached = MiniwinFopen(path, "rb");
    int i;

    Check(cached != NULL, "MiniwinFopen", 0, 0);
    for (i = 0; i < 4000; i++) {
        unsigned int op = Random() % 8;

        if (op == 0) {
            size_t size = RandomSize();
            size_t a = fread(want, 1, size, plain);
            size_t b = fread(got, 1, size, cached);
            Check(a == b, "fread: as many bytes", (long long)a, (long long)b);
            Check(memcmp(want, got, a < b ? a : b) == 0, "fread: the same bytes", ftell(plain), (long long)size);
        } else if (op == 1) {
            long off = (long)(Random() % (FILE_SIZE + 500));
            int whence = (int)(Random() % 3);
            int a;
            int b;

            if (whence == SEEK_CUR) {
                off = (long)(Random() % 200000) - 100000;
            } else if (whence == SEEK_END) {
                off = -(long)(Random() % 300000);
            }
            a = fseek(plain, off, whence);
            b = fseek(cached, off, whence);
            Check(a == b, "fseek: the same result", a, b);
        } else if (op == 2) {
            Check(ftell(plain) == ftell(cached), "ftell", ftell(plain), ftell(cached));
        } else if (op == 3) {
            int a = fgetc(plain);
            int b = fgetc(cached);
            Check(a == b, "fgetc", a, b);
        } else if (op == 4) {
            int a = fgetc(plain);
            int b = fgetc(cached);
            if (a != EOF && b != EOF) {
                Check(ungetc(a, plain) == ungetc(b, cached), "ungetc", a, b);
            }
        } else if (op == 5) {
            char la[300];
            char lb[300];
            char *a = fgets(la, sizeof(la), plain);
            char *b = fgets(lb, sizeof(lb), cached);
            Check((a == NULL) == (b == NULL), "fgets: the same result", a == NULL, b == NULL);
            Check(a == NULL || strcmp(la, lb) == 0, "fgets: the same line", 0, 0);
        } else if (op == 6) {
            Check(feof(plain) == feof(cached), "feof", feof(plain), feof(cached));
            Check(ferror(cached) == 0, "ferror", 0, 0);
        } else {
            rewind(plain);
            rewind(cached);
        }
    }
    fclose(plain);
    Check(fclose(cached) == 0, "fclose", 0, 0);
    Check(MiniwinFopen("no such file here.bin", "rb") == NULL, "MiniwinFopen of a file that isn't there", 0, 0);
}

int main(void) {
    char path[] = "/tmp/test_readcache_XXXXXX";
    int fd = mkstemp(path);
    unsigned char *data = (unsigned char *)malloc(FILE_SIZE);
    int i;

    for (i = 0; i < FILE_SIZE; i++) {
        /* bytes, with a line end now and then for fgets */
        data[i] = (unsigned char)(Random() % 23 == 0 ? '\n' : Random());
    }
    if (fd < 0 || write(fd, data, FILE_SIZE) != FILE_SIZE) {
        printf("can't write %s\n", path);
        return 1;
    }
    close(fd);
    TestCache(path);
    TestReadFile(path);
    TestStdio(path);
    unlink(path);
    free(data);
    printf("%d checks, %d failed\n", checks, failed);
    return failed != 0;
}
