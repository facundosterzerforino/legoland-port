#pragma once

#include "legoland.h"

#include <windows.h>

union RectPoints {
    RECT rect;
    POINT pt[2];
};

struct VideoArg {
    /* 0x00 */ int pitch;
    /* 0x04 */ int width;
    /* 0x08 */ int height;
    /* 0x0c */ void *bits;
    /* 0x10 */ int field_10;
    /* 0x14 */ int field_14;
};
struct Sprite;
struct DrawLLS;
struct Point;

struct AviFrame {
    /* 0x00 */ unsigned char pad_0[4];
    /* 0x04 */ int width;
    /* 0x08 */ int height;
    /* 0x0c */ unsigned char pad_c[0x1c];
    /* 0x28 */ unsigned short pixels[1];
};

LEGO_EXPORT int InitHostSystemGPU(void);
LEGO_EXPORT void KillHostSystemGPU(void);
LEGO_EXPORT int InitScreen(void);
LEGO_EXPORT unsigned int SetPointer(unsigned int param_1);
LEGO_EXPORT void PushRenderingStatusAndLockVideoSurface(void);
LEGO_EXPORT void PushRenderingStatusAndUnlockVideoSurface(void);
LEGO_EXPORT void PopRenderingStatus(void);
LEGO_EXPORT void PrintBackground(int x, int y);
LEGO_EXPORT int GetVideoSurface(struct VideoArg *arg);
LEGO_EXPORT void SetOverridePalette(unsigned int param_1);
LEGO_EXPORT void SetOverrideFrame(unsigned int param_1);
LEGO_EXPORT unsigned int GetOverridePalette(void);
LEGO_EXPORT unsigned int GetOverrideFrame(void);
LEGO_EXPORT void ClearSpriteOverrides(void);
LEGO_EXPORT void ZBufferHelper(struct DrawLLS *lls, RECT *rect, struct Point *pos, unsigned int *zbuf);
LEGO_EXPORT void ClearOverrideFrame(void);
LEGO_EXPORT void ClearOverridePalette(void);
void FUN_00465850(struct AviFrame *frame);
void FUN_004659a0(struct AviFrame *param_1, int param_2, int param_3);
void LoadWatchSprite(int a, int b);
void UnloadWatchSprite(void);
LEGO_EXPORT void CommitCliprectToHardware(void);
LEGO_EXPORT int RenderingComplete(void);
LEGO_EXPORT void PushSetTarget(struct Sprite *sprite);
LEGO_EXPORT void PopTarget(void);
LEGO_EXPORT int RecreateSprite(struct Sprite *sprite);
void FUN_004687f0(const char *param_1);
LEGO_EXPORT int CheckHostSystemGPU(void);
int BlitFrameToWindow(void);
void __fastcall FUN_00464ee0(struct Sprite *sprite, RECT *rect, int *off);
LEGO_EXPORT void SoftPrint_Clear(void);
LEGO_EXPORT void SoftPrint_XBltFast(struct Sprite *sprite, RECT *src, RECT *dst, unsigned int tint);
LEGO_EXPORT void SoftPrint_XBltFast(struct Sprite *sprite, RECT *a, RECT *b, unsigned int param_4);
void DrawWatchSprite(void);
int SetDisplayModeAndDetectPixelFormat(void);

struct DrawLLS {
    short frame;
    unsigned short delay;
    int width;
    int height;
    unsigned int field_c;
    short frame_count;
    short loop_delay;
    unsigned int flags;
};

struct DrawLLSFrame {
    unsigned int size;
    unsigned int pixel_count;
    unsigned int run_bytes;
    unsigned int field_c;
    unsigned short pixels[1];
};

/* A frame of an indexed-colour (type 2) animation: one palette index per visible pixel, then (frame 0 only)
 * a 256-entry 16-bit palette, then the 2-bit-code stream that says where the pixels go. */
struct DrawLLSIdxFrame {
    /* 0x00 */ unsigned int size; /* bytes to the next frame */
    /* 0x04 */ unsigned int pixel_count; /* bytes of palette indices */
    /* 0x08 */ unsigned char pixels[1];
};

void FUN_00464480(struct DrawLLS *lls, RECT *rect, struct Point *pos);
void FUN_00465240(struct DrawLLS *lls, RECT *rect, struct Point *pos);
void FUN_00466770(struct DrawLLS *lls, RECT *clip, struct Point *pos);
void FUN_00466d80(unsigned short *dst, unsigned short *src, unsigned char *runs, unsigned int *mask, int h, int stride, int skip, int left, int width, int flags, unsigned short *cursor);
void FUN_00467180(unsigned short *dst, unsigned short *src, unsigned char *runs, unsigned int *mask, int h, int stride, int skip, int left, int width, int flags, unsigned short *cursor);
void FUN_004673f0(unsigned short *dst, unsigned short *src, unsigned char *runs, unsigned int *mask, int h, int stride, int skip, int left, int width, int flags, unsigned short *cursor);
void FUN_00467640(unsigned short *dst, unsigned short *src, unsigned char *runs, unsigned int *mask, int h, int stride, int skip, int left, int width, int flags, unsigned short *cursor);
void FUN_004677b0(unsigned short *dst, unsigned short *src, unsigned char *runs, unsigned int *mask, int h, int stride, int skip, int left, int width, int flags, unsigned short *cursor);
void FUN_00467b00(unsigned short *dst, unsigned short *src, unsigned char *runs, unsigned int *mask, int h, int stride, int skip, int left, int width, int flags, unsigned short *cursor);
void FUN_00467d10(unsigned short *dst, unsigned short *src, unsigned char *runs, unsigned int *mask, int h, int stride, int skip, int left, int width, int flags, unsigned short *cursor);
void FUN_00467f00(unsigned short *dst, unsigned short *src, unsigned char *runs, unsigned int *mask, int h, int stride, int skip);
void FUN_00465ee0(struct DrawLLS *lls, RECT *clip, struct Point *pos);
void FUN_00468410(unsigned short *dst, unsigned short *src, unsigned char *runs, unsigned int *mask, int h, int stride, int skip, int left, int width, int flags, unsigned short *cursor);
void FUN_00468040(unsigned short *dst, unsigned short *src, unsigned char *runs, unsigned int *mask, int h, int stride, int skip, int left, int width, int flags, unsigned short *cursor);
