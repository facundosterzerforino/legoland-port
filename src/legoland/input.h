#pragma once

#include "controller.h"
#include "legoland.h"

LEGO_EXPORT int InitInputSystem(void);
LEGO_EXPORT void KillInputSystem(void);
LEGO_EXPORT char GetInputChar(void);
LEGO_EXPORT void ScanKeyboard(void);
LEGO_EXPORT void ScanMouse(void);
LEGO_EXPORT void UpdateControllerFromMouseData(struct CtrlBuffer *buffer);
LEGO_EXPORT void UpdateControllerFromKeyboardData(struct CtrlBuffer *buffer);
int CreateKeyboardDevice(void);
int CreateMouseDevice(void);
unsigned int IsLeftShiftDown(void);
unsigned int IsRightShiftDown(void);
char FUN_00474130(void);
void FUN_00474190(void);
void FUN_004741c0(void);
