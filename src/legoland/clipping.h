#pragma once

#include <windows.h>
#include "legoland.h"

// Per-TU header for clipping.c — canonical declarations for sprite clipping.

struct ClipQueryResult;
struct ClipQueryResult {
    unsigned char id;
    unsigned char pad_1[3];
    char *name;
    unsigned int field_8;
    unsigned int field_c;
};
extern struct ClipQueryResult FreePlayObjectTable[0x86];

struct Element;

void UnlockFreePlayObject(struct Element *object);
LEGO_EXPORT void SetClipping(RECT *rect);
LEGO_EXPORT void GetClipping(RECT *dest);
LEGO_EXPORT void StoreClipping(void);
LEGO_EXPORT void RestoreClipping(void);
void UnlockAllFreePlayObjects(void);
void FUN_0048a780(void *arg);
void FUN_0048a790(void);
void ClearFreePlaySelection(void);
unsigned int GetFreePlayObjectCost(const char *arg, struct ClipQueryResult **out);
