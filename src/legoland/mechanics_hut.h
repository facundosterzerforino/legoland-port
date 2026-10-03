#pragma once

#include "legoland.h"
#include "llidb.h"

struct Cursor;
struct Ride;

void LoadMechHutMaskSprite(Element *ctx);
unsigned int MechanicsHutAddObject(unsigned int param1, unsigned int param2);
void MechanicsHutRemoveObject(Element *obj, unsigned int tile, struct Cursor *cursor);
void FUN_0043d2f0(Element *obj);
void RenderMechanicsHut(Element *obj, unsigned int param_2, unsigned int param_3, unsigned short *tile);
void KillMechHutMaskSprite();
void MechanicsHutSetEditMode();
struct RideSpriteInfo *GetMechanicsHutSpriteInfo(void *ptr, unsigned short arg2);
void FUN_0043d7c0(struct Ride *hut, unsigned int tile, int flag);
