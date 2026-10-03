#include <stdlib.h>
#include <string.h>
#include "globals.h"
#include "legoland.h"

#include "bloke.h"
#include "gamemap.h"
#include "jungle_cruise.h"
#include "man3d.h"
#include "map_object.h"
#include "math.h"
#include "money.h"
#include "obj_instance.h"
#include "objclass.h"
#include "path_control.h"
#include "print_sprite.h"
#include "render3d.h"
#include "shops.h"
#include "tilemap.h"
#include "western_town.h"

struct Building {
    unsigned char pad_0[0x1c];
    unsigned int flags;
};

struct PathArea {
    unsigned char pad_0[0x3c];
    int x0;
    int y0;
    int x1;
    int y1;
};

struct MapObject {
    unsigned char pad_0[0xc];
    struct Building *building;
};

struct RideBuilding {
    unsigned char pad_0[0x10];
    unsigned int field_10;
    unsigned int field_14;
    unsigned int field_18;
    unsigned char pad_1c[0x64 - 0x1c];
    struct RideBuilding *layer;
};

struct ShopRideObject {
    unsigned char pad_0[0xc];
    struct RideBuilding *building;
};

struct ShopRemoveObject {
    unsigned char pad_0[0xc];
    void *ride;
};

struct BlokeNode {
    struct BlokeNode *next;
    unsigned char pad_4[4];
    struct Bloke *bloke;
    unsigned short uid;
};

struct ShopBuilding {
    unsigned char pad_0[0xcc];
    struct BlokeNode *blokes;
};

struct ShopObject {
    unsigned char pad_0[0xc];
    struct ShopBuilding *building;
};

struct Coords {
    int x;
    int y;
};

#include "image_sprite.h"

// FUNCTION: LEGOLAND 0x00439200
void LoadLegoShop1MatteSpriteAndMoneySFX(struct MapObject *obj) {
    struct Building *building = obj->building;
    LegoShop1Building = building;
    building->flags |= 0x420;
    // STRING: LEGOLAND 0x004b751c
    LegoShop1MatteSprite = LoadSprite("Lego Shop 1 Matte.LLS", 1);
    LoadMoneySFX();
}

// FUNCTION: LEGOLAND 0x00439230
void FUN_00439230(struct PathArea *area, struct Point *origin) {
    int xend = area->x1 + origin->x;
    int x = area->x0 + origin->x;
    int y = area->y0 + origin->y;
    int yend = area->y1 + origin->y;

    if (x > xend)
        return;

    do {
        if (y <= yend) {
            struct Point coord;
            do {
                coord.x = x;
                coord.y = y;
                AddPathTileGFX(&coord, *(unsigned short *)PathSprite);
                y++;
            } while (y <= yend);
        }
        y = area->y0 + origin->y;
        x++;
    } while (x <= xend);
}

// FUNCTION: LEGOLAND 0x004392b0
void FUN_004392b0(struct PathArea *area, struct Point *origin) {
    int xend = area->x1 + origin->x;
    int x = area->x0 + origin->x;
    int y = area->y0 + origin->y;
    int yend = area->y1 + origin->y;

    if (x > xend)
        return;

    do {
        if (y <= yend) {
            int coord[2];
            do {
                coord[0] = x;
                coord[1] = y;
                RemoveRollerCoasterPath(coord);
                y++;
            } while (y <= yend);
        }
        y = area->y0 + origin->y;
        x++;
    } while (x <= xend);
}

// FUNCTION: LEGOLAND 0x00439320
void LegoShop1AddObject(struct MapObject *obj, void *param_2) {
    struct Building *building = obj->building;
    AddBasicObject((unsigned int)obj, (unsigned int)param_2);
    FUN_00439230((unsigned int)building, param_2);
}

// FUNCTION: LEGOLAND 0x00439350
void LegoShop1RemoveObject(struct ShopRemoveObject *obj, TileId coords, void *cursor) {
    void *ride = obj->ride;
    struct Point local;

    StandardRemoveObject((Element *)obj, coords, (struct Cursor *)cursor);
    RemoveAllBlokesFromRide((struct Ride *)ride, coords);

    local.x = coords.pos.x;
    local.y = coords.pos.y;
    FUN_004392b0((struct PathArea *)ride, &local);
}

// FUNCTION: LEGOLAND 0x004393a0
void LegoShop1SetEditMode(void) {
    EditMode.unk0 = 1;
    EditMode.unk8 = LegoShop1Building;
    DefaultCursor(&EditCursor);
    SetEditCursorFootPrint((char *)EditMode.unk8 + 0x3c);
}

// FUNCTION: LEGOLAND 0x004393e0
void UnloadLegoShop1MatteSpriteAndMoneySFX(void) {
    if (LegoShop1MatteSprite != 0) {
        KillSprite(LegoShop1MatteSprite);
    }
    KillMoneySFX();
}

// FUNCTION: LEGOLAND 0x00439400
void RenderLegoShop1(struct ShopObject *obj, unsigned int param2, unsigned int param3, unsigned short *ride, unsigned int param5, unsigned int param1) {
    struct ShopBuilding *building = obj->building;
    struct BlokeNode *node = building->blokes;
    int count = 0;

    while (node != 0) {
        if (*ride == node->uid) {
            RenderBlokeIn3D(node->bloke);
            count++;
        }
        node = node->next;
    }

    if (count != 0) {
        struct Point q = GetScreenCoordsForObject((unsigned char *)ride, building);
        struct Coords *coords = (struct Coords *)&q;
        PrintSprite(LegoShop1MatteSprite, coords->x, coords->y, param1, (int *)node);
    }
}

// FUNCTION: LEGOLAND 0x00439460
void FUN_00439460(Element *obj) {
    Ride *ride = obj->ride;
    RideNode *node = ride->riders;
    RideNode *next;
    Bloke *bloke;
    TileId *tile;
    unsigned int x;
    unsigned int y;
    unsigned char dir;

    while (node != NULL) {
        next = node->next;
        bloke = node->rider;
        tile = &node->tile;
        x = tile->pos.x + ride->x;
        y = tile->pos.y + ride->y;
        if (bloke->low_level_action == 0) {
            switch (bloke->param_action) {
            case 0:
                bloke->flags |= 8;
                x = (x - 5) << 8;
                y <<= 8;
                bloke->dest.x = x;
                bloke->dest.y = y;
                dir = CalcMoveLine(bloke->pos, bloke->dest, &bloke->nav);
                bloke->low_level_action = 7;
                bloke->field_73 = dir + 0x10;
                NewDirForAction(bloke, (bloke->field_73 >> 5) + 3);
                bloke->param_action++;
                break;
            case 1:
                bloke->dest.x = (rand() % 2 + x - 5) << 8;
                bloke->dest.y = (rand() % 2 + y + 3) << 8;
                dir = CalcMoveLine(bloke->pos, bloke->dest, &bloke->nav);
                bloke->low_level_action = 7;
                bloke->field_73 = dir + 0x10;
                NewDirForAction(bloke, (bloke->field_73 >> 5) + 3);
                bloke->field_58 = rand() % 50;
                bloke->param_action++;
                break;
            case 2:
                bloke->field_58--;
                if (bloke->field_58 == 0) {
                    bloke->param_action++;
                } else if (bloke->field_58 % 10 == 0) {
                    bloke->dir++;
                    if (bloke->dir > 8) {
                        bloke->dir = 0;
                    }
                }
                break;
            case 3:
                bloke->dest.x = (x - 5) << 8;
                bloke->dest.y = (rand() % 3 + y) << 8;
                dir = CalcMoveLine(bloke->pos, bloke->dest, &bloke->nav);
                bloke->low_level_action = 7;
                bloke->field_73 = dir + 0x10;
                NewDirForAction(bloke, (bloke->field_73 >> 5) + 3);
                bloke->field_58 = rand() % 30;
                bloke->param_action++;
                break;
            case 4:
                FUN_00437570(node, obj, tile, 1);
                break;
            case 5:
                x <<= 8;
                bloke->dest.x = x;
                y <<= 8;
                bloke->dest.y = y;
                dir = CalcMoveLine(bloke->pos, bloke->dest, &bloke->nav);
                bloke->low_level_action = 7;
                bloke->field_73 = dir + 0x10;
                NewDirForAction(bloke, (bloke->field_73 >> 5) + 3);
                bloke->param_action++;
                break;
            case 6:
                RemoveBlokeFromRide(ride, node);
                bloke->flags &= ~8;
                break;
            }
        }
        node = next;
    }
}

// FUNCTION: LEGOLAND 0x004396d0
void LoadLegoShop2MatteSpriteAndMoneySFX(struct MapObject *obj) {
    struct Building *building = obj->building;
    LegoShop2Building = building;
    building->flags |= 0x420;
    // STRING: LEGOLAND 0x004b7534
    LegoShop2MatteSprite = LoadSprite("Lego Shop 2 Matte.LLS", 1);
    LoadMoneySFX();
}

// FUNCTION: LEGOLAND 0x00439700
void UnloadLegoShop2MatteSpriteAndMoneySFX(void) {
    if (LegoShop2MatteSprite != 0) {
        KillSprite(LegoShop2MatteSprite);
    }
    KillMoneySFX();
}

// FUNCTION: LEGOLAND 0x00439720
void LegoShop2SetEditMode(void) {
    EditMode.unk0 = 1;
    EditMode.unk8 = LegoShop2Building;
    DefaultCursor(&EditCursor);
    SetEditCursorFootPrint((char *)EditMode.unk8 + 0x3c);
}

// FUNCTION: LEGOLAND 0x00439760
void RenderLegoShop2(Element *obj, unsigned int param_2, unsigned int param_3, unsigned short *tile, unsigned int param_5, unsigned int param_6) {
    struct Ride *shop = obj->ride;
    struct RideNode *node = shop->riders;
    struct Bloke *blokes[10] = {0};
    char count = 0;
    char i;
    struct Point off;
    struct Point pos;

    while (node != NULL) {
        if (*tile == node->tile.id) {
            blokes[count++] = node->rider;
        }
        node = node->next;
    }
    if (count == 0) {
        return;
    }
    for (i = 0; i < count; i++) {
        if (blokes[i]->param_action == 4) {
            IP_RenderBlokeIn3DNow(blokes[i]);
        }
    }
    for (i = 0; i < count; i++) {
        if (blokes[i]->param_action == 5) {
            IP_RenderBlokeIn3DNow(blokes[i]);
        }
    }
    for (i = 0; i < count; i++) {
        if (blokes[i]->param_action == 3) {
            IP_RenderBlokeIn3DNow(blokes[i]);
        }
    }
    for (i = 0; i < count; i++) {
        if (blokes[i]->param_action == 2) {
            IP_RenderBlokeIn3DNow(blokes[i]);
        }
    }
    for (i = 0; i < count; i++) {
        if (blokes[i]->param_action == 9) {
            IP_RenderBlokeIn3DNow(blokes[i]);
        }
    }
    for (i = 0; i < count; i++) {
        if (blokes[i]->param_action == 10) {
            IP_RenderBlokeIn3DNow(blokes[i]);
        }
    }
    for (i = 0; i < count; i++) {
        if (blokes[i]->param_action == 11) {
            IP_RenderBlokeIn3DNow(blokes[i]);
        }
    }
    off = GetRenderOffsetForLayer(shop->layer, 0);
    pos = GetScreenCoordsForObject((unsigned char *)tile, shop);
    AdjustOffsetForViewMode(&off);
    PrintSprite(LegoShop2MatteSprite, pos.x + off.x, pos.y + off.y, param_6, 0);
    for (i = 0; i < count; i++) {
        if (blokes[i]->param_action == 1) {
            IP_RenderBlokeIn3DNow(blokes[i]);
        }
    }
    for (i = 0; i < count; i++) {
        if (blokes[i]->param_action == 12) {
            IP_RenderBlokeIn3DNow(blokes[i]);
        }
    }
}

// FUNCTION: LEGOLAND 0x00439950
void FUN_00439950(Element *obj) {
    Ride *ride = obj->ride;
    RideNode *node = ride->riders;
    RideNode *next;
    Bloke *bloke;
    TileId *tile;
    unsigned int x;
    unsigned int y;
    unsigned char dir;
    int r;

    while (node != NULL) {
        next = node->next;
        bloke = node->rider;
        tile = &node->tile;
        x = tile->pos.x + ride->x;
        y = tile->pos.y + ride->y;
        if (bloke->low_level_action == 0) {
            switch (bloke->param_action) {
            case 0:
                bloke->flags |= 8;
                x = (x << 8) - 0x80;
                y <<= 8;
                bloke->dest.x = x;
                bloke->dest.y = y;
                dir = CalcMoveLine(bloke->pos, bloke->dest, &bloke->nav);
                bloke->low_level_action = 7;
                bloke->field_73 = dir + 0x10;
                NewDirForAction(bloke, (bloke->field_73 >> 5) + 3);
                bloke->param_action++;
                break;
            case 2:
                x = (x << 8) - 0x160;
                y = (y << 8) - 0x60;
                bloke->dest.x = x;
                bloke->dest.y = y;
                dir = CalcMoveLine(bloke->pos, bloke->dest, &bloke->nav);
                bloke->low_level_action = 7;
                bloke->field_73 = dir + 0x10;
                NewDirForAction(bloke, (bloke->field_73 >> 5) + 3);
                bloke->param_action++;
                break;
            case 3:
                r = rand() % 3;
                if (r == 2) {
                    x = (x - 3) << 8;
                    y = (y << 8) - 0x80;
                    bloke->dest.x = x;
                    bloke->dest.y = y;
                } else if (r == 1) {
                    bloke->dest.x = (x << 8) - 0x100;
                    y = (y - 2) << 8;
                    bloke->dest.y = y;
                } else {
                    x = (x - 3) << 8;
                    y = (y << 8) - 0x280;
                    bloke->dest.x = x;
                    bloke->dest.y = y;
                }
                dir = CalcMoveLine(bloke->pos, bloke->dest, &bloke->nav);
                bloke->low_level_action = 7;
                bloke->field_73 = dir + 0x10;
                NewDirForAction(bloke, (bloke->field_73 >> 5) + 3);
                bloke->param_action++;
                break;
            case 4:
                FUN_00437570(node, obj, tile, 1);
                break;
            case 5:
                r = rand() % 3;
                if (r == 2) {
                    bloke->pos.x = (x - 3) << 8;
                    bloke->pos.y = y << 8;
                    bloke->field_58 = rand() % 50;
                    bloke->param_action = 8;
                } else if (r == 1) {
                    bloke->pos.x = (x << 8) - 0x100;
                    bloke->pos.y = (y - 2) << 8;
                    bloke->field_58 = rand() % 50;
                    bloke->param_action = 8;
                } else {
                    bloke->pos.x = (x - 3) << 8;
                    bloke->pos.y = (y << 8) - 0x280;
                    bloke->param_action = 9;
                }
                break;
            case 8:
                bloke->field_58--;
                if (bloke->field_58 == 0) {
                    bloke->param_action++;
                }
                break;
            case 1:
            case 9:
                x = (x << 8) - 0x100;
                y = (y << 8) - 0x80;
                bloke->dest.x = x;
                bloke->dest.y = y;
                dir = CalcMoveLine(bloke->pos, bloke->dest, &bloke->nav);
                bloke->low_level_action = 7;
                bloke->field_73 = dir + 0x10;
                NewDirForAction(bloke, (bloke->field_73 >> 5) + 3);
                bloke->param_action++;
                break;
            case 10:
                x = (x << 8) - 0x80;
                y <<= 8;
                bloke->dest.x = x;
                bloke->dest.y = y;
                dir = CalcMoveLine(bloke->pos, bloke->dest, &bloke->nav);
                bloke->low_level_action = 7;
                bloke->field_73 = dir + 0x10;
                NewDirForAction(bloke, (bloke->field_73 >> 5) + 3);
                bloke->param_action++;
                break;
            case 11:
                x <<= 8;
                bloke->dest.x = x;
                y <<= 8;
                bloke->dest.y = y;
                dir = CalcMoveLine(bloke->pos, bloke->dest, &bloke->nav);
                bloke->low_level_action = 7;
                bloke->field_73 = dir + 0x10;
                NewDirForAction(bloke, (bloke->field_73 >> 5) + 3);
                bloke->param_action++;
            case 12:
                RemoveBlokeFromRide(ride, node);
                bloke->flags &= ~8;
                break;
            }
        }
        node = next;
    }
}

// FUNCTION: LEGOLAND 0x00439c20
void LoadLegMediaShopMaskSpritesAndMoneySFX(struct MapObject *obj) {
    struct Building *building = obj->building;
    LegoMediaShopBuilding = building;
    building->flags |= 0x420;
    // STRING: LEGOLAND 0x004b7564
    LegMediaShopMask1Sprite = LoadSprite("LegMediaShopMask1.LLS", 1);
    // STRING: LEGOLAND 0x004b754c
    LegMediaShopMask2Sprite = LoadSprite("LegMediaShopMask2.LLS", 1);
    LoadMoneySFX();
}

// FUNCTION: LEGOLAND 0x00439c60
void LegoMediaShopAddObject(struct MapObject *obj, void *param_2) {
    struct Building *building = obj->building;
    AddBasicObject((unsigned int)obj, (unsigned int)param_2);
    FUN_00439230((unsigned int)building, param_2);
}

// FUNCTION: LEGOLAND 0x00439c90
void LegoMediaShopRemoveObject(struct ShopRemoveObject *obj, TileId coords, void *cursor) {
    void *ride = obj->ride;
    struct Point local;

    StandardRemoveObject((Element *)obj, coords, (struct Cursor *)cursor);
    RemoveAllBlokesFromRide((struct Ride *)ride, coords);

    local.x = coords.pos.x;
    local.y = coords.pos.y;
    FUN_004392b0((struct PathArea *)ride, &local);
}

// FUNCTION: LEGOLAND 0x00439ce0
void UnloadLegMediaShopMaskSpritesAndMoneySFX(void) {
    KillSprite(LegMediaShopMask1Sprite);
    KillSprite(LegMediaShopMask2Sprite);
    KillMoneySFX();
}

// FUNCTION: LEGOLAND 0x00439d00
void LegoMediaShopSetEditMode(void) {
    EditMode.unk0 = 1;
    EditMode.unk8 = LegoMediaShopBuilding;
    DefaultCursor(&EditCursor);
    SetEditCursorFootPrint((char *)EditMode.unk8 + 0x3c);
}

// FUNCTION: LEGOLAND 0x00439d40
void RenderLegoMediaShop(struct ShopObject *obj, unsigned int param2, unsigned int param3, unsigned short *ride, unsigned int param5, unsigned int param1) {
    struct ShopBuilding *building = obj->building;
    char count = 0;
    char i;
    struct BlokeNode *node = building->blokes;
    struct Bloke *blokes[10] = {0};
    struct Point pos;

    while (node != 0) {
        if (*ride == node->uid) {
            blokes[count++] = node->bloke;
        }
        node = node->next;
    }
    if (count == 0) {
        return;
    }
    pos = GetScreenCoordsForObject((unsigned char *)ride, building);
    for (i = 0; i < count; i++) {
        if (blokes[i]->param_action == 2) {
            IP_RenderBlokeIn3DNow(blokes[i]);
        }
    }
    for (i = 0; i < count; i++) {
        if (blokes[i]->param_action == 3) {
            IP_RenderBlokeIn3DNow(blokes[i]);
        }
    }
    for (i = 0; i < count; i++) {
        if (blokes[i]->param_action == 4) {
            IP_RenderBlokeIn3DNow(blokes[i]);
        }
    }
    for (i = 0; i < count; i++) {
        if (blokes[i]->param_action == 5) {
            IP_RenderBlokeIn3DNow(blokes[i]);
        }
    }
    PrintSprite(LegMediaShopMask2Sprite, pos.x, pos.y, param1, 0);
    for (i = 0; i < count; i++) {
        if (blokes[i]->param_action == 0) {
            IP_RenderBlokeIn3DNow(blokes[i]);
        }
    }
    for (i = 0; i < count; i++) {
        if (blokes[i]->param_action == 1) {
            IP_RenderBlokeIn3DNow(blokes[i]);
        }
    }
    for (i = 0; i < count; i++) {
        if (blokes[i]->param_action == 6) {
            IP_RenderBlokeIn3DNow(blokes[i]);
        }
    }
    PrintSprite(LegMediaShopMask1Sprite, pos.x, pos.y, param1, 0);
}

// FUNCTION: LEGOLAND 0x00439ef0
void FUN_00439ef0(Element *obj) {
    Ride *ride = obj->ride;
    RideNode *node = ride->riders;
    RideNode *next;
    Bloke *bloke;
    TileId *tile;
    unsigned int x;
    unsigned int y;
    unsigned char dir;
    char r;

    while (node != NULL) {
        next = node->next;
        bloke = node->rider;
        tile = &node->tile;
        x = tile->pos.x + ride->x;
        y = tile->pos.y + ride->y;
        if (bloke->low_level_action == 0) {
            switch (bloke->param_action) {
            case 0:
                bloke->flags |= 8;
                x = (x - 2) << 8;
                y <<= 8;
                bloke->dest.x = x;
                bloke->dest.y = y;
                dir = CalcMoveLine(bloke->pos, bloke->dest, &bloke->nav);
                bloke->low_level_action = 7;
                bloke->field_73 = dir + 0x10;
                NewDirForAction(bloke, (bloke->field_73 >> 5) + 3);
                bloke->param_action++;
                break;
            case 1:
                r = rand() % 3;
                if (r == 2) {
                    x = (x - 4) << 8;
                    y <<= 8;
                    bloke->dest.x = x;
                    bloke->dest.y = y;
                } else if (r == 1) {
                    bloke->dest.x = (x - 3) << 8;
                    y = (y - 2) << 8;
                    bloke->dest.y = y;
                } else {
                    x = (x - 4) << 8;
                    y = (y - 2) << 8;
                    bloke->dest.x = x;
                    bloke->dest.y = y;
                }
                dir = CalcMoveLine(bloke->pos, bloke->dest, &bloke->nav);
                bloke->low_level_action = 7;
                bloke->field_73 = dir + 0x10;
                NewDirForAction(bloke, (bloke->field_73 >> 5) + 3);
                bloke->param_action++;
                break;
            case 2:
                FUN_00437570(node, obj, tile, 1);
                break;
            case 3:
                r = rand() % 3;
                if (r == 2) {
                    bloke->pos.x = (x - 4) << 8;
                    bloke->pos.y = y << 8;
                    bloke->param_action++;
                } else if (r == 1) {
                    bloke->pos.x = (x - 3) << 8;
                    bloke->pos.y = (y - 2) << 8;
                    bloke->param_action++;
                } else {
                    bloke->pos.x = (x - 4) << 8;
                    bloke->pos.y = (y - 2) << 8;
                    bloke->param_action++;
                }
                break;
            case 4:
                x = (x - 2) << 8;
                y <<= 8;
                bloke->dest.x = x;
                bloke->dest.y = y;
                dir = CalcMoveLine(bloke->pos, bloke->dest, &bloke->nav);
                bloke->low_level_action = 7;
                bloke->field_73 = dir + 0x10;
                NewDirForAction(bloke, (bloke->field_73 >> 5) + 3);
                bloke->param_action++;
                break;
            case 5:
                x <<= 8;
                bloke->dest.x = x;
                y <<= 8;
                bloke->dest.y = y;
                dir = CalcMoveLine(bloke->pos, bloke->dest, &bloke->nav);
                bloke->low_level_action = 7;
                bloke->field_73 = dir + 0x10;
                NewDirForAction(bloke, (bloke->field_73 >> 5) + 3);
                bloke->param_action++;
                break;
            case 6:
                RemoveBlokeFromRide(ride, node);
                bloke->flags &= ~8;
                break;
            }
        }
        node = next;
    }
}

// FUNCTION: LEGOLAND 0x0043a0f0
void LoadExplorersInstituteMatteSpriteAndMoneySFX(struct MapObject *obj) {
    struct Building *building = obj->building;
    ExplorersInstituteBuilding = building;
    building->flags |= 0x420;
    // STRING: LEGOLAND 0x004b757c
    ExplorersInstituteMatteSprite = LoadSprite("Explorers Institute Matte.LLS", 1);
    LoadMoneySFX();
}

// FUNCTION: LEGOLAND 0x0043a120
void UnloadExplorersInstituteMatteSpriteAndMoneySFX(void) {
    KillSprite(ExplorersInstituteMatteSprite);
    KillMoneySFX();
}

// FUNCTION: LEGOLAND 0x0043a140
void ExplorersInstituteSetEditMode(void) {
    EditMode.unk0 = 1;
    EditMode.unk8 = ExplorersInstituteBuilding;
    DefaultCursor(&EditCursor);
    SetEditCursorFootPrint((char *)EditMode.unk8 + 0x3c);
}

// FUNCTION: LEGOLAND 0x0043a180
void RenderExplorersInstitute(struct ShopObject *obj, unsigned int param2, unsigned int param3, unsigned short *ride, unsigned int param5, unsigned int param1) {
    struct ShopBuilding *building = obj->building;
    struct BlokeNode *node = building->blokes;
    int count = 0;

    while (node != 0) {
        if (*ride == node->uid) {
            RenderBlokeIn3D(node->bloke);
            count++;
        }
        node = node->next;
    }

    if (count != 0) {
        struct Point q = GetScreenCoordsForObject((unsigned char *)ride, building);
        struct Coords *coords = (struct Coords *)&q;
        PrintSprite(ExplorersInstituteMatteSprite, coords->x, coords->y, param1, (int *)node);
    }
}

// FUNCTION: LEGOLAND 0x0043a1e0
void FUN_0043a1e0(struct Element *obj) {
    struct Ride *ride = obj->ride;
    struct RideNode *elem = ride->riders;
    struct RideNode *next;
    struct Bloke *bloke;
    int x;
    int y;

    while (elem != NULL) {
        next = elem->next;
        bloke = elem->rider;
        if (bloke->low_level_action == 0) {
            switch (bloke->param_action) {
            case 0:
                bloke->flags |= 8;
                bloke->dest.x = elem->tile.pos.x << 8;
                bloke->dest.y = (elem->tile.pos.y + 1) << 8;
                bloke->field_73 = CalcMoveLine(bloke->pos, bloke->dest, &bloke->nav) + 0x10;
                bloke->low_level_action = 7;
                NewDirForAction(bloke, (bloke->field_73 >> 5) + 3);
                bloke->param_action++;
                break;
            case 1:
                bloke->dest.x = elem->tile.pos.x << 8;
                bloke->dest.y = (elem->tile.pos.y + 2) << 8;
                bloke->field_73 = CalcMoveLine(bloke->pos, bloke->dest, &bloke->nav) + 0x10;
                bloke->low_level_action = 7;
                NewDirForAction(bloke, (bloke->field_73 >> 5) + 3);
                bloke->param_action++;
                bloke->field_58 = rand() % 70 + 20;
                break;
            case 2:
                if (bloke->field_58 == 0) {
                    bloke->param_action++;
                }
                bloke->field_58--;
                break;
            case 3:
                bloke->dest.x = elem->tile.pos.x << 8;
                bloke->dest.y = (elem->tile.pos.y + 1) << 8;
                bloke->field_73 = CalcMoveLine(bloke->pos, bloke->dest, &bloke->nav) + 0x10;
                bloke->low_level_action = 7;
                NewDirForAction(bloke, (bloke->field_73 >> 5) + 3);
                bloke->param_action++;
                break;
            case 4:
                bloke->dest.y = ((elem->tile.pos.y + ride->y) << 8) + 0x80;
                bloke->dest.x = ((ride->x + elem->tile.pos.x) << 8) + 0x80;
                bloke->field_73 = CalcMoveLine(bloke->pos, bloke->dest, &bloke->nav) + 0x10;
                bloke->low_level_action = 7;
                NewDirForAction(bloke, (bloke->field_73 >> 5) + 3);
                bloke->param_action++;
                break;
            case 5:
                RemoveBlokeFromRide(ride, elem);
                bloke->flags &= 0xfff7;
                break;
            }
        }
        elem = next;
    }
}

// FUNCTION: LEGOLAND 0x0043a390
unsigned int *GetShopSpriteInfo(struct ShopRideObject *obj, unsigned short param_2) {
    struct RideBuilding *building = obj->building;
    RideSpriteInfoBuffer.sprite = building->layer;
    RideSpriteInfoBuffer.x = building->field_14;
    RideSpriteInfoBuffer.y = building->field_18;
    RideSpriteInfoBuffer.id = param_2;
    building->layer->field_10 |= 0x2000;
    return &RideSpriteInfoBuffer;
}

// FUNCTION: LEGOLAND 0x0043a3d0
void RemoveObjectAndBlokes(struct ShopRideObject *obj, TileId tile, void *param_3) {
    StandardRemoveObject((unsigned int)obj, tile, (unsigned int)param_3);
    RemoveAllBlokesFromRide((unsigned int)obj->building, tile);
}

// FUNCTION: LEGOLAND 0x0043a400
void ShopsGetInterfaces(struct ClassNode *name, struct CallbackTable *ci) {
    // STRING: LEGOLAND 0x004b75fc
    if (_stricmp("GENERAL STORE", name->name) == 0) {
        ci->cb_a4 = LoadGStoreMatteSpritesAndMoneySFX;
        ci->cb_ac = KillGStoreMatteSpritesAndMoneySFX;
        ci->cb_8c = GeneralStoreSetEditMode;
        ci->cb_a0 = GetShopSpriteInfo;
        ci->cb_a8 = FUN_004378e0;
        ci->cb_9c = RemoveObjectAndBlokes;
        ci->cb_b0 = RenderGeneralStore;
        return;
    }
    // STRING: LEGOLAND 0x004b75f4
    if (_stricmp("SHERIFF", name->name) == 0) {
        ci->cb_a4 = LoadSherifshutMatteSpriteAndMoneySFX;
        ci->cb_ac = KillSherifshutMatteSpriteAndMoneySFX;
        ci->cb_8c = SheriffSetEditMode;
        ci->cb_a0 = GetShopSpriteInfo;
        ci->cb_a8 = FUN_00437c90;
        ci->cb_9c = RemoveObjectAndBlokes;
        ci->cb_b0 = RenderSheriff;
        return;
    }
    // STRING: LEGOLAND 0x004b75e8
    if (_stricmp("JAIL CELL", name->name) == 0) {
        ci->cb_a4 = FUN_00438070;
        ci->cb_ac = FUN_004380f0;
        ci->cb_8c = JailCellSetEditMode;
        ci->cb_a0 = GetShopSpriteInfo;
        ci->cb_a8 = FUN_00438430;
        ci->cb_98 = JailCellAddObject;
        ci->cb_9c = JailCellRemoveObject;
        ci->cb_b0 = RenderJailCell;
        ci->cb_b8 = LoadJailCells;
        ci->cb_bc = SaveJailCells;
        return;
    }
    // STRING: LEGOLAND 0x004b75e0
    if (_stricmp("BANK", name->name) == 0) {
        ci->cb_a4 = LoadBankMatteSpriteAndMoneySFX;
        ci->cb_ac = KillBankMatteSpriteAndMoneySFX;
        ci->cb_8c = BankSetEditMode;
        ci->cb_a0 = GetShopSpriteInfo;
        ci->cb_a8 = FUN_00438960;
        ci->cb_9c = RemoveObjectAndBlokes;
        ci->cb_b0 = RenderBank;
        return;
    }
    // STRING: LEGOLAND 0x004b75d8
    if (_stricmp("SALOON", name->name) == 0) {
        ci->cb_a4 = LoadSaloonMatteSpritesAndMoneySFX;
        ci->cb_ac = KillSaloonMatteSpritesAndMoneySFX;
        ci->cb_8c = SaloonSetEditMode;
        ci->cb_a0 = GetShopSpriteInfo;
        ci->cb_a8 = FUN_00438f10;
        ci->cb_9c = RemoveObjectAndBlokes;
        ci->cb_b0 = RenderSaloon;
        return;
    }
    // STRING: LEGOLAND 0x004b75c4
    if (_stricmp("EXPLORERS INSTITUTE", name->name) == 0) {
        ci->cb_a4 = LoadExplorersInstituteMatteSpriteAndMoneySFX;
        ci->cb_ac = UnloadExplorersInstituteMatteSpriteAndMoneySFX;
        ci->cb_8c = ExplorersInstituteSetEditMode;
        ci->cb_a0 = GetShopSpriteInfo;
        ci->cb_a8 = FUN_0043a1e0;
        ci->cb_9c = RemoveObjectAndBlokes;
        ci->cb_b0 = RenderExplorersInstitute;
        return;
    }
    // STRING: LEGOLAND 0x004b75b8
    if (_stricmp("LEGO SHOP 1", name->name) == 0) {
        ci->cb_a4 = LoadLegoShop1MatteSpriteAndMoneySFX;
        ci->cb_98 = LegoShop1AddObject;
        ci->cb_9c = LegoShop1RemoveObject;
        ci->cb_ac = UnloadLegoShop1MatteSpriteAndMoneySFX;
        ci->cb_8c = LegoShop1SetEditMode;
        ci->cb_a0 = GetShopSpriteInfo;
        ci->cb_a8 = FUN_00439460;
        ci->cb_b0 = RenderLegoShop1;
        return;
    }
    // STRING: LEGOLAND 0x004b75ac
    if (_stricmp("LEGO SHOP 2", name->name) == 0) {
        ci->cb_a4 = LoadLegoShop2MatteSpriteAndMoneySFX;
        ci->cb_ac = UnloadLegoShop2MatteSpriteAndMoneySFX;
        ci->cb_8c = LegoShop2SetEditMode;
        ci->cb_a0 = GetShopSpriteInfo;
        ci->cb_a8 = FUN_00439950;
        ci->cb_9c = RemoveObjectAndBlokes;
        ci->cb_b0 = RenderLegoShop2;
        return;
    }
    // STRING: LEGOLAND 0x004b759c
    if (_stricmp("LEGO MEDIA SHOP", name->name) == 0) {
        ci->cb_a4 = LoadLegMediaShopMaskSpritesAndMoneySFX;
        ci->cb_98 = LegoMediaShopAddObject;
        ci->cb_9c = LegoMediaShopRemoveObject;
        ci->cb_ac = UnloadLegMediaShopMaskSpritesAndMoneySFX;
        ci->cb_8c = LegoMediaShopSetEditMode;
        ci->cb_a0 = GetShopSpriteInfo;
        ci->cb_a8 = FUN_00439ef0;
        ci->cb_b0 = RenderLegoMediaShop;
    }
}
