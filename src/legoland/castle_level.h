#pragma once

#include "legoland.h"
#include "llidb.h"

struct CallbackTable;
struct Cursor;
struct ClassNode;

void CastleLevel1_GetInterfaces(struct ClassNode *name, struct CallbackTable *ci);
void LoadCastleLevelMatteSprite(Element *obj);
void KillCastleLevelMatteSprite(void);
void RenderCastleLevel(Element *obj, unsigned int param_2, unsigned int param_3, unsigned short *tile, unsigned int param_5, unsigned int clip);
void CastleLevel1Update(Element *obj);
void CastleLevelSetEditMode(void);
void CastleLevelRemoveObject(Element *obj, TileId tile, struct Cursor *cursor);
void CastleLevelAddObject(unsigned int param1, unsigned int param2);
