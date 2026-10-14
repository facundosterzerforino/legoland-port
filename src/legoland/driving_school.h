#pragma once

#include "llidb.h"

#include "obj_instance.h"

/* [port] file-scope forward declarations (structs first named in a prototype) */
struct DSCarLayer;

struct DSHead;
struct DSRenderRoot;

void FUN_00405310(TileId tile);
void FUN_00406020(unsigned short arg1, unsigned int arg2);
void LoadDrivingSchoolResources(struct Element *param_1);
void UnloadDrivingSchoolResources();
void DrivingSchoolSetEditMode();
struct Element;
void DrivingSchoolAddObject(struct Element *param_1, int *coords);
void DrivingSchoolCalcCursor(struct Element *param_1, int *param_2, unsigned int param_3);
struct Point;
void DrivingSchoolDCalcCursor(struct Element *param_1, struct Point *param_2);
struct Cursor;
void DrivingSchoolRemoveObject(Element *obj, TileId tile, struct Cursor *param_3);
struct RideSpriteInfo *GetDrivingSchoolSpriteInfo(struct Element *arg1, unsigned short arg2);
void RenderDrivingSchool(Element *obj, unsigned int param_2, unsigned int param_3, unsigned short *tile, void *param_5, unsigned int clip);
void DrivingSchoolUpdate(Element *obj);
int DrivingSchool_Save(struct Element *element);
int FUN_00406050(void);
int DrivingSchool_Load(struct Element *element);