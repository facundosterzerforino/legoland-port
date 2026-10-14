#pragma once

#include "legoland.h"

struct Element;

struct Cursor;
LEGO_EXPORT void RemoveSoundObject(struct Element *a, unsigned int b, struct Cursor *c);

void LoadFountainSFX();
void KillFountainSFX();
void FountainAddObject(struct Element *param_1, int *param_2);
void LoadPowerStationSFX();
void LoadDinoSFX();
void KillPowerStationSFX();
void SmallPowerStationAddObject(struct Element *param_1, int *param_2);
void CrystalPowerStationAddObject(struct Element *param_1, int *param_2);
void KillDinoSFX();
void DinoAddObject(struct Element *param_1, int *param_2);
