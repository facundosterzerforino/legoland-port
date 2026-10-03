#include "legoland.h"

#include <string.h>
#include "gamemap.h"
#include "garden.h"
#include "globals.h"
#include "llidb.h"
#include "map_object.h"
#include "objclass.h"

#pragma intrinsic(strcmp)

struct GardenLayer {
    unsigned char pad_0[0xc];
    struct GardenInner *field_c;
};

struct GardenInner {
    unsigned char pad_0[0x1c];
    unsigned int field_1c;
};

struct EditTarget {
    unsigned char pad_0[0x3c];
    unsigned int field_3c;
};

struct GardenTable {
    unsigned char pad_0[8];
    int *a;
    int *b;
    int *c;
};

// FUNCTION: LEGOLAND 0x00432480
void LoadHedgeImages(struct GardenLayer *arg0) {
    struct GardenInner *temp = arg0->field_c;
    HedgeObjectClass = temp;
    temp->field_1c |= 0x404;
    // STRING: LEGOLAND 0x004b7114
    if (LLIDB_FindElement("HEDGE IMAGES", &HedgeImagesHandle, 0) != 0) {
        return;
    }
    HedgeImagesData = (unsigned int)LLIDB_LoadData((void *)HedgeImagesHandle); /* TODO: fold — LLIDB_LoadData handle stored as uint global */
}

// FUNCTION: LEGOLAND 0x004324c0
void UnloadHedgeImages(void) {
    LLIDB_UnLoadData(HedgeImagesHandle);
}

// FUNCTION: LEGOLAND 0x004324d0
void FUN_004324d0(void) {
    void *var = HedgeObjectClass;
    EditMode.unk0 = 1;
    EditMode.unk8 = var;
    DefaultCursor(&EditCursor);
    SetEditCursorFootPrint(&((struct EditTarget *)EditMode.unk8)->field_3c);
}

// FUNCTION: LEGOLAND 0x00432510
void UpdateHedgeTileImage(int x, int y) {
    int pos[2];
    int mask = 0;

    pos[0] = x;
    pos[1] = y - 1;
    if (GetObjectClassAndInstance(pos, 0) == (unsigned int)HedgeObjectClass) {
        mask = 1;
    }
    pos[0] = x + 1;
    pos[1] = y;
    if (GetObjectClassAndInstance(pos, 0) == (unsigned int)HedgeObjectClass) {
        mask |= 2;
    }
    pos[0] = x;
    pos[1] = y + 1;
    if (GetObjectClassAndInstance(pos, 0) == (unsigned int)HedgeObjectClass) {
        mask |= 4;
    }
    pos[0] = x - 1;
    pos[1] = y;
    if (GetObjectClassAndInstance(pos, 0) == (unsigned int)HedgeObjectClass) {
        mask |= 8;
    }
    if (mask == 0) {
        Set_UserFlags(x << 8, y << 8, 0xe);
    } else {
        Set_UserFlags(x << 8, y << 8, mask - 1);
    }
}

// FUNCTION: LEGOLAND 0x004325e0
void HedgeAddObject(Element *obj, int *param_2) {
    TileId packed;
    int pos[2];
    packed.pos.x = param_2[0];
    packed.pos.y = param_2[1];
    AddObjectToMap(obj, packed, 0);
    UpdateHedgeTileImage(param_2[0], param_2[1]);
    pos[0] = param_2[0];
    pos[1] = param_2[1] - 1;
    if (GetObjectClassAndInstance(pos, 0) == (unsigned int)HedgeObjectClass) {
        UpdateHedgeTileImage(pos[0], pos[1]);
    }
    pos[0] = param_2[0] + 1;
    pos[1] = param_2[1];
    if (GetObjectClassAndInstance(pos, 0) == (unsigned int)HedgeObjectClass) {
        UpdateHedgeTileImage(pos[0], pos[1]);
    }
    pos[0] = param_2[0];
    pos[1] = param_2[1] + 1;
    if (GetObjectClassAndInstance(pos, 0) == (unsigned int)HedgeObjectClass) {
        UpdateHedgeTileImage(pos[0], pos[1]);
    }
    pos[0] = param_2[0] - 1;
    pos[1] = param_2[1];
    if (GetObjectClassAndInstance(pos, 0) == (unsigned int)HedgeObjectClass) {
        UpdateHedgeTileImage(pos[0], pos[1]);
    }
}

// FUNCTION: LEGOLAND 0x00432700
void HedgeRemoveObject(Element *obj, TileId tile, struct Cursor *cursor) {
    int pos[2];
    int x = tile.pos.x;
    int y = tile.pos.y;

    StandardRemoveObject(obj, tile, cursor);
    pos[0] = x;
    pos[1] = y - 1;
    if (GetObjectClassAndInstance(pos, 0) == (unsigned int)HedgeObjectClass) {
        UpdateHedgeTileImage(pos[0], pos[1]);
    }
    pos[0] = x + 1;
    pos[1] = y;
    if (GetObjectClassAndInstance(pos, 0) == (unsigned int)HedgeObjectClass) {
        UpdateHedgeTileImage(pos[0], pos[1]);
    }
    pos[0] = x;
    pos[1] = y + 1;
    if (GetObjectClassAndInstance(pos, 0) == (unsigned int)HedgeObjectClass) {
        UpdateHedgeTileImage(pos[0], pos[1]);
    }
    pos[0] = x - 1;
    pos[1] = y;
    if (GetObjectClassAndInstance(pos, 0) == (unsigned int)HedgeObjectClass) {
        UpdateHedgeTileImage(pos[0], pos[1]);
    }
}

// FUNCTION: LEGOLAND 0x00432810
struct RideSpriteInfo *GetHedgeSpriteInfo(int unused, TileId tile) {
    struct GardenTable *t;
    int i;

    i = Get_UserFlags(tile.pos.x << 8, tile.pos.y << 8) & 0xffff;
    t = (struct GardenTable *)HedgeImagesData;
    RideSpriteInfoBuffer.sprite = (void *)t->a[(unsigned char)i];
    RideSpriteInfoBuffer.x = t->b[(unsigned char)i] >> 1;
    RideSpriteInfoBuffer.y = t->c[(unsigned char)i] >> 1;
    return &RideSpriteInfoBuffer;
}

// FUNCTION: LEGOLAND 0x00432870
void LoadFlowerImages(struct GardenLayer *param) {
    struct GardenInner *temp = param->field_c;
    DAT_0081cd04 = temp;
    temp->field_1c |= 0x404;
    // STRING: LEGOLAND 0x004b7124
    if (LLIDB_FindElement("FLOWERS 1", &FlowerImagesHandle, 0) != 0) {
        return;
    }
    FlowerImagesData = (unsigned int)LLIDB_LoadData((void *)FlowerImagesHandle);
}

// FUNCTION: LEGOLAND 0x004328b0
void UnloadFlowerImages(void) {
    LLIDB_UnLoadData(FlowerImagesHandle);
}

// FUNCTION: LEGOLAND 0x004328c0
void FlowersSetEditMode(void) {
    void *var = DAT_0081cd04;
    EditMode.unk0 = 1;
    EditMode.unk8 = var;
    DefaultCursor(&EditCursor);
    SetEditCursorFootPrint(&((struct EditTarget *)EditMode.unk8)->field_3c);
}

// FUNCTION: LEGOLAND 0x00432900
void FlowersAddObject(int param_1, int *param_2) {
    TileId packed;
    packed.pos.x = param_2[0];
    packed.pos.y = param_2[1];
    AddObjectToMap(param_1, packed, 0);
    if (FlowerImagesData != 0) {
        int r = rand();
        Set_UserFlags(param_2[0] << 8, param_2[1] << 8, (unsigned short)(r % *(int *)(FlowerImagesData + 4)));
    }
}

// FUNCTION: LEGOLAND 0x00432960
struct RideSpriteInfo *GetFlowerSpriteInfo(int unused, TileId tile) {
    struct GardenTable *t;
    int i;

    i = Get_UserFlags(tile.pos.x << 8, tile.pos.y << 8) & 0xffff;
    t = (struct GardenTable *)FlowerImagesData;
    RideSpriteInfoBuffer.sprite = (void *)t->a[(unsigned char)i];
    RideSpriteInfoBuffer.x = t->b[(unsigned char)i] >> 1;
    RideSpriteInfoBuffer.y = t->c[(unsigned char)i] >> 1;
    return &RideSpriteInfoBuffer;
}

// FUNCTION: LEGOLAND 0x004329c0
void GardenGetInterfaces(struct ClassNode *head, struct CallbackTable *iface) {
    // STRING: LEGOLAND 0x004b7138
    if (strcmp(head->name, "HEDGE") == 0) {
        iface->cb_a4 = LoadHedgeImages;
        iface->cb_8c = FUN_004324d0;
        iface->cb_98 = HedgeAddObject;
        iface->cb_9c = HedgeRemoveObject;
        iface->cb_a0 = GetHedgeSpriteInfo;
        iface->cb_ac = UnloadHedgeImages;
        return;
    }
    // STRING: LEGOLAND 0x004b7130
    if (strcmp(head->name, "FLOWERS") == 0) {
        iface->cb_a4 = LoadFlowerImages;
        iface->cb_8c = FlowersSetEditMode;
        iface->cb_98 = FlowersAddObject;
        iface->cb_a0 = GetFlowerSpriteInfo;
        iface->cb_ac = UnloadFlowerImages;
    }
}
