#include "power.h"

#include <stdlib.h>

#include "globals.h"
#include "legoland.h"
#include "map_object.h"
#include "sound_music.h"

// FUNCTION: LEGOLAND 0x00452990
void LoadFountainSFX(void) {
    unsigned int counter;

    counter = FountainSFXRefCount;
    FountainSFXRefCount++;
    if (counter == 0) {
        Load_FXList(FountainSFX, 5);
    }
}

// FUNCTION: LEGOLAND 0x004529c0
void KillFountainSFX(void) {
    FountainSFXRefCount--;
    if (FountainSFXRefCount == 0) {
        Kill_FXList(FountainSFX, 5);
    }
}

// FUNCTION: LEGOLAND 0x004529e0
void FUN_004529e0(unsigned int param_1, int *param_2) {
    struct SampleParams params;
    AddBasicObject(param_1, (unsigned int)param_2);
    params.x = param_2[0];
    params.field_0 = 2;
    params.y = param_2[1];
    PlayInstanceOfSample(*(void **)&FountainSFX[8], 1, 1, &params);
}

// FUNCTION: LEGOLAND 0x00452a30
LEGO_EXPORT void RemoveSoundObject(unsigned int a, unsigned int b, unsigned int c) {
    unsigned char *bb = (unsigned char *)&b;
    struct SampleParams params;
    StandardRemoveObject(a, *(TileId *)&b, c);
    params.field_0 = 2;
    params.x = bb[0];
    params.y = bb[1];
    UnSourceAndFadeAllSamplesFromSource(&params, 0xffffff38);
}

// FUNCTION: LEGOLAND 0x00452a80
void LoadPowerStationSFX(void) {
    unsigned int counter;

    counter = PowerStationSFXRefCount;
    PowerStationSFXRefCount++;
    if (counter == 0) {
        Load_FXList(PowerStationSFX, 2);
    }
}

// FUNCTION: LEGOLAND 0x00452ab0
void KillPowerStationSFX(void) {
    PowerStationSFXRefCount--;
    if (PowerStationSFXRefCount == 0) {
        Kill_FXList(PowerStationSFX, 2);
    }
}

// FUNCTION: LEGOLAND 0x00452ad0
void FUN_00452ad0(unsigned int param_1, int *param_2) {
    struct SampleParams params;
    AddBasicObject(param_1, (unsigned int)param_2);
    params.x = param_2[0];
    params.field_0 = 2;
    params.y = param_2[1];
    PlayInstanceOfSample(*(void **)&PowerStationSFX[0x14], 1, 1, &params);
}

// FUNCTION: LEGOLAND 0x00452b20
void FUN_00452b20(unsigned int param_1, int *param_2) {
    struct SampleParams params;
    AddBasicObject(param_1, (unsigned int)param_2);
    params.x = param_2[0];
    params.field_0 = 2;
    params.y = param_2[1];
    PlayInstanceOfSample(*(void **)&PowerStationSFX[8], 1, 1, &params);
}

// FUNCTION: LEGOLAND 0x00452b70
void LoadDinoSFX(void) {
    unsigned int counter;

    counter = DinoSFXRefCount;
    DinoSFXRefCount++;
    if (counter == 0) {
        Load_FXList(DINO_SFX, 5);
    }
}

// FUNCTION: LEGOLAND 0x00452ba0
void KillDinoSFX(void) {
    DinoSFXRefCount--;
    if (DinoSFXRefCount == 0) {
        Kill_FXList(DINO_SFX, 5);
    }
}

// FUNCTION: LEGOLAND 0x00452bc0
void FUN_00452bc0(unsigned int param_1, int *param_2) {
    struct SampleParams params;
    unsigned int r;
    AddBasicObject(param_1, (unsigned int)param_2);
    params.field_0 = 2;
    params.x = param_2[0];
    params.y = param_2[1];
    r = (unsigned int)rand() % 5;
    PlayInstanceOfSample(PTR_004b8770[r * 3], 1, 1, &params);
}
