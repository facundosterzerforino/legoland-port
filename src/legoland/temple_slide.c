#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "legoland.h"

#include "binv.h"
#include "bloke.h"
#include "gamemap.h"
#include "globals.h"
#include "llidb.h"
#include "man3d.h"
#include "map_object.h"
#include "math.h"
#include "obj_instance.h"
#include "objclass.h"
#include "print_sprite.h"
#include "render3d.h"
#include "temple_slide.h"

struct TempleRide {
    unsigned char pad_0[4];
    unsigned int var_4;
    unsigned char pad_8[4];
    unsigned int var_c;
};

struct SlideNode {
    unsigned short key;
    unsigned char pad_2[6];
    struct SlideNode *next;
    unsigned char pad_c[4];
    unsigned int slots[4];
};

struct SlideCar {
    unsigned char pad_0[0x10];
    unsigned int var_10;
};

struct SlideTrack {
    unsigned char pad_0[0x14];
    unsigned int var_14;
    unsigned int var_18;
    unsigned int var_1c;
    unsigned char pad_20[0x44];
    struct SlideCar *var_64;
};

struct SlideContext {
    unsigned char pad_0[0xc];
    struct SlideTrack *var_c;
};

struct BlokeRender {
    unsigned char pad_0[0x2c];
    unsigned int field_2c;
    unsigned int field_30;
};

struct SlidePath {
    unsigned int field_0;
    unsigned int field_4;
};

struct BlokeData {
    unsigned char pad_0[0x54];
    struct SlidePath *path;
};

struct SlideRideNode {
    struct SlideRideNode *next;
    unsigned char pad_4[0x8 - 0x4];
    struct BlokeData *bloke;
    unsigned char pad_c[0x10 - 0xc];
    struct BlokeRender *render;
};

struct SlideRide {
    unsigned char pad_0[0xcc];
    struct SlideRideNode *blokes;
};

struct SlideObject {
    unsigned char pad_0[0xc];
    struct SlideRide *ride;
};

#include "image_sprite.h"

// FUNCTION: LEGOLAND 0x00416ec0
void FUN_00416ec0(TileId *key) {
    struct SlideNode *node = (struct SlideNode *)malloc(0x20);
    if (node != NULL) {
        memset(node, 0, 0x20);
        node->key = key->id;
        node->next = SlideNodeList;
        SlideNodeList = node;
        FUN_00417130((struct TempleRide *)node);
    }
}

// FUNCTION: LEGOLAND 0x00416f00
void RemoveSlideNode(struct SlideNode *node) {
    struct SlideNode *prev;
    struct SlideNode *cur;

    if (SlideNodeList == node) {
        SlideNodeList = node->next;
    } else {
        cur = SlideNodeList->next;
        prev = SlideNodeList;
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

// FUNCTION: LEGOLAND 0x00416f60
struct SlideNode *FindSlideNode(void *arg) {
    struct SlideNode *cur = SlideNodeList;

    if (cur != NULL) {
        do {
            if (memcmp(&cur->key, arg, 2) == 0) {
                return cur;
            }
            cur = cur->next;
        } while (cur != NULL);
    }
    return NULL;
}

// FUNCTION: LEGOLAND 0x00416f90
void FUN_00416f90(struct TempleRide *arg) {
    arg->var_c |= 1;
}

// FUNCTION: LEGOLAND 0x00416fa0
void FUN_00416fa0(Element *obj, unsigned int param_2, unsigned int param_3, unsigned short *tile, unsigned int param_5, unsigned int clip) {
    struct Ride *ride = obj->ride;
    struct RideNode *node;
    struct Point pos;
    struct Point off1;

    FindSlideNode(tile);
    pos = GetScreenCoordsForObject((TileId *)tile, ride);
    for (node = ride->riders; node != NULL; node = node->next) {
        if (*tile == node->tile.id && (node->rider->flags & 0x80) == 0) {
            IP_RenderBlokeIn3DNow(node->rider);
        }
    }
    PrintSprite(TempSlideMatteSprite, pos.x, pos.y, clip, 0);
    RenderItems_New();
    DAT_004cbf84 = NULL;
    for (node = ride->riders; node != NULL; node = node->next) {
        if (*tile == node->tile.id && (node->rider->flags & 0x80) != 0) {
            struct Bloke *bloke = node->rider;
            struct Person *person;

            struct Point off2;

            off1.x = DAT_004cbfc8;
            off1.y = DAT_004cbfcc[0];
            AdjustOffsetForViewMode(&off1);
            off2.x = DAT_004cbf88;
            off2.y = DAT_004cbf8c;
            AdjustOffsetForViewMode(&off2);
            person = bloke->person;
            person->offset.x = bloke->screen_x;
            person->offset.y = bloke->screen_y;
            person->offset.x += off1.x;
            person->offset.y += off1.y;
            AdjustBlokePosition(&person->offset);
            person->screen.x = bloke->screen_x + off1.x + off2.x + pos.x;
            person->screen.y = bloke->screen_y + off1.y + off2.y + pos.y;
            AdjustBlokePosition(&person->screen);
            AddBlokeToRenderList(&DAT_004cbf84, (struct BlokeRenderSrc *)node, node->person->field_20);
        }
    }
    RenderBlokeList((struct BlokeListHead *)&DAT_004cbf84);
}

// FUNCTION: LEGOLAND 0x00417130
void FUN_00417130(struct TempleRide *arg) {
    arg->var_4 = 0;
    arg->var_c &= 0xfffffffe;
}

// FUNCTION: LEGOLAND 0x00417150
void FUN_00417150(struct SlideContext *arg) {
    DAT_004cbf80 = arg->var_c;
    DAT_004cbf80->var_1c |= 0x20;
    DAT_004cbf7c = DAT_004cbf80->var_64;
    DAT_004cbf7c->var_10 |= 0x2000;
    ZTempSlideSprite = LoadSprite(
        // STRING: LEGOLAND 0x004b4f7c
        "z_tempslide.lls", 1);
    TempSlideBinV = LoadBinV(
        // STRING: LEGOLAND 0x004b4f64
        "Zbuffers\\tempslide.bnv");
    GetLLSForSprite((struct SpriteLLS *)DAT_004cbf80->var_64);
    DAT_004cbfc8 = 0;
    DAT_004cbfcc[0] = 1;
    DAT_004cbf88 = 13;
    DAT_004cbf8c = 93;
    TempSlideMatteSprite = LoadSprite(
        // STRING: LEGOLAND 0x004b4f50
        "tempslide_matte.lls", 1);
    DAT_004cbfb8[0] = (unsigned int)TempSlideBinV;
}

// FUNCTION: LEGOLAND 0x00417200
void FUN_00417200(struct SlideContext *arg) {
    DAT_004cbf80 = arg->var_c;
    if (TempSlideMatteSprite != 0) {
        KillSprite(TempSlideMatteSprite);
    }
    KillSprite(ZTempSlideSprite);
    FreeBinV(TempSlideBinV);
}

// FUNCTION: LEGOLAND 0x00417240
void FUN_00417240(void) {
    EditMode.unk0 = 1;
    EditMode.unk8 = (void *)DAT_004cbf80;
    DefaultCursor(&EditCursor);
    SetEditCursorFootPrint((char *)EditMode.unk8 + 0x3c);
}

// FUNCTION: LEGOLAND 0x00417280
void FUN_00417280(struct SlideObject *obj, TileId tile, struct Cursor *cursor) {
    struct SlideNode *node = FindSlideNode(&tile);
    if (node != NULL) {
        RemoveSlideNode(node);
    }
    StandardRemoveObject((Element *)obj, tile, cursor);
    RemoveAllBlokesFromRide((struct Ride *)obj->ride, tile);
}

// FUNCTION: LEGOLAND 0x004172d0
void FUN_004172d0(Element *obj, int *coords) {
    TileId tile;
    tile.pos.x = (unsigned char)coords[0];
    tile.pos.y = (unsigned char)coords[1];
    AddBasicObject(obj, coords);
    FUN_00416ec0(&tile);
}

// FUNCTION: LEGOLAND 0x00417300
unsigned int *FUN_00417300(struct SlideContext *ctx, unsigned short param) {
    struct SlideTrack *track = ctx->var_c;
    struct SlideCar *car = track->var_64;

    DAT_004cbf98 = (unsigned int)car;
    DAT_004cbf9c = track->var_14;
    DAT_004cbfa0 = track->var_18;
    DAT_004cbfa4 = param;

    car = track->var_64;
    car->var_10 |= 0x2000;

    return &DAT_004cbf98;
}

// FUNCTION: LEGOLAND 0x00417340
void FUN_00417340(void *arg) {
    unsigned int *array = FindSlideNode(arg)->slots;
    int i = 0;

    while (i < 4) {
        if (array[i] == 0) {
            Ride_ClearFlagToNotLetAnyoneOn(arg);
            return;
        }
        i++;
    }
    Ride_SetFlagToNotLetAnyoneOn(arg);
}

// FUNCTION: LEGOLAND 0x00417380
int FUN_00417380(void *arg) {
    int avail[4];
    int count;
    int pick;
    struct SlideNode *node = FindSlideNode(arg);

    if (node != NULL) {
        count = 0;
        if (node->slots[0] == 0) {
            avail[count] = count;
            count = 1;
        }
        if (node->slots[3] == 0) {
            avail[count++] = 3;
        }
        if (count != 0) {
            pick = avail[0];
            node->slots[pick] = 1;
            FUN_00417340(arg);
            return pick;
        }
        node->slots[0] = 1;
        FUN_00417340(arg);
        return 0;
    }
    return -1;
}

// FUNCTION: LEGOLAND 0x00417400
void FUN_00417400(unsigned int index, void *arg) {
    struct SlideNode *result = FindSlideNode(arg);

    if (result != NULL) {
        result->slots[index] = 0;
    }
    FUN_00417340(arg);
}

// FUNCTION: LEGOLAND 0x00417430
void FUN_00417430(Element *obj) {
    Ride *ride;
    Bloke *bloke;
    TileId *tile;
    unsigned int x;
    unsigned int y;
    unsigned char dir;
    int cx;
    int cy;
    int f;
    short sx;
    short sy;
    int off[4];
    Point coords;
    int dy;
    int dx;
    RideNode *next;
    int th;
    int tw;
    RideNode *node;

    ride = obj->ride;
    node = ride->riders;
    while (node != NULL) {
        next = node->next;
        bloke = node->rider;
        tile = &node->tile;
        x = ride->x + tile->pos.x;
        y = tile->pos.y + ride->y;
        if (bloke->low_level_action == 0) {
            switch (bloke->param_action) {
            case 0:
                bloke->flags |= 8;
                bloke->param_action++;
                bloke->field_36 = FUN_00417380(tile);
                if (bloke->field_36 > 1) {
                    x = (x << 8) + 0x380;
                    y = (y << 8) - 0x280;
                    bloke->dest.x = x;
                    bloke->dest.y = y;
                    dir = CalcMoveLine(bloke->pos, bloke->dest, &bloke->nav);
                    bloke->low_level_action = 7;
                    bloke->field_73 = dir + 0x10;
                    NewDirForAction(bloke, (bloke->field_73 >> 5) + 3);
                    bloke->param_action++;
                } else {
                    y -= 5;
                    x = (x << 8) + 0x80;
                    y <<= 8;
                    bloke->dest.x = x;
                    bloke->dest.y = y;
                    dir = CalcMoveLine(bloke->pos, bloke->dest, &bloke->nav);
                    bloke->low_level_action = 7;
                    bloke->field_73 = dir + 0x10;
                    NewDirForAction(bloke, (bloke->field_73 >> 5) + 3);
                    bloke->param_action = 3;
                }
                break;
            case 2:
                bloke->dest.x = (ride->footprint.x0 + tile->pos.x + 4) << 8;
                bloke->dest.y = (ride->footprint.y0 + tile->pos.y + 2) << 8;
                dir = CalcMoveLine(bloke->pos, bloke->dest, &bloke->nav);
                bloke->low_level_action = 7;
                bloke->field_73 = dir + 0x10;
                NewDirForAction(bloke, (bloke->field_73 >> 5) + 3);
                bloke->param_action++;
                break;
            case 3:
                coords = GetScreenCoordsForObject(tile, ride);
                {
                    int py = bloke->pos.y;
                    int px = bloke->pos.x;
                    GetTileDimensions(&tw, &th);
                    cy = (px + py) * th;
                    tw = (px - py) * tw;
                    tw >>= 9;
                    cy >>= 9;
                }
                sx = Get_XScroll();
                cx = (lpConfig->view_x - sx) + tw;
                sy = Get_YScroll();
                cy = cy + (lpConfig->view_y - sy);
                cx = cx - DAT_004cbfc8 / 2 - coords.x;
                bloke->flags |= 0x80;
                cy = cy - DAT_004cbfcc[0] / 2 - coords.y;
                off[0] = cx * 2;
                off[1] = cy * 2;
                bloke->person->sprite = ZTempSlideSprite;
                bloke->person->field_30 = 1;
                bloke->person->depth = GetUnitDepth(-1617664.875f, -1617913.0f);
                bloke->field_35 = 0;
                bloke->path = NewBNVPath((struct BinVFile *)DAT_004cbfb8[0], 0, DAT_004b4f08[bloke->field_36], -1617664.875f, -1617913.0f, off);
                BNVPath_SetDFrame(bloke, bloke->path, 0);
                UpdateBlokeFromBNVPath(bloke, bloke->path);
                bloke->param_action++;
                break;
            case 4:
                if (UpdateBlokeFromBNVPath(bloke, bloke->path) == 0) {
                    bloke->field_35 = 1;
                    bloke->param_action = 5;
                    free(bloke->path);
                    bloke->path = NULL;
                }
                BlokeSetFrame(bloke, bloke->frame);
                if (bloke->path != NULL) {
                    f = BNVPath_GetDFrame(bloke->path);
                    if (f == DAT_004b4f18[bloke->field_36]) {
                        BlokeWalkAnim(bloke);
                        bloke->field_44 = bloke->speed;
                        bloke->speed = 0x18;
                    } else if (f == DAT_004b4f1c[bloke->field_36]) {
                        bloke->field_35 = 1;
                        bloke->param_action = 5;
                        free(bloke->path);
                        bloke->path = NULL;
                    } else if (f >= DAT_004b4f18[bloke->field_36] && f <= DAT_004b4f1c[bloke->field_36]) {
                        BlokeSetFrame(bloke, 0);
                    }
                }
                break;
            case 5:
                BlokeWalkAnim(bloke);
                BlokeSetFrame(bloke, 0);
                bloke->flags &= 0xff7f;
                bloke->person->sprite = NULL;
                bloke->person->field_30 = 0;
                bloke->speed = (unsigned char)bloke->field_44;
                switch (bloke->field_36) {
                case 0:
                    dx = -0x280;
                    dy = 0x700;
                    break;
                case 1:
                    dx = -0x180;
                    dy = 0x180;
                    break;
                case 2:
                    dx = -0x180;
                    dy = 0;
                    break;
                case 3:
                    dx = -0x280;
                    dy = -0x500;
                    break;
                }
                x = ride->field_24 + tile->pos.x;
                y = ride->field_25 + tile->pos.y;
                bloke->pos.x = dx + (x << 8);
                bloke->pos.y = dy + (y << 8);
                bloke->dest.x = (x << 8) + 0x80;
                bloke->dest.y = (y << 8) + 0x80;
                dir = CalcMoveLine(bloke->pos, bloke->dest, &bloke->nav);
                bloke->low_level_action = 7;
                bloke->field_73 = dir + 0x10;
                NewDirForAction(bloke, (bloke->field_73 >> 5) + 3);
                bloke->param_action++;
                break;
            case 6:
                FUN_00417400(bloke->field_36, tile);
                RemoveBlokeFromRide(ride, node);
                bloke->flags &= 0xfff7;
                break;
            }
        }
        node = next;
    }
}

// FUNCTION: LEGOLAND 0x004178c0
LEGO_EXPORT int SaveTempleSlide(void) {
    int one = 1;
    int zero = 0;
    struct SlideNode *p;

    p = SlideNodeList;
    while (p != NULL) {
        if (SaveGameWrite(&one, 4) == 0) {
            return 0;
        }
        if (SaveGameWrite(p, 0x20) == 0) {
            return 0;
        }
        p = p->next;
    }
    return SaveGameWrite(&zero, 4) != 0;
}

// FUNCTION: LEGOLAND 0x00417930
LEGO_EXPORT int LoadTempleSlide(struct SlideObject *obj) {
    struct SlideRide *ride = obj->ride;
    struct SlideNode *last = NULL;
    struct SlideRideNode *node;
    unsigned int marker;

    if (!SaveGameRead(&marker, 4)) {
        return 0;
    }

    while (marker != 0) {
        struct SlideNode *save = malloc(0x20);
        if (!SaveGameRead(save, 0x20)) {
            return 0;
        }
        save->next = NULL;
        if (last != NULL) {
            last->next = save;
        } else {
            SlideNodeList = save;
        }
        last = save;
        if (!SaveGameRead(&marker, 4)) {
            return 0;
        }
    }

    for (node = ride->blokes; node != NULL; node = node->next) {
        struct BlokeRender *render = node->render;
        struct BlokeData *bloke;
        struct SlidePath *path;

        if (render->field_30 != 0) {
            render->field_2c = DAT_004cbfcc[render->field_30];
        } else {
            render->field_2c = 0;
            node->render->field_30 = 0;
        }

        bloke = node->bloke;
        path = bloke->path;
        if (path != NULL) {
            path->field_0 = DAT_004cbfb8[path->field_4];
        }
    }

    return 1;
}

// FUNCTION: LEGOLAND 0x00417a00
LEGO_EXPORT void TempleSlide_GetInterfaces(struct ClassNode *ctx, struct CallbackTable *interfaces) {
    // STRING: LEGOLAND 0x004b4f8c
    if (_stricmp("TEMPLE SLIDE", ctx->name) == 0) {
        interfaces->cb_a4 = FUN_00417150;
        interfaces->cb_ac = FUN_00417200;
        interfaces->cb_8c = FUN_00417240;
        interfaces->cb_a8 = FUN_00417430;
        interfaces->cb_b0 = FUN_00416fa0;
        interfaces->cb_9c = FUN_00417280;
        interfaces->cb_98 = FUN_004172d0;
        interfaces->cb_a0 = FUN_00417300;
        interfaces->cb_bc = SaveTempleSlide;
        interfaces->cb_b8 = LoadTempleSlide;
    }
}
