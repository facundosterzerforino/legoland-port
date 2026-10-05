#pragma once

#include "legoland.h"

struct ClassNode;
struct CallbackTable;

void ShopsGetInterfaces(struct ClassNode *name, struct CallbackTable *ci);

void LegoShop2Update(struct Element *obj);
