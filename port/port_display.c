/* [library:video] The game's picture in the SDL3 window. See port_display.h. */
#include <windows.h>
#include <ddraw.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "miniwin/miniwin.h"
#include "port_display.h"
#include "port_trace.h"

int PortDisplayWidth = PORT_GAME_WIDTH;
int PortDisplayHeight = PORT_GAME_HEIGHT;
int PortDisplayFullscreen = MINIWIN_WINDOW;

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

HRESULT PortPresent(LPDIRECTDRAWSURFACE frame) {
    DDSURFACEDESC desc;
    RECT dest;
    HRESULT hr;
    int width;
    int height;

    if (!MiniwinOutputSize(&width, &height) || width <= 0 || height <= 0) {
        return DD_OK; /* minimized */
    }
    desc.dwSize = sizeof(desc);
    hr = IDirectDrawSurface_Lock(frame, NULL, &desc, DDLOCK_WAIT | DDLOCK_READONLY, NULL);
    if (hr != DD_OK) {
        return hr;
    }
    PortDisplayDestRect(width, height, &dest);
    MiniwinPresent(desc.lpSurface, (int)desc.lPitch, (int)desc.dwWidth, (int)desc.dwHeight, &dest);
    IDirectDrawSurface_Unlock(frame, desc.lpSurface);
    return DD_OK;
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
            unsigned v = row[x];
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
            PortTrace("capture: %d frames to %s (%dx%d, full screen %d)", CAPTURE_FRAMES, capture_dir,
                PortDisplayWidth, PortDisplayHeight, PortDisplayFullscreen);
        }
        capture_key_down = down;
        return;
    }
    capture_key_down = down;
    PortTrace("capture: %d at %lu ms", capture_index, GetTickCount());
    CaptureFrame(frame, capture_index);
    capture_index++;
    capture_left--;
}
