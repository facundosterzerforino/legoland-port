#include <stdlib.h>
#include "globals.h"
#include "legoland.h"

#include "bloke.h"
#include "bloke_ai.h"
#include "eatery.h"
#include "gamemap.h"
#include "llidb.h"
#include "man3d.h"
#include "map_object.h"
#include "math.h"
#include "money.h"
#include "obj_instance.h"
#include "print_sprite.h"
#include "render3d.h"
#include "sound_music.h"

#include "image_sprite.h"

struct EateryObj;
struct EateryFX;
struct EateryInner;
struct BlokeNode;

struct EateryFX {
    /* 0x00 */ unsigned char pad_0[0x14];
    /* 0x14 */ unsigned int field_14;
    /* 0x18 */ unsigned int field_18;
    /* 0x1c */ unsigned int flags_1c;
    /* 0x20 */ unsigned char pad_20[0x44];
    /* 0x64 */ struct EateryInner *inner_64;
};

struct EateryInner {
    unsigned char pad_0[0x10];
    unsigned int flags_10;
};

struct EateryObj {
    unsigned char pad_0[0xc];
    struct EateryFX *fx_c;
};

struct BrollyData {
    /* 0x00 */ unsigned char pad_0[4];
    /* 0x04 */ int count_4;
    /* 0x08 */ int *table_8;
    /* 0x0c */ int *table_c;
    /* 0x10 */ int *table_10;
};

struct BlokeNode {
    struct BlokeNode *next;
    unsigned char pad_4[4];
    unsigned int field_8;
    unsigned short field_c;
};

struct Seats {
    unsigned char c[3];
};

struct BrollyNode {
    /* 0x00 */ struct BrollyNode *next;
    /* 0x04 */ unsigned short value;
    /* 0x06 */ struct Seats seats;
    /* 0x09 */ unsigned char field_9;
};

struct BlokeOwner {
    unsigned char pad_0[0xcc];
    struct BlokeNode *head_cc;
};

struct BlokeArg {
    unsigned char pad_0[0xc];
    struct BlokeOwner *owner_c;
};

struct EditArg {
    unsigned char pad_0[0xc];
    unsigned int var_c;
};

struct UserFlagsArg {
    int var_0;
    int var_4;
};

struct SaveBlock {
    /* 0x00 */ struct SaveBlock *next;
    /* 0x04 */ unsigned short value;
    /* 0x06 */ unsigned char field_6;
    /* 0x07 */ unsigned char field_7;
    /* 0x08 */ unsigned char field_8;
    /* 0x09 */ unsigned char field_9;
    /* 0x0a */ unsigned char pad_a[0xc - 0xa];
    /* 0x0c */ unsigned int field_c;
    /* 0x10 */ unsigned char field_10;
    /* 0x11 */ unsigned char field_11;
    /* 0x12 */ unsigned char pad_12[0x14 - 0x12];
    /* 0x14 */ unsigned int field_14;
    /* 0x18 */ unsigned int field_18;
    /* 0x1c */ unsigned int field_1c;
    /* 0x20 */ unsigned int field_20;
    /* 0x24 */ unsigned int field_24;
    /* 0x28 */ unsigned int field_28;
    /* 0x2c */ unsigned int field_2c;
    /* 0x30 */ unsigned int field_30;
    /* 0x34 */ unsigned int field_34;
    /* 0x38 */ unsigned int field_38;
    /* 0x3c */ unsigned int field_3c;
};

// FUNCTION: LEGOLAND 0x0042e220
void LoadChuckWagonResources(struct EateryObj *obj) {
    struct EateryFX *fx = obj->fx_c;
    struct EateryInner *inner;
    ChuckWagonRide = fx;
    fx->flags_1c |= 0x20;
    inner = ChuckWagonRide->inner_64;
    inner->flags_10 |= 0x2000;
    LoadMoneySFX();
}

// FUNCTION: LEGOLAND 0x0042e250
void UnloadChuckWagonResources(void) { KillMoneySFX(); }

// FUNCTION: LEGOLAND 0x0042e260
void RenderChuckWagon(struct BlokeArg *arg, unsigned int param2, unsigned int param3, unsigned short *value) {
    struct BlokeOwner *owner = arg->owner_c;
    struct BlokeNode *node = owner->head_cc;
    if (node != NULL) {
        while (node != NULL) {
            if (*value == node->field_c) {
                IP_RenderBlokeIn3DNow((struct Bloke *)node->field_8);
                GetScreenCoordsForObject((unsigned char *)value, owner);
                break;
            }
            node = node->next;
        }
    }
}

// FUNCTION: LEGOLAND 0x0042e2a0
void ChuckWagonUpdate(Element *obj) {
    struct Ride *ride = obj->ride;
    struct RideNode *node = ride->riders;
    struct RideNode *next;
    struct Bloke *bloke;
    TileId *tile;
    int x;
    int y;
    char dir;

    while (node != NULL) {
        next = node->next;
        tile = &node->tile;
        bloke = node->rider;
        x = tile->pos.x + ride->x;
        y = tile->pos.y + ride->y;
        if (bloke->low_level_action == 0) {
            switch (bloke->param_action) {
            case 0:
                bloke->flags |= 8;
                bloke->dest.x = x << 8;
                bloke->dest.y = (y + 1) << 8;
                dir = CalcMoveLine(bloke->pos, bloke->dest, &bloke->nav);
                bloke->low_level_action = 7;
                bloke->field_73 = dir + 0x10;
                NewDirForAction(bloke, ((unsigned char)(dir + 0x10) >> 5) + 3);
                bloke->param_action++;
                break;
            case 1:
                bloke->dest.x = (x << 8) - 0x80;
                bloke->dest.y = (y + 1) << 8;
                dir = CalcMoveLine(bloke->pos, bloke->dest, &bloke->nav);
                bloke->low_level_action = 7;
                bloke->field_73 = dir + 0x10;
                NewDirForAction(bloke, ((unsigned char)(dir + 0x10) >> 5) + 3);
                bloke->param_action++;
            case 2:
                bloke->dir = 7;
                bloke->param_action++;
                bloke->field_58 = (rand() & 0x1f) + 4;
                break;
            case 3:
                if (bloke->field_58 == 0) {
                    bloke->param_action++;
                    BuyItem((struct BuyItemArg *)obj, tile, 0);
                }
                bloke->field_58--;
                break;
            case 4:
                bloke->dest.x = (x << 8) + 0x80;
                bloke->dest.y = (y << 8) + 0x80;
                dir = CalcMoveLine(bloke->pos, bloke->dest, &bloke->nav);
                bloke->low_level_action = 7;
                bloke->field_73 = dir + 0x10;
                NewDirForAction(bloke, ((unsigned char)(dir + 0x10) >> 5) + 3);
                bloke->param_action++;
                break;
            case 5:
                RemoveBlokeFromRide(ride, node);
                bloke->flags &= 0xfff7;
                break;
            }
        }
        node = next;
    }
}

// FUNCTION: LEGOLAND 0x0042e460
void LoadBrollyImages(struct EateryObj *obj) {
    DAT_0081cd38 = obj->fx_c;
    DAT_0081cd38->flags_1c |= 0x400;
    // STRING: LEGOLAND 0x004b6ec8
    if (LLIDB_FindElement("BROLLY IMAGES", &BrollyImagesHandle, 0) == 0) {
        BrollyImagesData = (struct BrollyData *)LLIDB_LoadData((void *)BrollyImagesHandle);
    }
}

// FUNCTION: LEGOLAND 0x0042e4b0
void UnloadBrollyImages(void) {
    LLIDB_UnLoadData(BrollyImagesHandle);
}

// FUNCTION: LEGOLAND 0x0042e4c0
void BrollySetEditMode(void) {
    struct EateryFX *temp = DAT_0081cd38;
    EditMode.unk0 = 1;
    EditMode.unk8 = temp;
    DefaultCursor(&EditCursor);
    SetEditCursorFootPrint((void *)((unsigned int)EditMode.unk8 + 0x3c));
}

// FUNCTION: LEGOLAND 0x0042e500
void BrollyAddObject(int param_1, unsigned char *param_2) {
    TileId id;
    id.pos.x = param_2[0];
    id.pos.y = param_2[4];
    AddObjectToMap(param_1, id, 0);
    if (BrollyImagesData != NULL) {
        int r = rand();
        Set_UserFlags(*(int *)param_2 << 8, *(int *)(param_2 + 4) << 8, (unsigned short)(r % BrollyImagesData->count_4));
    }
}

// FUNCTION: LEGOLAND 0x0042e560
struct RideSpriteInfo *GetBrollySpriteInfo(int param_1, unsigned int param_2) {
    unsigned char *b = (unsigned char *)&param_2;
    int i = Get_UserFlags((unsigned int)b[0] << 8, (unsigned int)b[1] << 8) & 0xffff;
    RideSpriteInfoBuffer.sprite = (void *)BrollyImagesData->table_8[(unsigned char)i];
    RideSpriteInfoBuffer.x = BrollyImagesData->table_c[(unsigned char)i] >> 1;
    RideSpriteInfoBuffer.y = BrollyImagesData->table_10[(unsigned char)i] >> 1;
    RideSpriteInfoBuffer.field_10 = 0;
    return &RideSpriteInfoBuffer;
}

// FUNCTION: LEGOLAND 0x0042e5d0
void LoadSharkCafeResources(struct EateryObj *obj) {
    struct EateryFX *fx = obj->fx_c;
    struct EateryInner *inner;
    SharkCafeRide = fx;
    SharkCafeRide->flags_1c |= 0x20;
    inner = SharkCafeRide->inner_64;
    inner->flags_10 |= 0x2000;
    LoadMoneySFX();
}

// FUNCTION: LEGOLAND 0x0042e600
void UnloadSharkCafeResources(void) { KillMoneySFX(); }

// FUNCTION: LEGOLAND 0x0042e610
void SharkCafeUpdate(Element *obj) {
    struct Ride *ride = obj->ride;
    struct RideNode *node = ride->riders;
    struct RideNode *next;
    struct Bloke *bloke;
    TileId *tile;
    int x;
    int y;
    char dir;

    while (node != NULL) {
        next = node->next;
        tile = &node->tile;
        bloke = node->rider;
        x = tile->pos.x + ride->x;
        y = tile->pos.y + ride->y;
        if (bloke->low_level_action == 0) {
            switch (bloke->param_action) {
            case 0:
                bloke->flags |= 8;
                bloke->dest.x = (x << 8) - 0x80;
                bloke->dest.y = (y << 8) + 0x80;
                dir = CalcMoveLine(bloke->pos, bloke->dest, &bloke->nav);
                bloke->low_level_action = 7;
                bloke->field_73 = dir + 0x10;
                NewDirForAction(bloke, ((unsigned char)(dir + 0x10) >> 5) + 3);
                bloke->param_action++;
                break;
            case 1:
                bloke->dir = 7;
                bloke->param_action++;
                bloke->field_58 = (rand() & 0x1f) + 4;
                break;
            case 2:
                if (bloke->field_58 == 0) {
                    bloke->param_action++;
                    BuyItem((struct BuyItemArg *)obj, tile, 1);
                }
                bloke->field_58--;
                break;
            case 3:
                bloke->dest.x = (x << 8) + 0x80;
                bloke->dest.y = (y << 8) + 0x80;
                dir = CalcMoveLine(bloke->pos, bloke->dest, &bloke->nav);
                bloke->low_level_action = 7;
                bloke->field_73 = dir + 0x10;
                NewDirForAction(bloke, ((unsigned char)(dir + 0x10) >> 5) + 3);
                bloke->param_action++;
                break;
            case 4:
                RemoveBlokeFromRide(ride, node);
                bloke->flags &= 0xfff7;
                NewLongTermAction(bloke, 0xd);
                break;
            }
        }
        node = next;
    }
}

// FUNCTION: LEGOLAND 0x0042e770
void LoadFoodcartDrinkResources(struct EateryObj *obj) {
    struct EateryFX *fx = obj->fx_c;
    struct EateryInner *inner;
    FoodcartDrinkRide = (unsigned int)fx;
    fx->flags_1c |= 0x20;
    inner = ((struct EateryFX *)FoodcartDrinkRide)->inner_64;
    inner->flags_10 |= 0x2000;
    LoadMoneySFX();
}

// FUNCTION: LEGOLAND 0x0042e7a0
void UnloadFoodcartDrinkResources(void) { KillMoneySFX(); }

// FUNCTION: LEGOLAND 0x0042e7b0
void LoadFoodcartIcecreamResources(struct EateryObj *obj) {
    struct EateryFX *fx = obj->fx_c;
    struct EateryInner *inner;
    FoodcartIcecreamRide = fx;
    fx->flags_1c |= 0x20;
    inner = FoodcartIcecreamRide->inner_64;
    inner->flags_10 |= 0x2000;
    LoadMoneySFX();
}

// FUNCTION: LEGOLAND 0x0042e7e0
void UnloadFoodcartIcecreamResources(void) { KillMoneySFX(); }

// FUNCTION: LEGOLAND 0x0042e7f0
void LoadFoodcartFoodResources(struct EateryObj *obj) {
    struct EateryFX *fx = obj->fx_c;
    struct EateryInner *inner;
    FoodcartFoodRide = (unsigned int)fx;
    fx->flags_1c |= 0x20;
    inner = ((struct EateryFX *)FoodcartFoodRide)->inner_64;
    inner->flags_10 |= 0x2000;
    LoadMoneySFX();
}

// FUNCTION: LEGOLAND 0x0042e820
void UnloadFoodcartFoodResources(void) { KillMoneySFX(); }

// FUNCTION: LEGOLAND 0x0042e830
void FUN_0042e830(struct BlokeArg *arg, unsigned int param2, unsigned int param3, unsigned short *value) {
    struct BlokeOwner *owner = arg->owner_c;
    struct BlokeNode *node = owner->head_cc;
    if (node == NULL) {
        return;
    }
    while (node != NULL) {
        if (*value == node->field_c) {
            IP_RenderBlokeIn3DNow((struct Bloke *)node->field_8);
        }
        node = node->next;
    }
}

// FUNCTION: LEGOLAND 0x0042e870
void LoadCastleBBQResources(struct EateryObj *obj) {
    CastleBBQRide = (unsigned int)obj->fx_c;
    ((struct EateryFX *)CastleBBQRide)->flags_1c |= 0x20;
    CastleBBQLayer = (unsigned int)((struct EateryFX *)CastleBBQRide)->inner_64;
    ((struct EateryInner *)CastleBBQLayer)->flags_10 |= 0x2000;
    Load_FXList(RESTAURANT_SFX, 1);
    LoadMoneySFX();
}

// FUNCTION: LEGOLAND 0x0042e8b0
void UnloadCastleBBQResources(void) {
    Kill_FXList(RESTAURANT_SFX, 1);
    CastleBBQRide = 0;
    KillMoneySFX();
}

// FUNCTION: LEGOLAND 0x0042e8d0
void EaterySetEditMode(struct EditArg *arg) {
    unsigned int temp = arg->var_c;
    EditMode.unk0 = 1;
    EditMode.unk8 = (void *)temp;
    DefaultCursor(&EditCursor);
    SetEditCursorFootPrint((void *)((unsigned int)EditMode.unk8 + 0x3c));
}

// FUNCTION: LEGOLAND 0x0042e910
void RenderCastleBBQ(int param_1, unsigned int param_2, unsigned int param_3, short *param_4, unsigned int param_5, unsigned int param_6) {
    int ride = *(int *)(param_1 + 0xc);
    unsigned int *node;
    struct Point coords;
    struct Point offset;
    for (node = *(unsigned int **)(ride + 0xcc); node != NULL; node = (unsigned int *)*node) {
        if (*param_4 == *(short *)(node + 3)) {
            IP_RenderBlokeIn3DNow((struct Bloke *)node[2]);
        }
    }
    coords = GetScreenCoordsForObject((unsigned char *)param_4, (void *)ride);
    offset = GetRenderOffsetForLayer((struct LayerOffsetHolder *)CastleBBQLayer, 1);
    AdjustOffsetForViewMode(&offset);
    if (GetSpriteForLayer((struct LayerContainer *)CastleBBQLayer, 1) != 0) {
        PrintSprite((struct Sprite *)GetSpriteForLayer((struct LayerContainer *)CastleBBQLayer, 1),
            offset.x + coords.x, offset.y + coords.y, param_6, 0);
    }
}

// FUNCTION: LEGOLAND 0x0042e9c0
void CastleBBQAddObject(unsigned int param_1, unsigned int *param_2) {
    struct SampleParams params;
    AddBasicObject(param_1, (unsigned int)param_2);
    params.x = *param_2;
    params.y = param_2[1];
    params.field_0 = 2;
    PlayInstanceOfSample(*(void **)(RESTAURANT_SFX + 8), 1, 1, &params);
}

// FUNCTION: LEGOLAND 0x0042ea10
void CastleBBQRemoveObject(unsigned int param_1, unsigned int param_2, unsigned int param_3) {
    unsigned char *b = (unsigned char *)&param_2;
    struct SampleParams params;
    StandardRemoveObject(param_1, *(TileId *)&param_2, param_3);
    params.x = b[0];
    params.y = b[1];
    params.field_0 = 2;
    UnSourceAndFadeAllSamplesFromSource(&params, 0xffffff38);
}

// FUNCTION: LEGOLAND 0x0042ea60
void CastleBBQUpdate(Element *obj) {
    struct Ride *ride = obj->ride;
    struct RideNode *node = ride->riders;
    struct RideNode *next;
    struct Bloke *bloke;
    TileId *tile;
    int x;
    int y;
    char dir;

    while (node != NULL) {
        next = node->next;
        tile = &node->tile;
        bloke = node->rider;
        x = tile->pos.x + ride->x;
        y = tile->pos.y + ride->y;
        if (bloke->low_level_action == 0) {
            switch (bloke->param_action) {
            case 0:
                bloke->flags |= 8;
                bloke->dest.x = (x << 8) + 0x280;
                bloke->dest.y = (y << 8) + 0x80;
                dir = CalcMoveLine(bloke->pos, bloke->dest, &bloke->nav);
                bloke->low_level_action = 7;
                bloke->field_73 = dir + 0x10;
                NewDirForAction(bloke, ((unsigned char)(dir + 0x10) >> 5) + 3);
                bloke->param_action++;
                break;
            case 1:
                bloke->dest.x = (x << 8) + 0x280;
                bloke->dest.y = y << 8;
                dir = CalcMoveLine(bloke->pos, bloke->dest, &bloke->nav);
                bloke->low_level_action = 7;
                bloke->field_73 = dir + 0x10;
                NewDirForAction(bloke, ((unsigned char)(dir + 0x10) >> 5) + 3);
                bloke->param_action++;
                break;
            case 2:
                bloke->field_58 = (rand() & 0x1f) + 4;
                bloke->param_action++;
                break;
            case 3:
                if (bloke->field_58 == 0) {
                    bloke->param_action++;
                    PlayMoneySFX(tile, 0, 0);
                }
                bloke->field_58--;
                break;
            case 4:
                bloke->dest.x = (x << 8) + 0x280;
                bloke->dest.y = (y << 8) + 0x80;
                dir = CalcMoveLine(bloke->pos, bloke->dest, &bloke->nav);
                bloke->low_level_action = 7;
                bloke->field_73 = dir + 0x10;
                NewDirForAction(bloke, ((unsigned char)(dir + 0x10) >> 5) + 3);
                bloke->param_action++;
                break;
            case 5:
                bloke->dest.x = (x << 8) + 0x80;
                bloke->dest.y = (y << 8) + 0x80;
                dir = CalcMoveLine(bloke->pos, bloke->dest, &bloke->nav);
                bloke->low_level_action = 7;
                bloke->field_73 = dir + 0x10;
                NewDirForAction(bloke, ((unsigned char)(dir + 0x10) >> 5) + 3);
                bloke->param_action++;
                break;
            case 6:
                RemoveBlokeFromRide(ride, node);
                bloke->flags &= 0xfff7;
                break;
            }
        }
        node = next;
    }
}

// FUNCTION: LEGOLAND 0x0042ec10
void FoodcartDrinkUpdate(Element *obj) {
    struct Ride *ride = obj->ride;
    struct RideNode *node = ride->riders;
    struct RideNode *next;
    struct Bloke *bloke;
    TileId *tile;
    int x;
    int y;
    char dir;

    while (node != NULL) {
        next = node->next;
        tile = &node->tile;
        bloke = node->rider;
        x = tile->pos.x + ride->x;
        y = tile->pos.y + ride->y;
        if (bloke->low_level_action == 0) {
            switch (bloke->param_action) {
            case 0:
                bloke->flags |= 8;
                bloke->dest.x = (x << 8) - 0x80;
                bloke->dest.y = (y << 8) + 0x180;
                dir = CalcMoveLine(bloke->pos, bloke->dest, &bloke->nav);
                bloke->low_level_action = 7;
                bloke->field_73 = dir + 0x10;
                NewDirForAction(bloke, ((unsigned char)(dir + 0x10) >> 5) + 3);
                bloke->param_action++;
                break;
            case 1:
                bloke->dir = 7;
                bloke->param_action++;
                bloke->field_58 = (rand() & 0x1f) + 4;
                break;
            case 2:
                if (bloke->field_58 == 0) {
                    bloke->param_action++;
                    BuyItem((struct BuyItemArg *)obj, tile, 1);
                }
                bloke->field_58--;
                break;
            case 3:
                bloke->dir = 3;
                bloke->dest.x = (x << 8) + 0x80;
                bloke->dest.y = (y << 8) + 0x80;
                dir = CalcMoveLine(bloke->pos, bloke->dest, &bloke->nav);
                bloke->low_level_action = 7;
                bloke->field_73 = dir + 0x10;
                NewDirForAction(bloke, ((unsigned char)(dir + 0x10) >> 5) + 3);
                bloke->param_action++;
                break;
            case 4:
                RemoveBlokeFromRide(ride, node);
                bloke->flags &= 0xfff7;
                break;
            }
        }
        node = next;
    }
}

// FUNCTION: LEGOLAND 0x0042ed70
void FoodcartFoodUpdate(Element *obj) {
    struct Ride *ride = obj->ride;
    struct RideNode *node = ride->riders;
    struct RideNode *next;
    struct Bloke *bloke;
    TileId *tile;
    int x;
    int y;
    char dir;

    while (node != NULL) {
        next = node->next;
        tile = &node->tile;
        bloke = node->rider;
        x = tile->pos.x + ride->x;
        y = tile->pos.y + ride->y;
        if (bloke->low_level_action == 0) {
            switch (bloke->param_action) {
            case 0:
                bloke->flags |= 8;
                bloke->dest.x = (x << 8) - 0x80;
                bloke->dest.y = (y << 8) + 0x80;
                dir = CalcMoveLine(bloke->pos, bloke->dest, &bloke->nav);
                bloke->low_level_action = 7;
                bloke->field_73 = dir + 0x10;
                NewDirForAction(bloke, ((unsigned char)(dir + 0x10) >> 5) + 3);
                bloke->param_action++;
                break;
            case 1:
                bloke->dir = 7;
                bloke->param_action++;
                bloke->field_58 = (rand() & 0x1f) + 4;
                break;
            case 2:
                if (bloke->field_58 == 0) {
                    bloke->param_action++;
                    BuyItem((struct BuyItemArg *)obj, tile, 1);
                }
                bloke->field_58--;
                break;
            case 3:
                bloke->dest.x = (x << 8) + 0x80;
                bloke->dest.y = (y << 8) + 0x80;
                dir = CalcMoveLine(bloke->pos, bloke->dest, &bloke->nav);
                bloke->low_level_action = 7;
                bloke->field_73 = dir + 0x10;
                NewDirForAction(bloke, ((unsigned char)(dir + 0x10) >> 5) + 3);
                bloke->param_action++;
                break;
            case 4:
                RemoveBlokeFromRide(ride, node);
                bloke->flags &= 0xfff7;
                break;
            }
        }
        node = next;
    }
}

// FUNCTION: LEGOLAND 0x0042eec0
struct BrollyNode *FUN_0042eec0(unsigned short *param_1) {
    struct BrollyNode *node = malloc(sizeof(struct BrollyNode));
    if (node != NULL) {
        memset(node, 0, sizeof(struct BrollyNode));
        node->value = *param_1;
        node->next = DAT_00616144;
        memset(&node->seats, 0, sizeof(node->seats));
        node->field_9 = 0;
        DAT_00616144 = node;
    }
    return node;
}

// FUNCTION: LEGOLAND 0x0042ef10
void Restaurant1AddObject(unsigned int param_1, unsigned char *param_2) {
    unsigned char id[2];
    id[0] = param_2[0];
    id[1] = param_2[4];
    AddBasicObject(param_1, (unsigned int)param_2);
    FUN_0042eec0((unsigned short *)id);
}

// FUNCTION: LEGOLAND 0x0042ef40
struct BrollyNode *FUN_0042ef40(unsigned short *param_1) {
    struct BrollyNode *node = DAT_00616144;
    if (node == NULL) {
        return NULL;
    }
    while (memcmp(&node->value, param_1, 2) != 0) {
        node = node->next;
        if (node == NULL) {
            return NULL;
        }
    }
    return node;
}

// FUNCTION: LEGOLAND 0x0042ef70
void FUN_0042ef70(struct BrollyNode *node) {
    struct BrollyNode *head = DAT_00616144;
    if (head == node) {
        DAT_00616144 = node->next;
    } else {
        struct BrollyNode *current = head;
        if (current->next != node) {
            do {
                current = current->next;
            } while (current && current->next != node);
        }
        if (current) {
            current->next = node->next;
        }
    }
    free(node);
}

// FUNCTION: LEGOLAND 0x0042efb0
void Restaurant1RemoveObject(unsigned int param_1, TileId tile, unsigned int param_3) {
    struct SampleParams params;
    struct BrollyNode *node = FUN_0042ef40(&tile.id);
    if (node != NULL) {
        FUN_0042ef70(node);
    }
    StandardRemoveObject(param_1, tile, param_3);
    RemoveAllBlokesFromRide((unsigned int)((struct EateryObj *)param_1)->fx_c, tile);
    params.x = tile.pos.x;
    params.y = tile.pos.y;
    params.field_0 = 2;
    UnSourceAndFadeAllSamplesFromSource(&params, 0xffffff38);
}

// FUNCTION: LEGOLAND 0x0042f030
void LoadRestaurant1Resources(struct EateryObj *obj) {
    Restaurant1Ride = (unsigned int)obj->fx_c;
    ((struct EateryFX *)Restaurant1Ride)->flags_1c |= 0x20;
    EateryFxInner = (unsigned int)((struct EateryFX *)Restaurant1Ride)->inner_64;
    ((struct EateryInner *)EateryFxInner)->flags_10 |= 0x2000;
    // STRING: LEGOLAND 0x004b6f2c
    RestMaskMainSprite = LoadSprite("RestMask_Main.lls", 1);
    // STRING: LEGOLAND 0x004b6f14
    RestMaskLevel1aaSprite = LoadSprite("RestMaskLevel1aa.lls", 1);
    // STRING: LEGOLAND 0x004b6f00
    RestMaskLevel1Sprite = LoadSprite("RestMaskLevel1.lls", 1);
    // STRING: LEGOLAND 0x004b6eec
    RestMaskLevel2Sprite = LoadSprite("RestMaskLevel2.lls", 1);
    // STRING: LEGOLAND 0x004b6ed8
    RestMaskLevel3Sprite = LoadSprite("RestMaskLevel3.lls", 1);
    HideLayer((struct LayerOwner *)EateryFxInner, 1);
    StopLayerPlaying(EateryFxInner, 1);
    LLSSetFrame((struct LLS *)GetLLSForLayer(EateryFxInner, 1), 0);
    LoadMoneySFX();
}

// FUNCTION: LEGOLAND 0x0042f0f0
void FUN_0042f0f0(struct Bloke *bloke, int x, int y, int step) {
    int idx = (step + bloke->field_36 * 5) * 6;
    int y_add = DAT_004b66f4[idx + 1];
    int dir_flag = DAT_004b66f4[idx + 4];
    int frame = DAT_004b66f4[idx + 5];
    char dir;

    bloke->dest.x = (DAT_004b66f4[idx] + x) * 0x100 + DAT_004b66f4[idx + 2];
    bloke->dest.y = (y + y_add) * 0x100 + DAT_004b66f4[idx + 3];
    dir = CalcMoveLine(bloke->pos, bloke->dest, &bloke->nav);
    bloke->field_73 = dir + 0x10;
    bloke->low_level_action = 7;
    bloke->field_37 = (unsigned char)frame;
    if (dir_flag == 1) {
        NewDirForAction(bloke, ((unsigned char)(dir + 0x10) >> 5) + 3);
    }
}

// FUNCTION: LEGOLAND 0x0042f1a0
void Restaurant1Update(Element *obj) {
    struct Ride *ride = obj->ride;
    struct RideNode *node = ride->riders;
    struct RideNode *next;
    struct Bloke *bloke;
    TileId *tile;
    struct BrollyNode *state;
    struct Seats *slot;
    struct Seats seats;
    char last_zero = 0;
    char zero_count = 0;
    char i;
    char *p;
    char dir;
    char pick;
    int x;
    int y;

    while (node != NULL) {
        next = node->next;
        tile = &node->tile;
        bloke = node->rider;
        state = FUN_0042ef40((unsigned short *)tile);
        if (state == NULL) {
            return;
        }
        slot = &state->seats;
        seats = *slot;
        x = ride->x + tile->pos.x;
        y = tile->pos.y + ride->y;
        if (bloke->low_level_action == 0) {
            switch (bloke->param_action) {
            case 0:
                bloke->flags |= 8;
                bloke->field_37 = 3;
                bloke->dest.x = (x - 6) << 8;
                bloke->dest.y = y << 8;
                dir = CalcMoveLine(bloke->pos, bloke->dest, &bloke->nav);
                bloke->low_level_action = 7;
                bloke->field_73 = dir + 0x10;
                NewDirForAction(bloke, ((unsigned char)(dir + 0x10) >> 5) + 3);
                for (i = 0, p = (char *)seats.c; i < 3; i++, p++) {
                    if (*p == 0) {
                        zero_count++;
                        last_zero = i;
                    }
                }
                if (zero_count != 0) {
                    if (zero_count == 3) {
                        pick = rand() % 3;
                    } else {
                        pick = last_zero;
                    }
                    seats.c[pick] = 1;
                    bloke->field_36 = pick;
                    bloke->field_46 = 3;
                    bloke->field_5c = 300;
                    bloke->param_action++;
                } else {
                    bloke->field_46 = 4;
                    bloke->field_5c = 500;
                    bloke->param_action++;
                }
                break;
            case 1:
                BuyItem((struct BuyItemArg *)obj, tile, 1);
                if (bloke->field_46 == 4) {
                    bloke->param_action = 8;
                } else {
                    FUN_0042f0f0(bloke, x, y, 0);
                    bloke->param_action++;
                }
                break;
            case 2:
                FUN_0042f0f0(bloke, x, y, 1);
                bloke->param_action++;
                break;
            case 3:
                FUN_0042f0f0(bloke, x, y, 2);
                bloke->param_action++;
                break;
            case 4:
                bloke->flags |= 0x100;
                bloke->height = 10;
                BlokeSitAnim(bloke);
                BlokeSetFrame(bloke, 0);
                bloke->param_action++;
                break;
            case 5:
                if ((int)bloke->field_5c-- < 0) {
                    bloke->param_action++;
                }
                break;
            case 6:
                bloke->flags &= 0xfeff;
                bloke->height = 0;
                BlokeWalkAnim(bloke);
                FUN_0042f0f0(bloke, x, y, 3);
                bloke->param_action++;
                break;
            case 7:
                FUN_0042f0f0(bloke, x, y, 4);
                bloke->param_action += 2;
                seats.c[bloke->field_36] = 0;
                break;
            case 8:
                if ((int)bloke->field_5c-- < 0) {
                    bloke->param_action++;
                }
            case 9:
                bloke->field_37 = 3;
                bloke->dest.x = x << 8;
                bloke->dest.y = (y + 1) << 8;
                dir = CalcMoveLine(bloke->pos, bloke->dest, &bloke->nav);
                bloke->low_level_action = 7;
                bloke->field_73 = dir + 0x10;
                NewDirForAction(bloke, ((unsigned char)(dir + 0x10) >> 5) + 3);
                bloke->param_action++;
                break;
            case 10:
                RemoveBlokeFromRide(ride, node);
                bloke->flags &= 0xfff7;
                break;
            }
        }
        *slot = seats;
        node = next;
    }
}

// FUNCTION: LEGOLAND 0x0042f4c0
void RenderRestaurant1(int param_1, unsigned int param_2, unsigned int param_3, short *param_4, unsigned int param_5, unsigned int param_6) {
    int ride = *(int *)(param_1 + 0xc);
    int *node;
    int blokes[8];
    int count;
    int i;
    int *p;
    struct BrollyNode *state;
    struct Point coords;
    struct Point offset;
    struct {
        /* 0x00 */ int field_0;
        /* 0x04 */ int field_4;
        /* 0x08 */ short field_8;
    } cfg;
    node = *(int **)(ride + 0xcc);
    cfg.field_0 = 0x103;
    cfg.field_4 = *(int *)(ride + 0xc4);
    cfg.field_8 = *param_4;
    p = blokes;
    blokes[0] = 0;
    for (i = 7; p = p + 1, i != 0; i = i - 1) {
        *p = 0;
    }
    count = 0;
    coords = GetScreenCoordsForObject((unsigned char *)param_4, (void *)ride);
    if (node != NULL) {
        short v = *param_4;
        p = blokes;
        do {
            if (v == (short)node[3]) {
                count = count + 1;
                *p = node[2];
                p = p + 1;
            }
            node = (int *)*node;
        } while (node != NULL);
        if (count != 0) {
            for (i = 0; i < count; i = i + 1) {
                if (*(char *)(blokes[i] + 0x37) == 1) IP_RenderBlokeIn3DNow((struct Bloke *)blokes[i]);
            }
            PrintSprite(RestMaskLevel1aaSprite, param_2, param_3, param_6, 0);
            for (i = 0; i < count; i = i + 1) {
                if (*(char *)(blokes[i] + 0x37) == 2) IP_RenderBlokeIn3DNow((struct Bloke *)blokes[i]);
            }
            PrintSprite(RestMaskLevel1Sprite, param_2, param_3, param_6, 0);
            for (i = 0; i < count; i = i + 1) {
                if (*(char *)(blokes[i] + 0x37) == 3) IP_RenderBlokeIn3DNow((struct Bloke *)blokes[i]);
            }
            PrintSprite(RestMaskLevel2Sprite, param_2, param_3, param_6, 0);
            for (i = 0; i < count; i = i + 1) {
                if (*(char *)(blokes[i] + 0x37) == 4) IP_RenderBlokeIn3DNow((struct Bloke *)blokes[i]);
            }
            PrintSprite(RestMaskLevel3Sprite, param_2, param_3, param_6, 0);
            for (i = 0; i < count; i = i + 1) {
                if (*(char *)(blokes[i] + 0x37) == 5) IP_RenderBlokeIn3DNow((struct Bloke *)blokes[i]);
            }
            PrintSprite(RestMaskMainSprite, param_2, param_3, param_6, 0);
        }
    }
    state = FUN_0042ef40((unsigned short *)param_4);
    if (state != NULL) {
        char frame = state->field_9 + 1;
        if (frame > 0xf) {
            frame = 0;
        }
        LLSSetFrame((struct LLS *)GetLLSForLayer(EateryFxInner, 1), frame);
        offset = GetRenderOffsetForLayer((struct LayerOffsetHolder *)EateryFxInner, 1);
        AdjustOffsetForViewMode(&offset);
        PrintSprite((struct Sprite *)GetSpriteForLayer((struct LayerContainer *)EateryFxInner, 1),
            coords.x + offset.x, coords.y + offset.y, param_6, (int *)&cfg);
        state->field_9 = frame;
    }
}

// FUNCTION: LEGOLAND 0x0042f720
void KillRestMaskSpritesAndMoneySFX(void) {
    KillSprite(RestMaskMainSprite);
    KillSprite(RestMaskLevel1aaSprite);
    KillSprite(RestMaskLevel1Sprite);
    KillSprite(RestMaskLevel2Sprite);
    KillSprite(RestMaskLevel3Sprite);
    KillMoneySFX();
}

// FUNCTION: LEGOLAND 0x0042f770
void LoadRestaurant2Resources(struct EateryObj *obj) {
    Load_FXList(OCTOPUS_SFX, 3);
    LoadMoneySFX();
    Restaurant2Ride = (unsigned int)obj->fx_c;
    ((struct EateryFX *)Restaurant2Ride)->flags_1c |= 0x420;
    EateryLayerOwner = (unsigned int)((struct EateryFX *)Restaurant2Ride)->inner_64;
    ((struct EateryInner *)EateryLayerOwner)->flags_10 |= 0x2000;
    // STRING: LEGOLAND 0x004b6f70
    R2FdoormSprite = LoadSprite("R2Fdoor_m.lls", 1);
    // STRING: LEGOLAND 0x004b6f60
    R2Fdoorm1Sprite = LoadSprite("R2Fdoor_m1.lls", 1);
    // STRING: LEGOLAND 0x004b6f50
    R2BdoormSprite = LoadSprite("R2Bdoor_m.lls", 1);
    // STRING: LEGOLAND 0x004b6f40
    R2TowermSprite = LoadSprite("R2Tower_m.lls", 1);
    HideLayer((struct LayerOwner *)EateryLayerOwner, 0);
    StopLayerPlaying(EateryLayerOwner, 0);
    LLSSetFrame((struct LLS *)GetLLSForLayer(EateryLayerOwner, 0), 0);
    HideLayer((struct LayerOwner *)EateryLayerOwner, 6);
    StopLayerPlaying(EateryLayerOwner, 6);
    LLSSetFrame((struct LLS *)GetLLSForLayer(EateryLayerOwner, 6), 0);
    HideLayer((struct LayerOwner *)EateryLayerOwner, 2);
    StopLayerPlaying(EateryLayerOwner, 2);
    LLSSetFrame((struct LLS *)GetLLSForLayer(EateryLayerOwner, 2), 0);
    HideLayer((struct LayerOwner *)EateryLayerOwner, 5);
    HideLayer((struct LayerOwner *)EateryLayerOwner, 1);
    StopLayerPlaying(EateryLayerOwner, 1);
    LLSSetFrame((struct LLS *)GetLLSForLayer(EateryLayerOwner, 1), 0);
    HideLayer((struct LayerOwner *)EateryLayerOwner, 3);
    StopLayerPlaying(EateryLayerOwner, 3);
    LLSSetFrame((struct LLS *)GetLLSForLayer(EateryLayerOwner, 3), 0);
}

// FUNCTION: LEGOLAND 0x0042f920
void FUN_0042f920(unsigned short *param_1) {
    struct SaveBlock *node = malloc(sizeof(struct SaveBlock));
    if (node != NULL) {
        unsigned int *p = (unsigned int *)node;
        int i;
        for (i = 0x10; i != 0; i = i - 1) {
            *p = 0;
            p = p + 1;
        }
        node->value = *param_1;
        node->next = SaveBlockList;
        node->field_6 = 0;
        node->field_7 = 0;
        node->field_8 = 0;
        node->field_9 = 0;
        node->field_c = 0;
        node->field_10 = 0;
        node->field_11 = 0;
        node->field_14 = 0;
        node->field_18 = 0;
        node->field_1c = 0;
        node->field_20 = 0;
        node->field_24 = 0;
        node->field_28 = 0;
        node->field_2c = 0;
        node->field_30 = 0;
        node->field_34 = 0;
        node->field_38 = 0x143;
        node->field_3c = 0;
        SaveBlockList = node;
    }
}

// FUNCTION: LEGOLAND 0x0042f9a0
void Restaurant2AddObject(unsigned int param_1, unsigned char *param_2) {
    unsigned char id[2];
    id[0] = param_2[0];
    id[1] = param_2[4];
    AddBasicObject(param_1, (unsigned int)param_2);
    FUN_0042f920((unsigned short *)id);
}

// FUNCTION: LEGOLAND 0x0042f9d0
struct SaveBlock *FindSaveBlock(unsigned short *param) {
    struct SaveBlock *node = SaveBlockList;
    if (node == NULL) {
        return NULL;
    }
    while (memcmp(&node->value, param, 2) != 0) {
        node = node->next;
        if (node == NULL) {
            return NULL;
        }
    }
    return node;
}

// FUNCTION: LEGOLAND 0x0042fa00
void RemoveSaveBlock(struct SaveBlock *param) {
    struct SaveBlock *node;
    struct SaveBlock *prev;

    if (SaveBlockList == param) {
        SaveBlockList = param->next;
    } else {
        node = SaveBlockList->next;
        prev = SaveBlockList;
        while (node != param) {
            prev = prev->next;
            if (prev == NULL) {
                break;
            }
            node = prev->next;
        }
        if (prev != NULL) {
            prev->next = param->next;
        }
    }
    free(param);
}

// FUNCTION: LEGOLAND 0x0042fa40
void Restaurant2RemoveObject(unsigned int arg1, TileId tile, unsigned int arg3, unsigned int arg4, unsigned int arg5) {
    struct SaveBlock *result = FindSaveBlock(&tile.id);
    if (result != NULL) {
        RemoveSaveBlock(result);
    }
    StandardRemoveObject(arg1, tile, arg3);
    RemoveAllBlokesFromRide((unsigned int)((struct EateryObj *)arg1)->fx_c, tile);
}

// FUNCTION: LEGOLAND 0x0042fa90
void FUN_0042fa90(struct RideNode *node, int dir, int idx) {
    if (dir == 1) {
        node->rider->pos.x += DAT_004b6860[idx] * -8;
        node->rider->pos.y += DAT_004b6860[idx] * -8;
    } else {
        node->rider->pos.x += DAT_004b68e0[idx] * 8;
        node->rider->pos.y += DAT_004b68e0[idx] * 8;
    }
}

// FUNCTION: LEGOLAND 0x0042fb00
void FUN_0042fb00(unsigned int param_1) {
    unsigned char *b = (unsigned char *)&param_1;
    struct SampleParams params;
    params.x = b[0];
    params.y = b[1];
    params.field_0 = 2;
    PlayInstanceOfSample(*(void **)(OCTOPUS_SFX + 8), 0, 1, &params);
    PlayInstanceOfSample(*(void **)(OCTOPUS_SFX + 0x14), 1, 1, &params);
}

// FUNCTION: LEGOLAND 0x0042fb60
void FUN_0042fb60(unsigned int param_1) {
    unsigned char *b = (unsigned char *)&param_1;
    struct SampleParams params;
    params.field_0 = 2;
    params.x = b[0];
    params.y = b[1];
    UnSourceAndFadeAllSamplesFromSource(&params, 0xffffff38);
    PlayInstanceOfSample(*(void **)(OCTOPUS_SFX + 0x20), 0, 1, &params);
}

// FUNCTION: LEGOLAND 0x0042fbb0
void Restaurant2Update(int param_1) {
    unsigned char *pos;
    char cv;
    int bloke;
    unsigned int *node;
    unsigned int *next;
    struct SaveBlock *st;
    char f7;
    unsigned char f8;
    int fc;
    unsigned char f10;
    unsigned char f11;
    int f14;
    int f18;
    int f1c;
    int f20;
    int f24;
    int f28;
    int f2c;
    int f30;
    int f38;
    int f3c;
    int x;
    int y;
    int ride = *(int *)(param_1 + 0xc);
    int t;
    node = *(unsigned int **)(ride + 0xcc);
    while (node != NULL) {
        next = (unsigned int *)*node;
        bloke = node[2];
        pos = (unsigned char *)(node + 3);
        st = FindSaveBlock((unsigned short *)pos);
        if (st == NULL) {
            return;
        }
        f7 = st->field_7;
        f8 = st->field_8;
        fc = st->field_c;
        f10 = st->field_10;
        f11 = st->field_11;
        f14 = st->field_14;
        f18 = st->field_18;
        f1c = st->field_1c;
        f20 = st->field_20;
        f24 = st->field_24;
        f28 = st->field_28;
        f2c = st->field_2c;
        f30 = st->field_30;
        f38 = st->field_38;
        f3c = st->field_3c;
        x = *(int *)(ride + 0xc) + (unsigned int)*pos;
        y = (unsigned int)*((unsigned char *)node + 0xd) + *(int *)(ride + 0x10);
        if (*(short *)(bloke + 0xe) == 0) {
            cv = *(char *)(bloke + 0x60);
            switch (cv) {
            case 0:
                *(unsigned char *)(bloke + 0x62) |= 8;
                *(int *)(bloke + 0x24) = x * 0x100 + 0x3c8;
                y = (y + -2) * 0x100;
                *(int *)(bloke + 0x28) = y;
                cv = CalcMoveLine(*(struct Point *)(bloke + 0x68), *(struct Point *)(bloke + 0x24), (struct Navigator *)(bloke + 0x98));
                *(short *)(bloke + 0xe) = 7;
                *(unsigned char *)(bloke + 0x73) = cv + 0x10;
                NewDirForAction(bloke, ((unsigned char)(cv + 0x10) >> 5) + 3);
                fc = fc + 1;
                *(int *)(bloke + 0x5c) = 300;
                *(unsigned short *)(bloke + 0x44) = (unsigned short)*(unsigned char *)(bloke + 0x7f);
                *(unsigned char *)(bloke + 0x7f) = 0x15;
                *(char *)(bloke + 0x60) += 1;
                break;
            case 1:
                *(int *)(bloke + 0x24) = x * 0x100 + 0x3c8;
                y = (y + -4) * 0x100 + fc * 100;
                *(int *)(bloke + 0x28) = y;
                cv = CalcMoveLine(*(struct Point *)(bloke + 0x68), *(struct Point *)(bloke + 0x24), (struct Navigator *)(bloke + 0x98));
                *(short *)(bloke + 0xe) = 7;
                *(unsigned char *)(bloke + 0x73) = cv + 0x10;
                NewDirForAction(bloke, ((unsigned char)(cv + 0x10) >> 5) + 3);
                *(char *)(bloke + 0x60) += 1;
                break;
            case 2:
                if (f10 < 3 && f20 != 0) {
                    x = x * 0x100 + 0x2ce;
                    *(int *)(bloke + 0x28) = y * 0x100 + -0x39c;
                    *(int *)(bloke + 0x24) = x;
                    cv = CalcMoveLine(*(struct Point *)(bloke + 0x68), *(struct Point *)(bloke + 0x24), (struct Navigator *)(bloke + 0x98));
                    *(short *)(bloke + 0xe) = 7;
                    *(unsigned char *)(bloke + 0x73) = cv + 0x10;
                    NewDirForAction(bloke, ((unsigned char)(cv + 0x10) >> 5) + 3);
                    f10 = f10 + 1;
                    *(char *)(bloke + 0x60) += 1;
                }
                break;
            case 3:
                *(int *)(bloke + 0x24) = x * 0x100 + 0x16a;
                y = y * 0x100 + -0x39c;
                *(int *)(bloke + 0x28) = y;
                cv = CalcMoveLine(*(struct Point *)(bloke + 0x68), *(struct Point *)(bloke + 0x24), (struct Navigator *)(bloke + 0x98));
                *(short *)(bloke + 0xe) = 7;
                *(unsigned char *)(bloke + 0x73) = cv + 0x10;
                NewDirForAction(bloke, ((unsigned char)(cv + 0x10) >> 5) + 3);
                *(char *)(bloke + 0x60) += 1;
                break;
            case 4:
                *(int *)(bloke + 0x24) = x * 0x100 + 0x16a;
                y = y * 0x100 + -0x532 + (unsigned int)f10 * 100;
                *(int *)(bloke + 0x28) = y;
                cv = CalcMoveLine(*(struct Point *)(bloke + 0x68), *(struct Point *)(bloke + 0x24), (struct Navigator *)(bloke + 0x98));
                *(short *)(bloke + 0xe) = 7;
                *(unsigned char *)(bloke + 0x73) = cv + 0x10;
                NewDirForAction(bloke, ((unsigned char)(cv + 0x10) >> 5) + 3);
                fc = fc + -1;
                f14 = f14 + 1;
                *(char *)(bloke + 0x60) += 1;
                if (f10 == 3 || fc == 0) {
                    f20 = 0;
                    f14 = 3;
                }
                break;
            case 5:
                if (2 < f14) {
                    f24 = 1;
                    f14 = 0;
                }
                SetPersonDirection(node[4], 5);
                *(char *)(bloke + 0x60) += 1;
                break;
            case 6:
                if (f18 == 2) {
                    if (f28 == 0) {
                        FUN_0042fa90((struct RideNode *)node, 1, (int)f7);
                    } else {
                        *(char *)(bloke + 0x60) = cv + 1;
                    }
                } else if (f28 != 0) {
                    *(char *)(bloke + 0x60) = cv + 1;
                }
                break;
            case 7:
                if (f2c != 0) {
                    *(int *)(bloke + 0x24) = x * 0x100 + -0x79c;
                    y = y * 0x100 + -0xc9c;
                    *(int *)(bloke + 0x28) = y;
                    cv = CalcMoveLine(*(struct Point *)(bloke + 0x68), *(struct Point *)(bloke + 0x24), (struct Navigator *)(bloke + 0x98));
                    *(short *)(bloke + 0xe) = 7;
                    *(unsigned char *)(bloke + 0x73) = cv + 0x10;
                    NewDirForAction(bloke, ((unsigned char)(cv + 0x10) >> 5) + 3);
                    *(char *)(bloke + 0x60) += 1;
                }
                break;
            case 8:
                *(int *)(bloke + 0x24) = (x + -10) * 0x100;
                y = y * 0x100 + -0xc9c;
                *(int *)(bloke + 0x28) = y;
                cv = CalcMoveLine(*(struct Point *)(bloke + 0x68), *(struct Point *)(bloke + 0x24), (struct Navigator *)(bloke + 0x98));
                *(short *)(bloke + 0xe) = 7;
                *(unsigned char *)(bloke + 0x73) = cv + 0x10;
                NewDirForAction(bloke, ((unsigned char)(cv + 0x10) >> 5) + 3);
                *(char *)(bloke + 0x60) += 1;
                break;
            case 9:
                if (f10 == 0) {
                    f2c = 0;
                }
                *(char *)(bloke + 0x60) = cv + 1;
                f10 = f10 - 1;
                break;
            case 10:
                t = *(int *)(bloke + 0x5c);
                y = t + -1;
                *(int *)(bloke + 0x5c) = y;
                if (t < 0) {
                    *(char *)(bloke + 0x60) = cv + 1;
                } else if (y == 0xfa) {
                    BuyItem(param_1, pos, 1);
                }
                break;
            case 11:
                if (f18 < 2 && f11 < 3) {
                    *(int *)(bloke + 0x68) = x * 0x100 + -0xa9c;
                    *(char *)(bloke + 0x60) = cv + 1;
                    *(unsigned int *)(bloke + 0x6c) = y * 0x100 + -0xd2c + (unsigned int)f11 * 100;
                    f11 = f11 + 1;
                }
                break;
            case 12:
                if (f11 == 0 || f18 != 0) {
                    if (f18 != 2) {
                        if (f28 != 0) {
                            *(char *)(bloke + 0x60) = cv + 1;
                        }
                    } else if (f28 == 0) {
                        FUN_0042fa90((struct RideNode *)node, 2, (int)f7);
                    } else {
                        *(char *)(bloke + 0x60) = cv + 1;
                    }
                } else if (fc == 0 && 100 < f1c++) {
                    f18 = 2;
                    f7 = 0;
                    f38 = 0x143;
                    f3c = 0;
                    f2c = 0;
                    f28 = 0;
                    FUN_0042fa90((struct RideNode *)node, 2, (int)f7);
                }
                break;
            case 13:
                if (f30 != 0) {
                    *(int *)(bloke + 0x24) = (x + -2) * 0x100;
                    y = y * 0x100 + -0x46a;
                    *(int *)(bloke + 0x28) = y;
                    cv = CalcMoveLine(*(struct Point *)(bloke + 0x68), *(struct Point *)(bloke + 0x24), (struct Navigator *)(bloke + 0x98));
                    *(short *)(bloke + 0xe) = 7;
                    *(unsigned char *)(bloke + 0x73) = cv + 0x10;
                    NewDirForAction(bloke, ((unsigned char)(cv + 0x10) >> 5) + 3);
                    *(char *)(bloke + 0x60) += 1;
                }
                break;
            case 14:
                *(int *)(bloke + 0x24) = (x + -3) * 0x100;
                y = y * 0x100 + -0x46a;
                *(int *)(bloke + 0x28) = y;
                cv = CalcMoveLine(*(struct Point *)(bloke + 0x68), *(struct Point *)(bloke + 0x24), (struct Navigator *)(bloke + 0x98));
                *(short *)(bloke + 0xe) = 7;
                *(unsigned char *)(bloke + 0x73) = cv + 0x10;
                NewDirForAction(bloke, ((unsigned char)(cv + 0x10) >> 5) + 3);
                f11 = f11 - 1;
                *(unsigned char *)(bloke + 0x7f) = *(unsigned char *)(bloke + 0x44);
                *(char *)(bloke + 0x60) += 1;
                break;
            case 15:
                RemoveBlokeFromRide((void *)ride, node);
                *(unsigned short *)(bloke + 0x62) &= 0xfff7;
            }
        }
        st->field_8 = f8;
        st->field_7 = f7;
        st->field_11 = f11;
        st->field_c = fc;
        st->field_1c = f1c;
        st->field_10 = f10;
        st->field_14 = f14;
        st->field_28 = f28;
        st->field_18 = f18;
        st->field_20 = f20;
        st->field_38 = f38;
        st->field_24 = f24;
        st->field_2c = f2c;
        st->field_30 = f30;
        st->field_3c = f3c;
        node = next;
    }
    {
        struct SaveBlock *v = SaveBlockList;
        unsigned char counter;
        char vf6;
        int vf18;
        char vf9;
        int vfc;
        char v8;
        char v11;
        int prev_fc = 0;
        while (v != NULL) {
            counter = v->field_9;
            v8 = v->field_8;
            vfc = v->field_c;
            vf18 = v->field_18;
            v11 = v->field_11;
            vf6 = v->field_6;
            f7 = v->field_7;
            vf9 = v->field_9;
            (void)vf9;
            f24 = v->field_24;
            f2c = v->field_2c;
            f1c = v->field_1c;
            f30 = v->field_30;
            f28 = v->field_28;
            f38 = v->field_38;
            f3c = v->field_3c;
            switch (v->field_18) {
            case 0:
                if (vfc != 0) {
                    if (counter < 9) {
                        counter = counter + 1;
                    } else {
                        vf18 = 1;
                        counter = 8;
                        f28 = 1;
                    }
                }
                break;
            case 1:
                if (f24 != 0) {
                    f28 = 0;
                    if ((char)counter < 0) {
                        vf6 = 0;
                        f24 = 0;
                        vf18 = 2;
                        f7 = 0;
                        f38 = 0x143;
                        f3c = 0;
                        FUN_0042fb00(v->value);
                    } else {
                        counter = counter - 1;
                    }
                }
                break;
            case 2:
                if (f7 < 0x21) {
                    f7 = f7 + 1;
                    /* [port] the original reads (&DAT_004b685c)[f7]: DAT_004b6860[f7 - 1], and DAT_004b68e0[0] for f7 == 0x21 */
                    f38 = f38 - (f7 <= 0x20 ? DAT_004b6860[f7 - 1] : DAT_004b68e0[0]);
                    f3c = f3c + DAT_004b68e0[f7];
                } else {
                    f30 = 1;
                    f1c = 0;
                    vf18 = 4;
                    vf6 = 0;
                    FUN_0042fb60(v->value);
                }
                break;
            case 3:
                if (vfc != 0) {
                    if (f7 < 0) {
                        f30 = 0;
                        vf18 = 0;
                        vfc = 0;
                        f7 = 0;
                        counter = 0;
                        vf6 = 0;
                        FUN_0042fb60(v->value);
                    } else {
                        f30 = 0;
                        f7 = f7 - 1;
                    }
                }
                break;
            case 4:
                if (vf6 < 9) {
                    vf6 = vf6 + 1;
                } else {
                    vf18 = 5;
                    f2c = 1;
                    f30 = 1;
                    vf6 = 8;
                }
                break;
            case 5:
                if (v8 == 0 && v11 == 0) {
                    f2c = 0;
                    if (vf6 < 0) {
                        vfc = 1;
                        vf18 = 3;
                        f7 = 0x20;
                        FUN_0042fb00(v->value);
                    } else {
                        vf6 = vf6 - 1;
                    }
                }
            }
            counter = counter + 1;
            if (0x1f < (char)counter) {
                counter = 0;
            }
            v->field_7 = f7;
            v->field_9 = counter;
            v->field_8 = v8;
            v->field_11 = v11;
            v->field_1c = f1c;
            v->field_30 = f30;
            v->field_24 = f24;
            v->field_2c = f2c;
            v->field_6 = vf6;
            v->field_18 = vf18;
            v->field_28 = f28;
            v->field_c = vfc;
            v->field_38 = f38;
            v->field_3c = f3c;
            prev_fc = vfc;
            (void)prev_fc;
            v = v->next;
        }
    }
}

// FUNCTION: LEGOLAND 0x004304a0
void *GetRestaurant2SpriteInfo(struct EateryObj *obj, unsigned short a2) {
    struct EateryFX *fx = obj->fx_c;
    struct EateryInner *inner = fx->inner_64;
    DAT_00616120.sprite = inner;
    DAT_00616120.x = fx->field_14;
    DAT_00616120.y = fx->field_18;
    DAT_00616120.id = a2;
    inner = fx->inner_64;
    inner->flags_10 |= 0x2000;
    return (void *)&DAT_00616120;
}

// FUNCTION: LEGOLAND 0x004304e0
void RenderEateryBaseLayers(unsigned short *param_1, int param_2, unsigned int param_3) {
    struct SaveBlock *state;
    struct Point coords;
    struct Point off;
    struct {
        /* 0x00 */ int field_0;
        /* 0x04 */ int field_4;
        /* 0x08 */ short field_8;
    } cfg;
    char c8;
    char c9;
    char c6;
    char c7;
    int sx;
    int sy;
    cfg.field_0 = 0x103;
    cfg.field_4 = *(int *)(param_2 + 0xc4);
    cfg.field_8 = *param_1;
    state = FindSaveBlock(param_1);
    if (state == NULL) {
        return;
    }
    coords = GetScreenCoordsForObject((unsigned char *)param_1, (void *)param_2);
    sy = coords.y;
    sx = coords.x;
    c8 = state->field_8;
    c9 = state->field_9;
    c6 = state->field_6;
    c7 = state->field_7;
    switch (state->field_18) {
    case 0:
        LLSSetFrame((struct LLS *)GetLLSForLayer(EateryLayerOwner, 1), c7);
        off = GetRenderOffsetForLayer((struct LayerOffsetHolder *)EateryLayerOwner, 1);
        AdjustOffsetForViewMode(&off);
        PrintSprite((struct Sprite *)GetSpriteForLayer((struct LayerContainer *)EateryLayerOwner, 1), off.x + sx, off.y + sy, param_3, (int *)&cfg);
        LLSSetFrame((struct LLS *)GetLLSForLayer(EateryLayerOwner, 3), c9);
        off = GetRenderOffsetForLayer((struct LayerOffsetHolder *)EateryLayerOwner, 3);
        AdjustOffsetForViewMode(&off);
        PrintSprite((struct Sprite *)GetSpriteForLayer((struct LayerContainer *)EateryLayerOwner, 3), off.x + sx, off.y + sy, param_3, (int *)&cfg);
        if (state->field_c == 0) {
            return;
        }
        goto set_l6;
    case 1:
        LLSSetFrame((struct LLS *)GetLLSForLayer(EateryLayerOwner, 1), c7);
        off = GetRenderOffsetForLayer((struct LayerOffsetHolder *)EateryLayerOwner, 1);
        AdjustOffsetForViewMode(&off);
        PrintSprite((struct Sprite *)GetSpriteForLayer((struct LayerContainer *)EateryLayerOwner, 1), off.x + sx, off.y + sy, param_3, (int *)&cfg);
        LLSSetFrame((struct LLS *)GetLLSForLayer(EateryLayerOwner, 3), c9);
        off = GetRenderOffsetForLayer((struct LayerOffsetHolder *)EateryLayerOwner, 3);
        AdjustOffsetForViewMode(&off);
        PrintSprite((struct Sprite *)GetSpriteForLayer((struct LayerContainer *)EateryLayerOwner, 3), off.x + sx, off.y + sy, param_3, (int *)&cfg);
    set_l6:
        LLSSetFrame((struct LLS *)GetLLSForLayer(EateryLayerOwner, 6), c8);
        return;
    case 2:
        LLSSetFrame((struct LLS *)GetLLSForLayer(EateryLayerOwner, 1), c7);
        off = GetRenderOffsetForLayer((struct LayerOffsetHolder *)EateryLayerOwner, 1);
        AdjustOffsetForViewMode(&off);
        PrintSprite((struct Sprite *)GetSpriteForLayer((struct LayerContainer *)EateryLayerOwner, 1), off.x + sx, off.y + sy, param_3, (int *)&cfg);
        return;
    case 4:
        LLSSetFrame((struct LLS *)GetLLSForLayer(EateryLayerOwner, 1), c7);
        off = GetRenderOffsetForLayer((struct LayerOffsetHolder *)EateryLayerOwner, 1);
        AdjustOffsetForViewMode(&off);
        PrintSprite((struct Sprite *)GetSpriteForLayer((struct LayerContainer *)EateryLayerOwner, 1), off.x + sx, off.y + sy, param_3, (int *)&cfg);
        LLSSetFrame((struct LLS *)GetLLSForLayer(EateryLayerOwner, 2), c6);
        off = GetRenderOffsetForLayer((struct LayerOffsetHolder *)EateryLayerOwner, 2);
        AdjustOffsetForViewMode(&off);
        PrintSprite((struct Sprite *)GetSpriteForLayer((struct LayerContainer *)EateryLayerOwner, 2), off.x + sx, off.y + sy, param_3, (int *)&cfg);
        LLSSetFrame((struct LLS *)GetLLSForLayer(EateryLayerOwner, 0), c6);
        off = GetRenderOffsetForLayer((struct LayerOffsetHolder *)EateryLayerOwner, 0);
        AdjustOffsetForViewMode(&off);
        PrintSprite((struct Sprite *)GetSpriteForLayer((struct LayerContainer *)EateryLayerOwner, 0), off.x + sx, off.y + sy, param_3, (int *)&cfg);
        return;
    case 5:
        LLSSetFrame((struct LLS *)GetLLSForLayer(EateryLayerOwner, 1), c7);
        off = GetRenderOffsetForLayer((struct LayerOffsetHolder *)EateryLayerOwner, 1);
        AdjustOffsetForViewMode(&off);
        PrintSprite((struct Sprite *)GetSpriteForLayer((struct LayerContainer *)EateryLayerOwner, 1), off.x + sx, off.y + sy, param_3, (int *)&cfg);
        LLSSetFrame((struct LLS *)GetLLSForLayer(EateryLayerOwner, 2), c6);
        off = GetRenderOffsetForLayer((struct LayerOffsetHolder *)EateryLayerOwner, 2);
        AdjustOffsetForViewMode(&off);
        PrintSprite((struct Sprite *)GetSpriteForLayer((struct LayerContainer *)EateryLayerOwner, 2), off.x + sx, off.y + sy, param_3, (int *)&cfg);
        LLSSetFrame((struct LLS *)GetLLSForLayer(EateryLayerOwner, 0), c6);
        off = GetRenderOffsetForLayer((struct LayerOffsetHolder *)EateryLayerOwner, 0);
        AdjustOffsetForViewMode(&off);
        PrintSprite((struct Sprite *)GetSpriteForLayer((struct LayerContainer *)EateryLayerOwner, 0), off.x + sx, off.y + sy, param_3, (int *)&cfg);
        return;
    case 3:
        LLSSetFrame((struct LLS *)GetLLSForLayer(EateryLayerOwner, 1), c7);
        off = GetRenderOffsetForLayer((struct LayerOffsetHolder *)EateryLayerOwner, 1);
        AdjustOffsetForViewMode(&off);
        PrintSprite((struct Sprite *)GetSpriteForLayer((struct LayerContainer *)EateryLayerOwner, 1), off.x + sx, off.y + sy, param_3, (int *)&cfg);
        LLSSetFrame((struct LLS *)GetLLSForLayer(EateryLayerOwner, 3), c9);
        off = GetRenderOffsetForLayer((struct LayerOffsetHolder *)EateryLayerOwner, 3);
        AdjustOffsetForViewMode(&off);
        PrintSprite((struct Sprite *)GetSpriteForLayer((struct LayerContainer *)EateryLayerOwner, 3), off.x + sx, off.y + sy, param_3, (int *)&cfg);
        return;
    }
}

// FUNCTION: LEGOLAND 0x00430b10
void RenderRestaurant2(int param_1, unsigned int param_2, unsigned int param_3, short *param_4, unsigned int param_5, unsigned int param_6) {
    int ride = *(int *)(param_1 + 0xc);
    int *node = *(int **)(ride + 0xcc);
    int *p;
    int i;
    int count;
    struct SaveBlock *state;
    char c9;
    int s18;
    unsigned int f11;
    int f38;
    int f3c;
    int blokes[30];
    struct Point coords;
    struct Point off;
    struct {
        /* 0x00 */ int field_0;
        /* 0x04 */ int field_4;
        /* 0x08 */ short field_8;
    } cfg;
    int sx;
    int sy;
    cfg.field_4 = param_1;
    cfg.field_8 = *param_4;
    p = blokes;
    blokes[0] = 0;
    for (i = 0x1d; p = p + 1, i != 0; i = i - 1) {
        *p = 0;
    }
    cfg.field_0 = 0x103;
    count = 0;
    state = FindSaveBlock(param_4);
    if (state == NULL) {
        return;
    }
    c9 = state->field_9;
    s18 = state->field_18;
    f11 = state->field_11;
    f38 = state->field_38;
    f3c = state->field_3c;
    RenderEateryBaseLayers((unsigned short *)param_4, ride, param_6);
    coords = GetScreenCoordsForObject((unsigned char *)param_4, (void *)ride);
    if (node == NULL) {
        return;
    }
    p = blokes;
    do {
        if (*param_4 == (short)node[3]) {
            count = count + 1;
            *p = node[2];
            p = p + 1;
        }
        node = (int *)*node;
    } while (node != NULL);
    if (count == 0) {
        return;
    }
    sx = coords.x;
    sy = coords.y;
    if (s18 == 0 || s18 == 1) {
        off = GetRenderOffsetForLayer((struct LayerOffsetHolder *)EateryLayerOwner, 5);
        AdjustOffsetForViewMode(&off);
        PrintSprite((struct Sprite *)GetSpriteForLayer((struct LayerContainer *)EateryLayerOwner, 5), off.x + sx, off.y + sy, param_6, (int *)&cfg);
        for (i = 0; i < count; i = i + 1) {
            if (*(char *)(blokes[i] + 0x60) == 5) IP_RenderBlokeIn3DNow((struct Bloke *)blokes[i]);
        }
        for (i = 0; i < count; i = i + 1) {
            if (*(char *)(blokes[i] + 0x60) == 6) IP_RenderBlokeIn3DNow((struct Bloke *)blokes[i]);
        }
        off = GetRenderOffsetForLayer((struct LayerOffsetHolder *)EateryLayerOwner, 6);
        AdjustOffsetForViewMode(&off);
        PrintSprite((struct Sprite *)GetSpriteForLayer((struct LayerContainer *)EateryLayerOwner, 6), off.x + sx, off.y + sy, param_6, (int *)&cfg);
        for (i = 0; i < count; i = i + 1) {
            if (*(char *)(blokes[i] + 0x60) == 4) IP_RenderBlokeIn3DNow((struct Bloke *)blokes[i]);
        }
        PrintSprite(R2Fdoorm1Sprite, off.x + sx, off.y + sy, param_6, (int *)&cfg);
    } else if (s18 == 2) {
        if (f11 != 0) {
            for (i = 0; i < count; i = i + 1) {
                if (*(char *)(blokes[i] + 0x60) == 0xc) IP_RenderBlokeIn3DNow((struct Bloke *)blokes[i]);
            }
            PrintSprite(R2BdoormSprite, sx, f3c / 2 + sy, param_6, (int *)&cfg);
        }
        PrintSprite(R2TowermSprite, sx, sy, param_6, (int *)&cfg);
        for (i = 0; i < count; i = i + 1) {
            if (*(char *)(blokes[i] + 0x60) == 6) IP_RenderBlokeIn3DNow((struct Bloke *)blokes[i]);
        }
        off = GetRenderOffsetForLayer((struct LayerOffsetHolder *)EateryLayerOwner, 6);
        AdjustOffsetForViewMode(&off);
        PrintSprite(R2FdoormSprite, off.x + sx, f38 / 2 + sy, param_6, (int *)&cfg);
        LLSSetFrame((struct LLS *)GetLLSForLayer(EateryLayerOwner, 3), c9);
        off = GetRenderOffsetForLayer((struct LayerOffsetHolder *)EateryLayerOwner, 3);
        AdjustOffsetForViewMode(&off);
        PrintSprite((struct Sprite *)GetSpriteForLayer((struct LayerContainer *)EateryLayerOwner, 3), off.x + sx, off.y + sy, param_6, (int *)&cfg);
    } else if (s18 == 5 || s18 == 4) {
        for (i = 0; i < count; i = i + 1) {
            if (*(char *)(blokes[i] + 0x60) == 7) IP_RenderBlokeIn3DNow((struct Bloke *)blokes[i]);
        }
        for (i = 0; i < count; i = i + 1) {
            if (*(char *)(blokes[i] + 0x60) == 8) IP_RenderBlokeIn3DNow((struct Bloke *)blokes[i]);
        }
        for (i = 0; i < count; i = i + 1) {
            if (*(char *)(blokes[i] + 0x60) == 9) IP_RenderBlokeIn3DNow((struct Bloke *)blokes[i]);
        }
        off = GetRenderOffsetForLayer((struct LayerOffsetHolder *)EateryLayerOwner, 0);
        AdjustOffsetForViewMode(&off);
        for (i = 0; i < count; i = i + 1) {
            if (*(char *)(blokes[i] + 0x60) == 0xd) IP_RenderBlokeIn3DNow((struct Bloke *)blokes[i]);
        }
        for (i = 0; i < count; i = i + 1) {
            if (*(char *)(blokes[i] + 0x60) == 0xe) IP_RenderBlokeIn3DNow((struct Bloke *)blokes[i]);
        }
        for (i = 0; i < count; i = i + 1) {
            if (*(char *)(blokes[i] + 0x60) == 0xf) IP_RenderBlokeIn3DNow((struct Bloke *)blokes[i]);
        }
        PrintSprite(R2BdoormSprite, sx, off.y + sy, param_6, (int *)&cfg);
        PrintSprite(R2TowermSprite, sx, sy, param_6, (int *)&cfg);
        off = GetRenderOffsetForLayer((struct LayerOffsetHolder *)EateryLayerOwner, 2);
        AdjustOffsetForViewMode(&off);
        PrintSprite(R2FdoormSprite, off.x + sx, off.y + sy, param_6, (int *)&cfg);
        LLSSetFrame((struct LLS *)GetLLSForLayer(EateryLayerOwner, 3), c9);
        off = GetRenderOffsetForLayer((struct LayerOffsetHolder *)EateryLayerOwner, 3);
        AdjustOffsetForViewMode(&off);
        PrintSprite((struct Sprite *)GetSpriteForLayer((struct LayerContainer *)EateryLayerOwner, 3), off.x + sx, off.y + sy, param_6, (int *)&cfg);
    }
    for (i = 0; i < count; i = i + 1) {
        if (*(char *)(blokes[i] + 0x60) == 0) IP_RenderBlokeIn3DNow((struct Bloke *)blokes[i]);
    }
    for (i = 0; i < count; i = i + 1) {
        if (*(char *)(blokes[i] + 0x60) == 1) IP_RenderBlokeIn3DNow((struct Bloke *)blokes[i]);
    }
    for (i = 0; i < count; i = i + 1) {
        if (*(char *)(blokes[i] + 0x60) == 2) IP_RenderBlokeIn3DNow((struct Bloke *)blokes[i]);
    }
    for (i = 0; i < count; i = i + 1) {
        if (*(char *)(blokes[i] + 0x60) == 3) IP_RenderBlokeIn3DNow((struct Bloke *)blokes[i]);
    }
}

// FUNCTION: LEGOLAND 0x00431120
void UnloadRestaurant2Resources(void) {
    Kill_FXList(OCTOPUS_SFX, 3);
    KillMoneySFX();
    KillSprite(R2FdoormSprite);
    KillSprite(R2BdoormSprite);
    KillSprite(R2TowermSprite);
    KillSprite(R2Fdoorm1Sprite);
}

// FUNCTION: LEGOLAND 0x00431170
void FoodcartIcecreamUpdate(Element *obj) {
    struct Ride *ride = obj->ride;
    struct RideNode *node = ride->riders;
    struct RideNode *next;
    struct Bloke *bloke;
    TileId *tile;
    int x;
    int y;
    char dir;

    while (node != NULL) {
        next = node->next;
        tile = &node->tile;
        bloke = node->rider;
        x = tile->pos.x + ride->x;
        y = tile->pos.y + ride->y;
        if (bloke->low_level_action == 0) {
            switch (bloke->param_action) {
            case 0:
                bloke->flags |= 8;
                bloke->dest.x = (x << 8) + 0x80;
                bloke->dest.y = (y << 8) - 0x80;
                dir = CalcMoveLine(bloke->pos, bloke->dest, &bloke->nav);
                bloke->low_level_action = 7;
                bloke->field_73 = dir + 0x10;
                NewDirForAction(bloke, ((unsigned char)(dir + 0x10) >> 5) + 3);
                bloke->param_action++;
                break;
            case 1:
                bloke->param_action++;
                bloke->field_58 = (rand() & 0x1f) + 4;
                break;
            case 2:
                if (bloke->field_58 == 0) {
                    bloke->param_action++;
                    BuyItem((struct BuyItemArg *)obj, tile, 1);
                }
                bloke->field_58--;
                break;
            case 3:
                bloke->dest.x = (x << 8) + 0x80;
                bloke->dest.y = (y << 8) + 0x80;
                dir = CalcMoveLine(bloke->pos, bloke->dest, &bloke->nav);
                bloke->low_level_action = 7;
                bloke->field_73 = dir + 0x10;
                NewDirForAction(bloke, ((unsigned char)(dir + 0x10) >> 5) + 3);
                bloke->param_action++;
                break;
            case 4:
                RemoveBlokeFromRide(ride, node);
                bloke->flags &= 0xfff7;
                break;
            }
        }
        node = next;
    }
}

// FUNCTION: LEGOLAND 0x004312c0
void EateryRemoveObject(unsigned int param_1, TileId tile, unsigned int param_3) {
    StandardRemoveObject(param_1, tile, param_3);
    RemoveAllBlokesFromRide(*(unsigned int *)(param_1 + 0xc), tile);
    StopMoneySFX(&tile.pos.x);
}

// FUNCTION: LEGOLAND 0x00431300
void LoadOctopusCafeResources(struct EateryObj *obj) {
    struct EateryInner *inner;
    OctopusCafeRide = obj->fx_c;
    OctopusCafeRide->flags_1c |= 0x20;
    inner = OctopusCafeRide->inner_64;
    inner->flags_10 |= 0x2000;
    // STRING: LEGOLAND 0x004b7100
    OctTentASprite = LoadSprite("OctTentA.lls", 1);
    // STRING: LEGOLAND 0x004b70f0
    OctTentBSprite = LoadSprite("OctTentB.lls", 1);
    // STRING: LEGOLAND 0x004b70e0
    OctTentCSprite = LoadSprite("OctTentC.lls", 1);
    // STRING: LEGOLAND 0x004b70d0
    OctTentDSprite = LoadSprite("OctTentD.lls", 1);
    // STRING: LEGOLAND 0x004b70c0
    OctTentESprite = LoadSprite("OctTentE.lls", 1);
    // STRING: LEGOLAND 0x004b70b0
    OctTentFSprite = LoadSprite("OctTentF.lls", 1);
    // STRING: LEGOLAND 0x004b70a0
    OctTentGSprite = LoadSprite("OctTentG.lls", 1);
    // STRING: LEGOLAND 0x004b7090
    OctTentHSprite = LoadSprite("OctTentH.lls", 1);
    // STRING: LEGOLAND 0x004b7080
    OctoKioskSprite = LoadSprite("OctoKiosk.lls", 1);
    // STRING: LEGOLAND 0x004b7070
    OctTabAASprite = LoadSprite("OctTabAA.lls", 1);
    // STRING: LEGOLAND 0x004b7060
    OctTabABSprite = LoadSprite("OctTabAB.lls", 1);
    // STRING: LEGOLAND 0x004b7050
    OctTabBASprite = LoadSprite("OctTabBA.lls", 1);
    // STRING: LEGOLAND 0x004b7040
    OctTabBBSprite = LoadSprite("OctTabBB.lls", 1);
    // STRING: LEGOLAND 0x004b7030
    OctTabCASprite = LoadSprite("OctTabCA.lls", 1);
    // STRING: LEGOLAND 0x004b7020
    OctTabCBSprite = LoadSprite("OctTabCB.lls", 1);
    // STRING: LEGOLAND 0x004b7010
    OctTabDASprite = LoadSprite("OctTabDA.lls", 1);
    // STRING: LEGOLAND 0x004b7000
    OctTabDBSprite = LoadSprite("OctTabDB.lls", 1);
    // STRING: LEGOLAND 0x004b6ff0
    OctTabEASprite = LoadSprite("OctTabEA.lls", 1);
    // STRING: LEGOLAND 0x004b6fe0
    OctTabEBSprite = LoadSprite("OctTabEB.lls", 1);
    // STRING: LEGOLAND 0x004b6fd0
    OctTabFASprite = LoadSprite("OctTabFA.lls", 1);
    // STRING: LEGOLAND 0x004b6fc0
    OctTabFBSprite = LoadSprite("OctTabFB.lls", 1);
    // STRING: LEGOLAND 0x004b6fb0
    OctTabGASprite = LoadSprite("OctTabGA.lls", 1);
    // STRING: LEGOLAND 0x004b6fa0
    OctTabGBSprite = LoadSprite("OctTabGB.lls", 1);
    // STRING: LEGOLAND 0x004b6f90
    OctTabHASprite = LoadSprite("OctTabHA.lls", 1);
    // STRING: LEGOLAND 0x004b6f80
    OctTabHBSprite = LoadSprite("OctTabHB.lls", 1);
    LoadMoneySFX();
}

// FUNCTION: LEGOLAND 0x004314f0
void OctopusCafeAddObject(unsigned int param_1, struct UserFlagsArg *param_2) {
    AddBasicObject(param_1, (unsigned int)param_2);
    Set_UserFlags(param_2->var_0, param_2->var_4, 0);
}

// FUNCTION: LEGOLAND 0x00431520
void UnloadOctopusCafeResources(void) {
    if (OctTentASprite) {
        KillSprite(OctTentASprite);
    }
    if (OctTentBSprite) {
        KillSprite(OctTentBSprite);
    }
    if (OctTentCSprite) {
        KillSprite(OctTentCSprite);
    }
    if (OctTentDSprite) {
        KillSprite(OctTentDSprite);
    }
    if (OctTentESprite) {
        KillSprite(OctTentESprite);
    }
    if (OctTentFSprite) {
        KillSprite(OctTentFSprite);
    }
    if (OctTentGSprite) {
        KillSprite(OctTentGSprite);
    }
    if (OctTentHSprite) {
        KillSprite(OctTentHSprite);
    }
    if (OctoKioskSprite) {
        KillSprite(OctoKioskSprite);
    }
    if (OctTabAASprite) {
        KillSprite(OctTabAASprite);
    }
    if (OctTabABSprite) {
        KillSprite(OctTabABSprite);
    }
    if (OctTabBASprite) {
        KillSprite(OctTabBASprite);
    }
    if (OctTabBBSprite) {
        KillSprite(OctTabBBSprite);
    }
    if (OctTabCASprite) {
        KillSprite(OctTabCASprite);
    }
    if (OctTabCBSprite) {
        KillSprite(OctTabCBSprite);
    }
    if (OctTabDASprite) {
        KillSprite(OctTabDASprite);
    }
    if (OctTabDBSprite) {
        KillSprite(OctTabDBSprite);
    }
    if (OctTabEASprite) {
        KillSprite(OctTabEASprite);
    }
    if (OctTabEBSprite) {
        KillSprite(OctTabEBSprite);
    }
    if (OctTabFASprite) {
        KillSprite(OctTabFASprite);
    }
    if (OctTabFBSprite) {
        KillSprite(OctTabFBSprite);
    }
    if (OctTabGASprite) {
        KillSprite(OctTabGASprite);
    }
    if (OctTabGBSprite) {
        KillSprite(OctTabGBSprite);
    }
    if (OctTabHASprite) {
        KillSprite(OctTabHASprite);
    }
    if (OctTabHBSprite) {
        KillSprite(OctTabHBSprite);
    }
    KillMoneySFX();
}

// FUNCTION: LEGOLAND 0x004316f0
void OctopusCafeUpdate(int param_1) {
    unsigned char *pos;
    int bloke;
    unsigned int *node;
    unsigned int *next;
    char cv;
    char state;
    int t;
    int u;
    int y;
    int f36;
    int ride = *(int *)(param_1 + 0xc);
    node = *(unsigned int **)(ride + 0xcc);
    while (node != NULL) {
        bloke = node[2];
        next = (unsigned int *)*node;
        pos = (unsigned char *)(node + 3);
        if (*(short *)(bloke + 0xe) == 0) {
            state = *(char *)(bloke + 0x60);
            switch (state) {
            case 0:
                cv = (char)Get_UserFlags((unsigned int)pos[0] << 8, (unsigned int)*((unsigned char *)node + 0xd) << 8);
                *(unsigned char *)(bloke + 0x62) |= 8;
                *(char *)(bloke + 0x36) = cv;
                *(short *)(bloke + 0x40) = 0;
                *(int *)(bloke + 0x5c) = 0x32;
                *(char *)(bloke + 0x60) += 1;
                Set_UserFlags((unsigned int)pos[0] << 8, (unsigned int)*((unsigned char *)node + 0xd) << 8, cv + 1 & 0x1f);
                break;
            case 1:
            case 2:
                if (state != 2 || (t = *(int *)(bloke + 0x5c), *(int *)(bloke + 0x5c) = t + -1, t < 0)) {
                    goto buy;
                }
                break;
            case 3:
            case 4:
            buy:
                BuyItem(param_1, pos, 1);
                f36 = *(unsigned char *)(bloke + 0x36);
                t = DAT_004b6a34[(unsigned int)*(unsigned char *)(bloke + 0x60) + (f36 >> 1) * 4];
                if (t != -1) {
                    goto move_to;
                }
                *(unsigned char *)(bloke + 0x60) += 1;
                break;
            case 5:
                t = *(int *)(bloke + 0x5c);
                *(int *)(bloke + 0x5c) = t + -1;
                if (t < 0) {
                    *(char *)(bloke + 0x60) = state + 1;
                }
                break;
            case 6:
                f36 = *(unsigned char *)(bloke + 0x36);
                u = DAT_004b6a44[(f36 >> 1) * 4];
                *(int *)(bloke + 0x68) = *(int *)(bloke + 0x24);
                *(int *)(bloke + 0x6c) = *(int *)(bloke + 0x28);
                t = DAT_004b6b38[DAT_004b6be8[f36] * 4 + 1];
                *(unsigned int *)(bloke + 0x24) = DAT_004b6990[u * 2] + (unsigned int)pos[0] * 0x100 + -0x80 + DAT_004b6b38[DAT_004b6be8[f36] * 4];
                y = (unsigned int)*((unsigned char *)node + 0xd) * 0x100 + DAT_004b6990[u * 2 + 1] + 0x80 + t;
                *(int *)(bloke + 0x28) = y;
                cv = CalcMoveLine(*(struct Point *)(bloke + 0x68), *(struct Point *)(bloke + 0x24), (struct Navigator *)(bloke + 0x98));
                *(unsigned char *)(bloke + 0x73) = cv + 0x10;
                goto setdir;
            case 7:
                NewDirForAction(bloke, (*(char *)(bloke + 0x72) + -4) & 7);
                *(char *)(bloke + 0x60) += 1;
                break;
            case 8:
                *(int *)(bloke + 0x68) = *(int *)(bloke + 0x24);
                *(int *)(bloke + 0x6c) = *(int *)(bloke + 0x28);
                *(short *)(bloke + 0x40) = 1;
                f36 = *(unsigned char *)(bloke + 0x36);
                t = DAT_004b6b38[DAT_004b6be8[f36] * 4 + 3];
                u = DAT_004b6a44[(f36 >> 1) * 4];
                *(unsigned int *)(bloke + 0x24) = DAT_004b6990[u * 2] + (unsigned int)pos[0] * 0x100 + -0x80 + DAT_004b6b38[DAT_004b6be8[f36] * 4 + 2];
                y = (unsigned int)*((unsigned char *)node + 0xd) * 0x100 + DAT_004b6990[u * 2 + 1] + 0x80 + t;
                *(int *)(bloke + 0x28) = y;
                cv = CalcMoveLine(*(struct Point *)(bloke + 0x68), *(struct Point *)(bloke + 0x24), (struct Navigator *)(bloke + 0x98));
                *(char *)(bloke + 0x73) = cv + 0x10;
                *(short *)(bloke + 0xe) = 7;
                *(char *)(bloke + 0x60) += 1;
                *(int *)(bloke + 0x5c) = 200;
                break;
            case 9:
                *(unsigned char *)(bloke + 0x63) |= 1;
                *(int *)(bloke + 0x68) = *(int *)(bloke + 0x24);
                *(int *)(bloke + 0x6c) = *(int *)(bloke + 0x28);
                *(short *)(bloke + 0x70) = 10;
                BlokeSitAnim(bloke);
                BlokeSetFrame(bloke, 0);
                *(char *)(bloke + 0x60) += 1;
                break;
            case 10:
                t = *(int *)(bloke + 0x5c);
                *(int *)(bloke + 0x5c) = t + -1;
                if (t < 0) {
                    *(char *)(bloke + 0x60) = state + 1;
                }
                break;
            case 11:
                *(unsigned short *)(bloke + 0x62) &= 0xfeff;
                *(short *)(bloke + 0x70) = 0;
                BlokeWalkAnim((struct Bloke *)bloke);
                f36 = *(unsigned char *)(bloke + 0x36);
                t = DAT_004b6b38[DAT_004b6be8[f36] * 4 + 1];
                u = DAT_004b6a44[(f36 >> 1) * 4];
                *(unsigned int *)(bloke + 0x24) = DAT_004b6990[u * 2] + (unsigned int)pos[0] * 0x100 + -0x80 + DAT_004b6b38[DAT_004b6be8[f36] * 4];
                y = (unsigned int)*((unsigned char *)node + 0xd) * 0x100 + DAT_004b6990[u * 2 + 1] + 0x80 + t;
                *(int *)(bloke + 0x28) = y;
                cv = CalcMoveLine(*(struct Point *)(bloke + 0x68), *(struct Point *)(bloke + 0x24), (struct Navigator *)(bloke + 0x98));
                *(char *)(bloke + 0x73) = cv + 0x10;
                *(short *)(bloke + 0xe) = 7;
                *(char *)(bloke + 0x60) += 1;
                break;
            case 12:
                *(short *)(bloke + 0x40) = 0;
                f36 = *(unsigned char *)(bloke + 0x36);
                t = DAT_004b6a44[(f36 >> 1) * 4];
            move_to:
                *(unsigned int *)(bloke + 0x24) = DAT_004b6990[t * 2] + -0x80 + (unsigned int)pos[0] * 0x100;
                y = (unsigned int)*((unsigned char *)node + 0xd) * 0x100 + 0x80 + DAT_004b6990[t * 2 + 1];
                *(int *)(bloke + 0x28) = y;
                cv = CalcMoveLine(*(struct Point *)(bloke + 0x68), *(struct Point *)(bloke + 0x24), (struct Navigator *)(bloke + 0x98));
                *(unsigned char *)(bloke + 0x73) = cv + 0x10;
            setdir:
                *(short *)(bloke + 0xe) = 7;
                NewDirForAction(bloke, ((unsigned char)(*(unsigned char *)(bloke + 0x73)) >> 5) + 3);
                *(char *)(bloke + 0x60) += 1;
                break;
            case 13:
            case 14:
            case 15:
            case 16:
                do {
                    t = DAT_004b6a78[(*(unsigned char *)(bloke + 0x36) >> 1) * 4 - (unsigned int)*(unsigned char *)(bloke + 0x60)];
                    *(unsigned char *)(bloke + 0x60) += 1;
                } while (t == -1);
                *(unsigned int *)(bloke + 0x24) = DAT_004b6990[t * 2] + -0x80 + (unsigned int)pos[0] * 0x100;
                y = (unsigned int)*((unsigned char *)node + 0xd) * 0x100 + 0x80 + DAT_004b6990[t * 2 + 1];
                *(int *)(bloke + 0x28) = y;
                cv = CalcMoveLine(*(struct Point *)(bloke + 0x68), *(struct Point *)(bloke + 0x24), (struct Navigator *)(bloke + 0x98));
                *(short *)(bloke + 0xe) = 7;
                *(unsigned char *)(bloke + 0x73) = cv + 0x10;
                NewDirForAction(bloke, ((unsigned char)(cv + 0x10) >> 5) + 3);
                break;
            case 17:
                BlokeWalkAnim((struct Bloke *)bloke);
                *(unsigned int *)(bloke + 0x24) = (*(int *)(ride + 0xc) + (unsigned int)pos[0]) * 0x100 + 0x80;
                y = ((unsigned int)*((unsigned char *)node + 0xd) + *(int *)(ride + 0x10)) * 0x100 + 0x80;
                *(int *)(bloke + 0x28) = y;
                cv = CalcMoveLine(*(struct Point *)(bloke + 0x68), *(struct Point *)(bloke + 0x24), (struct Navigator *)(bloke + 0x98));
                *(short *)(bloke + 0xe) = 7;
                *(unsigned char *)(bloke + 0x73) = cv + 0x10;
                NewDirForAction(bloke, ((unsigned char)(cv + 0x10) >> 5) + 3);
                *(char *)(bloke + 0x60) += 1;
                break;
            case 18:
                RemoveBlokeFromRide((void *)ride, node);
                *(unsigned short *)(bloke + 0x62) &= 0xfff7;
            }
        }
        node = next;
    }
}

// FUNCTION: LEGOLAND 0x00431c50
void FUN_00431c50(unsigned int param_1, unsigned int param_2, struct Sprite *param_3, struct Sprite *param_4, struct Sprite *param_5, int param_6, int param_7, int param_8, int param_9, unsigned int param_10) {
    PrintSprite(param_3, param_1, param_2, param_10, 0);
    if (param_6 != 0 || param_7 != 0) {
        if (param_6 != 0) {
            IP_RenderBlokeIn3DNow((struct Bloke *)param_6);
        }
        if (param_7 != 0) {
            IP_RenderBlokeIn3DNow((struct Bloke *)param_7);
        }
        PrintSprite(param_4, param_1, param_2, param_10, 0);
    }
    if (param_8 != 0 || param_9 != 0) {
        if (param_8 != 0) {
            IP_RenderBlokeIn3DNow((struct Bloke *)param_8);
        }
        if (param_9 != 0) {
            IP_RenderBlokeIn3DNow((struct Bloke *)param_9);
        }
        PrintSprite(param_5, param_1, param_2, param_10, 0);
    }
}

// FUNCTION: LEGOLAND 0x00431d00
void RenderOctopusCafe(int param_1, unsigned int param_2, unsigned int param_3, unsigned char *param_4, unsigned int param_5, unsigned int param_6) {
    int buckets[0x20];
    int blokes[0x20];
    int count;
    int i;
    int *p;
    struct Bloke *bloke;
    struct RideNode *node;
    TileId *tile = (TileId *)param_4;
    p = buckets;
    buckets[0] = 0;
    for (i = 0x1f; p = p + 1, i != 0; i = i - 1) {
        *p = 0;
    }
    count = 0;
    node = ((struct Ride *)*(int *)(param_1 + 0xc))->riders;
    while (node != NULL) {
        if (tile->id == node->tile.id) {
            bloke = node->rider;
            if (bloke->field_40 != 0) {
                buckets[bloke->field_36] = (int)bloke;
            } else {
                blokes[count] = (int)bloke;
                count = count + 1;
                bloke->field_37 = DAT_004b6d58[(((bloke->pos.x >> 8) - tile->pos.x) * 0xb - (bloke->pos.y >> 8) + tile->pos.y) * 4];
            }
        }
        node = node->next;
    }
    for (i = 0; i < count; i = i + 1) {
        if (*(char *)(blokes[i] + 0x37) == 1) IP_RenderBlokeIn3DNow((struct Bloke *)blokes[i]);
    }
    for (i = 0; i < count; i = i + 1) {
        if (*(char *)(blokes[i] + 0x37) == 2) IP_RenderBlokeIn3DNow((struct Bloke *)blokes[i]);
    }
    FUN_00431c50(param_2, param_3, OctTentESprite, OctTabEASprite, OctTabEBSprite, buckets[14], buckets[15], buckets[12], buckets[13], param_6);
    for (i = 0; i < count; i = i + 1) {
        if (*(char *)(blokes[i] + 0x37) == 3) IP_RenderBlokeIn3DNow((struct Bloke *)blokes[i]);
    }
    for (i = 0; i < count; i = i + 1) {
        if (*(char *)(blokes[i] + 0x37) == 4) IP_RenderBlokeIn3DNow((struct Bloke *)blokes[i]);
    }
    FUN_00431c50(param_2, param_3, OctTentFSprite, OctTabFASprite, OctTabFBSprite, buckets[17], buckets[18], buckets[16], buckets[19], param_6);
    for (i = 0; i < count; i = i + 1) {
        if (*(char *)(blokes[i] + 0x37) == 5) IP_RenderBlokeIn3DNow((struct Bloke *)blokes[i]);
    }
    for (i = 0; i < count; i = i + 1) {
        if (*(char *)(blokes[i] + 0x37) == 6) IP_RenderBlokeIn3DNow((struct Bloke *)blokes[i]);
    }
    FUN_00431c50(param_2, param_3, OctTentDSprite, OctTabDASprite, OctTabDBSprite, buckets[10], buckets[11], buckets[8], buckets[9], param_6);
    for (i = 0; i < count; i = i + 1) {
        if (*(char *)(blokes[i] + 0x37) == 7) IP_RenderBlokeIn3DNow((struct Bloke *)blokes[i]);
    }
    FUN_00431c50(param_2, param_3, OctTentCSprite, OctTabCASprite, OctTabCBSprite, buckets[4], buckets[7], buckets[5], buckets[6], param_6);
    for (i = 0; i < count; i = i + 1) {
        if (*(char *)(blokes[i] + 0x37) == 8) IP_RenderBlokeIn3DNow((struct Bloke *)blokes[i]);
    }
    for (i = 0; i < count; i = i + 1) {
        if (*(char *)(blokes[i] + 0x37) == 9) IP_RenderBlokeIn3DNow((struct Bloke *)blokes[i]);
    }
    for (i = 0; i < count; i = i + 1) {
        if (*(char *)(blokes[i] + 0x37) == 10) IP_RenderBlokeIn3DNow((struct Bloke *)blokes[i]);
    }
    PrintSprite(OctoKioskSprite, param_2, param_3, param_6, 0);
    for (i = 0; i < count; i = i + 1) {
        if (*(char *)(blokes[i] + 0x37) == 0xb) IP_RenderBlokeIn3DNow((struct Bloke *)blokes[i]);
    }
    for (i = 0; i < count; i = i + 1) {
        if (*(char *)(blokes[i] + 0x37) == 0xc) IP_RenderBlokeIn3DNow((struct Bloke *)blokes[i]);
    }
    FUN_00431c50(param_2, param_3, OctTentGSprite, OctTabGASprite, OctTabGBSprite, buckets[21], buckets[22], buckets[20], buckets[23], param_6);
    for (i = 0; i < count; i = i + 1) {
        if (*(char *)(blokes[i] + 0x37) == 0xd) IP_RenderBlokeIn3DNow((struct Bloke *)blokes[i]);
    }
    for (i = 0; i < count; i = i + 1) {
        if (*(char *)(blokes[i] + 0x37) == 0xe) IP_RenderBlokeIn3DNow((struct Bloke *)blokes[i]);
    }
    FUN_00431c50(param_2, param_3, OctTentHSprite, OctTabHASprite, OctTabHBSprite, buckets[24], buckets[25], buckets[26], buckets[27], param_6);
    for (i = 0; i < count; i = i + 1) {
        if (*(char *)(blokes[i] + 0x37) == 0xf) IP_RenderBlokeIn3DNow((struct Bloke *)blokes[i]);
    }
    for (i = 0; i < count; i = i + 1) {
        if (*(char *)(blokes[i] + 0x37) == 0x10) IP_RenderBlokeIn3DNow((struct Bloke *)blokes[i]);
    }
    FUN_00431c50(param_2, param_3, OctTentBSprite, OctTabBASprite, OctTabBBSprite, buckets[0], buckets[3], buckets[1], buckets[2], param_6);
    for (i = 0; i < count; i = i + 1) {
        if (*(char *)(blokes[i] + 0x37) == 0x11) IP_RenderBlokeIn3DNow((struct Bloke *)blokes[i]);
    }
    for (i = 0; i < count; i = i + 1) {
        if (*(char *)(blokes[i] + 0x37) == 0x12) IP_RenderBlokeIn3DNow((struct Bloke *)blokes[i]);
    }
    FUN_00431c50(param_2, param_3, OctTentASprite, OctTabAASprite, OctTabABSprite, buckets[28], buckets[29], buckets[30], buckets[31], param_6);
    for (i = 0; i < count; i = i + 1) {
        if (*(char *)(blokes[i] + 0x37) == 0x13) IP_RenderBlokeIn3DNow((struct Bloke *)blokes[i]);
    }
}

// FUNCTION: LEGOLAND 0x004322a0
int Restaurant1_Save(void) {
    unsigned int one = 1;
    unsigned int zero = 0;
    struct BrollyNode *node = DAT_00616144;
    while (node != NULL) {
        if (SaveGameWrite(&one, 4) == 0) {
            return 0;
        }
        if (SaveGameWrite(node, 0xc) == 0) {
            return 0;
        }
        node = node->next;
    }
    return SaveGameWrite(&zero, 4) != 0;
}

// FUNCTION: LEGOLAND 0x00432310
int Restaurant1_Load(void) {
    unsigned int count;
    struct BrollyNode *prev = NULL;
    struct BrollyNode *node;
    if (!SaveGameRead(&count, 4)) {
        return 0;
    }
    if (count == 0) {
        return 1;
    }
    while (count != 0) {
        node = malloc(sizeof(struct BrollyNode));
        if (!SaveGameRead(node, 0xc)) {
            return 0;
        }
        node->next = NULL;
        if (prev != NULL) {
            prev->next = node;
        } else {
            DAT_00616144 = node;
        }
        prev = node;
        if (!SaveGameRead(&count, 4)) {
            return 0;
        }
    }
    return 1;
}

// FUNCTION: LEGOLAND 0x00432390
int Restaurant2_Save(void) {
    unsigned int one = 1;
    unsigned int zero = 0;
    struct SaveBlock *node = SaveBlockList;
    while (node != NULL) {
        if (SaveGameWrite(&one, 4) == 0) {
            return 0;
        }
        if (SaveGameWrite(node, 0x40) == 0) {
            return 0;
        }
        node = node->next;
    }
    return SaveGameWrite(&zero, 4) != 0;
}

// FUNCTION: LEGOLAND 0x00432400
int Restaurant2_Load(void) {
    unsigned int count;
    struct SaveBlock *current;
    struct SaveBlock *prev = NULL;

    if (!SaveGameRead(&count, 4)) {
        return 0;
    }
    if (count == 0) {
        return 1;
    }
    while (count != 0) {
        current = malloc(64);
        if (!SaveGameRead(current, 64)) {
            return 0;
        }
        current->next = 0;
        if (prev != NULL) {
            prev->next = current;
        } else {
            SaveBlockList = current;
        }
        prev = current;
        if (!SaveGameRead(&count, 4)) {
            return 0;
        }
    }
    return 1;
}
