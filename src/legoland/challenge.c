#include <stdio.h>
#include <stdlib.h>
#include "globals.h"
#include "legoland.h"

#include "bloke.h"
#include "bloke_ai.h"
#include "challenge.h"
#include "clipping.h"
#include "controller.h"
#include "debug.h"
#include "debug_alloc.h"
#include "draw.h"
#include "gamemain.h"
#include "gamemap.h"
#include "gfx.h"
#include "help.h"
#include "icon.h"
#include "imports.h"
#include "interface.h"
#include "llidb.h"
#include "map_object.h"
#include "math.h"
#include "nerps.h"
#include "pathfind.h"
#include "print_sprite.h"
#include "profile_io.h"
#include "render.h"
#include "resource.h"
#include "screens.h"
#include "sound_music.h"
#include "sound_sfx.h"
#include "stream.h"
#include "string.h"
#include "text.h"
#include "timer.h"

struct AnimHandle;
struct RideElem;
struct RideObj;
struct ObjectClass;

struct AnimHandle {
    /* 0x00 */ int length;
    /* 0x04 */ int fps;
    /* 0x08 */ int width;
    /* 0x0c */ int height;
    /* 0x10 */ void *avifile;
    /* 0x14 */ void *getframe;
    /* 0x18 */ unsigned char pad_18[0x1c - 0x18];
    /* 0x1c */ void *stream;
    /* 0x20 */ void (*open_cb)(struct AnimHandle *handle);
    /* 0x24 */ void *(*callback)(void);
};

struct RideObj {
    unsigned char pad_0[0xc0];
    unsigned int (*func)(unsigned int, unsigned int);
    unsigned int arg;
};

struct RideElem {
    unsigned char pad_0[8];
    unsigned char flags;
    unsigned char pad_9[0xc - 0x9];
    struct RideObj *obj;
};

struct ObjectClass {
    struct ObjectClass *next;
    unsigned char pad_4[0x8 - 0x4];
    unsigned int count;
    unsigned char pad_c[0x20 - 0xc];
    unsigned short type;
};

#include "image_sprite.h"
#ifdef LEGOLAND_PORT
#include "port_movie.h"
#endif

// FUNCTION: LEGOLAND 0x004434d0
unsigned int LoadBmpIntoImage(struct Image *param_1) {
    char header[0xe];
    char info[0x2c];
    char *path;
    register struct ResFile *file;
    unsigned int offbits;
    int pixel_offset;
    unsigned int row_size;
    int aligned_width;
    unsigned char *pixels;
    unsigned char *dst;
    unsigned char *src;
    int x;
    int y;

    path = GetGFXFName(param_1->name, param_1->type, NULL);
    file = RES_OpenFile(path);
    if (file == NULL) {
        return 0;
    }
    RES_ReadFile(file, header, 0xe);
    offbits = *(unsigned int *)(header + 0xa);
    pixel_offset = RES_GetFilePointer(file) + 0x28;
    RES_ReadFile(file, info, 0x2c);
    if (*(int *)(info + 0x10) != 0) {
        RES_CloseFile(file);
        return 0;
    }
    if (*(short *)(info + 0xe) != 8) {
        RES_CloseFile(file);
        return 0;
    }
    aligned_width = (*(int *)(info + 4) + 3) & 0xfffffffc;
    row_size = *(int *)(info + 8) * aligned_width;
    RES_SetFilePointer(file, *(unsigned int *)(header + 0xa));
    pixels = (unsigned char *)malloc(row_size);
    if (pixels == NULL) {
        free(param_1);
        RES_CloseFile(file);
        return 0;
    }
    param_1->aux = malloc(0x400);
    param_1->width = (short)*(int *)(info + 4);
    param_1->height = (short)*(int *)(info + 8);
    param_1->field_14 = 1;
    dst = (unsigned char *)malloc(*(int *)(info + 8) * *(int *)(info + 4));
    param_1->data = dst;
    if (dst == NULL) {
        free(pixels);
        free(param_1);
        RES_CloseFile(file);
        return 0;
    }
    if (*(short *)(info + 0xe) == 8) {
        src = pixels + row_size;
        RES_SetFilePointer(file, pixel_offset);
        x = 0;
        while (x < 0x100) {
            DAT_0081c4c0[x] = 0;
            x++;
        }
        RES_ReadFile(file, DAT_0081c4c0, *(int *)(info + 0x20) * 4);
        RES_SetFilePointer(file, offbits);
        RES_ReadFile(file, pixels, row_size);
        for (y = 0; y < (unsigned)param_1->height; y++) {
            src -= aligned_width;
            x = 0;
            while (x < param_1->width) {
                *dst = *src;
                x++;
                src++;
                dst = dst + 1;
            }
        }
    }
    free(pixels);
    RES_CloseFile(file);
    return 1;
}

// FUNCTION: LEGOLAND 0x004436d0
unsigned int LoadBmpImage(const char *param_1, unsigned char param_2) {
    struct Image *image;

    image = CreateSourceImage(param_1, param_2);
    if (image == NULL) {
        return 0;
    }
    if (LoadBmpIntoImage(image) == 0) {
        KillImage(image);
        return 0;
    }
    return (unsigned int)image;
}

// FUNCTION: LEGOLAND 0x00443710
unsigned int GetLoadedTextureCount(void) {
    return DAT_00665e8c;
}

struct LocFile {
    /* 0x00 */ int count;
    /* 0x04 */ unsigned char pad_4[0xc - 0x4];
    /* 0x0c */ char name[1];
};

// FUNCTION: LEGOLAND 0x00443720
void LoadTextureBitmaps(struct LocFile *param_1, const char *param_2) {
    int i;
    struct Image *image;
    char filename[64];

    for (i = 0; i < param_1->count; i++) {
        // STRING: LEGOLAND 0x004b7d58
        sprintf(filename, "%s\\%s%04d.BMP", param_2, param_1->name, i);
        image = (struct Image *)LoadBmpImage(filename, 9);
        if (image == NULL) {
            // STRING: LEGOLAND 0x004b7d3c
            DBPrintf("Failed to load texture %s", filename);
        } else {
            FUN_00488670(image, DAT_00665e8c);
            DAT_0081c0c0[DAT_00665e8c * 2] = image->width;
            DAT_0081c0c0[DAT_00665e8c * 2 + 1] = image->height;
            KillImage(image);
        }
        DAT_00665e8c = DAT_00665e8c + 1;
    }
}

// FUNCTION: LEGOLAND 0x004437d0
void BuildTextureNodeFromImage(struct Image *param_1, struct TextureNode *param_2) {
    int width;
    int height;
    int x;
    int y;
    unsigned char *src;
    unsigned char *out;
    unsigned int *dst;
    unsigned char index;

    param_2->width_m1 = param_1->width - 1;
    param_2->height_m1 = param_1->height - 1;
    width = param_1->width;
    switch (width) {
    case 1:
        param_2->format_w = 0;
        break;
    case 2:
        param_2->format_w = 1;
        break;
    case 4:
        param_2->format_w = 2;
        break;
    case 8:
        param_2->format_w = 3;
        break;
    case 0x10:
        param_2->format_w = 4;
        break;
    case 0x20:
        param_2->format_w = 5;
        break;
    case 0x40:
        param_2->format_w = 6;
        break;
    case 0x80:
        param_2->format_w = 7;
        break;
    case 0x100:
        param_2->format_w = 8;
    }
    height = param_1->height;
    switch (height) {
    case 1:
        param_2->format_h = 0;
        break;
    case 2:
        param_2->format_h = 1;
        break;
    case 4:
        param_2->format_h = 2;
        break;
    case 8:
        param_2->format_h = 3;
        break;
    case 0x10:
        param_2->format_h = 4;
        break;
    case 0x20:
        param_2->format_h = 5;
        break;
    case 0x40:
        param_2->format_h = 6;
        break;
    case 0x80:
        param_2->format_h = 7;
        break;
    case 0x100:
        param_2->format_h = 8;
    }
    param_2->data_8 = (unsigned char *)malloc(param_1->height * param_1->width);
    if (param_1 != NULL) {
        dst = (unsigned int *)malloc(0x404);
        param_2->data_c = dst;
        for (x = 0x101; x != 0; x--) {
            *dst = 0;
            dst++;
        }
        for (y = 0; y < param_1->height; y++) {
            for (x = 0; x < param_1->width; x++) {
                src = (unsigned char *)param_1->data + param_1->width * y;
                out = param_2->data_8 + param_1->width * y;
                index = src[x];
                out[x] = index;
                if (param_2->data_c[index] == 0) {
                    param_2->data_c[index] = FUN_00486280(0x40, &DAT_0081c4c0[index]);
                }
            }
        }
    }
}

struct AviFileInfo {
    /* 0x00 */ unsigned int dwMaxBytesPerSec;
    /* 0x04 */ unsigned int dwFlags;
    /* 0x08 */ unsigned int dwCaps;
    /* 0x0c */ int dwStreams;
    /* 0x10 */ unsigned char pad_10[0x6c - 0x10];
};

struct AviStreamInfo {
    /* 0x00 */ unsigned int fccType;
    /* 0x04 */ unsigned int fccHandler;
    /* 0x08 */ unsigned char pad_8[0x14 - 0x8];
    /* 0x14 */ unsigned int dwScale;
    /* 0x18 */ unsigned int dwRate;
    /* 0x1c */ unsigned int dwStart;
    /* 0x20 */ unsigned int dwLength;
    /* 0x24 */ unsigned char pad_24[0x34 - 0x24];
    /* 0x34 */ int rcFrameLeft;
    /* 0x38 */ int rcFrameTop;
    /* 0x3c */ int rcFrameRight;
    /* 0x40 */ int rcFrameBottom;
    /* 0x44 */ unsigned char pad_44[0x8c - 0x44];
};

// FUNCTION: LEGOLAND 0x00443bd0
struct AnimHandle *OpenAviAnim(const char *filename) {
    void *file;
    void *stream;
    struct AviFileInfo finfo;
    struct AviStreamInfo sinfo;
    struct AnimHandle *result;
    void *found;
    unsigned int length;
    unsigned int fps;
    int width;
    int height;
    int i;

    if (DAT_00665f48 == 0) {
        AVIFileInit();
    }
    found = NULL;
    if (AVIFileOpenA(&file, filename, 0, 0) == 0) {
        finfo.dwStreams = 0;
        AVIFileInfoA(file, &finfo, 0x6c);
        for (i = 0; i < finfo.dwStreams; i++) {
            if (AVIFileGetStream(file, &stream, 0, i) != 0) {
                break;
            }
            if (AVIStreamInfoA(stream, &sinfo, 0x8c) == 0 && sinfo.fccType == 0x73646976) {
                found = stream;
                AVIStreamAddRef(stream);
                length = sinfo.dwLength;
                fps = sinfo.dwRate / sinfo.dwScale;
                width = sinfo.rcFrameRight - sinfo.rcFrameLeft;
                height = sinfo.rcFrameBottom - sinfo.rcFrameTop;
            }
        }
        if (found != NULL) {
            result = (struct AnimHandle *)malloc(sizeof(struct AnimHandle));
            if (result == NULL) {
                AVIStreamRelease(found);
            } else {
                result->length = length;
                result->fps = fps;
                result->width = width;
                result->height = height;
                result->avifile = file;
                result->getframe = NULL;
                result->stream = found;
                result->open_cb = NULL;
                result->callback = NULL;
                DAT_00665f48 = DAT_00665f48 + 1;
                return result;
            }
        }
        AVIFileRelease(file);
    }
    if (DAT_00665f48 == 0) {
        AVIFileExit();
    }
    return NULL;
}

// FUNCTION: LEGOLAND 0x00443d50
void CloseAviAnim(struct AnimHandle *handle) {
#ifdef LEGOLAND_PORT
    PortMovieRelease(handle->stream); /* [library:movie] */
#endif
    if (handle->getframe != NULL) {
        AVIStreamGetFrameClose(handle->getframe);
    }
    if (handle->stream != NULL) {
        AVIStreamRelease(handle->stream);
    }
    free(handle);
    DAT_00665f48 = DAT_00665f48 - 1;
    if (DAT_00665f48 == 0) {
        AVIFileExit();
    }
}

// FUNCTION: LEGOLAND 0x00443d90
void InitAdvisorFrameFormat(void) {
    DAT_004b7d7e = 0x10;
    DAT_004b7d74 = 0x70;
    DAT_004b7d78 = 0x60;
    DAT_004b7d84 = 0x5400;
}

// FUNCTION: LEGOLAND 0x00443dc0
void SetAdvisorVidAnim(struct AnimHandle *handle) {
    struct AnimHandle *current;

    DAT_00665f68 = 0;
    DAT_00665f6c = 0;
    DAT_00665f64 = 0;
    DAT_00665eec = 0;
    if (handle->open_cb != NULL) {
        handle->open_cb(handle);
    }
    current = (struct AnimHandle *)DAT_00665f5c;
    if (current != NULL) {
        AVIStreamGetFrameClose(current->getframe);
        ((struct AnimHandle *)DAT_00665f5c)->getframe = NULL;
    }
    DAT_00665f5c = handle;
    if (handle != NULL) {
        handle->getframe = AVIStreamGetFrameOpen(handle->stream, &DAT_004b7d70);
    }
}

struct AdvisorObject {
    /* 0x00 */ unsigned char pad_0[4];
    /* 0x04 */ struct Sprite *sprite;
    /* 0x08 */ unsigned char pad_8[0xc - 0x8];
    /* 0x0c */ short x;
    /* 0x0e */ short y;
};

// FUNCTION: LEGOLAND 0x00443e30
unsigned int RenderAdvisorIcon(struct AdvisorObject *param_1) {
    struct AnimHandle *anim;
    struct AviFrame *frame;
    int state[3];

    state[0] = 2;
    state[1] = (int)param_1;
    state[2] = 0;
    if (IsScriptStopped() == 0) {
        anim = (struct AnimHandle *)DAT_00665f5c;
        if (anim != NULL) {
            if (DAT_00665eec >= anim->length) {
                if (anim->callback != NULL) {
                    anim->callback();
                    anim = (struct AnimHandle *)DAT_00665f5c;
                }
                if (DAT_00665f60 == NULL) {
                    DAT_00665f60 = anim;
                }
                // STRING: LEGOLAND 0x004b7dc4
                DAT_00667c40 = "SetVidAnim";
                SetAdvisorVidAnim((struct AnimHandle *)DAT_00665f60);
                anim = (struct AnimHandle *)DAT_00665f5c;
                DAT_00665f60 = NULL;
            }
            // STRING: LEGOLAND 0x004b7db4
            DAT_00667c40 = "AVI GetFrame";
            /* [library:movie] the advisor animations are Indeo 5 AVIs: without that codec (not on Windows 11) there
             * is no frame decompressor and no frame, which the original dereferenced. The port decodes them itself
             * (port_movie.c). */
            frame = anim->getframe != NULL ? (struct AviFrame *)AVIStreamGetFrame(anim->getframe, DAT_00665eec) : NULL;
#ifdef LEGOLAND_PORT
            if (frame == NULL) {
                frame = (struct AviFrame *)PortMovieGetFrame(anim->stream, DAT_00665eec); /* [library:movie] */
            }
#endif
            // STRING: LEGOLAND 0x004b7da8
            DAT_00667c40 = "BltAdvisor";
            if (frame != NULL) {
                PushRenderingStatusAndLockVideoSurface();
                FUN_004659a0(frame, param_1->x, param_1->y);
                PopRenderingStatus();
            } else {
                /* [library:movie] nothing else paints the advisor window, so without a frame it showed whatever
                 * an earlier screen left in the back buffer. Clear it to black (112x96, the size the advisor
                 * AVIs are decoded to, see InitAdvisorFrameFormat). */
                unsigned short *row;
                int x;
                int y;

                PushRenderingStatusAndLockVideoSurface();
                for (y = 0; y < 0x60 && param_1->y + y < lpConfig->screen_height; y++) {
                    row = (unsigned short *)((char *)CurrentSurfaceDesc.lpSurface +
                        CurrentSurfaceDesc.lPitch * (param_1->y + y) + param_1->x * 2);
                    for (x = 0; x < 0x70 && param_1->x + x < lpConfig->screen_width; x++) {
                        row[x] = 0;
                    }
                }
                PopRenderingStatus();
            }
            DAT_00665eec++;
            // STRING: LEGOLAND 0x004b7d98
            DAT_00667c40 = "Exit Advisor";
            if (frame != NULL && (int)MousePos.x >= param_1->x && (int)MousePos.y >= param_1->y &&
                (int)MousePos.x < frame->width + param_1->x && (int)MousePos.y < frame->height + param_1->y) {
                Hover.type = state[0];
                Hover.ptr = state[1];
                Hover.data.value = state[2];
            }
        }
    } else {
        PushRenderingStatusAndLockVideoSurface();
        PrintSprite(param_1->sprite, param_1->x, param_1->y, 0, state);
        PopRenderingStatus();
    }
    return 0;
}

// FUNCTION: LEGOLAND 0x00443f90
void *GetAdvisorAnimForState(unsigned int param_1) {
    switch (param_1) {
    case 1:
        return AdLRAnim;
    case 2:
        return AdPhoneAnim;
    case 3:
        return AdPhoneGestureAnim;
    case 4:
        return AdPhoneDownAnim;
    case 5:
        return AdWobbleAnim;
    default:
        return AdBlinkAnim;
    }
}

// FUNCTION: LEGOLAND 0x00443fe0
unsigned int GetNextAdvisorAnimState(unsigned int param_1) {
    switch (param_1) {
    case 1:
        return 1;
    case 2:
        return 3;
    case 3:
        return 4;
    case 4:
    case 5:
        return 5;
    default:
        return 0;
    }
}

// FUNCTION: LEGOLAND 0x00444020
void *OnAdvisorAnimEnd(void) {
    unsigned int handle;

    if (DAT_00665fec == DAT_00665fe8) {
        DAT_00665fec = GetNextAdvisorAnimState(DAT_00665fe8);
    }
    handle = DAT_00665fec;
    DAT_00665fe8 = handle;
    DAT_0081c088 = DAT_0081c09c;
    DAT_0081c09c = 0;
    DAT_00665f60 = GetAdvisorAnimForState(handle);
    return DAT_00665f60;
}

// FUNCTION: LEGOLAND 0x00444070
void SetNextAdvisorAnimState(unsigned int param_1, unsigned int param_2) {
    DAT_00665fec = param_1;
    DAT_0081c09c = param_2;
    DAT_0081c088 = 0;
}

// FUNCTION: LEGOLAND 0x00444090
void LoadAdvisorAnims(void) {
    InitAdvisorFrameFormat();

    // STRING: LEGOLAND 0x004b7e24
    AdBlinkAnim = OpenAviAnim("AD_Blink.avi");
    if (AdBlinkAnim != 0) {
        ((struct AnimHandle *)AdBlinkAnim)->callback = OnAdvisorAnimEnd;
    }
    // STRING: LEGOLAND 0x004b7e18
    AdLRAnim = OpenAviAnim("AD_LR.avi");
    if (AdLRAnim != 0) {
        ((struct AnimHandle *)AdLRAnim)->callback = OnAdvisorAnimEnd;
    }
    // STRING: LEGOLAND 0x004b7e08
    AdPhoneAnim = OpenAviAnim("AD_Phone.avi");
    if (AdPhoneAnim != 0) {
        ((struct AnimHandle *)AdPhoneAnim)->callback = OnAdvisorAnimEnd;
    }
    // STRING: LEGOLAND 0x004b7df4
    AdPhoneGestureAnim = OpenAviAnim("AD_PhoneGesture.avi");
    if (AdPhoneGestureAnim != 0) {
        ((struct AnimHandle *)AdPhoneGestureAnim)->callback = OnAdvisorAnimEnd;
    }
    // STRING: LEGOLAND 0x004b7de0
    AdPhoneDownAnim = OpenAviAnim("AD_PhoneDown.avi");
    if (AdPhoneDownAnim != 0) {
        ((struct AnimHandle *)AdPhoneDownAnim)->callback = OnAdvisorAnimEnd;
    }
    // STRING: LEGOLAND 0x004b7dd0
    AdWobbleAnim = OpenAviAnim("AD_Wobble.avi");
    if (AdWobbleAnim != 0) {
        ((struct AnimHandle *)AdWobbleAnim)->callback = OnAdvisorAnimEnd;
    }
    SetAdvisorVidAnim(AdBlinkAnim);
}

// FUNCTION: LEGOLAND 0x00444150
void FreeAdvisorAnims(void) {
    if (AdBlinkAnim != 0) {
        CloseAviAnim(AdBlinkAnim);
        AdBlinkAnim = 0;
    }
    if (AdLRAnim != 0) {
        CloseAviAnim(AdLRAnim);
        AdLRAnim = 0;
    }
    if (AdPhoneAnim != 0) {
        CloseAviAnim(AdPhoneAnim);
        AdPhoneAnim = 0;
    }
    if (AdPhoneGestureAnim != 0) {
        CloseAviAnim(AdPhoneGestureAnim);
        AdPhoneGestureAnim = 0;
    }
    if (AdPhoneDownAnim != 0) {
        CloseAviAnim(AdPhoneDownAnim);
        AdPhoneDownAnim = 0;
    }
    if (AdWobbleAnim != 0) {
        CloseAviAnim(AdWobbleAnim);
        AdWobbleAnim = 0;
    }
}

// FUNCTION: LEGOLAND 0x004441f0
void ResetReportFlags(void) {
    ReportFlags = 0;
}

// FUNCTION: LEGOLAND 0x00444200
unsigned int SaveReport(void) {
    unsigned int elapsed;

    if (SaveGameWrite(&ReportFlags, 0xa0) == 0) {
        return 0;
    }
    if (AppraisalDeadline == 0) {
        elapsed = 0xffffffff;
    } else {
        elapsed = AppraisalDeadline - GetGameTimer();
    }
    return SaveGameWrite(&elapsed, 0x4) != 0;
}

// FUNCTION: LEGOLAND 0x00444260
unsigned int LoadReport(void) {
    unsigned int elapsed;

    if (SaveGameRead(&ReportFlags, 0xa0) == 0) {
        return 0;
    }
    if (SaveGameRead(&elapsed, 0x4) == 0) {
        return 0;
    }
    if (elapsed == 0xffffffff) {
        AppraisalDeadline = 0;
        return 1;
    }
    AppraisalDeadline = GetGameTimer() + elapsed;
    return 1;
}

// FUNCTION: LEGOLAND 0x004442c0
int GetDrivingSchoolScore(void) {
    struct RideElem *elem;

    elem = (struct RideElem *)ElemID("DRIVING SCHOOL"); /* TODO: fold — ElemID handle (uint) viewed as RideElem* */
    if ((elem->flags & 1) != 0) {
        return elem->obj->func(elem->obj->arg, 0);
    }
    return 0;
}

// FUNCTION: LEGOLAND 0x004442f0
int GetBoatingSchoolScore(void) {
    struct RideElem *elem;

    elem = (struct RideElem *)ElemID("BOATING SCHOOL");
    if ((elem->flags & 1) != 0) {
        return elem->obj->func(elem->obj->arg, 0);
    }
    return 0;
}

// FUNCTION: LEGOLAND 0x00444320
int GetCastleScore(void) {
    struct RideElem *elem;

    elem = (struct RideElem *)ElemID("CASTLE OBJ");
    if ((elem->flags & 1) != 0) {
        return elem->obj->func(elem->obj->arg, 0);
    }
    return 0;
}

// FUNCTION: LEGOLAND 0x00444350
int GetLogFlumeScore(void) {
    struct RideElem *elem;

    elem = (struct RideElem *)ElemID("LOG FLUME ENTRANCE");
    if ((elem->flags & 1) != 0) {
        return elem->obj->func(elem->obj->arg, 0);
    }
    return 0;
}

// FUNCTION: LEGOLAND 0x00444380
int GetJungleCruiseScore(void) {
    struct RideElem *elem;

    elem = (struct RideElem *)ElemID("JUNGLE CRUISE");
    if ((elem->flags & 1) != 0) {
        return elem->obj->func(elem->obj->arg, 0);
    }
    return 0;
}

// FUNCTION: LEGOLAND 0x004443b0
unsigned int FUN_004443b0(unsigned int param_1, unsigned int param_2) {
    if ((param_1 | param_2) != 0) {
        DAT_00665ffc = param_1;
        ReportFlags |= 1;
        return param_1;
    }
    ReportFlags = (ReportFlags & 0xffffff00) | ((ReportFlags & 0xff) & 0xfe);
    return ReportFlags;
}

// FUNCTION: LEGOLAND 0x004443e0
unsigned int FUN_004443e0(unsigned int param_1, unsigned int param_2) {
    if ((param_1 | param_2) != 0) {
        DAT_00666000 = param_1;
        ReportFlags |= 2;
        return param_1;
    }
    ReportFlags &= ~2;
    return ReportFlags;
}

// FUNCTION: LEGOLAND 0x00444410
unsigned int FUN_00444410(unsigned int param_1, unsigned int param_2) {
    if ((param_1 | param_2) != 0) {
        DAT_00666004 = param_1;
        ReportFlags |= 4;
        return param_1;
    }
    ReportFlags &= ~4;
    return ReportFlags;
}

// FUNCTION: LEGOLAND 0x00444440
void FUN_00444440(unsigned int param_1, unsigned int param_2) {
    if ((param_1 | param_2) != 0) {
        ReportFlags |= 8;
        DAT_00666008 = param_1;
    } else {
        ReportFlags = (ReportFlags & 0xffffff00) | (ReportFlags & 0xf7);
    }
}

// FUNCTION: LEGOLAND 0x00444470
void SetReportCastleScoreGoal(unsigned int param_1, unsigned int param_2) {
    if ((param_1 | param_2) != 0) {
        ReportFlags |= (param_2 & 3) << 4;
        DAT_0066600c = param_1;
    } else {
        ReportFlags = (ReportFlags & 0xffffff00) | (ReportFlags & 0xcf);
    }
}

// FUNCTION: LEGOLAND 0x004444b0
void SetReportDrivingSchoolScoreGoal(unsigned int param_1, unsigned int param_2) {
    if ((param_1 | param_2) != 0) {
        ReportFlags |= (param_2 & 3) << 6;
        DAT_00666010 = param_1;
    } else {
        ReportFlags = (ReportFlags & 0xffffff00) | (ReportFlags & 0x3f);
    }
}

// FUNCTION: LEGOLAND 0x004444f0
void SetReportLogFlumeScoreGoal(unsigned int param_1, unsigned int param_2) {
    if ((param_1 | param_2) != 0) {
        ReportFlags |= (param_2 & 3) << 8;
        DAT_00666014 = param_1;
    } else {
        ReportFlags = (ReportFlags & 0xffff00ff) | ((ReportFlags >> 8 & 0xfc) << 8);
    }
}

// FUNCTION: LEGOLAND 0x00444530
void SetReportBoatingSchoolScoreGoal(unsigned int param_1, unsigned int param_2) {
    if ((param_1 | param_2) != 0) {
        ReportFlags |= (param_2 & 3) << 10;
        DAT_00666018 = param_1;
    } else {
        ReportFlags = (ReportFlags & 0xffff00ff) | ((ReportFlags >> 8 & 0xf3) << 8);
    }
}

// FUNCTION: LEGOLAND 0x00444570
void SetReportJungleCruiseScoreGoal(unsigned int param_1, unsigned int param_2) {
    if ((param_1 | param_2) != 0) {
        ReportFlags |= (param_2 & 3) << 12;
        DAT_0066601c = param_1;
    } else {
        ReportFlags = (ReportFlags & 0xffff00ff) | ((ReportFlags >> 8 & 0xcf) << 8);
    }
}

// FUNCTION: LEGOLAND 0x004445b0
void SetReportType1Or3ObjectCountGoal(unsigned int param_1, unsigned int param_2) {
    if ((param_1 | param_2) != 0) {
        ReportFlags |= 0x4000;
        DAT_00666020 = param_1;
        DAT_00666024 = param_2;
    } else {
        ReportFlags &= ~0x4000;
    }
}

// FUNCTION: LEGOLAND 0x004445f0
void SetReportType1Or3ClassCountGoal(unsigned int param_1, unsigned int param_2) {
    if ((param_1 | param_2) != 0) {
        ReportFlags |= 0x8000;
        DAT_00666028 = param_1;
        DAT_0066602c = param_2;
    } else {
        ReportFlags &= ~0x8000;
    }
}

// FUNCTION: LEGOLAND 0x00444630
void SetReportLinkedObjectPercentGoal(unsigned int param_1, unsigned int param_2) {
    if ((param_1 | param_2) != 0) {
        ReportFlags |= 0x40000;
        DAT_00666030 = param_1;
        DAT_00666034 = param_2;
    } else {
        ReportFlags &= 0xfffbffff;
    }
}

// FUNCTION: LEGOLAND 0x00444670
void SetReportType2ObjectCountGoal(unsigned int param_1, unsigned int param_2) {
    if ((param_1 | param_2) != 0) {
        ReportFlags |= 0x8000000;
        DAT_00666070 = param_1;
        DAT_00666074 = param_2;
    } else {
        ReportFlags &= 0xf7ffffff;
    }
}

// FUNCTION: LEGOLAND 0x004446b0
void SetReportType2ClassCountGoal(unsigned int param_1, unsigned int param_2) {
    if ((param_1 | param_2) != 0) {
        ReportFlags |= 0x10000000;
        DAT_00666078 = param_1;
        DAT_0066607c = param_2;
    } else {
        ReportFlags &= 0xefffffff;
    }
}

// FUNCTION: LEGOLAND 0x004446f0
void FUN_004446f0(unsigned int param_1, unsigned int param_2) {
    if ((param_1 | param_2) != 0) {
        ReportFlags |= 0x20000000;
        DAT_00666080 = param_1;
        DAT_00666084 = param_2;
    } else {
        ReportFlags &= 0xdfffffff;
    }
}

// FUNCTION: LEGOLAND 0x00444730
void SetReportFoodStallObjectCountGoal(unsigned int param_1, unsigned int param_2) {
    if ((param_1 | param_2) != 0) {
        ReportFlags |= 0x10000;
        DAT_00666040 = param_1;
        DAT_00666044 = param_2;
    } else {
        ReportFlags &= 0xfffeffff;
    }
}

// FUNCTION: LEGOLAND 0x00444770
void SetReportFoodStallClassCountGoal(unsigned int param_1, unsigned int param_2) {
    if ((param_1 | param_2) != 0) {
        ReportFlags |= 0x20000;
        DAT_00666048 = param_1;
        DAT_0066604c = param_2;
    } else {
        ReportFlags &= 0xfffdffff;
    }
}

// FUNCTION: LEGOLAND 0x004447b0
void SetReportType4ObjectCountGoal(unsigned int param_1, unsigned int param_2) {
    if ((param_1 | param_2) != 0) {
        ReportFlags |= 0x40000000;
        DAT_00666088 = param_1;
        DAT_0066608c = param_2;
    } else {
        ReportFlags &= 0xbfffffff;
    }
}

// FUNCTION: LEGOLAND 0x004447f0
void SetReportType4ClassCountGoal(unsigned int param_1, unsigned int param_2) {
    if ((param_1 | param_2) != 0) {
        ReportFlags |= 0x80000000;
        DAT_00666090 = param_1;
        DAT_00666094 = param_2;
    } else {
        ReportFlags &= 0x7fffffff;
    }
}

// FUNCTION: LEGOLAND 0x00444830
void SetReportVisitorCountGoal(unsigned int param_1, unsigned int param_2) {
    if ((param_1 | param_2) != 0) {
        ReportFlags |= 0x80000;
        DAT_00666038 = param_1;
        DAT_0066603c = param_2;
    } else {
        ReportFlags &= 0xfff7ffff;
    }
}

// FUNCTION: LEGOLAND 0x00444870
void SetReportVisitorHappinessGoal(unsigned int param_1, unsigned int param_2) {
    if ((param_1 | param_2) != 0) {
        ReportFlags |= 0x1000000;
        DAT_00666050 = param_1;
        DAT_00666054 = param_2;
    } else {
        ReportFlags &= 0xfeffffff;
    }
}

// FUNCTION: LEGOLAND 0x004448b0
void SetReportVisitorFullnessGoal(unsigned int param_1, unsigned int param_2) {
    if ((param_1 | param_2) != 0) {
        ReportFlags |= 0x4000000;
        DAT_00666050 = param_1;
        DAT_00666054 = param_2;
    } else {
        ReportFlags &= 0xfbffffff;
    }
}

// FUNCTION: LEGOLAND 0x004448f0
void SetReportPowerSupplyGoal(unsigned int param_1, unsigned int param_2) {
    if ((param_1 | param_2) != 0) {
        ReportFlags |= 0x200000;
        DAT_00666058 = param_1;
        DAT_0066605c = param_2;
    } else {
        ReportFlags &= 0xffdfffff;
    }
}

// FUNCTION: LEGOLAND 0x00444930
void FUN_00444930(unsigned int param_1, unsigned int param_2) {
    if ((param_1 | param_2) != 0) {
        ReportFlags |= 0x400000;
        DAT_00666060 = param_1;
        DAT_00666064 = param_2;
    } else {
        ReportFlags &= 0xffbfffff;
    }
}

// FUNCTION: LEGOLAND 0x00444970
void SetReportMapTileCountGoal(unsigned int param_1, unsigned int param_2) {
    if ((param_1 | param_2) != 0) {
        ReportFlags |= 0x800000;
        DAT_00666068 = param_1;
        DAT_0066606c = param_2;
    } else {
        ReportFlags &= 0xff7fffff;
    }
}

// FUNCTION: LEGOLAND 0x004449b0
void LoadAppraisalSprites(void) {
    int i;
    char buffer[0x20];

    buffer[0] = 0;
    memset(buffer + 1, 0, 0x1f);
    // [port:rewrite] the original walked AppCrossSprites with slot[-5] / slot[5] reaching into the neighbouring
    // arrays and stopped at &AppBulletSprites; index the three arrays instead so it doesn't rely on their layout.
    for (i = 0; i < 5; i++) {
        // STRING: LEGOLAND 0x004b8114
        sprintf(buffer, "App_tick%d.lls", i);
        AppTickSprites[i] = LoadSprite(buffer, 4);
        // STRING: LEGOLAND 0x004b8104
        sprintf(buffer, "App_cross%d.lls", i);
        AppCrossSprites[i] = LoadSprite(buffer, 4);
        // STRING: LEGOLAND 0x004b80f0
        sprintf(buffer, "App_bullet%d.lls", i);
        AppBulletSprites[i] = LoadSprite(buffer, 4);
    }
    // STRING: LEGOLAND 0x004b80dc
    AppBarMarkerSprite = LoadSprite("App_barmarker.lls", 4);
    // STRING: LEGOLAND 0x004b80d0
    AppBarSprite = LoadSprite("App_bar.lls", 4);
}

// FUNCTION: LEGOLAND 0x00444a70
void DrawAppraisalBar(RECT rc, int value, int max, int goal) {
    int negative;
    int bar;
    unsigned int colour;
    int mark;

    if (max < 0) {
        negative = 1;
        max = -max;
        goal = max - goal;
    } else {
        negative = 0;
    }
    if (value > max) {
        value = max;
    }
    if (negative != 0) {
        value = max - value;
    }
    if (value >= goal) {
        colour = GetNearestColour(0, 0xff, 0);
    } else {
        colour = GetNearestColour(0xff, 0, 0);
    }
    bar = ((rc.right - rc.left) * value) / max;
    PrintSprite(AppBarSprite, rc.left, rc.top, 0, 0);
    RenderBlock(rc.left + 3, rc.top + 2, bar - 2, 1, colour);
    RenderBlock(rc.left + 2, rc.top + 3, bar, (rc.bottom - rc.top) - 1, colour);
    mark = (((rc.right - rc.left) - 2) * goal) / max + 2 + rc.left;
    PrintSprite(AppBarMarkerSprite, mark, rc.top + 2, 0, 0);
}

// FUNCTION: LEGOLAND 0x00444b70
void DrawAppraisalMark(unsigned int param_1, unsigned int param_2, int param_3, unsigned int param_4) {
    PushRenderingStatusAndLockVideoSurface();
    if (param_3 == 1) {
        PrintSprite(AppTickSprites[param_4], param_1, param_2, 0, 0);
    } else if (param_3 == 0) {
        PrintSprite(AppCrossSprites[param_4], param_1, param_2, 0, 0);
    } else if (param_3 == -1) {
        PrintSprite(AppBulletSprites[param_4], param_1 + 5, param_2 + 5, 0, 0);
    }
    PopRenderingStatus();
}

// FUNCTION: LEGOLAND 0x00444bf0
void CountClassesOfType1Or3(int *param_1, int *param_2) {
    struct ObjectClass *node;

    node = ObjectClassList;
    *param_1 = 0;
    *param_2 = 0;
    if (node == 0) {
        return;
    }
    do {
        if (node->count != 0 && (node->type == 1 || node->type == 3)) {
            *param_1 += node->count;
            *param_2 += 1;
        }
        node = node->next;
    } while (node != 0);
}

// FUNCTION: LEGOLAND 0x00444c40
int IsRidePartClass(struct ObjectClass *node) {
    return FindStringNoCase((const char *)**(int **)((char *)node + 0xc4), &DAT_004b7e9c, 0x16) >= 0;
}

// FUNCTION: LEGOLAND 0x00444c70
void CountType2ClassesExceptRideParts(int *param_1, int *param_2) {
    struct ObjectClass *node;

    node = ObjectClassList;
    *param_1 = 0;
    *param_2 = 0;
    if (node == 0) {
        return;
    }
    do {
        if (node->count != 0 && node->type == 2 && IsRidePartClass(node) == 0) {
            *param_1 += node->count;
            *param_2 += 1;
        }
        node = node->next;
    } while (node != 0);
}

// FUNCTION: LEGOLAND 0x00444cd0
void CountClassesOfType4(int *param_1, int *param_2) {
    struct ObjectClass *node;

    node = ObjectClassList;
    *param_1 = 0;
    *param_2 = 0;
    if (node == 0) {
        return;
    }
    do {
        if (node->count != 0 && node->type == 4) {
            *param_1 += node->count;
            *param_2 += 1;
        }
        node = node->next;
    } while (node != 0);
}

// FUNCTION: LEGOLAND 0x00444d20
void CountFoodStallClasses(int *param_1, int *param_2) {
    struct ObjectClass *node;

    node = ObjectClassList;
    *param_1 = 0;
    *param_2 = 0;
    if (node == 0) {
        return;
    }
    do {
        if (node->count != 0 && node->type == 5) {
            *param_1 += node->count;
            *param_2 += 1;
        }
        node = node->next;
    } while (node != 0);
}

// FUNCTION: LEGOLAND 0x00444d70
void SurveyBlokes(int *count, int *happiness, int *fullness) {
    struct Bloke *bloke;
    signed char value;

    bloke = FirstBloke;
    *count = 0;
    *happiness = 0;
    *fullness = 0;
    if (bloke == 0) {
        return;
    }
    do {
        *count += 1;
        if (bloke->mood > MapStats.mood_threshold3) {
            *happiness += 1;
        }
        if (bloke->mood > MapStats.mood_threshold4) {
            *happiness += 1;
        }
        value = GetBlokeHungerLevel(bloke);
        *fullness += 4 - value;
        bloke = bloke->next;
    } while (bloke != 0);
}

struct GslImage {
    /* 0x00 */ unsigned char pad_0[0xc];
    /* 0x0c */ int field_c;
    /* 0x10 */ int field_10;
    /* 0x14 */ unsigned char pad_14[0x20 - 0x14];
    /* 0x20 */ short type;
    /* 0x22 */ unsigned char pad_22[0x78 - 0x22];
    /* 0x78 */ char *name;
};

struct RenderObjVtbl {
    /* 0x00 */ unsigned char pad_0[0xc];
    /* 0x0c */ struct GslImage *img;
};

struct RenderObj {
    /* 0x00 */ struct RenderObjVtbl *vtbl;
    /* 0x04 */ unsigned char field_4;
    /* 0x05 */ unsigned char field_5;
    /* 0x06 */ unsigned char pad_6[0xc - 0x6];
    /* 0x0c */ unsigned char flags;
};

// FUNCTION: LEGOLAND 0x00444df0
int GetLinkedObjectPercent(void) {
    struct RenderObj *obj;
    struct GslImage *img;
    short type;
    int ix;
    int iy;
    int linked;
    int total;
    struct Point coord;

    obj = (struct RenderObj *)GetFirstRenderObject();
    linked = 0;
    total = 0;
    UpdatePathLinks(1);
    if (obj != NULL) {
        do {
            if ((obj->flags & 0x80) != 0) {
                img = obj->vtbl->img;
                type = img->type;
                if (type == 1 || type == 4 || type == 5) {
                    ix = img->field_c;
                    iy = img->field_10;
                    coord.x = obj->field_4 + ix;
                    coord.y = obj->field_5 + iy;
                    total = total + 1;
                    if (FUN_00482b60(&coord) != 0) {
                        linked = linked + 1;
                    } else {
                        // STRING: LEGOLAND 0x004b8124
                        DBPrintf("Unlinked Object %s\n", img->name);
                    }
                }
            }
            obj = (struct RenderObj *)GetNextRenderObject((MapElement *)obj);
        } while (obj != NULL);
        if (total != 0) {
            return (linked * 100) / total;
        }
    }
    return 100;
}

// FUNCTION: LEGOLAND 0x00444eb0
unsigned char AppraisalBackClicked(unsigned int param_1, unsigned int param_2, unsigned int param_3, unsigned int param_4) {
    if ((param_2 & 2) != 0) {
        PlayInstanceOfSample(GameFX[FX_BUTTON4].sample, 0, 1, 0);
        AppraisalScreenActive = 0;
        AppraisalNextIcon = 0;
        AppraisalPrevIcon = 0;
    }
    return 1;
}

// FUNCTION: LEGOLAND 0x00444ef0
unsigned char AppraisalNextPageClicked(unsigned int param_1, unsigned int param_2) {
    struct IconNode *icon;

    icon = AppraisalNextIcon;
    if (icon == NULL) {
        return 1;
    }
    if (((icon->flags == 0) & 0x400) != 0) {
        SetIconSprite(icon, NextPageLitSprite);
        icon = AppraisalNextIcon;
    }
    if ((param_2 & 2) == 0) {
        return 1;
    }
    if ((icon->flags & 0x400) != 0) {
        return (unsigned char)AppraisalBackClicked(0, param_2, 0, 0);
    }
    AppraisalPageChanged = 1;
    PlayInstanceOfSample(GameFX[FX_BUTTON4].sample, 0, 1, 0);
    SpeechCloseFile();
    DAT_006687b0 = 4;
    if (AppraisalPage < AppraisalPageCount - 1) {
        AppraisalPage = AppraisalPage + 1;
    }
    UpdateAppraisalPageButtons();
    return 1;
}

// FUNCTION: LEGOLAND 0x00444f90
unsigned char AppraisalPrevPageClicked(unsigned int param_1, unsigned char param_2) {
    SetIconSprite(AppraisalPrevIcon, PreviousPageLitSprite);
    if ((param_2 & 2) != 0) {
        AppraisalPageChanged = 1;
        PlayInstanceOfSample(GameFX[FX_BUTTON4].sample, 0, 1, 0);
        if (AppraisalPage != 0) {
            AppraisalPage = AppraisalPage - 1;
        }
        SpeechCloseFile();
        DAT_006687b0 = 4;
        UpdateAppraisalPageButtons();
    }
    return 1;
}

// FUNCTION: LEGOLAND 0x00445000
void KillAppraisalSprites(void) {
    int i;

    // [port:rewrite] the original walked AppCrossSprites by address, reaching AppTickSprites at -0x14 and
    // AppBulletSprites at +0x14; index the three arrays instead so it doesn't rely on their layout.
    for (i = 0; i < 5; i++) {
        if (AppTickSprites[i] != 0) {
            KillSprite(AppTickSprites[i]);
            AppTickSprites[i] = 0;
        }
        if (AppCrossSprites[i] != 0) {
            KillSprite(AppCrossSprites[i]);
            AppCrossSprites[i] = 0;
        }
        if (AppBulletSprites[i] != 0) {
            KillSprite(AppBulletSprites[i]);
            AppBulletSprites[i] = 0;
        }
    }
    if (AppBarMarkerSprite != 0) {
        KillSprite(AppBarMarkerSprite);
        AppBarMarkerSprite = 0;
    }
    if (AppBarSprite != 0) {
        KillSprite(AppBarSprite);
        AppBarSprite = 0;
    }
    if (SPRITE_TitleScreenBk != 0) {
        KillSprite(SPRITE_TitleScreenBk);
        SPRITE_TitleScreenBk = 0;
    }
    if (NextPageSprite != 0) {
        KillSprite(NextPageSprite);
        NextPageSprite = 0;
    }
    if (NextPageLitSprite != 0) {
        KillSprite(NextPageLitSprite);
        NextPageLitSprite = 0;
    }
    if (PreviousPageSprite != 0) {
        KillSprite(PreviousPageSprite);
        PreviousPageSprite = 0;
    }
    if (PreviousPageLitSprite != 0) {
        KillSprite(PreviousPageLitSprite);
        PreviousPageLitSprite = 0;
    }
    RemoveIconGroup(1);
}

// FUNCTION: LEGOLAND 0x00445100
void UnlightAppraisalPageButtons(void) {
    struct IconNode *a;
    struct IconNode *b;
    int x;
    int y;

    a = AppraisalNextIcon;
    x = MousePos.x;
    if (x < a->x || x > a->width + a->x || (y = MousePos.y, y < a->y || y > a->height + a->y)) {
        SetIconSprite(a, NextPageSprite);
    }
    x = MousePos.x;
    y = MousePos.y;
    b = AppraisalPrevIcon;
    if (x < b->x || x > b->width + b->x ||
        y < b->y || y > b->height + b->y) {
        SetIconSprite(b, PreviousPageSprite);
    }
}

// FUNCTION: LEGOLAND 0x00445190
void InitAppraisalScreen(int pages) {
    struct IconNode *icon;

    NextPageSprite = LoadSprite("NextPage.lls", 4);
    NextPageLitSprite = LoadSprite("NextPageLit.lls", 4);
    PreviousPageSprite = LoadSprite("PreviousPage.lls", 4);
    PreviousPageLitSprite = LoadSprite("PreviousPageLit.lls", 4);
    // STRING: LEGOLAND 0x004b8150
    SPRITE_TitleScreenBk = LoadSprite("AppraisalBK.lls", 0);
    // STRING: LEGOLAND 0x004b8138
    icon = LoadSpriteIcon("GoBack_on_Report.lls", 4, 0x1fb, 0x161, 1);
    icon->string_id = 0xde;
    icon->string = GetString(0xde);
    icon->flags |= 0x6002;
    icon->event_handler = (void *)AppraisalBackClicked;
    DAT_006687c0 = (unsigned int)AppraisalBackClicked;
    AppraisalNextIcon = InsertIcon(0x1b9, 0x1ae, 1, NextPageSprite);
    AppraisalNextIcon->string_id = 0xdc;
    AppraisalNextIcon->string = GetString(0xdc);
    AppraisalNextIcon->flags |= 0x2000;
    AppraisalNextIcon->flags |= 0x4002;
    AppraisalNextIcon->event_handler = (void *)AppraisalNextPageClicked;
    DAT_006687bc = (unsigned int)AppraisalNextPageClicked;
    AppraisalPrevIcon = InsertIcon(6, 0x1ae, 1, PreviousPageSprite);
    AppraisalPrevIcon->string_id = 0xdd;
    AppraisalPrevIcon->string = GetString(0xdd);
    AppraisalPrevIcon->flags |= 0x2000;
    AppraisalPrevIcon->flags |= 0x4002;
    AppraisalPrevIcon->event_handler = (void *)AppraisalPrevPageClicked;
    UpdateAppraisalPageButtons();
    AppraisalScreenActive = 1;
}

// FUNCTION: LEGOLAND 0x00445310
void UpdateAppraisalPageButtons(void) {
    if (AppraisalPageCount <= 1 || AppraisalPage >= AppraisalPageCount - 1) {
        AppraisalNextIcon->event_handler = 0;
        AppraisalNextIcon->flags |= 0x400;
    } else {
        AppraisalNextIcon->event_handler = (void *)AppraisalNextPageClicked;
        AppraisalNextIcon->flags &= 0xfffffbff;
    }
    if (AppraisalPage != 0) {
        AppraisalPrevIcon->event_handler = (void *)AppraisalPrevPageClicked;
        AppraisalPrevIcon->flags &= 0xfffffbff;
    } else {
        AppraisalPrevIcon->event_handler = 0;
        AppraisalPrevIcon->flags |= 0x400;
    }
}

/* RunAppraisal's report layout. Each row of the report is a struct AppraisalRow; a row is 0x16 high and rows are
 * 0x18 apart. A row whose bottom would pass 0x1b5 goes to the top of a new page. rc.layout is the first row's
 * rectangle on a page, rc.cur the current row's. The report is built in sections (a header row, then one row per
 * check); a section that does not fit on the page it started on is written again from its header on a new page:
 * every page test of the section jumps to one restart block (the original has a single copy of it, at the
 * section's last page test). These restarts, and the retries of section headers, are gotos: MSVC6 only produces the
 * original's single shared restart block from explicit jumps (it does not merge separate copies reliably), so this
 * function is a deliberate exception to the no-goto rule. */

/* back to the top of the page: the original reloads the layout rectangle from memory at every page break */
#define APPR_TOP_OF_PAGE() \
    rc.cur.left = rc.layout.left; \
    y = rc.layout.top; \
    rc.cur.right = rc.layout.right; \
    bottom = rc.layout.bottom
/* ... and the row starts a new page */
#define APPR_NEW_PAGE() \
    AppraisalPageCount++; \
    pagestart = i; \
    rc.cur.left = rc.layout.left; \
    y = rc.layout.top; \
    rc.cur.right = rc.layout.right; \
    rc.cur.bottom = rc.layout.bottom
/* a section header that does not fit: retry it at the top of a new page */
#define APPR_RETRY(label) \
    { \
        AppraisalPageCount++; \
        pagestart = i; \
        goto label; \
    }
/* the common start of a text row */
#define APPR_ROW(kind, id) \
    recs[i].page = AppraisalPageCount; \
    recs[i].x = x; \
    recs[i].type = (kind); \
    recs[i].rnd = rand() % 5; \
    recs[i].text = GetString(id)

/* RunAppraisal's report layout. Each row of the report is a struct AppraisalRow; a row is 0x16 high and rows are
 * 0x18 apart. A row whose bottom would pass 0x1b5 goes to the top of a new page. rc.layout is the first row's
 * rectangle on a page, rc.cur the current row's. The report is built in sections (a header row, then one row per
 * check); a section that does not fit on the page it started on is written again from its header on a new page.
 * Every page test carries its own copy of that restart and jumps back to the section's head (the only gotos here);
 * the compiler merges the copies into one block per section, as in the original, and the copies' references are
 * what make pagestart and x the heaviest locals, so they come first in the frame, as in the original. */

/* back to the top of the page: the original reloads the layout rectangle from memory at every page break */
#define APPR_TOP_OF_PAGE() \
    rc.cur.left = rc.layout.left; \
    y = rc.layout.top; \
    rc.cur.right = rc.layout.right; \
    bottom = rc.layout.bottom
/* ... and the row starts a new page */
#define APPR_NEW_PAGE() \
    AppraisalPageCount++; \
    pagestart = i; \
    rc.cur.left = rc.layout.left; \
    y = rc.layout.top; \
    rc.cur.right = rc.layout.right; \
    rc.cur.bottom = rc.layout.bottom
/* a section header that does not fit: retry it at the top of a new page */
#define APPR_RETRY(label) \
    { \
        AppraisalPageCount++; \
        pagestart = i; \
        goto label; \
    }
/* the common start of a text row */
#define APPR_ROW(kind, id) \
    recs[i].page = AppraisalPageCount; \
    recs[i].x = x; \
    recs[i].type = (kind); \
    recs[i].rnd = rand() % 5; \
    recs[i].text = GetString(id)

/* RunAppraisal's report layout. Each row of the report is a struct AppraisalRow; a row is 0x16 high and rows are
 * 0x18 apart. A row whose bottom would pass 0x1b5 goes to the top of a new page. rc.layout is the first row's
 * rectangle on a page, rc.cur the current row's. The report is built in sections (a header row, then one row per
 * check); a section that does not fit on the page it started on is written again from its header on a new page.
 * Every page test carries its own copy of that restart and jumps back to the section's head (the only gotos here);
 * the compiler merges the copies into one block per section, as in the original, and the copies' references are
 * what make pagestart and x the heaviest locals, so they come first in the frame, as in the original. */

/* back to the top of the page: the original reloads the layout rectangle from memory at every page break */
#define APPR_TOP_OF_PAGE() \
    rc.cur.left = rc.layout.left; \
    y = rc.layout.top; \
    rc.cur.right = rc.layout.right; \
    bottom = rc.layout.bottom
/* ... and the row starts a new page */
#define APPR_NEW_PAGE() \
    AppraisalPageCount++; \
    pagestart = i; \
    rc.cur.left = rc.layout.left; \
    y = rc.layout.top; \
    rc.cur.right = rc.layout.right; \
    rc.cur.bottom = rc.layout.bottom
/* a section header that does not fit: retry it at the top of a new page */
#define APPR_RETRY(label) \
    { \
        AppraisalPageCount++; \
        pagestart = i; \
        goto label; \
    }
/* the common start of a text row */
#define APPR_ROW(kind, id) \
    recs[i].page = AppraisalPageCount; \
    recs[i].x = x; \
    recs[i].type = (kind); \
    recs[i].rnd = rand() % 5; \
    recs[i].text = GetString(id)

// FUNCTION: LEGOLAND 0x004453a0
unsigned int RunAppraisal(void) {
    int i; /* the row being written (= rows written so far) */
    int y; /* top of the row being written */
    int bottom; /* its bottom (y + 0x16), tested against the page bottom 0x1b5 */
    int pagestart; /* first row of the current page */
    int x; /* indent of the row; +0x30 inside a section */
    int first; /* the current section's header row */
    int total; /* checks in the current section */
    int pass; /* checks passed in the current section */
    int ok; /* the current check passed */
    int flags; /* failed checks, one bit each */
    int totacc; /* checks in all sections */
    int passacc; /* checks passed in all sections */
    int val; /* a check's measured value, when it is kept for the row */
    int *xp; /* &recs[first].x: x of the current section's header row, restored on a restart */
    int bf0_total;
    int bf0_count;
    int c70_total;
    int c70_count;
    int cd0_total;
    int cd0_count;
    int d20_total;
    int d20_count;
    int d70_count;
    int d70_p2;
    int d70_p3;
    TileId coord;
    struct MapElement *obj; /* render object being counted (0x400000 check) */
    struct Ride *ride; /* its class */
    int objcount; /* render objects that FUN_0044f360 accepts */
    int tiles; /* GetMapTileCount() (0x800000 check) */
    int *nidsp; /* &recs[k].nids of a continuation row, zeroed after its text is fetched */
    int chances; /* appraisals the park may still fail before it is closed */
    int *nidp; /* &recs[i].nids: the speech-id count of the advice row being written */
    int shown; /* the row being drawn */
    int played; /* next queued speech id to play */
    int nids; /* speech ids of the row being drawn */
    RECT r; /* a rectangle passed by value */
    struct {
        RECT layout;
        RECT cur;
    } rc;
    struct AppraisalRow recs[100];
    char wavbuf[0x80];
    char fmtbuf[0x200];
    int queue[200]; /* speech ids queued for the page being shown */

    x = 0;
    i = 0;
    pagestart = 0;
    totacc = 0;
    passacc = 0;
    flags = 0;
    if (IsScriptStopped() != 0) {
        return 0;
    }
    AppraisalPageCount = 0;
    AppraisalPage = 0;
    AppraisalPageChanged = 1;
    PushRenderingStatusAndUnlockVideoSurface();
    ReadGameButtons();
    rc.layout.left = 0x50;
    rc.layout.top = 0x6d;
    rc.layout.right = 0x1a4;
    rc.layout.bottom = 0x83;
    y = rc.layout.top;
    bottom = rc.layout.bottom;
    if (0) {
        /* Never runs. The original takes the address of the two rectangles in code the optimizer removes,
         * which keeps them in memory: layout is reloaded at every page break and cur's stores are kept.
         * Nothing else reproduces that, so this does it the same way. */
        SetClipping(&rc.layout);
    }

    /* section 0 (0x4453a0-0x445539): the report title row (ReportFlags & 0xf) */
top:
    if (ReportFlags & 0xf) {
        recs[i].x = x;
        if (bottom > 0x1b5) {
            APPR_TOP_OF_PAGE();
            if (pagestart != i) APPR_RETRY(top);
            APPR_NEW_PAGE();
        }
        recs[i].page = AppraisalPageCount;
        recs[i].x = x;
        recs[i].type = 0;
        recs[i].rnd = rand() % 5;
        recs[i].text = GetString(0x12c);
        recs[i].arg = 0;
        recs[i].bar = 0;
        recs[i].value = 0;
        recs[i].goal = 0;
        recs[i].max = 0;
        recs[i].nids = 0;
        recs[i].type = 1;
        i++;
        y += 0x18;
        bottom = y + 0x16;
    }
    /* section 1 (0x445539-0x44672e): the 0x4fff0 checks */
s1:
    if (ReportFlags & 0x4fff0) {
        total = 0;
        pass = 0;
        first = i;
        xp = &recs[i].x;
        *xp = x;
        if (bottom > 0x1b5) {
            APPR_TOP_OF_PAGE();
            if (pagestart != i) APPR_RETRY(top);
            APPR_NEW_PAGE();
        }
        APPR_ROW(0, 0x131);
        recs[i].arg = 0;
        recs[i].bar = 0;
        recs[i].value = 0;
        recs[i].goal = 0;
        recs[i].max = 0;
        recs[i].nids = 0;
        i++;
        y += 0x18;
        x += 0x30;
        CountClassesOfType1Or3(&bf0_total, &bf0_count);
        if (ReportFlags & 0x4000) {
            total++;
            ok = bf0_total >= DAT_00666020;
            if (ok) {
                pass++;
            } else {
                flags |= 0x10;
            }
            if (y + 0x16 > 0x1b5) {
                APPR_TOP_OF_PAGE();
                if (pagestart != first) {
                    i = first;
                    x = *xp;
                    AppraisalPageCount++;
                    pagestart = first;
                    goto s1;
                }
                APPR_NEW_PAGE();
            }
            APPR_ROW(ok, 0x132);
            recs[i].arg = 0;
            recs[i].bar = 1;
            recs[i].value = bf0_total;
            recs[i].goal = DAT_00666020;
            recs[i].max = DAT_00666024;
            recs[i].nids = 0;
            i++;
            y += 0x18;
        }
        if (ReportFlags & 0x8000) {
            total++;
            ok = bf0_count >= DAT_00666028;
            if (ok) {
                pass++;
            } else {
                flags |= 0x20;
            }
            if (y + 0x16 > 0x1b5) {
                APPR_TOP_OF_PAGE();
                if (pagestart != first) {
                    i = first;
                    x = *xp;
                    AppraisalPageCount++;
                    pagestart = first;
                    goto s1;
                }
                APPR_NEW_PAGE();
            }
            APPR_ROW(ok, 0x133);
            recs[i].arg = 0;
            recs[i].bar = 1;
            recs[i].value = bf0_count;
            recs[i].goal = DAT_00666028;
            recs[i].max = DAT_0066602c;
            recs[i].nids = 0;
            i++;
            y += 0x18;
        }
        if (ReportFlags & 0x40000) {
            val = GetLinkedObjectPercent();
            total++;
            ok = val >= DAT_00666030;
            if (ok) {
                pass++;
            } else {
                flags |= 0x40;
            }
            if (y + 0x16 > 0x1b5) {
                APPR_TOP_OF_PAGE();
                if (pagestart != first) {
                    i = first;
                    x = *xp;
                    AppraisalPageCount++;
                    pagestart = first;
                    goto s1;
                }
                APPR_NEW_PAGE();
            }
            APPR_ROW(ok, 0x134);
            recs[i].arg = 0;
            recs[i].bar = 1;
            recs[i].value = val;
            recs[i].goal = DAT_00666030;
            recs[i].max = DAT_00666034;
            recs[i].nids = 0;
            i++;
            y += 0x18;
        }
        if (ReportFlags & 0x30) {
            total++;
            ok = GetCastleScore() >= DAT_0066600c;
            if (ok) {
                pass++;
            } else {
                flags |= 0x80;
            }
            switch ((ReportFlags >> 4) & 3) {
            case 1:
                if (y + 0x16 > 0x1b5) {
                    APPR_TOP_OF_PAGE();
                    if (pagestart != first) {
                        i = first;
                        x = *xp;
                        AppraisalPageCount++;
                        pagestart = first;
                        goto s1;
                    }
                    APPR_NEW_PAGE();
                }
                APPR_ROW(ok, 0x135);
                recs[i].arg = 0;
                recs[i].bar = 0;
                recs[i].value = 0;
                recs[i].goal = 0;
                recs[i].max = 0;
                recs[i].nids = 0;
                i++;
                break;
            case 2:
                if (y + 0x16 > 0x1b5) {
                    APPR_TOP_OF_PAGE();
                    if (pagestart != first) {
                        i = first;
                        x = *xp;
                        AppraisalPageCount++;
                        pagestart = first;
                        goto s1;
                    }
                    APPR_NEW_PAGE();
                }
                APPR_ROW(ok, 0x136);
                recs[i].arg = 0;
                recs[i].bar = 0;
                recs[i].value = 0;
                recs[i].goal = 0;
                recs[i].max = 0;
                recs[i].nids = 0;
                i++;
                break;
            case 3:
                if (y + 0x16 > 0x1b5) {
                    APPR_TOP_OF_PAGE();
                    if (pagestart != first) {
                        i = first;
                        x = *xp;
                        AppraisalPageCount++;
                        pagestart = first;
                        goto s1;
                    }
                    APPR_NEW_PAGE();
                }
                APPR_ROW(ok, 0x137);
                recs[i].arg = 0;
                recs[i].bar = 0;
                recs[i].value = 0;
                recs[i].goal = 0;
                recs[i].max = 0;
                recs[i].nids = 0;
                i++;
                break;
            }
            y += 0x18;
        }
        if (ReportFlags & 0xc0) {
            total++;
            ok = GetDrivingSchoolScore() >= DAT_00666010;
            if (ok) {
                pass++;
            } else {
                flags |= 0x100;
            }
            switch ((ReportFlags >> 6) & 3) {
            case 1:
                if (y + 0x16 > 0x1b5) {
                    APPR_TOP_OF_PAGE();
                    if (pagestart != first) {
                        i = first;
                        x = *xp;
                        AppraisalPageCount++;
                        pagestart = first;
                        goto s1;
                    }
                    APPR_NEW_PAGE();
                }
                APPR_ROW(ok, 0x138);
                recs[i].arg = 0;
                recs[i].bar = 0;
                recs[i].value = 0;
                recs[i].goal = 0;
                recs[i].max = 0;
                recs[i].nids = 0;
                i++;
                break;
            case 2:
                if (y + 0x16 > 0x1b5) {
                    APPR_TOP_OF_PAGE();
                    if (pagestart != first) {
                        i = first;
                        x = *xp;
                        AppraisalPageCount++;
                        pagestart = first;
                        goto s1;
                    }
                    APPR_NEW_PAGE();
                }
                APPR_ROW(ok, 0x139);
                recs[i].arg = 0;
                recs[i].bar = 0;
                recs[i].value = 0;
                recs[i].goal = 0;
                recs[i].max = 0;
                recs[i].nids = 0;
                i++;
                break;
            case 3:
                if (y + 0x16 > 0x1b5) {
                    APPR_TOP_OF_PAGE();
                    if (pagestart != first) {
                        i = first;
                        x = *xp;
                        AppraisalPageCount++;
                        pagestart = first;
                        goto s1;
                    }
                    APPR_NEW_PAGE();
                }
                APPR_ROW(ok, 0x13a);
                recs[i].arg = 0;
                recs[i].bar = 0;
                recs[i].value = 0;
                recs[i].goal = 0;
                recs[i].max = 0;
                recs[i].nids = 0;
                i++;
                break;
            }
            y += 0x18;
        }
        if (ReportFlags & 0x300) {
            total++;
            ok = GetLogFlumeScore() >= DAT_00666014;
            if (ok) {
                pass++;
            } else {
                flags |= 0x200;
            }
            switch ((ReportFlags >> 8) & 3) {
            case 1:
                if (y + 0x16 > 0x1b5) {
                    APPR_TOP_OF_PAGE();
                    if (pagestart != first) {
                        i = first;
                        x = *xp;
                        AppraisalPageCount++;
                        pagestart = first;
                        goto s1;
                    }
                    APPR_NEW_PAGE();
                }
                APPR_ROW(ok, 0x13b);
                recs[i].arg = 0;
                recs[i].bar = 0;
                recs[i].value = 0;
                recs[i].goal = 0;
                recs[i].max = 0;
                recs[i].nids = 0;
                i++;
                break;
            case 2:
                if (y + 0x16 > 0x1b5) {
                    APPR_TOP_OF_PAGE();
                    if (pagestart != first) {
                        i = first;
                        x = *xp;
                        AppraisalPageCount++;
                        pagestart = first;
                        goto s1;
                    }
                    APPR_NEW_PAGE();
                }
                APPR_ROW(ok, 0x13c);
                recs[i].arg = 0;
                recs[i].bar = 0;
                recs[i].value = 0;
                recs[i].goal = 0;
                recs[i].max = 0;
                recs[i].nids = 0;
                i++;
                break;
            case 3:
                if (y + 0x16 > 0x1b5) {
                    APPR_TOP_OF_PAGE();
                    if (pagestart != first) {
                        i = first;
                        x = *xp;
                        AppraisalPageCount++;
                        pagestart = first;
                        goto s1;
                    }
                    APPR_NEW_PAGE();
                }
                APPR_ROW(ok, 0x13d);
                recs[i].arg = 0;
                recs[i].bar = 0;
                recs[i].value = 0;
                recs[i].goal = 0;
                recs[i].max = 0;
                recs[i].nids = 0;
                i++;
                break;
            }
            y += 0x18;
        }
        if (ReportFlags & 0xc00) {
            total++;
            ok = GetBoatingSchoolScore() >= DAT_00666018;
            if (ok) {
                pass++;
            } else {
                flags |= 0x400;
            }
            switch ((ReportFlags >> 10) & 3) {
            case 1:
                if (y + 0x16 > 0x1b5) {
                    APPR_TOP_OF_PAGE();
                    if (pagestart != first) {
                        i = first;
                        x = *xp;
                        AppraisalPageCount++;
                        pagestart = first;
                        goto s1;
                    }
                    APPR_NEW_PAGE();
                }
                APPR_ROW(ok, 0x13e);
                recs[i].arg = 0;
                recs[i].bar = 0;
                recs[i].value = 0;
                recs[i].goal = 0;
                recs[i].max = 0;
                recs[i].nids = 0;
                i++;
                break;
            case 2:
                if (y + 0x16 > 0x1b5) {
                    APPR_TOP_OF_PAGE();
                    if (pagestart != first) {
                        i = first;
                        x = *xp;
                        AppraisalPageCount++;
                        pagestart = first;
                        goto s1;
                    }
                    APPR_NEW_PAGE();
                }
                APPR_ROW(ok, 0x13f);
                recs[i].arg = 0;
                recs[i].bar = 0;
                recs[i].value = 0;
                recs[i].goal = 0;
                recs[i].max = 0;
                recs[i].nids = 0;
                i++;
                break;
            case 3:
                if (y + 0x16 > 0x1b5) {
                    APPR_TOP_OF_PAGE();
                    if (pagestart != first) {
                        i = first;
                        x = *xp;
                        AppraisalPageCount++;
                        pagestart = first;
                        goto s1;
                    }
                    APPR_NEW_PAGE();
                }
                APPR_ROW(ok, 0x140);
                recs[i].arg = 0;
                recs[i].bar = 0;
                recs[i].value = 0;
                recs[i].goal = 0;
                recs[i].max = 0;
                recs[i].nids = 0;
                i++;
                break;
            }
            y += 0x18;
        }
        if (ReportFlags & 0x3000) {
            total++;
            ok = GetJungleCruiseScore() >= DAT_0066601c;
            if (ok) {
                pass++;
            } else {
                flags |= 0x800;
            }
            switch ((ReportFlags >> 12) & 3) {
            case 1:
                if (y + 0x16 > 0x1b5) {
                    APPR_TOP_OF_PAGE();
                    if (pagestart != first) {
                        i = first;
                        x = *xp;
                        AppraisalPageCount++;
                        pagestart = first;
                        goto s1;
                    }
                    APPR_NEW_PAGE();
                }
                APPR_ROW(ok, 0x141);
                recs[i].arg = 0;
                recs[i].bar = 0;
                recs[i].value = 0;
                recs[i].goal = 0;
                recs[i].max = 0;
                recs[i].nids = 0;
                i++;
                break;
            case 2:
                if (y + 0x16 > 0x1b5) {
                    APPR_TOP_OF_PAGE();
                    if (pagestart != first) {
                        i = first;
                        x = *xp;
                        AppraisalPageCount++;
                        pagestart = first;
                        goto s1;
                    }
                    APPR_NEW_PAGE();
                }
                APPR_ROW(ok, 0x142);
                recs[i].arg = 0;
                recs[i].bar = 0;
                recs[i].value = 0;
                recs[i].goal = 0;
                recs[i].max = 0;
                recs[i].nids = 0;
                i++;
                break;
            case 3:
                if (y + 0x16 > 0x1b5) {
                    APPR_TOP_OF_PAGE();
                    if (pagestart != first) {
                        /* every page break in this section that cannot start a new page restarts the section here */
                        i = first;
                        x = *xp;
                        AppraisalPageCount++;
                        pagestart = first;
                        goto s1;
                    }
                    APPR_NEW_PAGE();
                }
                APPR_ROW(ok, 0x143);
                recs[i].arg = 0;
                recs[i].bar = 0;
                recs[i].value = 0;
                recs[i].goal = 0;
                recs[i].max = 0;
                recs[i].nids = 0;
                i++;
                break;
            }
            y += 0x18;
        }
        recs[first].type = pass == total;
        x -= 0x30;
        bottom = y + 0x16;
        totacc += total;
        passacc += pass;
    }
    /* section 2 (0x44672e-0x4473e2): the loop sections 0x38000000, 0xc0000000 and 0x30000 */
    if (ReportFlags & 0x38000000) {
        int lpass; /* checks passed in it */

        do {
            total = 0;
            lpass = 0;
            first = i;
            xp = &recs[i].x;
            *xp = x;
            if (bottom > 0x1b5) {
                APPR_TOP_OF_PAGE();
                if (pagestart != i) {
                    /* the header does not fit: retry it at the top of a new page */
                    AppraisalPageCount++;
                    pagestart = i;
                    continue;
                }
                APPR_NEW_PAGE();
            }
            APPR_ROW(ok, 0x144);
            recs[i].arg = 0;
            recs[i].bar = 0;
            recs[i].value = 0;
            recs[i].goal = 0;
            recs[i].max = 0;
            recs[i].nids = 0;
            i++;
            y += 0x18;
            x += 0x30;
            CountType2ClassesExceptRideParts(&c70_total, &c70_count);
            if (ReportFlags & 0x8000000) {
                total++;
                ok = c70_total >= DAT_00666070;
                if (ok) {
                    lpass++;
                } else {
                    flags |= 0x1000;
                }
                if (y + 0x16 > 0x1b5) {
                    APPR_TOP_OF_PAGE();
                    if (pagestart != first) {
                        i = first;
                        x = *xp;
                        AppraisalPageCount++;
                        pagestart = first;
                        continue;
                    }
                    APPR_NEW_PAGE();
                }
                APPR_ROW(ok, 0x132);
                recs[i].arg = 0;
                recs[i].bar = 1;
                recs[i].value = c70_total;
                recs[i].goal = DAT_00666070;
                recs[i].max = DAT_00666074;
                recs[i].nids = 0;
                i++;
                y += 0x18;
            }
            if (ReportFlags & 0x10000000) {
                total++;
                ok = c70_count >= DAT_00666078;
                if (ok) {
                    lpass++;
                } else {
                    flags |= 0x2000;
                }
                if (y + 0x16 > 0x1b5) {
                    APPR_TOP_OF_PAGE();
                    if (pagestart != first) {
                        /* every page break in this section that cannot start a new page restarts the section here */
                        i = first;
                        x = *xp;
                        AppraisalPageCount++;
                        pagestart = first;
                        continue;
                    }
                    APPR_NEW_PAGE();
                }
                APPR_ROW(ok, 0x133);
                recs[i].arg = 0;
                recs[i].bar = 1;
                recs[i].value = c70_count;
                recs[i].goal = DAT_00666078;
                recs[i].max = DAT_0066607c;
                recs[i].nids = 0;
                i++;
                y += 0x18;
            }
            recs[first].type = lpass == total;
            totacc += total;
            passacc += lpass;
            x -= 0x30;
            bottom = y + 0x16;
            break;
        } while (ReportFlags & 0x38000000);
    }
    if (ReportFlags & 0xc0000000) {
        int lpass; /* checks passed in it */

        do {
            total = 0;
            lpass = 0;
            first = i;
            xp = &recs[i].x;
            *xp = x;
            if (bottom > 0x1b5) {
                APPR_TOP_OF_PAGE();
                if (pagestart != i) {
                    /* the header does not fit: retry it at the top of a new page */
                    AppraisalPageCount++;
                    pagestart = i;
                    continue;
                }
                APPR_NEW_PAGE();
            }
            APPR_ROW(ok, 0x145);
            recs[i].arg = 0;
            recs[i].bar = 0;
            recs[i].value = 0;
            recs[i].goal = 0;
            recs[i].max = 0;
            recs[i].nids = 0;
            i++;
            y += 0x18;
            x += 0x30;
            CountClassesOfType4(&cd0_total, &cd0_count);
            if (ReportFlags & 0x40000000) {
                total++;
                ok = cd0_total >= DAT_00666088;
                if (ok) {
                    lpass++;
                } else {
                    flags |= 0x8000;
                }
                if (y + 0x16 > 0x1b5) {
                    APPR_TOP_OF_PAGE();
                    if (pagestart != first) {
                        i = first;
                        x = *xp;
                        AppraisalPageCount++;
                        pagestart = first;
                        continue;
                    }
                    APPR_NEW_PAGE();
                }
                APPR_ROW(ok, 0x132);
                recs[i].arg = 0;
                recs[i].bar = 1;
                recs[i].value = cd0_total;
                recs[i].goal = DAT_00666088;
                recs[i].max = DAT_0066608c;
                recs[i].nids = 0;
                i++;
                y += 0x18;
            }
            if (ReportFlags & 0x80000000) {
                total++;
                ok = cd0_count >= DAT_00666090;
                if (ok) {
                    lpass++;
                } else {
                    flags |= 0x10000;
                }
                if (y + 0x16 > 0x1b5) {
                    APPR_TOP_OF_PAGE();
                    if (pagestart != first) {
                        /* every page break in this section that cannot start a new page restarts the section here */
                        i = first;
                        x = *xp;
                        AppraisalPageCount++;
                        pagestart = first;
                        continue;
                    }
                    APPR_NEW_PAGE();
                }
                APPR_ROW(ok, 0x133);
                recs[i].arg = 0;
                recs[i].bar = 1;
                recs[i].value = cd0_count;
                recs[i].goal = DAT_00666090;
                recs[i].max = DAT_00666094;
                recs[i].nids = 0;
                i++;
                y += 0x18;
            }
            recs[first].type = lpass == total;
            totacc += total;
            passacc += lpass;
            x -= 0x30;
            bottom = y + 0x16;
            break;
        } while (ReportFlags & 0xc0000000);
    }
    if (ReportFlags & 0x30000) {
        int lpass; /* checks passed in it */

        do {
            total = 0;
            lpass = 0;
            first = i;
            xp = &recs[i].x;
            *xp = x;
            if (bottom > 0x1b5) {
                APPR_TOP_OF_PAGE();
                if (pagestart != i) {
                    /* the header does not fit: retry it at the top of a new page */
                    AppraisalPageCount++;
                    pagestart = i;
                    continue;
                }
                APPR_NEW_PAGE();
            }
            APPR_ROW(ok, 0x146);
            recs[i].arg = 0;
            recs[i].bar = 0;
            recs[i].value = 0;
            recs[i].goal = 0;
            recs[i].max = 0;
            recs[i].nids = 0;
            i++;
            y += 0x18;
            x += 0x30;
            CountFoodStallClasses(&d20_total, &d20_count);
            if (ReportFlags & 0x10000) {
                total++;
                ok = d20_total >= DAT_00666040;
                if (ok) {
                    lpass++;
                } else {
                    flags |= 0x20000;
                }
                if (y + 0x16 > 0x1b5) {
                    APPR_TOP_OF_PAGE();
                    if (pagestart != first) {
                        i = first;
                        x = *xp;
                        AppraisalPageCount++;
                        pagestart = first;
                        continue;
                    }
                    APPR_NEW_PAGE();
                }
                APPR_ROW(ok, 0x132);
                recs[i].arg = 0;
                recs[i].bar = 1;
                recs[i].value = d20_total;
                recs[i].goal = DAT_00666040;
                recs[i].max = DAT_00666044;
                recs[i].nids = 0;
                i++;
                y += 0x18;
            }
            if (ReportFlags & 0x20000) {
                total++;
                ok = d20_count >= DAT_00666048;
                if (ok) {
                    lpass++;
                } else {
                    flags |= 0x40000;
                }
                if (y + 0x16 > 0x1b5) {
                    APPR_TOP_OF_PAGE();
                    if (pagestart != first) {
                        /* every page break in this section that cannot start a new page restarts the section here */
                        i = first;
                        x = *xp;
                        AppraisalPageCount++;
                        pagestart = first;
                        continue;
                    }
                    APPR_NEW_PAGE();
                }
                APPR_ROW(ok, 0x133);
                recs[i].arg = 0;
                recs[i].bar = 1;
                recs[i].value = d20_count;
                recs[i].goal = DAT_00666048;
                recs[i].max = DAT_0066604c;
                recs[i].nids = 0;
                i++;
                y += 0x18;
            }
            recs[first].type = lpass == total;
            totacc += total;
            passacc += lpass;
            x -= 0x30;
            bottom = y + 0x16;
            break;
        } while (ReportFlags & 0x30000);
    }
    /* section 3 (0x4473e2-0x447e73): loop sections 0x5080000 (paths) and 0xe00000 (power, objects, map size) */
    if (ReportFlags & 0x5080000) {
        int lpass; /* checks passed in it */

        do {
            total = 0;
            lpass = 0;
            first = i;
            xp = &recs[i].x;
            *xp = x;
            if (bottom > 0x1b5) {
                APPR_TOP_OF_PAGE();
                if (pagestart != i) {
                    /* the header does not fit: retry it on a new page */
                    AppraisalPageCount++;
                    pagestart = i;
                    continue;
                }
                APPR_NEW_PAGE();
            }
            APPR_ROW(ok, 0x147);
            recs[i].arg = 0;
            recs[i].bar = 0;
            recs[i].value = 0;
            recs[i].goal = 0;
            recs[i].max = 0;
            recs[i].nids = 0;
            i++;
            y += 0x18;
            x += 0x30;
            SurveyBlokes(&d70_count, &d70_p2, &d70_p3);
            if (ReportFlags & 0x80000) {
                total++;
                ok = d70_count >= DAT_00666038;
                if (ok) {
                    lpass++;
                } else {
                    flags |= 0x80000;
                }
                y += 0x18;
            }
            if (ReportFlags & 0x1000000) {
                total++;
                ok = d70_p2 >= DAT_00666050;
                if (ok) {
                    lpass++;
                } else {
                    flags |= 0x100000;
                }
                if (y + 0x16 > 0x1b5) {
                    APPR_TOP_OF_PAGE();
                    if (pagestart != first) {
                        i = first;
                        x = *xp;
                        AppraisalPageCount++;
                        pagestart = first;
                        continue;
                    }
                    APPR_NEW_PAGE();
                }
                APPR_ROW(ok, 0x148);
                recs[i].arg = 0;
                recs[i].bar = 1;
                recs[i].value = d70_p2;
                recs[i].goal = DAT_00666050;
                recs[i].max = DAT_00666054;
                recs[i].nids = 0;
                i++;
                y += 0x18;
            }
            if (ReportFlags & 0x4000000) {
                total++;
                ok = d70_p3 >= DAT_00666050;
                if (ok) {
                    lpass++;
                } else {
                    flags |= 0x200000;
                }
                if (y + 0x16 > 0x1b5) {
                    APPR_TOP_OF_PAGE();
                    if (pagestart != first) {
                        /* every page break in this section that cannot start a new page restarts it here */
                        i = first;
                        x = *xp;
                        AppraisalPageCount++;
                        pagestart = first;
                        continue;
                    }
                    APPR_NEW_PAGE();
                }
                APPR_ROW(ok, 0x149);
                recs[i].arg = 0;
                recs[i].bar = 1;
                recs[i].value = d70_p3;
                recs[i].goal = DAT_00666050;
                recs[i].max = DAT_00666054;
                recs[i].nids = 0;
                i++;
                y += 0x18;
            }
            recs[first].type = lpass == total;
            totacc += total;
            passacc += lpass;
            x -= 0x30;
            bottom = y + 0x16;
            break;
        } while (ReportFlags & 0x5080000);
    }
    if (ReportFlags & 0xe00000) {
        int lpass; /* checks passed in it */

        do {
            total = 0;
            lpass = 0;
            first = i;
            xp = &recs[i].x;
            *xp = x;
            if (bottom > 0x1b5) {
                APPR_TOP_OF_PAGE();
                if (pagestart != i) {
                    /* the header does not fit: retry it on a new page */
                    AppraisalPageCount++;
                    pagestart = i;
                    continue;
                }
                APPR_NEW_PAGE();
            }
            APPR_ROW(ok, 0x14a);
            recs[i].arg = 0;
            recs[i].bar = 0;
            recs[i].value = 0;
            recs[i].goal = 0;
            recs[i].max = 0;
            recs[i].nids = 0;
            i++;
            y += 0x18;
            x += 0x30;
            if (ReportFlags & 0x200000) {
                total++;
                ok = MapStats.power_supply >= DAT_00666058;
                if (ok) {
                    lpass++;
                } else {
                    flags |= 0x400000;
                }
                if (y + 0x16 > 0x1b5) {
                    APPR_TOP_OF_PAGE();
                    if (pagestart != first) {
                        i = first;
                        x = *xp;
                        AppraisalPageCount++;
                        pagestart = first;
                        continue;
                    }
                    APPR_NEW_PAGE();
                }
                APPR_ROW(ok, 0x14b);
                recs[i].arg = 0;
                recs[i].bar = 1;
                recs[i].value = MapStats.power_supply;
                recs[i].goal = DAT_00666058;
                recs[i].max = DAT_0066605c;
                recs[i].nids = 0;
                i++;
                y += 0x18;
            }
            if (ReportFlags & 0x400000) {
                objcount = 0;
                for (obj = GetFirstRenderObject(); obj != NULL; obj = GetNextRenderObject(obj)) {
                    ride = obj->field_0->ride;
                    coord = obj->anchor;
                    if (ride->type != 0 && ride->type != 2 && FUN_0044f360(obj->field_0->ride, &coord)) {
                        objcount++;
                    }
                }
                total++;
                ok = objcount >= DAT_00666060;
                if (ok) {
                    lpass++;
                } else {
                    flags |= 0x800000;
                }
                if (y + 0x16 > 0x1b5) {
                    APPR_TOP_OF_PAGE();
                    if (pagestart != first) {
                        i = first;
                        x = *xp;
                        AppraisalPageCount++;
                        pagestart = first;
                        continue;
                    }
                    APPR_NEW_PAGE();
                }
                APPR_ROW(ok, 0x14c);
                recs[i].arg = 0;
                recs[i].bar = 1;
                recs[i].value = objcount;
                recs[i].goal = DAT_00666060;
                recs[i].max = DAT_00666064;
                recs[i].nids = 0;
                i++;
                y += 0x18;
            }
            if (ReportFlags & 0x800000) {
                tiles = GetMapTileCount();
                total++;
                ok = tiles >= DAT_00666068;
                if (ok) {
                    lpass++;
                } else {
                    flags |= 0x1000000;
                }
                if (y + 0x16 > 0x1b5) {
                    APPR_TOP_OF_PAGE();
                    if (pagestart != first) {
                        /* every page break in this section that cannot start a new page restarts it here */
                        i = first;
                        x = *xp;
                        AppraisalPageCount++;
                        pagestart = first;
                        continue;
                    }
                    APPR_NEW_PAGE();
                }
                APPR_ROW(ok, 0x14d);
                recs[i].arg = 0;
                recs[i].bar = 1;
                recs[i].value = tiles;
                recs[i].goal = DAT_00666068;
                recs[i].max = DAT_0066606c;
                recs[i].nids = 0;
                i++;
                y += 0x18;
            }
            recs[first].type = lpass == total;
            totacc += total;
            passacc += lpass;
            x -= 0x30;
            bottom = y + 0x16;
            break;
        } while (ReportFlags & 0xe00000);
    }
    /* section 4 (0x447e73-0x44855b): the summary header (0x230) and the verdict rows 0x14f..0x154 */
    /* the summary part (S4-S7); every page break in it restarts here through summary_restart */
summary:
    first = i;
    xp = &recs[i].x;
    *xp = x;
    if (bottom > 0x1b5) {
        APPR_TOP_OF_PAGE();
        if (pagestart != i) APPR_RETRY(summary);
        APPR_NEW_PAGE();
    }
    APPR_ROW(-2, 0x230);
    recs[i].arg = 0;
    recs[i].bar = 0;
    recs[i].value = 0;
    recs[i].goal = 0;
    recs[i].max = 0;
    recs[i].nids = 0;
    i++;
    y += 0x18;
    x += 0x30;
    bottom = y + 0x16;
    if (passacc < totacc / 2) {
        if (bottom > 0x1b5) {
            APPR_TOP_OF_PAGE();
            if (pagestart != first) {
                i = first;
                x = *xp;
                AppraisalPageCount++;
                pagestart = first;
                goto summary;
            }
            APPR_NEW_PAGE();
        }
        APPR_ROW(-2, 0x14f);
        recs[i].arg = 0;
        recs[i].bar = 0;
        recs[i].value = 0;
        recs[i].goal = 0;
        recs[i].max = 0;
        recs[i].nids = 0;
        i++;
        y += 0x18;
        recs[i - 1].ids[recs[i - 1].nids] = 0x14f;
        recs[i - 1].nids++;
        if (y + 0x16 > 0x1b5) {
            APPR_TOP_OF_PAGE();
            if (pagestart != first) {
                i = first;
                x = *xp;
                AppraisalPageCount++;
                pagestart = first;
                goto summary;
            }
            APPR_NEW_PAGE();
        }
        APPR_ROW(-2, 0x150);
        recs[i].arg = 0;
        recs[i].bar = 0;
        recs[i].value = 0;
        recs[i].goal = 0;
        recs[i].max = 0;
        recs[i].nids = 0;
        i++;
        recs[i - 1].ids[recs[i - 1].nids] = 0x150;
        recs[i - 1].nids++;
        y += 0x18;
        rc.cur.bottom = y + 0x16;
    } else if (passacc < totacc) {
        if (bottom > 0x1b5) {
            APPR_TOP_OF_PAGE();
            if (pagestart != first) {
                i = first;
                x = *xp;
                AppraisalPageCount++;
                pagestart = first;
                goto summary;
            }
            APPR_NEW_PAGE();
        }
        APPR_ROW(-2, 0x151);
        recs[i].arg = 0;
        recs[i].bar = 0;
        recs[i].value = 0;
        recs[i].goal = 0;
        recs[i].max = 0;
        recs[i].nids = 0;
        i++;
        y += 0x18;
        recs[i - 1].ids[recs[i - 1].nids] = 0x151;
        recs[i - 1].nids++;
        if (y + 0x16 > 0x1b5) {
            APPR_TOP_OF_PAGE();
            if (pagestart != first) {
                i = first;
                x = *xp;
                AppraisalPageCount++;
                pagestart = first;
                goto summary;
            }
            APPR_NEW_PAGE();
        }
        APPR_ROW(-2, 0x150);
        recs[i].arg = 0;
        recs[i].bar = 0;
        recs[i].value = 0;
        recs[i].goal = 0;
        recs[i].max = 0;
        y += 0x18;
        rc.cur.bottom = y + 0x16;
        recs[i].nids = 0;
        i++;
    } else {
        if (bottom > 0x1b5) {
            APPR_TOP_OF_PAGE();
            if (pagestart != first) {
                i = first;
                x = *xp;
                AppraisalPageCount++;
                pagestart = first;
                goto summary;
            }
            APPR_NEW_PAGE();
        }
        APPR_ROW(-2, 0x153);
        recs[i].arg = 0;
        recs[i].bar = 0;
        recs[i].value = 0;
        recs[i].goal = 0;
        recs[i].max = 0;
        recs[i].nids = 0;
        i++;
        y += 0x18;
        recs[i - 1].ids[recs[i - 1].nids] = 0x153;
        recs[i - 1].nids++;
        if (y + 0x16 > 0x1b5) {
            APPR_TOP_OF_PAGE();
            if (pagestart != first) {
                i = first;
                x = *xp;
                AppraisalPageCount++;
                pagestart = first;
                goto summary;
            }
            APPR_NEW_PAGE();
        }
        APPR_ROW(-2, 0x154);
        recs[i].arg = 0;
        recs[i].bar = 0;
        recs[i].value = 0;
        recs[i].goal = 0;
        recs[i].max = 0;
        y += 0x18;
        rc.cur.bottom = y + 0x16;
        recs[i].nids = 0;
        i++;
    }
    /* section 5 (0x44855b-0x449504): the failure-flag rows: flags&1,2,4,8, switch(flags&0x30), the &0x40 pair, &0x80..&0x800 */
    if (flags & 1) {
        if (rc.cur.bottom > 0x1b5) {
            APPR_TOP_OF_PAGE();
            if (pagestart != first) {
                i = first;
                x = *xp;
                AppraisalPageCount++;
                pagestart = first;
                goto summary;
            }
            APPR_NEW_PAGE();
        }
        APPR_ROW(-1, 0x155);
        recs[i].arg = 0;
        recs[i].bar = 0;
        recs[i].value = 0;
        recs[i].goal = 0;
        recs[i].max = 0;
        recs[i].nids = 0;
        i++;
        recs[i - 1].ids[recs[i - 1].nids++] = 0x155;
        y += 0x18;
        rc.cur.bottom = y + 0x16;
    }
    if (flags & 2) {
        if (rc.cur.bottom > 0x1b5) {
            APPR_TOP_OF_PAGE();
            if (pagestart != first) {
                i = first;
                x = *xp;
                AppraisalPageCount++;
                pagestart = first;
                goto summary;
            }
            APPR_NEW_PAGE();
        }
        APPR_ROW(-1, 0x156);
        recs[i].arg = 0;
        recs[i].bar = 0;
        recs[i].value = 0;
        recs[i].goal = 0;
        recs[i].max = 0;
        recs[i].nids = 0;
        i++;
        recs[i - 1].ids[recs[i - 1].nids++] = 0x156;
        y += 0x18;
        rc.cur.bottom = y + 0x16;
    }
    if (flags & 4) {
        if (rc.cur.bottom > 0x1b5) {
            APPR_TOP_OF_PAGE();
            if (pagestart != first) {
                i = first;
                x = *xp;
                AppraisalPageCount++;
                pagestart = first;
                goto summary;
            }
            APPR_NEW_PAGE();
        }
        APPR_ROW(-1, 0x157);
        recs[i].arg = 0;
        recs[i].bar = 0;
        recs[i].value = 0;
        recs[i].goal = 0;
        recs[i].max = 0;
        recs[i].nids = 0;
        i++;
        recs[i - 1].ids[recs[i - 1].nids++] = 0x157;
        y += 0x18;
        rc.cur.bottom = y + 0x16;
    }
    if (flags & 8) {
        if (rc.cur.bottom > 0x1b5) {
            APPR_TOP_OF_PAGE();
            if (pagestart != first) {
                i = first;
                x = *xp;
                AppraisalPageCount++;
                pagestart = first;
                goto summary;
            }
            APPR_NEW_PAGE();
        }
        APPR_ROW(-1, 0x158);
        recs[i].arg = 0;
        recs[i].bar = 0;
        recs[i].value = 0;
        recs[i].goal = 0;
        recs[i].max = 0;
        recs[i].nids = 0;
        i++;
        recs[i - 1].ids[recs[i - 1].nids++] = 0x158;
        y += 0x18;
        rc.cur.bottom = y + 0x16;
    }
    switch (flags & 0x30) {
    case 0x10:
        if (rc.cur.bottom > 0x1b5) {
            APPR_TOP_OF_PAGE();
            if (pagestart != first) {
                i = first;
                x = *xp;
                AppraisalPageCount++;
                pagestart = first;
                goto summary;
            }
            APPR_NEW_PAGE();
        }
        APPR_ROW(-1, 0x159);
        recs[i].arg = 0;
        recs[i].bar = 0;
        recs[i].value = 0;
        recs[i].goal = 0;
        recs[i].max = 0;
        recs[i].nids = 0;
        i++;
        recs[i - 1].ids[recs[i - 1].nids++] = 0x159;
        y += 0x18;
        rc.cur.bottom = y + 0x16;
        break;
    case 0x20:
        if (rc.cur.bottom > 0x1b5) {
            APPR_TOP_OF_PAGE();
            if (pagestart != first) {
                i = first;
                x = *xp;
                AppraisalPageCount++;
                pagestart = first;
                goto summary;
            }
            APPR_NEW_PAGE();
        }
        APPR_ROW(-1, 0x15a);
        recs[i].arg = 0;
        recs[i].bar = 0;
        recs[i].value = 0;
        recs[i].goal = 0;
        recs[i].max = 0;
        recs[i].nids = 0;
        i++;
        recs[i - 1].ids[recs[i - 1].nids++] = 0x15a;
        y += 0x18;
        rc.cur.bottom = y + 0x16;
        break;
    case 0x30:
        /* the original does not advance y between these two rows, so the 0x133 row tests the same bottom */
        if (rc.cur.bottom > 0x1b5) {
            APPR_TOP_OF_PAGE();
            if (pagestart != first) {
                i = first;
                x = *xp;
                AppraisalPageCount++;
                pagestart = first;
                goto summary;
            }
            APPR_NEW_PAGE();
        }
        APPR_ROW(-1, 0x15b);
        recs[i].arg = 0;
        recs[i].bar = 0;
        recs[i].value = 0;
        recs[i].goal = 0;
        recs[i].max = 0;
        recs[i].nids = 0;
        i++;
        if (rc.cur.bottom > 0x1b5) {
            APPR_TOP_OF_PAGE();
            if (pagestart != first) {
                i = first;
                x = *xp;
                AppraisalPageCount++;
                pagestart = first;
                goto summary;
            }
            APPR_NEW_PAGE();
        }
        APPR_ROW(-3, 0x133);
        recs[i].arg = 0;
        recs[i].bar = 0;
        recs[i].value = 0;
        recs[i].goal = 0;
        recs[i].max = 0;
        recs[i].nids = 0;
        i++;
        recs[i - 1].ids[recs[i - 1].nids++] = 0x15b;
        y += 0x18;
        rc.cur.bottom = y + 0x16;
        break;
    }
    if (flags & 0x40) {
        if (rc.cur.bottom > 0x1b5) {
            APPR_TOP_OF_PAGE();
            if (pagestart != first) {
                i = first;
                x = *xp;
                AppraisalPageCount++;
                pagestart = first;
                goto summary;
            }
            APPR_NEW_PAGE();
        }
        APPR_ROW(-1, 0x15c);
        recs[i].arg = 0;
        recs[i].bar = 0;
        recs[i].value = 0;
        recs[i].goal = 0;
        recs[i].max = 0;
        recs[i].nids = 0;
        i++;
        nidsp = &recs[i].nids;
        recs[i - 1].ids[recs[i - 1].nids++] = 0x15c;
        y += 0x18;
        if (y + 0x16 > 0x1b5) {
            APPR_TOP_OF_PAGE();
            if (pagestart != first) {
                i = first;
                x = *xp;
                AppraisalPageCount++;
                pagestart = first;
                goto summary;
            }
            APPR_NEW_PAGE();
        }
        APPR_ROW(-3, 0x15d);
        recs[i].arg = 0;
        recs[i].bar = 0;
        recs[i].value = 0;
        recs[i].goal = 0;
        recs[i].max = 0;
        i++;
        y += 0x18;
        rc.cur.bottom = y + 0x16;
        *nidsp = 0;
    }
    if (flags & 0x80) {
        if (rc.cur.bottom > 0x1b5) {
            APPR_TOP_OF_PAGE();
            if (pagestart != first) {
                i = first;
                x = *xp;
                AppraisalPageCount++;
                pagestart = first;
                goto summary;
            }
            APPR_NEW_PAGE();
        }
        APPR_ROW(-1, 0x15e);
        recs[i].arg = 0;
        recs[i].bar = 0;
        recs[i].value = 0;
        recs[i].goal = 0;
        recs[i].max = 0;
        recs[i].nids = 0;
        i++;
        recs[i - 1].ids[recs[i - 1].nids++] = 0x15e;
        y += 0x18;
        rc.cur.bottom = y + 0x16;
    }
    if (flags & 0x100) {
        if (rc.cur.bottom > 0x1b5) {
            APPR_TOP_OF_PAGE();
            if (pagestart != first) {
                i = first;
                x = *xp;
                AppraisalPageCount++;
                pagestart = first;
                goto summary;
            }
            APPR_NEW_PAGE();
        }
        APPR_ROW(-1, 0x15f);
        recs[i].arg = 0;
        recs[i].bar = 0;
        recs[i].value = 0;
        recs[i].goal = 0;
        recs[i].max = 0;
        recs[i].nids = 0;
        i++;
        recs[i - 1].ids[recs[i - 1].nids++] = 0x15f;
        y += 0x18;
        rc.cur.bottom = y + 0x16;
    }
    if (flags & 0x200) {
        if (rc.cur.bottom > 0x1b5) {
            APPR_TOP_OF_PAGE();
            if (pagestart != first) {
                i = first;
                x = *xp;
                AppraisalPageCount++;
                pagestart = first;
                goto summary;
            }
            APPR_NEW_PAGE();
        }
        APPR_ROW(-1, 0x160);
        recs[i].arg = 0;
        recs[i].bar = 0;
        recs[i].value = 0;
        recs[i].goal = 0;
        recs[i].max = 0;
        recs[i].nids = 0;
        i++;
        recs[i - 1].ids[recs[i - 1].nids++] = 0x160;
        y += 0x18;
        rc.cur.bottom = y + 0x16;
    }
    if (flags & 0x400) {
        if (rc.cur.bottom > 0x1b5) {
            APPR_TOP_OF_PAGE();
            if (pagestart != first) {
                i = first;
                x = *xp;
                AppraisalPageCount++;
                pagestart = first;
                goto summary;
            }
            APPR_NEW_PAGE();
        }
        APPR_ROW(-1, 0x161);
        recs[i].arg = 0;
        recs[i].bar = 0;
        recs[i].value = 0;
        recs[i].goal = 0;
        recs[i].max = 0;
        recs[i].nids = 0;
        i++;
        recs[i - 1].ids[recs[i - 1].nids++] = 0x161;
        y += 0x18;
        rc.cur.bottom = y + 0x16;
    }
    if (flags & 0x800) {
        if (rc.cur.bottom > 0x1b5) {
            APPR_TOP_OF_PAGE();
            if (pagestart != first) {
                i = first;
                x = *xp;
                AppraisalPageCount++;
                pagestart = first;
                goto summary;
            }
            APPR_NEW_PAGE();
        }
        APPR_ROW(-1, 0x162);
        recs[i].arg = 0;
        recs[i].bar = 0;
        recs[i].value = 0;
        recs[i].goal = 0;
        recs[i].max = 0;
        recs[i].nids = 0;
        i++;
        recs[i - 1].ids[recs[i - 1].nids++] = 0x162;
        y += 0x18;
        rc.cur.bottom = y + 0x16;
    }
    /* section 6 (0x449504-0x44a3d3): failure-flag rows for switch (flags & 0x3000), (flags & 0x18000), (flags & 0x60000) and flags & 0x80000..0x400000 */
    switch (flags & 0x3000) {
    case 0x1000:
        if (rc.cur.bottom > 0x1b5) {
            APPR_TOP_OF_PAGE();
            if (pagestart != first) {
                i = first;
                x = *xp;
                AppraisalPageCount++;
                pagestart = first;
                goto summary;
            }
            APPR_NEW_PAGE();
        }
        APPR_ROW(-1, 0x163);
        recs[i].arg = 0;
        recs[i].bar = 0;
        recs[i].value = 0;
        recs[i].goal = 0;
        recs[i].max = 0;
        recs[i].nids = 0;
        i++;
        recs[i - 1].ids[recs[i - 1].nids++] = 0x163;
        y += 0x18;
        rc.cur.bottom = y + 0x16;
        break;
    case 0x2000:
        if (rc.cur.bottom > 0x1b5) {
            APPR_TOP_OF_PAGE();
            if (pagestart != first) {
                i = first;
                x = *xp;
                AppraisalPageCount++;
                pagestart = first;
                goto summary;
            }
            APPR_NEW_PAGE();
        }
        APPR_ROW(-1, 0x164);
        recs[i].arg = 0;
        recs[i].bar = 0;
        recs[i].value = 0;
        recs[i].goal = 0;
        recs[i].max = 0;
        recs[i].nids = 0;
        i++;
        recs[i - 1].ids[recs[i - 1].nids++] = 0x164;
        y += 0x18;
        rc.cur.bottom = y + 0x16;
        break;
    case 0x3000:
        if (rc.cur.bottom > 0x1b5) {
            APPR_TOP_OF_PAGE();
            if (pagestart != first) {
                i = first;
                x = *xp;
                AppraisalPageCount++;
                pagestart = first;
                goto summary;
            }
            APPR_NEW_PAGE();
        }
        APPR_ROW(-1, 0x165);
        recs[i].arg = 0;
        recs[i].bar = 0;
        recs[i].value = 0;
        recs[i].goal = 0;
        recs[i].max = 0;
        recs[i].nids = 0;
        i++;
        recs[i - 1].ids[recs[i - 1].nids++] = 0x165;
        y += 0x18;
        rc.cur.bottom = y + 0x16;
        break;
    }
    switch (flags & 0x18000) {
    case 0x8000:
        if (rc.cur.bottom > 0x1b5) {
            APPR_TOP_OF_PAGE();
            if (pagestart != first) {
                i = first;
                x = *xp;
                AppraisalPageCount++;
                pagestart = first;
                goto summary;
            }
            APPR_NEW_PAGE();
        }
        APPR_ROW(-1, 0x166);
        recs[i].arg = 0;
        recs[i].bar = 0;
        recs[i].value = 0;
        recs[i].goal = 0;
        recs[i].max = 0;
        recs[i].nids = 0;
        i++;
        recs[i - 1].ids[recs[i - 1].nids++] = 0x166;
        y += 0x18;
        rc.cur.bottom = y + 0x16;
        break;
    case 0x10000:
        if (rc.cur.bottom > 0x1b5) {
            APPR_TOP_OF_PAGE();
            if (pagestart != first) {
                i = first;
                x = *xp;
                AppraisalPageCount++;
                pagestart = first;
                goto summary;
            }
            APPR_NEW_PAGE();
        }
        APPR_ROW(-1, 0x167);
        recs[i].arg = 0;
        recs[i].bar = 0;
        recs[i].value = 0;
        recs[i].goal = 0;
        recs[i].max = 0;
        recs[i].nids = 0;
        i++;
        recs[i - 1].ids[recs[i - 1].nids++] = 0x167;
        y += 0x18;
        rc.cur.bottom = y + 0x16;
        break;
    case 0x18000:
        if (rc.cur.bottom > 0x1b5) {
            APPR_TOP_OF_PAGE();
            if (pagestart != first) {
                i = first;
                x = *xp;
                AppraisalPageCount++;
                pagestart = first;
                goto summary;
            }
            APPR_NEW_PAGE();
        }
        APPR_ROW(-1, 0x168);
        recs[i].arg = 0;
        recs[i].bar = 0;
        recs[i].value = 0;
        recs[i].goal = 0;
        recs[i].max = 0;
        recs[i].nids = 0;
        i++;
        recs[i - 1].ids[recs[i - 1].nids++] = 0x168;
        y += 0x18;
        rc.cur.bottom = y + 0x16;
        break;
    }
    switch (flags & 0x60000) {
    case 0x20000:
        if (rc.cur.bottom > 0x1b5) {
            APPR_TOP_OF_PAGE();
            if (pagestart != first) {
                i = first;
                x = *xp;
                AppraisalPageCount++;
                pagestart = first;
                goto summary;
            }
            APPR_NEW_PAGE();
        }
        APPR_ROW(-1, 0x169);
        recs[i].arg = 0;
        recs[i].bar = 0;
        recs[i].value = 0;
        recs[i].goal = 0;
        recs[i].max = 0;
        recs[i].nids = 0;
        i++;
        recs[i - 1].ids[recs[i - 1].nids++] = 0x169;
        y += 0x18;
        rc.cur.bottom = y + 0x16;
        break;
    case 0x40000:
        if (rc.cur.bottom > 0x1b5) {
            APPR_TOP_OF_PAGE();
            if (pagestart != first) {
                i = first;
                x = *xp;
                AppraisalPageCount++;
                pagestart = first;
                goto summary;
            }
            APPR_NEW_PAGE();
        }
        APPR_ROW(-1, 0x16a);
        recs[i].arg = 0;
        recs[i].bar = 0;
        recs[i].value = 0;
        recs[i].goal = 0;
        recs[i].max = 0;
        recs[i].nids = 0;
        i++;
        recs[i - 1].ids[recs[i - 1].nids++] = 0x16a;
        y += 0x18;
        rc.cur.bottom = y + 0x16;
        break;
    case 0x60000:
        if (rc.cur.bottom > 0x1b5) {
            APPR_TOP_OF_PAGE();
            if (pagestart != first) {
                i = first;
                x = *xp;
                AppraisalPageCount++;
                pagestart = first;
                goto summary;
            }
            APPR_NEW_PAGE();
        }
        APPR_ROW(-1, 0x16b);
        recs[i].arg = 0;
        recs[i].bar = 0;
        recs[i].value = 0;
        recs[i].goal = 0;
        recs[i].max = 0;
        recs[i].nids = 0;
        i++;
        y += 0x18;
        nidsp = &recs[i].nids;
        recs[i - 1].ids[recs[i - 1].nids++] = 0x16b;
        if (y + 0x16 > 0x1b5) {
            APPR_TOP_OF_PAGE();
            if (pagestart != first) {
                i = first;
                x = *xp;
                AppraisalPageCount++;
                pagestart = first;
                goto summary;
            }
            APPR_NEW_PAGE();
        }
        APPR_ROW(-3, 0x231);
        recs[i].arg = 0;
        recs[i].bar = 0;
        recs[i].value = 0;
        recs[i].goal = 0;
        recs[i].max = 0;
        i++;
        y += 0x18;
        rc.cur.bottom = y + 0x16;
        *nidsp = 0;
        break;
    }
    if (flags & 0x80000) {
        if (rc.cur.bottom > 0x1b5) {
            APPR_TOP_OF_PAGE();
            if (pagestart != first) {
                i = first;
                x = *xp;
                AppraisalPageCount++;
                pagestart = first;
                goto summary;
            }
            APPR_NEW_PAGE();
        }
        APPR_ROW(-1, 0x16c);
        recs[i].arg = 0;
        recs[i].bar = 0;
        recs[i].value = 0;
        recs[i].goal = 0;
        recs[i].max = 0;
        recs[i].nids = 0;
        i++;
        recs[i - 1].ids[recs[i - 1].nids++] = 0x16c;
        y += 0x18;
        rc.cur.bottom = y + 0x16;
    }
    if (flags & 0x100000) {
        if (rc.cur.bottom > 0x1b5) {
            APPR_TOP_OF_PAGE();
            if (pagestart != first) {
                i = first;
                x = *xp;
                AppraisalPageCount++;
                pagestart = first;
                goto summary;
            }
            APPR_NEW_PAGE();
        }
        APPR_ROW(-1, 0x16d);
        recs[i].arg = 0;
        recs[i].bar = 0;
        recs[i].value = 0;
        recs[i].goal = 0;
        recs[i].max = 0;
        recs[i].nids = 0;
        i++;
        recs[i - 1].ids[recs[i - 1].nids++] = 0x16d;
        y += 0x18;
        rc.cur.bottom = y + 0x16;
    }
    if (flags & 0x200000) {
        if (rc.cur.bottom > 0x1b5) {
            APPR_TOP_OF_PAGE();
            if (pagestart != first) {
                i = first;
                x = *xp;
                AppraisalPageCount++;
                pagestart = first;
                goto summary;
            }
            APPR_NEW_PAGE();
        }
        APPR_ROW(-1, 0x16f);
        recs[i].arg = 0;
        recs[i].bar = 0;
        recs[i].value = 0;
        recs[i].goal = 0;
        recs[i].max = 0;
        recs[i].nids = 0;
        i++;
        recs[i - 1].ids[recs[i - 1].nids++] = 0x16f;
        y += 0x18;
        rc.cur.bottom = y + 0x16;
    }
    if (flags & 0x400000) {
        if (rc.cur.bottom > 0x1b5) {
            APPR_TOP_OF_PAGE();
            if (pagestart != first) {
                i = first;
                x = *xp;
                AppraisalPageCount++;
                pagestart = first;
                goto summary;
            }
            APPR_NEW_PAGE();
        }
        APPR_ROW(-1, 0x170);
        recs[i].arg = 0;
        recs[i].bar = 0;
        recs[i].value = 0;
        recs[i].goal = 0;
        recs[i].max = 0;
        recs[i].nids = 0;
        i++;
        recs[i - 1].ids[recs[i - 1].nids++] = 0x170;
        y += 0x18;
        rc.cur.bottom = y + 0x16;
    }
    /* section 7 (0x44a3d3-0x44acbb): the 0x800000 pair, the 0x1000000 row (+ summary_restart), the fail-limit rows (+ limit_restart) */
    if (flags & 0x800000) {
        if (rc.cur.bottom > 0x1b5) {
            APPR_TOP_OF_PAGE();
            if (pagestart != first) {
                i = first;
                x = *xp;
                AppraisalPageCount++;
                pagestart = first;
                goto summary;
            }
            APPR_NEW_PAGE();
        }
        APPR_ROW(-1, 0x171);
        recs[i].arg = 0;
        recs[i].bar = 0;
        recs[i].value = 0;
        recs[i].goal = 0;
        recs[i].max = 0;
        recs[i].nids = 0;
        i++;
        y += 0x18;
        recs[i - 1].ids[recs[i - 1].nids++] = 0x171;
        if (y + 0x16 > 0x1b5) {
            APPR_TOP_OF_PAGE();
            if (pagestart != first) {
                i = first;
                x = *xp;
                AppraisalPageCount++;
                pagestart = first;
                goto summary;
            }
            APPR_NEW_PAGE();
        }
        APPR_ROW(-3, 0x172);
        recs[i].arg = 0;
        recs[i].bar = 0;
        recs[i].value = 0;
        recs[i].goal = 0;
        recs[i].max = 0;
        recs[i].nids = 0;
        i++;
        y += 0x18;
        rc.cur.bottom = y + 0x16;
    }
    if (flags & 0x1000000) {
        if (rc.cur.bottom > 0x1b5) {
            APPR_TOP_OF_PAGE();
            if (pagestart != first) {
                /* every page break of the summary and the failure rows restarts the summary here */
                i = first;
                x = *xp;
                AppraisalPageCount++;
                pagestart = first;
                goto summary;
            }
            APPR_NEW_PAGE();
        }
        APPR_ROW(-1, 0x173);
        recs[i].arg = 0;
        recs[i].bar = 0;
        recs[i].value = 0;
        recs[i].goal = 0;
        recs[i].max = 0;
        recs[i].nids = 0;
        i++;
        recs[i - 1].ids[recs[i - 1].nids++] = 0x173;
        y += 0x18;
        rc.cur.bottom = y + 0x16;
    }
    if (flags != 0 && MapStats.appraisal_fail_limit != 0) {
        if (MapStats.appraisal_streak < 0) {
            chances = MapStats.appraisal_streak + MapStats.appraisal_fail_limit - 1;
        } else {
            chances = MapStats.appraisal_fail_limit - 1;
        }
        if (chances > 1) {
            sprintf(fmtbuf, GetString(0x235), GetString(chances + 0x514));
            if (rc.cur.bottom > 0x1b5) {
                APPR_TOP_OF_PAGE();
                if (pagestart != first) {
                    i = first;
                    x = recs[first].x;
                    AppraisalPageCount++;
                    pagestart = first;
                    goto advice;
                }
                APPR_NEW_PAGE();
            }
            recs[i].page = AppraisalPageCount;
            recs[i].x = x;
            recs[i].type = -2;
            recs[i].rnd = rand() % 5;
            recs[i].text = fmtbuf;
            recs[i].arg = 0;
            recs[i].bar = 0;
            recs[i].value = 0;
            recs[i].goal = 0;
            recs[i].max = 0;
            recs[i].nids = 0;
            i++;
            recs[i - 1].ids[recs[i - 1].nids++] = 0x235;
            recs[i - 1].ids[recs[i - 1].nids++] = chances + 0x514;
            recs[i - 1].ids[recs[i - 1].nids++] = 0x236;
            if (rc.cur.bottom > 0x1b5) {
                APPR_TOP_OF_PAGE();
                if (pagestart != first) {
                    i = first;
                    x = recs[first].x;
                    AppraisalPageCount++;
                    pagestart = first;
                    goto advice;
                }
                APPR_NEW_PAGE();
            }
            APPR_ROW(-2, 0x236);
            recs[i].arg = 0;
            recs[i].bar = 0;
            recs[i].value = 0;
            recs[i].goal = 0;
            recs[i].max = 0;
            recs[i].nids = 0;
            i++;
        } else if (chances > 0) {
            if (rc.cur.bottom > 0x1b5) {
                APPR_TOP_OF_PAGE();
                if (pagestart != first) {
                    i = first;
                    x = recs[first].x;
                    AppraisalPageCount++;
                    pagestart = first;
                    goto advice;
                }
                APPR_NEW_PAGE();
            }
            APPR_ROW(-2, 0x514);
            recs[i].arg = 0;
            recs[i].bar = 0;
            recs[i].value = 0;
            recs[i].goal = 0;
            recs[i].max = 0;
            recs[i].nids = 0;
            i++;
            recs[i - 1].ids[recs[i - 1].nids++] = 0x514;
            if (rc.cur.bottom > 0x1b5) {
                APPR_TOP_OF_PAGE();
                if (pagestart != first) {
                    i = first;
                    x = recs[first].x;
                    AppraisalPageCount++;
                    pagestart = first;
                    goto advice;
                }
                APPR_NEW_PAGE();
            }
            APPR_ROW(-2, 0x236);
            recs[i].arg = 0;
            recs[i].bar = 0;
            recs[i].value = 0;
            recs[i].goal = 0;
            recs[i].max = 0;
            recs[i].nids = 0;
            i++;
        } else {
            if (rc.cur.bottom > 0x1b5) {
                APPR_TOP_OF_PAGE();
                if (pagestart != first) {
                    i = first;
                    x = recs[first].x;
                    AppraisalPageCount++;
                    pagestart = first;
                    goto advice;
                }
                APPR_NEW_PAGE();
            }
            APPR_ROW(-2, 0x237);
            recs[i].arg = 0;
            recs[i].bar = 0;
            recs[i].value = 0;
            recs[i].goal = 0;
            recs[i].max = 0;
            recs[i].nids = 0;
            i++;
            recs[i - 1].ids[recs[i - 1].nids++] = 0x237;
            if (rc.cur.bottom > 0x1b5) {
                APPR_TOP_OF_PAGE();
                if (pagestart != first) {
                    /* every page break of the fail-limit rows restarts the advice here */
                    i = first;
                    x = recs[first].x;
                    AppraisalPageCount++;
                    pagestart = first;
                    goto advice;
                }
                APPR_NEW_PAGE();
            }
            APPR_ROW(-2, 0x238);
            recs[i].arg = 0;
            recs[i].bar = 0;
            recs[i].value = 0;
            recs[i].goal = 0;
            recs[i].max = 0;
            recs[i].nids = 0;
            i++;
        }
    }
    x -= 0x30;
    bottom = y + 0x16;
    /* section 8 (0x44acbb-0x44bed0): advice head (0x174) and the advice for flags & 0xf and flags & 0x70 */
    /* the advice part: rewritten from here when it does not fit on its page (advice_restart) */
advice:
    first = i;
    xp = &recs[i].x;
    *xp = x;
    if (bottom > 0x1b5) {
        APPR_TOP_OF_PAGE();
        if (pagestart != i) APPR_RETRY(advice);
        APPR_NEW_PAGE();
    }
    APPR_ROW(-2, 0x174);
    recs[i].arg = 0;
    recs[i].bar = 0;
    recs[i].value = 0;
    recs[i].goal = 0;
    recs[i].max = 0;
    recs[i].nids = 0;
    i++;
    nidp = &recs[i].nids;
    recs[i - 1].ids[recs[i - 1].nids] = 0x174;
    recs[i - 1].nids++;
    y += 0x18;
    total = 0; /* from here on, total counts the advice rows (the checks' total is no longer needed) */
    x += 0x30;
    /* one of four pieces of advice for the failed checks 0x10..0x80 (flags & 0xf) */
    if (flags & 0xf) {
        switch (rand() & 3) {
        case 0:
            if (y + 0x16 > 0x1b5) {
                APPR_TOP_OF_PAGE();
                if (pagestart != first) {
                    i = first;
                    x = *xp;
                    AppraisalPageCount++;
                    pagestart = first;
                    goto advice;
                }
                APPR_NEW_PAGE();
            }
            APPR_ROW(-1, 0x17c);
            recs[i].arg = 0;
            recs[i].bar = 0;
            recs[i].value = 0;
            recs[i].goal = 0;
            recs[i].max = 0;
            *nidp = 0;
            i++;
            nidp = &recs[i].nids;
            recs[i - 1].ids[recs[i - 1].nids] = 0x17c;
            recs[i - 1].nids++;
            y += 0x18;
            if (y + 0x16 > 0x1b5) {
                APPR_TOP_OF_PAGE();
                if (pagestart != first) {
                    i = first;
                    x = *xp;
                    AppraisalPageCount++;
                    pagestart = first;
                    goto advice;
                }
                APPR_NEW_PAGE();
            }
            APPR_ROW(-3, 0x17d);
            recs[i].arg = 0;
            recs[i].bar = 0;
            recs[i].value = 0;
            recs[i].goal = 0;
            recs[i].max = 0;
            *nidp = 0;
            i++;
            nidp = &recs[i].nids;
            recs[i - 1].ids[recs[i - 1].nids] = 0x17d;
            recs[i - 1].nids++;
            y += 0x18;
            total++;
            break;
        case 1:
            if (y + 0x16 > 0x1b5) {
                APPR_TOP_OF_PAGE();
                if (pagestart != first) {
                    i = first;
                    x = *xp;
                    AppraisalPageCount++;
                    pagestart = first;
                    goto advice;
                }
                APPR_NEW_PAGE();
            }
            APPR_ROW(-1, 0x187);
            recs[i].arg = 0;
            recs[i].bar = 0;
            recs[i].value = 0;
            recs[i].goal = 0;
            recs[i].max = 0;
            *nidp = 0;
            i++;
            nidp = &recs[i].nids;
            recs[i - 1].ids[recs[i - 1].nids] = 0x187;
            recs[i - 1].nids++;
            y += 0x18;
            if (y + 0x16 > 0x1b5) {
                APPR_TOP_OF_PAGE();
                if (pagestart != first) {
                    i = first;
                    x = *xp;
                    AppraisalPageCount++;
                    pagestart = first;
                    goto advice;
                }
                APPR_NEW_PAGE();
            }
            APPR_ROW(-3, 0x188);
            recs[i].arg = 0;
            recs[i].bar = 0;
            recs[i].value = 0;
            recs[i].goal = 0;
            recs[i].max = 0;
            *nidp = 0;
            i++;
            nidp = &recs[i].nids;
            recs[i - 1].ids[recs[i - 1].nids] = 0x188;
            recs[i - 1].nids++;
            y += 0x18;
            total++;
            break;
        case 2:
            if (y + 0x16 > 0x1b5) {
                APPR_TOP_OF_PAGE();
                if (pagestart != first) {
                    i = first;
                    x = *xp;
                    AppraisalPageCount++;
                    pagestart = first;
                    goto advice;
                }
                APPR_NEW_PAGE();
            }
            APPR_ROW(-1, 0x190);
            recs[i].arg = 0;
            recs[i].bar = 0;
            recs[i].value = 0;
            recs[i].goal = 0;
            recs[i].max = 0;
            *nidp = 0;
            i++;
            nidp = &recs[i].nids;
            recs[i - 1].ids[recs[i - 1].nids] = 0x190;
            recs[i - 1].nids++;
            y += 0x18;
            if (y + 0x16 > 0x1b5) {
                APPR_TOP_OF_PAGE();
                if (pagestart != first) {
                    i = first;
                    x = *xp;
                    AppraisalPageCount++;
                    pagestart = first;
                    goto advice;
                }
                APPR_NEW_PAGE();
            }
            APPR_ROW(-3, 0x191);
            recs[i].arg = 0;
            recs[i].bar = 0;
            recs[i].value = 0;
            recs[i].goal = 0;
            recs[i].max = 0;
            *nidp = 0;
            i++;
            nidp = &recs[i].nids;
            recs[i - 1].ids[recs[i - 1].nids] = 0x191;
            recs[i - 1].nids++;
            y += 0x18;
            if (y + 0x16 > 0x1b5) {
                APPR_TOP_OF_PAGE();
                if (pagestart != first) {
                    i = first;
                    x = *xp;
                    AppraisalPageCount++;
                    pagestart = first;
                    goto advice;
                }
                APPR_NEW_PAGE();
            }
            APPR_ROW(-3, 0x192);
            recs[i].arg = 0;
            recs[i].bar = 0;
            recs[i].value = 0;
            recs[i].goal = 0;
            recs[i].max = 0;
            *nidp = 0;
            i++;
            nidp = &recs[i].nids;
            recs[i - 1].ids[recs[i - 1].nids] = 0x192;
            recs[i - 1].nids++;
            y += 0x18;
            total++;
            break;
        case 3:
            if (y + 0x16 > 0x1b5) {
                APPR_TOP_OF_PAGE();
                if (pagestart != first) {
                    i = first;
                    x = *xp;
                    AppraisalPageCount++;
                    pagestart = first;
                    goto advice;
                }
                APPR_NEW_PAGE();
            }
            APPR_ROW(-1, 0x19a);
            recs[i].arg = 0;
            recs[i].bar = 0;
            recs[i].value = 0;
            recs[i].goal = 0;
            recs[i].max = 0;
            *nidp = 0;
            i++;
            nidp = &recs[i].nids;
            recs[i - 1].ids[recs[i - 1].nids] = 0x19a;
            recs[i - 1].nids++;
            y += 0x18;
            if (y + 0x16 > 0x1b5) {
                APPR_TOP_OF_PAGE();
                if (pagestart != first) {
                    i = first;
                    x = *xp;
                    AppraisalPageCount++;
                    pagestart = first;
                    goto advice;
                }
                APPR_NEW_PAGE();
            }
            APPR_ROW(-3, 0x19b);
            recs[i].arg = 0;
            recs[i].bar = 0;
            recs[i].value = 0;
            recs[i].goal = 0;
            recs[i].max = 0;
            *nidp = 0;
            i++;
            nidp = &recs[i].nids;
            recs[i - 1].ids[recs[i - 1].nids] = 0x19b;
            recs[i - 1].nids++;
            y += 0x18;
            total++;
            break;
        }
    }
    /* flags & 0x70: one of three, or none */
    if (flags & 0x70) {
        switch (rand() & 3) {
        case 0:
            if (y + 0x16 > 0x1b5) {
                APPR_TOP_OF_PAGE();
                if (pagestart != first) {
                    i = first;
                    x = *xp;
                    AppraisalPageCount++;
                    pagestart = first;
                    goto advice;
                }
                APPR_NEW_PAGE();
            }
            APPR_ROW(-1, 0x1a4);
            recs[i].arg = 0;
            recs[i].bar = 0;
            recs[i].value = 0;
            recs[i].goal = 0;
            recs[i].max = 0;
            *nidp = 0;
            i++;
            nidp = &recs[i].nids;
            recs[i - 1].ids[recs[i - 1].nids] = 0x1a4;
            recs[i - 1].nids++;
            y += 0x18;
            if (y + 0x16 > 0x1b5) {
                APPR_TOP_OF_PAGE();
                if (pagestart != first) {
                    i = first;
                    x = *xp;
                    AppraisalPageCount++;
                    pagestart = first;
                    goto advice;
                }
                APPR_NEW_PAGE();
            }
            APPR_ROW(-3, 0x1a5);
            recs[i].arg = 0;
            recs[i].bar = 0;
            recs[i].value = 0;
            recs[i].goal = 0;
            recs[i].max = 0;
            *nidp = 0;
            i++;
            nidp = &recs[i].nids;
            recs[i - 1].ids[recs[i - 1].nids] = 0x1a5;
            recs[i - 1].nids++;
            y += 0x18;
            if (y + 0x16 > 0x1b5) {
                APPR_TOP_OF_PAGE();
                if (pagestart != first) {
                    i = first;
                    x = *xp;
                    AppraisalPageCount++;
                    pagestart = first;
                    goto advice;
                }
                APPR_NEW_PAGE();
            }
            APPR_ROW(-3, 0x1a6);
            recs[i].arg = 0;
            recs[i].bar = 0;
            recs[i].value = 0;
            recs[i].goal = 0;
            recs[i].max = 0;
            *nidp = 0;
            i++;
            nidp = &recs[i].nids;
            recs[i - 1].ids[recs[i - 1].nids] = 0x1a6;
            recs[i - 1].nids++;
            y += 0x18;
            total++;
            break;
        case 1:
            if (y + 0x16 > 0x1b5) {
                APPR_TOP_OF_PAGE();
                if (pagestart != first) {
                    i = first;
                    x = *xp;
                    AppraisalPageCount++;
                    pagestart = first;
                    goto advice;
                }
                APPR_NEW_PAGE();
            }
            APPR_ROW(-1, 0x1ae);
            recs[i].arg = 0;
            recs[i].bar = 0;
            recs[i].value = 0;
            recs[i].goal = 0;
            recs[i].max = 0;
            *nidp = 0;
            i++;
            nidp = &recs[i].nids;
            recs[i - 1].ids[recs[i - 1].nids] = 0x1ae;
            recs[i - 1].nids++;
            y += 0x18;
            if (y + 0x16 > 0x1b5) {
                APPR_TOP_OF_PAGE();
                if (pagestart != first) {
                    i = first;
                    x = *xp;
                    AppraisalPageCount++;
                    pagestart = first;
                    goto advice;
                }
                APPR_NEW_PAGE();
            }
            APPR_ROW(-3, 0x1af);
            recs[i].arg = 0;
            recs[i].bar = 0;
            recs[i].value = 0;
            recs[i].goal = 0;
            recs[i].max = 0;
            *nidp = 0;
            i++;
            nidp = &recs[i].nids;
            recs[i - 1].ids[recs[i - 1].nids] = 0x1af;
            recs[i - 1].nids++;
            y += 0x18;
            if (y + 0x16 > 0x1b5) {
                APPR_TOP_OF_PAGE();
                if (pagestart != first) {
                    i = first;
                    x = *xp;
                    AppraisalPageCount++;
                    pagestart = first;
                    goto advice;
                }
                APPR_NEW_PAGE();
            }
            APPR_ROW(-3, 0x1b0);
            recs[i].arg = 0;
            recs[i].bar = 0;
            recs[i].value = 0;
            recs[i].goal = 0;
            recs[i].max = 0;
            *nidp = 0;
            i++;
            nidp = &recs[i].nids;
            recs[i - 1].ids[recs[i - 1].nids] = 0x1b0;
            recs[i - 1].nids++;
            y += 0x18;
            total++;
            break;
        case 2:
            if (y + 0x16 > 0x1b5) {
                APPR_TOP_OF_PAGE();
                if (pagestart != first) {
                    i = first;
                    x = *xp;
                    AppraisalPageCount++;
                    pagestart = first;
                    goto advice;
                }
                APPR_NEW_PAGE();
            }
            APPR_ROW(-1, 0x1b8);
            recs[i].arg = 0;
            recs[i].bar = 0;
            recs[i].value = 0;
            recs[i].goal = 0;
            recs[i].max = 0;
            *nidp = 0;
            i++;
            nidp = &recs[i].nids;
            recs[i - 1].ids[recs[i - 1].nids] = 0x1b8;
            recs[i - 1].nids++;
            y += 0x18;
            if (y + 0x16 > 0x1b5) {
                APPR_TOP_OF_PAGE();
                if (pagestart != first) {
                    i = first;
                    x = *xp;
                    AppraisalPageCount++;
                    pagestart = first;
                    goto advice;
                }
                APPR_NEW_PAGE();
            }
            APPR_ROW(-3, 0x1b9);
            recs[i].arg = 0;
            recs[i].bar = 0;
            recs[i].value = 0;
            recs[i].goal = 0;
            recs[i].max = 0;
            *nidp = 0;
            i++;
            nidp = &recs[i].nids;
            recs[i - 1].ids[recs[i - 1].nids] = 0x1b9;
            recs[i - 1].nids++;
            y += 0x18;
            total++;
            break;
        }
    }
    /* section 9 (0x44bed0-0x44d744): advice rows for the failed checks 0x7000, 0x18000, 0x260000,
     * 0x1080000 and 0xc00000; holds advice_restart, the restart of the advice part */
    if (flags & 0x7000) {
        switch (rand() % 3) {
        case 0:
            if (y + 0x16 > 0x1b5) {
                APPR_TOP_OF_PAGE();
                if (pagestart != first) {
                    i = first;
                    x = *xp;
                    AppraisalPageCount++;
                    pagestart = first;
                    goto advice;
                }
                APPR_NEW_PAGE();
            }
            APPR_ROW(-1, 0x1c2);
            recs[i].arg = 0;
            recs[i].bar = 0;
            recs[i].value = 0;
            recs[i].goal = 0;
            recs[i].max = 0;
            *nidp = 0;
            i++;
            nidp = &recs[i].nids;
            recs[i - 1].ids[recs[i - 1].nids] = 0x1c2;
            recs[i - 1].nids++;
            y += 0x18;
            if (y + 0x16 > 0x1b5) {
                APPR_TOP_OF_PAGE();
                if (pagestart != first) {
                    i = first;
                    x = *xp;
                    AppraisalPageCount++;
                    pagestart = first;
                    goto advice;
                }
                APPR_NEW_PAGE();
            }
            APPR_ROW(-3, 0x1c3);
            recs[i].arg = 0;
            recs[i].bar = 0;
            recs[i].value = 0;
            recs[i].goal = 0;
            recs[i].max = 0;
            *nidp = 0;
            i++;
            nidp = &recs[i].nids;
            recs[i - 1].ids[recs[i - 1].nids] = 0x1c3;
            recs[i - 1].nids++;
            y += 0x18;
            if (y + 0x16 > 0x1b5) {
                APPR_TOP_OF_PAGE();
                if (pagestart != first) {
                    i = first;
                    x = *xp;
                    AppraisalPageCount++;
                    pagestart = first;
                    goto advice;
                }
                APPR_NEW_PAGE();
            }
            APPR_ROW(-3, 0x1c4);
            recs[i].arg = 0;
            recs[i].bar = 0;
            recs[i].value = 0;
            recs[i].goal = 0;
            recs[i].max = 0;
            *nidp = 0;
            i++;
            nidp = &recs[i].nids;
            recs[i - 1].ids[recs[i - 1].nids] = 0x1c4;
            recs[i - 1].nids++;
            y += 0x18;
            total++;
            break;
        case 1:
            if (y + 0x16 > 0x1b5) {
                APPR_TOP_OF_PAGE();
                if (pagestart != first) {
                    i = first;
                    x = *xp;
                    AppraisalPageCount++;
                    pagestart = first;
                    goto advice;
                }
                APPR_NEW_PAGE();
            }
            APPR_ROW(-1, 0x1cc);
            recs[i].arg = 0;
            recs[i].bar = 0;
            recs[i].value = 0;
            recs[i].goal = 0;
            recs[i].max = 0;
            *nidp = 0;
            i++;
            nidp = &recs[i].nids;
            recs[i - 1].ids[recs[i - 1].nids] = 0x1cc;
            recs[i - 1].nids++;
            y += 0x18;
            if (y + 0x16 > 0x1b5) {
                APPR_TOP_OF_PAGE();
                if (pagestart != first) {
                    i = first;
                    x = *xp;
                    AppraisalPageCount++;
                    pagestart = first;
                    goto advice;
                }
                APPR_NEW_PAGE();
            }
            APPR_ROW(-3, 0x1cd);
            recs[i].arg = 0;
            recs[i].bar = 0;
            recs[i].value = 0;
            recs[i].goal = 0;
            recs[i].max = 0;
            *nidp = 0;
            i++;
            nidp = &recs[i].nids;
            recs[i - 1].ids[recs[i - 1].nids] = 0x1cd;
            recs[i - 1].nids++;
            y += 0x18;
            total++;
            break;
        }
    }
    if (flags & 0x18000) {
        switch (rand() % 3) {
        case 0:
            if (y + 0x16 > 0x1b5) {
                APPR_TOP_OF_PAGE();
                if (pagestart != first) {
                    i = first;
                    x = *xp;
                    AppraisalPageCount++;
                    pagestart = first;
                    goto advice;
                }
                APPR_NEW_PAGE();
            }
            APPR_ROW(-1, 0x1d6);
            recs[i].arg = 0;
            recs[i].bar = 0;
            recs[i].value = 0;
            recs[i].goal = 0;
            recs[i].max = 0;
            *nidp = 0;
            i++;
            nidp = &recs[i].nids;
            recs[i - 1].ids[recs[i - 1].nids] = 0x1d6;
            recs[i - 1].nids++;
            y += 0x18;
            if (y + 0x16 > 0x1b5) {
                APPR_TOP_OF_PAGE();
                if (pagestart != first) {
                    i = first;
                    x = *xp;
                    AppraisalPageCount++;
                    pagestart = first;
                    goto advice;
                }
                APPR_NEW_PAGE();
            }
            APPR_ROW(-3, 0x1d7);
            recs[i].arg = 0;
            recs[i].bar = 0;
            recs[i].value = 0;
            recs[i].goal = 0;
            recs[i].max = 0;
            *nidp = 0;
            i++;
            nidp = &recs[i].nids;
            recs[i - 1].ids[recs[i - 1].nids] = 0x1d7;
            recs[i - 1].nids++;
            y += 0x18;
            if (y + 0x16 > 0x1b5) {
                APPR_TOP_OF_PAGE();
                if (pagestart != first) {
                    i = first;
                    x = *xp;
                    AppraisalPageCount++;
                    pagestart = first;
                    goto advice;
                }
                APPR_NEW_PAGE();
            }
            APPR_ROW(-3, 0x1d8);
            recs[i].arg = 0;
            recs[i].bar = 0;
            recs[i].value = 0;
            recs[i].goal = 0;
            recs[i].max = 0;
            *nidp = 0;
            i++;
            nidp = &recs[i].nids;
            recs[i - 1].ids[recs[i - 1].nids] = 0x1d8;
            recs[i - 1].nids++;
            y += 0x18;
            total++;
            break;
        case 1:
            if (y + 0x16 > 0x1b5) {
                APPR_TOP_OF_PAGE();
                if (pagestart != first) {
                    i = first;
                    x = *xp;
                    AppraisalPageCount++;
                    pagestart = first;
                    goto advice;
                }
                APPR_NEW_PAGE();
            }
            APPR_ROW(-1, 0x1e0);
            recs[i].arg = 0;
            recs[i].bar = 0;
            recs[i].value = 0;
            recs[i].goal = 0;
            recs[i].max = 0;
            *nidp = 0;
            i++;
            nidp = &recs[i].nids;
            recs[i - 1].ids[recs[i - 1].nids] = 0x1e0;
            recs[i - 1].nids++;
            y += 0x18;
            if (y + 0x16 > 0x1b5) {
                APPR_TOP_OF_PAGE();
                if (pagestart != first) {
                    i = first;
                    x = *xp;
                    AppraisalPageCount++;
                    pagestart = first;
                    goto advice;
                }
                APPR_NEW_PAGE();
            }
            APPR_ROW(-3, 0x1e1);
            recs[i].arg = 0;
            recs[i].bar = 0;
            recs[i].value = 0;
            recs[i].goal = 0;
            recs[i].max = 0;
            *nidp = 0;
            i++;
            nidp = &recs[i].nids;
            recs[i - 1].ids[recs[i - 1].nids] = 0x1e1;
            recs[i - 1].nids++;
            y += 0x18;
            total++;
            break;
        }
    }
    if (flags & 0x260000) {
        switch (rand() % 3) {
        case 0:
            if (flags & 0x200000) {
                if (y + 0x16 > 0x1b5) {
                    APPR_TOP_OF_PAGE();
                    if (pagestart != first) {
                        i = first;
                        x = *xp;
                        AppraisalPageCount++;
                        pagestart = first;
                        goto advice;
                    }
                    APPR_NEW_PAGE();
                }
                APPR_ROW(-1, 0x1ea);
                recs[i].arg = 0;
                recs[i].bar = 0;
                recs[i].value = 0;
                recs[i].goal = 0;
                recs[i].max = 0;
                *nidp = 0;
                i++;
                nidp = &recs[i].nids;
                recs[i - 1].ids[recs[i - 1].nids] = 0x1ea;
                recs[i - 1].nids++;
                y += 0x18;
                if (y + 0x16 > 0x1b5) {
                    APPR_TOP_OF_PAGE();
                    if (pagestart != first) {
                        i = first;
                        x = *xp;
                        AppraisalPageCount++;
                        pagestart = first;
                        goto advice;
                    }
                    APPR_NEW_PAGE();
                }
                APPR_ROW(-3, 0x1eb);
                recs[i].arg = 0;
                recs[i].bar = 0;
                recs[i].value = 0;
                recs[i].goal = 0;
                recs[i].max = 0;
                *nidp = 0;
                i++;
                nidp = &recs[i].nids;
                recs[i - 1].ids[recs[i - 1].nids] = 0x1eb;
                recs[i - 1].nids++;
                y += 0x18;
                if (y + 0x16 > 0x1b5) {
                    APPR_TOP_OF_PAGE();
                    if (pagestart != first) {
                        i = first;
                        x = *xp;
                        AppraisalPageCount++;
                        pagestart = first;
                        goto advice;
                    }
                    APPR_NEW_PAGE();
                }
                APPR_ROW(-3, 0x1ec);
                recs[i].arg = 0;
                recs[i].bar = 0;
                recs[i].value = 0;
                recs[i].goal = 0;
                recs[i].max = 0;
                *nidp = 0;
                i++;
                nidp = &recs[i].nids;
                recs[i - 1].ids[recs[i - 1].nids] = 0x1ec;
                recs[i - 1].nids++;
                y += 0x18;
                total++;
            }
            break;
        case 1:
            if (y + 0x16 > 0x1b5) {
                APPR_TOP_OF_PAGE();
                if (pagestart != first) {
                    i = first;
                    x = *xp;
                    AppraisalPageCount++;
                    pagestart = first;
                    goto advice;
                }
                APPR_NEW_PAGE();
            }
            APPR_ROW(-1, 0x1f4);
            recs[i].arg = 0;
            recs[i].bar = 0;
            recs[i].value = 0;
            recs[i].goal = 0;
            recs[i].max = 0;
            *nidp = 0;
            i++;
            nidp = &recs[i].nids;
            recs[i - 1].ids[recs[i - 1].nids] = 0x1f4;
            recs[i - 1].nids++;
            y += 0x18;
            if (y + 0x16 > 0x1b5) {
                APPR_TOP_OF_PAGE();
                if (pagestart != first) {
                    i = first;
                    x = *xp;
                    AppraisalPageCount++;
                    pagestart = first;
                    goto advice;
                }
                APPR_NEW_PAGE();
            }
            APPR_ROW(-3, 0x1f5);
            recs[i].arg = 0;
            recs[i].bar = 0;
            recs[i].value = 0;
            recs[i].goal = 0;
            recs[i].max = 0;
            *nidp = 0;
            i++;
            nidp = &recs[i].nids;
            recs[i - 1].ids[recs[i - 1].nids] = 0x1f5;
            recs[i - 1].nids++;
            y += 0x18;
            total++;
            break;
        case 2:
            if (ReportFlags & 0xf) {
                if (y + 0x16 > 0x1b5) {
                    APPR_TOP_OF_PAGE();
                    if (pagestart != first) {
                        i = first;
                        x = *xp;
                        AppraisalPageCount++;
                        pagestart = first;
                        goto advice;
                    }
                    APPR_NEW_PAGE();
                }
                APPR_ROW(-1, 0x1fe);
                recs[i].arg = 0;
                recs[i].bar = 0;
                recs[i].value = 0;
                recs[i].goal = 0;
                recs[i].max = 0;
                *nidp = 0;
                i++;
                nidp = &recs[i].nids;
                recs[i - 1].ids[recs[i - 1].nids] = 0x1fe;
                recs[i - 1].nids++;
                y += 0x18;
                if (y + 0x16 > 0x1b5) {
                    APPR_TOP_OF_PAGE();
                    if (pagestart != first) {
                        i = first;
                        x = *xp;
                        AppraisalPageCount++;
                        pagestart = first;
                        goto advice;
                    }
                    APPR_NEW_PAGE();
                }
                APPR_ROW(-3, 0x1ff);
                recs[i].arg = 0;
                recs[i].bar = 0;
                recs[i].value = 0;
                recs[i].goal = 0;
                recs[i].max = 0;
                *nidp = 0;
                i++;
                nidp = &recs[i].nids;
                recs[i - 1].ids[recs[i - 1].nids] = 0x1ff;
                recs[i - 1].nids++;
                y += 0x18;
                total++;
            }
            break;
        }
    }
    if (flags & 0x1080000) {
        if (rand() & 1) {
            if (y + 0x16 > 0x1b5) {
                APPR_TOP_OF_PAGE();
                if (pagestart != first) {
                    i = first;
                    x = *xp;
                    AppraisalPageCount++;
                    pagestart = first;
                    goto advice;
                }
                APPR_NEW_PAGE();
            }
            APPR_ROW(-1, 0x208);
            recs[i].arg = 0;
            recs[i].bar = 0;
            recs[i].value = 0;
            recs[i].goal = 0;
            recs[i].max = 0;
            *nidp = 0;
            i++;
            nidp = &recs[i].nids;
            recs[i - 1].ids[recs[i - 1].nids] = 0x208;
            recs[i - 1].nids++;
            y += 0x18;
        } else {
            if (y + 0x16 > 0x1b5) {
                APPR_TOP_OF_PAGE();
                if (pagestart != first) {
                    i = first;
                    x = *xp;
                    AppraisalPageCount++;
                    pagestart = first;
                    goto advice;
                }
                APPR_NEW_PAGE();
            }
            APPR_ROW(-1, 0x212);
            recs[i].arg = 0;
            recs[i].bar = 0;
            recs[i].value = 0;
            recs[i].goal = 0;
            recs[i].max = 0;
            *nidp = 0;
            i++;
            nidp = &recs[i].nids;
            recs[i - 1].ids[recs[i - 1].nids] = 0x212;
            recs[i - 1].nids++;
            y += 0x18;
            if (y + 0x16 > 0x1b5) {
                APPR_TOP_OF_PAGE();
                if (pagestart != first) {
                    i = first;
                    x = *xp;
                    AppraisalPageCount++;
                    pagestart = first;
                    goto advice;
                }
                APPR_NEW_PAGE();
            }
            APPR_ROW(-3, 0x213);
            recs[i].arg = 0;
            recs[i].bar = 0;
            recs[i].value = 0;
            recs[i].goal = 0;
            recs[i].max = 0;
            *nidp = 0;
            i++;
            nidp = &recs[i].nids;
            recs[i - 1].ids[recs[i - 1].nids] = 0x213;
            recs[i - 1].nids++;
            y += 0x18;
        }
        total++;
    }
    if (flags & 0xc00000) {
        if (rand() & 1) {
            if (flags & 0x800000) {
                if (y + 0x16 > 0x1b5) {
                    APPR_TOP_OF_PAGE();
                    if (pagestart != first) {
                        i = first;
                        x = *xp;
                        AppraisalPageCount++;
                        pagestart = first;
                        goto advice;
                    }
                    APPR_NEW_PAGE();
                }
                APPR_ROW(-1, 0x21c);
                recs[i].arg = 0;
                recs[i].bar = 0;
                recs[i].value = 0;
                recs[i].goal = 0;
                recs[i].max = 0;
                *nidp = 0;
                i++;
                /* nidp is not moved on here */
                recs[i - 1].ids[recs[i - 1].nids] = 0x21c;
                recs[i - 1].nids++;
                y += 0x18;
                if (y + 0x16 > 0x1b5) {
                    APPR_TOP_OF_PAGE();
                    if (pagestart != first) {
                        i = first;
                        x = *xp;
                        AppraisalPageCount++;
                        pagestart = first;
                        goto advice;
                    }
                    /* the new page does not move pagestart */
                    AppraisalPageCount++;
                    rc.cur.left = rc.layout.left;
                    y = rc.layout.top;
                    rc.cur.right = rc.layout.right;
                    rc.cur.bottom = rc.layout.bottom;
                }
                APPR_ROW(-3, 0x21d);
                recs[i].arg = 0;
                recs[i].bar = 0;
                recs[i].value = 0;
                recs[i].goal = 0;
                recs[i].max = 0;
                recs[i].nids = 0; /* nidp still points at the previous row's count */
                i++;
                /* nidp is not moved on here */
                recs[i - 1].ids[recs[i - 1].nids] = 0x21d;
                recs[i - 1].nids++;
                y += 0x18;
                total++;
            }
        } else {
            if (flags & 0x400000) {
                if (y + 0x16 > 0x1b5) {
                    APPR_TOP_OF_PAGE();
                    if (pagestart != first) {
                        i = first;
                        x = *xp;
                        AppraisalPageCount++;
                        pagestart = first;
                        goto advice;
                    }
                    APPR_NEW_PAGE();
                }
                APPR_ROW(-1, 0x226);
                recs[i].arg = 0;
                recs[i].bar = 0;
                recs[i].value = 0;
                recs[i].goal = 0;
                recs[i].max = 0;
                *nidp = 0;
                i++;
                /* nidp is not moved on here */
                recs[i - 1].ids[recs[i - 1].nids] = 0x226;
                recs[i - 1].nids++;
                y += 0x18;
                if (y + 0x16 > 0x1b5) {
                    APPR_TOP_OF_PAGE();
                    if (pagestart != first) {
                        /* every page break in the advice rows that cannot start a new page restarts the advice here */
                        i = first;
                        x = *xp;
                        AppraisalPageCount++;
                        pagestart = first;
                        goto advice;
                    }
                    /* the new page does not move pagestart */
                    AppraisalPageCount++;
                    rc.cur.left = rc.layout.left;
                    y = rc.layout.top;
                    rc.cur.right = rc.layout.right;
                    rc.cur.bottom = rc.layout.bottom;
                }
                APPR_ROW(-3, 0x227);
                recs[i].arg = 0;
                recs[i].bar = 0;
                recs[i].value = 0;
                recs[i].goal = 0;
                recs[i].max = 0;
                recs[i].nids = 0; /* nidp still points at the previous row's count */
                i++;
                /* nidp is not moved on here */
                recs[i - 1].ids[recs[i - 1].nids] = 0x227;
                recs[i - 1].nids++;
                y += 0x18;
                total++;
            }
        }
    }
    /* section 10 (0x44d744-0x44db06): end of the layout, the display/speech loop, the result */
    if (total == 0) {
        /* no advice rows: drop the advice header */
        i--;
    }
    /* the original sets up a further row rectangle here that nothing uses */
    rc.cur.bottom = y + 0x16;
    rc.cur.left = x + 0x20;
    rc.cur.right = 0x1a4;
    InitAppraisalScreen(++AppraisalPageCount);
    LoadAppraisalSprites();
    while (AppraisalScreenActive) {
        SpeechStreamUpdate();
        SetPointer(5);
        ReadGameButtons();
        ResetHitInfo();
        PushRenderingStatusAndLockVideoSurface();
        PrintSprite(SPRITE_TitleScreenBk, 0, 0, 0, 0);
        UnlightAppraisalPageButtons();
        RenderIcons2(1, 0, 0);
        r.top = 0x45;
        r.bottom = 0x6d;
        r.left = 0x28;
        r.right = 0x1a4;
        NewPrintCent(GetString(0x228), 3, r, 0);
        /* the rows of the page being shown start at the top */
        rc.cur = rc.layout;
        for (shown = 0; shown < i; shown++) {
            if (recs[shown].page == AppraisalPage) {
                break;
            }
        }
        if (AppraisalPageChanged) {
            played = 0;
            flags = 0; /* flags is free now: it counts the speech ids queued for the page */
        }
        for (; shown < i; shown++) {
            if (recs[shown].page != AppraisalPage) {
                break;
            }
            if (AppraisalPageChanged) {
                nids = recs[shown].nids;
                if (nids != 0) {
                    queue[flags++] = recs[shown].ids[0];
                    if (nids > 1) {
                        queue[flags++] = recs[shown].ids[1];
                    }
                    if (nids > 2) {
                        queue[flags++] = recs[shown].ids[2];
                    }
                    if (nids > 3) {
                        queue[flags++] = recs[shown].ids[3];
                    }
                    if (nids > 4) {
                        queue[flags++] = recs[shown].ids[4];
                    }
                    if (nids > 5) {
                        queue[flags++] = recs[shown].ids[5];
                    }
                    if (nids > 6) {
                        queue[flags++] = recs[shown].ids[6];
                    }
                    if (nids > 7) {
                        queue[flags++] = recs[shown].ids[7];
                    }
                }
            }
            if (recs[shown].type == -2) {
                rc.cur.left = recs[shown].x + 0x28;
            } else {
                rc.cur.left = recs[shown].x + 0x50;
            }
            DrawAppraisalMark(rc.cur.left - 0x28, rc.cur.top, recs[shown].type, recs[shown].rnd);
            r.left = rc.cur.left;
            r.top = rc.cur.top;
            r.right = 0x1a4;
            rc.cur.bottom = r.top + 0x16;
            r.bottom = rc.cur.bottom;
            DrawTextOnRenderSurface(recs[shown].text, 2, r, recs[shown].arg);
            if (recs[shown].bar) {
                r.left = 0x126;
                r.right = 0x1a4;
                r.top = rc.cur.top;
                rc.cur.bottom = r.top + 8;
                r.bottom = rc.cur.bottom;
                DrawAppraisalBar(r, recs[shown].value, recs[shown].max, recs[shown].goal);
            }
            rc.cur.top += 0x18;
        }
        if (AppraisalPageChanged) {
            AppraisalPageChanged = 0;
        }
        if (played >= flags) {
            if (SpeechIsPlaying() == 0) {
                UpdateSpeechPlayback();
            }
        } else if (SpeechIsPlaying() == 0) {
            if (queue[played] != -1) {
                // STRING: LEGOLAND 0x004b81a8
                sprintf(wavbuf, "TEXT%04d.WAV", queue[played++]);
                SpeechCloseFile();
                SpeechLoadWavFile(wavbuf);
                SpeechPlay();
            }
        }
        ProcessFrontEndHelp();
        UpdateFocussedIconPtr();
        PopRenderingStatus();
        if (FocussedIconPtr != 0) {
            SetPointer(6);
        }
        CheckFocussedIcon();
        RenderingComplete();
    }
    KillAppraisalSprites();
    PopRenderingStatus();
    SpeechCloseFile();
    FUN_00474880();
    /* passed if no check failed */
    for (shown = 0; shown < i; shown++) {
        if (recs[shown].type == 0) {
            return 0;
        }
    }
    return 1;
    return 0;
}
#undef APPR_TOP_OF_PAGE
#undef APPR_NEW_PAGE
#undef APPR_RETRY
#undef APPR_ROW
#undef APPR_TOP_OF_PAGE
#undef APPR_NEW_PAGE
#undef APPR_RETRY
#undef APPR_ROW
#undef APPR_TOP_OF_PAGE
#undef APPR_NEW_PAGE
#undef APPR_RETRY
#undef APPR_ROW

// FUNCTION: LEGOLAND 0x0044db20
void ResetAppraisalStreak(void) {
    MapStats.appraisal_streak = 0;
}

// FUNCTION: LEGOLAND 0x0044db40
void StartAppraisalTimer(void) {
    unsigned int t;

    if (MapStats.timer_minutes != 0) {
        t = GetGameTimer() + MapStats.timer_minutes * 60000;
    } else {
        t = 0;
    }
    AppraisalDeadline = t;
#ifdef LEGOLAND_PORT
    /* [port] log the inspector's timer, to tell a waiting level from a stuck one */
    DebugTrace("appraisal: next in %u min (deadline %u, game time %u)", MapStats.timer_minutes, t, GetGameTimer());
#endif
}

// FUNCTION: LEGOLAND 0x0044db80
void StopAppraisalTimer(void) {
    MapStats.timer_minutes = 0;
    AppraisalDeadline = 0;
}

// FUNCTION: LEGOLAND 0x0044db90
int CheckAppraisalDue(void) {
    int now;
    int v;

    now = GetGameTimer();
    if (IsScriptStopped() == 0 && AppraisalDeadline != 0 && (int)AppraisalDeadline <= now) {
        PauseGameTimer();
        PauseAllSamples();
        SpeechCloseFile();
        DAT_006687b0 = 4;
        DAT_0066609c = RunAppraisal();
#ifdef LEGOLAND_PORT
        /* [port] log each appraisal's result (appraisal_streak: passes > 0, failures < 0) */
        DebugTrace("appraisal: %s (streak before this %d, failures allowed %d)", DAT_0066609c != 0 ? "PASSED" : "failed",
            MapStats.appraisal_streak, MapStats.appraisal_fail_limit);
#endif
        if (DAT_0066609c != 0) {
            v = MapStats.appraisal_streak;
            if (v > 0) {
                MapStats.appraisal_streak = v + 1;
            } else {
                MapStats.appraisal_streak = 1;
            }
            SetScriptStopped(1);
            lpConfig->field_30 = 1;
            FUN_0048a750();
        } else {
            if (MapStats.appraisal_streak < 0) {
                MapStats.appraisal_streak = MapStats.appraisal_streak - 1;
            } else {
                MapStats.appraisal_streak = -1;
            }
            if (MapStats.appraisal_fail_limit != 0 && MapStats.appraisal_streak <= -MapStats.appraisal_fail_limit) {
                EndLevel(2);
            }
        }
        AppraisalDeadline = 0;
        StartAppraisalTimer();
        ResumeGameTimer();
        ResumeAllSamples();
        return 1;
    }
    return 0;
}

// FUNCTION: LEGOLAND 0x0044dc70
void ConfigureAppraisal(unsigned int param_1, unsigned int param_2) {
    MapStats.appraisal_fail_limit = param_1;
    ResetAppraisalStreak();
    FUN_004597e0(0, (const char *)param_2);
}
