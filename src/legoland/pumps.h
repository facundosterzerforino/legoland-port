#pragma once

#include "legoland.h"

struct Cursor;
#include "llidb.h"

struct PumpSource;

struct PumpTile {
    unsigned char pad_0[8];
    unsigned short var_8;
    unsigned char pad_a[2];
    int var_c;
    int var_10;
    unsigned char var_14;
};

struct PumpNode {
    unsigned short id;
    unsigned short var_2;
    unsigned int var_4;
    unsigned int var_8;
    struct PumpNode *next;
};

struct PumpNode *FUN_00411aa0(unsigned int arg1, unsigned int arg2);
void FUN_00411b20(struct PumpNode *node);
void FUN_00411ba0(unsigned short param_1);
void FUN_00411bd0(void);

void InitDrivingSchoolPumps(struct PumpSource *param_1);
void DrivingSchoolPumpsSetEditMode();
struct PumpTile *FUN_00411dc0(struct Cursor *cursor);
void DrivingSchoolPumpsAddObject(Element *obj, int *coords);
void DrivingSchoolPumpsRemoveObject(void *param_1, TileId tile, struct Cursor *cursor);
void DrivingSchoolPumpsCalcCursor(Element *obj, int *screen, unsigned int param_3);
