/* [library:movie] Indeo 3, Indeo 5 and Cinepak frames for AVI streams, decoded by the port instead of a Video for Windows codec.
 * See port_movie.h. The streams are miniwin's (miniwin/avifile.h); vfw.h is here for its types and constants. */
#include <windows.h>
#include <dsound.h>
#include <vfw.h>
#include <SDL3/SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ffmpeg/port_ffmpeg.h"
#include "miniwin/avifile.h"
#include "miniwin/miniwin.h"
#include "miniwin/timer.h"
#include "port_movie.h"

/* One stream is decoded at a time (the game shows one advisor animation or one movie at once). Frames depend
 * on the previous ones, so the decoder is kept between calls and fed every sample from the last key frame. */
static struct {
    PAVISTREAM stream;
    PortVideoDecoder *decoder;
    int width;
    int height;
    long position; /* last sample fed to the decoder, -1 for none */
    int have_picture; /* the DIB holds a decoded picture */
    unsigned char *sample;
    long sample_size;
    BITMAPINFOHEADER *dib; /* header + RGB555 pixels */
} movie;

static void ResetMovie(void) {
    if (movie.decoder != NULL) {
        PortVideoDecoder_Close(movie.decoder);
    }
    free(movie.sample);
    free(movie.dib);
    memset(&movie, 0, sizeof(movie));
    movie.position = -1;
}

static int OpenMovie(PAVISTREAM stream) {
    BITMAPINFOHEADER format;
    BITMAPINFOHEADER *full;
    LONG format_size = 0;

    ResetMovie();
    /* the stream's format can be longer than a BITMAPINFOHEADER (codec data after it) */
    if (MiniwinAVIStreamReadFormat(stream, 0, NULL, (int *)&format_size) != 0 || format_size < (LONG)sizeof(format)) {
        return 0;
    }
    full = (BITMAPINFOHEADER *)calloc(1, format_size);
    if (full == NULL) {
        return 0;
    }
    if (MiniwinAVIStreamReadFormat(stream, 0, full, (int *)&format_size) != 0) {
        free(full);
        return 0;
    }
    format = *full;
    free(full);
    if (format.biWidth <= 0 || format.biHeight <= 0 || format.biWidth > 1024 || format.biHeight > 1024) {
        return 0;
    }
    movie.width = format.biWidth;
    movie.height = format.biHeight;
    movie.dib = (BITMAPINFOHEADER *)calloc(1, sizeof(BITMAPINFOHEADER) + (size_t)movie.width * movie.height * 2);
    movie.decoder = PortVideoDecoder_Open(format.biCompression, movie.width, movie.height, format.biBitCount);
    if (movie.dib == NULL || movie.decoder == NULL) {
        ResetMovie();
        return 0;
    }
    movie.dib->biSize = sizeof(BITMAPINFOHEADER);
    movie.dib->biWidth = movie.width;
    movie.dib->biHeight = movie.height;
    movie.dib->biPlanes = 1;
    movie.dib->biBitCount = 16;
    movie.dib->biCompression = BI_RGB;
    movie.dib->biSizeImage = movie.width * movie.height * 2;
    movie.stream = stream;
    return 1;
}

static int Clamp255(int v) { return v < 0 ? 0 : (v > 255 ? 255 : v); }

/* YVU9 (4x4 chroma subsampling) to bottom-up RGB555, BT.601 studio range as the Indeo codec output it. */
static void ConvertPicture(const unsigned char *planes[3], const int linesize[3]) {
    unsigned short *pixels = (unsigned short *)(movie.dib + 1);
    int x;
    int y;

    for (y = 0; y < movie.height; y++) {
        const unsigned char *luma = planes[0] + (size_t)y * linesize[0];
        const unsigned char *cb = planes[1] + (size_t)(y >> 2) * linesize[1];
        const unsigned char *cr = planes[2] + (size_t)(y >> 2) * linesize[2];
        unsigned short *out = pixels + (size_t)(movie.height - 1 - y) * movie.width;

        for (x = 0; x < movie.width; x++) {
            int c = 298 * (luma[x] - 16);
            int d = cb[x >> 2] - 128;
            int e = cr[x >> 2] - 128;
            int r = Clamp255((c + 409 * e + 128) >> 8);
            int g = Clamp255((c - 100 * d - 208 * e + 128) >> 8);
            int b = Clamp255((c + 516 * d + 128) >> 8);

            out[x] = (unsigned short)(((r >> 3) << 10) | ((g >> 3) << 5) | (b >> 3));
        }
    }
}

/* RGB24, top-down (Cinepak), to bottom-up RGB555, packed as the YUV pictures are */
static void ConvertRgbPicture(const unsigned char *rgb, int linesize) {
    unsigned short *pixels = (unsigned short *)(movie.dib + 1);
    int x;
    int y;

    for (y = 0; y < movie.height; y++) {
        const unsigned char *in = rgb + (size_t)y * linesize;
        unsigned short *out = pixels + (size_t)(movie.height - 1 - y) * movie.width;

        for (x = 0; x < movie.width; x++) {
            int r = in[x * 3], g = in[x * 3 + 1], b = in[x * 3 + 2];

            out[x] = (unsigned short)(((r >> 3) << 10) | ((g >> 3) << 5) | (b >> 3));
        }
    }
}

static int DecodeSample(long position) {
    const unsigned char *planes[3];
    int linesize[3];
    LONG bytes = 0;
    int result;

    if (MiniwinAVIStreamRead(movie.stream, position, 1, NULL, 0, (int *)&bytes, NULL) != 0 || bytes < 0) {
        return 0;
    }
    movie.position = position;
    if (bytes == 0) {
        return 1; /* a dropped frame: the picture stays as it was */
    }
    if (bytes > movie.sample_size) {
        /* the decoder may read a little past the end of the data */
        unsigned char *grown = (unsigned char *)realloc(movie.sample, bytes + PORT_FFMPEG_PADDING);
        if (grown == NULL) {
            return 0;
        }
        movie.sample = grown;
        movie.sample_size = bytes;
    }
    if (MiniwinAVIStreamRead(movie.stream, position, 1, movie.sample, bytes, (int *)&bytes, NULL) != 0) {
        return 0;
    }
    memset(movie.sample + bytes, 0, PORT_FFMPEG_PADDING);
    result = PortVideoDecoder_Decode(movie.decoder, movie.sample, bytes, planes, linesize);
    if (result < 0) {
        return 0;
    }
    if (result > 0) {
        if (PortVideoDecoder_IsRgb(movie.decoder)) {
            ConvertRgbPicture(planes[0], linesize[0]);
        } else {
            ConvertPicture(planes, linesize);
        }
        movie.have_picture = 1;
    }
    return 1;
}

void *PortMovieGetFrame(void *stream, long position) {
    long start;

    if (stream == NULL || position < 0) {
        return NULL;
    }
    if (movie.stream != (PAVISTREAM)stream && OpenMovie((PAVISTREAM)stream) == 0) {
        return NULL;
    }
    if (position != movie.position) {
        /* decode from the key frame at or before the wanted one, or carry on from the last decoded frame
         * when that is closer */
        start = MiniwinAVIStreamFindSample(movie.stream, position, FIND_KEY | FIND_PREV);
        if (start < 0) {
            start = 0;
        }
        if (movie.position >= 0 && position > movie.position && start <= movie.position) {
            start = movie.position + 1;
        }
        for (; start <= position; start++) {
            if (DecodeSample(start) == 0) {
                movie.position = -1;
                return NULL;
            }
        }
    }
    return movie.have_picture ? movie.dib : NULL;
}

void PortMovieRelease(void *stream) {
    if (stream != NULL && movie.stream == (PAVISTREAM)stream) {
        ResetMovie();
    }
}

static double Milliseconds(void) {
    return (double)SDL_GetTicksNS() / 1000000.0;
}

/* one movie: every frame in order (what the game's movie player asks for), timed */
static int TimeMovie(FILE *report, const char *name) {
    char path[MAX_PATH];
    void *file = NULL;
    void *stream = NULL;
    AVISTREAMINFOA info;
    long start;
    long length;
    long i;
    double total = 0;
    double worst = 0;
    double rate;
    int problems = 0;

    sprintf(path, "FMV\\%s", name);
    if (MiniwinAVIFileOpenA(&file, path, OF_READ, NULL) != 0 ||
        MiniwinAVIFileGetStream(file, &stream, streamtypeVIDEO, 0) != 0) {
        fprintf(report, "movie %s: can't open\n", name);
        if (file != NULL) {
            MiniwinAVIFileRelease(file);
        }
        return 1;
    }
    memset(&info, 0, sizeof(info));
    MiniwinAVIStreamInfoA(stream, &info, sizeof(info));
    rate = info.dwScale != 0 ? (double)info.dwRate / info.dwScale : 0;
    start = MiniwinAVIStreamStart(stream);
    length = MiniwinAVIStreamLength(stream);
    for (i = start; i < start + length; i++) {
        double before = Milliseconds();
        double took;

        if (PortMovieGetFrame(stream, i) == NULL) {
            fprintf(report, "movie %s: frame %ld doesn't decode\n", name, i);
            problems = 1;
            break;
        }
        took = Milliseconds() - before;
        total += took;
        if (took > worst) {
            worst = took;
        }
    }
    if (problems == 0) {
        fprintf(report, "movie %s: %ld frames %dx%d at %.1f a second (%.1f ms each): decoded in %.2f ms a frame, the "
                        "slowest %.2f ms\n",
            name, length, movie.width, movie.height, rate, rate > 0 ? 1000.0 / rate : 0.0,
            length > 0 ? total / length : 0.0, worst);
    }
    PortMovieRelease(stream);
    MiniwinAVIStreamRelease(stream);
    MiniwinAVIFileRelease(file);
    return problems;
}

/* The movie's sound while the game waits: a buffer plays (silence) and the game polls its play cursor and the clock
 * for a second, as the movie player does between frames. The cursor must keep up with the clock; in a browser it
 * moves only when the page gets its turn (miniwin/timer.h). No sound device: not checked. */
static int TimeMovieSound(FILE *report) {
    LPDIRECTSOUND ds = NULL;
    LPDIRECTSOUNDBUFFER buffer = NULL;
    DSBUFFERDESC desc;
    WAVEFORMATEX format;
    DWORD start;
    DWORD last = 0;
    DWORD moved = 0;
    DWORD play;
    DWORD write;
    double ms;
    int problems = 0;

#ifdef LEGOLAND_WEB_NODE
    (void)ds;
    (void)buffer;
    fprintf(report, "movie sound: Node.js plays no sound, not checked\n");
    return 0;
#endif
    if (MiniwinDirectSoundCreate(NULL, &ds, NULL) != DS_OK) {
        fprintf(report, "movie sound: no sound device, not checked\n");
        return 0;
    }
    memset(&format, 0, sizeof(format));
    format.wFormatTag = WAVE_FORMAT_PCM;
    format.nChannels = 1;
    format.nSamplesPerSec = 22050;
    format.wBitsPerSample = 16;
    format.nBlockAlign = 2;
    format.nAvgBytesPerSec = 44100;
    memset(&desc, 0, sizeof(desc));
    desc.dwSize = sizeof(desc);
    desc.dwFlags = DSBCAPS_GLOBALFOCUS;
    desc.dwBufferBytes = 44100 / 4; /* a quarter of a second, looping */
    desc.lpwfxFormat = &format;
    if (ds->lpVtbl->CreateSoundBuffer(ds, &desc, &buffer, NULL) != DS_OK) {
        fprintf(report, "movie sound: can't create a buffer\n");
        ds->lpVtbl->Release(ds);
        return 1;
    }
    buffer->lpVtbl->Play(buffer, 0, 0, DSBPLAY_LOOPING);
    start = MiniwinGetTickCount();
    while (MiniwinGetTickCount() - start < 1000) {
        if (buffer->lpVtbl->GetCurrentPosition(buffer, &play, &write) == DS_OK) {
            moved += (play + desc.dwBufferBytes - last) % desc.dwBufferBytes;
            last = play;
        }
    }
    buffer->lpVtbl->Stop(buffer);
    ms = moved * 1000.0 / format.nAvgBytesPerSec;
    fprintf(report, "movie sound: played %.0f ms while the game polled for 1000 ms\n", ms);
    if (ms < 500) {
        fprintf(report, "movie sound: the play cursor doesn't keep up with the clock\n");
        problems = 1;
    }
    buffer->lpVtbl->Release(buffer);
    ds->lpVtbl->Release(ds);
    return problems;
}

int PortMovieSelfTest(FILE *report) {
    WIN32_FIND_DATAA found;
    HANDLE find;
    int problems = 0;
    int movies = 0;

    MiniwinAVIFileInit();
    find = FindFirstFileA("FMV\\*.avi", &found);
    if (find != INVALID_HANDLE_VALUE) {
        do {
            problems += TimeMovie(report, found.cFileName);
            movies++;
        } while (FindNextFileA(find, &found));
        FindClose(find);
    }
    fprintf(report, "movies: %d decoded (FMV\\)\n", movies - problems);
    problems += TimeMovieSound(report);
    MiniwinAVIFileExit();
    return problems;
}
