#include <windows.h>
#include <ddraw.h>
#include "legoland.h"

#include <stdlib.h>
#include <string.h>
#include "clipping.h"
#include "debug.h"
#include "debug_alloc.h"
#include "globals.h"

#include "draw.h"
#include "gfx.h"
#include "llidb.h"
#include "math.h"
#include "print_sprite.h"
#include "text.h"
#include "timer.h"
#include "wndenv.h"
#ifdef LEGOLAND_PORT
#include "port_display.h"
#include "port_watchdog.h"
#endif

#include "image_sprite.h"

// FUNCTION: LEGOLAND 0x004636f0
LEGO_EXPORT int InstallDirectDraw(void) { return 0; }

// FUNCTION: LEGOLAND 0x00463700
LEGO_EXPORT int InitHostSystemGPU(void) {
    LPDIRECTDRAW ddraw;
    LPDIRECTDRAW2 ddraw2;

    if (DDRAWENV.ddraw2 != 0) {
        return 1;
    }
    if (DirectDrawCreate(NULL, &DDRAWENV.ddraw, NULL) == 0) {
        ddraw = DDRAWENV.ddraw;
        if (IDirectDraw_QueryInterface(ddraw, &IID_IDirectDraw2_Guid, &DDRAWENV.ddraw2) != 0) {
            IDirectDraw_Release(DDRAWENV.ddraw);
            // STRING: LEGOLAND 0x004b9cd0
            DBPrintf("Can't Create DDCOM");
            return 0;
        }
        DDRAWENV.caps.dwSize = 0x17c;
        DDRAWENV.hel_caps.dwSize = 0x17c;
        ddraw2 = DDRAWENV.ddraw2;
        if (IDirectDraw2_GetCaps(ddraw2, &DDRAWENV.caps, &DDRAWENV.hel_caps) != 0) {
            IDirectDraw2_Release(DDRAWENV.ddraw2);
            IDirectDraw_Release(DDRAWENV.ddraw);
            // STRING: LEGOLAND 0x004b9cb8
            DBPrintf("Can't get DDCOM caps");
            return 0;
        }
        // STRING: LEGOLAND 0x004b9cac
        AddFontResourceA("Lego.ttf");
        return 1;
    }
    return 0;
}

// FUNCTION: LEGOLAND 0x004637c0
LEGO_EXPORT int CheckHostSystemGPU(void) {
    memset(&DDRAWENV, 0, sizeof(DDRAWENV));
    InitHostSystemGPU();
}

// FUNCTION: LEGOLAND 0x004637e0
LEGO_EXPORT void KillHostSystemGPU(void) {
    // STRING: LEGOLAND 0x004b9ce4
    RemoveFontResourceA("lego.ttf");
    if (DDRAWENV.ddraw2 != 0) {
        IDirectDraw2_Release(DDRAWENV.ddraw2);
        DDRAWENV.ddraw2 = 0;
    }
    DeleteObject((HGDIOBJ)LegoFont24Bold);
    DeleteObject((HGDIOBJ)LegoFont28Normal);
    DeleteObject((HGDIOBJ)LegoFont20Bold);
    DeleteObject((HGDIOBJ)LegoFont18SemiBold);
    if (DDRAWENV.ddraw != 0) {
        IDirectDraw_Release(DDRAWENV.ddraw);
        DDRAWENV.ddraw = 0;
    }
}

// FUNCTION: LEGOLAND 0x00463850
LEGO_EXPORT unsigned int SetPointer(unsigned int param_1) {
    unsigned int old = DAT_0066814c;
    DAT_0066814c = param_1;
    DAT_00668148 = PointerSprites[param_1];
    return old;
}

/* [library:video] port-only helper. In windowed mode (WINDEBUG) the game no longer takes the desktop's pixel
 * format, which on Windows 11 is 32-bit: its own surfaces are always RGB565, the format the 16-bit renderer
 * draws, and BlitFrameToWindow converts to the desktop with GDI. */
static void SetWindowedSurfaceFormat(DDSURFACEDESC *desc) {
#ifdef LEGOLAND_PORT
    if (WinDebugMode == 0 && PortDisplayBpp != 32) { /* also full screen in a 32-bit mode (port_display.c) */
        return;
    }
#else
    if (WinDebugMode == 0) {
        return;
    }
#endif
    desc->dwFlags |= DDSD_PIXELFORMAT;
    memset(&desc->ddpfPixelFormat, 0, sizeof(desc->ddpfPixelFormat));
    desc->ddpfPixelFormat.dwSize = sizeof(desc->ddpfPixelFormat);
    desc->ddpfPixelFormat.dwFlags = DDPF_RGB;
    desc->ddpfPixelFormat.dwRGBBitCount = 16;
    desc->ddpfPixelFormat.dwRBitMask = 0xf800;
    desc->ddpfPixelFormat.dwGBitMask = 0x07e0;
    desc->ddpfPixelFormat.dwBBitMask = 0x001f;
}

// FUNCTION: LEGOLAND 0x00463870
LEGO_EXPORT int InitScreen(void) {
    HRESULT hr;
    LOGFONTA font;
    WNDCLASSEXA wc;
    DDSURFACEDESC desc;
    RECT rect_slot;
    RECT window_rect;
    RGNDATA *rgn;

    rect_slot.left = 0;
    rect_slot.top = 0;
    rect_slot.right = lpConfig->screen_width;
    rect_slot.bottom = lpConfig->screen_height;
    FrameNumber = 0;

    wc.cbSize = sizeof(wc);
    wc.style = 0;
    wc.lpfnWndProc = LegoLandWindowProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = WNDENV_GethInstance();
    wc.hIcon = LoadIconA(WNDENV_GethInstance(), (LPCSTR)0x65);
    wc.hCursor = LoadCursorA(WNDENV_GethInstance(), (LPCSTR)0x7d);
    wc.hbrBackground = (HBRUSH)GetStockObject(5);
    wc.lpszMenuName = NULL;
    // STRING: LEGOLAND 0x004b9cfc
    wc.lpszClassName = "LEGOLANDMAIN";
    wc.hIconSm = LoadIconA(WNDENV_GethInstance(), (LPCSTR)0x65);
    RegisterClassExA(&wc);

    font.lfHeight = 0x18;
    font.lfWidth = 0;
    font.lfEscapement = 0;
    font.lfOrientation = 0;
    font.lfWeight = 0x2bc;
    font.lfItalic = 0;
    font.lfUnderline = 0;
    font.lfStrikeOut = 0;
    font.lfCharSet = 1;
    font.lfOutPrecision = 0;
    font.lfClipPrecision = 0;
    font.lfQuality = 2;
    font.lfPitchAndFamily = 0;
    // STRING: LEGOLAND 0x004b86e0
    strcpy(font.lfFaceName, "Lego");
    LegoFont24Bold = CreateFontIndirectA(&font);

    font.lfWeight = 0x190;
    font.lfHeight = 0x1c;
    font.lfWidth = 0;
    LegoFont28Normal = CreateFontIndirectA(&font);
    font.lfHeight = 0x14;
    font.lfWidth = 0;
    font.lfWeight = 0x2bc;
    LegoFont20Bold = CreateFontIndirectA(&font);
    font.lfWeight = 0x258;
    font.lfHeight = 0x12;
    font.lfWidth = 0;
    strcpy(font.lfFaceName, "Lego");
    LegoFont18SemiBold = CreateFontIndirectA(&font);

    if (WinDebugMode == 0) {
        DisplayPixelFormat = 2;
        WNDENV_Sethwnd(CreateWindowExA(8, "LEGOLANDMAIN",
            // STRING: LEGOLAND 0x004b86d0
            "LEGOLAND", 0x90000000, 0, 0,
#ifdef LEGOLAND_PORT
            PortDisplayWidth, PortDisplayHeight, /* [library:video] the chosen display mode (port_display.h) */
#else
            lpConfig->screen_width, lpConfig->screen_height,
#endif
            GetDesktopWindow(), NULL, WNDENV_GethInstance(), NULL));
        if (WNDENV_Gethwnd() == NULL) {
            DebugTrace("InitScreen: CreateWindowExA failed err=%lu", GetLastError());
            return 0;
        }
#ifdef LEGOLAND_PORT
        /* [library:window] after the launcher closed, Windows gave the focus to another program: the game pauses
         * while its window is inactive (ProcessSystemEvents), so a full-screen game sat paused behind it */
        SetForegroundWindow(WNDENV_Gethwnd());
        DebugTrace("InitScreen: full screen %dx%d, window %s", PortDisplayWidth, PortDisplayHeight,
            GetForegroundWindow() == WNDENV_Gethwnd() ? "active" : "NOT active");
#endif
        if ((hr = IDirectDraw2_SetCooperativeLevel(DDRAWENV.ddraw2, WNDENV_Gethwnd(), 0x11)) != 0) {
            DebugTrace("InitScreen: IDirectDraw2_SetCooperativeLevel failed hr=%lx", hr);
            return 0;
        }
        if (SetDisplayModeAndDetectPixelFormat() == 0) {
            return 0;
        }
        while ((lpConfig->field_1c & 1) != 0) {
            ProcessSystemEvents();
            ShowWindow(WNDENV_Gethwnd(), 3);
        }
        if (lpConfig->field_1e == 0) {
            ShowCursor(0);
        }
        BlitFrameFunc = BlitFrameToWindow;
        desc.dwSize = sizeof(desc);
        desc.dwFlags = 1;
        desc.ddsCaps.dwCaps = 0x4200;
        if ((hr = IDirectDraw2_CreateSurface(DDRAWENV.ddraw2, &desc, &PrimarySurface, NULL)) != 0) {
            DebugTrace("InitScreen: IDirectDraw2_CreateSurface failed hr=%lx", hr);
            return 0;
        }
        LoadColourTable();
        desc.dwSize = sizeof(desc);
        desc.dwFlags = 7;
        desc.ddsCaps.dwCaps = 0x800;
        desc.dwWidth = lpConfig->screen_width;
        desc.dwHeight = lpConfig->screen_height;
#ifdef LEGOLAND_PORT
        if (PortDisplayBpp == 32) {
            /* [library:video] RGB565 on a 32-bit display mode. A system-memory surface with its own pixel format
             * must also be off-screen plain (0x800 alone: DDERR_INVALIDPIXELFORMAT; tools/test_display) */
            desc.ddsCaps.dwCaps = 0x840;
            SetWindowedSurfaceFormat(&desc);
        }
#endif
        if ((hr = IDirectDraw2_CreateSurface(DDRAWENV.ddraw2, &desc, &OffscreenSurface, NULL)) != 0) {
            DebugTrace("InitScreen: IDirectDraw2_CreateSurface failed hr=%lx", hr);
            return 0;
        }
        renderEngine = OffscreenSurface;
        desc.dwSize = sizeof(desc);
        desc.dwFlags = 7;
        desc.ddsCaps.dwCaps = 0x4000;
        desc.dwWidth = lpConfig->screen_width;
        desc.dwHeight = lpConfig->screen_height;
#ifdef LEGOLAND_PORT
        if (PortDisplayBpp == 32) {
            /* [library:video] RGB565 on a 32-bit display mode: a plain off-screen surface, as in windowed mode */
            desc.ddsCaps.dwCaps = 0x40;
            SetWindowedSurfaceFormat(&desc);
        }
#endif
        if ((hr = IDirectDraw2_CreateSurface(DDRAWENV.ddraw2, &desc, &DAT_00668074, NULL)) != 0) {
            DebugTrace("InitScreen: IDirectDraw2_CreateSurface failed hr=%lx", hr);
            return 0;
        }
        IDirectDraw2_CreateClipper(DDRAWENV.ddraw2, 0, &DDrawClipper, NULL);
        SetClipping(&rect_slot);
        rgn = malloc(sizeof(RGNDATA) - 1 + sizeof(RECT));
        rgn->rdh.dwSize = 0x20;
        rgn->rdh.iType = 1;
        rgn->rdh.nCount = 1;
        rgn->rdh.nRgnSize = 0x10;
        rgn->rdh.rcBound = SPRITE_ClipRect;
        memcpy(rgn->Buffer, &SPRITE_ClipRect, sizeof(RECT));
        IDirectDrawClipper_SetClipList(DDrawClipper, (LPRGNDATA)rgn, 0);
        free(rgn);
    } else {
        window_rect.left = 0;
        window_rect.top = 0;
        window_rect.right = lpConfig->screen_width - 1;
        window_rect.bottom = lpConfig->screen_height - 1;
        AdjustWindowRect(&window_rect, 0x10cf0000, 0);
#ifdef LEGOLAND_PORT
        /* [library:video] the window's client area is the chosen size (port_display.h) */
        {
            int w;
            int h;
            PortDisplayWindowSize(0x10cf0000, &w, &h);
            window_rect.left = 0;
            window_rect.top = 0;
            window_rect.right = w - 1;
            window_rect.bottom = h - 1;
        }
#endif
#ifdef LEGOLAND_PORT
        if (PortDisplayBorderless) {
            /* [library:video] full screen at the desktop's resolution: a borderless window over the whole
             * monitor, the display mode is left alone (port_display.h) */
            WNDENV_Sethwnd(CreateWindowExA(0, "LEGOLANDMAIN", "Lego Land", WS_POPUP | WS_VISIBLE, 0,
                0, PortDisplayWidth, PortDisplayHeight, NULL, NULL, WNDENV_GethInstance(), NULL));
        } else
#endif
            WNDENV_Sethwnd(CreateWindowExA(0, "LEGOLANDMAIN",
                // STRING: LEGOLAND 0x004b9cf0
                "Lego Land", 0x10cf0000, 0, 0, window_rect.right - window_rect.left + 1, window_rect.bottom - window_rect.top + 1, NULL, NULL, WNDENV_GethInstance(), NULL));
        if (WNDENV_Gethwnd() == NULL) {
            DebugTrace("InitScreen: CreateWindowExA failed err=%lu", GetLastError());
            return 0;
        }
#ifdef LEGOLAND_PORT
        SetForegroundWindow(WNDENV_Gethwnd()); /* [library:window] see the full-screen case */
#endif
        if ((hr = IDirectDraw2_SetCooperativeLevel(DDRAWENV.ddraw2, WNDENV_Gethwnd(), 8)) != 0) {
            DebugTrace("InitScreen: IDirectDraw2_SetCooperativeLevel failed hr=%lx", hr);
            DestroyWindow(WNDENV_Gethwnd());
            return 0;
        }
        if (SetDisplayModeAndDetectPixelFormat() == 0) {
            DestroyWindow(WNDENV_Gethwnd());
            return 0;
        }
        IDirectDraw2_CreateClipper(DDRAWENV.ddraw2, 0, &DDrawClipper, NULL);
        IDirectDrawClipper_SetHWnd(DDrawClipper, 0, WNDENV_Gethwnd());
        desc.dwSize = sizeof(desc);
        desc.dwFlags = 1;
        desc.ddsCaps.dwCaps = 0x200;
        if ((hr = IDirectDraw2_CreateSurface(DDRAWENV.ddraw2, &desc, &PrimarySurface, NULL)) != 0) {
            DebugTrace("InitScreen: IDirectDraw2_CreateSurface failed hr=%lx", hr);
            DestroyWindow(WNDENV_Gethwnd());
            return 0;
        }
        IDirectDrawSurface_SetClipper(PrimarySurface, DDrawClipper);
        desc.dwSize = sizeof(desc);
        desc.dwFlags = 7;
        desc.ddsCaps.dwCaps = 0x40;
        desc.dwWidth = lpConfig->screen_width;
        desc.dwHeight = lpConfig->screen_height;
        SetWindowedSurfaceFormat(&desc); /* [library:video] RGB565, not the desktop's format */
        if ((hr = IDirectDraw2_CreateSurface(DDRAWENV.ddraw2, &desc, &OffscreenSurface, NULL)) != 0) {
            DebugTrace("InitScreen: IDirectDraw2_CreateSurface failed hr=%lx", hr);
            IDirectDrawSurface_Release(PrimarySurface);
            DestroyWindow(WNDENV_Gethwnd());
            return 0;
        }
        renderEngine = OffscreenSurface;
        desc.dwSize = sizeof(desc);
        desc.dwFlags = 7;
        desc.ddsCaps.dwCaps = 0x40;
        desc.dwWidth = lpConfig->screen_width;
        desc.dwHeight = lpConfig->screen_height;
        SetWindowedSurfaceFormat(&desc); /* [library:video] */
        if ((hr = IDirectDraw2_CreateSurface(DDRAWENV.ddraw2, &desc, &DAT_00668074, NULL)) != 0) {
            DebugTrace("InitScreen: IDirectDraw2_CreateSurface failed hr=%lx", hr);
            return 0;
        }
    }
    DebugTrace("InitScreen: ok");
    return 1;
}

// FUNCTION: LEGOLAND 0x00463ef0
int SetDisplayModeAndDetectPixelFormat(void) {
    DDSURFACEDESC desc;
    LPDIRECTDRAW2 ddraw2;

    if (WinDebugMode == 0) {
        ddraw2 = DDRAWENV.ddraw2;
#ifdef LEGOLAND_PORT
        /* [library:video] the chosen display mode; the frame stays 640x480 and is scaled (port_display.h).
         * If the mode is refused, the original 640x480. */
        if (IDirectDraw2_SetDisplayMode(ddraw2, PortDisplayWidth, PortDisplayHeight, 32, 0, 0) == 0) {
            /* 32-bit: no 16-bit mode for Windows to emulate; the game's surfaces stay RGB565 and the frame is
             * converted when shown (SetWindowedSurfaceFormat, port_display.c) */
            PortDisplayBpp = 32;
            goto mode_set;
        }
        DebugTrace("SetDisplayMode: %dx%d 32bpp refused, trying 16bpp", PortDisplayWidth, PortDisplayHeight);
        PortDisplayBpp = 16;
        if (PortDisplayScaled()) {
            if (IDirectDraw2_SetDisplayMode(ddraw2, PortDisplayWidth, PortDisplayHeight, 0x10, 0, 0) == 0) {
                goto mode_set;
            }
            DebugTrace("SetDisplayMode: %dx%d 16bpp refused, using 640x480", PortDisplayWidth, PortDisplayHeight);
            PortDisplayWidth = PORT_GAME_WIDTH;
            PortDisplayHeight = PORT_GAME_HEIGHT;
            MoveWindow(WNDENV_Gethwnd(), 0, 0, PortDisplayWidth, PortDisplayHeight, FALSE);
        }
#endif
        if (IDirectDraw2_SetDisplayMode(ddraw2, lpConfig->screen_width, lpConfig->screen_height, 0x10, 0, 0) != 0) {
            DebugTrace("SetDisplayMode: %dx%d 16bpp refused", lpConfig->screen_width, lpConfig->screen_height);
            ddraw2 = DDRAWENV.ddraw2;
            if (IDirectDraw2_SetDisplayMode(ddraw2, lpConfig->screen_width, lpConfig->screen_height, 8, 0, 0) != 0) {
                DebugTrace("SetDisplayMode: 8bpp refused too");
                return 0;
            }
        }
    }
#ifdef LEGOLAND_PORT
mode_set:
#endif
    desc.dwSize = 0x6c;
    ddraw2 = DDRAWENV.ddraw2;
    IDirectDraw2_GetDisplayMode(ddraw2, &desc);
    DebugTrace("SetDisplayMode: display is %lux%lu %lu bpp (masks %lx %lx %lx)", desc.dwWidth, desc.dwHeight,
        desc.ddpfPixelFormat.dwRGBBitCount, desc.ddpfPixelFormat.dwRBitMask, desc.ddpfPixelFormat.dwGBitMask,
        desc.ddpfPixelFormat.dwBBitMask);
#ifdef LEGOLAND_PORT
    if (WinDebugMode != 0 || PortDisplayBpp == 32) {
#else
    if (WinDebugMode != 0) {
#endif
        /* [library:video] windowed mode can't change the desktop's format (32-bit on Windows 11, which the
         * original refused), and full screen uses a 32-bit mode too; the game's surfaces are RGB565 instead
         * (SetWindowedSurfaceFormat). */
        DisplayPixelFormat = 2;
        return 1;
    }
    if (desc.ddpfPixelFormat.dwRGBBitCount != 8) {
        if (desc.ddpfPixelFormat.dwRGBBitCount != 0x10) {
            return 0;
        }
        DisplayPixelFormat = (desc.ddpfPixelFormat.dwGBitMask == 0x7e0) + 1;
        return 1;
    }
    DisplayPixelFormat = 0;
    return 1;
}

// FUNCTION: LEGOLAND 0x00463fc0
LEGO_EXPORT void PushRenderingStatusAndLockVideoSurface(void) {
    RECT local;
    LPDIRECTDRAWSURFACE surface;
    int value;

    value = VideoSurfaceLocked;
    DAT_00668164[RenderingStatusStackDepth] = VideoSurfaceLocked;
    RenderingStatusStackDepth = RenderingStatusStackDepth + 1;
    if (value == 0) {
        local.left = value;
        local.top = value;
        local.right = lpConfig->screen_width - 1;
        local.bottom = lpConfig->screen_height - 1;
        CurrentSurfaceDesc.dwSize = 0x6c;
        IntersectRect(&DAT_00668108, &local, &SPRITE_ClipRect);
        surface = renderEngine;
        if (IDirectDrawSurface_Lock(surface, NULL, &CurrentSurfaceDesc, 0x21, NULL) == 0x887601c2) {
            IDirectDrawSurface_Restore(renderEngine);
            IDirectDrawSurface_Lock(renderEngine, NULL, &CurrentSurfaceDesc, 0x21, NULL);
        }
        StoredTransparentColour = GetTransparentColour();
    }
    VideoSurfaceLocked = 1;
}

// FUNCTION: LEGOLAND 0x00464080
LEGO_EXPORT void PushRenderingStatusAndUnlockVideoSurface(void) {
    LPDIRECTDRAWSURFACE surface;

    DAT_00668164[RenderingStatusStackDepth] = VideoSurfaceLocked;
    RenderingStatusStackDepth = RenderingStatusStackDepth + 1;
    if (VideoSurfaceLocked != 0) {
        surface = renderEngine;
        if (IDirectDrawSurface_Unlock(surface, CurrentSurfaceDesc.lpSurface) == 0x887601c2) {
            IDirectDrawSurface_Restore(renderEngine);
            IDirectDrawSurface_Unlock(renderEngine, CurrentSurfaceDesc.lpSurface);
        }
    }
    VideoSurfaceLocked = 0;
}

// FUNCTION: LEGOLAND 0x004640f0
void FUN_004640f0(void) {
    RECT local;
    LPDIRECTDRAWSURFACE surface;
    int wasLocked;

    wasLocked = VideoSurfaceLocked;
    local.left = 0;
    local.top = 0;
    local.right = lpConfig->screen_width - 1;
    local.bottom = lpConfig->screen_height - 1;
    DAT_00668164[RenderingStatusStackDepth] = VideoSurfaceLocked;
    RenderingStatusStackDepth = RenderingStatusStackDepth + 1;
    if (wasLocked != 0) {
        surface = renderEngine;
        if (IDirectDrawSurface_Unlock(surface, CurrentSurfaceDesc.lpSurface) == 0x887601c2) {
            IDirectDrawSurface_Restore(renderEngine);
            IDirectDrawSurface_Unlock(renderEngine, CurrentSurfaceDesc.lpSurface);
        }
    }
    CurrentSurfaceDesc.dwSize = 0x6c;
    IntersectRect(&DAT_00668108, &local, &SPRITE_ClipRect);
    surface = renderEngine;
    if (IDirectDrawSurface_Lock(surface, NULL, &CurrentSurfaceDesc, 0x21, NULL) == 0x887601c2) {
        IDirectDrawSurface_Restore(renderEngine);
        IDirectDrawSurface_Lock(renderEngine, NULL, &CurrentSurfaceDesc, 0x21, NULL);
    }
    StoredTransparentColour = GetTransparentColour();
    VideoSurfaceLocked = 1;
}

// FUNCTION: LEGOLAND 0x004641f0
LEGO_EXPORT void PopRenderingStatus(void) {
    RECT local;
    LPDIRECTDRAWSURFACE surface;

    RenderingStatusStackDepth = RenderingStatusStackDepth - 1;
    if (DAT_00668164[RenderingStatusStackDepth] != 0) {
        if (VideoSurfaceLocked == 0) {
            local.left = 0;
            local.top = 0;
            local.right = lpConfig->screen_width - 1;
            local.bottom = lpConfig->screen_height - 1;
            CurrentSurfaceDesc.dwSize = 0x6c;
            IntersectRect(&DAT_00668108, &local, &SPRITE_ClipRect);
            surface = renderEngine;
            if (IDirectDrawSurface_Lock(surface, NULL, &CurrentSurfaceDesc, 0x21, NULL) == 0x887601c2) {
                IDirectDrawSurface_Restore(renderEngine);
                IDirectDrawSurface_Lock(renderEngine, NULL, &CurrentSurfaceDesc, 0x21, NULL);
            }
            StoredTransparentColour = GetTransparentColour();
            VideoSurfaceLocked = 1;
        }
        return;
    }
    if (VideoSurfaceLocked != 0) {
        surface = renderEngine;
        if (IDirectDrawSurface_Unlock(surface, CurrentSurfaceDesc.lpSurface) == 0x887601c2) {
            IDirectDrawSurface_Restore(renderEngine);
            IDirectDrawSurface_Unlock(renderEngine, CurrentSurfaceDesc.lpSurface);
        }
        VideoSurfaceLocked = 0;
    }
}

// FUNCTION: LEGOLAND 0x00464310
LEGO_EXPORT int GetVideoSurface(struct VideoArg *arg) {
    if (VideoSurfaceLocked == 0) {
        return 0;
    }
    arg->pitch = CurrentSurfaceDesc.lPitch;
    arg->width = lpConfig->screen_width;
    arg->height = lpConfig->screen_height;
    arg->bits = CurrentSurfaceDesc.lpSurface;
    arg->field_14 = 2;
    return 1;
}

// FUNCTION: LEGOLAND 0x00464360
LEGO_EXPORT void PrintBackground(int x, int y) { return; }

// FUNCTION: LEGOLAND 0x00464370
LEGO_EXPORT void CommitCliprectToHardware(void) {
    RGNDATA *rgn;
    LPDIRECTDRAWCLIPPER clipper;

    rgn = malloc(sizeof(RGNDATA) - 1 + sizeof(RECT));
    rgn->rdh.iType = 1;
    rgn->rdh.nCount = 1;
    rgn->rdh.dwSize = 0x20;
    rgn->rdh.nRgnSize = 0x10;
    rgn->rdh.rcBound = SPRITE_ClipRect;
    memcpy(rgn->Buffer, &SPRITE_ClipRect, sizeof(RECT));
    clipper = DDrawClipper;
    IDirectDrawClipper_SetClipList(clipper, (LPRGNDATA)rgn, 0);
    free(rgn);
}

// FUNCTION: LEGOLAND 0x00464400
LEGO_EXPORT void SetOverridePalette(unsigned int param_1) { OverridePalette = param_1; }

// FUNCTION: LEGOLAND 0x00464410
LEGO_EXPORT unsigned int GetOverridePalette(void) { return OverridePalette; }

// FUNCTION: LEGOLAND 0x00464420
LEGO_EXPORT void SetOverrideFrame(unsigned int param_1) { OverrideFrame = param_1; }

// FUNCTION: LEGOLAND 0x00464430
LEGO_EXPORT unsigned int GetOverrideFrame(void) { return OverrideFrame; }

// FUNCTION: LEGOLAND 0x00464440
LEGO_EXPORT void ClearOverrideFrame(void) { OverrideFrame = 0xffffffff; }

// FUNCTION: LEGOLAND 0x00464450
LEGO_EXPORT void ClearOverridePalette(void) { OverridePalette = 0; }

// FUNCTION: LEGOLAND 0x00464460
LEGO_EXPORT void ClearSpriteOverrides(void) {
    OverrideFrame = 0xffffffff;
    OverridePalette = 0;
}

/* Port [library:asm]: bit reader for the stream that places the pixels of an indexed-colour animation frame
 * (FUN_00464480, FUN_00465240, ZBufferHelper). The stream is a list of 32-bit words of sixteen 2-bit codes each,
 * lowest bits first. A run length is the next 8 bits (four codes) of the same word; if fewer than four codes are
 * left in the word, the rest of the word is dropped and the length is the low byte of the next word. */
struct PortIdxBits {
    unsigned int bits; /* current word, shifted so the next code is in bits 0-1 */
    int left; /* codes not yet read from the current word */
    const unsigned int *next; /* next word of the stream */
};

enum {
    PORT_IDX_LEFT, /* still inside the clipped-off left margin of the row */
    PORT_IDX_BODY, /* inside the visible part of the row */
    PORT_IDX_COPY, /* draw `run` pixels from the source */
    PORT_IDX_RUN, /* draw a run: fill if is_fill, else copy */
    PORT_IDX_END, /* past the right clip: skip the rest of the row's stream */
    PORT_IDX_NEXT /* row finished */
};

static unsigned int PortIdxCode(struct PortIdxBits *r) {
    r->bits >>= 2;
    if (r->left == 0) {
        r->bits = *r->next++;
        r->left = 16;
    }
    r->left--;
    return r->bits & 3;
}

static unsigned int PortIdxCount(struct PortIdxBits *r) {
    unsigned int count;

    r->bits >>= 2;
    if (r->left < 4) {
        r->bits = *r->next++;
        r->left = 16;
    }
    count = r->bits & 0xff;
    r->bits >>= 6;
    r->left -= 4;
    return count;
}

/* Codes: 00/01 one source pixel; 10 one transparent pixel; 11 a run, whose length is read with PortIdxCount
 * (0 ends the row) and whose kind is the next code: 1x transparent, 01 one source pixel repeated, 00 that many
 * source pixels. Skips the rest of one row, advancing the source pointer past the pixels it would have used. */
static const unsigned char *PortIdxSkipRow(struct PortIdxBits *r, const unsigned char *src) {
    unsigned int c;
    unsigned int run;

    for (;;) {
        c = PortIdxCode(r);
        if (!(c & 2)) {
            src++;
        } else if (c & 1) {
            run = PortIdxCount(r);
            if (run == 0) {
                return src;
            }
            c = PortIdxCode(r);
            if (!(c & 2)) {
                if (c & 1) {
                    src++;
                } else {
                    src += run;
                }
            }
        }
    }
}

static struct DrawLLSIdxFrame *PortIdxFrameAt(struct DrawLLS *lls, int index) {
    struct DrawLLSIdxFrame *frame = (struct DrawLLSIdxFrame *)(lls + 1);

    while (index-- != 0) {
        frame = (struct DrawLLSIdxFrame *)((unsigned char *)frame + frame->size);
    }
    return frame;
}

/* The frame to draw: the override (OverrideFrame) if set, else the animation's own, clamped to the last frame. */
static int PortLLSFrameIndex(struct DrawLLS *lls) {
    int index = (int)OverrideFrame;

    if (index < 0) {
        index = lls->frame;
    }
    if (index >= lls->frame_count) {
        index = lls->frame_count - 1;
    }
    return index;
}

// FUNCTION: LEGOLAND 0x00464480
void FUN_00464480(struct DrawLLS *lls, RECT *rect, struct Point *pos) {
    /* Port [library:asm]: the original is inline asm. Draws one frame (two, for animations with flag 1: the base
     * frame, then the delta frame on top) of an indexed-colour animation into the software screen at pos, showing
     * only the part inside rect. Each source byte indexes a 16-bit palette (OverridePalette if overridden, else the one
     * stored with frame 0). Sets DAT_007feb14 bit 0 when a drawn run touches the mouse cursor (DAT_007fe9a8).
     * Quirks kept: when a transparent or copy run ends exactly at the left clip edge the decoder stays in its
     * left-margin state (so the next item is dropped); a single-pixel skip after the visible width is used up wraps
     * the remaining width; a fill run that starts with no width left still stores one pixel.
     * [port:rewrite] every pixel store is kept inside the row's visible span (row .. row + width): after the
     * width wrap above, the original kept copying runs past the clip edge, which in the port ran off the end of
     * the software screen (crash when the Space Tower popup was drawn). Pixels inside the clip are unchanged. */
    struct PortIdxBits bits;
    struct DrawLLSIdxFrame *first;
    struct DrawLLSIdxFrame *frame;
    const unsigned short *palette;
    const unsigned char *src;
    unsigned short *row;
    unsigned short *dst;
    unsigned short *row_end;
    unsigned short *cursor;
    unsigned short colour;
    unsigned int width;
    unsigned int left;
    unsigned int rem;
    unsigned int run;
    unsigned int count;
    unsigned int leftover;
    unsigned int k;
    unsigned int c;
    int frame_index;
    int passes;
    int pass;
    int pitch;
    int top;
    int height;
    int rows;
    int state;
    int is_fill;
    int with_palette;
    int hit;

    frame_index = PortLLSFrameIndex(lls);
    passes = (lls->flags & 1) ? 2 : 1;
    pitch = CurrentSurfaceDesc.lPitch;
    cursor = (unsigned short *)DAT_007fe9a8;
    left = rect->left;
    width = rect->right - rect->left;
    top = rect->top;
    height = rect->bottom - rect->top;
    palette = NULL;
    is_fill = 0;
    hit = 0;
    first = PortIdxFrameAt(lls, 0);
    for (pass = 0; pass < passes; pass++) {
        if (passes == 1) {
            frame = PortIdxFrameAt(lls, frame_index);
            with_palette = (frame_index == 0);
        } else if (pass == 0) {
            frame = first;
            with_palette = 1;
        } else {
            frame = PortIdxFrameAt(lls, frame_index + 1);
            with_palette = 0;
        }
        if (passes == 1 || pass == 0) {
            if (OverridePalette != 0) {
                palette = (const unsigned short *)OverridePalette;
            } else {
                palette = (const unsigned short *)(first->pixels + first->pixel_count);
            }
        }
        src = frame->pixels;
        bits.bits = 0;
        bits.left = 0;
        bits.next = (const unsigned int *)(frame->pixels + frame->pixel_count + (with_palette ? 0x200 : 0));
        row = (unsigned short *)((unsigned char *)CurrentSurfaceDesc.lpSurface + pos->x * 2 + pos->y * pitch);
        for (rows = top; rows > 0; rows--) {
            src = PortIdxSkipRow(&bits, src);
        }
        for (rows = height; rows != 0; rows--) {
            dst = row;
            row_end = row + width;
            if (left != 0) {
                rem = left;
                state = PORT_IDX_LEFT;
            } else {
                rem = width;
                state = PORT_IDX_BODY;
            }
            while (state != PORT_IDX_NEXT) {
                switch (state) {
                case PORT_IDX_LEFT:
                    c = PortIdxCode(&bits);
                    if (!(c & 2) || !(c & 1)) {
                        /* one source pixel, or one transparent pixel */
                        if (!(c & 2)) {
                            src++;
                        }
                        rem--;
                        if (rem == 0) {
                            rem = width;
                            state = PORT_IDX_BODY;
                        }
                        break;
                    }
                    run = PortIdxCount(&bits);
                    if (run == 0) {
                        state = PORT_IDX_NEXT;
                        break;
                    }
                    c = PortIdxCode(&bits);
                    if (c & 2) {
                        rem -= run;
                        if ((int)rem >= 0) {
                            break;
                        }
                        rem = 0 - rem;
                        dst += rem;
                        if ((int)(width - rem) < 0) {
                            state = PORT_IDX_END;
                        } else {
                            rem = width - rem;
                            state = PORT_IDX_BODY;
                        }
                    } else if (c & 1) {
                        src++;
                        rem -= run;
                        if ((int)rem >= 0) {
                            break;
                        }
                        run = 0 - rem;
                        rem = width;
                        if (rem == 0) {
                            state = PORT_IDX_END;
                            break;
                        }
                        hit = (dst <= cursor);
                        if (run != 0) {
                            src--;
                            is_fill = 1;
                            state = PORT_IDX_RUN;
                        } else {
                            state = PORT_IDX_BODY;
                        }
                    } else {
                        src += run;
                        rem -= run;
                        if ((int)rem >= 0) {
                            break;
                        }
                        run = 0 - rem;
                        rem = width;
                        if (rem == 0) {
                            state = PORT_IDX_END;
                            break;
                        }
                        src -= run;
                        hit = (dst <= cursor);
                        state = (run != 0) ? PORT_IDX_COPY : PORT_IDX_BODY;
                    }
                    break;
                case PORT_IDX_BODY:
                    c = PortIdxCode(&bits);
                    if (!(c & 2)) {
                        run = 1;
                        hit = (dst == cursor);
                        state = PORT_IDX_COPY;
                        break;
                    }
                    if (!(c & 1)) {
                        rem--;
                        dst++;
                        break;
                    }
                    run = PortIdxCount(&bits);
                    if (run == 0) {
                        state = PORT_IDX_NEXT;
                        break;
                    }
                    c = PortIdxCode(&bits);
                    hit = (dst <= cursor);
                    if (c & 2) {
                        rem -= run;
                        if ((int)rem < 0) {
                            state = PORT_IDX_END;
                        } else {
                            dst += run;
                        }
                        break;
                    }
                    is_fill = c & 1;
                    state = PORT_IDX_RUN;
                    break;
                case PORT_IDX_RUN:
                    if (!is_fill) {
                        state = PORT_IDX_COPY;
                        break;
                    }
                    colour = palette[*src++];
                    if (rem <= run) {
                        if (rem == 0 && dst < row_end) {
                            *dst = colour;
                        }
                        for (k = 0; k < rem && dst + k < row_end; k++) {
                            dst[k] = colour;
                        }
                        dst += rem;
                        state = PORT_IDX_END;
                    } else {
                        rem -= run;
                        for (k = 0; k < run; k++, dst++) {
                            if (dst < row_end) {
                                *dst = colour;
                            }
                        }
                        state = PORT_IDX_BODY;
                    }
                    if (hit && dst >= cursor) {
                        DAT_007feb14 |= 1;
                    }
                    break;
                case PORT_IDX_COPY:
                    if (rem <= run) {
                        count = rem;
                        leftover = run - rem;
                        if (count != 0) {
                            for (k = 0; k < count; k++, dst++, src++) {
                                if (dst < row_end) {
                                    *dst = palette[*src];
                                }
                            }
                            if (hit && dst >= cursor) {
                                DAT_007feb14 |= 1;
                            }
                        }
                        src += leftover;
                        state = PORT_IDX_END;
                    } else {
                        rem -= run;
                        for (k = 0; k < run; k++, dst++, src++) {
                            if (dst < row_end) {
                                *dst = palette[*src];
                            }
                        }
                        if (hit && dst >= cursor) {
                            DAT_007feb14 |= 1;
                        }
                        state = PORT_IDX_BODY;
                    }
                    break;
                case PORT_IDX_END:
                    src = PortIdxSkipRow(&bits, src);
                    state = PORT_IDX_NEXT;
                    break;
                }
            }
            row = (unsigned short *)((unsigned char *)row + pitch);
        }
    }
}

// FUNCTION: LEGOLAND 0x00464a90
LEGO_EXPORT void ZBufferHelper(struct DrawLLS *lls, RECT *rect, struct Point *pos, unsigned int *zbuf) {
    /* Port [library:asm]: the original is inline asm. Draws one frame of an indexed-colour animation into a
     * 32-bit buffer (rows 0x200 bytes apart) at pos, showing only the part inside rect: each visible pixel is
     * written as its source byte shifted into the top byte (its depth); the palette is not used. Same stream
     * decoder as FUN_00464480. Quirks kept: a transparent run that crosses the left clip edge advances the
     * destination by 2 bytes per pixel instead of 4; a run ending exactly at the left clip edge leaves the decoder
     * in its left-margin state. */
    struct PortIdxBits bits;
    struct DrawLLSIdxFrame *frame;
    const unsigned char *src;
    unsigned int *row;
    unsigned int *dst;
    unsigned int depth;
    unsigned int width;
    unsigned int left;
    unsigned int rem;
    unsigned int run;
    unsigned int count;
    unsigned int leftover;
    unsigned int k;
    unsigned int c;
    int frame_index;
    int top;
    int height;
    int rows;
    int state;
    int is_fill;

    frame_index = PortLLSFrameIndex(lls);
    frame = PortIdxFrameAt(lls, frame_index);
    src = frame->pixels;
    bits.bits = 0;
    bits.left = 0;
    bits.next = (const unsigned int *)(frame->pixels + frame->pixel_count + (frame_index == 0 ? 0x200 : 0));
    row = (unsigned int *)((unsigned char *)zbuf + pos->x * 4 + pos->y * 0x200);
    left = rect->left;
    width = rect->right - rect->left;
    top = rect->top;
    height = rect->bottom - rect->top;
    is_fill = 0;
    for (rows = top; rows > 0; rows--) {
        src = PortIdxSkipRow(&bits, src);
    }
    for (rows = height; rows != 0; rows--) {
        dst = row;
        if (left != 0) {
            rem = left;
            state = PORT_IDX_LEFT;
        } else {
            rem = width;
            state = PORT_IDX_BODY;
        }
        while (state != PORT_IDX_NEXT) {
            switch (state) {
            case PORT_IDX_LEFT:
                c = PortIdxCode(&bits);
                if (!(c & 2) || !(c & 1)) {
                    if (!(c & 2)) {
                        src++;
                    }
                    rem--;
                    if (rem == 0) {
                        rem = width;
                        state = PORT_IDX_BODY;
                    }
                    break;
                }
                run = PortIdxCount(&bits);
                if (run == 0) {
                    state = PORT_IDX_NEXT;
                    break;
                }
                c = PortIdxCode(&bits);
                if (c & 2) {
                    rem -= run;
                    if ((int)rem >= 0) {
                        break;
                    }
                    rem = 0 - rem;
                    dst = (unsigned int *)((unsigned char *)dst + rem * 2);
                    if ((int)(width - rem) < 0) {
                        state = PORT_IDX_END;
                    } else {
                        rem = width - rem;
                        state = PORT_IDX_BODY;
                    }
                } else if (c & 1) {
                    src++;
                    rem -= run;
                    if ((int)rem >= 0) {
                        break;
                    }
                    run = 0 - rem;
                    src--;
                    rem = width;
                    if (rem == 0) {
                        state = PORT_IDX_END;
                    } else {
                        is_fill = 1;
                        state = PORT_IDX_RUN;
                    }
                } else {
                    src += run;
                    rem -= run;
                    if ((int)rem >= 0) {
                        break;
                    }
                    src += (int)rem;
                    run = 0 - rem;
                    rem = width;
                    state = (rem == 0) ? PORT_IDX_END : PORT_IDX_COPY;
                }
                break;
            case PORT_IDX_BODY:
                c = PortIdxCode(&bits);
                if (!(c & 2)) {
                    run = 1;
                    state = PORT_IDX_COPY;
                    break;
                }
                if (!(c & 1)) {
                    rem--;
                    if (rem == 0) {
                        state = PORT_IDX_END;
                    } else {
                        dst++;
                    }
                    break;
                }
                run = PortIdxCount(&bits);
                if (run == 0) {
                    state = PORT_IDX_NEXT;
                    break;
                }
                c = PortIdxCode(&bits);
                if (c & 2) {
                    rem -= run;
                    if ((int)rem < 0) {
                        state = PORT_IDX_END;
                    } else {
                        dst += run;
                    }
                    break;
                }
                is_fill = c & 1;
                state = PORT_IDX_RUN;
                break;
            case PORT_IDX_RUN:
                if (!is_fill) {
                    state = PORT_IDX_COPY;
                    break;
                }
                depth = (unsigned int)*src++ << 24;
                if (rem <= run) {
                    for (k = 0; k < rem; k++) {
                        *dst++ = depth;
                    }
                    state = PORT_IDX_END;
                } else {
                    rem -= run;
                    for (k = 0; k < run; k++) {
                        *dst++ = depth;
                    }
                    state = PORT_IDX_BODY;
                }
                break;
            case PORT_IDX_COPY:
                if (rem <= run) {
                    count = rem;
                    leftover = run - rem;
                    for (k = 0; k < count; k++) {
                        *dst++ = (unsigned int)*src++ << 24;
                    }
                    src += leftover;
                    state = PORT_IDX_END;
                } else {
                    rem -= run;
                    for (k = 0; k < run; k++) {
                        *dst++ = (unsigned int)*src++ << 24;
                    }
                    state = PORT_IDX_BODY;
                }
                break;
            case PORT_IDX_END:
                src = PortIdxSkipRow(&bits, src);
                state = PORT_IDX_NEXT;
                break;
            }
        }
        row = (unsigned int *)((unsigned char *)row + 0x200);
    }
}

// FUNCTION: LEGOLAND 0x00464ee0
void __fastcall FUN_00464ee0(struct Sprite *sprite, RECT *rect, int *off) {
    /* Port [library:asm]: the original is inline asm in its 16-bit copy loops. Software BltFast: draws the part
     * of the sprite inside rect (relative to the sprite, then moved by the sprite's source offset) into the
     * software screen at off = {x, y}, skipping pixels equal to the transparent colour StoredTransparentColour. Animated
     * sprites (image types 2 and 3) go to their own blitters; type 0 images are 8-bit with a palette, others 16-bit.
     * Sprites with flag 0x20 are DirectDraw surfaces, locked for the copy. Moves *rect by the source offset. */
    struct VideoArg lock;
    struct Image fake;
    struct Image *image;
    const unsigned short *palette;
    const unsigned char *src8;
    const unsigned short *src16;
    unsigned short *dst;
    unsigned short colour;
    int pitch;
    int stride;
    int width;
    int height;
    int x;
    int y;

    if (sprite->flags & 0x20) {
        GetSprite((unsigned int *)&lock, sprite);
        fake.data = lock.bits;
        fake.width = (short)((short)lock.pitch / 2);
        fake.height = (short)lock.height;
        fake.field_14 = 1;
        image = &fake;
    } else {
        image = sprite->image;
        if (IsBadReadPtr(image->data, 1)) {
            // STRING: LEGOLAND 0x004b9d0c
            DebugTrace("BltFast:Sprite Not Available:%s\n", image->name);
            return;
        }
    }
    rect->left += (short)sprite->src_x;
    rect->top += (short)sprite->src_y;
    rect->right += (short)sprite->src_x;
    rect->bottom += (short)sprite->src_y;
    pitch = CurrentSurfaceDesc.lPitch;
    DAT_007fe9a8 = (unsigned int)((unsigned char *)CurrentSurfaceDesc.lpSurface + MousePos.y * pitch + MousePos.x * 2);
    if (image->field_14 == 2) {
        FUN_00464480((struct DrawLLS *)image->data, rect, (struct Point *)off);
        return;
    }
    if (image->field_14 == 3) {
        FUN_00466770((struct DrawLLS *)image->data, rect, (struct Point *)off);
        return;
    }
    width = rect->right - rect->left;
    height = rect->bottom - rect->top;
    dst = (unsigned short *)((unsigned char *)CurrentSurfaceDesc.lpSurface + off[0] * 2 + off[1] * pitch);
    if (image->field_14 == 0) {
        stride = (image->width + 3) & ~3;
        palette = (const unsigned short *)((unsigned char *)image->aux + 4);
        src8 = (const unsigned char *)image->data + rect->left + rect->top * stride;
        for (y = 0; y < height; y++) {
            for (x = 0; x < width; x++) {
                colour = palette[src8[x]];
                if (colour != StoredTransparentColour) {
                    dst[x] = colour;
                }
            }
            src8 += stride;
            dst = (unsigned short *)((unsigned char *)dst + pitch);
        }
    } else {
        stride = image->width * 2;
        src16 = (const unsigned short *)((unsigned char *)image->data + rect->left * 2 + rect->top * stride);
        for (y = 0; y < height; y++) {
            for (x = 0; x < width; x++) {
                colour = src16[x];
                if (colour != StoredTransparentColour) {
                    dst[x] = colour;
                }
            }
            src16 = (const unsigned short *)((const unsigned char *)src16 + stride);
            dst = (unsigned short *)((unsigned char *)dst + pitch);
        }
    }
    if (sprite->flags & 0x20) {
        ReleaseSprite((struct Sprite *)&lock);
    }
}

// The original fills the rows with an inline __asm block (pusha; rep stosw per row; popa) that
// reads its loop bounds from the globals below. This is the C equivalent: same effect, but it
// cannot byte-match without __asm.
// FUNCTION: LEGOLAND 0x004651d0
LEGO_EXPORT void SoftPrint_Clear(void) {
    unsigned short colour = GetTransparentColour();
    unsigned short *row;
    int y;
    int x;

    DAT_007fea14 = CurrentSurfaceDesc.dwHeight;
    DAT_007fea1c = CurrentSurfaceDesc.dwWidth;
    DAT_007fe9a4 = DAT_007fea1c;
    row = CurrentSurfaceDesc.lpSurface;
    for (y = DAT_007fea14; y != 0; y--) {
        for (x = 0; x < DAT_007fe9a4; x++) {
            row[x] = colour;
        }
        row = (unsigned short *)((char *)row + CurrentSurfaceDesc.lPitch);
    }
}

// FUNCTION: LEGOLAND 0x00465240
void FUN_00465240(struct DrawLLS *lls, RECT *rect, struct Point *pos) {
    /* Port [library:asm]: the original is inline asm. Same as FUN_00464480 (indexed-colour animation frame drawn
     * into the software screen at pos, clipped to rect) but every pixel is ANDed with the colour mask DAT_007fe998
     * (the tint of SoftPrint_XBltFast), the palette is never overridden, and the cursor test is stricter. Differences
     * kept: with an overridden frame the palette is re-read from the current frame only if the animation's own
     * frame is 0; the delta frame of a two-frame animation comes from lls->frame, not the override. */
    struct PortIdxBits bits;
    struct DrawLLSIdxFrame *first;
    struct DrawLLSIdxFrame *frame;
    const unsigned short *palette;
    const unsigned char *src;
    unsigned short *row;
    unsigned short *dst;
    unsigned short *cursor;
    unsigned short colour;
    unsigned short tint;
    unsigned int width;
    unsigned int left;
    unsigned int rem;
    unsigned int run;
    unsigned int count;
    unsigned int leftover;
    unsigned int k;
    unsigned int c;
    int frame_index;
    int passes;
    int pass;
    int pitch;
    int top;
    int height;
    int rows;
    int state;
    int is_fill;
    int with_palette;
    int hit;

    frame_index = PortLLSFrameIndex(lls);
    passes = (lls->flags & 1) ? 2 : 1;
    pitch = CurrentSurfaceDesc.lPitch;
    cursor = (unsigned short *)DAT_007fe9a8;
    tint = (unsigned short)DAT_007fe998;
    left = rect->left;
    width = rect->right - rect->left;
    top = rect->top;
    height = rect->bottom - rect->top;
    palette = NULL;
    is_fill = 0;
    hit = 0;
    first = PortIdxFrameAt(lls, 0);
    for (pass = 0; pass < passes; pass++) {
        if (passes == 1) {
            frame = PortIdxFrameAt(lls, frame_index);
            palette = (const unsigned short *)(first->pixels + first->pixel_count);
            with_palette = (lls->frame == 0);
            if (with_palette) {
                palette = (const unsigned short *)(frame->pixels + frame->pixel_count);
            }
        } else if (pass == 0) {
            frame = first;
            palette = (const unsigned short *)(first->pixels + first->pixel_count);
            with_palette = 1;
        } else {
            frame = PortIdxFrameAt(lls, lls->frame + 1);
            with_palette = 0;
        }
        src = frame->pixels;
        bits.bits = 0;
        bits.left = 0;
        bits.next = (const unsigned int *)(frame->pixels + frame->pixel_count + (with_palette ? 0x200 : 0));
        row = (unsigned short *)((unsigned char *)CurrentSurfaceDesc.lpSurface + pos->x * 2 + pos->y * pitch);
        for (rows = top; rows > 0; rows--) {
            src = PortIdxSkipRow(&bits, src);
        }
        for (rows = height; rows != 0; rows--) {
            dst = row;
            if (left != 0) {
                rem = left;
                state = PORT_IDX_LEFT;
            } else {
                rem = width;
                state = PORT_IDX_BODY;
            }
            while (state != PORT_IDX_NEXT) {
                switch (state) {
                case PORT_IDX_LEFT:
                    c = PortIdxCode(&bits);
                    if (!(c & 2) || !(c & 1)) {
                        if (!(c & 2)) {
                            src++;
                        }
                        rem--;
                        if (rem == 0) {
                            rem = width;
                            state = PORT_IDX_BODY;
                        }
                        break;
                    }
                    run = PortIdxCount(&bits);
                    if (run == 0) {
                        state = PORT_IDX_NEXT;
                        break;
                    }
                    c = PortIdxCode(&bits);
                    if (c & 2) {
                        if (rem > run) {
                            rem -= run;
                            break;
                        }
                        rem = run - rem;
                        dst += rem;
                        if (width <= rem) {
                            state = PORT_IDX_END;
                        } else {
                            rem = width - rem;
                            state = PORT_IDX_BODY;
                        }
                    } else if (c & 1) {
                        src++;
                        if (rem > run) {
                            rem -= run;
                            break;
                        }
                        run = run - rem;
                        rem = width;
                        if (rem == 0) {
                            state = PORT_IDX_END;
                            break;
                        }
                        hit = (dst <= cursor);
                        if (run != 0) {
                            src--;
                            is_fill = 1;
                            state = PORT_IDX_RUN;
                        } else {
                            state = PORT_IDX_BODY;
                        }
                    } else {
                        src += run;
                        if (rem > run) {
                            rem -= run;
                            break;
                        }
                        src += (int)(rem - run);
                        run = run - rem;
                        rem = width;
                        if (rem == 0) {
                            state = PORT_IDX_END;
                            break;
                        }
                        hit = (dst <= cursor);
                        state = (run != 0) ? PORT_IDX_COPY : PORT_IDX_BODY;
                    }
                    break;
                case PORT_IDX_BODY:
                    c = PortIdxCode(&bits);
                    if (!(c & 2)) {
                        run = 1;
                        hit = (dst == cursor);
                        state = PORT_IDX_COPY;
                        break;
                    }
                    if (!(c & 1)) {
                        rem--;
                        if (rem == 0) {
                            state = PORT_IDX_END;
                        } else {
                            dst++;
                        }
                        break;
                    }
                    run = PortIdxCount(&bits);
                    if (run == 0) {
                        state = PORT_IDX_NEXT;
                        break;
                    }
                    c = PortIdxCode(&bits);
                    hit = (dst <= cursor);
                    if (c & 2) {
                        rem -= run;
                        if ((int)rem < 0) {
                            state = PORT_IDX_END;
                        } else {
                            dst += run;
                        }
                        break;
                    }
                    is_fill = c & 1;
                    state = PORT_IDX_RUN;
                    break;
                case PORT_IDX_RUN:
                    if (!is_fill) {
                        state = PORT_IDX_COPY;
                        break;
                    }
                    colour = (unsigned short)(palette[*src++] & tint);
                    if (rem <= run) {
                        count = rem;
                        state = PORT_IDX_END;
                    } else {
                        count = run;
                        rem -= run;
                        state = PORT_IDX_BODY;
                    }
                    if (count != 0) {
                        for (k = 0; k < count; k++) {
                            *dst++ = colour;
                        }
                        if (hit && dst > cursor) {
                            DAT_007feb14 |= 1;
                        }
                    }
                    break;
                case PORT_IDX_COPY:
                    if (rem <= run) {
                        count = rem;
                        leftover = run - rem;
                        state = PORT_IDX_END;
                    } else {
                        count = run;
                        leftover = 0;
                        rem -= run;
                        state = PORT_IDX_BODY;
                    }
                    if (count != 0) {
                        for (k = 0; k < count; k++) {
                            *dst++ = (unsigned short)(palette[*src++] & tint);
                        }
                        if (hit && dst > cursor) {
                            DAT_007feb14 |= 1;
                        }
                    }
                    src += leftover;
                    break;
                case PORT_IDX_END:
                    src = PortIdxSkipRow(&bits, src);
                    state = PORT_IDX_NEXT;
                    break;
                }
            }
            row = (unsigned short *)((unsigned char *)row + pitch);
        }
    }
}

// FUNCTION: LEGOLAND 0x00465850
void FUN_00465850(struct AviFrame *frame) {
    unsigned char *dst;
    unsigned char *next;
    unsigned short *row;
    unsigned short *s;
    unsigned short *p0;
    unsigned short *p1;
    unsigned short v;
    int width;
    int height;
    int half;
    int rest;
    int y;
    int x;

    dst = CurrentSurfaceDesc.lpSurface;
    DAT_006681ec = (DAT_006681ec != dst) ? dst : 0;
    height = frame->height;
    width = frame->width;
    rest = lpConfig->screen_height - height * 2;
    half = rest / 2;
    rest = rest - half;
    row = frame->pixels + (height - 1) * width;
    for (y = half; y > 0; y--) {
        memset(dst, 0, 0x500);
        dst += CurrentSurfaceDesc.lPitch;
    }
    for (y = height; y != 0; y--) {
        next = dst + CurrentSurfaceDesc.lPitch;
        s = row;
        p0 = (unsigned short *)dst;
        p1 = (unsigned short *)next;
        for (x = width; x > 0; x--) {
            v = *s;
            if (DisplayPixelFormat == 2) {
                v = (v & 0x1f) | (v & 0xffe0) << 1;
            }
            p0[0] = v;
            p1[0] = v;
            p0[1] = v;
            p1[1] = v;
            s++;
            p0 += 2;
            p1 += 2;
        }
        row -= width;
        dst += CurrentSurfaceDesc.lPitch * 2;
    }
    dst = next + CurrentSurfaceDesc.lPitch;
    for (y = rest; y > 0; y--) {
        memset(dst, 0, 0x500);
        dst += CurrentSurfaceDesc.lPitch;
    }
}

// FUNCTION: LEGOLAND 0x004659a0
void FUN_004659a0(struct AviFrame *param_1, int param_2, int param_3) {
    int height;
    int width;
    int last;
    unsigned short *src;
    unsigned short *dst;
    int offset;
    int i;
    int j;
    unsigned short *p;
    short v;

    height = param_1->height;
    width = param_1->width;
    dst = (unsigned short *)((char *)CurrentSurfaceDesc.lpSurface + CurrentSurfaceDesc.lPitch * param_3 + param_2 * 2);
    last = height - 1;
    src = param_1->pixels + last * width;
    if (height != 0) {
        i = last + 1;
        do {
            if (width > 0) {
                offset = (char *)src - (char *)dst;
                p = dst;
                j = width;
                do {
                    v = *(short *)(offset + (char *)p);
                    if (DisplayPixelFormat == 2) {
                        v = (v & 0x1f) | (v & ~0x1f) << 1;
                    }
                    *p = v;
                    p++;
                    j--;
                } while (j != 0);
            }
            dst = (unsigned short *)((char *)dst + CurrentSurfaceDesc.lPitch);
            src -= width;
            i--;
        } while (i != 0);
    }
}

/* Port [library:asm]: draws one frame of a 16-bit animation darkened to half brightness through FUN_00468410
 * (the original sets up its 11 arguments by hand in SoftPrint_XBltFast). */
static void PortHalfFrame(unsigned short *dst, struct DrawLLSFrame *frame, int height, int pitch, int top, int left, int width, unsigned short *cursor) {
    unsigned char *runs = (unsigned char *)(frame->pixels + frame->pixel_count);
    unsigned int *mask = (unsigned int *)(runs + frame->run_bytes);

    FUN_00468410(dst, frame->pixels, runs, mask, height, pitch, top, left, width, 0, cursor);
}

// FUNCTION: LEGOLAND 0x00465a40
LEGO_EXPORT void SoftPrint_XBltFast(struct Sprite *sprite, RECT *src, RECT *dst, unsigned int tint) {
    /* Port [library:asm]: the original is inline asm in its 16-bit copy loops. Same as FUN_00464ee0 (software
     * BltFast of the part of the sprite inside src, moved by the sprite's source offset, to dst->left/top) but the
     * colour of every pixel is ANDed with a mask from tint (0x00RRGGBB, via GetNearestColour) stored in
     * DAT_007fe998. Type 3 animations with tint alpha (the top byte) are drawn at half brightness instead, with the
     * mask ~GetNearestColour(15, 15, 15), by FUN_00468410; without alpha they use the plain FUN_00465ee0. */
    struct VideoArg lock;
    struct Image fake;
    struct Image *image;
    struct DrawLLS *lls;
    struct DrawLLSFrame *frame;
    const unsigned short *palette;
    const unsigned char *src8;
    const unsigned short *src16;
    unsigned short *out;
    unsigned short colour;
    unsigned short mask;
    int pitch;
    int stride;
    int width;
    int height;
    int frame_index;
    int i;
    int x;
    int y;

    if (sprite->flags & 0x20) {
        GetSprite((unsigned int *)&lock, sprite);
        fake.data = lock.bits;
        fake.width = (short)((short)lock.pitch / 2);
        fake.height = (short)lock.height;
        fake.field_14 = 1;
        image = &fake;
    } else {
        image = sprite->image;
        if (IsBadReadPtr(image->data, 1)) {
            DebugTrace("BltFast:Sprite Not Available:%s\n", image->name);
            return;
        }
    }
    DAT_007fe998 = GetNearestColour((tint >> 16) & 0xff, (tint >> 8) & 0xff, tint & 0xff);
    src->left += (short)sprite->src_x;
    src->top += (short)sprite->src_y;
    src->right += (short)sprite->src_x;
    src->bottom += (short)sprite->src_y;
    pitch = CurrentSurfaceDesc.lPitch;
    DAT_007fe9a8 = (unsigned int)((unsigned char *)CurrentSurfaceDesc.lpSurface + MousePos.y * pitch + MousePos.x * 2);
    if (image->field_14 == 2) {
        FUN_00465240((struct DrawLLS *)image->data, src, (struct Point *)dst);
        return;
    }
    if (image->field_14 == 3) {
        lls = (struct DrawLLS *)image->data;
        if (!(tint & 0xff000000)) {
            FUN_00465ee0(lls, src, (struct Point *)dst);
            return;
        }
        DAT_007fe998 = ~GetNearestColour(15, 15, 15);
        width = src->right - src->left;
        height = src->bottom - src->top;
        frame_index = PortLLSFrameIndex(lls);
        out = (unsigned short *)((unsigned char *)CurrentSurfaceDesc.lpSurface + dst->top * pitch + (dst->left - src->left) * 2);
        frame = (struct DrawLLSFrame *)(lls + 1);
        if (lls->flags & 1) {
            PortHalfFrame(out, frame, height, pitch, src->top, src->left, width, (unsigned short *)DAT_007fe9a8);
            for (i = lls->frame + 1; i != 0; i--) {
                frame = (struct DrawLLSFrame *)((unsigned char *)frame + frame->size);
            }
            PortHalfFrame(out, frame, height, pitch, src->top, src->left, width, (unsigned short *)DAT_007fe9a8);
        } else {
            for (i = frame_index; i != 0; i--) {
                frame = (struct DrawLLSFrame *)((unsigned char *)frame + frame->size);
            }
            PortHalfFrame(out, frame, height, pitch, src->top, src->left, width, (unsigned short *)DAT_007fe9a8);
        }
        return;
    }
    mask = (unsigned short)DAT_007fe998;
    width = src->right - src->left;
    height = src->bottom - src->top;
    out = (unsigned short *)((unsigned char *)CurrentSurfaceDesc.lpSurface + dst->left * 2 + dst->top * pitch);
    if (image->field_14 == 0) {
        stride = (image->width + 3) & ~3;
        palette = (const unsigned short *)((unsigned char *)image->aux + 4);
        src8 = (const unsigned char *)image->data + src->left + src->top * stride;
        for (y = 0; y < height; y++) {
            for (x = 0; x < width; x++) {
                colour = palette[src8[x]];
                if (colour != StoredTransparentColour) {
                    out[x] = (unsigned short)(colour & mask);
                }
            }
            src8 += stride;
            out = (unsigned short *)((unsigned char *)out + pitch);
        }
    } else {
        stride = image->width * 2;
        src16 = (const unsigned short *)((unsigned char *)image->data + src->left * 2 + src->top * stride);
        for (y = 0; y < height; y++) {
            for (x = 0; x < width; x++) {
                colour = src16[x];
                if (colour != StoredTransparentColour) {
                    out[x] = (unsigned short)(colour & mask);
                }
            }
            src16 = (const unsigned short *)((const unsigned char *)src16 + stride);
            out = (unsigned short *)((unsigned char *)out + pitch);
        }
    }
    if (sprite->flags & 0x20) {
        ReleaseSprite((struct Sprite *)&lock);
    }
}

// FUNCTION: LEGOLAND 0x00465ee0
void FUN_00465ee0(struct DrawLLS *lls, RECT *clip, struct Point *pos) {
    struct DrawLLSFrame *frame = (struct DrawLLSFrame *)(lls + 1);
    unsigned short *dst;
    unsigned short *pixels;
    unsigned char *runs;
    unsigned int *mask;
    int frame_index;
    int i;
    int off;

    DAT_007fe9a4 = CurrentSurfaceDesc.lPitch;
    SpriteClipOrigin.left = clip->left;
    DrawClipExtent.width = clip->right - clip->left;
    SpriteClipOrigin.top = clip->top;
    DrawClipExtent.height = clip->bottom - clip->top;
    if ((int)OverrideFrame < 0) {
        frame_index = lls->frame;
    } else {
        frame_index = (int)OverrideFrame;
    }
    if (frame_index >= lls->frame_count) {
        frame_index = lls->frame_count - 1;
    }
    DAT_007fe9a8 = (unsigned int)((unsigned char *)CurrentSurfaceDesc.lpSurface + MousePos.y * CurrentSurfaceDesc.lPitch + MousePos.x * 2);
    dst = (unsigned short *)((unsigned char *)CurrentSurfaceDesc.lpSurface + pos->y * CurrentSurfaceDesc.lPitch + (pos->x - SpriteClipOrigin.left) * 2);
    if (lls->flags & 1) {
        pixels = frame->pixels;
        off = frame->pixel_count * 2 + 0x10;
        runs = (unsigned char *)frame + off;
        mask = (unsigned int *)((unsigned char *)frame + (frame->run_bytes + off));
        FUN_00468040(dst, pixels, runs, mask, DrawClipExtent.height, CurrentSurfaceDesc.lPitch, SpriteClipOrigin.top, SpriteClipOrigin.left, DrawClipExtent.width, 0, (unsigned short *)DAT_007fe9a8);
        i = lls->frame + 1;
        while (i-- != 0) {
            frame = (struct DrawLLSFrame *)((unsigned char *)frame + frame->size);
        }
        pixels = frame->pixels;
        off = frame->pixel_count * 2 + 0x10;
        runs = (unsigned char *)frame + off;
        mask = (unsigned int *)((unsigned char *)frame + (frame->run_bytes + off));
        FUN_00468040(dst, pixels, runs, mask, DrawClipExtent.height, CurrentSurfaceDesc.lPitch, SpriteClipOrigin.top, SpriteClipOrigin.left, DrawClipExtent.width, 0, (unsigned short *)DAT_007fe9a8);
    } else {
        i = frame_index;
        while (i-- != 0) {
            frame = (struct DrawLLSFrame *)((unsigned char *)frame + frame->size);
        }
        pixels = frame->pixels;
        off = frame->pixel_count * 2 + 0x10;
        runs = (unsigned char *)frame + off;
        mask = (unsigned int *)((unsigned char *)frame + (frame->run_bytes + off));
        FUN_00468040(dst, pixels, runs, mask, DrawClipExtent.height, CurrentSurfaceDesc.lPitch, SpriteClipOrigin.top, SpriteClipOrigin.left, DrawClipExtent.width, 0, (unsigned short *)DAT_007fe9a8);
    }
}

// FUNCTION: LEGOLAND 0x00466080
int FlipFrame(void) {
    DWORD tick;
    int result;
    int frames;
    LPDIRECTDRAWSURFACE primary;
    LPDIRECTDRAWSURFACE back;

    LLSAuto();
    if (lpConfig->field_1e != 0 && DAT_00668148 != 0) {
        PushRenderingStatusAndLockVideoSurface();
        PrintSprite(DAT_00668148, MousePos.x, MousePos.y, 0, 0);
        PopRenderingStatus();
    }
    tick = GetTickCount();
    while (tick - LastPresentTicks < 0x1c) {
        tick = GetTickCount();
    }
    LastPresentTicks = GetTickCount();
    primary = PrimarySurface;
    result = IDirectDrawSurface_Flip(primary, NULL, 1);
    while (result != 0) {
        if (result == 0x887601c2) {
            IDirectDrawSurface_Restore(PrimarySurface);
            IDirectDrawSurface_Restore(OffscreenSurface);
            return 0;
        }
        if (result != 0x887601ae && result != 0x8876021c) {
            return 0;
        }
        primary = PrimarySurface;
        result = IDirectDrawSurface_Flip(primary, NULL, 1);
    }
    back = OffscreenSurface;
    result = IDirectDrawSurface_GetFlipStatus(back, 2);
    while (result != 0) {
        back = OffscreenSurface;
        result = IDirectDrawSurface_GetFlipStatus(back, 2);
    }
    tick = GetTickCount();
    FrameNumber = FrameNumber + 1;
    FramesThisSecond = FramesThisSecond + 1;
    if (tick - FpsWindowStartTicks >= 0x3e8) {
        FramesPerSecond = FramesThisSecond;
        FramesThisSecond = 0;
        FpsWindowStartTicks = tick;
    }
    LastFrameMS = tick - LastFrameTicks;
    LastFrameTicks = tick;
    return 1;
}

#ifdef LEGOLAND_PORT
/* [library:video] port-only helper. Back from Alt+Tab, Windows can leave the screen in the desktop's mode (32-bit)
 * instead of the game's: the 16-bit frame then came out shifted and in the wrong colours. When the game's window
 * is in front and the mode isn't the one it set, set it again and restore the surfaces. */
static void KeepDisplayMode(void) {
    DDSURFACEDESC desc;

    if (WinDebugMode != 0 || DisplayPixelFormat == 0 || GetForegroundWindow() != WNDENV_Gethwnd()) {
        return;
    }
    desc.dwSize = sizeof(desc);
    if (IDirectDraw2_GetDisplayMode(DDRAWENV.ddraw2, &desc) != 0) {
        return;
    }
    if ((int)desc.dwWidth == PortDisplayWidth && (int)desc.dwHeight == PortDisplayHeight &&
        (int)desc.ddpfPixelFormat.dwRGBBitCount == PortDisplayBpp) {
        return;
    }
    DebugTrace("KeepDisplayMode: display is %lux%lu %lu bpp, setting %dx%d %d bpp again", desc.dwWidth, desc.dwHeight,
        desc.ddpfPixelFormat.dwRGBBitCount, PortDisplayWidth, PortDisplayHeight, PortDisplayBpp);
    IDirectDraw2_SetDisplayMode(DDRAWENV.ddraw2, PortDisplayWidth, PortDisplayHeight, PortDisplayBpp, 0, 0);
    IDirectDrawSurface_Restore(PrimarySurface);
    IDirectDrawSurface_Restore(OffscreenSurface);
}
#endif

// FUNCTION: LEGOLAND 0x004661d0
int BlitFrameToWindow(void) {
    RECT dst;
    union RectPoints client;
    DWORD tick;
    LPDIRECTDRAWSURFACE surface;
    int result;
    int frames;

    dst.left = 0;
    dst.top = 0;
    dst.right = 0x280;
    dst.bottom = 0x1e0;
    LLSAuto();
    if (lpConfig->field_1e != 0 && DAT_00668148 != 0) {
        PushRenderingStatusAndLockVideoSurface();
        PrintSprite(DAT_00668148, MousePos.x, MousePos.y, 0, 0);
        PopRenderingStatus();
    }
    tick = GetTickCount();
    while (tick - LastPresentTicks < 0x1c) {
        tick = GetTickCount();
    }
    LastPresentTicks = GetTickCount();
#ifdef LEGOLAND_PORT
    KeepDisplayMode();
    PortDisplayCaptureBefore(WNDENV_Gethwnd()); /* [library:video] Ctrl+F12 (port_display.h) */
    if (PortDisplayCustomPresent()) {
        /* [library:video] windowed (RGB565 frame through GDI) or another resolution: the frame is scaled to the
         * window or display mode (port_display.c) */
        result = PortPresent(WNDENV_Gethwnd(), OffscreenSurface, PrimarySurface, NULL);
        if (result == 0x887601c2) {
            IDirectDrawSurface_Restore(PrimarySurface);
            IDirectDrawSurface_Restore(OffscreenSurface);
            result = PortPresent(WNDENV_Gethwnd(), OffscreenSurface, PrimarySurface, NULL);
        }
        PortDisplayCapture(WNDENV_Gethwnd(), OffscreenSurface); /* [library:video] Ctrl+F12 (port_display.h) */
    } else
#endif
    {
        GetClientRect(WNDENV_Gethwnd(), &client.rect);
        ClientToScreen(WNDENV_Gethwnd(), &client.pt[0]);
        OffsetRect(&dst, client.rect.left, client.rect.top);
        surface = PrimarySurface;
        result = IDirectDrawSurface_Blt(surface, &dst, OffscreenSurface, NULL, 0x1000000, NULL);
        if (result == 0x887601c2) {
            IDirectDrawSurface_Restore(PrimarySurface);
            result = IDirectDrawSurface_Blt(PrimarySurface, &dst, OffscreenSurface, NULL, 0x1000000, NULL);
        }
    }
    if (result != 0) {
        return 0;
    }
    tick = GetTickCount();
    FrameNumber = FrameNumber + 1;
    FramesThisSecond = FramesThisSecond + 1;
    if (tick - FpsWindowStartTicks >= 0x3e8) {
        FramesPerSecond = FramesThisSecond;
        FramesThisSecond = 0;
        FpsWindowStartTicks = tick;
    }
    LastFrameMS = tick - LastFrameTicks;
    LastFrameTicks = tick;
    return 1;
}

// FUNCTION: LEGOLAND 0x00466360
void LoadWatchSprite(int a, int b) {
    short w;
    short h;

    if (WatchSprite == NULL) {
        // STRING: LEGOLAND 0x004b9d30
        WatchSprite = LoadSprite("Watch.lls", 4);
        if (WatchSprite == NULL) {
            return;
        }
    }
    WatchActive = 1;
    w = WatchSprite->width;
    h = WatchSprite->height;
    WatchRect.left = a;
    WatchRect.top = b;
    WatchRect.right = w + a;
    WatchRect.bottom = h + b;
}

// FUNCTION: LEGOLAND 0x004663c0
void UnloadWatchSprite(void) {
    if (WatchSprite != 0) {
        KillSprite(WatchSprite);
        WatchSprite = 0;
    }
    WatchActive = 0;
}

// FUNCTION: LEGOLAND 0x004663f0
void DrawWatchSprite(void) {
    union RectPoints cursor;
    LPDIRECTDRAWSURFACE surface;
    struct Image *image;

#ifdef LEGOLAND_PORT
    PortHeartbeat(); /* [port] loading screens call this between steps: not a freeze */
#endif
    if (WatchActive != 0) {
        if ((int)(GetTicks() - LastWatchDrawTicks) > 0xc8) {
            cursor.rect.left = WatchRect.left;
            cursor.rect.top = WatchRect.top;
            cursor.rect.right = WatchRect.right;
            cursor.rect.bottom = WatchRect.bottom;
            LastWatchDrawTicks = GetTicks();
            image = WatchSprite->image;
            LLSAdvanceFrame((struct LLS *)image->data);
            PushRenderingStatusAndLockVideoSurface();
            PrintSprite(WatchSprite, WatchRect.left, WatchRect.top, 0, 0);
            PopRenderingStatus();
#ifdef LEGOLAND_PORT
            if (PortDisplayCustomPresent()) {
                /* [library:video] only the watch's part of the frame, scaled (port_display.c) */
                PortPresent(WNDENV_Gethwnd(), OffscreenSurface, PrimarySurface, &WatchRect);
                return;
            }
#endif
            ClientToScreen(WNDENV_Gethwnd(), &cursor.pt[0]);
            ClientToScreen(WNDENV_Gethwnd(), &cursor.pt[1]);
            surface = PrimarySurface;
            if (IDirectDrawSurface_Blt(surface, &cursor.rect, OffscreenSurface, &WatchRect, 0x1000000, NULL) == 0x887601c2) {
                IDirectDrawSurface_Restore(PrimarySurface);
                IDirectDrawSurface_Blt(PrimarySurface, &cursor, OffscreenSurface, NULL, 0x1000000, NULL);
            }
        }
    }
}

// FUNCTION: LEGOLAND 0x00466500
LEGO_EXPORT int RenderingComplete(void) {
    int result;

    ProcessSystemEvents();
    FlushTextCells(0);
#if defined(_MSC_VER) && (_MSC_VER <= 1200) && defined(_M_IX86)
    __asm {
        pushad
        rdtsc
        mov ebx, DAT_00813a18
        sub eax, ebx
        mov DAT_00813a18, eax
        add DAT_00813a2c, eax
        popad
    }
#else
    {
        int tsc = (int)GetTickCount();
        tsc -= DAT_00813a18;
        DAT_00813a18 = tsc;
        DAT_00813a2c += tsc;
    }
#endif
    LastRenderingCompleteTick = GetTickCount();
    result = BlitFrameFunc();
    DAT_00813a2c = 0;
#if defined(_MSC_VER) && (_MSC_VER <= 1200) && defined(_M_IX86)
    __asm {
        pushad
        rdtsc
        mov DAT_00813a18, eax
        popad
    }
#else
    DAT_00813a18 = (int)GetTickCount();
#endif
    return result;
}

// FUNCTION: LEGOLAND 0x00466560
LEGO_EXPORT void PushSetTarget(struct Sprite *sprite) {
    LPDIRECTDRAWSURFACE surface;
    int locked;

    locked = VideoSurfaceLocked;
    DAT_00668164[RenderingStatusStackDepth] = VideoSurfaceLocked;
    RenderingStatusStackDepth = RenderingStatusStackDepth + 1;
    if (locked != 0) {
        surface = renderEngine;
        if (IDirectDrawSurface_Unlock(surface, CurrentSurfaceDesc.lpSurface) == 0x887601c2) {
            IDirectDrawSurface_Restore(renderEngine);
            IDirectDrawSurface_Unlock(renderEngine, CurrentSurfaceDesc.lpSurface);
        }
    }
    VideoSurfaceLocked = 0;
    renderEngineTargets[renderEngineTargetIdx] = renderEngine;
    renderEngine = sprite->surface;
    renderEngineTargetIdx++;
    FUN_004640f0();
}

// FUNCTION: LEGOLAND 0x00466600
LEGO_EXPORT void PopTarget(void) {
    PopRenderingStatus();
    renderEngineTargetIdx--;
    renderEngine = renderEngineTargets[renderEngineTargetIdx];
    PopRenderingStatus();
    if (renderEngineTargetIdx >= 0) {
        return;
    }
    exit(2);
}

// FUNCTION: LEGOLAND 0x00466640
LEGO_EXPORT int RecreateSprite(struct Sprite *sprite) {
    DDSURFACEDESC desc;
    DDCOLORKEY colorkey;
    LPDIRECTDRAW2 ddraw2;
    LPDIRECTDRAWSURFACE surface;

    desc.dwSize = 0x6c;
    desc.dwWidth = (short)sprite->width;
    desc.dwFlags = 7;
    desc.ddsCaps.dwCaps = 0x40;
    desc.dwHeight = (short)sprite->height;
    SetWindowedSurfaceFormat(&desc); /* [library:video] sprites match the RGB565 back buffer */
    for (;;) {
        if ((sprite->flags & 0x10) == 0) {
            ddraw2 = DDRAWENV.ddraw2;
            if (IDirectDraw2_CreateSurface(ddraw2, &desc, &sprite->surface, NULL) == 0) {
                break;
            }
        }
        desc.dwWidth = (short)sprite->width;
        desc.dwHeight = (short)sprite->height;
        desc.dwSize = 0x6c;
        desc.dwFlags = 7;
        desc.ddsCaps.dwCaps = 0x840;
        SetWindowedSurfaceFormat(&desc); /* [library:video] */
        ddraw2 = DDRAWENV.ddraw2;
        if (IDirectDraw2_CreateSurface(ddraw2, &desc, &sprite->surface, NULL) != 0) {
            ddraw2 = DDRAWENV.ddraw2;
            if (IDirectDraw2_Compact(ddraw2) == 0) {
                IDirectDraw2_CreateSurface(DDRAWENV.ddraw2, &desc, &sprite->surface, NULL);
            }
            return 0;
        }
        break;
    }
    if ((sprite->flags & 0x80) != 0) {
        colorkey.dwColorSpaceLowValue = GetNearestColour(0xff, 0, 0xff);
        surface = sprite->surface;
        colorkey.dwColorSpaceHighValue = colorkey.dwColorSpaceLowValue;
        IDirectDrawSurface_SetColorKey(surface, 8, &colorkey);
        return 1;
    }
    colorkey.dwColorSpaceLowValue = GetTransparentColour();
    surface = sprite->surface;
    colorkey.dwColorSpaceHighValue = colorkey.dwColorSpaceLowValue;
    IDirectDrawSurface_SetColorKey(surface, 8, &colorkey);
    return 1;
}

// FUNCTION: LEGOLAND 0x00466770
void FUN_00466770(struct DrawLLS *lls, RECT *clip, struct Point *pos) {
    struct DrawLLSFrame *frame = (struct DrawLLSFrame *)(lls + 1);
    unsigned short *dst;
    unsigned short *pixels;
    unsigned char *runs;
    unsigned int *mask;
    int frame_index;
    int hit;
    int i;

    DAT_007fe9a4 = CurrentSurfaceDesc.lPitch;
    SpriteClipOrigin.left = clip->left;
    DrawClipExtent.width = clip->right - clip->left;
    SpriteClipOrigin.top = clip->top;
    DrawClipExtent.height = clip->bottom - clip->top;
    frame_index = (int)OverrideFrame;
    if (frame_index < 0) {
        frame_index = lls->frame;
    }
    if (frame_index >= lls->frame_count) {
        frame_index = lls->frame_count - 1;
    }
    hit = 0;
    DAT_007fe9a8 = (unsigned int)((unsigned char *)CurrentSurfaceDesc.lpSurface + MousePos.y * CurrentSurfaceDesc.lPitch + MousePos.x * 2);
    if (MousePos.x >= pos->x && MousePos.x <= pos->x + lls->width && MousePos.y >= pos->y && MousePos.y <= pos->y + lls->height) {
        hit = 1;
    }
    dst = (unsigned short *)((unsigned char *)CurrentSurfaceDesc.lpSurface + pos->y * CurrentSurfaceDesc.lPitch + (pos->x - SpriteClipOrigin.left) * 2);
    if (lls->flags & 1) {
        pixels = frame->pixels;
        runs = (unsigned char *)frame + frame->pixel_count * 2 + 0x10;
        mask = (unsigned int *)(runs + frame->run_bytes);
        if (hit) {
            if (lls->width <= DrawClipExtent.width - SpriteClipOrigin.left) {
                FUN_00467640(dst, pixels, runs, mask, DrawClipExtent.height, CurrentSurfaceDesc.lPitch, SpriteClipOrigin.top, SpriteClipOrigin.left, DrawClipExtent.width, 0, (unsigned short *)DAT_007fe9a8);
            } else if (SpriteClipOrigin.left != 0) {
                if (lls->width - SpriteClipOrigin.left > DrawClipExtent.width) {
                    FUN_00466d80(dst, pixels, runs, mask, DrawClipExtent.height, CurrentSurfaceDesc.lPitch, SpriteClipOrigin.top, SpriteClipOrigin.left, DrawClipExtent.width, 0, (unsigned short *)DAT_007fe9a8);
                } else {
                    FUN_00467180(dst, pixels, runs, mask, DrawClipExtent.height, CurrentSurfaceDesc.lPitch, SpriteClipOrigin.top, SpriteClipOrigin.left, DrawClipExtent.width, 0, (unsigned short *)DAT_007fe9a8);
                }
            } else {
                FUN_004673f0(dst, pixels, runs, mask, DrawClipExtent.height, CurrentSurfaceDesc.lPitch, SpriteClipOrigin.top, SpriteClipOrigin.left, DrawClipExtent.width, 0, (unsigned short *)DAT_007fe9a8);
            }
        } else {
            if (lls->width <= DrawClipExtent.width - SpriteClipOrigin.left) {
                FUN_00467f00(dst, pixels, runs, mask, DrawClipExtent.height, CurrentSurfaceDesc.lPitch, SpriteClipOrigin.top);
            } else if (SpriteClipOrigin.left != 0) {
                if (lls->width - SpriteClipOrigin.left > DrawClipExtent.width) {
                    FUN_004677b0(dst, pixels, runs, mask, DrawClipExtent.height, CurrentSurfaceDesc.lPitch, SpriteClipOrigin.top, SpriteClipOrigin.left, DrawClipExtent.width, 0, (unsigned short *)DAT_007fe9a8);
                } else {
                    FUN_00467b00(dst, pixels, runs, mask, DrawClipExtent.height, CurrentSurfaceDesc.lPitch, SpriteClipOrigin.top, SpriteClipOrigin.left, DrawClipExtent.width, 0, (unsigned short *)DAT_007fe9a8);
                }
            } else {
                FUN_00467d10(dst, pixels, runs, mask, DrawClipExtent.height, CurrentSurfaceDesc.lPitch, SpriteClipOrigin.top, SpriteClipOrigin.left, DrawClipExtent.width, 0, (unsigned short *)DAT_007fe9a8);
            }
        }
        i = lls->frame + 1;
        while (i-- != 0) {
            frame = (struct DrawLLSFrame *)((unsigned char *)frame + frame->size);
        }
        pixels = frame->pixels;
        runs = (unsigned char *)frame + frame->pixel_count * 2 + 0x10;
        mask = (unsigned int *)(runs + frame->run_bytes);
        if (hit) {
            if (lls->width <= DrawClipExtent.width - SpriteClipOrigin.left) {
                FUN_00467640(dst, pixels, runs, mask, DrawClipExtent.height, CurrentSurfaceDesc.lPitch, SpriteClipOrigin.top, SpriteClipOrigin.left, DrawClipExtent.width, 0, (unsigned short *)DAT_007fe9a8);
            } else if (SpriteClipOrigin.left != 0) {
                if (lls->width - SpriteClipOrigin.left > DrawClipExtent.width) {
                    FUN_00466d80(dst, pixels, runs, mask, DrawClipExtent.height, CurrentSurfaceDesc.lPitch, SpriteClipOrigin.top, SpriteClipOrigin.left, DrawClipExtent.width, 0, (unsigned short *)DAT_007fe9a8);
                } else {
                    FUN_00467180(dst, pixels, runs, mask, DrawClipExtent.height, CurrentSurfaceDesc.lPitch, SpriteClipOrigin.top, SpriteClipOrigin.left, DrawClipExtent.width, 0, (unsigned short *)DAT_007fe9a8);
                }
            } else {
                FUN_004673f0(dst, pixels, runs, mask, DrawClipExtent.height, CurrentSurfaceDesc.lPitch, SpriteClipOrigin.top, SpriteClipOrigin.left, DrawClipExtent.width, 0, (unsigned short *)DAT_007fe9a8);
            }
        } else {
            if (lls->width <= DrawClipExtent.width - SpriteClipOrigin.left) {
                FUN_00467f00(dst, pixels, runs, mask, DrawClipExtent.height, CurrentSurfaceDesc.lPitch, SpriteClipOrigin.top);
            } else if (SpriteClipOrigin.left != 0) {
                if (lls->width - SpriteClipOrigin.left > DrawClipExtent.width) {
                    FUN_004677b0(dst, pixels, runs, mask, DrawClipExtent.height, CurrentSurfaceDesc.lPitch, SpriteClipOrigin.top, SpriteClipOrigin.left, DrawClipExtent.width, 0, (unsigned short *)DAT_007fe9a8);
                } else {
                    FUN_00467b00(dst, pixels, runs, mask, DrawClipExtent.height, CurrentSurfaceDesc.lPitch, SpriteClipOrigin.top, SpriteClipOrigin.left, DrawClipExtent.width, 0, (unsigned short *)DAT_007fe9a8);
                }
            } else {
                FUN_00467d10(dst, pixels, runs, mask, DrawClipExtent.height, CurrentSurfaceDesc.lPitch, SpriteClipOrigin.top, SpriteClipOrigin.left, DrawClipExtent.width, 0, (unsigned short *)DAT_007fe9a8);
            }
        }
    } else {
        i = frame_index;
        while (i-- != 0) {
            frame = (struct DrawLLSFrame *)((unsigned char *)frame + frame->size);
        }
        pixels = frame->pixels;
        runs = (unsigned char *)frame + frame->pixel_count * 2 + 0x10;
        mask = (unsigned int *)(runs + frame->run_bytes);
        if (hit) {
            if (lls->width <= DrawClipExtent.width - SpriteClipOrigin.left) {
                FUN_00467640(dst, pixels, runs, mask, DrawClipExtent.height, CurrentSurfaceDesc.lPitch, SpriteClipOrigin.top, SpriteClipOrigin.left, DrawClipExtent.width, 0, (unsigned short *)DAT_007fe9a8);
            } else if (SpriteClipOrigin.left != 0) {
                if (lls->width - SpriteClipOrigin.left > DrawClipExtent.width) {
                    FUN_00466d80(dst, pixels, runs, mask, DrawClipExtent.height, CurrentSurfaceDesc.lPitch, SpriteClipOrigin.top, SpriteClipOrigin.left, DrawClipExtent.width, 0, (unsigned short *)DAT_007fe9a8);
                } else {
                    FUN_00467180(dst, pixels, runs, mask, DrawClipExtent.height, CurrentSurfaceDesc.lPitch, SpriteClipOrigin.top, SpriteClipOrigin.left, DrawClipExtent.width, 0, (unsigned short *)DAT_007fe9a8);
                }
            } else {
                FUN_004673f0(dst, pixels, runs, mask, DrawClipExtent.height, CurrentSurfaceDesc.lPitch, SpriteClipOrigin.top, SpriteClipOrigin.left, DrawClipExtent.width, 0, (unsigned short *)DAT_007fe9a8);
            }
        } else {
            if (lls->width <= DrawClipExtent.width - SpriteClipOrigin.left) {
                FUN_00467f00(dst, pixels, runs, mask, DrawClipExtent.height, CurrentSurfaceDesc.lPitch, SpriteClipOrigin.top);
            } else if (SpriteClipOrigin.left != 0) {
                if (lls->width - SpriteClipOrigin.left > DrawClipExtent.width) {
                    FUN_004677b0(dst, pixels, runs, mask, DrawClipExtent.height, CurrentSurfaceDesc.lPitch, SpriteClipOrigin.top, SpriteClipOrigin.left, DrawClipExtent.width, 0, (unsigned short *)DAT_007fe9a8);
                } else {
                    FUN_00467b00(dst, pixels, runs, mask, DrawClipExtent.height, CurrentSurfaceDesc.lPitch, SpriteClipOrigin.top, SpriteClipOrigin.left, DrawClipExtent.width, 0, (unsigned short *)DAT_007fe9a8);
                }
            } else {
                FUN_00467d10(dst, pixels, runs, mask, DrawClipExtent.height, CurrentSurfaceDesc.lPitch, SpriteClipOrigin.top, SpriteClipOrigin.left, DrawClipExtent.width, 0, (unsigned short *)DAT_007fe9a8);
            }
        }
    }
}

/* Port: the run-length sprite format that FUN_00466d80 and its siblings draw (the original blits are inline asm).
 * A sprite frame is three streams: 16-bit pixels, a byte stream of run lengths, and a stream of 2-bit codes
 * (16 per dword, lowest bits first). Each row is a sequence of codes that ends with a run length of 0:
 *   0 or 1  one literal pixel, copied from the pixel stream
 *   2       one transparent pixel (nothing drawn, nothing consumed)
 *   3       a run: take a length n from the run stream (0 ends the row), then one more code:
 *           0 copy n pixels from the pixel stream, 1 fill n pixels with the next single pixel,
 *           2 or 3 skip n transparent pixels. */
typedef struct PortBlitStream {
    const unsigned short *pixels;
    const unsigned char *runs;
    const unsigned int *mask; /* code dword being read */
    unsigned int shift; /* bit offset of the next code in *mask */
} PortBlitStream;

/* Clip and hit-test options for PortBlitRuns. */
#define PORT_BLIT_LEFT 0x01 /* skip the first `left` pixels of every row */
#define PORT_BLIT_RIGHT 0x02 /* draw at most `width` pixels after that */
#define PORT_BLIT_HIT 0x04 /* set bit 0 of DAT_007feb14 if the cursor is on a pixel that gets drawn */
#define PORT_BLIT_HIT_RUN 0x08 /* like HIT but for runs only, tested on the whole run before clipping (FUN_00468040) */
#define PORT_BLIT_AND 0x10 /* AND every written pixel with DAT_007fe998 */

static __inline unsigned int PortBlitNextCode(PortBlitStream *s) {
    unsigned int code = (*s->mask >> s->shift) & 3;

    s->shift += 2;
    if (s->shift == 32) {
        s->shift = 0;
        s->mask++;
    }
    return code;
}

/* True if the cursor lies in the count pixels starting at p (a cursor before p gives a huge unsigned distance). */
static __inline int PortBlitHits(const unsigned short *cursor, const unsigned short *p, unsigned int count) {
    return (unsigned int)(cursor - p) < count;
}

/* Decode one row. Only columns lo <= x < hi of the row are written, to dst[x]; the rest is decoded and dropped. */
static void PortBlitRow(unsigned short *dst, PortBlitStream *s, int lo, int hi, const unsigned short *cursor, int flags) {
    int x = 0;
    int n;
    int first;
    int last;
    int i;
    unsigned int code;
    unsigned short *p;
    unsigned short v;

    for (;;) {
        code = PortBlitNextCode(s);
        if (!(code & 2)) {
            if (x >= lo && x < hi) {
                p = dst + x;
                if ((flags & PORT_BLIT_HIT) && cursor == p) {
                    DAT_007feb14 |= 1;
                }
                v = *s->pixels;
                if (flags & PORT_BLIT_AND) {
                    v &= DAT_007fe998;
                }
                *p = v;
            }
            s->pixels++;
            x++;
        } else if (!(code & 1)) {
            x++;
        } else {
            n = *s->runs++;
            if (n == 0) {
                break;
            }
            code = PortBlitNextCode(s);
            if (code & 2) {
                x += n;
                continue;
            }
            if ((flags & PORT_BLIT_HIT_RUN) && x >= lo && x < hi && PortBlitHits(cursor, dst + x, n)) {
                DAT_007feb14 |= 1;
            }
            first = x > lo ? x : lo;
            last = x + n < hi ? x + n : hi;
            if (first < last) {
                p = dst + first;
                if ((flags & PORT_BLIT_HIT) && PortBlitHits(cursor, p, last - first)) {
                    DAT_007feb14 |= 1;
                }
                for (i = 0; i < last - first; i++) {
                    v = (code & 1) ? s->pixels[0] : s->pixels[first - x + i];
                    if (flags & PORT_BLIT_AND) {
                        v &= DAT_007fe998;
                    }
                    p[i] = v;
                }
            }
            s->pixels += (code & 1) ? 1 : n;
            x += n;
        }
    }
}

/* Draw h rows of a sprite frame, after first skipping `skip` rows at the top. dst is the first row's start and
 * stride is in bytes. The visible columns are chosen by flags: with LEFT the first `left` columns are dropped,
 * with RIGHT only `width` columns after that are drawn (the caller already shifted dst left by `left` so the
 * first visible column lands on the destination). The original variants assume left > 0 when only LEFT is set,
 * width > 0 when RIGHT is set, and h > 0; a negative `skip` skips one row. */
static void PortBlitRuns(unsigned short *dst, const unsigned short *src, const unsigned char *runs, const unsigned int *mask, int h, int stride, int skip, int left, int width, const unsigned short *cursor, int flags) {
    PortBlitStream s;
    int lo;
    int hi;

    s.pixels = src;
    s.runs = runs;
    s.mask = mask;
    s.shift = 0;
    if (skip != 0) {
        do {
            PortBlitRow(NULL, &s, 0, 0, cursor, 0);
        } while (--skip > 0);
    }
    lo = (flags & PORT_BLIT_LEFT) ? left : 0;
    hi = (flags & PORT_BLIT_RIGHT) ? lo + width : 0x7fffffff;
    for (; h > 0; h--) {
        PortBlitRow(dst, &s, lo, hi, cursor, flags);
        dst = (unsigned short *)((char *)dst + stride);
    }
}

// FUNCTION: LEGOLAND 0x00466d80
void FUN_00466d80(unsigned short *dst, unsigned short *src, unsigned char *runs, unsigned int *mask, int h, int stride, int skip, int left, int width, int flags, unsigned short *cursor) {
    /* Port [library:asm]: the original is inline asm. Draws a run-length sprite clipped on both sides (columns left .. left+width) and records in DAT_007feb14 whether the cursor is on a drawn pixel (mouse hit test). */
    PortBlitRuns(dst, src, runs, mask, h, stride, skip, left, width, cursor, PORT_BLIT_LEFT | PORT_BLIT_RIGHT | PORT_BLIT_HIT);
}

// FUNCTION: LEGOLAND 0x00467180
void FUN_00467180(unsigned short *dst, unsigned short *src, unsigned char *runs, unsigned int *mask, int h, int stride, int skip, int left, int width, int flags, unsigned short *cursor) {
    /* Port [library:asm]: the original is inline asm. As FUN_00466d80 but clipped only on the left (the sprite already fits on the right); caller guarantees left > 0. */
    PortBlitRuns(dst, src, runs, mask, h, stride, skip, left, width, cursor, PORT_BLIT_LEFT | PORT_BLIT_HIT);
}

// FUNCTION: LEGOLAND 0x004673f0
void FUN_004673f0(unsigned short *dst, unsigned short *src, unsigned char *runs, unsigned int *mask, int h, int stride, int skip, int left, int width, int flags, unsigned short *cursor) {
    /* Port [library:asm]: the original is inline asm. As FUN_00466d80 but clipped only on the right (draws at most width columns, no left clip); caller guarantees width > 0. */
    PortBlitRuns(dst, src, runs, mask, h, stride, skip, left, width, cursor, PORT_BLIT_RIGHT | PORT_BLIT_HIT);
}

// FUNCTION: LEGOLAND 0x00467640
void FUN_00467640(unsigned short *dst, unsigned short *src, unsigned char *runs, unsigned int *mask, int h, int stride, int skip, int left, int width, int flags, unsigned short *cursor) {
    unsigned int m;
    unsigned int c;
    unsigned int n;
    unsigned short *p;
    unsigned short v;

    m = 3;
    if (skip != 0) {
        do {
            for (;;) {
                c = *mask & m;
                m = _rotl(m, 2);
                mask += m & 1;
                if (!(c & 0xaaaaaaaa)) {
                    src++;
                }
                if (c & 0x55555555) {
                    n = *runs++;
                    if (n == 0) {
                        break;
                    }
                    c = *mask & m;
                    m = _rotl(m, 2);
                    mask += m & 1;
                    if (!(c & 0xaaaaaaaa)) {
                        if (c & 0x55555555) {
                            src++;
                        } else {
                            src += n;
                        }
                    }
                }
            }
        } while (--skip > 0);
    }
    p = dst;
    do {
        for (;;) {
            c = *mask & m;
            m = _rotl(m, 2);
            mask += m & 1;
            if (c & 0xaaaaaaaa) {
                p++;
                if (c & 0x55555555) {
                    p--;
                    n = *runs++;
                    if (n == 0) {
                        break;
                    }
                    c = *mask & m;
                    m = _rotl(m, 2);
                    mask += m & 1;
                    if (!(c & 0xaaaaaaaa)) {
                        if ((unsigned int)(cursor - p) < n) {
                            DAT_007feb14 |= 1;
                        }
                        if (!(c & 0x55555555)) {
                            do {
                                *p++ = *src++;
                            } while (--n);
                        } else {
                            v = *src++;
                            do {
                                *p++ = v;
                            } while (--n);
                        }
                    } else {
                        p += n;
                    }
                }
            } else {
                if (cursor == p) {
                    DAT_007feb14 |= 1;
                }
                *p++ = *src++;
            }
        }
        dst = (unsigned short *)((char *)dst + stride);
        p = dst;
    } while (--h != 0);
}

// FUNCTION: LEGOLAND 0x004677b0
void FUN_004677b0(unsigned short *dst, unsigned short *src, unsigned char *runs, unsigned int *mask, int h, int stride, int skip, int left, int width, int flags, unsigned short *cursor) {
    /* Port [library:asm]: the original is inline asm. As FUN_00466d80 without the cursor hit test: clipped on both sides, cursor and DAT_007feb14 untouched. */
    PortBlitRuns(dst, src, runs, mask, h, stride, skip, left, width, cursor, PORT_BLIT_LEFT | PORT_BLIT_RIGHT);
}

// FUNCTION: LEGOLAND 0x00467b00
void FUN_00467b00(unsigned short *dst, unsigned short *src, unsigned char *runs, unsigned int *mask, int h, int stride, int skip, int left, int width, int flags, unsigned short *cursor) {
    /* Port [library:asm]: the original is inline asm. As FUN_00467180 without the hit test: clipped only on the left. */
    PortBlitRuns(dst, src, runs, mask, h, stride, skip, left, width, cursor, PORT_BLIT_LEFT);
}

// FUNCTION: LEGOLAND 0x00467d10
void FUN_00467d10(unsigned short *dst, unsigned short *src, unsigned char *runs, unsigned int *mask, int h, int stride, int skip, int left, int width, int flags, unsigned short *cursor) {
    /* Port [library:asm]: the original is inline asm. As FUN_004673f0 without the hit test: clipped only on the right. */
    PortBlitRuns(dst, src, runs, mask, h, stride, skip, left, width, cursor, PORT_BLIT_RIGHT);
}

// FUNCTION: LEGOLAND 0x00467f00
void FUN_00467f00(unsigned short *dst, unsigned short *src, unsigned char *runs, unsigned int *mask, int h, int stride, int skip) {
    unsigned int m;
    unsigned int c;
    unsigned int n;
    unsigned short *p;
    unsigned short v;

    m = 3;
    if (skip != 0) {
        do {
            for (;;) {
                c = *mask & m;
                m = _rotl(m, 2);
                mask += m & 1;
                if (!(c & 0xaaaaaaaa)) {
                    src++;
                } else if (c & 0x55555555) {
                    n = *runs++;
                    if (n == 0) {
                        break;
                    }
                    c = *mask & m;
                    m = _rotl(m, 2);
                    mask += m & 1;
                    if (!(c & 0xaaaaaaaa)) {
                        if (c & 0x55555555) {
                            src++;
                        } else {
                            src += n;
                        }
                    }
                }
            }
        } while (--skip > 0);
    }
    p = dst;
    do {
        for (;;) {
            c = *mask & m;
            m = _rotl(m, 2);
            mask += m & 1;
            if (c & 0xaaaaaaaa) {
                p++;
                if (c & 0x55555555) {
                    p--;
                    n = *runs++;
                    if (n == 0) {
                        break;
                    }
                    c = *mask & m;
                    m = _rotl(m, 2);
                    mask += m & 1;
                    if (!(c & 0xaaaaaaaa)) {
                        if (!(c & 0x55555555)) {
                            do {
                                *p++ = *src++;
                            } while (--n);
                        } else {
                            v = *src++;
                            do {
                                *p++ = v;
                            } while (--n);
                        }
                    } else {
                        p += n;
                    }
                }
            } else {
                *p++ = *src++;
            }
        }
        dst = (unsigned short *)((char *)dst + stride);
        p = dst;
    } while (--h != 0);
}

// FUNCTION: LEGOLAND 0x00468040
void FUN_00468040(unsigned short *dst, unsigned short *src, unsigned char *runs, unsigned int *mask, int h, int stride, int skip, int left, int width, int flags, unsigned short *cursor) {
    /* Port [library:asm]: the original is inline asm. Clipped on both sides like FUN_00466d80, but every written pixel is ANDed with the 16-bit mask DAT_007fe998 (colour masking), and the cursor hit test only looks at runs and uses the whole run length even when the run is clipped (a quirk, kept). */
    PortBlitRuns(dst, src, runs, mask, h, stride, skip, left, width, cursor, PORT_BLIT_LEFT | PORT_BLIT_RIGHT | PORT_BLIT_AND | PORT_BLIT_HIT_RUN);
}

/* Port [library:asm]: reads the next 2-bit code of a 16-bit animation's mask stream (16 codes per word, lowest
 * bits first). The caller tests the result against 0xaaaaaaaa (high bit) and 0x55555555 (low bit). */
static unsigned int PortHalfCode(unsigned int **mask, unsigned int *m) {
    unsigned int c = **mask & *m;

    *m = _rotl(*m, 2);
    *mask += *m & 1;
    return c;
}

/* Port [library:asm]: a pixel at half brightness: clear the low bit of each colour channel, then shift right. */
static unsigned short PortHalfPixel(unsigned short pixel) {
    return (unsigned short)((pixel & (unsigned short)DAT_007fe998) >> 1);
}

enum {
    PORT_HALF_LEFT, /* in the clipped-off left margin */
    PORT_HALF_TAIL, /* left margin handled: decide where to go next */
    PORT_HALF_BODY, /* in the visible part of the row */
    PORT_HALF_END, /* past the right clip: skip the rest of the row's stream */
    PORT_HALF_NEXT /* row finished */
};

// FUNCTION: LEGOLAND 0x00468410
void FUN_00468410(unsigned short *dst, unsigned short *src, unsigned char *runs, unsigned int *mask, int h, int stride, int skip, int left, int width, int flags, unsigned short *cursor) {
    /* Port [library:asm]: the original is inline asm. Draws one frame of a 16-bit run-length animation at half
     * brightness (each pixel PortHalfPixel of the source, using the mask DAT_007fe998), clipped on both sides: the
     * first `skip` rows and the first `left` columns are not drawn, and at most `width` columns per row are. dst
     * is the first row's pixel for column `left`; rows are `stride` bytes apart. Runs of source pixels and fills
     * that touch the cursor set DAT_007feb14 bit 0. The same decoder as the other run-length blitters. `flags`
     * is unused (the original reuses its slot as scratch). */
    unsigned short *line;
    unsigned short colour;
    unsigned int m;
    unsigned int c;
    int left_rem;
    int width_rem;
    int n;
    int e;
    int cnt;
    int k;
    int state;

    m = 3;
    if (skip != 0) {
        do {
            for (;;) {
                c = PortHalfCode(&mask, &m);
                if (!(c & 0xaaaaaaaa)) {
                    src++;
                }
                if (c & 0x55555555) {
                    n = *runs++;
                    if (n == 0) {
                        break;
                    }
                    c = PortHalfCode(&mask, &m);
                    if (!(c & 0xaaaaaaaa)) {
                        if (c & 0x55555555) {
                            src++;
                        } else {
                            src += n;
                        }
                    }
                }
            }
        } while (--skip > 0);
    }
    line = dst;
    left_rem = left;
    width_rem = width;
    state = (left_rem > 0) ? PORT_HALF_LEFT : PORT_HALF_BODY;
    for (;;) {
        switch (state) {
        case PORT_HALF_LEFT:
            left_rem--;
            dst++;
            src++;
            c = PortHalfCode(&mask, &m);
            if (!(c & 0xaaaaaaaa)) {
                state = PORT_HALF_TAIL;
                break;
            }
            src--;
            if (!(c & 0x55555555)) {
                state = PORT_HALF_TAIL;
                break;
            }
            left_rem++;
            dst--;
            n = *runs++;
            if (n == 0) {
                state = PORT_HALF_NEXT;
                break;
            }
            c = PortHalfCode(&mask, &m);
            if (c & 0xaaaaaaaa) {
                /* transparent run */
                dst += n;
                left_rem -= n;
                if (left_rem < 0) {
                    width_rem += left_rem;
                }
                state = PORT_HALF_TAIL;
            } else if (!(c & 0x55555555)) {
                /* run of source pixels */
                left_rem -= n;
                if (left_rem >= 0) {
                    src += n;
                    dst += n;
                    state = PORT_HALF_TAIL;
                    break;
                }
                width_rem += left_rem;
                e = left_rem + n;
                src += e;
                dst += e;
                if (width_rem > 0) {
                    cnt = -left_rem;
                    state = PORT_HALF_TAIL;
                } else {
                    cnt = n - e + width_rem;
                    state = PORT_HALF_END;
                }
                for (k = 0; k < cnt; k++) {
                    *dst++ = PortHalfPixel(*src++);
                }
                if (state == PORT_HALF_END) {
                    src += -width_rem;
                }
            } else {
                /* one source pixel repeated */
                left_rem -= n;
                if (left_rem >= 0) {
                    src++;
                    dst += n;
                    state = PORT_HALF_TAIL;
                    break;
                }
                width_rem += left_rem;
                e = left_rem + n;
                dst += e;
                colour = PortHalfPixel(*src++);
                if (width_rem > 0) {
                    cnt = -left_rem;
                    state = PORT_HALF_TAIL;
                } else {
                    cnt = n - e + width_rem;
                    state = PORT_HALF_END;
                }
                for (k = 0; k < cnt; k++) {
                    *dst++ = colour;
                }
            }
            break;
        case PORT_HALF_TAIL:
            if (left_rem > 0) {
                state = PORT_HALF_LEFT;
            } else if (width_rem <= 0) {
                state = PORT_HALF_END;
            } else {
                state = PORT_HALF_BODY;
            }
            break;
        case PORT_HALF_BODY:
            width_rem--;
            dst++;
            c = PortHalfCode(&mask, &m);
            if (!(c & 0xaaaaaaaa)) {
                dst[-1] = PortHalfPixel(*src++);
            } else if (c & 0x55555555) {
                width_rem++;
                dst--;
                n = *runs++;
                if (n == 0) {
                    state = PORT_HALF_NEXT;
                    break;
                }
                c = PortHalfCode(&mask, &m);
                if (c & 0xaaaaaaaa) {
                    dst += n;
                    width_rem -= n;
                } else {
                    if ((unsigned int)(cursor - dst) < (unsigned int)n) {
                        DAT_007feb14 |= 1;
                    }
                    e = width_rem - n;
                    if (!(c & 0x55555555)) {
                        if (e >= 0) {
                            width_rem = e;
                            for (k = 0; k < n; k++) {
                                *dst++ = PortHalfPixel(*src++);
                            }
                        } else {
                            for (k = 0; k < width_rem; k++) {
                                *dst++ = PortHalfPixel(*src++);
                            }
                            src += -e;
                            state = PORT_HALF_END;
                            break;
                        }
                    } else {
                        colour = PortHalfPixel(*src++);
                        if (e >= 0) {
                            width_rem = e;
                            for (k = 0; k < n; k++) {
                                *dst++ = colour;
                            }
                        } else {
                            for (k = 0; k < width_rem; k++) {
                                *dst++ = colour;
                            }
                            state = PORT_HALF_END;
                            break;
                        }
                    }
                }
            }
            if (width_rem <= 0) {
                state = PORT_HALF_END;
            }
            break;
        case PORT_HALF_END:
            for (;;) {
                c = PortHalfCode(&mask, &m);
                if (!(c & 0xaaaaaaaa)) {
                    src++;
                    continue;
                }
                if (!(c & 0x55555555)) {
                    continue;
                }
                n = *runs++;
                if (n == 0) {
                    break;
                }
                c = PortHalfCode(&mask, &m);
                if (c & 0xaaaaaaaa) {
                    continue;
                }
                if (c & 0x55555555) {
                    src++;
                } else {
                    src += n;
                }
            }
            state = PORT_HALF_NEXT;
            break;
        case PORT_HALF_NEXT:
            line = (unsigned short *)((unsigned char *)line + stride);
            dst = line;
            h--;
            left_rem = left;
            width_rem = width;
            if (h == 0) {
                return;
            }
            state = (left_rem > 0) ? PORT_HALF_LEFT : PORT_HALF_BODY;
            break;
        }
    }
}

// FUNCTION: LEGOLAND 0x004687f0
void FUN_004687f0(const char *param_1) {
    strncpy(DAT_0066861c, param_1, 0x80);
    DAT_0066861c[0x7f] = 0;
}
