#pragma once

#include "legoland.h"
#include "objclass.h"

struct Ride;
struct ClassNode;
LEGO_EXPORT void TempleSlide_GetInterfaces(struct ClassNode *head, struct Ride *iface);

struct TempleRide;
void FUN_00417130(struct TempleRide *arg);
