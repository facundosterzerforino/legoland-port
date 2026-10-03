#include <string.h>

#include "legoland.h"

#include "castle.h"
#include "clipping.h"
#include "draw.h"
#include "gamemap.h"
#include "gfx.h"
#include "globals.h"
#include "interface.h"
#include "llidb.h"
#include "map_object.h"
#include "mapscreen.h"
#include "obj_instance.h"
#include "print_sprite.h"
#include "render.h"
#include "ride_queue.h"
#include "tilemap.h"

struct MapPoint {
    int field0;
    int field1;
};

/* A sprite overlaid on the map (OverlayList). */
struct OverlayNode {
    unsigned char pad_0[0x14];
    int x;
    int y;
    struct OverlayNode *next;
    struct Sprite *sprite;
};

#define HALF(v) ((v) < 0 ? -(-(v) >> 1) : (v) >> 1)

#include "image_sprite.h"

// FUNCTION: LEGOLAND 0x004562c0
void FUN_004562c0(void) {
    DAT_00667c10 = DAT_006687bc;
    DAT_00667c28 = DAT_006687c0;
}

// FUNCTION: LEGOLAND 0x004562e0
void FUN_004562e0(void) {
    DAT_006687bc = DAT_00667c10;
    DAT_006687c0 = DAT_00667c28;
}

// FUNCTION: LEGOLAND 0x00456300
LEGO_EXPORT void InitMapScreen(void) {
    if (DAT_00667c30 != 0) {
        return;
    }
    FullMapSprite = CreateFunctionBasedSprite(
        (int (*)(struct Sprite *))RenderFullMap, 0x280, 0x154);
    if (FullMapSprite != NULL) {
        // STRING: LEGOLAND 0x004b90b4
        MapSpannerSprite = LoadSprite("mapSpanner.lls", 0);
        FullMapSprite->src_x = 0;
        FullMapSprite->src_y = 0;
    }
    FUN_004562c0();
    DAT_006687c0 = (unsigned int)FUN_00475080;
    DAT_006687bc = (unsigned int)FUN_00475080;
}

// FUNCTION: LEGOLAND 0x00456370
LEGO_EXPORT void KillMapScreen(void) {
    if (FullMapSprite != NULL) {
        KillSprite(FullMapSprite);
        FullMapSprite = NULL;
        if (MapSpannerSprite != 0) {
            KillSprite(MapSpannerSprite);
            MapSpannerSprite = 0;
        }
    }
    DAT_00667c30 = 0;
}

// FUNCTION: LEGOLAND 0x004563b0
void FUN_004563b0(void) {
    int x = ((ScrollX >> 8) - MapWorldMinX) * MapViewWidth / MapWorldWidth;
    int y = ((ScrollY >> 8) - DAT_00667c04) * MapViewWidth / MapWorldHeight / 2 +
        MapCenterOffsetY;
    int w = MapViewWidth * MapViewWidth / MapWorldWidth;
    int h = MapViewHeight * MapViewWidth / MapWorldHeight / 2;

    RenderThickBox(MapViewX + x, MapViewY + y, w, h, 2,
        GetNearestColour(255, 255, 255));
}

// FUNCTION: LEGOLAND 0x00456460
LEGO_EXPORT void RenderMouseBounds(void) {
    int in[2];
    int out[2];
    int x;
    int y;
    int w;
    int h;

    in[0] = (MousePos.x - MapViewX) * MapWorldWidth / MapViewWidth +
        MapWorldMinX;
    in[1] = (MousePos.y - MapViewY - MapCenterOffsetY + 1) * MapWorldHeight *
            2 / MapViewWidth +
        DAT_00667c04;
    PointToIsoPlane(in, out);
    if (out[0] >= 0 && out[1] >= 0 && out[0] < MapViewWidth &&
        out[1] < MapViewHeight) {
        x = MousePos.x - MapViewWidth / 2 * MapViewWidth / MapWorldWidth -
            MapViewX;
        y = MousePos.y - MapViewHeight / 2 * MapViewWidth / MapWorldHeight / 2 -
            MapViewY;
        w = MapViewWidth * MapViewWidth / MapWorldWidth;
        h = MapViewHeight * MapViewWidth / (MapWorldHeight * 2);
        RenderBox(MapViewX + x, MapViewY + y, w, h,
            GetNearestColour(0, 255, 0));
    }
}

// FUNCTION: LEGOLAND 0x004565b0
LEGO_EXPORT void MapScreenSetScrollPos(struct Point *point) {
    int in[2];
    int out[2];

    in[0] = (MousePos.x - MapViewX) * MapWorldWidth / MapViewWidth +
        MapWorldMinX;
    in[1] = (MousePos.y - MapViewY - MapCenterOffsetY + 1) * MapWorldHeight *
            2 / MapViewWidth +
        DAT_00667c04;
    PointToIsoPlane(in, out);
    if (out[0] >= 0 && out[1] >= 0 && out[0] < MapViewWidth &&
        out[1] < MapViewHeight) {
        ScrollX = ((point->x - MapViewX + 1) * MapWorldWidth / MapViewWidth -
                      MapViewWidth / 2 + MapWorldMinX)
            << 8;
        ScrollY = ((point->y - MapViewY - MapCenterOffsetY + 1) * MapWorldHeight * 2 /
                          MapViewWidth -
                      MapViewHeight / 2 + DAT_00667c04)
            << 8;
        FUN_00461290(lpConfig->view_width << 8, lpConfig->view_height << 8, 0, 0);
    }
}

// FUNCTION: LEGOLAND 0x004566f0
LEGO_EXPORT void DrawMapScreen(void) {
    struct MapMarker(*row)[32];
    struct MapMarker *m;
    int j;

    PrintSprite(FullMapSprite, MapViewX, MapViewY, 0, 0);
    if (FrameCounter & 0x10) {
        for (row = DAT_008119c0; (int)row < (int)DAT_008138c0; row++) {
            m = *row;
            for (j = 0; j < 31; j++, m++) {
                if (m->x != 0 || m->y != 0) {
                    PrintSprite(MapSpannerSprite, m->x, m->y, 0, 0);
                }
            }
        }
    }
    FUN_004563b0();
}

// FUNCTION: LEGOLAND 0x00456770
void FUN_00456770(struct MapPoint *arg) {
    int v0 = arg->field0;
    int v1 = arg->field1;

    if (v0 < 0) {
        v0 = -((-v0) >> 1);
    } else {
        v0 = v0 >> 1;
    }

    if (v1 < 0) {
        v1 = -((-v1) >> 1);
    } else {
        v1 = v1 >> 1;
    }

    arg->field0 = v0;
    arg->field1 = v1;
}

// FUNCTION: LEGOLAND 0x004567a0
LEGO_EXPORT void RenderFullMap(void) {
    struct Element *square_track;
    struct Element *square_track_height;
    struct Element *square_track_height_0;
    struct Element *square_track_height_path;
    struct Element *castle_dummy;
    struct Element *driving_school_roads;
    struct Sprite *blob;
    struct Sprite *stick;
    struct Sprite *lights;
    HPEN pen;

    // STRING: LEGOLAND 0x004b5bfc
    square_track = ElemID("SQUARE_TRACK");
    // STRING: LEGOLAND 0x004b5be8
    square_track_height = ElemID("SQUARE_TRACK_HEIGHT");
    // STRING: LEGOLAND 0x004b5bd0
    square_track_height_0 = ElemID("SQUARE_TRACK_HEIGHT_0");
    // STRING: LEGOLAND 0x004b5bb4
    square_track_height_path = ElemID("SQUARE_TRACK_HEIGHT_PATH");
    // STRING: LEGOLAND 0x004b5b7c
    castle_dummy = ElemID("CASTLE_DUMMY");
    // STRING: LEGOLAND 0x004b89cc
    driving_school_roads = ElemID("DRIVING SCHOOL ROADS");
    // STRING: LEGOLAND 0x004b90e4
    blob = LoadSprite("TRACKBLOB.LLS", 1);
    // STRING: LEGOLAND 0x004b90d4
    stick = LoadSprite("TRACKSTICK.LLS", 1);
    // STRING: LEGOLAND 0x004b90c4
    lights = LoadSprite("MAPLIGHTS.LLS", 1);
    pen = CreatePen(PS_SOLID, 2, 0xff4000);

    if (DAT_00667c30 == 0) {
        struct Point tile_size;
        int half;
        struct Point map_scale;
        struct {
            int y;
            int x;
        } saved_scroll;
        struct Point tile_pos;
        int base_x;
        int base_y;
        int bounds[4];
        /* These height values belong to separate rendering passes. */
        union {
            float syf;
            float f1;
        } height_value;
        /* Clipping and terrain rendering use this storage at different times. */
        union {
            RECT clip;
            struct MapElement tile;
        } tile_scratch;
        struct MapElement *obj;
        struct OverlayNode *node;

        FUN_0045a660();
        memset(DAT_008119c0, 0, 0x2000);
        MapViewX = 0;
        MapViewY = 0x20;
        MapViewWidth = 0x280;
        MapViewHeight = 0x154;
        StoreClipping();
        {
            tile_scratch.clip.left = 0;
            tile_scratch.clip.right = MapViewWidth;
            tile_scratch.clip.top = 0;
            tile_scratch.clip.bottom = MapViewHeight;
            SetClipping(&tile_scratch.clip);
        }
        GetTileDimensions(&tile_size.x, &tile_size.y);
        DAT_00667c04 = 0;
        MapWorldMinX = -(lpConfig->height * tile_size.x / 2);
        DAT_00667c08 = lpConfig->width * tile_size.x / 2;
        DAT_00667c0c = (lpConfig->width + lpConfig->height) * tile_size.y / 2;
        MapWorldWidth = DAT_00667c08 - MapWorldMinX + 1;
        MapWorldHeight = DAT_00667c0c + 1;
        map_scale.x = (MapViewWidth << 16) / MapWorldWidth;
        map_scale.y = (MapViewHeight << 16) / MapWorldHeight;
        DAT_00667c16 =
            (short)((float)MapViewWidth * tile_size.x / MapWorldWidth + 1.0f);
        DAT_00667c14 =
            (short)((float)MapViewHeight * tile_size.y / MapWorldHeight + 1.0f);
        half = (tile_size.y + 1) >> 1;
        RenderBlock(0, 0, MapViewWidth, MapViewHeight, GetNearestColour(0, 0, 0));
        saved_scroll.x = lpConfig->view_x;
        saved_scroll.y = lpConfig->view_y;
        lpConfig->view_x = 0;
        lpConfig->view_y = 0;
        DAT_00667c30 = 1;
        MapCenterOffsetY =
            (MapViewHeight -
                ((lpConfig->width + lpConfig->height - 2) * half - DAT_00667c04) *
                    MapViewWidth / MapWorldHeight / 2) >>
            1;

        for (tile_pos.y = 0; tile_pos.y < (int)lpConfig->height; tile_pos.y++) {
            for (tile_pos.x = 0; tile_pos.x < (int)lpConfig->width; tile_pos.x++) {
                if (tile_pos.x >= 0 && tile_pos.x < (int)lpConfig->width &&
                    tile_pos.y >= 0 && tile_pos.y < (int)lpConfig->height) {
                    tile_scratch.tile = GameMap[tile_pos.y][tile_pos.x];
                } else {
                    tile_scratch.tile.flags = 0x40;
                }
                if ((tile_scratch.tile.flags & 8) == 0) {
                    struct FXSpriteList *src;
                    unsigned int id;
                    int px;
                    int py;
                    struct Point pt;

                    id = GameMap[tile_pos.y][tile_pos.x].field_8;
                    src = TileSpriteInfo[id].src;
                    pt.x = tile_pos.x;
                    pt.y = tile_pos.y;
                    GetTileBounds(&pt, bounds);
                    bounds[0] += ScrollX >> 8;
                    bounds[1] += ScrollY >> 8;
                    px = ((bounds[0] - MapWorldMinX) * map_scale.x) >> 16;
                    py =
                        (((bounds[1] - DAT_00667c04) * map_scale.y) >> 16) + MapCenterOffsetY;
                    if ((tile_scratch.tile.flags & 0x10) &&
                        (tile_scratch.tile.flags & 0x80)) {
                        RenderBlock(px - 7, py - 2, DAT_00667c16 + 5, DAT_00667c14 + 4,
                            GetNearestColour(0x80, 0x80, 0x80));
                    } else {
                        switch (src->sprite_ids[id - src->base] & 0x3f) {
                        case 0:
                        case 0x20:
                            RenderBlock(px - 7, py - 2, DAT_00667c16 + 5, DAT_00667c14 + 4,
                                GetNearestColour(0, 0x8f, 0x4f));
                            break;
                        case 1:
                        case 0x21:
                            RenderBlock(px - 7, py - 2, DAT_00667c16 + 5, DAT_00667c14 + 4,
                                GetNearestColour(0xff, 0xe0, 0x8f));
                            break;
                        case 0x30:
                            RenderBlock(px - 7, py - 2, DAT_00667c16 + 5, DAT_00667c14 + 4,
                                GetNearestColour(0x5b, 0xbe, 2));
                            break;
                        }
                    }
                }
            }
        }

        PushRenderingStatusAndUnlockVideoSurface();
        for (tile_pos.y = 0; tile_pos.y < (int)lpConfig->height; tile_pos.y++) {
            for (tile_pos.x = 0; tile_pos.x < (int)lpConfig->width; tile_pos.x++) {
                if (tile_pos.x >= 0 && tile_pos.x < (int)lpConfig->width &&
                    tile_pos.y >= 0 && tile_pos.y < (int)lpConfig->height) {
                    tile_scratch.tile = GameMap[tile_pos.y][tile_pos.x];
                } else {
                    tile_scratch.tile.field_8 = 0;
                    tile_scratch.tile.flags = 0x40;
                }
                if (tile_scratch.tile.flags & 8) {
                    struct Ride *ride;
                    struct MapPoint off;
                    struct Point pt;
                    float sxf;

                    sxf = (float)MapViewWidth / MapWorldWidth;
                    height_value.syf = (float)MapViewHeight / MapWorldHeight;
                    ride = tile_scratch.tile.field_0->ride;
                    pt.x = tile_pos.x;
                    pt.y = tile_pos.y;
                    GetTileBounds(&pt, bounds);
                    off.field0 = ride->field_14;
                    off.field1 = ride->field_18;
                    FUN_00456770(&off);
                    bounds[1] += off.field1;
                    bounds[0] += off.field0;
                    bounds[0] += ScrollX >> 8;
                    bounds[1] += ScrollY >> 8;
                    PrintScaledSprite(
                        TileSpriteArray[tile_scratch.tile.field_8],
                        (int)((bounds[0] - MapWorldMinX) * sxf),
                        (int)((bounds[1] - DAT_00667c04) * height_value.syf) +
                            MapCenterOffsetY,
                        DAT_00667c16 + 1, DAT_00667c14 + 1);
                }
            }
        }

        {
            tile_scratch.clip.left = 0;
            tile_scratch.clip.right = MapViewWidth;
            tile_scratch.clip.top = 0;
            tile_scratch.clip.bottom = MapViewHeight;
            SetClipping(&tile_scratch.clip);
        }
        {
            int origin[4];
            struct Point pt;

            node = OverlayList;
            pt.x = 0;
            pt.y = 0;
            GetTileBounds(&pt, origin);
            origin[0] += ScrollX >> 8;
            origin[1] += ScrollY >> 8;
            base_x = ((origin[0] - MapWorldMinX) * map_scale.x) >> 16;
            base_y =
                (((origin[1] - DAT_00667c04) * map_scale.y) >> 16) + MapCenterOffsetY;
            for (; node != NULL; node = node->next) {
                PrintScaledSprite(
                    node->sprite,
                    (((tile_size.x * 2 / 3 + node->x) * map_scale.x) >> 16) + base_x,
                    ((node->y * map_scale.y) >> 16) + base_y,
                    ((short)node->sprite->width * map_scale.x) >> 16,
                    ((short)node->sprite->height * map_scale.y) >> 16);
            }
        }

        for (obj = GetFirstRenderObject(); obj != NULL;
            obj = GetNextRenderObject(obj)) {
            struct MapElement tile;
            TileId uid;
            struct Ride *ride;
            RideSpriteInfo *info;
            RideSpriteInfo loc;

            uid.id = obj->anchor.id;
            tile = *obj;
            if ((tile.flags & 0x200) && (tile.flags & 4) == 0) {
                struct Point pt;

                pt.x = (tile.field_4 & ~7) + 4;
                pt.y = (tile.field_5 & ~7) + 4;
                GetTileBounds(&pt, bounds);
                bounds[0] += ScrollX >> 8;
                bounds[1] += ScrollY >> 8;
                DAT_008119c0[tile.field_5 >> 3][tile.field_4 >> 3].x =
                    ((bounds[0] - MapWorldMinX) * map_scale.x) >> 16;
                DAT_008119c0[tile.field_5 >> 3][tile.field_4 >> 3].y =
                    (((bounds[1] - DAT_00667c04) * map_scale.y) >> 16) + MapCenterOffsetY;
            }
            ride = tile.field_0->ride;
            info = NULL;
            if ((ride->flags & 4) == 0 && (ride->flags & 0x400) == 0) {
                if (ride == driving_school_roads->ride) {
                    struct RideQueueEntry *entry;
                    struct Point pt;

                    loc.sprite = lights;
                    loc.x = -102;
                    loc.y = -88;
                    pt.x = uid.pos.x;
                    pt.y = uid.pos.y;
                    entry = FindQueueEntryAtTile(pt.x, pt.y);
                    if (entry != NULL && (entry->field_14 & 0xf) == 5) {
                        GetTileBounds(&pt, bounds);
                        bounds[0] += HALF(loc.x);
                        bounds[1] += HALF(loc.y);
                        bounds[0] += ScrollX >> 8;
                        bounds[1] += ScrollY >> 8;
                        DAT_00667c16 = ((struct Sprite *)loc.sprite)->width;
                        DAT_00667c14 = ((struct Sprite *)loc.sprite)->height;
                        PrintScaledSprite(
                            loc.sprite, ((bounds[0] - MapWorldMinX) * map_scale.x) >> 16,
                            (((bounds[1] - DAT_00667c04) * map_scale.y) >> 16) +
                                MapCenterOffsetY,
                            (DAT_00667c16 * map_scale.x) >> 16,
                            (DAT_00667c14 * map_scale.y) >> 16);
                    }
                }
            } else if (ride->element == square_track ||
                ride->element == square_track_height ||
                ride->element == square_track_height_0 ||
                ride->element == square_track_height_path ||
                ride->element == castle_dummy) {
                struct Point pos;
                struct Point dest;

                float f2;
                int flag;

                pos.x = tile.anchor.pos.x;
                pos.y = tile.field_5;
                if (FUN_00424050(&pos, &height_value.f1, &dest, &f2, &flag)) {
                    struct Point pt;
                    int x1;
                    int y1;
                    int x2;
                    int y2;
                    int rise;
                    HDC hdc;
                    HGDIOBJ old;

                    if (height_value.f1 < 0.0f) {
                        height_value.f1 = 0.0f;
                    }
                    if (f2 < 0.0f) {
                        f2 = 0.0f;
                    }
                    pt.x = pos.x;
                    pt.y = pos.y;
                    GetTileBounds(&pt, bounds);
                    rise = -(int)height_value.f1;
                    bounds[1] += HALF(rise);
                    bounds[0] += ScrollX >> 8;
                    bounds[1] += ScrollY >> 8;
                    x1 = ((bounds[0] - MapWorldMinX) * map_scale.x) >> 16;
                    y1 =
                        (((bounds[1] - DAT_00667c04) * map_scale.y) >> 16) + MapCenterOffsetY;
                    pt.x = dest.x;
                    pt.y = dest.y;
                    GetTileBounds(&pt, bounds);
                    rise = -(int)f2;
                    bounds[1] += HALF(rise);
                    bounds[0] += ScrollX >> 8;
                    bounds[1] += ScrollY >> 8;
                    x2 = ((bounds[0] - MapWorldMinX) * map_scale.x) >> 16;
                    y2 =
                        (((bounds[1] - DAT_00667c04) * map_scale.y) >> 16) + MapCenterOffsetY;
                    if (ride->element != square_track_height_path) {
                        DAT_00667c16 = stick->width;
                        DAT_00667c14 = (short)((float)map_scale.y * height_value.f1 *
                            7.62939453125e-06f);
                        if (DAT_00667c14 > 0) {
                            PrintScaledSprite(
                                stick, x1 - ((DAT_00667c16 * map_scale.x) >> 17), y1,
                                (DAT_00667c16 * map_scale.x) >> 16, DAT_00667c14);
                        }
                    }
                    PushRenderingStatusAndUnlockVideoSurface();
                    renderEngine->lpVtbl->GetDC(renderEngine, &hdc);
                    old = SelectObject(hdc, pen);
                    MoveToEx(hdc, x2, y2, NULL);
                    LineTo(hdc, x1, y1);
                    if (flag != 0) {
                        pt.x = pos.x - 10;
                        pt.y = pos.y;
                        GetTileBounds(&pt, bounds);
                        rise = -(int)height_value.f1;
                        bounds[1] += HALF(rise);
                        bounds[1] += ScrollY >> 8;
                        bounds[0] += ScrollX >> 8;
                        LineTo(hdc, ((bounds[0] - MapWorldMinX) * map_scale.x) >> 16,
                            (((bounds[1] - DAT_00667c04) * map_scale.y) >> 16) +
                                MapCenterOffsetY);
                    }
                    SelectObject(hdc, old);
                    renderEngine->lpVtbl->ReleaseDC(renderEngine, hdc);
                    PopRenderingStatus();
                }
                loc.sprite = blob;
                loc.x = 0;
                loc.y = (int)-height_value.f1;
            } else if (ride->flags & 0x400) {
                if (ride->cb_sprite == NULL) {
                    continue;
                }
                info = ride->cb_sprite(ride->element, uid);
            } else {
                loc.sprite = ride->layer;
                loc.x = ride->field_14;
                loc.y = ride->field_18;
                info = &loc;
            }
            if (info == NULL || info->sprite == NULL) {
                continue;
            }
            if ((((struct Sprite *)info->sprite)->flags & 0x8000) == 0) {
                struct Point pt;
                int hx;
                int hy;
                struct Sprite *sprite;

                pt.x = uid.pos.x;
                pt.y = uid.pos.y;
                GetTileBounds(&pt, bounds);
                hx = info->x;
                hy = info->y;
                hx = HALF(hx);
                hy = HALF(hy);
                bounds[0] += hx;
                bounds[1] += hy;
                bounds[0] += ScrollX >> 8;
                bounds[1] += ScrollY >> 8;
                sprite = info->sprite;
                DAT_00667c16 = sprite->width;
                DAT_00667c14 = sprite->height;
                PrintScaledSprite(
                    info->sprite, ((bounds[0] - MapWorldMinX) * map_scale.x) >> 16,
                    (((bounds[1] - DAT_00667c04) * map_scale.y) >> 16) + MapCenterOffsetY,
                    (DAT_00667c16 * map_scale.x) >> 16,
                    (DAT_00667c14 * map_scale.y) >> 16);
            } else {
                int i;

                for (i = 0; i < ((struct Sprite *)info->sprite)->group->count; i++) {
                    struct Sprite *sub;
                    int sub_x;
                    int sub_y;
                    int hx;
                    int hy;
                    int ox;
                    int oy;
                    struct Point pt;

                    sub = ((struct Sprite *)info->sprite)->group->subs[i];
                    sub_x = ((struct Sprite *)info->sprite)->group->xoffs[i];
                    sub_y = ((struct Sprite *)info->sprite)->group->yoffs[i];
                    pt.x = uid.pos.x;
                    pt.y = uid.pos.y;
                    GetTileBounds(&pt, bounds);
                    hx = HALF(sub_x);
                    hy = HALF(sub_y);
                    ox = info->x;
                    oy = info->y;
                    ox = HALF(ox);
                    oy = HALF(oy);
                    bounds[0] += ox;
                    bounds[1] += oy;
                    bounds[0] += hx;
                    bounds[1] += hy;
                    bounds[0] += ScrollX >> 8;
                    bounds[1] += ScrollY >> 8;
                    DAT_00667c16 = sub->width;
                    DAT_00667c14 = sub->height;
                    PrintScaledSprite(
                        sub, ((bounds[0] - MapWorldMinX) * map_scale.x) >> 16,
                        (((bounds[1] - DAT_00667c04) * map_scale.y) >> 16) + MapCenterOffsetY,
                        (DAT_00667c16 * map_scale.x) >> 16,
                        (DAT_00667c14 * map_scale.y) >> 16);
                }
            }
        }
        PopRenderingStatus();
        lpConfig->view_x = saved_scroll.x;
        lpConfig->view_y = saved_scroll.y;
        RestoreClipping();
        CommitCliprectToHardware();
        if (blob != NULL) {
            KillSprite(blob);
        }
        if (stick != NULL) {
            KillSprite(stick);
        }
        if (lights != NULL) {
            KillSprite(lights);
        }
        DeleteObject(pen);
        CalculateMapRenderOrder();
    }
}
