#pragma once

#include "legoland.h"
#include "llidb.h"
#include "obj_instance.h"

/* [port] file-scope forward declarations (structs first named in a prototype) */
struct EditObject;

struct Cursor;

struct BoatRide {
    /* 0x000 */ unsigned short id;
    /* 0x002 */ unsigned char pad_2[2];
    /* 0x004 */ int tile_x;
    /* 0x008 */ int tile_y;
    /* 0x00c */ int next_x;
    /* 0x010 */ int next_y;
    /* 0x014 */ unsigned int screen_x;
    /* 0x018 */ unsigned int screen_y;
    /* 0x01c */ int step_xy[0xa0];
    /* 0x29c */ unsigned int step_sprite[0x50];
    /* 0x3dc */ unsigned int field_3dc;
    /* 0x3e0 */ unsigned int field_3e0;
    /* 0x3e4 */ unsigned int field_3e4;
    /* 0x3e8 */ unsigned int field_3e8;
    /* 0x3ec */ unsigned int bloke;
    /* 0x3f0 */ struct BoatRide *next;
};

struct BoatRideNode {
    /* 0x00 */ unsigned short id;
    /* 0x02 */ TileId start;
    /* 0x04 */ TileId end;
    /* 0x06 */ unsigned char pad_6[2];
    /* 0x08 */ unsigned int connected;
    /* 0x0c */ unsigned int field_c;
    /* 0x10 */ unsigned int field_10;
    /* 0x14 */ unsigned int bloke_count;
    /* 0x18 */ unsigned int blokes[5];
    /* 0x2c */ struct BoatRideNode *next;
    /* 0x30 */ unsigned int value;
};

struct PathNode {
    /* 0x00 */ TileId tile;
    /* 0x02 */ TileId owner;
    /* 0x04 */ unsigned int dir_mask;
    /* 0x08 */ unsigned int dist_to_end;
    /* 0x0c */ unsigned int visited;
    /* 0x10 */ struct PathNode *next;
    /* 0x14 */ struct PathNode *wave_next;
    /* 0x18 */ struct PathNode *parent;
};

struct MermaidNode {
    /* 0x00 */ TileId tile;
    /* 0x02 */ unsigned short owner;
    /* 0x04 */ struct MermaidNode *next;
};

/* LLIDB "BOATING SCHOOL BOATS" data: boat sprites with per-frame screen offsets. */
int FUN_00418e60(TileId tile, unsigned int bloke);
void FUN_00418fe0(int param_1);
void FUN_00419300(void);
void FUN_004193c0(struct BoatRide *param_1);
struct BoatRide *FUN_00419420(struct BoatRide *param_1);
void FUN_00419520(struct BoatRide *param_1, int param_2);
void BoatingSchoolBuildStepPath(struct BoatRide *ride, int from, int to);
unsigned int FUN_004192d0(struct BoatRide *param_1);
void FUN_00418f90(struct BoatRide *param_1);
void FUN_0041b0d0(unsigned short id, unsigned int value);
struct PathNode *FindBoatPathAt(unsigned int a, unsigned int b);
unsigned int FUN_0041c690(int param_1, int param_2, unsigned short *param_3);
void FUN_0041c4c0(int param_1, int param_2, int param_3, unsigned short *param_4);
void FUN_0041c620(void *param_1, TileId tile, struct Cursor *param_3);
void FUN_0041bab0(int param_1, int param_2, unsigned short *param_3);
void FUN_0041caa0(unsigned short param_1);
void FUN_0041cb20(short param_1);
int FUN_0041c8c0(int a, int b, int c, int d);
void SearchBoatPathConnected(int x, int y, int tx, int ty, TileId *owner, int *found);
void BoatingSchoolMermaidRemoveObject(void *param_1, TileId tile, struct Cursor *param_3);
void FUN_0041a3d0(void *param_1, unsigned int param_2);
void BoatingSchoolRemoveObject(Element *obj, TileId tile, struct Cursor *cursor);

void LoadBoatingSchoolResources(Element *obj);
void UnloadBoatingSchoolResources();
void FUN_0041a000();
void BoatingSchoolAddObject(struct EditObject *obj, int *coords);
void FUN_0041a2f0(int param_1, unsigned int param_2, unsigned int param_3);
void FUN_0041a720();
void RenderBoatingSchool(Element *obj, unsigned int param_2, unsigned int param_3, unsigned short *tile, void *param_5, unsigned int clip);
int BoatingSchool_Save(void);
int BoatingSchool_Load(void);
int FUN_0041b100(int dummy, int arg);
void InitBoatingSchoolMermaid(Element *param_1);
void BoatingSchoolMermaidSetEditMode();
void BoatingSchoolMermaidAddObject(struct EditObject *obj, int *coords);
void FUN_0041b4c0(Element *obj, unsigned int param_2, unsigned int param_3);
unsigned int FUN_0041b6d0(unsigned int param_1, unsigned int param_2);
void FUN_0041b830(Element *arg);
void BoatingSchoolSetEditMode();
void FUN_0041b8e0(Element *obj, int *coords);
void FUN_0041bd40(Element *obj, unsigned int param_2, unsigned int param_3);
void FUN_0041bfb0(unsigned int param_1, int *coords);
void FUN_0041c130(Element *obj, TileId tile, struct Cursor *cursor);
