#pragma once

#include "legoland.h"
#include "math.h"

struct Bloke;

/* One frame of a .pos animation: the position, then the 3x3 orientation. */
struct PosFrame {
    float pos[3];
    float mat[3][3];
};

struct Position {
    int count_inner; /* frames per track */
    int count; /* tracks */
    float field_8;
    float field_c;
    float field_10;
    int field_14;
    int field_18;
    unsigned char pad_1c[0x24 - 0x1c];
    struct PosFrame **entries; /* one array of count_inner frames per track */
};

struct MeshShared {
    /* 0x00 */ int count;
    /* 0x04 */ int field_4;
    /* 0x08 */ void *field_8;
};

struct MeshElem {
    /* 0x00 */ int min_x;
    /* 0x04 */ int min_y;
    /* 0x08 */ int min_z;
    /* 0x0c */ int max_x;
    /* 0x10 */ int max_y;
    /* 0x14 */ int max_z;
    /* 0x18 */ int vert_count;
    /* 0x1c */ void *verts;
    /* 0x20 */ struct MeshShared *shared;
    /* 0x24 */ int norm_count;
    /* 0x28 */ void *norms;
    /* 0x2c */ unsigned char pad_2c[0x38 - 0x2c];
};

/* A bloke animation (the same object as man3d.c's Mesh). */
struct Anim3D {
    int divisor;
    struct MeshElem *elems;
    void *field_8;
};

struct Person {
    struct Person *prev;
    struct Person *next;
    unsigned int field_8;
    struct Bloke *bloke;
    union {
        struct {
            unsigned int field_10;
            unsigned int field_14;
            unsigned int field_18;
        };
        Vector3 scale;
    };
    union {
        struct {
            unsigned int field_1c;
            unsigned int field_20;
        };
        struct Point screen;
    };
    struct Point offset;
    union {
        unsigned int field_2c;
        struct Sprite *sprite;
    };
    unsigned int field_30;
    unsigned int field_34;
    float field_38;
    float depth;
    union {
        struct {
            float field_40;
            float field_44;
            float field_48;
        };
        Vector3 rotation;
    };
    int field_4c;
    void *field_50;
    unsigned int sort_id;
    union {
        int m[9]; /* orientation, 16.16 fixed point */
        float fm[9]; /* the same matrix before conversion */
        struct {
            unsigned char pad_58[0x62 - 0x58];
            unsigned char flags;
            unsigned char field_63;
            unsigned char pad_64[0x68 - 0x64];
            int field_68;
            int field_6c;
            unsigned short field_70;
            unsigned char field_72;
            unsigned char pad_73[1];
            unsigned char field_74;
        };
    };
    unsigned int field_7c;
    unsigned int field_80;
    unsigned int random;
    unsigned int field_88;
    unsigned int field_8c;
    unsigned int field_90;
};
typedef struct Person Person;
struct Bloke;

void FUN_0043f840(struct Person *person);
void FUN_0043f870(struct Person *person);

LEGO_EXPORT struct Person *Find3DPersonFromBloke(struct Bloke *bloke);
LEGO_EXPORT void SetPersonRotation(struct Person *person, float *src);
LEGO_EXPORT void SetPersonDirection(struct Person *person, unsigned int direction);
LEGO_EXPORT void BlokeSetAnim(struct Bloke *bloke, int anim);
LEGO_EXPORT void BlokeSitAnim(struct Bloke *bloke);
LEGO_EXPORT void BlokeSetFrame(struct Bloke *bloke, int frame);
LEGO_EXPORT int PlayBlokeAnim(struct Bloke *bloke);

LEGO_EXPORT struct Position *LoadPos(const char *path);
LEGO_EXPORT void UnloadPos(struct Position *pos);
LEGO_EXPORT void RenderBlokeIn3D(struct Bloke *bloke);
LEGO_EXPORT void SortBlokeIn3D(struct Bloke *bloke);
LEGO_EXPORT void IP_RenderBlokeIn3DNow(struct Bloke *bloke);
LEGO_EXPORT void UpdatePerson(struct Bloke *bloke);
LEGO_EXPORT void Control3DPeople(void);
LEGO_EXPORT void Add3DBlokeToList(struct Bloke *bloke, unsigned int param_2);
LEGO_EXPORT void BlokeWalkAnim(struct Bloke *bloke);
LEGO_EXPORT void BlokePanWithPan(struct Bloke *bloke);
LEGO_EXPORT void BlokeAnimNextFrame(struct Bloke *bloke);
LEGO_EXPORT void BlokeWalkWithPan(struct Bloke *bloke);
LEGO_EXPORT void Render3DPerson(struct Person *person);
void *FUN_004402d0(const char *param_1, const char *param_2);
struct Bloke;
void FUN_004401b0(struct Person *person, struct Bloke *bloke);
void FUN_0043f810(struct Person *person);
void FUN_00440a30(struct Person *person);
LEGO_EXPORT void SetPersonPosition(struct Person *person, unsigned int x, unsigned int y);
LEGO_EXPORT void InitMan(void);
LEGO_EXPORT void UnInitMan(void);
LEGO_EXPORT struct Anim3D *GetBlokeAnim3DFromPerson(struct Person *person);
