/* test_gdi_print: miniwin's picture printer (miniwin/src/posix/gdi.c) the way the game's PrintCertificate uses it:
 * the default printer, its DC, an 8-bit DIB stretched onto the page, a line of text, EndDoc. Checks that a BMP of the
 * page was saved with the picture's colours and the text on it, then deletes it. Linux only; Lego.TTF optional (no
 * font: no text, and that check is skipped).
 *
 *   test_gdi_print [Lego.TTF] */
#include <windows.h>
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char **argv) {
    char printer[256];
    DWORD size = sizeof(printer);
    HDC dc;
    DOCINFOA di;
    struct {
        BITMAPINFOHEADER h;
        RGBQUAD colors[256];
    } bi;
    unsigned char pixels[64 * 32];
    int i;
    int problems = 0;
    int has_font = argc > 1 && AddFontResourceA(argv[1]) > 0;
    DIR *d;
    struct dirent *e;
    char found[512] = "";

    if (!GetDefaultPrinterA(printer, &size) || (dc = CreateDCA(NULL, printer, NULL, NULL)) == NULL) {
        printf("FAIL: no printer DC\n");
        return 1;
    }
    memset(&bi, 0, sizeof(bi));
    bi.h.biSize = sizeof(bi.h);
    bi.h.biWidth = 64;
    bi.h.biHeight = 32; /* bottom-up */
    bi.h.biPlanes = 1;
    bi.h.biBitCount = 8;
    bi.colors[1].rgbRed = 255; /* the bottom half red, the top half blue */
    bi.colors[2].rgbBlue = 255;
    for (i = 0; i < 64 * 32; i++) {
        pixels[i] = i < 64 * 16 ? 1 : 2;
    }
    di.cbSize = sizeof(di);
    di.lpszDocName = "Lego certificate";
    di.lpszOutput = NULL;
    di.lpszDatatype = NULL;
    di.fwType = 0;
    if (StartDocA(dc, &di) <= 0 || StartPage(dc) <= 0) {
        printf("FAIL: StartDoc/StartPage\n");
        problems++;
    }
    {
        int w = GetDeviceCaps(dc, HORZRES);
        int h = GetDeviceCaps(dc, VERTRES);
        LOGFONTA lf;
        HFONT font;

        if (!StretchDIBits(dc, w / 8, h / 64, w - (w / 8) * 2, h - (h / 64) * 2, 0, 0, 64, 32, pixels,
                (BITMAPINFO *)&bi, DIB_RGB_COLORS, SRCCOPY)) {
            printf("FAIL: StretchDIBits\n");
            problems++;
        }
        memset(&lf, 0, sizeof(lf));
        lf.lfHeight = -MulDiv(20, GetDeviceCaps(dc, LOGPIXELSY), 72);
        strcpy(lf.lfFaceName, "Lego");
        font = CreateFontIndirectA(&lf);
        SelectObject(dc, font);
        SetBkMode(dc, TRANSPARENT);
        SetTextColor(dc, 0x00ff00);
        SetTextAlign(dc, TA_CENTER | TA_TOP);
        TextOutA(dc, w / 2, h / 2, "Ada Lovelace", 12);
    }
    EndPage(dc);
    if (EndDoc(dc) <= 0) {
        printf("FAIL: EndDoc\n");
        problems++;
    }
    DeleteDC(dc);
    d = opendir(".");
    while (d != NULL && (e = readdir(d)) != NULL) {
        if (strncmp(e->d_name, "Lego certificate ", 17) == 0 && strstr(e->d_name, ".bmp") != NULL) {
            snprintf(found, sizeof(found), "%s", e->d_name);
        }
    }
    if (d != NULL) {
        closedir(d);
    }
    if (found[0] == '\0') {
        printf("FAIL: no picture saved\n");
        return 1;
    }
    {
        FILE *f = fopen(found, "rb");
        static unsigned char data[1754 * 3 * 1240 + 54 + 4096];
        size_t n = fread(data, 1, sizeof(data), f);
        int width = data[18] | data[19] << 8;
        int height = data[22] | data[23] << 8;
        size_t stride = ((size_t)width * 3 + 3) & ~(size_t)3;
        int red = 0;
        int blue = 0;
        int green = 0;
        int y;
        int x;

        fclose(f);
        printf("saved \"%s\": %dx%d, %zu bytes\n", found, width, height, n);
        for (y = 0; y < height; y++) {
            for (x = 0; x < width; x++) {
                const unsigned char *p = data + 54 + (size_t)y * stride + (size_t)x * 3;
                red += p[2] > 200 && p[1] < 50 && p[0] < 50;
                blue += p[0] > 200 && p[1] < 50 && p[2] < 50;
                green += p[1] > 200 && p[0] < 50 && p[2] < 50;
            }
        }
        if (red < width * height / 4 || blue < width * height / 4) {
            printf("FAIL: the picture isn't on the page (red %d, blue %d pixels)\n", red, blue);
            problems++;
        }
        /* bottom-up rows: row 0 of the file is the page's bottom, which the picture's bottom (red) covers */
        if (has_font && green < 200) {
            printf("FAIL: the text isn't on the page (%d green pixels)\n", green);
            problems++;
        }
        remove(found);
    }
    printf(problems == 0 ? "OK\n" : "%d problem(s)\n", problems);
    return problems != 0;
}
