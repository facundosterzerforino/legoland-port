#include <stdlib.h>
#include <string.h>
#include "globals.h"
#include "legoland.h"

#include "bloke.h"
#include "bricks.h"
#include "driving_school.h"
#include "gamemap.h"
#include "llidb.h"
#include "man3d.h"
#include "map_object.h"
#include "math.h"
#include "obj_instance.h"
#include "objclass.h"
#include "print_sprite.h"
#include "render3d.h"
#include "ride_bloke.h"
#include "ride_queue.h"
#include "roads.h"
#include "sound_music.h"
#include "sound_sfx.h"
#include "tilemap.h"

struct DSCursorSource {
    /* 0x00 */ unsigned char pad_0[0x1c];
    /* 0x1c */ unsigned int flags;
    /* 0x20 */ unsigned char pad_20[0x3c - 0x20];
    /* 0x3c */ int footprint_x0;
    /* 0x40 */ int footprint_y0;
};

struct DSHead {
    /* 0x00 */ unsigned char pad_0[0xc];
    /* 0x0c */ struct DSCursorSource *ride;
};

struct Node8 {
    unsigned char pad_0[8];
    void *next;
};

struct DSCarLayer {
    unsigned char pad_0[12];
    struct DSCarSub *field_c;
};

struct DSCarSub {
    unsigned char pad_0[20];
    unsigned int field_14;
    unsigned int field_18;
    unsigned char pad_1c[72];
    struct DSCarInner *layer;
};

struct DSCarInner {
    unsigned char pad_0[16];
    unsigned int field_10;
};

struct CountNode {
    TileId tile;
    unsigned char pad_2[2];
    int field_4;
    struct CountNode *next;
};

struct DSRoadElem {
    /* 0x00 */ unsigned char pad_0[0x8];
    /* 0x08 */ unsigned char flags;
    /* 0x09 */ unsigned char pad_9[0xc - 0x9];
    /* 0x0c */ void *obj;
};

struct DSSampleConfig {
    /* 0x00 */ int field_0;
    /* 0x04 */ int field_4;
};

struct DSObjClass {
    /* 0x00 */ unsigned char pad_0[0x26];
    /* 0x26 */ short cost;
    /* 0x28 */ unsigned char pad_28[0xc4 - 0x28];
    /* 0xc4 */ unsigned int element;
};

struct DSBlokeNode {
    /* 0x00 */ struct DSBlokeNode *next;
    /* 0x04 */ short field_4;
};

struct DSRenderNode {
    /* 0x00 */ struct DSRenderNode *next;
    /* 0x04 */ unsigned char pad_4[0x8 - 0x4];
    /* 0x08 */ struct Bloke *field_8;
    /* 0x0c */ short field_c;
};

struct DSRenderSub {
    /* 0x00 */ unsigned char pad_0[0xc];
    /* 0x0c */ int field_c;
    /* 0x10 */ int field_10;
    /* 0x14 */ int field_14;
    /* 0x18 */ int field_18;
    /* 0x1c */ unsigned char pad_1c[0xcc - 0x1c];
    /* 0xcc */ struct DSRenderNode *field_cc;
};

struct DSRenderRoot {
    /* 0x00 */ unsigned char pad_0[0xc];
    /* 0x0c */ struct DSRenderSub *field_c;
};

#include "image_sprite.h"
#include "pumps.h"

// FUNCTION: LEGOLAND 0x004051a0
void FUN_004051a0(short param_1) {
    struct RideQueueEntry *node;
    struct RideQueueEntry *e0;
    struct RideQueueEntry *e1;
    struct RideQueueEntry *e2;
    struct RideQueueEntry *e3;

    for (node = DAT_004c11c4; node != NULL; node = node->field_4) {
        e0 = FUN_004125a0(node->x, node->y - 4);
        e1 = FUN_004125a0(node->x + 4, node->y);
        e2 = FUN_004125a0(node->x, node->y + 4);
        e3 = FUN_004125a0(node->x - 4, node->y);
        if ((node->field_14 & 0xf) == 6) {
            e2 = NULL;
            e3 = NULL;
            e0 = NULL;
        } else if (e0 != NULL && ((short)e0->id != param_1 || e0->field_18 != NULL)) {
            e0 = NULL;
        }
        if (e1 != NULL && ((short)e1->id != param_1 || e1->field_18 != NULL)) {
            e1 = NULL;
        }
        if (e2 != NULL && ((short)e2->id != param_1 || e2->field_18 != NULL)) {
            e2 = NULL;
        }
        if (e3 != NULL && ((short)e3->id != param_1 || e3->field_18 != NULL)) {
            e3 = NULL;
        }
        if (e0 != NULL) {
            e0->field_18 = node;
            e0->field_15 = node->field_15 + 1;
        }
        if (e1 != NULL) {
            e1->field_18 = node;
            e1->field_15 = node->field_15 + 1;
        }
        if (e2 != NULL) {
            e2->field_18 = node;
            e2->field_15 = node->field_15 + 1;
        }
        if (e3 != NULL) {
            e3->field_18 = node;
            e3->field_15 = node->field_15 + 1;
        }
        if (e0 != NULL) {
            e0->field_4 = DAT_004c11c8;
            DAT_004c11c8 = e0;
        }
        if (e1 != NULL) {
            e1->field_4 = DAT_004c11c8;
            DAT_004c11c8 = e1;
        }
        if (e2 != NULL) {
            e2->field_4 = DAT_004c11c8;
            DAT_004c11c8 = e2;
        }
        if (e3 != NULL) {
            e3->field_4 = DAT_004c11c8;
            DAT_004c11c8 = e3;
        }
    }
}

// FUNCTION: LEGOLAND 0x00405310
void FUN_00405310(TileId tile) {
    struct RideQueueEntry *cur;
    struct RideQueueEntry *ret;
    struct RideQueueEntry *tmp;

    cur = DAT_004cbeac;
    while (cur != NULL) {
        if (cur->id == tile.id) {
            cur->field_18 = NULL;
        }
        cur = cur->next;
    }

    ret = FUN_00412650(tile.id);
    ret->field_15 = 0;
    ret->field_4 = NULL;

    DAT_004c11c4 = ret;
    DAT_004c11c8 = NULL;

    do {
        FUN_004051a0(tile.id);
        tmp = DAT_004c11c8;
        DAT_004c11c4 = tmp;
        DAT_004c11c8 = NULL;
    } while (tmp != NULL);
}

// FUNCTION: LEGOLAND 0x00405370
void LoadDrivingSchoolResources(struct DSHead *param_1) {
    unsigned int lls;

    DrivingSchoolRide = param_1->ride;
    if (LLIDB_FindElement("DSCHOOL MAPPING", (unsigned int *)&param_1, 0) == 0) {
        DSchoolMappingData = LLIDB_LoadData(param_1);
    }
    if (LLIDB_FindElement("DSCHOOL BLUE CAR", (unsigned int *)&param_1, 0) == 0) {
        DSchoolBlueCarData = LLIDB_LoadData(param_1);
    }
    DrivingSchoolRide->flags |= 0x420;
    Load_FXList(DRIVING_SCHOOL_SFX, 6);
    // STRING: LEGOLAND 0x004b4524
    DSchoolMatteSprite = LoadSprite("DSchool Matte.lls", 1);
    // STRING: LEGOLAND 0x004b4510
    DSchoolBluePalette = LoadPalette((unsigned int)".\\3ddata\\blu.col");
    // STRING: LEGOLAND 0x004b44fc
    DSchoolYellowPalette = LoadPalette((unsigned int)".\\3ddata\\yel.col");
    // STRING: LEGOLAND 0x004b44e8
    DSchoolRedPalette = LoadPalette((unsigned int)".\\3ddata\\red.col");
    // STRING: LEGOLAND 0x004b44d8
    DSCarSprite = LoadSprite("ds_car&m.lls", 1);
    if (DSCarSprite != NULL) {
        lls = GetLLSForSprite((struct SpriteLLS *)DSCarSprite);
        if (lls != 0) {
            LLSStop(lls);
        }
    }
}

// FUNCTION: LEGOLAND 0x00405460
void UnloadDrivingSchoolResources(void) {
    unsigned int local;

    // STRING: LEGOLAND 0x004b454c
    if (LLIDB_FindElement("DSCHOOL MAPPING", &local, 0) == 0) {
        LLIDB_UnLoadData(local);
    }

    // STRING: LEGOLAND 0x004b4538
    if (LLIDB_FindElement("DSCHOOL BLUE CAR", &local, 0) == 0) {
        LLIDB_UnLoadData(local);
    }

    while (DAT_004c10d4 != NULL) {
        FUN_00401c60(DAT_004c10d4);
    }

    FUN_00411bd0();

    while (DAT_004cbeac != NULL) {
        struct RideQueueEntry *next = DAT_004cbeac->next;
        free(DAT_004cbeac);
        DAT_004cbeac = next;
    }

    while (DrivingSchoolCountList != NULL) {
        void *next = ((struct Node8 *)DrivingSchoolCountList)->next;
        free(DrivingSchoolCountList);
        DrivingSchoolCountList = next;
    }

    Kill_FXList(DRIVING_SCHOOL_SFX, 6);
    KillSprite(DSchoolMatteSprite);
    free(DSchoolBluePalette);
    free(DSchoolYellowPalette);
    free(DSchoolRedPalette);
    KillSprite(DSCarSprite);
    DrivingSchoolRide = NULL;
}

// FUNCTION: LEGOLAND 0x00405570
void DrivingSchoolSetEditMode(void) {
    EditMode.unk0 = 1;
    EditMode.unk8 = DrivingSchoolRide;
    DefaultCursor(&EditCursor);
    SetEditCursorFootPrint((char *)EditMode.unk8 + 0x3c);
    DefaultCursor(&DAT_0082f760);
    memcpy(DAT_0082f760.field_1414, DAT_004b4440, 20);
    DAT_0082f760.field_1828 |= 0x100;
    DefaultCursor(&DAT_0082c6e0);
    memcpy(DAT_0082c6e0.field_1414, DAT_004b4458, 20);
    DAT_0082c6e0.field_1828 |= 0x200;
    DefaultCursor(&DAT_0082df20);
    memcpy(DAT_0082df20.field_1414, DAT_004b4470, 20);
    EditCursor.field_1830 = (unsigned int)&DAT_0082f760;
    DAT_0082f760.field_1830 = (unsigned int)&DAT_0082c6e0;
}

// FUNCTION: LEGOLAND 0x00405630
void DrivingSchoolAddObject(unsigned int param_1, int *coords) {
    TileId tile;
    struct CountNode *node;
    struct Cursor *cursor;
    int x;
    int y;
    struct DSRoadElem *elem;

    tile.pos.x = coords[0];
    tile.pos.y = coords[1];
    AddBasicObject(param_1, coords);

    node = (struct CountNode *)malloc(sizeof(struct CountNode));
    node->tile = tile;
    node->next = (struct CountNode *)DrivingSchoolCountList;
    DrivingSchoolCountList = node;
    cursor = (struct Cursor *)EditCursor.field_1830;
    x = cursor->tile_x;
    y = cursor->tile_y;

    FUN_004132a0(tile, x - 3, y - 4, 6, 1);
    FUN_004132a0(tile, x + 1, y - 4, 0, 1);
    FUN_004132a0(tile, x + 5, y - 4, 3, 1);
    FUN_004132a0(tile, x + 5, y, 0, 0);
    FUN_004132a0(tile, x + 5, y + 4, 0, 0);

    // STRING: LEGOLAND 0x004b455c
    elem = (struct DSRoadElem *)ElemID("Driving School Roads");
    if (elem != NULL && (elem->flags & 4) != 0) {
        IncrementObjectCount(elem->obj);
        IncrementObjectCount(elem->obj);
        IncrementObjectCount(elem->obj);
        IncrementObjectCount(elem->obj);
        IncrementObjectCount(elem->obj);
    }

    node->field_4 = 5;
    FUN_00405310(tile);
}

// FUNCTION: LEGOLAND 0x00405740
void DrivingSchoolCalcCursor(struct DSHead *param_1, unsigned int param_2, unsigned int param_3) {
    struct DSCursorSource *src = param_1->ride;
    struct DSCursorSource *c694;
    unsigned int mapx;
    unsigned int mapy;

    memcpy(EditCursor.field_1414, (char *)src + 0x3c, 20);
    EditCursor.field_1830 = 0;
    EditCursor.field_1828 = 0x4408;
    ScreenToMapRef(param_2, &EditCursor.tile_x, param_3);

    c694 = DrivingSchoolRide;
    mapx = EditCursor.tile_x;
    memcpy(DAT_0082f760.field_1414, DAT_004b4440, 20);
    DAT_0082f760.field_1830 = 0;
    mapy = EditCursor.tile_y;
    DAT_0082f760.field_1828 = 0x4108;
    DAT_0082f760.tile_x = c694->footprint_x0 + mapx;
    DAT_0082f760.tile_y = c694->footprint_y0 + mapy;

    memcpy(DAT_0082c6e0.field_1414, DAT_004b4458, 20);
    DAT_0082c6e0.field_1830 = 0;
    DAT_0082c6e0.field_1828 = 0x4208;
    DAT_0082c6e0.tile_x = c694->footprint_x0 + mapx;
    DAT_0082c6e0.tile_y = c694->footprint_y0 + mapy;

    memcpy(DAT_0082df20.field_1414, DAT_004b4470, 20);
    DAT_0082df20.field_1830 = 0;
    DAT_0082df20.field_1828 = 0x5008;
    DAT_0082df20.tile_x = c694->footprint_x0 + mapx;
    DAT_0082df20.tile_y = c694->footprint_y0 + mapy;

    ValidateCursor(&DAT_0082df20, (unsigned int)src);
    ValidateCursor(&DAT_0082c6e0, (unsigned int)src);
    ValidateCursor(&DAT_0082f760, (unsigned int)src);
    ValidateCursor(&EditCursor, (unsigned int)src);

    EditCursor.field_1830 = (unsigned int)&DAT_0082f760;
    DAT_0082f760.field_1830 = (unsigned int)&DAT_0082c6e0;
    DAT_0082c6e0.field_1830 = (unsigned int)&DAT_0082df20;
    FUN_0045f4d0(&EditCursor);
}

// FUNCTION: LEGOLAND 0x004058a0
void DrivingSchoolDCalcCursor(unsigned int param_1, unsigned int param_2) {
    struct RideQueueEntry *node = DAT_004cbeac;

    BasicObjectDCalcCursor(param_1, param_2);
    DefaultCursor(&DAT_0082f760);
    memcpy(DAT_0082f760.field_1414, DAT_004b4bf0, 20);

    while (node != NULL) {
        if (node->id == QueryObj.id) {
            DAT_0082f760.tile_x = node->x;
            DAT_0082f760.tile_y = node->y;
            FUN_0045f460(&DAT_0082f760);
            DAT_0082f760.field_1828 = 0x18;
            BuildCursorPtr(&DAT_0082f760, 0, 0);
            RenderCursor(&DAT_0082f760);
        }
        node = node->next;
    }
}

// FUNCTION: LEGOLAND 0x00405940
void DrivingSchoolRemoveObject(Element *obj, TileId tile, unsigned int param_3) {
    struct RideQueueEntry *queue = DAT_004cbeac;
    struct CountNode *count = (struct CountNode *)DrivingSchoolCountList;
    struct DSBlokeNode *blokes = (struct DSBlokeNode *)DAT_004c10d4;
    struct RideQueueEntry *next;
    struct DSBlokeNode *nextBloke;

    StandardRemoveObject((Element *)obj, tile, (struct Cursor *)param_3);
    DefaultCursor(&DAT_0082f760);
    memcpy(DAT_0082f760.field_1414, DAT_004b4bf0, 20);

    if (count->tile.id == tile.id) {
        DrivingSchoolCountList = count->next;
        free(count);
    } else {
        while (count->next != NULL) {
            if (count->next->tile.id == tile.id) {
                count->next = count->next->next;
                free(count->next);
                break;
            }
            count = count->next;
        }
    }

    FUN_00411ba0(QueryObj.id);

    while (queue != NULL) {
        next = queue->next;
        if (queue->id == QueryObj.id) {
            if (queue->field_14 & 0x10) {
                queue->field_14 &= 0xef;
                UpdateQueuePathShape(queue->id, queue->x, queue->y);
                AddBricks(((struct DSObjClass *)ZebraCrossingRide)->cost);
            }
            DAT_0082f760.tile_x = queue->x;
            DAT_0082f760.tile_y = queue->y;
            StandardRemoveObject((Element *)((struct DSObjClass *)DAT_0082c684)->element, *(TileId *)&queue->id, &DAT_0082f760);
            FUN_004133e0(queue->x, queue->y);
        }
        queue = next;
    }

    AddBricks(((struct DSObjClass *)DAT_0082c684)->cost * 5);

    while (blokes != NULL) {
        nextBloke = blokes->next;
        if (blokes->field_4 == (short)QueryObj.id) {
            FUN_00401c60(blokes);
        }
        blokes = nextBloke;
    }

    RemoveAllBlokesFromRide(obj->ride, tile);
}

// FUNCTION: LEGOLAND 0x00405ad0
struct RideSpriteInfo *GetDrivingSchoolSpriteInfo(struct DSCarLayer *arg1, unsigned short arg2) {
    struct DSCarSub *sub = arg1->field_c;
    struct DSCarInner *inner = sub->layer;

    RideSpriteInfoBuffer.sprite = inner;
    RideSpriteInfoBuffer.x = sub->field_14;
    RideSpriteInfoBuffer.y = sub->field_18;
    RideSpriteInfoBuffer.id = arg2;

    inner = sub->layer;
    inner->field_10 |= 0x2000;

    return &RideSpriteInfoBuffer;
}

// FUNCTION: LEGOLAND 0x00405b10
void RenderDrivingSchool(Element *obj, unsigned int param_2, unsigned int param_3, unsigned short *tile, unsigned int param_5, unsigned int clip) {
    struct Ride *ride = obj->ride;
    struct RideNode *node;
    struct Point ref;
    int bounds[4];
    int dx;
    int dy;

    node = ride->riders;
    ref.x = ((unsigned char *)tile)[0];
    ref.y = ((unsigned char *)tile)[1];
    for (; node != NULL; node = node->next) {
        if (*tile == node->tile.id) {
            unsigned char state = node->rider->param_action;
            if (state <= 1 || state >= 3) {
                IP_RenderBlokeIn3DNow(node->rider);
            }
        }
    }
    GetTileBounds(&ref, bounds);
    if (DSchoolMatteSprite != NULL) {
        dx = ride->field_14;
        dy = ride->field_18;
        if (dx < 0) {
            dx = -(-dx >> 1);
        } else {
            dx = dx >> 1;
        }
        if (dy < 0) {
            dy = -(-dy >> 1);
        } else {
            dy = dy >> 1;
        }
        PrintSprite(DSchoolMatteSprite, bounds[0] + dx, bounds[1] + dy, clip, 0);
    }
}

// FUNCTION: LEGOLAND 0x00405bd0
void DrivingSchoolUpdate(Element *obj) {
    struct Ride *ride = obj->ride;
    struct RideNode *node = ride->riders;
    struct RideNode *next;
    struct Bloke *bloke;
    struct SampleSource source;
    struct SampleSource source2;
    char move;
    int res;
    int count;
    int r;
    struct Sample *sample;

    FUN_00402c10();
    FUN_00414440();
    while (node != NULL) {
        bloke = node->rider;
        next = node->next;
        if (bloke->low_level_action == 0) {
            switch (bloke->param_action) {
            case 0:
                bloke->dest.x = (node->tile.pos.x + ride->x) << 8;
                bloke->dest.y = (node->tile.pos.y - 3 + ride->y) << 8;
                move = CalcMoveLine(bloke->pos, bloke->dest, &bloke->nav);
                bloke->low_level_action = 7;
                bloke->field_73 = move + 0x10;
                NewDirForAction(bloke, ((unsigned char)(move + 0x10) >> 5) + 3);
                bloke->field_58 = (rand() & 7) + 1;
                bloke->param_action++;
                break;
            case 1:
                if (bloke->field_58 == 0) {
                    count = FUN_00401c40(node->tile.id);
                    if (count * 5 < (int)FUN_00413970(node->tile.id)) {
                        res = FUN_00401ae0(node->tile.id, (int)bloke);
                        if (res == 0) {
                            source.type = 1;
                            source.bloke = bloke;
                            PlayInstanceOfSample(*(void **)(DRIVING_SCHOOL_SFX + 8), 0, 1, &source);
                            r = rand() % 4 + 1;
                            sample = PlayInstanceOfSample(((void **)(DRIVING_SCHOOL_SFX + 8))[r * 3], 1, 1, &source);
                            AdjustPSampleFreq(sample, 10);
                            BlokeSitAnim(bloke);
                            BlokeSetFrame(bloke, 0);
                            bloke->flags |= 0x80;
                            bloke->param_action++;
                        } else if (res == -1) {
                            bloke->param_action = 3;
                        } else if (res == -2) {
                            bloke->field_58 = (rand() & 0x1f) + 2;
                        }
                    }
                } else {
                    bloke->field_58--;
                }
                break;
            case 3:
                bloke->flags &= 0xff7f;
                BlokeWalkAnim(bloke);
                BlokeSetFrame(bloke, 0);
                bloke->pos.x = (node->tile.pos.x + ride->x) << 8;
                bloke->pos.y = (node->tile.pos.y - 3 + ride->y) << 8;
                bloke->dest.x = ((node->tile.pos.x + ride->x) << 8) + 0x80;
                bloke->dest.y = ((node->tile.pos.y + ride->y) << 8) + 0x80;
                move = CalcMoveLine(bloke->pos, bloke->dest, &bloke->nav);
                bloke->low_level_action = 7;
                bloke->field_73 = move + 0x10;
                NewDirForAction(bloke, ((unsigned char)(move + 0x10) >> 5) + 3);
                bloke->param_action++;
                break;
            case 4:
                RemoveBlokeFromRide(ride, node);
                bloke->flags &= 0xfff7;
                source2.type = 1;
                source2.bloke = bloke;
                KillAllSamplesFromSource(&source2);
                break;
            }
        }
        node = next;
    }
}

// FUNCTION: LEGOLAND 0x00405e70
int DrivingSchool_Save(void) {
    struct CountNode *countNode;
    struct RideQueueEntry *queue;
    struct PumpNode *pump;
    struct DSBlokeNode *bloke;
    struct CountNode *countCur;
    struct RideQueueEntry *queueCur;
    struct PumpNode *pumpCur;
    struct DSBlokeNode *blokeCur;
    int count;
    int buf[52];

    count = 0;
    for (countCur = (struct CountNode *)DrivingSchoolCountList; countCur != NULL; countCur = countCur->next) {
        count++;
    }
    countNode = (struct CountNode *)DrivingSchoolCountList;
    SaveGameWrite(&count, 4);
    while (count--) {
        SaveGameWrite(countNode, 0xc);
        countNode = countNode->next;
    }

    count = 0;
    for (queueCur = DAT_004cbeac; queueCur != NULL; queueCur = queueCur->next) {
        count++;
    }
    queue = DAT_004cbeac;
    SaveGameWrite(&count, 4);
    while (count--) {
        SaveGameWrite(queue, 0x20);
        queue = queue->next;
    }

    count = 0;
    for (pumpCur = (struct PumpNode *)PumpList; pumpCur != NULL; pumpCur = pumpCur->next) {
        count++;
    }
    pump = (struct PumpNode *)PumpList;
    SaveGameWrite(&count, 4);
    while (count--) {
        SaveGameWrite(pump, 0x10);
        pump = pump->next;
    }

    count = 0;
    for (blokeCur = (struct DSBlokeNode *)DAT_004c10d4; blokeCur != NULL; blokeCur = blokeCur->next) {
        count++;
    }
    bloke = (struct DSBlokeNode *)DAT_004c10d4;
    SaveGameWrite(&count, 4);
    while (count--) {
        memcpy(buf, bloke, 0xd0);
        buf[51] = GetBlokeNum(buf[51]);
        SaveGameWrite(buf, 0xd0);
        bloke = bloke->next;
    }

    return 1;
}

// FUNCTION: LEGOLAND 0x00406020
void FUN_00406020(unsigned short arg1, unsigned int arg2) {
    struct CountNode *current = (struct CountNode *)DrivingSchoolCountList;
    while (current != NULL) {
        if (current->tile.id == arg1) {
            if (current != NULL) {
                current->field_4 += arg2;
            }
            break;
        }
        current = current->next;
    }
}

// FUNCTION: LEGOLAND 0x00406050
int FUN_00406050(void) {
    struct CountNode *current = (struct CountNode *)DrivingSchoolCountList;
    int max = 0;

    while (current != NULL) {
        if (current->field_4 > max) {
            max = current->field_4;
        }
        current = current->next;
    }

    return max;
}

// FUNCTION: LEGOLAND 0x00406070
int DrivingSchool_Load(void) {
    struct CountNode *countPrev;
    struct CountNode *countNode;
    struct RideQueueEntry *queuePrev;
    struct RideQueueEntry *queueNode;
    struct PumpNode *pumpPrev;
    struct PumpNode *pumpNode;
    struct DSBlokeNode *blokePrev;
    struct DSBlokeNode *blokeNode;
    int count;

    countPrev = NULL;
    queuePrev = NULL;
    pumpPrev = NULL;
    blokePrev = NULL;

    DrivingSchoolCountList = NULL;
    SaveGameRead(&count, 4);
    while (count--) {
        if (countPrev == NULL) {
            DrivingSchoolCountList = (struct CountNode *)malloc(0xc);
            countPrev = DrivingSchoolCountList;
        } else {
            countNode = (struct CountNode *)malloc(0xc);
            countPrev->next = countNode;
            countPrev = countNode;
        }
        SaveGameRead(countPrev, 0xc);
    }

    DAT_004cbeac = NULL;
    SaveGameRead(&count, 4);
    while (count--) {
        if (queuePrev == NULL) {
            DAT_004cbeac = (struct RideQueueEntry *)malloc(0x20);
            queuePrev = DAT_004cbeac;
        } else {
            queueNode = (struct RideQueueEntry *)malloc(0x20);
            queuePrev->next = queueNode;
            queuePrev = queueNode;
        }
        SaveGameRead(queuePrev, 0x20);
    }

    PumpList = NULL;
    SaveGameRead(&count, 4);
    while (count--) {
        if (pumpPrev == NULL) {
            PumpList = (struct PumpNode *)malloc(0x10);
            pumpPrev = PumpList;
        } else {
            pumpNode = (struct PumpNode *)malloc(0x10);
            pumpPrev->next = pumpNode;
            pumpPrev = pumpNode;
        }
        SaveGameRead(pumpPrev, 0x10);
    }

    DAT_004c10d4 = NULL;
    SaveGameRead(&count, 4);
    while (count--) {
        if (blokePrev == NULL) {
            DAT_004c10d4 = (struct DSBlokeNode *)malloc(0xd0);
            blokePrev = DAT_004c10d4;
        } else {
            blokeNode = (struct DSBlokeNode *)malloc(0xd0);
            blokePrev->next = blokeNode;
            blokePrev = blokeNode;
        }
        SaveGameRead(blokePrev, 0xd0);
        *(int *)((char *)blokePrev + 0xcc) = GetBlokePtr(*(int *)((char *)blokePrev + 0xcc));
    }

    for (countNode = (struct CountNode *)DrivingSchoolCountList; countNode != NULL; countNode = countNode->next) {
        FUN_00405310(countNode->tile);
    }

    return 1;
}
