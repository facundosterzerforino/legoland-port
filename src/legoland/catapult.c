#include <stdlib.h>
#include <string.h>
#include "legoland.h"

#include "bloke.h"
#include "catapult.h"
#include "gamemap.h"
#include "globals.h"
#include "llidb.h"
#include "man3d.h"
#include "map_object.h"
#include "obj_instance.h"
#include "objclass.h"
#include "print_sprite.h"
#include "render3d.h"
#include "sound_music.h"

struct CatapultNode {
    unsigned short tile_id;
    unsigned char pad_2[2];
    struct CatapultNode *next;
    void *field_8;
    signed char mode;
    unsigned char pad_d[3];
    struct RideNode *slots[4];
    unsigned char active[4];
    signed char frame[4];
    unsigned int flags[4];
};

struct CatapultItem {
    unsigned char field_0;
    unsigned char field_1;
    unsigned char pad_2[6];
    unsigned int field_8;
    signed char field_c;
    signed char field_d;
    unsigned char pad_e[0x12];
    unsigned char active[4];
};

struct CatapultLayer {
    unsigned char pad_0[0xc];
    struct CatapultSprite *ride;
};

struct CatapultSprite {
    unsigned char pad_0[0x14];
    unsigned int field_14;
    unsigned int field_18;
    unsigned int field_1c;
    unsigned char pad_20[0x44];
    struct CatapultInner *field_64;
};

struct CatapultInner {
    unsigned char pad_0[0x10];
    unsigned int field_10;
};

// SFX list table (Catapult_SFX); Load_FXList fills the sample-def slots.
struct CatapultFX {
    /* 0x00 */ unsigned char pad_0[8];
    /* 0x08 */ void *field_8[(0x70 - 8) / 4];
};

struct CatapultRideNode {
    unsigned char pad_0[0x10];
    struct RideNode *slots[4];
    unsigned char pad_20[8];
    unsigned int flags[4];
};

struct ChainNode {
    struct ChainNode *next;
    unsigned char pad_4[4];
    struct Bloke *bloke;
    unsigned short id;
};

struct CatapultRide {
    unsigned char pad_0[0xcc];
    struct ChainNode *chain;
};

struct CatapultLoadNode {
    unsigned int field_0;
    struct CatapultLoadNode *next;
    unsigned char pad_8[8];
    struct ChainNode *slots[4];
    unsigned char pad_20[0x3c - 0x20];
};

struct CatapultEdit {
    unsigned char field_0;
    unsigned char pad_1[3];
    unsigned char field_4;
};

struct CatapultKey {
    unsigned char field_0;
    unsigned char field_1;
};

struct CatapultRemoveEdit {
    unsigned char pad_0[0xc];
    unsigned int ride;
};

#include "image_sprite.h"

// FUNCTION: LEGOLAND 0x004030f0
struct CatapultNode *Catapult_AddNode(const unsigned short *arg) {
    struct CatapultNode *node = (struct CatapultNode *)malloc(0x3c);
    if (node != NULL) {
        memset(node, 0, 0x3c);
        node->tile_id = *arg;
        node->next = CatapultNodeList;
        CatapultNodeList = node;
    }
}

// FUNCTION: LEGOLAND 0x00403130
void Catapult_RemoveNode(struct CatapultNode *node) {
    struct CatapultNode *prev;
    struct CatapultNode *cur;

    if (CatapultNodeList == node) {
        CatapultNodeList = node->next;
    } else {
        cur = CatapultNodeList->next;
        prev = CatapultNodeList;
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

// FUNCTION: LEGOLAND 0x00403190
void Catapult_FreeNodes(void) {
    while (CatapultNodeList != NULL) {
        Catapult_RemoveNode(CatapultNodeList);
    }
}

// FUNCTION: LEGOLAND 0x004031b0
struct CatapultRideNode *Catapult_FindNode(const unsigned short *key) {
    struct CatapultNode *cur = CatapultNodeList;

    if (cur != NULL) {
        do {
            if (memcmp(&cur->tile_id, key, 2) == 0) {
                return (struct CatapultRideNode *)cur;
            }
            cur = cur->next;
        } while (cur != NULL);
    }
    return NULL;
}

// FUNCTION: LEGOLAND 0x004031e0
void LoadCatapultResources(struct CatapultLayer *param1) {
    struct CatapultSprite *sprite;
    struct CatapultInner *inner;

    sprite = param1->ride;
    ActiveCatapultRide = sprite;
    if (sprite != NULL) {
        sprite->field_1c |= 0x420;
        inner = ((struct CatapultSprite *)ActiveCatapultRide)->field_64;
        if (inner != NULL) {
            inner->field_10 |= 0x2000;
            DAT_004c10f0 = ((struct CatapultSprite *)ActiveCatapultRide)->field_64;
        }
    }
    HideLayer(DAT_004c10f0, 0);
    HideLayer(DAT_004c10f0, 1);
    Load_FXList(Catapult_SFX, 4);
}

// FUNCTION: LEGOLAND 0x00403250
void CatapultFreeNodesAndKillSfx(void) {
    Catapult_FreeNodes();
    Kill_FXList(Catapult_SFX, 4);
}

// FUNCTION: LEGOLAND 0x00403270
void RenderCatapult(struct Element *obj, int unused, int unused2, TileId *tile, int unused3, int arg5) {
    struct CatapultItem *item;
    struct Ride *ride = obj->ride;
    struct ChainNode *chain;
    struct Point off;
    struct Point pos;
    int i;
    unsigned int *lp;
    unsigned char *fp;

    chain = ((struct CatapultRide *)ride)->chain;
    item = (struct CatapultItem *)Catapult_FindNode((unsigned short *)tile);
    if (item != NULL) {
        pos = GetScreenCoordsForObject(tile, ride);
        if (item->field_8 & 1) {
            LLSSetFrame(GetLLSForLayer(DAT_004c10f0, 1), item->field_d);
            off = GetRenderOffsetForLayer(DAT_004c10f0, 1);
            AdjustOffsetForViewMode(&off);
            PrintSprite(GetSpriteForLayer(DAT_004c10f0, 1), pos.x + off.x, pos.y + off.y, arg5, 0);
        } else {
            LLSSetFrame(GetLLSForLayer(DAT_004c10f0, 0), item->field_c);
            off = GetRenderOffsetForLayer(DAT_004c10f0, 0);
            AdjustOffsetForViewMode(&off);
            PrintSprite(GetSpriteForLayer(DAT_004c10f0, 0), pos.x + off.x, pos.y + off.y, arg5, 0);
        }
        lp = DAT_004b40a4;
        fp = item->active;
        for (i = 4; i != 0; i--) {
            if (*fp & 1) {
                off = GetRenderOffsetForLayer(DAT_004c10f0, *lp);
                AdjustOffsetForViewMode(&off);
                PrintSprite(GetSpriteForLayer(DAT_004c10f0, *lp), pos.x + off.x, pos.y + off.y, arg5, 0);
            }
            fp++;
            lp++;
        }
        while (chain != NULL) {
            if (tile->id == chain->id) {
                IP_RenderBlokeIn3DNow(chain->bloke);
            }
            chain = chain->next;
        }
    }
}

// FUNCTION: LEGOLAND 0x00403430
void FUN_00403430(struct CatapultItem *node) {
    struct SampleParams params;

    if (node->field_8 != 0) {
        return;
    }

    node->field_8 = 1;
    node->field_d = 0;
    params.field_0 = 2;
    params.x = node->field_0;
    params.y = node->field_1;
    PlayInstanceOfSample(((struct CatapultFX *)Catapult_SFX)->field_8[9], 0, 1, &params);
}

// FUNCTION: LEGOLAND 0x00403480
void FUN_00403480(struct CatapultItem *item) {
    if (item->field_8 & 0x1) {
        GetSpriteForLayer(DAT_004c10f0, 1);
        item->field_d++;
        if (item->field_d >= 0x20) {
            item->field_8 &= 0xfffffffe;
        }
    }
}

// FUNCTION: LEGOLAND 0x004034c0
void FUN_004034c0(unsigned char *data, unsigned int index) {
    struct SampleParams params;
    struct CatapultFX *fx = (struct CatapultFX *)Catapult_SFX;

    if (data[index + 0x20] != 0) {
        return;
    }

    data[index + 0x20] = 1;
    data[index + 0x24] = 0;
    params.field_0 = 2;
    params.x = data[0];
    params.y = data[1];
    {
        int r = rand() % 3;
        PlayInstanceOfSample(fx->field_8[r * 3], 0, 1, &params);
    }
}

// FUNCTION: LEGOLAND 0x00403530
void FUN_00403530(struct CatapultNode *node, unsigned int index) {
    if (node->active[index] & 1) {
        if (++node->frame[index] >= 32) {
            node->active[index] = 0;
        }
        LLSSetFrame(GetLLSForLayer(DAT_004c10f0, DAT_004b40a4[index]), node->frame[index]);
    }
}

// FUNCTION: LEGOLAND 0x00403580
void FUN_00403580(void *arg, unsigned int index) {
    unsigned char *p = (unsigned char *)arg + index * 4;
    *(unsigned int *)(p + 0x28) = 0;
    *(unsigned int *)(p + 0x10) = 0;
}

// FUNCTION: LEGOLAND 0x004035a0
void FUN_004035a0(struct CatapultNode *node) {
    int i;
    struct RideNode *entry;

    node->mode = node->mode + 1;
    if (node->mode >= 0x10) {
        node->mode = 0;
    }

    for (i = 0; i < 4; i++) {
        struct RideNode **pentry = &node->slots[i];
        entry = *pentry;
        if (entry == NULL) {
            continue;
        }
        if ((((unsigned char *)pentry)[0x18] & 1) == 0) {
            continue;
        }

        entry->rider->dir = 7;
        entry->rider->field_58 = entry->rider->field_58 - 1;
        if (entry->rider->field_58 > 0) {
            continue;
        }

        entry->rider->field_58 = rand() % 0x14 + 0x32;
        entry->rider->field_3a = entry->rider->field_3a - 1;
        if ((rand() % 0x64) <= 0x2d) {
            FUN_004034c0((unsigned char *)node, i);
            if (node->field_8 == NULL) {
                FUN_00403430((struct CatapultItem *)node);
            }
        }
        entry->rider->field_3a = entry->rider->field_3a - 1;
        if (entry->rider->field_3a > 0) {
            continue;
        }
        entry->rider->param_action = entry->rider->param_action + 1;
        FUN_00403580(node, i);
    }

    FUN_00403480((struct CatapultItem *)node);

    for (i = 0; i < 4; i++) {
        FUN_00403530(node, i);
    }
}

// FUNCTION: LEGOLAND 0x00403690
void FUN_00403690(void) {
    struct CatapultNode *node = CatapultNodeList;
    while (node != NULL) {
        FUN_004035a0(node);
        node = node->next;
    }
}

// FUNCTION: LEGOLAND 0x004036b0
signed char FUN_004036b0(struct CatapultRideNode *node) {
    signed char i = rand() % 4;

    while (node->slots[i] != NULL) {
        i++;
        if (i >= 4) {
            i = 0;
        }
    }
    return i;
}

// FUNCTION: LEGOLAND 0x004036f0
void FUN_004036f0(const unsigned short *key, struct RideNode *entry, int x, int y) {
    struct Bloke *bloke = entry->rider;
    struct CatapultRideNode *node = Catapult_FindNode(key);
    signed char slot;
    char dir;

    if (node == NULL) {
        return;
    }
    slot = FUN_004036b0(node);
    node->slots[slot] = entry;
    bloke->field_3a = 3;
    bloke->dest.x = (x << 8) - (rand() % 2 ? 0x10 : -0x10) - 0xe0;
    bloke->dest.y = (y << 8) + (rand() % 2 ? 0x20 : -0x20) + DAT_004b40b4[slot];
    dir = CalcMoveLine(bloke->pos, bloke->dest, &bloke->nav);
    bloke->low_level_action = 7;
    bloke->field_73 = dir + 0x10;
    NewDirForAction(bloke, (bloke->field_73 >> 5) + 3);
    bloke->param_action++;
}

// FUNCTION: LEGOLAND 0x004037d0
void FUN_004037d0(const unsigned short *key, struct RideNode *entry) {
    struct CatapultRideNode *node = Catapult_FindNode(key);
    int i;

    if (node == NULL) {
        return;
    }

    for (i = 0; i < 4; i++) {
        if (node->slots[i] == entry) {
            node->flags[i] = 1;
        }
    }

    entry->rider->field_58 = (rand() & 0x1f) + 3;
    entry->rider->field_3a = 3;
}

// FUNCTION: LEGOLAND 0x00403820
void FUN_00403820(struct Element *elem) {
    struct Ride *ride = elem->ride;
    struct RideNode *node;
    struct RideNode *next;

    FUN_00403690();
    for (node = ride->riders; node != NULL; node = next) {
        struct Bloke *bloke = node->rider;
        TileId *tile = &node->tile;
        int x, y;
        char dir;

        next = node->next;
        x = tile->pos.x + ride->x;
        y = tile->pos.y + ride->y;
        if (bloke->low_level_action != 0) {
            continue;
        }
        switch (bloke->param_action) {
        case 0:
            bloke->flags |= 8;
            FUN_004036f0(&tile->id, node, x, y);
            break;
        case 1:
            FUN_004037d0(&tile->id, node);
            bloke->param_action++;
            break;
        case 3:
            bloke->dest.x = (x << 8) + 0x80;
            bloke->dest.y = (y << 8) + 0x80;
            dir = CalcMoveLine(bloke->pos, bloke->dest, &bloke->nav);
            bloke->low_level_action = 7;
            bloke->field_73 = dir + 0x10;
            NewDirForAction(bloke, (bloke->field_73 >> 5) + 3);
            bloke->param_action++;
            break;
        case 4:
            RemoveBlokeFromRide(ride, node);
            bloke->flags &= 0xfff7;
            break;
        }
    }
}

// FUNCTION: LEGOLAND 0x00403930
void CatapultSetEditMode(void) {
    EditMode.unk8 = ActiveCatapultRide;
    EditMode.unk0 = 1;
    EditMode.unk8 = ActiveCatapultRide;
    DefaultCursor(&EditCursor);
    SetEditCursorFootPrint((unsigned char *)EditMode.unk8 + 0x3c);
}

// FUNCTION: LEGOLAND 0x00403970
void CatapultAddObject(struct EditObject *edit2, struct CatapultEdit *edit) {
    struct CatapultKey key;

    key.field_0 = edit->field_0;
    key.field_1 = edit->field_4;
    AddBasicObject(edit2, (int *)edit);
    Catapult_AddNode((const unsigned short *)&key);
}

// FUNCTION: LEGOLAND 0x004039a0
void CatapultRemoveObject(struct CatapultRemoveEdit *edit, TileId key, void *cursor, unsigned int param_4) {
    struct CatapultRideNode *node;

    StandardRemoveObject((Element *)edit, key, (struct Cursor *)cursor);
    RemoveAllBlokesFromRide((struct Ride *)edit->ride, key);
    node = Catapult_FindNode(&key.id);
    if (node != NULL) {
        Catapult_RemoveNode((struct CatapultNode *)node);
    }
}

// FUNCTION: LEGOLAND 0x004039e0
unsigned int *FUN_004039e0(struct CatapultLayer *arg1, unsigned short arg2) {
    struct CatapultSprite *sprite = arg1->ride;

    DAT_004c1100 = sprite->field_64;
    DAT_004c1104 = sprite->field_14;
    DAT_004c1108 = sprite->field_18;
    DAT_004c110c = arg2;

    sprite->field_64->field_10 |= 0x2000;

    return (unsigned int *)&DAT_004c1100;
}

// FUNCTION: LEGOLAND 0x00403a20
LEGO_EXPORT int Catapult_Save(void) {
    struct CatapultSaveBuf {
        unsigned int data[15];
    };
    struct CatapultNode *node;
    int *field;
    unsigned int *cursor;
    int i;
    struct Ride *ride;
    unsigned int *value;
    int index;
    unsigned int one;
    unsigned int zero;
    struct CatapultSaveBuf scratch;

    node = CatapultNodeList;
    one = 1;
    zero = 0;
    if (CatapultNodeList != NULL) {
        while (node != NULL) {
            scratch = *(struct CatapultSaveBuf *)node;
            if (SaveGameWrite(&one, 4) == 0) {
                return 0;
            }
            ride = ActiveCatapultRide;
            field = (int *)&scratch.data[4];
            i = 4;
            do {
                value = (unsigned int *)*field;
                index = 0;
                for (cursor = (unsigned int *)ride->riders; cursor != NULL; cursor = (unsigned int *)*cursor) {
                    if (cursor == value) {
                        break;
                    }
                    index = index + 1;
                }
                if (cursor != NULL) {
                    *field = index + 1;
                } else {
                    *field = 0;
                }
                field++;
                i--;
            } while (i != 0);
            if (SaveGameWrite(&scratch, 0x3c) == 0) {
                return 0;
            }
            node = node->next;
        }
    }
    return SaveGameWrite(&zero, 4) != 0;
}

// FUNCTION: LEGOLAND 0x00403af0
LEGO_EXPORT int Catapult_Load(void) {
    unsigned int header;
    struct CatapultLoadNode *prev = NULL;
    struct CatapultLoadNode *node;
    struct ChainNode *chain;
    unsigned int count;
    int i;

    if (SaveGameRead(&header, 4) == 0) {
        return 0;
    }

    while (header != 0) {
        node = (struct CatapultLoadNode *)malloc(0x3c);
        if (SaveGameRead(node, 0x3c) == 0) {
            return 0;
        }

        node->next = NULL;
        if (prev != NULL) {
            prev->next = node;
        } else {
            CatapultNodeList = (struct CatapultNode *)node;
        }
        prev = node;

        for (i = 0; i < 4; i++) {
            count = (unsigned int)node->slots[i];
            chain = ((struct CatapultRide *)ActiveCatapultRide)->chain;
            if (count != 0) {
                while (--count != 0) {
                    chain = chain->next;
                }
                node->slots[i] = chain;
            } else {
                node->slots[i] = NULL;
            }
        }

        if (SaveGameRead(&header, 4) == 0) {
            return 0;
        }
    }

    return 1;
}

// FUNCTION: LEGOLAND 0x00403bb0
void Catapult_GetInterfaces(struct ClassNode *name, struct CallbackTable *ci) {
    // STRING: LEGOLAND 0x004b412c
    if (_stricmp("CATAPULT", name->name) == 0) {
        ci->cb_a4 = LoadCatapultResources;
        ci->cb_ac = CatapultFreeNodesAndKillSfx;
        ci->cb_8c = CatapultSetEditMode;
        ci->cb_a8 = FUN_00403820;
        ci->cb_b0 = RenderCatapult;
        ci->cb_9c = CatapultRemoveObject;
        ci->cb_98 = CatapultAddObject;
        ci->cb_a0 = FUN_004039e0;
        ci->cb_bc = Catapult_Save;
        ci->cb_b8 = Catapult_Load;
    }
}
