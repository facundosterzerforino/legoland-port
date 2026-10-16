/* test_gdi_text: the game's text drawing (its four Lego fonts, DrawTextA's formats, TextOutA's alignment) on a
 * 640x480 RGB565 surface, with Windows' GDI (a DIB section, as miniwin's DirectDraw surfaces) or miniwin's own
 * (miniwin/src/posix/gdi.c, off Windows). Writes each case's measurements and the drawn pixels' bounds to
 * <out>.txt and the pixels to <out>.raw, so that the two can be compared (tools/test_miniwin/compare_gdi_text.py).
 *
 * test_gdi_text <Lego.TTF> <out>   (Lego.TTF is the game's: from the CD, not in the repository) */
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef _WIN32
HDC PosixSurfaceDc(void *bits, int width, int height, int pitch);
#endif

#define W 640
#define H 480

static unsigned short *pixels;
static FILE *report;

struct FontSpec {
    int height;
    int weight;
    BYTE charset;
};

static const struct FontSpec fonts[] = {
    {0x18, 0x2bc, 1}, /* LegoFont24Bold */
    {0x1c, 0x190, 1}, /* LegoFont28Normal */
    {0x14, 0x2bc, 1}, /* LegoFont20Bold */
    {0x12, 0x258, 1}, /* LegoFont18SemiBold */
    {0x12, 0x1f4, 1}, /* the German and Dutch weight */
    {0x14, 0x2bc, 238}, /* EASTEUROPE_CHARSET: the Czech CD */
};

static const char *const texts[] = {
    "Build a theme park!",
    "Welcome to LEGOLAND. Click on the Brick Bank to see how much money you have, and build rides for the visitors.",
    "Score: 12,345  Visitors: 678",
    "Tab\there & there &&",
    "Line one\r\nLine two\nLine three",
    "\xc8"
    "esk\xfd park: \x9e"
    "lu\x9d"
    "ou\xe8k\xfd k\xf9\xf2",
    "WWWWWWWWWWWWWWWWWWWWWWWWWWWWWW iiiiii",
    "",
};

static void Bounds(int y0, int y1, RECT *out) {
    int x;
    int y;

    SetRect(out, W, H, -1, -1);
    for (y = y0; y < y1; y++) {
        for (x = 0; x < W; x++) {
            if (pixels[y * W + x] != 0) {
                if (x < out->left) out->left = x;
                if (x > out->right) out->right = x;
                if (y < out->top) out->top = y;
                if (y > out->bottom) out->bottom = y;
            }
        }
    }
}

int main(int argc, char **argv) {
    HDC dc;
    char path[512];
    FILE *raw;
    int f;
    int t;
    int band = 0;

    if (argc < 3) {
        fprintf(stderr, "usage: test_gdi_text <Lego.TTF> <out>\n");
        return 2;
    }
#ifdef _WIN32
    {
        struct {
            BITMAPINFOHEADER h;
            DWORD masks[3];
        } bi;
        void *bits = NULL;
        HBITMAP dib;

        memset(&bi, 0, sizeof(bi));
        bi.h.biSize = sizeof(bi.h);
        bi.h.biWidth = W;
        bi.h.biHeight = -H;
        bi.h.biPlanes = 1;
        bi.h.biBitCount = 16;
        bi.h.biCompression = BI_BITFIELDS;
        bi.masks[0] = 0xf800;
        bi.masks[1] = 0x07e0;
        bi.masks[2] = 0x001f;
        dib = CreateDIBSection(NULL, (BITMAPINFO *)&bi, DIB_RGB_COLORS, &bits, NULL, 0);
        dc = CreateCompatibleDC(NULL);
        SelectObject(dc, dib);
        pixels = (unsigned short *)bits;
        if (AddFontResourceExA(argv[1], FR_PRIVATE, 0) == 0) {
            fprintf(stderr, "can't load %s\n", argv[1]);
            return 1;
        }
    }
#else
    pixels = (unsigned short *)calloc(W * H, 2);
    dc = PosixSurfaceDc(pixels, W, H, W * 2);
    if (AddFontResourceA(argv[1]) == 0) {
        fprintf(stderr, "can't load %s\n", argv[1]);
        return 1;
    }
#endif
    snprintf(path, sizeof(path), "%s.txt", argv[2]);
    report = fopen(path, "w");
    for (f = 0; f < (int)(sizeof(fonts) / sizeof(fonts[0])); f++) {
        LOGFONTA lf;
        HFONT font;
        HGDIOBJ old;

        memset(&lf, 0, sizeof(lf));
        lf.lfHeight = fonts[f].height;
        lf.lfWeight = fonts[f].weight;
        lf.lfCharSet = fonts[f].charset;
        lf.lfQuality = 2;
        strcpy(lf.lfFaceName, "Lego");
        font = CreateFontIndirectA(&lf);
        old = SelectObject(dc, font);
        SetBkMode(dc, TRANSPARENT);
        SetTextColor(dc, 0xffffff);
        for (t = 0; t < (int)(sizeof(texts) / sizeof(texts[0])); t++) {
            static const UINT formats[] = {0x11, 0x410, 0x25, 0x20, 0x01, 0x50, 0x450, 0x10};
            int k;
            for (k = 0; k < (int)(sizeof(formats) / sizeof(formats[0])); k++) {
                RECT box;
                RECT ink;
                int r;

                memset(pixels, 0, W * H * 2);
                SetRect(&box, 20, 20, 260, 200);
                r = DrawTextA(dc, texts[t], -1, &box, formats[k]);
                Bounds(0, H, &ink);
                fprintf(report, "font %d text %d format %#x: returns %d, rect %ld %ld %ld %ld, ink %ld %ld %ld %ld\n", f,
                    t, formats[k], r, (long)box.left, (long)box.top, (long)box.right, (long)box.bottom,
                    (long)ink.left, (long)ink.top, (long)ink.right, (long)ink.bottom);
                if (band < 64 && (formats[k] & DT_CALCRECT) == 0 && t < 3) {
                    /* keep the picture of the first texts, for the pixel comparison */
                    snprintf(path, sizeof(path), "%s.%02d.raw", argv[2], band++);
                    raw = fopen(path, "wb");
                    fwrite(pixels, 2, W * H, raw);
                    fclose(raw);
                }
            }
        }
        {
            /* every character's width, and its ink's top and left in a cell drawn at (100, 100) */
            int c;
            for (c = 32; c < 256; c++) {
                char one[2];
                RECT box;
                RECT ink;
                one[0] = (char)c;
                one[1] = 0;
                SetRect(&box, 0, 0, 0, 0);
                DrawTextA(dc, one, 1, &box, DT_CALCRECT | DT_NOPREFIX | DT_SINGLELINE);
                memset(pixels, 0, W * H * 2);
                SetRect(&ink, 100, 100, 300, 200);
                DrawTextA(dc, one, 1, &ink, DT_NOPREFIX | DT_SINGLELINE);
                Bounds(0, H, &ink);
                fprintf(report, "font %d char %d: width %ld, ink %ld %ld %ld %ld\n", f, c, (long)box.right,
                    (long)ink.left - 100, (long)ink.top - 100, (long)ink.right - 100, (long)ink.bottom - 100);
            }
        }
        {
            static const UINT aligns[] = {TA_LEFT | TA_TOP, TA_CENTER | TA_TOP, TA_RIGHT | TA_BASELINE};
            int a;
            for (a = 0; a < 3; a++) {
                RECT ink;
                memset(pixels, 0, W * H * 2);
                SetTextAlign(dc, aligns[a]);
                TextOutA(dc, 320, 100, texts[0], (int)strlen(texts[0]));
                Bounds(0, H, &ink);
                fprintf(report, "font %d textout align %#x: ink %ld %ld %ld %ld\n", f, aligns[a], (long)ink.left,
                    (long)ink.top, (long)ink.right, (long)ink.bottom);
            }
            SetTextAlign(dc, TA_LEFT | TA_TOP);
        }
        SelectObject(dc, old);
        DeleteObject(font);
    }
    fclose(report);
    printf("bands: %d\n", band);
    return 0;
}
