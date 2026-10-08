#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "globals.h"
#include "legoland.h"

#include "binv.h"
#include "bloke.h"
#include "gamemap.h"
#include "llidb.h"
#include "man3d.h"
#include "map_object.h"
#include "obj_instance.h"
#include "objclass.h"
#include "print_sprite.h"
#include "render3d.h"
#include "spinning_barrels.h"

struct BarrelSource {
    unsigned short field_0;
};

struct BarrelCarNode {
    /* 0x00 */ unsigned char pad_0[0x10];
    /* 0x10 */ unsigned int field_10;
    /* 0x14 */ unsigned int field_14;
    /* 0x18 */ unsigned int field_18;
    /* 0x1c */ unsigned char pad_1c[0x48];
    /* 0x64 */ struct BarrelCarNode *next;
};

struct BarrelRoot {
    /* 0x00 */ unsigned char pad_0[0xc];
    /* 0x0c */ struct BarrelCarNode *car;
};

#include "image_sprite.h"

// FUNCTION: LEGOLAND 0x0043bdb0
void FUN_0043bdb0(void *param1) {
    struct BarrelNode *block = (struct BarrelNode *)malloc(0x34);
    if (block == NULL) {
        return;
    }
    memset(block, 0, 0x34);
    block->tile_id = ((struct BarrelSource *)param1)->field_0;
    block->frame = 0;
    block->next = SpinningBarrelList;
    SpinningBarrelList = block;
    FUN_0043c2f0(block);
}

// FUNCTION: LEGOLAND 0x0043be00
void FUN_0043be00(struct BarrelNode *node) {
    struct BarrelNode *prev;
    struct BarrelNode *cur;

    if (SpinningBarrelList == node) {
        SpinningBarrelList = node->next;
    } else {
        cur = SpinningBarrelList->next;
        prev = SpinningBarrelList;
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

// FUNCTION: LEGOLAND 0x0043be40
struct BarrelNode *FUN_0043be40(unsigned short *key) {
    struct BarrelNode *cur = SpinningBarrelList;

    if (cur != NULL) {
        do {
            if (memcmp(&cur->tile_id, key, 2) == 0) {
                return cur;
            }
            cur = cur->next;
        } while (cur != NULL);
    }
    return NULL;
}

// FUNCTION: LEGOLAND 0x0043be70
void RenderSpinningBarrels(Element *obj, void *param_2, void *param_3, TileId *tile) {
    Ride *ride = obj->ride;
    RideNode *elem = ride->riders;
    RideNode *riders;
    char count = 0;
    Bloke *blokes[16] = {0};
    Bloke *bloke;
    Person *person;
    struct BarrelNode *state;
    Point screen;
    Point off;
    Point off2;
    Point seat;
    LayerResult layer;
    char i;

    state = FUN_0043be40(&tile->id);
    if (state == NULL) {
        return;
    }
    screen = GetScreenCoordsForObject(tile, ride);
    GetLayer(ride->layer, &layer, 3);
    layer.field_10 = 0;
    for (; elem != NULL; elem = elem->next) {
        if (tile->id == elem->tile.id) {
            blokes[count++] = elem->rider;
        }
    }
    if (count != 0) {
        LLSSetFrame(GetLLSForLayer(SpinningBarrelsLayer, 3), state->frame);
        off = GetRenderOffsetForLayer(SpinningBarrelsLayer, 3);
        AdjustOffsetForViewMode(&off);
        PrintSprite(GetSpriteForLayer(SpinningBarrelsLayer, 3), screen.x + off.x, screen.y + off.y, 0, 0);
        riders = ride->riders;
        (*((struct Sprite *)DAT_0062fe00[0])->lls)->frame = state->frame;
        for (; riders != NULL; riders = riders->next) {
            if (tile->id == riders->tile.id && (riders->rider->flags & 0x80) != 0) {
                bloke = riders->rider;
                person = bloke->person;
                seat = DAT_0062fdd8;
                person->offset.x = bloke->screen_x;
                person->offset.y = bloke->screen_y;
                AdjustBlokePosition(&person->offset);
                AdjustOffsetForViewMode(&seat);
                person->screen.x = bloke->screen_x + seat.x + screen.x;
                person->screen.y = bloke->screen_y + seat.y + screen.y;
                AdjustBlokePosition(&person->screen);
                IP_RenderBlokeIn3DNow(riders->rider);
            }
        }
        for (i = 0; i < count; i++) {
            if (blokes[i]->param_action == 0) {
                IP_RenderBlokeIn3DNow(blokes[i]);
            }
        }
        for (i = 0; i < count; i++) {
            if (blokes[i]->param_action == 1) {
                IP_RenderBlokeIn3DNow(blokes[i]);
            }
        }
        for (i = 0; i < count; i++) {
            if (blokes[i]->param_action == 15) {
                IP_RenderBlokeIn3DNow(blokes[i]);
            }
        }
        off2 = GetRenderOffsetForLayer(SpinningBarrelsLayer, 1);
        AdjustOffsetForViewMode(&off2);
        PrintSprite(SpinningBarrelsEntranceMatteSprite, screen.x + off2.x, screen.y + off2.y, 0, 0);
        LLSSetFrame(GetLLSForLayer(SpinningBarrelsLayer, 2), state->layer2_frame);
        off2 = GetRenderOffsetForLayer(SpinningBarrelsLayer, 2);
        AdjustOffsetForViewMode(&off2);
        PrintSprite(GetSpriteForLayer(SpinningBarrelsLayer, 2), screen.x + off2.x, screen.y + off2.y, 0, 0);
        for (i = 0; i < count; i++) {
            if (blokes[i]->param_action == 16) {
                IP_RenderBlokeIn3DNow(blokes[i]);
            }
        }
        for (i = 0; i < count; i++) {
            if (blokes[i]->param_action == 17) {
                IP_RenderBlokeIn3DNow(blokes[i]);
            }
        }
        off2 = GetRenderOffsetForLayer(SpinningBarrelsLayer, 1);
        AdjustOffsetForViewMode(&off2);
        PrintSprite(SpinningBarrelsEntranceMatte2Sprite, screen.x + off2.x, screen.y + off2.y, 0, 0);
    } else {
        LLSSetFrame(GetLLSForLayer(SpinningBarrelsLayer, 3), state->frame);
        off = GetRenderOffsetForLayer(SpinningBarrelsLayer, 3);
        AdjustOffsetForViewMode(&off);
        PrintSprite(GetSpriteForLayer(SpinningBarrelsLayer, 3), screen.x + off.x, screen.y + off.y, 0, 0);
        LLSSetFrame(GetLLSForLayer(SpinningBarrelsLayer, 2), state->layer2_frame);
        off2 = GetRenderOffsetForLayer(SpinningBarrelsLayer, 2);
        AdjustOffsetForViewMode(&off2);
        PrintSprite(GetSpriteForLayer(SpinningBarrelsLayer, 2), screen.x + off2.x, screen.y + off2.y, 0, 0);
    }
}

// FUNCTION: LEGOLAND 0x0043c2f0
void FUN_0043c2f0(struct BarrelNode *node) {
    node->frame_ticks = 0;
    node->frame = 0;
    node->boarding_count = 0;
    node->seated_count = 0;
    node->flags = node->flags & 0xffffbffe;
    node->cycles_left = 3;
}

// FUNCTION: LEGOLAND 0x0043c320
void FUN_0043c320(struct BarrelNode *node) {
    unsigned int packed = node->flags;
    unsigned char prev = node->seated_count;
    packed &= 0xffffbfff;
    node->leaving_count = prev;
    packed |= 0x1;
    node->seated_count = 0;
    node->flags = packed;
}

// FUNCTION: LEGOLAND 0x0043c340
void SpinningBarrelsRideLoad(struct Element *elem) {
    struct LayerResult layer;

    SpinningBarrelsRide = elem->ride;
    SpinningBarrelsRide->flags |= 0x420;
    SpinningBarrelsLayer = SpinningBarrelsRide->layer;
    SpinningBarrelsLayer->flags |= 0x2000;
    // STRING: LEGOLAND 0x004b7960
    DAT_0062fe00[1] = DAT_0062fe00[0] = LoadSprite("z_SpinningBarrels.lls", 1);
    // STRING: LEGOLAND 0x004b7944
    BoxBlokesOn1BNV = DAT_0062fdf0[0] = LoadBinV("Zbuffers\\BoxBlokesOn1.bnv");
    // STRING: LEGOLAND 0x004b7924
    SpinningBarrelsBNV = DAT_0062fdf0[1] = LoadBinV("Zbuffers\\SpinningBarrels.bnv");
    // STRING: LEGOLAND 0x004b7908
    BoxBlokesOffBNV = DAT_0062fdf0[2] = LoadBinV("Zbuffers\\BoxBlokesOff.bnv");
    HideLayer(SpinningBarrelsLayer, 3);
    StopLayerPlaying(SpinningBarrelsLayer, 3);
    LLSSetFrame(GetLLSForLayer(SpinningBarrelsLayer, 3), 0);
    HideLayer(SpinningBarrelsLayer, 2);
    StopLayerPlaying(SpinningBarrelsLayer, 2);
    LLSSetFrame(GetLLSForLayer(SpinningBarrelsLayer, 2), 0);
    GetLayer(SpinningBarrelsRide->layer, &layer, 3);
    DAT_0062fdd8.x = layer.x - 103;
    DAT_0062fdd8.y = layer.y - 55;
    // STRING: LEGOLAND 0x004b78e4
    SpinningBarrelsEntranceMatteSprite = LoadSprite("SpinningBarrelsEntranceMatte.lls", 1);
    // STRING: LEGOLAND 0x004b78c0
    SpinningBarrelsEntranceMatte2Sprite = LoadSprite("SpinningBarrelsEntranceMatte2.lls", 1);
}

// FUNCTION: LEGOLAND 0x0043c490
void SpinningBarrelsSetEditMode(void) {
    EditMode.unk0 = 1;
    EditMode.unk8 = SpinningBarrelsRide;
    DefaultCursor(&EditCursor);
    SetEditCursorFootPrint((unsigned int *)((unsigned int)EditMode.unk8 + 0x3c));
}

// FUNCTION: LEGOLAND 0x0043c4d0
void FUN_0043c4d0(void) {
    while (SpinningBarrelList != NULL) {
        FUN_0043be00(SpinningBarrelList);
    }
}

// FUNCTION: LEGOLAND 0x0043c4f0
void SpinningBarrelsRemoveObject(Element *editObj, TileId tile, struct Cursor *cursor) {
    struct BarrelNode *node = FUN_0043be40(&tile.id);
    if (node != NULL) {
        FUN_0043be00(node);
    }
    StandardRemoveObject(editObj, tile, cursor);
    RemoveAllBlokesFromRide(editObj->ride, tile);
}

// FUNCTION: LEGOLAND 0x0043c540
void SpinningBarrelsAddObject(Element *editObj, int *coords) {
    TileId tile;
    tile.pos.x = (unsigned char)coords[0];
    tile.pos.y = (unsigned char)coords[1];
    AddBasicObject(editObj, coords);
    FUN_0043bdb0(&tile);
}

// FUNCTION: LEGOLAND 0x0043c570
unsigned int *GetSpinningBarrelsRideSpriteInfo(struct BarrelRoot *ride, unsigned short param2) {
    struct BarrelCarNode *target = ride->car;

    DAT_0062fdb0.sprite = target->next;
    DAT_0062fdb0.x = target->field_14;
    DAT_0062fdb0.y = target->field_18;
    DAT_0062fdb0.id = param2;

    target = target->next;
    target->field_10 |= 0x2000;
    return (void *)&DAT_0062fdb0;
}

// FUNCTION: LEGOLAND 0x0043c5b0
void SpinningBarrelsRideUnload(void) {
    KillSprite(SpinningBarrelsEntranceMatteSprite);
    KillSprite(SpinningBarrelsEntranceMatte2Sprite);
    KillSprite(DAT_0062fe00[1]);
    if (SpinningBarrelsBNV != NULL) {
        FreeBinV(SpinningBarrelsBNV);
    }
    if (BoxBlokesOn1BNV != NULL) {
        FreeBinV(BoxBlokesOn1BNV);
    }
    if (BoxBlokesOffBNV != NULL) {
        FreeBinV(BoxBlokesOffBNV);
    }
    FUN_0043c4d0();
}

// FUNCTION: LEGOLAND 0x0043c620
LEGO_EXPORT int SaveSBarrel(void) {
    struct BarrelNode *node = SpinningBarrelList;
    unsigned int marker = 1;
    unsigned int terminator = 0;

    if (node != NULL) {
        do {
            if (SaveGameWrite(&marker, 4)) {
                if (SaveGameWrite(node, 0x34)) {
                    node = node->next;
                } else {
                    return 0;
                }
            } else {
                return 0;
            }
        } while (node != NULL);
    }

    return SaveGameWrite(&terminator, 4) != 0;
}

struct BarrelTypeC {
    void *field_0;
    unsigned int field_4;
};

struct BarrelData {
    unsigned char pad_0[0x54];
    struct BarrelTypeC *field_54;
};

struct BarrelCar {
    unsigned char pad_0[0x2c];
    void *field_2c;
    unsigned int field_30;
};

struct BarrelListNode {
    struct BarrelListNode *next;
    unsigned char pad_4[4];
    struct BarrelData *rider;
    unsigned char pad_c[4];
    struct BarrelCar *person;
};

struct BarrelGameObject {
    unsigned char pad_0[0xcc];
    struct BarrelListNode *riders;
};

struct BarrelLoadArg {
    unsigned char pad_0[0xc];
    struct BarrelGameObject *field_c;
};

// FUNCTION: LEGOLAND 0x0043c690
LEGO_EXPORT int LoadSBarrel(struct BarrelLoadArg *arg) {
    struct BarrelGameObject *obj = arg->field_c;
    struct BarrelNode *prev = NULL;
    struct BarrelListNode *list;
    struct BarrelCar *car;
    struct BarrelData *data;
    struct BarrelTypeC *tc;
    unsigned int marker;

    if (!SaveGameRead(&marker, 4)) {
        return 0;
    }
    while (marker != 0) {
        struct BarrelNode *node = (struct BarrelNode *)malloc(0x34);
        if (!SaveGameRead(node, 0x34)) {
            return 0;
        }
        node->next = NULL;
        if (prev != NULL) {
            prev->next = node;
        } else {
            SpinningBarrelList = node;
        }
        prev = node;
        if (!SaveGameRead(&marker, 4)) {
            return 0;
        }
    }

    list = obj->riders;
    while (list != NULL) {
        car = list->person;
        if (car->field_30 != 0) {
            car->field_2c = DAT_0062fe00[car->field_30];
        } else {
            car->field_2c = NULL;
            list->person->field_30 = 0;
        }
        data = list->rider;
        tc = data->field_54;
        if (tc != NULL) {
            tc->field_0 = DAT_0062fdf0[tc->field_4];
        }
        list = list->next;
    }
    return 1;
}

// FUNCTION: LEGOLAND 0x0043c760
void SpinningBarrelsGetInterfaces(struct ClassNode *str, struct CallbackTable *ride) {
    // STRING: LEGOLAND 0x004b7978
    if (_stricmp("SPINNING BARRELS RIDE", str->name) == 0) {
        ride->cb_a4 = SpinningBarrelsRideLoad;
        ride->cb_8c = SpinningBarrelsSetEditMode;
        ride->cb_a8 = SpinningBarrelsUpdate;
        ride->cb_b0 = RenderSpinningBarrels;
        ride->cb_9c = SpinningBarrelsRemoveObject;
        ride->cb_98 = SpinningBarrelsAddObject;
        ride->cb_ac = SpinningBarrelsRideUnload;
        ride->cb_a0 = GetSpinningBarrelsRideSpriteInfo;
        ride->cb_bc = SaveSBarrel;
        ride->cb_b8 = LoadSBarrel;
    }
}

// FUNCTION: LEGOLAND 0x0043c7f0
void FUN_0043c7f0(struct BarrelNode *node) {
    struct RideNode *r = SpinningBarrelsRide->riders;
    unsigned int flags;

    node->layer2_frame++;
    if (node->layer2_frame >= 0x20) {
        node->layer2_frame = 0;
    }
    flags = node->flags;
    if (flags & 1) {
        unsigned char c;
        int v = ++node->frame_ticks;
        c = node->cycles_left;
        if (c == 0) {
            if (GetAllBlokesOffRide(SpinningBarrelsRide, node->tile_id) == 0) {
                return;
            }
            FUN_0043c2f0(node);
            return;
        }
        if (v > 2) {
            node->frame_ticks = 0;
            node->frame++;
            if (node->frame >= 0x40) {
                node->frame = 0;
                node->cycles_left = c - 1;
            }
        }
    } else if (flags & 0x4000) {
        if (node->seated_count == node->boarding_count) {
            node->flags = flags & 0xffffbfff;
            FUN_0043c320(node);
            return;
        }
    } else if (node->seated_count != 0) {
        if (node->boarding_timer == 0) {
            node->flags = flags | 0x4000;
            Ride_SetFlagToNotLetAnyoneOn(&node->tile_id);
        } else {
            node->boarding_timer--;
        }
    }
    for (; r != NULL; r = r->next) {
        if (node->tile_id == r->tile.id && r->rider->field_35 == 1) {
            sprintf(&BoxBlokeBnvName[8], "%02d", r->rider->field_36);
            SetBlokePositionFromBNV(SpinningBarrelsBNV, r->rider, BoxBlokeBnvName, node->frame, -1617922.25f, -1618065.75f, 0);
        }
    }
    *(short *)*((struct Sprite *)DAT_0062fe00[0])->lls = (short)node->frame;
}

// FUNCTION: LEGOLAND 0x0043c930
void FUN_0043c930(void) {
    struct BarrelNode *node = SpinningBarrelList;
    while (node != NULL) {
        FUN_0043c7f0(node);
        node = node->next;
    }
}

// FUNCTION: LEGOLAND 0x0043c950
void SpinningBarrelsUpdate(struct Element *elem) {
    int view_x;
    int field_73;
    struct Ride *ride = elem->ride;
    struct RideNode *rn = ride->riders;
    struct RideNode *next;
    struct BarrelNode *node;
    struct Bloke *bloke;
    TileId *pos;
    int iv12, iv13;
    int tw, th;
    int coords[2];
    int coords2[2];

    while (rn) {
        next = rn->next;
        bloke = rn->rider;
        pos = &rn->tile;
        node = FUN_0043be40(&pos->id);
        if (node == NULL) {
            return;
        }
        iv12 = ride->x + pos->pos.x;
        iv13 = ride->y + pos->pos.y;
        if (bloke->low_level_action == 0) {
            switch (bloke->param_action) {
            case 0:
                node->boarding_count++;
                iv13 -= 5;
                node->boarding_timer = 0x190;
                bloke->flags = bloke->flags | 8;
                iv12 <<= 8;
                iv13 <<= 8;
                bloke->dest.x = iv12;
                bloke->dest.y = iv13;
                bloke->field_73 = CalcMoveLine(bloke->pos, bloke->dest, &bloke->nav) + 0x10;
                bloke->low_level_action = 7;
                NewDirForAction(bloke, (bloke->field_73 >> 5) + 3);
                bloke->param_action++;
                bloke->field_58 = 0;
                break;
            case 1: {
                struct Point sc;
                int ix, iy;

                sc = GetScreenCoordsForObject(pos, ride);
                iy = bloke->pos.y;
                ix = bloke->pos.x;
                GetTileDimensions(&tw, &th);
                iv13 = (ix + iy) * th;
                iv12 = (ix - iy) * tw;
                iv12 >>= 9;
                iv13 >>= 9;
                view_x = lpConfig->view_x;
                coords[0] = ((((unsigned int)view_x - Get_XScroll()) + iv12) - DAT_0062fdd8.x / 2 - sc.x) * 2;
                coords[1] = ((iv13 + ((unsigned int)lpConfig->view_y - Get_YScroll())) - DAT_0062fdd8.y / 2 - sc.y) * 2;
                bloke->person->sprite = DAT_0062fe00[1];
                bloke->person->field_30 = 1;
                bloke->person->depth = GetUnitDepth(-1617922.25f, -1618065.75f);
                bloke->field_35 = 0;
                // STRING: LEGOLAND 0x004b4704
                sprintf(BoxBlokeBnvName + 8, "%02d", PickBarrelSeat(rn, node, (char)SpinningBarrelsRide->seats));
                bloke->path = NewBNVPath(DAT_0062fdf0[0], 0, BoxBlokeBnvName, -1617922.25f, -1618065.75f, coords);
                bloke->param_action++;
                break;
            }
            case 2:
                bloke->flags |= 0x80;
                if (UpdateBlokeFromBNVPath(bloke, bloke->path) == 0) {
                    bloke->field_35 = 1;
                    bloke->param_action = 6;
                }
                BlokeSetFrame(bloke, bloke->frame);
                break;
            case 6: {
                BlokeSitAnim(bloke);
            }
                BlokeSetFrame(bloke, 0);
                bloke->param_action++;
                if ((short)(signed char)++node->seated_count == SpinningBarrelsRide->seats) {
                    FUN_0043c320(node);
                }
                break;
            case 8:
                coords2[0] = bloke->screen_x * 2;
                coords2[1] = bloke->screen_y * 2;
                BlokeWalkAnim(bloke);
                BlokeSetFrame(bloke, 0);
                bloke->person->sprite = DAT_0062fe00[1];
                bloke->person->field_30 = 1;
                bloke->person->depth = GetUnitDepth(-1617922.25f, -1618065.75f);
                bloke->field_35 = 2;
                sprintf(BoxBlokeBnvName + 8, "%02d", bloke->field_36);
                bloke->path = NewBNVPath(DAT_0062fdf0[2], 2, BoxBlokeBnvName, -1617922.25f, -1618065.75f, coords2);
                bloke->param_action++;
                break;
            case 9:
                if (UpdateBlokeFromBNVPath(bloke, bloke->path) == 0) {
                    bloke->param_action = 0xf;
                    free(bloke->path);
                    bloke->path = NULL;
                }
                BlokeSetFrame(bloke, bloke->frame);
                break;
            case 0xf:
                bloke->flags &= 0xff7f;
                node->slots[bloke->field_36 - 1] = 0;
                iv12 <<= 8;
                iv13 <<= 8;
                bloke->pos.x = iv12 + 0x280;
                bloke->pos.y = iv13 - 0x180;
                bloke->dest.x = iv12 + 0x180;
                bloke->dest.y = iv13 - 0x80;
                bloke->field_73 = CalcMoveLine(bloke->pos, bloke->dest, &bloke->nav) + 0x10;
                bloke->low_level_action = 7;
                NewDirForAction(bloke, (bloke->field_73 >> 5) + 3);
                bloke->param_action++;
                break;
            case 0x10:
                bloke->dest.x = ((2 + iv12) << 8) - (rand() % 2 ? 0x80 : 0);
                bloke->dest.y = (iv13 << 8) + 0x80;
                bloke->field_73 = CalcMoveLine(bloke->pos, bloke->dest, &bloke->nav) + 0x10;
                bloke->low_level_action = 7;
                field_73 = bloke->field_73;
                NewDirForAction(bloke, (field_73 >> 5) + 3);
                bloke->param_action++;
                break;
            case 0x11:
                RemoveBlokeFromRide(ride, rn);
                bloke->flags = bloke->flags & 0xfff7;
                if (--node->leaving_count == 0) {
                    node->seated_count = 0;
                    Ride_ClearFlagToNotLetAnyoneOn(&node->tile_id);
                }
                break;
            }
        }
        rn = next;
    }
    FUN_0043c930();
}

// FUNCTION: LEGOLAND 0x0043ce10
unsigned int PickBarrelSeat(struct RideNode *rn, struct BarrelNode *node, signed char n) {
    int count = n;
    int eax = rand();
    int index = eax % count;
    signed char slot = node->slots[index];

    while (slot != 0) {
        index++;
        if (index >= count) {
            index = 0;
        }
        slot = node->slots[index];
    }

    node->slots[index] = 1;
    rn->rider->field_36 = (signed char)(index + 1);
    return index + 1;
}
