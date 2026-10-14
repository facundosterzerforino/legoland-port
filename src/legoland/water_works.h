#pragma once

#include "legoland.h"

struct WaterNode {
    /* 0x00 */ struct WaterNode *next;
    /* 0x04 */ unsigned short key;
    /* 0x06 */ unsigned char pad_6[2];
    /* 0x08 */ unsigned char field_8;
    /* 0x09 */ unsigned char frame;
    /* 0x0a */ unsigned char block_index;
    /* 0x0b */ unsigned char pad_b[1];
};

struct ClassNode;

struct Ride;
void WaterWorksGetInterfaces(struct ClassNode *name, struct Ride *ci);
