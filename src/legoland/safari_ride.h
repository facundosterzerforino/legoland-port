#pragma once

#include "objclass.h"

/* [port] declared at file scope so prototypes below refer to the same type */
struct SafariListEntry;

struct SafariNode {
    unsigned short field_0;
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
struct SafariLoadArg;

void *FUN_00414a80(struct SafariKey *key);
void FUN_00414b10(struct SafariNode *node);
void SafariRideGetInterfaces(struct ClassNode *name, struct CallbackTable *interfaces);
void FUN_00415220(Element *obj);
int FUN_00415760(struct SafariListEntry *node, unsigned short *key);
struct SafariSample;
struct SafariListEntry;
void FUN_00414ab0(struct SafariSample *a1);
LEGO_EXPORT int SaveSafariRide(void);
LEGO_EXPORT int LoadSafariRide(struct SafariLoadArg *arg);
