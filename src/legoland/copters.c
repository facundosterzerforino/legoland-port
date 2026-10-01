#include <stdlib.h>
#include <string.h>
#include "legoland.h"

#include "bloke.h"
#include "copters.h"
#include "gamemap.h"
#include "globals.h"
#include "llidb.h"
#include "man3d.h"
#include "map_object.h"
#include "obj_instance.h"
#include "objclass.h"
#include "port_asm.h"
#include "print_sprite.h"
#include "render3d.h"
#include "ride_queue.h"
#include "sound_music.h"
#include "sound_sfx.h"

struct CopterSource {
    unsigned short field_0;
};

struct CopterItem {
    unsigned char pad_0[8];
    struct CopterSub *field_8;
};

struct CopterSub {
    unsigned char pad_0[0x50];
    int field_50;
};

struct CopterModel {
    unsigned char pad_0[0x10];
    unsigned int field_10;
};

struct CopterRide {
    unsigned char pad_0[0x14];
    int field_14;
    int field_18;
    unsigned int field_1c;
    unsigned char pad_20[0x64 - 0x20];
    struct CopterModel *field_64;
};

struct CopterEditObject {
    unsigned char pad_0[0xc];
    struct CopterRide *field_c;
};

// Same object as CopterNode, viewed with the two id bytes split apart.
struct CopterSfxNode {
    /* 0x00 */ unsigned char field_0;
    /* 0x01 */ unsigned char field_1;
    /* 0x02 */ unsigned char field_2;
    /* 0x03 */ unsigned char field_3;
    /* 0x04 */ struct CopterNode *next;
    /* 0x08 */ unsigned int field_8;
    /* 0x0c */ int field_c;
    /* 0x10 */ unsigned char field_10;
    /* 0x11 */ unsigned char pad_11[3];
    /* 0x14 */ int field_14;
    /* 0x18 */ struct CopterLayer layer[6];
};

// SFX list table (Helicopter_SFX); Load_FXList fills the sample-def slots.
struct CopterFXList {
    /* 0x00 */ unsigned char pad_0[8];
    /* 0x08 */ void *field_8;
    /* 0x0c */ unsigned char pad_c[0x14 - 0xc];
    /* 0x14 */ void *field_14;
    /* 0x18 */ unsigned char pad_18[0x20 - 0x18];
    /* 0x20 */ void *field_20;
};

struct CopterChainNode {
    /* 0x00 */ struct CopterChainNode *next;
};

struct CopterChainRide {
    /* 0x00 */ unsigned char pad_0[0xcc];
    /* 0xcc */ struct CopterChainNode *chain;
};

// One saved layer record (stride 0x20); field_0 holds a 1-based chain index.
struct CopterSaveLayer {
    /* 0x00 */ int field_0;
    /* 0x04 */ unsigned char pad_4[0x20 - 4];
};

// Save-record view of CopterNode used by Copters_Load.
struct CopterLoadNode {
    /* 0x00 */ unsigned char pad_0[4];
    /* 0x04 */ struct CopterLoadNode *next;
    /* 0x08 */ unsigned char pad_8[0x30 - 8];
    /* 0x30 */ struct CopterSaveLayer slots[6];
};

#include "image_sprite.h"

// GLOBAL: LEGOLAND 0x004b4198
static struct PathPair Copters_PathPairs0[6] = {{0, -1}, {1, 0}, {1, 0}, {1, 0}, {-1, -1}, {-1, -1}};
// GLOBAL: LEGOLAND 0x004b41c8
struct PathTable DAT_004b41c8 = {6, Copters_PathPairs0};
// GLOBAL: LEGOLAND 0x004b41d0
static struct PathPair Copters_PathPairs1[7] = {{0, -1}, {-1, 0}, {0, -1}, {0, -4}, {-3, -1}, {1, 0}, {1, 0}};
// GLOBAL: LEGOLAND 0x004b4208
struct PathTable DAT_004b4208 = {7, Copters_PathPairs1};
// GLOBAL: LEGOLAND 0x004b4210
static struct PathPair Copters_PathPairs2[6] = {{0, -1}, {-1, -1}, {2, -4}, {2, -2}, {-2, 0}, {0, -1}};
// GLOBAL: LEGOLAND 0x004b4240
struct PathTable DAT_004b4240 = {6, Copters_PathPairs2};
// GLOBAL: LEGOLAND 0x004b4248
static struct PathPair Copters_PathPairs3[5] = {{0, -1}, {4, -1}, {0, -1}, {-1, -1}, {-1, -2}};
// GLOBAL: LEGOLAND 0x004b4270
struct PathTable DAT_004b4270 = {5, Copters_PathPairs3};
// GLOBAL: LEGOLAND 0x004b4278
static struct PathPair Copters_PathPairs4[4] = {{0, -1}, {-1, -1}, {0, -1}, {0, -1}};
// GLOBAL: LEGOLAND 0x004b4298
struct PathTable DAT_004b4298 = {4, Copters_PathPairs4};

// GLOBAL: LEGOLAND 0x004b4170
static const char *Copters_LLSNames[10] = {
    "mcop_gs.lls",
    "mcop_b2s.lls",
    "mcop_rs.lls",
    "mcop_b1s.lls",
    "mcop_ys.lls",
    "mcop_b1m.lls",
    "mcop_gm.lls",
    "mcop_rm.lls",
    "mcop_ym.lls",
    "mcop_b2m.lls",
};

// FUNCTION: LEGOLAND 0x00403c40
void FUN_00403c40(struct CopterSource *src) {
    struct CopterNode *node = (struct CopterNode *)malloc(sizeof(struct CopterNode));
    if (node != NULL) {
        memset(node, 0, sizeof(struct CopterNode));
        node->field_0 = src->field_0;
        node->next = DAT_004c11b4;
        DAT_004c11b4 = node;
    }
    FUN_00403e90(node);
}

// FUNCTION: LEGOLAND 0x00403c80
void FUN_00403c80(struct CopterNode *node) {
    struct CopterNode *prev;
    struct CopterNode *cur;

    if (DAT_004c11b4 == node) {
        DAT_004c11b4 = node->next;
    } else {
        cur = DAT_004c11b4->next;
        prev = DAT_004c11b4;
        while (cur != node) {
            prev = prev->next;
            if (prev == NULL) {
                break;
            }
            cur = prev->next;
        }
        if (prev != NULL) {
            prev->next = node->next;
        }
    }
    free(node);
}

// FUNCTION: LEGOLAND 0x00403ce0
void FUN_00403ce0(void) {
    while (DAT_004c11b4 != NULL) {
        FUN_00403c80(DAT_004c11b4);
    }
}

// FUNCTION: LEGOLAND 0x00403d00
struct CopterNode *FUN_00403d00(struct CopterSource *src) {
    struct CopterNode *node;

    if (DAT_004c11b4 != NULL) {
        node = DAT_004c11b4;
        if (DAT_004c11b4->field_0 == src->field_0) {
            return DAT_004c11b4;
        }
        while (1) {
            node = node->next;
            if (node == NULL) {
                break;
            }
            if (node->field_0 == src->field_0) {
                return node;
            }
        }
    }
    return NULL;
}

// FUNCTION: LEGOLAND 0x00403d30
int FUN_00403d30(struct CopterItem *item) {
    struct CopterSub *sub = item->field_8;
    int *ptr = &DAT_004c1124[0];
    int index = 0;

    while (1) {
        if (sub->field_50 == *ptr) {
            break;
        }
        ptr++;
        index++;
        if ((int)ptr < (int)&DAT_004c113c) {
            continue;
        }
        index = -1;
        break;
    }

    sub->field_50 = index;
    return index;
}

// FUNCTION: LEGOLAND 0x00403d60
void FUN_00403d60(struct CopterItem *item) {
    struct CopterSub *sub = item->field_8;
    int index = sub->field_50;

    if (index < 0 || index >= 6) {
        sub->field_50 = 0;
    } else {
        sub->field_50 = DAT_004c1124[index];
    }
}

// FUNCTION: LEGOLAND 0x00403d90
void FUN_00403d90(struct CopterEditObject *param_1) {
    struct CopterRide *ride;
    int i;

    ride = param_1->field_c;
    DAT_004c1198 = ride;
    ride->field_1c |= 0x420;
    DAT_004c1138 = ((struct CopterRide *)DAT_004c1198)->field_64;
    ((struct CopterModel *)DAT_004c1138)->field_10 |= 0x2000;
    for (i = 0; i < 10; i++) {
        DAT_004c113c[i] = LoadSprite(Copters_LLSNames[i], 1);
    }
    // STRING: LEGOLAND 0x004b43d0
    DAT_00830f98 = LoadPos("3ddata\\copters.pos");
    DAT_004c1124[0] = FUN_00412100(&DAT_004b41c8);
    DAT_004c1124[2] = FUN_00412100(&DAT_004b4208);
    DAT_004c1124[1] = FUN_00412100(&DAT_004b4240);
    DAT_004c1124[3] = FUN_00412100(&DAT_004b4270);
    DAT_004c1124[4] = FUN_00412100(&DAT_004b4298);
    DAT_004c1194 = &DAT_004c119c;
    DAT_004c1190 = &DAT_004c11a0;
    DAT_004c1164 = &DAT_004c11a4;
    DAT_004c1168 = &DAT_004c11a8;
    DAT_004c1188 = &DAT_004c11ac;
    // STRING: LEGOLAND 0x004b43bc
    DAT_004c1120 = LoadSprite("cop_base Matte.lls", 1);
    Load_FXList(Helicopter_SFX, 4);
}

// FUNCTION: LEGOLAND 0x00403e90
void FUN_00403e90(struct CopterNode *node) {
    unsigned int handle;
    struct LLS *lls;
    char frame;

    node->layer[1].field_8 = 10;
    node->layer[1].field_c = 3;
    node->layer[1].field_10 = 2;
    node->layer[1].field_14 = 7;
    node->layer[1].flags = 0;
    handle = GetSpriteForLayer((struct LayerContainer *)DAT_004c1138, 3);
    if (handle != 0) {
        lls = (struct LLS *)GetLLSForSprite((struct SpriteLLS *)handle);
        if (lls != NULL) {
            frame = (char)lls->frame_count;
            node->layer[1].field_1c = frame;
            node->layer[1].field_4 = frame - 1;
        }
    }
    node->layer[0].field_8 = 2;
    node->layer[0].field_c = 1;
    node->layer[0].field_10 = 0;
    node->layer[0].field_14 = 6;
    node->layer[0].flags = 0;
    handle = GetSpriteForLayer((struct LayerContainer *)DAT_004c1138, 1);
    if (handle != 0) {
        lls = (struct LLS *)GetLLSForSprite((struct SpriteLLS *)handle);
        if (lls != NULL) {
            frame = (char)lls->frame_count;
            node->layer[0].field_1c = frame;
            node->layer[0].field_4 = frame - 1;
        }
    }
    node->layer[2].field_c = 0xb;
    node->layer[2].field_8 = 4;
    node->layer[2].field_10 = 4;
    node->layer[2].field_14 = 8;
    node->layer[2].flags = 0;
    handle = GetSpriteForLayer((struct LayerContainer *)DAT_004c1138, 0xb);
    if (handle != 0) {
        lls = (struct LLS *)GetLLSForSprite((struct SpriteLLS *)handle);
        if (lls != NULL) {
            frame = (char)lls->frame_count;
            node->layer[2].field_1c = frame;
            node->layer[2].field_4 = frame - 1;
        }
    }
    node->layer[3].field_c = 6;
    node->layer[3].field_8 = 5;
    node->layer[3].field_10 = 3;
    node->layer[3].field_14 = 5;
    node->layer[3].flags = 0;
    handle = GetSpriteForLayer((struct LayerContainer *)DAT_004c1138, 6);
    if (handle != 0) {
        lls = (struct LLS *)GetLLSForSprite((struct SpriteLLS *)handle);
        if (lls != NULL) {
            frame = (char)lls->frame_count;
            node->layer[3].field_1c = frame;
            node->layer[3].field_4 = frame - 1;
        }
    }
    node->layer[4].field_8 = 8;
    node->layer[4].field_c = 7;
    node->layer[4].field_10 = 1;
    node->layer[4].field_14 = 9;
    node->layer[4].flags = 0;
    handle = GetSpriteForLayer((struct LayerContainer *)DAT_004c1138, 7);
    if (handle != 0) {
        lls = (struct LLS *)GetLLSForSprite((struct SpriteLLS *)handle);
        if (lls != NULL) {
            frame = (char)lls->frame_count;
            node->layer[4].field_1c = frame;
            node->layer[4].field_4 = frame - 1;
        }
    }
    FUN_004049a0(node, 1);
}

// FUNCTION: LEGOLAND 0x00404040
void FUN_00404040(void) {
    struct Sprite **current;

    if (DAT_004c1120 != NULL) {
        KillSprite(DAT_004c1120);
    }

    current = DAT_004c113c;
    while ((int)current < (int)&DAT_004c1164) {
        if (*current != NULL) {
            KillSprite(*current);
        }
        current++;
    }

    if (DAT_004c1124[0] != 0) {
        FUN_00412290((struct Sprite *)DAT_004c1124[0]);
    }
    if (DAT_004c1124[1] != 0) {
        FUN_00412290((struct Sprite *)DAT_004c1124[1]);
    }
    if (DAT_004c1124[3] != 0) {
        FUN_00412290((struct Sprite *)DAT_004c1124[3]);
    }
    if (DAT_004c1124[4] != 0) {
        FUN_00412290((struct Sprite *)DAT_004c1124[4]);
    }
    if (DAT_004c1124[2] != 0) {
        FUN_00412290((struct Sprite *)DAT_004c1124[2]);
    }

    FUN_00403ce0();
    Kill_FXList(Helicopter_SFX, 4);
    UnloadPos(DAT_00830f98);
}

// FUNCTION: LEGOLAND 0x004040f0
void FUN_004040f0(struct CopterNode *node, int index, unsigned int param_3) {
    struct CopterLayer *layer;
    struct LLS *lls;
    struct Point off;
    struct Point sc;
    struct Sprite *sprite;
    int a;
    int b;

    layer = &node->layer[index];
    if (layer->flags & 1) {
        b = layer->field_c;
        a = layer->field_14;
    } else {
        b = layer->field_8;
        a = layer->field_10;
    }
    lls = GetLLSForLayer(DAT_004c1138, layer->field_8);
    if (lls != NULL) {
        LLSStop((unsigned int)lls);
        LLSSetFrame(lls, 0);
    }
    lls = GetLLSForLayer(DAT_004c1138, layer->field_10);
    if (lls != NULL) {
        LLSStop((unsigned int)lls);
        LLSSetFrame(lls, 0);
    }
    lls = GetLLSForLayer(DAT_004c1138, layer->field_c);
    if (lls != NULL) {
        LLSStop((unsigned int)lls);
        LLSSetFrame(lls, 0);
    }
    sc = GetScreenCoordsForObject((TileId *)node, (struct Ride *)DAT_004c1198);
    off = GetRenderOffsetForLayer(((struct Ride *)DAT_004c1198)->layer, b);
    AdjustOffsetForViewMode(&off);
    sprite = GetSpriteForLayer(((struct Ride *)DAT_004c1198)->layer, b);
    if (sprite != NULL) {
        lls = (struct LLS *)GetLLSForSprite((struct SpriteLLS *)sprite);
        if (lls != NULL) {
            LLSSetFrame(lls, layer->field_4);
        }
    }
    PrintSprite(sprite, sc.x + off.x, sc.y + off.y, param_3, NULL);
    if (layer->rider != NULL) {
        IP_RenderBlokeIn3DNow(layer->rider->rider);
    }
    sprite = DAT_004c113c[a];
    if (sprite != NULL) {
        lls = (struct LLS *)GetLLSForSprite((struct SpriteLLS *)sprite);
        if (lls != NULL) {
            LLSSetFrame(lls, layer->field_4);
        }
        PrintSprite(sprite, sc.x + off.x, sc.y + off.y, param_3, NULL);
    }
}

// FUNCTION: LEGOLAND 0x00404290
void FUN_00404290(Element *obj, int unused, int unused2, TileId *tile, int unused3, unsigned int clip) {
    struct Ride *ride = obj->ride;
    struct CopterNode *node;
    struct Point pos;

    node = FUN_00403d00((struct CopterSource *)tile);
    if (node != NULL) {
        struct RideNode *rn;
        struct Point base;
        int d;
        int key;

        RenderItems_New();
        DAT_004c119c = NULL;
        DAT_004c11a0 = NULL;
        DAT_004c11a4 = NULL;
        DAT_004c11a8 = NULL;
        DAT_004c11ac = NULL;
        DAT_004c11b0 = NULL;
        base.y = node->field_1 + ((struct Ride *)DAT_004c1198)->y;
        for (rn = ((struct Ride *)DAT_004c1198)->riders; rn != NULL; rn = rn->next) {
            if ((rn->rider->flags & 0x80) == 0) {
                key = rn->person->field_20;
                d = base.y - (rn->rider->pos.y >> 8);
                if (d >= 5) {
                    AddBlokeToRenderList(DAT_004c1190, (struct BlokeRenderSrc *)rn, key);
                }
                if (d >= 3 && d <= 4) {
                    AddBlokeToRenderList(DAT_004c1168, (struct BlokeRenderSrc *)rn, key);
                }
                if (d >= 0 && d <= 2) {
                    AddBlokeToRenderList(DAT_004c1194, (struct BlokeRenderSrc *)rn, key);
                }
            }
        }
        FUN_004040f0(node, 0, clip);
        FUN_004040f0(node, 2, clip);
        RenderBlokeList((struct BlokeListHead *)DAT_004c1164);
        RenderBlokeList((struct BlokeListHead *)DAT_004c1190);
        FUN_004040f0(node, 3, clip);
        FUN_004040f0(node, 4, clip);
        RenderBlokeList((struct BlokeListHead *)DAT_004c1168);
        RenderBlokeList((struct BlokeListHead *)DAT_004c1188);
        FUN_004040f0(node, 1, clip);
        RenderBlokeList((struct BlokeListHead *)DAT_004c1194);
    }
    pos = GetScreenCoordsForObject(tile, ride);
    {
        struct Point off = GetRenderOffsetForLayer(ride->layer, 0);
        AdjustOffsetForViewMode(&off);
        PrintSprite(DAT_004c1120, pos.x + off.x, pos.y + off.y, clip, 0);
    }
}

// FUNCTION: LEGOLAND 0x00404450
void FUN_00404450(void) {
    EditMode.unk0 = 1;
    EditMode.unk8 = DAT_004c1198;
    DefaultCursor(&EditCursor);
    SetEditCursorFootPrint((unsigned char *)EditMode.unk8 + 0x3c);
}

// FUNCTION: LEGOLAND 0x00404490
unsigned int *FUN_00404490(struct CopterEditObject *editobj, unsigned short uid) {
    struct CopterRide *ride = editobj->field_c;

    DAT_004c1170 = (int)ride->field_64;
    DAT_004c1174 = ride->field_14;
    DAT_004c1178 = ride->field_18;
    DAT_004c117c = uid;
    ride->field_64->field_10 |= 0x2000;

    if (FUN_00403d00((struct CopterSource *)&uid) != 0) {
        HideLayer(DAT_004c1138, 1);
        HideLayer(DAT_004c1138, 2);
        HideLayer(DAT_004c1138, 3);
        HideLayer(DAT_004c1138, 0xa);
        HideLayer(DAT_004c1138, 6);
        HideLayer(DAT_004c1138, 5);
        HideLayer(DAT_004c1138, 7);
        HideLayer(DAT_004c1138, 8);
        HideLayer(DAT_004c1138, 0xb);
        HideLayer(DAT_004c1138, 4);
    }

    return (unsigned int *)&DAT_004c1170;
}

// FUNCTION: LEGOLAND 0x00404580
void FUN_00404580(Element *obj, TileId tile, struct Cursor *cursor) {
    struct CopterSfxNode *node;
    struct SampleSource src;
    unsigned int x;
    unsigned int y;

    node = (struct CopterSfxNode *)FUN_00403d00((struct CopterSource *)&tile);
    if (node != NULL) {
        FUN_00403c80((struct CopterNode *)node);
    }
    StandardRemoveObject(obj, tile, cursor);
    RemoveAllBlokesFromRide(obj->ride, tile);
    x = node->field_0;
    y = node->field_1;
    src.type = 2;
    src.field_8 = x;
    src.field_c = y;
    UnSourceAndFadeAllSamplesFromSource(&src, -200);
}

// FUNCTION: LEGOLAND 0x00404600
void FUN_00404600(Element *obj, int *coords) {
    TileId tile;

    tile.pos.x = coords[0];
    tile.pos.y = coords[1];
    AddBasicObject(obj, coords);
    FUN_00403c40((struct CopterSource *)&tile);
}

// FUNCTION: LEGOLAND 0x00404630
void FUN_00404630(struct CopterNode *node, int index) {
    /* Port [copters:asm]: the original is inline asm (x87). Poses the rider of helicopter seat layer `index`: places
     * the rider's person at the seat's screen position plus the recorded animation offset, and sets the person's
     * orientation from the current frame of the copters.pos animation (track `track` of DAT_00830f98). */
    static const int src_col[3] = {0, 2, 1};
    static const int row_sign[3] = {1, -1, -1};
    static const int col_sign[3] = {-1, 1, 1};
    struct CopterLayer *layer = &node->layer[index];
    struct Point screen;
    struct Point sprite_off;
    struct Point seat_off;
    struct Point person_pos;
    struct Sprite *sprite;
    struct Person *person;
    struct PosFrame *frame;
    int sprite_layer;
    int track;
    int y_bias;
    int i;
    int j;

    screen = GetScreenCoordsForObject((TileId *)node, (struct Ride *)DAT_004c1198);
    if (layer->rider == NULL) {
        return;
    }
    sprite_layer = layer->field_c;
    /* index is 0..4 for every caller; anything else falls through with track 1 and bias = index (as the original) */
    track = 1;
    y_bias = index;
    switch (index) {
    case 0:
        track = 3;
        y_bias = 0xd7;
        break;
    case 1:
        track = 0;
        y_bias = 0xeb;
        break;
    case 2:
        track = 4;
        y_bias = 0xe1;
        break;
    case 3:
        track = 1;
        y_bias = 0xe6;
        break;
    case 4:
        track = 2;
        y_bias = 0xe6;
        break;
    }
    sprite_off = GetRenderOffsetForLayer((struct Sprite *)DAT_004c1138, sprite_layer);
    sprite = GetSpriteForLayer((struct Sprite *)DAT_004c1138, sprite_layer);
    AdjustOffsetForViewMode(&sprite_off);
    /* layer->field_4 is the current animation frame (a signed char) */
    frame = &DAT_00830f98->entries[track][(signed char)layer->field_4];
    seat_off.x = 0;
    seat_off.y = (int)frame->pos[1] + y_bias;
    AdjustOffsetForViewMode(&seat_off);
    seat_off.x = sprite->width >> 1;
    person_pos.x = screen.x + sprite_off.x + seat_off.x;
    person_pos.y = screen.y + sprite_off.y + seat_off.y;
    AdjustBlokePosition(&person_pos);
    person = layer->rider->person;
    SetPersonPosition(person, person_pos.x, person_pos.y);

    /* The frame's third matrix row is rebuilt as a cross product of the first two (this edits the shared frame data) */
    frame->mat[2][0] = frame->mat[0][2] * frame->mat[1][1] - frame->mat[1][2] * frame->mat[0][1];
    frame->mat[2][1] = frame->mat[1][2] * frame->mat[0][0] - frame->mat[1][0] * frame->mat[0][2];
    frame->mat[2][2] = frame->mat[1][0] * frame->mat[0][1] - frame->mat[0][0] * frame->mat[1][1];
    /* person->m is the orientation in 16.16 fixed point, with axes 1 and 2 swapped and some signs flipped */
    for (i = 0; i < 3; i++) {
        for (j = 0; j < 3; j++) {
            person->m[j * 3 + i] = row_sign[j] * col_sign[i] * PortRound(frame->mat[i][src_col[j]] * 65536.0f);
        }
    }
}

// FUNCTION: LEGOLAND 0x00404860
void FUN_00404860(struct CopterNode *node, int index) {
    struct CopterLayer *layer = &node->layer[index];

    if (layer->flags & 1) {
        layer->field_4 = layer->field_4 + 1;
        if (layer->field_4 < layer->field_1c) {
            return;
        }
        layer->field_4 = 0;
        layer->field_1d = layer->field_1d - 1;
        if (layer->field_1d >= 0) {
            return;
        }
        layer->flags &= 0xfffffffe;
    }
}

// FUNCTION: LEGOLAND 0x004048a0
unsigned int FUN_004048a0(unsigned int param) {
    ResumeSinglyPausedSample((struct Sample *)param);
    return 0;
}

// FUNCTION: LEGOLAND 0x004048b0
void FUN_004048b0(struct CopterSfxNode *node) {
    struct CopterFXList *fx = (struct CopterFXList *)Helicopter_SFX;
    struct SampleParams params;
    struct Sample *sample;

    if (node->layer[1].rider != 0) node->layer[1].flags |= 1;
    if (node->layer[0].rider != 0) node->layer[0].flags |= 1;
    if (node->layer[2].rider != 0) node->layer[2].flags |= 1;
    if (node->layer[3].rider != 0) node->layer[3].flags |= 1;
    if (node->layer[4].rider != 0) node->layer[4].flags |= 1;

    node->field_3 = node->field_2;
    node->field_2 = 0;
    node->field_8 = (node->field_8 & ~0x4000u) | 1;
    node->field_c = 2;
    node->layer[1].field_1d = 3;
    node->layer[0].field_1d = 3;
    node->layer[2].field_1d = 3;
    node->layer[3].field_1d = 3;
    node->layer[4].field_1d = 3;
    node->layer[1].field_4 = 0;
    node->layer[0].field_4 = 0;
    node->layer[2].field_4 = 0;
    node->layer[3].field_4 = 0;
    node->layer[4].field_4 = 0;

    params.field_8 = node->field_0;
    params.field_c = node->field_1;
    params.field_0 = 2;

    PlayInstanceOfSample(fx->field_8, 0, 1, &params);
    sample = PlayInstanceOfSample(fx->field_14, 1, 1, &params);
    PauseSingleSample(sample);
    AddSFX_Callback((struct CallbackEntry *)sample, 0xb54,
        (unsigned int (*)(struct CallbackEntry *))FUN_004048a0);
}

// FUNCTION: LEGOLAND 0x004049a0
void FUN_004049a0(struct CopterNode *node, int param) {
    struct CopterFXList *fx = (struct CopterFXList *)Helicopter_SFX;
    struct SampleParams params;
    struct CopterSfxNode *n = (struct CopterSfxNode *)node;

    n->layer[1].flags &= ~1u;
    n->layer[0].flags &= ~1u;
    n->layer[2].flags &= ~1u;
    n->layer[3].flags &= ~1u;
    n->layer[4].flags &= ~1u;
    n->field_c = 0;
    n->field_10 = 0;
    n->field_2 = 0;
    n->layer[1].field_4 = n->layer[1].field_1c - 1;
    n->layer[0].field_4 = n->layer[0].field_1c - 1;
    n->layer[2].field_4 = n->layer[2].field_1c - 1;
    n->layer[3].field_4 = n->layer[3].field_1c - 1;
    n->layer[4].field_4 = n->layer[4].field_1c - 1;
    n->layer[1].rider = 0;
    n->layer[0].rider = 0;
    n->layer[2].rider = 0;
    n->layer[3].rider = 0;
    n->layer[4].rider = 0;
    n->field_8 &= ~0x4001u;
    if (param == 0) {
        params.field_8 = n->field_0;
        params.field_0 = 2;
        params.field_c = n->field_1;
        UnSourceAndFadeAllSamplesFromSource(&params, -200);
        PlayInstanceOfSample(fx->field_20, 0, 1, &params);
    }
}

// FUNCTION: LEGOLAND 0x00404a90
void FUN_00404a90(struct CopterNode *node) {
    node->field_c = node->field_c - 1;
    if (node->field_c < 0) {
        node->field_c = 2;
        FUN_00404860(node, 1);
        FUN_00404860(node, 0);
        FUN_00404860(node, 2);
        FUN_00404860(node, 3);
        FUN_00404860(node, 4);
        if ((node->field_8 & 1) && !(node->layer[1].flags & 1) && !(node->layer[0].flags & 1) &&
            !(node->layer[2].flags & 1) && !(node->layer[3].flags & 1) &&
            !(node->layer[4].flags & 1)) {
            if (GetAllBlokesOffRide(DAT_004c1198, node->field_0) != 0) {
                FUN_004049a0(node, 0);
            }
            return;
        }
    }
    FUN_00404630(node, 1);
    FUN_00404630(node, 0);
    FUN_00404630(node, 2);
    FUN_00404630(node, 3);
    FUN_00404630(node, 4);
    if (!(node->field_8 & 1)) {
        if (node->field_8 & 0x4000) {
            if (node->field_2 == node->field_10) {
                node->field_8 &= ~0x4000u;
                FUN_004048b0((struct CopterSfxNode *)node);
                return;
            }
        } else if (node->field_2 != 0) {
            if (node->field_14 == 0) {
                node->field_8 |= 0x4000;
                Ride_SetFlagToNotLetAnyoneOn(node);
            } else {
                node->field_14 = node->field_14 - 1;
            }
        }
    }
    Put3DBlokesOnRide2(DAT_004c1198, (Element *)node);
}

// FUNCTION: LEGOLAND 0x00404bc0
void FUN_00404bc0(void) {
    struct CopterNode *node = DAT_004c11b4;
    if (node == NULL) {
        return;
    }
    do {
        FUN_00404a90(node);
        node = node->next;
    } while (node != NULL);
}

// FUNCTION: LEGOLAND 0x00404be0
void FUN_00404be0(struct Element *elem) {
    unsigned int x;
    unsigned int y;
    struct Bloke *b;
    struct CopterNode *cn;
    struct CopterChainNode *link;
    struct Ride *ride = elem->ride;
    struct RideNode *next;
    struct RideNode *node;
    int spr;
    int spr2;

    FUN_00404bc0();
    node = ride->riders;
    while (node != NULL) {
        next = node->next;
        b = node->rider;
        cn = FUN_00403d00((struct CopterSource *)&node->tile);
        if (cn == NULL) {
            break;
        }
        x = node->tile.pos.x + ride->x;
        y = node->tile.pos.y + ride->y;
        if (b->field_e == 0) {
            switch (b->param_action) {
            case 0:
                b->flags |= 8;
                link = (struct CopterChainNode *)node;
                cn->layer[FUN_00404f20(link, (struct CopterSource *)&node->tile)].rider = node;
                cn->field_10++;
                cn->field_14 = 0xb4;
                b->field_58 = 0;
                b->param_action++;
                break;
            case 1:
                link = (struct CopterChainNode *)node;
                switch (FUN_00404f20(link, (struct CopterSource *)&node->tile)) {
                case 0:
                    spr = DAT_004c1124[2];
                    break;
                case 1:
                    spr = DAT_004c1124[0];
                    break;
                case 2:
                    spr = DAT_004c1124[1];
                    break;
                case 3:
                    spr = DAT_004c1124[3];
                    break;
                case 4:
                    spr = DAT_004c1124[4];
                    break;
                }
                FUN_004122d0((struct RideSlotArg *)spr, (struct RideSlot *)b);
                FUN_00403d30((struct CopterItem *)link);
                break;
            case 2:
                FUN_00412300((struct QueueTable *)DAT_004c1124[(int)FUN_004122f0((struct RideSlot *)b)], x, y, b);
                break;
            case 3:
            case 7:
                b->param_action++;
                break;
            case 4:
                b->param_action++;
                cn->field_2++;
                if ((short)cn->field_2 == ride->seats) {
                    FUN_004048b0((struct CopterSfxNode *)cn);
                }
                break;
            case 5:
                b->flags |= 0x80;
                BlokeSitAnim(b);
                BlokeSetFrame(b, 0);
                break;
            case 6:
                BlokeWalkAnim(b);
                BlokeSetFrame(b, 0);
                b->flags &= 0xff7f;
                b->param_action++;
                break;
            case 8:
                link = (struct CopterChainNode *)node;
                switch (FUN_00404f20(link, (struct CopterSource *)&node->tile)) {
                case 0:
                    spr2 = DAT_004c1124[2];
                    break;
                case 1:
                    spr2 = DAT_004c1124[0];
                    break;
                case 2:
                    spr2 = DAT_004c1124[1];
                    break;
                case 3:
                    spr2 = DAT_004c1124[3];
                    break;
                case 4:
                    spr2 = DAT_004c1124[4];
                    break;
                }
                FUN_004122a0((struct RideSlotArg *)spr2, (struct RideSlot *)b);
                FUN_00403d30((struct CopterItem *)link);
                break;
            case 9:
                FUN_00412300((struct QueueTable *)DAT_004c1124[(int)FUN_004122f0((struct RideSlot *)b)], x, y, b);
                break;
            case 10:
                b->dest.x = (x << 8) + 0x80;
                b->dest.y = (y << 8) + 0x80;
                b->field_73 = CalcMoveLine(b->pos, b->dest, &b->nav) + 0x10;
                b->field_e = 7;
                NewDirForAction(b, (unsigned char)((b->field_73 >> 5) + 3));
                b->param_action++;
                break;
            case 11:
                RemoveBlokeFromRide(ride, node);
                b->flags &= 0xfff7;
                cn->field_3--;
                if (cn->field_3 == 0) {
                    Ride_ClearFlagToNotLetAnyoneOn(cn);
                }
                break;
            }
        }
        node = next;
    }
}

// FUNCTION: LEGOLAND 0x00404f20
unsigned int FUN_00404f20(struct CopterChainNode *node, struct CopterSource *id) {
    struct CopterChainNode *cur;
    unsigned int idx = 0;

    for (cur = ((struct CopterChainRide *)DAT_004c1198)->chain; cur != NULL; cur = cur->next) {
        struct CopterSource *p = (struct CopterSource *)((char *)cur + 0xc);
        if (memcmp(p, id, 2) == 0) {
            if (cur == node) {
                return idx;
            }
            idx++;
        }
    }
    return 0;
}

// FUNCTION: LEGOLAND 0x00404f60
LEGO_EXPORT int Copters_Save(void) {
    struct CopterNode *node;
    struct RideNode *cur;
    struct RideNode *rider;
    struct RideNode *saved[6];
    int index;
    int i;
    unsigned int one;
    unsigned int zero;

    one = 1;
    zero = 0;
    node = DAT_004c11b4;
    if (DAT_004c11b4 != NULL) {
        while (node != NULL) {
            if (SaveGameWrite(&one, 4) == 0) {
                return 0;
            }
            for (i = 0; i < 6; i++) {
                rider = node->layer[i].rider;
                index = 0;
                saved[i] = rider;
                for (cur = ((struct Ride *)DAT_004c1198)->riders; cur != NULL; cur = cur->next) {
                    if (cur == rider) {
                        break;
                    }
                    index++;
                }
                if (cur != NULL) {
                    node->layer[i].rider = (struct RideNode *)(index + 1);
                } else {
                    node->layer[i].rider = NULL;
                }
            }
            if (SaveGameWrite(node, 0xd8) == 0) {
                return 0;
            }
            for (i = 0; i < 6; i++) {
                node->layer[i].rider = saved[i];
            }
            node = node->next;
        }
    }
    return SaveGameWrite(&zero, 4) != 0;
}

// FUNCTION: LEGOLAND 0x00405050
LEGO_EXPORT int Copters_Load(void) {
    int count;
    struct CopterLoadNode *prev = NULL;
    struct CopterLoadNode *node;
    struct CopterChainNode *chain;
    int n;
    int i;

    if (!SaveGameRead(&count, 4)) {
        return 0;
    }

    while (count != 0) {
        node = (struct CopterLoadNode *)malloc(0xd8);
        if (!SaveGameRead(node, 0xd8)) {
            return 0;
        }
        node->next = NULL;
        if (prev != NULL) {
            prev->next = node;
        } else {
            DAT_004c11b4 = (struct CopterNode *)node;
        }
        prev = node;

        for (i = 6; i != 0; i--) {
            n = node->slots[6 - i].field_0;
            chain = ((struct CopterChainRide *)DAT_004c1198)->chain;
            if (n != 0) {
                while (--n != 0) {
                    chain = chain->next;
                }
                node->slots[6 - i].field_0 = (int)chain;
            } else {
                node->slots[6 - i].field_0 = 0;
            }
        }

        if (!SaveGameRead(&count, 4)) {
            return 0;
        }
    }

    return 1;
}

// FUNCTION: LEGOLAND 0x00405110
void FUN_00405110(struct ClassNode *name, struct CallbackTable *interfaces) {
    // STRING: LEGOLAND 0x004b43e4
    if (_stricmp("COPTERS", name->name) == 0) {
        interfaces->cb_a4 = FUN_00403d90;
        interfaces->cb_8c = FUN_00404450;
        interfaces->cb_98 = FUN_00404600;
        interfaces->cb_9c = FUN_00404580;
        interfaces->cb_a8 = FUN_00404be0;
        interfaces->cb_a0 = FUN_00404490;
        interfaces->cb_b0 = FUN_00404290;
        interfaces->cb_ac = FUN_00404040;
        interfaces->cb_bc = Copters_Save;
        interfaces->cb_b8 = Copters_Load;
    }
}
