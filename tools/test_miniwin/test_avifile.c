/* test_avifile: checks miniwin's AVIFile (miniwin/src/avifile.c) against Windows' own, without starting the game:
 * every AVI in the data folder (the movies in FMV\ and the advisor animations) opened on both, comparing file and
 * stream info, formats, start and length, key frame searches, reference counts, and the reads the game and
 * port/port_movie.c make: every video frame, and the sound in the game's own read sizes, byte for byte.
 *
 *   tools/test_miniwin/build.sh && tools/test_miniwin/test_avifile.exe <data folder>   (FAIL lines; exits 1 on any) */
#include <windows.h>
#include <vfw.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "miniwin/avifile.h"

static int failures;
static int checks;
static int reported; /* FAIL lines for the current file, to keep the output short */

static void Same(const char *file, const char *what, long a, long b) {
    checks++;
    if (a != b) {
        failures++;
        if (reported++ < 8) {
            printf("FAIL %s: %s: %ld (AVIFile) vs %ld (miniwin)\n", file, what, a, b);
        }
    }
}

static void SameBytes(const char *file, const char *what, const void *a, const void *b, size_t n) {
    const BYTE *x = (const BYTE *)a, *y = (const BYTE *)b;
    size_t i;

    checks++;
    for (i = 0; i < n; i++) {
        if (x[i] != y[i]) {
            failures++;
            if (reported++ < 8) {
                printf("FAIL %s: %s: byte %u of %u: %02x (AVIFile) vs %02x (miniwin)\n", file, what, (unsigned)i,
                    (unsigned)n, x[i], y[i]);
            }
            return;
        }
    }
}

/* Compares two structures DWORD by DWORD (the names in the info structures are compared as bytes). */
static void SameDwords(const char *file, const char *what, const void *a, const void *b, size_t n) {
    size_t i;
    char name[200];

    for (i = 0; i + 4 <= n; i += 4) {
        sprintf(name, "%s +0x%02x", what, (unsigned)i);
        Same(file, name, *(const long *)((const BYTE *)a + i), *(const long *)((const BYTE *)b + i));
    }
}

static BYTE *bufa, *bufb;
#define BUF 4000000

static void Read(const char *file, const char *what, PAVISTREAM ra, void *mb, LONG pos, LONG n, LONG size, int null) {
    LONG bytes_a = -7, samples_a = -7;
    int bytes_b = -7, samples_b = -7;
    HRESULT a, b;
    char name[200];

    memset(bufa, 0xcd, size + 16);
    memset(bufb, 0xcd, size + 16);
    a = AVIStreamRead(ra, pos, n, null ? NULL : bufa, size, &bytes_a, &samples_a);
    b = MiniwinAVIStreamRead(mb, pos, n, null ? NULL : bufb, size, &bytes_b, &samples_b);
    sprintf(name, "%s read %ld x%ld into %ld%s: result", what, pos, n, size, null ? " (no buffer)" : "");
    Same(file, name, a, b);
    sprintf(name, "%s read %ld x%ld into %ld%s: bytes", what, pos, n, size, null ? " (no buffer)" : "");
    Same(file, name, bytes_a, bytes_b);
    sprintf(name, "%s read %ld x%ld into %ld%s: samples", what, pos, n, size, null ? " (no buffer)" : "");
    Same(file, name, samples_a, samples_b);
    if (!null) {
        sprintf(name, "%s read %ld x%ld into %ld: data", what, pos, n, size);
        SameBytes(file, name, bufa, bufb, size + 16);
    }
}

static void Stream(const char *file, PAVISTREAM ra, void *mb, int fps) {
    AVISTREAMINFOA ia, ib;
    LONG fa = 0;
    int fb = 0;
    LONG len, i;
    BYTE *fmta, *fmtb;

    memset(&ia, 0xcd, sizeof(ia));
    memset(&ib, 0xcd, sizeof(ib));
    Same(file, "stream info", AVIStreamInfoA(ra, &ia, sizeof(ia)), MiniwinAVIStreamInfoA(mb, &ib, sizeof(ib)));
    /* all but the name, which Windows gives in the system's language when the file has none */
    SameDwords(file, "stream info", &ia, &ib, offsetof(AVISTREAMINFOA, szName));
    Same(file, "start", AVIStreamStart(ra), MiniwinAVIStreamStart(mb));
    Same(file, "length", AVIStreamLength(ra), MiniwinAVIStreamLength(mb));
    Same(file, "format size query", AVIStreamReadFormat(ra, 0, NULL, &fa), MiniwinAVIStreamReadFormat(mb, 0, NULL, &fb));
    Same(file, "format size", fa, fb);
    fmta = (BYTE *)calloc(1, fa + 16);
    fmtb = (BYTE *)calloc(1, fa + 16);
    fb = fa;
    Same(file, "format", AVIStreamReadFormat(ra, 0, fmta, &fa), MiniwinAVIStreamReadFormat(mb, 0, fmtb, &fb));
    SameBytes(file, "format", fmta, fmtb, fa + 16);
    len = AVIStreamLength(ra);
    if (ia.fccType == streamtypeVIDEO) {
        for (i = -1; i <= len + 1; i++) {
            char name[100];

            sprintf(name, "previous key frame of %ld", i);
            Same(file, name, AVIStreamFindSample(ra, i, FIND_KEY | FIND_PREV),
                MiniwinAVIStreamFindSample(mb, i, FIND_KEY | FIND_PREV));
        }
        for (i = -1; i <= len; i++) {
            Read(file, "video", ra, mb, i, 1, 0, 1);
            Read(file, "video", ra, mb, i, 1, BUF, 0);
        }
        Read(file, "video", ra, mb, 0, 1, 16, 0);
    } else if (ia.fccType == streamtypeAUDIO) {
        const WAVEFORMATEX *w = (const WAVEFORMATEX *)fmta;

        if (w->wFormatTag == WAVE_FORMAT_PCM) {
            /* UpdateAviAudioBuffer: a frame's samples at a time */
            LONG per_frame = w->nSamplesPerSec / fps, bytes = w->nAvgBytesPerSec / fps;

            for (i = 0; i <= len + per_frame; i += per_frame) {
                Read(file, "PCM sound", ra, mb, i, per_frame, bytes, 0);
            }
        } else {
            /* UpdateAviAudioBuffer: 0x100 blocks into a quarter of a frame's decoded bytes */
            LONG bytes = w->nChannels * 2 * w->nSamplesPerSec / fps;

            for (i = 0; i <= len + 1; i++) {
                Read(file, "ADPCM sound", ra, mb, i, 0x100, bytes >> 2, 0);
            }
        }
        Read(file, "sound", ra, mb, 0, 0x100, 0, 1);
        Read(file, "sound", ra, mb, 3, 5000, BUF, 0);
        Read(file, "sound", ra, mb, len - 2, 10, BUF, 0);
        Read(file, "sound", ra, mb, 1, 10, 7, 0);
        Read(file, "sound", ra, mb, 0, -1, BUF, 0);
    }
    free(fmta);
    free(fmtb);
}

static void File(const char *path, const char *name) {
    PAVIFILE fa;
    void *fb;
    AVIFILEINFOA ia, ib;
    HRESULT a, b;
    int i, fps = 15;

    reported = 0;
    a = AVIFileOpenA(&fa, path, OF_READ, NULL);
    b = MiniwinAVIFileOpenA(&fb, path, OF_READ, NULL);
    Same(name, "open", a, b);
    if (a != 0 || b != 0) {
        return;
    }
    memset(&ia, 0xcd, sizeof(ia));
    memset(&ib, 0xcd, sizeof(ib));
    Same(name, "file info", AVIFileInfoA(fa, &ia, sizeof(ia)), MiniwinAVIFileInfoA(fb, &ib, sizeof(ib)));
    /* all but the file type, a description in the system's language */
    SameDwords(name, "file info", &ia, &ib, offsetof(AVIFILEINFOA, szFileType));
    /* the game's frame rate (PlayMovieFrames, OpenMovieAudio) comes from the video stream */
    for (i = 0; i < 4; i++) {
        PAVISTREAM sa;
        void *sb;
        AVISTREAMINFOA si;

        a = AVIFileGetStream(fa, &sa, 0, i);
        b = MiniwinAVIFileGetStream(fb, &sb, 0, i);
        Same(name, "get stream", a, b);
        if (a != 0 || b != 0) {
            break;
        }
        AVIStreamInfoA(sa, &si, sizeof(si));
        if (si.fccType == streamtypeVIDEO && si.dwScale != 0) {
            fps = si.dwRate / si.dwScale;
        }
    }
    for (i = 0; i < 4; i++) {
        PAVISTREAM sa;
        void *sb;

        if (AVIFileGetStream(fa, &sa, 0, i) != 0 || MiniwinAVIFileGetStream(fb, &sb, 0, i) != 0) {
            break;
        }
        Stream(name, sa, sb, fps);
        Same(name, "stream add ref", AVIStreamAddRef(sa), MiniwinAVIStreamAddRef(sb));
        Same(name, "stream release", AVIStreamRelease(sa), MiniwinAVIStreamRelease(sb));
        Same(name, "stream release", AVIStreamRelease(sa), MiniwinAVIStreamRelease(sb));
    }
    Same(name, "file release", AVIFileRelease(fa), MiniwinAVIFileRelease(fb));
    printf("%s: %s\n", name, reported ? "DIFFERS" : "same");
}

static void Folder(const char *folder, const char *sub) {
    char pattern[MAX_PATH], path[MAX_PATH];
    WIN32_FIND_DATAA fd;
    HANDLE find;

    sprintf(pattern, "%s%s*.avi", folder, sub);
    find = FindFirstFileA(pattern, &fd);
    if (find == INVALID_HANDLE_VALUE) {
        printf("FAIL: no %s\n", pattern);
        failures++;
        return;
    }
    do {
        sprintf(path, "%s%s%s", folder, sub, fd.cFileName);
        File(path, fd.cFileName);
    } while (FindNextFileA(find, &fd));
    FindClose(find);
}

int main(int argc, char **argv) {
    PAVIFILE fa;
    void *fb;

    if (argc < 2) {
        printf("usage: test_avifile <data folder>\n");
        return 2;
    }
    bufa = (BYTE *)malloc(BUF + 16);
    bufb = (BYTE *)malloc(BUF + 16);
    AVIFileInit();
    MiniwinAVIFileInit();
    Same("missing.avi", "open", AVIFileOpenA(&fa, "no such file.avi", OF_READ, NULL),
        MiniwinAVIFileOpenA(&fb, "no such file.avi", OF_READ, NULL));
    Folder(argv[1], "\\FMV\\");
    Folder(argv[1], "\\");
    AVIFileExit();
    MiniwinAVIFileExit();
    printf("%d checks, %d failed\n", checks, failures);
    return failures != 0;
}
