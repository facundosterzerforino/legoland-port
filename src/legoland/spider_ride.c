#include <stdlib.h>
#include <string.h>
#include "globals.h"
#include "legoland.h"

#include <stdio.h>
#include "binv.h"
#include "bloke.h"
#include "gamemap.h"
#include "llidb.h"
#include "man3d.h"
#include "map_object.h"
#include "math.h"
#include "obj_instance.h"
#include "objclass.h"
#include "print_sprite.h"
#include "render3d.h"
#include "sound_music.h"
#include "spider_ride.h"
#include "tilemap.h"

struct FadeParams {
    unsigned int field_0;
    unsigned char pad_4[4];
    unsigned int field_8;
    unsigned int field_c;
};

struct SpiderState {
    unsigned short id;
    unsigned char field_2;
    unsigned char field_3;
    char frame;
    unsigned char pad_5[3];
    unsigned int field_8;
    unsigned char field_c;
    unsigned char pad_d[3];
    unsigned int field_10;
    unsigned char field_14;
    unsigned char pad_15[3];
    int field_18;
};

struct CarNode {
    /* 0x00 */ unsigned char pad_0[0x10];
    /* 0x10 */ unsigned int field_10;
    /* 0x14 */ unsigned int field_14;
    /* 0x18 */ unsigned int field_18;
    /* 0x1c */ unsigned char pad_1c[0x48];
    /* 0x64 */ struct CarNode *next;
};

struct SlotBloke {
    unsigned char pad_0[0x36];
    unsigned char field_36;
};

struct SlotOwner {
    unsigned char pad_0[8];
    struct SlotBloke *rider;
};

struct SlotArray {
    unsigned char pad_0[0x1c];
    unsigned char slots[1];
};

#include "image_sprite.h"

// FUNCTION: LEGOLAND 0x004158f0
int AddSpiderNode(TileId *key) {
    struct SpiderNode *node = (struct SpiderNode *)malloc(0x30);
    if (node == NULL) {
        return;
    }
    memset(node, 0, 0x30);
    node->tile_id = key->id;
    node->next = SpiderNodeList;
    SpiderNodeList = node;
    return FUN_00415a90(node);
}

// FUNCTION: LEGOLAND 0x00415930
void RemoveSpiderNode(struct SpiderNode *node) {
    struct SpiderNode *prev;
    struct SpiderNode *cur;

    if (SpiderNodeList == node) {
        SpiderNodeList = node->next;
    } else {
        cur = SpiderNodeList->next;
        prev = SpiderNodeList;
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

// FUNCTION: LEGOLAND 0x00415990
void FreeAllSpiderNodes(void) {
    struct SpiderNode *node = SpiderNodeList;
    while (node != NULL) {
        RemoveSpiderNode(node);
        node = SpiderNodeList;
    }
}

// FUNCTION: LEGOLAND 0x004159b0
struct SpiderNode *FindSpiderNode(TileId *key) {
    struct SpiderNode *cur = SpiderNodeList;

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

// FUNCTION: LEGOLAND 0x004159e0
void FUN_004159e0(const unsigned char *arg0) {
    struct SampleParams params;
    params.field_0 = 0x2;
    params.x = arg0[0];
    params.y = arg0[1];
    PlayInstanceOfSample(DAT_004b4d90, 1, 1, &params);
}

// FUNCTION: LEGOLAND 0x00415a20
void FUN_00415a20(TileId *tile) {
    struct FadeParams params;
    params.field_0 = 2;
    params.field_8 = tile->pos.x;
    params.field_c = tile->pos.y;
    UnSourceAndFadeAllSamplesFromSource(&params, 0xffffff38);
}

// FUNCTION: LEGOLAND 0x00415a60
void FUN_00415a60(struct SpiderState *a) {
    a->field_3 = a->field_2;
    a->field_2 = 0;
    a->field_8 = (a->field_8 & 0xffffbfff) | 0x1;
    a->frame = 0;
    a->field_10 = 0;
    FUN_004159e0((const unsigned char *)a);
}

// FUNCTION: LEGOLAND 0x00415a90
int FUN_00415a90(struct SpiderNode *node) {
    struct SpiderState *a = (struct SpiderState *)node;
    a->field_10 = 0;
    a->frame = 0;
    a->field_c = rand() % 2 != 0 ? 4 : 3;
    a->field_8 &= 0xffffbffe;
    a->field_14 = 0;
    a->field_2 = 0;
    FUN_00415a20((TileId *)node);
}

// FUNCTION: LEGOLAND 0x00415ae0
void RenderSpider(Element *obj, void *param_2, void *param_3, TileId *tile, unsigned int param_5, unsigned int param_6) {
    Ride *ride = obj->ride;
    RideNode *elem = ride->riders;
    char count = 0;
    Bloke *blokes[16] = {0};
    struct SpiderNode *state;
    Point off;
    Point screen;
    char i;
    unsigned short id;

    state = FindSpiderNode(tile);
    if (state == NULL) {
        return;
    }
    screen = GetScreenCoordsForObject(tile, ride);
    if (elem != NULL) {
        do {
            id = tile->id;
            if (id == elem->tile.id) {
                blokes[count++] = elem->rider;
            }
            elem = elem->next;
        } while (elem != NULL);
        if (count != 0) {
            for (i = 0; i < count; i++) {
                if (blokes[i]->param_action == 14) {
                    IP_RenderBlokeIn3DNow(blokes[i]);
                }
            }
            LLSSetFrame(GetLLSForLayer(SpiderRideLayer, 1), state->frame);
            off = GetRenderOffsetForLayer(SpiderRideLayer, 1);
            AdjustOffsetForViewMode(&off);
            PrintSprite(GetSpriteForLayer(SpiderRideLayer, 1), screen.x + off.x, screen.y + off.y, param_6, 0);
            off = GetRenderOffsetForLayer(SpiderRideLayer, 2);
            AdjustOffsetForViewMode(&off);
            PrintSprite(SpiderHutMask2Sprite, screen.x + off.x, screen.y + off.y, param_6, 0);
            *(short *)*ZSpiderSprite->lls = state->frame;
            for (elem = ride->riders; elem != NULL; elem = elem->next) {
                Bloke *b;
                if (tile->id == elem->tile.id && ((b = elem->rider)->flags & 0x80) != 0) {
                    Person *p = b->person;
                    Point base;
                    Point adj;
                    base = DAT_0082c660;
                    adj.x = 0;
                    adj.y = 0;
                    if (b->field_35 == 1) {
                        adj = DAT_004b4e20;
                        AdjustOffsetForViewMode(&adj);
                    }
                    p->offset.x = b->screen_x;
                    p->offset.y = b->screen_y;
                    AdjustBlokePosition(&p->offset);
                    AdjustOffsetForViewMode(&base);
                    p->screen.x = b->screen_x + base.x + screen.x;
                    p->screen.y = b->screen_y + base.y + screen.y;
                    p->screen.x += adj.x;
                    p->screen.y += adj.y;
                    AdjustBlokePosition(&p->screen);
                    IP_RenderBlokeIn3DNow(elem->rider);
                }
            }
            off = GetRenderOffsetForLayer(SpiderRideLayer, 2);
            AdjustOffsetForViewMode(&off);
            PrintSprite(SpiderHutMask1Sprite, screen.x + off.x, screen.y + off.y, param_6, 0);
            return;
        }
    }
    {
        LLSSetFrame(GetLLSForLayer(SpiderRideLayer, 1), state->frame);
        off = GetRenderOffsetForLayer(SpiderRideLayer, 1);
        AdjustOffsetForViewMode(&off);
        PrintSprite(GetSpriteForLayer(SpiderRideLayer, 1), screen.x + off.x, screen.y + off.y, param_6, 0);
        off = GetRenderOffsetForLayer(SpiderRideLayer, 2);
        AdjustOffsetForViewMode(&off);
        PrintSprite(GetSpriteForLayer(SpiderRideLayer, 2), screen.x + off.x, screen.y + off.y, param_6, 0);
    }
}

// FUNCTION: LEGOLAND 0x00415e80
void FUN_00415e80(struct CarNode *param_1) {
    DAT_004cbf20 = ((unsigned int *)param_1)[3];
    ((unsigned int *)DAT_004cbf20)[7] |= 0x420;
    SpiderRideLayer = (struct Sprite *)((unsigned int *)DAT_004cbf20)[25];
    SpiderRideLayer->flags |= 0x2000;
    // STRING: LEGOLAND 0x004b4ea0
    SpiderRunBinV = LoadBinV("Zbuffers\\spiderrun.bnv");
    // STRING: LEGOLAND 0x004b4e88
    SpiderOnBinV = LoadBinV("Zbuffers\\spideron.bnv");
    // STRING: LEGOLAND 0x004b4e70
    SpiderOffBinV = LoadBinV("Zbuffers\\spideroff.bnv");
    // STRING: LEGOLAND 0x004b4e5c
    SpiderHutMask1Sprite = LoadSprite("SpiderHutMask1.lls", 1);
    // STRING: LEGOLAND 0x004b4e48
    SpiderHutMask2Sprite = LoadSprite("SpiderHutMask2.lls", 1);
    // STRING: LEGOLAND 0x004b4e38
    ZSpiderSprite = LoadSprite("z_spider.lls", 1);
    DAT_004cbf30[0] = SpiderRunBinV;
    DAT_0082c660.x = -1;
    DAT_0082c660.y = 2;
    DAT_004cbf38[1] = ZSpiderSprite;
    DAT_004cbf30[1] = SpiderOnBinV;
    DAT_004cbf38[0] = SpiderOffBinV;
    HideLayer(SpiderRideLayer, 2);
    StopLayerPlaying(SpiderRideLayer, 2);
    LLSSetFrame(GetLLSForLayer(SpiderRideLayer, 2), 0);
    HideLayer(SpiderRideLayer, 1);
    StopLayerPlaying(SpiderRideLayer, 1);
    LLSSetFrame(GetLLSForLayer(SpiderRideLayer, 1), 0);
    Load_FXList(SpiderRide_SFX, 1);
}

// FUNCTION: LEGOLAND 0x00415fd0
void FUN_00415fd0(struct CarNode *param_1) {
    DAT_004cbf20 = ((unsigned int *)param_1)[3];

    if (ZSpiderSprite != NULL) {
        KillSprite(ZSpiderSprite);
    }
    if (SpiderRunBinV != 0) {
        FreeBinV(SpiderRunBinV);
    }
    if (SpiderOnBinV != 0) {
        FreeBinV(SpiderOnBinV);
    }
    if (SpiderOffBinV != 0) {
        FreeBinV(SpiderOffBinV);
    }
    KillSprite(SpiderHutMask1Sprite);
    KillSprite(SpiderHutMask2Sprite);
    FreeAllSpiderNodes();
    Kill_FXList(SpiderRide_SFX, 1);
}

// FUNCTION: LEGOLAND 0x00416060
void SpiderSetEditMode(void) {
    EditMode.unk0 = 1;
    EditMode.unk8 = (void *)DAT_004cbf20;
    DefaultCursor(&EditCursor);
    SetEditCursorFootPrint((char *)EditMode.unk8 + 0x3c);
}

// FUNCTION: LEGOLAND 0x004160a0
void SpiderRemoveObject(Element *obj, TileId tile, struct Cursor *cursor) {
    struct SpiderNode *node = FindSpiderNode(&tile);

    if (node != NULL) {
        RemoveSpiderNode(node);
    }
    FUN_00415a20(&tile);
    StandardRemoveObject(obj, tile, cursor);
    RemoveAllBlokesFromRide(obj->ride, tile);
}

// FUNCTION: LEGOLAND 0x004160f0
void SpiderAddObject(Element *editObj, int *coords) {
    TileId key;

    key.pos.x = coords[0];
    key.pos.y = coords[1];
    AddBasicObject(editObj, coords);
    AddSpiderNode(&key);
}

// FUNCTION: LEGOLAND 0x00416120
unsigned int *FUN_00416120(unsigned int *a1, unsigned short a2) {
    struct CarNode *car = *(struct CarNode **)((unsigned char *)a1 + 0xc);
    DAT_004cbf40 = (unsigned int)car->next;
    DAT_004cbf44 = car->field_14;
    DAT_004cbf48 = car->field_18;
    DAT_004cbf4c = a2;
    car = car->next;
    car->field_10 = car->field_10 | 0x2000;
    return &DAT_004cbf40;
}

// FUNCTION: LEGOLAND 0x00416160
void SpiderRide(struct ClassNode *name_ptr, struct CallbackTable *obj) {
    // STRING: LEGOLAND 0x004b4eb8
    if (_stricmp("SPIDER RIDE", name_ptr->name) == 0) {
        obj->cb_a4 = FUN_00415e80;
        obj->cb_ac = FUN_00415fd0;
        obj->cb_8c = SpiderSetEditMode;
        obj->cb_a8 = FUN_00416330;
        obj->cb_b0 = RenderSpider;
        obj->cb_9c = SpiderRemoveObject;
        obj->cb_98 = SpiderAddObject;
        obj->cb_a0 = FUN_00416120;
        obj->cb_bc = SaveSpider;
        obj->cb_b8 = LoadSpider;
    }
}

// FUNCTION: LEGOLAND 0x004161f0
void FUN_004161f0(struct SpiderNode *node) {
    struct SpiderState *s = (struct SpiderState *)node;
    struct RideNode *r = ((struct Ride *)DAT_004cbf20)->riders;
    unsigned int flags = s->field_8;

    if (flags & 1) {
        unsigned char c;
        int v = ++s->field_10;
        c = s->field_c;
        if (c == 0) {
            if (GetAllBlokesOffRide((struct Ride *)DAT_004cbf20, s->id) == 0) {
                return;
            }
            FUN_00415a90(node);
            return;
        }
        if (v >= 2) {
            s->field_10 = 0;
            s->frame++;
            if (s->frame >= 0x20) {
                s->frame = 0;
                s->field_c = c - 1;
            }
        }
    } else if (flags & 0x4000) {
        if (s->field_2 == s->field_14) {
            s->field_8 = flags & 0xffffbfff;
            FUN_00415a60(s);
            return;
        }
    } else if (s->field_2 != 0) {
        if (s->field_18 == 0) {
            s->field_8 = flags | 0x4000;
            Ride_SetFlagToNotLetAnyoneOn(s);
        }
        s->field_18--;
    }
    for (; r != NULL; r = r->next) {
        if (s->id == r->tile.id && r->rider->field_35 == 1) {
            sprintf(SpiderBnvInfo.name + 6, "%02d", r->rider->field_36);
            SetBlokePositionFromBNV(SpiderRunBinV, r->rider, SpiderBnvInfo.name, s->frame, -1617787.75f, -1618096.5f, 0);
        }
    }
    *(short *)*ZSpiderSprite->lls = (short)s->frame;
}

// FUNCTION: LEGOLAND 0x00416310
void FUN_00416310(void) {
    struct SpiderNode *node = SpiderNodeList;
    while (node != NULL) {
        FUN_004161f0(node);
        node = node->next;
    }
}

// FUNCTION: LEGOLAND 0x00416330
void FUN_00416330(Element *obj) {
    struct Ride *ride = obj->ride;
    struct RideNode *elem = ride->riders;
    struct SpiderState *state;
    struct Bloke *bloke;
    TileId *tile;
    struct RideNode *next;
    struct Point sc;
    int h, w;
    int ex, ey;
    int coords[2];
    int walk[2];
    int dx, dy;
    char dir;
    int ox, oy;

    FUN_00416310();
    while (elem != NULL) {
        next = elem->next;
        bloke = elem->rider;
        tile = &elem->tile;
        state = (struct SpiderState *)FindSpiderNode(tile);
        if (state == NULL) {
            return;
        }
        if (bloke->low_level_action == 0) {
            switch (bloke->param_action) {
            case 0:
                state->field_14++;
                state->field_18 = 0xb4;
                bloke->flags |= 8;
                sc = GetScreenCoordsForObject(tile, ride);
                dy = bloke->pos.y;
                dx = bloke->pos.x;
                GetTileDimensions(&w, &h);
                ey = (dx + dy) * h >> 9;
                ex = (dx - dy) * w >> 9;
                ox = lpConfig->view_x - (short)Get_XScroll() + ex;
                oy = ey + (lpConfig->view_y - (short)Get_YScroll());
                coords[0] = (ox - DAT_0082c660.x / 2 - sc.x) * 2;
                coords[1] = (oy - DAT_0082c660.y / 2 - sc.y) * 2;
                bloke->flags |= 0x80;
                bloke->person->sprite = (struct Sprite *)DAT_004cbf38[1];
                bloke->person->field_30 = 1;
                bloke->person->depth = GetUnitDepth(-1617787.75f, -1618096.5f);
                bloke->field_35 = 0;
                // STRING: LEGOLAND 0x004b4704
                sprintf(SpiderBnvInfo.name + 6, "%02d", FUN_00416830((struct SlotOwner *)elem, (struct SlotArray *)state, ((struct Ride *)DAT_004cbf20)->seats));
                bloke->path = NewBNVPath(DAT_004cbf30[1], 1, SpiderBnvInfo.name, -1617787.75f, -1618096.5f, coords);
                UpdateBlokeFromBNVPath(bloke, bloke->path);
                bloke->field_58 = 0;
                bloke->param_action++;
                break;
            case 1:
                if (UpdateBlokeFromBNVPath(bloke, bloke->path) == 0) {
                    bloke->field_35 = 1;
                    bloke->param_action = 5;
                    free(bloke->path);
                    bloke->path = NULL;
                }
                if (bloke->path != NULL && (int)BNVPath_GetDFrame(bloke->path) >= SpiderBnvInfo.tab1[bloke->field_36]) {
                    bloke->field_35 = 1;
                    bloke->param_action = 5;
                    free(bloke->path);
                    bloke->path = NULL;
                }
                BlokeSetFrame(bloke, bloke->frame);
                break;
            case 5:
                bloke->flags |= 0x80;
                BlokeSitAnim(bloke);
                BlokeSetFrame(bloke, 0);
                bloke->person->sprite = (struct Sprite *)DAT_004cbf38[1];
                bloke->field_35 = 1;
                bloke->person->field_30 = 1;
                bloke->person->depth = GetUnitDepth(-1617787.75f, -1618096.5f);
                bloke->param_action++;
                state->field_2++;
                if ((short)(char)state->field_2 == ((struct Ride *)DAT_004cbf20)->seats) {
                    FUN_00415a60(state);
                }
                break;
            case 7:
                walk[0] = bloke->field_38 << 1;
                walk[1] = bloke->field_3a << 1;
                BlokeWalkAnim(bloke);
                BlokeSetFrame(bloke, 0);
                bloke->flags |= 0x80;
                bloke->person->sprite = (struct Sprite *)DAT_004cbf38[1];
                bloke->person->field_30 = 1;
                bloke->person->depth = GetUnitDepth(-1617787.75f, -1618096.5f);
                bloke->field_35 = 2;
                sprintf(SpiderBnvInfo.name + 6, "%02d", bloke->field_36);
                bloke->path = NewBNVPath(DAT_004cbf38[0], 2, SpiderBnvInfo.name, -1617787.75f, -1618096.5f, walk);
                BNVPath_SetDFrame(bloke, bloke->path, 0);
                UpdateBlokeFromBNVPath(bloke, bloke->path);
                bloke->param_action++;
                break;
            case 8:
                if (UpdateBlokeFromBNVPath(bloke, bloke->path) == 0) {
                    bloke->field_35 = 2;
                    bloke->param_action = 0xd;
                    free(bloke->path);
                    bloke->path = NULL;
                }
                if (bloke->path != NULL && (int)BNVPath_GetDFrame(bloke->path) >= SpiderBnvInfo.tab2[bloke->field_36]) {
                    bloke->field_35 = 2;
                    bloke->param_action = 0xd;
                    free(bloke->path);
                    bloke->path = NULL;
                }
                BlokeSetFrame(bloke, bloke->frame);
                break;
            case 0xd:
                ex = ride->field_24 + tile->pos.x;
                ey = tile->pos.y + ride->field_25;
                ((unsigned char *)state)[0x1b + bloke->field_36] = 0;
                bloke->flags &= 0xff7f;
                bloke->person->sprite = NULL;
                bloke->person->field_30 = 0;
                UnAdjustBlokePosition(&bloke->person->screen);
                ScreenToMapRef((int *)&bloke->person->screen, (int *)&bloke->pos, 0);
                bloke->person->field_34 = 0;
                bloke->dest.y = ey << 8;
                bloke->pos.y <<= 8;
                bloke->pos.x <<= 8;
                bloke->dest.x = ex << 8;
                dir = CalcMoveLine(bloke->pos, bloke->dest, &bloke->nav);
                bloke->low_level_action = 7;
                bloke->field_73 = dir + 0x10;
                NewDirForAction(bloke, (bloke->field_73 >> 5) + 3);
                bloke->param_action++;
                break;
            case 0xe:
                RemoveBlokeFromRide(ride, elem);
                bloke->flags &= 0xfff7;
                state->field_3--;
                if (state->field_3 == 0) {
                    state->field_2 = 0;
                    Ride_ClearFlagToNotLetAnyoneOn(state);
                }
                break;
            }
        }
        elem = next;
    }
}

// FUNCTION: LEGOLAND 0x00416830
int FUN_00416830(struct SlotOwner *owner, struct SlotArray *arr, signed char count) {
    int n = count;
    int i = rand() % n;

    if (arr->slots[i] != 0) {
        do {
            i++;
            if (i >= n) {
                i = 0;
            }
        } while (arr->slots[i] != 0);
    }

    arr->slots[i] = 1;
    owner->rider->field_36 = (unsigned char)(i + 1);
    return i + 1;
}

// FUNCTION: LEGOLAND 0x00416880
LEGO_EXPORT int SaveSpider(void) {
    struct SpiderNode *node = SpiderNodeList;
    unsigned int one = 1;
    unsigned int zero = 0;

    while (node != NULL) {
        if (SaveGameWrite(&one, 4) == 0) {
            return 0;
        }
        if (SaveGameWrite(node, 0x30) == 0) {
            return 0;
        }
        node = node->next;
    }
    return SaveGameWrite(&zero, 4) != 0;
}

struct SpiderTypeC {
    void *file;
    unsigned int index;
};

struct SpiderData {
    unsigned char pad_0[0x54];
    struct SpiderTypeC *field_54;
};

struct SpiderCar2 {
    unsigned char pad_0[0x2c];
    void *field_2c;
    unsigned int field_30;
};

struct SpiderListNode {
    struct SpiderListNode *next;
    unsigned char pad_4[4];
    struct SpiderData *rider;
    unsigned char pad_c[4];
    struct SpiderCar2 *person;
};

struct SpiderGameObject {
    unsigned char pad_0[0xcc];
    struct SpiderListNode *riders;
};

struct SpiderLoadArg {
    unsigned char pad_0[0xc];
    struct SpiderGameObject *ride;
};

// FUNCTION: LEGOLAND 0x004168f0
LEGO_EXPORT int LoadSpider(struct SpiderLoadArg *arg) {
    struct SpiderGameObject *obj = arg->ride;
    struct SpiderNode *prev = NULL;
    struct SpiderListNode *list;
    struct SpiderCar2 *car;
    struct SpiderData *data;
    struct SpiderTypeC *tc;
    unsigned int marker;

    if (!SaveGameRead(&marker, 4)) {
        return 0;
    }
    while (marker != 0) {
        struct SpiderNode *node = (struct SpiderNode *)malloc(0x30);
        if (!SaveGameRead(node, 0x30)) {
            return 0;
        }
        node->next = NULL;
        if (prev != NULL) {
            prev->next = node;
        } else {
            SpiderNodeList = node;
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
            car->field_2c = DAT_004cbf38[car->field_30];
        } else {
            car->field_2c = NULL;
            list->person->field_30 = 0;
        }
        data = list->rider;
        tc = data->field_54;
        if (tc != NULL) {
            tc->file = DAT_004cbf30[tc->index];
        }
        list = list->next;
    }
    return 1;
}
