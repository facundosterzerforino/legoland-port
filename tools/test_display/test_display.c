/* test_display: checks the scaler in port/port_display.c without starting the game. Builds a 640x480 RGB565
 * frame with a known pattern in a DirectDraw off-screen surface, presents it into off-screen stand-ins for the
 * fullscreen primary surface (4:3 and wider sizes, the whole frame and a part of it) and checks every pixel.
 *
 *   tools/test_display/build.sh && tools/test_display/test_display.exe      (prints FAIL lines, exits 1 on any) */
#include <windows.h>
#include <ddraw.h>
#include <stdint.h>
#include <stdio.h>

#include "port_display.h"

static LPDIRECTDRAW dd;
static int failures;

static LPDIRECTDRAWSURFACE MakeSurface(int w, int h) {
    DDSURFACEDESC desc;
    LPDIRECTDRAWSURFACE s = NULL;

    memset(&desc, 0, sizeof(desc));
    desc.dwSize = sizeof(desc);
    desc.dwFlags = DDSD_CAPS | DDSD_WIDTH | DDSD_HEIGHT | DDSD_PIXELFORMAT;
    desc.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN | DDSCAPS_SYSTEMMEMORY;
    desc.dwWidth = w;
    desc.dwHeight = h;
    desc.ddpfPixelFormat.dwSize = sizeof(desc.ddpfPixelFormat);
    desc.ddpfPixelFormat.dwFlags = DDPF_RGB;
    desc.ddpfPixelFormat.dwRGBBitCount = 16;
    desc.ddpfPixelFormat.dwRBitMask = 0xf800;
    desc.ddpfPixelFormat.dwGBitMask = 0x07e0;
    desc.ddpfPixelFormat.dwBBitMask = 0x001f;
    if (IDirectDraw_CreateSurface(dd, &desc, &s, NULL) != DD_OK) {
        printf("FAIL: CreateSurface %dx%d\n", w, h);
        exit(1);
    }
    return s;
}

static uint16_t Pattern(int x, int y) { return (uint16_t)(x * 7 + y * 131 + 1); }

static void Fill(LPDIRECTDRAWSURFACE s, int w, int h, int pattern, uint16_t value) {
    DDSURFACEDESC d;
    int x, y;
    d.dwSize = sizeof(d);
    IDirectDrawSurface_Lock(s, NULL, &d, DDLOCK_WAIT, NULL);
    for (y = 0; y < h; y++) {
        uint16_t *row = (uint16_t *)((char *)d.lpSurface + y * d.lPitch);
        for (x = 0; x < w; x++) {
            row[x] = pattern ? Pattern(x, y) : value;
        }
    }
    IDirectDrawSurface_Unlock(s, d.lpSurface);
}

/* Checks out against the expected result of presenting src (NULL: all) of the pattern frame onto a surface
 * that held `before` everywhere. */
static void Check(const char *name, LPDIRECTDRAWSURFACE out, int w, int h, const RECT *src, uint16_t before) {
    DDSURFACEDESC d;
    RECT dest;
    RECT part = {0, 0, 640, 480};
    int x, y, bad = 0;

    PortDisplayDestRect(w, h, &dest);
    if (src != NULL) {
        part = *src;
    }
    d.dwSize = sizeof(d);
    IDirectDrawSurface_Lock(out, NULL, &d, DDLOCK_WAIT, NULL);
    for (y = 0; y < h; y++) {
        uint16_t *row = (uint16_t *)((char *)d.lpSurface + y * d.lPitch);
        for (x = 0; x < w; x++) {
            uint16_t want;
            int inside = x >= dest.left && x < dest.right && y >= dest.top && y < dest.bottom;
            int sx = inside ? (x - dest.left) * 640 / (dest.right - dest.left) : -1;
            int sy = inside ? (y - dest.top) * 480 / (dest.bottom - dest.top) : -1;
            if (inside && sx >= part.left && sx < part.right && sy >= part.top && sy < part.bottom) {
                want = Pattern(sx, sy); /* every pixel of the picture, from its nearest source pixel */
            } else if (src == NULL) {
                want = 0; /* black bars */
            } else {
                want = before; /* a partial update leaves the rest alone */
            }
            if (row[x] != want && bad++ < 5) {
                printf("FAIL %s: (%d,%d) = %04x, want %04x\n", name, x, y, row[x], want);
            }
        }
    }
    IDirectDrawSurface_Unlock(out, d.lpSurface);
    printf("%s %s: %dx%d dest (%ld,%ld)-(%ld,%ld)\n", bad ? "FAIL" : "ok  ", name, w, h, dest.left, dest.top,
        dest.right, dest.bottom);
    failures += bad != 0;
}

int main(void) {
    static const int sizes[][2] = {{640, 480}, {800, 600}, {1024, 768}, {1280, 960}, {1400, 1050}, {1920, 1440},
        {1920, 1080}, {1280, 1024}, {2560, 1920}};
    RECT watch = {301, 207, 341, 249};
    LPDIRECTDRAWSURFACE frame;
    int i;

    if (DirectDrawCreate(NULL, &dd, NULL) != DD_OK || IDirectDraw_SetCooperativeLevel(dd, NULL, DDSCL_NORMAL) != DD_OK) {
        printf("FAIL: DirectDraw\n");
        return 1;
    }
    frame = MakeSurface(640, 480);
    Fill(frame, 640, 480, 1, 0);
    for (i = 0; i < (int)(sizeof(sizes) / sizeof(sizes[0])); i++) {
        int w = sizes[i][0], h = sizes[i][1];
        char name[64];
        LPDIRECTDRAWSURFACE out = MakeSurface(w, h);

        PortDisplayWidth = w;
        PortDisplayHeight = h;
        Fill(out, w, h, 0, 0x5555);
        PortPresent(NULL, frame, out, NULL);
        sprintf(name, "frame %dx%d", w, h);
        Check(name, out, w, h, NULL, 0x5555);
        Fill(out, w, h, 0, 0x1234);
        PortPresent(NULL, frame, out, &watch);
        sprintf(name, "watch %dx%d", w, h);
        Check(name, out, w, h, &watch, 0x1234);
        IDirectDrawSurface_Release(out);
    }
    {
        /* a screen in another format (the desktop's 32-bit mode after Alt+Tab) is refused and left alone */
        DDSURFACEDESC desc;
        LPDIRECTDRAWSURFACE out = NULL;
        HRESULT hr;
        memset(&desc, 0, sizeof(desc));
        desc.dwSize = sizeof(desc);
        desc.dwFlags = DDSD_CAPS | DDSD_WIDTH | DDSD_HEIGHT | DDSD_PIXELFORMAT;
        desc.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN | DDSCAPS_SYSTEMMEMORY;
        desc.dwWidth = 1024;
        desc.dwHeight = 768;
        desc.ddpfPixelFormat.dwSize = sizeof(desc.ddpfPixelFormat);
        desc.ddpfPixelFormat.dwFlags = DDPF_RGB;
        desc.ddpfPixelFormat.dwRGBBitCount = 32;
        desc.ddpfPixelFormat.dwRBitMask = 0xff0000;
        desc.ddpfPixelFormat.dwGBitMask = 0xff00;
        desc.ddpfPixelFormat.dwBBitMask = 0xff;
        IDirectDraw_CreateSurface(dd, &desc, &out, NULL);
        PortDisplayWidth = 1024;
        PortDisplayHeight = 768;
        hr = PortPresent(NULL, frame, out, NULL);
        printf("%s 32-bit screen refused (hr %lx)\n", hr == DDERR_WRONGMODE ? "ok  " : "FAIL", hr);
        failures += hr != DDERR_WRONGMODE;
        IDirectDrawSurface_Release(out);
    }
    {
        /* cost of one full present at the largest size (the game presents at most every 28 ms) */
        LPDIRECTDRAWSURFACE out = MakeSurface(2560, 1920);
        LARGE_INTEGER f, t0, t1;
        PortDisplayWidth = 2560;
        PortDisplayHeight = 1920;
        QueryPerformanceFrequency(&f);
        QueryPerformanceCounter(&t0);
        for (i = 0; i < 50; i++) {
            PortPresent(NULL, frame, out, NULL);
        }
        QueryPerformanceCounter(&t1);
        printf("present 2560x1920: %.2f ms\n", (double)(t1.QuadPart - t0.QuadPart) * 1000.0 / f.QuadPart / 50);
        IDirectDrawSurface_Release(out);
    }
    printf(failures ? "%d FAILED\n" : "all ok\n", failures);
    return failures != 0;
}
