#pragma once

#include "legoland.h"

struct Cursor;
struct Element;

void FUN_0042d970(TileId *tile, unsigned int arg);
void RenderEntrance(struct Element *obj, unsigned int param_2, unsigned int param_3, TileId *tile, unsigned int param_5, unsigned int clip);
void LoadEntranceResources(struct Element *param);
void UnloadEntranceResources(struct Element *param);
void EntranceRemoveObject(struct Element *obj, TileId tile, struct Cursor *cursor);
void Entrance1Update(struct Element *elem);
