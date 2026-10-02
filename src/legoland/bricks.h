#pragma once

#include "legoland.h"

// Per-TU header for bricks.c — canonical declarations for the brick currency.

int AreBricksLimited(void);
void SetBricksLimited(int param_1);
LEGO_EXPORT void AddBricks(unsigned int param_1);
LEGO_EXPORT void UseBricks(unsigned int param_1);
LEGO_EXPORT int GetBrickCount(void);
void SetBrickCount(unsigned int param_1);
int FUN_00457970(int dx, int dy);
void FUN_00457a70(void);
int SaveCurrency(void);
int LoadCurrency(void);
