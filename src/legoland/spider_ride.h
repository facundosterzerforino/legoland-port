#pragma once

#include "legoland.h"
#include "objclass.h"

/* [port] declared at file scope so prototypes below refer to the same type */
struct SpiderLoadArg;

struct SpiderNode {
    unsigned short tile_id;
    unsigned char pad_2[2];
    char frame;
    unsigned char pad_5[0x27];
    struct SpiderNode *next;
};

int FUN_00415a90(struct SpiderNode *node);
void FUN_004161f0(struct SpiderNode *node);
struct SpiderNode *FindSpiderNode(TileId *key);
void SpiderRideUpdate(Element *obj);
LEGO_EXPORT int SaveSpider(struct Element *element);
LEGO_EXPORT int LoadSpider(struct Element *element);

struct Ride;
void SpiderRide(struct Element *head, struct Ride *iface);
struct SlotOwner;
struct SlotArray;
int FUN_00416830(struct RideNode *owner, struct SlotArray *arr, signed char count);
