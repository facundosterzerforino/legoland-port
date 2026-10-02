#pragma once

#include "gamemap.h"
#include "legoland.h"

struct Point;

struct ObjClass {
    /* 0x00 */ unsigned char pad_0[0x1c];
    /* 0x1c */ unsigned int flags;
    /* 0x20 */ short type;
    /* 0x22 */ unsigned char pad_22[0x3c - 0x22];
    /* 0x3c */ struct Footprint footprint;
    /* 0x50 */ unsigned char pad_50[0x58 - 0x50];
    /* 0x58 */ unsigned char *field_58;
    /* 0x5c */ unsigned char pad_5c[0x78 - 0x5c];
    /* 0x78 */ char *name;
    /* 0x7c */ unsigned char pad_7c[0x90 - 0x7c];
    /* 0x90 */ void (*method_90)(unsigned int object, struct Point *pos, int param_3);
    /* 0x94 */ void (*method_94)(unsigned int *param_1, void *cursor);
    /* 0x98 */ void (*method_98)(unsigned int classid, struct Point *pos);
    /* 0x9c */ void (*method_9c)(unsigned int classid, TileId coords, void *cursor);
    /* 0xa0 */ unsigned char pad_a0[0xc4 - 0xa0];
    /* 0xc4 */ unsigned int *element;
};

struct WorkArea {
    unsigned int field_0;
    unsigned int field_4;
    unsigned int field_8;
    unsigned int field_c;
};

struct CtrlBuffer {
    /* 0x00 */ int prev_x;
    /* 0x04 */ int prev_y;
    /* 0x08 */ int x;
    /* 0x0c */ int y;
    /* 0x10 */ int delta_x;
    /* 0x14 */ int delta_y;
    /* 0x18 */ unsigned int buttons;
    /* 0x1c */ int mouse_threshold1;
    /* 0x20 */ int mouse_threshold2;
    /* 0x24 */ int mouse_accel;
};

LEGO_EXPORT int SetupControllers(void);
LEGO_EXPORT void ReadGameButtons(void);
void FreeControllers(void);
void FUN_00452030(void);
