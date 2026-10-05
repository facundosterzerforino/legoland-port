#include "legoland.h"

#include <string.h>

#include "bricks.h"
#include "controller.h"
#include "draw.h"
#include "gamemain.h"
#include "gamemap.h"
#include "globals.h"
#include "interface.h"
#include "llidb.h"
#include "map_object.h"
#include "math.h"
#include "obj_instance.h"
#include "popupinfo.h"
#include "ride_queue.h"
#include "sound_music.h"
#include "tilemap.h"
#include "worker.h"

/* An FXSpriteList as seen by the map hover code: its first tile and the per-tile flag words. */
struct TileSetView {
    int first_tile;
    unsigned char pad_4[0x8];
    int *sprite_ids;
};

// FUNCTION: LEGOLAND 0x00457870
void SetBricksLimited(int param_1) {
    UnlimitedBricks = (param_1 == 0);
}

// FUNCTION: LEGOLAND 0x00457890
int AreBricksLimited(void) {
    return UnlimitedBricks == 0;
}

// FUNCTION: LEGOLAND 0x004578a0
LEGO_EXPORT void AddBricks(unsigned int param_1) {
    if (UnlimitedBricks == 0) {
        BrickCount += param_1;
    }
}

// FUNCTION: LEGOLAND 0x004578c0
LEGO_EXPORT void UseBricks(unsigned int param_1) {
    if (UnlimitedBricks == 0) {
        BrickCount -= param_1;
    }
}

// FUNCTION: LEGOLAND 0x004578e0
LEGO_EXPORT int GetBrickCount(void) {
    if (UnlimitedBricks != 0) {
        return 0x7fffffff;
    }
    return BrickCount;
}

// FUNCTION: LEGOLAND 0x00457900
void SetBrickCount(unsigned int param_1) {
    BrickCount = param_1;
}

// FUNCTION: LEGOLAND 0x00457910
int SaveCurrency(void) {
    if (SaveGameWrite(&UnlimitedBricks, 4) == 0) {
        return 0;
    }
    return SaveGameWrite(&BrickCount, 4) != 0;
}

// FUNCTION: LEGOLAND 0x00457940
int LoadCurrency(void) {
    if (SaveGameRead(&UnlimitedBricks, 4) == 0) {
        return 0;
    }
    return SaveGameRead(&BrickCount, 4) != 0;
}

// FUNCTION: LEGOLAND 0x00457970
int FUN_00457970(int dx, int dy) {
    struct Ride *ride = EditMode.unk8;
    int x, y;
    struct MapElement *elem;

    dx += ride->footprint.x0;
    dy += ride->footprint.y0;
    for (y = dy; y < dy + (int)FootprintHeight; y++) {
        for (x = dx; x < dx + (int)FootprintWidth; x++) {
            if (x >= 0 && x < lpConfig->width && y >= 0 && y < lpConfig->height) {
                elem = &GameMap[y][x];
            } else {
                elem = NULL;
            }
            if (elem == NULL) return 0;
            if (elem->flags & 0x8f8) return 0;
            if (elem->field_12 != 0) {
                unsigned int idx;
                // STRING: LEGOLAND 0x004b8a70
                LLIDB_FindElement("PATH CONTROL", &idx, 0);
                if (EditMode.unk8->element != (struct Element *)idx) return 0;
            }
        }
    }
    return 1;
}

// FUNCTION: LEGOLAND 0x00457a70
void FUN_00457a70(void) {
    struct Element *roads;
    struct Element *zebra;
    struct Point pt;
    struct MapElement *tile;
    unsigned int v;
    struct Element *elem;
    unsigned short id;
    struct ObjClass *cls;
    int x;
    int y;

    // STRING: LEGOLAND 0x004b89cc
    roads = ElemID("DRIVING SCHOOL ROADS");
    // STRING: LEGOLAND 0x004b89bc
    zebra = ElemID("ZEBRA CROSSING");
    if (EditMode.unk0 == 0 || EditMode.unk0 == 2) {
        QueryClass = NULL;
        if (Hover.type == 0x100 || Hover.type == 0x103) {
            if ((int)MouseTileX >= 0 && (int)MouseTileX < lpConfig->width && (int)MouseTileY >= 0 &&
                (int)MouseTileY < lpConfig->height) {
                tile = &GameMap[MouseTileY][MouseTileX];
            } else {
                tile = NULL;
            }
            if (tile) {
                if (tile->flags & 0x888) {
                    if (tile->flags & 0x800) {
                        if ((Hover.ptr = (struct Bloke *)GetGardenerWorkOrderAt(MouseTileX, MouseTileY)) != 0) {
                            Hover.type = 0x10b;
                        } else if ((Hover.ptr = (struct Bloke *)GetMechanicWorkOrderAt(MouseTileX, MouseTileY)) != 0) {
                            Hover.type = 0x10c;
                        }
                        id = tile->anchor.id;
                        Hover.data.tile.id = id;
                        elem = tile->field_0;
                        if (elem) {
                            QueryClass = (struct ObjClass *)elem->ride;
                            QueryObj.id = id;
                        }
                    } else {
                        Hover.ptr = (struct Bloke *)tile->field_0;
                        Hover.data.tile.id = tile->anchor.id;
                        if (tile->flags & 0x88) {
                            Hover.type = 0x103;
                            if (tile->field_0 == roads) {
                                struct RideQueueEntry *entry;
                                if ((entry = FindQueueEntryAtTile(tile->field_4, tile->field_5)) != 0 && (entry->field_14 & 0x10)) {
                                    Hover.ptr = (struct Bloke *)zebra;
                                }
                            }
                        }
                        elem = (struct Element *)Hover.ptr;
                        QueryClass = (struct ObjClass *)elem->ride;
                        QueryObj.id = Hover.data.tile.id;
                    }
                } else if (Hover.type != 0x103) {
                    struct TileSetView *src;
                    if ((src = (struct TileSetView *)TileSpriteInfo[tile->field_8].src) != 0 && (src->sprite_ids[tile->field_8 - src->first_tile] & 0x10)) {
                        Hover.type = 0x10d;
                    } else {
                        Hover.type = 0x109;
                    }
                } else {
                    elem = (struct Element *)Hover.ptr;
                    QueryClass = (struct ObjClass *)elem->ride;
                    QueryObj.id = Hover.data.tile.id;
                }
            } else if (Hover.type == 0x103 && EditMode.unk0 == 2) {
                elem = (struct Element *)Hover.ptr;
                QueryClass = (struct ObjClass *)elem->ride;
                QueryObj.id = Hover.data.tile.id;
            } else {
                Hover.type = 0x10a;
            }
        }
    }
    cls = QueryClass;
    switch (EditMode.unk0) {
    case 2:
        v = Hover.data.value & 0xffff;
        pt.x = v & 0xff;
        pt.y = v >> 8;
        DAT_00810144 = 0;
        if (DAT_0080ff6c != NULL && (DAT_0080ff6c == PathControlObject || DAT_0080ff6c == HedgeObjectClass)) {
            GamePad |= 0x800;
        } else {
            GamePad &= ~0x800;
        }
        if (!(GamePad & 0x1000)) {
            if (Hover.type == 0x103) {
                QueryClass->method_94(QueryClass->element, &pt);
            } else if (Hover.type == 0x10c) {
                cls->method_94(cls->element, &pt);
            } else if (Hover.type == 0x10b) {
                cls->method_94(cls->element, &pt);
            } else {
                memset(QueryCursor.field_1414, 0, 20);
                QueryCursor.field_1828 = 8;
                FUN_0045f480(&QueryCursor, 1);
                QueryCursor.tile_x = MouseTileX;
                QueryCursor.tile_y = MouseTileY;
                DAT_00667c5c = 0;
                GamePad &= ~0x400;
                if (!(DAT_00813ac4 & 2)) {
                    DAT_0080ff6c = NULL;
                }
            }
        } else {
            FUN_00452030();
            QueryCursor = EditCursor;
        }
        if (DAT_00810144 == 0) {
            QueryCursor.field_1830 = 0;
        }
        BuildCursorPtr(&QueryCursor, 0, 0);
        RenderCursor(&QueryCursor);
        if (FUN_0045f4b0(&QueryCursor)) {
            SetPointer(2);
        } else {
            SetPointer(1);
        }
        if (DAT_00813ac4 & 0x11) {
            if (DAT_0080ff6c != NULL && (((struct ObjClass *)DAT_0080ff6c)->flags & 0x2000000)) {
                DAT_00667cd8 = 1;
                DAT_00667cdc = 0;
                for (y = DAT_00813a88; y <= (int)DAT_00813a90; y += DAT_00813a3c) {
                    for (x = DAT_00813a84; x <= (int)DAT_00813a8c; x += DAT_00813a38) {
                        QueryCursor.field_1414[4] = 0;
                        QueryCursor.tile_x = x;
                        QueryCursor.tile_y = y;
                        if (x >= 0 && x < lpConfig->width && y >= 0 && y < lpConfig->height) {
                            tile = &GameMap[y][x];
                        } else {
                            tile = NULL;
                        }
                        if (tile != NULL) {
                            QueryObj.pos.x = QueryCursor.tile_x = tile->field_4;
                            QueryObj.pos.y = QueryCursor.tile_y = tile->field_5;
                            if ((tile->flags & 0x88) && !(tile->flags & 0x40) &&
                                tile->field_0 == (struct Element *)((struct ObjClass *)DAT_0080ff6c)->element) {
                                RemObjFromMap((struct ObjClass *)DAT_0080ff6c,
                                    (unsigned int)((struct ObjClass *)DAT_0080ff6c)->element, QueryObj,
                                    &QueryCursor);
                            }
                        }
                    }
                }
                if (DAT_00667cdc != 0) {
                    PlayInstanceOfSample(GameFX[FX_PUNCH].sample, 0, 1, 0);
                    CalculateMapRenderOrder();
                }
                DAT_00667cd8 = 0;
            } else if (FUN_0045f4b0(&QueryCursor)) {
                if (Hover.type == 0x10c) {
                    FUN_0045e850((struct ObjNode *)((WorkOrder *)Hover.ptr)->element, &((WorkOrder *)Hover.ptr)->pos.x);
                    FUN_0045d3d0((struct PathFootprint *)((WorkOrder *)Hover.ptr)->element->ride, &((WorkOrder *)Hover.ptr)->pos.x);
                    EraseMechanicOrder((WorkOrder *)Hover.ptr);
                } else if (Hover.type == 0x10b) {
                    FUN_0045d3d0((struct PathFootprint *)((WorkOrder *)Hover.ptr)->element->ride, &((WorkOrder *)Hover.ptr)->pos.x);
                    EraseGardenerOrder((WorkOrder *)Hover.ptr);
                } else {
                    pt.x = *(unsigned int *)&QueryObj & 0xff;
                    pt.y = QueryObj.pos.y;
                    if (pt.x >= 0 && pt.x < lpConfig->width && pt.y >= 0 && pt.y < lpConfig->height) {
                        tile = &GameMap[pt.y][pt.x];
                    } else {
                        tile = NULL;
                    }
                    QueryClass = (struct ObjClass *)tile->field_0->ride;
                    if (DAT_00810144 == 0) {
                        FUN_0045d3d0((struct PathFootprint *)QueryClass, &pt.x);
                    }
                    RemObjFromMap(QueryClass, (unsigned int)QueryClass->element, QueryObj, &QueryCursor);
                }
            }
        }
        if (!(GamePad & 0x1000) && Hover.type == 0x103) {
            if (QueryClass->flags & 0x2000000) {
                GamePad |= 0x400;
                DAT_0080ff6c = QueryClass;
            } else {
                GamePad &= ~0x400;
                DAT_0080ff6c = NULL;
            }
        }
        if (DAT_00813acc & 2) {
            EditMode.unk0 = 0;
            GamePad &= ~0x1400;
        }
        break;
    case 1:
        if (FUN_0045f4b0(&EditCursor)) {
            SetPointer(4);
        } else {
            SetPointer(3);
        }
        if (EditMode.unk8 == PathControlObject || EditMode.unk8 == HedgeObjectClass) {
            GamePad |= 0x800;
        } else {
            GamePad &= ~0x800;
        }
        if (!(GamePad & 0x400) && EditMode.unk8 != NULL) {
            ((struct ObjClass *)EditMode.unk8)->method_90((unsigned int)EditMode.unk8->element, &MousePos, 0x8f8);
        }
        BuildCursorPtr(&EditCursor, 0x8f8, FUN_0045ead0((struct ObjState *)EditMode.unk8));
        if (DAT_00813ac4 & 0x11) {
            if (FUN_0045f4b0(&EditCursor)) {
                if (EditMode.unk8->flags & 0x2000000) {
                    DAT_00667cd8 = 1;
                    DAT_00667cdc = 0;
                    if (EditMode.unk8 == PathControlObject) {
                        for (y = DAT_00813a88; y <= (int)DAT_00813a90; y += FootprintHeight) {
                            for (x = DAT_00813a84; x <= (int)DAT_00813a8c; x += FootprintWidth) {
                                pt.x = x - EditMode.unk8->footprint.x0;
                                pt.y = y - EditMode.unk8->footprint.y0;
                                if (pt.x >= 0 && pt.x < lpConfig->width && pt.y >= 0 && pt.y < lpConfig->height) {
                                    tile = &GameMap[pt.y][pt.x];
                                } else {
                                    tile = NULL;
                                }
                                if (tile != NULL && (tile->flags & 0x8a0) && (tile->field_0->ride->flags & 0x200000)) {
                                    FUN_004779d0(&pt);
                                }
                            }
                        }
                    }
                    for (y = DAT_00813a88; y <= (int)DAT_00813a90; y += FootprintHeight) {
                        for (x = DAT_00813a84; x <= (int)DAT_00813a8c; x += FootprintWidth) {
                            pt.x = x - EditMode.unk8->footprint.x0;
                            pt.y = y - EditMode.unk8->footprint.y0;
                            if (FUN_00457970(pt.x, pt.y)) {
                                if (WorkOrderBuildObject(EditMode.unk8->element, &pt)) {
                                    ScriptDirtyCategories |= 0x10;
                                }
                                PathUpdateNeeded = 1;
                            }
                        }
                    }
                    DAT_00667cd8 = 0;
                    if (DAT_00667cdc != 0) {
                        PlayAppropriateBuildEffect((struct ObjClass *)EditMode.unk8, NULL);
                        PlayInstanceOfSample(GameFX[FX_PUNCH].sample, 0, 1, 0);
                        CalculateMapRenderOrder();
                    }
                    DAT_00667cd8 = 0;
                } else if (WorkOrderBuildObject(EditMode.unk8->element, (Point *)&EditCursor.tile_x)) {
                    FUN_00475f40();
                    ScriptDirtyCategories |= 2;
                }
            } else {
                FUN_00473640(EditCursor.field_1410);
            }
        } else {
            RenderCursor(&EditCursor);
        }
        if (DAT_00813acc & 2) {
            EditMode.unk0 = 0;
            GamePad &= ~0x1400;
        }
        break;
    case 0:
        v = Hover.data.value & 0xffff;
        pt.x = v & 0xff;
        pt.y = v >> 8;
        if (Hover.type == 0x103) {
            QueryClass->method_94(QueryClass->element, &pt);
        } else if (Hover.type == 0x10c) {
            cls->method_94(cls->element, &pt);
        } else if (Hover.type == 0x10b) {
            cls->method_94(cls->element, &pt);
        } else {
            memset(QueryCursor.field_1414, 0, 20);
            QueryCursor.field_1828 = 8;
            FUN_0045f480(&QueryCursor, 1);
            QueryCursor.tile_x = MouseTileX;
            QueryCursor.tile_y = MouseTileY;
            DAT_00667c5c = 0;
            GamePad &= ~0x400;
            if (!(DAT_00813ac4 & 2)) {
                DAT_0080ff6c = NULL;
            }
            break;
        }
        QueryCursor.field_1830 = 0;
        BuildCursorPtr(&QueryCursor, 0, 0);
        RenderCursor(&QueryCursor);
        break;
    }
    if ((DAT_00813a50 & 2) && DAT_00667c48 == 0 && EditMode.unk0 == 0 && DAT_00668954 == 0) {
        PopUpInfoSetUp(Hover, MousePos.x, MousePos.y);
        DAT_00667c48 = 1;
    }
}
