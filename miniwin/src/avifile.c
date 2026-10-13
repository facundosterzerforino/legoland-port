/* [library:movie] miniwin's AVIFile: AVI files read without Video for Windows. See miniwin/avifile.h.
 * tools/test_miniwin/test_avifile.exe compares it with Windows' AVIFile on the game's movies. */
#include <windows.h>
#include <vfw.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "miniwin/avifile.h"
#include "miniwin/files.h"

#define FILE_MAGIC 0x66697661 /* "avif" */
#define STREAM_MAGIC 0x73697661 /* "avis" */
#define MAX_STREAMS 8

struct Chunk {
    DWORD offset; /* of the data in the file */
    DWORD size;
    DWORD flags; /* AVIIF_KEYFRAME... */
};

struct AviFile;

struct AviStream {
    int magic;
    LONG refs;
    struct AviFile *file;
    AVISTREAMINFOA info;
    BYTE *format;
    LONG format_size;
    struct Chunk *chunks;
    LONG count;
    DWORD *first; /* sampled streams (dwSampleSize != 0): where each chunk starts in the stream's bytes, then the
                     total */
};

struct AviFile {
    int magic;
    LONG refs;
    char *path;
    AVIFILEINFOA info;
    int streams;
    struct AviStream stream[MAX_STREAMS];
};

/* One file is kept open at a time, for reading samples: the game leaks its AVIFile references (Windows keeps those
 * files open for good), so holding a handle per file would run out of them in a long session. */
static FILE *open_file;
static const struct AviFile *open_owner;

static DWORD Dword(const BYTE *p) {
    return p[0] | p[1] << 8 | p[2] << 16 | (DWORD)p[3] << 24;
}

static int ReadAt(const struct AviFile *af, DWORD offset, void *buf, DWORD size) {
    if (open_owner != af) {
        if (open_file != NULL) {
            fclose(open_file);
        }
        open_owner = NULL;
        open_file = MiniwinFopen(af->path, "rb");
        if (open_file == NULL) {
            return 0;
        }
        open_owner = af;
    }
    return fseek(open_file, (long)offset, SEEK_SET) == 0 && fread(buf, 1, size, open_file) == size;
}

static void FreeFile(struct AviFile *af) {
    int i;

    if (open_owner == af) {
        fclose(open_file);
        open_file = NULL;
        open_owner = NULL;
    }
    for (i = 0; i < af->streams; i++) {
        free(af->stream[i].format);
        free(af->stream[i].chunks);
        free(af->stream[i].first);
    }
    free(af->path);
    af->magic = 0;
    free(af);
}

static struct AviFile *ValidFile(void *p) {
    struct AviFile *af = (struct AviFile *)p;

    return af != NULL && af->magic == FILE_MAGIC ? af : NULL;
}

static struct AviStream *ValidStream(void *p) {
    struct AviStream *s = (struct AviStream *)p;

    return s != NULL && s->magic == STREAM_MAGIC ? s : NULL;
}

/* ---- parsing ---- */

/* The stream list (LIST strl): its header, format and name. */
static int ParseStream(struct AviStream *s, const BYTE *p, DWORD size) {
    DWORD pos = 0;

    while (pos + 8 <= size) {
        DWORD id = Dword(p + pos), n = Dword(p + pos + 4);
        const BYTE *d = p + pos + 8;

        if (n > size - pos - 8) {
            n = size - pos - 8;
        }
        if (id == ckidSTREAMHEADER && n >= 48) {
            s->info.fccType = Dword(d);
            s->info.fccHandler = Dword(d + 4);
            s->info.dwFlags = Dword(d + 8);
            s->info.wPriority = (WORD)(d[12] | d[13] << 8);
            s->info.wLanguage = (WORD)(d[14] | d[15] << 8);
            s->info.dwInitialFrames = Dword(d + 16);
            s->info.dwScale = Dword(d + 20);
            s->info.dwRate = Dword(d + 24);
            s->info.dwStart = Dword(d + 28);
            s->info.dwLength = Dword(d + 32);
            s->info.dwSuggestedBufferSize = Dword(d + 36);
            s->info.dwQuality = Dword(d + 40);
            s->info.dwSampleSize = Dword(d + 44);
            if (n >= 56) {
                s->info.rcFrame.left = (short)(d[48] | d[49] << 8);
                s->info.rcFrame.top = (short)(d[50] | d[51] << 8);
                s->info.rcFrame.right = (short)(d[52] | d[53] << 8);
                s->info.rcFrame.bottom = (short)(d[54] | d[55] << 8);
            }
        } else if (id == ckidSTREAMFORMAT && s->format == NULL) {
            s->format = (BYTE *)malloc(n ? n : 1);
            if (s->format == NULL) {
                return 0;
            }
            memcpy(s->format, d, n);
            s->format_size = (LONG)n;
        } else if (id == ckidSTREAMNAME) {
            DWORD len = n < sizeof(s->info.szName) - 1 ? n : sizeof(s->info.szName) - 1;

            memcpy(s->info.szName, d, len);
            s->info.szName[len] = 0;
        }
        pos += 8 + n + (n & 1);
    }
    return s->info.fccType != 0 && s->format != NULL;
}

/* The header list (LIST hdrl): the main header and the stream lists. */
static int ParseHeaders(struct AviFile *af, const BYTE *p, DWORD size) {
    DWORD pos = 0;

    while (pos + 8 <= size) {
        DWORD id = Dword(p + pos), n = Dword(p + pos + 4);
        const BYTE *d = p + pos + 8;

        if (n > size - pos - 8) {
            n = size - pos - 8;
        }
        if (id == ckidAVIMAINHDR && n >= 40) {
            af->info.dwMaxBytesPerSec = Dword(d + 4);
            /* AVIFile passes on only these flags (not HASINDEX, MUSTUSEINDEX, TRUSTCKTYPE) */
            af->info.dwFlags =
                Dword(d + 12) & (AVIFILEINFO_ISINTERLEAVED | AVIFILEINFO_WASCAPTUREFILE | AVIFILEINFO_COPYRIGHTED);
            af->info.dwLength = Dword(d + 16);
            af->info.dwSuggestedBufferSize = Dword(d + 28);
            af->info.dwWidth = Dword(d + 32);
            af->info.dwHeight = Dword(d + 36);
            af->info.dwScale = Dword(d); /* microseconds per frame, turned into a rate below */
        } else if (id == FOURCC_LIST && n >= 4 && Dword(d) == listtypeSTREAMHEADER && af->streams < MAX_STREAMS) {
            struct AviStream *s = &af->stream[af->streams];

            s->magic = STREAM_MAGIC;
            s->file = af;
            if (!ParseStream(s, d + 4, n - 4)) {
                return 0;
            }
            af->streams++;
        }
        pos += 8 + n + (n & 1);
    }
    return af->streams > 0;
}

/* The index (idx1): every chunk in order, sorted out by stream. movi is the file offset of the "movi" list type,
 * which the offsets count from (or the start of the file, in some writers' files). */
static int ParseIndex(struct AviFile *af, const BYTE *p, DWORD size, DWORD movi) {
    DWORD entries = size / 16, i, base = movi;
    int k, have_records = 0;

    if (entries > 0) {
        /* relative offsets point the first entry at its chunk past the "movi" list type */
        DWORD first = Dword(p + 8);

        base = first < movi ? movi : 0;
    }
    for (k = 0; k < af->streams; k++) {
        af->stream[k].chunks = (struct Chunk *)malloc(sizeof(struct Chunk) * (entries ? entries : 1));
        if (af->stream[k].chunks == NULL) {
            return 0;
        }
    }
    for (i = 0; i < entries; i++) {
        const BYTE *e = p + i * 16;
        DWORD id = Dword(e);
        int a = e[0], b = e[1], n;
        struct AviStream *s;

        if (id == listtypeAVIRECORD) {
            /* an interleaved file's records: AVIFile reports the largest as the file's suggested buffer */
            if (!have_records || Dword(e + 12) > af->info.dwSuggestedBufferSize) {
                af->info.dwSuggestedBufferSize = Dword(e + 12);
            }
            have_records = 1;
            continue;
        }
        if (a < '0' || a > '9' || b < '0' || b > '9') {
            continue;
        }
        n = (a - '0') * 10 + (b - '0');
        if (n >= af->streams) {
            continue;
        }
        s = &af->stream[n];
        s->chunks[s->count].offset = base + Dword(e + 8) + 8;
        s->chunks[s->count].size = Dword(e + 12);
        s->chunks[s->count].flags = Dword(e + 4);
        s->count++;
    }
    for (k = 0; k < af->streams; k++) {
        struct AviStream *s = &af->stream[k];
        LONG c;

        if (s->info.dwSampleSize != 0) {
            s->first = (DWORD *)malloc(sizeof(DWORD) * (s->count + 1));
            if (s->first == NULL) {
                return 0;
            }
            s->first[0] = 0;
            for (c = 0; c < s->count; c++) {
                s->first[c + 1] = s->first[c] + s->chunks[c].size;
            }
        }
    }
    return 1;
}

static long Open(struct AviFile *af, const char *name) {
    FILE *f = MiniwinFopen(name, "rb");
    BYTE head[12], ck[8];
    BYTE *data = NULL;
    DWORD pos = 12, end, movi = 0;
    int have_headers = 0, have_index = 0;

    if (f == NULL) {
        return AVIERR_FILEOPEN;
    }
    if (fread(head, 1, 12, f) != 12 || Dword(head) != FOURCC_RIFF || Dword(head + 8) != formtypeAVI) {
        fclose(f);
        return AVIERR_BADFORMAT;
    }
    end = 8 + Dword(head + 4);
    while (pos + 8 <= end && fseek(f, (long)pos, SEEK_SET) == 0 && fread(ck, 1, 8, f) == 8) {
        DWORD id = Dword(ck), n = Dword(ck + 4);
        BYTE type[4];

        if (id == FOURCC_LIST && fread(type, 1, 4, f) == 4 && Dword(type) == listtypeAVIMOVIE) {
            movi = pos + 8;
        } else if ((id == FOURCC_LIST && Dword(type) == listtypeAVIHEADER) || id == ckidAVINEWINDEX) {
            data = (BYTE *)malloc(n ? n : 1);
            if (data == NULL || fseek(f, (long)pos + 8, SEEK_SET) != 0 || fread(data, 1, n, f) != n) {
                free(data);
                fclose(f);
                return AVIERR_BADFORMAT;
            }
            if (id == FOURCC_LIST) {
                have_headers = ParseHeaders(af, data + 4, n - 4);
            } else if (have_headers) {
                have_index = ParseIndex(af, data, n, movi);
            }
            free(data);
        }
        pos += 8 + n + (n & 1);
    }
    fclose(f);
    return have_headers && have_index ? 0 : AVIERR_BADFORMAT;
}

/* ---- the calls ---- */

void __stdcall MiniwinAVIFileInit(void) {
}

void __stdcall MiniwinAVIFileExit(void) {
}

long __stdcall MiniwinAVIFileOpenA(void **file, const char *name, int mode, void *handler) {
    struct AviFile *af;
    long result;
    int i;

    (void)mode;
    (void)handler;
    if (file == NULL || name == NULL) {
        return AVIERR_BADPARAM;
    }
    *file = NULL;
    af = (struct AviFile *)calloc(1, sizeof(*af));
    if (af == NULL) {
        return AVIERR_MEMORY;
    }
    af->magic = FILE_MAGIC;
    af->path = (char *)malloc(strlen(name) + 1);
    if (af->path == NULL) {
        FreeFile(af);
        return AVIERR_MEMORY;
    }
    strcpy(af->path, name);
    result = Open(af, name);
    if (result != 0) {
        FreeFile(af);
        return result;
    }
    /* the file's rate in frames per second, from the main header's microseconds per frame */
    af->info.dwRate = 1000000;
    af->info.dwStreams = af->streams;
    af->info.dwCaps = AVIFILECAPS_CANREAD | AVIFILECAPS_CANWRITE; /* as AVIFile reports, though only read here */
    strcpy(af->info.szFileType, "Default AVI file handler");
    for (i = 0; i < af->streams; i++) {
        struct AviStream *s = &af->stream[i];
        const char *base = name, *p;
        int number = 1, k;
        LONG c;

        /* as AVIFile: the suggested buffer is the largest chunk, and a video stream without a frame rectangle
         * gets the picture's size */
        s->info.dwSuggestedBufferSize = 0;
        for (c = 0; c < s->count; c++) {
            if (s->chunks[c].size > s->info.dwSuggestedBufferSize) {
                s->info.dwSuggestedBufferSize = s->chunks[c].size;
            }
        }
        if (s->info.fccType == streamtypeVIDEO && IsRectEmpty(&s->info.rcFrame) &&
            s->format_size >= (LONG)sizeof(BITMAPINFOHEADER)) {
            const BITMAPINFOHEADER *bih = (const BITMAPINFOHEADER *)s->format;

            SetRect(&s->info.rcFrame, 0, 0, bih->biWidth, bih->biHeight < 0 ? -bih->biHeight : bih->biHeight);
        }
        /* a stream without a name of its own gets one like AVIFile's (which is in the system's language):
         * "<file> Video #1" */
        if (s->info.szName[0] != 0) {
            continue;
        }
        for (p = name; *p != 0; p++) {
            if (*p == '\\' || *p == '/' || *p == ':') {
                base = p + 1;
            }
        }
        for (k = 0; k < i; k++) {
            number += af->stream[k].info.fccType == s->info.fccType;
        }
        _snprintf(s->info.szName, sizeof(s->info.szName) - 1, "%s %s #%d", base,
            s->info.fccType == streamtypeVIDEO ? "Video" : s->info.fccType == streamtypeAUDIO ? "Audio"
                                                                                              : "Stream",
            number);
    }
    af->refs = 1;
    *file = af;
    return 0;
}

long __stdcall MiniwinAVIFileInfoA(void *file, void *info, int size) {
    struct AviFile *af = ValidFile(file);

    if (af == NULL) {
        return AVIERR_BADHANDLE;
    }
    if (info == NULL || size < 0) {
        return AVIERR_BADPARAM;
    }
    memcpy(info, &af->info, (size_t)size < sizeof(af->info) ? (size_t)size : sizeof(af->info));
    return (size_t)size < sizeof(af->info) ? AVIERR_BUFFERTOOSMALL : 0;
}

long __stdcall MiniwinAVIFileGetStream(void *file, void **stream, unsigned int type, int n) {
    struct AviFile *af = ValidFile(file);
    int i;

    if (stream != NULL) {
        *stream = NULL;
    }
    if (af == NULL) {
        return AVIERR_BADHANDLE;
    }
    if (stream == NULL) {
        return AVIERR_BADPARAM;
    }
    for (i = 0; i < af->streams; i++) {
        if (type == 0 || af->stream[i].info.fccType == type) {
            if (n-- == 0) {
                af->stream[i].refs++;
                af->refs++; /* a stream keeps its file */
                *stream = &af->stream[i];
                return 0;
            }
        }
    }
    return AVIERR_NODATA;
}

long __stdcall MiniwinAVIFileRelease(void *file) {
    struct AviFile *af = ValidFile(file);
    LONG refs;

    if (af == NULL) {
        return 0;
    }
    refs = --af->refs;
    if (refs == 0) {
        FreeFile(af);
    }
    return refs;
}

long __stdcall MiniwinAVIStreamInfoA(void *stream, void *info, int size) {
    struct AviStream *s = ValidStream(stream);

    if (s == NULL) {
        return AVIERR_BADHANDLE;
    }
    if (info == NULL || size < 0) {
        return AVIERR_BADPARAM;
    }
    memcpy(info, &s->info, (size_t)size < sizeof(s->info) ? (size_t)size : sizeof(s->info));
    return (size_t)size < sizeof(s->info) ? AVIERR_BUFFERTOOSMALL : 0;
}

long __stdcall MiniwinAVIStreamAddRef(void *stream) {
    struct AviStream *s = ValidStream(stream);

    if (s == NULL) {
        return 0;
    }
    s->file->refs++;
    return ++s->refs;
}

long __stdcall MiniwinAVIStreamRelease(void *stream) {
    struct AviStream *s = ValidStream(stream);
    LONG refs;

    if (s == NULL || s->refs == 0) {
        return 0;
    }
    refs = --s->refs;
    MiniwinAVIFileRelease(s->file);
    return refs;
}

long __stdcall MiniwinAVIStreamReadFormat(void *stream, int pos, void *format, int *size) {
    struct AviStream *s = ValidStream(stream);

    (void)pos;
    if (s == NULL) {
        return AVIERR_BADHANDLE;
    }
    if (size == NULL) {
        return AVIERR_BADPARAM;
    }
    if (format == NULL) {
        *size = s->format_size;
        return 0;
    }
    memcpy(format, s->format, *size < s->format_size ? (size_t)*size : (size_t)s->format_size);
    if (*size < s->format_size) {
        *size = s->format_size;
        return AVIERR_BUFFERTOOSMALL;
    }
    *size = s->format_size;
    return 0;
}

long __stdcall MiniwinAVIStreamStart(void *stream) {
    struct AviStream *s = ValidStream(stream);

    return s != NULL ? (long)s->info.dwStart : -1;
}

long __stdcall MiniwinAVIStreamLength(void *stream) {
    struct AviStream *s = ValidStream(stream);

    return s != NULL ? (long)s->info.dwLength : -1;
}

/* The chunk holding byte pos of a sampled stream (binary search on first[]). */
static LONG ChunkOf(const struct AviStream *s, DWORD pos) {
    LONG lo = 0, hi = s->count;

    while (hi - lo > 1) {
        LONG mid = (lo + hi) / 2;

        if (s->first[mid] <= pos) {
            lo = mid;
        } else {
            hi = mid;
        }
    }
    return lo;
}

long __stdcall MiniwinAVIStreamRead(void *stream, int start, int samples, void *buffer, int buffer_size, int *bytes,
    int *samples_out) {
    struct AviStream *s = ValidStream(stream);
    LONG pos;
    DWORD total, from, size, done = 0, n;
    BYTE *out = (BYTE *)buffer;

    if (s == NULL) {
        return AVIERR_BADHANDLE;
    }
    pos = start - (LONG)s->info.dwStart;
    if (pos < 0) {
        return AVIERR_BADPARAM;
    }
    if (bytes != NULL) {
        *bytes = 0;
    }
    if (samples_out != NULL) {
        *samples_out = 0;
    }
    if (s->info.dwSampleSize == 0) {
        /* one chunk per sample, one sample per read */
        if (pos >= s->count) {
            return buffer != NULL ? AVIERR_FILEREAD : AVIERR_ERROR;
        }
        size = s->chunks[pos].size;
        if (buffer != NULL) {
            if ((DWORD)buffer_size < size) {
                if (bytes != NULL) {
                    *bytes = (int)size; /* the size it needs */
                }
                return AVIERR_BUFFERTOOSMALL;
            }
            if (size != 0 && !ReadAt(s->file, s->chunks[pos].offset, out, size)) {
                return AVIERR_FILEREAD;
            }
        }
        if (bytes != NULL) {
            *bytes = (int)size;
        }
        if (samples_out != NULL) {
            *samples_out = 1;
        }
        return 0;
    }
    /* Sampled streams read as AVIFile reads them: as one run of bytes, sample pos starting pos * dwSampleSize bytes
     * in, so a chunk that ends in part of a sample hands that part out too. AVISTREAMREAD_CONVENIENT (samples < 0)
     * reads to the end of the chunk. Reading where not even one whole sample is left is an error. */
    total = s->first[s->count];
    from = (DWORD)pos * s->info.dwSampleSize;
    if (from >= total || total - from < s->info.dwSampleSize) {
        return buffer != NULL ? AVIERR_FILEREAD : AVIERR_ERROR;
    }
    n = samples < 0 ? s->first[ChunkOf(s, from) + 1] - from : (DWORD)samples * s->info.dwSampleSize;
    if (n > total - from) {
        n = total - from;
    }
    if (buffer != NULL && n > (DWORD)buffer_size / s->info.dwSampleSize * s->info.dwSampleSize) {
        n = (DWORD)buffer_size / s->info.dwSampleSize * s->info.dwSampleSize;
    }
    if (buffer != NULL) {
        DWORD at = from;

        while (done < n) {
            LONG c = ChunkOf(s, at);
            DWORD take = s->first[c + 1] - at;

            if (take > n - done) {
                take = n - done;
            }
            if (!ReadAt(s->file, s->chunks[c].offset + (at - s->first[c]), out + done, take)) {
                return AVIERR_FILEREAD;
            }
            done += take;
            at += take;
        }
    }
    if (bytes != NULL) {
        *bytes = (int)n;
    }
    if (samples_out != NULL) {
        *samples_out = (int)(n / s->info.dwSampleSize);
    }
    return 0;
}

long __stdcall MiniwinAVIStreamFindSample(void *stream, long pos, long flags) {
    struct AviStream *s = ValidStream(stream);
    LONG i;

    if (s == NULL) {
        return -1;
    }
    if (s->info.dwSampleSize != 0 || !(flags & FIND_KEY)) {
        return pos; /* every sample is a key sample */
    }
    i = pos - (LONG)s->info.dwStart;
    if (i >= s->count) {
        return -1;
    }
    if (flags & FIND_NEXT) {
        for (i = i < 0 ? 0 : i; i < s->count; i++) {
            if (s->chunks[i].flags & AVIIF_KEYFRAME) {
                return i + (LONG)s->info.dwStart;
            }
        }
        return -1;
    }
    for (; i >= 0; i--) {
        if (s->chunks[i].flags & AVIIF_KEYFRAME) {
            return i + (LONG)s->info.dwStart;
        }
    }
    return -1;
}

void *__stdcall MiniwinAVIStreamGetFrameOpen(void *stream, void *format) {
    (void)stream;
    (void)format;
    return NULL; /* no decompressors: the port decodes the frames (port/port_movie.c) */
}

void *__stdcall MiniwinAVIStreamGetFrame(void *getframe, int position) {
    (void)getframe;
    (void)position;
    return NULL;
}

long __stdcall MiniwinAVIStreamGetFrameClose(void *getframe) {
    (void)getframe;
    return 0;
}
