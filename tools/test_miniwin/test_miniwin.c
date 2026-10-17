/* test_miniwin: checks miniwin's DirectDraw (miniwin/src/ddraw.c) against Windows' own, without starting the game.
 * Every test runs the same calls on a surface of each, as the game makes them (RGB565 system-memory surfaces,
 * IDirectDraw2), then compares the return codes and every pixel.
 *
 *   tools/test_miniwin/build.sh && tools/test_miniwin/test_miniwin.exe      (prints FAIL lines, exits 1 on any) */
#include <windows.h>
#include <ddraw.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "miniwin/miniwin.h"

enum {
    REAL,
    MINI,
    SIDES
};
static const char *const side_names[SIDES] = {"DirectDraw", "miniwin"};

static LPDIRECTDRAW2 dd[SIDES];
static int failures;
static int tests;

static void Fail(const char *test, const char *what) {
    printf("FAIL %s: %s\n", test, what);
    failures++;
}

static LPDIRECTDRAWSURFACE Make(int side, int w, int h, DWORD caps) {
    DDSURFACEDESC desc;
    LPDIRECTDRAWSURFACE s = NULL;
    HRESULT hr;

    memset(&desc, 0, sizeof(desc));
    desc.dwSize = sizeof(desc);
    desc.dwFlags = DDSD_CAPS | DDSD_WIDTH | DDSD_HEIGHT | DDSD_PIXELFORMAT;
    desc.ddsCaps.dwCaps = caps;
    desc.dwWidth = w;
    desc.dwHeight = h;
    desc.ddpfPixelFormat.dwSize = sizeof(desc.ddpfPixelFormat);
    desc.ddpfPixelFormat.dwFlags = DDPF_RGB;
    desc.ddpfPixelFormat.dwRGBBitCount = 16;
    desc.ddpfPixelFormat.dwRBitMask = 0xf800;
    desc.ddpfPixelFormat.dwGBitMask = 0x07e0;
    desc.ddpfPixelFormat.dwBBitMask = 0x001f;
    hr = IDirectDraw2_CreateSurface(dd[side], &desc, &s, NULL);
    if (hr != DD_OK) {
        printf("FAIL: %s CreateSurface %dx%d caps %lx: %08lx\n", side_names[side], w, h, caps, hr);
        exit(1);
    }
    return s;
}

/* Fills a surface with a pattern from seed; about one pixel in five is key (when key isn't 0). */
static void Pattern(LPDIRECTDRAWSURFACE s, unsigned seed, WORD key) {
    DDSURFACEDESC desc;
    unsigned x;
    unsigned y;
    unsigned v = seed * 2654435761u + 1;

    desc.dwSize = sizeof(desc);
    if (IDirectDrawSurface_Lock(s, NULL, &desc, DDLOCK_WAIT, NULL) != DD_OK) {
        printf("FAIL: Lock for the pattern\n");
        exit(1);
    }
    for (y = 0; y < desc.dwHeight; y++) {
        WORD *row = (WORD *)((char *)desc.lpSurface + y * desc.lPitch);
        for (x = 0; x < desc.dwWidth; x++) {
            v = v * 1103515245u + 12345u;
            row[x] = key != 0 && (v >> 16) % 5 == 0 ? key : (WORD)(v >> 8);
        }
    }
    IDirectDrawSurface_Unlock(s, desc.lpSurface);
}

static void Compare(const char *test, LPDIRECTDRAWSURFACE a, LPDIRECTDRAWSURFACE b) {
    DDSURFACEDESC da;
    DDSURFACEDESC db;
    unsigned x;
    unsigned y;
    int differ = 0;
    char text[160];

    tests++;
    da.dwSize = sizeof(da);
    db.dwSize = sizeof(db);
    if (IDirectDrawSurface_Lock(a, NULL, &da, DDLOCK_WAIT, NULL) != DD_OK ||
        IDirectDrawSurface_Lock(b, NULL, &db, DDLOCK_WAIT, NULL) != DD_OK) {
        Fail(test, "can't lock to compare");
        return;
    }
    if (da.dwWidth != db.dwWidth || da.dwHeight != db.dwHeight) {
        Fail(test, "sizes differ");
    } else {
        for (y = 0; y < da.dwHeight; y++) {
            const WORD *ra = (const WORD *)((const char *)da.lpSurface + y * da.lPitch);
            const WORD *rb = (const WORD *)((const char *)db.lpSurface + y * db.lPitch);
            for (x = 0; x < da.dwWidth; x++) {
                if (ra[x] != rb[x]) {
                    if (differ == 0) {
                        sprintf(text, "first difference at %u,%u: %04x (DirectDraw) vs %04x (miniwin)", x, y, ra[x],
                            rb[x]);
                        Fail(test, text);
                    }
                    differ++;
                }
            }
        }
        if (differ > 1) {
            printf("     %d pixels differ\n", differ);
        }
    }
    IDirectDrawSurface_Unlock(a, da.lpSurface);
    IDirectDrawSurface_Unlock(b, db.lpSurface);
}

static void SameResult(const char *test, HRESULT a, HRESULT b) {
    char text[100];

    tests++;
    if (a != b) {
        sprintf(text, "returned %08lx (DirectDraw) vs %08lx (miniwin)", a, b);
        Fail(test, text);
    }
}

static void SetClip(int side, LPDIRECTDRAWSURFACE s, const RECT *rects, int count) {
    LPDIRECTDRAWCLIPPER clipper;
    struct {
        RGNDATAHEADER rdh;
        RECT r[4];
    } rgn;
    int i;

    IDirectDraw2_CreateClipper(dd[side], 0, &clipper, NULL);
    memset(&rgn, 0, sizeof(rgn));
    rgn.rdh.dwSize = sizeof(rgn.rdh);
    rgn.rdh.iType = RDH_RECTANGLES;
    rgn.rdh.nCount = count;
    rgn.rdh.nRgnSize = count * sizeof(RECT);
    rgn.rdh.rcBound = rects[0];
    for (i = 1; i < count; i++) {
        UnionRect(&rgn.rdh.rcBound, &rgn.rdh.rcBound, &rects[i]);
    }
    memcpy(rgn.r, rects, count * sizeof(RECT));
    IDirectDrawClipper_SetClipList(clipper, (LPRGNDATA)&rgn, 0);
    IDirectDrawSurface_SetClipper(s, clipper);
    IDirectDrawClipper_Release(clipper);
}

/* One Blt test: a 640x480 destination and a w x h source with patterns, a colour key on the source, optionally a
 * clip list on the destination; dst/src rects (NULL: whole), flags. */
struct BltCase {
    const char *name;
    int src_w, src_h;
    RECT dst;
    int use_src_rect;
    RECT src;
    DWORD flags;
    int clip_count;
    RECT clip[2];
};

static void RunBlt(const struct BltCase *c) {
    LPDIRECTDRAWSURFACE dst[SIDES];
    LPDIRECTDRAWSURFACE src[SIDES];
    HRESULT hr[SIDES];
    DDCOLORKEY key = {0xf81f, 0xf81f};
    DDBLTFX fx;
    int side;

    for (side = 0; side < SIDES; side++) {
        RECT d = c->dst;
        RECT s = c->src;
        dst[side] = Make(side, 640, 480, DDSCAPS_OFFSCREENPLAIN | DDSCAPS_SYSTEMMEMORY);
        src[side] = Make(side, c->src_w, c->src_h, DDSCAPS_OFFSCREENPLAIN | DDSCAPS_SYSTEMMEMORY);
        Pattern(dst[side], 1, 0);
        Pattern(src[side], 2, 0xf81f);
        IDirectDrawSurface_SetColorKey(src[side], DDCKEY_SRCBLT, &key);
        if (c->clip_count > 0) {
            SetClip(side, dst[side], c->clip, c->clip_count);
        }
        memset(&fx, 0, sizeof(fx));
        fx.dwSize = sizeof(fx);
        fx.dwFillColor = 0x1234;
        hr[side] = IDirectDrawSurface_Blt(dst[side], &d, (c->flags & DDBLT_COLORFILL) ? NULL : src[side],
            c->use_src_rect ? &s : NULL, c->flags, &fx);
    }
    SameResult(c->name, hr[REAL], hr[MINI]);
    Compare(c->name, dst[REAL], dst[MINI]);
    for (side = 0; side < SIDES; side++) {
        IDirectDrawSurface_Release(dst[side]);
        IDirectDrawSurface_Release(src[side]);
    }
}

static const struct BltCase blt_cases[] = {
    {"copy", 37, 23, {100, 50, 137, 73}, 0, {0}, DDBLT_WAIT, 0, {{0}}},
    {"copy, key", 37, 23, {100, 50, 137, 73}, 0, {0}, DDBLT_WAIT | DDBLT_KEYSRC, 0, {{0}}},
    {"copy part, key", 64, 64, {5, 6, 25, 36}, 1, {10, 20, 30, 50}, DDBLT_WAIT | DDBLT_KEYSRC, 0, {{0}}},
    {"stretch up 37x23 -> 80x50, key", 37, 23, {200, 100, 280, 150}, 0, {0}, DDBLT_WAIT | DDBLT_KEYSRC, 0, {{0}}},
    {"stretch up 37x23 -> 81x47", 37, 23, {200, 100, 281, 147}, 0, {0}, DDBLT_WAIT, 0, {{0}}},
    {"stretch up 36x24 -> 41x50 (even sizes: rows and columns round differently)", 36, 24, {200, 100, 241, 150}, 0,
        {0}, DDBLT_WAIT, 0, {{0}}},
    {"stretch up 30x10 -> 70x33, top cut by the clip list", 30, 10, {100, 100, 170, 133}, 0, {0}, DDBLT_WAIT, 1,
        {{0, 113, 640, 480}}},
    {"stretch down 37x23 -> 20x11, key", 37, 23, {10, 10, 30, 21}, 0, {0}, DDBLT_WAIT | DDBLT_KEYSRC, 0, {{0}}},
    {"stretch down 64x64 -> 17x9", 64, 64, {10, 10, 27, 19}, 0, {0}, DDBLT_WAIT, 0, {{0}}},
    {"stretch 2x 50x40 -> 100x80, key", 50, 40, {300, 300, 400, 380}, 0, {0}, DDBLT_WAIT | DDBLT_KEYSRC, 0, {{0}}},
    {"stretch 1280x960 part -> 41x33 (scaled sprite)", 1280, 960, {320, 200, 361, 233}, 1, {0, 0, 29, 25},
        DDBLT_WAIT | DDBLT_KEYSRC, 0, {{0}}},
    {"stretch, clipped", 37, 23, {200, 100, 280, 150}, 0, {0}, DDBLT_WAIT | DDBLT_KEYSRC, 1,
        {{220, 90, 260, 140}}},
    {"stretch, two clip rects", 37, 23, {200, 100, 280, 150}, 0, {0}, DDBLT_WAIT, 2,
        {{200, 100, 230, 120}, {250, 130, 300, 160}}},
    {"stretch, clip past the edge", 37, 23, {600, 450, 680, 500}, 0, {0}, DDBLT_WAIT | DDBLT_KEYSRC, 1,
        {{0, 0, 640, 480}}},
    {"copy past the edge, no clipper", 37, 23, {620, 470, 657, 493}, 0, {0}, DDBLT_WAIT, 0, {{0}}},
    {"empty destination", 37, 23, {100, 100, 100, 120}, 0, {0}, DDBLT_WAIT, 0, {{0}}},
    {"source rect past its edge", 37, 23, {100, 100, 120, 120}, 1, {30, 10, 50, 30}, DDBLT_WAIT, 0, {{0}}},
    {"fill", 8, 8, {10, 20, 300, 200}, 0, {0}, DDBLT_WAIT | DDBLT_COLORFILL, 0, {{0}}},
    {"fill, clipped", 8, 8, {10, 20, 300, 200}, 0, {0}, DDBLT_WAIT | DDBLT_COLORFILL, 1, {{50, 0, 640, 100}}},
};

/* The game's clipper in a window (InitScreen: SetHWnd, no clip list until the map's first render gives it one) against
 * the full-screen game's (a clip list of the screen, here the 640x340 destination's part of it): a keyed stretch out
 * of the scaled-sprite scratch surface, partly outside the destination, as the map draws its scenery (RenderScaledSpriteOffset).
 * DirectDraw clips the full-screen one to its list; miniwin must clip the window one the same, not refuse it. */
static void RunWindowClipper(void) {
    static const RECT dsts[] = {{300, -20, 341, 13}, {100, 320, 141, 353}, {-15, 100, 26, 133}, {620, 200, 661, 233}};
    static const char *const edges[] = {"top", "bottom", "left", "right"};
    LPDIRECTDRAWSURFACE dst[SIDES];
    LPDIRECTDRAWSURFACE src[SIDES];
    HRESULT hr[SIDES];
    DDCOLORKEY key = {0xf81f, 0xf81f};
    RECT screen = {0, 0, 640, 340};
    char name[100];
    unsigned i;
    int side;

    for (i = 0; i < sizeof(dsts) / sizeof(dsts[0]); i++) {
        for (side = 0; side < SIDES; side++) {
            RECT d = dsts[i];
            RECT s = {0, 0, 29, 25};

            dst[side] = Make(side, 640, 340, DDSCAPS_OFFSCREENPLAIN | DDSCAPS_SYSTEMMEMORY);
            src[side] = Make(side, 1280, 960, DDSCAPS_OFFSCREENPLAIN | DDSCAPS_SYSTEMMEMORY);
            Pattern(dst[side], 5, 0);
            Pattern(src[side], 6, 0xf81f);
            IDirectDrawSurface_SetColorKey(src[side], DDCKEY_SRCBLT, &key);
            if (side == REAL) {
                SetClip(side, dst[side], &screen, 1);
            } else {
                LPDIRECTDRAWCLIPPER clipper;

                IDirectDraw2_CreateClipper(dd[side], 0, &clipper, NULL);
                IDirectDrawClipper_SetHWnd(clipper, 0, GetDesktopWindow());
                IDirectDrawSurface_SetClipper(dst[side], clipper);
                IDirectDrawClipper_Release(clipper);
            }
            hr[side] = IDirectDrawSurface_Blt(dst[side], &d, src[side], &s, DDBLT_WAIT | DDBLT_KEYSRC, NULL);
        }
        sprintf(name, "window clipper: stretch past the %s edge", edges[i]);
        SameResult(name, hr[REAL], hr[MINI]);
        Compare(name, dst[REAL], dst[MINI]);
        for (side = 0; side < SIDES; side++) {
            IDirectDrawSurface_Release(dst[side]);
            IDirectDrawSurface_Release(src[side]);
        }
    }
}

/* Blt within one surface, overlapping. */
static void RunOverlap(void) {
    LPDIRECTDRAWSURFACE s[SIDES];
    HRESULT hr[SIDES];
    int side;

    for (side = 0; side < SIDES; side++) {
        RECT d = {10, 10, 110, 90};
        RECT r = {20, 15, 120, 95};
        s[side] = Make(side, 200, 150, DDSCAPS_OFFSCREENPLAIN | DDSCAPS_SYSTEMMEMORY);
        Pattern(s[side], 3, 0);
        hr[side] = IDirectDrawSurface_Blt(s[side], &d, s[side], &r, DDBLT_WAIT, NULL);
    }
    SameResult("overlapping copy in one surface", hr[REAL], hr[MINI]);
    Compare("overlapping copy in one surface", s[REAL], s[MINI]);
    for (side = 0; side < SIDES; side++) {
        IDirectDrawSurface_Release(s[side]);
    }
}

/* GDI on the surface (GetDC), the way text.c draws: a font, transparent background, coloured text, DrawText. */
static void RunGdi(void) {
    LPDIRECTDRAWSURFACE s[SIDES];
    HRESULT hr[SIDES];
    LOGFONTA lf;
    HFONT font;
    int side;

    memset(&lf, 0, sizeof(lf));
    lf.lfHeight = 0x18;
    lf.lfWeight = 0x2bc;
    lf.lfCharSet = 1;
    lf.lfQuality = 2;
    strcpy(lf.lfFaceName, "Arial");
    font = CreateFontIndirectA(&lf);
    for (side = 0; side < SIDES; side++) {
        HDC dc;
        RECT box = {20, 30, 300, 200};
        s[side] = Make(side, 640, 480, DDSCAPS_OFFSCREENPLAIN | DDSCAPS_SYSTEMMEMORY);
        Pattern(s[side], 4, 0);
        hr[side] = IDirectDrawSurface_GetDC(s[side], &dc);
        if (hr[side] == DD_OK) {
            HGDIOBJ old = SelectObject(dc, font);
            SetBkMode(dc, TRANSPARENT);
            SetTextColor(dc, RGB(255, 255, 0));
            TextOutA(dc, 10, 5, "LEGOLAND 0123 The quick brown fox", 33);
            SetTextColor(dc, RGB(10, 200, 90));
            DrawTextA(dc, "Build a park that visitors love. Wrapped text, centred.", -1, &box,
                DT_WORDBREAK | DT_CENTER);
            SetBkMode(dc, OPAQUE);
            SetBkColor(dc, RGB(0, 0, 128));
            TextOutA(dc, 10, 300, "opaque background", 17);
            SelectObject(dc, old);
            IDirectDrawSurface_ReleaseDC(s[side], dc);
        }
    }
    SameResult("GetDC", hr[REAL], hr[MINI]);
    Compare("GDI text through GetDC", s[REAL], s[MINI]);
    for (side = 0; side < SIDES; side++) {
        IDirectDrawSurface_Release(s[side]);
    }
    DeleteObject(font);
}

/* Lock of a part, and the description the game reads back. */
static void RunLock(void) {
    int side;
    DDSURFACEDESC desc[SIDES];
    void *base[SIDES];

    for (side = 0; side < SIDES; side++) {
        LPDIRECTDRAWSURFACE s = Make(side, 640, 480, DDSCAPS_OFFSCREENPLAIN | DDSCAPS_SYSTEMMEMORY);
        RECT r = {5, 7, 50, 60};
        DDSURFACEDESC whole;

        whole.dwSize = sizeof(whole);
        IDirectDrawSurface_Lock(s, NULL, &whole, DDLOCK_WAIT, NULL);
        base[side] = whole.lpSurface;
        IDirectDrawSurface_Unlock(s, whole.lpSurface);
        desc[side].dwSize = sizeof(desc[side]);
        IDirectDrawSurface_Lock(s, &r, &desc[side], DDLOCK_WAIT, NULL);
        IDirectDrawSurface_Unlock(s, desc[side].lpSurface);
        IDirectDrawSurface_Release(s);
    }
    tests++;
    for (side = 0; side < SIDES; side++) {
        long offset = (long)((char *)desc[side].lpSurface - (char *)base[side]);
        if (offset != 7 * desc[side].lPitch + 5 * 2) {
            printf("FAIL Lock of a part: %s's pointer is %ld bytes in, not row 7 column 5\n", side_names[side],
                offset);
            failures++;
        }
    }
    tests++;
    if (desc[REAL].dwWidth != desc[MINI].dwWidth || desc[REAL].dwHeight != desc[MINI].dwHeight ||
        desc[REAL].ddpfPixelFormat.dwRGBBitCount != desc[MINI].ddpfPixelFormat.dwRGBBitCount ||
        desc[REAL].ddpfPixelFormat.dwGBitMask != desc[MINI].ddpfPixelFormat.dwGBitMask) {
        printf("FAIL Lock description: %lux%lu %lu bpp G %lx vs %lux%lu %lu bpp G %lx\n", desc[REAL].dwWidth,
            desc[REAL].dwHeight, desc[REAL].ddpfPixelFormat.dwRGBBitCount, desc[REAL].ddpfPixelFormat.dwGBitMask,
            desc[MINI].dwWidth, desc[MINI].dwHeight, desc[MINI].ddpfPixelFormat.dwRGBBitCount,
            desc[MINI].ddpfPixelFormat.dwGBitMask);
        failures++;
    }
}

int main(void) {
    LPDIRECTDRAW dd1[SIDES];
    unsigned i;

    if (DirectDrawCreate(NULL, &dd1[REAL], NULL) != DD_OK || MiniwinDirectDrawCreate(NULL, &dd1[MINI], NULL) != DD_OK) {
        printf("FAIL: DirectDrawCreate\n");
        return 1;
    }
    for (i = 0; i < SIDES; i++) {
        if (IDirectDraw_QueryInterface(dd1[i], &IID_IDirectDraw2, (void **)&dd[i]) != DD_OK) {
            printf("FAIL: %s QueryInterface IDirectDraw2\n", side_names[i]);
            return 1;
        }
        IDirectDraw2_SetCooperativeLevel(dd[i], NULL, DDSCL_NORMAL);
    }
    for (i = 0; i < sizeof(blt_cases) / sizeof(blt_cases[0]); i++) {
        RunBlt(&blt_cases[i]);
    }
    RunWindowClipper();
    RunOverlap();
    RunGdi();
    RunLock();
    for (i = 0; i < SIDES; i++) {
        IDirectDraw2_Release(dd[i]);
        IDirectDraw_Release(dd1[i]);
    }
    printf("%d checks, %d failed\n", tests, failures);
    return failures != 0;
}
