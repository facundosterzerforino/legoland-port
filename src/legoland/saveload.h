#pragma once

#include "legoland.h"

struct Element;

/* An element pointer that FindeIneList replaces with its save-game index. */
union SavedElement {
    struct Element *element;
    int index;
};

LEGO_EXPORT int SaveGame(char *filename);
LEGO_EXPORT int FindeIneList(union SavedElement *handle);
LEGO_EXPORT struct Element *GeteListPtr(int idx);
void FUN_0047f810(void);
int FUN_0047f820(void);
unsigned int OpenLogFile(const char *path);
int CloseLogFile(void);
void FUN_0047f850(void);
LEGO_EXPORT void UnloadSaveGameMap(void);
LEGO_EXPORT int LoadGame(char *path);
int SkipSaveGameDword(void);
