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

struct CatapultEdit {
    unsigned char field_0;
    unsigned char pad_1[3];
    unsigned char field_4;
};

struct CatapultKey {
    unsigned char field_0;
    unsigned char field_1;
};

#include "image_sprite.h"

// FUNCTION: LEGOLAND 0x004030f0
struct CatapultNode *Catapult_AddNode(const unsigned short *arg) {
    struct CatapultNode *node = (struct CatapultNode *)malloc(sizeof(struct CatapultNode));
    if (node != NULL) {
        memset(node, 0, sizeof(struct CatapultNode));
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
struct CatapultNode *Catapult_FindNode(const unsigned short *key) {
    struct CatapultNode *cur = CatapultNodeList;

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

// FUNCTION: LEGOLAND 0x004031e0
void LoadCatapultResources(struct Element *param1) {
    struct Ride *sprite;
    struct Sprite *inner;

    sprite = param1->ride;
    ActiveCatapultRide = sprite;
    if (sprite != NULL) {
        sprite->flags |= 0x420;
        inner = ActiveCatapultRide->layer;
        if (inner != NULL) {
            inner->flags |= 0x2000;
            DAT_004c10f0 = ActiveCatapultRide->layer;
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
void RenderCatapult(struct Element *obj, int unused, int unused2, TileId *tile, void *unused3, int arg5) {
    struct CatapultNode *item;
    struct Ride *ride = obj->ride;
    struct RideNode *chain;
    struct Point off;
    struct Point pos;
    int i;
    unsigned int *lp;
    unsigned char *fp;

    chain = ride->riders;
    item = Catapult_FindNode((unsigned short *)tile);
    if (item != NULL) {
        pos = GetScreenCoordsForObject(tile, ride);
        if (item->field_8 & 1) {
            LLSSetFrame(GetLLSForLayer(DAT_004c10f0, 1), item->field_d);
            off = GetRenderOffsetForLayer(DAT_004c10f0, 1);
            AdjustOffsetForViewMode(&off);
            PrintSprite(GetSpriteForLayer(DAT_004c10f0, 1), pos.x + off.x, pos.y + off.y, arg5, 0);
        } else {
            LLSSetFrame(GetLLSForLayer(DAT_004c10f0, 0), item->mode);
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
            if (tile->id == chain->tile.id) {
                IP_RenderBlokeIn3DNow(chain->rider);
            }
            chain = chain->next;
        }
    }
}

// FUNCTION: LEGOLAND 0x00403430
void FUN_00403430(struct CatapultNode *node) {
    struct SampleSource params;

    if (node->field_8 != 0) {
        return;
    }

    node->field_8 = 1;
    node->field_d = 0;
    params.type = 2;
    params.x = node->field_0;
    params.y = node->field_1;
    PlayInstanceOfSample(Catapult_SFX[3].sample, 0, 1, &params);
}

// FUNCTION: LEGOLAND 0x00403480
void FUN_00403480(struct CatapultNode *item) {
    if (item->field_8 & 0x1) {
        GetSpriteForLayer(DAT_004c10f0, 1);
        item->field_d++;
        if (item->field_d >= 0x20) {
            item->field_8 &= 0xfffffffe;
        }
    }
}

// FUNCTION: LEGOLAND 0x004034c0
void FUN_004034c0(struct CatapultNode *node, unsigned int index) {
    struct SampleSource params;
    struct FXItem *fx = Catapult_SFX;

    if (node->active[index] != 0) {
        return;
    }

    node->active[index] = 1;
    node->frame[index] = 0;
    params.type = 2;
    params.x = node->field_0;
    params.y = node->field_1;
    {
        int r = rand() % 3;
        PlayInstanceOfSample(fx[r].sample, 0, 1, &params);
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
        if (node->slots[i] == NULL) {
            continue;
        }
        if ((node->flags[i] & 1) == 0) {
            continue;
        }
        entry = node->slots[i];

        entry->rider->dir = 7;
        entry->rider->field_58 = entry->rider->field_58 - 1;
        if (entry->rider->field_58 > 0) {
            continue;
        }

        entry->rider->field_58 = rand() % 0x14 + 0x32;
        entry->rider->field_3a = entry->rider->field_3a - 1;
        if ((rand() % 0x64) <= 0x2d) {
            FUN_004034c0(node, i);
            if (node->field_8 == 0) {
                FUN_00403430(node);
            }
        }
        entry->rider->field_3a = entry->rider->field_3a - 1;
        if (entry->rider->field_3a > 0) {
            continue;
        }
        entry->rider->param_action = entry->rider->param_action + 1;
        FUN_00403580(node, i);
    }

    FUN_00403480(node);

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
signed char FUN_004036b0(struct CatapultNode *node) {
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
    struct CatapultNode *node = Catapult_FindNode(key);
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
    struct CatapultNode *node = Catapult_FindNode(key);
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
void CatapultUpdate(struct Element *elem) {
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
    SetEditCursorFootPrint(&EditMode.unk8->footprint);
}

// FUNCTION: LEGOLAND 0x00403970
void CatapultAddObject(struct EditObject *edit2, struct CatapultEdit *edit) {
    struct CatapultKey key;

    key.field_0 = edit->field_0;
    key.field_1 = edit->field_4;
    AddBasicObject(edit2, edit);
    Catapult_AddNode((const unsigned short *)&key);
}

// FUNCTION: LEGOLAND 0x004039a0
void CatapultRemoveObject(struct Element *edit, TileId key, void *cursor, unsigned int param_4) {
    struct CatapultNode *node;

    StandardRemoveObject(edit, key, cursor);
    RemoveAllBlokesFromRide(edit->ride, key);
    node = Catapult_FindNode(&key.id);
    if (node != NULL) {
        Catapult_RemoveNode(node);
    }
}

// FUNCTION: LEGOLAND 0x004039e0
struct RideSpriteInfo *GetCatapultSpriteInfo(struct Element *arg1, unsigned short arg2) {
    struct Ride *sprite = arg1->ride;

    DAT_004c1100.sprite = sprite->layer;
    DAT_004c1100.x = sprite->field_14;
    DAT_004c1100.y = sprite->field_18;
    DAT_004c1100.id = arg2;

    sprite->layer->flags |= 0x2000;

    return &DAT_004c1100;
}

// FUNCTION: LEGOLAND 0x00403a20
LEGO_EXPORT int Catapult_Save(struct Element *element) {
    struct CatapultNode *node;
    struct RideNode **field;
    struct RideNode *cursor;
    int i;
    struct Ride *ride;
    struct RideNode *value;
    int index;
    unsigned int one;
    unsigned int zero;
    struct CatapultNode scratch;

    node = CatapultNodeList;
    one = 1;
    zero = 0;
    if (CatapultNodeList != NULL) {
        while (node != NULL) {
            scratch = *node;
            if (SaveGameWrite(&one, 4) == 0) {
                return 0;
            }
            ride = ActiveCatapultRide;
            field = scratch.slots;
            i = 4;
            do {
                value = *field;
                index = 0;
                for (cursor = ride->riders; cursor != NULL; cursor = cursor->next) {
                    if (cursor == value) {
                        break;
                    }
                    index = index + 1;
                }
                if (cursor != NULL) {
                    *field = (struct RideNode *)(INT_PTR)(index + 1);
                } else {
                    *field = NULL;
                }
                field++;
                i--;
            } while (i != 0);
            if (SaveGameWrite(&scratch, sizeof(scratch)) == 0) {
                return 0;
            }
            node = node->next;
        }
    }
    return SaveGameWrite(&zero, 4) != 0;
}

// FUNCTION: LEGOLAND 0x00403af0
LEGO_EXPORT int Catapult_Load(struct Element *element) {
    unsigned int header;
    struct CatapultNode *prev = NULL;
    struct CatapultNode *node;
    struct RideNode *chain;
    unsigned int count;
    int i;

    if (SaveGameRead(&header, 4) == 0) {
        return 0;
    }

    while (header != 0) {
        node = (struct CatapultNode *)malloc(sizeof(struct CatapultNode));
        if (SaveGameRead(node, sizeof(struct CatapultNode)) == 0) {
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
            count = (UINT_PTR)node->slots[i];
            chain = ActiveCatapultRide->riders;
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
void Catapult_GetInterfaces(struct ClassNode *name, struct Ride *ci) {
    // STRING: LEGOLAND 0x004b412c
    if (_stricmp("CATAPULT", name->name) == 0) {
        ci->cb_a4 = LoadCatapultResources;
        ci->cb_ac = CatapultFreeNodesAndKillSfx;
        ci->cb_8c = CatapultSetEditMode;
        ci->cb_pre_render = CatapultUpdate;
        ci->cb_b0 = RenderCatapult;
        ci->cb_9c = CatapultRemoveObject;
        ci->cb_98 = CatapultAddObject;
        ci->cb_sprite = GetCatapultSpriteInfo;
        ci->save_hook = Catapult_Save;
        ci->load_hook = Catapult_Load;
    }
}
