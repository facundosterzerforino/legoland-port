#include <stdlib.h>
#include <string.h>
#include "legoland.h"

#include "bloke.h"
#include "gamemap.h"
#include "globals.h"
#include "llidb.h"
#include "man3d.h"
#include "map_object.h"
#include "math.h"
#include "obj_instance.h"
#include "objclass.h"
#include "print_sprite.h"
#include "render3d.h"
#include "sound_music.h"
#include "space_tower.h"

struct SpaceTowerRideNode;

struct SpaceTowerSeat {
    /* 0x00 */ unsigned int flags;
    /* 0x04 */ unsigned char pad_4[4];
    /* 0x08 */ int pos;
    /* 0x0c */ int state;
    /* 0x10 */ int field_10;
    /* 0x14 */ int descending;
    /* 0x18 */ struct SpaceTowerRideNode *rider_slot0;
    /* 0x1c */ struct SpaceTowerRideNode *rider_slot1;
    /* 0x20 */ signed char delta;
    /* 0x21 */ unsigned char pad_21[3];
};

struct SpaceTowerCar {
    /* 0x00 */ unsigned short var_0;
    /* 0x02 */ unsigned char var_2;
    /* 0x03 */ unsigned char var_3;
    /* 0x04 */ unsigned char var_4;
    /* 0x05 */ unsigned char pad_5[3];
    /* 0x08 */ struct SpaceTowerCar *next;
    /* 0x0c */ unsigned int var_c;
    /* 0x10 */ unsigned int var_10;
    /* 0x14 */ struct SpaceTowerSeat seats[4];
    /* 0xa4 */ unsigned char flags_a4[8];
    /* 0xac */ unsigned char var_ac;
    /* 0xad */ unsigned char var_ad;
    /* 0xae */ unsigned char pad_ae[2];
    /* 0xb0 */ unsigned int var_b0;
    /* 0xb4 */ unsigned char pad_b4[24];
    /* 0xcc */ unsigned int var_cc;
};

struct AnimStep {
    /* 0x00 */ int x;
    /* 0x04 */ int y;
    /* 0x08 */ int dx;
    /* 0x0c */ int dy;
};

struct AnimEntry {
    /* 0x00 */ int count;
    /* 0x04 */ struct AnimStep *steps;
};

struct AnimLayout {
    /* 0x00 */ int count;
    /* 0x04 */ struct AnimEntry **entries;
};

struct SpaceTowerRideNode {
    /* 0x00 */ struct SpaceTowerRideNode *next;
    /* 0x04 */ unsigned char pad_4[4];
    /* 0x08 */ struct Bloke *bloke;
    /* 0x0c */ union {
        unsigned short id;
        struct {
            unsigned char x;
            unsigned char y;
        } coord;
    };
    /* 0x0e */ unsigned char pad_e[2];
    /* 0x10 */ struct Person *person;
};

struct SpaceTowerRide {
    /* 0x00 */ unsigned char pad_0[0xc];
    /* 0x0c */ int x;
    /* 0x10 */ int y;
    /* 0x14 */ unsigned char pad_14[0x1c - 0x14];
    /* 0x1c */ unsigned int flags;
    /* 0x20 */ unsigned char pad_20[0x24 - 0x20];
    /* 0x24 */ char field_24;
    /* 0x25 */ char field_25;
    /* 0x26 */ unsigned char pad_26[0x2e - 0x26];
    /* 0x2e */ short seats;
    /* 0x30 */ unsigned char pad_30[0x64 - 0x30];
    /* 0x64 */ void *layers;
    /* 0x68 */ unsigned char pad_68[0xcc - 0x68];
    /* 0xcc */ struct SpaceTowerRideNode *list;
};

struct SpaceTowerCtx {
    /* 0x00 */ unsigned char pad_0[0xc];
    /* 0x0c */ struct SpaceTowerRide *ride;
};

struct ListNode {
    unsigned char pad_0[8];
    struct ListNode *next;
};

struct FadeParams {
    unsigned int field_0;
    unsigned char pad_4[4];
    unsigned int field_8;
    unsigned int field_c;
};

#include "image_sprite.h"

// FUNCTION: LEGOLAND 0x0043a7a0
struct Point FUN_0043a7a0(struct AnimLayout *layout, int seg, int step) {
    struct Point sum;
    struct AnimEntry *entry;
    struct AnimEntry **pp;
    struct AnimStep *p;
    int i;
    int n;

    sum.x = 0;
    sum.y = 0;
    pp = layout->entries;
    for (i = 0; i < seg + 1; i++) {
        entry = *pp;
        if (i != seg) {
            n = entry->count;
        } else {
            n = step + 1;
        }
        if (n > 0) {
            p = entry->steps;
            do {
                sum.x += p->dx + (p->x << 8);
                sum.y += (p->y << 8) + p->dy;
                p++;
            } while (--n);
        }
        pp++;
    }
    return sum;
}

// FUNCTION: LEGOLAND 0x0043a820
void FUN_0043a820(struct AnimEntry *param_1, struct SpaceTowerRideNode *param_2) {
    struct Bloke *bloke;
    struct SpaceTowerRide *ride;
    struct Point base;
    char dir;

    bloke = param_2->bloke;
    base = FUN_0043a7a0((struct AnimLayout *)DAT_004b7758[bloke->field_50].anim_layout, bloke->field_4a, bloke->field_38);
    ride = (struct SpaceTowerRide *)SpaceTowerRideObj;
    base.x += (param_2->coord.x + ride->x) << 8;
    base.y += (param_2->coord.y + ride->y) << 8;
    bloke->dest.x = base.x;
    bloke->dest.y = base.y;
    dir = CalcMoveLine(bloke->pos, bloke->dest, &bloke->nav);
    bloke->low_level_action = 7;
    bloke->field_73 = dir + 0x10;
    NewDirForAction(bloke, ((unsigned char)(dir + 0x10) >> 5) + 3);
    param_2->bloke->field_38++;
}

// FUNCTION: LEGOLAND 0x0043a8c0
void FUN_0043a8c0(struct SpaceTowerRideNode *param_1) {
    struct Bloke *bloke;
    struct AnimLayout *layout;
    struct AnimEntry *entry;

    bloke = param_1->bloke;
    layout = (struct AnimLayout *)DAT_004b7758[bloke->field_50].anim_layout;
    entry = layout->entries[bloke->field_4a];
    if ((int)bloke->field_4a >= layout->count) {
        bloke->param_action = bloke->param_action + '\x01';
        return;
    }
    if ((int)bloke->field_38 >= entry->count) {
        bloke->field_38 = 0;
        param_1->bloke->field_4a = param_1->bloke->field_4a + 1;
        bloke = param_1->bloke;
        if (bloke->field_4a == (short)bloke->field_4c || (int)bloke->field_4a == layout->count) {
            bloke->param_action = bloke->param_action + '\x01';
            return;
        }
    }
    if ((int)bloke->field_38 != entry->count) {
        FUN_0043a820(entry, param_1);
    }
}

// FUNCTION: LEGOLAND 0x0043a940
void FUN_0043a940(struct SpaceTowerSeat *seat) {
    if (seat->flags & 1) {
        switch (seat->state) {
        case 1:
            if (seat->field_10 == 0) {
                seat->field_10 = -1;
            } else {
                seat->state = 2;
            }
            break;
        case 2:
            if (seat->descending == 0) {
                seat->pos += seat->delta;
                if (seat->pos > 200) {
                    seat->pos = 200;
                    seat->descending = 1;
                }
            } else {
                seat->pos -= 2;
                if (seat->pos < 0) {
                    seat->pos = 0;
                    seat->descending = 0;
                    seat->state = 0;
                }
            }
            break;
        }
    }
}

// FUNCTION: LEGOLAND 0x0043a9b0
void FUN_0043a9b0(struct SpaceTowerCar *arg) {
    int i;

    for (i = 0; i < 4; i++) {
        arg->seats[i].flags &= ~1;
        arg->seats[i].state = 0;
    }
    for (i = 0; i < 8; i++) {
        if (arg->flags_a4[i] != 0) {
            arg->seats[i >> 1].flags |= 1;
            arg->seats[i >> 1].state = 2;
            arg->seats[i >> 1].descending = 0;
            arg->seats[i >> 1].pos = 0;
        }
    }
}

// FUNCTION: LEGOLAND 0x0043aa10
void FUN_0043aa10(unsigned char *arg) {
    unsigned int buffer[4];

    buffer[0] = 2;
    buffer[2] = arg[0];
    buffer[3] = arg[1];
    PlayInstanceOfSample(SPACE_TOWER_SFX[0].sample, 1, 1, buffer);
}

// FUNCTION: LEGOLAND 0x0043aa50
void FUN_0043aa50(unsigned char *arg) {
    struct FadeParams params;
    params.field_0 = 2;
    params.field_8 = arg[0];
    params.field_c = arg[1];
    UnSourceAndFadeAllSamplesFromSource(&params, 0xffffff38);
}

// FUNCTION: LEGOLAND 0x0043aa90
void FUN_0043aa90(struct SpaceTowerCar *arg) {
    unsigned char tmp = arg->var_2;
    arg->var_c |= 1;
    arg->var_3 = tmp;
    arg->var_10 = 0;
    FUN_0043a9b0(arg);
    FUN_0043aa10((unsigned char *)arg);
}

// FUNCTION: LEGOLAND 0x0043aac0
void FUN_0043aac0(struct SpaceTowerCar *arg) {
    int r;

    arg->var_c = arg->var_c & 0xffffbffe;
    arg->seats[0].flags = arg->seats[0].flags & 0xfffffffe;
    arg->seats[1].flags = arg->seats[1].flags & 0xfffffffe;
    arg->seats[0].pos = 0;
    arg->seats[1].pos = 0;
    arg->seats[2].pos = 0;
    arg->seats[3].pos = 0;
    arg->seats[2].flags = arg->seats[2].flags & 0xfffffffe;
    arg->seats[3].flags = arg->seats[3].flags & 0xfffffffe;
    r = rand();
    arg->seats[0].delta = (char)(r % 3) + '\a';
    r = rand();
    arg->seats[1].delta = (char)(r % 3) + '\a';
    r = rand();
    arg->seats[2].delta = (char)(r % 3) + '\a';
    r = rand();
    arg->var_4 = 0;
    arg->var_2 = 0;
    arg->seats[3].delta = (char)(r % 3) + '\a';
    FUN_0043aa50((unsigned char *)arg);
}

// FUNCTION: LEGOLAND 0x0043ab70
void AddSpaceTowerCar(unsigned short *param_1) {
    struct SpaceTowerCar *node;
    unsigned int *fill;
    int i;

    node = (struct SpaceTowerCar *)malloc(0xb4);
    if (node != NULL) {
        fill = (unsigned int *)node;
        for (i = 0x2d; i != 0; i = i + -1) {
            *fill = 0;
            fill = fill + 1;
        }
        node->var_0 = *param_1;
        node->next = SpaceTowerCarList;
        SpaceTowerCarList = node;
        FUN_0043aac0(node);
    }
}

// FUNCTION: LEGOLAND 0x0043abc0
void RemoveSpaceTowerCar(struct SpaceTowerCar *arg) {
    struct SpaceTowerCar *next;
    struct SpaceTowerCar *prev;

    if (SpaceTowerCarList == arg) {
        SpaceTowerCarList = arg->next;
    } else {
        next = SpaceTowerCarList->next;
        prev = SpaceTowerCarList;
        while (next != arg) {
            prev = prev->next;
            if (prev == NULL) {
                break;
            }
            next = prev->next;
        }
        if (prev != NULL) {
            prev->next = arg->next;
        }
    }
    free(arg);
}

// FUNCTION: LEGOLAND 0x0043ac20
void FreeAllSpaceTowerCars(void) {
    while (SpaceTowerCarList != NULL) {
        RemoveSpaceTowerCar(SpaceTowerCarList);
    }
}

// FUNCTION: LEGOLAND 0x0043ac40
struct SpaceTowerCar *FindSpaceTowerCar(unsigned short *param_1) {
    struct SpaceTowerCar *node;

    node = SpaceTowerCarList;
    if (node == NULL) {
        return NULL;
    }
    while (memcmp(node, param_1, 2) != 0) {
        node = node->next;
        if (node == NULL) {
            return NULL;
        }
    }
    return node;
}

// FUNCTION: LEGOLAND 0x0043ac70
void FUN_0043ac70(struct SpaceTowerRideNode *param_1, unsigned short *param_2) {
    struct SpaceTowerCar *ride;
    unsigned int slot;

    ride = FindSpaceTowerCar(param_2);
    if (ride != NULL) {
        slot = FUN_0043acb0(param_1, ride);
        param_1->bloke->field_50 = slot;
        param_1->bloke->field_4c = DAT_004b7758[slot].field_0;
    }
}

// FUNCTION: LEGOLAND 0x0043acb0
unsigned int FUN_0043acb0(struct SpaceTowerRideNode *param_1, struct SpaceTowerCar *param_2) {
    int slot;

    slot = rand() % 8;
    while (param_2->flags_a4[slot] != 0) {
        slot++;
        if (slot >= 8) {
            slot = 0;
        }
    }
    param_2->flags_a4[slot] = 1;
    param_1->bloke->field_36 = (char)slot;
    return slot;
}

// FUNCTION: LEGOLAND 0x0043ad00
int FUN_0043ad00(unsigned char *param_1, int param_2) {
    int y;
    int x;
    struct Point p;
    int w;
    int h;

    x = param_1[0];
    y = param_1[1];
    switch (param_2) {
    case 0:
    case 3:
        x += 2;
        y += 2;
        break;
    case 1:
    case 2:
        x -= 2;
        y -= 2;
        break;
    }
    p.y = y << 8;
    p.x = x << 8;
    GetTileDimensions(&w, &h);
    p.y = ((p.y + p.x) * h) >> 9;
    Get_XScroll();
    return lpConfig->view_y - (short)Get_YScroll() + p.y;
}

// FUNCTION: LEGOLAND 0x0043ad90
void FUN_0043ad90(struct SpaceTowerCar *param_1, int param_2, unsigned int param_3) {
    struct Point coords;
    struct Point off;

    coords = GetScreenCoordsForObject((unsigned char *)param_1, SpaceTowerRideObj);
    if (SpaceTowerSeatMatteSprites[param_2] != NULL) {
        off = DAT_0062fd88[param_2];
        off.y -= param_1->seats[param_2].pos;
        AdjustOffsetForViewMode(&off);
        PrintSprite(SpaceTowerSeatMatteSprites[param_2], coords.x + off.x, coords.y + off.y, param_3, 0);
    }
}

// FUNCTION: LEGOLAND 0x0043ae20
void FUN_0043ae20(struct SpaceTowerCar *param_1, int param_2, unsigned int param_3) {
    struct Point coords;
    struct Point off;
    int id;

    coords = GetScreenCoordsForObject((unsigned char *)param_1, SpaceTowerRideObj);
    off = DAT_0062fd88[param_2];
    off.y -= param_1->seats[param_2].pos;
    AdjustOffsetForViewMode(&off);
    switch (param_2) {
    case 0:
        id = 6;
        break;
    case 1:
        id = 4;
        break;
    case 2:
        id = 0;
        break;
    case 3:
        id = 2;
        break;
    }
    PrintSprite(GetSpriteForLayer((struct LayerContainer *)SpaceTowerLayers, id), coords.x + off.x, coords.y + off.y, param_3, 0);
}

// FUNCTION: LEGOLAND 0x0043aee0
void RenderSpaceTowerSeat(struct SpaceTowerCar *param_1, int param_2, unsigned int param_3) {
    FUN_0043ae20(param_1, param_2, param_3);
    if ((param_1->seats[param_2].flags & 1) != 0) {
        FUN_0043ad00((unsigned char *)param_1, param_2);
        if (param_1->seats[param_2].rider_slot0 != NULL) {
            IP_RenderBlokeIn3DNow(param_1->seats[param_2].rider_slot0->bloke);
        }
        if (param_1->seats[param_2].rider_slot1 != NULL) {
            IP_RenderBlokeIn3DNow(param_1->seats[param_2].rider_slot1->bloke);
        }
        FUN_0043ad90(param_1, param_2, param_3);
    }
}

// FUNCTION: LEGOLAND 0x0043af50
void RenderSpaceTower(struct SpaceTowerCtx *param_1, unsigned int param_2, unsigned int param_3, unsigned short *param_4, unsigned int param_5, unsigned int param_6) {
    struct SpaceTowerRide *ride;
    struct SpaceTowerRideNode *node;
    struct SpaceTowerCar *car;
    struct Bloke *bloke;
    struct Point coords;
    struct Point off1;
    struct Point off2;

    ride = param_1->ride;
    node = ride->list;
    car = FindSpaceTowerCar(param_4);
    if (car == NULL) {
        return;
    }
    coords = GetScreenCoordsForObject((unsigned char *)param_4, ride);
    off1 = GetRenderOffsetForLayer((struct LayerOffsetHolder *)SpaceTowerLayers, 1);
    AdjustOffsetForViewMode(&off1);
    if (node != NULL) {
        for (; node != NULL; node = node->next) {
            if (memcmp(&node->id, param_4, 2) == 0) {
                bloke = node->bloke;
                if ((bloke->flags & 0x80) == 0 && DAT_004b7758[bloke->field_50].anim_layout == &DAT_004b7750) {
                    IP_RenderBlokeIn3DNow(bloke);
                }
            }
        }
        RenderSpaceTowerSeat(car, 1, param_6);
        RenderSpaceTowerSeat(car, 2, param_6);
        PrintSprite(SpaceTowerMatte2Sprite, coords.x + off1.x, coords.y + off1.y, param_6, 0);
        RenderSpaceTowerSeat(car, 0, param_6);
        RenderSpaceTowerSeat(car, 3, param_6);
        node = ride->list;
        for (; node != NULL; node = node->next) {
            if (memcmp(&node->id, param_4, 2) == 0) {
                bloke = node->bloke;
                if ((bloke->flags & 0x80) == 0 && DAT_004b7758[bloke->field_50].anim_layout == &DAT_004b76b8) {
                    IP_RenderBlokeIn3DNow(bloke);
                }
            }
        }
        PrintSprite(SpaceTowerMatte1Sprite, coords.x + off1.x, coords.y + off1.y, param_6, 0);
        LLSSetFrame(GetLLSForLayer(SpaceTowerLayers, 5), (char)car->var_ad);
        off2 = GetRenderOffsetForLayer((struct LayerOffsetHolder *)SpaceTowerLayers, 5);
        AdjustOffsetForViewMode(&off2);
        PrintSprite(GetSpriteForLayer((struct LayerContainer *)SpaceTowerLayers, 5), off2.x + coords.x, off2.y + coords.y, param_6, 0);
        LLSSetFrame(GetLLSForLayer(SpaceTowerLayers, 3), (char)car->var_ac);
        off2 = GetRenderOffsetForLayer((struct LayerOffsetHolder *)SpaceTowerLayers, 3);
        AdjustOffsetForViewMode(&off2);
        PrintSprite(GetSpriteForLayer((struct LayerContainer *)SpaceTowerLayers, 3), off2.x + coords.x, off2.y + coords.y, param_6, 0);
    } else {
        RenderSpaceTowerSeat(car, 1, param_6);
        RenderSpaceTowerSeat(car, 2, param_6);
        PrintSprite(SpaceTowerMatte2Sprite, coords.x + off1.x, coords.y + off1.y, param_6, 0);
        RenderSpaceTowerSeat(car, 0, param_6);
        RenderSpaceTowerSeat(car, 3, param_6);
        LLSSetFrame(GetLLSForLayer(SpaceTowerLayers, 5), (char)car->var_ad);
        off2 = GetRenderOffsetForLayer((struct LayerOffsetHolder *)SpaceTowerLayers, 5);
        AdjustOffsetForViewMode(&off2);
        PrintSprite(GetSpriteForLayer((struct LayerContainer *)SpaceTowerLayers, 5), off2.x + coords.x, off2.y + coords.y, param_6, 0);
        LLSSetFrame(GetLLSForLayer(SpaceTowerLayers, 3), (char)car->var_ac);
        off2 = GetRenderOffsetForLayer((struct LayerOffsetHolder *)SpaceTowerLayers, 3);
        AdjustOffsetForViewMode(&off2);
        PrintSprite(GetSpriteForLayer((struct LayerContainer *)SpaceTowerLayers, 3), off2.x + coords.x, off2.y + coords.y, param_6, 0);
    }
}

// FUNCTION: LEGOLAND 0x0043b2b0
void FUN_0043b2b0(struct SpaceTowerCtx *param_1) {
    SpaceTowerRideObj = param_1->ride;
    ((struct SpaceTowerRide *)SpaceTowerRideObj)->flags |= 0x420;
    SpaceTowerLayers = ((struct SpaceTowerRide *)SpaceTowerRideObj)->layers;
    *(unsigned int *)((char *)SpaceTowerLayers + 0x10) = *(unsigned int *)((char *)SpaceTowerLayers + 0x10) | 0x2000;
    HideLayer(SpaceTowerLayers, 5);
    StopLayerPlaying(SpaceTowerLayers, 5);
    LLSSetFrame(GetLLSForLayer(SpaceTowerLayers, 5), 0);
    HideLayer(SpaceTowerLayers, 3);
    StopLayerPlaying(SpaceTowerLayers, 3);
    LLSSetFrame(GetLLSForLayer(SpaceTowerLayers, 3), 0);
    DAT_0062fd88[0] = GetRenderOffsetForLayer((struct LayerOffsetHolder *)SpaceTowerLayers, 6);
    DAT_0062fd88[1] = GetRenderOffsetForLayer((struct LayerOffsetHolder *)SpaceTowerLayers, 4);
    DAT_0062fd88[2] = GetRenderOffsetForLayer((struct LayerOffsetHolder *)SpaceTowerLayers, 0);
    DAT_0062fd88[3] = GetRenderOffsetForLayer((struct LayerOffsetHolder *)SpaceTowerLayers, 2);
    SpaceTowerSeatMatteSprites[0] = NULL;
    // STRING: LEGOLAND 0x004b7880
    SpaceTowerSeatMatteSprites[1] = LoadSprite("SpaceTower Seat2 Matte.lls", 1);
    // STRING: LEGOLAND 0x004b7864
    SpaceTowerSeatMatteSprites[2] = LoadSprite("SpaceTower Seat3 Matte.lls", 1);
    SpaceTowerSeatMatteSprites[3] = NULL;
    // STRING: LEGOLAND 0x004b7850
    SpaceTowerMatte1Sprite = LoadSprite("Spacet Matte1.lls", 1);
    // STRING: LEGOLAND 0x004b783c
    SpaceTowerMatte2Sprite = LoadSprite("Spacet Matte2.lls", 1);
    Load_FXList(SPACE_TOWER_SFX, 1);
}

// FUNCTION: LEGOLAND 0x0043b420
void SpaceTowerSetEditMode(void) {
    EditMode.unk0 = 1;
    EditMode.unk8 = SpaceTowerRideObj;
    DefaultCursor(&EditCursor);
    SetEditCursorFootPrint((void *)((unsigned int)EditMode.unk8 + 0x3c));
}

// FUNCTION: LEGOLAND 0x0043b460
void SpaceTowerRemoveObject(struct EditObject *param_1, TileId tile, struct Cursor *param_3) {
    struct SpaceTowerCar *node;

    node = FindSpaceTowerCar(&tile.id);
    if (node != NULL) {
        RemoveSpaceTowerCar(node);
    }
    StandardRemoveObject(param_1, tile, param_3);
    RemoveAllBlokesFromRide(*(struct Ride **)((char *)param_1 + 0xc), tile);
    FUN_0043aa50(&tile.pos.x);
}

// FUNCTION: LEGOLAND 0x0043b4b0
void SpaceTowerAddObject(struct EditObject *param_1, int *coords) {
    TileId id;

    id.pos.x = (unsigned char)coords[0];
    id.pos.y = (unsigned char)coords[1];
    AddBasicObject(param_1, coords);
    AddSpaceTowerCar(&id.id);
}

// FUNCTION: LEGOLAND 0x0043b4e0
void *FUN_0043b4e0(int param_1, unsigned short param_2) {
    int ride;

    ride = *(int *)(param_1 + 0xc);
    DAT_0062fd48 = *(unsigned int *)(ride + 0x64);
    DAT_0062fd4c = *(unsigned int *)(ride + 0x14);
    DAT_0062fd50 = *(unsigned int *)(ride + 0x18);
    DAT_0062fd54 = param_2;
    *(unsigned int *)(*(int *)(ride + 0x64) + 0x10) |= 0x2000;
    HideLayer(SpaceTowerLayers, 6);
    HideLayer(SpaceTowerLayers, 4);
    HideLayer(SpaceTowerLayers, 0);
    HideLayer(SpaceTowerLayers, 2);
    HideLayer(SpaceTowerLayers, 5);
    return &DAT_0062fd48;
}

// FUNCTION: LEGOLAND 0x0043b570
void FUN_0043b570(void) {
    KillSprite(SpaceTowerMatte1Sprite);
    KillSprite(SpaceTowerMatte2Sprite);
    KillSprite(SpaceTowerSeatMatteSprites[1]);
    KillSprite(SpaceTowerSeatMatteSprites[2]);
    FreeAllSpaceTowerCars();
    Kill_FXList(SPACE_TOWER_SFX, 1);
    ((struct SpaceTowerCar *)SpaceTowerRideObj)->var_cc = 0;
}

// FUNCTION: LEGOLAND 0x0043b5d0
LEGO_EXPORT int SpaceTower_Save(void) {
    struct SpaceTowerCar *car;
    struct SpaceTowerRideNode **field;
    struct SpaceTowerRideNode *cur;
    int index;
    int i;
    unsigned int one;
    unsigned int zero;

    car = SpaceTowerCarList;
    one = 1;
    zero = 0;
    if (SpaceTowerCarList != NULL) {
        while (car != NULL) {
            if (SaveGameWrite(&one, 4) == 0) {
                return 0;
            }
            for (i = 0; i < 8; i++) {
                if (i & 1) {
                    field = &car->seats[i >> 1].rider_slot1;
                } else {
                    field = &car->seats[i >> 1].rider_slot0;
                }
                index = 0;
                for (cur = ((struct SpaceTowerRide *)SpaceTowerRideObj)->list; cur != NULL; cur = cur->next) {
                    if (cur == *field) {
                        break;
                    }
                    index++;
                }
                if (cur != NULL) {
                    *field = (struct SpaceTowerRideNode *)(index + 1);
                } else {
                    *field = NULL;
                }
            }
            if (SaveGameWrite(car, 0xb4) == 0) {
                return 0;
            }
            car = car->next;
        }
    }
    return SaveGameWrite(&zero, 4) != 0;
}

// FUNCTION: LEGOLAND 0x0043b6a0
LEGO_EXPORT int SpaceTower_Load(void) {
    struct SpaceTowerCar *node;
    struct SpaceTowerCar *prev;
    struct SpaceTowerRideNode **field;
    struct SpaceTowerRideNode *cur;
    int flag;
    int n;
    int i;

    prev = NULL;
    if (SaveGameRead(&flag, 4) == 0) {
        return 0;
    }
    while (flag != 0) {
        node = (struct SpaceTowerCar *)malloc(0xb4);
        if (SaveGameRead(node, 0xb4) == 0) {
            return 0;
        }
        node->next = NULL;
        if (prev != NULL) {
            prev->next = node;
        } else {
            SpaceTowerCarList = node;
        }
        prev = node;
        for (i = 0; i < 8; i++) {
            if (i & 1) {
                field = &node->seats[i >> 1].rider_slot1;
            } else {
                field = &node->seats[i >> 1].rider_slot0;
            }
            n = (int)*field;
            cur = ((struct SpaceTowerRide *)SpaceTowerRideObj)->list;
            if (n != 0) {
                while (--n != 0) {
                    cur = cur->next;
                }
                *field = cur;
            } else {
                *field = NULL;
            }
        }
        if (SaveGameRead(&flag, 4) == 0) {
            return 0;
        }
    }
    return 1;
}

// FUNCTION: LEGOLAND 0x0043b780
void SpaceTowerRide(struct ClassNode *name, struct CallbackTable *obj) {
    // STRING: LEGOLAND 0x004b789c
    if (_stricmp("SPACE TOWER RIDE", name->name) == 0) {
        obj->cb_a4 = FUN_0043b2b0;
        obj->cb_8c = SpaceTowerSetEditMode;
        obj->cb_a8 = FUN_0043bac0;
        obj->cb_a0 = FUN_0043b4e0;
        obj->cb_b0 = RenderSpaceTower;
        obj->cb_9c = SpaceTowerRemoveObject;
        obj->cb_98 = SpaceTowerAddObject;
        obj->cb_ac = FUN_0043b570;
        obj->cb_bc = SpaceTower_Save;
        obj->cb_b8 = SpaceTower_Load;
    }
}

// FUNCTION: LEGOLAND 0x0043b810
void FUN_0043b810(struct SpaceTowerCar *param_1) {
    struct SpaceTowerRideNode *node;
    int slot;
    int idx;
    int odd;
    struct Point coords;
    struct Point off;
    struct Point dat;
    struct Point pos;
    struct Person *person;

    node = ((struct SpaceTowerRide *)SpaceTowerRideObj)->list;
    param_1->seats[0].rider_slot0 = NULL;
    param_1->seats[0].rider_slot1 = NULL;
    param_1->seats[1].rider_slot0 = NULL;
    param_1->seats[1].rider_slot1 = NULL;
    param_1->seats[2].rider_slot0 = NULL;
    param_1->seats[2].rider_slot1 = NULL;
    param_1->seats[3].rider_slot0 = NULL;
    param_1->seats[3].rider_slot1 = NULL;
    coords = GetScreenCoordsForObject((unsigned char *)param_1, SpaceTowerRideObj);
    for (; node != NULL; node = node->next) {
        if (memcmp(&node->id, &param_1->var_0, 2) == 0 && (node->bloke->flags & 0x80) != 0) {
            slot = node->bloke->field_36;
            idx = slot >> 1;
            odd = slot & 1;
            if (odd == 0) {
                param_1->seats[idx].rider_slot0 = node;
            } else {
                param_1->seats[idx].rider_slot1 = node;
            }
            off = DAT_0062fd88[idx];
            off.y -= param_1->seats[idx].pos;
            AdjustOffsetForViewMode(&off);
            pos.x = coords.x + off.x;
            pos.y = coords.y + off.y;
            if (odd == 0) {
                dat = DAT_004b7798[idx].inner;
            } else {
                dat = DAT_004b7798[idx].outer;
            }
            AdjustOffsetForViewMode(&dat);
            pos.x += dat.x;
            pos.y += dat.y;
            AdjustBlokePosition((struct Point *)&pos);
            person = node->person;
            SetPersonPosition(person, pos.x, pos.y);
            SetPersonDirection(person, DAT_004b7798[idx].direction);
        }
    }
}

// FUNCTION: LEGOLAND 0x0043b990
void FUN_0043b990(struct SpaceTowerCar *esi) {
    esi->var_ad++;
    if ((signed char)esi->var_ad >= 16) {
        esi->var_ad = 0;
    }
    esi->var_ac++;
    if ((signed char)esi->var_ac >= 16) {
        esi->var_ac = 0;
    }
    if (esi->var_c & 1) {
        FUN_0043a940(&esi->seats[0]);
        FUN_0043a940(&esi->seats[1]);
        FUN_0043a940(&esi->seats[2]);
        FUN_0043a940(&esi->seats[3]);
        if (esi->seats[0].state == 0 && esi->seats[1].state == 0 && esi->seats[2].state == 0 && esi->seats[3].state == 0) {
            if (GetAllBlokesOffRide((struct Ride *)SpaceTowerRideObj, esi->var_0) == 0) {
                return;
            }
            FUN_0043aac0(esi);
            return;
        }
    }
    FUN_0043b810(esi);
    Put3DBlokesOnRide2(SpaceTowerRideObj, esi);
    if (esi->var_c & 1) {
        return;
    }
    if (esi->var_c & 0x4000) {
        if (esi->var_2 == esi->var_4) {
            esi->var_c &= ~0x4000;
            FUN_0043aa90(esi);
        }
        return;
    }
    if (esi->var_2 == 0) {
        return;
    }
    if (esi->var_b0 == 0) {
        esi->var_c |= 0x4000;
        Ride_SetFlagToNotLetAnyoneOn(esi);
    } else {
        esi->var_b0--;
    }
}

// FUNCTION: LEGOLAND 0x0043baa0
void FUN_0043baa0(void) {
    struct ListNode *node;

    node = SpaceTowerCarList;
    if (node != NULL) {
        do {
            FUN_0043b990((struct SpaceTowerCar *)node);
            node = node->next;
        } while (node != NULL);
    }
}

// FUNCTION: LEGOLAND 0x0043bac0
void FUN_0043bac0(struct SpaceTowerCtx *param_1) {
    struct SpaceTowerRide *ride;
    struct SpaceTowerRideNode *node;
    struct SpaceTowerRideNode *next;
    struct Bloke *bloke;
    struct SpaceTowerCar *obj;
    int x;
    int y;
    TileId *tile;
    int ride_x;
    int ride_y;
    struct Point to;
    char dir;
    unsigned char seat;

    ride = param_1->ride;
    FUN_0043baa0();
    for (node = ride->list; node != NULL; node = next) {
        next = node->next;
        bloke = node->bloke;
        obj = FindSpaceTowerCar(&node->id);
        if (obj == NULL) {
            return;
        }
        x = node->coord.x;
        ride_x = ride->x + x;
        tile = (TileId *)&node->id;
        y = tile->pos.y;
        ride_y = ride->y + y;
        if (bloke->low_level_action == 0) {
            switch (bloke->param_action) {
            case 0:
                obj->var_4++;
                obj->var_b0 = 200;
                FUN_0043ac70(node, &tile->id);
                bloke->flags |= 8;
                ride_x <<= 8;
                ride_y = (ride_y + 1) << 8;
                bloke->dest.y = ride_y;
                bloke->dest.x = ride_x;
                dir = CalcMoveLine(bloke->pos, bloke->dest, &bloke->nav);
                bloke->low_level_action = 7;
                bloke->field_73 = dir + 0x10;
                NewDirForAction(bloke, ((unsigned char)(dir + 0x10) >> 5) + 3);
                bloke->param_action++;
                break;
            case 1:
                node->bloke->field_4a = 0;
                node->bloke->field_38 = 0;
                bloke->param_action++;
                break;
            case 3:
                seat = bloke->field_36;
                bloke->dest.x = (DAT_004b77e8[seat].x + x) << 8;
                bloke->dest.y = (DAT_004b77e8[seat].y + node->coord.y) << 8;
                dir = CalcMoveLine(bloke->pos, bloke->dest, &bloke->nav);
                bloke->low_level_action = 7;
                bloke->field_73 = dir + 0x10;
                NewDirForAction(bloke, (char)DAT_004b7798[seat >> 1].direction);
                bloke->param_action++;
                break;
            case 4:
                bloke->flags |= 0x80;
                BlokeSitAnim(bloke);
                BlokeSetFrame(bloke, 0);
                bloke->field_58 = 0;
                bloke->param_action++;
                obj->var_2++;
                FUN_0043a9b0(obj);
                if ((short)(char)obj->var_2 == ((struct SpaceTowerRide *)SpaceTowerRideObj)->seats) {
                    FUN_0043aa90(obj);
                }
                break;
            case 6:
                node->bloke->field_4c = 0xffff;
                obj->flags_a4[bloke->field_36] = 0;
                bloke->flags &= 0xff7f;
                BlokeWalkAnim(bloke);
                BlokeSetFrame(bloke, 0);
                bloke->param_action++;
                break;
            case 2:
            case 7:
                FUN_0043a8c0(node);
                break;
            case 8:
                bloke->dest.x = (((signed char)ride->field_24 + x) << 8) + 0x80;
                bloke->dest.y = (((signed char)ride->field_25 + tile->pos.y) << 8) + 0x80;
                dir = CalcMoveLine(bloke->pos, bloke->dest, &bloke->nav);
                bloke->low_level_action = 7;
                bloke->field_73 = dir + 0x10;
                NewDirForAction(bloke, ((unsigned char)(dir + 0x10) >> 5) + 3);
                bloke->param_action++;
                break;
            case 9:
                RemoveBlokeFromRide((struct Ride *)ride, (struct RideNode *)node);
                bloke->flags &= 0xfff7;
                if (--obj->var_3 == 0) {
                    obj->var_2 = 0;
                    Ride_ClearFlagToNotLetAnyoneOn(obj);
                }
                break;
            }
        }
    }
}
