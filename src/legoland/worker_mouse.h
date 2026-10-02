#pragma once

#include "legoland.h"

/* [port] file-scope forward declarations (structs first named in a prototype) */
struct Point;

LEGO_EXPORT void CheckWorkerOnMouseStatus(int a);
LEGO_EXPORT void ResetMoveAWorkerStruct(void);
LEGO_EXPORT void ResetWorkersOldCoords(void);
unsigned int FUN_004700c0(void *object);
void *GetWorkerOnMouse(void);
void PickUpWorker(unsigned int type, struct Bloke *worker);
int FUN_00470270(void);
struct WorkOrder *FUN_00470410(struct Point *out);
void FUN_00470950(void *a, void *b);
void KillPUOKAndCBSprites(void);
LEGO_EXPORT void RenderWorkerOnMouse(void);
