#pragma once

#include "objclass.h"

/* [port] declared at file scope so prototypes below refer to the same type */
struct SafariListEntry;

struct SafariNode {
    unsigned short tile_id;
    unsigned char pad_2[2];
    int field_4;
    int field_8;
    int frame;
    struct SafariNode *next;
    unsigned char pad_14[0xc];
    int field_20;
    int field_24;
};

struct SafariKey;

void *FUN_00414a80(struct SafariKey *key);
void FUN_00414b10(struct SafariNode *node);
struct Ride;
struct ClassNode;
void SafariRideGetInterfaces(struct ClassNode *name, struct Ride *interfaces);
struct Element;
void SafariRideUpdate(struct Element *obj);
int FUN_00415760(struct RideNode *node, unsigned short *key);
struct SafariSample;
void FUN_00414ab0(struct SafariSample *a1);
LEGO_EXPORT int SaveSafariRide(struct Element *element);
LEGO_EXPORT int LoadSafariRide(struct Element *arg);
