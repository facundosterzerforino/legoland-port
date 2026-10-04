/* [library:video] Output resolution: the 640x480 frame shown scaled. See port_display.h. */
#include <windows.h>
#include <ddraw.h>
#include <stdint.h>
#include <string.h>

#include "port_display.h"

int PortDisplayWidth = PORT_GAME_WIDTH;
int PortDisplayHeight = PORT_GAME_HEIGHT;
int PortDisplayWindowed = 0;

/* Source column for each destination column of the current fullscreen destination (rebuilt when it changes). */
static int column_map[4096];
static int column_map_width;

int PortDisplayScaled(void) {
    return PortDisplayWidth != PORT_GAME_WIDTH || PortDisplayHeight != PORT_GAME_HEIGHT;
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

/* Fullscreen: nearest-neighbour copy of the frame into the primary surface (both in the display's format). */
static HRESULT PresentFullscreen(LPDIRECTDRAWSURFACE frame, LPDIRECTDRAWSURFACE primary, const RECT *src) {
    DDSURFACEDESC fd;
    DDSURFACEDESC pd;
    RECT dest;
    RECT area;
    RECT part;
    HRESULT hr;
    int bytes;
    int dw;
    int dh;
    int x;
    int y;

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
    if (pd.ddpfPixelFormat.dwRGBBitCount != fd.ddpfPixelFormat.dwRGBBitCount || (bytes != 1 && bytes != 2)) {
        /* the screen isn't in the frame's format (e.g. the desktop's 32-bit mode after Alt+Tab): copying would
         * show garbage; the caller sets the mode again (KeepDisplayMode in draw.c) */
        IDirectDrawSurface_Unlock(primary, pd.lpSurface);
        IDirectDrawSurface_Unlock(frame, fd.lpSurface);
        return DDERR_WRONGMODE;
    }
    PortDisplayDestRect((int)pd.dwWidth, (int)pd.dwHeight, &dest);
    dw = dest.right - dest.left;
    dh = dest.bottom - dest.top;
    if (dw > (int)(sizeof(column_map) / sizeof(column_map[0]))) {
        dw = sizeof(column_map) / sizeof(column_map[0]);
        dest.right = dest.left + dw;
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
        /* black bars around a 4:3 picture on a wider (or taller) mode */
        FillRows(pd.lpSurface, pd.lPitch, bytes, 0, (int)pd.dwWidth, 0, dest.top);
        FillRows(pd.lpSurface, pd.lPitch, bytes, 0, (int)pd.dwWidth, dest.bottom, (int)pd.dwHeight);
        FillRows(pd.lpSurface, pd.lPitch, bytes, 0, dest.left, dest.top, dest.bottom);
        FillRows(pd.lpSurface, pd.lPitch, bytes, dest.right, (int)pd.dwWidth, dest.top, dest.bottom);
    }
    for (y = part.top; y < part.bottom; y++) {
        int sy = (y - dest.top) * PORT_GAME_HEIGHT / dh;
        const unsigned char *srow = (const unsigned char *)fd.lpSurface + (size_t)sy * fd.lPitch;
        unsigned char *drow = (unsigned char *)pd.lpSurface + (size_t)y * pd.lPitch;

        if (bytes == 2) {
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
    IDirectDrawSurface_Unlock(primary, pd.lpSurface);
    IDirectDrawSurface_Unlock(frame, fd.lpSurface);
    return DD_OK;
}

/* Windowed: the RGB565 frame (SetWindowedSurfaceFormat) is copied into the client area with GDI, which also
 * converts it to the desktop's format. */
static HRESULT PresentWindowed(HWND hwnd, LPDIRECTDRAWSURFACE frame, const RECT *src) {
    struct {
        BITMAPINFOHEADER header;
        DWORD masks[3];
    } bmi;
    DDSURFACEDESC desc;
    RECT client;
    RECT dest;
    RECT area;
    RECT part;
    HRESULT hr;
    HDC dc;

    desc.dwSize = sizeof(desc);
    hr = IDirectDrawSurface_Lock(frame, NULL, &desc, DDLOCK_WAIT | DDLOCK_READONLY, NULL);
    if (hr == DDERR_SURFACELOST) {
        IDirectDrawSurface_Restore(frame);
        hr = IDirectDrawSurface_Lock(frame, NULL, &desc, DDLOCK_WAIT | DDLOCK_READONLY, NULL);
    }
    if (hr != DD_OK) {
        return hr;
    }
    GetClientRect(hwnd, &client);
    PortDisplayDestRect(client.right, client.bottom, &dest);
    ClampToFrame(src, &area);
    MapRect(&dest, &area, &part);
    memset(&bmi, 0, sizeof(bmi));
    bmi.header.biSize = sizeof(bmi.header);
    bmi.header.biWidth = desc.lPitch / 2;
    /* top-down, starting at the first row of the part (StretchDIBits' source y is awkward for top-down DIBs) */
    bmi.header.biHeight = -(area.bottom - area.top);
    bmi.header.biPlanes = 1;
    bmi.header.biBitCount = 16;
    bmi.header.biCompression = BI_BITFIELDS;
    bmi.masks[0] = 0xf800;
    bmi.masks[1] = 0x07e0;
    bmi.masks[2] = 0x001f;
    dc = GetDC(hwnd);
    if (dc != NULL) {
        if (src == NULL) {
            HBRUSH black = (HBRUSH)GetStockObject(BLACK_BRUSH);
            RECT bar;

            SetRect(&bar, 0, 0, client.right, dest.top);
            FillRect(dc, &bar, black);
            SetRect(&bar, 0, dest.bottom, client.right, client.bottom);
            FillRect(dc, &bar, black);
            SetRect(&bar, 0, dest.top, dest.left, dest.bottom);
            FillRect(dc, &bar, black);
            SetRect(&bar, dest.right, dest.top, client.right, dest.bottom);
            FillRect(dc, &bar, black);
        }
        SetStretchBltMode(dc, COLORONCOLOR); /* nearest-neighbour */
        StretchDIBits(dc, part.left, part.top, part.right - part.left, part.bottom - part.top, area.left, 0,
            area.right - area.left, area.bottom - area.top,
            (const unsigned char *)desc.lpSurface + (size_t)area.top * desc.lPitch, (BITMAPINFO *)&bmi,
            DIB_RGB_COLORS, SRCCOPY);
        ReleaseDC(hwnd, dc);
    }
    IDirectDrawSurface_Unlock(frame, desc.lpSurface);
    return DD_OK;
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
