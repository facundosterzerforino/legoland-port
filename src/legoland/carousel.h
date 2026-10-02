#pragma once

#include "legoland.h"

struct Bloke;

/* Carousel ride node (one 0x2c allocation), head at CarouselNodeList. */
struct CarouselNode {
    /* 0x00 */ struct CarouselNode *next;
    /* 0x04 */ unsigned short id;
    /* 0x06 */ unsigned char seated_count;
    /* 0x07 */ unsigned char leaving_count;
    /* 0x08 */ unsigned char frame;
    /* 0x09 */ unsigned char pad_9[0xc - 0x9];
    /* 0x0c */ unsigned int flags;
    /* 0x10 */ unsigned char cycles_left;
    /* 0x11 */ unsigned char pad_11[0x14 - 0x11];
    /* 0x14 */ unsigned int frame_ticks;
    /* 0x18 */ unsigned char boarding_count;
    /* 0x19 */ unsigned char pad_19[0x1c - 0x19];
    /* 0x1c */ unsigned int boarding_timer;
    /* 0x20 */ unsigned char slots[0x2c - 0x20];
};

struct CarouselListElem {
    /* 0x00 */ struct CarouselListElem *next;
    /* 0x04 */ unsigned char pad_4[0x8 - 0x4];
    /* 0x08 */ struct Bloke *bloke;
    /* 0x0c */ unsigned short id;
};

struct CarouselRide {
    /* 0x00 */ unsigned char pad_0[0xc];
    /* 0x0c */ unsigned int x;
    /* 0x10 */ unsigned int y;
    /* 0x14 */ unsigned char pad_14[0x1c - 0x14];
    /* 0x1c */ unsigned int flags;
    /* 0x20 */ unsigned char pad_20[0x2e - 0x20];
    /* 0x2e */ unsigned short capacity;
    /* 0x30 */ unsigned char pad_30[0x64 - 0x30];
    /* 0x64 */ void *layer;
    /* 0x68 */ unsigned char pad_68[0xcc - 0x68];
    /* 0xcc */ struct CarouselListElem *list;
};

struct CarouselRideObj {
    /* 0x00 */ unsigned char pad_0[0xc];
    /* 0x0c */ struct CarouselRide *ride;
};

void AddCarouselNode(unsigned short *param_1);
void RemoveCarouselNode(struct CarouselNode *node);
void FreeAllCarouselNodes(void);
struct CarouselNode *FindCarouselNode(unsigned short *param_1);
void FUN_0042bc90(struct CarouselNode *node);
void FUN_0042c210(struct CarouselNode *node);
void FUN_0042c800(void);
int FUN_0042cd20(struct CarouselListElem *elem, struct CarouselNode *node, signed char divisor);

void FUN_0042bcf0(struct CarouselRideObj *param_1, unsigned int param_2, unsigned int param_3, unsigned short *param_4, unsigned int param_5, unsigned int param_6);
void FUN_0042c280(struct CarouselRideObj *param_1);
void FUN_0042c3f0(struct CarouselRideObj *input);
void FUN_0042c460();
void FUN_0042c4a0(struct CarouselRideObj *param_1, TileId tile, unsigned int param_3);
void FUN_0042c520(unsigned int param_1, unsigned char *param_2);
struct RideSpriteInfo *FUN_0042c550(struct CarouselRideObj *param1, unsigned short param2);
int Carousel_Save(void);
int Carousel_Load(struct CarouselRideObj *param_1);
void FUN_0042c820(struct CarouselRideObj *param_1);
