#pragma once

#include "objclass.h"

/* [port] declared at file scope so prototypes below refer to the same type */
struct SpaceTowerCtx;

struct SpaceTowerRideNode;
struct SpaceTowerCar;

unsigned int FUN_0043acb0(struct SpaceTowerRideNode *param_1, struct SpaceTowerCar *param_2);
void FUN_0043bac0(struct SpaceTowerCtx *param_1);

void SpaceTowerRide(struct ClassNode *head, struct CallbackTable *iface);
