#include <stdlib.h>
#include <string.h>
#include "globals.h"
#include "legoland.h"

#include "bloke.h"
#include "debug_alloc.h"
#include "draw.h"
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
#include "tilemap.h"
#include "water_works.h"

struct WaterRect {
    int x0;
    int y0;
    int x1;
    int y1;
};

struct AdjustOffset {
    int x;
    int y;
};

struct WaterListNode {
    struct WaterListNode *next;
    unsigned char pad_4[4];
    unsigned char field_8;
    unsigned char field_9;
};

#include "image_sprite.h"
#include "power.h"

// FUNCTION: LEGOLAND 0x00417a90
unsigned int AcquireWaterWorksSfx(void) {
    if (WaterWorksSfxRefCount++ == 0) {
        Load_FXList(WATERWORKS_SFX, 3);
    }
}

// FUNCTION: LEGOLAND 0x00417ac0
void ReleaseWaterWorksSfx(void) {
    WaterWorksSfxRefCount--;
    if (WaterWorksSfxRefCount == 0) {
        Kill_FXList(WATERWORKS_SFX, 3);
    }
}

// FUNCTION: LEGOLAND 0x00417ae0
void thunk_FUN_00417ac0(void) { ReleaseWaterWorksSfx(); }

// FUNCTION: LEGOLAND 0x00417af0
void FUN_00417af0(unsigned char *src, int index, unsigned int looping) {
    struct SampleSource params;

    params.type = 2;
    params.x = src[0];
    params.y = src[1];
    PlayInstanceOfSample(WATERWORKS_SFX[index].sample, looping, 1, &params);
}

// FUNCTION: LEGOLAND 0x00417b40
void FUN_00417b40(unsigned char *src) {
    struct SampleSource source;

    source.type = 2;
    source.x = src[0];
    source.y = src[1];
    UnSourceAndFadeAllSamplesFromSource(&source, -400);
}

// FUNCTION: LEGOLAND 0x00417b80
void InsertWaterNodeAtHead(struct WaterNode **head, struct WaterNode *node) {
    if (node != NULL) {
        node->next = *head;
        *head = node;
    }
}

// FUNCTION: LEGOLAND 0x00417ba0
struct WaterNode *FindWaterNodeByKey(struct WaterNode *list, short *key) {
    if (list == NULL) {
        return NULL;
    }
    while (memcmp(&list->key, key, sizeof(list->key)) != 0) {
        list = list->next;
        if (list == NULL) {
            return NULL;
        }
    }
    return list;
}

// FUNCTION: LEGOLAND 0x00417bd0
void FUN_00417bd0(struct WaterNode **head, struct WaterNode *node) {
    struct WaterNode *cur;

    cur = *head;
    if (cur == node) {
        *head = node->next;
        return;
    }
    while (cur->next != node) {
        cur = cur->next;
        if (cur == NULL) {
            break;
        }
    }
    if (cur != NULL) {
        cur->next = node->next;
    }
}

// FUNCTION: LEGOLAND 0x00417c00
unsigned int WaterWorksEntranceLoad(struct Element *arg) {
    DAT_004cbfe4 = arg;
    DAT_004cc01c = arg->ride;
    return AcquireWaterWorksSfx();
}

// FUNCTION: LEGOLAND 0x00417c20
void WaterWorksEntranceAddObject(struct Element *editObj, int *coords) {
    struct SampleSource params;

    AddBasicObject(editObj, coords);
    params.x = coords[0];
    params.type = 2;
    params.y = coords[1];
    PlayInstanceOfSample(WATERWORKS_SFX[1].sample, 1, 1, &params);
}

// FUNCTION: LEGOLAND 0x00417c70
void WaterWorksEntranceRemoveObject(struct Element *a, unsigned int b, struct Cursor *c) {
    RemoveSoundObject(a, b, c);
}

// FUNCTION: LEGOLAND 0x00417c90
unsigned int FUN_00417c90(void) {
    // DAT_004cbfe4 is an int-typed global that stores an object pointer (set to a
    // WaterArg*); pass it as the match vtable. Cast justified: the global is genuinely
    // used as both an int and various struct pointers, so it stays unsigned int.
    return GetFirstObjectMatching((Element *)DAT_004cbfe4) != 0;
}

// FUNCTION: LEGOLAND 0x00417cb0
void FUN_00417cb0(unsigned short key) {
    struct WaterNode *node;

    node = (struct WaterNode *)malloc(sizeof(struct WaterNode));
    if (node != NULL) {
        memset(node, 0, sizeof(struct WaterNode));
        node->key = key;
        InsertWaterNodeAtHead(&WaterNodeList, node);
    }
}

// FUNCTION: LEGOLAND 0x00417cf0
void FUN_00417cf0(struct WaterNode *node) {
    FUN_00417bd0(&WaterNodeList, node);
    free(node);
}

// FUNCTION: LEGOLAND 0x00417d10
struct WaterNode *FUN_00417d10(unsigned short *key) {
    return FindWaterNodeByKey(WaterNodeList, key);
}

// FUNCTION: LEGOLAND 0x00417d30
void WaterBlockLoad(struct Element *arg) {
    AcquireWaterWorksSfx();
    WaterBlockRide = arg->ride;
    WaterBlockRide->flags |= 0x420;
    WaterBlockRide->footprint.x0 = 0;
    WaterBlockRide->footprint.x1 = 0;
    WaterBlockRide->footprint.y0 = 0;
    WaterBlockRide->footprint.y1 = 0;
    WaterBlockRide->field_14 = 0;
    WaterBlockRide->field_18 = 0;
    // STRING: LEGOLAND 0x004b501c
    if (LLIDB_FindElement("WATER WORKS IMAGE LIST", &WaterWorksImageListHandle, 0) == 0) {
        WaterWorksImageListData = LLIDB_LoadData((void *)WaterWorksImageListHandle);
    }
    // STRING: LEGOLAND 0x004b500c
    TopwaterSprite = LoadSprite("topwater.lls", 1);
}

// FUNCTION: LEGOLAND 0x00417dd0
void WaterWorksWaterBlockCalcCursor(struct Element *arg, int *a, unsigned int b) {
    struct Ride *context;

    context = arg->ride;
    *(struct Footprint *)EditCursor.field_1414 = *(struct Footprint *)&context->footprint.x0;
    EditCursor.field_1830 = 0;
    ScreenToMapRef(a, &EditCursor.tile_x, b);
    ValidateCursor(&EditCursor, context);
    if (FUN_00417c90() != 0) {
        return;
    }
    SetCursorBlocked(&EditCursor, 0xc);
    PropagateCursorBlocked(&EditCursor);
}

// FUNCTION: LEGOLAND 0x00417e40
void WaterBlockUnload(void) {
    if (TopwaterSprite != 0) {
        KillSprite(TopwaterSprite);
    }
    LLIDB_UnLoadData(WaterWorksImageListHandle);
    ReleaseWaterWorksSfx();
}

// FUNCTION: LEGOLAND 0x00417e70
int ListHasPointInRect(struct Bloke *list, struct WaterRect *rect) {
    struct Bloke *current;

    current = list;
    while (current != NULL) {
        if ((current->pos.x >> 8) >= rect->x0 && (current->pos.x >> 8) <= rect->x1) {
            if ((current->pos.y >> 8) >= rect->y0 && (current->pos.y >> 8) <= rect->y1) {
                return 1;
            }
        }
        current = current->next;
    }
    return 0;
}

// FUNCTION: LEGOLAND 0x00417ec0
unsigned int IsPointInBlokeGardenerOrMechanicList(unsigned char *point) {
    struct WaterRect rect;

    rect.x0 = point[0];
    rect.x1 = point[0];
    rect.y0 = point[1];
    rect.y1 = point[1];
    if (ListHasPointInRect(FirstBloke, &rect) != 0) {
        return 1;
    }
    if (ListHasPointInRect(GardenerList, &rect) != 0) {
        return 1;
    }
    return ListHasPointInRect(MechanicList, &rect) != 0;
}

// FUNCTION: LEGOLAND 0x00417f40
void FUN_00417f40(void) {
    struct WaterNode *node;

    node = (struct WaterNode *)WaterNodeList;
    while (node != NULL) {
        if (IsPointInBlokeGardenerOrMechanicList((unsigned char *)&node->key) != 0) {
            if (node->field_8 == 0) {
                node->field_8 = 1;
                node->frame = 0;
                FUN_00417af0((unsigned char *)&node->key, 0, 0);
            }
        }
        node = node->next;
    }
}

// FUNCTION: LEGOLAND 0x00417f90
void WaterWorksWaterBlockUpdate(Element *obj) {
    struct WaterNode *node;
    Ride *ride;
    RideNode *rnode;
    RideNode *next;
    Bloke *bloke;
    TileId *tile;
    union {
        struct Sprite *spr;
        struct LLS *lls;
    } u;
    int limit;
    int rx;
    int ry;
    int idx;

    char dir;

    FUN_00417f40();
    node = (struct WaterNode *)WaterNodeList;
    while (node != NULL) {
        if (node->field_8 == 1) {
            limit = 0;
            node->frame++;
            idx = node->block_index;
            u.spr = WaterWorksImageListData->sprites[(unsigned char)idx];
            if (u.spr != NULL) {
                u.lls = GetLLSForSprite(u.spr);
                if (u.lls != NULL) {
                    limit = u.lls->frame_count;
                } else {
                    // STRING: LEGOLAND 0x004b504c
                    DBPrintf("Bad Sprite in Waterworks block\n");
                }
            } else {
                // STRING: LEGOLAND 0x004b5034
                DBPrintf("Bad Block Index (%d)\n", idx);
            }
            if ((int)node->frame >= limit) {
                node->field_8 = 0;
                node->frame = 0;
            }
        } else {
            u.spr = WaterWorksImageListData->sprites[node->block_index];
            u.lls = GetLLSForSprite(u.spr);
            if (u.lls != NULL) {
                node->frame = 0;
            } else {
                DBPrintf("Bad Sprite in Waterworks block\n");
            }
        }
        node = node->next;
    }
    ride = obj->ride;
    rnode = ride->riders;
    while (rnode != NULL) {
        bloke = rnode->rider;
        tile = &rnode->tile;
        next = rnode->next;
        rx = tile->pos.x + ride->x;
        ry = tile->pos.y + ride->y;
        if (bloke->low_level_action == 0) {
            switch (bloke->param_action) {
            case 0:
                bloke->flags |= 8;
                ry = ry * 0x100 + 0x80;
                bloke->dest.x = rx * 0x100 + -0x80;
                bloke->dest.y = ry;
                dir = CalcMoveLine(bloke->pos, bloke->dest, &bloke->nav);
                bloke->low_level_action = 7;
                bloke->field_73 = dir + 0x10;
                NewDirForAction(bloke, (bloke->field_73 >> 5) + 3);
                bloke->param_action++;
                break;
            case 1:
                RemoveBlokeFromRide(ride, rnode);
                bloke->flags &= 0xfff7;
                break;
            }
        }
        rnode = next;
    }
}

// FUNCTION: LEGOLAND 0x00418110
struct RideSpriteInfo *GetWaterWorksWaterBlockSpriteInfo(struct Element *arg1, unsigned short arg2) {
    struct WaterNode *node;
    int idx;

    node = FUN_00417d10(&arg2);
    if (node != NULL) {
        idx = node->block_index;
        DAT_004cbff0.id = arg2;
        DAT_004cbff0.sprite = WaterWorksImageListData->sprites[(unsigned char)idx];
        DAT_004cbff0.x = WaterWorksImageListData->offset_x[(unsigned char)idx] >> 1;
        DAT_004cbff0.y = WaterWorksImageListData->offset_y[(unsigned char)idx] >> 1;
        DAT_004cbff0.field_10 = 0;
        ((struct Sprite *)DAT_004cbff0.sprite)->flags |= 0x2000;
        SetOverrideFrame(node->frame);
    }
    return &DAT_004cbff0;
}

// FUNCTION: LEGOLAND 0x004181a0
void RenderWaterBlock(struct Element *arg, int a, int b, unsigned short *key) {
    struct RideNode *node;

    node = ((struct Ride *)arg->ride)->riders;
    while (node != NULL) {
        if (*key == node->tile.id) {
            IP_RenderBlokeIn3DNow(node->rider);
        }
        node = node->next;
    }
}

// FUNCTION: LEGOLAND 0x004181e0
void WaterBlockAddObject(struct Element *editObj, unsigned char *coords) {
    unsigned short key;
    struct WaterNode *node;

    *(unsigned char *)&key = coords[0];
    *((unsigned char *)&key + 1) = coords[4];
    AddBasicObject(editObj, (int *)coords);
    FUN_00417cb0(key);
    node = FUN_00417d10(&key);
    if (node != NULL) {
        node->block_index = (char)(rand() % WaterWorksImageListData->count);
    }
}

// FUNCTION: LEGOLAND 0x00418230
void WaterBlockRemoveObject(struct Element *a, unsigned int b, struct Cursor *c) {
    struct WaterNode *node;

    StandardRemoveObject(a, *(TileId *)&b, c);
    node = FUN_00417d10((unsigned short *)&b);
    if (node != NULL) {
        FUN_00417cf0(node);
    }
}

// FUNCTION: LEGOLAND 0x00418260
void FUN_00418260(unsigned short key) {
    struct WaterNode *node;

    node = (struct WaterNode *)malloc(sizeof(struct WaterNode));
    if (node != NULL) {
        memset(node, 0, sizeof(struct WaterNode));
        node->key = key;
        InsertWaterNodeAtHead(&DAT_004cc030, node);
    }
}

// FUNCTION: LEGOLAND 0x004182a0
void FUN_004182a0(struct WaterNode *node) {
    FUN_00417bd0(&DAT_004cc030, node);
    free(node);
}

// FUNCTION: LEGOLAND 0x004182c0
struct WaterNode *FUN_004182c0(unsigned short *key) {
    return FindWaterNodeByKey(DAT_004cc030, key);
}

// FUNCTION: LEGOLAND 0x004182e0
void ShowerLoad(struct Element *arg) {
    AcquireWaterWorksSfx();
    ShowerRide = arg->ride;
    ShowerRide->flags |= 0x420;
    ShowerLayer = ShowerRide->layer;
    ShowerLayer->flags |= 0x2000;
    // STRING: LEGOLAND 0x004b506c
    ShowerSprite = LoadSprite("shower.lls", 1);
}

// FUNCTION: LEGOLAND 0x00418330
void ShowerUnload(void) {
    if (ShowerSprite != 0) {
        KillSprite(ShowerSprite);
    }
    ReleaseWaterWorksSfx();
}

// FUNCTION: LEGOLAND 0x00418350
void FUN_00418350(void) {
    struct WaterNode *node;

    node = (struct WaterNode *)DAT_004cc030;
    while (node != NULL) {
        if (IsPointInBlokeGardenerOrMechanicList((unsigned char *)&node->key) != 0) {
            if (node->field_8 == 0) {
                node->field_8 = 1;
                node->frame = 0;
                FUN_00417af0((unsigned char *)&node->key, 0, 0);
            }
        }
        node = node->next;
    }
}

// FUNCTION: LEGOLAND 0x004183a0
void WaterWorksShowerUpdate(void) {
    struct WaterNode *node;
    struct LLS *lls;
    int limit;

    FUN_00418350();
    for (node = DAT_004cc030; node != NULL; node = node->next) {
        if (node->field_8 == 1 && (unsigned char)(node->frame += 1) >= 0xb) {
            node->field_8 = 2;
            node->frame = 0;
        }
        if (node->field_8 == 2) {
            limit = 0;
            node->frame++;
            lls = GetLLSForSprite((struct Sprite *)ShowerSprite);
            if (lls != NULL) {
                limit = lls->frame_count;
            }
            if ((int)node->frame >= limit) {
                if (IsPointInBlokeGardenerOrMechanicList((unsigned char *)&node->key) != 0) {
                    node->field_8 = 2;
                    node->frame = 0xb;
                } else {
                    node->field_8 = 3;
                    node->frame = 0x24;
                }
            }
        }
        if (node->field_8 == 3 && (unsigned char)(node->frame += 1) >= 0x2c) {
            node->field_8 = 0;
            node->frame = 0;
        }
    }
}

// FUNCTION: LEGOLAND 0x00418450
void RenderShower(struct Element *render, unsigned int p2, unsigned int p3, unsigned char *coords, void *p5, unsigned int p6) {
    struct WaterNode *node;
    struct Ride *ctx;
    struct AdjustOffset offset;
    struct Point sc;
    struct LLS *lls;

    ctx = render->ride;
    node = FUN_004182c0((unsigned short *)coords);
    if (node != NULL && node->field_8 == 2) {
        offset.x = -8;
        offset.y = 0x14;
        AdjustOffsetForViewMode((struct Point *)&offset);
        sc = GetScreenCoordsForObject((TileId *)coords, ctx);
        lls = GetLLSForSprite((struct Sprite *)ShowerSprite);
        LLSSetFrame(lls, node->frame);
        PrintSprite(ShowerSprite, sc.x + offset.x, sc.y + offset.y, p6, 0);
    }
}

// FUNCTION: LEGOLAND 0x004184e0
void ShowerAddObject(struct Element *editObj, unsigned char *coords) {
    unsigned short key;

    *(unsigned char *)&key = coords[0];
    *((unsigned char *)&key + 1) = coords[4];
    AddBasicObject(editObj, (int *)coords);
    FUN_00418260(key);
}

// FUNCTION: LEGOLAND 0x00418510
void ShowerRemoveObject(struct Element *a, unsigned int b, struct Cursor *c) {
    struct WaterNode *node;

    StandardRemoveObject(a, *(TileId *)&b, c);
    node = FUN_004182c0((unsigned short *)&b);
    if (node != NULL) {
        FUN_004182a0(node);
    }
}

// FUNCTION: LEGOLAND 0x00418540
struct RideSpriteInfo *GetWaterWorksShowerSpriteInfo(struct Element *arg1, unsigned short arg2) {
    struct Ride *sub;
    struct WaterListNode *node;

    sub = (struct Ride *)arg1->ride;
    DAT_004cbff0.id = arg2;
    DAT_004cbff0.sprite = sub->layer;
    DAT_004cbff0.x = sub->field_14;
    DAT_004cbff0.y = sub->field_18;
    DAT_004cbff0.field_10 = 0;
    node = (struct WaterListNode *)FUN_004182c0(&arg2);
    if (node != NULL) {
        if (node->field_8 == 2) {
            SetOverrideFrame(0xb);
        } else {
            SetOverrideFrame(node->field_9);
        }
    }
    return &DAT_004cbff0;
}

// FUNCTION: LEGOLAND 0x004185c0
void WaterWorksShowerCalcCursor(struct Element *arg, int *a, unsigned int b) {
    struct Ride *context;

    context = arg->ride;
    ScreenToMapRef(a, &EditCursor.tile_x, b);
    *(struct Footprint *)EditCursor.field_1414 = *(struct Footprint *)&context->footprint.x0;
    EditCursor.field_1830 = 0;
    ValidateCursor(&EditCursor, context);
    if (FUN_00417c90() != 0) {
        return;
    }
    SetCursorBlocked(&EditCursor, 0xc);
    PropagateCursorBlocked(&EditCursor);
}

// FUNCTION: LEGOLAND 0x00418630
void FUN_00418630(unsigned short key) {
    struct WaterNode *node;

    node = (struct WaterNode *)malloc(sizeof(struct WaterNode));
    if (node != NULL) {
        memset(node, 0, sizeof(struct WaterNode));
        node->key = key;
        InsertWaterNodeAtHead(&ElephantFountainList, node);
    }
}

// FUNCTION: LEGOLAND 0x00418670
void FUN_00418670(struct WaterNode *node) {
    FUN_00417bd0(&ElephantFountainList, node);
    free(node);
}

// FUNCTION: LEGOLAND 0x00418690
struct WaterNode *FUN_00418690(unsigned short *key) {
    return FindWaterNodeByKey(ElephantFountainList, key);
}

// FUNCTION: LEGOLAND 0x004186b0
void ElephantFountainLoad(struct Element *arg) {
    AcquireWaterWorksSfx();
    ElephantFountainRide = arg->ride;
    ElephantFountainRide->flags |= 0x20;
    ElephantFountainLayer = ElephantFountainRide->layer;
    // STRING: LEGOLAND 0x004b5078
    WwElsquirtSprite = LoadSprite("ww_elsquirt.lls", 1);
}

// FUNCTION: LEGOLAND 0x004186f0
void ElephantFountainUnload(void) {
    if (WwElsquirtSprite != 0) {
        KillSprite(WwElsquirtSprite);
    }
    ReleaseWaterWorksSfx();
}

// FUNCTION: LEGOLAND 0x00418710
unsigned int FUN_00418710(unsigned char *coords, int *base) {
    struct WaterRect rect;

    rect.x0 = coords[0] + base[0];
    rect.x1 = rect.x0 + 3;
    rect.y0 = coords[1] + base[3];
    rect.y1 = rect.y0 + 3;
    if (ListHasPointInRect(FirstBloke, &rect) != 0) {
        return 1;
    }
    if (ListHasPointInRect(GardenerList, &rect) != 0) {
        return 1;
    }
    return ListHasPointInRect(MechanicList, &rect) != 0;
}

// FUNCTION: LEGOLAND 0x004187b0
void FUN_004187b0(void) {
    struct WaterNode *node;

    node = (struct WaterNode *)ElephantFountainList;
    while (node != NULL) {
        if (FUN_00418710((unsigned char *)&node->key, (int *)&ElephantFountainRide->footprint.x0) != 0) {
            if (node->field_8 == 0) {
                node->field_8 = 1;
                node->frame = 0;
            }
        }
        node = node->next;
    }
}

// FUNCTION: LEGOLAND 0x004187f0
void WaterWorksElephantFountainUpdate(void) {
    struct WaterNode *node;
    struct LLS *lls;
    int limit;
    struct AdjustOffset offset;
    int y;
    struct Point sc;

    FUN_004187b0();
    for (node = ElephantFountainList; node != NULL; node = node->next) {
        if (node->field_8 == 1) {
            limit = 0;
            lls = GetLLSForSprite((struct Sprite *)WwElsquirtSprite);
            if (lls != NULL) {
                limit = lls->frame_count;
            }
            node->frame++;
            if ((int)node->frame >= limit) {
                node->field_8 = 0;
                node->frame = 0;
            }
            if (node->field_8 == 1) {
                offset.x = -0x75;
                offset.y = 0x8a;
                AdjustOffsetForViewMode((struct Point *)&offset);
                sc = GetScreenCoordsForObject((TileId *)&node->key, ElephantFountainRide);
                SetOverrideFrame(node->frame);
                y = sc.y + offset.y;
                SortSprite(WwElsquirtSprite, sc.x + offset.x, y, y + 0x20, 0, 0);
                ClearOverrideFrame();
            }
        }
    }
}

// FUNCTION: LEGOLAND 0x004188c0
void RenderWaterWorksElephantFountain(void) {}

// FUNCTION: LEGOLAND 0x004188d0
void ElephantFountainAddObject(struct Element *editObj, unsigned char *coords) {
    unsigned short key;

    *(unsigned char *)&key = coords[0];
    *((unsigned char *)&key + 1) = coords[4];
    AddBasicObject(editObj, (int *)coords);
    FUN_00418630(key);
    FUN_00417af0((unsigned char *)&key, 2, 1);
}

// FUNCTION: LEGOLAND 0x00418910
void ElephantFountainRemoveObject(struct Element *a, unsigned int b, struct Cursor *c) {
    struct WaterNode *node;

    StandardRemoveObject(a, *(TileId *)&b, c);
    node = FUN_00418690((unsigned short *)&b);
    if (node != NULL) {
        FUN_00418670(node);
    }
    FUN_00417b40((unsigned char *)&b);
}

// FUNCTION: LEGOLAND 0x00418950
void WaterWorksElephantFountainCalcCursor(struct Element *arg, int *a, unsigned int b) {
    struct Ride *context;

    context = arg->ride;
    *(struct Footprint *)EditCursor.field_1414 = *(struct Footprint *)&context->footprint.x0;
    ScreenToMapRef(a, &EditCursor.tile_x, b);
    EditCursor.field_1830 = 0;
    ValidateCursor(&EditCursor, context);
    if (FUN_00417c90() != 0) {
        return;
    }
    SetCursorBlocked(&EditCursor, 0xc);
    PropagateCursorBlocked(&EditCursor);
}

// FUNCTION: LEGOLAND 0x004189c0
void CrocodileFountainAddObject(struct Element *editObj, int *coords) {
    struct SampleSource params;

    AddBasicObject(editObj, coords);
    params.x = coords[0];
    params.type = 2;
    params.y = coords[1];
    PlayInstanceOfSample(WATERWORKS_SFX[2].sample, 1, 1, &params);
}

// FUNCTION: LEGOLAND 0x00418a10
void CrocodileFountainRemoveObject(struct Element *a, unsigned int b, struct Cursor *c) {
    RemoveSoundObject(a, b, c);
}

// FUNCTION: LEGOLAND 0x00418a30
void WaterWorksCrocodileFountainCalcCursor(struct Element *arg, int *a, unsigned int b) {
    struct Ride *data;

    data = arg->ride;
    memcpy(EditCursor.field_1414, &data->footprint.x0, 5 * sizeof(unsigned int));
    EditCursor.field_1830 = 0;
    ScreenToMapRef(a, &EditCursor.tile_x, b);
    ValidateCursor(&EditCursor, data);
    if (FUN_00417c90() != 0) {
        return;
    }
    SetCursorBlocked(&EditCursor, 0xc);
    PropagateCursorBlocked(&EditCursor);
}

// FUNCTION: LEGOLAND 0x00418aa0
LEGO_EXPORT int Save_WaterBlock(struct Element *element) {
    struct WaterNode *current;
    unsigned int one;
    unsigned int zero;

    one = 1;
    zero = 0;
    current = WaterNodeList;
    while (current != NULL) {
        if (SaveGameWrite(&one, 4) == 0) {
            return 0;
        }
        if (SaveGameWrite(current, sizeof(struct WaterNode)) == 0) {
            return 0;
        }
        current = current->next;
    }
    return SaveGameWrite(&zero, 4) != 0;
}

// FUNCTION: LEGOLAND 0x00418b10
LEGO_EXPORT int Load_WaterBlock(struct Element *element) {
    struct WaterNode *node;
    struct WaterNode *prev;
    unsigned int count;

    prev = NULL;
    if (SaveGameRead(&count, 4) == 0) {
        return 0;
    }
    while (count != 0) {
        node = (struct WaterNode *)malloc(sizeof(struct WaterNode));
        if (SaveGameRead(node, sizeof(struct WaterNode)) == 0) {
            return 0;
        }
        node->next = NULL;
        if (prev != NULL) {
            prev->next = node;
        } else {
            WaterNodeList = node;
        }
        prev = node;
        if (SaveGameRead(&count, 4) == 0) {
            return 0;
        }
    }
    return 1;
}

// FUNCTION: LEGOLAND 0x00418b90
LEGO_EXPORT int Save_ElephantF(struct Element *element) {
    struct WaterNode *current;
    unsigned int one;
    unsigned int zero;

    one = 1;
    zero = 0;
    current = ElephantFountainList;
    while (current != NULL) {
        if (SaveGameWrite(&one, 4) == 0) {
            return 0;
        }
        if (SaveGameWrite(current, sizeof(struct WaterNode)) == 0) {
            return 0;
        }
        current = current->next;
    }
    return SaveGameWrite(&zero, 4) != 0;
}

// FUNCTION: LEGOLAND 0x00418c00
LEGO_EXPORT int Load_ElephantF(struct Element *element) {
    struct WaterNode *node;
    struct WaterNode *prev;
    unsigned int count;

    prev = NULL;
    if (SaveGameRead(&count, 4) == 0) {
        return 0;
    }
    while (count != 0) {
        node = (struct WaterNode *)malloc(sizeof(struct WaterNode));
        if (SaveGameRead(node, sizeof(struct WaterNode)) == 0) {
            return 0;
        }
        node->next = NULL;
        if (prev != NULL) {
            prev->next = node;
        } else {
            ElephantFountainList = node;
        }
        prev = node;
        if (SaveGameRead(&count, 4) == 0) {
            return 0;
        }
    }
    return 1;
}

// FUNCTION: LEGOLAND 0x00418c80
void WaterWorksGetInterfaces(struct Element *name, struct Ride *ci) {
    // STRING: LEGOLAND 0x004b50f4
    if (_stricmp("WATER WORKS ENTRANCE", name->name) == 0) {
        ci->cb_a4 = (void (*)())WaterWorksEntranceLoad;
        ci->cb_ac = thunk_FUN_00417ac0;
        ci->cb_98 = WaterWorksEntranceAddObject;
        ci->cb_9c = WaterWorksEntranceRemoveObject;
        return;
    }
    // STRING: LEGOLAND 0x004b50dc
    if (_stricmp("WATER WORKS WATER BLOCK", name->name) == 0) {
        ci->cb_a4 = WaterBlockLoad;
        ci->cb_98 = WaterBlockAddObject;
        ci->cb_9c = WaterBlockRemoveObject;
        ci->cb_pre_render = WaterWorksWaterBlockUpdate;
        ci->cb_ac = WaterBlockUnload;
        ci->cb_sprite = GetWaterWorksWaterBlockSpriteInfo;
        ci->cb_b0 = RenderWaterBlock;
        ci->cb_90 = WaterWorksWaterBlockCalcCursor;
        ci->save_hook = Save_WaterBlock;
        ci->load_hook = Load_WaterBlock;
        return;
    }
    // STRING: LEGOLAND 0x004b50c8
    if (_stricmp("WATER WORKS SHOWER", name->name) == 0) {
        ci->cb_a4 = ShowerLoad;
        ci->cb_98 = ShowerAddObject;
        ci->cb_9c = ShowerRemoveObject;
        ci->cb_ac = ShowerUnload;
        ci->cb_sprite = GetWaterWorksShowerSpriteInfo;
        ci->cb_b0 = RenderShower;
        ci->cb_pre_render = WaterWorksShowerUpdate;
        ci->cb_90 = WaterWorksShowerCalcCursor;
        return;
    }
    // STRING: LEGOLAND 0x004b50a8
    if (_stricmp("WATER WORKS ELEPHANT FOUNTAIN", name->name) == 0) {
        ci->cb_a4 = ElephantFountainLoad;
        ci->cb_98 = ElephantFountainAddObject;
        ci->cb_9c = ElephantFountainRemoveObject;
        ci->cb_ac = ElephantFountainUnload;
        ci->cb_b0 = RenderWaterWorksElephantFountain;
        ci->cb_pre_render = WaterWorksElephantFountainUpdate;
        ci->cb_90 = WaterWorksElephantFountainCalcCursor;
        ci->save_hook = Save_ElephantF;
        ci->load_hook = Load_ElephantF;
        return;
    }
    // STRING: LEGOLAND 0x004b5088
    if (_stricmp("WATER WORKS CROCODILE FOUNTAIN", name->name) == 0) {
        ci->cb_90 = WaterWorksCrocodileFountainCalcCursor;
        ci->cb_98 = CrocodileFountainAddObject;
        ci->cb_9c = CrocodileFountainRemoveObject;
    }
}
