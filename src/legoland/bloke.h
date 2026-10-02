#pragma once

#include "legoland.h"
#include "math.h"

/* [port] declared at file scope so prototypes below refer to the same type */
struct MapRect;
struct Bloke;

/* Canonical "bloke" record (one 0xac allocation).  Shared by the AI code
   (bloke_ai.c) and the visitor allocator (bloke.c), which previously each
   declared their own private view of the same object.  Gardeners and mechanics
   (worker.c) are blokes too.
   man3d.c keeps its own thin render-handle view too (its offset 4 is the owned
   Person*, not this list's prev pointer). */
struct Person;
struct Element;
struct BinVFile;
struct WorkOrder;

/* A bloke's reference into a loaded BNV file: the file pointer is re-resolved
   from the owning ride's table by index after a save game is loaded. */
struct BNVRef {
    struct BinVFile *file;
    int index;
};
typedef struct BNVRef BNVRef;

/* A bloke walking along an animated BNV path (rides with scripted movement). */
struct BNVPath {
    /* 0x00 */ struct BinVFile *file;
    /* 0x04 */ unsigned int field_4;
    /* 0x08 */ char name[0x14];
    /* 0x1c */ float z_scale;
    /* 0x20 */ float z_origin;
    /* 0x24 */ float x;
    /* 0x28 */ float y;
    /* 0x2c */ unsigned char pad_2c[0x30 - 0x2c];
    /* 0x30 */ float dx;
    /* 0x34 */ float dy;
    /* 0x38 */ unsigned char pad_38[0x3c - 0x38];
    /* 0x3c */ float z_skew;
    /* 0x40 */ unsigned int frame_index;
    /* 0x44 */ unsigned int field_44;
};
typedef struct BNVPath BNVPath;

struct Bloke {
    struct Bloke *next;
    union {
        struct Bloke *prev;
        struct Person *person; /* ride code: the bloke's 3D render person */
    };
    unsigned char prev_action;
    unsigned char pad_9[0x1];
    unsigned short prev_param;
    unsigned short action;
    unsigned short low_level_action;
    unsigned short pending_action;
    unsigned char pad_12[0x14 - 0x12];
    struct Element *target; /* class of the ride/shop the bloke is heading for */
    struct Element *last_ride; /* class of the last ride the bloke went on */
    unsigned int field_1c;
    int field_20;
    Point dest;
    Point goal; /* where the bloke is walking to */
    unsigned char field_34;
    unsigned char field_35;
    unsigned char field_36;
    unsigned char field_37;
    short field_38;
    short field_3a;
    short screen_x;
    short screen_y;
    unsigned short field_40;
    unsigned short field_42;
    short field_44;
    union {
        short field_46;
        TileId brolly; /* Shark Cafe brolly tile being walked to */
    };
    unsigned char pad_48[0x4a - 0x48];
    short field_4a;
    unsigned short field_4c;
    unsigned char pad_4e[0x50 - 0x4e];
    union {
        int field_50;
        struct WorkOrder *order; /* gardeners and mechanics */
    };
    union {
        unsigned int field_54;
        struct BNVPath *path; /* ride code: scripted path being walked */
        struct BNVRef *bnv; /* ride code: BNV file slot of the bloke's path */
    };
    int field_58;
    int field_5c;
    unsigned char param_action;
    unsigned char pad_61[0x1];
    unsigned short flags;
    unsigned char field_64;
    unsigned char pad_65[0x68 - 0x65];
    Point pos;
    unsigned short height;
    unsigned char dir;
    unsigned char field_73;
    unsigned char frame;
    unsigned char field_75;
    unsigned char pad_76[0x78 - 0x76];
    short field_78;
    short mood;
    unsigned short field_7c;
    unsigned char field_7e;
    unsigned char speed;
    unsigned char field_80;
    unsigned char field_81;
    unsigned char field_82;
    unsigned char first_name_index;
    unsigned char last_name_index;
    unsigned char pad_85[0x88 - 0x85];
    struct Element *favourite_attraction_0;
    struct Element *favourite_attraction_1;
    struct Element *favourite_attraction_2;
    struct Element *favourite_food;
    Navigator nav;
};
typedef struct Bloke Bloke;

/* The 0x124-byte save-game record of a visitor and its Person (scratch buffer DAT_007fda60). */
struct BlokeSave {
    /* 0x00 */ unsigned short action;
    /* 0x02 */ unsigned short low_level_action;
    /* 0x04 */ unsigned short pending_action;
    /* 0x06 */ unsigned char pad_6[0x8 - 0x6];
    /* 0x08 */ int target; /* index into the save game's element list, or -1 */
    /* 0x0c */ int last_ride;
    /* 0x10 */ unsigned int field_1c;
    /* 0x14 */ int field_20;
    /* 0x18 */ Point dest;
    /* 0x20 */ Point goal;
    /* 0x28 */ unsigned int block_34[10]; /* Bloke 0x34..0x5c; [8] is the BNVPath pointer */
    /* 0x50 */ int field_5c;
    /* 0x54 */ unsigned char param_action;
    /* 0x55 */ unsigned char pad_55[1];
    /* 0x56 */ unsigned short flags;
    /* 0x58 */ unsigned char field_64;
    /* 0x59 */ unsigned char pad_59[1];
    /* 0x5a */ short field_78;
    /* 0x5c */ short mood;
    /* 0x5e */ unsigned short field_7c;
    /* 0x60 */ unsigned char field_7e;
    /* 0x61 */ unsigned char speed;
    /* 0x62 */ unsigned char field_80;
    /* 0x63 */ unsigned char field_81;
    /* 0x64 */ unsigned char field_82;
    /* 0x65 */ unsigned char pad_65[3];
    /* 0x68 */ int favourite[4]; /* indices into the save game's element list */
    /* 0x78 */ Point pos;
    /* 0x80 */ unsigned short height;
    /* 0x82 */ unsigned char dir;
    /* 0x83 */ unsigned char field_73;
    /* 0x84 */ unsigned char frame;
    /* 0x85 */ unsigned char field_75;
    /* 0x86 */ unsigned char pad_86[2];
    /* 0x88 */ Navigator nav;
    /* 0x9c */ unsigned int person_8;
    /* 0xa0 */ Vector3 scale;
    /* 0xac */ Point screen;
    /* 0xb4 */ Point offset;
    /* 0xbc */ unsigned int field_34;
    /* 0xc0 */ unsigned int field_30;
    /* 0xc4 */ float field_38;
    /* 0xc8 */ float depth;
    /* 0xcc */ Vector3 rotation;
    /* 0xd8 */ int person_4c;
    /* 0xdc */ unsigned int anim; /* Person.field_88 */
    /* 0xe0 */ unsigned int sort_id;
    /* 0xe4 */ int m[9];
    /* 0x108 */ unsigned int field_7c_p;
    /* 0x10c */ unsigned int field_80_p;
    /* 0x110 */ unsigned int field_8c_p;
    /* 0x114 */ unsigned int field_90_p;
    /* 0x118 */ unsigned int random;
    /* 0x11c */ unsigned short prev_param;
    /* 0x11e */ unsigned char pad_11e[2];
    /* 0x120 */ unsigned char prev_action;
    /* 0x121 */ unsigned char pad_121[3];
};

/* A per-state low-level AI handler (indexed by Bloke.field_e). */
typedef void (*BlokeAction)(Bloke *bloke);

/* Low-level AI handlers (PTR_FUN_004bd34c). */
void LogBlokeRethinking(Bloke *bloke);
void FUN_004838c0(Bloke *bloke);
void FUN_00483ef0(Bloke *bloke);
void FUN_00484090(Bloke *bloke);
void FUN_00483d10(Bloke *bloke);
void FUN_004838e0(Bloke *bloke);
void FUN_00484220(Bloke *bloke);
void FUN_004845d0(Bloke *bloke);
void FUN_00484630(Bloke *bloke);
void FUN_00484790(Bloke *bloke);
void FUN_00483e20(Bloke *bloke);
void FUN_00484470(Bloke *bloke);
void FUN_00484520(Bloke *bloke);
void FUN_004848e0(Bloke *bloke);
void FUN_00483d90(Bloke *bloke);
void FUN_00484350(Bloke *bloke);

struct OverTile;
struct ActionState;
struct BNVPerson;
LEGO_EXPORT int NewDirForAction(Bloke *bloke, unsigned char dir);
LEGO_EXPORT Bloke *GetBlokePtr(int index);
int CheckForPeople(struct MapRect *rect);
LEGO_EXPORT void SetBlokePositionFromBNV(struct BinVFile *file, Bloke *bloke, char *name, int frame, float near_z, float far_z, float *orient);
LEGO_EXPORT BNVPath *NewBNVPath(struct BinVFile *file, unsigned int param_2, char *name, float param_4, float param_5, int *coords);
LEGO_EXPORT int UpdateBlokeFromBNVPath(Bloke *bloke, BNVPath *path);
LEGO_EXPORT int BNVPath_GetDFrame(BNVPath *path);
LEGO_EXPORT void BNVPath_SetDFrame(Bloke *bloke, BNVPath *path, int frame);
Point GetOffsetInDir(unsigned char dir, short dist);
LEGO_EXPORT Point GetTileInDir(Point pos, unsigned char dir);
LEGO_EXPORT int OverNewTile(struct Bloke *bloke, unsigned int x, unsigned int y);
void ResetPathUpdateTimer(void);
void UpdatePathLinks(int force);
int FUN_00482b60(Point *pos);
struct Person;
LEGO_EXPORT char *GetVisitorName(Bloke *bloke);
int FUN_00482cb0(Bloke *bloke);
struct BlokeNameView;
void RandomiseBlokeName(Bloke *bloke);
int FUN_00482df0(Bloke *bloke, int index, int mul);
int GetBlokeMood(Bloke *bloke);
void FUN_00482d60(unsigned int index, int value);
void FUN_00482d70(void);
void DestroyAllBlokes(void);
LEGO_EXPORT Bloke *MakeBloke(int param_1);
LEGO_EXPORT Bloke *NewBlokeWOList(int type);
LEGO_EXPORT Bloke *NewBloke(void);
LEGO_EXPORT int GetBlokeNum(Bloke *bloke);
LEGO_EXPORT void DestroyBloke(Bloke *bloke);
LEGO_EXPORT void DoLowLevelAI(Bloke *bloke);
struct MapRect;
struct BinVFile;
struct BinVObject;
struct Vertex;
void FreeBlokePool(void);
LEGO_EXPORT void InitialiseBlokes(void);
LEGO_EXPORT void RenderPeople(void);
