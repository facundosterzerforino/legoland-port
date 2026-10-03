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
#include "obj_instance.h"
#include "print_sprite.h"
#include "render3d.h"
#include "safari_ride.h"
#include "sound_music.h"
#include "tilemap.h"

#include "image_sprite.h"

struct SafariBlockData {
    unsigned char pad_0[0x10];
    unsigned int field_10;
};

struct SafariBlock {
    unsigned char pad_0[0x14];
    unsigned int field_14;
    unsigned int field_18;
    unsigned char pad_1c[0x48];
    struct SafariBlockData *layer;
};

struct SafariRoot {
    unsigned char pad_0[0xc];
    struct SafariBlock *ride;
};

struct SafariListEntry;

struct SafariOwner {
    unsigned char pad_0[0x1c];
    unsigned int flags;
    unsigned char pad_20[0x44];
    struct Sprite *layer;
    unsigned char pad_68[0x64];
    struct SafariListEntry *head;
};

struct SafariObject {
    unsigned char pad_0[0xc];
    struct SafariOwner *field_c;
};

struct SafariSample {
    unsigned char tile_x;
    unsigned char tile_y;
    unsigned char pad_2[2];
    unsigned int seated_count;
    unsigned int field_8;
    unsigned int frame;
    unsigned char pad_10[4];
    unsigned int field_14;
    unsigned int loops_left;
    unsigned int frame_ticks;
    unsigned int boarding_count;
};

// FUNCTION: LEGOLAND 0x004149c0
void FUN_004149c0(struct SafariNode *param) {
    struct SafariNode *s = (struct SafariNode *)malloc(sizeof(struct SafariNode));
    if (s) {
        memset(s, 0, sizeof(struct SafariNode));
        s->tile_id = param->tile_id;
        s->next = SafariNodeList;
        SafariNodeList = s;
        FUN_00414b10(s);
    }
}

// FUNCTION: LEGOLAND 0x00414a00
void RemoveSafariNode(struct SafariNode *node) {
    struct SafariNode *prev;
    struct SafariNode *cur;

    if (SafariNodeList == node) {
        SafariNodeList = node->next;
    } else {
        cur = SafariNodeList->next;
        prev = SafariNodeList;
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

// FUNCTION: LEGOLAND 0x00414a60
void FUN_00414a60(void) {
    while (SafariNodeList != NULL) {
        RemoveSafariNode(SafariNodeList);
    }
}

struct SafariKey {
    unsigned short id;
    unsigned short pad;
};

// FUNCTION: LEGOLAND 0x00414a80
void *FUN_00414a80(struct SafariKey *key) {
    struct SafariNode *cur = SafariNodeList;

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

// FUNCTION: LEGOLAND 0x00414ab0
void FUN_00414ab0(struct SafariSample *a1) {
    struct SampleSource src;

    src.type = 2;
    a1->field_8 = a1->seated_count;
    a1->field_14 = (a1->field_14 & 0xFFFFBFFF) | 1;
    a1->seated_count = 0;
    a1->frame = 0;
    a1->frame_ticks = 0;
    src.x = a1->tile_x;
    src.y = a1->tile_y;
    PlayInstanceOfSample(SAFARI_SFX[0].sample, 1, 1, &src);
}

// FUNCTION: LEGOLAND 0x00414b10
void FUN_00414b10(struct SafariNode *node) {
    struct SafariSample *a1 = (struct SafariSample *)node;
    struct SampleSource src;

    a1->frame_ticks = 0;
    a1->frame = 0;
    a1->loops_left = rand() % 2 + 3;
    a1->boarding_count = 0;
    a1->seated_count = 0;
    src.type = 2;
    a1->field_14 &= 0xFFFFBFFE;
    src.x = a1->tile_x;
    src.y = a1->tile_y;
    UnSourceAndFadeAllSamplesFromSource(&src, -200);
}

// FUNCTION: LEGOLAND 0x00414b80
void RenderSafari(Element *obj, unsigned int param_2, unsigned int param_3, unsigned short *tile, unsigned int param_5, unsigned int clip) {
    struct Ride *ride = obj->ride;
    struct RideNode *node = ride->riders;
    struct SafariNode *sn;
    struct Point pos;
    char found = 0;

    sn = FUN_00414a80((struct SafariKey *)tile);
    if (sn != NULL) {
        RenderItems_New();
        DAT_004cbecc = NULL;
        pos = GetScreenCoordsForObject((TileId *)tile, ride);
        for (; node != NULL; node = node->next) {
            if (*tile == node->tile.id) {
                found = 1;
                break;
            }
        }
        *(short *)*ZSafariSprite->lls = (short)sn->frame;
        if (found) {
            LLSSetFrame(GetLLSForLayer(SafariLayer, 0), sn->frame);
            PrintSprite(GetSpriteForLayer(SafariLayer, 0), pos.x, pos.y, clip, 0);
            for (node = ride->riders; node != NULL; node = node->next) {
                if (*tile == node->tile.id) {
                    struct Bloke *bloke = node->rider;

                    if ((bloke->flags & 0x80) != 0) {
                        struct Point off2;
                        struct Person *person;
                        struct Point off1;

                        off2 = DAT_0082c670;
                        person = bloke->person;
                        off1.x = DAT_004cbee8;
                        off1.y = DAT_004cbeec;
                        AdjustOffsetForViewMode(&off1);
                        person->offset.x = bloke->screen_x + off1.x;
                        person->offset.y = bloke->screen_y + off1.y;
                        AdjustBlokePosition(&person->offset);
                        AdjustOffsetForViewMode(&off2);
                        person->screen.x = bloke->screen_x + (off2.x + off1.x) + pos.x;
                        person->screen.y = bloke->screen_y + (off2.y + off1.y) + pos.y;
                        AdjustBlokePosition(&person->screen);
                    }
                    AddBlokeToRenderList(&DAT_004cbecc, (struct BlokeRenderSrc *)node, node->person->field_20);
                }
            }
        } else {
            LLSSetFrame(GetLLSForLayer(SafariLayer, 0), sn->frame);
            PrintSprite(GetSpriteForLayer(SafariLayer, 0), pos.x, pos.y, clip, 0);
        }
        RenderBlokeList((struct BlokeListHead *)&DAT_004cbecc);
    }
}

// FUNCTION: LEGOLAND 0x00414d90
void FUN_00414d90(struct SafariObject *a1) {
    SafariRide = a1->field_c;
    SafariRide->flags |= 0x420;
    SafariLayer = SafariRide->layer;
    SafariLayer->flags |= 0x2000;
    // STRING: LEGOLAND 0x004b4d5c
    SafariRunBNV = LoadBinV("Zbuffers\\Safarirun.bnv");
    // STRING: LEGOLAND 0x004b4d44
    DAT_004cbf04[0] = LoadBinV("Zbuffers\\Safarion.bnv");
    // STRING: LEGOLAND 0x004b4d2c
    SafariOffBNV = LoadBinV("Zbuffers\\Safarioff.bnv");
    // STRING: LEGOLAND 0x004b4d1c
    ZSafariSprite = LoadSprite("z_Safari.lls", 1);
    DAT_0082c670.x = 0;
    DAT_0082c670.y = -1;
    DAT_004cbee8 = -41;
    DAT_004cbeec = -95;
    HideLayer(SafariLayer, 0);
    StopLayerPlaying(SafariLayer, 0);
    LLSSetFrame(GetLLSForLayer(SafariLayer, 0), 0);
    DAT_004cbef8 = SafariRunBNV;
    DAT_004cbefc = DAT_004cbf04[0];
    DAT_004cbf00 = SafariOffBNV;
    DAT_004cbf08 = ZSafariSprite;
    Load_FXList(SAFARI_SFX, 1);
}

// FUNCTION: LEGOLAND 0x00414ea0
void FUN_00414ea0(struct SafariObject *a1) {
    SafariRide = a1->field_c;
    FUN_00414a60();
    Kill_FXList(SAFARI_SFX, 1);
    FreeBinV(DAT_004cbef8);
    FreeBinV(DAT_004cbefc);
    FreeBinV(DAT_004cbf00);
    KillSprite(DAT_004cbf08);
}

// FUNCTION: LEGOLAND 0x00414f00
void SafariSetEditMode(void) {
    EditMode.unk0 = 1;
    EditMode.unk8 = (void *)SafariRide;
    DefaultCursor(&EditCursor);
    SetEditCursorFootPrint((char *)EditMode.unk8 + 0x3c);
}

struct SafariEditObj {
    unsigned char pad_0[0xc];
    unsigned int ride;
};

// FUNCTION: LEGOLAND 0x00414f40
void SafariRemoveObject(struct SafariEditObj *obj, TileId key, unsigned int coords, unsigned int cursor) {
    void *node = FUN_00414a80(&key);
    struct SampleSource src;

    if (node == 0) {
        return;
    }
    RemoveSafariNode((struct SafariNode *)node);
    StandardRemoveObject((Element *)obj, key, (struct Cursor *)coords);
    RemoveAllBlokesFromRide((struct Ride *)obj->ride, key);

    src.type = 2;
    src.x = key.pos.x;
    src.y = key.pos.y;
    UnSourceAndFadeAllSamplesFromSource(&src, 0xffffff38);
}

struct SafariBasicObject {
    unsigned char field_0;
    unsigned char pad_1[3];
    unsigned char field_4;
};

// FUNCTION: LEGOLAND 0x00414fc0
void SafariAddObject(unsigned int uid, struct SafariBasicObject *a1) {
    unsigned short local;

    *(unsigned char *)&local = a1->field_0;
    *((unsigned char *)&local + 1) = a1->field_4;
    AddBasicObject((Element *)uid, (int *)a1);
    FUN_004149c0((struct SafariNode *)&local);
}

// FUNCTION: LEGOLAND 0x00414ff0
unsigned int *FUN_00414ff0(struct SafariRoot *p1, unsigned short arg2) {
    struct SafariBlock *pB = p1->ride;
    DAT_004cbed0.sprite = pB->layer;
    DAT_004cbed0.x = pB->field_14;
    DAT_004cbed0.y = pB->field_18;
    DAT_004cbed0.id = arg2;
    pB->layer->field_10 |= 0x2000;
    return (void *)&DAT_004cbed0;
}

// FUNCTION: LEGOLAND 0x00415030
void SafariRideGetInterfaces(struct ClassNode *name, struct CallbackTable *interfaces) {
    // STRING: LEGOLAND 0x004b4d74
    if (_stricmp("SAFARI RIDE", name->name) == 0) {
        interfaces->cb_a4 = FUN_00414d90;
        interfaces->cb_ac = FUN_00414ea0;
        interfaces->cb_8c = SafariSetEditMode;
        interfaces->cb_a8 = FUN_00415220;
        interfaces->cb_b0 = RenderSafari;
        interfaces->cb_9c = SafariRemoveObject;
        interfaces->cb_98 = SafariAddObject;
        interfaces->cb_a0 = FUN_00414ff0;
        interfaces->cb_bc = SaveSafariRide;
        interfaces->cb_b8 = LoadSafariRide;
    }
}

struct SafariState {
    unsigned short id;
    unsigned char pad_2[2];
    int seated_count;
    unsigned char pad_8[4];
    int frame;
    unsigned char pad_10[4];
    unsigned int flags;
    int loops_left;
    int frame_ticks;
    int boarding_count;
    int board_timer;
};

// FUNCTION: LEGOLAND 0x004150c0
void FUN_004150c0(struct SafariNode *node) {
    struct SafariState *s = (struct SafariState *)node;
    struct RideNode *r = ((struct Ride *)SafariRide)->riders;
    unsigned int flags = s->flags;

    if (flags & 1) {
        int v = ++s->frame_ticks;
        int n = s->loops_left;
        if (n == 0) {
            if (GetAllBlokesOffRide((struct Ride *)SafariRide, s->id) == 0) {
                return;
            }
            FUN_00414b10(node);
            return;
        }
        if (v >= 2) {
            s->frame_ticks = 0;
            s->frame++;
            if (s->frame >= 0x30) {
                s->frame = 0;
                s->loops_left = n - 1;
            }
        }
    } else {
        int cur = s->seated_count;
        if (flags & 0x4000) {
            if (cur == s->boarding_count) {
                s->flags = flags & 0xffffbfff;
                FUN_00414ab0((struct SafariSample *)s);
                return;
            }
        } else if (cur != 0) {
            int k = s->board_timer;
            if (k == 0) {
                s->flags = flags | 0x4000;
                Ride_SetFlagToNotLetAnyoneOn(s);
            } else {
                s->board_timer = k - 1;
            }
        }
    }
    for (; r != NULL; r = r->next) {
        if (s->id == r->tile.id && r->rider->field_35 == 1) {
            // STRING: LEGOLAND 0x004b4704
            sprintf(DAT_004b4cac + 6, "%02d", r->rider->field_36 + 1);
            SetBlokePositionFromBNV(SafariRunBNV, r->rider, DAT_004b4cac, s->frame, -1617787.0f, -1618006.0f, 0);
        }
    }
    *(short *)*ZSafariSprite->lls = (short)s->frame;
    Put3DBlokesOnRide2((Element *)SafariRide, (Element *)node);
}

// FUNCTION: LEGOLAND 0x00415200
void FUN_00415200(void) {
    struct SafariNode *current = SafariNodeList;
    if (current != NULL) {
        while (current != NULL) {
            FUN_004150c0(current);
            current = current->next;
        }
    }
}

// FUNCTION: LEGOLAND 0x00415220
void FUN_00415220(Element *obj) {
    struct Ride *ride = obj->ride;
    struct RideNode *node;
    struct RideNode *next;
    struct Bloke *bloke;
    struct SafariNode *s;
    TileId *tile;
    int x;
    int y;
    int w;
    int h;
    int coords[4];
    int coords2[4];
    struct Point pos;
    char dir;

    FUN_00415200();
    for (node = ride->riders; node != NULL; node = next) {
        tile = &node->tile;
        next = node->next;
        bloke = node->rider;
        s = (struct SafariNode *)FUN_00414a80((struct SafariKey *)tile);
        if (s == NULL) {
            return;
        }
        y = ride->field_25 + tile->pos.y;
        x = ride->field_24 + tile->pos.x;
        if (bloke->low_level_action != 0) {
            continue;
        }
        switch (bloke->param_action) {
        case 0:
            s->field_20++;
            s->field_24 = 0xb4;
            bloke->param_action++;
            bloke->flags |= 8;
            bloke->field_58 = 0;
            break;
        case 1:
            pos = GetScreenCoordsForObject(tile, ride);
            GetTileDimensions(&w, &h);
            {
                int px = bloke->pos.x;
                int py = bloke->pos.y;
                int sx = ((px - py) * w) >> 9;
                int sy = ((px + py) * h) >> 9;
                coords[0] = ((unsigned short)lpConfig->view_x - (short)Get_XScroll() + sx - DAT_0082c670.x / 2 - pos.x) * 2;
                coords[1] = ((unsigned short)lpConfig->view_y - (short)Get_YScroll() + sy - DAT_0082c670.y / 2 - pos.y) * 2;
            }
            bloke->person->sprite = DAT_004cbf08;
            bloke->person->field_30 = 1;
            bloke->person->depth = GetUnitDepth(-1617787.0f, -1618006.0f);
            bloke->field_35 = 0;
            // STRING: LEGOLAND 0x004b4704
            sprintf(DAT_004b4cac + 6, "%02d", (FUN_00415760((struct SafariListEntry *)node, &node->tile.id) >> 1) + 1);
            bloke->path = NewBNVPath(DAT_004cbefc, 1, DAT_004b4cac, -1617787.0f, -1618006.0f, coords);
            UpdateBlokeFromBNVPath(bloke, bloke->path);
            BNVPath_SetDFrame(bloke, bloke->path, 0);
            bloke->param_action++;
            break;
        case 2:
            bloke->flags |= 0x80;
            if (UpdateBlokeFromBNVPath(bloke, bloke->path) == 0) {
                bloke->field_35 = 1;
                bloke->param_action = 5;
                free(bloke->path);
                bloke->field_54 = 0;
            }
            if (bloke->path != NULL) {
                if ((int)BNVPath_GetDFrame(bloke->path) >= DAT_004b4cc4[bloke->field_36]) {
                    bloke->field_35 = 1;
                    bloke->param_action = 5;
                    free(bloke->path);
                    bloke->field_54 = 0;
                }
            }
            BlokeSetFrame(bloke, bloke->frame);
            break;
        case 5:
            bloke->flags |= 0x80;
            BlokeSitAnim(bloke);
            BlokeSetFrame(bloke, 0);
            bloke->field_35 = 1;
            bloke->person->depth = GetUnitDepth(-1617787.0f, -1618006.0f);
            bloke->param_action++;
            s->field_4++;
            if (s->field_4 == ((struct Ride *)SafariRide)->seats) {
                FUN_00414ab0((struct SafariSample *)s);
            }
            break;
        case 7:
            bloke->flags |= 0x80;
            coords2[0] = bloke->screen_x * 2;
            coords2[1] = bloke->screen_y * 2;
            BlokeWalkAnim(bloke);
            BlokeSetFrame(bloke, 0);
            bloke->person->sprite = DAT_004cbf08;
            bloke->person->field_30 = 1;
            bloke->person->depth = GetUnitDepth(-1617787.0f, -1618006.0f);
            bloke->field_35 = 2;
            sprintf(DAT_004b4cac + 6, "%02d", (bloke->field_36 >> 1) + 1);
            node->rider->path = NewBNVPath(DAT_004cbf00, 2, DAT_004b4cac, -1617787.0f, -1618006.0f, coords2);
            BNVPath_SetDFrame(bloke, bloke->path, 0);
            UpdateBlokeFromBNVPath(bloke, bloke->path);
            bloke->param_action++;
            break;
        case 8:
            if (UpdateBlokeFromBNVPath(bloke, bloke->path) == 0) {
                bloke->field_35 = 2;
                bloke->param_action = 13;
                free(node->rider->path);
                bloke->field_54 = 0;
            }
            if (bloke->path != NULL) {
                if ((int)BNVPath_GetDFrame(bloke->path) >= DAT_004b4ce4[bloke->field_36]) {
                    bloke->field_35 = 2;
                    bloke->param_action = 13;
                    free(bloke->path);
                    bloke->field_54 = 0;
                }
            }
            BlokeSetFrame(bloke, bloke->frame);
            break;
        case 13:
            bloke->flags &= ~0x80;
            bloke->person->sprite = NULL;
            bloke->person->field_30 = 0;
            UnAdjustBlokePosition(&bloke->person->screen);
            ScreenToMapRef((int *)&bloke->person->screen, (int *)&bloke->pos, 0);
            bloke->person->field_34 = 0;
            bloke->pos.x <<= 8;
            bloke->pos.y <<= 8;
            bloke->dest.x = (x << 8) + 0x80;
            bloke->dest.y = (y << 8) + 0x80;
            dir = CalcMoveLine(bloke->pos, bloke->dest, &bloke->nav);
            bloke->low_level_action = 7;
            bloke->field_73 = dir + 0x10;
            NewDirForAction(bloke, (bloke->field_73 >> 5) + 3);
            bloke->param_action++;
            break;
        case 14:
            RemoveBlokeFromRide(ride, node);
            bloke->flags &= ~8;
            if (--s->field_8 == 0) {
                s->field_4 = 0;
                Ride_ClearFlagToNotLetAnyoneOn(s);
            }
            break;
        }
    }
}

struct SafariSlotData {
    unsigned char pad_0[0x36];
    unsigned char index;
};

struct SafariListEntry {
    struct SafariListEntry *next;
    unsigned char pad_4[4];
    struct SafariSlotData *data;
    unsigned short key;
};

// FUNCTION: LEGOLAND 0x00415760
int FUN_00415760(struct SafariListEntry *node, unsigned short *key) {
    struct SafariListEntry *cur;
    int n = 0;

    for (cur = SafariRide->head; cur != NULL; cur = cur->next) {
        if (memcmp(&cur->key, key, 2) == 0) {
            if (cur == node) {
                node->data->index = (unsigned char)n;
                return n;
            }
            n++;
        }
    }
    return 0;
}

// FUNCTION: LEGOLAND 0x004157b0
LEGO_EXPORT int SaveSafariRide(void) {
    struct SafariNode *current = SafariNodeList;
    unsigned int value1 = 1;
    unsigned int value0 = 0;

    while (current != NULL) {
        if (SaveGameWrite(&value1, 0x4) == 0) {
            return 0;
        }
        if (SaveGameWrite(current, 0x28) == 0) {
            return 0;
        }
        current = current->next;
    }

    if (SaveGameWrite(&value0, 0x4) == 0) {
        return 0;
    }
    return 1;
}

struct SafariTypeC {
    void *field_0;
    unsigned int field_4;
};

struct SafariData {
    unsigned char pad_0[0x54];
    struct SafariTypeC *bnv;
};

struct SafariCar {
    unsigned char pad_0[0x2c];
    void *sprite;
    unsigned int field_30;
};

struct SafariListNode {
    struct SafariListNode *next;
    unsigned char pad_4[4];
    struct SafariData *field_8;
    unsigned char pad_c[4];
    struct SafariCar *field_10;
};

struct SafariGameObject {
    unsigned char pad_0[0xcc];
    struct SafariListNode *field_cc;
};

struct SafariLoadArg {
    unsigned char pad_0[0xc];
    struct SafariGameObject *ride;
};

// FUNCTION: LEGOLAND 0x00415820
LEGO_EXPORT int LoadSafariRide(struct SafariLoadArg *arg) {
    struct SafariGameObject *obj = arg->ride;
    struct SafariNode *prev = NULL;
    struct SafariListNode *list;
    struct SafariCar *car;
    struct SafariData *data;
    struct SafariTypeC *tc;
    unsigned int marker;

    if (!SaveGameRead(&marker, 4)) {
        return 0;
    }
    while (marker != 0) {
        struct SafariNode *node = (struct SafariNode *)malloc(0x28);
        if (!SaveGameRead(node, 0x28)) {
            return 0;
        }
        node->next = NULL;
        if (prev != NULL) {
            prev->next = node;
        } else {
            SafariNodeList = node;
        }
        prev = node;
        if (!SaveGameRead(&marker, 4)) {
            return 0;
        }
    }

    list = obj->field_cc;
    while (list != NULL) {
        car = list->field_10;
        if (car->field_30 != 0) {
            /* [port] the original reads DAT_004cbf04[field_30]; field_30 is 1 here, the global after it */
            car->sprite = car->field_30 == 1 ? DAT_004cbf08 : DAT_004cbf04[car->field_30];
        } else {
            car->sprite = NULL;
            list->field_10->field_30 = 0;
        }
        data = list->field_8;
        tc = data->bnv;
        if (tc != NULL) {
            /* [port] the original reads (&DAT_004cbef8)[field_4], the three path BNVs laid out one after another */
            void **const bnvs[3] = {&DAT_004cbef8, &DAT_004cbefc, &DAT_004cbf00};
            tc->field_0 = *bnvs[tc->field_4];
        }
        list = list->next;
    }
    return 1;
}
