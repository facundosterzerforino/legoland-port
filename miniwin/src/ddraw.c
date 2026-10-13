/* [library:video] miniwin's DirectDraw: surfaces in memory, drawn in C. See miniwin/miniwin.h.
 *
 * Only what the game uses is implemented; everything else returns DDERR_UNSUPPORTED. Every surface is 16-bit
 * RGB565 (the format the game draws in its windowed mode). The results are checked pixel by pixel against
 * Windows' own DirectDraw by tools/test_miniwin. */
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "miniwin/miniwin.h"

#define CLIP_RECTS_MAX 16

struct Clipper {
    IDirectDrawClipper iface;
    LONG refs;
    HWND hwnd;
    int count; /* 0: no clip list (only the surface's own bounds) */
    RECT rects[CLIP_RECTS_MAX];
};

struct Palette {
    IDirectDrawPalette iface;
    LONG refs;
    DWORD caps;
    PALETTEENTRY entries[256];
};

struct Surface {
    IDirectDrawSurface iface;
    LONG refs;
    DWORD caps;
    int width;
    int height;
    LONG pitch;
    unsigned char *bits;
#ifdef _WIN32
    HBITMAP dib; /* the pixels belong to this DIB section, so that GDI can draw on them (GetDC) */
    HDC dc;
    HGDIOBJ dc_old;
#endif
    int has_colorkey;
    DDCOLORKEY colorkey; /* source colour key (DDCKEY_SRCBLT) */
    struct Clipper *clipper;
    struct Palette *palette;
};

struct DirectDraw {
    IDirectDraw v1;
    IDirectDraw2 v2;
    LONG refs;
    HWND hwnd;
    DWORD width; /* the "display mode": the primary surface's size */
    DWORD height;
};

static struct Surface *AsSurface(IDirectDrawSurface *p) {
    return (struct Surface *)p;
}

static struct Clipper *AsClipper(IDirectDrawClipper *p) {
    return (struct Clipper *)p;
}

static struct Palette *AsPalette(IDirectDrawPalette *p) {
    return (struct Palette *)p;
}

static struct DirectDraw *FromV1(IDirectDraw *p) {
    return (struct DirectDraw *)p;
}

static struct DirectDraw *FromV2(IDirectDraw2 *p) {
    return (struct DirectDraw *)((char *)p - offsetof(struct DirectDraw, v2));
}

static int SameGuid(REFIID a, const GUID *b) {
    return memcmp(a, b, sizeof(GUID)) == 0;
}

/* ---- rectangles ---- */

static int RectIntersect(RECT *out, const RECT *a, const RECT *b) {
    out->left = a->left > b->left ? a->left : b->left;
    out->top = a->top > b->top ? a->top : b->top;
    out->right = a->right < b->right ? a->right : b->right;
    out->bottom = a->bottom < b->bottom ? a->bottom : b->bottom;
    return out->left < out->right && out->top < out->bottom;
}

static int RectInside(const RECT *r, int width, int height) {
    return r->left >= 0 && r->top >= 0 && r->right <= width && r->bottom <= height && r->left < r->right &&
        r->top < r->bottom;
}

static void WholeSurface(const struct Surface *s, RECT *r) {
    r->left = 0;
    r->top = 0;
    r->right = s->width;
    r->bottom = s->height;
}

/* ---- surface description ---- */

static void FillPixelFormat(DDPIXELFORMAT *pf) {
    memset(pf, 0, sizeof(*pf));
    pf->dwSize = sizeof(*pf);
    pf->dwFlags = DDPF_RGB;
    pf->dwRGBBitCount = 16;
    pf->dwRBitMask = 0xf800;
    pf->dwGBitMask = 0x07e0;
    pf->dwBBitMask = 0x001f;
}

static void Describe(const struct Surface *s, DDSURFACEDESC *desc) {
    memset(desc, 0, sizeof(*desc));
    desc->dwSize = sizeof(*desc);
    desc->dwFlags = DDSD_CAPS | DDSD_WIDTH | DDSD_HEIGHT | DDSD_PITCH | DDSD_PIXELFORMAT;
    desc->dwWidth = (DWORD)s->width;
    desc->dwHeight = (DWORD)s->height;
    desc->lPitch = s->pitch;
    desc->ddsCaps.dwCaps = s->caps;
    FillPixelFormat(&desc->ddpfPixelFormat);
}

/* ---- Blt ---- */

static void Fill(struct Surface *d, const RECT *r, WORD colour) {
    int x;
    int y;

    for (y = r->top; y < r->bottom; y++) {
        WORD *row = (WORD *)(d->bits + (size_t)y * d->pitch);
        for (x = r->left; x < r->right; x++) {
            row[x] = colour;
        }
    }
}

/* Stretching, as Windows' DirectDraw does it (measured with tools/test_miniwin and a probe of every pair of sizes
 * from 1 to 640, clipped and not; exact). */

/* Which of s source pixels destination pixel i (of d) shows: enlarging starts half a source pixel in, shrinking
 * skips half of the whole source pixels each destination pixel covers. */
static int StretchIndex(int i, int s, int d) {
    if (d >= s) {
        return (int)(((long long)i * s + (s - 1) / 2) / d);
    }
    return s / d / 2 + (int)(((long long)i * s + d / 2) / d);
}

/* For the visible destination pixels a..b-1 of a stretch from s pixels to d (a = 0, b = d when nothing is clipped
 * off): the source pixels *first..*end-1 they are stretched from. Clipping trims the source in 16.16 fixed point,
 * rounded to the nearest pixel, and keeps at least one pixel inside the source. */
static void TrimSource(int a, int b, int s, int d, int *first, int *end) {
    long long step = ((long long)s << 16) / d;
    int s0 = (int)((a * step + 0x8000) >> 16);
    int s1 = (int)((b * step + 0x8000) >> 16);

    if (s1 <= s0) {
        if (s0 >= s) {
            s0 = s - 1;
        }
        s1 = s0 + 1;
    }
    *first = s0;
    *end = s1;
}

/* Copies the source rectangle sr (in bits/pitch) to the destination rectangle dr of d, scaled to dr's size, writing
 * only the pixels inside part (dr clipped). */
static HRESULT Copy(struct Surface *d, const RECT *part, const RECT *dr, const unsigned char *bits, LONG pitch,
    const RECT *sr, const DDCOLORKEY *key) {
    int dw = dr->right - dr->left;
    int dh = dr->bottom - dr->top;
    int sw = sr->right - sr->left;
    int sh = sr->bottom - sr->top;
    int pw = part->right - part->left;
    int ph = part->bottom - part->top;
    int x0;
    int x1;
    int y0;
    int y1;
    int *columns;
    int x;
    int y;

    if (sw == dw && sh == dh) {
        /* not scaled: a plain copy of the matching source pixels */
        for (y = 0; y < ph; y++) {
            const WORD *srow = (const WORD *)(bits + (size_t)(sr->top + part->top - dr->top + y) * pitch) + sr->left +
                (part->left - dr->left);
            WORD *drow = (WORD *)(d->bits + (size_t)(part->top + y) * d->pitch) + part->left;

            if (key == NULL) {
                memcpy(drow, srow, (size_t)pw * 2);
                continue;
            }
            for (x = 0; x < pw; x++) {
                if (srow[x] < key->dwColorSpaceLowValue || srow[x] > key->dwColorSpaceHighValue) {
                    drow[x] = srow[x];
                }
            }
        }
        return DD_OK;
    }
    TrimSource(part->left - dr->left, part->right - dr->left, sw, dw, &x0, &x1);
    TrimSource(part->top - dr->top, part->bottom - dr->top, sh, dh, &y0, &y1);
    columns = (int *)malloc((size_t)pw * sizeof(int));
    if (columns == NULL) {
        return DDERR_OUTOFMEMORY;
    }
    for (x = 0; x < pw; x++) {
        columns[x] = sr->left + x0 + StretchIndex(x, x1 - x0, pw);
    }
    for (y = 0; y < ph; y++) {
        const WORD *srow = (const WORD *)(bits + (size_t)(sr->top + y0 + StretchIndex(y, y1 - y0, ph)) * pitch);
        WORD *drow = (WORD *)(d->bits + (size_t)(part->top + y) * d->pitch) + part->left;

        for (x = 0; x < pw; x++) {
            WORD v = srow[columns[x]];
            if (key == NULL || v < key->dwColorSpaceLowValue || v > key->dwColorSpaceHighValue) {
                drow[x] = v;
            }
        }
    }
    free(columns);
    return DD_OK;
}

#define BLT_SUPPORTED (DDBLT_WAIT | DDBLT_ASYNC | DDBLT_KEYSRC | DDBLT_COLORFILL | DDBLT_DONOTWAIT)

static HRESULT STDMETHODCALLTYPE Surface_Blt(IDirectDrawSurface *self, LPRECT dst_rect, LPDIRECTDRAWSURFACE src_iface,
    LPRECT src_rect, DWORD flags, LPDDBLTFX fx) {
    struct Surface *d = AsSurface(self);
    struct Surface *s = AsSurface(src_iface);
    RECT dr;
    RECT sr;
    RECT whole;
    RECT part;
    unsigned char *copy = NULL;
    const unsigned char *bits = NULL;
    LONG pitch = 0;
    const DDCOLORKEY *key = NULL;
    int i;

    if ((flags & ~BLT_SUPPORTED) != 0) {
        return DDERR_UNSUPPORTED;
    }
    WholeSurface(d, &whole);
    if (dst_rect != NULL) {
        dr = *dst_rect;
    } else {
        dr = whole;
    }
    if (dr.left >= dr.right || dr.top >= dr.bottom) {
        return DDERR_INVALIDRECT;
    }
    if ((d->clipper == NULL || d->clipper->count == 0) && !RectInside(&dr, d->width, d->height)) {
        /* without a clip list, DirectDraw refuses a destination outside the surface rather than clipping it */
        return DDERR_INVALIDRECT;
    }
    if ((flags & DDBLT_COLORFILL) != 0) {
        if (fx == NULL) {
            return DDERR_INVALIDPARAMS;
        }
    } else {
        if (s == NULL) {
            return DDERR_INVALIDPARAMS;
        }
        if (src_rect != NULL) {
            sr = *src_rect;
        } else {
            WholeSurface(s, &sr);
        }
        if (!RectInside(&sr, s->width, s->height)) {
            return DDERR_INVALIDRECT;
        }
        if ((flags & DDBLT_KEYSRC) != 0) {
            if (!s->has_colorkey) {
                return DDERR_NOCOLORKEY;
            }
            key = &s->colorkey;
        }
        bits = s->bits;
        pitch = s->pitch;
        if (s == d) {
            /* within one surface: read from a copy, so overlapping rectangles come out as if copied at once */
            copy = (unsigned char *)malloc((size_t)s->pitch * s->height);
            if (copy == NULL) {
                return DDERR_OUTOFMEMORY;
            }
            memcpy(copy, s->bits, (size_t)s->pitch * s->height);
            bits = copy;
        }
    }
    for (i = 0; i < (d->clipper != NULL && d->clipper->count > 0 ? d->clipper->count : 1); i++) {
        RECT area;

        if (d->clipper != NULL && d->clipper->count > 0) {
            if (!RectIntersect(&area, &d->clipper->rects[i], &whole)) {
                continue;
            }
        } else {
            area = whole;
        }
        if (!RectIntersect(&part, &dr, &area)) {
            continue;
        }
        if ((flags & DDBLT_COLORFILL) != 0) {
            Fill(d, &part, (WORD)fx->dwFillColor);
        } else if (Copy(d, &part, &dr, bits, pitch, &sr, key) != DD_OK) {
            free(copy);
            return DDERR_OUTOFMEMORY;
        }
    }
    free(copy);
    return DD_OK;
}

/* ---- the rest of IDirectDrawSurface ---- */

static HRESULT STDMETHODCALLTYPE Surface_QueryInterface(IDirectDrawSurface *self, REFIID riid, LPVOID *out) {
    if (SameGuid(riid, &IID_IUnknown) || SameGuid(riid, &IID_IDirectDrawSurface)) {
        self->lpVtbl->AddRef(self);
        *out = self;
        return S_OK;
    }
    *out = NULL;
    return E_NOINTERFACE;
}

static ULONG STDMETHODCALLTYPE Surface_AddRef(IDirectDrawSurface *self) {
    return (ULONG)++AsSurface(self)->refs;
}

static ULONG STDMETHODCALLTYPE Surface_Release(IDirectDrawSurface *self) {
    struct Surface *s = AsSurface(self);
    LONG refs = --s->refs;

    if (refs > 0) {
        return (ULONG)refs;
    }
    if (s->clipper != NULL) {
        s->clipper->iface.lpVtbl->Release(&s->clipper->iface);
    }
    if (s->palette != NULL) {
        s->palette->iface.lpVtbl->Release(&s->palette->iface);
    }
#ifdef _WIN32
    if (s->dc != NULL) {
        SelectObject(s->dc, s->dc_old);
        DeleteDC(s->dc);
    }
    if (s->dib != NULL) {
        DeleteObject(s->dib);
    } else
#endif
    {
        free(s->bits);
    }
    free(s);
    return 0;
}

static HRESULT STDMETHODCALLTYPE Surface_Lock(IDirectDrawSurface *self, LPRECT rect, LPDDSURFACEDESC desc, DWORD flags,
    HANDLE event) {
    struct Surface *s = AsSurface(self);

    (void)flags;
    (void)event;
    if (desc == NULL || desc->dwSize != sizeof(*desc)) {
        return DDERR_INVALIDPARAMS;
    }
    if (rect != NULL && !RectInside(rect, s->width, s->height)) {
        return DDERR_INVALIDRECT;
    }
#ifdef _WIN32
    GdiFlush(); /* GDI may still be drawing on the DIB section */
#endif
    Describe(s, desc);
    desc->dwFlags |= DDSD_LPSURFACE;
    desc->lpSurface = s->bits;
    if (rect != NULL) {
        desc->lpSurface = s->bits + (size_t)rect->top * s->pitch + (size_t)rect->left * 2;
    }
    return DD_OK;
}

static HRESULT STDMETHODCALLTYPE Surface_Unlock(IDirectDrawSurface *self, LPVOID bits) {
    (void)self;
    (void)bits;
    return DD_OK;
}

static HRESULT STDMETHODCALLTYPE Surface_GetSurfaceDesc(IDirectDrawSurface *self, LPDDSURFACEDESC desc) {
    if (desc == NULL || desc->dwSize != sizeof(*desc)) {
        return DDERR_INVALIDPARAMS;
    }
    Describe(AsSurface(self), desc);
    return DD_OK;
}

static HRESULT STDMETHODCALLTYPE Surface_GetPixelFormat(IDirectDrawSurface *self, LPDDPIXELFORMAT pf) {
    (void)self;
    FillPixelFormat(pf);
    return DD_OK;
}

static HRESULT STDMETHODCALLTYPE Surface_GetCaps(IDirectDrawSurface *self, LPDDSCAPS caps) {
    caps->dwCaps = AsSurface(self)->caps;
    return DD_OK;
}

/* A fresh DC each time, as DirectDraw gives: the game's drawing starts from GDI's defaults (font, colours, mode). */
static HRESULT STDMETHODCALLTYPE Surface_GetDC(IDirectDrawSurface *self, HDC *out) {
#ifdef _WIN32
    struct Surface *s = AsSurface(self);

    if (s->dib == NULL) {
        return DDERR_CANTCREATEDC;
    }
    if (s->dc != NULL) {
        return DDERR_DCALREADYCREATED;
    }
    s->dc = CreateCompatibleDC(NULL);
    if (s->dc == NULL) {
        return DDERR_CANTCREATEDC;
    }
    s->dc_old = SelectObject(s->dc, s->dib);
    *out = s->dc;
    return DD_OK;
#else
    (void)self;
    *out = NULL;
    return DDERR_CANTCREATEDC;
#endif
}

static HRESULT STDMETHODCALLTYPE Surface_ReleaseDC(IDirectDrawSurface *self, HDC dc) {
#ifdef _WIN32
    struct Surface *s = AsSurface(self);

    if (s->dc == NULL || dc != s->dc) {
        return DDERR_INVALIDPARAMS;
    }
    GdiFlush();
    SelectObject(s->dc, s->dc_old);
    DeleteDC(s->dc);
    s->dc = NULL;
    return DD_OK;
#else
    (void)self;
    (void)dc;
    return DDERR_INVALIDPARAMS;
#endif
}

static HRESULT STDMETHODCALLTYPE Surface_SetColorKey(IDirectDrawSurface *self, DWORD flags, LPDDCOLORKEY key) {
    struct Surface *s = AsSurface(self);

    if ((flags & ~DDCKEY_COLORSPACE) != DDCKEY_SRCBLT) {
        return DDERR_UNSUPPORTED;
    }
    if (key == NULL) {
        s->has_colorkey = 0;
        return DD_OK;
    }
    s->colorkey.dwColorSpaceLowValue = key->dwColorSpaceLowValue & 0xffff;
    s->colorkey.dwColorSpaceHighValue =
        (flags & DDCKEY_COLORSPACE) != 0 ? key->dwColorSpaceHighValue & 0xffff : s->colorkey.dwColorSpaceLowValue;
    s->has_colorkey = 1;
    return DD_OK;
}

static HRESULT STDMETHODCALLTYPE Surface_GetColorKey(IDirectDrawSurface *self, DWORD flags, LPDDCOLORKEY key) {
    struct Surface *s = AsSurface(self);

    if (flags != DDCKEY_SRCBLT) {
        return DDERR_UNSUPPORTED;
    }
    if (!s->has_colorkey) {
        return DDERR_NOCOLORKEY;
    }
    *key = s->colorkey;
    return DD_OK;
}

static HRESULT STDMETHODCALLTYPE Surface_SetClipper(IDirectDrawSurface *self, LPDIRECTDRAWCLIPPER clipper) {
    struct Surface *s = AsSurface(self);

    if (clipper != NULL) {
        clipper->lpVtbl->AddRef(clipper);
    }
    if (s->clipper != NULL) {
        s->clipper->iface.lpVtbl->Release(&s->clipper->iface);
    }
    s->clipper = AsClipper(clipper);
    return DD_OK;
}

static HRESULT STDMETHODCALLTYPE Surface_GetClipper(IDirectDrawSurface *self, LPDIRECTDRAWCLIPPER *out) {
    struct Surface *s = AsSurface(self);

    if (s->clipper == NULL) {
        *out = NULL;
        return DDERR_NOCLIPPERATTACHED;
    }
    s->clipper->iface.lpVtbl->AddRef(&s->clipper->iface);
    *out = &s->clipper->iface;
    return DD_OK;
}

static HRESULT STDMETHODCALLTYPE Surface_SetPalette(IDirectDrawSurface *self, LPDIRECTDRAWPALETTE palette) {
    struct Surface *s = AsSurface(self);

    if (palette != NULL) {
        palette->lpVtbl->AddRef(palette);
    }
    if (s->palette != NULL) {
        s->palette->iface.lpVtbl->Release(&s->palette->iface);
    }
    s->palette = AsPalette(palette);
    return DD_OK;
}

static HRESULT STDMETHODCALLTYPE Surface_GetPalette(IDirectDrawSurface *self, LPDIRECTDRAWPALETTE *out) {
    struct Surface *s = AsSurface(self);

    if (s->palette == NULL) {
        *out = NULL;
        return DDERR_NOPALETTEATTACHED;
    }
    s->palette->iface.lpVtbl->AddRef(&s->palette->iface);
    *out = &s->palette->iface;
    return DD_OK;
}

/* Nothing is ever on screen or in video memory: flips, status queries and restores always succeed. */
static HRESULT STDMETHODCALLTYPE Surface_Flip(IDirectDrawSurface *self, LPDIRECTDRAWSURFACE target, DWORD flags) {
    (void)self;
    (void)target;
    (void)flags;
    return DD_OK;
}

static HRESULT STDMETHODCALLTYPE Surface_GetStatus(IDirectDrawSurface *self, DWORD flags) {
    (void)self;
    (void)flags;
    return DD_OK;
}

static HRESULT STDMETHODCALLTYPE Surface_IsLost(IDirectDrawSurface *self) {
    (void)self;
    return DD_OK;
}

static HRESULT STDMETHODCALLTYPE Surface_Restore(IDirectDrawSurface *self) {
    (void)self;
    return DD_OK;
}

/* unsupported */
static HRESULT STDMETHODCALLTYPE Surface_AddAttachedSurface(IDirectDrawSurface *self, LPDIRECTDRAWSURFACE a) {
    (void)self;
    (void)a;
    return DDERR_UNSUPPORTED;
}

static HRESULT STDMETHODCALLTYPE Surface_AddOverlayDirtyRect(IDirectDrawSurface *self, LPRECT a) {
    (void)self;
    (void)a;
    return DDERR_UNSUPPORTED;
}

static HRESULT STDMETHODCALLTYPE Surface_BltBatch(IDirectDrawSurface *self, LPDDBLTBATCH a, DWORD b, DWORD c) {
    (void)self;
    (void)a;
    (void)b;
    (void)c;
    return DDERR_UNSUPPORTED;
}

static HRESULT STDMETHODCALLTYPE Surface_BltFast(IDirectDrawSurface *self, DWORD a, DWORD b, LPDIRECTDRAWSURFACE c,
    LPRECT d, DWORD e) {
    (void)self;
    (void)a;
    (void)b;
    (void)c;
    (void)d;
    (void)e;
    return DDERR_UNSUPPORTED;
}

static HRESULT STDMETHODCALLTYPE Surface_DeleteAttachedSurface(IDirectDrawSurface *self, DWORD a,
    LPDIRECTDRAWSURFACE b) {
    (void)self;
    (void)a;
    (void)b;
    return DDERR_UNSUPPORTED;
}

static HRESULT STDMETHODCALLTYPE Surface_EnumAttachedSurfaces(IDirectDrawSurface *self, LPVOID a,
    LPDDENUMSURFACESCALLBACK b) {
    (void)self;
    (void)a;
    (void)b;
    return DDERR_UNSUPPORTED;
}

static HRESULT STDMETHODCALLTYPE Surface_EnumOverlayZOrders(IDirectDrawSurface *self, DWORD a, LPVOID b,
    LPDDENUMSURFACESCALLBACK c) {
    (void)self;
    (void)a;
    (void)b;
    (void)c;
    return DDERR_UNSUPPORTED;
}

static HRESULT STDMETHODCALLTYPE Surface_GetAttachedSurface(IDirectDrawSurface *self, LPDDSCAPS a,
    LPDIRECTDRAWSURFACE *b) {
    (void)self;
    (void)a;
    *b = NULL;
    return DDERR_NOTFOUND;
}

static HRESULT STDMETHODCALLTYPE Surface_GetOverlayPosition(IDirectDrawSurface *self, LPLONG a, LPLONG b) {
    (void)self;
    (void)a;
    (void)b;
    return DDERR_UNSUPPORTED;
}

static HRESULT STDMETHODCALLTYPE Surface_Initialize(IDirectDrawSurface *self, LPDIRECTDRAW a, LPDDSURFACEDESC b) {
    (void)self;
    (void)a;
    (void)b;
    return DDERR_ALREADYINITIALIZED;
}

static HRESULT STDMETHODCALLTYPE Surface_SetOverlayPosition(IDirectDrawSurface *self, LONG a, LONG b) {
    (void)self;
    (void)a;
    (void)b;
    return DDERR_UNSUPPORTED;
}

static HRESULT STDMETHODCALLTYPE Surface_UpdateOverlay(IDirectDrawSurface *self, LPRECT a, LPDIRECTDRAWSURFACE b,
    LPRECT c, DWORD d, LPDDOVERLAYFX e) {
    (void)self;
    (void)a;
    (void)b;
    (void)c;
    (void)d;
    (void)e;
    return DDERR_UNSUPPORTED;
}

static HRESULT STDMETHODCALLTYPE Surface_UpdateOverlayDisplay(IDirectDrawSurface *self, DWORD a) {
    (void)self;
    (void)a;
    return DDERR_UNSUPPORTED;
}

static HRESULT STDMETHODCALLTYPE Surface_UpdateOverlayZOrder(IDirectDrawSurface *self, DWORD a,
    LPDIRECTDRAWSURFACE b) {
    (void)self;
    (void)a;
    (void)b;
    return DDERR_UNSUPPORTED;
}

static IDirectDrawSurfaceVtbl SurfaceVtbl = {
    Surface_QueryInterface,
    Surface_AddRef,
    Surface_Release,
    Surface_AddAttachedSurface,
    Surface_AddOverlayDirtyRect,
    Surface_Blt,
    Surface_BltBatch,
    Surface_BltFast,
    Surface_DeleteAttachedSurface,
    Surface_EnumAttachedSurfaces,
    Surface_EnumOverlayZOrders,
    Surface_Flip,
    Surface_GetAttachedSurface,
    Surface_GetStatus, /* GetBltStatus */
    Surface_GetCaps,
    Surface_GetClipper,
    Surface_GetColorKey,
    Surface_GetDC,
    Surface_GetStatus, /* GetFlipStatus */
    Surface_GetOverlayPosition,
    Surface_GetPalette,
    Surface_GetPixelFormat,
    Surface_GetSurfaceDesc,
    Surface_Initialize,
    Surface_IsLost,
    Surface_Lock,
    Surface_ReleaseDC,
    Surface_Restore,
    Surface_SetClipper,
    Surface_SetColorKey,
    Surface_SetOverlayPosition,
    Surface_SetPalette,
    Surface_Unlock,
    Surface_UpdateOverlay,
    Surface_UpdateOverlayDisplay,
    Surface_UpdateOverlayZOrder,
};

/* ---- IDirectDrawClipper ---- */

static HRESULT STDMETHODCALLTYPE Clipper_QueryInterface(IDirectDrawClipper *self, REFIID riid, LPVOID *out) {
    if (SameGuid(riid, &IID_IUnknown) || SameGuid(riid, &IID_IDirectDrawClipper)) {
        self->lpVtbl->AddRef(self);
        *out = self;
        return S_OK;
    }
    *out = NULL;
    return E_NOINTERFACE;
}

static ULONG STDMETHODCALLTYPE Clipper_AddRef(IDirectDrawClipper *self) {
    return (ULONG)++AsClipper(self)->refs;
}

static ULONG STDMETHODCALLTYPE Clipper_Release(IDirectDrawClipper *self) {
    struct Clipper *c = AsClipper(self);
    LONG refs = --c->refs;

    if (refs == 0) {
        free(c);
    }
    return (ULONG)refs;
}

static HRESULT STDMETHODCALLTYPE Clipper_GetClipList(IDirectDrawClipper *self, LPRECT clip, LPRGNDATA data,
    LPDWORD size) {
    struct Clipper *c = AsClipper(self);
    DWORD needed;

    (void)clip;
    if (c->count == 0) {
        return DDERR_NOCLIPLIST;
    }
    needed = (DWORD)(sizeof(RGNDATAHEADER) + c->count * sizeof(RECT));
    if (data == NULL || *size < needed) {
        *size = needed;
        return data == NULL ? DD_OK : DDERR_REGIONTOOSMALL;
    }
    memset(&data->rdh, 0, sizeof(data->rdh));
    data->rdh.dwSize = sizeof(data->rdh);
    data->rdh.iType = RDH_RECTANGLES;
    data->rdh.nCount = (DWORD)c->count;
    data->rdh.nRgnSize = (DWORD)(c->count * sizeof(RECT));
    memcpy(data->Buffer, c->rects, c->count * sizeof(RECT));
    return DD_OK;
}

static HRESULT STDMETHODCALLTYPE Clipper_GetHWnd(IDirectDrawClipper *self, HWND *out) {
    *out = AsClipper(self)->hwnd;
    return DD_OK;
}

static HRESULT STDMETHODCALLTYPE Clipper_Initialize(IDirectDrawClipper *self, LPDIRECTDRAW dd, DWORD flags) {
    (void)self;
    (void)dd;
    (void)flags;
    return DDERR_ALREADYINITIALIZED;
}

static HRESULT STDMETHODCALLTYPE Clipper_IsClipListChanged(IDirectDrawClipper *self, BOOL *out) {
    (void)self;
    *out = FALSE;
    return DD_OK;
}

static HRESULT STDMETHODCALLTYPE Clipper_SetClipList(IDirectDrawClipper *self, LPRGNDATA data, DWORD flags) {
    struct Clipper *c = AsClipper(self);
    DWORD i;

    (void)flags;
    if (data == NULL) {
        c->count = 0;
        return DD_OK;
    }
    if (data->rdh.nCount > CLIP_RECTS_MAX) {
        return DDERR_OUTOFMEMORY;
    }
    for (i = 0; i < data->rdh.nCount; i++) {
        memcpy(&c->rects[i], data->Buffer + i * sizeof(RECT), sizeof(RECT));
    }
    c->count = (int)data->rdh.nCount;
    return DD_OK;
}

static HRESULT STDMETHODCALLTYPE Clipper_SetHWnd(IDirectDrawClipper *self, DWORD flags, HWND hwnd) {
    (void)flags;
    AsClipper(self)->hwnd = hwnd;
    return DD_OK;
}

static IDirectDrawClipperVtbl ClipperVtbl = {
    Clipper_QueryInterface,
    Clipper_AddRef,
    Clipper_Release,
    Clipper_GetClipList,
    Clipper_GetHWnd,
    Clipper_Initialize,
    Clipper_IsClipListChanged,
    Clipper_SetClipList,
    Clipper_SetHWnd,
};

/* ---- IDirectDrawPalette (kept, but 16-bit surfaces never use it) ---- */

static HRESULT STDMETHODCALLTYPE Palette_QueryInterface(IDirectDrawPalette *self, REFIID riid, LPVOID *out) {
    if (SameGuid(riid, &IID_IUnknown) || SameGuid(riid, &IID_IDirectDrawPalette)) {
        self->lpVtbl->AddRef(self);
        *out = self;
        return S_OK;
    }
    *out = NULL;
    return E_NOINTERFACE;
}

static ULONG STDMETHODCALLTYPE Palette_AddRef(IDirectDrawPalette *self) {
    return (ULONG)++AsPalette(self)->refs;
}

static ULONG STDMETHODCALLTYPE Palette_Release(IDirectDrawPalette *self) {
    struct Palette *p = AsPalette(self);
    LONG refs = --p->refs;

    if (refs == 0) {
        free(p);
    }
    return (ULONG)refs;
}

static HRESULT STDMETHODCALLTYPE Palette_GetCaps(IDirectDrawPalette *self, LPDWORD caps) {
    *caps = AsPalette(self)->caps;
    return DD_OK;
}

static HRESULT STDMETHODCALLTYPE Palette_GetEntries(IDirectDrawPalette *self, DWORD flags, DWORD base, DWORD count,
    LPPALETTEENTRY entries) {
    (void)flags;
    if (base > 256 || count > 256 - base) {
        return DDERR_INVALIDPARAMS;
    }
    memcpy(entries, &AsPalette(self)->entries[base], count * sizeof(PALETTEENTRY));
    return DD_OK;
}

static HRESULT STDMETHODCALLTYPE Palette_Initialize(IDirectDrawPalette *self, LPDIRECTDRAW dd, DWORD flags,
    LPPALETTEENTRY entries) {
    (void)self;
    (void)dd;
    (void)flags;
    (void)entries;
    return DDERR_ALREADYINITIALIZED;
}

static HRESULT STDMETHODCALLTYPE Palette_SetEntries(IDirectDrawPalette *self, DWORD flags, DWORD base, DWORD count,
    LPPALETTEENTRY entries) {
    (void)flags;
    if (base > 256 || count > 256 - base) {
        return DDERR_INVALIDPARAMS;
    }
    memcpy(&AsPalette(self)->entries[base], entries, count * sizeof(PALETTEENTRY));
    return DD_OK;
}

static IDirectDrawPaletteVtbl PaletteVtbl = {
    Palette_QueryInterface,
    Palette_AddRef,
    Palette_Release,
    Palette_GetCaps,
    Palette_GetEntries,
    Palette_Initialize,
    Palette_SetEntries,
};

/* ---- IDirectDraw and IDirectDraw2 (one object; the shared work is done here) ---- */

static HRESULT DoCreateSurface(struct DirectDraw *dd, LPDDSURFACEDESC desc, LPDIRECTDRAWSURFACE *out) {
    struct Surface *s;
    DWORD caps;
    int width;
    int height;

    if (out == NULL) {
        return DDERR_INVALIDPARAMS;
    }
    *out = NULL;
    if (desc == NULL || desc->dwSize != sizeof(*desc)) {
        return DDERR_INVALIDPARAMS;
    }
    caps = (desc->dwFlags & DDSD_CAPS) != 0 ? desc->ddsCaps.dwCaps : 0;
    if ((caps & DDSCAPS_PRIMARYSURFACE) != 0) {
        width = (int)dd->width;
        height = (int)dd->height;
    } else if ((desc->dwFlags & (DDSD_WIDTH | DDSD_HEIGHT)) == (DDSD_WIDTH | DDSD_HEIGHT)) {
        width = (int)desc->dwWidth;
        height = (int)desc->dwHeight;
    } else {
        return DDERR_INVALIDPARAMS;
    }
    if (width <= 0 || height <= 0 || width > 16384 || height > 16384) {
        return DDERR_INVALIDPARAMS;
    }
    if ((desc->dwFlags & DDSD_PIXELFORMAT) != 0 &&
        (desc->ddpfPixelFormat.dwRGBBitCount != 16 || desc->ddpfPixelFormat.dwRBitMask != 0xf800 ||
            desc->ddpfPixelFormat.dwGBitMask != 0x07e0 || desc->ddpfPixelFormat.dwBBitMask != 0x001f)) {
        return DDERR_INVALIDPIXELFORMAT; /* only RGB565 */
    }
    s = (struct Surface *)calloc(1, sizeof(*s));
    if (s == NULL) {
        return DDERR_OUTOFMEMORY;
    }
    s->iface.lpVtbl = &SurfaceVtbl;
    s->refs = 1;
    s->caps = (caps & ~(DDSCAPS_VIDEOMEMORY | DDSCAPS_LOCALVIDMEM | DDSCAPS_NONLOCALVIDMEM)) | DDSCAPS_SYSTEMMEMORY;
    s->width = width;
    s->height = height;
    s->pitch = (LONG)(((unsigned)width * 2 + 3) & ~3u); /* DIB rows are 4-byte aligned */
#ifdef _WIN32
    {
        struct {
            BITMAPINFOHEADER header;
            DWORD masks[3];
        } info;
        void *bits = NULL;

        memset(&info, 0, sizeof(info));
        info.header.biSize = sizeof(info.header);
        info.header.biWidth = width;
        info.header.biHeight = -height; /* top-down, like a DirectDraw surface */
        info.header.biPlanes = 1;
        info.header.biBitCount = 16;
        info.header.biCompression = BI_BITFIELDS;
        info.masks[0] = 0xf800;
        info.masks[1] = 0x07e0;
        info.masks[2] = 0x001f;
        s->dib = CreateDIBSection(NULL, (BITMAPINFO *)&info, DIB_RGB_COLORS, &bits, NULL, 0);
        s->bits = (unsigned char *)bits;
    }
#endif
    if (s->bits == NULL) {
        s->bits = (unsigned char *)calloc((size_t)s->pitch, (size_t)height);
        if (s->bits == NULL) {
            free(s);
            return DDERR_OUTOFMEMORY;
        }
    }
    *out = &s->iface;
    return DD_OK;
}

static HRESULT DoCreateClipper(LPDIRECTDRAWCLIPPER *out) {
    struct Clipper *c = (struct Clipper *)calloc(1, sizeof(*c));

    if (c == NULL) {
        *out = NULL;
        return DDERR_OUTOFMEMORY;
    }
    c->iface.lpVtbl = &ClipperVtbl;
    c->refs = 1;
    *out = &c->iface;
    return DD_OK;
}

static HRESULT DoCreatePalette(DWORD flags, LPPALETTEENTRY entries, LPDIRECTDRAWPALETTE *out) {
    struct Palette *p = (struct Palette *)calloc(1, sizeof(*p));

    if (p == NULL) {
        *out = NULL;
        return DDERR_OUTOFMEMORY;
    }
    p->iface.lpVtbl = &PaletteVtbl;
    p->refs = 1;
    p->caps = flags;
    if (entries != NULL) {
        memcpy(p->entries, entries, sizeof(p->entries));
    }
    *out = &p->iface;
    return DD_OK;
}

static HRESULT DoGetCaps(LPDDCAPS hal, LPDDCAPS hel) {
    if (hal != NULL) {
        DWORD size = hal->dwSize < sizeof(DDCAPS) ? hal->dwSize : sizeof(DDCAPS);
        memset(hal, 0, size);
        hal->dwSize = size;
    }
    if (hel != NULL) {
        DWORD size = hel->dwSize < sizeof(DDCAPS) ? hel->dwSize : sizeof(DDCAPS);
        memset(hel, 0, size);
        hel->dwSize = size;
        hel->dwCaps = DDCAPS_BLT | DDCAPS_BLTSTRETCH | DDCAPS_BLTCOLORFILL | DDCAPS_COLORKEY;
        hel->dwCKeyCaps = DDCKEYCAPS_SRCBLT;
    }
    return DD_OK;
}

static HRESULT DoGetDisplayMode(struct DirectDraw *dd, LPDDSURFACEDESC desc) {
    if (desc == NULL || desc->dwSize != sizeof(*desc)) {
        return DDERR_INVALIDPARAMS;
    }
    memset(desc, 0, sizeof(*desc));
    desc->dwSize = sizeof(*desc);
    desc->dwFlags = DDSD_WIDTH | DDSD_HEIGHT | DDSD_PITCH | DDSD_PIXELFORMAT;
    desc->dwWidth = dd->width;
    desc->dwHeight = dd->height;
    desc->lPitch = (LONG)(dd->width * 2);
    FillPixelFormat(&desc->ddpfPixelFormat);
    return DD_OK;
}

static HRESULT DoSetDisplayMode(struct DirectDraw *dd, DWORD width, DWORD height, DWORD bpp) {
    if (bpp != 16 || width == 0 || height == 0) {
        return DDERR_UNSUPPORTEDMODE;
    }
    dd->width = width;
    dd->height = height;
    return DD_OK;
}

static ULONG DoRelease(struct DirectDraw *dd) {
    LONG refs = --dd->refs;

    if (refs == 0) {
        free(dd);
    }
    return (ULONG)refs;
}

static HRESULT DoQueryInterface(struct DirectDraw *dd, REFIID riid, LPVOID *out) {
    if (SameGuid(riid, &IID_IUnknown) || SameGuid(riid, &IID_IDirectDraw)) {
        dd->refs++;
        *out = &dd->v1;
        return S_OK;
    }
    if (SameGuid(riid, &IID_IDirectDraw2)) {
        dd->refs++;
        *out = &dd->v2;
        return S_OK;
    }
    *out = NULL;
    return E_NOINTERFACE;
}

/* The two interfaces' methods, written once with a macro for each: IDirectDraw and IDirectDraw2 differ only in
 * SetDisplayMode's arguments and IDirectDraw2's extra GetAvailableVidMem. */
#define DD_METHODS(P, I, FROM) \
    static HRESULT STDMETHODCALLTYPE P##_QueryInterface(I *self, REFIID riid, LPVOID *out) { \
        return DoQueryInterface(FROM(self), riid, out); \
    } \
    static ULONG STDMETHODCALLTYPE P##_AddRef(I *self) { \
        return (ULONG)++FROM(self)->refs; \
    } \
    static ULONG STDMETHODCALLTYPE P##_Release(I *self) { \
        return DoRelease(FROM(self)); \
    } \
    static HRESULT STDMETHODCALLTYPE P##_Compact(I *self) { \
        (void)self; \
        return DD_OK; \
    } \
    static HRESULT STDMETHODCALLTYPE P##_CreateClipper(I *self, DWORD flags, LPDIRECTDRAWCLIPPER *out, \
        IUnknown *outer) { \
        (void)self; \
        (void)flags; \
        (void)outer; \
        return DoCreateClipper(out); \
    } \
    static HRESULT STDMETHODCALLTYPE P##_CreatePalette(I *self, DWORD flags, LPPALETTEENTRY entries, \
        LPDIRECTDRAWPALETTE *out, IUnknown *outer) { \
        (void)self; \
        (void)outer; \
        return DoCreatePalette(flags, entries, out); \
    } \
    static HRESULT STDMETHODCALLTYPE P##_CreateSurface(I *self, LPDDSURFACEDESC desc, LPDIRECTDRAWSURFACE *out, \
        IUnknown *outer) { \
        (void)outer; \
        return DoCreateSurface(FROM(self), desc, out); \
    } \
    static HRESULT STDMETHODCALLTYPE P##_DuplicateSurface(I *self, LPDIRECTDRAWSURFACE a, LPDIRECTDRAWSURFACE *b) { \
        (void)self; \
        (void)a; \
        (void)b; \
        return DDERR_UNSUPPORTED; \
    } \
    static HRESULT STDMETHODCALLTYPE P##_EnumDisplayModes(I *self, DWORD a, LPDDSURFACEDESC b, LPVOID c, \
        LPDDENUMMODESCALLBACK d) { \
        (void)self; \
        (void)a; \
        (void)b; \
        (void)c; \
        (void)d; \
        return DDERR_UNSUPPORTED; \
    } \
    static HRESULT STDMETHODCALLTYPE P##_EnumSurfaces(I *self, DWORD a, LPDDSURFACEDESC b, LPVOID c, \
        LPDDENUMSURFACESCALLBACK d) { \
        (void)self; \
        (void)a; \
        (void)b; \
        (void)c; \
        (void)d; \
        return DDERR_UNSUPPORTED; \
    } \
    static HRESULT STDMETHODCALLTYPE P##_FlipToGDISurface(I *self) { \
        (void)self; \
        return DD_OK; \
    } \
    static HRESULT STDMETHODCALLTYPE P##_GetCaps(I *self, LPDDCAPS hal, LPDDCAPS hel) { \
        (void)self; \
        return DoGetCaps(hal, hel); \
    } \
    static HRESULT STDMETHODCALLTYPE P##_GetDisplayMode(I *self, LPDDSURFACEDESC desc) { \
        return DoGetDisplayMode(FROM(self), desc); \
    } \
    static HRESULT STDMETHODCALLTYPE P##_GetFourCCCodes(I *self, LPDWORD count, LPDWORD codes) { \
        (void)self; \
        (void)codes; \
        *count = 0; \
        return DD_OK; \
    } \
    static HRESULT STDMETHODCALLTYPE P##_GetGDISurface(I *self, LPDIRECTDRAWSURFACE *out) { \
        (void)self; \
        *out = NULL; \
        return DDERR_NOTFOUND; \
    } \
    static HRESULT STDMETHODCALLTYPE P##_GetMonitorFrequency(I *self, LPDWORD out) { \
        (void)self; \
        *out = 60; \
        return DD_OK; \
    } \
    static HRESULT STDMETHODCALLTYPE P##_GetScanLine(I *self, LPDWORD out) { \
        (void)self; \
        *out = 0; \
        return DDERR_VERTICALBLANKINPROGRESS; \
    } \
    static HRESULT STDMETHODCALLTYPE P##_GetVerticalBlankStatus(I *self, LPBOOL out) { \
        (void)self; \
        *out = TRUE; \
        return DD_OK; \
    } \
    static HRESULT STDMETHODCALLTYPE P##_Initialize(I *self, GUID *guid) { \
        (void)self; \
        (void)guid; \
        return DDERR_ALREADYINITIALIZED; \
    } \
    static HRESULT STDMETHODCALLTYPE P##_RestoreDisplayMode(I *self) { \
        (void)self; \
        return DD_OK; \
    } \
    static HRESULT STDMETHODCALLTYPE P##_SetCooperativeLevel(I *self, HWND hwnd, DWORD flags) { \
        (void)flags; \
        FROM(self)->hwnd = hwnd; \
        return DD_OK; \
    } \
    static HRESULT STDMETHODCALLTYPE P##_WaitForVerticalBlank(I *self, DWORD flags, HANDLE event) { \
        (void)self; \
        (void)flags; \
        (void)event; \
        return DD_OK; \
    }

DD_METHODS(DD1, IDirectDraw, FromV1)
DD_METHODS(DD2, IDirectDraw2, FromV2)

static HRESULT STDMETHODCALLTYPE DD1_SetDisplayMode(IDirectDraw *self, DWORD width, DWORD height, DWORD bpp) {
    return DoSetDisplayMode(FromV1(self), width, height, bpp);
}

static HRESULT STDMETHODCALLTYPE DD2_SetDisplayMode(IDirectDraw2 *self, DWORD width, DWORD height, DWORD bpp,
    DWORD refresh, DWORD flags) {
    (void)refresh;
    (void)flags;
    return DoSetDisplayMode(FromV2(self), width, height, bpp);
}

static HRESULT STDMETHODCALLTYPE DD2_GetAvailableVidMem(IDirectDraw2 *self, LPDDSCAPS caps, LPDWORD total,
    LPDWORD free_bytes) {
    (void)self;
    (void)caps;
    if (total != NULL) {
        *total = 0;
    }
    if (free_bytes != NULL) {
        *free_bytes = 0;
    }
    return DD_OK;
}

#define DD_VTBL_ENTRIES(P) \
    P##_QueryInterface, P##_AddRef, P##_Release, P##_Compact, P##_CreateClipper, P##_CreatePalette, P##_CreateSurface, \
        P##_DuplicateSurface, P##_EnumDisplayModes, P##_EnumSurfaces, P##_FlipToGDISurface, P##_GetCaps, \
        P##_GetDisplayMode, P##_GetFourCCCodes, P##_GetGDISurface, P##_GetMonitorFrequency, P##_GetScanLine, \
        P##_GetVerticalBlankStatus, P##_Initialize, P##_RestoreDisplayMode, P##_SetCooperativeLevel, \
        P##_SetDisplayMode, P##_WaitForVerticalBlank

static IDirectDrawVtbl DirectDrawVtbl = {DD_VTBL_ENTRIES(DD1)};
static IDirectDraw2Vtbl DirectDraw2Vtbl = {DD_VTBL_ENTRIES(DD2), DD2_GetAvailableVidMem};

HRESULT WINAPI MiniwinDirectDrawCreate(GUID *guid, LPDIRECTDRAW *out, IUnknown *outer) {
    struct DirectDraw *dd;

    (void)guid;
    if (out == NULL || outer != NULL) {
        return DDERR_INVALIDPARAMS;
    }
    dd = (struct DirectDraw *)calloc(1, sizeof(*dd));
    if (dd == NULL) {
        *out = NULL;
        return DDERR_OUTOFMEMORY;
    }
    dd->v1.lpVtbl = &DirectDrawVtbl;
    dd->v2.lpVtbl = &DirectDraw2Vtbl;
    dd->refs = 1;
    dd->width = 640;
    dd->height = 480;
    *out = &dd->v1;
    return DD_OK;
}
