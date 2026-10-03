/* [library:movie] Indeo 5 frames for AVI streams, decoded by the port instead of a Video for Windows codec.
 * See port_movie.h. */
#include <windows.h>
#include <vfw.h>
#include <stdlib.h>
#include <string.h>

#include "indeo5/port_indeo5.h"
#include "port_movie.h"

#define IV50_FOURCC mmioFOURCC('I', 'V', '5', '0')

/* One stream is decoded at a time (the game shows one advisor animation or one movie at once). Frames depend
 * on the previous ones, so the decoder is kept between calls and fed every sample from the last key frame. */
static struct {
    PAVISTREAM stream;
    PortIndeo5 *decoder;
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
        PortIndeo5_Close(movie.decoder);
    }
    free(movie.sample);
    free(movie.dib);
    memset(&movie, 0, sizeof(movie));
    movie.position = -1;
}

static int OpenMovie(PAVISTREAM stream) {
    BITMAPINFOHEADER format;
    LONG format_size = sizeof(format);

    ResetMovie();
    memset(&format, 0, sizeof(format));
    if (AVIStreamReadFormat(stream, 0, &format, &format_size) != 0 || format.biCompression != IV50_FOURCC ||
        format.biWidth <= 0 || format.biHeight <= 0 || format.biWidth > 1024 || format.biHeight > 1024) {
        return 0;
    }
    movie.width = format.biWidth;
    movie.height = format.biHeight;
    movie.dib = (BITMAPINFOHEADER *)calloc(1, sizeof(BITMAPINFOHEADER) + (size_t)movie.width * movie.height * 2);
    movie.decoder = PortIndeo5_Open(movie.width, movie.height);
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

static int DecodeSample(long position) {
    const unsigned char *planes[3];
    int linesize[3];
    LONG bytes = 0;
    int result;

    if (AVIStreamRead(movie.stream, position, 1, NULL, 0, &bytes, NULL) != 0 || bytes < 0) {
        return 0;
    }
    movie.position = position;
    if (bytes == 0) {
        return 1; /* a dropped frame: the picture stays as it was */
    }
    if (bytes > movie.sample_size) {
        /* the decoder may read a little past the end of the data */
        unsigned char *grown = (unsigned char *)realloc(movie.sample, bytes + PORT_INDEO5_PADDING);
        if (grown == NULL) {
            return 0;
        }
        movie.sample = grown;
        movie.sample_size = bytes;
    }
    if (AVIStreamRead(movie.stream, position, 1, movie.sample, bytes, &bytes, NULL) != 0) {
        return 0;
    }
    memset(movie.sample + bytes, 0, PORT_INDEO5_PADDING);
    result = PortIndeo5_Decode(movie.decoder, movie.sample, bytes, planes, linesize);
    if (result < 0) {
        return 0;
    }
    if (result > 0) {
        ConvertPicture(planes, linesize);
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
        start = AVIStreamFindSample(movie.stream, position, FIND_KEY | FIND_PREV);
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
