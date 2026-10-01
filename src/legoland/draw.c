#include <windows.h>
#include <ddraw.h>
#include "legoland.h"

#include <stdlib.h>
#include <string.h>
#include "clipping.h"
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
        if (IDirectDraw_QueryInterface(ddraw, &DAT_004acf80, &DDRAWENV.ddraw2) != 0) {
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
    DeleteObject((HGDIOBJ)PTR_00668090);
    DeleteObject((HGDIOBJ)PTR_00668098);
    DeleteObject((HGDIOBJ)PTR_0066808c);
    DeleteObject((HGDIOBJ)PTR_00668094);
    if (DDRAWENV.ddraw != 0) {
        IDirectDraw_Release(DDRAWENV.ddraw);
        DDRAWENV.ddraw = 0;
    }
}

// FUNCTION: LEGOLAND 0x00463850
LEGO_EXPORT unsigned int SetPointer(unsigned int param_1) {
    unsigned int old = DAT_0066814c;
    DAT_0066814c = param_1;
    DAT_00668148 = DAT_007fe9c0[param_1];
    return old;
}

// FUNCTION: LEGOLAND 0x00463870
LEGO_EXPORT int InitScreen(void) {
    LOGFONTA font;
    WNDCLASSEXA wc;
    DDSURFACEDESC desc;
    RECT rect_slot;
    RECT window_rect;
    RGNDATA *rgn;

    rect_slot.left = 0;
    rect_slot.top = 0;
    rect_slot.right = lpConfig->field_0;
    rect_slot.bottom = lpConfig->field_2;
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
    PTR_00668090 = CreateFontIndirectA(&font);

    font.lfWeight = 0x190;
    font.lfHeight = 0x1c;
    font.lfWidth = 0;
    PTR_00668098 = CreateFontIndirectA(&font);
    font.lfHeight = 0x14;
    font.lfWidth = 0;
    font.lfWeight = 0x2bc;
    PTR_0066808c = CreateFontIndirectA(&font);
    font.lfWeight = 0x258;
    font.lfHeight = 0x12;
    font.lfWidth = 0;
    strcpy(font.lfFaceName, "Lego");
    PTR_00668094 = CreateFontIndirectA(&font);

    if (DAT_00667d6c == 0) {
        DAT_00668088 = 2;
        WNDENV_Sethwnd(CreateWindowExA(8, "LEGOLANDMAIN",
            // STRING: LEGOLAND 0x004b86d0
            "LEGOLAND", 0x90000000, 0, 0, lpConfig->field_0, lpConfig->field_2, GetDesktopWindow(), NULL, WNDENV_GethInstance(), NULL));
        if (WNDENV_Gethwnd() == NULL) {
            return 0;
        }
        if (IDirectDraw2_SetCooperativeLevel(DDRAWENV.ddraw2, WNDENV_Gethwnd(), 0x11) != 0) {
            return 0;
        }
        if (FUN_00463ef0() == 0) {
            return 0;
        }
        while ((lpConfig->field_1c & 1) != 0) {
            ProcessSystemEvents();
            ShowWindow(WNDENV_Gethwnd(), 3);
        }
        if (lpConfig->field_1e == 0) {
            ShowCursor(0);
        }
        DAT_004b9ca4 = FUN_004661d0;
        desc.dwSize = sizeof(desc);
        desc.dwFlags = 1;
        desc.ddsCaps.dwCaps = 0x4200;
        if (IDirectDraw2_CreateSurface(DDRAWENV.ddraw2, &desc, &DAT_00668070, NULL) != 0) {
            return 0;
        }
        LoadColourTable();
        desc.dwSize = sizeof(desc);
        desc.dwFlags = 7;
        desc.ddsCaps.dwCaps = 0x800;
        desc.dwWidth = lpConfig->field_0;
        desc.dwHeight = lpConfig->field_2;
        if (IDirectDraw2_CreateSurface(DDRAWENV.ddraw2, &desc, &DAT_00668078, NULL) != 0) {
            return 0;
        }
        renderEngine = DAT_00668078;
        desc.dwSize = sizeof(desc);
        desc.dwFlags = 7;
        desc.ddsCaps.dwCaps = 0x4000;
        desc.dwWidth = lpConfig->field_0;
        desc.dwHeight = lpConfig->field_2;
        if (IDirectDraw2_CreateSurface(DDRAWENV.ddraw2, &desc, &DAT_00668074, NULL) != 0) {
            return 0;
        }
        IDirectDraw2_CreateClipper(DDRAWENV.ddraw2, 0, &DAT_00668080, NULL);
        SetClipping(&rect_slot);
        rgn = malloc(sizeof(RGNDATA) - 1 + sizeof(RECT));
        rgn->rdh.dwSize = 0x20;
        rgn->rdh.iType = 1;
        rgn->rdh.nCount = 1;
        rgn->rdh.nRgnSize = 0x10;
        rgn->rdh.rcBound = SPRITE_ClipRect;
        memcpy(rgn->Buffer, &SPRITE_ClipRect, sizeof(RECT));
        IDirectDrawClipper_SetClipList(DAT_00668080, (LPRGNDATA)rgn, 0);
        free(rgn);
    } else {
        window_rect.left = 0;
        window_rect.top = 0;
        window_rect.right = lpConfig->field_0 - 1;
        window_rect.bottom = lpConfig->field_2 - 1;
        AdjustWindowRect(&window_rect, 0x10cf0000, 0);
        WNDENV_Sethwnd(CreateWindowExA(0, "LEGOLANDMAIN",
            // STRING: LEGOLAND 0x004b9cf0
            "Lego Land", 0x10cf0000, 0, 0, window_rect.right - window_rect.left + 1, window_rect.bottom - window_rect.top + 1, NULL, NULL, WNDENV_GethInstance(), NULL));
        if (WNDENV_Gethwnd() == NULL) {
            return 0;
        }
        if (IDirectDraw2_SetCooperativeLevel(DDRAWENV.ddraw2, WNDENV_Gethwnd(), 8) != 0) {
            DestroyWindow(WNDENV_Gethwnd());
            return 0;
        }
        if (FUN_00463ef0() == 0) {
            DestroyWindow(WNDENV_Gethwnd());
            return 0;
        }
        IDirectDraw2_CreateClipper(DDRAWENV.ddraw2, 0, &DAT_00668080, NULL);
        IDirectDrawClipper_SetHWnd(DAT_00668080, 0, WNDENV_Gethwnd());
        desc.dwSize = sizeof(desc);
        desc.dwFlags = 1;
        desc.ddsCaps.dwCaps = 0x200;
        if (IDirectDraw2_CreateSurface(DDRAWENV.ddraw2, &desc, &DAT_00668070, NULL) != 0) {
            DestroyWindow(WNDENV_Gethwnd());
            return 0;
        }
        IDirectDrawSurface_SetClipper(DAT_00668070, DAT_00668080);
        desc.dwSize = sizeof(desc);
        desc.dwFlags = 7;
        desc.ddsCaps.dwCaps = 0x40;
        desc.dwWidth = lpConfig->field_0;
        desc.dwHeight = lpConfig->field_2;
        if (IDirectDraw2_CreateSurface(DDRAWENV.ddraw2, &desc, &DAT_00668078, NULL) != 0) {
            IDirectDrawSurface_Release(DAT_00668070);
            DestroyWindow(WNDENV_Gethwnd());
            return 0;
        }
        renderEngine = DAT_00668078;
        desc.dwSize = sizeof(desc);
        desc.dwFlags = 7;
        desc.ddsCaps.dwCaps = 0x40;
        desc.dwWidth = lpConfig->field_0;
        desc.dwHeight = lpConfig->field_2;
        if (IDirectDraw2_CreateSurface(DDRAWENV.ddraw2, &desc, &DAT_00668074, NULL) != 0) {
            return 0;
        }
    }
    return 1;
}

// FUNCTION: LEGOLAND 0x00463ef0
int FUN_00463ef0(void) {
    DDSURFACEDESC desc;
    LPDIRECTDRAW2 ddraw2;

    if (DAT_00667d6c == 0) {
        ddraw2 = DDRAWENV.ddraw2;
        if (IDirectDraw2_SetDisplayMode(ddraw2, lpConfig->field_0, lpConfig->field_2, 0x10, 0, 0) != 0) {
            ddraw2 = DDRAWENV.ddraw2;
            if (IDirectDraw2_SetDisplayMode(ddraw2, lpConfig->field_0, lpConfig->field_2, 8, 0, 0) != 0) {
                return 0;
            }
        }
    }
    desc.dwSize = 0x6c;
    ddraw2 = DDRAWENV.ddraw2;
    IDirectDraw2_GetDisplayMode(ddraw2, &desc);
    if (desc.ddpfPixelFormat.dwRGBBitCount != 8) {
        if (desc.ddpfPixelFormat.dwRGBBitCount != 0x10) {
            return 0;
        }
        DAT_00668088 = (desc.ddpfPixelFormat.dwGBitMask == 0x7e0) + 1;
        return 1;
    }
    DAT_00668088 = 0;
    return 1;
}

// FUNCTION: LEGOLAND 0x00463fc0
LEGO_EXPORT void PushRenderingStatusAndLockVideoSurface(void) {
    RECT local;
    LPDIRECTDRAWSURFACE surface;
    int value;

    value = DAT_00668144;
    DAT_00668164[DAT_006681e4] = DAT_00668144;
    DAT_006681e4 = DAT_006681e4 + 1;
    if (value == 0) {
        local.left = value;
        local.top = value;
        local.right = lpConfig->field_0 - 1;
        local.bottom = lpConfig->field_2 - 1;
        DAT_0066809c.dwSize = 0x6c;
        IntersectRect(&DAT_00668108, &local, &SPRITE_ClipRect);
        surface = renderEngine;
        if (IDirectDrawSurface_Lock(surface, NULL, &DAT_0066809c, 0x21, NULL) == 0x887601c2) {
            IDirectDrawSurface_Restore(renderEngine);
            IDirectDrawSurface_Lock(renderEngine, NULL, &DAT_0066809c, 0x21, NULL);
        }
        DAT_007fea44 = GetTransparentColour();
    }
    DAT_00668144 = 1;
}

// FUNCTION: LEGOLAND 0x00464080
LEGO_EXPORT void PushRenderingStatusAndUnlockVideoSurface(void) {
    LPDIRECTDRAWSURFACE surface;

    DAT_00668164[DAT_006681e4] = DAT_00668144;
    DAT_006681e4 = DAT_006681e4 + 1;
    if (DAT_00668144 != 0) {
        surface = renderEngine;
        if (IDirectDrawSurface_Unlock(surface, DAT_0066809c.lpSurface) == 0x887601c2) {
            IDirectDrawSurface_Restore(renderEngine);
            IDirectDrawSurface_Unlock(renderEngine, DAT_0066809c.lpSurface);
        }
    }
    DAT_00668144 = 0;
}

// FUNCTION: LEGOLAND 0x004640f0
void FUN_004640f0(void) {
    RECT local;
    LPDIRECTDRAWSURFACE surface;
    int wasLocked;

    wasLocked = DAT_00668144;
    local.left = 0;
    local.top = 0;
    local.right = lpConfig->field_0 - 1;
    local.bottom = lpConfig->field_2 - 1;
    DAT_00668164[DAT_006681e4] = DAT_00668144;
    DAT_006681e4 = DAT_006681e4 + 1;
    if (wasLocked != 0) {
        surface = renderEngine;
        if (IDirectDrawSurface_Unlock(surface, DAT_0066809c.lpSurface) == 0x887601c2) {
            IDirectDrawSurface_Restore(renderEngine);
            IDirectDrawSurface_Unlock(renderEngine, DAT_0066809c.lpSurface);
        }
    }
    DAT_0066809c.dwSize = 0x6c;
    IntersectRect(&DAT_00668108, &local, &SPRITE_ClipRect);
    surface = renderEngine;
    if (IDirectDrawSurface_Lock(surface, NULL, &DAT_0066809c, 0x21, NULL) == 0x887601c2) {
        IDirectDrawSurface_Restore(renderEngine);
        IDirectDrawSurface_Lock(renderEngine, NULL, &DAT_0066809c, 0x21, NULL);
    }
    DAT_007fea44 = GetTransparentColour();
    DAT_00668144 = 1;
}

// FUNCTION: LEGOLAND 0x004641f0
LEGO_EXPORT void PopRenderingStatus(void) {
    RECT local;
    LPDIRECTDRAWSURFACE surface;

    DAT_006681e4 = DAT_006681e4 - 1;
    if (DAT_00668164[DAT_006681e4] != 0) {
        if (DAT_00668144 == 0) {
            local.left = 0;
            local.top = 0;
            local.right = lpConfig->field_0 - 1;
            local.bottom = lpConfig->field_2 - 1;
            DAT_0066809c.dwSize = 0x6c;
            IntersectRect(&DAT_00668108, &local, &SPRITE_ClipRect);
            surface = renderEngine;
            if (IDirectDrawSurface_Lock(surface, NULL, &DAT_0066809c, 0x21, NULL) == 0x887601c2) {
                IDirectDrawSurface_Restore(renderEngine);
                IDirectDrawSurface_Lock(renderEngine, NULL, &DAT_0066809c, 0x21, NULL);
            }
            DAT_007fea44 = GetTransparentColour();
            DAT_00668144 = 1;
        }
        return;
    }
    if (DAT_00668144 != 0) {
        surface = renderEngine;
        if (IDirectDrawSurface_Unlock(surface, DAT_0066809c.lpSurface) == 0x887601c2) {
            IDirectDrawSurface_Restore(renderEngine);
            IDirectDrawSurface_Unlock(renderEngine, DAT_0066809c.lpSurface);
        }
        DAT_00668144 = 0;
    }
}

// FUNCTION: LEGOLAND 0x00464310
LEGO_EXPORT int GetVideoSurface(struct VideoArg *arg) {
    if (DAT_00668144 == 0) {
        return 0;
    }
    arg->field_0 = DAT_0066809c.lPitch;
    arg->field_4 = lpConfig->field_0;
    arg->field_8 = lpConfig->field_2;
    arg->field_c = DAT_0066809c.lpSurface;
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
    clipper = DAT_00668080;
    IDirectDrawClipper_SetClipList(clipper, (LPRGNDATA)rgn, 0);
    free(rgn);
}

// FUNCTION: LEGOLAND 0x00464400
LEGO_EXPORT void SetOverridePalette(unsigned int param_1) { DAT_006681e8 = param_1; }

// FUNCTION: LEGOLAND 0x00464410
LEGO_EXPORT unsigned int GetOverridePalette(void) { return DAT_006681e8; }

// FUNCTION: LEGOLAND 0x00464420
LEGO_EXPORT void SetOverrideFrame(unsigned int param_1) { DAT_004b9ca8 = param_1; }

// FUNCTION: LEGOLAND 0x00464430
LEGO_EXPORT unsigned int GetOverrideFrame(void) { return DAT_004b9ca8; }

// FUNCTION: LEGOLAND 0x00464440
LEGO_EXPORT void ClearOverrideFrame(void) { DAT_004b9ca8 = 0xffffffff; }

// FUNCTION: LEGOLAND 0x00464450
LEGO_EXPORT void ClearOverridePalette(void) { DAT_006681e8 = 0; }

// FUNCTION: LEGOLAND 0x00464460
LEGO_EXPORT void ClearSpriteOverrides(void) {
    DAT_004b9ca8 = 0xffffffff;
    DAT_006681e8 = 0;
}

// FUNCTION: LEGOLAND 0x00464480
void FUN_00464480(void) { STUB(); }

// FUNCTION: LEGOLAND 0x00464a90
LEGO_EXPORT void ZBufferHelper(unsigned int *param_1, int *param_2, int *param_3, void *param_4) { STUB(); }

// FUNCTION: LEGOLAND 0x00464ee0
void __fastcall FUN_00464ee0(struct Sprite *sprite, RECT *rect, int *off) { STUB(); }

// The original fills the rows with an inline __asm block (pusha; rep stosw per row; popa) that
// reads its loop bounds from the globals below. This is the C equivalent: same effect, but it
// cannot byte-match without __asm.
// FUNCTION: LEGOLAND 0x004651d0
LEGO_EXPORT void SoftPrint_Clear(void) {
    unsigned short colour = GetTransparentColour();
    unsigned short *row;
    int y;
    int x;

    DAT_007fea14 = DAT_0066809c.dwHeight;
    DAT_007fea1c = DAT_0066809c.dwWidth;
    DAT_007fe9a4 = DAT_007fea1c;
    row = DAT_0066809c.lpSurface;
    for (y = DAT_007fea14; y != 0; y--) {
        for (x = 0; x < DAT_007fe9a4; x++) {
            row[x] = colour;
        }
        row = (unsigned short *)((char *)row + DAT_0066809c.lPitch);
    }
}

// FUNCTION: LEGOLAND 0x00465240
void FUN_00465240(void) { STUB(); }

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

    dst = DAT_0066809c.lpSurface;
    DAT_006681ec = (DAT_006681ec != dst) ? dst : 0;
    height = frame->height;
    width = frame->width;
    rest = lpConfig->field_2 - height * 2;
    half = rest / 2;
    rest = rest - half;
    row = frame->pixels + (height - 1) * width;
    for (y = half; y > 0; y--) {
        memset(dst, 0, 0x500);
        dst += DAT_0066809c.lPitch;
    }
    for (y = height; y != 0; y--) {
        next = dst + DAT_0066809c.lPitch;
        s = row;
        p0 = (unsigned short *)dst;
        p1 = (unsigned short *)next;
        for (x = width; x > 0; x--) {
            v = *s;
            if (DAT_00668088 == 2) {
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
        dst += DAT_0066809c.lPitch * 2;
    }
    dst = next + DAT_0066809c.lPitch;
    for (y = rest; y > 0; y--) {
        memset(dst, 0, 0x500);
        dst += DAT_0066809c.lPitch;
    }
}

// FUNCTION: LEGOLAND 0x004659a0
void FUN_004659a0(struct AviFrame *param_1, int param_2, int param_3) {
    int height;
    int width;
    unsigned short *src;
    unsigned short *dst;
    int offset;
    int i;
    int j;
    unsigned short *p;
    unsigned short v;

    height = param_1->height;
    width = param_1->width;
    dst = (unsigned short *)((char *)DAT_0066809c.lpSurface + DAT_0066809c.lPitch * param_3 + param_2 * 2);
    src = param_1->pixels + (height - 1) * width;
    for (i = height; i != 0; i--) {
        if (width > 0) {
            offset = (char *)src - (char *)dst;
            p = dst;
            j = width;
            do {
                v = *(unsigned short *)((char *)p + offset);
                if (DAT_00668088 == 2) {
                    v = (v & 0x1f) | (v & 0xffe0) << 1;
                }
                *p = v;
                p++;
                j--;
            } while (j != 0);
        }
        dst = (unsigned short *)((char *)dst + DAT_0066809c.lPitch);
        src -= width;
    }
}

// FUNCTION: LEGOLAND 0x00465a40
LEGO_EXPORT void SoftPrint_XBltFast(struct Sprite *sprite, RECT *a, RECT *b, unsigned int param_4) { STUB(); }

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

    DAT_007fe9a4 = DAT_0066809c.lPitch;
    DAT_007fea4c.left = clip->left;
    DAT_007febac.width = clip->right - clip->left;
    DAT_007fea4c.top = clip->top;
    DAT_007febac.height = clip->bottom - clip->top;
    if ((int)DAT_004b9ca8 < 0) {
        frame_index = lls->frame;
    } else {
        frame_index = (int)DAT_004b9ca8;
    }
    if (frame_index >= lls->frame_count) {
        frame_index = lls->frame_count - 1;
    }
    DAT_007fe9a8 = (unsigned int)((unsigned char *)DAT_0066809c.lpSurface + DAT_00813a44.y * DAT_0066809c.lPitch + DAT_00813a44.x * 2);
    dst = (unsigned short *)((unsigned char *)DAT_0066809c.lpSurface + pos->y * DAT_0066809c.lPitch + (pos->x - DAT_007fea4c.left) * 2);
    if (lls->flags & 1) {
        pixels = frame->pixels;
        off = frame->pixel_count * 2 + 0x10;
        runs = (unsigned char *)frame + off;
        mask = (unsigned int *)((unsigned char *)frame + (frame->run_bytes + off));
        FUN_00468040(dst, pixels, runs, mask, DAT_007febac.height, DAT_0066809c.lPitch, DAT_007fea4c.top, DAT_007fea4c.left, DAT_007febac.width, 0, (unsigned short *)DAT_007fe9a8);
        i = lls->frame + 1;
        while (i-- != 0) {
            frame = (struct DrawLLSFrame *)((unsigned char *)frame + frame->size);
        }
        pixels = frame->pixels;
        off = frame->pixel_count * 2 + 0x10;
        runs = (unsigned char *)frame + off;
        mask = (unsigned int *)((unsigned char *)frame + (frame->run_bytes + off));
        FUN_00468040(dst, pixels, runs, mask, DAT_007febac.height, DAT_0066809c.lPitch, DAT_007fea4c.top, DAT_007fea4c.left, DAT_007febac.width, 0, (unsigned short *)DAT_007fe9a8);
    } else {
        i = frame_index;
        while (i-- != 0) {
            frame = (struct DrawLLSFrame *)((unsigned char *)frame + frame->size);
        }
        pixels = frame->pixels;
        off = frame->pixel_count * 2 + 0x10;
        runs = (unsigned char *)frame + off;
        mask = (unsigned int *)((unsigned char *)frame + (frame->run_bytes + off));
        FUN_00468040(dst, pixels, runs, mask, DAT_007febac.height, DAT_0066809c.lPitch, DAT_007fea4c.top, DAT_007fea4c.left, DAT_007febac.width, 0, (unsigned short *)DAT_007fe9a8);
    }
}

// FUNCTION: LEGOLAND 0x00466080
int FUN_00466080(void) {
    DWORD tick;
    int result;
    int frames;
    LPDIRECTDRAWSURFACE primary;
    LPDIRECTDRAWSURFACE back;

    LLSAuto();
    if (lpConfig->field_1e != 0 && DAT_00668148 != 0) {
        PushRenderingStatusAndLockVideoSurface();
        PrintSprite(DAT_00668148, DAT_00813a44.x, DAT_00813a44.y, 0, 0);
        PopRenderingStatus();
    }
    tick = GetTickCount();
    while (tick - DAT_00668200 < 0x1c) {
        tick = GetTickCount();
    }
    DAT_00668200 = GetTickCount();
    primary = DAT_00668070;
    result = IDirectDrawSurface_Flip(primary, NULL, 1);
    while (result != 0) {
        if (result == 0x887601c2) {
            IDirectDrawSurface_Restore(DAT_00668070);
            IDirectDrawSurface_Restore(DAT_00668078);
            return 0;
        }
        if (result != 0x887601ae && result != 0x8876021c) {
            return 0;
        }
        primary = DAT_00668070;
        result = IDirectDrawSurface_Flip(primary, NULL, 1);
    }
    back = DAT_00668078;
    result = IDirectDrawSurface_GetFlipStatus(back, 2);
    while (result != 0) {
        back = DAT_00668078;
        result = IDirectDrawSurface_GetFlipStatus(back, 2);
    }
    tick = GetTickCount();
    FrameNumber = FrameNumber + 1;
    DAT_006681f8 = DAT_006681f8 + 1;
    if (tick - DAT_006681f0 >= 0x3e8) {
        FramesPerSecond = DAT_006681f8;
        DAT_006681f8 = 0;
        DAT_006681f0 = tick;
    }
    LastFrameMS = tick - DAT_006681f4;
    DAT_006681f4 = tick;
    return 1;
}

// FUNCTION: LEGOLAND 0x004661d0
int FUN_004661d0(void) {
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
        PrintSprite(DAT_00668148, DAT_00813a44.x, DAT_00813a44.y, 0, 0);
        PopRenderingStatus();
    }
    tick = GetTickCount();
    while (tick - DAT_00668200 < 0x1c) {
        tick = GetTickCount();
    }
    DAT_00668200 = GetTickCount();
    GetClientRect(WNDENV_Gethwnd(), &client.rect);
    ClientToScreen(WNDENV_Gethwnd(), &client.pt[0]);
    OffsetRect(&dst, client.rect.left, client.rect.top);
    surface = DAT_00668070;
    result = IDirectDrawSurface_Blt(surface, &dst, DAT_00668078, NULL, 0x1000000, NULL);
    if (result == 0x887601c2) {
        IDirectDrawSurface_Restore(DAT_00668070);
        result = IDirectDrawSurface_Blt(DAT_00668070, &dst, DAT_00668078, NULL, 0x1000000, NULL);
    }
    if (result != 0) {
        return 0;
    }
    tick = GetTickCount();
    FrameNumber = FrameNumber + 1;
    DAT_006681f8 = DAT_006681f8 + 1;
    if (tick - DAT_006681f0 >= 0x3e8) {
        FramesPerSecond = DAT_006681f8;
        DAT_006681f8 = 0;
        DAT_006681f0 = tick;
    }
    LastFrameMS = tick - DAT_006681f4;
    DAT_006681f4 = tick;
    return 1;
}

// FUNCTION: LEGOLAND 0x00466360
void FUN_00466360(int a, int b) {
    short w;
    short h;

    if (DAT_00668208 == NULL) {
        // STRING: LEGOLAND 0x004b9d30
        DAT_00668208 = LoadSprite("Watch.lls", 4);
        if (DAT_00668208 == NULL) {
            return;
        }
    }
    DAT_00668204 = 1;
    w = DAT_00668208->width;
    h = DAT_00668208->height;
    DAT_007fea30.left = a;
    DAT_007fea30.top = b;
    DAT_007fea30.right = w + a;
    DAT_007fea30.bottom = h + b;
}

// FUNCTION: LEGOLAND 0x004663c0
void FUN_004663c0(void) {
    if (DAT_00668208 != 0) {
        KillSprite(DAT_00668208);
        DAT_00668208 = 0;
    }
    DAT_00668204 = 0;
}

// FUNCTION: LEGOLAND 0x004663f0
void FUN_004663f0(void) {
    union RectPoints cursor;
    LPDIRECTDRAWSURFACE surface;
    struct Image *image;

    if (DAT_00668204 != 0) {
        if ((int)(GetTicks() - DAT_00667d60) > 0xc8) {
            cursor.rect.left = DAT_007fea30.left;
            cursor.rect.top = DAT_007fea30.top;
            cursor.rect.right = DAT_007fea30.right;
            cursor.rect.bottom = DAT_007fea30.bottom;
            DAT_00667d60 = GetTicks();
            image = DAT_00668208->image;
            FUN_0047d610((struct LLS *)image->data);
            PushRenderingStatusAndLockVideoSurface();
            PrintSprite(DAT_00668208, DAT_007fea30.left, DAT_007fea30.top, 0, 0);
            PopRenderingStatus();
            ClientToScreen(WNDENV_Gethwnd(), &cursor.pt[0]);
            ClientToScreen(WNDENV_Gethwnd(), &cursor.pt[1]);
            surface = DAT_00668070;
            if (IDirectDrawSurface_Blt(surface, &cursor.rect, DAT_00668078, &DAT_007fea30, 0x1000000, NULL) == 0x887601c2) {
                IDirectDrawSurface_Restore(DAT_00668070);
                IDirectDrawSurface_Blt(DAT_00668070, &cursor, DAT_00668078, NULL, 0x1000000, NULL);
            }
        }
    }
}

// FUNCTION: LEGOLAND 0x00466500
LEGO_EXPORT int RenderingComplete(void) {
    int result;

    ProcessSystemEvents();
    FUN_00455f70(0);
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
    DAT_00667d68 = GetTickCount();
    result = DAT_004b9ca4();
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

    locked = DAT_00668144;
    DAT_00668164[DAT_006681e4] = DAT_00668144;
    DAT_006681e4 = DAT_006681e4 + 1;
    if (locked != 0) {
        surface = renderEngine;
        if (IDirectDrawSurface_Unlock(surface, DAT_0066809c.lpSurface) == 0x887601c2) {
            IDirectDrawSurface_Restore(renderEngine);
            IDirectDrawSurface_Unlock(renderEngine, DAT_0066809c.lpSurface);
        }
    }
    DAT_00668144 = 0;
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

    DAT_007fe9a4 = DAT_0066809c.lPitch;
    DAT_007fea4c.left = clip->left;
    DAT_007febac.width = clip->right - clip->left;
    DAT_007fea4c.top = clip->top;
    DAT_007febac.height = clip->bottom - clip->top;
    frame_index = (int)DAT_004b9ca8;
    if (frame_index < 0) {
        frame_index = lls->frame;
    }
    if (frame_index >= lls->frame_count) {
        frame_index = lls->frame_count - 1;
    }
    hit = 0;
    DAT_007fe9a8 = (unsigned int)((unsigned char *)DAT_0066809c.lpSurface + DAT_00813a44.y * DAT_0066809c.lPitch + DAT_00813a44.x * 2);
    if (DAT_00813a44.x >= pos->x && DAT_00813a44.x <= pos->x + lls->width && DAT_00813a44.y >= pos->y && DAT_00813a44.y <= pos->y + lls->height) {
        hit = 1;
    }
    dst = (unsigned short *)((unsigned char *)DAT_0066809c.lpSurface + pos->y * DAT_0066809c.lPitch + (pos->x - DAT_007fea4c.left) * 2);
    if (lls->flags & 1) {
        pixels = frame->pixels;
        runs = (unsigned char *)frame + frame->pixel_count * 2 + 0x10;
        mask = (unsigned int *)(runs + frame->run_bytes);
        if (hit) {
            if (lls->width <= DAT_007febac.width - DAT_007fea4c.left) {
                FUN_00467640(dst, pixels, runs, mask, DAT_007febac.height, DAT_0066809c.lPitch, DAT_007fea4c.top, DAT_007fea4c.left, DAT_007febac.width, 0, (unsigned short *)DAT_007fe9a8);
            } else if (DAT_007fea4c.left != 0) {
                if (lls->width - DAT_007fea4c.left > DAT_007febac.width) {
                    FUN_00466d80(dst, pixels, runs, mask, DAT_007febac.height, DAT_0066809c.lPitch, DAT_007fea4c.top, DAT_007fea4c.left, DAT_007febac.width, 0, (unsigned short *)DAT_007fe9a8);
                } else {
                    FUN_00467180(dst, pixels, runs, mask, DAT_007febac.height, DAT_0066809c.lPitch, DAT_007fea4c.top, DAT_007fea4c.left, DAT_007febac.width, 0, (unsigned short *)DAT_007fe9a8);
                }
            } else {
                FUN_004673f0(dst, pixels, runs, mask, DAT_007febac.height, DAT_0066809c.lPitch, DAT_007fea4c.top, DAT_007fea4c.left, DAT_007febac.width, 0, (unsigned short *)DAT_007fe9a8);
            }
        } else {
            if (lls->width <= DAT_007febac.width - DAT_007fea4c.left) {
                FUN_00467f00(dst, pixels, runs, mask, DAT_007febac.height, DAT_0066809c.lPitch, DAT_007fea4c.top);
            } else if (DAT_007fea4c.left != 0) {
                if (lls->width - DAT_007fea4c.left > DAT_007febac.width) {
                    FUN_004677b0(dst, pixels, runs, mask, DAT_007febac.height, DAT_0066809c.lPitch, DAT_007fea4c.top, DAT_007fea4c.left, DAT_007febac.width, 0, (unsigned short *)DAT_007fe9a8);
                } else {
                    FUN_00467b00(dst, pixels, runs, mask, DAT_007febac.height, DAT_0066809c.lPitch, DAT_007fea4c.top, DAT_007fea4c.left, DAT_007febac.width, 0, (unsigned short *)DAT_007fe9a8);
                }
            } else {
                FUN_00467d10(dst, pixels, runs, mask, DAT_007febac.height, DAT_0066809c.lPitch, DAT_007fea4c.top, DAT_007fea4c.left, DAT_007febac.width, 0, (unsigned short *)DAT_007fe9a8);
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
            if (lls->width <= DAT_007febac.width - DAT_007fea4c.left) {
                FUN_00467640(dst, pixels, runs, mask, DAT_007febac.height, DAT_0066809c.lPitch, DAT_007fea4c.top, DAT_007fea4c.left, DAT_007febac.width, 0, (unsigned short *)DAT_007fe9a8);
            } else if (DAT_007fea4c.left != 0) {
                if (lls->width - DAT_007fea4c.left > DAT_007febac.width) {
                    FUN_00466d80(dst, pixels, runs, mask, DAT_007febac.height, DAT_0066809c.lPitch, DAT_007fea4c.top, DAT_007fea4c.left, DAT_007febac.width, 0, (unsigned short *)DAT_007fe9a8);
                } else {
                    FUN_00467180(dst, pixels, runs, mask, DAT_007febac.height, DAT_0066809c.lPitch, DAT_007fea4c.top, DAT_007fea4c.left, DAT_007febac.width, 0, (unsigned short *)DAT_007fe9a8);
                }
            } else {
                FUN_004673f0(dst, pixels, runs, mask, DAT_007febac.height, DAT_0066809c.lPitch, DAT_007fea4c.top, DAT_007fea4c.left, DAT_007febac.width, 0, (unsigned short *)DAT_007fe9a8);
            }
        } else {
            if (lls->width <= DAT_007febac.width - DAT_007fea4c.left) {
                FUN_00467f00(dst, pixels, runs, mask, DAT_007febac.height, DAT_0066809c.lPitch, DAT_007fea4c.top);
            } else if (DAT_007fea4c.left != 0) {
                if (lls->width - DAT_007fea4c.left > DAT_007febac.width) {
                    FUN_004677b0(dst, pixels, runs, mask, DAT_007febac.height, DAT_0066809c.lPitch, DAT_007fea4c.top, DAT_007fea4c.left, DAT_007febac.width, 0, (unsigned short *)DAT_007fe9a8);
                } else {
                    FUN_00467b00(dst, pixels, runs, mask, DAT_007febac.height, DAT_0066809c.lPitch, DAT_007fea4c.top, DAT_007fea4c.left, DAT_007febac.width, 0, (unsigned short *)DAT_007fe9a8);
                }
            } else {
                FUN_00467d10(dst, pixels, runs, mask, DAT_007febac.height, DAT_0066809c.lPitch, DAT_007fea4c.top, DAT_007fea4c.left, DAT_007febac.width, 0, (unsigned short *)DAT_007fe9a8);
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
            if (lls->width <= DAT_007febac.width - DAT_007fea4c.left) {
                FUN_00467640(dst, pixels, runs, mask, DAT_007febac.height, DAT_0066809c.lPitch, DAT_007fea4c.top, DAT_007fea4c.left, DAT_007febac.width, 0, (unsigned short *)DAT_007fe9a8);
            } else if (DAT_007fea4c.left != 0) {
                if (lls->width - DAT_007fea4c.left > DAT_007febac.width) {
                    FUN_00466d80(dst, pixels, runs, mask, DAT_007febac.height, DAT_0066809c.lPitch, DAT_007fea4c.top, DAT_007fea4c.left, DAT_007febac.width, 0, (unsigned short *)DAT_007fe9a8);
                } else {
                    FUN_00467180(dst, pixels, runs, mask, DAT_007febac.height, DAT_0066809c.lPitch, DAT_007fea4c.top, DAT_007fea4c.left, DAT_007febac.width, 0, (unsigned short *)DAT_007fe9a8);
                }
            } else {
                FUN_004673f0(dst, pixels, runs, mask, DAT_007febac.height, DAT_0066809c.lPitch, DAT_007fea4c.top, DAT_007fea4c.left, DAT_007febac.width, 0, (unsigned short *)DAT_007fe9a8);
            }
        } else {
            if (lls->width <= DAT_007febac.width - DAT_007fea4c.left) {
                FUN_00467f00(dst, pixels, runs, mask, DAT_007febac.height, DAT_0066809c.lPitch, DAT_007fea4c.top);
            } else if (DAT_007fea4c.left != 0) {
                if (lls->width - DAT_007fea4c.left > DAT_007febac.width) {
                    FUN_004677b0(dst, pixels, runs, mask, DAT_007febac.height, DAT_0066809c.lPitch, DAT_007fea4c.top, DAT_007fea4c.left, DAT_007febac.width, 0, (unsigned short *)DAT_007fe9a8);
                } else {
                    FUN_00467b00(dst, pixels, runs, mask, DAT_007febac.height, DAT_0066809c.lPitch, DAT_007fea4c.top, DAT_007fea4c.left, DAT_007febac.width, 0, (unsigned short *)DAT_007fe9a8);
                }
            } else {
                FUN_00467d10(dst, pixels, runs, mask, DAT_007febac.height, DAT_0066809c.lPitch, DAT_007fea4c.top, DAT_007fea4c.left, DAT_007febac.width, 0, (unsigned short *)DAT_007fe9a8);
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

// FUNCTION: LEGOLAND 0x00468410
void FUN_00468410(void) { STUB(); }

// FUNCTION: LEGOLAND 0x004687f0
void FUN_004687f0(const char *param_1) {
    strncpy(DAT_0066861c, param_1, 0x80);
    DAT_0066861c[0x7f] = 0;
}
