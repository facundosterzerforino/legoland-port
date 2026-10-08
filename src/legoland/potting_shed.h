#pragma once

#include "legoland.h"
#include "llidb.h"

struct Cursor;
struct PSCarLayer;

void LoadGShedMatteSprite(Element *obj);
unsigned int PottingShedAddObject(unsigned int param_1, unsigned int param_2);
void PottingShedRemoveObject(Element *obj, unsigned int tile, struct Cursor *cursor);
void PottingShedUpdate(Element *obj);
void RenderPottingShed(Element *obj, unsigned int param_2, unsigned int param_3, unsigned short *tile);
void KillGShedMatteSprite();
void PottingShedSetEditMode();
struct RideSpriteInfo *GetPottingShedSpriteInfo(struct PSCarLayer *param1, unsigned short param2);