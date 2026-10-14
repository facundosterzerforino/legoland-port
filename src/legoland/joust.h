#pragma once

#include "legoland.h"
#include "objclass.h"

struct Ride;
struct ClassNode;
LEGO_EXPORT void Joust_GetInterfaces(struct ClassNode *head, struct Ride *iface);
