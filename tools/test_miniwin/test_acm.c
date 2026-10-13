/* test_acm: checks miniwin's ACM (miniwin/src/acm.c) against Windows' own, without starting the game: the same
 * calls on both, comparing results, sizes, header fields and every decoded byte, on made-up blocks of every format
 * the game's data uses (MS ADPCM mono and stereo, IMA ADPCM stereo) and a few it could meet. Given the data folder,
 * it also decodes every speech file (in the game's chunks, as SpeechLoadWavFile and its reader do) and the ADPCM
 * sound of every movie in FMV\.
 *
 *   tools/test_miniwin/build.sh && tools/test_miniwin/test_acm.exe [data folder]   (prints FAIL lines, exits 1 on any) */
#include <windows.h>
#include <mmreg.h>
#include <msacm.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "miniwin/acm.h"

static int failures;
static int checks;
static unsigned int seed = 12345;

static unsigned int Random(void) {
    seed = seed * 1103515245 + 12345;
    return seed >> 8;
}

static void Same(const char *what, long a, long b) {
    checks++;
    if (a != b) {
        printf("FAIL %s: %ld (ACM) vs %ld (miniwin)\n", what, a, b);
        failures++;
    }
}

static void SameBytes(const char *what, const BYTE *a, const BYTE *b, unsigned int n) {
    unsigned int i;

    checks++;
    for (i = 0; i < n; i++) {
        if (a[i] != b[i]) {
            printf("FAIL %s: byte %u of %u: %02x (ACM) vs %02x (miniwin)\n", what, i, n, a[i], b[i]);
            failures++;
            return;
        }
    }
}

/* Room for any format: WAVEFORMATEX plus MS ADPCM's coefficients. */
union Format {
    WAVEFORMATEX wfx;
    BYTE bytes[256];
};

static const short standard_coefs[7][2] = {{256, 0}, {512, -256}, {0, 0}, {192, 64}, {240, 0}, {460, -208},
    {392, -232}};

static void MakePcm(union Format *f, int channels, int rate, int bits) {
    memset(f, 0, sizeof(*f));
    f->wfx.wFormatTag = WAVE_FORMAT_PCM;
    f->wfx.nChannels = (WORD)channels;
    f->wfx.nSamplesPerSec = rate;
    f->wfx.wBitsPerSample = (WORD)bits;
    f->wfx.nBlockAlign = (WORD)(channels * bits / 8);
    f->wfx.nAvgBytesPerSec = rate * f->wfx.nBlockAlign;
}

static void MakeMs(union Format *f, int channels, int rate, int align) {
    ADPCMWAVEFORMAT *ms = (ADPCMWAVEFORMAT *)f;
    int i;

    memset(f, 0, sizeof(*f));
    ms->wfx.wFormatTag = WAVE_FORMAT_ADPCM;
    ms->wfx.nChannels = (WORD)channels;
    ms->wfx.nSamplesPerSec = rate;
    ms->wfx.nBlockAlign = (WORD)align;
    ms->wfx.wBitsPerSample = 4;
    ms->wfx.cbSize = 32;
    ms->wSamplesPerBlock = (WORD)((align - 7 * channels) * 2 / channels + 2);
    ms->wfx.nAvgBytesPerSec = rate * align / ms->wSamplesPerBlock;
    ms->wNumCoef = 7;
    for (i = 0; i < 7; i++) {
        ms->aCoef[i].iCoef1 = standard_coefs[i][0];
        ms->aCoef[i].iCoef2 = standard_coefs[i][1];
    }
}

static void MakeIma(union Format *f, int channels, int rate, int align) {
    IMAADPCMWAVEFORMAT *ima = (IMAADPCMWAVEFORMAT *)f;

    memset(f, 0, sizeof(*f));
    ima->wfx.wFormatTag = WAVE_FORMAT_IMA_ADPCM;
    ima->wfx.nChannels = (WORD)channels;
    ima->wfx.nSamplesPerSec = rate;
    ima->wfx.nBlockAlign = (WORD)align;
    ima->wfx.wBitsPerSample = 4;
    ima->wfx.cbSize = 2;
    ima->wSamplesPerBlock = (WORD)((align - 4 * channels) * 2 / channels + 1);
    ima->wfx.nAvgBytesPerSec = rate * align / ima->wSamplesPerBlock;
}

/* The game's destination: 16-bit PCM in the source's channels and rate (or 8-bit, which ACM can do too). */
static int dst_bits = 16;

static void Destination(union Format *d, const WAVEFORMATEX *src) {
    MakePcm(d, src->nChannels, src->nSamplesPerSec, dst_bits);
}

/* Made-up source blocks with plausible headers; "wild" ones have out-of-range header fields too. */
static void FillBlocks(const WAVEFORMATEX *f, BYTE *p, unsigned int bytes, int wild) {
    unsigned int i, b;
    int c, ch = f->nChannels;

    for (i = 0; i < bytes; i++) {
        p[i] = (BYTE)Random();
    }
    if (f->wFormatTag == WAVE_FORMAT_PCM) {
        return;
    }
    for (b = 0; b + f->nBlockAlign <= bytes; b += f->nBlockAlign) {
        BYTE *h = p + b;

        if (f->wFormatTag == WAVE_FORMAT_ADPCM) {
            for (c = 0; c < ch; c++) {
                h[c] = (BYTE)(wild ? Random() % 9 : Random() % 7);
            }
            for (c = 0; c < ch; c++) {
                int delta = wild ? (int)(Random() % 65536) - 32768 : 16 + (int)(Random() % 2000);

                h[ch + 2 * c] = (BYTE)delta;
                h[ch + 2 * c + 1] = (BYTE)(delta >> 8);
            }
        } else {
            for (c = 0; c < ch; c++) {
                h[4 * c + 2] = (BYTE)(wild ? Random() % 256 : Random() % 89);
                h[4 * c + 3] = (BYTE)(wild ? Random() : 0);
            }
        }
    }
}

struct Pair {
    HACMSTREAM real;
    void *mini;
};

static int OpenBoth(const char *what, struct Pair *s, const WAVEFORMATEX *src, const WAVEFORMATEX *dst) {
    MMRESULT a = acmStreamOpen(&s->real, NULL, (LPWAVEFORMATEX)src, (LPWAVEFORMATEX)dst, NULL, 0, 0,
        ACM_STREAMOPENF_NONREALTIME);
    int b = MiniwinAcmStreamOpen(&s->mini, NULL, (void *)src, (void *)dst, NULL, 0, 0, ACM_STREAMOPENF_NONREALTIME);
    char name[200];

    sprintf(name, "%s: open", what);
    Same(name, a, b);
    if (a != 0 && b == 0) {
        MiniwinAcmStreamClose(s->mini, 0);
    }
    if (a == 0 && b != 0) {
        acmStreamClose(s->real, 0);
    }
    return a == 0 && b == 0;
}

static void CloseBoth(struct Pair *s) {
    acmStreamClose(s->real, 0);
    MiniwinAcmStreamClose(s->mini, 0);
}

static void Sizes(const char *what, struct Pair *s, const WAVEFORMATEX *src) {
    unsigned int align = src->nBlockAlign, out;
    unsigned int inputs[] = {0, 1, 2, 3, 4, 7, align - 1, align, align + 1, 2 * align - 1, 2 * align, 10 * align,
        10 * align + 5, 1470, 2940, 5880, 4048, 4068, 4069, 8136, 2024, 2023, 2025, 4096, 65536};
    int i, dir;

    for (dir = 0; dir < 2; dir++) {
        DWORD flag = dir == 0 ? ACM_STREAMSIZEF_SOURCE : ACM_STREAMSIZEF_DESTINATION;

        for (i = 0; i < (int)(sizeof(inputs) / sizeof(inputs[0])); i++) {
            DWORD ra = 0xdeadbeef;
            unsigned int rb = 0xdeadbeef;
            MMRESULT a = acmStreamSize(s->real, inputs[i], &ra, flag);
            int b = MiniwinAcmStreamSize(s->mini, inputs[i], &rb, flag);
            char name[200];

            sprintf(name, "%s: size of %u %s bytes (result)", what, inputs[i], dir == 0 ? "source" : "destination");
            Same(name, a, b);
            if (a == 0 && b == 0) {
                sprintf(name, "%s: size of %u %s bytes", what, inputs[i], dir == 0 ? "source" : "destination");
                Same(name, (long)ra, (long)rb);
            }
        }
        (void)out;
    }
}

/* One prepare / convert / unprepare on both sides, comparing everything. */
static void ConvertBoth(const char *what, struct Pair *s, const BYTE *src, unsigned int src_len, unsigned int dst_len,
    DWORD flags) {
    ACMSTREAMHEADER ha, hb;
    BYTE *da = (BYTE *)malloc(dst_len + 16), *db = (BYTE *)malloc(dst_len + 16);
    BYTE *sa = (BYTE *)malloc(src_len + 1), *sb = (BYTE *)malloc(src_len + 1);
    MMRESULT a;
    int b;
    char name[200];

    memcpy(sa, src, src_len);
    memcpy(sb, src, src_len);
    memset(da, 0xcd, dst_len + 16);
    memset(db, 0xcd, dst_len + 16);
    memset(&ha, 0, sizeof(ha));
    ha.cbStruct = sizeof(ha);
    ha.pbSrc = sa;
    ha.cbSrcLength = src_len;
    ha.pbDst = da;
    ha.cbDstLength = dst_len;
    hb = ha;
    hb.pbSrc = sb;
    hb.pbDst = db;
    a = acmStreamPrepareHeader(s->real, &ha, 0);
    b = MiniwinAcmStreamPrepareHeader(s->mini, &hb, 0);
    sprintf(name, "%s: prepare", what);
    Same(name, a, b);
    sprintf(name, "%s: status after prepare", what);
    Same(name, ha.fdwStatus, hb.fdwStatus);
    a = acmStreamConvert(s->real, &ha, flags);
    b = MiniwinAcmStreamConvert(s->mini, &hb, flags);
    sprintf(name, "%s: convert", what);
    Same(name, a, b);
    sprintf(name, "%s: status after convert", what);
    Same(name, ha.fdwStatus, hb.fdwStatus);
    sprintf(name, "%s: source used", what);
    Same(name, ha.cbSrcLengthUsed, hb.cbSrcLengthUsed);
    sprintf(name, "%s: destination used", what);
    Same(name, ha.cbDstLengthUsed, hb.cbDstLengthUsed);
    sprintf(name, "%s: output", what);
    SameBytes(name, da, db, dst_len + 16);
    acmStreamUnprepareHeader(s->real, &ha, 0);
    MiniwinAcmStreamUnprepareHeader(s->mini, &hb, 0);
    sprintf(name, "%s: status after unprepare", what);
    Same(name, ha.fdwStatus, hb.fdwStatus);
    free(da);
    free(db);
    free(sa);
    free(sb);
}

static void Converts(const char *what, struct Pair *s, const WAVEFORMATEX *src, int wild) {
    unsigned int align = src->nBlockAlign, out_block, len;
    BYTE *data = (BYTE *)malloc(12 * align + 64);
    char name[200];

    acmStreamSize(s->real, align, (DWORD *)&out_block, ACM_STREAMSIZEF_SOURCE);
    FillBlocks(src, data, 12 * align + 64, wild);
    sprintf(name, "%s, 10 blocks", what);
    ConvertBoth(name, s, data, 10 * align, 10 * out_block, ACM_STREAMCONVERTF_BLOCKALIGN);
    sprintf(name, "%s, 10 blocks, no flags", what);
    ConvertBoth(name, s, data, 10 * align, 10 * out_block, 0);
    len = 3 * align + align / 2;
    sprintf(name, "%s, 3.5 blocks", what);
    ConvertBoth(name, s, data, len, 4 * out_block, ACM_STREAMCONVERTF_BLOCKALIGN);
    sprintf(name, "%s, 3.5 blocks, no flags", what);
    ConvertBoth(name, s, data, len, 4 * out_block, 0);
    sprintf(name, "%s, half a block", what);
    ConvertBoth(name, s, data, align / 2, out_block, ACM_STREAMCONVERTF_BLOCKALIGN);
    sprintf(name, "%s, room for 1.5 blocks", what);
    ConvertBoth(name, s, data, 3 * align, out_block + out_block / 2, ACM_STREAMCONVERTF_BLOCKALIGN);
    free(data);
}

/* The speech reader's pattern: one prepare, then many converts of the same header, the last one shorter. */
static void SpeechPattern(const char *what, struct Pair *s, const WAVEFORMATEX *src) {
    unsigned int chunk = src->nBlockAlign * 10, out;
    ACMSTREAMHEADER ha, hb;
    BYTE *data = (BYTE *)malloc(chunk), *da, *db;
    int i;
    char name[200];

    acmStreamSize(s->real, chunk, (DWORD *)&out, ACM_STREAMSIZEF_SOURCE);
    da = (BYTE *)malloc(out);
    db = (BYTE *)malloc(out);
    memset(&ha, 0, sizeof(ha));
    ha.cbStruct = 0x54;
    ha.pbSrc = data;
    ha.cbSrcLength = chunk;
    ha.pbDst = da;
    ha.cbDstLength = out;
    hb = ha;
    hb.pbDst = db;
    Same("speech pattern: prepare", acmStreamPrepareHeader(s->real, &ha, 0),
        MiniwinAcmStreamPrepareHeader(s->mini, &hb, 0));
    for (i = 0; i < 4; i++) {
        unsigned int len = i == 3 ? chunk / 3 : chunk;

        FillBlocks(src, data, chunk, 0);
        ha.cbSrcLength = hb.cbSrcLength = len;
        sprintf(name, "%s: speech pattern convert %d", what, i);
        Same(name, acmStreamConvert(s->real, &ha, ACM_STREAMCONVERTF_BLOCKALIGN),
            MiniwinAcmStreamConvert(s->mini, &hb, ACM_STREAMCONVERTF_BLOCKALIGN));
        Same(name, ha.fdwStatus, hb.fdwStatus);
        Same(name, ha.cbSrcLengthUsed, hb.cbSrcLengthUsed);
        Same(name, ha.cbDstLengthUsed, hb.cbDstLengthUsed);
        SameBytes(name, da, db, ha.cbDstLengthUsed);
    }
    acmStreamUnprepareHeader(s->real, &ha, 0);
    MiniwinAcmStreamUnprepareHeader(s->mini, &hb, 0);
    free(data);
    free(da);
    free(db);
}

static void Format(const char *what, const WAVEFORMATEX *src) {
    union Format dst;
    struct Pair s;

    Destination(&dst, src);
    if (!OpenBoth(what, &s, src, &dst.wfx)) {
        return;
    }
    Sizes(what, &s, src);
    Converts(what, &s, src, 0);
    Converts(what, &s, src, 1);
    SpeechPattern(what, &s, src);
    CloseBoth(&s);
}

/* ---- the game's data ---- */

static BYTE *ReadAll(const char *path, unsigned int *size) {
    FILE *f = fopen(path, "rb");
    BYTE *p;
    long n;

    if (f == NULL) {
        return NULL;
    }
    fseek(f, 0, SEEK_END);
    n = ftell(f);
    fseek(f, 0, SEEK_SET);
    p = (BYTE *)malloc(n + 1);
    fread(p, 1, n, f);
    fclose(f);
    *size = (unsigned int)n;
    return p;
}

/* Decodes a whole stream in the given chunk size on both sides and compares. */
static int DecodeBoth(const char *what, const WAVEFORMATEX *fmt, const BYTE *data, unsigned int size,
    unsigned int chunk) {
    union Format dst;
    struct Pair s;
    unsigned int pos, out;
    ACMSTREAMHEADER ha, hb;
    BYTE *da, *db;
    int bad = 0;

    Destination(&dst, fmt);
    if (!OpenBoth(what, &s, fmt, &dst.wfx)) {
        return 1;
    }
    acmStreamSize(s.real, chunk, (DWORD *)&out, ACM_STREAMSIZEF_SOURCE);
    da = (BYTE *)malloc(out);
    db = (BYTE *)malloc(out);
    for (pos = 0; pos < size && !bad; pos += chunk) {
        unsigned int len = size - pos < chunk ? size - pos : chunk;

        memset(&ha, 0, sizeof(ha));
        ha.cbStruct = sizeof(ha);
        ha.pbSrc = (LPBYTE)data + pos;
        ha.cbSrcLength = len;
        ha.pbDst = da;
        ha.cbDstLength = out;
        hb = ha;
        hb.pbDst = db;
        acmStreamPrepareHeader(s.real, &ha, 0);
        MiniwinAcmStreamPrepareHeader(s.mini, &hb, 0);
        acmStreamConvert(s.real, &ha, ACM_STREAMCONVERTF_BLOCKALIGN);
        MiniwinAcmStreamConvert(s.mini, &hb, ACM_STREAMCONVERTF_BLOCKALIGN);
        acmStreamUnprepareHeader(s.real, &ha, 0);
        MiniwinAcmStreamUnprepareHeader(s.mini, &hb, 0);
        if (ha.cbDstLengthUsed != hb.cbDstLengthUsed || ha.cbSrcLengthUsed != hb.cbSrcLengthUsed ||
            memcmp(da, db, ha.cbDstLengthUsed) != 0) {
            Same(what, ha.cbSrcLengthUsed, hb.cbSrcLengthUsed);
            Same(what, ha.cbDstLengthUsed, hb.cbDstLengthUsed);
            SameBytes(what, da, db, ha.cbDstLengthUsed);
            bad = 1;
        }
    }
    checks++;
    free(da);
    free(db);
    CloseBoth(&s);
    return bad;
}

static const BYTE *FindChunk(const BYTE *p, const BYTE *end, const char *id, unsigned int *size) {
    while (p + 8 <= end) {
        unsigned int n = p[4] | p[5] << 8 | p[6] << 16 | (unsigned int)p[7] << 24;

        if (memcmp(p, id, 4) == 0) {
            *size = n;
            return p + 8;
        }
        if (memcmp(p, "RIFF", 4) == 0 || memcmp(p, "LIST", 4) == 0) {
            p += 12; /* look inside */
        } else {
            p += 8 + n + (n & 1);
        }
    }
    return NULL;
}

static void Speech(const char *folder) {
    char pattern[MAX_PATH], path[MAX_PATH];
    WIN32_FIND_DATAA fd;
    HANDLE find;
    int files = 0, bad = 0;

    sprintf(pattern, "%s\\speech\\*.wav", folder);
    find = FindFirstFileA(pattern, &fd);
    if (find == INVALID_HANDLE_VALUE) {
        printf("no %s\n", pattern);
        failures++;
        return;
    }
    do {
        unsigned int size, fmt_size, data_size;
        BYTE *file;
        const BYTE *fmt, *data;

        sprintf(path, "%s\\speech\\%s", folder, fd.cFileName);
        file = ReadAll(path, &size);
        fmt = file != NULL ? FindChunk(file, file + size, "fmt ", &fmt_size) : NULL;
        data = file != NULL ? FindChunk(file, file + size, "data", &data_size) : NULL;
        if (fmt == NULL || data == NULL) {
            printf("FAIL %s: not a WAV\n", path);
            failures++;
        } else {
            if (data + data_size > file + size) {
                data_size = (unsigned int)(file + size - data);
            }
            bad += DecodeBoth(fd.cFileName, (const WAVEFORMATEX *)fmt, data, data_size,
                ((const WAVEFORMATEX *)fmt)->nBlockAlign * 10);
            files++;
        }
        free(file);
    } while (FindNextFileA(find, &fd));
    FindClose(find);
    printf("speech: %d files decoded, %d differ\n", files, bad);
}

/* A movie's sound: the audio stream's format and its chunks ("01wb") in order. */
static void Movies(const char *folder) {
    char pattern[MAX_PATH], path[MAX_PATH];
    WIN32_FIND_DATAA fd;
    HANDLE find;
    int movies = 0, bad = 0;

    sprintf(pattern, "%s\\FMV\\*.avi", folder);
    find = FindFirstFileA(pattern, &fd);
    if (find == INVALID_HANDLE_VALUE) {
        printf("no %s\n", pattern);
        failures++;
        return;
    }
    do {
        unsigned int size, n, total = 0;
        BYTE *file, *audio;
        const BYTE *p, *fmt = NULL, *end;

        sprintf(path, "%s\\FMV\\%s", folder, fd.cFileName);
        file = ReadAll(path, &size);
        if (file == NULL) {
            continue;
        }
        end = file + size;
        /* the strf after the "auds" strh */
        for (p = file; p + 64 < end; p++) {
            if (memcmp(p, "strh", 4) == 0 && memcmp(p + 8, "auds", 4) == 0) {
                fmt = FindChunk(p, end, "strf", &n);
                break;
            }
        }
        if (fmt == NULL || (((const WAVEFORMATEX *)fmt)->wFormatTag != WAVE_FORMAT_ADPCM && ((const WAVEFORMATEX *)fmt)->wFormatTag != WAVE_FORMAT_IMA_ADPCM)) {
            free(file);
            continue;
        }
        audio = (BYTE *)malloc(size);
        p = FindChunk(file, end, "movi", &n);
        p = file + 12;
        while ((p = FindChunk(p, end, "01wb", &n)) != NULL) {
            if (p + n > end) {
                break;
            }
            memcpy(audio + total, p, n);
            total += n;
            p += n + (n & 1);
        }
        bad += DecodeBoth(fd.cFileName, (const WAVEFORMATEX *)fmt, audio, total,
            ((const WAVEFORMATEX *)fmt)->nBlockAlign);
        movies++;
        printf("movie %s: %u bytes of sound\n", fd.cFileName, total);
        free(audio);
        free(file);
    } while (FindNextFileA(find, &fd));
    FindClose(find);
    printf("movies: %d decoded, %d differ\n", movies, bad);
}

int main(int argc, char **argv) {
    union Format f, d;
    struct Pair s;

    MakeMs(&f, 1, 22050, 512);
    Format("MS ADPCM mono 512", &f.wfx);
    MakeMs(&f, 2, 22050, 1024);
    Format("MS ADPCM stereo 1024", &f.wfx);
    MakeIma(&f, 2, 22050, 1024);
    Format("IMA ADPCM stereo 1024", &f.wfx);
    MakeIma(&f, 1, 22050, 512);
    Format("IMA ADPCM mono 512", &f.wfx);
    MakePcm(&f, 1, 22050, 8);
    Format("PCM 8-bit mono", &f.wfx);
    MakePcm(&f, 2, 44100, 16);
    Format("PCM 16-bit stereo", &f.wfx);

    /* conversions the game never asks for */
    dst_bits = 8;
    MakeMs(&f, 2, 22050, 1024);
    Format("MS ADPCM stereo to 8-bit", &f.wfx);
    MakeIma(&f, 1, 22050, 512);
    Format("IMA ADPCM mono to 8-bit", &f.wfx);
    MakePcm(&f, 2, 44100, 16);
    Format("PCM 16-bit stereo to 8-bit", &f.wfx);
    dst_bits = 16;
    MakeMs(&f, 1, 22050, 512);
    MakePcm(&d, 2, 22050, 16);
    if (OpenBoth("MS ADPCM mono to stereo", &s, &f.wfx, &d.wfx)) {
        CloseBoth(&s);
    }
    MakePcm(&d, 1, 44100, 16);
    if (OpenBoth("MS ADPCM to another rate", &s, &f.wfx, &d.wfx)) {
        CloseBoth(&s);
    }

    if (argc > 1) {
        Speech(argv[1]);
        Movies(argv[1]);
    }
    printf("%d checks, %d failed\n", checks, failures);
    return failures != 0;
}
