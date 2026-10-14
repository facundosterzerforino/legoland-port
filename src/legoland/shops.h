#pragma once

#include "legoland.h"

struct ClassNode;

struct Ride;
void ShopsGetInterfaces(struct ClassNode *name, struct Ride *ci);

struct Element;
void LegoShop2Update(struct Element *obj);
