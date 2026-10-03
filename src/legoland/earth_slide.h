#pragma once

#include "legoland.h"

struct Cursor;

/* Earth-slide class node (one 0x24 allocation), head at EarthNodeHead. */
struct EarthNode {
    /* 0x00 */ unsigned short id;
    /* 0x02 */ unsigned char pad_2[0x4 - 0x2];
    /* 0x04 */ unsigned int field_4;
    /* 0x08 */ unsigned char field_8;
    /* 0x09 */ unsigned char field_9;
    /* 0x0a */ unsigned char field_a;
    /* 0x0b */ unsigned char field_b;
    /* 0x0c */ struct EarthNode *next;
    /* 0x10 */ unsigned int field_10;
    /* 0x14 */ unsigned int field_14;
    /* 0x18 */ unsigned char queue_count;
    /* 0x19 */ unsigned char pad_19[0x1c - 0x19];
    /* 0x1c */ struct EarthQueue *queue_head;
    /* 0x20 */ struct EarthQueue *queue_tail;
};

/* Per-node bloke queue element (8-byte allocation). */
struct EarthQueue {
    /* 0x00 */ struct EarthQueue *next;
    /* 0x04 */ struct EarthBlokeElem *elem;
};

/* Bloke-list element hung off the ride object (Cursor) at offset 0xcc. */
struct EarthBlokeElem {
    /* 0x00 */ struct EarthBlokeElem *next;
    /* 0x04 */ unsigned char pad_4[0x8 - 0x4];
    /* 0x08 */ void *bloke;
    /* 0x0c */ unsigned char id_x;
    /* 0x0d */ unsigned char id_y;
};

/* Callback argument; field_c points at the ride/cursor object (EarthSlideRide). */
struct EarthRideObj {
    /* 0x00 */ unsigned char pad_0[0xc];
    /* 0x0c */ struct Cursor *ride;
};

void FUN_0042cd70(unsigned short *a1);
void RemoveEarthNode(struct EarthNode *node);
struct EarthNode *FUN_0042ce20(volatile unsigned short *param_1);
void FUN_0042ce50(struct EarthNode *node, struct EarthBlokeElem *elem);
void EarthNodeAppendQueueEntry(struct EarthNode *p, struct EarthQueue *value);
void FUN_0042cec0(struct Cursor *param_1, unsigned char *param_2, int *param_3);
int FUN_0042cf40(struct EarthNode *param_1, void *param_2);
void FUN_0042cf70(struct EarthNode *param_1);
void FUN_0042d040(struct EarthNode *list);
unsigned int CountEarthBlokeElemsUntil(struct EarthBlokeElem *param_1, struct EarthBlokeElem *param_2);
struct EarthBlokeElem *GetNthNextEarthBlokeElem(struct EarthBlokeElem *param_1, unsigned int param_2);
void FUN_0042d5f0(void);

void RenderEarthSlide(struct EarthRideObj *param_1, unsigned int param_2, unsigned int param_3, unsigned char *param_4, unsigned int param_5, unsigned int param_6);
void LoadEarthSlideResources(struct EarthRideObj *param_1);
void UnloadEarthSlideResources(struct EarthRideObj *arg1);
void EarthSlideSetEditMode();
void EarthSlideRemoveObject(struct EarthRideObj *param_1, TileId tile, unsigned int param_3);
void EarthSlideAddObject(unsigned int param_1, unsigned char *param_2);
int EarthSlideRide_Save(void);
int EarthSlideRide_Load(void);
void FUN_0042d610(struct EarthRideObj *param_1);
