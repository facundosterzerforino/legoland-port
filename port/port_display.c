/* [library:video] Output resolution: the 640x480 frame shown scaled. See port_display.h. */
#include <windows.h>
#include <ddraw.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "port_display.h"
#include "port_trace.h"

int PortDisplayWidth = PORT_GAME_WIDTH;
int PortDisplayHeight = PORT_GAME_HEIGHT;
int PortDisplayWindowed = 0;
int PortDisplayBorderless = 0;
int PortDisplayBpp = 16;

/* RGB565 -> the 32-bit screen's format, one entry per 16-bit colour (built for the screen's channel masks) */
static uint32_t *rgb565_to_32;
static DWORD table_masks[3];

/* Source column for each destination column of the current fullscreen destination (rebuilt when it changes). */
static int column_map[4096];
static int column_map_width;

int PortDisplayScaled(void) {
    return PortDisplayWidth != PORT_GAME_WIDTH || PortDisplayHeight != PORT_GAME_HEIGHT;
}

int PortDisplayCustomPresent(void) {
    return PortDisplayWindowed || PortDisplayScaled() || PortDisplayBpp == 32;
}

static int LowestBit(DWORD mask) {
    int shift = 0;
    while (shift < 32 && (mask & (1u << shift)) == 0) {
        shift++;
    }
    return shift;
}

/* The table for a 32-bit screen with 8-bit channels at these masks; 0 if the format is anything else. Each 5- or
 * 6-bit channel is widened by repeating its top bits (31 -> 255, 0 -> 0), as a 16-bit display shows it. */
static int Build565Table(const DDPIXELFORMAT *pf) {
    int rs = LowestBit(pf->dwRBitMask);
    int gs = LowestBit(pf->dwGBitMask);
    int bs = LowestBit(pf->dwBBitMask);
    unsigned v;

    if (pf->dwRGBBitCount != 32 || pf->dwRBitMask != (0xffu << rs) || pf->dwGBitMask != (0xffu << gs) ||
        pf->dwBBitMask != (0xffu << bs)) {
        return 0;
    }
    if (rgb565_to_32 != NULL && table_masks[0] == pf->dwRBitMask && table_masks[1] == pf->dwGBitMask &&
        table_masks[2] == pf->dwBBitMask) {
        return 1;
    }
    if (rgb565_to_32 == NULL) {
        rgb565_to_32 = (uint32_t *)malloc(0x10000 * sizeof(uint32_t));
        if (rgb565_to_32 == NULL) {
            return 0;
        }
    }
    for (v = 0; v < 0x10000; v++) {
        unsigned r = (v >> 11) & 0x1f;
        unsigned g = (v >> 5) & 0x3f;
        unsigned b = v & 0x1f;
        r = (r << 3) | (r >> 2);
        g = (g << 2) | (g >> 4);
        b = (b << 3) | (b >> 2);
        rgb565_to_32[v] = (uint32_t)((r << rs) | (g << gs) | (b << bs));
    }
    table_masks[0] = pf->dwRBitMask;
    table_masks[1] = pf->dwGBitMask;
    table_masks[2] = pf->dwBBitMask;
    return 1;
}

void PortDisplayDestRect(int area_width, int area_height, RECT *dest) {
    int width = area_width;
    int height = area_height;

    if (width * 3 > height * 4) {
        width = height * 4 / 3;
    } else {
        height = width * 3 / 4;
    }
    dest->left = (area_width - width) / 2;
    dest->top = (area_height - height) / 2;
    dest->right = dest->left + width;
    dest->bottom = dest->top + height;
}

/* The destination pixels covering src (game coordinates) inside dest. */
static void MapRect(const RECT *dest, const RECT *src, RECT *out) {
    int dw = dest->right - dest->left;
    int dh = dest->bottom - dest->top;

    /* the first destination pixel whose source (x * 640 / dw) is >= src->left, and the same for the end */
    out->left = dest->left + (src->left * dw + PORT_GAME_WIDTH - 1) / PORT_GAME_WIDTH;
    out->top = dest->top + (src->top * dh + PORT_GAME_HEIGHT - 1) / PORT_GAME_HEIGHT;
    out->right = dest->left + (src->right * dw + PORT_GAME_WIDTH - 1) / PORT_GAME_WIDTH;
    out->bottom = dest->top + (src->bottom * dh + PORT_GAME_HEIGHT - 1) / PORT_GAME_HEIGHT;
}

static void ClampToFrame(const RECT *src, RECT *out) {
    RECT whole = {0, 0, PORT_GAME_WIDTH, PORT_GAME_HEIGHT};

    if (src == NULL || !IntersectRect(out, src, &whole)) {
        *out = whole;
    }
}

static void FillRows(unsigned char *base, LONG pitch, int bytes, int x0, int x1, int y0, int y1) {
    int y;

    for (y = y0; y < y1; y++) {
        memset(base + (size_t)y * pitch + (size_t)x0 * bytes, 0, (size_t)(x1 - x0) * bytes);
    }
}

/* Scales the part src (game coordinates; NULL: all, with black bars around the picture) of the locked frame fd
 * into a destination of dwidth x dheight pixels: nearest-neighbour, either copied (bytes 1 or 2, same format) or
 * converted from RGB565 to 32-bit (convert, through rgb565_to_32). The same code for every mode, so a pixel always
 * comes from the same source pixel. */
static void Scale(const DDSURFACEDESC *fd, unsigned char *dst, LONG dpitch, int dwidth, int dheight, int bytes,
    int convert, const RECT *src) {
    RECT dest;
    RECT area;
    RECT part;
    int dw;
    int dh;
    int x;
    int y;

    PortDisplayDestRect(dwidth, dheight, &dest);
    dw = dest.right - dest.left;
    dh = dest.bottom - dest.top;
    if (dw > (int)(sizeof(column_map) / sizeof(column_map[0]))) {
        dw = sizeof(column_map) / sizeof(column_map[0]);
        dest.right = dest.left + dw;
    }
    if (dw <= 0 || dh <= 0) {
        return;
    }
    if (column_map_width != dw) {
        for (x = 0; x < dw; x++) {
            column_map[x] = x * PORT_GAME_WIDTH / dw;
        }
        column_map_width = dw;
    }
    ClampToFrame(src, &area);
    MapRect(&dest, &area, &part);
    if (src == NULL) {
        /* black bars around a 4:3 picture on a wider (or taller) screen or window */
        FillRows(dst, dpitch, bytes, 0, dwidth, 0, dest.top);
        FillRows(dst, dpitch, bytes, 0, dwidth, dest.bottom, dheight);
        FillRows(dst, dpitch, bytes, 0, dest.left, dest.top, dest.bottom);
        FillRows(dst, dpitch, bytes, dest.right, dwidth, dest.top, dest.bottom);
    }
    for (y = part.top; y < part.bottom; y++) {
        int sy = (y - dest.top) * PORT_GAME_HEIGHT / dh;
        const unsigned char *srow = (const unsigned char *)fd->lpSurface + (size_t)sy * fd->lPitch;
        unsigned char *drow = dst + (size_t)y * dpitch;

        if (convert) {
            const uint16_t *s = (const uint16_t *)srow;
            uint32_t *d = (uint32_t *)drow;
            for (x = part.left; x < part.right; x++) {
                d[x] = rgb565_to_32[s[column_map[x - dest.left]]];
            }
        } else if (bytes == 2) {
            const uint16_t *s = (const uint16_t *)srow;
            uint16_t *d = (uint16_t *)drow;
            for (x = part.left; x < part.right; x++) {
                d[x] = s[column_map[x - dest.left]];
            }
        } else {
            for (x = part.left; x < part.right; x++) {
                drow[x] = srow[column_map[x - dest.left]];
            }
        }
    }
}

/* Fullscreen: the frame scaled into the primary surface, either in the same format (a 16- or 8-bit display mode)
 * or from the RGB565 frame to a 32-bit display mode. */
static HRESULT PresentFullscreen(LPDIRECTDRAWSURFACE frame, LPDIRECTDRAWSURFACE primary, const RECT *src) {
    DDSURFACEDESC fd;
    DDSURFACEDESC pd;
    HRESULT hr;
    int bytes;
    int convert;

    fd.dwSize = sizeof(fd);
    hr = IDirectDrawSurface_Lock(frame, NULL, &fd, DDLOCK_WAIT | DDLOCK_READONLY, NULL);
    if (hr != DD_OK) {
        return hr;
    }
    pd.dwSize = sizeof(pd);
    hr = IDirectDrawSurface_Lock(primary, NULL, &pd, DDLOCK_WAIT | DDLOCK_WRITEONLY, NULL);
    if (hr != DD_OK) {
        IDirectDrawSurface_Unlock(frame, fd.lpSurface);
        return hr;
    }
    bytes = (int)(pd.ddpfPixelFormat.dwRGBBitCount / 8);
    convert = fd.ddpfPixelFormat.dwRGBBitCount == 16 && fd.ddpfPixelFormat.dwGBitMask == 0x07e0 &&
        Build565Table(&pd.ddpfPixelFormat);
    if (!convert &&
        (pd.ddpfPixelFormat.dwRGBBitCount != fd.ddpfPixelFormat.dwRGBBitCount || (bytes != 1 && bytes != 2))) {
        /* the screen is in a format the frame can't be copied or converted to (e.g. a 16-bit game on the
         * desktop's 32-bit mode after Alt+Tab): the caller sets the mode again (KeepDisplayMode in draw.c) */
        IDirectDrawSurface_Unlock(primary, pd.lpSurface);
        IDirectDrawSurface_Unlock(frame, fd.lpSurface);
        return DDERR_WRONGMODE;
    }
    Scale(&fd, (unsigned char *)pd.lpSurface, pd.lPitch, (int)pd.dwWidth, (int)pd.dwHeight, bytes, convert, src);
    IDirectDrawSurface_Unlock(primary, pd.lpSurface);
    IDirectDrawSurface_Unlock(frame, fd.lpSurface);
    return DD_OK;
}

/* Windowed (and full screen at the desktop's resolution): the frame is scaled and converted into this 32-bit
 * bitmap of the client area's size, which GDI then copies 1:1. Letting GDI stretch (StretchDIBits) flickered: from
 * one call to the next it mapped some screen rows and columns to a neighbouring source pixel. */
static HDC window_dc;
static HBITMAP window_bitmap;
static HGDIOBJ window_old_bitmap;
static uint32_t *window_bits;
static int window_width;
static int window_height;

static int WindowImage(int width, int height) {
    BITMAPINFO info;
    void *bits = NULL;

    if (window_bits != NULL && window_width == width && window_height == height) {
        return 1;
    }
    if (window_dc != NULL) {
        SelectObject(window_dc, window_old_bitmap);
        DeleteObject(window_bitmap);
        DeleteDC(window_dc);
        window_dc = NULL;
        window_bits = NULL;
    }
    memset(&info, 0, sizeof(info));
    info.bmiHeader.biSize = sizeof(info.bmiHeader);
    info.bmiHeader.biWidth = width;
    info.bmiHeader.biHeight = -height; /* top-down */
    info.bmiHeader.biPlanes = 1;
    info.bmiHeader.biBitCount = 32;
    window_dc = CreateCompatibleDC(NULL);
    window_bitmap = CreateDIBSection(window_dc, &info, DIB_RGB_COLORS, &bits, NULL, 0);
    if (window_dc == NULL || window_bitmap == NULL || bits == NULL) {
        if (window_bitmap != NULL) {
            DeleteObject(window_bitmap);
        }
        if (window_dc != NULL) {
            DeleteDC(window_dc);
        }
        window_dc = NULL;
        return 0;
    }
    window_old_bitmap = SelectObject(window_dc, window_bitmap);
    window_bits = (uint32_t *)bits;
    window_width = width;
    window_height = height;
    return 1;
}

static HRESULT PresentWindowed(HWND hwnd, LPDIRECTDRAWSURFACE frame, const RECT *src) {
    DDPIXELFORMAT xrgb;
    DDSURFACEDESC desc;
    RECT client;
    RECT dest;
    RECT area;
    RECT part;
    HRESULT hr;
    HDC dc;

    memset(&xrgb, 0, sizeof(xrgb));
    xrgb.dwSize = sizeof(xrgb);
    xrgb.dwFlags = DDPF_RGB;
    xrgb.dwRGBBitCount = 32;
    xrgb.dwRBitMask = 0xff0000;
    xrgb.dwGBitMask = 0xff00;
    xrgb.dwBBitMask = 0xff;
    GetClientRect(hwnd, &client);
    if (client.right <= 0 || client.bottom <= 0 || !WindowImage(client.right, client.bottom) ||
        !Build565Table(&xrgb)) {
        return DD_OK; /* minimized */
    }
    desc.dwSize = sizeof(desc);
    hr = IDirectDrawSurface_Lock(frame, NULL, &desc, DDLOCK_WAIT | DDLOCK_READONLY, NULL);
    if (hr == DDERR_SURFACELOST) {
        IDirectDrawSurface_Restore(frame);
        hr = IDirectDrawSurface_Lock(frame, NULL, &desc, DDLOCK_WAIT | DDLOCK_READONLY, NULL);
    }
    if (hr != DD_OK) {
        return hr;
    }
    Scale(&desc, (unsigned char *)window_bits, (LONG)window_width * 4, window_width, window_height, 4, 1, src);
    IDirectDrawSurface_Unlock(frame, desc.lpSurface);
    GdiFlush();
    if (src == NULL) {
        part = client;
    } else {
        PortDisplayDestRect(client.right, client.bottom, &dest);
        ClampToFrame(src, &area);
        MapRect(&dest, &area, &part);
    }
    dc = GetDC(hwnd);
    if (dc != NULL) {
        BitBlt(dc, part.left, part.top, part.right - part.left, part.bottom - part.top, window_dc, part.left, part.top,
            SRCCOPY);
        ReleaseDC(hwnd, dc);
    }
    return DD_OK;
}

const unsigned int *PortDisplayWindowImage(int *width, int *height) {
    *width = window_width;
    *height = window_height;
    return (const unsigned int *)window_bits;
}

HRESULT PortPresent(HWND hwnd, LPDIRECTDRAWSURFACE frame, LPDIRECTDRAWSURFACE primary, const RECT *src) {
    if (PortDisplayWindowed) {
        return PresentWindowed(hwnd, frame, src);
    }
    return PresentFullscreen(frame, primary, src);
}

void PortDisplayClientToGame(HWND hwnd, POINT *pt) {
    RECT client;
    RECT dest;
    int dw;
    int dh;

    GetClientRect(hwnd, &client);
    PortDisplayDestRect(client.right, client.bottom, &dest);
    dw = dest.right - dest.left;
    dh = dest.bottom - dest.top;
    if (dw <= 0 || dh <= 0) {
        return;
    }
    pt->x = (pt->x - dest.left) * PORT_GAME_WIDTH / dw;
    pt->y = (pt->y - dest.top) * PORT_GAME_HEIGHT / dh;
}

void PortDisplayWindowSize(DWORD style, int *width, int *height) {
    RECT work;
    RECT frame;
    int cw = PortDisplayWidth;
    int ch = PortDisplayHeight;

    SetRect(&frame, 0, 0, cw, ch);
    AdjustWindowRect(&frame, style, FALSE);
    if (SystemParametersInfoA(SPI_GETWORKAREA, 0, &work, 0)) {
        int extra_w = (frame.right - frame.left) - cw;
        int extra_h = (frame.bottom - frame.top) - ch;
        int max_w = (work.right - work.left) - extra_w;
        int max_h = (work.bottom - work.top) - extra_h;

        if (cw > max_w || ch > max_h) {
            RECT fit;
            PortDisplayDestRect(max_w, max_h, &fit);
            cw = fit.right - fit.left;
            ch = fit.bottom - fit.top;
            SetRect(&frame, 0, 0, cw, ch);
            AdjustWindowRect(&frame, style, FALSE);
        }
    }
    *width = frame.right - frame.left;
    *height = frame.bottom - frame.top;
}

/* ---- Ctrl+F12: capture frames for debugging (see port_display.h) ---- */

#define CAPTURE_FRAMES 4

static int capture_left;
static int capture_index;
static int capture_key_down;
static char capture_dir[MAX_PATH];

/* A top-down 32-bit BMP. */
static void WriteBmp32(const char *name, const uint32_t *pixels, int width, int height) {
    BITMAPFILEHEADER file;
    BITMAPINFOHEADER info;
    char path[MAX_PATH];
    FILE *f;

    sprintf(path, "%s%s", capture_dir, name);
    f = fopen(path, "wb");
    if (f == NULL) {
        return;
    }
    memset(&file, 0, sizeof(file));
    memset(&info, 0, sizeof(info));
    file.bfType = 0x4d42;
    file.bfOffBits = sizeof(file) + sizeof(info);
    file.bfSize = file.bfOffBits + (DWORD)width * height * 4;
    info.biSize = sizeof(info);
    info.biWidth = width;
    info.biHeight = -height;
    info.biPlanes = 1;
    info.biBitCount = 32;
    fwrite(&file, sizeof(file), 1, f);
    fwrite(&info, sizeof(info), 1, f);
    fwrite(pixels, 4, (size_t)width * height, f);
    fclose(f);
}

/* The game's 640x480 frame, RGB565 widened to 32-bit. */
static void CaptureFrame(LPDIRECTDRAWSURFACE frame, int index) {
    static uint32_t pixels[PORT_GAME_WIDTH * PORT_GAME_HEIGHT];
    DDSURFACEDESC desc;
    char name[32];
    int x;
    int y;

    desc.dwSize = sizeof(desc);
    if (IDirectDrawSurface_Lock(frame, NULL, &desc, DDLOCK_WAIT | DDLOCK_READONLY, NULL) != DD_OK) {
        PortTrace("capture: frame %d: can't lock the frame", index);
        return;
    }
    for (y = 0; y < PORT_GAME_HEIGHT; y++) {
        const uint16_t *row = (const uint16_t *)((const char *)desc.lpSurface + (size_t)y * desc.lPitch);
        for (x = 0; x < PORT_GAME_WIDTH; x++) {
            unsigned v = desc.ddpfPixelFormat.dwRGBBitCount == 16 ? row[x] : 0;
            unsigned r = (v >> 11) & 31;
            unsigned g = (v >> 5) & 63;
            unsigned b = v & 31;
            pixels[y * PORT_GAME_WIDTH + x] =
                (((r << 3) | (r >> 2)) << 16) | (((g << 2) | (g >> 4)) << 8) | ((b << 3) | (b >> 2));
        }
    }
    IDirectDrawSurface_Unlock(frame, desc.lpSurface);
    sprintf(name, "frame-%d.bmp", index);
    WriteBmp32(name, pixels, PORT_GAME_WIDTH, PORT_GAME_HEIGHT);
}

/* What the screen shows over the window's client area (the desktop as composed by Windows, not the window's own
 * image): read just after a present ("after") and just before the next one ("before"). */
static void CaptureScreen(HWND hwnd, int index, const char *when) {
    BITMAPINFO info;
    RECT client;
    POINT origin = {0, 0};
    void *bits = NULL;
    HBITMAP bmp;
    HDC dc;
    HDC mem;
    char name[40];

    GetClientRect(hwnd, &client);
    ClientToScreen(hwnd, &origin);
    if (client.right <= 0 || client.bottom <= 0) {
        return;
    }
    memset(&info, 0, sizeof(info));
    info.bmiHeader.biSize = sizeof(info.bmiHeader);
    info.bmiHeader.biWidth = client.right;
    info.bmiHeader.biHeight = -client.bottom;
    info.bmiHeader.biPlanes = 1;
    info.bmiHeader.biBitCount = 32;
    dc = GetDC(NULL);
    mem = CreateCompatibleDC(dc);
    bmp = CreateDIBSection(dc, &info, DIB_RGB_COLORS, &bits, NULL, 0);
    if (bmp != NULL && bits != NULL) {
        HGDIOBJ old = SelectObject(mem, bmp);
        BitBlt(mem, 0, 0, client.right, client.bottom, dc, origin.x, origin.y, SRCCOPY);
        GdiFlush();
        sprintf(name, "desktop-%d-%s.bmp", index, when);
        WriteBmp32(name, (const uint32_t *)bits, client.right, client.bottom);
        SelectObject(mem, old);
        DeleteObject(bmp);
    }
    DeleteDC(mem);
    ReleaseDC(NULL, dc);
}

/* What the window shows (its client area), read back with GDI. */
static void CaptureWindow(HWND hwnd, int index) {
    BITMAPINFO info;
    RECT client;
    void *bits = NULL;
    HBITMAP bmp;
    HDC dc;
    HDC mem;
    char name[32];

    GetClientRect(hwnd, &client);
    if (client.right <= 0 || client.bottom <= 0) {
        return;
    }
    memset(&info, 0, sizeof(info));
    info.bmiHeader.biSize = sizeof(info.bmiHeader);
    info.bmiHeader.biWidth = client.right;
    info.bmiHeader.biHeight = -client.bottom;
    info.bmiHeader.biPlanes = 1;
    info.bmiHeader.biBitCount = 32;
    dc = GetDC(hwnd);
    mem = CreateCompatibleDC(dc);
    bmp = CreateDIBSection(dc, &info, DIB_RGB_COLORS, &bits, NULL, 0);
    if (bmp != NULL && bits != NULL) {
        HGDIOBJ old = SelectObject(mem, bmp);
        BitBlt(mem, 0, 0, client.right, client.bottom, dc, 0, 0, SRCCOPY);
        GdiFlush();
        sprintf(name, "screen-%d.bmp", index);
        WriteBmp32(name, (const uint32_t *)bits, client.right, client.bottom);
        SelectObject(mem, old);
        DeleteObject(bmp);
    }
    DeleteDC(mem);
    ReleaseDC(hwnd, dc);
}

void PortDisplayCapture(HWND hwnd, LPDIRECTDRAWSURFACE frame) {
    int down = (GetAsyncKeyState(VK_F12) & 0x8000) != 0 && (GetAsyncKeyState(VK_CONTROL) & 0x8000) != 0;

    if (capture_left == 0) {
        if (down && !capture_key_down && GetForegroundWindow() == hwnd) {
            char *slash;
            GetModuleFileNameA(NULL, capture_dir, sizeof(capture_dir) - 16);
            slash = strrchr(capture_dir, '\\');
            strcpy(slash != NULL ? slash + 1 : capture_dir, "capture\\");
            CreateDirectoryA(capture_dir, NULL);
            capture_left = CAPTURE_FRAMES;
            capture_index = 0;
            PortTrace("capture: %d frames to %s (mode %dx%d, windowed %d, borderless %d, %d bpp)", CAPTURE_FRAMES,
                capture_dir, PortDisplayWidth, PortDisplayHeight, PortDisplayWindowed, PortDisplayBorderless,
                PortDisplayBpp);
        }
        capture_key_down = down;
        return;
    }
    capture_key_down = down;
    PortTrace("capture: %d at %lu ms", capture_index, GetTickCount());
    CaptureFrame(frame, capture_index);
    CaptureWindow(hwnd, capture_index);
    CaptureScreen(hwnd, capture_index, "after");
    capture_index++;
    capture_left--;
}

void PortDisplayCaptureBefore(HWND hwnd) {
    if (capture_left > 0 && capture_index > 0) {
        /* the screen during the time since the last present, just before the next one */
        CaptureScreen(hwnd, capture_index - 1, "before");
    }
}
