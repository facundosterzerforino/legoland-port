#pragma once

#include "objclass.h"

/* [port] declared at file scope so prototypes below refer to the same type */
struct SpaceTowerCtx;

struct SpaceTowerRideNode;
struct SpaceTowerCar;

struct AnimEntry {
    /* 0x00 */ int count;
    /* 0x04 */ struct AnimStep *steps;
};

struct AnimLayout {
    /* 0x00 */ int count;
    /* 0x04 */ struct AnimEntry **entries;
};

unsigned int FUN_0043acb0(struct SpaceTowerRideNode *param_1, struct SpaceTowerCar *param_2);
void SpaceTowerUpdate(struct SpaceTowerCtx *param_1);

struct Ride;
struct ClassNode;
void SpaceTowerRide(struct ClassNode *head, struct Ride *iface);
