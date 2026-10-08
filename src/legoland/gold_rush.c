#include <stdlib.h>
#include <string.h>
#include "legoland.h"

#include "bloke.h"
#include "gamemap.h"
#include "globals.h"
#include "gold_rush.h"
#include "llidb.h"
#include "man3d.h"
#include "map_object.h"
#include "obj_instance.h"
#include "objclass.h"
#include "path_control.h"
#include "print_sprite.h"
#include "render3d.h"
#include "ride_queue.h"
#include "tilemap.h"

struct GoldArray {
    unsigned char pad_0[0x14];
    unsigned int field_14[6];
};

struct GoldItem {
    /* 0x00 */ unsigned char pad_0[8];
    /* 0x08 */ struct GoldSub *rider;
    /* 0x0c */ unsigned char tile;
};

struct GoldSub {
    unsigned char pad_0[0x36];
    unsigned char field_36;
};

struct GoldRandSub {
    unsigned char pad_0[0x58];
    unsigned int field_58;
};

struct GoldRandItem {
    unsigned char pad_0[8];
    struct GoldRandSub *rider;
};

struct GoldBloke {
    unsigned char pad_0[0x58];
    int counter;
    unsigned char pad_5c[4];
    unsigned char var_60;
    unsigned char pad_61;
    unsigned short var_62;
    unsigned char pad_64[0xe];
    unsigned char var_72;
};

struct GoldBlokeRef {
    unsigned char pad_0[8];
    struct GoldBloke *bloke;
    unsigned char field_c;
};

struct GoldRide {
    unsigned char pad_0[0xc];
    int x;
    int y;
};

struct GoldEditObject {
    unsigned char pad_0[0xc];
    struct GoldRide *ride;
};

struct GoldNode {
    unsigned short key;
    unsigned char pad_2[0xa];
    struct GoldNode *next;
    unsigned char pad_10[0x1c];
};

struct GoldObj {
    unsigned char pad_0[0xc];
    struct GoldInner *inner;
};

struct GoldInner {
    unsigned char pad_0[0x1c];
    unsigned int flags;
    unsigned char pad_20[0x44];
    struct GoldLayer *layer;
};

struct GoldLayer {
    unsigned char pad_0[0x10];
    unsigned int flags;
};

struct GoldWalkItem {
    unsigned char pad_0[8];
    struct Bloke *bloke;
};

struct GoldSlot {
    int index;
    float weight;
};

#include "image_sprite.h"

// GLOBAL: LEGOLAND 0x004b45b0
static struct GoldSlot Gold_Slots[6] = {{0, 0.8f}, {0, 0.2f}, {1, 0.8f}, {1, 0.2f}, {2, 0.8f}, {2, 0.2f}};

// GLOBAL: LEGOLAND 0x004b45e0
static struct PathPair Gold_PathPairs[5] = {{-2, 0}, {0, -6}, {-6, 0}, {0, 1}, {3, 0}};
// GLOBAL: LEGOLAND 0x004b4608
struct PathTable DAT_004b4608 = {5, Gold_PathPairs};

// GLOBAL: LEGOLAND 0x004b4610
static struct Point Gold_Points[3] = {{0x400, 0x60}, {0x400, 0x3d0}, {0x400, 0x700}};

// FUNCTION: LEGOLAND 0x00406920
void AddGoldWashNode(struct GoldNode *src) {
    struct GoldNode *node = (struct GoldNode *)malloc(0x2c);
    if (node != NULL) {
        memset(node, 0, 0x2c);
        node->key = src->key;
        node->next = GoldWashList;
        GoldWashList = node;
    }
}

// FUNCTION: LEGOLAND 0x00406960
void RemoveGoldWashNode(struct GoldNode *node) {
    struct GoldNode *prev;
    struct GoldNode *cur;

    if (GoldWashList == node) {
        GoldWashList = node->next;
    } else {
        cur = GoldWashList->next;
        prev = GoldWashList;
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

// FUNCTION: LEGOLAND 0x004069c0
void FreeGoldWashList(void) {
    while (GoldWashList != NULL) {
        RemoveGoldWashNode(GoldWashList);
    }
}

// FUNCTION: LEGOLAND 0x004069e0
void *FindGoldWashNode(void *param) {
    struct GoldNode *cur = GoldWashList;

    if (cur != NULL) {
        do {
            if (memcmp(&cur->key, param, 2) == 0) {
                return cur;
            }
            cur = cur->next;
        } while (cur != NULL);
    }
    return NULL;
}

// FUNCTION: LEGOLAND 0x00406a10
void GoldRushLoad(struct GoldObj *obj) {
    struct GoldInner *inner = obj->inner;
    GoldRushRide = inner;
    if (inner != NULL) {
        inner->flags |= 0x20;
        if (((struct GoldInner *)GoldRushRide)->layer != NULL) {
            ((struct GoldInner *)GoldRushRide)->layer->flags |= 0x2000;
            GoldRushLayer = (struct Sprite *)((struct GoldInner *)GoldRushRide)->layer;
        }
    }

    DAT_004c11e4 = (struct Sprite *)FUN_00412100(&DAT_004b4608);
    // STRING: LEGOLAND 0x004b465c
    GoldWashMatte1Sprite = LoadSprite("goldwashmatte1.lls", 1);
    // STRING: LEGOLAND 0x004b464c
    GoldWashSprite = LoadSprite("goldwash.lls", 1);
    // STRING: LEGOLAND 0x004b4638
    GoldWashMatte2Sprite = LoadSprite("goldwashmatte2.lls", 1);
    // STRING: LEGOLAND 0x004b4628
    GoldMaskSprite = LoadSprite("goldmask.lls", 1);
}

// FUNCTION: LEGOLAND 0x00406ab0
void GoldRushUnload(void) {
    if (GoldWashMatte1Sprite != NULL) {
        KillSprite(GoldWashMatte1Sprite);
    }
    if (GoldWashSprite != NULL) {
        KillSprite(GoldWashSprite);
    }
    if (GoldWashMatte2Sprite != NULL) {
        KillSprite(GoldWashMatte2Sprite);
    }
    if (GoldMaskSprite != NULL) {
        KillSprite(GoldMaskSprite);
    }
    if (DAT_004c11e4 != NULL) {
        FreeIfNotNull(DAT_004c11e4);
    }
    FreeGoldWashList();
}

// FUNCTION: LEGOLAND 0x00406b10
void RenderGoldRush(Element *obj, unsigned int param_2, unsigned int param_3, unsigned short *tile, unsigned int param_5, unsigned int clip) {
    struct Ride *ride = obj->ride;
    struct RideNode *node;
    struct Point pos;
    int frame;
    struct Sprite *spr;
    struct LLS *lls;

    pos = GetScreenCoordsForObject((TileId *)tile, ride);
    RenderItems_New();
    GoldRushBlokeRenderList = NULL;
    for (node = ride->riders; node != NULL; node = node->next) {
        if (*tile == node->tile.id && node->rider->pos.x <= (((unsigned char *)tile)[0] << 8) + 0x780 && node->rider->pos.y <= (((unsigned char *)tile)[1] << 8) - 0x280) {
            AddBlokeToRenderList(&GoldRushBlokeRenderList, (struct BlokeRenderSrc *)node, node->person->field_20);
        }
    }
    RenderBlokeList((struct BlokeListHead *)&GoldRushBlokeRenderList);
    if (GoldMaskSprite != NULL) {
        struct Point off;
        frame = 0;
        spr = GetSpriteForLayer(GoldRushLayer, 1);
        if (spr != NULL) {
            lls = (struct LLS *)GetLLSForSprite((struct SpriteLLS *)spr);
            if (lls != NULL) {
                frame = *(short *)lls % 8;
            }
        }
        lls = (struct LLS *)GetLLSForSprite((struct SpriteLLS *)GoldMaskSprite);
        if (lls != NULL) {
            LLSSetFrame(lls, frame);
        }
        off = GetRenderOffsetForLayer(GoldRushLayer, 1);
        AdjustOffsetForViewMode(&off);
        PrintSprite(GoldMaskSprite, pos.x + off.x, pos.y + off.y, clip, 0);
    }
    RenderItems_New();
    GoldRushBlokeRenderList = NULL;
    for (node = ride->riders; node != NULL; node = node->next) {
        if (*tile == node->tile.id && node->rider->pos.x <= (((unsigned char *)tile)[0] << 8) + 0x780 && node->rider->pos.y >= (((unsigned char *)tile)[1] << 8) - 0x280) {
            AddBlokeToRenderList(&GoldRushBlokeRenderList, (struct BlokeRenderSrc *)node, node->person->field_20);
        }
    }
    RenderBlokeList((struct BlokeListHead *)&GoldRushBlokeRenderList);
    if (GoldWashMatte2Sprite != NULL) {
        struct Point off;
        off = GetRenderOffsetForLayer(GoldRushLayer, 1);
        AdjustOffsetForViewMode(&off);
        PrintSprite(GoldWashMatte2Sprite, pos.x + off.x, pos.y + off.y, clip, 0);
    }
    RenderItems_New();
    GoldRushBlokeRenderList = NULL;
    for (node = ride->riders; node != NULL; node = node->next) {
        if (*tile == node->tile.id && node->rider->pos.x > (((unsigned char *)tile)[0] << 8) + 0x780) {
            AddBlokeToRenderList(&GoldRushBlokeRenderList, (struct BlokeRenderSrc *)node, node->person->field_20);
        }
    }
    RenderBlokeList((struct BlokeListHead *)&GoldRushBlokeRenderList);
    if (GoldWashMatte1Sprite != NULL) {
        struct Point off;
        off = GetRenderOffsetForLayer(GoldRushLayer, 1);
        AdjustOffsetForViewMode(&off);
        PrintSprite(GoldWashMatte1Sprite, pos.x + off.x, pos.y + off.y, clip, 0);
    }
    RenderItems_New();
    GoldRushBlokeRenderList = NULL;
    for (node = ride->riders; node != NULL; node = node->next) {
        if (*tile == node->tile.id && (node->rider->pos.x >> 8) > ((unsigned char *)tile)[0] + 8) {
            AddBlokeToRenderList(&GoldRushBlokeRenderList, (struct BlokeRenderSrc *)node, node->person->field_20);
        }
    }
    RenderBlokeList((struct BlokeListHead *)&GoldRushBlokeRenderList);
    if (GoldRushLayer != NULL) {
        struct Point off;
        off = GetRenderOffsetForLayer(GoldRushLayer, 3);
        AdjustOffsetForViewMode(&off);
        PrintSprite(GetSpriteForLayer(GoldRushLayer, 3), pos.x + off.x, pos.y + off.y, clip, 0);
    }
}

// FUNCTION: LEGOLAND 0x00406e90
unsigned int FUN_00406e90(void *param) {
    struct GoldArray *a = (struct GoldArray *)FindGoldWashNode(param);
    unsigned int i;

    if (a == NULL) {
        return 0;
    }
    for (i = 0; i < 6; i++) {
        if (a->field_14[i] == 0) {
            return 1;
        }
    }
    return 0;
}

// FUNCTION: LEGOLAND 0x00406ec0
void FUN_00406ec0(struct GoldItem *item, void *param2) {
    struct GoldArray *a = (struct GoldArray *)FindGoldWashNode(param2);
    unsigned int i;

    if (a == NULL) {
        return;
    }
    for (i = 0; i < 6; i++) {
        if (a->field_14[i] == 0) {
            a->field_14[i] = 1;
            item->rider->field_36 = (unsigned char)i;
            return;
        }
    }
}

// FUNCTION: LEGOLAND 0x00406f00
struct GoldArray *FUN_00406f00(struct GoldItem *item) {
    unsigned int idx = item->rider->field_36;
    struct GoldArray *a = (struct GoldArray *)FindGoldWashNode(&item->tile);

    if (a != NULL) {
        a->field_14[idx] = 0;
    }
    return a;
}

// FUNCTION: LEGOLAND 0x00406f30
void FUN_00406f30(void *param) {
    if (FUN_00406e90(param) == 0) {
        Ride_SetFlagToNotLetAnyoneOn(param);
    } else {
        Ride_ClearFlagToNotLetAnyoneOn(param);
    }
}

// FUNCTION: LEGOLAND 0x00406f60
void FUN_00406f60(struct GoldWalkItem *item, unsigned char *p) {
    struct Bloke *b = item->bloke;
    int slot = Gold_Slots[b->field_36].index;

    b->dest.x = Gold_Points[slot].x;
    b->dest.y = Gold_Points[slot].y;
    b->dest.x += p[0] << 8;
    b->dest.y += p[1] << 8;
    b->dest.x += 0x80;
    b->dest.y += 0x80;
    b->field_73 = CalcMoveLine(b->pos, b->dest, &b->nav) + 0x10;
    b->low_level_action = 7;
    NewDirForAction(b, (unsigned char)((b->field_73 >> 5) + 3));
}

// FUNCTION: LEGOLAND 0x00407000
void FUN_00407000(struct GoldWalkItem *item, unsigned char *p) {
    struct Bloke *b = item->bloke;
    struct GoldSlot *s = &Gold_Slots[b->field_36];
    int slot = s->index;
    float w = s->weight;

    b->dest.x = Gold_Points[slot].x - 0x280;
    b->dest.y = Gold_Points[slot].y;
    b->dest.x += p[0] << 8;
    b->dest.y += (p[1] << 8) + 0x80;
    b->dest.x += 0x80 - (int)(w * 512.0f);
    b->field_73 = CalcMoveLine(b->pos, b->dest, &b->nav) + 0x10;
    b->low_level_action = 7;
    NewDirForAction(b, (unsigned char)((b->field_73 >> 5) + 3));
}

// FUNCTION: LEGOLAND 0x004070b0
void FUN_004070b0(struct GoldWalkItem *item, unsigned char *p) {
    struct Bloke *b = item->bloke;
    struct GoldSlot *s = &Gold_Slots[b->field_36];
    int slot = s->index;
    float w = s->weight;

    b->dest.x = Gold_Points[slot].x - 0x280;
    b->dest.y = Gold_Points[slot].y;
    b->dest.x += p[0] << 8;
    b->dest.y += p[1] << 8;
    b->dest.x += 0x80 - (int)(w * 512.0f);
    b->dest.y -= 0x50;
    b->field_73 = CalcMoveLine(b->pos, b->dest, &b->nav) + 0x10;
    b->low_level_action = 7;
    NewDirForAction(b, (unsigned char)((b->field_73 >> 5) + 3));
}

// FUNCTION: LEGOLAND 0x00407170
void FUN_00407170(struct GoldWalkItem *item, unsigned char *p) {
    struct Bloke *b = item->bloke;
    struct GoldSlot *s = &Gold_Slots[b->field_36];
    int slot = s->index;
    float w = s->weight * 512.0f;

    b->pos.y += 0x80;
    b->dest.x = Gold_Points[slot].x - 0x280;
    b->dest.y = Gold_Points[slot].y;
    b->dest.x += p[0] << 8;
    b->dest.y += (p[1] << 8) + 0x80;
    b->dest.x += 0x80 - (int)w;
    b->field_73 = CalcMoveLine(b->pos, b->dest, &b->nav) + 0x10;
    b->low_level_action = 7;
    NewDirForAction(b, (unsigned char)((b->field_73 >> 5) + 3));
}

// FUNCTION: LEGOLAND 0x00407230
void FUN_00407230(struct GoldRandItem *param) {
    int value = rand() % 31;
    param->rider->field_58 = value + 15;
}

// FUNCTION: LEGOLAND 0x00407250
void FUN_00407250(struct GoldBlokeRef *ref) {
    struct GoldBloke *b = ref->bloke;

    b->var_62 |= 0x100;
    BlokePanWithPan((struct Bloke *)b);
    BlokeAnimNextFrame((struct Bloke *)b);
    b->var_72 = 1;
    b->counter = b->counter - 1;
    if (b->counter <= 0) {
        b->var_62 &= 0xfeff;
        BlokeWalkWithPan((struct Bloke *)b);
        FUN_00406f00((struct GoldItem *)ref);
        FUN_00406f30(&ref->field_c);
        b->var_60++;
    }
}

// FUNCTION: LEGOLAND 0x004072b0
void GoldRushUpdate(struct Element *elem) {
    struct Ride *ride = elem->ride;
    struct RideNode *node = ride->riders;
    struct RideNode *next;
    struct Bloke *b;
    unsigned char *t;
    int x;
    int y;

    while (node != NULL) {
        next = node->next;
        t = (unsigned char *)&node->tile;
        b = node->rider;
        x = t[0] + ride->x;
        y = t[1] + ride->y;
        if (b->low_level_action == 0) {
            switch (b->param_action) {
            case 0:
                b->flags |= 8;
                FUN_00406ec0((struct GoldItem *)node, t);
                FUN_00406f30(t);
                FUN_004122d0((struct RideSlotArg *)DAT_004c11e4, (struct RideSlot *)b);
                break;
            case 1:
                FUN_00412300((struct QueueTable *)DAT_004c11e4, x, y, b);
                if ((b->pos.x >> 8) == t[0] + 5 && (b->pos.y >> 8) == t[1] - 3) {
                    BlokeWalkWithPan(b);
                }
                break;
            case 2:
                b->param_action++;
                b->dest.x = (t[0] << 8) + 0x480;
                b->dest.y = (t[1] << 8) - 0x180;
                b->field_73 = CalcMoveLine(b->pos, b->dest, &b->nav) + 0x10;
                b->low_level_action = 7;
                NewDirForAction(b, (unsigned char)((b->field_73 >> 5) + 3));
                break;
            case 3:
            case 8:
                FUN_00406f60((struct GoldWalkItem *)node, t);
                b->param_action++;
                break;
            case 4:
                FUN_00407000((struct GoldWalkItem *)node, t);
                b->param_action++;
                break;
            case 5:
                FUN_004070b0((struct GoldWalkItem *)node, t);
                FUN_00407230((struct GoldRandItem *)node);
                b->param_action++;
                break;
            case 6:
                FUN_00407250((struct GoldBlokeRef *)node);
                break;
            case 7:
                FUN_00407170((struct GoldWalkItem *)node, t);
                b->param_action++;
                break;
            case 9:
                b->dest.x = (t[0] << 8) + 0x480;
                b->dest.y = (t[1] << 8) - 0x180;
                b->field_73 = CalcMoveLine(b->pos, b->dest, &b->nav) + 0x10;
                b->low_level_action = 7;
                NewDirForAction(b, (unsigned char)((b->field_73 >> 5) + 3));
                b->param_action++;
                break;
            case 10:
                FUN_004122a0((struct RideSlotArg *)DAT_004c11e4, (struct RideSlot *)b);
                break;
            case 11:
                FUN_00412300((struct QueueTable *)DAT_004c11e4, x, y, b);
                if ((b->pos.x >> 8) == t[0] + 5 && (b->pos.y >> 8) == t[1] - 3) {
                    BlokeWalkAnim(b);
                }
                break;
            case 12:
                b->dest.x = (x << 8) + 0x80;
                b->dest.y = (y << 8) + 0x80;
                b->field_73 = CalcMoveLine(b->pos, b->dest, &b->nav) + 0x10;
                b->low_level_action = 7;
                NewDirForAction(b, (unsigned char)((b->field_73 >> 5) + 3));
                b->param_action++;
                break;
            case 13:
                RemoveBlokeFromRide(ride, node);
                b->flags &= 0xfff7;
                break;
            }
        }
        node = next;
    }
}

// FUNCTION: LEGOLAND 0x004075b0
void GoldRushSetEditMode(void) {
    EditMode.unk0 = 1;
    EditMode.unk8 = GoldRushRide;
    DefaultCursor(&EditCursor);
    SetEditCursorFootPrint((unsigned char *)EditMode.unk8 + 0x3c);
}

// FUNCTION: LEGOLAND 0x004075f0
void GoldRushAddObject(struct GoldEditObject *editObj, struct Point *pos) {
    TileId id;
    struct Point p;
    struct GoldRide *ride;

    id.pos.x = (unsigned char)pos->x;
    id.pos.y = (unsigned char)pos->y;
    ride = editObj->ride;
    AddBasicObject((Element *)editObj, (int *)pos);
    AddGoldWashNode((struct GoldNode *)&id);

    p.x = pos->x + ride->x - 1;
    p.y = pos->y + ride->y;
    AddPathTileGFX(&p, *(unsigned short *)PathSprite);

    p.x = pos->x + ride->x - 2;
    p.y = pos->y + ride->y;
    AddPathTileGFX(&p, *(unsigned short *)PathSprite);

    p.x = pos->x + ride->x - 2;
    p.y = pos->y + ride->y - 1;
    AddPathTileGFX(&p, *(unsigned short *)PathSprite);

    p.x = pos->x + ride->x - 2;
    p.y = pos->y + ride->y - 2;
    AddPathTileGFX(&p, *(unsigned short *)PathSprite);
}

// FUNCTION: LEGOLAND 0x004076e0
void GoldRushRemoveObject(struct GoldEditObject *editObj, TileId coords, struct Cursor *cursor) {
    int p[2];
    struct GoldRide *ride = editObj->ride;
    void *found = FindGoldWashNode(&coords);

    if (found != NULL) {
        RemoveGoldWashNode((struct GoldNode *)found);
    }

    StandardRemoveObject((Element *)editObj, coords, cursor);
    RemoveAllBlokesFromRide((struct Ride *)ride, coords);

    p[0] = coords.pos.x + ride->x - 1;
    p[1] = coords.pos.y + ride->y;
    RemoveRollerCoasterPath(p);

    p[0] = coords.pos.x + ride->x - 2;
    p[1] = coords.pos.y + ride->y;
    RemoveRollerCoasterPath(p);

    p[0] = coords.pos.x + ride->x - 2;
    p[1] = coords.pos.y + ride->y - 1;
    RemoveRollerCoasterPath(p);

    p[0] = coords.pos.x + ride->x - 2;
    p[1] = coords.pos.y + ride->y - 2;
    RemoveRollerCoasterPath(p);
}

// FUNCTION: LEGOLAND 0x00407800
LEGO_EXPORT int SaveGoldWash(void) {
    struct GoldNode *node;
    unsigned int one;
    unsigned int zero;

    one = 1;
    zero = 0;
    node = GoldWashList;
    while (node != NULL) {
        if (SaveGameWrite(&one, 4) == 0) {
            return 0;
        }
        if (SaveGameWrite(node, 0x2c) == 0) {
            return 0;
        }
        node = node->next;
    }
    return SaveGameWrite(&zero, 4) != 0;
}

// FUNCTION: LEGOLAND 0x00407870
LEGO_EXPORT int LoadGoldWash(void) {
    unsigned int count;
    struct GoldNode *node;
    struct GoldNode *prev = NULL;

    if (!SaveGameRead(&count, 4)) {
        return 0;
    }
    while (count != 0) {
        node = malloc(0x2c);
        if (!SaveGameRead(node, 0x2c)) {
            return 0;
        }
        node->next = NULL;
        if (prev != NULL) {
            prev->next = node;
        } else {
            GoldWashList = node;
        }
        prev = node;
        if (!SaveGameRead(&count, 4)) {
            return 0;
        }
    }
    return 1;
}

// FUNCTION: LEGOLAND 0x004078f0
void GoldRush_GetInterfaces(struct ClassNode *str, struct CallbackTable *module) {
    // STRING: LEGOLAND 0x004b4670
    if (_stricmp("GOLD RUSH", str->name) == 0) {
        module->cb_a4 = GoldRushLoad;
        module->cb_ac = GoldRushUnload;
        module->cb_8c = GoldRushSetEditMode;
        module->cb_a8 = GoldRushUpdate;
        module->cb_b0 = RenderGoldRush;
        module->cb_98 = GoldRushAddObject;
        module->cb_9c = GoldRushRemoveObject;
        module->cb_b8 = LoadGoldWash;
        module->cb_bc = SaveGoldWash;
    }
}
