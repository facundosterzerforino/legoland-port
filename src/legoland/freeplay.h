#pragma once

#include "legoland.h"

struct Element;

unsigned int FUN_0048aef0(const char *arg1, struct Element *arg2);
void FUN_0048b770(void);
LEGO_EXPORT void InitFreePlayScreen(void);
void FUN_0048b6c0(void);
unsigned char FUN_0048ac60(unsigned int p1, unsigned int p2);
LEGO_EXPORT void InitFreePlayLists(void);
LEGO_EXPORT void Add2FreePlayPanelLists(struct Sprite *a, char **name, char *name2, int key, char **after, int type);
LEGO_EXPORT unsigned int FreePlayObjectList(int a, int b, int c, int d, int e);
LEGO_EXPORT void CleanUpFreePlay(void);
void FUN_0048b6d0(void);
void FUN_0048b700(void);
void FUN_0048b740(void);
