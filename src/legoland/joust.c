#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "globals.h"
#include "legoland.h"

#include "binv.h"
#include "bloke.h"
#include "gamemap.h"
#include "joust.h"
#include "llidb.h"
#include "man3d.h"
#include "map_object.h"
#include "obj_instance.h"
#include "objclass.h"
#include "print_sprite.h"
#include "render3d.h"
#include "sound_music.h"

#pragma pack(push, 1)
struct JoustSub12 {
    union {
        struct {
            unsigned int a;
            unsigned short b;
        };
        unsigned char v[6];
    };
};

struct JoustPair {
    char c[2];
};

struct JoustNode {
    TileId id;
    unsigned char pad_2[2];
    struct JoustNode *next;
    struct Sample *sample;
    int busy;
    char x;
    char y;
    struct JoustSub12 sub12;
    struct JoustPair pair;
    char phase;
    char frame;
    unsigned int field_1c;
    unsigned int field_20;
};
#pragma pack(pop)

struct JoustSource {
    unsigned int kind;
    unsigned int pad;
    unsigned int x;
    unsigned int y;
};

struct JoustObject {
    unsigned char pad_0[0xc];
    unsigned int field_c;
};

struct JoustBlockData {
    unsigned char pad_0[0x10];
    unsigned int field_10;
};

struct JoustBlock {
    unsigned char pad_0[0x14];
    unsigned int field_14;
    unsigned int field_18;
    unsigned int flags_1c;
    unsigned char pad_20[0x44];
    struct JoustBlockData *layer;
};

struct JoustRoot {
    unsigned char pad_0[0xc];
    struct JoustBlock *field_c;
};

#include "image_sprite.h"

// FUNCTION: LEGOLAND 0x00407970
struct JoustNode *AddJoustNode(TileId *key) {
    struct JoustNode *node = (struct JoustNode *)malloc(0x24);

    if (node != NULL) {
        memset(node, 0, 0x24);
        node->id.id = key->id;
        node->next = JoustNodeList;
        node->busy = 0;
        node->x = 0;
        node->y = 0;
        memset(&node->sub12, 0, 6);
        memset(&node->pair, 0, 2);
        node->phase = 0;
        node->frame = 0;
        node->field_1c = 0;
        node->field_20 = 0;
        JoustNodeList = node;
    }
    return node;
}

// FUNCTION: LEGOLAND 0x004079e0
void JoustAddObject(Element *editObj, int *coords) {
    TileId key;

    key.pos.x = (unsigned char)coords[0];
    key.pos.y = (unsigned char)coords[1];
    AddBasicObject(editObj, coords);
    AddJoustNode(&key)->sample = 0;
}

// FUNCTION: LEGOLAND 0x00407a20
struct JoustNode *FindJoustNode(TileId *key) {
    struct JoustNode *cur = JoustNodeList;

    if (cur != NULL) {
        do {
            if (memcmp(&cur->id, key, 2) == 0) {
                return cur;
            }
            cur = cur->next;
        } while (cur != NULL);
    }
    return NULL;
}

// FUNCTION: LEGOLAND 0x00407a50
void RemoveJoustNode(struct JoustNode *node) {
    struct JoustNode *prev;
    struct JoustNode *cur;

    if (JoustNodeList == node) {
        JoustNodeList = node->next;
    } else {
        cur = JoustNodeList->next;
        prev = JoustNodeList;
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

// FUNCTION: LEGOLAND 0x00407ab0
void FreeJoustNodeList(void) {
    while (JoustNodeList != NULL) {
        RemoveJoustNode(JoustNodeList);
    }
}

// FUNCTION: LEGOLAND 0x00407ad0
void JoustRemoveObject(Element *editObj, TileId coords, struct Cursor *cursor) {
    struct JoustNode *node;
    struct {
        unsigned int kind;
        unsigned int pad;
        unsigned int x;
        unsigned int y;
    } source;

    node = FindJoustNode(&coords);
    if (node != NULL) {
        source.x = node->id.pos.x;
        source.kind = 2;
        source.y = node->id.pos.y;
        UnSourceAndFadeAllSamplesFromSource(&source, -200);
        node->sample = 0;
        RemoveJoustNode(node);
    }
    StandardRemoveObject(editObj, coords, cursor);
    RemoveAllBlokesFromRide(editObj->ride, coords);
}

// FUNCTION: LEGOLAND 0x00407b50
void JoustLoadResources(struct JoustRoot *root) {
    Load_FXList(JOUST_SFX, 1);
    JoustRide = (unsigned int)root->field_c;
    ((struct JoustBlock *)JoustRide)->flags_1c |= 0x420;
    JoustLayer = (unsigned int)((struct JoustBlock *)JoustRide)->layer;
    ((struct JoustBlockData *)JoustLayer)->field_10 |= 0x2000;
    // STRING: LEGOLAND 0x004b46f4
    JoustFMaskSprite = LoadSprite("Joust_fmask.lls", 1);
    // STRING: LEGOLAND 0x004b46e0
    JoustSpecRMSprite = LoadSprite("Joust_SpecR_m.lls", 1);
    // STRING: LEGOLAND 0x004b46cc
    JoustSpecLMSprite = LoadSprite("Joust_SpecL_m.lls", 1);
    // STRING: LEGOLAND 0x004b46c0
    DAT_004c1240 = ZJoustSprite = LoadSprite("z_joust.lls", 1);
    // STRING: LEGOLAND 0x004b46a8
    JoustRideBnv = LoadBinV("Zbuffers\\joustride.bnv");
    HideLayer((struct Sprite *)JoustLayer, 1);
    StopLayerPlaying((struct Sprite *)JoustLayer, 1);
    LLSSetFrame((struct LLS *)GetLLSForLayer((struct Sprite *)JoustLayer, 1), 0);
}

// FUNCTION: LEGOLAND 0x00407c20
char FUN_00407c20(unsigned char param_1) {
    return (unsigned int)(param_1 != 0);
}

// FUNCTION: LEGOLAND 0x00407c30
void FUN_00407c30(struct Element *elem) {
    struct Ride *ride = elem->ride;
    struct RideNode *node;
    struct RideNode *next;
    struct Bloke *b;
    struct JoustNode *jn;
    unsigned char *t;
    char name[9];

    node = ride->riders;
    // STRING: LEGOLAND 0x004b470c
    strcpy(name, "manBox??");
    for (; node != NULL; node = next) {
        int x;
        int y;
        int busy;
        char count;
        char horses;
        struct JoustSub12 flags;
        struct JoustPair h;
        char hb;
        unsigned int f1c;
        unsigned int f20;

        next = node->next;
        b = node->rider;
        t = (unsigned char *)&node->tile;
        jn = FindJoustNode((TileId *)t);
        if (jn == NULL) {
            return;
        }
        busy = jn->busy;
        count = jn->x;
        horses = jn->y;
        flags = jn->sub12;
        h = jn->pair;
        hb = jn->phase;
        f1c = jn->field_1c;
        f20 = jn->field_20;
        (*ZJoustSprite->lls)->frame = jn->frame;
        x = t[0] + ride->x;
        y = t[1] + ride->y;
        if (b->low_level_action == 0) {
            switch (b->param_action) {
            case 0:
                b->flags |= 8;
                b->dest.x = (x + 1) << 8;
                b->dest.y = (y << 8) - 0x100;
                b->field_73 = CalcMoveLine(b->pos, b->dest, &b->nav) + 0x10;
                b->low_level_action = 7;
                NewDirForAction(b, (unsigned char)((b->field_73 >> 5) + 3));
                b->param_action++;
                b->field_58 = 0;
                break;
            case 1:
                if (busy == 0) {
                    b->dest.x = (x + 2) << 8;
                    b->dest.y = (y << 8) - 0x100;
                    b->field_73 = CalcMoveLine(b->pos, b->dest, &b->nav) + 0x10;
                    b->low_level_action = 7;
                    NewDirForAction(b, (unsigned char)((b->field_73 >> 5) + 3));
                    b->param_action++;
                    busy = 1;
                } else if (horses < 6) {
                    char slot = rand() % 6;

                    while (flags.v[slot] != 0) {
                        slot++;
                        if (slot > 5) {
                            slot = 0;
                        }
                    }
                    b->field_36 = slot;
                    flags.v[slot] = 2;
                    horses++;
                    b->param_action = (slot < 3) ? 10 : 0x14;
                }
                break;
            case 2:
                if (f1c != 0) {
                    b->dest.x = (x + 1) << 8;
                    b->dest.y = (y - 3) << 8;
                    b->field_73 = CalcMoveLine(b->pos, b->dest, &b->nav) + 0x10;
                    b->low_level_action = 7;
                    NewDirForAction(b, (unsigned char)((b->field_73 >> 5) + 3));
                    b->field_44 = FUN_00407c20(hb);
                    h.c[b->field_44] = 1;
                    b->param_action++;
                    count++;
                    busy--;
                }
                break;
            case 3:
                b->flags |= 0x80;
                b->person->sprite = DAT_004c1240;
                b->person->field_30 = 1;
                // STRING: LEGOLAND 0x004b4704
                sprintf(name + 6, "%02d", b->field_44 + 1);
                SetBlokePositionFromBNV(JoustRideBnv, b, name, (char)hb, -1617735.0f, -1617993.5f, 0);
                b->param_action++;
                break;
            case 4:
                h.c[b->field_44] = 2;
                sprintf(name + 6, "%02d", b->field_44 + 1);
                SetBlokePositionFromBNV(JoustRideBnv, b, name, (char)hb, -1617735.0f, -1617993.5f, 0);
                if (b->field_58++ > 0x96) {
                    b->param_action++;
                }
                break;
            case 5:
                sprintf(name + 6, "%02d", b->field_44 + 1);
                SetBlokePositionFromBNV(JoustRideBnv, b, name, (char)hb, -1617735.0f, -1617993.5f, 0);
                h.c[b->field_44] = 3;
                b->param_action++;
                break;
            case 6:
                sprintf(name + 6, "%02d", b->field_44 + 1);
                SetBlokePositionFromBNV(JoustRideBnv, b, name, (char)hb, -1617735.0f, -1617993.5f, 0);
                if (f20 != 0 && (char)FUN_00407c20(hb) == b->field_44) {
                    b->dest.x = (x + 2) << 8;
                    b->dest.y = (y << 8) - 0x100;
                    b->flags &= 0xff7f;
                    b->field_73 = CalcMoveLine(b->pos, b->dest, &b->nav) + 0x10;
                    b->low_level_action = 7;
                    b->person->field_2c = 0;
                    b->person->field_30 = 0;
                    NewDirForAction(b, (unsigned char)((b->field_73 >> 5) + 3));
                    h.c[b->field_44] = 1;
                    b->param_action++;
                    count--;
                }
                break;
            case 7:
                b->dest.x = x << 8;
                b->dest.y = y << 8;
                b->field_73 = CalcMoveLine(b->pos, b->dest, &b->nav) + 0x10;
                b->low_level_action = 7;
                NewDirForAction(b, (unsigned char)((b->field_73 >> 5) + 3));
                h.c[b->field_44] = 0;
                b->param_action = 0x1e;
                break;
            case 10:
            case 15:
                b->dest.x = (x << 8) - 0x100;
                b->dest.y = (y << 8) - 0x100;
                b->field_73 = CalcMoveLine(b->pos, b->dest, &b->nav) + 0x10;
                b->low_level_action = 7;
                NewDirForAction(b, (unsigned char)((b->field_73 >> 5) + 3));
                b->param_action++;
                break;
            case 11:
                b->dest.x = (x << 8) - 0x364;
                b->dest.y = (y - 3) << 8;
                b->field_73 = CalcMoveLine(b->pos, b->dest, &b->nav) + 0x10;
                b->low_level_action = 7;
                NewDirForAction(b, (unsigned char)((b->field_73 >> 5) + 3));
                b->param_action++;
                break;
            case 12:
                b->dest.x = (x << 8) - 0x364;
                b->dest.y = (y << 8) + b->field_36 * 200 - 0x638;
                b->field_73 = CalcMoveLine(b->pos, b->dest, &b->nav) + 0x10;
                b->low_level_action = 7;
                NewDirForAction(b, (unsigned char)((b->field_73 >> 5) + 3));
                b->param_action++;
                break;
            case 13:
            case 25:
                b->dir = 3;
                if (b->field_58++ > 0x96) {
                    horses--;
                    flags.v[b->field_36] = 0;
                    b->param_action++;
                }
                break;
            case 14:
                b->dest.x = (x << 8) - 0x364;
                b->dest.y = (y - 3) << 8;
                b->field_73 = CalcMoveLine(b->pos, b->dest, &b->nav) + 0x10;
                b->low_level_action = 7;
                NewDirForAction(b, (unsigned char)((b->field_73 >> 5) + 3));
                b->param_action++;
                break;
            case 16:
                b->dest.x = x << 8;
                b->dest.y = y << 8;
                b->field_73 = CalcMoveLine(b->pos, b->dest, &b->nav) + 0x10;
                b->low_level_action = 7;
                NewDirForAction(b, (unsigned char)((b->field_73 >> 5) + 3));
                b->param_action = 0x1e;
                break;
            case 20:
                b->dest.x = (x << 8) - 0x100;
                b->dest.y = (y << 8) - 0x100;
                b->field_73 = CalcMoveLine(b->pos, b->dest, &b->nav) + 0x10;
                b->low_level_action = 7;
                NewDirForAction(b, (unsigned char)((b->field_73 >> 5) + 3));
                b->param_action++;
                break;
            case 21:
                b->dest.x = (x << 8) - 0x16a;
                b->dest.y = (y - 2) << 8;
                b->field_73 = CalcMoveLine(b->pos, b->dest, &b->nav) + 0x10;
                b->low_level_action = 7;
                NewDirForAction(b, (unsigned char)((b->field_73 >> 5) + 3));
                b->param_action++;
                break;
            case 22:
                b->dest.x = (x << 8) - 0x16a;
                b->dest.y = (y << 8) - 0xf46;
                b->field_73 = CalcMoveLine(b->pos, b->dest, &b->nav) + 0x10;
                b->low_level_action = 7;
                NewDirForAction(b, (unsigned char)((b->field_73 >> 5) + 3));
                b->param_action++;
                break;
            case 24:
                b->dest.x = (x << 8) - 0x364;
                b->dest.y = (y << 8) + b->field_36 * 200 - 0x102e;
                b->field_73 = CalcMoveLine(b->pos, b->dest, &b->nav) + 0x10;
                b->low_level_action = 7;
                NewDirForAction(b, (unsigned char)((b->field_73 >> 5) + 3));
                b->param_action++;
                break;
            case 23:
            case 26:
                b->dest.x = (x << 8) - 0x364;
                b->dest.y = (y << 8) - 0xf46;
                b->field_73 = CalcMoveLine(b->pos, b->dest, &b->nav) + 0x10;
                b->low_level_action = 7;
                NewDirForAction(b, (unsigned char)((b->field_73 >> 5) + 3));
                b->param_action++;
                break;
            case 27:
                b->dest.x = (x << 8) - 0x16a;
                b->dest.y = (y << 8) - 0xf46;
                b->field_73 = CalcMoveLine(b->pos, b->dest, &b->nav) + 0x10;
                b->low_level_action = 7;
                NewDirForAction(b, (unsigned char)((b->field_73 >> 5) + 3));
                b->param_action++;
                break;
            case 28:
                b->dest.x = (x << 8) - 0x16a;
                b->dest.y = (y - 2) << 8;
                b->field_73 = CalcMoveLine(b->pos, b->dest, &b->nav) + 0x10;
                b->low_level_action = 7;
                NewDirForAction(b, (unsigned char)((b->field_73 >> 5) + 3));
                b->param_action++;
                break;
            case 29:
                b->dest.x = x << 8;
                b->dest.y = y << 8;
                b->field_73 = CalcMoveLine(b->pos, b->dest, &b->nav) + 0x10;
                b->low_level_action = 7;
                NewDirForAction(b, (unsigned char)((b->field_73 >> 5) + 3));
                b->param_action = 0x1e;
                break;
            case 30:
                RemoveBlokeFromRide(ride, node);
                b->flags &= 0xfff7;
                break;
            }
        }
        jn->busy = busy;
        jn->x = count;
        jn->sub12 = flags;
        jn->y = horses;
        jn->pair = h;
        jn->frame = hb;
        jn->field_1c = f1c;
        jn->field_20 = f20;
    }
    for (jn = JoustNodeList; jn != NULL; jn = jn->next) {
        struct JoustPair s;
        char step = jn->phase;
        int busy = jn->busy;
        char count = jn->x;
        unsigned int f20 = jn->field_20;
        unsigned int f1c = 0;

        s = jn->pair;
        do {
            switch (0) {
            default:
                if (step == 0) {
                    if ((busy != 0 && count < 2) || count == 0) {
                        if (s.c[0] != 0) {
                            break;
                        }
                        f1c = 1;
                    } else if (s.c[0] != 1) {
                        if (s.c[0] != 3) {
                            break;
                        }
                        f20 = 1;
                    }
                } else if (step == 0x20) {
                    if ((busy != 0 && count < 2) || count == 0) {
                        if (s.c[1] != 0) {
                            break;
                        }
                        f1c = 1;
                    } else if (s.c[1] != 1) {
                        if (s.c[1] != 3) {
                            break;
                        }
                        f20 = 1;
                    }
                } else {
                    break;
                }
                if (jn->sample != 0) {
                    struct JoustSource stop;

                    stop.kind = 2;
                    stop.x = jn->id.pos.x;
                    stop.y = jn->id.pos.y;
                    UnSourceAndFadeAllSamplesFromSource(&stop, -1000);
                    jn->sample = 0;
                }
                continue;
            }
            if (jn->sample == 0) {
                struct JoustSource play;

                play.kind = 2;
                play.x = jn->id.pos.x;
                play.y = jn->id.pos.y;
                jn->sample = PlayInstanceOfSample(*(void **)(JOUST_SFX + 8), 1, 0, &play);
                FUN_00496d10(jn->sample);
            }
            step++;
            if (step > 0x3f) {
                step = 0;
            }
            f1c = 0;
            f20 = 0;
        } while (0);
        jn->pair = s;
        jn->phase = step;
        jn->field_1c = f1c;
        jn->field_20 = f20;
    }
}

// FUNCTION: LEGOLAND 0x00408580
void RenderJoust(struct Element *element, unsigned int param_2, unsigned int param_3, TileId *tile, unsigned int param_5, unsigned int param_6) {
    struct Ride *ride;
    struct RideNode *r;
    struct JoustNode *node;
    struct Bloke *bloke;
    struct Person *person;
    struct Point off;
    struct Point coords;
    char frame;
    char i;

    ride = element->ride;
    r = ride->riders;
    {
        struct Bloke *riders[8] = {0};
        char n = 0;
        node = FindJoustNode(tile);
        if (node == NULL) {
            return;
        }
        frame = node->frame;
        coords = GetScreenCoordsForObject(tile, ride);
        if (r != NULL) {
            for (; r != NULL; r = r->next) {
                if (tile->id == r->tile.id) {
                    riders[n++] = r->rider;
                }
            }
            if (n != 0) {
                for (i = 0; i < n; i++) {
                    if (riders[i]->param_action == 0x18) {
                        IP_RenderBlokeIn3DNow(riders[i]);
                    }
                }
                for (i = 0; i < n; i++) {
                    if (riders[i]->param_action == 0x19) {
                        IP_RenderBlokeIn3DNow(riders[i]);
                    }
                }
                for (i = 0; i < n; i++) {
                    if (riders[i]->param_action == 0x1a) {
                        IP_RenderBlokeIn3DNow(riders[i]);
                    }
                }
                for (i = 0; i < n; i++) {
                    if (riders[i]->param_action == 0x1b) {
                        IP_RenderBlokeIn3DNow(riders[i]);
                    }
                }
                for (i = 0; i < n; i++) {
                    if (riders[i]->param_action == 0x1c) {
                        IP_RenderBlokeIn3DNow(riders[i]);
                    }
                }
                off = GetRenderOffsetForLayer((struct Sprite *)JoustLayer, 0);
                AdjustOffsetForViewMode(&off);
                PrintSprite(JoustSpecRMSprite, coords.x + off.x, coords.y + off.y, param_6, 0);
                for (i = 0; i < n; i++) {
                    if (riders[i]->param_action == 0xc) {
                        IP_RenderBlokeIn3DNow(riders[i]);
                    }
                }
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
                for (i = 0; i < n; i++) {
                    if (riders[i]->param_action == 0xf) {
                        IP_RenderBlokeIn3DNow(riders[i]);
                    }
                }
                off = GetRenderOffsetForLayer((struct Sprite *)JoustLayer, 0);
                AdjustOffsetForViewMode(&off);
                PrintSprite(JoustSpecLMSprite, coords.x + off.x, coords.y + off.y, param_6, 0);
                for (i = 0; i < n; i++) {
                    if (riders[i]->param_action == 0x16) {
                        IP_RenderBlokeIn3DNow(riders[i]);
                    }
                }
                for (i = 0; i < n; i++) {
                    if (riders[i]->param_action == 0x17) {
                        IP_RenderBlokeIn3DNow(riders[i]);
                    }
                }
                for (i = 0; i < n; i++) {
                    if (riders[i]->param_action == 0x1d) {
                        IP_RenderBlokeIn3DNow(riders[i]);
                    }
                }
                LLSSetFrame(GetLLSForLayer((struct Sprite *)JoustLayer, 1), frame);
                off = GetRenderOffsetForLayer((struct Sprite *)JoustLayer, 1);
                AdjustOffsetForViewMode(&off);
                PrintSprite(GetSpriteForLayer((struct Sprite *)JoustLayer, 1), coords.x + off.x, coords.y + off.y, param_6, 0);
                for (r = ride->riders; r != NULL; r = r->next) {
                    if (tile->id == r->tile.id) {
                        bloke = r->rider;
                        if (bloke->flags & 0x80) {
                            struct Point poff;
                            person = bloke->person;
                            poff.x = 0;
                            poff.y = -48;
                            person->offset.x = bloke->screen_x;
                            person->offset.y = bloke->screen_y;
                            AdjustBlokePosition(&person->offset);
                            AdjustOffsetForViewMode(&poff);
                            person->screen.x = bloke->screen_x + poff.x + coords.x;
                            person->screen.y = bloke->screen_y + coords.y + poff.y;
                            AdjustBlokePosition(&person->screen);
                            IP_RenderBlokeIn3DNow(r->rider);
                        }
                    }
                }
                for (i = 0; i < n; i++) {
                    if (riders[i]->param_action == 0x10) {
                        IP_RenderBlokeIn3DNow(riders[i]);
                    }
                }
                for (i = 0; i < n; i++) {
                    if (riders[i]->param_action == 0xb) {
                        IP_RenderBlokeIn3DNow(riders[i]);
                    }
                }
                for (i = 0; i < n; i++) {
                    if (riders[i]->param_action == 0xa) {
                        IP_RenderBlokeIn3DNow(riders[i]);
                    }
                }
                for (i = 0; i < n; i++) {
                    if (riders[i]->param_action == 1) {
                        IP_RenderBlokeIn3DNow(riders[i]);
                    }
                }
                for (i = 0; i < n; i++) {
                    if (riders[i]->param_action == 2) {
                        IP_RenderBlokeIn3DNow(riders[i]);
                    }
                }
                for (i = 0; i < n; i++) {
                    if (riders[i]->param_action == 3) {
                        IP_RenderBlokeIn3DNow(riders[i]);
                    }
                }
                for (i = 0; i < n; i++) {
                    if (riders[i]->param_action == 7) {
                        IP_RenderBlokeIn3DNow(riders[i]);
                    }
                }
                for (i = 0; i < n; i++) {
                    if (riders[i]->param_action == 0x14) {
                        IP_RenderBlokeIn3DNow(riders[i]);
                    }
                }
                for (i = 0; i < n; i++) {
                    if (riders[i]->param_action == 0x15) {
                        IP_RenderBlokeIn3DNow(riders[i]);
                    }
                }
                for (i = 0; i < n; i++) {
                    if (riders[i]->param_action == 0x1e) {
                        IP_RenderBlokeIn3DNow(riders[i]);
                    }
                }
                off = GetRenderOffsetForLayer((struct Sprite *)JoustLayer, 1);
                AdjustOffsetForViewMode(&off);
                PrintSprite(JoustFMaskSprite, coords.x + off.x, coords.y + off.y, param_6, 0);
                return;
            }
        }
        LLSSetFrame(GetLLSForLayer((struct Sprite *)JoustLayer, 1), frame);
        off = GetRenderOffsetForLayer((struct Sprite *)JoustLayer, 1);
        AdjustOffsetForViewMode(&off);
        PrintSprite(GetSpriteForLayer((struct Sprite *)JoustLayer, 1), coords.x + off.x, coords.y + off.y, param_6, 0);
    }
}

// FUNCTION: LEGOLAND 0x00408bc0
void JoustSetEditMode(void) {
    EditMode.unk0 = 1;
    EditMode.unk8 = (void *)JoustRide;
    DefaultCursor(&EditCursor);
    SetEditCursorFootPrint((char *)EditMode.unk8 + 0x3c);
}

// FUNCTION: LEGOLAND 0x00408c00
void JoustFreeResources(void) {
    Kill_FXList(JOUST_SFX, 1);
    KillSprite(JoustFMaskSprite);
    KillSprite(JoustSpecRMSprite);
    KillSprite(JoustSpecLMSprite);
    FreeBinV(JoustRideBnv);
    KillSprite(DAT_004c1240);
    FreeJoustNodeList();
}

// FUNCTION: LEGOLAND 0x00408c50
unsigned int *FUN_00408c50(struct JoustRoot *param1, unsigned short param2) {
    struct JoustBlock *block = param1->field_c;

    DAT_004c1228.sprite = block->layer;
    DAT_004c1228.x = block->field_14;
    DAT_004c1228.y = block->field_18;
    DAT_004c1228.id = param2;
    block->layer->field_10 |= 0x2000;

    return (void *)&DAT_004c1228;
}

// FUNCTION: LEGOLAND 0x00408c90
LEGO_EXPORT int SaveJoust(void) {
    struct JoustNode *current = JoustNodeList;
    unsigned int flag = 1;
    unsigned int terminator = 0;

    while (current != NULL) {
        if (!SaveGameWrite(&flag, 4)) {
            return 0;
        }
        if (!SaveGameWrite(current, 0x24)) {
            return 0;
        }
        current = current->next;
    }

    if (SaveGameWrite(&terminator, 4)) {
        return 1;
    }
    return 0;
}

struct JoustCar {
    unsigned char pad_0[0x2c];
    void *field_2c;
    unsigned int field_30;
};

struct JoustListNode {
    struct JoustListNode *next;
    unsigned char pad_4[0xc];
    struct JoustCar *person;
};

struct JoustGameObject {
    unsigned char pad_0[0xcc];
    struct JoustListNode *riders;
};

struct JoustLoadArg {
    unsigned char pad_0[0xc];
    struct JoustGameObject *field_c;
};

// FUNCTION: LEGOLAND 0x00408d00
LEGO_EXPORT int LoadJoust(struct JoustLoadArg *arg) {
    struct JoustGameObject *obj = arg->field_c;
    struct JoustNode *prev = NULL;
    struct JoustListNode *list;
    struct JoustCar *car;
    unsigned int marker;

    if (!SaveGameRead(&marker, 4)) {
        return 0;
    }
    while (marker != 0) {
        struct JoustNode *node = (struct JoustNode *)malloc(0x24);
        if (!SaveGameRead(node, 0x24)) {
            return 0;
        }
        node->next = NULL;
        if (prev != NULL) {
            prev->next = node;
        } else {
            JoustNodeList = node;
        }
        node->sample = 0;
        prev = node;
        if (!SaveGameRead(&marker, 4)) {
            return 0;
        }
    }

    list = obj->riders;
    while (list != NULL) {
        car = list->person;
        if (car->field_30 != 0) {
            car->field_2c = DAT_004c123c[car->field_30];
        } else {
            car->field_2c = NULL;
            list->person->field_30 = 0;
        }
        list = list->next;
    }
    return 1;
}

// FUNCTION: LEGOLAND 0x00408db0
LEGO_EXPORT void Joust_GetInterfaces(struct ClassNode *head, struct CallbackTable *iface) {
    // STRING: LEGOLAND 0x004b4718
    if (_stricmp("JOUST", head->name) == 0) {
        iface->cb_a4 = JoustLoadResources;
        iface->cb_ac = JoustFreeResources;
        iface->cb_8c = JoustSetEditMode;
        iface->cb_a8 = FUN_00407c30;
        iface->cb_b0 = RenderJoust;
        iface->cb_9c = JoustRemoveObject;
        iface->cb_98 = JoustAddObject;
        iface->cb_a0 = FUN_00408c50;
        iface->cb_bc = SaveJoust;
        iface->cb_b8 = LoadJoust;
    }
}
