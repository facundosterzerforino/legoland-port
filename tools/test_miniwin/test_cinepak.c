/* test_cinepak: decodes every frame of a Cinepak AVI (the intro of the European CDs) with Windows' Cinepak codec
 * (iccvid.dll, through Video for Windows, as the original game did) and with the port's decoder (FFmpeg's
 * cinepak.c in port/ffmpeg), and compares the pictures: RGB24, and the RGB555 the game draws (packed as
 * port/port_movie.c packs them).
 *
 *   tools/test_miniwin/test_cinepak.exe <movie.avi> */
#include <windows.h>
#include <vfw.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ffmpeg/port_ffmpeg.h"

int main(int argc, char **argv) {
    PAVIFILE file;
    PAVISTREAM stream;
    BITMAPINFOHEADER in;
    LONG size = sizeof(in);
    struct {
        BITMAPINFOHEADER h;
    } out24, out16;
    HIC hic;
    PortVideoDecoder *decoder;
    unsigned char *sample = NULL, *win24, *win16;
    long frames, n, sample_cap = 0;
    long frames_bad24 = 0, frames_bad16 = 0, decoded = 0;
    unsigned long long pixels_bad24 = 0, pixels_bad16 = 0;
    int maxdiff = 0;

    if (argc < 2) {
        printf("usage: test_cinepak <movie.avi>\n");
        return 2;
    }
    AVIFileInit();
    if (AVIFileOpenA(&file, argv[1], OF_READ, NULL) != 0 ||
        AVIFileGetStream(file, &stream, streamtypeVIDEO, 0) != 0) {
        printf("can't open %s\n", argv[1]);
        return 1;
    }
    AVIStreamReadFormat(stream, 0, &in, &size);
    printf("%s: %c%c%c%c %ldx%ld, %d bits, %ld frames\n", argv[1], (char)in.biCompression,
        (char)(in.biCompression >> 8), (char)(in.biCompression >> 16), (char)(in.biCompression >> 24), in.biWidth,
        in.biHeight, in.biBitCount, AVIStreamLength(stream));
    hic = ICDecompressOpen(ICTYPE_VIDEO, in.biCompression, &in, NULL);
    decoder = PortVideoDecoder_Open(in.biCompression, in.biWidth, in.biHeight, in.biBitCount);
    if (hic == NULL || decoder == NULL || !PortVideoDecoder_IsRgb(decoder)) {
        printf("no decoder (Windows %p, port %p)\n", (void *)hic, (void *)decoder);
        return 1;
    }
    out24.h = in;
    out24.h.biSize = sizeof(BITMAPINFOHEADER);
    out24.h.biCompression = BI_RGB;
    out24.h.biBitCount = 24;
    out24.h.biSizeImage = ((in.biWidth * 3 + 3) & ~3) * in.biHeight;
    out16.h = out24.h;
    out16.h.biBitCount = 16;
    out16.h.biSizeImage = ((in.biWidth * 2 + 3) & ~3) * in.biHeight;
    win24 = (unsigned char *)calloc(1, out24.h.biSizeImage);
    win16 = (unsigned char *)calloc(1, out16.h.biSizeImage);
    frames = AVIStreamLength(stream);
    for (n = 0; n < frames; n++) {
        LONG bytes = 0;
        const unsigned char *planes[3];
        int linesize[3];
        int r, x, y, bad24 = 0, bad16 = 0;

        AVIStreamRead(stream, n, 1, NULL, 0, &bytes, NULL);
        if (bytes + PORT_FFMPEG_PADDING > sample_cap) {
            sample_cap = bytes + PORT_FFMPEG_PADDING;
            sample = (unsigned char *)realloc(sample, sample_cap);
        }
        AVIStreamRead(stream, n, 1, sample, bytes, &bytes, NULL);
        memset(sample + bytes, 0, PORT_FFMPEG_PADDING);
        if (bytes == 0) {
            continue; /* a dropped frame: both keep the previous picture */
        }
        in.biSizeImage = bytes;
        /* Windows: the same compressed frame to 24 bits, and (a fresh decompressor state each time would differ:
         * Cinepak frames build on the previous one) to 16 bits through a second decompressor */
        if (ICDecompress(hic, 0, &in, sample, &out24.h, win24) != ICERR_OK) {
            printf("frame %ld: Windows can't decode it\n", n);
            return 1;
        }
        r = PortVideoDecoder_Decode(decoder, sample, bytes, planes, linesize);
        if (r < 0) {
            printf("frame %ld: the port can't decode it (%d)\n", n, r);
            return 1;
        }
        if (r == 0) {
            continue;
        }
        decoded++;
        for (y = 0; y < in.biHeight; y++) {
            /* Windows' DIB is bottom-up BGR; the port's picture top-down RGB */
            const unsigned char *w = win24 + (size_t)(in.biHeight - 1 - y) * ((in.biWidth * 3 + 3) & ~3);
            const unsigned char *p = planes[0] + (size_t)y * linesize[0];

            for (x = 0; x < in.biWidth; x++) {
                int db = abs(w[x * 3] - p[x * 3 + 2]), dg = abs(w[x * 3 + 1] - p[x * 3 + 1]),
                    dr = abs(w[x * 3 + 2] - p[x * 3]);
                int d = db > dg ? (db > dr ? db : dr) : (dg > dr ? dg : dr);
                /* the RGB555 the game draws (port_movie.c: the top 5 bits of each) */
                int w555 = ((w[x * 3 + 2] >> 3) << 10) | ((w[x * 3 + 1] >> 3) << 5) | (w[x * 3] >> 3);
                int p555 = ((p[x * 3] >> 3) << 10) | ((p[x * 3 + 1] >> 3) << 5) | (p[x * 3 + 2] >> 3);

                if (d != 0) {
                    bad24++;
                }
                if (d > maxdiff) {
                    maxdiff = d;
                }
                if (w555 != p555) {
                    bad16++;
                }
            }
        }
        if (bad24 && frames_bad24 < 12) {
            int x0 = 9999, y0 = 9999, x1 = -1, y1 = -1;
            for (y = 0; y < in.biHeight; y++) {
                const unsigned char *w = win24 + (size_t)(in.biHeight - 1 - y) * ((in.biWidth * 3 + 3) & ~3);
                const unsigned char *p = planes[0] + (size_t)y * linesize[0];
                for (x = 0; x < in.biWidth; x++) {
                    if (w[x * 3] != p[x * 3 + 2] || w[x * 3 + 1] != p[x * 3 + 1] || w[x * 3 + 2] != p[x * 3]) {
                        if (x < x0) x0 = x;
                        if (y < y0) y0 = y;
                        if (x > x1) x1 = x;
                        if (y > y1) y1 = y;
                    }
                }
            }
            printf("  frame %ld (%ld bytes, key %d): %d pixels in x %d-%d, y %d-%d\n", n, (long)bytes,
                AVIStreamIsKeyFrame(stream, n), bad24, x0, x1, y0, y1);
        }
        if (bad24) {
            frames_bad24++;
            pixels_bad24 += bad24;
        }
        if (bad16) {
            frames_bad16++;
            pixels_bad16 += bad16;
        }
    }
    printf("%ld pictures compared: RGB24 differs in %ld (%llu pixels, at most %d levels), RGB555 in %ld (%llu "
           "pixels)\n",
        decoded, frames_bad24, pixels_bad24, maxdiff, frames_bad16, pixels_bad16);
    printf(frames_bad16 == 0 ? "IDENTICAL as the game draws them\n" : "DIFFERENT\n");
    (void)win16;
    (void)out16;
    ICClose(hic);
    PortVideoDecoder_Close(decoder);
    AVIStreamRelease(stream);
    AVIFileRelease(file);
    AVIFileExit();
    return frames_bad16 != 0;
}
