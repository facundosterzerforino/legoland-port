/* [library:video] GDI off Windows: device contexts over miniwin's RGB565 surfaces (DirectDraw's GetDC), the
 * objects the game selects into them (fonts, brushes, pens, rectangular clip regions), and its drawing: text
 * (TextOutA, DrawTextA) in the fonts AddFontResourceA loads (Lego.TTF), rasterized by stb_truetype with GDI's metrics
 * (the cell height from the OS/2 table's Windows ascent and descent, whole-pixel advances, no kerning, 1 pixel of
 * emboldening for bold weights of a regular face), FillRect and solid lines. A DC without pixels (the screen's,
 * a memory DC) measures but draws nothing. The one printer saves its pages as pictures (the game prints its
 * certificate) as 24-bit BMP files in the current folder. See posix.h. */
#include <ctype.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <SDL3/SDL.h>

#define STB_TRUETYPE_IMPLEMENTATION
#define STBTT_STATIC
#include "stb/stb_truetype.h"

#include "posix.h"

enum GdiKind {
    GDI_FONT = 0x47440001,
    GDI_BRUSH,
    GDI_PEN,
    GDI_REGION,
    GDI_DC,
};

struct GdiObject {
    enum GdiKind kind;
    int stock;
    struct GdiObject *next; /* the live objects: a handle is checked against them before it's used */
};

struct FontFile {
    char path[MAX_PATH];
    char face[LF_FACESIZE];
    unsigned char *data;
    stbtt_fontinfo info;
    int units_per_em;
    int win_ascent;
    int win_descent;
    int avg_width;
    int weight;
    int registered; /* AddFontResource'd (RemoveFontResource unregisters; fonts made from it keep it) */
    struct FontFile *next;
};

struct Font {
    struct GdiObject base;
    LOGFONTA lf;
    struct FontFile *file;
    float scale;
    int ascent;
    int descent;
    int avg_width;
    int embolden;
};

struct Brush {
    struct GdiObject base;
    COLORREF color;
    int hollow;
};

struct Pen {
    struct GdiObject base;
    COLORREF color;
    int width;
    int null;
};

struct Region {
    struct GdiObject base;
    RECT rect;
};

struct Dc {
    struct GdiObject base;
    unsigned short *bits; /* NULL: measures only */
    int printer; /* the picture printer's: bits are its own, and EndDoc saves them */
    int dpi;
    char document[64];
    int width;
    int height;
    int pitch; /* bytes */
    struct Font *font;
    struct Brush *brush;
    struct Pen *pen;
    COLORREF text_color;
    COLORREF bk_color;
    int bk_mode;
    UINT align;
    RECT clip; /* the selected region, within the pixels */
    POINT position;
};

static struct GdiObject *live;
static struct FontFile *font_files;
static pthread_mutex_t gdi_lock = PTHREAD_MUTEX_INITIALIZER;

static void *NewObject(enum GdiKind kind, size_t size) {
    struct GdiObject *o = (struct GdiObject *)calloc(1, size);

    if (o == NULL) {
        PosixSetError(ERROR_NOT_ENOUGH_MEMORY);
        return NULL;
    }
    o->kind = kind;
    pthread_mutex_lock(&gdi_lock);
    o->next = live;
    live = o;
    pthread_mutex_unlock(&gdi_lock);
    return o;
}

/* the object behind a handle, if it is a live one (of that kind, unless kind is 0) */
static struct GdiObject *Lookup(const void *handle, enum GdiKind kind) {
    struct GdiObject *o;

    if (handle == NULL) {
        return NULL;
    }
    pthread_mutex_lock(&gdi_lock);
    for (o = live; o != NULL && (const void *)o != handle; o = o->next) {
    }
    pthread_mutex_unlock(&gdi_lock);
    if (o == NULL || (kind != 0 && o->kind != kind)) {
        return NULL;
    }
    return o;
}

static void FreeObject(struct GdiObject *o) {
    struct GdiObject **p;

    pthread_mutex_lock(&gdi_lock);
    for (p = &live; *p != NULL; p = &(*p)->next) {
        if (*p == o) {
            *p = o->next;
            break;
        }
    }
    pthread_mutex_unlock(&gdi_lock);
    o->kind = 0;
    free(o);
}

/* ---- colours: COLORREF (0x00bbggrr) and RGB565 ---- */

static unsigned short To565(COLORREF c) {
    return (unsigned short)(((GetRValue(c) >> 3) << 11) | ((GetGValue(c) >> 2) << 5) | (GetBValue(c) >> 3));
}

static void Blend(unsigned short *pixel, COLORREF color, int coverage) {
    unsigned int p = *pixel;
    int r = (int)((p >> 11) & 0x1f) * 255 / 31;
    int g = (int)((p >> 5) & 0x3f) * 255 / 63;
    int b = (int)(p & 0x1f) * 255 / 31;

    if (coverage >= 255) {
        *pixel = To565(color);
        return;
    }
    r += (GetRValue(color) - r) * coverage / 255;
    g += (GetGValue(color) - g) * coverage / 255;
    b += (GetBValue(color) - b) * coverage / 255;
    *pixel = To565(RGB(r, g, b));
}

COLORREF GetNearestColor(HDC dc, COLORREF color) {
    unsigned int r = GetRValue(color) >> 3;
    unsigned int g = GetGValue(color) >> 2;
    unsigned int b = GetBValue(color) >> 3;

    (void)dc;
    return RGB((r << 3) | (r >> 2), (g << 2) | (g >> 4), (b << 3) | (b >> 2));
}

/* ---- the code pages of the fonts' character sets: their bytes 0x80-0xff as Unicode ---- */

static const unsigned short cp1250[128] = {
    0x20ac,
    0x0000,
    0x201a,
    0x0000,
    0x201e,
    0x2026,
    0x2020,
    0x2021,
    0x0000,
    0x2030,
    0x0160,
    0x2039,
    0x015a,
    0x0164,
    0x017d,
    0x0179,
    0x0000,
    0x2018,
    0x2019,
    0x201c,
    0x201d,
    0x2022,
    0x2013,
    0x2014,
    0x0000,
    0x2122,
    0x0161,
    0x203a,
    0x015b,
    0x0165,
    0x017e,
    0x017a,
    0x00a0,
    0x02c7,
    0x02d8,
    0x0141,
    0x00a4,
    0x0104,
    0x00a6,
    0x00a7,
    0x00a8,
    0x00a9,
    0x015e,
    0x00ab,
    0x00ac,
    0x00ad,
    0x00ae,
    0x017b,
    0x00b0,
    0x00b1,
    0x02db,
    0x0142,
    0x00b4,
    0x00b5,
    0x00b6,
    0x00b7,
    0x00b8,
    0x0105,
    0x015f,
    0x00bb,
    0x013d,
    0x02dd,
    0x013e,
    0x017c,
    0x0154,
    0x00c1,
    0x00c2,
    0x0102,
    0x00c4,
    0x0139,
    0x0106,
    0x00c7,
    0x010c,
    0x00c9,
    0x0118,
    0x00cb,
    0x011a,
    0x00cd,
    0x00ce,
    0x010e,
    0x0110,
    0x0143,
    0x0147,
    0x00d3,
    0x00d4,
    0x0150,
    0x00d6,
    0x00d7,
    0x0158,
    0x016e,
    0x00da,
    0x0170,
    0x00dc,
    0x00dd,
    0x0162,
    0x00df,
    0x0155,
    0x00e1,
    0x00e2,
    0x0103,
    0x00e4,
    0x013a,
    0x0107,
    0x00e7,
    0x010d,
    0x00e9,
    0x0119,
    0x00eb,
    0x011b,
    0x00ed,
    0x00ee,
    0x010f,
    0x0111,
    0x0144,
    0x0148,
    0x00f3,
    0x00f4,
    0x0151,
    0x00f6,
    0x00f7,
    0x0159,
    0x016f,
    0x00fa,
    0x0171,
    0x00fc,
    0x00fd,
    0x0163,
    0x02d9,
};

static const unsigned short cp1251[128] = {
    0x0402,
    0x0403,
    0x201a,
    0x0453,
    0x201e,
    0x2026,
    0x2020,
    0x2021,
    0x20ac,
    0x2030,
    0x0409,
    0x2039,
    0x040a,
    0x040c,
    0x040b,
    0x040f,
    0x0452,
    0x2018,
    0x2019,
    0x201c,
    0x201d,
    0x2022,
    0x2013,
    0x2014,
    0x0000,
    0x2122,
    0x0459,
    0x203a,
    0x045a,
    0x045c,
    0x045b,
    0x045f,
    0x00a0,
    0x040e,
    0x045e,
    0x0408,
    0x00a4,
    0x0490,
    0x00a6,
    0x00a7,
    0x0401,
    0x00a9,
    0x0404,
    0x00ab,
    0x00ac,
    0x00ad,
    0x00ae,
    0x0407,
    0x00b0,
    0x00b1,
    0x0406,
    0x0456,
    0x0491,
    0x00b5,
    0x00b6,
    0x00b7,
    0x0451,
    0x2116,
    0x0454,
    0x00bb,
    0x0458,
    0x0405,
    0x0455,
    0x0457,
    0x0410,
    0x0411,
    0x0412,
    0x0413,
    0x0414,
    0x0415,
    0x0416,
    0x0417,
    0x0418,
    0x0419,
    0x041a,
    0x041b,
    0x041c,
    0x041d,
    0x041e,
    0x041f,
    0x0420,
    0x0421,
    0x0422,
    0x0423,
    0x0424,
    0x0425,
    0x0426,
    0x0427,
    0x0428,
    0x0429,
    0x042a,
    0x042b,
    0x042c,
    0x042d,
    0x042e,
    0x042f,
    0x0430,
    0x0431,
    0x0432,
    0x0433,
    0x0434,
    0x0435,
    0x0436,
    0x0437,
    0x0438,
    0x0439,
    0x043a,
    0x043b,
    0x043c,
    0x043d,
    0x043e,
    0x043f,
    0x0440,
    0x0441,
    0x0442,
    0x0443,
    0x0444,
    0x0445,
    0x0446,
    0x0447,
    0x0448,
    0x0449,
    0x044a,
    0x044b,
    0x044c,
    0x044d,
    0x044e,
    0x044f,
};

static const unsigned short cp1252[128] = {
    0x20ac,
    0x0000,
    0x201a,
    0x0192,
    0x201e,
    0x2026,
    0x2020,
    0x2021,
    0x02c6,
    0x2030,
    0x0160,
    0x2039,
    0x0152,
    0x0000,
    0x017d,
    0x0000,
    0x0000,
    0x2018,
    0x2019,
    0x201c,
    0x201d,
    0x2022,
    0x2013,
    0x2014,
    0x02dc,
    0x2122,
    0x0161,
    0x203a,
    0x0153,
    0x0000,
    0x017e,
    0x0178,
    0x00a0,
    0x00a1,
    0x00a2,
    0x00a3,
    0x00a4,
    0x00a5,
    0x00a6,
    0x00a7,
    0x00a8,
    0x00a9,
    0x00aa,
    0x00ab,
    0x00ac,
    0x00ad,
    0x00ae,
    0x00af,
    0x00b0,
    0x00b1,
    0x00b2,
    0x00b3,
    0x00b4,
    0x00b5,
    0x00b6,
    0x00b7,
    0x00b8,
    0x00b9,
    0x00ba,
    0x00bb,
    0x00bc,
    0x00bd,
    0x00be,
    0x00bf,
    0x00c0,
    0x00c1,
    0x00c2,
    0x00c3,
    0x00c4,
    0x00c5,
    0x00c6,
    0x00c7,
    0x00c8,
    0x00c9,
    0x00ca,
    0x00cb,
    0x00cc,
    0x00cd,
    0x00ce,
    0x00cf,
    0x00d0,
    0x00d1,
    0x00d2,
    0x00d3,
    0x00d4,
    0x00d5,
    0x00d6,
    0x00d7,
    0x00d8,
    0x00d9,
    0x00da,
    0x00db,
    0x00dc,
    0x00dd,
    0x00de,
    0x00df,
    0x00e0,
    0x00e1,
    0x00e2,
    0x00e3,
    0x00e4,
    0x00e5,
    0x00e6,
    0x00e7,
    0x00e8,
    0x00e9,
    0x00ea,
    0x00eb,
    0x00ec,
    0x00ed,
    0x00ee,
    0x00ef,
    0x00f0,
    0x00f1,
    0x00f2,
    0x00f3,
    0x00f4,
    0x00f5,
    0x00f6,
    0x00f7,
    0x00f8,
    0x00f9,
    0x00fa,
    0x00fb,
    0x00fc,
    0x00fd,
    0x00fe,
    0x00ff,
};

static const unsigned short cp1253[128] = {
    0x20ac,
    0x0000,
    0x201a,
    0x0192,
    0x201e,
    0x2026,
    0x2020,
    0x2021,
    0x0000,
    0x2030,
    0x0000,
    0x2039,
    0x0000,
    0x0000,
    0x0000,
    0x0000,
    0x0000,
    0x2018,
    0x2019,
    0x201c,
    0x201d,
    0x2022,
    0x2013,
    0x2014,
    0x0000,
    0x2122,
    0x0000,
    0x203a,
    0x0000,
    0x0000,
    0x0000,
    0x0000,
    0x00a0,
    0x0385,
    0x0386,
    0x00a3,
    0x00a4,
    0x00a5,
    0x00a6,
    0x00a7,
    0x00a8,
    0x00a9,
    0x0000,
    0x00ab,
    0x00ac,
    0x00ad,
    0x00ae,
    0x2015,
    0x00b0,
    0x00b1,
    0x00b2,
    0x00b3,
    0x0384,
    0x00b5,
    0x00b6,
    0x00b7,
    0x0388,
    0x0389,
    0x038a,
    0x00bb,
    0x038c,
    0x00bd,
    0x038e,
    0x038f,
    0x0390,
    0x0391,
    0x0392,
    0x0393,
    0x0394,
    0x0395,
    0x0396,
    0x0397,
    0x0398,
    0x0399,
    0x039a,
    0x039b,
    0x039c,
    0x039d,
    0x039e,
    0x039f,
    0x03a0,
    0x03a1,
    0x0000,
    0x03a3,
    0x03a4,
    0x03a5,
    0x03a6,
    0x03a7,
    0x03a8,
    0x03a9,
    0x03aa,
    0x03ab,
    0x03ac,
    0x03ad,
    0x03ae,
    0x03af,
    0x03b0,
    0x03b1,
    0x03b2,
    0x03b3,
    0x03b4,
    0x03b5,
    0x03b6,
    0x03b7,
    0x03b8,
    0x03b9,
    0x03ba,
    0x03bb,
    0x03bc,
    0x03bd,
    0x03be,
    0x03bf,
    0x03c0,
    0x03c1,
    0x03c2,
    0x03c3,
    0x03c4,
    0x03c5,
    0x03c6,
    0x03c7,
    0x03c8,
    0x03c9,
    0x03ca,
    0x03cb,
    0x03cc,
    0x03cd,
    0x03ce,
    0x0000,
};

static unsigned int Unicode(BYTE charset, unsigned char c) {
    if (c < 0x80) {
        return c;
    }
    switch (charset) {
    case EASTEUROPE_CHARSET:
        return cp1250[c - 0x80];
    case RUSSIAN_CHARSET:
        return cp1251[c - 0x80];
    case GREEK_CHARSET:
        return cp1253[c - 0x80];
    default:
        return cp1252[c - 0x80];
    }
}

/* ---- font files ---- */

static unsigned int BE16(const unsigned char *p) {
    return (unsigned int)(p[0] << 8 | p[1]);
}

/* the family name from the 'name' table: Windows' (platform 3, UTF-16BE) or the Mac's (platform 1, ASCII) */
static void FamilyName(const stbtt_fontinfo *info, char *out, size_t size) {
    int length;
    const char *name;
    int i;
    size_t n = 0;

    out[0] = '\0';
    name = stbtt_GetFontNameString(info, &length, STBTT_PLATFORM_ID_MICROSOFT, STBTT_MS_EID_UNICODE_BMP,
        STBTT_MS_LANG_ENGLISH, 1);
    if (name != NULL) {
        for (i = 0; i + 1 < length && n + 1 < size; i += 2) {
            unsigned int c = BE16((const unsigned char *)name + i);
            out[n++] = c < 0x80 ? (char)c : '?';
        }
        out[n] = '\0';
        return;
    }
    name = stbtt_GetFontNameString(info, &length, STBTT_PLATFORM_ID_MAC, STBTT_MAC_EID_ROMAN, STBTT_MAC_LANG_ENGLISH, 1);
    if (name != NULL) {
        snprintf(out, size, "%.*s", length, name);
    }
}

int AddFontResourceA(LPCSTR name) {
    char path[MAX_PATH];
    struct FontFile *f;
    FILE *file;
    long size;
    int os2;

    PosixPath(name, path, sizeof(path));
    pthread_mutex_lock(&gdi_lock);
    for (f = font_files; f != NULL; f = f->next) {
        if (strcmp(f->path, path) == 0) {
            f->registered++;
            pthread_mutex_unlock(&gdi_lock);
            return 1;
        }
    }
    pthread_mutex_unlock(&gdi_lock);
    file = fopen(path, "rb");
    if (file == NULL) {
        PosixSetError(ERROR_FILE_NOT_FOUND);
        return 0;
    }
    f = (struct FontFile *)calloc(1, sizeof(struct FontFile));
    if (f == NULL || fseek(file, 0, SEEK_END) != 0 || (size = ftell(file)) <= 0 || fseek(file, 0, SEEK_SET) != 0 ||
        (f->data = (unsigned char *)malloc((size_t)size)) == NULL || fread(f->data, 1, (size_t)size, file) != (size_t)size ||
        !stbtt_InitFont(&f->info, f->data, stbtt_GetFontOffsetForIndex(f->data, 0))) {
        fclose(file);
        if (f != NULL) {
            free(f->data);
            free(f);
        }
        PosixSetError(ERROR_INVALID_DATA);
        return 0;
    }
    fclose(file);
    snprintf(f->path, sizeof(f->path), "%s", path);
    FamilyName(&f->info, f->face, sizeof(f->face));
    f->units_per_em = (int)BE16(f->data + f->info.head + 18);
    os2 = (int)stbtt__find_table(f->data, (stbtt_uint32)f->info.fontstart, "OS/2");
    if (os2 != 0) {
        f->avg_width = (short)BE16(f->data + os2 + 2);
        f->weight = (int)BE16(f->data + os2 + 4);
        f->win_ascent = (int)BE16(f->data + os2 + 74);
        f->win_descent = (int)BE16(f->data + os2 + 76);
    } else {
        int ascent, descent, gap;
        stbtt_GetFontVMetrics(&f->info, &ascent, &descent, &gap);
        f->win_ascent = ascent;
        f->win_descent = -descent;
        f->avg_width = f->units_per_em / 2;
        f->weight = FW_NORMAL;
    }
    f->registered = 1;
    pthread_mutex_lock(&gdi_lock);
    f->next = font_files;
    font_files = f;
    pthread_mutex_unlock(&gdi_lock);
    return 1;
}

BOOL RemoveFontResourceA(LPCSTR name) {
    char path[MAX_PATH];
    struct FontFile *f;

    PosixPath(name, path, sizeof(path));
    pthread_mutex_lock(&gdi_lock);
    for (f = font_files; f != NULL; f = f->next) {
        if (strcmp(f->path, path) == 0 && f->registered > 0) {
            f->registered--; /* its data stays: fonts made from it may still draw */
            pthread_mutex_unlock(&gdi_lock);
            return TRUE;
        }
    }
    pthread_mutex_unlock(&gdi_lock);
    PosixSetError(ERROR_FILE_NOT_FOUND);
    return FALSE;
}

static struct FontFile *FindFace(const char *face) {
    struct FontFile *f;
    struct FontFile *any = NULL;

    pthread_mutex_lock(&gdi_lock);
    for (f = font_files; f != NULL; f = f->next) {
        if (f->registered > 0 && strcasecmp(f->face, face) == 0) {
            break;
        }
        if (f->registered > 0 && any == NULL) {
            any = f;
        }
    }
    pthread_mutex_unlock(&gdi_lock);
    /* the font mapper's substitute when the face isn't installed: the one font there is */
    return f != NULL ? f : any;
}

/* ---- objects ---- */

HFONT CreateFontIndirectA(const LOGFONTA *lf) {
    struct Font *font = (struct Font *)NewObject(GDI_FONT, sizeof(struct Font));
    struct FontFile *f;
    int height;

    if (font == NULL) {
        return NULL;
    }
    font->lf = *lf;
    f = FindFace(lf->lfFaceName);
    font->file = f;
    height = lf->lfHeight != 0 ? lf->lfHeight : 16;
    if (f != NULL) {
        /* a positive height is the cell's (ascent and descent), a negative one the em's */
        font->scale = height > 0 ? (float)height / (float)(f->win_ascent + f->win_descent)
                                 : (float)-height / (float)f->units_per_em;
        font->ascent = (int)(f->win_ascent * font->scale + 0.5f);
        font->descent = (int)(f->win_descent * font->scale + 0.5f);
        font->embolden = lf->lfWeight >= FW_SEMIBOLD && f->weight < FW_SEMIBOLD;
        font->avg_width = (int)(f->avg_width * font->scale + 0.5f) + font->embolden;
    } else {
        font->ascent = height > 0 ? height * 4 / 5 : -height;
        font->descent = height > 0 ? height - font->ascent : -height / 4;
        font->avg_width = font->ascent / 2;
    }
    return (HFONT)font;
}

HBRUSH CreateSolidBrush(COLORREF color) {
    struct Brush *b = (struct Brush *)NewObject(GDI_BRUSH, sizeof(struct Brush));

    if (b != NULL) {
        b->color = color & 0xffffff;
    }
    return (HBRUSH)b;
}

HPEN CreatePen(int style, int width, COLORREF color) {
    struct Pen *p = (struct Pen *)NewObject(GDI_PEN, sizeof(struct Pen));

    if (p != NULL) {
        p->color = color & 0xffffff;
        p->width = width > 0 ? width : 1;
        p->null = style == PS_NULL;
    }
    return (HPEN)p;
}

HRGN CreateRectRgn(int left, int top, int right, int bottom) {
    struct Region *r = (struct Region *)NewObject(GDI_REGION, sizeof(struct Region));

    if (r != NULL) {
        /* normalized, as GDI does */
        r->rect.left = left < right ? left : right;
        r->rect.right = left < right ? right : left;
        r->rect.top = top < bottom ? top : bottom;
        r->rect.bottom = top < bottom ? bottom : top;
    }
    return (HRGN)r;
}

HRGN CreateRectRgnIndirect(const RECT *rect) {
    return CreateRectRgn(rect->left, rect->top, rect->right, rect->bottom);
}

HGDIOBJ GetStockObject(int index) {
    static struct Brush *brushes[6];
    static struct Pen *pens[3];
    static struct Font *system_font;
    static const COLORREF brush_colors[6] = {0xffffff, 0xc0c0c0, 0x808080, 0x404040, 0, 0};
    struct GdiObject *o = NULL;

    pthread_mutex_lock(&posix_objects_lock); /* once each */
    if (index >= WHITE_BRUSH && index <= NULL_BRUSH) {
        if (brushes[index] == NULL) {
            brushes[index] = (struct Brush *)CreateSolidBrush(brush_colors[index]);
            brushes[index]->hollow = index == NULL_BRUSH;
        }
        o = &brushes[index]->base;
    } else if (index >= WHITE_PEN && index <= NULL_PEN) {
        if (pens[index - WHITE_PEN] == NULL) {
            pens[index - WHITE_PEN] = (struct Pen *)CreatePen(index == NULL_PEN ? PS_NULL : PS_SOLID, 1,
                index == WHITE_PEN ? 0xffffff : 0);
        }
        o = &pens[index - WHITE_PEN]->base;
    } else if (index == SYSTEM_FONT || index == DEFAULT_GUI_FONT || index == ANSI_VAR_FONT || index == DEVICE_DEFAULT_FONT) {
        if (system_font == NULL) {
            LOGFONTA lf;
            memset(&lf, 0, sizeof(lf));
            lf.lfHeight = 16;
            lf.lfWeight = FW_BOLD;
            system_font = (struct Font *)CreateFontIndirectA(&lf);
        }
        o = &system_font->base;
    }
    if (o != NULL) {
        o->stock = 1;
    }
    pthread_mutex_unlock(&posix_objects_lock);
    return (HGDIOBJ)o;
}

BOOL DeleteObject(HGDIOBJ handle) {
    struct GdiObject *o = Lookup(handle, 0);

    if (o == NULL || o->kind == GDI_DC) {
        return FALSE;
    }
    if (!o->stock) {
        FreeObject(o);
    }
    return TRUE;
}

/* ---- device contexts ---- */

static HDC NewDc(void *bits, int width, int height, int pitch) {
    struct Dc *dc = (struct Dc *)NewObject(GDI_DC, sizeof(struct Dc));

    if (dc == NULL) {
        return NULL;
    }
    dc->bits = (unsigned short *)bits;
    dc->width = width;
    dc->height = height;
    dc->pitch = pitch;
    dc->font = (struct Font *)GetStockObject(SYSTEM_FONT);
    dc->brush = (struct Brush *)GetStockObject(WHITE_BRUSH);
    dc->pen = (struct Pen *)GetStockObject(BLACK_PEN);
    dc->text_color = 0;
    dc->bk_color = 0xffffff;
    dc->bk_mode = OPAQUE;
    dc->align = TA_LEFT | TA_TOP;
    SetRect(&dc->clip, 0, 0, width, height);
    return (HDC)dc;
}

HDC PosixSurfaceDc(void *bits, int width, int height, int pitch) {
    return NewDc(bits, width, height, pitch);
}

/* -port-selftest: the game's text as the game draws it. Lego.ttf added (as InitHostSystemGPU does), the game's four
 * fonts made (InitScreen: face "Lego", 24 bold, 28, 20 bold, 18 semibold), and a line drawn with each, by TextOutA and
 * DrawTextA, on a 16-bit surface's DC (a DirectDraw surface's, miniwin/src/ddraw.c): the pixels it set are counted.
 * None set: the text would be missing in the game. */
int MiniwinTextSelfTest(FILE *report) {
    static const struct {
        int height;
        int weight;
    } fonts[] = {{24, 700}, {28, 400}, {20, 700}, {18, 600}};
    static unsigned short pixels[64 * 320];
    const char *line = "LEGOLAND Park 1999";
    int problems = 0;
    size_t i;

    if (AddFontResourceA("Lego.ttf") == 0) {
        fprintf(report, "text: Lego.ttf doesn't load (error %lu)\n", (unsigned long)GetLastError());
        return 1;
    }
    for (i = 0; i < sizeof(fonts) / sizeof(fonts[0]); i++) {
        LOGFONTA lf;
        HFONT font;
        HDC dc;
        RECT rect = {0, 32, 320, 64};
        int counts[2];
        int k;
        int j;

        memset(&lf, 0, sizeof(lf));
        lf.lfHeight = fonts[i].height;
        lf.lfWeight = fonts[i].weight;
        lf.lfCharSet = 1;
        lf.lfQuality = 2;
        strcpy(lf.lfFaceName, "Lego");
        font = CreateFontIndirectA(&lf);
        for (k = 0; k < 2; k++) {
            memset(pixels, 0, sizeof(pixels));
            dc = PosixSurfaceDc(pixels, 320, 64, 320 * 2);
            SelectObject(dc, font);
            SetBkMode(dc, TRANSPARENT);
            SetTextColor(dc, RGB(255, 255, 255));
            if (k == 0) {
                TextOutA(dc, 0, 0, line, (int)strlen(line));
            } else {
                DrawTextA(dc, line, -1, &rect, DT_LEFT | DT_TOP | DT_SINGLELINE);
            }
            DeleteDC(dc);
            counts[k] = 0;
            for (j = 0; j < 64 * 320; j++) {
                counts[k] += pixels[j] != 0;
            }
        }
        fprintf(report, "text: \"%s\" %d weight %d (face %s): %d pixels by TextOutA, %d by DrawTextA\n", lf.lfFaceName,
            fonts[i].height, fonts[i].weight, ((struct Font *)font)->file != NULL ? ((struct Font *)font)->file->face : "none",
            counts[0], counts[1]);
        if (counts[0] == 0 || counts[1] == 0) {
            problems++;
        }
        DeleteObject(font);
    }
    RemoveFontResourceA("Lego.ttf");
    return problems;
}

void PosixDeleteSurfaceDc(HDC dc) {
    DeleteDC(dc);
}

static struct Dc *AsDc(HDC handle) {
    return (struct Dc *)Lookup(handle, GDI_DC);
}

HDC GetDC(HWND window) {
    int w = 0;
    int h = 0;

    if (window == NULL) {
        SDL_DisplayID display;
        const SDL_DisplayMode *mode;

        SDL_InitSubSystem(SDL_INIT_VIDEO);
        display = SDL_GetPrimaryDisplay();
        mode = display != 0 ? SDL_GetDesktopDisplayMode(display) : NULL;
        if (mode != NULL) {
            w = mode->w;
            h = mode->h;
        }
    }
    return NewDc(NULL, w, h, 0);
}

int ReleaseDC(HWND window, HDC dc) {
    (void)window;
    return DeleteDC(dc);
}

HDC CreateCompatibleDC(HDC dc) {
    (void)dc;
    return NewDc(NULL, 1, 1, 0);
}

BOOL DeleteDC(HDC handle) {
    struct Dc *dc = AsDc(handle);

    if (dc == NULL) {
        return FALSE;
    }
    if (dc->printer) {
        free(dc->bits);
    }
    FreeObject(&dc->base);
    return TRUE;
}

/* the picture printer: an A4 page in landscape at 150 dpi */
#define PRINTER_NAME "LEGOLAND certificate (saved as a picture)"
#define PRINTER_DPI 150
#define PAGE_WIDTH 1754
#define PAGE_HEIGHT 1240

HDC CreateDCA(LPCSTR driver, LPCSTR device, LPCSTR port, const DEVMODEA *mode) {
    struct Dc *dc;
    unsigned short *page;
    int i;

    (void)driver;
    (void)port;
    (void)mode;
    if (device == NULL || strcmp(device, PRINTER_NAME) != 0) {
        PosixSetError(ERROR_INVALID_PRINTER_NAME);
        return NULL;
    }
    page = (unsigned short *)malloc((size_t)PAGE_WIDTH * PAGE_HEIGHT * 2);
    if (page == NULL) {
        PosixSetError(ERROR_NOT_ENOUGH_MEMORY);
        return NULL;
    }
    for (i = 0; i < PAGE_WIDTH * PAGE_HEIGHT; i++) {
        page[i] = 0xffff; /* paper */
    }
    dc = (struct Dc *)NewDc(page, PAGE_WIDTH, PAGE_HEIGHT, PAGE_WIDTH * 2);
    if (dc == NULL) {
        free(page);
        return NULL;
    }
    dc->printer = 1;
    dc->dpi = PRINTER_DPI;
    return (HDC)dc;
}

int GetDeviceCaps(HDC handle, int index) {
    struct Dc *dc = AsDc(handle);

    if (dc == NULL) {
        return 0;
    }
    switch (index) {
    case LOGPIXELSX:
    case LOGPIXELSY:
        return dc->dpi != 0 ? dc->dpi : 96;
    case HORZRES:
        return dc->width;
    case VERTRES:
        return dc->height;
    case BITSPIXEL:
        return dc->bits != NULL ? 16 : 32;
    case PLANES:
        return 1;
    case NUMCOLORS:
        return -1;
    case RASTERCAPS:
        return RC_BITBLT | RC_STRETCHDIB | RC_DI_BITMAP;
    default:
        return 0;
    }
}

HGDIOBJ SelectObject(HDC handle, HGDIOBJ object) {
    struct Dc *dc = AsDc(handle);
    struct GdiObject *o = Lookup(object, 0);
    HGDIOBJ old = NULL;

    if (dc == NULL || o == NULL) {
        return NULL;
    }
    switch (o->kind) {
    case GDI_FONT:
        old = (HGDIOBJ)dc->font;
        dc->font = (struct Font *)o;
        break;
    case GDI_BRUSH:
        old = (HGDIOBJ)dc->brush;
        dc->brush = (struct Brush *)o;
        break;
    case GDI_PEN:
        old = (HGDIOBJ)dc->pen;
        dc->pen = (struct Pen *)o;
        break;
    case GDI_REGION: {
        /* the region becomes the clip region (a copy); GDI returns its complexity, not an object */
        RECT all;
        SetRect(&all, 0, 0, dc->width, dc->height);
        if (!IntersectRect(&dc->clip, &((struct Region *)o)->rect, &all)) {
            SetRect(&dc->clip, 0, 0, 0, 0);
            return (HGDIOBJ)(INT_PTR)NULLREGION;
        }
        return (HGDIOBJ)(INT_PTR)SIMPLEREGION;
    }
    default:
        return NULL;
    }
    return old;
}

COLORREF SetTextColor(HDC handle, COLORREF color) {
    struct Dc *dc = AsDc(handle);
    COLORREF old;

    if (dc == NULL) {
        return CLR_INVALID;
    }
    old = dc->text_color;
    dc->text_color = color & 0xffffff;
    return old;
}

COLORREF SetBkColor(HDC handle, COLORREF color) {
    struct Dc *dc = AsDc(handle);
    COLORREF old;

    if (dc == NULL) {
        return CLR_INVALID;
    }
    old = dc->bk_color;
    dc->bk_color = color & 0xffffff;
    return old;
}

int SetBkMode(HDC handle, int mode) {
    struct Dc *dc = AsDc(handle);
    int old;

    if (dc == NULL || (mode != TRANSPARENT && mode != OPAQUE)) {
        return 0;
    }
    old = dc->bk_mode;
    dc->bk_mode = mode;
    return old;
}

UINT SetTextAlign(HDC handle, UINT align) {
    struct Dc *dc = AsDc(handle);
    UINT old;

    if (dc == NULL) {
        return GDI_ERROR;
    }
    old = dc->align;
    dc->align = align;
    return old;
}

/* ---- filling and lines ---- */

static void Fill(struct Dc *dc, const RECT *r, COLORREF color) {
    RECT c;
    int x;
    int y;
    unsigned short v = To565(color);

    if (dc->bits == NULL || !IntersectRect(&c, r, &dc->clip)) {
        return;
    }
    for (y = c.top; y < c.bottom; y++) {
        unsigned short *row = (unsigned short *)((unsigned char *)dc->bits + (size_t)y * (size_t)dc->pitch);
        for (x = c.left; x < c.right; x++) {
            row[x] = v;
        }
    }
}

int FillRect(HDC handle, const RECT *r, HBRUSH brush_handle) {
    struct Dc *dc = AsDc(handle);
    struct Brush *brush = (struct Brush *)Lookup(brush_handle, GDI_BRUSH);

    if (dc == NULL || brush == NULL) {
        return 0;
    }
    if (!brush->hollow) {
        Fill(dc, r, brush->color);
    }
    return 1;
}

BOOL MoveToEx(HDC handle, int x, int y, LPPOINT old) {
    struct Dc *dc = AsDc(handle);

    if (dc == NULL) {
        return FALSE;
    }
    if (old != NULL) {
        *old = dc->position;
    }
    dc->position.x = x;
    dc->position.y = y;
    return TRUE;
}

/* a solid line from the current position to (x, y), the end point left out as GDI does, w pixels wide */
BOOL LineTo(HDC handle, int x, int y) {
    struct Dc *dc = AsDc(handle);
    int x0;
    int y0;
    int dx;
    int dy;
    int sx;
    int sy;
    int err;

    if (dc == NULL) {
        return FALSE;
    }
    x0 = dc->position.x;
    y0 = dc->position.y;
    dc->position.x = x;
    dc->position.y = y;
    if (dc->pen == NULL || dc->pen->null || dc->bits == NULL) {
        return TRUE;
    }
    dx = abs(x - x0);
    dy = -abs(y - y0);
    sx = x0 < x ? 1 : -1;
    sy = y0 < y ? 1 : -1;
    err = dx + dy;
    while (x0 != x || y0 != y) {
        RECT dot;
        int w = dc->pen->width;
        int e2;

        SetRect(&dot, x0 - w / 2, y0 - w / 2, x0 - w / 2 + w, y0 - w / 2 + w);
        Fill(dc, &dot, dc->pen->color);
        e2 = 2 * err;
        if (e2 >= dy) {
            err += dy;
            x0 += sx;
        }
        if (e2 <= dx) {
            err += dx;
            y0 += sy;
        }
    }
    return TRUE;
}

/* ---- text ---- */

static int Advance(const struct Font *font, unsigned int ch) {
    int advance;
    int lsb;

    if (ch == 0 || ch == '	') {
        return 0; /* a byte the code page leaves undefined, or a tab that isn't expanded: nothing, as GDI */
    }
    if (font->file == NULL) {
        return font->avg_width;
    }
    stbtt_GetCodepointHMetrics(&font->file->info, (int)ch, &advance, &lsb);
    return (int)(advance * font->scale + 0.5f) + font->embolden;
}

/* the width of a run of bytes, tabs expanded (every 8 average widths) when tabs is set */
static int Width(const struct Font *font, const unsigned char *s, int n, int tabs) {
    int w = 0;
    int i;

    for (i = 0; i < n; i++) {
        if (s[i] == '\t' && tabs) {
            int stop = font->avg_width * 8;
            w = stop > 0 ? (w / stop + 1) * stop : w;
        } else {
            w += Advance(font, Unicode(font->lf.lfCharSet, s[i]));
        }
    }
    return w;
}

void (*MiniwinTrace)(const char *line);

/* Text that drew nothing, to the trace (the first few): the game's screens print theirs with TextOutA and
 * DrawTextA, and a text missing in the game shows here with why (no pixels, no typeface, clipped away). */
static long drawn_pixels;
static int nothing_reported;

static void ReportNothingDrawn(const char *call, const struct Dc *dc, const char *text, int count, const RECT *rect,
    const RECT *clip) {
    char line[320];
    char shown[48];
    int i;
    int n = 0;
    int visible = 0;

    for (i = 0; i < count && text[i] != 0; i++) {
        visible |= text[i] != ' ' && text[i] != '\r' && text[i] != '\n' && text[i] != '\t';
        if (n < (int)sizeof(shown) - 1) {
            shown[n++] = text[i] >= ' ' ? text[i] : ' ';
        }
    }
    shown[n] = 0;
    if (!visible || nothing_reported >= 40 || MiniwinTrace == NULL) {
        return;
    }
    nothing_reported++;
    snprintf(line, sizeof(line),
        "gdi: %s \"%s\" drew nothing: DC %dx%d%s, font %s (height %d, charset %d), colour %06lx, rect (%ld,%ld,%ld,%ld), "
        "clip (%ld,%ld,%ld,%ld)",
        call, shown, dc->width, dc->height, dc->bits == NULL ? " without pixels" : "",
        dc->font == NULL ? "none" : dc->font->file == NULL ? "without a typeface"
                                                           : dc->font->file->face,
        dc->font != NULL ? (int)dc->font->lf.lfHeight : 0, dc->font != NULL ? (int)dc->font->lf.lfCharSet : 0,
        (unsigned long)dc->text_color, rect != NULL ? (long)rect->left : 0, rect != NULL ? (long)rect->top : 0,
        rect != NULL ? (long)rect->right : 0, rect != NULL ? (long)rect->bottom : 0, (long)clip->left, (long)clip->top,
        (long)clip->right, (long)clip->bottom);
    MiniwinTrace(line);
}

/* draws a run with its cell's top-left at (x, y), within clip */
static void DrawRun(struct Dc *dc, int x, int y, const unsigned char *s, int n, int tabs, const RECT *clip) {
    const struct Font *font = dc->font;
    int pen = x;
    int i;

    if (font == NULL || dc->bits == NULL) {
        return;
    }
    if (dc->bk_mode == OPAQUE) {
        RECT cell;
        RECT c;
        SetRect(&cell, x, y, x + Width(font, s, n, tabs), y + font->ascent + font->descent);
        if (IntersectRect(&c, &cell, clip)) {
            Fill(dc, &c, dc->bk_color);
        }
    }
    for (i = 0; i < n; i++) {
        unsigned int ch;
        int x0, y0, x1, y1;
        int w, h;
        unsigned char *bitmap;
        int pass;

        if (s[i] == '\t' && tabs) {
            int stop = font->avg_width * 8;
            pen = stop > 0 ? x + ((pen - x) / stop + 1) * stop : pen;
            continue;
        }
        ch = Unicode(font->lf.lfCharSet, s[i]);
        if (font->file != NULL && ch != ' ' && ch != 0 && ch != '	') {
            stbtt_GetCodepointBitmapBox(&font->file->info, (int)ch, font->scale, font->scale, &x0, &y0, &x1, &y1);
            w = x1 - x0;
            h = y1 - y0;
            bitmap = w > 0 && h > 0 ? (unsigned char *)malloc((size_t)(w * h)) : NULL;
            if (bitmap != NULL) {
                stbtt_MakeCodepointBitmap(&font->file->info, bitmap, w, h, w, font->scale, font->scale, (int)ch);
                for (pass = 0; pass <= font->embolden; pass++) {
                    int gx;
                    int gy;
                    for (gy = 0; gy < h; gy++) {
                        int py = y + font->ascent + y0 + gy;
                        unsigned short *row;
                        if (py < clip->top || py >= clip->bottom) {
                            continue;
                        }
                        row = (unsigned short *)((unsigned char *)dc->bits + (size_t)py * (size_t)dc->pitch);
                        for (gx = 0; gx < w; gx++) {
                            int px = pen + x0 + gx + pass;
                            int coverage = bitmap[gy * w + gx];
                            if (coverage > 0 && px >= clip->left && px < clip->right) {
                                Blend(&row[px], dc->text_color, coverage);
                                drawn_pixels++;
                            }
                        }
                    }
                }
                free(bitmap);
            }
        }
        pen += Advance(font, ch);
    }
}

BOOL TextOutA(HDC handle, int x, int y, LPCSTR text, int count) {
    struct Dc *dc = AsDc(handle);
    const struct Font *font;
    int w;

    if (dc == NULL || text == NULL) {
        return FALSE;
    }
    font = dc->font;
    w = Width(font, (const unsigned char *)text, count, 0);
    if ((dc->align & TA_CENTER) == TA_CENTER) {
        x -= w / 2;
    } else if ((dc->align & TA_RIGHT) != 0) {
        x -= w;
    }
    if ((dc->align & TA_BASELINE) == TA_BASELINE) {
        y -= font->ascent;
    } else if ((dc->align & TA_BOTTOM) != 0) {
        y -= font->ascent + font->descent;
    }
    {
        long before = drawn_pixels;
        RECT at;

        DrawRun(dc, x, y, (const unsigned char *)text, count, 0, &dc->clip);
        if (drawn_pixels == before && dc->bits != NULL) {
            SetRect(&at, x, y, x + w, y + font->ascent + font->descent);
            ReportNothingDrawn("TextOutA", dc, text, count, &at, &dc->clip);
        }
    }
    if ((dc->align & TA_UPDATECP) != 0) {
        dc->position.x += w;
    }
    return TRUE;
}

struct Line {
    int start;
    int length;
    int spaces; /* the spaces it broke at: DT_CALCRECT counts one of them while the line still fits, as Windows */
};

/* DrawText: the text's lines (at newlines unless single-line; at spaces where too wide, with DT_WORDBREAK), each
 * aligned in the rectangle; with DT_CALCRECT only measured. The & prefixes are removed and the next character
 * underlined (DT_NOPREFIX keeps them). Returns the text's height. */
int DrawTextA(HDC handle, LPCSTR text, int count, LPRECT rect, UINT format) {
    struct Dc *dc = AsDc(handle);
    const struct Font *font;
    unsigned char *s;
    int *underline; /* the prefixed character's index in s, per character: -1 none */
    int n = 0;
    int i;
    struct Line *lines;
    int line_count = 0;
    int line_height;
    int max_width = 0;
    int box_width;
    int tabs = (format & DT_EXPANDTABS) != 0;
    int y;
    int result;
    RECT clip;
    long drawn_before = 0;
    int drew_any = 0;

    if (dc == NULL || text == NULL || rect == NULL) {
        return 0;
    }
    font = dc->font;
    if (count < 0) {
        count = (int)strlen(text);
    }
    s = (unsigned char *)malloc((size_t)count + 1);
    underline = (int *)malloc(sizeof(int) * ((size_t)count + 1));
    lines = (struct Line *)malloc(sizeof(struct Line) * ((size_t)count + 1));
    if (s == NULL || underline == NULL || lines == NULL) {
        free(s);
        free(underline);
        free(lines);
        return 0;
    }
    for (i = 0; i < count; i++) {
        unsigned char c = (unsigned char)text[i];
        underline[n] = 0;
        if (c == '&' && (format & DT_NOPREFIX) == 0) {
            if (i + 1 < count && text[i + 1] == '&') {
                i++;
            } else {
                if (i + 1 < count) {
                    underline[n] = 1;
                }
                continue;
            }
        }
        if ((format & DT_SINGLELINE) != 0 && (c == '\r' || c == '\n')) {
            continue;
        }
        s[n++] = c;
    }
    box_width = rect->right - rect->left;
    if (n == 0 && (format & DT_SINGLELINE) == 0) {
        /* nothing to draw: Windows returns 1, and measures an empty rectangle (a single line is measured) */
        if ((format & DT_CALCRECT) != 0) {
            rect->right = rect->left;
            rect->bottom = rect->top;
        }
        free(s);
        free(underline);
        free(lines);
        return 1;
    }
    /* lines */
    {
        int start = 0;
        while (start <= n) {
            int end = start;
            while (end < n && s[end] != '\n' && s[end] != '\r') {
                end++;
            }
            if ((format & DT_WORDBREAK) != 0 && (format & DT_SINGLELINE) == 0) {
                int at = start;
                for (;;) {
                    int last = -1; /* the end of the last word that still fits (the first word always does) */
                    int j = at;
                    int k;

                    while (j < end) {
                        int word_end = j;
                        int w;
                        while (word_end < end && s[word_end] != ' ') {
                            word_end++;
                        }
                        w = Width(font, s + at, word_end - at, tabs);
                        if (w > box_width && last >= 0) {
                            break;
                        }
                        last = word_end;
                        if (w > box_width) {
                            break; /* a word wider than the box: alone on its line */
                        }
                        j = word_end;
                        while (j < end && s[j] == ' ') {
                            j++;
                        }
                    }
                    if (last < 0) {
                        last = end;
                    }
                    k = last;
                    while (k > at && s[k - 1] == ' ') {
                        k--; /* the spaces at a break aren't part of the line */
                    }
                    lines[line_count].start = at;
                    lines[line_count].length = k - at;
                    {
                        int sp = 0;
                        while (last + sp < end && s[last + sp] == ' ') {
                            sp++;
                        }
                        lines[line_count].spaces = (last - k) + sp;
                    }
                    line_count++;
                    at = last;
                    while (at < end && s[at] == ' ') {
                        at++;
                    }
                    if (at >= end) {
                        break;
                    }
                }
            } else {
                lines[line_count].start = start;
                lines[line_count].length = end - start;
                lines[line_count].spaces = 0;
                line_count++;
            }
            if (end >= n) {
                break;
            }
            start = end + (s[end] == '\r' && end + 1 < n && s[end + 1] == '\n' ? 2 : 1);
            if ((format & DT_SINGLELINE) != 0) {
                break;
            }
        }
    }
    line_height = font->ascent + font->descent;
    for (i = 0; i < line_count; i++) {
        int w = Width(font, s + lines[i].start, lines[i].length, tabs);
        if (lines[i].spaces > 0 && w + Advance(font, ' ') <= box_width) {
            w += Advance(font, ' ');
        }
        if (w > max_width) {
            max_width = w;
        }
    }
    result = line_count * line_height;
    y = rect->top;
    if ((format & DT_SINGLELINE) != 0) {
        if ((format & DT_VCENTER) != 0) {
            y = rect->top + (rect->bottom - rect->top - line_height) / 2;
        } else if ((format & DT_BOTTOM) != 0) {
            y = rect->bottom - line_height;
        }
        if ((format & (DT_VCENTER | DT_BOTTOM)) != 0) {
            result = y - rect->top + line_height;
        }
    }
    if ((format & DT_CALCRECT) != 0) {
        /* to the widest line, narrower or wider than the box */
        rect->right = rect->left + max_width;
        rect->bottom = rect->top + line_count * line_height;
        free(s);
        free(underline);
        free(lines);
        return line_count * line_height;
    }
    drawn_before = drawn_pixels;
    if ((format & DT_NOCLIP) != 0 || !IntersectRect(&clip, rect, &dc->clip)) {
        if ((format & DT_NOCLIP) != 0) {
            clip = dc->clip;
        } else {
            SetRect(&clip, 0, 0, 0, 0);
        }
    }
    for (i = 0; i < line_count; i++) {
        int w = Width(font, s + lines[i].start, lines[i].length, tabs);
        int x = rect->left;
        int j;

        if ((format & DT_CENTER) != 0) {
            x = rect->left + (box_width - w) / 2;
        } else if ((format & DT_RIGHT) != 0) {
            x = rect->right - w;
        }
        DrawRun(dc, x, y, s + lines[i].start, lines[i].length, tabs, &clip);
        drew_any |= drawn_pixels != drawn_before;
        for (j = 0; j < lines[i].length; j++) {
            if (underline[lines[i].start + j]) {
                RECT u;
                int ux = x + Width(font, s + lines[i].start, j, tabs);
                SetRect(&u, ux, y + font->ascent + 1, ux + Width(font, s + lines[i].start + j, 1, tabs),
                    y + font->ascent + 2);
                if (IntersectRect(&u, &u, &clip)) {
                    Fill(dc, &u, dc->text_color);
                }
            }
        }
        y += line_height;
    }
    if (!drew_any && dc->bits != NULL) {
        ReportNothingDrawn("DrawTextA", dc, text, count, rect, &clip);
    }
    free(s);
    free(underline);
    free(lines);
    return result;
}

/* ---- bitmaps (the certificate's, for its printer: none here) ---- */

HBITMAP CreateDIBitmap(HDC dc, const BITMAPINFOHEADER *header, DWORD init, const void *bits, const BITMAPINFO *info,
    UINT usage) {
    /* the game only selects it into a memory DC: a brush-like object is enough to stand for it */
    (void)dc;
    (void)header;
    (void)init;
    (void)bits;
    (void)info;
    (void)usage;
    return (HBITMAP)CreateSolidBrush(0);
}

/* one source pixel of a DIB (1, 4, 8, 16 (5-5-5), 24 or 32 bits, uncompressed), as a COLORREF */
static COLORREF DibPixel(const BITMAPINFO *info, const unsigned char *bits, int x, int y) {
    const BITMAPINFOHEADER *h = &info->bmiHeader;
    int bpp = h->biBitCount;
    int height = h->biHeight < 0 ? -h->biHeight : h->biHeight;
    size_t stride = (((size_t)h->biWidth * (size_t)bpp + 31) / 32) * 4;
    const unsigned char *row = bits + stride * (size_t)(h->biHeight > 0 ? height - 1 - y : y);
    const RGBQUAD *palette = info->bmiColors;

    switch (bpp) {
    case 1:
        return RGB(palette[(row[x / 8] >> (7 - x % 8)) & 1].rgbRed, palette[(row[x / 8] >> (7 - x % 8)) & 1].rgbGreen,
            palette[(row[x / 8] >> (7 - x % 8)) & 1].rgbBlue);
    case 4: {
        int k = (row[x / 2] >> (x % 2 == 0 ? 4 : 0)) & 15;
        return RGB(palette[k].rgbRed, palette[k].rgbGreen, palette[k].rgbBlue);
    }
    case 8:
        return RGB(palette[row[x]].rgbRed, palette[row[x]].rgbGreen, palette[row[x]].rgbBlue);
    case 16: {
        unsigned int v = row[x * 2] | (row[x * 2 + 1] << 8);
        return RGB(((v >> 10) & 31) * 255 / 31, ((v >> 5) & 31) * 255 / 31, (v & 31) * 255 / 31);
    }
    case 24:
        return RGB(row[x * 3 + 2], row[x * 3 + 1], row[x * 3]);
    case 32:
        return RGB(row[x * 4 + 2], row[x * 4 + 1], row[x * 4]);
    default:
        return 0;
    }
}

/* the source rectangle scaled into the destination (nearest pixel), within the DC's clip region */
int StretchDIBits(HDC handle, int x, int y, int w, int h, int sx, int sy, int sw, int sh, const void *bits,
    const BITMAPINFO *info, UINT usage, DWORD rop) {
    struct Dc *dc = AsDc(handle);
    int dx;
    int dy;

    (void)usage;
    if (dc == NULL || bits == NULL || info == NULL || w <= 0 || h <= 0 || sw <= 0 || sh <= 0 || rop != SRCCOPY ||
        info->bmiHeader.biCompression != BI_RGB) {
        return 0;
    }
    if (dc->bits == NULL) {
        return h;
    }
    for (dy = 0; dy < h; dy++) {
        int py = y + dy;
        int srcy = sy + (int)((long long)dy * sh / h);
        unsigned short *row;
        if (py < dc->clip.top || py >= dc->clip.bottom) {
            continue;
        }
        row = (unsigned short *)((unsigned char *)dc->bits + (size_t)py * (size_t)dc->pitch);
        for (dx = 0; dx < w; dx++) {
            int px = x + dx;
            if (px >= dc->clip.left && px < dc->clip.right) {
                row[px] = To565(DibPixel(info, (const unsigned char *)bits, sx + (int)((long long)dx * sw / w), srcy));
            }
        }
    }
    return h;
}

/* ---- printing: the picture printer ---- */

BOOL GetDefaultPrinterA(LPSTR name, LPDWORD size) {
    DWORD need = (DWORD)sizeof(PRINTER_NAME);

    if (name == NULL || *size < need) {
        *size = need;
        PosixSetError(ERROR_INSUFFICIENT_BUFFER);
        return FALSE;
    }
    strcpy(name, PRINTER_NAME);
    *size = need;
    return TRUE;
}

BOOL OpenPrinterA(LPSTR name, LPHANDLE printer, LPPRINTER_DEFAULTSA defaults) {
    static char handle_tag;

    (void)defaults;
    if (name == NULL || strcmp(name, PRINTER_NAME) != 0) {
        *printer = NULL;
        PosixSetError(ERROR_INVALID_PRINTER_NAME);
        return FALSE;
    }
    *printer = (HANDLE)&handle_tag;
    return TRUE;
}

BOOL ClosePrinter(HANDLE printer) {
    return printer != NULL;
}

LONG DocumentPropertiesA(HWND w, HANDLE printer, LPSTR name, PDEVMODEA out, PDEVMODEA in, DWORD mode) {
    (void)w;
    (void)name;
    if (printer == NULL) {
        return -1;
    }
    if (mode == 0) {
        return (LONG)sizeof(DEVMODEA);
    }
    if ((mode & DM_OUT_BUFFER) != 0 && out != NULL) {
        if (in != NULL && in != out) {
            *out = *in;
        } else if (in == NULL) {
            memset(out, 0, sizeof(*out));
            out->dmSize = sizeof(*out);
            out->dmOrientation = DMORIENT_LANDSCAPE;
            out->dmFields = DM_ORIENTATION;
        }
    }
    return IDOK;
}

BOOL EnumPrintersA(DWORD flags, LPSTR name, DWORD level, LPBYTE buffer, DWORD size, LPDWORD needed,
    LPDWORD returned) {
    (void)flags;
    (void)name;
    (void)level;
    (void)buffer;
    (void)size;
    if (needed != NULL) {
        *needed = 0;
    }
    if (returned != NULL) {
        *returned = 0;
    }
    return TRUE; /* the original's PRINTER_ENUM_DEFAULT query lists none on Windows NT either (certificate.c) */
}

int StartDocA(HDC handle, const DOCINFOA *info) {
    struct Dc *dc = AsDc(handle);

    if (dc == NULL || !dc->printer) {
        return -1;
    }
    snprintf(dc->document, sizeof(dc->document), "%s", info != NULL && info->lpszDocName != NULL ? info->lpszDocName : "page");
    return 1;
}

int StartPage(HDC handle) {
    struct Dc *dc = AsDc(handle);

    return dc != NULL && dc->printer ? 1 : 0;
}

int EndPage(HDC handle) {
    struct Dc *dc = AsDc(handle);

    return dc != NULL && dc->printer ? 1 : 0;
}

static void Put32(unsigned char *p, unsigned int v) {
    p[0] = (unsigned char)v;
    p[1] = (unsigned char)(v >> 8);
    p[2] = (unsigned char)(v >> 16);
    p[3] = (unsigned char)(v >> 24);
}

/* the page as a 24-bit BMP named after the document and the time, in the current folder */
int EndDoc(HDC handle) {
    struct Dc *dc = AsDc(handle);
    char name[160];
    char text[400];
    char *c;
    time_t now = time(NULL);
    struct tm tm;
    FILE *f;
    size_t stride;
    unsigned char header[54];
    unsigned char *row;
    int x;
    int y;

    if (dc == NULL || !dc->printer) {
        return 0;
    }
    localtime_r(&now, &tm);
    snprintf(name, sizeof(name), "%s %04d-%02d-%02d %02d.%02d.%02d.bmp", dc->document, tm.tm_year + 1900,
        tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
    for (c = name; *c != '\0'; c++) {
        if (*c == '/' || *c == '\\') {
            *c = '-';
        }
    }
    stride = ((size_t)dc->width * 3 + 3) & ~(size_t)3;
    memset(header, 0, sizeof(header));
    header[0] = 'B';
    header[1] = 'M';
    Put32(header + 2, (unsigned int)(54 + stride * (size_t)dc->height));
    Put32(header + 10, 54);
    Put32(header + 14, 40);
    Put32(header + 18, (unsigned int)dc->width);
    Put32(header + 22, (unsigned int)dc->height);
    header[26] = 1;
    header[28] = 24;
    Put32(header + 38, (unsigned int)(dc->dpi * 10000 / 254));
    Put32(header + 42, (unsigned int)(dc->dpi * 10000 / 254));
    f = fopen(name, "wb");
    row = (unsigned char *)calloc(1, stride);
    if (f == NULL || row == NULL) {
        if (f != NULL) {
            fclose(f);
        }
        free(row);
        PosixSetError(ERROR_ACCESS_DENIED);
        return 0;
    }
    fwrite(header, 1, sizeof(header), f);
    for (y = dc->height - 1; y >= 0; y--) {
        const unsigned short *src = (const unsigned short *)((const unsigned char *)dc->bits + (size_t)y * (size_t)dc->pitch);
        for (x = 0; x < dc->width; x++) {
            unsigned int v = src[x];
            row[x * 3 + 2] = (unsigned char)(((v >> 11) & 31) * 255 / 31);
            row[x * 3 + 1] = (unsigned char)(((v >> 5) & 63) * 255 / 63);
            row[x * 3] = (unsigned char)((v & 31) * 255 / 31);
        }
        fwrite(row, 1, stride, f);
    }
    free(row);
    fclose(f);
    {
        char cwd[PATH_MAX];
        snprintf(text, sizeof(text), "The certificate was saved as a picture:\n%s/%s",
            PosixGetcwd(cwd, sizeof(cwd)) != NULL ? cwd : ".", name);
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_INFORMATION, "LEGOLAND", text, NULL);
    }
    return 1;
}

/* ---- COM: no system objects here (the game's DirectMusic is miniwin's) ---- */

HRESULT CoInitialize(LPVOID reserved) {
    (void)reserved;
    return S_OK;
}

HRESULT CoCreateInstance(REFCLSID clsid, LPUNKNOWN outer, DWORD context, REFIID iid, LPVOID *out) {
    (void)clsid;
    (void)outer;
    (void)context;
    (void)iid;
    if (out != NULL) {
        *out = NULL;
    }
    return REGDB_E_CLASSNOTREG;
}
