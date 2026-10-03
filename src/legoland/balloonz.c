#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "globals.h"
#include "legoland.h"

#include "balloonz.h"
#include "binv.h"
#include "bloke.h"
#include "gamemap.h"
#include "image_sprite.h"
#include "llidb.h"
#include "man3d.h"
#include "map_object.h"
#include "math.h"
#include "obj_instance.h"
#include "print_sprite.h"
#include "render3d.h"

// FUNCTION: LEGOLAND 0x0042a7b0
void LoadBalloonzResources(Element *obj) {
    BalloonzRide = obj->ride;
    BalloonzRide->flags |= 0x420;
    BalloonzLayer = BalloonzRide->layer;
    BalloonzRide->flags |= 0x2000;
    // STRING: LEGOLAND 0x004b64ac
    BallBaseM1Sprite = LoadSprite("Ballbasem1.lls", 1);
    // STRING: LEGOLAND 0x004b649c
    BallBaseM2Sprite = LoadSprite("Ballbasem2.lls", 1);
    // STRING: LEGOLAND 0x004b648c
    BallBaseM3Sprite = LoadSprite("Ballbasem3.lls", 1);
    // STRING: LEGOLAND 0x004b647c
    BZRedCarM1Sprite = LoadSprite("BZRedCarM1.lls", 1);
    // STRING: LEGOLAND 0x004b6468
    BZGreenCarM1Sprite = LoadSprite("BZGreenCarM1.lls", 1);
    // STRING: LEGOLAND 0x004b6458
    BZBlueCarM1Sprite = LoadSprite("BZBlueCarM1.lls", 1);
    // STRING: LEGOLAND 0x004b6448
    ZBalloon2Sprite = LoadSprite("z_Balloon2.lls", 1);
    DAT_00616040 = ZBalloon2Sprite;
    // STRING: LEGOLAND 0x004b6430
    BalloonzBinV = LoadBinV("Zbuffers\\balloonz.bnv");
    DAT_00616018[0] = BalloonzBinV;
    HideLayer(BalloonzLayer, 2);
    StopLayerPlaying(BalloonzLayer, 2);
    LLSSetFrame(GetLLSForLayer(BalloonzLayer, 2), 0);
    HideLayer(BalloonzLayer, 1);
    StopLayerPlaying(BalloonzLayer, 1);
    LLSSetFrame(GetLLSForLayer(BalloonzLayer, 1), 0);
}

// FUNCTION: LEGOLAND 0x0042a8f0
void AddBalloonNode(TileId *tile) {
    BalloonNode *node;

    node = malloc(sizeof(BalloonNode));
    if (node != NULL) {
        memset(node, 0, sizeof(BalloonNode));
        node->tile = *tile;
        node->next = BalloonNodeList;
        node->queued = 0;
        node->riders = 0;
        memset(node->cars, 0, sizeof(node->cars));
        node->lap = 0;
        node->pos = 0;
        node->frame = 0;
        node->anim = 0;
        node->leaving = 0;
        node->can_board = 0;
        node->can_unload = 0;
        BalloonNodeList = node;
    }
}

// FUNCTION: LEGOLAND 0x0042a950
void BalloonzAddObject(Element *obj, int *coords) {
    TileId tile;

    tile.pos.x = coords[0];
    tile.pos.y = coords[1];
    AddBasicObject(obj, coords);
    AddBalloonNode(&tile);
}

// FUNCTION: LEGOLAND 0x0042a980
BalloonNode *FindBalloonNode(TileId *tile) {
    BalloonNode *node;

    node = BalloonNodeList;
    if (node == NULL) {
        return NULL;
    }
    while (memcmp(&node->tile, tile, sizeof(TileId)) != 0) {
        node = node->next;
        if (node == NULL) {
            return NULL;
        }
    }
    return node;
}

// FUNCTION: LEGOLAND 0x0042a9b0
void RemoveBalloonNode(BalloonNode *node) {
    BalloonNode *cur;
    BalloonNode *prev;

    if (BalloonNodeList == node) {
        BalloonNodeList = node->next;
    } else {
        cur = BalloonNodeList->next;
        prev = BalloonNodeList;
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

// FUNCTION: LEGOLAND 0x0042a9f0
void RemoveAllBalloonNodes(void) {
    BalloonNode *node;

    node = BalloonNodeList;
    if (node == NULL) {
        return;
    }
    do {
        RemoveBalloonNode(node);
        node = BalloonNodeList;
    } while (node != NULL);
}

// FUNCTION: LEGOLAND 0x0042aa10
void BalloonzRemoveObject(Element *obj, TileId tile, Cursor *cursor) {
    BalloonNode *node;

    node = FindBalloonNode(&tile);
    if (node != NULL) {
        RemoveBalloonNode(node);
    }
    StandardRemoveObject(obj, tile, cursor);
    RemoveAllBlokesFromRide(obj->ride, tile);
}

// FUNCTION: LEGOLAND 0x0042aa60
int FUN_0042aa60(char pos, char lap) {
    if (lap == 1 && pos > 23) {
        return 0;
    }
    return pos / 8 + lap * 3;
}

// FUNCTION: LEGOLAND 0x0042aa90
void FUN_0042aa90(Element *obj) {
    Ride *ride;
    RideNode *elem;
    RideNode *next;
    Bloke *bloke;
    BalloonNode *state;
    TileId *tile;
    unsigned int x;
    unsigned int y;
    int queued;
    char riders;
    char cars[6];
    char pos;
    char lap;
    char leaving;
    int can_board;
    int can_unload;
    char car;
    unsigned char dir;
    // STRING: LEGOLAND 0x004b64bc
    char buf[8] = "Bloke??";

    ride = obj->ride;
    elem = ride->riders;
    while (elem != NULL) {
        next = elem->next;
        bloke = elem->rider;
        tile = &elem->tile;
        state = FindBalloonNode(tile);
        if (state == NULL) {
            return;
        }
        riders = state->riders;
        queued = state->queued;
        memcpy(cars, state->cars, sizeof(cars));
        pos = state->pos;
        lap = state->lap;
        leaving = state->leaving;
        can_unload = state->can_unload;
        can_board = state->can_board;
        (*ZBalloon2Sprite->lls)->frame = state->frame;
        x = ride->x + tile->pos.x;
        y = tile->pos.y + ride->y;
        if (bloke->low_level_action == 0) {
            switch (bloke->param_action) {
            case 0:
                bloke->flags |= 8;
                y = (y << 8) + 0xfa;
                x = (x << 8) - 0x9c;
                bloke->dest.x = x;
                bloke->dest.y = y;
                dir = CalcMoveLine(bloke->pos, bloke->dest, &bloke->nav);
                bloke->low_level_action = 7;
                bloke->field_73 = dir + 0x10;
                NewDirForAction(bloke, (bloke->field_73 >> 5) + 3);
                bloke->field_5c = 500;
                bloke->param_action++;
                break;
            case 1:
                bloke->dest.x = (x << 8) - 0x100;
                y = (y + 7) << 8;
                bloke->dest.y = y;
                dir = CalcMoveLine(bloke->pos, bloke->dest, &bloke->nav);
                bloke->low_level_action = 7;
                bloke->field_73 = dir + 0x10;
                NewDirForAction(bloke, (bloke->field_73 >> 5) + 3);
                bloke->param_action++;
                break;
            case 2:
                bloke->dest.x = (x - 2) << 8;
                y += 7;
                y <<= 8;
                bloke->dest.y = y;
                dir = CalcMoveLine(bloke->pos, bloke->dest, &bloke->nav);
                bloke->low_level_action = 7;
                bloke->field_73 = dir + 0x10;
                NewDirForAction(bloke, (bloke->field_73 >> 5) + 3);
                bloke->param_action++;
                break;
            case 3:
                x = (x - 2) << 8;
                y = (y + 1) << 8;
                bloke->dest.x = x;
                bloke->dest.y = y;
                dir = CalcMoveLine(bloke->pos, bloke->dest, &bloke->nav);
                bloke->low_level_action = 7;
                bloke->field_73 = dir + 0x10;
                NewDirForAction(bloke, (bloke->field_73 >> 5) + 3);
                bloke->param_action++;
                break;
            case 4:
                x = (x - 4) << 8;
                y = (y + 1) << 8;
                bloke->dest.x = x;
                bloke->dest.y = y;
                dir = CalcMoveLine(bloke->pos, bloke->dest, &bloke->nav);
                bloke->low_level_action = 7;
                bloke->field_73 = dir + 0x10;
                NewDirForAction(bloke, (bloke->field_73 >> 5) + 3);
                bloke->param_action++;
                break;
            case 5:
                bloke->dest.x = (x - 6) << 8;
                y += 2;
                y <<= 8;
                bloke->dest.y = y;
                dir = CalcMoveLine(bloke->pos, bloke->dest, &bloke->nav);
                bloke->low_level_action = 7;
                bloke->field_73 = dir + 0x10;
                NewDirForAction(bloke, (bloke->field_73 >> 5) + 3);
                bloke->param_action++;
                break;
            case 6:
                y = (y + 3) << 8;
                x = (x << 8) - 0x632;
                bloke->dest.x = x;
                bloke->dest.y = y;
                dir = CalcMoveLine(bloke->pos, bloke->dest, &bloke->nav);
                bloke->low_level_action = 7;
                bloke->field_73 = dir + 0x10;
                NewDirForAction(bloke, (bloke->field_73 >> 5) + 3);
                bloke->param_action++;
                queued++;
                break;
            case 7:
                bloke->dir = 3;
                if (can_board == 1) {
                    y = (y + 3) << 8;
                    x = (x << 8) - 0x564;
                    bloke->dest.x = x;
                    bloke->dest.y = y;
                    dir = CalcMoveLine(bloke->pos, bloke->dest, &bloke->nav);
                    bloke->low_level_action = 7;
                    bloke->field_73 = dir + 0x10;
                    NewDirForAction(bloke, (bloke->field_73 >> 5) + 3);
                    bloke->field_36 = FUN_0042aa60(pos, lap);
                    cars[bloke->field_36] = 1;
                    bloke->field_58 = (rand() % 3 + 4) * 50;
                    riders++;
                    queued--;
                    bloke->param_action++;
                }
                break;
            case 8:
                bloke->dest.x = (x - 5) << 8;
                y = (y << 8) + 0x26a;
                bloke->dest.y = y;
                dir = CalcMoveLine(bloke->pos, bloke->dest, &bloke->nav);
                bloke->low_level_action = 7;
                bloke->field_73 = dir + 0x10;
                NewDirForAction(bloke, (bloke->field_73 >> 5) + 3);
                bloke->param_action++;
                break;
            case 9:
                bloke->flags |= 0x80;
                bloke->person->sprite = DAT_00616040;
                bloke->person->field_30 = 1;
                bloke->person->depth = GetUnitDepth(-1617692.375f, -1617904.25f);
                sprintf(&buf[5], "%02d", bloke->field_36);
                SetBlokePositionFromBNV(BalloonzBinV, bloke, buf, 0, -1617692.375f, -1617904.25f, 0);
                bloke->param_action++;
                break;
            case 10:
                cars[bloke->field_36] = 2;
                sprintf(&buf[5], "%02d", bloke->field_36);
                SetBlokePositionFromBNV(BalloonzBinV, bloke, buf, pos + lap * 24, -1617692.375f, -1617904.25f, 0);
                if (--bloke->field_58 == 0) {
                    bloke->param_action++;
                }
                break;
            case 11:
                sprintf(&buf[5], "%02d", bloke->field_36);
                SetBlokePositionFromBNV(BalloonzBinV, bloke, buf, pos + lap * 24, -1617692.375f, -1617904.25f, 0);
                bloke->param_action++;
                leaving++;
                cars[bloke->field_36] = 3;
                break;
            case 12:
                sprintf(&buf[5], "%02d", bloke->field_36);
                SetBlokePositionFromBNV(BalloonzBinV, bloke, buf, pos + lap * 24, -1617692.375f, -1617904.25f, 0);
                if (can_unload == 1) {
                    car = FUN_0042aa60(pos, lap);
                    if (car == bloke->field_36) {
                        bloke->flags &= 0xff7f;
                        y = (y + 3) << 8;
                        x = (x << 8) - 0x564;
                        bloke->dest.x = x;
                        bloke->dest.y = y;
                        dir = CalcMoveLine(bloke->pos, bloke->dest, &bloke->nav);
                        bloke->field_73 = dir + 0x10;
                        bloke->low_level_action = 7;
                        bloke->person->sprite = NULL;
                        bloke->person->field_30 = 0;
                        NewDirForAction(bloke, (bloke->field_73 >> 5) + 3);
                        bloke->param_action++;
                        riders--;
                        cars[car] = 1;
                    }
                }
                break;
            case 13:
                y += 3;
                bloke->dest.x = (x << 8) - 0x632;
                y <<= 8;
                bloke->dest.y = y;
                dir = CalcMoveLine(bloke->pos, bloke->dest, &bloke->nav);
                bloke->low_level_action = 7;
                bloke->field_73 = dir + 0x10;
                NewDirForAction(bloke, (bloke->field_73 >> 5) + 3);
                bloke->param_action++;
                break;
            case 14:
                y = (y + 9) << 8;
                x = (x - 5) << 8;
                bloke->dest.x = x;
                bloke->dest.y = y;
                dir = CalcMoveLine(bloke->pos, bloke->dest, &bloke->nav);
                bloke->low_level_action = 7;
                bloke->field_73 = dir + 0x10;
                NewDirForAction(bloke, (bloke->field_73 >> 5) + 3);
                bloke->param_action++;
                cars[bloke->field_36] = 0;
                leaving--;
                break;
            case 15:
                RemoveBlokeFromRide(ride, elem);
                bloke->flags &= 0xfff7;
                break;
            }
        }
        state->queued = queued;
        state->lap = lap;
        memcpy(state->cars, cars, sizeof(cars));
        state->riders = riders;
        state->leaving = leaving;
        state->frame = pos;
        state->can_board = can_board;
        state->can_unload = can_unload;
        elem = next;
    }
    for (state = BalloonNodeList; state != NULL; state = state->next) {
        char pos;
        int stop;
        char car;

        queued = state->queued;
        riders = state->riders;
        stop = 0;
        can_unload = state->can_unload;
        memcpy(cars, state->cars, sizeof(cars));
        leaving = state->leaving;
        pos = state->pos;
        lap = state->lap;
        can_board = 0;
        if (pos % 8 == 0 || pos == 0) {
            car = FUN_0042aa60(pos, lap);
            if (queued != 0 && riders < 6 && cars[car] == 0 && rand() % 3 == 0) {
                can_board = 1;
                stop = 1;
            }
            if (cars[car] == 1) {
                stop = 1;
            }
            if (leaving != 0 && cars[car] == 3) {
                can_unload = 1;
                stop = 1;
            }
        }
        if (riders != 0 && !stop) {
            pos++;
            if (pos > 23) {
                pos = 0;
                lap++;
                if (lap > 1) {
                    lap = 0;
                }
            }
            can_unload = 0;
            can_board = 0;
        }
        state->riders = riders;
        state->queued = queued;
        memcpy(state->cars, cars, sizeof(cars));
        state->lap = lap;
        state->pos = pos;
        state->leaving = leaving;
        state->can_board = can_board;
        state->can_unload = can_unload;
    }
}

// FUNCTION: LEGOLAND 0x0042b2a0
RideSpriteInfo *GetBalloonzSpriteInfo(Element *obj, unsigned short id) {
    Ride *ride = obj->ride;

    DAT_00616028.sprite = ride->layer;
    DAT_00616028.x = ride->field_14;
    DAT_00616028.y = ride->field_18;
    DAT_00616028.id = id;
    ride->layer->flags |= 0x2000;
    return &DAT_00616028;
}

// FUNCTION: LEGOLAND 0x0042b2e0
void RenderBalloonz(Element *obj, void *param_2, void *param_3, TileId *tile, unsigned int param_5, unsigned int param_6) {
    Ride *ride = obj->ride;
    RideNode *elem;
    RideNode *riders;
    Bloke *blokes[6] = {0};
    Bloke *bloke;
    Person *person;
    BalloonNode *state;
    Point screen;
    Point off;
    LayerResult layer;
    char count;
    char frame;
    char anim;
    char lap;
    char i;

    elem = ride->riders;
    count = 0;
    AdjustOffsetForViewMode(&off);
    state = FindBalloonNode(tile);
    if (state == NULL) {
        return;
    }
    frame = state->frame;
    anim = state->anim;
    lap = state->lap;
    screen = GetScreenCoordsForObject(tile, ride);
    GetLayer(ride->layer, &layer, 1);
    layer.field_10 = 0;
    for (; elem != NULL; elem = elem->next) {
        if (tile->id == elem->tile.id) {
            blokes[count++] = elem->rider;
        }
    }
    if (count != 0) {
        riders = ride->riders;
        off = GetRenderOffsetForLayer(BalloonzLayer, 0);
        AdjustOffsetForViewMode(&off);
        for (i = 0; i < count; i++) {
            if (blokes[i]->param_action == 6) {
                IP_RenderBlokeIn3DNow(blokes[i]);
            }
        }
        for (i = 0; i < count; i++) {
            if (blokes[i]->param_action == 5) {
                IP_RenderBlokeIn3DNow(blokes[i]);
            }
        }
        PrintSprite(BallBaseM1Sprite, screen.x + off.x, screen.y + off.y, param_6, 0);
        for (i = 0; i < count; i++) {
            if (blokes[i]->param_action == 4) {
                IP_RenderBlokeIn3DNow(blokes[i]);
            }
        }
        PrintSprite(BallBaseM2Sprite, screen.x + off.x, screen.y + off.y, param_6, 0);
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
            if (blokes[i]->param_action == 2) {
                IP_RenderBlokeIn3DNow(blokes[i]);
            }
        }
        for (i = 0; i < count; i++) {
            if (blokes[i]->param_action == 3) {
                IP_RenderBlokeIn3DNow(blokes[i]);
            }
        }
        PrintSprite(BallBaseM3Sprite, screen.x + off.x, screen.y + off.y, param_6, 0);
        for (i = 0; i < count; i++) {
            if (blokes[i]->param_action == 7) {
                IP_RenderBlokeIn3DNow(blokes[i]);
            }
        }
        for (i = 0; i < count; i++) {
            if (blokes[i]->param_action == 14) {
                IP_RenderBlokeIn3DNow(blokes[i]);
            }
        }
        for (i = 0; i < count; i++) {
            if (blokes[i]->param_action == 15) {
                IP_RenderBlokeIn3DNow(blokes[i]);
            }
        }
        LLSSetFrame(GetLLSForLayer(BalloonzLayer, 1), frame);
        off = GetRenderOffsetForLayer(BalloonzLayer, 1);
        AdjustOffsetForViewMode(&off);
        PrintSprite(GetSpriteForLayer(BalloonzLayer, 1), screen.x + off.x, screen.y + off.y, param_6, 0);
        if (frame % 8 == 0 || frame == 0) {
            LLSSetFrame(GetLLSForLayer(BalloonzLayer, 1), frame);
            off = GetRenderOffsetForLayer(BalloonzLayer, 1);
            AdjustOffsetForViewMode(&off);
            for (i = 0; i < count; i++) {
                if (blokes[i]->param_action == 8) {
                    IP_RenderBlokeIn3DNow(blokes[i]);
                }
            }
            for (i = 0; i < count; i++) {
                if (blokes[i]->param_action == 9) {
                    IP_RenderBlokeIn3DNow(blokes[i]);
                }
            }
            for (i = 0; i < count; i++) {
                if (blokes[i]->param_action == 13) {
                    IP_RenderBlokeIn3DNow(blokes[i]);
                }
            }
            for (i = 0; i < count; i++) {
                if (blokes[i]->param_action == 14) {
                    IP_RenderBlokeIn3DNow(blokes[i]);
                }
            }
            switch (FUN_0042aa60(frame, lap)) {
            case 0:
            case 3:
                PrintSprite(BZRedCarM1Sprite, screen.x + off.x, screen.y + off.y, param_6, 0);
                break;
            case 1:
            case 4:
                PrintSprite(BZGreenCarM1Sprite, screen.x + off.x, screen.y + off.y, param_6, 0);
                break;
            case 2:
            case 5:
                PrintSprite(BZBlueCarM1Sprite, screen.x + off.x, screen.y + off.y, param_6, 0);
                break;
            }
        }
        for (; riders != NULL; riders = riders->next) {
            if (tile->id == riders->tile.id && (riders->rider->flags & 0x80) != 0) {
                Point seat;
                Point adjust;
                int x;

                bloke = riders->rider;
                seat.x = layer.x + 0xc;
                seat.y = layer.y - 6;
                adjust.x = 0x12;
                adjust.y = 0;
                AdjustOffsetForViewMode(&adjust);
                x = bloke->screen_x - adjust.x;
                adjust.y -= 8;
                person = bloke->person;
                person->offset.x = x;
                person->offset.y = bloke->screen_y - adjust.y;
                AdjustBlokePosition(&person->offset);
                AdjustOffsetForViewMode(&seat);
                person->screen.x = bloke->screen_x - adjust.x + seat.x + screen.x;
                person->screen.y = bloke->screen_y - adjust.y + seat.y + screen.y;
                AdjustBlokePosition(&person->screen);
                IP_RenderBlokeIn3DNow(riders->rider);
            }
        }
    } else {
        LLSSetFrame(GetLLSForLayer(BalloonzLayer, 1), frame);
        off = GetRenderOffsetForLayer(BalloonzLayer, 1);
        AdjustOffsetForViewMode(&off);
        PrintSprite(GetSpriteForLayer(BalloonzLayer, 1), screen.x + off.x, screen.y + off.y, param_6, 0);
    }
    anim++;
    if (anim > 48) {
        anim = 0;
    }
    state->anim = anim;
    LLSSetFrame(GetLLSForLayer(BalloonzLayer, 2), anim);
    off = GetRenderOffsetForLayer(BalloonzLayer, 2);
    AdjustOffsetForViewMode(&off);
    PrintSprite(GetSpriteForLayer(BalloonzLayer, 2), screen.x + off.x, screen.y + off.y, param_6, 0);
}

// FUNCTION: LEGOLAND 0x0042b9d0
void UnloadBalloonzResources(void) {
    KillSprite(BallBaseM1Sprite);
    KillSprite(BallBaseM2Sprite);
    KillSprite(BallBaseM3Sprite);
    KillSprite(BZRedCarM1Sprite);
    KillSprite(BZGreenCarM1Sprite);
    KillSprite(BZBlueCarM1Sprite);
    KillSprite(ZBalloon2Sprite);
    FreeBinV(DAT_00616018[0]);
    RemoveAllBalloonNodes();
}

// FUNCTION: LEGOLAND 0x0042ba40
void BalloonzSetEditMode(void) {
    EditMode.unk0 = 1;
    EditMode.unk8 = BalloonzRide;
    DefaultCursor(&EditCursor);
    SetEditCursorFootPrint(&EditMode.unk8->footprint);
}

// FUNCTION: LEGOLAND 0x0042ba80
unsigned int SaveBalloonNodes(void) {
    unsigned int marker;
    unsigned int terminator;
    BalloonNode *node;

    marker = 1;
    terminator = 0;
    node = BalloonNodeList;
    while (node != NULL) {
        if (SaveGameWrite(&marker, 4) == 0) {
            return 0;
        }
        if (SaveGameWrite(node, sizeof(BalloonNode)) == 0) {
            return 0;
        }
        node = node->next;
    }
    if (SaveGameWrite(&terminator, 4) != 0) {
        return 1;
    }
    return 0;
}

// FUNCTION: LEGOLAND 0x0042baf0
unsigned int Balloonz_Load(Element *obj) {
    Ride *ride = obj->ride;
    BalloonNode *node;
    BalloonNode *prev;
    RideNode *rider;
    BNVRef *bnv;
    int more;

    prev = NULL;
    if (SaveGameRead(&more, 4) == 0) {
        return 0;
    }
    while (more != 0) {
        node = malloc(sizeof(BalloonNode));
        if (SaveGameRead(node, sizeof(BalloonNode)) == 0) {
            return 0;
        }
        node->next = NULL;
        if (prev != NULL) {
            prev->next = node;
        } else {
            BalloonNodeList = node;
        }
        prev = node;
        if (SaveGameRead(&more, 4) == 0) {
            return 0;
        }
    }
    for (rider = ride->riders; rider != NULL; rider = rider->next) {
        if (rider->person->field_30 != 0) {
            rider->person->sprite = DAT_0061603c[rider->person->field_30];
        } else {
            rider->person->sprite = NULL;
            rider->person->field_30 = 0;
        }
        bnv = rider->rider->bnv;
        if (bnv != NULL) {
            bnv->file = DAT_00616018[bnv->index];
        }
    }
    return 1;
}
