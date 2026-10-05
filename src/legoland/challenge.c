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
    struct ResFile *file;
    unsigned int offbits;
    int pixel_offset;
    unsigned int row_size;
    unsigned int aligned_width;
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
        for (x = 0; x < 0x100; x++) {
            DAT_0081c4c0[x] = 0;
        }
        RES_ReadFile(file, DAT_0081c4c0, *(int *)(info + 0x20) * 4);
        RES_SetFilePointer(file, offbits);
        RES_ReadFile(file, pixels, row_size);
        for (y = 0; y < param_1->height; y++) {
            src -= aligned_width;
            for (x = 0; x < param_1->width; x++) {
                *dst = *src;
                src++;
                dst++;
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
unsigned int FUN_00443710(void) {
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
void FUN_004437d0(struct Image *param_1, struct TextureNode *param_2) {
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
void FUN_00443d90(void) {
    DAT_004b7d7e = 0x10;
    DAT_004b7d74 = 0x70;
    DAT_004b7d78 = 0x60;
    DAT_004b7d84 = 0x5400;
}

// FUNCTION: LEGOLAND 0x00443dc0
void FUN_00443dc0(struct AnimHandle *handle) {
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
unsigned int FUN_00443e30(struct AdvisorObject *param_1) {
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
                FUN_00443dc0((struct AnimHandle *)DAT_00665f60);
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
                 * AVIs are decoded to, see FUN_00443d90). */
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
void *FUN_00443f90(unsigned int param_1) {
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
unsigned int FUN_00443fe0(unsigned int param_1) {
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
void *FUN_00444020(void) {
    unsigned int handle;

    if (DAT_00665fec == DAT_00665fe8) {
        DAT_00665fec = FUN_00443fe0(DAT_00665fe8);
    }
    handle = DAT_00665fec;
    DAT_00665fe8 = handle;
    DAT_0081c088 = DAT_0081c09c;
    DAT_0081c09c = 0;
    DAT_00665f60 = FUN_00443f90(handle);
    return DAT_00665f60;
}

// FUNCTION: LEGOLAND 0x00444070
void FUN_00444070(unsigned int param_1, unsigned int param_2) {
    DAT_00665fec = param_1;
    DAT_0081c09c = param_2;
    DAT_0081c088 = 0;
}

// FUNCTION: LEGOLAND 0x00444090
void LoadAdvisorAnims(void) {
    FUN_00443d90();

    // STRING: LEGOLAND 0x004b7e24
    AdBlinkAnim = OpenAviAnim("AD_Blink.avi");
    if (AdBlinkAnim != 0) {
        ((struct AnimHandle *)AdBlinkAnim)->callback = FUN_00444020;
    }
    // STRING: LEGOLAND 0x004b7e18
    AdLRAnim = OpenAviAnim("AD_LR.avi");
    if (AdLRAnim != 0) {
        ((struct AnimHandle *)AdLRAnim)->callback = FUN_00444020;
    }
    // STRING: LEGOLAND 0x004b7e08
    AdPhoneAnim = OpenAviAnim("AD_Phone.avi");
    if (AdPhoneAnim != 0) {
        ((struct AnimHandle *)AdPhoneAnim)->callback = FUN_00444020;
    }
    // STRING: LEGOLAND 0x004b7df4
    AdPhoneGestureAnim = OpenAviAnim("AD_PhoneGesture.avi");
    if (AdPhoneGestureAnim != 0) {
        ((struct AnimHandle *)AdPhoneGestureAnim)->callback = FUN_00444020;
    }
    // STRING: LEGOLAND 0x004b7de0
    AdPhoneDownAnim = OpenAviAnim("AD_PhoneDown.avi");
    if (AdPhoneDownAnim != 0) {
        ((struct AnimHandle *)AdPhoneDownAnim)->callback = FUN_00444020;
    }
    // STRING: LEGOLAND 0x004b7dd0
    AdWobbleAnim = OpenAviAnim("AD_Wobble.avi");
    if (AdWobbleAnim != 0) {
        ((struct AnimHandle *)AdWobbleAnim)->callback = FUN_00444020;
    }
    FUN_00443dc0(AdBlinkAnim);
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
void FUN_004441f0(void) {
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
unsigned int FUN_004442c0(void) {
    struct RideElem *elem;

    elem = (struct RideElem *)ElemID("DRIVING SCHOOL"); /* TODO: fold — ElemID handle (uint) viewed as RideElem* */
    if ((elem->flags & 1) != 0) {
        return elem->obj->func(elem->obj->arg, 0);
    }
    return 0;
}

// FUNCTION: LEGOLAND 0x004442f0
unsigned int FUN_004442f0(void) {
    struct RideElem *elem;

    elem = (struct RideElem *)ElemID("BOATING SCHOOL");
    if ((elem->flags & 1) != 0) {
        return elem->obj->func(elem->obj->arg, 0);
    }
    return 0;
}

// FUNCTION: LEGOLAND 0x00444320
unsigned int FUN_00444320(void) {
    struct RideElem *elem;

    elem = (struct RideElem *)ElemID("CASTLE OBJ");
    if ((elem->flags & 1) != 0) {
        return elem->obj->func(elem->obj->arg, 0);
    }
    return 0;
}

// FUNCTION: LEGOLAND 0x00444350
unsigned int FUN_00444350(void) {
    struct RideElem *elem;

    elem = (struct RideElem *)ElemID("LOG FLUME ENTRANCE");
    if ((elem->flags & 1) != 0) {
        return elem->obj->func(elem->obj->arg, 0);
    }
    return 0;
}

// FUNCTION: LEGOLAND 0x00444380
unsigned int FUN_00444380(void) {
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
void FUN_00444470(unsigned int param_1, unsigned int param_2) {
    if ((param_1 | param_2) != 0) {
        ReportFlags |= (param_2 & 3) << 4;
        DAT_0066600c = param_1;
    } else {
        ReportFlags = (ReportFlags & 0xffffff00) | (ReportFlags & 0xcf);
    }
}

// FUNCTION: LEGOLAND 0x004444b0
void FUN_004444b0(unsigned int param_1, unsigned int param_2) {
    if ((param_1 | param_2) != 0) {
        ReportFlags |= (param_2 & 3) << 6;
        DAT_00666010 = param_1;
    } else {
        ReportFlags = (ReportFlags & 0xffffff00) | (ReportFlags & 0x3f);
    }
}

// FUNCTION: LEGOLAND 0x004444f0
void FUN_004444f0(unsigned int param_1, unsigned int param_2) {
    if ((param_1 | param_2) != 0) {
        ReportFlags |= (param_2 & 3) << 8;
        DAT_00666014 = param_1;
    } else {
        ReportFlags = (ReportFlags & 0xffff00ff) | ((ReportFlags >> 8 & 0xfc) << 8);
    }
}

// FUNCTION: LEGOLAND 0x00444530
void FUN_00444530(unsigned int param_1, unsigned int param_2) {
    if ((param_1 | param_2) != 0) {
        ReportFlags |= (param_2 & 3) << 10;
        DAT_00666018 = param_1;
    } else {
        ReportFlags = (ReportFlags & 0xffff00ff) | ((ReportFlags >> 8 & 0xf3) << 8);
    }
}

// FUNCTION: LEGOLAND 0x00444570
void FUN_00444570(unsigned int param_1, unsigned int param_2) {
    if ((param_1 | param_2) != 0) {
        ReportFlags |= (param_2 & 3) << 12;
        DAT_0066601c = param_1;
    } else {
        ReportFlags = (ReportFlags & 0xffff00ff) | ((ReportFlags >> 8 & 0xcf) << 8);
    }
}

// FUNCTION: LEGOLAND 0x004445b0
void FUN_004445b0(unsigned int param_1, unsigned int param_2) {
    if ((param_1 | param_2) != 0) {
        ReportFlags |= 0x4000;
        DAT_00666020 = param_1;
        DAT_00666024 = param_2;
    } else {
        ReportFlags &= ~0x4000;
    }
}

// FUNCTION: LEGOLAND 0x004445f0
void FUN_004445f0(unsigned int param_1, unsigned int param_2) {
    if ((param_1 | param_2) != 0) {
        ReportFlags |= 0x8000;
        DAT_00666028 = param_1;
        DAT_0066602c = param_2;
    } else {
        ReportFlags &= ~0x8000;
    }
}

// FUNCTION: LEGOLAND 0x00444630
void FUN_00444630(unsigned int param_1, unsigned int param_2) {
    if ((param_1 | param_2) != 0) {
        ReportFlags |= 0x40000;
        DAT_00666030 = param_1;
        DAT_00666034 = param_2;
    } else {
        ReportFlags &= 0xfffbffff;
    }
}

// FUNCTION: LEGOLAND 0x00444670
void FUN_00444670(unsigned int param_1, unsigned int param_2) {
    if ((param_1 | param_2) != 0) {
        ReportFlags |= 0x8000000;
        DAT_00666070 = param_1;
        DAT_00666074 = param_2;
    } else {
        ReportFlags &= 0xf7ffffff;
    }
}

// FUNCTION: LEGOLAND 0x004446b0
void FUN_004446b0(unsigned int param_1, unsigned int param_2) {
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
void FUN_00444730(unsigned int param_1, unsigned int param_2) {
    if ((param_1 | param_2) != 0) {
        ReportFlags |= 0x10000;
        DAT_00666040 = param_1;
        DAT_00666044 = param_2;
    } else {
        ReportFlags &= 0xfffeffff;
    }
}

// FUNCTION: LEGOLAND 0x00444770
void FUN_00444770(unsigned int param_1, unsigned int param_2) {
    if ((param_1 | param_2) != 0) {
        ReportFlags |= 0x20000;
        DAT_00666048 = param_1;
        DAT_0066604c = param_2;
    } else {
        ReportFlags &= 0xfffdffff;
    }
}

// FUNCTION: LEGOLAND 0x004447b0
void FUN_004447b0(unsigned int param_1, unsigned int param_2) {
    if ((param_1 | param_2) != 0) {
        ReportFlags |= 0x40000000;
        DAT_00666088 = param_1;
        DAT_0066608c = param_2;
    } else {
        ReportFlags &= 0xbfffffff;
    }
}

// FUNCTION: LEGOLAND 0x004447f0
void FUN_004447f0(unsigned int param_1, unsigned int param_2) {
    if ((param_1 | param_2) != 0) {
        ReportFlags |= 0x80000000;
        DAT_00666090 = param_1;
        DAT_00666094 = param_2;
    } else {
        ReportFlags &= 0x7fffffff;
    }
}

// FUNCTION: LEGOLAND 0x00444830
void FUN_00444830(unsigned int param_1, unsigned int param_2) {
    if ((param_1 | param_2) != 0) {
        ReportFlags |= 0x80000;
        DAT_00666038 = param_1;
        DAT_0066603c = param_2;
    } else {
        ReportFlags &= 0xfff7ffff;
    }
}

// FUNCTION: LEGOLAND 0x00444870
void FUN_00444870(unsigned int param_1, unsigned int param_2) {
    if ((param_1 | param_2) != 0) {
        ReportFlags |= 0x1000000;
        DAT_00666050 = param_1;
        DAT_00666054 = param_2;
    } else {
        ReportFlags &= 0xfeffffff;
    }
}

// FUNCTION: LEGOLAND 0x004448b0
void FUN_004448b0(unsigned int param_1, unsigned int param_2) {
    if ((param_1 | param_2) != 0) {
        ReportFlags |= 0x4000000;
        DAT_00666050 = param_1;
        DAT_00666054 = param_2;
    } else {
        ReportFlags &= 0xfbffffff;
    }
}

// FUNCTION: LEGOLAND 0x004448f0
void FUN_004448f0(unsigned int param_1, unsigned int param_2) {
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
void FUN_00444970(unsigned int param_1, unsigned int param_2) {
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
void DrawAppraisalBar(int param_1, int param_2, int param_3, int param_4, int param_5, int param_6, int param_7) {
    int negative;
    int bar;
    unsigned int colour;
    int mark;

    if (param_6 < 0) {
        negative = 1;
        param_6 = -param_6;
        param_7 = param_6 - param_7;
    } else {
        negative = 0;
    }
    if (param_5 > param_6) {
        param_5 = param_6;
    }
    if (negative != 0) {
        param_5 = param_6 - param_5;
    }
    if (param_5 >= param_7) {
        colour = GetNearestColour(0, 0xff, 0);
    } else {
        colour = GetNearestColour(0xff, 0, 0);
    }
    bar = ((param_3 - param_1) * param_5) / param_6;
    PrintSprite(AppBarSprite, param_1, param_2, 0, 0);
    RenderBlock(param_1 + 3, param_2 + 2, bar - 2, 1, colour);
    RenderBlock(param_1 + 2, param_2 + 3, bar, (param_4 - param_2) - 1, colour);
    mark = (((param_3 - param_1) - 2) * param_7) / param_6 + 2 + param_1;
    PrintSprite(AppBarMarkerSprite, mark, param_2 + 2, 0, 0);
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
void FUN_00444bf0(unsigned int *param_1, unsigned int *param_2) {
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
int FUN_00444c40(struct ObjectClass *node) {
    return FindStringNoCase((const char *)**(int **)((char *)node + 0xc4), &DAT_004b7e9c, 0x16) >= 0;
}

// FUNCTION: LEGOLAND 0x00444c70
void FUN_00444c70(unsigned int *param_1, unsigned int *param_2) {
    struct ObjectClass *node;

    node = ObjectClassList;
    *param_1 = 0;
    *param_2 = 0;
    if (node == 0) {
        return;
    }
    do {
        if (node->count != 0 && node->type == 2 && FUN_00444c40(node) == 0) {
            *param_1 += node->count;
            *param_2 += 1;
        }
        node = node->next;
    } while (node != 0);
}

// FUNCTION: LEGOLAND 0x00444cd0
void FUN_00444cd0(unsigned int *param_1, unsigned int *param_2) {
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
void FUN_00444d20(unsigned int *param_1, unsigned int *param_2) {
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
void FUN_00444d70(unsigned int *param_1, unsigned int *param_2, int *param_3) {
    struct Bloke *bloke;
    signed char value;

    bloke = FirstBloke;
    *param_1 = 0;
    *param_2 = 0;
    *param_3 = 0;
    if (bloke == 0) {
        return;
    }
    do {
        *param_1 += 1;
        if (bloke->mood > MapStats.mood_threshold3) {
            *param_2 += 1;
        }
        if (bloke->mood > MapStats.mood_threshold4) {
            *param_2 += 1;
        }
        value = FUN_0044eb10(bloke);
        *param_3 += 4 - value;
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
int FUN_00444df0(void) {
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
void InitAppraisalScreen(void) {
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

// FUNCTION: LEGOLAND 0x004453a0
unsigned int RunAppraisal(void) {
    short coord;
    int *piVar2;
    int iVar3;
    int iVar4;
    char *uVar5;
    int iVar6;
    int iVar7;
    int iVar8;
    unsigned int uVar9;
    int *piVar10;
    int *piVar11;
    char *text;
    int iVar12;
    int iVar13;
    int iVar14;
    int iVar15;
    int xbase;
    int tmp8;
    int colstep;
    int ystart;
    int rectr;
    unsigned int wstart;
    int ysave;
    int rowy;
    int closing_text;
    int *flatp;
    unsigned int subpass;
    unsigned int flags;
    unsigned int passtotal;
    unsigned int total;
    int *rowp;
    unsigned int passtotacc;
    unsigned int tottotacc;
    unsigned int passflag;
    int out58;
    int out5c;
    int out60;
    int out64;
    int out68;
    int out6c;
    int out70;
    int out74;
    int out78;
    int out7c;
    int out80;
    int rep[0x777];
    char wavbuf[0x80];
    char fmtbuf[0x1fc];
    int flat[0xc4];

    xbase = 0;
    tottotacc = 0;
    passtotacc = 0;
    flags = 0;
    if (IsScriptStopped() != 0) {
        return 0;
    }
    AppraisalPageCount = 0;
    AppraisalPage = 0;
    AppraisalPageChanged = 1;
    PushRenderingStatusAndUnlockVideoSurface();
    ReadGameButtons();
    colstep = 0x50;
    ystart = 0x6d;
    rectr = 0x1a4;
    wstart = 0x83;
    iVar3 = 0;
LAB_00445422:
    uVar9 = 0x83;
    iVar14 = iVar3;
    iVar4 = 0x6d;
    if ((ReportFlags & 0xf) != 0) {
        rep[iVar3 * 0x13 + 10] = xbase;
        rep[iVar3 * 0x13 + 9] = AppraisalPageCount;
        rep[iVar3 * 0x13 + 10] = xbase;
        rep[iVar3 * 0x13 + 11] = 0;
        rep[iVar3 * 0x13 + 12] = rand() % 5;
        uVar5 = GetString(0x12c);
        iVar14 = iVar3 + 1;
        rep[iVar3 * 0x13 + 13] = (int)uVar5;
        rep[iVar3 * 0x13 + 14] = 0;
        rep[iVar3 * 0x13 + 15] = 0;
        rep[iVar3 * 0x13 + 16] = 0;
        rep[iVar3 * 0x13 + 17] = 0;
        rep[iVar3 * 0x13 + 18] = 0;
        rep[iVar3 * 0x13 + 19] = 0;
        rep[iVar3 * 0x13 + 11] = 1;
        uVar9 = 0x9b;
        iVar4 = 0x85;
    }
    do {
        iVar12 = iVar14;
        if ((ReportFlags & 0x4fff0) == 0) {
        LAB_0044672e:
            if ((ReportFlags & 0x38000000) == 0)
                goto LAB_00446b71;
            iVar14 = iVar12 * 0x4c;
            goto LAB_00446751;
        }
        total = 0;
        passtotal = 0;
        iVar13 = iVar14 * 0x4c;
        piVar10 = &rep[iVar14 * 0x13 + 10];
        *piVar10 = xbase;
        iVar6 = iVar4;
        if (0x1b5 < uVar9) {
            if (iVar3 != iVar14)
                break;
            AppraisalPageCount = AppraisalPageCount + 1;
            iVar6 = ystart;
            iVar3 = iVar14;
        }
        rep[iVar14 * 0x13 + 9] = AppraisalPageCount;
        *piVar10 = xbase;
        rep[iVar14 * 0x13 + 11] = 0;
        rep[iVar14 * 0x13 + 12] = rand() % 5;
        uVar5 = GetString(0x131);
        iVar12 = iVar14 + 1;
        iVar4 = iVar6 + 0x18;
        rep[iVar14 * 0x13 + 13] = (int)uVar5;
        rep[iVar14 * 0x13 + 14] = 0;
        rep[iVar14 * 0x13 + 15] = 0;
        rep[iVar14 * 0x13 + 16] = 0;
        rep[iVar14 * 0x13 + 17] = 0;
        rep[iVar14 * 0x13 + 18] = 0;
        rep[iVar14 * 0x13 + 19] = 0;
        iVar13 = xbase + 0x30;
        FUN_00444bf0((unsigned int *)&out58, (unsigned int *)&out5c);
        if ((ReportFlags & 0x4000) == 0) {
        LAB_00445794:
            if ((ReportFlags & 0x8000) != 0) {
                total = total + 1;
                passflag = (int)DAT_00666028 <= out58;
                if (passflag == 0) {
                    flags = flags | 0x20;
                } else {
                    passtotal = passtotal + 1;
                }
                if (0x1b5 < (unsigned int)(iVar4 + 0x16)) {
                    if (iVar3 != iVar14)
                        goto LAB_004464b7;
                    AppraisalPageCount = AppraisalPageCount + 1;
                    iVar4 = ystart;
                    iVar3 = iVar12;
                }
                rep[iVar12 * 0x13 + 9] = AppraisalPageCount;
                rep[iVar12 * 0x13 + 10] = iVar13;
                rep[iVar12 * 0x13 + 11] = passflag;
                rep[iVar12 * 0x13 + 12] = rand() % 5;
                uVar5 = GetString(0x132);
                iVar12 = iVar12 + 1;
                rep[(iVar12 - 1) * 0x13 + 13] = (int)uVar5;
                rep[(iVar12 - 1) * 0x13 + 14] = 0;
                rep[(iVar12 - 1) * 0x13 + 15] = 1;
                rep[(iVar12 - 1) * 0x13 + 16] = out58;
                rep[(iVar12 - 1) * 0x13 + 17] = DAT_00666028;
                rep[(iVar12 - 1) * 0x13 + 18] = DAT_0066602c;
                rep[(iVar12 - 1) * 0x13 + 19] = 0;
                iVar4 = iVar4 + 0x18;
            }
            if ((ReportFlags & 0x40000) != 0) {
                iVar6 = FUN_00444df0();
                total = total + 1;
                passflag = (int)DAT_00666030 <= iVar6;
                if (passflag == 0) {
                    flags = flags | 0x40;
                } else {
                    passtotal = passtotal + 1;
                }
                if (0x1b5 < (unsigned int)(iVar4 + 0x16)) {
                    if (iVar3 != iVar14)
                        goto LAB_004464b7;
                    AppraisalPageCount = AppraisalPageCount + 1;
                    iVar4 = ystart;
                    iVar3 = iVar12;
                }
                rep[iVar12 * 0x13 + 9] = AppraisalPageCount;
                rep[iVar12 * 0x13 + 10] = iVar13;
                rep[iVar12 * 0x13 + 11] = passflag;
                rep[iVar12 * 0x13 + 12] = rand() % 5;
                uVar5 = GetString(0x133);
                iVar12 = iVar12 + 1;
                rep[(iVar12 - 1) * 0x13 + 13] = (int)uVar5;
                rep[(iVar12 - 1) * 0x13 + 14] = 0;
                rep[(iVar12 - 1) * 0x13 + 15] = 1;
                rep[(iVar12 - 1) * 0x13 + 16] = iVar6;
                rep[(iVar12 - 1) * 0x13 + 17] = DAT_00666030;
                rep[(iVar12 - 1) * 0x13 + 18] = DAT_00666034;
                rep[(iVar12 - 1) * 0x13 + 19] = 0;
                iVar4 = iVar4 + 0x18;
            }
            if ((ReportFlags & 0x30) != 0) {
                total = total + 1;
                iVar6 = FUN_00444320();
                passflag = (int)DAT_0066600c <= iVar6;
                if (passflag == 0) {
                    flags = flags | 0x80;
                } else {
                    passtotal = passtotal + 1;
                }
                uVar9 = ReportFlags >> 4 & 3;
                if (uVar9 == 1) {
                    if (0x1b5 < (unsigned int)(iVar4 + 0x16)) {
                        if (iVar3 != iVar14)
                            goto LAB_004464b7;
                        AppraisalPageCount = AppraisalPageCount + 1;
                        iVar4 = ystart;
                        iVar3 = iVar12;
                    }
                    rep[iVar12 * 0x13 + 9] = AppraisalPageCount;
                    rep[iVar12 * 0x13 + 10] = iVar13;
                    rep[iVar12 * 0x13 + 11] = passflag;
                    rep[iVar12 * 0x13 + 12] = rand() % 5;
                LAB_00445c4c:
                    tmp8 = iVar12 * 0x4c;
                    uVar5 = GetString(0x135);
                    iVar12 = iVar12 + 1;
                    rep[(iVar12 - 1) * 0x13 + 13] = (int)uVar5;
                    rep[(iVar12 - 1) * 0x13 + 14] = 0;
                    rep[(iVar12 - 1) * 0x13 + 15] = 0;
                    rep[(iVar12 - 1) * 0x13 + 16] = 0;
                    rep[(iVar12 - 1) * 0x13 + 17] = 0;
                    rep[(iVar12 - 1) * 0x13 + 18] = 0;
                    rep[(iVar12 - 1) * 0x13 + 19] = 0;
                } else {
                    if (uVar9 == 2) {
                        if (0x1b5 < (unsigned int)(iVar4 + 0x16)) {
                            if (iVar3 != iVar14)
                                goto LAB_004464b7;
                            AppraisalPageCount = AppraisalPageCount + 1;
                            iVar4 = ystart;
                            iVar3 = iVar12;
                        }
                        rep[iVar12 * 0x13 + 9] = AppraisalPageCount;
                        rep[iVar12 * 0x13 + 10] = iVar13;
                        rep[iVar12 * 0x13 + 11] = passflag;
                        rep[iVar12 * 0x13 + 12] = rand() % 5;
                        goto LAB_00445c4c;
                    }
                    if (uVar9 == 3) {
                        if (0x1b5 < (unsigned int)(iVar4 + 0x16)) {
                            if (iVar3 != iVar14)
                                goto LAB_004464b7;
                            AppraisalPageCount = AppraisalPageCount + 1;
                            iVar4 = ystart;
                            iVar3 = iVar12;
                        }
                        rep[iVar12 * 0x13 + 9] = AppraisalPageCount;
                        rep[iVar12 * 0x13 + 10] = iVar13;
                        rep[iVar12 * 0x13 + 11] = passflag;
                        rep[iVar12 * 0x13 + 12] = rand() % 5;
                        goto LAB_00445c4c;
                    }
                }
                iVar4 = iVar4 + 0x18;
            }
            if ((ReportFlags & 0xc0) != 0) {
                total = total + 1;
                iVar6 = FUN_004442c0();
                passflag = (int)DAT_00666010 <= iVar6;
                if (passflag == 0) {
                    flags = flags | 0x100;
                } else {
                    passtotal = passtotal + 1;
                }
                uVar9 = ReportFlags >> 6 & 3;
                if (uVar9 == 1) {
                    if (0x1b5 < (unsigned int)(iVar4 + 0x16)) {
                        if (iVar3 != iVar14)
                            goto LAB_004464b7;
                        AppraisalPageCount = AppraisalPageCount + 1;
                        iVar4 = ystart;
                        iVar3 = iVar12;
                    }
                    rep[iVar12 * 0x13 + 9] = AppraisalPageCount;
                    rep[iVar12 * 0x13 + 10] = iVar13;
                    rep[iVar12 * 0x13 + 11] = passflag;
                    rep[iVar12 * 0x13 + 12] = rand() % 5;
                LAB_00445ed5:
                    tmp8 = iVar12 * 0x4c;
                    uVar5 = GetString(0x138);
                    iVar12 = iVar12 + 1;
                    rep[(iVar12 - 1) * 0x13 + 13] = (int)uVar5;
                    rep[(iVar12 - 1) * 0x13 + 14] = 0;
                    rep[(iVar12 - 1) * 0x13 + 15] = 0;
                    rep[(iVar12 - 1) * 0x13 + 16] = 0;
                    rep[(iVar12 - 1) * 0x13 + 17] = 0;
                    rep[(iVar12 - 1) * 0x13 + 18] = 0;
                    rep[(iVar12 - 1) * 0x13 + 19] = 0;
                } else {
                    if (uVar9 == 2) {
                        if (0x1b5 < (unsigned int)(iVar4 + 0x16)) {
                            if (iVar3 != iVar14)
                                goto LAB_004464b7;
                            AppraisalPageCount = AppraisalPageCount + 1;
                            iVar4 = ystart;
                            iVar3 = iVar12;
                        }
                        rep[iVar12 * 0x13 + 9] = AppraisalPageCount;
                        rep[iVar12 * 0x13 + 10] = iVar13;
                        rep[iVar12 * 0x13 + 11] = passflag;
                        rep[iVar12 * 0x13 + 12] = rand() % 5;
                        goto LAB_00445ed5;
                    }
                    if (uVar9 == 3) {
                        if (0x1b5 < (unsigned int)(iVar4 + 0x16)) {
                            if (iVar3 != iVar14)
                                goto LAB_004464b7;
                            AppraisalPageCount = AppraisalPageCount + 1;
                            iVar4 = ystart;
                            iVar3 = iVar12;
                        }
                        rep[iVar12 * 0x13 + 9] = AppraisalPageCount;
                        rep[iVar12 * 0x13 + 10] = iVar13;
                        rep[iVar12 * 0x13 + 11] = passflag;
                        rep[iVar12 * 0x13 + 12] = rand() % 5;
                        goto LAB_00445ed5;
                    }
                }
                iVar4 = iVar4 + 0x18;
            }
            if ((ReportFlags & 0x300) != 0) {
                total = total + 1;
                iVar6 = FUN_00444350();
                passflag = (int)DAT_00666014 <= iVar6;
                if (passflag == 0) {
                    flags = flags | 0x200;
                } else {
                    passtotal = passtotal + 1;
                }
                uVar9 = ReportFlags >> 8 & 3;
                if (uVar9 == 1) {
                    if (0x1b5 < (unsigned int)(iVar4 + 0x16)) {
                        if (iVar3 != iVar14)
                            goto LAB_004464b7;
                        AppraisalPageCount = AppraisalPageCount + 1;
                        iVar4 = ystart;
                        iVar3 = iVar12;
                    }
                    rep[iVar12 * 0x13 + 9] = AppraisalPageCount;
                    rep[iVar12 * 0x13 + 10] = iVar13;
                    rep[iVar12 * 0x13 + 11] = passflag;
                    rep[iVar12 * 0x13 + 12] = rand() % 5;
                LAB_0044615f:
                    tmp8 = iVar12 * 0x4c;
                    uVar5 = GetString(0x13b);
                    iVar12 = iVar12 + 1;
                    rep[(iVar12 - 1) * 0x13 + 13] = (int)uVar5;
                    rep[(iVar12 - 1) * 0x13 + 14] = 0;
                    rep[(iVar12 - 1) * 0x13 + 15] = 0;
                    rep[(iVar12 - 1) * 0x13 + 16] = 0;
                    rep[(iVar12 - 1) * 0x13 + 17] = 0;
                    rep[(iVar12 - 1) * 0x13 + 18] = 0;
                    rep[(iVar12 - 1) * 0x13 + 19] = 0;
                } else {
                    if (uVar9 == 2) {
                        if (0x1b5 < (unsigned int)(iVar4 + 0x16)) {
                            if (iVar3 != iVar14)
                                goto LAB_004464b7;
                            AppraisalPageCount = AppraisalPageCount + 1;
                            iVar4 = ystart;
                            iVar3 = iVar12;
                        }
                        rep[iVar12 * 0x13 + 9] = AppraisalPageCount;
                        rep[iVar12 * 0x13 + 10] = iVar13;
                        rep[iVar12 * 0x13 + 11] = passflag;
                        rep[iVar12 * 0x13 + 12] = rand() % 5;
                        goto LAB_0044615f;
                    }
                    if (uVar9 == 3) {
                        if (0x1b5 < (unsigned int)(iVar4 + 0x16)) {
                            if (iVar3 != iVar14)
                                goto LAB_004464b7;
                            AppraisalPageCount = AppraisalPageCount + 1;
                            iVar4 = ystart;
                            iVar3 = iVar12;
                        }
                        rep[iVar12 * 0x13 + 9] = AppraisalPageCount;
                        rep[iVar12 * 0x13 + 10] = iVar13;
                        rep[iVar12 * 0x13 + 11] = passflag;
                        rep[iVar12 * 0x13 + 12] = rand() % 5;
                        goto LAB_0044615f;
                    }
                }
                iVar4 = iVar4 + 0x18;
            }
            if ((ReportFlags & 0xc00) != 0) {
                total = total + 1;
                iVar6 = FUN_004442f0();
                passflag = (int)DAT_00666018 <= iVar6;
                if (passflag == 0) {
                    flags = flags | 0x400;
                } else {
                    passtotal = passtotal + 1;
                }
                uVar9 = ReportFlags >> 10 & 3;
                if (uVar9 == 1) {
                    if (0x1b5 < (unsigned int)(iVar4 + 0x16)) {
                        if (iVar3 != iVar14)
                            goto LAB_004464b7;
                        AppraisalPageCount = AppraisalPageCount + 1;
                        iVar4 = ystart;
                        iVar3 = iVar12;
                    }
                    rep[iVar12 * 0x13 + 9] = AppraisalPageCount;
                    rep[iVar12 * 0x13 + 10] = iVar13;
                    rep[iVar12 * 0x13 + 11] = passflag;
                    rep[iVar12 * 0x13 + 12] = rand() % 5;
                LAB_004463e9:
                    tmp8 = iVar12 * 0x4c;
                    uVar5 = GetString(0x13e);
                    iVar12 = iVar12 + 1;
                    rep[(iVar12 - 1) * 0x13 + 13] = (int)uVar5;
                    rep[(iVar12 - 1) * 0x13 + 14] = 0;
                    rep[(iVar12 - 1) * 0x13 + 15] = 0;
                    rep[(iVar12 - 1) * 0x13 + 16] = 0;
                    rep[(iVar12 - 1) * 0x13 + 17] = 0;
                    rep[(iVar12 - 1) * 0x13 + 18] = 0;
                    rep[(iVar12 - 1) * 0x13 + 19] = 0;
                } else {
                    if (uVar9 == 2) {
                        if (0x1b5 < (unsigned int)(iVar4 + 0x16)) {
                            if (iVar3 != iVar14)
                                goto LAB_004464b7;
                            AppraisalPageCount = AppraisalPageCount + 1;
                            iVar4 = ystart;
                            iVar3 = iVar12;
                        }
                        rep[iVar12 * 0x13 + 9] = AppraisalPageCount;
                        rep[iVar12 * 0x13 + 10] = iVar13;
                        rep[iVar12 * 0x13 + 11] = passflag;
                        rep[iVar12 * 0x13 + 12] = rand() % 5;
                        goto LAB_004463e9;
                    }
                    if (uVar9 == 3) {
                        if (0x1b5 < (unsigned int)(iVar4 + 0x16)) {
                            if (iVar3 != iVar14)
                                goto LAB_004464b7;
                            AppraisalPageCount = AppraisalPageCount + 1;
                            iVar4 = ystart;
                            iVar3 = iVar12;
                        }
                        rep[iVar12 * 0x13 + 9] = AppraisalPageCount;
                        rep[iVar12 * 0x13 + 10] = iVar13;
                        rep[iVar12 * 0x13 + 11] = passflag;
                        rep[iVar12 * 0x13 + 12] = rand() % 5;
                        goto LAB_004463e9;
                    }
                }
                iVar4 = iVar4 + 0x18;
            }
            if ((ReportFlags & 0x3000) != 0) {
                total = total + 1;
                iVar6 = FUN_00444380();
                passflag = (int)DAT_0066601c <= iVar6;
                if (passflag == 0) {
                    flags = flags | 0x800;
                } else {
                    passtotal = passtotal + 1;
                }
                uVar9 = ReportFlags >> 0xc & 3;
                if (uVar9 == 1) {
                    if (0x1b5 < (unsigned int)(iVar4 + 0x16)) {
                        if (iVar3 != iVar14)
                            goto LAB_004464b7;
                        AppraisalPageCount = AppraisalPageCount + 1;
                        iVar4 = ystart;
                        iVar3 = iVar12;
                    }
                    rep[iVar12 * 0x13 + 9] = AppraisalPageCount;
                    rep[iVar12 * 0x13 + 10] = iVar13;
                    rep[iVar12 * 0x13 + 11] = passflag;
                    rep[iVar12 * 0x13 + 12] = rand() % 5;
                LAB_004466af:
                    tmp8 = iVar12 * 0x4c;
                    uVar5 = GetString(0x141);
                    iVar12 = iVar12 + 1;
                    rep[(iVar12 - 1) * 0x13 + 13] = (int)uVar5;
                    rep[(iVar12 - 1) * 0x13 + 14] = 0;
                    rep[(iVar12 - 1) * 0x13 + 15] = 0;
                    rep[(iVar12 - 1) * 0x13 + 16] = 0;
                    rep[(iVar12 - 1) * 0x13 + 17] = 0;
                    rep[(iVar12 - 1) * 0x13 + 18] = 0;
                    rep[(iVar12 - 1) * 0x13 + 19] = 0;
                } else {
                    if (uVar9 == 2) {
                        if (0x1b5 < (unsigned int)(iVar4 + 0x16)) {
                            if (iVar3 != iVar14)
                                goto LAB_004464b7;
                            AppraisalPageCount = AppraisalPageCount + 1;
                            iVar4 = ystart;
                            iVar3 = iVar12;
                        }
                        rep[iVar12 * 0x13 + 9] = AppraisalPageCount;
                        rep[iVar12 * 0x13 + 10] = iVar13;
                        rep[iVar12 * 0x13 + 11] = passflag;
                        rep[iVar12 * 0x13 + 12] = rand() % 5;
                        goto LAB_004466af;
                    }
                    if (uVar9 == 3) {
                        if (0x1b5 < (unsigned int)(iVar4 + 0x16)) {
                            if (iVar3 != iVar14)
                                goto LAB_004464b7;
                            AppraisalPageCount = AppraisalPageCount + 1;
                            iVar4 = ystart;
                            iVar3 = iVar12;
                        }
                        rep[iVar12 * 0x13 + 9] = AppraisalPageCount;
                        rep[iVar12 * 0x13 + 10] = iVar13;
                        rep[iVar12 * 0x13 + 11] = passflag;
                        rep[iVar12 * 0x13 + 12] = rand() % 5;
                        goto LAB_004466af;
                    }
                }
                iVar4 = iVar4 + 0x18;
            }
            passtotacc = passtotal;
            rep[iVar14 * 0x13 + 11] = (passtotal == total);
            uVar9 = iVar4 + 0x16;
            tottotacc = total;
            goto LAB_0044672e;
        }
        total = 1;
        flatp = (int *)((int)DAT_00666020 <= out7c);
        if (flatp == (int *)0x0) {
            flags = flags | 0x10;
        }
        passtotal = (flatp != (int *)0x0);
        if ((unsigned int)(iVar6 + 0x2e) < 0x1b6) {
        LAB_004456f0:
            rep[iVar12 * 0x13 + 9] = AppraisalPageCount;
            rep[iVar12 * 0x13 + 10] = iVar13;
            rep[iVar12 * 0x13 + 11] = (int)flatp;
            rep[iVar12 * 0x13 + 12] = rand() % 5;
            uVar5 = GetString(0x134);
            iVar7 = iVar12;
            iVar12 = iVar14 + 2;
            rep[iVar7 * 0x13 + 13] = (int)uVar5;
            rep[iVar7 * 0x13 + 14] = 0;
            rep[iVar7 * 0x13 + 15] = 1;
            rep[iVar7 * 0x13 + 16] = out7c;
            rep[iVar7 * 0x13 + 17] = DAT_00666020;
            rep[iVar7 * 0x13 + 18] = DAT_00666024;
            rep[iVar7 * 0x13 + 19] = 0;
            iVar4 = iVar4 + 0x18;
            goto LAB_00445794;
        }
        if (iVar3 == iVar14) {
            AppraisalPageCount = AppraisalPageCount + 1;
            iVar4 = ystart;
            iVar3 = iVar12;
            goto LAB_004456f0;
        }
    LAB_004464b7:
        xbase = *piVar10;
        AppraisalPageCount = AppraisalPageCount + 1;
        uVar9 = wstart;
        iVar4 = ystart;
        iVar3 = iVar14;
    } while (1);
    AppraisalPageCount = AppraisalPageCount + 1;
    iVar3 = iVar14;
    goto LAB_00445422;
LAB_00446751:
    do {
        rowp = &rep[iVar12 * 0x13 + 10];
        total = 0;
        subpass = 0;
        *rowp = xbase;
        if (uVar9 < 0x1b6) {
        LAB_004467ce:
            rep[iVar12 * 0x13 + 9] = AppraisalPageCount;
            *rowp = xbase;
            rep[iVar12 * 0x13 + 11] = (int)flatp;
            rep[iVar12 * 0x13 + 12] = rand() % 5;
            uVar5 = GetString(0x144);
            iVar13 = iVar12 + 1;
            iVar6 = iVar4 + 0x18;
            rep[iVar12 * 0x13 + 13] = (int)uVar5;
            rep[iVar12 * 0x13 + 14] = 0;
            rep[iVar12 * 0x13 + 15] = 0;
            rep[iVar12 * 0x13 + 16] = 0;
            rep[iVar12 * 0x13 + 17] = 0;
            rep[iVar12 * 0x13 + 18] = 0;
            rep[iVar12 * 0x13 + 19] = 0;
            FUN_00444c70((unsigned int *)&out68, (unsigned int *)&out6c);
            if ((ReportFlags & 0x8000000) == 0) {
            LAB_004469ab:
                if ((ReportFlags & 0x10000000) != 0) {
                    total = total + 1;
                    flatp = (int *)((int)DAT_00666078 <= out6c);
                    if (flatp == (int *)0x0) {
                        flags = flags | 0x2000;
                    } else {
                        subpass = subpass + 1;
                    }
                    if (0x1b5 < (unsigned int)(iVar6 + 0x16)) {
                        if (iVar3 != iVar12)
                            goto LAB_00446a1c;
                        AppraisalPageCount = AppraisalPageCount + 1;
                        iVar6 = ystart;
                        iVar3 = iVar13;
                    }
                    rep[iVar13 * 0x13 + 9] = AppraisalPageCount;
                    rep[iVar13 * 0x13 + 10] = xbase + 0x30;
                    rep[iVar13 * 0x13 + 11] = (int)flatp;
                    rep[iVar13 * 0x13 + 12] = rand() % 5;
                    uVar5 = GetString(0x132);
                    iVar13 = iVar13 + 1;
                    rep[(iVar13 - 1) * 0x13 + 13] = (int)uVar5;
                    rep[(iVar13 - 1) * 0x13 + 14] = 0;
                    rep[(iVar13 - 1) * 0x13 + 15] = 1;
                    rep[(iVar13 - 1) * 0x13 + 16] = out6c;
                    rep[(iVar13 - 1) * 0x13 + 17] = DAT_00666078;
                    rep[(iVar13 - 1) * 0x13 + 18] = DAT_0066607c;
                    rep[(iVar13 - 1) * 0x13 + 19] = 0;
                    iVar6 = iVar6 + 0x18;
                }
                rep[iVar12 * 0x13 + 11] = (subpass == total);
                tottotacc = tottotacc + total;
                passtotacc = passtotacc + subpass;
                uVar9 = iVar6 + 0x16;
                iVar12 = iVar13;
                iVar4 = iVar6;
                break;
            }
            total = 1;
            flatp = (int *)((int)DAT_00666070 <= out70);
            if (flatp == (int *)0x0) {
                flags = flags | 0x1000;
            }
            subpass = (flatp != (int *)0x0);
            if ((unsigned int)(iVar4 + 0x2e) < 0x1b6) {
            LAB_00446910:
                rep[(iVar12 + 1) * 0x13 + 9] = AppraisalPageCount;
                rep[(iVar12 + 1) * 0x13 + 10] = xbase + 0x30;
                rep[(iVar12 + 1) * 0x13 + 11] = (int)flatp;
                rep[(iVar12 + 1) * 0x13 + 12] = rand() % 5;
                uVar5 = GetString(0x133);
                iVar13 = iVar12 + 2;
                rep[(iVar12 + 1) * 0x13 + 13] = (int)uVar5;
                rep[(iVar12 + 1) * 0x13 + 14] = 0;
                rep[(iVar12 + 1) * 0x13 + 15] = 1;
                rep[(iVar12 + 1) * 0x13 + 16] = out70;
                rep[(iVar12 + 1) * 0x13 + 17] = DAT_00666070;
                rep[(iVar12 + 1) * 0x13 + 18] = DAT_00666074;
                rep[(iVar12 + 1) * 0x13 + 19] = 0;
                iVar6 = iVar6 + 0x18;
                goto LAB_004469ab;
            }
            if (iVar3 == iVar12) {
                AppraisalPageCount = AppraisalPageCount + 1;
                iVar6 = ystart;
                iVar3 = iVar13;
                goto LAB_00446910;
            }
        LAB_00446a1c:
            xbase = *rowp;
        } else if (iVar3 == iVar12) {
            AppraisalPageCount = AppraisalPageCount + 1;
            iVar4 = ystart;
            iVar3 = iVar12;
            goto LAB_004467ce;
        }
        AppraisalPageCount = AppraisalPageCount + 1;
        uVar9 = wstart;
        iVar4 = ystart;
        iVar3 = iVar12;
    } while ((ReportFlags & 0x38000000) != 0);
LAB_00446b71:
    if ((ReportFlags & 0xc0000000) != 0) {
        do {
            rowp = &rep[iVar12 * 0x13 + 10];
            total = 0;
            subpass = 0;
            *rowp = xbase;
            if (uVar9 < 0x1b6) {
            LAB_00446c11:
                rep[iVar12 * 0x13 + 9] = AppraisalPageCount;
                *rowp = xbase;
                rep[iVar12 * 0x13 + 11] = (int)flatp;
                rep[iVar12 * 0x13 + 12] = rand() % 5;
                uVar5 = GetString(0x145);
                iVar13 = iVar12 + 1;
                iVar6 = iVar4 + 0x18;
                rep[iVar12 * 0x13 + 13] = (int)uVar5;
                rep[iVar12 * 0x13 + 14] = 0;
                rep[iVar12 * 0x13 + 15] = 0;
                rep[iVar12 * 0x13 + 16] = 0;
                rep[iVar12 * 0x13 + 17] = 0;
                rep[iVar12 * 0x13 + 18] = 0;
                rep[iVar12 * 0x13 + 19] = 0;
                FUN_00444cd0((unsigned int *)&out68, (unsigned int *)&out6c);
                if ((ReportFlags & 0x40000000) == 0) {
                LAB_00446deb:
                    if ((ReportFlags & 0x80000000) != 0) {
                        total = total + 1;
                        flatp = (int *)((int)DAT_00666090 <= out68);
                        if (flatp == (int *)0x0) {
                            flags = flags | 0x10000;
                        } else {
                            subpass = subpass + 1;
                        }
                        if (0x1b5 < (unsigned int)(iVar6 + 0x16)) {
                            if (iVar3 != iVar12)
                                goto LAB_00446e59;
                            AppraisalPageCount = AppraisalPageCount + 1;
                            iVar6 = ystart;
                            iVar3 = iVar13;
                        }
                        rep[iVar13 * 0x13 + 9] = AppraisalPageCount;
                        rep[iVar13 * 0x13 + 10] = xbase + 0x30;
                        rep[iVar13 * 0x13 + 11] = (int)flatp;
                        rep[iVar13 * 0x13 + 12] = rand() % 5;
                        uVar5 = GetString(0x132);
                        iVar13 = iVar13 + 1;
                        rep[(iVar13 - 1) * 0x13 + 13] = (int)uVar5;
                        rep[(iVar13 - 1) * 0x13 + 14] = 0;
                        rep[(iVar13 - 1) * 0x13 + 15] = 1;
                        rep[(iVar13 - 1) * 0x13 + 16] = out68;
                        rep[(iVar13 - 1) * 0x13 + 17] = DAT_00666090;
                        rep[(iVar13 - 1) * 0x13 + 18] = DAT_00666094;
                        rep[(iVar13 - 1) * 0x13 + 19] = 0;
                        iVar6 = iVar6 + 0x18;
                    }
                    rep[iVar12 * 0x13 + 11] = (subpass == total);
                    tottotacc = tottotacc + total;
                    passtotacc = passtotacc + subpass;
                    uVar9 = iVar6 + 0x16;
                    iVar12 = iVar13;
                    iVar4 = iVar6;
                    break;
                }
                total = 1;
                flatp = (int *)((int)DAT_00666088 <= out74);
                if (flatp == (int *)0x0) {
                    flags = flags | 0x8000;
                }
                subpass = (flatp != (int *)0x0);
                if ((unsigned int)(iVar4 + 0x2e) < 0x1b6) {
                LAB_00446d50:
                    rep[(iVar12 + 1) * 0x13 + 9] = AppraisalPageCount;
                    rep[(iVar12 + 1) * 0x13 + 10] = xbase + 0x30;
                    rep[(iVar12 + 1) * 0x13 + 11] = (int)flatp;
                    rep[(iVar12 + 1) * 0x13 + 12] = rand() % 5;
                    uVar5 = GetString(0x133);
                    iVar13 = iVar12 + 2;
                    rep[(iVar12 + 1) * 0x13 + 13] = (int)uVar5;
                    rep[(iVar12 + 1) * 0x13 + 14] = 0;
                    rep[(iVar12 + 1) * 0x13 + 15] = 1;
                    rep[(iVar12 + 1) * 0x13 + 16] = out74;
                    rep[(iVar12 + 1) * 0x13 + 17] = DAT_00666088;
                    rep[(iVar12 + 1) * 0x13 + 18] = DAT_0066608c;
                    rep[(iVar12 + 1) * 0x13 + 19] = 0;
                    iVar6 = iVar6 + 0x18;
                    goto LAB_00446deb;
                }
                if (iVar3 == iVar12) {
                    AppraisalPageCount = AppraisalPageCount + 1;
                    iVar6 = ystart;
                    iVar3 = iVar13;
                    goto LAB_00446d50;
                }
            LAB_00446e59:
                xbase = *rowp;
            } else if (iVar3 == iVar12) {
                AppraisalPageCount = AppraisalPageCount + 1;
                iVar4 = ystart;
                iVar3 = iVar12;
                goto LAB_00446c11;
            }
            AppraisalPageCount = AppraisalPageCount + 1;
            uVar9 = wstart;
            iVar4 = ystart;
            iVar3 = iVar12;
        } while ((ReportFlags & 0xc0000000) != 0);
    }
    if ((ReportFlags & 0x30000) != 0) {
        do {
            rowp = &rep[iVar12 * 0x13 + 10];
            total = 0;
            subpass = 0;
            *rowp = xbase;
            if (uVar9 < 0x1b6) {
            LAB_0044704b:
                rep[iVar12 * 0x13 + 9] = AppraisalPageCount;
                *rowp = xbase;
                rep[iVar12 * 0x13 + 11] = (int)flatp;
                rep[iVar12 * 0x13 + 12] = rand() % 5;
                uVar5 = GetString(0x146);
                iVar13 = iVar12 + 1;
                iVar6 = iVar4 + 0x18;
                rep[iVar12 * 0x13 + 13] = (int)uVar5;
                rep[iVar12 * 0x13 + 14] = 0;
                rep[iVar12 * 0x13 + 15] = 0;
                rep[iVar12 * 0x13 + 16] = 0;
                rep[iVar12 * 0x13 + 17] = 0;
                rep[iVar12 * 0x13 + 18] = 0;
                rep[iVar12 * 0x13 + 19] = 0;
                FUN_00444d20((unsigned int *)&out60, (unsigned int *)&out6c);
                if ((ReportFlags & 0x10000) == 0) {
                LAB_00447222:
                    if ((ReportFlags & 0x20000) != 0) {
                        total = total + 1;
                        flatp = (int *)((int)DAT_00666048 <= out60);
                        if (flatp == (int *)0x0) {
                            flags = flags | 0x40000;
                        } else {
                            subpass = subpass + 1;
                        }
                        if (0x1b5 < (unsigned int)(iVar6 + 0x16)) {
                            if (iVar3 != iVar12)
                                goto LAB_00447290;
                            AppraisalPageCount = AppraisalPageCount + 1;
                            iVar6 = ystart;
                            iVar3 = iVar13;
                        }
                        rep[iVar13 * 0x13 + 9] = AppraisalPageCount;
                        rep[iVar13 * 0x13 + 10] = xbase + 0x30;
                        rep[iVar13 * 0x13 + 11] = (int)flatp;
                        rep[iVar13 * 0x13 + 12] = rand() % 5;
                        uVar5 = GetString(0x132);
                        iVar13 = iVar13 + 1;
                        rep[(iVar13 - 1) * 0x13 + 13] = (int)uVar5;
                        rep[(iVar13 - 1) * 0x13 + 14] = 0;
                        rep[(iVar13 - 1) * 0x13 + 15] = 1;
                        rep[(iVar13 - 1) * 0x13 + 16] = out60;
                        rep[(iVar13 - 1) * 0x13 + 17] = DAT_00666048;
                        rep[(iVar13 - 1) * 0x13 + 18] = DAT_0066604c;
                        rep[(iVar13 - 1) * 0x13 + 19] = 0;
                        iVar6 = iVar6 + 0x18;
                    }
                    rep[iVar12 * 0x13 + 11] = (subpass == total);
                    tottotacc = tottotacc + total;
                    passtotacc = passtotacc + subpass;
                    uVar9 = iVar6 + 0x16;
                    iVar12 = iVar13;
                    iVar4 = iVar6;
                    break;
                }
                total = 1;
                flatp = (int *)((int)DAT_00666040 <= out78);
                if (flatp == (int *)0x0) {
                    flags = flags | 0x20000;
                }
                subpass = (flatp != (int *)0x0);
                if ((unsigned int)(iVar4 + 0x2e) < 0x1b6) {
                LAB_00447187:
                    rep[(iVar12 + 1) * 0x13 + 9] = AppraisalPageCount;
                    rep[(iVar12 + 1) * 0x13 + 10] = xbase + 0x30;
                    rep[(iVar12 + 1) * 0x13 + 11] = (int)flatp;
                    rep[(iVar12 + 1) * 0x13 + 12] = rand() % 5;
                    uVar5 = GetString(0x133);
                    iVar13 = iVar12 + 2;
                    rep[(iVar12 + 1) * 0x13 + 13] = (int)uVar5;
                    rep[(iVar12 + 1) * 0x13 + 14] = 0;
                    rep[(iVar12 + 1) * 0x13 + 15] = 1;
                    rep[(iVar12 + 1) * 0x13 + 16] = out78;
                    rep[(iVar12 + 1) * 0x13 + 17] = DAT_00666040;
                    rep[(iVar12 + 1) * 0x13 + 18] = DAT_00666044;
                    rep[(iVar12 + 1) * 0x13 + 19] = 0;
                    iVar6 = iVar6 + 0x18;
                    goto LAB_00447222;
                }
                if (iVar3 == iVar12) {
                    AppraisalPageCount = AppraisalPageCount + 1;
                    iVar6 = ystart;
                    iVar3 = iVar13;
                    goto LAB_00447187;
                }
            LAB_00447290:
                xbase = *rowp;
            } else if (iVar3 == iVar12) {
                AppraisalPageCount = AppraisalPageCount + 1;
                iVar4 = ystart;
                iVar3 = iVar12;
                goto LAB_0044704b;
            }
            AppraisalPageCount = AppraisalPageCount + 1;
            uVar9 = wstart;
            iVar4 = ystart;
            iVar3 = iVar12;
        } while ((ReportFlags & 0x30000) != 0);
    }
    if ((ReportFlags & 0x5080000) != 0) {
        do {
            rowp = &rep[iVar12 * 0x13 + 10];
            total = 0;
            subpass = 0;
            *rowp = xbase;
            if (uVar9 < 0x1b6) {
            LAB_00447482:
                rep[iVar12 * 0x13 + 9] = AppraisalPageCount;
                *rowp = xbase;
                rep[iVar12 * 0x13 + 11] = (int)flatp;
                rep[iVar12 * 0x13 + 12] = rand() % 5;
                uVar5 = GetString(0x147);
                iVar13 = iVar12 + 1;
                iVar6 = iVar4 + 0x18;
                rep[iVar12 * 0x13 + 13] = (int)uVar5;
                rep[iVar12 * 0x13 + 14] = 0;
                rep[iVar12 * 0x13 + 15] = 0;
                rep[iVar12 * 0x13 + 16] = 0;
                rep[iVar12 * 0x13 + 17] = 0;
                rep[iVar12 * 0x13 + 18] = 0;
                rep[iVar12 * 0x13 + 19] = 0;
                FUN_00444d70((unsigned int *)&out58, (unsigned int *)&out5c, &out64);
                if ((ReportFlags & 0x80000) != 0) {
                    total = 1;
                    flatp = (int *)((int)DAT_00666038 <= out80);
                    if (flatp == (int *)0x0) {
                        flags = flags | 0x80000;
                    }
                    subpass = (flatp != (int *)0x0);
                    iVar6 = iVar4 + 0x30;
                }
                if ((ReportFlags & 0x1000000) == 0) {
                LAB_004476a1:
                    if ((ReportFlags & 0x4000000) != 0) {
                        total = total + 1;
                        flatp = (int *)((int)DAT_00666050 <= out5c);
                        if (flatp == (int *)0x0) {
                            flags = flags | 0x200000;
                        } else {
                            subpass = subpass + 1;
                        }
                        if (0x1b5 < (unsigned int)(iVar6 + 0x16)) {
                            if (iVar3 != iVar12)
                                goto LAB_0044770b;
                            AppraisalPageCount = AppraisalPageCount + 1;
                            iVar6 = ystart;
                            iVar3 = iVar13;
                        }
                        rep[iVar13 * 0x13 + 9] = AppraisalPageCount;
                        rep[iVar13 * 0x13 + 10] = xbase + 0x30;
                        rep[iVar13 * 0x13 + 11] = (int)flatp;
                        rep[iVar13 * 0x13 + 12] = rand() % 5;
                        uVar5 = GetString(0x149);
                        iVar13 = iVar13 + 1;
                        rep[(iVar13 - 1) * 0x13 + 13] = (int)uVar5;
                        rep[(iVar13 - 1) * 0x13 + 14] = 0;
                        rep[(iVar13 - 1) * 0x13 + 15] = 1;
                        rep[(iVar13 - 1) * 0x13 + 16] = out5c;
                        rep[(iVar13 - 1) * 0x13 + 17] = DAT_00666050;
                        rep[(iVar13 - 1) * 0x13 + 18] = DAT_00666054;
                        rep[(iVar13 - 1) * 0x13 + 19] = 0;
                        iVar6 = iVar6 + 0x18;
                    }
                    rep[iVar12 * 0x13 + 11] = (subpass == total);
                    tottotacc = tottotacc + total;
                    passtotacc = passtotacc + subpass;
                    uVar9 = iVar6 + 0x16;
                    iVar12 = iVar13;
                    iVar4 = iVar6;
                    break;
                }
                total = total + 1;
                flatp = (int *)((int)DAT_00666050 <= out64);
                if (flatp == (int *)0x0) {
                    flags = flags | 0x100000;
                } else {
                    subpass = subpass + 1;
                }
                if ((unsigned int)(iVar6 + 0x16) < 0x1b6) {
                LAB_00447603:
                    rep[(iVar12 + 1) * 0x13 + 9] = AppraisalPageCount;
                    rep[(iVar12 + 1) * 0x13 + 10] = xbase + 0x30;
                    rep[(iVar12 + 1) * 0x13 + 11] = (int)flatp;
                    rep[(iVar12 + 1) * 0x13 + 12] = rand() % 5;
                    uVar5 = GetString(0x148);
                    iVar13 = iVar12 + 2;
                    rep[(iVar12 + 1) * 0x13 + 13] = (int)uVar5;
                    rep[(iVar12 + 1) * 0x13 + 14] = 0;
                    rep[(iVar12 + 1) * 0x13 + 15] = 1;
                    rep[(iVar12 + 1) * 0x13 + 16] = out64;
                    rep[(iVar12 + 1) * 0x13 + 17] = DAT_00666050;
                    rep[(iVar12 + 1) * 0x13 + 18] = DAT_00666054;
                    rep[(iVar12 + 1) * 0x13 + 19] = 0;
                    iVar6 = iVar6 + 0x18;
                    goto LAB_004476a1;
                }
                if (iVar3 == iVar12) {
                    AppraisalPageCount = AppraisalPageCount + 1;
                    iVar6 = ystart;
                    iVar3 = iVar13;
                    goto LAB_00447603;
                }
            LAB_0044770b:
                xbase = *rowp;
            } else if (iVar3 == iVar12) {
                AppraisalPageCount = AppraisalPageCount + 1;
                iVar4 = ystart;
                iVar3 = iVar12;
                goto LAB_00447482;
            }
            AppraisalPageCount = AppraisalPageCount + 1;
            uVar9 = wstart;
            iVar4 = ystart;
            iVar3 = iVar12;
        } while ((ReportFlags & 0x5080000) != 0);
    }
    if ((ReportFlags & 0xe00000) != 0) {
        do {
            rowp = &rep[iVar12 * 0x13 + 10];
            total = 0;
            subpass = 0;
            *rowp = xbase;
            if (uVar9 < 0x1b6) {
            LAB_004478fd:
                rep[iVar12 * 0x13 + 9] = AppraisalPageCount;
                *rowp = xbase;
                rep[iVar12 * 0x13 + 11] = (int)flatp;
                rep[iVar12 * 0x13 + 12] = rand() % 5;
                uVar5 = GetString(0x14a);
                iVar7 = iVar12 + 1;
                iVar6 = iVar4 + 0x18;
                rep[iVar12 * 0x13 + 13] = (int)uVar5;
                rep[iVar12 * 0x13 + 14] = 0;
                rep[iVar12 * 0x13 + 15] = 0;
                rep[iVar12 * 0x13 + 16] = 0;
                rep[iVar12 * 0x13 + 17] = 0;
                rep[iVar12 * 0x13 + 18] = 0;
                passtotal = (iVar12 + 1) * 0x13;
                rep[iVar12 * 0x13 + 19] = 0;
                iVar13 = xbase + 0x30;
                if ((ReportFlags & 0x200000) == 0) {
                LAB_00447b1b:
                    if ((ReportFlags & 0x400000) != 0) {
                        tmp8 = 0;
                        piVar10 = (int *)GetFirstRenderObject();
                        while (piVar10 != (int *)0x0) {
                            coord = *(short *)(*(int *)(*piVar10 + 0xc) + 0x20);
                            if (((coord != 0) && (coord != 2)) && (iVar4 = FUN_0044f360(*(int *)(*piVar10 + 0xc), &coord), iVar4 != 0)) {
                                tmp8 = tmp8 + 1;
                            }
                            piVar10 = (int *)GetNextRenderObject((MapElement *)piVar10);
                        }
                        total = total + 1;
                        flatp = (int *)((int)DAT_00666060 <= tmp8);
                        if (flatp == (int *)0x0) {
                            flags = flags | 0x800000;
                        } else {
                            subpass = subpass + 1;
                        }
                        if (0x1b5 < iVar6 + 0x16) {
                            if (iVar3 != iVar12)
                                goto LAB_00447d21;
                            AppraisalPageCount = AppraisalPageCount + 1;
                            iVar6 = ystart;
                            iVar3 = iVar7;
                        }
                        rep[passtotal + 9] = AppraisalPageCount;
                        rep[passtotal + 10] = iVar13;
                        rep[passtotal + 11] = (int)flatp;
                        rep[passtotal + 12] = rand() % 5;
                        uVar5 = GetString(0x14c);
                        iVar7 = iVar7 + 1;
                        rep[passtotal + 13] = (int)uVar5;
                        rep[passtotal + 14] = 0;
                        rep[passtotal + 15] = 1;
                        rep[passtotal + 16] = tmp8;
                        rep[passtotal + 17] = DAT_00666060;
                        rep[passtotal + 18] = DAT_00666064;
                        rep[passtotal + 19] = 0;
                        iVar6 = iVar6 + 0x18;
                    }
                    if ((ReportFlags & 0x800000) != 0) {
                        iVar4 = GetMapTileCount();
                        total = total + 1;
                        flatp = (int *)((int)DAT_00666068 <= iVar4);
                        if (flatp == (int *)0x0) {
                            flags = flags | 0x1000000;
                        } else {
                            subpass = subpass + 1;
                        }
                        if (0x1b5 < iVar6 + 0x16) {
                            if (iVar3 != iVar12)
                                goto LAB_00447d21;
                            AppraisalPageCount = AppraisalPageCount + 1;
                            iVar6 = ystart;
                            iVar3 = iVar7;
                        }
                        rep[iVar7 * 0x13 + 9] = AppraisalPageCount;
                        rep[iVar7 * 0x13 + 10] = iVar13;
                        rep[iVar7 * 0x13 + 11] = (int)flatp;
                        rep[iVar7 * 0x13 + 12] = rand() % 5;
                        uVar5 = GetString(0x14d);
                        iVar7 = iVar7 + 1;
                        rep[(iVar7 - 1) * 0x13 + 13] = (int)uVar5;
                        rep[(iVar7 - 1) * 0x13 + 14] = 0;
                        rep[(iVar7 - 1) * 0x13 + 15] = 1;
                        rep[(iVar7 - 1) * 0x13 + 16] = iVar4;
                        rep[(iVar7 - 1) * 0x13 + 17] = DAT_00666068;
                        rep[(iVar7 - 1) * 0x13 + 18] = DAT_0066606c;
                        rep[(iVar7 - 1) * 0x13 + 19] = 0;
                        iVar6 = iVar6 + 0x18;
                    }
                    rep[iVar12 * 0x13 + 11] = (subpass == total);
                    tottotacc = tottotacc + total;
                    passtotacc = passtotacc + subpass;
                    uVar9 = iVar6 + 0x16;
                    iVar12 = iVar7;
                    iVar4 = iVar6;
                    break;
                }
                total = 1;
                flatp = (int *)((int)DAT_00666058 <= (int)MapStats.power_supply);
                if (flatp == (int *)0x0) {
                    flags = flags | 0x400000;
                }
                subpass = (flatp != (int *)0x0);
                if ((unsigned int)(iVar4 + 0x2e) < 0x1b6) {
                LAB_00447a7a:
                    rep[(iVar12 + 1) * 0x13 + 9] = AppraisalPageCount;
                    rep[(iVar12 + 1) * 0x13 + 10] = iVar13;
                    rep[(iVar12 + 1) * 0x13 + 11] = (int)flatp;
                    rep[(iVar12 + 1) * 0x13 + 12] = rand() % 5;
                    uVar5 = GetString(0x14b);
                    iVar7 = iVar12 + 2;
                    rep[(iVar12 + 1) * 0x13 + 13] = (int)uVar5;
                    rep[(iVar12 + 1) * 0x13 + 14] = 0;
                    rep[(iVar12 + 1) * 0x13 + 15] = 1;
                    rep[(iVar12 + 1) * 0x13 + 16] = MapStats.power_supply;
                    rep[(iVar12 + 1) * 0x13 + 17] = DAT_00666058;
                    rep[(iVar12 + 1) * 0x13 + 18] = DAT_0066605c;
                    rep[(iVar12 + 1) * 0x13 + 19] = 0;
                    passtotal = (iVar12 + 2) * 0x13;
                    iVar6 = iVar6 + 0x18;
                    goto LAB_00447b1b;
                }
                if (iVar3 == iVar12) {
                    AppraisalPageCount = AppraisalPageCount + 1;
                    iVar6 = ystart;
                    iVar3 = iVar7;
                    goto LAB_00447a7a;
                }
            LAB_00447d21:
                xbase = *rowp;
            } else if (iVar3 == iVar12) {
                AppraisalPageCount = AppraisalPageCount + 1;
                iVar4 = ystart;
                iVar3 = iVar12;
                goto LAB_004478fd;
            }
            AppraisalPageCount = AppraisalPageCount + 1;
            uVar9 = wstart;
            iVar4 = ystart;
            iVar3 = iVar12;
        } while ((ReportFlags & 0xe00000) != 0);
    }
LAB_00447e73:
    do {
        iVar14 = iVar12 * 0x4c;
        piVar10 = (int *)(((char *)rep + 0x28) + iVar14);
        *piVar10 = xbase;
        if ((int)uVar9 < 0x1b6) {
        LAB_00447ef2:
            *(int *)((char *)rep + 0x24 + iVar14) = AppraisalPageCount;
            *piVar10 = xbase;
            *(int *)((char *)rep + 0x2c + iVar14) = 0xfffffffe;
            iVar6 = rand();
            *(int *)((char *)rep + 0x30 + iVar14) = iVar6 % 5;
            uVar5 = GetString(0x230);
            iVar6 = iVar4 + 0x18;
            iVar7 = iVar12 + 1;
            *(int *)((char *)rep + 0x34 + iVar14) = (int)uVar5;
            iVar13 = xbase + 0x30;
            *(int *)((char *)rep + 0x38 + iVar14) = 0;
            *(int *)((char *)rep + 0x3c + iVar14) = 0;
            *(int *)((char *)rep + 0x40 + iVar14) = 0;
            *(int *)((char *)rep + 0x44 + iVar14) = 0;
            *(int *)((char *)rep + 0x48 + iVar14) = 0;
            *(int *)((char *)rep + 0x4c + iVar14) = 0;
            iVar4 = iVar4 + 0x2e;
            if (passtotacc < tottotacc / 2) {
                if (0x1b5 < iVar4) {
                    if (iVar3 != iVar12) goto LAB_0044a60b;
                    AppraisalPageCount = AppraisalPageCount + 1;
                    iVar6 = ystart;
                    iVar3 = iVar7;
                }
                iVar7 = iVar7 * 0x4c;
                *(int *)((char *)rep + 0x24 + iVar7) = AppraisalPageCount;
                *(int *)((char *)rep + 0x28 + iVar7) = iVar13;
                *(int *)((char *)rep + 0x2c + iVar7) = 0xfffffffe;
                iVar4 = rand();
                *(int *)((char *)rep + 0x30 + iVar7) = iVar4 % 5;
                uVar5 = GetString(0x14f);
                iVar8 = iVar12 + 2;
                *(int *)((char *)rep + 0x34 + iVar7) = (int)uVar5;
                *(int *)((char *)rep + 0x38 + iVar7) = 0;
                *(int *)((char *)rep + 0x3c + iVar7) = 0;
                *(int *)((char *)rep + 0x40 + iVar7) = 0;
                *(int *)((char *)rep + 0x44 + iVar7) = 0;
                *(int *)((char *)rep + 0x48 + iVar7) = 0;
                *(int *)((char *)rep + 0x4c + iVar7) = 0;
                iVar14 = iVar8 * 0x4c;
                (rep + 1)[iVar8 * 0x13 + rep[iVar8 * 0x13]] = 0x14f;
                rep[iVar8 * 0x13] = rep[iVar8 * 0x13] + 1;
                iVar4 = iVar6 + 0x18;
                if (0x1b5 < iVar6 + 0x2e) {
                    if (iVar3 != iVar12) goto LAB_0044a60b;
                    AppraisalPageCount = AppraisalPageCount + 1;
                    iVar4 = ystart;
                    iVar3 = iVar8;
                }
                *(int *)((char *)rep + 0x24 + iVar14) = AppraisalPageCount;
                *(int *)((char *)rep + 0x28 + iVar14) = iVar13;
                *(int *)((char *)rep + 0x2c + iVar14) = 0xfffffffe;
                iVar6 = rand();
                *(int *)((char *)rep + 0x30 + iVar14) = iVar6 % 5;
                uVar5 = GetString(0x150);
                *(int *)((char *)rep + 0x34 + iVar14) = (int)uVar5;
                *(int *)((char *)rep + 0x38 + iVar14) = 0;
                *(int *)((char *)rep + 0x3c + iVar14) = 0;
                *(int *)((char *)rep + 0x40 + iVar14) = 0;
                *(int *)((char *)rep + 0x44 + iVar14) = 0;
                *(int *)((char *)rep + 0x48 + iVar14) = 0;
                *(int *)((char *)rep + 0x4c + iVar14) = 0;
                piVar11 = rep + (iVar12 + 3) * 0x13;
                (rep + 1)[(iVar12 + 3) * 0x13 + *piVar11] = 0x150;
                *piVar11 = *piVar11 + 1;
                iVar14 = iVar12 + 3;
                iVar6 = iVar14 * 0x4c;
                *(int *)((char *)rep + 0x24 + iVar6) = AppraisalPageCount;
                *(int *)((char *)rep + 0x28 + iVar6) = iVar13;
                *(int *)((char *)rep + 0x2c + iVar6) = 0xfffffffe;
                iVar7 = rand();
                *(int *)((char *)rep + 0x30 + iVar6) = iVar7 % 5;
                uVar5 = GetString(0x151);
                *(int *)((char *)rep + 0x34 + iVar6) = (int)uVar5;
                *(int *)((char *)rep + 0x38 + iVar6) = 0;
                *(int *)((char *)rep + 0x3c + iVar6) = 0;
                *(int *)((char *)rep + 0x40 + iVar6) = 0;
                *(int *)((char *)rep + 0x44 + iVar6) = 0;
                *(int *)((char *)rep + 0x48 + iVar6) = 0;
                *(int *)((char *)rep + 0x4c + iVar6) = 0;
            LAB_0044855b:
                ysave = iVar4 + 0x2e;
                iVar4 = iVar4 + 0x18;
                iVar14 = iVar12 + 3;
                iVar6 = iVar4;
                if ((flags & 1) != 0) {
                    if (0x1b5 < ysave) {
                        if (iVar3 != iVar12) goto LAB_0044a60b;
                        AppraisalPageCount = AppraisalPageCount + 1;
                        iVar6 = ystart;
                        iVar3 = iVar14;
                    }
                    iVar4 = iVar14 * 0x4c;
                    *(int *)((char *)rep + 0x24 + iVar4) = AppraisalPageCount;
                    *(int *)((char *)rep + 0x28 + iVar4) = iVar13;
                    *(int *)((char *)rep + 0x2c + iVar4) = 0xffffffff;
                    iVar7 = rand();
                    *(int *)((char *)rep + 0x30 + iVar4) = iVar7 % 5;
                    uVar5 = GetString(0x155);
                    iVar14 = iVar14 + 1;
                    *(int *)((char *)rep + 0x34 + iVar4) = (int)uVar5;
                    *(int *)((char *)rep + 0x38 + iVar4) = 0;
                    *(int *)((char *)rep + 0x3c + iVar4) = 0;
                    *(int *)((char *)rep + 0x40 + iVar4) = 0;
                    *(int *)((char *)rep + 0x44 + iVar4) = 0;
                    *(int *)((char *)rep + 0x48 + iVar4) = 0;
                    *(int *)((char *)rep + 0x4c + iVar4) = 0;
                    piVar11 = rep + iVar14 * 0x13;
                    (rep + 1)[iVar14 * 0x13 + *piVar11] = 0x155;
                    *piVar11 = *piVar11 + 1;
                    iVar4 = iVar6 + 0x18;
                    ysave = iVar6 + 0x2e;
                }
                iVar6 = iVar4;
                if ((flags & 2) != 0) {
                    if (0x1b5 < ysave) {
                        if (iVar3 != iVar12) goto LAB_0044a60b;
                        AppraisalPageCount = AppraisalPageCount + 1;
                        iVar6 = ystart;
                        iVar3 = iVar14;
                    }
                    iVar4 = iVar14 * 0x4c;
                    *(int *)((char *)rep + 0x24 + iVar4) = AppraisalPageCount;
                    *(int *)((char *)rep + 0x28 + iVar4) = iVar13;
                    *(int *)((char *)rep + 0x2c + iVar4) = 0xffffffff;
                    iVar7 = rand();
                    *(int *)((char *)rep + 0x30 + iVar4) = iVar7 % 5;
                    uVar5 = GetString(0x156);
                    iVar14 = iVar14 + 1;
                    *(int *)((char *)rep + 0x34 + iVar4) = (int)uVar5;
                    *(int *)((char *)rep + 0x38 + iVar4) = 0;
                    *(int *)((char *)rep + 0x3c + iVar4) = 0;
                    *(int *)((char *)rep + 0x40 + iVar4) = 0;
                    *(int *)((char *)rep + 0x44 + iVar4) = 0;
                    *(int *)((char *)rep + 0x48 + iVar4) = 0;
                    *(int *)((char *)rep + 0x4c + iVar4) = 0;
                    piVar11 = rep + iVar14 * 0x13;
                    (rep + 1)[iVar14 * 0x13 + *piVar11] = 0x156;
                    *piVar11 = *piVar11 + 1;
                    iVar4 = iVar6 + 0x18;
                    ysave = iVar6 + 0x2e;
                }
                iVar6 = iVar4;
                if ((flags & 4) != 0) {
                    if (0x1b5 < ysave) {
                        if (iVar3 != iVar12) goto LAB_0044a60b;
                        AppraisalPageCount = AppraisalPageCount + 1;
                        iVar6 = ystart;
                        iVar3 = iVar14;
                    }
                    iVar4 = iVar14 * 0x4c;
                    *(int *)((char *)rep + 0x24 + iVar4) = AppraisalPageCount;
                    *(int *)((char *)rep + 0x28 + iVar4) = iVar13;
                    *(int *)((char *)rep + 0x2c + iVar4) = 0xffffffff;
                    iVar7 = rand();
                    *(int *)((char *)rep + 0x30 + iVar4) = iVar7 % 5;
                    uVar5 = GetString(0x157);
                    iVar14 = iVar14 + 1;
                    *(int *)((char *)rep + 0x34 + iVar4) = (int)uVar5;
                    *(int *)((char *)rep + 0x38 + iVar4) = 0;
                    *(int *)((char *)rep + 0x3c + iVar4) = 0;
                    *(int *)((char *)rep + 0x40 + iVar4) = 0;
                    *(int *)((char *)rep + 0x44 + iVar4) = 0;
                    *(int *)((char *)rep + 0x48 + iVar4) = 0;
                    *(int *)((char *)rep + 0x4c + iVar4) = 0;
                    piVar11 = rep + iVar14 * 0x13;
                    (rep + 1)[iVar14 * 0x13 + *piVar11] = 0x157;
                    *piVar11 = *piVar11 + 1;
                    iVar4 = iVar6 + 0x18;
                    ysave = iVar6 + 0x2e;
                }
                iVar6 = iVar4;
                if ((flags & 8) != 0) {
                    if (0x1b5 < ysave) {
                        if (iVar3 != iVar12) goto LAB_0044a60b;
                        AppraisalPageCount = AppraisalPageCount + 1;
                        iVar6 = ystart;
                        iVar3 = iVar14;
                    }
                    iVar4 = iVar14 * 0x4c;
                    *(int *)((char *)rep + 0x24 + iVar4) = AppraisalPageCount;
                    *(int *)((char *)rep + 0x28 + iVar4) = iVar13;
                    *(int *)((char *)rep + 0x2c + iVar4) = 0xffffffff;
                    iVar7 = rand();
                    *(int *)((char *)rep + 0x30 + iVar4) = iVar7 % 5;
                    uVar5 = GetString(0x158);
                    iVar14 = iVar14 + 1;
                    *(int *)((char *)rep + 0x34 + iVar4) = (int)uVar5;
                    *(int *)((char *)rep + 0x38 + iVar4) = 0;
                    *(int *)((char *)rep + 0x3c + iVar4) = 0;
                    *(int *)((char *)rep + 0x40 + iVar4) = 0;
                    *(int *)((char *)rep + 0x44 + iVar4) = 0;
                    *(int *)((char *)rep + 0x48 + iVar4) = 0;
                    *(int *)((char *)rep + 0x4c + iVar4) = 0;
                    piVar11 = rep + iVar14 * 0x13;
                    (rep + 1)[iVar14 * 0x13 + *piVar11] = 0x158;
                    *piVar11 = *piVar11 + 1;
                    iVar4 = iVar6 + 0x18;
                    ysave = iVar6 + 0x2e;
                }
                uVar9 = flags & 0x30;
                iVar6 = iVar4;
                if (uVar9 == 0x10) {
                    if (0x1b5 < ysave) {
                        if (iVar3 != iVar12) goto LAB_0044a60b;
                        AppraisalPageCount = AppraisalPageCount + 1;
                        iVar6 = ystart;
                        iVar3 = iVar14;
                    }
                    iVar4 = iVar14 * 0x4c;
                    *(int *)((char *)rep + 0x24 + iVar4) = AppraisalPageCount;
                    *(int *)((char *)rep + 0x28 + iVar4) = iVar13;
                    *(int *)((char *)rep + 0x2c + iVar4) = 0xffffffff;
                    iVar14 = rand();
                    *(int *)((char *)rep + 0x30 + iVar4) = iVar14 % 5;
                    uVar5 = GetString(0x159);
                    iVar14 = iVar12 + 4;
                    *(int *)((char *)rep + 0x34 + iVar4) = (int)uVar5;
                    *(int *)((char *)rep + 0x38 + iVar4) = 0;
                    *(int *)((char *)rep + 0x3c + iVar4) = 0;
                    *(int *)((char *)rep + 0x40 + iVar4) = 0;
                    *(int *)((char *)rep + 0x44 + iVar4) = 0;
                    *(int *)((char *)rep + 0x48 + iVar4) = 0;
                    *(int *)((char *)rep + 0x4c + iVar4) = 0;
                    piVar11 = rep + iVar14 * 0x13;
                    (rep + 1)[iVar14 * 0x13 + *piVar11] = 0x159;
                LAB_00448d96:
                    iVar4 = iVar6 + 0x18;
                    *piVar11 = *piVar11 + 1;
                    ysave = iVar6 + 0x2e;
                } else {
                    if (uVar9 == 0x20) {
                        if (0x1b5 < ysave) {
                            if (iVar3 != iVar12) goto LAB_0044a60b;
                            AppraisalPageCount = AppraisalPageCount + 1;
                            iVar6 = ystart;
                            iVar3 = iVar14;
                        }
                        iVar4 = iVar14 * 0x4c;
                        *(int *)((char *)rep + 0x24 + iVar4) = AppraisalPageCount;
                        *(int *)((char *)rep + 0x28 + iVar4) = iVar13;
                        *(int *)((char *)rep + 0x2c + iVar4) = 0xffffffff;
                        iVar14 = rand();
                        *(int *)((char *)rep + 0x30 + iVar4) = iVar14 % 5;
                        uVar5 = GetString(0x15a);
                        iVar14 = iVar12 + 4;
                        *(int *)((char *)rep + 0x34 + iVar4) = (int)uVar5;
                        *(int *)((char *)rep + 0x38 + iVar4) = 0;
                        *(int *)((char *)rep + 0x3c + iVar4) = 0;
                        *(int *)((char *)rep + 0x40 + iVar4) = 0;
                        *(int *)((char *)rep + 0x44 + iVar4) = 0;
                        *(int *)((char *)rep + 0x48 + iVar4) = 0;
                        *(int *)((char *)rep + 0x4c + iVar4) = 0;
                        piVar11 = rep + iVar14 * 0x13;
                        (rep + 1)[iVar14 * 0x13 + *piVar11] = 0x15a;
                        goto LAB_00448d96;
                    }
                    if (uVar9 == 0x30) {
                        if (0x1b5 < ysave) {
                            if (iVar3 != iVar12) goto LAB_0044a60b;
                            AppraisalPageCount = AppraisalPageCount + 1;
                            ysave = 0x83;
                            iVar4 = ystart;
                            iVar3 = iVar14;
                        }
                        iVar14 = iVar14 * 0x4c;
                        *(int *)((char *)rep + 0x24 + iVar14) = AppraisalPageCount;
                        *(int *)((char *)rep + 0x28 + iVar14) = iVar13;
                        *(int *)((char *)rep + 0x2c + iVar14) = 0xffffffff;
                        iVar6 = rand();
                        *(int *)((char *)rep + 0x30 + iVar14) = iVar6 % 5;
                        uVar5 = GetString(0x15b);
                        *(int *)((char *)rep + 0x34 + iVar14) = (int)uVar5;
                        *(int *)((char *)rep + 0x38 + iVar14) = 0;
                        *(int *)((char *)rep + 0x3c + iVar14) = 0;
                        *(int *)((char *)rep + 0x40 + iVar14) = 0;
                        *(int *)((char *)rep + 0x44 + iVar14) = 0;
                        *(int *)((char *)rep + 0x48 + iVar14) = 0;
                        *(int *)((char *)rep + 0x4c + iVar14) = 0;
                        iVar6 = iVar4;
                        if (0x1b5 < ysave) {
                            if (iVar3 != iVar12) goto LAB_0044a60b;
                            AppraisalPageCount = AppraisalPageCount + 1;
                            iVar6 = ystart;
                            iVar3 = iVar12 + 4;
                        }
                        iVar4 = (iVar12 + 4) * 0x4c;
                        *(int *)((char *)rep + 0x24 + iVar4) = AppraisalPageCount;
                        *(int *)((char *)rep + 0x28 + iVar4) = iVar13;
                        *(int *)((char *)rep + 0x2c + iVar4) = 0xfffffffd;
                        iVar14 = rand();
                        *(int *)((char *)rep + 0x30 + iVar4) = iVar14 % 5;
                        uVar5 = GetString(0x133);
                        iVar14 = iVar12 + 5;
                        *(int *)((char *)rep + 0x34 + iVar4) = (int)uVar5;
                        *(int *)((char *)rep + 0x38 + iVar4) = 0;
                        *(int *)((char *)rep + 0x3c + iVar4) = 0;
                        *(int *)((char *)rep + 0x40 + iVar4) = 0;
                        *(int *)((char *)rep + 0x44 + iVar4) = 0;
                        *(int *)((char *)rep + 0x48 + iVar4) = 0;
                        *(int *)((char *)rep + 0x4c + iVar4) = 0;
                        piVar11 = rep + iVar14 * 0x13;
                        (rep + 1)[iVar14 * 0x13 + *piVar11] = 0x15b;
                        goto LAB_00448d96;
                    }
                }
                if ((flags & 0x40) != 0) {
                    if (0x1b5 < ysave) {
                        if (iVar3 != iVar12) goto LAB_0044a60b;
                        AppraisalPageCount = AppraisalPageCount + 1;
                        iVar4 = ystart;
                        iVar3 = iVar14;
                    }
                    iVar6 = iVar14 * 0x4c;
                    *(int *)((char *)rep + 0x24 + iVar6) = AppraisalPageCount;
                    *(int *)((char *)rep + 0x28 + iVar6) = iVar13;
                    *(int *)((char *)rep + 0x2c + iVar6) = 0xffffffff;
                    iVar7 = rand();
                    *(int *)((char *)rep + 0x30 + iVar6) = iVar7 % 5;
                    uVar5 = GetString(0x15c);
                    iVar7 = iVar14 + 1;
                    *(int *)((char *)rep + 0x34 + iVar6) = (int)uVar5;
                    *(int *)((char *)rep + 0x38 + iVar6) = 0;
                    *(int *)((char *)rep + 0x3c + iVar6) = 0;
                    *(int *)((char *)rep + 0x40 + iVar6) = 0;
                    *(int *)((char *)rep + 0x44 + iVar6) = 0;
                    *(int *)((char *)rep + 0x48 + iVar6) = 0;
                    *(int *)((char *)rep + 0x4c + iVar6) = 0;
                    iVar6 = iVar7 * 0x4c;
                    (rep + 1)[iVar7 * 0x13 + rep[iVar7 * 0x13]] = 0x15c;
                    rep[iVar7 * 0x13] = rep[iVar7 * 0x13] + 1;
                    ysave = iVar4 + 0x18;
                    if (0x1b5 < iVar4 + 0x2e) {
                        if (iVar3 != iVar12) goto LAB_0044a60b;
                        AppraisalPageCount = AppraisalPageCount + 1;
                        ysave = ystart;
                        iVar3 = iVar7;
                    }
                    *(int *)((char *)rep + 0x24 + iVar6) = AppraisalPageCount;
                    *(int *)((char *)rep + 0x28 + iVar6) = iVar13;
                    *(int *)((char *)rep + 0x2c + iVar6) = 0xfffffffd;
                    iVar4 = rand();
                    *(int *)((char *)rep + 0x30 + iVar6) = iVar4 % 5;
                    uVar5 = GetString(0x15d);
                    iVar14 = iVar14 + 2;
                    iVar4 = ysave + 0x18;
                    *(int *)((char *)rep + 0x34 + iVar6) = (int)uVar5;
                    *(int *)((char *)rep + 0x38 + iVar6) = 0;
                    *(int *)((char *)rep + 0x3c + iVar6) = 0;
                    *(int *)((char *)rep + 0x40 + iVar6) = 0;
                    *(int *)((char *)rep + 0x44 + iVar6) = 0;
                    *(int *)((char *)rep + 0x48 + iVar6) = 0;
                    ysave = ysave + 0x2e;
                    *(int *)((char *)rep + 0x4c + iVar6) = 0;
                }
                if ((flags & 0x80) != 0) {
                    iVar6 = iVar4;
                    if (0x1b5 < ysave) {
                        if (iVar3 != iVar12) goto LAB_0044a60b;
                        AppraisalPageCount = AppraisalPageCount + 1;
                        iVar6 = ystart;
                        iVar3 = iVar14;
                    }
                    iVar4 = iVar14 * 0x4c;
                    *(int *)((char *)rep + 0x24 + iVar4) = AppraisalPageCount;
                    *(int *)((char *)rep + 0x28 + iVar4) = iVar13;
                    *(int *)((char *)rep + 0x2c + iVar4) = 0xffffffff;
                    iVar7 = rand();
                    *(int *)((char *)rep + 0x30 + iVar4) = iVar7 % 5;
                    uVar5 = GetString(0x15e);
                    iVar14 = iVar14 + 1;
                    *(int *)((char *)rep + 0x34 + iVar4) = (int)uVar5;
                    *(int *)((char *)rep + 0x38 + iVar4) = 0;
                    *(int *)((char *)rep + 0x3c + iVar4) = 0;
                    *(int *)((char *)rep + 0x40 + iVar4) = 0;
                    *(int *)((char *)rep + 0x44 + iVar4) = 0;
                    *(int *)((char *)rep + 0x48 + iVar4) = 0;
                    *(int *)((char *)rep + 0x4c + iVar4) = 0;
                    piVar11 = rep + iVar14 * 0x13;
                    (rep + 1)[iVar14 * 0x13 + *piVar11] = 0x15e;
                    iVar4 = iVar6 + 0x18;
                    *piVar11 = *piVar11 + 1;
                    ysave = iVar6 + 0x2e;
                }
                if ((flags & 0x100) != 0) {
                    iVar6 = iVar4;
                    if (0x1b5 < ysave) {
                        if (iVar3 != iVar12) goto LAB_0044a60b;
                        AppraisalPageCount = AppraisalPageCount + 1;
                        iVar6 = ystart;
                        iVar3 = iVar14;
                    }
                    iVar4 = iVar14 * 0x4c;
                    *(int *)((char *)rep + 0x24 + iVar4) = AppraisalPageCount;
                    *(int *)((char *)rep + 0x28 + iVar4) = iVar13;
                    *(int *)((char *)rep + 0x2c + iVar4) = 0xffffffff;
                    iVar7 = rand();
                    *(int *)((char *)rep + 0x30 + iVar4) = iVar7 % 5;
                    uVar5 = GetString(0x15f);
                    iVar14 = iVar14 + 1;
                    *(int *)((char *)rep + 0x34 + iVar4) = (int)uVar5;
                    *(int *)((char *)rep + 0x38 + iVar4) = 0;
                    *(int *)((char *)rep + 0x3c + iVar4) = 0;
                    *(int *)((char *)rep + 0x40 + iVar4) = 0;
                    *(int *)((char *)rep + 0x44 + iVar4) = 0;
                    *(int *)((char *)rep + 0x48 + iVar4) = 0;
                    *(int *)((char *)rep + 0x4c + iVar4) = 0;
                    piVar11 = rep + iVar14 * 0x13;
                    (rep + 1)[iVar14 * 0x13 + *piVar11] = 0x15f;
                    iVar4 = iVar6 + 0x18;
                    *piVar11 = *piVar11 + 1;
                    ysave = iVar6 + 0x2e;
                }
                if ((flags & 0x200) != 0) {
                    iVar6 = iVar4;
                    if (0x1b5 < ysave) {
                        if (iVar3 != iVar12) goto LAB_0044a60b;
                        AppraisalPageCount = AppraisalPageCount + 1;
                        iVar6 = ystart;
                        iVar3 = iVar14;
                    }
                    iVar4 = iVar14 * 0x4c;
                    *(int *)((char *)rep + 0x24 + iVar4) = AppraisalPageCount;
                    *(int *)((char *)rep + 0x28 + iVar4) = iVar13;
                    *(int *)((char *)rep + 0x2c + iVar4) = 0xffffffff;
                    iVar7 = rand();
                    *(int *)((char *)rep + 0x30 + iVar4) = iVar7 % 5;
                    uVar5 = GetString(0x160);
                    iVar14 = iVar14 + 1;
                    *(int *)((char *)rep + 0x34 + iVar4) = (int)uVar5;
                    *(int *)((char *)rep + 0x38 + iVar4) = 0;
                    *(int *)((char *)rep + 0x3c + iVar4) = 0;
                    *(int *)((char *)rep + 0x40 + iVar4) = 0;
                    *(int *)((char *)rep + 0x44 + iVar4) = 0;
                    *(int *)((char *)rep + 0x48 + iVar4) = 0;
                    *(int *)((char *)rep + 0x4c + iVar4) = 0;
                    piVar11 = rep + iVar14 * 0x13;
                    (rep + 1)[iVar14 * 0x13 + *piVar11] = 0x160;
                    iVar4 = iVar6 + 0x18;
                    *piVar11 = *piVar11 + 1;
                    ysave = iVar6 + 0x2e;
                }
                if ((flags & 0x400) != 0) {
                    iVar6 = iVar4;
                    if (0x1b5 < ysave) {
                        if (iVar3 != iVar12) goto LAB_0044a60b;
                        AppraisalPageCount = AppraisalPageCount + 1;
                        iVar6 = ystart;
                        iVar3 = iVar14;
                    }
                    iVar4 = iVar14 * 0x4c;
                    *(int *)((char *)rep + 0x24 + iVar4) = AppraisalPageCount;
                    *(int *)((char *)rep + 0x28 + iVar4) = iVar13;
                    *(int *)((char *)rep + 0x2c + iVar4) = 0xffffffff;
                    iVar7 = rand();
                    *(int *)((char *)rep + 0x30 + iVar4) = iVar7 % 5;
                    uVar5 = GetString(0x161);
                    iVar14 = iVar14 + 1;
                    *(int *)((char *)rep + 0x34 + iVar4) = (int)uVar5;
                    *(int *)((char *)rep + 0x38 + iVar4) = 0;
                    *(int *)((char *)rep + 0x3c + iVar4) = 0;
                    *(int *)((char *)rep + 0x40 + iVar4) = 0;
                    *(int *)((char *)rep + 0x44 + iVar4) = 0;
                    *(int *)((char *)rep + 0x48 + iVar4) = 0;
                    *(int *)((char *)rep + 0x4c + iVar4) = 0;
                    piVar11 = rep + iVar14 * 0x13;
                    (rep + 1)[iVar14 * 0x13 + *piVar11] = 0x161;
                    iVar4 = iVar6 + 0x18;
                    *piVar11 = *piVar11 + 1;
                    ysave = iVar6 + 0x2e;
                }
                if ((flags & 0x800) != 0) {
                    iVar6 = iVar4;
                    if (0x1b5 < ysave) {
                        if (iVar3 != iVar12) goto LAB_0044a60b;
                        AppraisalPageCount = AppraisalPageCount + 1;
                        iVar6 = ystart;
                        iVar3 = iVar14;
                    }
                    iVar4 = iVar14 * 0x4c;
                    *(int *)((char *)rep + 0x24 + iVar4) = AppraisalPageCount;
                    *(int *)((char *)rep + 0x28 + iVar4) = iVar13;
                    *(int *)((char *)rep + 0x2c + iVar4) = 0xffffffff;
                    iVar7 = rand();
                    *(int *)((char *)rep + 0x30 + iVar4) = iVar7 % 5;
                    uVar5 = GetString(0x162);
                    iVar14 = iVar14 + 1;
                    *(int *)((char *)rep + 0x34 + iVar4) = (int)uVar5;
                    *(int *)((char *)rep + 0x38 + iVar4) = 0;
                    *(int *)((char *)rep + 0x3c + iVar4) = 0;
                    *(int *)((char *)rep + 0x40 + iVar4) = 0;
                    *(int *)((char *)rep + 0x44 + iVar4) = 0;
                    *(int *)((char *)rep + 0x48 + iVar4) = 0;
                    *(int *)((char *)rep + 0x4c + iVar4) = 0;
                    piVar11 = rep + iVar14 * 0x13;
                    (rep + 1)[iVar14 * 0x13 + *piVar11] = 0x162;
                    iVar4 = iVar6 + 0x18;
                    *piVar11 = *piVar11 + 1;
                    ysave = iVar6 + 0x2e;
                }
                uVar9 = flags & 0x3000;
                iVar6 = iVar4;
                if (uVar9 == 0x1000) {
                    if (0x1b5 < ysave) {
                        if (iVar3 != iVar12) goto LAB_0044a60b;
                        AppraisalPageCount = AppraisalPageCount + 1;
                        iVar6 = ystart;
                        iVar3 = iVar14;
                    }
                    iVar4 = iVar14 * 0x4c;
                    *(int *)((char *)rep + 0x24 + iVar4) = AppraisalPageCount;
                    *(int *)((char *)rep + 0x28 + iVar4) = iVar13;
                    *(int *)((char *)rep + 0x2c + iVar4) = 0xffffffff;
                    iVar7 = rand();
                    *(int *)((char *)rep + 0x30 + iVar4) = iVar7 % 5;
                    uVar5 = GetString(0x163);
                    *(int *)((char *)rep + 0x34 + iVar4) = (int)uVar5;
                    *(int *)((char *)rep + 0x38 + iVar4) = 0;
                    *(int *)((char *)rep + 0x3c + iVar4) = 0;
                    *(int *)((char *)rep + 0x40 + iVar4) = 0;
                    *(int *)((char *)rep + 0x44 + iVar4) = 0;
                    *(int *)((char *)rep + 0x48 + iVar4) = 0;
                    *(int *)((char *)rep + 0x4c + iVar4) = 0;
                    piVar11 = rep + (iVar14 + 1) * 0x13;
                    (rep + 1)[(iVar14 + 1) * 0x13 + *piVar11] = 0x163;
                LAB_00449820:
                    iVar14 = iVar14 + 1;
                    iVar4 = iVar6 + 0x18;
                    *piVar11 = *piVar11 + 1;
                    ysave = iVar6 + 0x2e;
                } else {
                    if (uVar9 == 0x2000) {
                        if (0x1b5 < ysave) {
                            if (iVar3 != iVar12) goto LAB_0044a60b;
                            AppraisalPageCount = AppraisalPageCount + 1;
                            iVar6 = ystart;
                            iVar3 = iVar14;
                        }
                        iVar4 = iVar14 * 0x4c;
                        *(int *)((char *)rep + 0x24 + iVar4) = AppraisalPageCount;
                        *(int *)((char *)rep + 0x28 + iVar4) = iVar13;
                        *(int *)((char *)rep + 0x2c + iVar4) = 0xffffffff;
                        iVar7 = rand();
                        *(int *)((char *)rep + 0x30 + iVar4) = iVar7 % 5;
                        uVar5 = GetString(0x164);
                        *(int *)((char *)rep + 0x34 + iVar4) = (int)uVar5;
                        *(int *)((char *)rep + 0x38 + iVar4) = 0;
                        *(int *)((char *)rep + 0x3c + iVar4) = 0;
                        *(int *)((char *)rep + 0x40 + iVar4) = 0;
                        *(int *)((char *)rep + 0x44 + iVar4) = 0;
                        *(int *)((char *)rep + 0x48 + iVar4) = 0;
                        *(int *)((char *)rep + 0x4c + iVar4) = 0;
                        piVar11 = rep + (iVar14 + 1) * 0x13;
                        (rep + 1)[(iVar14 + 1) * 0x13 + *piVar11] = 0x164;
                        goto LAB_00449820;
                    }
                    if (uVar9 == 0x3000) {
                        if (0x1b5 < ysave) {
                            if (iVar3 != iVar12) goto LAB_0044a60b;
                            AppraisalPageCount = AppraisalPageCount + 1;
                            iVar6 = ystart;
                            iVar3 = iVar14;
                        }
                        iVar4 = iVar14 * 0x4c;
                        *(int *)((char *)rep + 0x24 + iVar4) = AppraisalPageCount;
                        *(int *)((char *)rep + 0x28 + iVar4) = iVar13;
                        *(int *)((char *)rep + 0x2c + iVar4) = 0xffffffff;
                        iVar7 = rand();
                        *(int *)((char *)rep + 0x30 + iVar4) = iVar7 % 5;
                        uVar5 = GetString(0x165);
                        *(int *)((char *)rep + 0x34 + iVar4) = (int)uVar5;
                        *(int *)((char *)rep + 0x38 + iVar4) = 0;
                        *(int *)((char *)rep + 0x3c + iVar4) = 0;
                        *(int *)((char *)rep + 0x40 + iVar4) = 0;
                        *(int *)((char *)rep + 0x44 + iVar4) = 0;
                        *(int *)((char *)rep + 0x48 + iVar4) = 0;
                        *(int *)((char *)rep + 0x4c + iVar4) = 0;
                        piVar11 = rep + (iVar14 + 1) * 0x13;
                        (rep + 1)[(iVar14 + 1) * 0x13 + *piVar11] = 0x165;
                        goto LAB_00449820;
                    }
                }
                uVar9 = flags & 0x18000;
                iVar6 = iVar4;
                if (uVar9 == 0x8000) {
                    if (0x1b5 < ysave) {
                        if (iVar3 != iVar12) goto LAB_0044a60b;
                        AppraisalPageCount = AppraisalPageCount + 1;
                        iVar6 = ystart;
                        iVar3 = iVar14;
                    }
                    iVar4 = iVar14 * 0x4c;
                    *(int *)((char *)rep + 0x24 + iVar4) = AppraisalPageCount;
                    *(int *)((char *)rep + 0x28 + iVar4) = iVar13;
                    *(int *)((char *)rep + 0x2c + iVar4) = 0xffffffff;
                    iVar7 = rand();
                    *(int *)((char *)rep + 0x30 + iVar4) = iVar7 % 5;
                    uVar5 = GetString(0x166);
                    *(int *)((char *)rep + 0x34 + iVar4) = (int)uVar5;
                    *(int *)((char *)rep + 0x38 + iVar4) = 0;
                    *(int *)((char *)rep + 0x3c + iVar4) = 0;
                    *(int *)((char *)rep + 0x40 + iVar4) = 0;
                    *(int *)((char *)rep + 0x44 + iVar4) = 0;
                    *(int *)((char *)rep + 0x48 + iVar4) = 0;
                    *(int *)((char *)rep + 0x4c + iVar4) = 0;
                    piVar11 = rep + (iVar14 + 1) * 0x13;
                    (rep + 1)[(iVar14 + 1) * 0x13 + *piVar11] = 0x166;
                LAB_00449b4b:
                    iVar14 = iVar14 + 1;
                    iVar4 = iVar6 + 0x18;
                    *piVar11 = *piVar11 + 1;
                    ysave = iVar6 + 0x2e;
                } else {
                    if (uVar9 == 0x10000) {
                        if (0x1b5 < ysave) {
                            if (iVar3 != iVar12) goto LAB_0044a60b;
                            AppraisalPageCount = AppraisalPageCount + 1;
                            iVar6 = ystart;
                            iVar3 = iVar14;
                        }
                        iVar4 = iVar14 * 0x4c;
                        *(int *)((char *)rep + 0x24 + iVar4) = AppraisalPageCount;
                        *(int *)((char *)rep + 0x28 + iVar4) = iVar13;
                        *(int *)((char *)rep + 0x2c + iVar4) = 0xffffffff;
                        iVar7 = rand();
                        *(int *)((char *)rep + 0x30 + iVar4) = iVar7 % 5;
                        uVar5 = GetString(0x167);
                        *(int *)((char *)rep + 0x34 + iVar4) = (int)uVar5;
                        *(int *)((char *)rep + 0x38 + iVar4) = 0;
                        *(int *)((char *)rep + 0x3c + iVar4) = 0;
                        *(int *)((char *)rep + 0x40 + iVar4) = 0;
                        *(int *)((char *)rep + 0x44 + iVar4) = 0;
                        *(int *)((char *)rep + 0x48 + iVar4) = 0;
                        *(int *)((char *)rep + 0x4c + iVar4) = 0;
                        piVar11 = rep + (iVar14 + 1) * 0x13;
                        (rep + 1)[(iVar14 + 1) * 0x13 + *piVar11] = 0x167;
                        goto LAB_00449b4b;
                    }
                    if (uVar9 == 0x18000) {
                        if (0x1b5 < ysave) {
                            if (iVar3 != iVar12) goto LAB_0044a60b;
                            AppraisalPageCount = AppraisalPageCount + 1;
                            iVar6 = ystart;
                            iVar3 = iVar14;
                        }
                        iVar4 = iVar14 * 0x4c;
                        *(int *)((char *)rep + 0x24 + iVar4) = AppraisalPageCount;
                        *(int *)((char *)rep + 0x28 + iVar4) = iVar13;
                        *(int *)((char *)rep + 0x2c + iVar4) = 0xffffffff;
                        iVar7 = rand();
                        *(int *)((char *)rep + 0x30 + iVar4) = iVar7 % 5;
                        uVar5 = GetString(0x168);
                        *(int *)((char *)rep + 0x34 + iVar4) = (int)uVar5;
                        *(int *)((char *)rep + 0x38 + iVar4) = 0;
                        *(int *)((char *)rep + 0x3c + iVar4) = 0;
                        *(int *)((char *)rep + 0x40 + iVar4) = 0;
                        *(int *)((char *)rep + 0x44 + iVar4) = 0;
                        *(int *)((char *)rep + 0x48 + iVar4) = 0;
                        *(int *)((char *)rep + 0x4c + iVar4) = 0;
                        piVar11 = rep + (iVar14 + 1) * 0x13;
                        (rep + 1)[(iVar14 + 1) * 0x13 + *piVar11] = 0x168;
                        goto LAB_00449b4b;
                    }
                }
                uVar9 = flags & 0x60000;
                iVar6 = iVar4;
                if (uVar9 == 0x20000) {
                    if (0x1b5 < ysave) {
                        if (iVar3 != iVar12) goto LAB_0044a60b;
                        AppraisalPageCount = AppraisalPageCount + 1;
                        iVar6 = ystart;
                        iVar3 = iVar14;
                    }
                    iVar4 = iVar14 * 0x4c;
                    *(int *)((char *)rep + 0x24 + iVar4) = AppraisalPageCount;
                    *(int *)((char *)rep + 0x28 + iVar4) = iVar13;
                    *(int *)((char *)rep + 0x2c + iVar4) = 0xffffffff;
                    iVar7 = rand();
                    *(int *)((char *)rep + 0x30 + iVar4) = iVar7 % 5;
                    uVar5 = GetString(0x169);
                    *(int *)((char *)rep + 0x34 + iVar4) = (int)uVar5;
                    *(int *)((char *)rep + 0x38 + iVar4) = 0;
                    *(int *)((char *)rep + 0x3c + iVar4) = 0;
                    *(int *)((char *)rep + 0x40 + iVar4) = 0;
                    *(int *)((char *)rep + 0x44 + iVar4) = 0;
                    *(int *)((char *)rep + 0x48 + iVar4) = 0;
                    *(int *)((char *)rep + 0x4c + iVar4) = 0;
                    piVar11 = rep + (iVar14 + 1) * 0x13;
                    (rep + 1)[(iVar14 + 1) * 0x13 + *piVar11] = 0x169;
                LAB_00449f70:
                    iVar14 = iVar14 + 1;
                    iVar4 = iVar6 + 0x18;
                    *piVar11 = *piVar11 + 1;
                    ysave = iVar6 + 0x2e;
                } else {
                    if (uVar9 == 0x40000) {
                        if (0x1b5 < ysave) {
                            if (iVar3 != iVar12) goto LAB_0044a60b;
                            AppraisalPageCount = AppraisalPageCount + 1;
                            iVar6 = ystart;
                            iVar3 = iVar14;
                        }
                        iVar4 = iVar14 * 0x4c;
                        *(int *)((char *)rep + 0x24 + iVar4) = AppraisalPageCount;
                        *(int *)((char *)rep + 0x28 + iVar4) = iVar13;
                        *(int *)((char *)rep + 0x2c + iVar4) = 0xffffffff;
                        iVar7 = rand();
                        *(int *)((char *)rep + 0x30 + iVar4) = iVar7 % 5;
                        uVar5 = GetString(0x16a);
                        *(int *)((char *)rep + 0x34 + iVar4) = (int)uVar5;
                        *(int *)((char *)rep + 0x38 + iVar4) = 0;
                        *(int *)((char *)rep + 0x3c + iVar4) = 0;
                        *(int *)((char *)rep + 0x40 + iVar4) = 0;
                        *(int *)((char *)rep + 0x44 + iVar4) = 0;
                        *(int *)((char *)rep + 0x48 + iVar4) = 0;
                        *(int *)((char *)rep + 0x4c + iVar4) = 0;
                        piVar11 = rep + (iVar14 + 1) * 0x13;
                        (rep + 1)[(iVar14 + 1) * 0x13 + *piVar11] = 0x16a;
                        goto LAB_00449f70;
                    }
                    if (uVar9 == 0x60000) {
                        if (0x1b5 < ysave) {
                            if (iVar3 != iVar12) goto LAB_0044a60b;
                            AppraisalPageCount = AppraisalPageCount + 1;
                            iVar4 = ystart;
                            iVar3 = iVar14;
                        }
                        iVar6 = iVar14 * 0x4c;
                        *(int *)((char *)rep + 0x24 + iVar6) = AppraisalPageCount;
                        *(int *)((char *)rep + 0x28 + iVar6) = iVar13;
                        *(int *)((char *)rep + 0x2c + iVar6) = 0xffffffff;
                        iVar7 = rand();
                        *(int *)((char *)rep + 0x30 + iVar6) = iVar7 % 5;
                        uVar5 = GetString(0x16b);
                        iVar7 = iVar14 + 1;
                        *(int *)((char *)rep + 0x34 + iVar6) = (int)uVar5;
                        *(int *)((char *)rep + 0x38 + iVar6) = 0;
                        *(int *)((char *)rep + 0x3c + iVar6) = 0;
                        *(int *)((char *)rep + 0x40 + iVar6) = 0;
                        *(int *)((char *)rep + 0x44 + iVar6) = 0;
                        *(int *)((char *)rep + 0x48 + iVar6) = 0;
                        *(int *)((char *)rep + 0x4c + iVar6) = 0;
                        iVar6 = iVar7 * 0x4c;
                        (rep + 1)[iVar7 * 0x13 + rep[iVar7 * 0x13]] = 0x16b;
                        rep[iVar7 * 0x13] = rep[iVar7 * 0x13] + 1;
                        ysave = iVar4 + 0x18;
                        if (0x1b5 < iVar4 + 0x2e) {
                            if (iVar3 != iVar12) goto LAB_0044a60b;
                            AppraisalPageCount = AppraisalPageCount + 1;
                            ysave = ystart;
                            iVar3 = iVar7;
                        }
                        *(int *)((char *)rep + 0x24 + iVar6) = AppraisalPageCount;
                        *(int *)((char *)rep + 0x28 + iVar6) = iVar13;
                        *(int *)((char *)rep + 0x2c + iVar6) = 0xfffffffd;
                        iVar4 = rand();
                        *(int *)((char *)rep + 0x30 + iVar6) = iVar4 % 5;
                        uVar5 = GetString(0x231);
                        iVar14 = iVar14 + 2;
                        iVar4 = ysave + 0x18;
                        *(int *)((char *)rep + 0x34 + iVar6) = (int)uVar5;
                        *(int *)((char *)rep + 0x38 + iVar6) = 0;
                        *(int *)((char *)rep + 0x3c + iVar6) = 0;
                        *(int *)((char *)rep + 0x40 + iVar6) = 0;
                        *(int *)((char *)rep + 0x44 + iVar6) = 0;
                        *(int *)((char *)rep + 0x48 + iVar6) = 0;
                        ysave = ysave + 0x2e;
                        *(int *)((char *)rep + 0x4c + iVar6) = 0;
                    }
                }
                if ((flags & 0x80000) != 0) {
                    iVar6 = iVar4;
                    if (0x1b5 < ysave) {
                        if (iVar3 != iVar12) goto LAB_0044a60b;
                        AppraisalPageCount = AppraisalPageCount + 1;
                        iVar6 = ystart;
                        iVar3 = iVar14;
                    }
                    iVar4 = iVar14 * 0x4c;
                    *(int *)((char *)rep + 0x24 + iVar4) = AppraisalPageCount;
                    *(int *)((char *)rep + 0x28 + iVar4) = iVar13;
                    *(int *)((char *)rep + 0x2c + iVar4) = 0xffffffff;
                    iVar7 = rand();
                    *(int *)((char *)rep + 0x30 + iVar4) = iVar7 % 5;
                    uVar5 = GetString(0x16c);
                    iVar14 = iVar14 + 1;
                    *(int *)((char *)rep + 0x34 + iVar4) = (int)uVar5;
                    *(int *)((char *)rep + 0x38 + iVar4) = 0;
                    *(int *)((char *)rep + 0x3c + iVar4) = 0;
                    *(int *)((char *)rep + 0x40 + iVar4) = 0;
                    *(int *)((char *)rep + 0x44 + iVar4) = 0;
                    *(int *)((char *)rep + 0x48 + iVar4) = 0;
                    *(int *)((char *)rep + 0x4c + iVar4) = 0;
                    piVar11 = rep + iVar14 * 0x13;
                    (rep + 1)[iVar14 * 0x13 + *piVar11] = 0x16c;
                    iVar4 = iVar6 + 0x18;
                    *piVar11 = *piVar11 + 1;
                    ysave = iVar6 + 0x2e;
                }
                if ((flags & 0x100000) != 0) {
                    iVar6 = iVar4;
                    if (0x1b5 < ysave) {
                        if (iVar3 != iVar12) goto LAB_0044a60b;
                        AppraisalPageCount = AppraisalPageCount + 1;
                        iVar6 = ystart;
                        iVar3 = iVar14;
                    }
                    iVar4 = iVar14 * 0x4c;
                    *(int *)((char *)rep + 0x24 + iVar4) = AppraisalPageCount;
                    *(int *)((char *)rep + 0x28 + iVar4) = iVar13;
                    *(int *)((char *)rep + 0x2c + iVar4) = 0xffffffff;
                    iVar7 = rand();
                    *(int *)((char *)rep + 0x30 + iVar4) = iVar7 % 5;
                    uVar5 = GetString(0x16d);
                    iVar14 = iVar14 + 1;
                    *(int *)((char *)rep + 0x34 + iVar4) = (int)uVar5;
                    *(int *)((char *)rep + 0x38 + iVar4) = 0;
                    *(int *)((char *)rep + 0x3c + iVar4) = 0;
                    *(int *)((char *)rep + 0x40 + iVar4) = 0;
                    *(int *)((char *)rep + 0x44 + iVar4) = 0;
                    *(int *)((char *)rep + 0x48 + iVar4) = 0;
                    *(int *)((char *)rep + 0x4c + iVar4) = 0;
                    piVar11 = rep + iVar14 * 0x13;
                    (rep + 1)[iVar14 * 0x13 + *piVar11] = 0x16d;
                    iVar4 = iVar6 + 0x18;
                    *piVar11 = *piVar11 + 1;
                    ysave = iVar6 + 0x2e;
                }
                if ((flags & 0x200000) != 0) {
                    iVar6 = iVar4;
                    if (0x1b5 < ysave) {
                        if (iVar3 != iVar12) goto LAB_0044a60b;
                        AppraisalPageCount = AppraisalPageCount + 1;
                        iVar6 = ystart;
                        iVar3 = iVar14;
                    }
                    iVar4 = iVar14 * 0x4c;
                    *(int *)((char *)rep + 0x24 + iVar4) = AppraisalPageCount;
                    *(int *)((char *)rep + 0x28 + iVar4) = iVar13;
                    *(int *)((char *)rep + 0x2c + iVar4) = 0xffffffff;
                    iVar7 = rand();
                    *(int *)((char *)rep + 0x30 + iVar4) = iVar7 % 5;
                    uVar5 = GetString(0x16f);
                    iVar14 = iVar14 + 1;
                    *(int *)((char *)rep + 0x34 + iVar4) = (int)uVar5;
                    *(int *)((char *)rep + 0x38 + iVar4) = 0;
                    *(int *)((char *)rep + 0x3c + iVar4) = 0;
                    *(int *)((char *)rep + 0x40 + iVar4) = 0;
                    *(int *)((char *)rep + 0x44 + iVar4) = 0;
                    *(int *)((char *)rep + 0x48 + iVar4) = 0;
                    *(int *)((char *)rep + 0x4c + iVar4) = 0;
                    piVar11 = rep + iVar14 * 0x13;
                    (rep + 1)[iVar14 * 0x13 + *piVar11] = 0x16f;
                    iVar4 = iVar6 + 0x18;
                    *piVar11 = *piVar11 + 1;
                    ysave = iVar6 + 0x2e;
                }
                if ((flags & 0x400000) != 0) {
                    iVar6 = iVar4;
                    if (0x1b5 < ysave) {
                        if (iVar3 != iVar12) goto LAB_0044a60b;
                        AppraisalPageCount = AppraisalPageCount + 1;
                        iVar6 = ystart;
                        iVar3 = iVar14;
                    }
                    iVar4 = iVar14 * 0x4c;
                    *(int *)((char *)rep + 0x24 + iVar4) = AppraisalPageCount;
                    *(int *)((char *)rep + 0x28 + iVar4) = iVar13;
                    *(int *)((char *)rep + 0x2c + iVar4) = 0xffffffff;
                    iVar7 = rand();
                    *(int *)((char *)rep + 0x30 + iVar4) = iVar7 % 5;
                    uVar5 = GetString(0x170);
                    iVar14 = iVar14 + 1;
                    *(int *)((char *)rep + 0x34 + iVar4) = (int)uVar5;
                    *(int *)((char *)rep + 0x38 + iVar4) = 0;
                    *(int *)((char *)rep + 0x3c + iVar4) = 0;
                    *(int *)((char *)rep + 0x40 + iVar4) = 0;
                    *(int *)((char *)rep + 0x44 + iVar4) = 0;
                    *(int *)((char *)rep + 0x48 + iVar4) = 0;
                    *(int *)((char *)rep + 0x4c + iVar4) = 0;
                    piVar11 = rep + iVar14 * 0x13;
                    (rep + 1)[iVar14 * 0x13 + *piVar11] = 0x170;
                    iVar4 = iVar6 + 0x18;
                    *piVar11 = *piVar11 + 1;
                    ysave = iVar6 + 0x2e;
                }
                if ((flags & 0x800000) != 0) {
                    if (0x1b5 < ysave) {
                        if (iVar3 != iVar12) goto LAB_0044a60b;
                        AppraisalPageCount = AppraisalPageCount + 1;
                        iVar4 = ystart;
                        iVar3 = iVar14;
                    }
                    iVar6 = iVar14 * 0x4c;
                    *(int *)((char *)rep + 0x24 + iVar6) = AppraisalPageCount;
                    *(int *)((char *)rep + 0x28 + iVar6) = iVar13;
                    *(int *)((char *)rep + 0x2c + iVar6) = 0xffffffff;
                    iVar7 = rand();
                    *(int *)((char *)rep + 0x30 + iVar6) = iVar7 % 5;
                    uVar5 = GetString(0x171);
                    iVar7 = iVar14 + 1;
                    *(int *)((char *)rep + 0x34 + iVar6) = (int)uVar5;
                    *(int *)((char *)rep + 0x38 + iVar6) = 0;
                    *(int *)((char *)rep + 0x3c + iVar6) = 0;
                    *(int *)((char *)rep + 0x40 + iVar6) = 0;
                    *(int *)((char *)rep + 0x44 + iVar6) = 0;
                    *(int *)((char *)rep + 0x48 + iVar6) = 0;
                    *(int *)((char *)rep + 0x4c + iVar6) = 0;
                    iVar6 = iVar7 * 0x4c;
                    (rep + 1)[iVar7 * 0x13 + rep[iVar7 * 0x13]] = 0x171;
                    rep[iVar7 * 0x13] = rep[iVar7 * 0x13] + 1;
                    ysave = iVar4 + 0x18;
                    if (0x1b5 < iVar4 + 0x2e) {
                        if (iVar3 != iVar12) goto LAB_0044a60b;
                        AppraisalPageCount = AppraisalPageCount + 1;
                        ysave = ystart;
                        iVar3 = iVar7;
                    }
                    *(int *)((char *)rep + 0x24 + iVar6) = AppraisalPageCount;
                    *(int *)((char *)rep + 0x28 + iVar6) = iVar13;
                    *(int *)((char *)rep + 0x2c + iVar6) = 0xfffffffd;
                    iVar4 = rand();
                    *(int *)((char *)rep + 0x30 + iVar6) = iVar4 % 5;
                    uVar5 = GetString(0x172);
                    iVar14 = iVar14 + 2;
                    iVar4 = ysave + 0x18;
                    *(int *)((char *)rep + 0x34 + iVar6) = (int)uVar5;
                    *(int *)((char *)rep + 0x38 + iVar6) = 0;
                    *(int *)((char *)rep + 0x3c + iVar6) = 0;
                    *(int *)((char *)rep + 0x40 + iVar6) = 0;
                    *(int *)((char *)rep + 0x44 + iVar6) = 0;
                    *(int *)((char *)rep + 0x48 + iVar6) = 0;
                    ysave = ysave + 0x2e;
                    *(int *)((char *)rep + 0x4c + iVar6) = 0;
                }
                if ((flags & 0x1000000) == 0) goto LAB_0044a70c;
                iVar6 = iVar4;
                if (ysave < 0x1b6) goto LAB_0044a658;
                if (iVar3 == iVar12) break;
            } else {
                if (tottotacc <= passtotacc) {
                    if (0x1b5 < iVar4) {
                        if (iVar3 != iVar12) goto LAB_0044a60b;
                        AppraisalPageCount = AppraisalPageCount + 1;
                        iVar6 = ystart;
                        iVar3 = iVar7;
                    }
                    iVar7 = iVar7 * 0x4c;
                    *(int *)((char *)rep + 0x24 + iVar7) = AppraisalPageCount;
                    *(int *)((char *)rep + 0x28 + iVar7) = iVar13;
                    *(int *)((char *)rep + 0x2c + iVar7) = 0xfffffffe;
                    iVar4 = rand();
                    *(int *)((char *)rep + 0x30 + iVar7) = iVar4 % 5;
                    uVar5 = GetString(0x153);
                    iVar14 = iVar12 + 2;
                    *(int *)((char *)rep + 0x34 + iVar7) = (int)uVar5;
                    *(int *)((char *)rep + 0x38 + iVar7) = 0;
                    *(int *)((char *)rep + 0x3c + iVar7) = 0;
                    *(int *)((char *)rep + 0x40 + iVar7) = 0;
                    *(int *)((char *)rep + 0x44 + iVar7) = 0;
                    *(int *)((char *)rep + 0x48 + iVar7) = 0;
                    *(int *)((char *)rep + 0x4c + iVar7) = 0;
                    tmp8 = iVar14 * 0x4c;
                    flatp = (int *)(((char *)rep + 0x4c) + tmp8);
                    (rep + 1)[iVar14 * 0x13 + rep[iVar14 * 0x13]] = 0x153;
                    rep[iVar14 * 0x13] = rep[iVar14 * 0x13] + 1;
                    iVar4 = iVar6 + 0x18;
                    if (0x1b5 < iVar6 + 0x2e) {
                        if (iVar3 != iVar12) goto LAB_0044a60b;
                        AppraisalPageCount = AppraisalPageCount + 1;
                        iVar4 = ystart;
                        iVar3 = iVar14;
                    }
                    *(int *)((char *)rep + 0x24 + tmp8) = AppraisalPageCount;
                    *(int *)((char *)rep + 0x28 + tmp8) = iVar13;
                    *(int *)((char *)rep + 0x2c + tmp8) = 0xfffffffe;
                    iVar14 = rand();
                    *(int *)((char *)rep + 0x30 + tmp8) = iVar14 % 5;
                LAB_00448514:
                    uVar5 = GetString(0x154);
                    *(int *)((char *)rep + 0x34 + tmp8) = (int)uVar5;
                    *(int *)((char *)rep + 0x38 + tmp8) = 0;
                    *(int *)((char *)rep + 0x3c + tmp8) = 0;
                    *(int *)((char *)rep + 0x40 + tmp8) = 0;
                    *(int *)((char *)rep + 0x44 + tmp8) = 0;
                    *(int *)((char *)rep + 0x48 + tmp8) = 0;
                    *flatp = 0;
                    goto LAB_0044855b;
                }
                if (iVar4 < 0x1b6) {
                LAB_0044820e:
                    iVar7 = iVar7 * 0x4c;
                    *(int *)((char *)rep + 0x24 + iVar7) = AppraisalPageCount;
                    *(int *)((char *)rep + 0x28 + iVar7) = iVar13;
                    *(int *)((char *)rep + 0x2c + iVar7) = 0xfffffffe;
                    iVar4 = rand();
                    *(int *)((char *)rep + 0x30 + iVar7) = iVar4 % 5;
                    uVar5 = GetString(0x150);
                    iVar14 = iVar12 + 2;
                    *(int *)((char *)rep + 0x34 + iVar7) = (int)uVar5;
                    *(int *)((char *)rep + 0x38 + iVar7) = 0;
                    *(int *)((char *)rep + 0x3c + iVar7) = 0;
                    *(int *)((char *)rep + 0x40 + iVar7) = 0;
                    *(int *)((char *)rep + 0x44 + iVar7) = 0;
                    *(int *)((char *)rep + 0x48 + iVar7) = 0;
                    *(int *)((char *)rep + 0x4c + iVar7) = 0;
                    tmp8 = iVar14 * 0x4c;
                    flatp = (int *)(((char *)rep + 0x4c) + tmp8);
                    (rep + 1)[iVar14 * 0x13 + rep[iVar14 * 0x13]] = 0x151;
                    rep[iVar14 * 0x13] = rep[iVar14 * 0x13] + 1;
                    iVar4 = iVar6 + 0x18;
                    if (0x1b5 < iVar6 + 0x2e) {
                        if (iVar3 != iVar12) goto LAB_0044a60b;
                        AppraisalPageCount = AppraisalPageCount + 1;
                        iVar4 = ystart;
                        iVar3 = iVar14;
                    }
                    *(int *)((char *)rep + 0x24 + tmp8) = AppraisalPageCount;
                    *(int *)((char *)rep + 0x28 + tmp8) = iVar13;
                    *(int *)((char *)rep + 0x2c + tmp8) = 0xfffffffe;
                    iVar14 = rand();
                    *(int *)((char *)rep + 0x30 + tmp8) = iVar14 % 5;
                    goto LAB_00448514;
                }
                if (iVar3 == iVar12) {
                    AppraisalPageCount = AppraisalPageCount + 1;
                    iVar6 = ystart;
                    iVar3 = iVar7;
                    goto LAB_0044820e;
                }
            }
        LAB_0044a60b:
            xbase = *piVar10;
            AppraisalPageCount = AppraisalPageCount + 1;
            uVar9 = 0x83;
            iVar4 = 0x6d;
            iVar3 = iVar12;
            goto LAB_00447e73;
        }
        if (iVar3 == iVar12) {
            AppraisalPageCount = AppraisalPageCount + 1;
            iVar4 = ystart;
            iVar3 = iVar12;
            goto LAB_00447ef2;
        }
        AppraisalPageCount = AppraisalPageCount + 1;
        uVar9 = wstart;
        iVar4 = ystart;
        iVar3 = iVar12;
    } while (1);
    AppraisalPageCount = AppraisalPageCount + 1;
    iVar6 = ystart;
    iVar3 = iVar14;
LAB_0044a658:
    rep[iVar14 * 0x13 + 9] = AppraisalPageCount;
    rep[iVar14 * 0x13 + 10] = iVar13;
    rep[iVar14 * 0x13 + 11] = 0xffffffff;
    rep[iVar14 * 0x13 + 12] = rand() % 5;
    uVar5 = GetString(0x173);
    iVar14 = iVar14 + 1;
    rep[(iVar14 - 1) * 0x13 + 13] = (int)uVar5;
    rep[(iVar14 - 1) * 0x13 + 14] = 0;
    rep[(iVar14 - 1) * 0x13 + 15] = 0;
    rep[(iVar14 - 1) * 0x13 + 16] = 0;
    rep[(iVar14 - 1) * 0x13 + 17] = 0;
    rep[(iVar14 - 1) * 0x13 + 18] = 0;
    rep[(iVar14 - 1) * 0x13 + 19] = 0;
    piVar10 = &rep[iVar14 * 0x13];
    piVar10[1 + *piVar10] = 0x173;
    iVar4 = iVar6 + 0x18;
    *piVar10 = *piVar10 + 1;
    ysave = iVar6 + 0x2e;
LAB_0044a70c:
    if ((flags != 0) && (MapStats.appraisal_fail_limit != 0)) {
        if (MapStats.appraisal_streak < 0) {
            iVar6 = MapStats.appraisal_streak + -1 + MapStats.appraisal_fail_limit;
        } else {
            iVar6 = MapStats.appraisal_fail_limit + -1;
        }
        if (iVar6 < 2) {
            if (0 < iVar6) {
                if (0x1b5 < ysave) {
                    if (iVar3 != iVar12)
                        goto LAB_0044abd8;
                    AppraisalPageCount = AppraisalPageCount + 1;
                    ysave = 0x83;
                    iVar4 = ystart;
                    iVar3 = iVar14;
                }
                rep[iVar14 * 0x13 + 9] = AppraisalPageCount;
                rep[iVar14 * 0x13 + 10] = iVar13;
                rep[iVar14 * 0x13 + 11] = 0xfffffffe;
                rep[iVar14 * 0x13 + 12] = rand() % 5;
                uVar5 = GetString(0x514);
                iVar7 = iVar14 + 1;
                rep[iVar14 * 0x13 + 13] = (int)uVar5;
                rep[iVar14 * 0x13 + 14] = 0;
                rep[iVar14 * 0x13 + 15] = 0;
                rep[iVar14 * 0x13 + 16] = 0;
                rep[iVar14 * 0x13 + 17] = 0;
                rep[iVar14 * 0x13 + 18] = 0;
                rep[iVar14 * 0x13 + 19] = 0;
                tmp8 = iVar7 * 0x4c;
                flatp = &rep[iVar7 * 0x13 + 19];
                (rep + 1)[iVar7 * 0x13 + rep[iVar7 * 0x13]] = 0x514;
                rep[iVar7 * 0x13] = rep[iVar7 * 0x13] + 1;
                goto joined_r0x0044aa07;
            }
            if (0x1b5 < ysave) {
                if (iVar3 == iVar12) {
                    AppraisalPageCount = AppraisalPageCount + 1;
                    ysave = 0x83;
                    iVar4 = ystart;
                    iVar3 = iVar14;
                    goto LAB_0044aaef;
                }
                goto LAB_0044abd8;
            }
        LAB_0044aaef:
            rep[iVar14 * 0x13 + 9] = AppraisalPageCount;
            rep[iVar14 * 0x13 + 10] = iVar13;
            rep[iVar14 * 0x13 + 11] = 0xfffffffe;
            rep[iVar14 * 0x13 + 12] = rand() % 5;
            uVar5 = GetString(0x237);
            iVar7 = iVar14 + 1;
            rep[iVar14 * 0x13 + 13] = (int)uVar5;
            rep[iVar14 * 0x13 + 14] = 0;
            rep[iVar14 * 0x13 + 15] = 0;
            rep[iVar14 * 0x13 + 16] = 0;
            rep[iVar14 * 0x13 + 17] = 0;
            rep[iVar14 * 0x13 + 18] = 0;
            rep[iVar14 * 0x13 + 19] = 0;
            tmp8 = iVar7 * 0x4c;
            flatp = &rep[iVar7 * 0x13 + 19];
            (rep + 1)[iVar7 * 0x13 + rep[iVar7 * 0x13]] = 0x237;
            rep[iVar7 * 0x13] = rep[iVar7 * 0x13] + 1;
            if (0x1b5 < ysave) {
                if (iVar3 != iVar12)
                    goto LAB_0044abd8;
                AppraisalPageCount = AppraisalPageCount + 1;
                iVar4 = ystart;
                iVar3 = iVar7;
            }
            rep[iVar7 * 0x13 + 9] = AppraisalPageCount;
            rep[iVar7 * 0x13 + 10] = iVar13;
            rep[iVar7 * 0x13 + 11] = 0xfffffffe;
            iVar12 = rand();
            rep[iVar7 * 0x13 + 12] = iVar12 % 5;
            /* last chance gone: "appraisal. The park will now be closed" (0x44ac5e) */
            closing_text = 0x238;
        } else {
            sprintf(fmtbuf, GetString(0x235), GetString(iVar6 + 0x514));
            if (0x1b5 < ysave) {
                if (iVar3 != iVar12)
                    goto LAB_0044abd8;
                AppraisalPageCount = AppraisalPageCount + 1;
                ysave = 0x83;
                iVar4 = ystart;
                iVar3 = iVar14;
            }
            rep[iVar14 * 0x13 + 9] = AppraisalPageCount;
            rep[iVar14 * 0x13 + 10] = iVar13;
            rep[iVar14 * 0x13 + 11] = 0xfffffffe;
            iVar8 = rand();
            iVar15 = iVar14 + 1;
            rep[iVar14 * 0x13 + 12] = iVar8 % 5;
            rep[iVar14 * 0x13 + 13] = (int)fmtbuf;
            rep[iVar14 * 0x13 + 14] = 0;
            rep[iVar14 * 0x13 + 15] = 0;
            rep[iVar14 * 0x13 + 16] = 0;
            rep[iVar14 * 0x13 + 17] = 0;
            rep[iVar14 * 0x13 + 18] = 0;
            rep[iVar14 * 0x13 + 19] = 0;
            rowp = (int *)(iVar14 + -0x12 + iVar15 * 0x12);
            tmp8 = iVar15 * 0x4c;
            flatp = &rep[iVar15 * 0x13 + 19];
            (rep + 1)[(int)rowp + rep[iVar15 * 0x13]] = 0x235;
            iVar7 = rep[iVar15 * 0x13];
            rep[iVar15 * 0x13] = iVar7 + 1;
            (rep + 1)[iVar7 + 1 + (int)rowp] = iVar6 + 0x514;
            iVar6 = rep[iVar15 * 0x13];
            rep[iVar15 * 0x13] = iVar6 + 1;
            (rep + 1)[iVar6 + 1 + (int)rowp] = 0x236;
            rep[iVar15 * 0x13] = rep[iVar15 * 0x13] + 1;
            /* the second row is the next one (the original keeps its offset in [esp+0x18]); iVar7 was stale
             * here and the closing text overwrote the first row */
            iVar7 = iVar15;
        joined_r0x0044aa07:
            if (0x1b5 < ysave) {
                if (iVar3 != iVar12) {
                LAB_0044abd8:
                    xbase = rep[iVar12 * 0x13 + 10];
                    AppraisalPageCount = AppraisalPageCount + 1;
                    uVar9 = wstart;
                    iVar14 = iVar12;
                    iVar4 = ystart;
                    iVar3 = iVar12;
                    goto LAB_0044acbb;
                }
                iVar3 = iVar14 + 1;
                AppraisalPageCount = AppraisalPageCount + 1;
                iVar4 = ystart;
            }
            rep[iVar7 * 0x13 + 9] = AppraisalPageCount;
            rep[iVar7 * 0x13 + 10] = iVar13;
            rep[iVar7 * 0x13 + 11] = 0xfffffffe;
            iVar12 = rand();
            rep[iVar7 * 0x13 + 12] = iVar12 % 5;
            /* chances left: "...an appraisal before the park is closed" (0x44aa88) */
            closing_text = 0x236;
        }
        uVar5 = GetString(closing_text);
        iVar14 = iVar14 + 2;
        rep[iVar7 * 0x13 + 13] = (int)uVar5;
        rep[iVar7 * 0x13 + 14] = 0;
        rep[iVar7 * 0x13 + 15] = 0;
        rep[iVar7 * 0x13 + 16] = 0;
        rep[iVar7 * 0x13 + 17] = 0;
        rep[iVar7 * 0x13 + 18] = 0;
        *flatp = 0;
    }
    uVar9 = iVar4 + 0x16;
LAB_0044acbb:
    do {
        while (1) {
            iVar12 = iVar14 * 0x4c;
            piVar10 = (int *)(((char *)rep + 0x28) + iVar12);
            *piVar10 = xbase;
            if ((int)uVar9 < 0x1b6) break;
            if (iVar3 == iVar14) {
                AppraisalPageCount = AppraisalPageCount + 1;
                iVar4 = ystart;
                iVar3 = iVar14;
                break;
            }
            AppraisalPageCount = AppraisalPageCount + 1;
            uVar9 = wstart;
            iVar4 = ystart;
            iVar3 = iVar14;
        }
        *(int *)((char *)rep + 0x24 + iVar12) = AppraisalPageCount;
        *piVar10 = xbase;
        *(int *)((char *)rep + 0x2c + iVar12) = 0xfffffffe;
        iVar6 = rand();
        *(int *)((char *)rep + 0x30 + iVar12) = iVar6 % 5;
        uVar5 = GetString(0x174);
        *(int *)((char *)rep + 0x34 + iVar12) = (int)uVar5;
        *(int *)((char *)rep + 0x38 + iVar12) = 0;
        iVar13 = iVar14 + 1;
        *(int *)((char *)rep + 0x3c + iVar12) = 0;
        *(int *)((char *)rep + 0x40 + iVar12) = 0;
        *(int *)((char *)rep + 0x44 + iVar12) = 0;
        *(int *)((char *)rep + 0x48 + iVar12) = 0;
        *(int *)((char *)rep + 0x4c + iVar12) = 0;
        iVar6 = iVar13 * 0x4c;
        iVar12 = iVar4 + 0x18;
        flatp = (int *)(((char *)rep + 0x4c) + iVar6);
        (rep + 1)[iVar13 * 0x13 + rep[iVar13 * 0x13]] = 0x174;
        rep[iVar13 * 0x13] = rep[iVar13 * 0x13] + 1;
        xbase = xbase + 0x30;
        tmp8 = 0;
        if ((flags & 0xf) != 0) {
            uVar9 = rand();
            uVar9 = uVar9 & 3;
            if (uVar9 == 0) {
                if (0x1b5 < iVar12 + 0x16) {
                    if (iVar3 != iVar14) goto LAB_0044d594;
                    AppraisalPageCount = AppraisalPageCount + 1;
                    iVar12 = ystart;
                    iVar3 = iVar13;
                }
                *(int *)((char *)rep + 0x24 + iVar6) = AppraisalPageCount;
                *(int *)((char *)rep + 0x28 + iVar6) = xbase;
                *(int *)((char *)rep + 0x2c + iVar6) = 0xffffffff;
                iVar4 = rand();
                *(int *)((char *)rep + 0x30 + iVar6) = iVar4 % 5;
                uVar5 = GetString(0x17c);
                *(int *)((char *)rep + 0x34 + iVar6) = (int)uVar5;
                iVar13 = iVar14 + 2;
                *(int *)((char *)rep + 0x38 + iVar6) = 0;
                *(int *)((char *)rep + 0x3c + iVar6) = 0;
                *(int *)((char *)rep + 0x40 + iVar6) = 0;
                *(int *)((char *)rep + 0x44 + iVar6) = 0;
                *(int *)((char *)rep + 0x48 + iVar6) = 0;
                iVar6 = iVar13 * 0x4c;
                *flatp = 0;
                (rep + 1)[iVar13 * 0x13 + rep[iVar13 * 0x13]] = 0x17c;
                rep[iVar13 * 0x13] = rep[iVar13 * 0x13] + 1;
                iVar4 = iVar12 + 0x18;
                if (0x1b5 < iVar12 + 0x2e) {
                    if (iVar3 != iVar14) goto LAB_0044d594;
                    AppraisalPageCount = AppraisalPageCount + 1;
                    iVar4 = ystart;
                    iVar3 = iVar13;
                }
                *(int *)((char *)rep + 0x24 + iVar6) = AppraisalPageCount;
                *(int *)((char *)rep + 0x28 + iVar6) = xbase;
                *(int *)((char *)rep + 0x2c + iVar6) = 0xfffffffd;
                iVar12 = rand();
                *(int *)((char *)rep + 0x30 + iVar6) = iVar12 % 5;
                uVar5 = GetString(0x17d);
                *(int *)((char *)rep + 0x34 + iVar6) = (int)uVar5;
                iVar13 = iVar14 + 3;
                *(int *)((char *)rep + 0x38 + iVar6) = 0;
                *(int *)((char *)rep + 0x3c + iVar6) = 0;
                *(int *)((char *)rep + 0x40 + iVar6) = 0;
                *(int *)((char *)rep + 0x44 + iVar6) = 0;
                *(int *)((char *)rep + 0x48 + iVar6) = 0;
                iVar6 = iVar13 * 0x4c;
                *(int *)((char *)rep + 0x4c + iVar6) = 0;
                flatp = (int *)(((char *)rep + 0x4c) + iVar6);
                (rep + 1)[iVar13 * 0x13 + rep[iVar13 * 0x13]] = 0x17d;
                iVar12 = iVar4;
            } else if (uVar9 == 1) {
                if (0x1b5 < iVar12 + 0x16) {
                    if (iVar3 != iVar14) goto LAB_0044d594;
                    AppraisalPageCount = AppraisalPageCount + 1;
                    iVar12 = ystart;
                    iVar3 = iVar13;
                }
                *(int *)((char *)rep + 0x24 + iVar6) = AppraisalPageCount;
                *(int *)((char *)rep + 0x28 + iVar6) = xbase;
                *(int *)((char *)rep + 0x2c + iVar6) = 0xffffffff;
                iVar4 = rand();
                *(int *)((char *)rep + 0x30 + iVar6) = iVar4 % 5;
                uVar5 = GetString(0x187);
                *(int *)((char *)rep + 0x34 + iVar6) = (int)uVar5;
                iVar13 = iVar14 + 2;
                *(int *)((char *)rep + 0x38 + iVar6) = 0;
                *(int *)((char *)rep + 0x3c + iVar6) = 0;
                *(int *)((char *)rep + 0x40 + iVar6) = 0;
                *(int *)((char *)rep + 0x44 + iVar6) = 0;
                *(int *)((char *)rep + 0x48 + iVar6) = 0;
                iVar6 = iVar13 * 0x4c;
                *flatp = 0;
                (rep + 1)[iVar13 * 0x13 + rep[iVar13 * 0x13]] = 0x187;
                rep[iVar13 * 0x13] = rep[iVar13 * 0x13] + 1;
                iVar4 = iVar12 + 0x18;
                if (0x1b5 < iVar12 + 0x2e) {
                    if (iVar3 != iVar14) goto LAB_0044d594;
                    AppraisalPageCount = AppraisalPageCount + 1;
                    iVar4 = ystart;
                    iVar3 = iVar13;
                }
                *(int *)((char *)rep + 0x24 + iVar6) = AppraisalPageCount;
                *(int *)((char *)rep + 0x28 + iVar6) = xbase;
                *(int *)((char *)rep + 0x2c + iVar6) = 0xfffffffd;
                iVar12 = rand();
                *(int *)((char *)rep + 0x30 + iVar6) = iVar12 % 5;
                uVar5 = GetString(0x188);
                *(int *)((char *)rep + 0x34 + iVar6) = (int)uVar5;
                iVar13 = iVar14 + 3;
                *(int *)((char *)rep + 0x38 + iVar6) = 0;
                *(int *)((char *)rep + 0x3c + iVar6) = 0;
                *(int *)((char *)rep + 0x40 + iVar6) = 0;
                *(int *)((char *)rep + 0x44 + iVar6) = 0;
                *(int *)((char *)rep + 0x48 + iVar6) = 0;
                iVar6 = iVar13 * 0x4c;
                *(int *)((char *)rep + 0x4c + iVar6) = 0;
                flatp = (int *)(((char *)rep + 0x4c) + iVar6);
                (rep + 1)[iVar13 * 0x13 + rep[iVar13 * 0x13]] = 0x188;
                iVar12 = iVar4;
            } else if (uVar9 == 2) {
                if (0x1b5 < iVar12 + 0x16) {
                    if (iVar3 != iVar14) goto LAB_0044d594;
                    AppraisalPageCount = AppraisalPageCount + 1;
                    iVar12 = ystart;
                    iVar3 = iVar13;
                }
                *(int *)((char *)rep + 0x24 + iVar6) = AppraisalPageCount;
                *(int *)((char *)rep + 0x28 + iVar6) = xbase;
                *(int *)((char *)rep + 0x2c + iVar6) = 0xffffffff;
                iVar4 = rand();
                *(int *)((char *)rep + 0x30 + iVar6) = iVar4 % 5;
                uVar5 = GetString(0x190);
                *(int *)((char *)rep + 0x34 + iVar6) = (int)uVar5;
                iVar13 = iVar14 + 2;
                *(int *)((char *)rep + 0x38 + iVar6) = 0;
                *(int *)((char *)rep + 0x3c + iVar6) = 0;
                *(int *)((char *)rep + 0x40 + iVar6) = 0;
                *(int *)((char *)rep + 0x44 + iVar6) = 0;
                *(int *)((char *)rep + 0x48 + iVar6) = 0;
                iVar6 = iVar13 * 0x4c;
                *flatp = 0;
                (rep + 1)[iVar13 * 0x13 + rep[iVar13 * 0x13]] = 0x190;
                rep[iVar13 * 0x13] = rep[iVar13 * 0x13] + 1;
                iVar4 = iVar12 + 0x18;
                if (0x1b5 < iVar12 + 0x2e) {
                    if (iVar3 != iVar14) goto LAB_0044d594;
                    AppraisalPageCount = AppraisalPageCount + 1;
                    iVar4 = ystart;
                    iVar3 = iVar13;
                }
                *(int *)((char *)rep + 0x24 + iVar6) = AppraisalPageCount;
                *(int *)((char *)rep + 0x28 + iVar6) = xbase;
                *(int *)((char *)rep + 0x2c + iVar6) = 0xfffffffd;
                iVar12 = rand();
                *(int *)((char *)rep + 0x30 + iVar6) = iVar12 % 5;
                uVar5 = GetString(0x191);
                *(int *)((char *)rep + 0x34 + iVar6) = (int)uVar5;
                iVar13 = iVar14 + 3;
                *(int *)((char *)rep + 0x38 + iVar6) = 0;
                *(int *)((char *)rep + 0x3c + iVar6) = 0;
                *(int *)((char *)rep + 0x40 + iVar6) = 0;
                *(int *)((char *)rep + 0x44 + iVar6) = 0;
                *(int *)((char *)rep + 0x48 + iVar6) = 0;
                iVar6 = iVar13 * 0x4c;
                *(int *)((char *)rep + 0x4c + iVar6) = 0;
                (rep + 1)[iVar13 * 0x13 + rep[iVar13 * 0x13]] = 0x191;
                rep[iVar13 * 0x13] = rep[iVar13 * 0x13] + 1;
                iVar4 = iVar4 + 0x18;
                if (0x1b5 < iVar4 + 0x2e) {
                    if (iVar3 != iVar14) goto LAB_0044d594;
                    AppraisalPageCount = AppraisalPageCount + 1;
                    iVar4 = ystart;
                    iVar3 = iVar13;
                }
                *(int *)((char *)rep + 0x24 + iVar6) = AppraisalPageCount;
                *(int *)((char *)rep + 0x28 + iVar6) = xbase;
                *(int *)((char *)rep + 0x2c + iVar6) = 0xfffffffd;
                iVar12 = rand();
                *(int *)((char *)rep + 0x30 + iVar6) = iVar12 % 5;
                uVar5 = GetString(0x192);
                *(int *)((char *)rep + 0x34 + iVar6) = (int)uVar5;
                iVar13 = iVar14 + 4;
                *(int *)((char *)rep + 0x38 + iVar6) = 0;
                *(int *)((char *)rep + 0x3c + iVar6) = 0;
                *(int *)((char *)rep + 0x40 + iVar6) = 0;
                *(int *)((char *)rep + 0x44 + iVar6) = 0;
                *(int *)((char *)rep + 0x48 + iVar6) = 0;
                iVar6 = iVar13 * 0x4c;
                *(int *)((char *)rep + 0x4c + iVar6) = 0;
                flatp = (int *)(((char *)rep + 0x4c) + iVar6);
                (rep + 1)[iVar13 * 0x13 + rep[iVar13 * 0x13]] = 0x192;
                iVar12 = iVar4;
            } else {
                if (0x1b5 < iVar12 + 0x16) {
                    if (iVar3 != iVar14) goto LAB_0044d594;
                    AppraisalPageCount = AppraisalPageCount + 1;
                    iVar12 = ystart;
                    iVar3 = iVar13;
                }
                *(int *)((char *)rep + 0x24 + iVar6) = AppraisalPageCount;
                *(int *)((char *)rep + 0x28 + iVar6) = xbase;
                *(int *)((char *)rep + 0x2c + iVar6) = 0xffffffff;
                iVar4 = rand();
                *(int *)((char *)rep + 0x30 + iVar6) = iVar4 % 5;
                uVar5 = GetString(0x19a);
                *(int *)((char *)rep + 0x34 + iVar6) = (int)uVar5;
                iVar13 = iVar14 + 2;
                *(int *)((char *)rep + 0x38 + iVar6) = 0;
                *(int *)((char *)rep + 0x3c + iVar6) = 0;
                *(int *)((char *)rep + 0x40 + iVar6) = 0;
                *(int *)((char *)rep + 0x44 + iVar6) = 0;
                *(int *)((char *)rep + 0x48 + iVar6) = 0;
                iVar6 = iVar13 * 0x4c;
                *flatp = 0;
                (rep + 1)[iVar13 * 0x13 + rep[iVar13 * 0x13]] = 0x19a;
                rep[iVar13 * 0x13] = rep[iVar13 * 0x13] + 1;
                iVar4 = iVar12 + 0x18;
                if (0x1b5 < iVar12 + 0x2e) {
                    if (iVar3 != iVar14) goto LAB_0044d594;
                    AppraisalPageCount = AppraisalPageCount + 1;
                    iVar4 = ystart;
                    iVar3 = iVar13;
                }
                *(int *)((char *)rep + 0x24 + iVar6) = AppraisalPageCount;
                *(int *)((char *)rep + 0x28 + iVar6) = xbase;
                *(int *)((char *)rep + 0x2c + iVar6) = 0xfffffffd;
                iVar12 = rand();
                *(int *)((char *)rep + 0x30 + iVar6) = iVar12 % 5;
                uVar5 = GetString(0x19b);
                *(int *)((char *)rep + 0x34 + iVar6) = (int)uVar5;
                iVar13 = iVar14 + 3;
                *(int *)((char *)rep + 0x38 + iVar6) = 0;
                *(int *)((char *)rep + 0x3c + iVar6) = 0;
                *(int *)((char *)rep + 0x40 + iVar6) = 0;
                *(int *)((char *)rep + 0x44 + iVar6) = 0;
                *(int *)((char *)rep + 0x48 + iVar6) = 0;
                iVar6 = iVar13 * 0x4c;
                *(int *)((char *)rep + 0x4c + iVar6) = 0;
                flatp = (int *)(((char *)rep + 0x4c) + iVar6);
                (rep + 1)[iVar13 * 0x13 + rep[iVar13 * 0x13]] = 0x19b;
                iVar12 = iVar4;
            }
            rep[iVar13 * 0x13] = rep[iVar13 * 0x13] + 1;
        }
        if ((flags & 0x70) == 0) {
        LAB_0044bed0:
            if ((flags & 0x7000) != 0) {
                iVar4 = rand();
                if (iVar4 % 3 == 0) {
                    if (0x1b5 < iVar12 + 0x16) {
                        if (iVar3 != iVar14) goto LAB_0044d594;
                        AppraisalPageCount = AppraisalPageCount + 1;
                        iVar12 = ystart;
                        iVar3 = iVar13;
                    }
                    *(int *)((char *)rep + 0x24 + iVar6) = AppraisalPageCount;
                    *(int *)((char *)rep + 0x28 + iVar6) = xbase;
                    *(int *)((char *)rep + 0x2c + iVar6) = 0xffffffff;
                    iVar4 = rand();
                    *(int *)((char *)rep + 0x30 + iVar6) = iVar4 % 5;
                    uVar5 = GetString(0x1c2);
                    *(int *)((char *)rep + 0x34 + iVar6) = (int)uVar5;
                    iVar7 = iVar13 + 1;
                    *(int *)((char *)rep + 0x38 + iVar6) = 0;
                    *(int *)((char *)rep + 0x3c + iVar6) = 0;
                    *(int *)((char *)rep + 0x40 + iVar6) = 0;
                    *(int *)((char *)rep + 0x44 + iVar6) = 0;
                    *(int *)((char *)rep + 0x48 + iVar6) = 0;
                    iVar6 = iVar7 * 0x4c;
                    *flatp = 0;
                    (rep + 1)[iVar7 * 0x13 + rep[iVar7 * 0x13]] = 0x1c2;
                    rep[iVar7 * 0x13] = rep[iVar7 * 0x13] + 1;
                    iVar4 = iVar12 + 0x18;
                    if (0x1b5 < iVar12 + 0x2e) {
                        if (iVar3 != iVar14) goto LAB_0044d594;
                        AppraisalPageCount = AppraisalPageCount + 1;
                        iVar4 = ystart;
                        iVar3 = iVar7;
                    }
                    *(int *)((char *)rep + 0x24 + iVar6) = AppraisalPageCount;
                    *(int *)((char *)rep + 0x28 + iVar6) = xbase;
                    *(int *)((char *)rep + 0x2c + iVar6) = 0xfffffffd;
                    iVar12 = rand();
                    *(int *)((char *)rep + 0x30 + iVar6) = iVar12 % 5;
                    uVar5 = GetString(0x1c3);
                    *(int *)((char *)rep + 0x34 + iVar6) = (int)uVar5;
                    iVar8 = iVar13 + 2;
                    *(int *)((char *)rep + 0x38 + iVar6) = 0;
                    *(int *)((char *)rep + 0x3c + iVar6) = 0;
                    *(int *)((char *)rep + 0x40 + iVar6) = 0;
                    *(int *)((char *)rep + 0x44 + iVar6) = 0;
                    *(int *)((char *)rep + 0x48 + iVar6) = 0;
                    iVar12 = iVar8 * 0x4c;
                    *(int *)((char *)rep + 0x4c + iVar6) = 0;
                    (rep + 1)[iVar8 * 0x13 + rep[iVar8 * 0x13]] = 0x1c3;
                    rep[iVar8 * 0x13] = rep[iVar8 * 0x13] + 1;
                    iVar7 = iVar4 + 0x18;
                    if (0x1b5 < iVar4 + 0x2e) {
                        if (iVar3 != iVar14) goto LAB_0044d594;
                        AppraisalPageCount = AppraisalPageCount + 1;
                        iVar7 = ystart;
                        iVar3 = iVar8;
                    }
                    *(int *)((char *)rep + 0x24 + iVar12) = AppraisalPageCount;
                    *(int *)((char *)rep + 0x28 + iVar12) = xbase;
                    *(int *)((char *)rep + 0x2c + iVar12) = 0xfffffffd;
                    iVar4 = rand();
                    *(int *)((char *)rep + 0x30 + iVar12) = iVar4 % 5;
                    uVar5 = GetString(0x1c4);
                    *(int *)((char *)rep + 0x34 + iVar12) = (int)uVar5;
                    iVar13 = iVar13 + 3;
                    *(int *)((char *)rep + 0x38 + iVar12) = 0;
                    *(int *)((char *)rep + 0x3c + iVar12) = 0;
                    *(int *)((char *)rep + 0x40 + iVar12) = 0;
                    *(int *)((char *)rep + 0x44 + iVar12) = 0;
                    *(int *)((char *)rep + 0x48 + iVar12) = 0;
                    iVar6 = iVar13 * 0x4c;
                    *(int *)((char *)rep + 0x4c + iVar12) = 0;
                    flatp = (int *)(((char *)rep + 0x4c) + iVar6);
                    (rep + 1)[iVar13 * 0x13 + rep[iVar13 * 0x13]] = 0x1c4;
                } else {
                    if (iVar4 % 3 != 1) goto LAB_0044c3df;
                    if (0x1b5 < iVar12 + 0x16) {
                        if (iVar3 != iVar14) goto LAB_0044d594;
                        AppraisalPageCount = AppraisalPageCount + 1;
                        iVar12 = ystart;
                        iVar3 = iVar13;
                    }
                    *(int *)((char *)rep + 0x24 + iVar6) = AppraisalPageCount;
                    *(int *)((char *)rep + 0x28 + iVar6) = xbase;
                    *(int *)((char *)rep + 0x2c + iVar6) = 0xffffffff;
                    iVar4 = rand();
                    *(int *)((char *)rep + 0x30 + iVar6) = iVar4 % 5;
                    uVar5 = GetString(0x1cc);
                    *(int *)((char *)rep + 0x34 + iVar6) = (int)uVar5;
                    iVar8 = iVar13 + 1;
                    *(int *)((char *)rep + 0x38 + iVar6) = 0;
                    *(int *)((char *)rep + 0x3c + iVar6) = 0;
                    *(int *)((char *)rep + 0x40 + iVar6) = 0;
                    *(int *)((char *)rep + 0x44 + iVar6) = 0;
                    *(int *)((char *)rep + 0x48 + iVar6) = 0;
                    iVar4 = iVar8 * 0x4c;
                    *flatp = 0;
                    (rep + 1)[iVar8 * 0x13 + rep[iVar8 * 0x13]] = 0x1cc;
                    rep[iVar8 * 0x13] = rep[iVar8 * 0x13] + 1;
                    iVar7 = iVar12 + 0x18;
                    if (0x1b5 < iVar12 + 0x2e) {
                        if (iVar3 != iVar14) goto LAB_0044d594;
                        AppraisalPageCount = AppraisalPageCount + 1;
                        iVar7 = ystart;
                        iVar3 = iVar8;
                    }
                    *(int *)((char *)rep + 0x24 + iVar4) = AppraisalPageCount;
                    *(int *)((char *)rep + 0x28 + iVar4) = xbase;
                    *(int *)((char *)rep + 0x2c + iVar4) = 0xfffffffd;
                    iVar12 = rand();
                    *(int *)((char *)rep + 0x30 + iVar4) = iVar12 % 5;
                    uVar5 = GetString(0x1cd);
                    *(int *)((char *)rep + 0x34 + iVar4) = (int)uVar5;
                    iVar13 = iVar13 + 2;
                    *(int *)((char *)rep + 0x38 + iVar4) = 0;
                    *(int *)((char *)rep + 0x3c + iVar4) = 0;
                    *(int *)((char *)rep + 0x40 + iVar4) = 0;
                    *(int *)((char *)rep + 0x44 + iVar4) = 0;
                    *(int *)((char *)rep + 0x48 + iVar4) = 0;
                    iVar6 = iVar13 * 0x4c;
                    *(int *)((char *)rep + 0x4c + iVar4) = 0;
                    flatp = (int *)(((char *)rep + 0x4c) + iVar6);
                    (rep + 1)[iVar13 * 0x13 + rep[iVar13 * 0x13]] = 0x1cd;
                }
                iVar12 = iVar7 + 0x18;
                flatp[-0x13] = flatp[-0x13] + 1;
                tmp8 = tmp8 + 1;
            }
        LAB_0044c3df:
            if ((flags & 0x18000) != 0) {
                iVar4 = rand();
                if (iVar4 % 3 == 0) {
                    if (0x1b5 < iVar12 + 0x16) {
                        if (iVar3 != iVar14) goto LAB_0044d594;
                        AppraisalPageCount = AppraisalPageCount + 1;
                        iVar12 = ystart;
                        iVar3 = iVar13;
                    }
                    *(int *)((char *)rep + 0x24 + iVar6) = AppraisalPageCount;
                    *(int *)((char *)rep + 0x28 + iVar6) = xbase;
                    *(int *)((char *)rep + 0x2c + iVar6) = 0xffffffff;
                    iVar4 = rand();
                    *(int *)((char *)rep + 0x30 + iVar6) = iVar4 % 5;
                    uVar5 = GetString(0x1d6);
                    *(int *)((char *)rep + 0x34 + iVar6) = (int)uVar5;
                    iVar7 = iVar13 + 1;
                    *(int *)((char *)rep + 0x38 + iVar6) = 0;
                    *(int *)((char *)rep + 0x3c + iVar6) = 0;
                    *(int *)((char *)rep + 0x40 + iVar6) = 0;
                    *(int *)((char *)rep + 0x44 + iVar6) = 0;
                    *(int *)((char *)rep + 0x48 + iVar6) = 0;
                    iVar6 = iVar7 * 0x4c;
                    *flatp = 0;
                    (rep + 1)[iVar7 * 0x13 + rep[iVar7 * 0x13]] = 0x1d6;
                    rep[iVar7 * 0x13] = rep[iVar7 * 0x13] + 1;
                    iVar4 = iVar12 + 0x18;
                    if (0x1b5 < iVar12 + 0x2e) {
                        if (iVar3 != iVar14) goto LAB_0044d594;
                        AppraisalPageCount = AppraisalPageCount + 1;
                        iVar4 = ystart;
                        iVar3 = iVar7;
                    }
                    *(int *)((char *)rep + 0x24 + iVar6) = AppraisalPageCount;
                    *(int *)((char *)rep + 0x28 + iVar6) = xbase;
                    *(int *)((char *)rep + 0x2c + iVar6) = 0xfffffffd;
                    iVar12 = rand();
                    *(int *)((char *)rep + 0x30 + iVar6) = iVar12 % 5;
                    uVar5 = GetString(0x1d7);
                    *(int *)((char *)rep + 0x34 + iVar6) = (int)uVar5;
                    iVar8 = iVar13 + 2;
                    *(int *)((char *)rep + 0x38 + iVar6) = 0;
                    *(int *)((char *)rep + 0x3c + iVar6) = 0;
                    *(int *)((char *)rep + 0x40 + iVar6) = 0;
                    *(int *)((char *)rep + 0x44 + iVar6) = 0;
                    *(int *)((char *)rep + 0x48 + iVar6) = 0;
                    iVar12 = iVar8 * 0x4c;
                    *(int *)((char *)rep + 0x4c + iVar6) = 0;
                    (rep + 1)[iVar8 * 0x13 + rep[iVar8 * 0x13]] = 0x1d7;
                    rep[iVar8 * 0x13] = rep[iVar8 * 0x13] + 1;
                    iVar7 = iVar4 + 0x18;
                    if (0x1b5 < iVar4 + 0x2e) {
                        if (iVar3 != iVar14) goto LAB_0044d594;
                        AppraisalPageCount = AppraisalPageCount + 1;
                        iVar7 = ystart;
                        iVar3 = iVar8;
                    }
                    *(int *)((char *)rep + 0x24 + iVar12) = AppraisalPageCount;
                    *(int *)((char *)rep + 0x28 + iVar12) = xbase;
                    *(int *)((char *)rep + 0x2c + iVar12) = 0xfffffffd;
                    iVar4 = rand();
                    *(int *)((char *)rep + 0x30 + iVar12) = iVar4 % 5;
                    uVar5 = GetString(0x1d8);
                    *(int *)((char *)rep + 0x34 + iVar12) = (int)uVar5;
                    iVar13 = iVar13 + 3;
                    *(int *)((char *)rep + 0x38 + iVar12) = 0;
                    *(int *)((char *)rep + 0x3c + iVar12) = 0;
                    *(int *)((char *)rep + 0x40 + iVar12) = 0;
                    *(int *)((char *)rep + 0x44 + iVar12) = 0;
                    *(int *)((char *)rep + 0x48 + iVar12) = 0;
                    iVar6 = iVar13 * 0x4c;
                    *(int *)((char *)rep + 0x4c + iVar12) = 0;
                    flatp = (int *)(((char *)rep + 0x4c) + iVar6);
                    (rep + 1)[iVar13 * 0x13 + rep[iVar13 * 0x13]] = 0x1d8;
                } else {
                    if (iVar4 % 3 != 1) goto LAB_0044c8ef;
                    if (0x1b5 < iVar12 + 0x16) {
                        if (iVar3 != iVar14) goto LAB_0044d594;
                        AppraisalPageCount = AppraisalPageCount + 1;
                        iVar12 = ystart;
                        iVar3 = iVar13;
                    }
                    *(int *)((char *)rep + 0x24 + iVar6) = AppraisalPageCount;
                    *(int *)((char *)rep + 0x28 + iVar6) = xbase;
                    *(int *)((char *)rep + 0x2c + iVar6) = 0xffffffff;
                    iVar4 = rand();
                    *(int *)((char *)rep + 0x30 + iVar6) = iVar4 % 5;
                    uVar5 = GetString(0x1e0);
                    *(int *)((char *)rep + 0x34 + iVar6) = (int)uVar5;
                    iVar8 = iVar13 + 1;
                    *(int *)((char *)rep + 0x38 + iVar6) = 0;
                    *(int *)((char *)rep + 0x3c + iVar6) = 0;
                    *(int *)((char *)rep + 0x40 + iVar6) = 0;
                    *(int *)((char *)rep + 0x44 + iVar6) = 0;
                    *(int *)((char *)rep + 0x48 + iVar6) = 0;
                    iVar4 = iVar8 * 0x4c;
                    *flatp = 0;
                    (rep + 1)[iVar8 * 0x13 + rep[iVar8 * 0x13]] = 0x1e0;
                    rep[iVar8 * 0x13] = rep[iVar8 * 0x13] + 1;
                    iVar7 = iVar12 + 0x18;
                    if (0x1b5 < iVar12 + 0x2e) {
                        if (iVar3 != iVar14) goto LAB_0044d594;
                        AppraisalPageCount = AppraisalPageCount + 1;
                        iVar7 = ystart;
                        iVar3 = iVar8;
                    }
                    *(int *)((char *)rep + 0x24 + iVar4) = AppraisalPageCount;
                    *(int *)((char *)rep + 0x28 + iVar4) = xbase;
                    *(int *)((char *)rep + 0x2c + iVar4) = 0xfffffffd;
                    iVar12 = rand();
                    *(int *)((char *)rep + 0x30 + iVar4) = iVar12 % 5;
                    uVar5 = GetString(0x1e1);
                    *(int *)((char *)rep + 0x34 + iVar4) = (int)uVar5;
                    iVar13 = iVar13 + 2;
                    *(int *)((char *)rep + 0x38 + iVar4) = 0;
                    *(int *)((char *)rep + 0x3c + iVar4) = 0;
                    *(int *)((char *)rep + 0x40 + iVar4) = 0;
                    *(int *)((char *)rep + 0x44 + iVar4) = 0;
                    *(int *)((char *)rep + 0x48 + iVar4) = 0;
                    iVar6 = iVar13 * 0x4c;
                    *(int *)((char *)rep + 0x4c + iVar4) = 0;
                    flatp = (int *)(((char *)rep + 0x4c) + iVar6);
                    (rep + 1)[iVar13 * 0x13 + rep[iVar13 * 0x13]] = 0x1e1;
                }
                iVar12 = iVar7 + 0x18;
                flatp[-0x13] = flatp[-0x13] + 1;
                tmp8 = tmp8 + 1;
            }
        LAB_0044c8ef:
            if ((flags & 0x260000) != 0) {
                iVar4 = rand();
                iVar4 = iVar4 % 3;
                if (iVar4 == 0) {
                    if ((flags & 0x200000) == 0) goto LAB_0044d010;
                    if (0x1b5 < iVar12 + 0x16) {
                        if (iVar3 != iVar14) goto LAB_0044d594;
                        AppraisalPageCount = AppraisalPageCount + 1;
                        iVar12 = ystart;
                        iVar3 = iVar13;
                    }
                    *(int *)((char *)rep + 0x24 + iVar6) = AppraisalPageCount;
                    *(int *)((char *)rep + 0x28 + iVar6) = xbase;
                    *(int *)((char *)rep + 0x2c + iVar6) = 0xffffffff;
                    iVar4 = rand();
                    *(int *)((char *)rep + 0x30 + iVar6) = iVar4 % 5;
                    uVar5 = GetString(0x1ea);
                    *(int *)((char *)rep + 0x34 + iVar6) = (int)uVar5;
                    iVar7 = iVar13 + 1;
                    *(int *)((char *)rep + 0x38 + iVar6) = 0;
                    *(int *)((char *)rep + 0x3c + iVar6) = 0;
                    *(int *)((char *)rep + 0x40 + iVar6) = 0;
                    *(int *)((char *)rep + 0x44 + iVar6) = 0;
                    *(int *)((char *)rep + 0x48 + iVar6) = 0;
                    iVar6 = iVar7 * 0x4c;
                    *flatp = 0;
                    (rep + 1)[iVar7 * 0x13 + rep[iVar7 * 0x13]] = 0x1ea;
                    rep[iVar7 * 0x13] = rep[iVar7 * 0x13] + 1;
                    iVar4 = iVar12 + 0x18;
                    if (0x1b5 < iVar12 + 0x2e) {
                        if (iVar3 != iVar14) goto LAB_0044d594;
                        AppraisalPageCount = AppraisalPageCount + 1;
                        iVar4 = ystart;
                        iVar3 = iVar7;
                    }
                    *(int *)((char *)rep + 0x24 + iVar6) = AppraisalPageCount;
                    *(int *)((char *)rep + 0x28 + iVar6) = xbase;
                    *(int *)((char *)rep + 0x2c + iVar6) = 0xfffffffd;
                    iVar12 = rand();
                    *(int *)((char *)rep + 0x30 + iVar6) = iVar12 % 5;
                    uVar5 = GetString(0x1eb);
                    *(int *)((char *)rep + 0x34 + iVar6) = (int)uVar5;
                    iVar8 = iVar13 + 2;
                    *(int *)((char *)rep + 0x38 + iVar6) = 0;
                    *(int *)((char *)rep + 0x3c + iVar6) = 0;
                    *(int *)((char *)rep + 0x40 + iVar6) = 0;
                    *(int *)((char *)rep + 0x44 + iVar6) = 0;
                    *(int *)((char *)rep + 0x48 + iVar6) = 0;
                    iVar12 = iVar8 * 0x4c;
                    *(int *)((char *)rep + 0x4c + iVar6) = 0;
                    (rep + 1)[iVar8 * 0x13 + rep[iVar8 * 0x13]] = 0x1eb;
                    rep[iVar8 * 0x13] = rep[iVar8 * 0x13] + 1;
                    iVar7 = iVar4 + 0x18;
                    if (0x1b5 < iVar4 + 0x2e) {
                        if (iVar3 != iVar14) goto LAB_0044d594;
                        AppraisalPageCount = AppraisalPageCount + 1;
                        iVar7 = ystart;
                        iVar3 = iVar8;
                    }
                    *(int *)((char *)rep + 0x24 + iVar12) = AppraisalPageCount;
                    *(int *)((char *)rep + 0x28 + iVar12) = xbase;
                    *(int *)((char *)rep + 0x2c + iVar12) = 0xfffffffd;
                    iVar4 = rand();
                    *(int *)((char *)rep + 0x30 + iVar12) = iVar4 % 5;
                    uVar5 = GetString(0x1ec);
                    *(int *)((char *)rep + 0x34 + iVar12) = (int)uVar5;
                    iVar13 = iVar13 + 3;
                    *(int *)((char *)rep + 0x38 + iVar12) = 0;
                    *(int *)((char *)rep + 0x3c + iVar12) = 0;
                    *(int *)((char *)rep + 0x40 + iVar12) = 0;
                    *(int *)((char *)rep + 0x44 + iVar12) = 0;
                    *(int *)((char *)rep + 0x48 + iVar12) = 0;
                    iVar6 = iVar13 * 0x4c;
                    *(int *)((char *)rep + 0x4c + iVar12) = 0;
                    flatp = (int *)(((char *)rep + 0x4c) + iVar6);
                    (rep + 1)[iVar13 * 0x13 + rep[iVar13 * 0x13]] = 0x1ec;
                } else if (iVar4 == 1) {
                    if (0x1b5 < iVar12 + 0x16) {
                        if (iVar3 != iVar14) goto LAB_0044d594;
                        AppraisalPageCount = AppraisalPageCount + 1;
                        iVar12 = ystart;
                        iVar3 = iVar13;
                    }
                    *(int *)((char *)rep + 0x24 + iVar6) = AppraisalPageCount;
                    *(int *)((char *)rep + 0x28 + iVar6) = xbase;
                    *(int *)((char *)rep + 0x2c + iVar6) = 0xffffffff;
                    iVar4 = rand();
                    *(int *)((char *)rep + 0x30 + iVar6) = iVar4 % 5;
                    uVar5 = GetString(0x1f4);
                    *(int *)((char *)rep + 0x34 + iVar6) = (int)uVar5;
                    iVar8 = iVar13 + 1;
                    *(int *)((char *)rep + 0x38 + iVar6) = 0;
                    *(int *)((char *)rep + 0x3c + iVar6) = 0;
                    *(int *)((char *)rep + 0x40 + iVar6) = 0;
                    *(int *)((char *)rep + 0x44 + iVar6) = 0;
                    *(int *)((char *)rep + 0x48 + iVar6) = 0;
                    iVar4 = iVar8 * 0x4c;
                    *flatp = 0;
                    (rep + 1)[iVar8 * 0x13 + rep[iVar8 * 0x13]] = 500;
                    rep[iVar8 * 0x13] = rep[iVar8 * 0x13] + 1;
                    iVar7 = iVar12 + 0x18;
                    if (0x1b5 < iVar12 + 0x2e) {
                        if (iVar3 != iVar14) goto LAB_0044d594;
                        AppraisalPageCount = AppraisalPageCount + 1;
                        iVar7 = ystart;
                        iVar3 = iVar8;
                    }
                    *(int *)((char *)rep + 0x24 + iVar4) = AppraisalPageCount;
                    *(int *)((char *)rep + 0x28 + iVar4) = xbase;
                    *(int *)((char *)rep + 0x2c + iVar4) = 0xfffffffd;
                    iVar12 = rand();
                    *(int *)((char *)rep + 0x30 + iVar4) = iVar12 % 5;
                    uVar5 = GetString(0x1f5);
                    *(int *)((char *)rep + 0x34 + iVar4) = (int)uVar5;
                    iVar13 = iVar13 + 2;
                    *(int *)((char *)rep + 0x38 + iVar4) = 0;
                    *(int *)((char *)rep + 0x3c + iVar4) = 0;
                    *(int *)((char *)rep + 0x40 + iVar4) = 0;
                    *(int *)((char *)rep + 0x44 + iVar4) = 0;
                    *(int *)((char *)rep + 0x48 + iVar4) = 0;
                    iVar6 = iVar13 * 0x4c;
                    *(int *)((char *)rep + 0x4c + iVar4) = 0;
                    flatp = (int *)(((char *)rep + 0x4c) + iVar6);
                    (rep + 1)[iVar13 * 0x13 + rep[iVar13 * 0x13]] = 0x1f5;
                } else {
                    if ((iVar4 != 2) || ((ReportFlags & 0xf) == 0)) goto LAB_0044d010;
                    if (0x1b5 < iVar12 + 0x16) {
                        if (iVar3 != iVar14) goto LAB_0044d594;
                        AppraisalPageCount = AppraisalPageCount + 1;
                        iVar12 = ystart;
                        iVar3 = iVar13;
                    }
                    *(int *)((char *)rep + 0x24 + iVar6) = AppraisalPageCount;
                    *(int *)((char *)rep + 0x28 + iVar6) = xbase;
                    *(int *)((char *)rep + 0x2c + iVar6) = 0xffffffff;
                    iVar4 = rand();
                    *(int *)((char *)rep + 0x30 + iVar6) = iVar4 % 5;
                    uVar5 = GetString(0x1fe);
                    *(int *)((char *)rep + 0x34 + iVar6) = (int)uVar5;
                    iVar8 = iVar13 + 1;
                    *(int *)((char *)rep + 0x38 + iVar6) = 0;
                    *(int *)((char *)rep + 0x3c + iVar6) = 0;
                    *(int *)((char *)rep + 0x40 + iVar6) = 0;
                    *(int *)((char *)rep + 0x44 + iVar6) = 0;
                    *(int *)((char *)rep + 0x48 + iVar6) = 0;
                    iVar4 = iVar8 * 0x4c;
                    *flatp = 0;
                    (rep + 1)[iVar8 * 0x13 + rep[iVar8 * 0x13]] = 0x1fe;
                    rep[iVar8 * 0x13] = rep[iVar8 * 0x13] + 1;
                    iVar7 = iVar12 + 0x18;
                    if (0x1b5 < iVar12 + 0x2e) {
                        if (iVar3 != iVar14) goto LAB_0044d594;
                        AppraisalPageCount = AppraisalPageCount + 1;
                        iVar7 = ystart;
                        iVar3 = iVar8;
                    }
                    *(int *)((char *)rep + 0x24 + iVar4) = AppraisalPageCount;
                    *(int *)((char *)rep + 0x28 + iVar4) = xbase;
                    *(int *)((char *)rep + 0x2c + iVar4) = 0xfffffffd;
                    iVar12 = rand();
                    *(int *)((char *)rep + 0x30 + iVar4) = iVar12 % 5;
                    uVar5 = GetString(0x1ff);
                    *(int *)((char *)rep + 0x34 + iVar4) = (int)uVar5;
                    iVar13 = iVar13 + 2;
                    *(int *)((char *)rep + 0x38 + iVar4) = 0;
                    *(int *)((char *)rep + 0x3c + iVar4) = 0;
                    *(int *)((char *)rep + 0x40 + iVar4) = 0;
                    *(int *)((char *)rep + 0x44 + iVar4) = 0;
                    *(int *)((char *)rep + 0x48 + iVar4) = 0;
                    iVar6 = iVar13 * 0x4c;
                    *(int *)((char *)rep + 0x4c + iVar4) = 0;
                    flatp = (int *)(((char *)rep + 0x4c) + iVar6);
                    (rep + 1)[iVar13 * 0x13 + rep[iVar13 * 0x13]] = 0x1ff;
                }
                iVar12 = iVar7 + 0x18;
                flatp[-0x13] = flatp[-0x13] + 1;
                tmp8 = tmp8 + 1;
            }
        LAB_0044d010:
            if ((flags & 0x1080000) != 0) {
                uVar9 = rand();
                if ((uVar9 & 1) == 0) {
                    if (0x1b5 < iVar12 + 0x16) {
                        if (iVar3 != iVar14) goto LAB_0044d594;
                        AppraisalPageCount = AppraisalPageCount + 1;
                        iVar12 = ystart;
                        iVar3 = iVar13;
                    }
                    *(int *)((char *)rep + 0x24 + iVar6) = AppraisalPageCount;
                    *(int *)((char *)rep + 0x28 + iVar6) = xbase;
                    *(int *)((char *)rep + 0x2c + iVar6) = 0xffffffff;
                    iVar4 = rand();
                    *(int *)((char *)rep + 0x30 + iVar6) = iVar4 % 5;
                    uVar5 = GetString(0x212);
                    *(int *)((char *)rep + 0x34 + iVar6) = (int)uVar5;
                    iVar8 = iVar13 + 1;
                    *(int *)((char *)rep + 0x38 + iVar6) = 0;
                    *(int *)((char *)rep + 0x3c + iVar6) = 0;
                    *(int *)((char *)rep + 0x40 + iVar6) = 0;
                    *(int *)((char *)rep + 0x44 + iVar6) = 0;
                    *(int *)((char *)rep + 0x48 + iVar6) = 0;
                    iVar7 = iVar8 * 0x4c;
                    *flatp = 0;
                    (rep + 1)[iVar8 * 0x13 + rep[iVar8 * 0x13]] = 0x212;
                    rep[iVar8 * 0x13] = rep[iVar8 * 0x13] + 1;
                    iVar4 = iVar12 + 0x18;
                    if (0x1b5 < iVar12 + 0x2e) {
                        if (iVar3 != iVar14) goto LAB_0044d594;
                        AppraisalPageCount = AppraisalPageCount + 1;
                        iVar4 = ystart;
                        iVar3 = iVar8;
                    }
                    *(int *)((char *)rep + 0x24 + iVar7) = AppraisalPageCount;
                    *(int *)((char *)rep + 0x28 + iVar7) = xbase;
                    *(int *)((char *)rep + 0x2c + iVar7) = 0xfffffffd;
                    iVar12 = rand();
                    *(int *)((char *)rep + 0x30 + iVar7) = iVar12 % 5;
                    uVar5 = GetString(0x213);
                    *(int *)((char *)rep + 0x34 + iVar7) = (int)uVar5;
                    iVar13 = iVar13 + 2;
                    *(int *)((char *)rep + 0x38 + iVar7) = 0;
                    *(int *)((char *)rep + 0x3c + iVar7) = 0;
                    *(int *)((char *)rep + 0x40 + iVar7) = 0;
                    *(int *)((char *)rep + 0x44 + iVar7) = 0;
                    *(int *)((char *)rep + 0x48 + iVar7) = 0;
                    iVar6 = iVar13 * 0x4c;
                    *(int *)((char *)rep + 0x4c + iVar7) = 0;
                    flatp = (int *)(((char *)rep + 0x4c) + iVar6);
                    (rep + 1)[iVar13 * 0x13 + rep[iVar13 * 0x13]] = 0x213;
                } else {
                    iVar4 = iVar12;
                    if (0x1b5 < iVar12 + 0x16) {
                        if (iVar3 != iVar14) goto LAB_0044d594;
                        AppraisalPageCount = AppraisalPageCount + 1;
                        iVar4 = ystart;
                        iVar3 = iVar13;
                    }
                    *(int *)((char *)rep + 0x24 + iVar6) = AppraisalPageCount;
                    *(int *)((char *)rep + 0x28 + iVar6) = xbase;
                    *(int *)((char *)rep + 0x2c + iVar6) = 0xffffffff;
                    iVar12 = rand();
                    *(int *)((char *)rep + 0x30 + iVar6) = iVar12 % 5;
                    uVar5 = GetString(0x208);
                    *(int *)((char *)rep + 0x34 + iVar6) = (int)uVar5;
                    iVar13 = iVar13 + 1;
                    *(int *)((char *)rep + 0x38 + iVar6) = 0;
                    *(int *)((char *)rep + 0x3c + iVar6) = 0;
                    *(int *)((char *)rep + 0x40 + iVar6) = 0;
                    *(int *)((char *)rep + 0x44 + iVar6) = 0;
                    *(int *)((char *)rep + 0x48 + iVar6) = 0;
                    iVar6 = iVar13 * 0x4c;
                    *flatp = 0;
                    flatp = (int *)(((char *)rep + 0x4c) + iVar6);
                    (rep + 1)[iVar13 * 0x13 + rep[iVar13 * 0x13]] = 0x208;
                }
                iVar12 = iVar4 + 0x18;
                flatp[-0x13] = flatp[-0x13] + 1;
                tmp8 = tmp8 + 1;
            }
            if ((flags & 0xc00000) == 0) goto LAB_0044d744;
            uVar9 = rand();
            if ((uVar9 & 1) != 0) {
                if ((flags & 0x800000) == 0) goto LAB_0044d744;
                if (0x1b5 < iVar12 + 0x16) {
                    if (iVar3 != iVar14) goto LAB_0044d594;
                    AppraisalPageCount = AppraisalPageCount + 1;
                    iVar12 = ystart;
                    iVar3 = iVar13;
                }
                *(int *)((char *)rep + 0x24 + iVar6) = AppraisalPageCount;
                *(int *)((char *)rep + 0x28 + iVar6) = xbase;
                *(int *)((char *)rep + 0x2c + iVar6) = 0xffffffff;
                iVar4 = rand();
                *(int *)((char *)rep + 0x30 + iVar6) = iVar4 % 5;
                uVar5 = GetString(0x21c);
                iVar4 = iVar13 + 1;
                *(int *)((char *)rep + 0x34 + iVar6) = (int)uVar5;
                *(int *)((char *)rep + 0x38 + iVar6) = 0;
                *(int *)((char *)rep + 0x3c + iVar6) = 0;
                *(int *)((char *)rep + 0x40 + iVar6) = 0;
                *(int *)((char *)rep + 0x44 + iVar6) = 0;
                *(int *)((char *)rep + 0x48 + iVar6) = 0;
                *flatp = 0;
                piVar11 = rep + iVar4 * 0x13;
                (rep + 1)[iVar4 * 0x13 + *piVar11] = 0x21c;
                *piVar11 = *piVar11 + 1;
                if (0x1b5 < iVar12 + 0x2e) {
                    if (iVar3 != iVar14) goto LAB_0044d594;
                    AppraisalPageCount = AppraisalPageCount + 1;
                }
                iVar4 = iVar4 * 0x4c;
                *(int *)((char *)rep + 0x24 + iVar4) = AppraisalPageCount;
                *(int *)((char *)rep + 0x28 + iVar4) = xbase;
                *(int *)((char *)rep + 0x2c + iVar4) = 0xfffffffd;
                iVar3 = rand();
                *(int *)((char *)rep + 0x30 + iVar4) = iVar3 % 5;
                uVar5 = GetString(0x21d);
                iVar3 = iVar13 + 2;
                *(int *)((char *)rep + 0x34 + iVar4) = (int)uVar5;
                *(int *)((char *)rep + 0x38 + iVar4) = 0;
                *(int *)((char *)rep + 0x3c + iVar4) = 0;
                *(int *)((char *)rep + 0x40 + iVar4) = 0;
                *(int *)((char *)rep + 0x44 + iVar4) = 0;
                *(int *)((char *)rep + 0x48 + iVar4) = 0;
                *(int *)((char *)rep + 0x4c + iVar4) = 0;
                piVar10 = rep + iVar3 * 0x13;
                (rep + 1)[iVar3 * 0x13 + rep[iVar3 * 0x13]] = 0x21d;
                goto LAB_0044d733;
            }
            if ((flags & 0x400000) == 0) goto LAB_0044d744;
            if (0x1b5 < iVar12 + 0x16) {
                if (iVar3 != iVar14) goto LAB_0044d594;
                AppraisalPageCount = AppraisalPageCount + 1;
                iVar12 = ystart;
                iVar3 = iVar13;
            }
            *(int *)((char *)rep + 0x24 + iVar6) = AppraisalPageCount;
            *(int *)((char *)rep + 0x28 + iVar6) = xbase;
            *(int *)((char *)rep + 0x2c + iVar6) = 0xffffffff;
            iVar4 = rand();
            *(int *)((char *)rep + 0x30 + iVar6) = iVar4 % 5;
            uVar5 = GetString(0x226);
            iVar4 = iVar13 + 1;
            *(int *)((char *)rep + 0x34 + iVar6) = (int)uVar5;
            *(int *)((char *)rep + 0x38 + iVar6) = 0;
            *(int *)((char *)rep + 0x3c + iVar6) = 0;
            *(int *)((char *)rep + 0x40 + iVar6) = 0;
            *(int *)((char *)rep + 0x44 + iVar6) = 0;
            *(int *)((char *)rep + 0x48 + iVar6) = 0;
            *flatp = 0;
            piVar11 = rep + iVar4 * 0x13;
            (rep + 1)[iVar4 * 0x13 + *piVar11] = 0x226;
            *piVar11 = *piVar11 + 1;
            if (iVar12 + 0x2e < 0x1b6) goto LAB_0044d697;
            if (iVar3 == iVar14) {
                AppraisalPageCount = AppraisalPageCount + 1;
            LAB_0044d697:
                iVar4 = iVar4 * 0x4c;
                *(int *)((char *)rep + 0x24 + iVar4) = AppraisalPageCount;
                *(int *)((char *)rep + 0x28 + iVar4) = xbase;
                *(int *)((char *)rep + 0x2c + iVar4) = 0xfffffffd;
                iVar3 = rand();
                *(int *)((char *)rep + 0x30 + iVar4) = iVar3 % 5;
                uVar5 = GetString(0x227);
                iVar3 = iVar13 + 2;
                *(int *)((char *)rep + 0x34 + iVar4) = (int)uVar5;
                *(int *)((char *)rep + 0x38 + iVar4) = 0;
                *(int *)((char *)rep + 0x3c + iVar4) = 0;
                *(int *)((char *)rep + 0x40 + iVar4) = 0;
                *(int *)((char *)rep + 0x44 + iVar4) = 0;
                *(int *)((char *)rep + 0x48 + iVar4) = 0;
                *(int *)((char *)rep + 0x4c + iVar4) = 0;
                piVar10 = rep + iVar3 * 0x13;
                (rep + 1)[iVar3 * 0x13 + rep[iVar3 * 0x13]] = 0x227;
            LAB_0044d733:
                iVar13 = iVar13 + 2;
                *piVar10 = *piVar10 + 1;
                tmp8 = tmp8 + 1;
            LAB_0044d744:
                if (tmp8 == 0) {
                    iVar13 = iVar13 + -1;
                }
                AppraisalPageCount = AppraisalPageCount + 1;
#ifdef LEGOLAND_PORT
                {
                    /* [port] log the report's rows (page, x, kind, text) to debug its layout */
                    int row;
                    DebugTrace("appraisal report: %d row(s) on %d page(s)", iVar13, AppraisalPageCount);
                    for (row = 0; row < iVar13; row++) {
                        /* kind: 1 tick, 0 cross (fails the appraisal), -1 bullet, -2 text; bar: value, maximum, pass
                         * mark (as DrawAppraisalBar gets them) */
                        DebugTrace("  row %d: page %d kind %d bar %d (value %d max %d pass %d) \"%s\"", row,
                            rep[row * 0x13 + 9], rep[row * 0x13 + 11], rep[row * 0x13 + 15], rep[row * 0x13 + 16],
                            rep[row * 0x13 + 18], rep[row * 0x13 + 17],
                            rep[row * 0x13 + 13] != 0 ? (const char *)rep[row * 0x13 + 13] : "");
                    }
                }
#endif
                InitAppraisalScreen();
                LoadAppraisalSprites();
                do {
                    if (AppraisalScreenActive == 0) {
                        KillAppraisalSprites();
                        PopRenderingStatus();
                        SpeechCloseFile();
                        FUN_00474880();
                        iVar3 = 0;
                        if (0 < iVar13) {
                            piVar10 = (int *)&rep[11];
                            do {
                                if (*piVar10 == 0) {
                                    return 0;
                                }
                                iVar3 = iVar3 + 1;
                                piVar10 = piVar10 + 0x13;
                            } while (iVar3 < iVar13);
                        }
                        return 1;
                    }
                    SpeechStreamUpdate();
                    SetPointer(5);
                    ReadGameButtons();
                    ResetHitInfo();
                    PushRenderingStatusAndLockVideoSurface();
                    PrintSprite(SPRITE_TitleScreenBk, 0, 0, 0, 0);
                    UnlightAppraisalPageButtons();
                    RenderIcons2(1, 0, 0);
                    {
                        RECT rcTitle;
                        rcTitle.left = 0x28;
                        rcTitle.top = 0x45;
                        rcTitle.right = 0x1a4;
                        rcTitle.bottom = 0x6d;
                        text = GetString(0x228);
                        NewPrintCent(text, 3, rcTitle, 0);
                    }
                    iVar3 = 0;
                    passtotacc = 0;
                    if (0 < iVar13) {
                        piVar10 = (int *)&rep[9];
                        do {
                            passtotacc = iVar3;
                            if (*piVar10 == AppraisalPage) break;
                            iVar3 = iVar3 + 1;
                            piVar10 = piVar10 + 0x13;
                            passtotacc = iVar3;
                        } while (iVar3 < iVar13);
                    }
                    if (AppraisalPageChanged != 0) {
                        rowp = (int *)0x0;
                        flags = 0;
                    }
                    /* each frame the rows start at the top of the page (0x50,0x6d)-(0x1a4,0x83) and go down 0x18 */
                    rowy = 0x6d;
                    if ((int)passtotacc < iVar13) {
                        piVar10 = (int *)((char *)flat + flags * 4);
                        piVar11 = (int *)(((char *)rep + 0x4c) + passtotacc * 0x4c);
                        do {
                            if (piVar11[-10] != AppraisalPage) break;
                            if ((AppraisalPageChanged != 0) && (iVar3 = *piVar11, iVar3 != 0)) {
                                *piVar10 = piVar11[1];
                                piVar2 = piVar10 + 1;
                                uVar9 = flags + 1;
                                if (1 < iVar3) {
                                    piVar10[1] = piVar11[2];
                                    piVar2 = piVar10 + 2;
                                    uVar9 = flags + 2;
                                }
                                flags = uVar9;
                                piVar10 = piVar2;
                                if (2 < iVar3) {
                                    flags = flags + 1;
                                    *piVar10 = piVar11[3];
                                    piVar10 = piVar10 + 1;
                                }
                                if (3 < iVar3) {
                                    *piVar10 = piVar11[4];
                                    flags = flags + 1;
                                    piVar10 = piVar10 + 1;
                                }
                                if (4 < iVar3) {
                                    flags = flags + 1;
                                    *piVar10 = piVar11[5];
                                    piVar10 = piVar10 + 1;
                                }
                                if (5 < iVar3) {
                                    *piVar10 = piVar11[6];
                                    flags = flags + 1;
                                    piVar10 = piVar10 + 1;
                                }
                                if (6 < iVar3) {
                                    flags = flags + 1;
                                    *piVar10 = piVar11[7];
                                    piVar10 = piVar10 + 1;
                                }
                                if (7 < iVar3) {
                                    flags = flags + 1;
                                    *piVar10 = piVar11[8];
                                    piVar10 = piVar10 + 1;
                                }
                            }
                            if (piVar11[-8] == -2) {
                                iVar4 = piVar11[-9] + 0x28;
                            } else {
                                iVar4 = piVar11[-9] + 0x50;
                            }
                            DrawAppraisalMark(iVar4 - 0x28, rowy, piVar11[-8], piVar11[-7]);
                            {
                                RECT rcRow;
                                rcRow.left = iVar4;
                                rcRow.top = rowy;
                                rcRow.right = 0x1a4;
                                rcRow.bottom = rowy + 0x16;
                                DrawTextOnRenderSurface((char *)piVar11[-6], 2, rcRow, piVar11[-5]);
                            }
                            if (piVar11[-4] != 0) {
                                DrawAppraisalBar(0x126, rowy, 0x1a4, rowy + 8, piVar11[-3], piVar11[-1], piVar11[-2]);
                            }
                            rowy = rowy + 0x18;
                            passtotacc = passtotacc + 1;
                            piVar11 = piVar11 + 0x13;
                        } while ((int)passtotacc < iVar13);
                    }
                    if (AppraisalPageChanged != 0) {
                        AppraisalPageChanged = 0;
                    }
                    if ((int)rowp < (int)flags) {
                        iVar3 = SpeechIsPlaying();
                        iVar4 = flat[(int)rowp];
                        if ((iVar3 == 0) && (iVar4 != -1)) {
                            rowp = (int *)((int)rowp + 1);
                            // STRING: LEGOLAND 0x004b81a8
                            sprintf(wavbuf, "TEXT%04d.WAV", iVar4);
                            SpeechCloseFile();
                            SpeechLoadWavFile(wavbuf + 8);
                            SpeechPlay();
                        }
                    } else {
                        iVar3 = SpeechIsPlaying();
                        if (iVar3 == 0) {
                            UpdateSpeechPlayback();
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
                } while (1);
            }
        } else {
            uVar9 = rand();
            uVar9 = uVar9 & 3;
            if (uVar9 != 0) {
                if (uVar9 == 1) {
                    if (0x1b5 < iVar4 + 0x2e) {
                        if (iVar3 != iVar14) goto LAB_0044d594;
                        AppraisalPageCount = AppraisalPageCount + 1;
                        iVar12 = ystart;
                        iVar3 = iVar13;
                    }
                    *(int *)((char *)rep + 0x24 + iVar6) = AppraisalPageCount;
                    *(int *)((char *)rep + 0x28 + iVar6) = xbase;
                    *(int *)((char *)rep + 0x2c + iVar6) = 0xffffffff;
                    iVar4 = rand();
                    *(int *)((char *)rep + 0x30 + iVar6) = iVar4 % 5;
                    uVar5 = GetString(0x1ae);
                    *(int *)((char *)rep + 0x34 + iVar6) = (int)uVar5;
                    iVar13 = iVar14 + 2;
                    *(int *)((char *)rep + 0x38 + iVar6) = 0;
                    *(int *)((char *)rep + 0x3c + iVar6) = 0;
                    *(int *)((char *)rep + 0x40 + iVar6) = 0;
                    *(int *)((char *)rep + 0x44 + iVar6) = 0;
                    *(int *)((char *)rep + 0x48 + iVar6) = 0;
                    iVar6 = iVar13 * 0x4c;
                    *flatp = 0;
                    (rep + 1)[iVar13 * 0x13 + rep[iVar13 * 0x13]] = 0x1ae;
                    rep[iVar13 * 0x13] = rep[iVar13 * 0x13] + 1;
                    iVar4 = iVar12 + 0x18;
                    if (0x1b5 < iVar12 + 0x2e) {
                        if (iVar3 != iVar14) goto LAB_0044d594;
                        AppraisalPageCount = AppraisalPageCount + 1;
                        iVar4 = ystart;
                        iVar3 = iVar13;
                    }
                    *(int *)((char *)rep + 0x24 + iVar6) = AppraisalPageCount;
                    *(int *)((char *)rep + 0x28 + iVar6) = xbase;
                    *(int *)((char *)rep + 0x2c + iVar6) = 0xfffffffd;
                    iVar12 = rand();
                    *(int *)((char *)rep + 0x30 + iVar6) = iVar12 % 5;
                    uVar5 = GetString(0x1af);
                    *(int *)((char *)rep + 0x34 + iVar6) = (int)uVar5;
                    iVar13 = iVar14 + 3;
                    *(int *)((char *)rep + 0x38 + iVar6) = 0;
                    *(int *)((char *)rep + 0x3c + iVar6) = 0;
                    *(int *)((char *)rep + 0x40 + iVar6) = 0;
                    *(int *)((char *)rep + 0x44 + iVar6) = 0;
                    *(int *)((char *)rep + 0x48 + iVar6) = 0;
                    iVar12 = iVar13 * 0x4c;
                    *(int *)((char *)rep + 0x4c + iVar6) = 0;
                    (rep + 1)[iVar13 * 0x13 + rep[iVar13 * 0x13]] = 0x1af;
                    rep[iVar13 * 0x13] = rep[iVar13 * 0x13] + 1;
                    iVar7 = iVar4 + 0x18;
                    if (0x1b5 < iVar4 + 0x2e) {
                        if (iVar3 != iVar14) goto LAB_0044d594;
                        AppraisalPageCount = AppraisalPageCount + 1;
                        iVar7 = ystart;
                        iVar3 = iVar13;
                    }
                    *(int *)((char *)rep + 0x24 + iVar12) = AppraisalPageCount;
                    *(int *)((char *)rep + 0x28 + iVar12) = xbase;
                    *(int *)((char *)rep + 0x2c + iVar12) = 0xfffffffd;
                    iVar4 = rand();
                    *(int *)((char *)rep + 0x30 + iVar12) = iVar4 % 5;
                    uVar5 = GetString(0x1b0);
                    *(int *)((char *)rep + 0x34 + iVar12) = (int)uVar5;
                    iVar13 = iVar14 + 4;
                    *(int *)((char *)rep + 0x38 + iVar12) = 0;
                    *(int *)((char *)rep + 0x3c + iVar12) = 0;
                    *(int *)((char *)rep + 0x40 + iVar12) = 0;
                    *(int *)((char *)rep + 0x44 + iVar12) = 0;
                    *(int *)((char *)rep + 0x48 + iVar12) = 0;
                    iVar6 = iVar13 * 0x4c;
                    *(int *)((char *)rep + 0x4c + iVar12) = 0;
                    flatp = (int *)(((char *)rep + 0x4c) + iVar6);
                    (rep + 1)[iVar13 * 0x13 + rep[iVar13 * 0x13]] = 0x1b0;
                } else {
                    if (uVar9 != 2) goto LAB_0044bed0;
                    if (0x1b5 < iVar4 + 0x2e) {
                        if (iVar3 != iVar14) goto LAB_0044d594;
                        AppraisalPageCount = AppraisalPageCount + 1;
                        iVar12 = ystart;
                        iVar3 = iVar13;
                    }
                    *(int *)((char *)rep + 0x24 + iVar6) = AppraisalPageCount;
                    *(int *)((char *)rep + 0x28 + iVar6) = xbase;
                    *(int *)((char *)rep + 0x2c + iVar6) = 0xffffffff;
                    iVar4 = rand();
                    *(int *)((char *)rep + 0x30 + iVar6) = iVar4 % 5;
                    uVar5 = GetString(0x1b8);
                    *(int *)((char *)rep + 0x34 + iVar6) = (int)uVar5;
                    iVar13 = iVar14 + 2;
                    *(int *)((char *)rep + 0x38 + iVar6) = 0;
                    *(int *)((char *)rep + 0x3c + iVar6) = 0;
                    *(int *)((char *)rep + 0x40 + iVar6) = 0;
                    *(int *)((char *)rep + 0x44 + iVar6) = 0;
                    *(int *)((char *)rep + 0x48 + iVar6) = 0;
                    iVar4 = iVar13 * 0x4c;
                    *flatp = 0;
                    (rep + 1)[iVar13 * 0x13 + rep[iVar13 * 0x13]] = 0x1b8;
                    rep[iVar13 * 0x13] = rep[iVar13 * 0x13] + 1;
                    iVar7 = iVar12 + 0x18;
                    if (0x1b5 < iVar12 + 0x2e) {
                        if (iVar3 != iVar14) goto LAB_0044d594;
                        AppraisalPageCount = AppraisalPageCount + 1;
                        iVar7 = ystart;
                        iVar3 = iVar13;
                    }
                    *(int *)((char *)rep + 0x24 + iVar4) = AppraisalPageCount;
                    *(int *)((char *)rep + 0x28 + iVar4) = xbase;
                    *(int *)((char *)rep + 0x2c + iVar4) = 0xfffffffd;
                    iVar12 = rand();
                    *(int *)((char *)rep + 0x30 + iVar4) = iVar12 % 5;
                    uVar5 = GetString(0x1b9);
                    *(int *)((char *)rep + 0x34 + iVar4) = (int)uVar5;
                    iVar13 = iVar14 + 3;
                    *(int *)((char *)rep + 0x38 + iVar4) = 0;
                    *(int *)((char *)rep + 0x3c + iVar4) = 0;
                    *(int *)((char *)rep + 0x40 + iVar4) = 0;
                    *(int *)((char *)rep + 0x44 + iVar4) = 0;
                    *(int *)((char *)rep + 0x48 + iVar4) = 0;
                    iVar6 = iVar13 * 0x4c;
                    *(int *)((char *)rep + 0x4c + iVar4) = 0;
                    flatp = (int *)(((char *)rep + 0x4c) + iVar6);
                    (rep + 1)[iVar13 * 0x13 + rep[iVar13 * 0x13]] = 0x1b9;
                }
            LAB_0044bebd:
                iVar12 = iVar7 + 0x18;
                flatp[-0x13] = flatp[-0x13] + 1;
                tmp8 = 1;
                goto LAB_0044bed0;
            }
            if (iVar4 + 0x2e < 0x1b6) {
            LAB_0044bc2c:
                *(int *)((char *)rep + 0x24 + iVar6) = AppraisalPageCount;
                *(int *)((char *)rep + 0x28 + iVar6) = xbase;
                *(int *)((char *)rep + 0x2c + iVar6) = 0xffffffff;
                iVar4 = rand();
                *(int *)((char *)rep + 0x30 + iVar6) = iVar4 % 5;
                uVar5 = GetString(0x1a4);
                *(int *)((char *)rep + 0x34 + iVar6) = (int)uVar5;
                iVar13 = iVar14 + 2;
                *(int *)((char *)rep + 0x38 + iVar6) = 0;
                *(int *)((char *)rep + 0x3c + iVar6) = 0;
                *(int *)((char *)rep + 0x40 + iVar6) = 0;
                *(int *)((char *)rep + 0x44 + iVar6) = 0;
                *(int *)((char *)rep + 0x48 + iVar6) = 0;
                iVar6 = iVar13 * 0x4c;
                *flatp = 0;
                (rep + 1)[iVar13 * 0x13 + rep[iVar13 * 0x13]] = 0x1a4;
                rep[iVar13 * 0x13] = rep[iVar13 * 0x13] + 1;
                iVar4 = iVar12 + 0x18;
                if (0x1b5 < iVar12 + 0x2e) {
                    if (iVar3 != iVar14) goto LAB_0044d594;
                    AppraisalPageCount = AppraisalPageCount + 1;
                    iVar4 = ystart;
                    iVar3 = iVar13;
                }
                *(int *)((char *)rep + 0x24 + iVar6) = AppraisalPageCount;
                *(int *)((char *)rep + 0x28 + iVar6) = xbase;
                *(int *)((char *)rep + 0x2c + iVar6) = 0xfffffffd;
                iVar12 = rand();
                *(int *)((char *)rep + 0x30 + iVar6) = iVar12 % 5;
                uVar5 = GetString(0x1a5);
                *(int *)((char *)rep + 0x34 + iVar6) = (int)uVar5;
                iVar13 = iVar14 + 3;
                *(int *)((char *)rep + 0x38 + iVar6) = 0;
                *(int *)((char *)rep + 0x3c + iVar6) = 0;
                *(int *)((char *)rep + 0x40 + iVar6) = 0;
                *(int *)((char *)rep + 0x44 + iVar6) = 0;
                *(int *)((char *)rep + 0x48 + iVar6) = 0;
                iVar12 = iVar13 * 0x4c;
                *(int *)((char *)rep + 0x4c + iVar6) = 0;
                (rep + 1)[iVar13 * 0x13 + rep[iVar13 * 0x13]] = 0x1a5;
                rep[iVar13 * 0x13] = rep[iVar13 * 0x13] + 1;
                iVar7 = iVar4 + 0x18;
                if (0x1b5 < iVar4 + 0x2e) {
                    if (iVar3 != iVar14) goto LAB_0044d594;
                    AppraisalPageCount = AppraisalPageCount + 1;
                    iVar7 = ystart;
                    iVar3 = iVar13;
                }
                *(int *)((char *)rep + 0x24 + iVar12) = AppraisalPageCount;
                *(int *)((char *)rep + 0x28 + iVar12) = xbase;
                *(int *)((char *)rep + 0x2c + iVar12) = 0xfffffffd;
                iVar4 = rand();
                *(int *)((char *)rep + 0x30 + iVar12) = iVar4 % 5;
                uVar5 = GetString(0x1a6);
                *(int *)((char *)rep + 0x34 + iVar12) = (int)uVar5;
                iVar13 = iVar14 + 4;
                *(int *)((char *)rep + 0x38 + iVar12) = 0;
                *(int *)((char *)rep + 0x3c + iVar12) = 0;
                *(int *)((char *)rep + 0x40 + iVar12) = 0;
                *(int *)((char *)rep + 0x44 + iVar12) = 0;
                *(int *)((char *)rep + 0x48 + iVar12) = 0;
                iVar6 = iVar13 * 0x4c;
                *(int *)((char *)rep + 0x4c + iVar12) = 0;
                flatp = (int *)(((char *)rep + 0x4c) + iVar6);
                (rep + 1)[iVar13 * 0x13 + rep[iVar13 * 0x13]] = 0x1a6;
                goto LAB_0044bebd;
            }
            if (iVar3 == iVar14) {
                AppraisalPageCount = AppraisalPageCount + 1;
                iVar12 = ystart;
                iVar3 = iVar13;
                goto LAB_0044bc2c;
            }
        }
    LAB_0044d594:
        iVar4 = 0x6d;
        uVar9 = 0x83;
        xbase = *piVar10;
        AppraisalPageCount = AppraisalPageCount + 1;
        iVar3 = iVar14;
    } while (1);
}

// FUNCTION: LEGOLAND 0x0044db20
void FUN_0044db20(void) {
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
void FUN_0044dc70(unsigned int param_1, unsigned int param_2) {
    MapStats.appraisal_fail_limit = param_1;
    FUN_0044db20();
    FUN_004597e0(0, (const char *)param_2);
}
