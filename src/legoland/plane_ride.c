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
#include "plane_ride.h"
#include "print_sprite.h"
#include "render3d.h"
#include "sound_music.h"
#include "sound_sfx.h"
#include "tilemap.h"

struct PlaneRideNode {
    unsigned char b0;
    unsigned char b1;
    unsigned char b2;
    unsigned char b3;
    char b4;
    char b5;
    unsigned char pad_6[2];
    unsigned int flags;
    char b12;
    unsigned char pad_d[3];
    unsigned int f10;
    unsigned char b14;
    unsigned char pad_15[3];
    int f18;
    signed char slots[4];
    struct PlaneRideNode *next;
};

struct PlaneRideObject {
    unsigned char pad_0[0xc];
    unsigned int ride;
};

struct PlaneRideBlockData {
    unsigned char pad_0[0x10];
    unsigned int field_10;
};

struct PlaneRideBlock {
    unsigned char pad_0[0x14];
    unsigned int field_14;
    unsigned int field_18;
    unsigned char pad_1c[0x48];
    struct PlaneRideBlockData *layer;
};

struct PlaneRideRoot {
    unsigned char pad_0[0xc];
    struct PlaneRideBlock *field_c;
};

#include "image_sprite.h"
#include "objclass.h"

// FUNCTION: LEGOLAND 0x0043d880
void FUN_0043d880(void *param_1) {
    void *node = malloc(0x24);
    if (node == NULL) {
        return;
    }
    memset(node, 0, 0x24);
    *(unsigned short *)node = *(unsigned short *)param_1;
    ((unsigned int *)node)[8] = (unsigned int)PlaneRideNodeList;
    PlaneRideNodeList = node;
    FUN_0043d9f0(node);
}

// FUNCTION: LEGOLAND 0x0043d8c0
void RemovePlaneRideNode(struct PlaneRideNode *node) {
    struct PlaneRideNode *prev;
    struct PlaneRideNode *cur;
    struct SampleSource src;

    if (PlaneRideNodeList == node) {
        PlaneRideNodeList = node->next;
    } else {
        cur = PlaneRideNodeList->next;
        prev = PlaneRideNodeList;
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
    src.type = 2;
    src.x = node->b0;
    src.y = node->b1;
    UnSourceAndFadeAllSamplesFromSource(&src, -200);
    free(node);
}

// FUNCTION: LEGOLAND 0x0043d940
void RemoveAllPlaneRideNodes(void) {
    while (PlaneRideNodeList != NULL) {
        RemovePlaneRideNode(PlaneRideNodeList);
    }
}

// FUNCTION: LEGOLAND 0x0043d960
unsigned int FindPlaneRideNode(TileId *arg) {
    struct PlaneRideNode *cur = PlaneRideNodeList;

    if (cur != NULL) {
        do {
            if (memcmp(&cur->b0, arg, 2) == 0) {
                return (unsigned int)cur;
            }
            cur = cur->next;
        } while (cur != NULL);
    }
    return 0;
}

// FUNCTION: LEGOLAND 0x0043d990
void FUN_0043d990(struct PlaneRideNode *node) {
    int cfg[4];

    cfg[0] = 2;
    node->flags &= ~0x4000;
    node->b3 = node->b2;
    node->flags |= 1;
    node->b2 = 0;
    node->b4 = 0;
    node->f10 = 0;
    cfg[2] = node->b0;
    cfg[3] = node->b1;
    PlayInstanceOfSample(*(void **)(DAT_004b79d0 + 8), 1, 1, cfg);
}

// FUNCTION: LEGOLAND 0x0043d9f0
void FUN_0043d9f0(struct PlaneRideNode *node) {
    int cfg[4];

    node->f10 = 0;
    node->b4 = 0;
    node->b12 = rand() % 2 != 0 ? 2 : 1;
    node->b14 = 0;
    node->b2 = 0;
    node->flags &= ~0x4001;
    cfg[0] = 2;
    cfg[2] = node->b0;
    cfg[3] = node->b1;
    UnSourceAndFadeAllSamplesFromSource(cfg, -200);
}

// FUNCTION: LEGOLAND 0x0043da60
void RenderPlaneRide(struct Element *element, unsigned int param_2, unsigned int param_3, TileId *tile, unsigned int param_5, unsigned int param_6) {
    struct Ride *ride;
    struct RideNode *r;
    struct Point coords;
    char i;
    struct Point off;
    struct PlaneRideNode *node;
    struct Bloke *riders[4] = {0};
    char n = 0;
    struct Bloke *bloke;
    struct Person *person;

    ride = element->ride;
    r = ride->riders;
    node = (struct PlaneRideNode *)FindPlaneRideNode(tile);
    if (node == NULL) {
        return;
    }
    coords = GetScreenCoordsForObject(tile, ride);
    if (r != NULL) {
        for (; r != NULL; r = r->next) {
            if (tile->id == r->tile.id) {
                riders[n++] = r->rider;
            }
        }
        if (n != 0) {
            for (i = 0; i < n; i++) {
                if (riders[i]->param_action == 0xd) {
                    IP_RenderBlokeIn3DNow(riders[i]);
                }
            }
            for (i = 0; i < n; i++) {
                if (riders[i]->param_action == 0xe) {
                    IP_RenderBlokeIn3DNow(riders[i]);
                }
            }
            LLSSetFrame(GetLLSForLayer(PlaneRideLayer, 1), node->b4);
            off = GetRenderOffsetForLayer(ride->layer, 1);
            AdjustOffsetForViewMode(&off);
            PrintSprite(GetSpriteForLayer(ride->layer, 1), coords.x + off.x, coords.y + off.y, param_6, 0);
            *(short *)*ZoomerSprite->lls = node->b4;
            for (r = ride->riders; r != NULL; r = r->next) {
                if (tile->id == r->tile.id) {
                    struct Point off2;
                    bloke = r->rider;
                    if (bloke->flags & 0x80) {
                        person = bloke->person;
                        off2.x = DAT_0081cae8;
                        off2.y = DAT_0081caec;
                        person->offset.x = bloke->screen_x;
                        person->offset.y = bloke->screen_y;
                        AdjustBlokePosition(&person->offset);
                        AdjustOffsetForViewMode(&off2);
                        person->screen.x = bloke->screen_x + coords.x + off2.x;
                        person->screen.y = bloke->screen_y + coords.y + off2.y;
                        AdjustBlokePosition(&person->screen);
                        IP_RenderBlokeIn3DNow(r->rider);
                    }
                }
            }
            LLSSetFrame(GetLLSForLayer(PlaneRideLayer, 2), node->b5);
            off = GetRenderOffsetForLayer(ride->layer, 2);
            AdjustOffsetForViewMode(&off);
            PrintSprite(GetSpriteForLayer(ride->layer, 2), coords.x + off.x, coords.y + off.y, param_6, 0);
            return;
        }
    }
    LLSSetFrame(GetLLSForLayer(PlaneRideLayer, 1), node->b4);
    off = GetRenderOffsetForLayer(ride->layer, 1);
    AdjustOffsetForViewMode(&off);
    PrintSprite(GetSpriteForLayer(ride->layer, 1), coords.x + off.x, coords.y + off.y, param_6, 0);
    LLSSetFrame(GetLLSForLayer(PlaneRideLayer, 2), node->b5);
    off = GetRenderOffsetForLayer(ride->layer, 2);
    AdjustOffsetForViewMode(&off);
    PrintSprite(GetSpriteForLayer(ride->layer, 2), coords.x + off.x, coords.y + off.y, param_6, 0);
}

// FUNCTION: LEGOLAND 0x0043dda0
void FUN_0043dda0(Element *input) {
    struct LayerResult layer;

    PlaneRide = input->ride;
    PlaneRide->flags |= 0x420;
    PlaneRideLayer = PlaneRide->layer;
    PlaneRideLayer->flags |= 0x2000;
    // STRING: LEGOLAND 0x004b7a54
    ZoomerideBinV = LoadBinV("Zbuffers\\Zoomeride.bnv");
    // STRING: LEGOLAND 0x004b7a3c
    Zoomer0nBinV = LoadBinV("Zbuffers\\Zoomer0n.bnv");
    // STRING: LEGOLAND 0x004b7a24
    Zoomer0ffBinV = LoadBinV("Zbuffers\\Zoomer0ff.bnv");
    // STRING: LEGOLAND 0x004b7a14
    DAT_0062fe98 = ZoomerSprite = LoadSprite("z_Zoomer.lls", 1);
    DAT_0081cae8 = -10;
    DAT_0081caec = -0x6b;
    DAT_0062fe84[0] = ZoomerideBinV;
    DAT_0062fe84[1] = Zoomer0nBinV;
    DAT_0062fe84[2] = Zoomer0ffBinV;
    Load_FXList(DAT_004b79d0, 2);
    HideLayer(PlaneRideLayer, 1);
    StopLayerPlaying(PlaneRideLayer, 1);
    LLSSetFrame(GetLLSForLayer(PlaneRideLayer, 1), 0);
    HideLayer(PlaneRideLayer, 2);
    StopLayerPlaying(PlaneRideLayer, 2);
    LLSSetFrame(GetLLSForLayer(PlaneRideLayer, 2), 0);
    GetLayer(PlaneRide->layer, &layer, 1);
}

// FUNCTION: LEGOLAND 0x0043dee0
void FUN_0043dee0(struct PlaneRideObject *input) {
    PlaneRide = (struct Ride *)input->ride;
    if (ZoomerSprite) {
        KillSprite(ZoomerSprite);
    }
    if (ZoomerideBinV) {
        FreeBinV(ZoomerideBinV);
    }
    if (Zoomer0nBinV) {
        FreeBinV(Zoomer0nBinV);
    }
    if (Zoomer0ffBinV) {
        FreeBinV(Zoomer0ffBinV);
    }
    RemoveAllPlaneRideNodes();
    Kill_FXList(DAT_004b79d0, 2);
}

// FUNCTION: LEGOLAND 0x0043df50
void PlaneRideSetEditMode(void) {
    EditMode.unk0 = 1;
    EditMode.unk8 = (void *)PlaneRide;
    DefaultCursor(&EditCursor);
    SetEditCursorFootPrint((char *)EditMode.unk8 + 0x3c);
}

// FUNCTION: LEGOLAND 0x0043df90
void PlaneRideRemoveObject(struct PlaneRideObject *a1, TileId a2, struct PlaneRideObject *a3) {
    unsigned int temp = FindPlaneRideNode(&a2);
    if (temp != 0) {
        RemovePlaneRideNode((void *)temp);
    }
    StandardRemoveObject((unsigned int)a1, a2, (unsigned int)a3);
    RemoveAllBlokesFromRide(a1->ride, a2);
}

// FUNCTION: LEGOLAND 0x0043dfe0
void PlaneRideAddObject(Element *a, int *p) {
    unsigned char c[2];

    c[0] = *(unsigned char *)p;
    c[1] = ((unsigned char *)p)[4];
    AddBasicObject(a, p);
    FUN_0043d880(c);
}

// FUNCTION: LEGOLAND 0x0043e010
unsigned int *FUN_0043e010(struct PlaneRideRoot *param1, unsigned short param2) {
    struct PlaneRideBlock *block = param1->field_c;

    DAT_0062fe60.sprite = block->layer;
    DAT_0062fe60.x = block->field_14;
    DAT_0062fe60.y = block->field_18;
    DAT_0062fe60.id = param2;
    block->layer->field_10 |= 0x2000;

    return (void *)&DAT_0062fe60;
}

// FUNCTION: LEGOLAND 0x0043e050
unsigned int FUN_0043e050(struct RideNode *rn, struct PlaneRideNode *node, signed char n) {
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

// FUNCTION: LEGOLAND 0x0043e0a0
LEGO_EXPORT int SaveZoomer(void) {
    struct PlaneRideNode *current = PlaneRideNodeList;
    unsigned int flag = 1;
    unsigned int terminator = 0;

    while (current != NULL) {
        if (!SaveGameWrite(&flag, 4)) {
            return 0;
        }
        if (!SaveGameWrite(current, 36)) {
            return 0;
        }
        current = current->next;
    }

    if (SaveGameWrite(&terminator, 4)) {
        return 1;
    }
    return 0;
}

struct ZoomerSampleParams {
    int field_0;
    unsigned char pad_4[4];
    unsigned int field_8;
    unsigned int field_c;
};

struct ZoomerTypeC {
    void *field_0;
    unsigned int field_4;
};

struct ZoomerData {
    unsigned char pad_0[0x54];
    struct ZoomerTypeC *field_54;
};

struct ZoomerCar {
    unsigned char pad_0[0x2c];
    void *field_2c;
    unsigned int field_30;
};

struct ZoomerListNode {
    struct ZoomerListNode *next;
    unsigned char pad_4[4];
    struct ZoomerData *rider;
    unsigned char pad_c[4];
    struct ZoomerCar *person;
};

struct ZoomerGameObject {
    unsigned char pad_0[0xcc];
    struct ZoomerListNode *riders;
};

struct ZoomerLoadArg {
    unsigned char pad_0[0xc];
    struct ZoomerGameObject *field_c;
};

// FUNCTION: LEGOLAND 0x0043e110
LEGO_EXPORT int LoadZoomer(struct ZoomerLoadArg *arg) {
    struct ZoomerGameObject *obj = arg->field_c;
    struct PlaneRideNode *prev = NULL;
    struct ZoomerListNode *list;
    struct ZoomerCar *car;
    struct ZoomerData *data;
    struct ZoomerTypeC *tc;
    struct ZoomerSampleParams params;
    unsigned int marker;

    if (!SaveGameRead(&marker, 4)) {
        return 0;
    }
    while (marker != 0) {
        struct PlaneRideNode *node = (struct PlaneRideNode *)malloc(0x24);
        if (!SaveGameRead(node, 0x24)) {
            return 0;
        }
        node->next = NULL;
        if (prev != NULL) {
            prev->next = node;
        } else {
            PlaneRideNodeList = node;
        }
        params.field_8 = ((unsigned char *)node)[0];
        params.field_0 = 2;
        params.field_c = ((unsigned char *)node)[1];
        PauseSingleSample(PlayInstanceOfSample(*(void **)(DAT_004b79d0 + 8), 1, 1, &params));
        prev = node;
        if (!SaveGameRead(&marker, 4)) {
            return 0;
        }
    }

    list = obj->riders;
    while (list != NULL) {
        car = list->person;
        if (car->field_30 != 0) {
            /* [port] the original reads (&Zoomer0nBinV)[field_30]; field_30 is 1 here, the global after it */
            car->field_2c = car->field_30 == 1 ? (void *)DAT_0062fe98 : (&Zoomer0nBinV)[car->field_30];
        } else {
            car->field_2c = NULL;
            list->person->field_30 = 0;
        }
        data = list->rider;
        tc = data->field_54;
        if (tc != NULL) {
            tc->field_0 = DAT_0062fe84[tc->field_4];
        }
        list = list->next;
    }
    return 1;
}

// FUNCTION: LEGOLAND 0x0043e220
void PlaneRide_GetInterfaces(struct ClassNode *name, struct CallbackTable *iface) {
    // STRING: LEGOLAND 0x004b7a6c
    if (_stricmp("PLANE RIDE", name->name) == 0) {
        iface->cb_a4 = FUN_0043dda0;
        iface->cb_ac = FUN_0043dee0;
        iface->cb_8c = PlaneRideSetEditMode;
        iface->cb_a8 = PlaneRideUpdate;
        iface->cb_b0 = RenderPlaneRide;
        iface->cb_9c = PlaneRideRemoveObject;
        iface->cb_98 = PlaneRideAddObject;
        iface->cb_a0 = FUN_0043e010;
        iface->cb_b8 = LoadZoomer;
        iface->cb_bc = SaveZoomer;
    }
}

// FUNCTION: LEGOLAND 0x0043e2b0
void FUN_0043e2b0(struct PlaneRideNode *node) {
    struct RideNode *r = PlaneRide->riders;
    unsigned int flags;

    if (++node->b5 >= 0x18) {
        node->b5 = 0;
    }
    flags = node->flags;
    if (flags & 1) {
        int v = ++node->f10;
        char c = node->b12;
        if (c == 0) {
            if (GetAllBlokesOffRide(PlaneRide, *(unsigned short *)node) == 0) {
                return;
            }
            FUN_0043d9f0(node);
            return;
        }
        if (v >= 2) {
            node->f10 = 0;
            if (++node->b4 >= 0x61) {
                node->b4 = 0;
                node->b12 = c - 1;
            }
        }
    } else {
        unsigned char cur = node->b2;
        if (flags & 0x4000) {
            if (cur == node->b14) {
                node->flags = flags & 0xffffbfff;
                FUN_0043d990(node);
                return;
            }
        } else if (cur != 0) {
            int k = node->f18;
            if (k == 0) {
                node->flags = flags | 0x4000;
                Ride_SetFlagToNotLetAnyoneOn(node);
            } else {
                node->f18 = k - 1;
            }
        }
    }
    for (; r != NULL; r = r->next) {
        if (*(unsigned short *)node == r->tile.id && r->rider->field_35 == 1) {
            // STRING: LEGOLAND 0x004b4704
            sprintf(DAT_004b79bc + 6, "%02d", r->rider->field_36);
            SetBlokePositionFromBNV(ZoomerideBinV, r->rider, DAT_004b79bc, node->b4, -1617706.75f, -1617948.625f, 0);
        }
    }
    *(short *)*ZoomerSprite->lls = (short)node->b4;
}

// FUNCTION: LEGOLAND 0x0043e3f0
void FUN_0043e3f0(void) {
    struct PlaneRideNode *current = PlaneRideNodeList;

    while (current != NULL) {
        FUN_0043e2b0(current);
        current = current->next;
    }
}

// FUNCTION: LEGOLAND 0x0043e410
void PlaneRideUpdate(struct Element *elem) {
    struct Ride *ride = elem->ride;
    struct RideNode *rn = ride->riders;
    struct RideNode *next;
    struct PlaneRideNode *node;
    struct Bloke *bloke;
    TileId *pos;
    int iv12, iv13;
    int tw, th;
    int coords[2];
    struct Point tmp;
    int coords2[2];

    while (rn != NULL) {
        next = rn->next;
        bloke = rn->rider;
        pos = &rn->tile;
        node = (struct PlaneRideNode *)FindPlaneRideNode(pos);
        if (node == NULL) {
            return;
        }
        if (bloke->low_level_action == 0) {
            switch (bloke->param_action) {
            case 0: {
                struct Point sc;
                int ix, iy;
                short sXs, sYs;

                node->b14++;
                node->f18 = 0xb4;
                bloke->flags |= 8;
                sc = GetScreenCoordsForObject(pos, ride);
                ix = bloke->pos.x;
                iy = bloke->pos.y;
                GetTileDimensions(&tw, &th);
                iv13 = (ix + iy) * th;
                iv12 = (ix - iy) * tw;
                sXs = Get_XScroll();
                sYs = Get_YScroll();
                coords[0] = ((((unsigned int)lpConfig->view_x - (int)sXs) + (iv12 >> 9)) - DAT_0081cae8 / 2 - sc.x) * 2;
                coords[1] = (((iv13 >> 9) + ((unsigned int)lpConfig->view_y - (int)sYs)) - DAT_0081caec / 2 - sc.y) * 2;
                bloke->flags |= 0x80;
                bloke->person->sprite = DAT_0062fe98;
                bloke->person->field_30 = 1;
                bloke->person->depth = GetUnitDepth(-1617706.75f, -1617948.625f);
                bloke->field_35 = 0;
                // STRING: LEGOLAND 0x004b4704
                sprintf(DAT_004b79bc + 6, "%02d", FUN_0043e050(rn, node, (char)PlaneRide->seats));
                bloke->path = NewBNVPath(DAT_0062fe84[1], 1, DAT_004b79bc, -1617706.75f, -1617948.625f, coords);
                UpdateBlokeFromBNVPath(bloke, bloke->path);
                bloke->param_action++;
                bloke->field_58 = 0;
                break;
            }
            case 1:
                if (UpdateBlokeFromBNVPath(bloke, bloke->path) == 0) {
                    bloke->field_35 = 1;
                    bloke->param_action = 5;
                    free(bloke->path);
                    bloke->path = NULL;
                }
                if (bloke->path != NULL && BNVPath_GetDFrame(bloke->path) >= 0x3f) {
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
                bloke->field_35 = 1;
                bloke->person->depth = GetUnitDepth(-1617706.75f, -1617948.625f);
                bloke->param_action++;
                if ((char)++node->b2 == PlaneRide->seats) {
                    FUN_0043d990(node);
                }
                break;
            case 7:
                tmp.x = bloke->screen_x * 2;
                tmp.y = bloke->screen_y * 2;
                BlokeWalkAnim(bloke);
                BlokeSetFrame(bloke, 0);
                UnAdjustBlokePosition(&tmp);
                bloke->flags |= 0x80;
                coords2[0] = tmp.x;
                coords2[1] = tmp.y;
                bloke->person->sprite = DAT_0062fe98;
                bloke->person->field_30 = 1;
                bloke->person->depth = GetUnitDepth(-1617706.75f, -1617948.625f);
                bloke->field_35 = 2;
                sprintf(DAT_004b79bc + 6, "%02d", bloke->field_36);
                bloke->path = NewBNVPath(DAT_0062fe84[2], 2, DAT_004b79bc, -1617706.75f, -1617948.625f, coords2);
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
                if (bloke->path != NULL && BNVPath_GetDFrame(bloke->path) >= 0x20) {
                    bloke->field_35 = 2;
                    bloke->param_action = 0xd;
                    free(bloke->path);
                    bloke->path = NULL;
                }
                BlokeSetFrame(bloke, bloke->frame);
                break;
            case 0xd: {
                iv12 = ride->field_24 + pos->pos.x;
                iv13 = pos->pos.y + ride->field_25;

                node->slots[bloke->field_36 - 1] = 0;
                bloke->flags &= 0xff7f;
                bloke->person->sprite = NULL;
                bloke->person->field_30 = 0;
                UnAdjustBlokePosition(&bloke->person->screen);
                ScreenToMapRef((int *)&bloke->person->screen, (int *)&bloke->pos, 0);
                bloke->person->field_34 = 0;
                bloke->pos.x <<= 8;
                bloke->pos.y <<= 8;
                bloke->dest.x = iv12 * 256 + 128;
                bloke->dest.y = iv13 * 256 + 128;
                bloke->field_73 = CalcMoveLine(bloke->pos, bloke->dest, &bloke->nav) + 0x10;
                bloke->low_level_action = 7;
                NewDirForAction(bloke, (bloke->field_73 >> 5) + 3);
                bloke->param_action++;
                break;
            }
            case 0xe:
                bloke->flags &= 0xfff7;
                RemoveBlokeFromRide(ride, rn);
                if (--node->b3 == 0) {
                    node->b2 = 0;
                    Ride_ClearFlagToNotLetAnyoneOn(node);
                }
                break;
            }
        }
        rn = next;
    }
    FUN_0043e3f0();
}
