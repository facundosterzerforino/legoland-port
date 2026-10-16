/* test_charset: how GDI draws the Czech CD's text (cp1250 bytes) with the game's Lego font, by character set: the
 * bytes for "čřžŘŮ" drawn with DrawTextA in a font of DEFAULT_CHARSET (what the game asks for) and of
 * EASTEUROPE_CHARSET, each compared pixel for pixel with the right text drawn with DrawTextW.
 *
 *   tools/test_miniwin/test_charset.exe <folder with Lego.TTF> */
#include <windows.h>
#include <stdio.h>
#include <string.h>

#define W 240
#define H 48

static unsigned int *Render(HDC dc, HBITMAP bmp, unsigned int *bits, BYTE charset, const char *a, const WCHAR *w) {
    static unsigned int copy[W * H];
    LOGFONTA lf;
    HFONT font, old;
    RECT rc = {0, 0, W, H};

    memset(&lf, 0, sizeof(lf));
    lf.lfHeight = 0x18;
    lf.lfWeight = 0x2bc;
    lf.lfCharSet = charset;
    lf.lfQuality = 2;
    strcpy(lf.lfFaceName, "Lego");
    font = CreateFontIndirectA(&lf);
    old = (HFONT)SelectObject(dc, font);
    (void)bmp;
    memset(bits, 0xff, W * H * 4);
    if (a != NULL) {
        DrawTextA(dc, a, -1, &rc, DT_LEFT | DT_TOP | DT_SINGLELINE);
    } else {
        DrawTextW(dc, w, -1, &rc, DT_LEFT | DT_TOP | DT_SINGLELINE);
    }
    GdiFlush();
    memcpy(copy, bits, sizeof(copy));
    SelectObject(dc, old);
    DeleteObject(font);
    return copy;
}

static int Differ(const unsigned int *a, const unsigned int *b) {
    int i, n = 0;

    for (i = 0; i < W * H; i++) {
        n += a[i] != b[i];
    }
    return n;
}

int main(int argc, char **argv) {
    char ttf[MAX_PATH];
    BITMAPINFO bi;
    unsigned int *bits;
    HDC dc;
    HBITMAP bmp;
    static unsigned int want_default[W * H], want_ce[W * H];
    /* "čřžŘŮ" in cp1250 and in UTF-16 */
    const char cz[] = "\xE8\xF8\x9E\xD8\xD9";
    const WCHAR czw[] = {0x010D, 0x0159, 0x017E, 0x0158, 0x016E, 0};
    int d1, d2;

    if (argc < 2) {
        printf("usage: test_charset <folder with Lego.TTF>\n");
        return 2;
    }
    snprintf(ttf, sizeof(ttf), "%s\\Lego.TTF", argv[1]);
    if (AddFontResourceExA(ttf, FR_PRIVATE, NULL) == 0) {
        printf("can't load %s\n", ttf);
        return 1;
    }
    printf("system ANSI code page: %u\n", GetACP());
    memset(&bi, 0, sizeof(bi));
    bi.bmiHeader.biSize = sizeof(bi.bmiHeader);
    bi.bmiHeader.biWidth = W;
    bi.bmiHeader.biHeight = -H;
    bi.bmiHeader.biPlanes = 1;
    bi.bmiHeader.biBitCount = 32;
    dc = CreateCompatibleDC(NULL);
    bmp = CreateDIBSection(dc, &bi, DIB_RGB_COLORS, (void **)&bits, NULL, 0);
    SelectObject(dc, bmp);
    SetBkMode(dc, TRANSPARENT);
    memcpy(want_default, Render(dc, bmp, bits, DEFAULT_CHARSET, NULL, czw), sizeof(want_default));
    memcpy(want_ce, Render(dc, bmp, bits, EASTEUROPE_CHARSET, NULL, czw), sizeof(want_ce));
    d1 = Differ(Render(dc, bmp, bits, DEFAULT_CHARSET, cz, NULL), want_default);
    d2 = Differ(Render(dc, bmp, bits, EASTEUROPE_CHARSET, cz, NULL), want_ce);
    printf("cp1250 bytes, DEFAULT_CHARSET font:    %5d pixels differ from the Czech letters%s\n", d1,
        d1 ? " (wrong letters)" : "");
    printf("cp1250 bytes, EASTEUROPE_CHARSET font: %5d pixels differ from the Czech letters%s\n", d2,
        d2 ? " (wrong letters)" : " (right)");
    printf("same glyphs either way for the Unicode text: %s\n", Differ(want_default, want_ce) ? "no" : "yes");
    DeleteObject(bmp);
    DeleteDC(dc);
    RemoveFontResourceExA(ttf, FR_PRIVATE, NULL);
    return d2 != 0;
}
