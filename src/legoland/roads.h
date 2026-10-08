#pragma once

#include "obj_instance.h"

/* [port] file-scope forward declarations (structs first named in a prototype) */
struct RideQueueEntry;

struct LLIDB_Head;
struct RoadEditArg;
struct RoadPlaceArg;
struct NeighborResult;
struct RoadTile;
struct Sprite;

struct RoadLights {
    unsigned char pad_0[0x8];
    struct Sprite **sprites;
    int *off_x;
    int *off_y;
};

unsigned int FUN_00413970(unsigned short param_1);
void FUN_004132a0(TileId tile, int param_2, int param_3, unsigned int param_4, unsigned int param_5);
void FUN_004133e0(int param_1, int param_2);
void UpdateQueuePathShape(unsigned short param_1, int param_2, int param_3);

int FUN_00413450(int x, int y, struct RideQueueEntry **out);
struct RoadTile *FUN_004134f0(int arg1, int arg2, struct RoadTile *tile);
int FUN_00413520(int x, int y, struct NeighborResult *out);
int FUN_004135d0(int x, int y, struct NeighborResult *out);

void LoadDrivingSchoolRoadsResources(struct LLIDB_Head *head);
void UnloadDrivingSchoolRoadsResources();
void DrivingSchoolRoadsSetEditMode();
void DrivingSchoolRoadsCalcCursor(Element *obj, int *param_2, unsigned int param_3);
struct RoadTile *FUN_00413e30(struct Cursor *cur);
void FUN_00413fa0(unsigned int dummy, struct RoadPlaceArg *param);
void DrivingSchoolRoadsAddObject(struct RoadEditArg *edit, struct RoadPlaceArg *place);
void FUN_00414220(Element *edit, TileId tile, struct Cursor *cursor);
void FUN_00414440(void);
void ZebraCrossingSetEditMode();
void ZebraCrossingCalcCursor(struct RoadEditArg *param_1, unsigned int param_2, unsigned int param_3);
void InitZebraCrossing(struct RoadEditArg *param_1);
void ZebraCrossingAddObject(struct RoadEditArg *edit, struct RoadPlaceArg *place);
