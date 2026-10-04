/* test_display: checks the scaler in port/port_display.c without starting the game. Builds a 640x480 RGB565
 * frame with a known pattern in a DirectDraw off-screen surface, presents it into off-screen stand-ins for the
 * fullscreen primary surface (4:3 and wider sizes, the whole frame and a part of it, 16-bit and 32-bit screens)
 * and checks every pixel.
 *
 *   tools/test_display/build.sh && tools/test_display/test_display.exe      (prints FAIL lines, exits 1 on any) */
#include <windows.h>
#include <ddraw.h>
#include <stdint.h>
#include <stdio.h>

#include "port_display.h"

static LPDIRECTDRAW dd;
static int failures;

static LPDIRECTDRAWSURFACE MakeSurfaceFormat(int w, int h, int bits, DWORD r, DWORD g, DWORD b) {
    DDSURFACEDESC desc;
    LPDIRECTDRAWSURFACE s = NULL;

    memset(&desc, 0, sizeof(desc));
    desc.dwSize = sizeof(desc);
    desc.dwFlags = DDSD_CAPS | DDSD_WIDTH | DDSD_HEIGHT | DDSD_PIXELFORMAT;
    desc.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN | DDSCAPS_SYSTEMMEMORY;
    desc.dwWidth = w;
    desc.dwHeight = h;
    desc.ddpfPixelFormat.dwSize = sizeof(desc.ddpfPixelFormat);
    desc.ddpfPixelFormat.dwFlags = bits == 8 ? DDPF_RGB | DDPF_PALETTEINDEXED8 : DDPF_RGB;
    desc.ddpfPixelFormat.dwRGBBitCount = bits;
    desc.ddpfPixelFormat.dwRBitMask = r;
    desc.ddpfPixelFormat.dwGBitMask = g;
    desc.ddpfPixelFormat.dwBBitMask = b;
    if (IDirectDraw_CreateSurface(dd, &desc, &s, NULL) != DD_OK) {
        printf("FAIL: CreateSurface %dx%d %d-bit\n", w, h, bits);
        exit(1);
    }
    return s;
}

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
        /* windowed / desktop-resolution full screen: the picture is scaled by the same code into a 32-bit image the
         * size of the client area (a window that is never shown), the same every time (GDI's own stretching mapped
         * some rows and columns differently from one present to the next: flicker) */
        static const int clients[][2] = {{1920, 1200}, {1400, 1050}, {1152, 864}, {1000, 900}};
        WNDCLASSA wc;
        int k;
        memset(&wc, 0, sizeof(wc));
        wc.lpfnWndProc = DefWindowProcA;
        wc.hInstance = GetModuleHandleA(NULL);
        wc.lpszClassName = "TestDisplay";
        RegisterClassA(&wc);
        PortDisplayWindowed = 1;
        for (k = 0; k < 4; k++) {
            int cw = clients[k][0], ch = clients[k][1], w, h, x, y, bad = 0, pass;
            HWND hwnd = CreateWindowExA(0, "TestDisplay", "", WS_POPUP, 0, 0, cw, ch, NULL, NULL, wc.hInstance, NULL);
            RECT dest;
            const unsigned int *img;
            PortDisplayDestRect(cw, ch, &dest);
            for (pass = 0; pass < 3; pass++) {
                PortPresent(hwnd, frame, NULL, pass == 2 ? &watch : NULL);
                img = PortDisplayWindowImage(&w, &h);
                if (img == NULL || w != cw || h != ch) {
                    printf("FAIL window %dx%d: no image\n", cw, ch);
                    bad++;
                    break;
                }
                for (y = 0; y < ch; y++) {
                    for (x = 0; x < cw; x++) {
                        unsigned want = 0;
                        if (x >= dest.left && x < dest.right && y >= dest.top && y < dest.bottom) {
                            unsigned v = Pattern((x - dest.left) * 640 / (dest.right - dest.left),
                                (y - dest.top) * 480 / (dest.bottom - dest.top));
                            unsigned r = (v >> 11) & 31, g = (v >> 5) & 63, b = v & 31;
                            want = (((r << 3) | (r >> 2)) << 16) | (((g << 2) | (g >> 4)) << 8) | ((b << 3) | (b >> 2));
                        }
                        if ((img[y * cw + x] & 0xffffff) != want && bad++ < 3) {
                            printf("FAIL window %dx%d pass %d: (%d,%d) = %06x, want %06x\n", cw, ch, pass, x, y,
                                img[y * cw + x] & 0xffffff, want);
                        }
                    }
                }
            }
            printf("%s window %dx%d: picture (%ld,%ld)-(%ld,%ld), same in 3 presents\n", bad ? "FAIL" : "ok  ", cw, ch,
                dest.left, dest.top, dest.right, dest.bottom);
            if (k == 0) {
                /* cost of one windowed present at full-screen size (the game presents at most every 28 ms) */
                LARGE_INTEGER f, t0, t1;
                QueryPerformanceFrequency(&f);
                QueryPerformanceCounter(&t0);
                for (pass = 0; pass < 50; pass++) {
                    PortPresent(hwnd, frame, NULL, NULL);
                }
                QueryPerformanceCounter(&t1);
                printf("present window %dx%d: %.2f ms\n", cw, ch,
                    (double)(t1.QuadPart - t0.QuadPart) * 1000.0 / f.QuadPart / 50);
            }
            failures += bad != 0;
            DestroyWindow(hwnd);
        }
        PortDisplayWindowed = 0;
    }
    {
        /* a 32-bit screen: every pixel converted from RGB565 (channels widened by repeating their top bits) */
        LPDIRECTDRAWSURFACE out = MakeSurfaceFormat(1024, 768, 32, 0xff0000, 0xff00, 0xff);
        DDSURFACEDESC d;
        RECT dest;
        int x, y, bad = 0;
        HRESULT hr;
        PortDisplayWidth = 1024;
        PortDisplayHeight = 768;
        hr = PortPresent(NULL, frame, out, NULL);
        PortDisplayDestRect(1024, 768, &dest);
        d.dwSize = sizeof(d);
        IDirectDrawSurface_Lock(out, NULL, &d, DDLOCK_WAIT, NULL);
        for (y = 0; y < 768; y++) {
            uint32_t *row = (uint32_t *)((char *)d.lpSurface + y * d.lPitch);
            for (x = 0; x < 1024; x++) {
                unsigned v = Pattern(x * 640 / 1024, y * 480 / 768);
                unsigned r = (v >> 11) & 31, g = (v >> 5) & 63, b = v & 31;
                uint32_t want = (((r << 3) | (r >> 2)) << 16) | (((g << 2) | (g >> 4)) << 8) | ((b << 3) | (b >> 2));
                if ((row[x] & 0xffffff) != want && bad++ < 5) {
                    printf("FAIL 32-bit: (%d,%d) = %06lx, want %06lx\n", x, y, (unsigned long)row[x], (unsigned long)want);
                }
            }
        }
        IDirectDrawSurface_Unlock(out, d.lpSurface);
        printf("%s 32-bit screen converted (hr %lx)\n", hr == DD_OK && !bad ? "ok  " : "FAIL", hr);
        failures += hr != DD_OK || bad;
        IDirectDrawSurface_Release(out);
    }
    {
        /* white and black come out as pure white and black */
        LPDIRECTDRAWSURFACE tiny_frame = MakeSurface(640, 480);
        LPDIRECTDRAWSURFACE out = MakeSurfaceFormat(640, 480, 32, 0xff0000, 0xff00, 0xff);
        DDSURFACEDESC d;
        uint32_t white, black;
        Fill(tiny_frame, 640, 480, 0, 0xffff);
        {
            DDSURFACEDESC f;
            f.dwSize = sizeof(f);
            IDirectDrawSurface_Lock(tiny_frame, NULL, &f, DDLOCK_WAIT, NULL);
            ((uint16_t *)f.lpSurface)[1] = 0;
            IDirectDrawSurface_Unlock(tiny_frame, f.lpSurface);
        }
        PortDisplayWidth = 640;
        PortDisplayHeight = 480;
        PortPresent(NULL, tiny_frame, out, NULL);
        d.dwSize = sizeof(d);
        IDirectDrawSurface_Lock(out, NULL, &d, DDLOCK_WAIT, NULL);
        white = ((uint32_t *)d.lpSurface)[0] & 0xffffff;
        black = ((uint32_t *)d.lpSurface)[1] & 0xffffff;
        IDirectDrawSurface_Unlock(out, d.lpSurface);
        printf("%s white %06lx, black %06lx\n", white == 0xffffff && black == 0 ? "ok  " : "FAIL", (unsigned long)white,
            (unsigned long)black);
        failures += white != 0xffffff || black != 0;
        IDirectDrawSurface_Release(out);
        IDirectDrawSurface_Release(tiny_frame);
    }
    {
        /* a screen the frame can't be copied or converted to (8-bit) is refused and left alone */
        LPDIRECTDRAWSURFACE out = MakeSurfaceFormat(1024, 768, 8, 0, 0, 0);
        HRESULT hr;
        PortDisplayWidth = 1024;
        PortDisplayHeight = 768;
        hr = PortPresent(NULL, frame, out, NULL);
        printf("%s 8-bit screen refused (hr %lx)\n", hr == DDERR_WRONGMODE ? "ok  " : "FAIL", hr);
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
    {
        /* the surfaces InitScreen creates with the forced RGB565 format (windowed, and full screen in a 32-bit
         * mode): DirectDraw must accept each caps/format pair (it refused 0x800 alone) */
        static const DWORD caps[] = {0x40, 0x840, 0x4000 | 0x40, 0x40 | 0x800};
        static const char *const what[] = {"windowed frame / sprites", "32-bit full screen frame / extra surface",
            "video-memory off-screen", "sprites (system memory retry)"};
        int k;
        for (k = 0; k < 4; k++) {
            DDSURFACEDESC desc;
            LPDIRECTDRAWSURFACE surf = NULL;
            HRESULT hr;
            memset(&desc, 0, sizeof(desc));
            desc.dwSize = sizeof(desc);
            desc.dwFlags = DDSD_CAPS | DDSD_WIDTH | DDSD_HEIGHT | DDSD_PIXELFORMAT;
            desc.ddsCaps.dwCaps = caps[k];
            desc.dwWidth = 640;
            desc.dwHeight = 480;
            desc.ddpfPixelFormat.dwSize = sizeof(desc.ddpfPixelFormat);
            desc.ddpfPixelFormat.dwFlags = DDPF_RGB;
            desc.ddpfPixelFormat.dwRGBBitCount = 16;
            desc.ddpfPixelFormat.dwRBitMask = 0xf800;
            desc.ddpfPixelFormat.dwGBitMask = 0x07e0;
            desc.ddpfPixelFormat.dwBBitMask = 0x001f;
            hr = IDirectDraw_CreateSurface(dd, &desc, &surf, NULL);
            printf("%s RGB565 surface caps %04lx (%s): hr %lx\n", hr == DD_OK ? "ok  " : "FAIL", caps[k], what[k], hr);
            failures += hr != DD_OK;
            if (surf != NULL) {
                IDirectDrawSurface_Release(surf);
            }
        }
    }
    printf(failures ? "%d FAILED\n" : "all ok\n", failures);
    return failures != 0;
}
