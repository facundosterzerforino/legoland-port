#include <stdlib.h>
#include <string.h>
#include "globals.h"
#include "legoland.h"

#include "controller.h"
#include "gamemap.h"
#include "llidb.h"
#include "map_object.h"
#include "math.h"
#include "obj_instance.h"
#include "objclass.h"
#include "pathfind.h"
#include "sound_music.h"
#include "tilemap.h"
#include "timer.h"
#include "worker.h"

struct PowerEntry {
    const char *name;
    int value;
};

struct MapCellObjKind {
    /* 0x00 */ unsigned char pad_0[0x20];
    /* 0x20 */ short field_20;
    /* 0x22 */ unsigned char pad_22[0x44 - 0x22];
    /* 0x44 */ int footprint_x1;
    /* 0x48 */ char footprint_y1;
};

struct MapCellObj {
    unsigned char pad_0[0xc];
    struct MapCellObjKind *ride;
};

struct MapCell {
    /* 0x00 */ struct MapCellObj *obj;
    /* 0x04 */ union {
        unsigned short coords;
        struct {
            unsigned char byte_4;
            unsigned char byte_5;
        } b;
    } src;
    /* 0x06 */ unsigned short field_6;
    /* 0x08 */ unsigned short word_8;
    /* 0x0a */ unsigned short word_a;
    /* 0x0c */ union {
        unsigned short word;
        unsigned char bytes[2];
    } flags;
    /* 0x0e */ unsigned char pad_e[2];
    /* 0x10 */ unsigned char byte_10;
    /* 0x11 */ unsigned char pad_11[3];
};

struct RectListNode {
    /* 0x00 */ struct RectListNode *next;
    /* 0x04 */ unsigned char pad_4[4];
    /* 0x08 */ int x0;
    /* 0x0c */ int y0;
    /* 0x10 */ int x1;
    /* 0x14 */ int y1;
};

struct RemBlock {
    /* 0x00 */ int f0;
    /* 0x04 */ int f4;
    /* 0x08 */ int f8;
    /* 0x0c */ int fc;
};

// FUNCTION: LEGOLAND 0x00459850
LEGO_EXPORT void InitGameMap(void) {
    // STRING: LEGOLAND 0x004b5c0c
    CastleObjElem = ElemID("CASTLE OBJ");
    Load_FXList(GameFX, FX_COUNT);
}

// FUNCTION: LEGOLAND 0x00459870
LEGO_EXPORT void KillGameMap(void) {
    Kill_FXList(GameFX, FX_COUNT);
}

// FUNCTION: LEGOLAND 0x00459880
void FUN_00459880(void) {
    DAT_00667ce0 = 0;
    DAT_00667ce4 = 0;
    DAT_00667ce8 = 0;
    DAT_00667cec = 0;
    DAT_00667cf0 = 0;
    DAT_00667cf4 = 0;
    DAT_00667cf8 = 0;
    DAT_00667cfc = 0;
}

// FUNCTION: LEGOLAND 0x004598d0
void FUN_004598d0(struct Point *coord, int *param_2, int *param_3) {
    struct MapCell *cell;
    short kind;

    if (coord->x < 0 || coord->x >= (int)lpConfig->width || coord->y < 0 || coord->y >= (int)lpConfig->height) {
        cell = NULL;
    } else {
        cell = (struct MapCell *)((char *)GameMap[coord->y] + coord->x * 0x14);
    }
    if (cell == NULL) {
        *param_2 = *param_2 + -1;
        return;
    }
    if ((cell->flags.word & 0x10) != 0) {
        *param_2 = *param_2 + -1;
        return;
    }
    if ((cell->flags.word & 0x80) != 0) {
        if (cell->obj->ride == PathControlObject) {
            *param_2 = *param_2 + -1;
            return;
        }
        kind = cell->obj->ride->field_20;
        if (kind == 2 || kind == 3) {
            *param_3 = *param_3 + 1;
        }
    }
}

// FUNCTION: LEGOLAND 0x00459960
void FUN_00459960(void) {
    DAT_00667d10 = GetGameTimer();
}

// FUNCTION: LEGOLAND 0x00459970
void FUN_00459970(void) {
    int now;
    struct RectListNode *node;
    int total;
    int matched;
    struct Point coord;

    now = GetGameTimer();
    if (10000 < now - (int)DAT_00667d10) {
        DAT_00667d0c = 0;
        matched = 0;
        total = 0;
        DAT_00667d10 = now;
        for (node = (struct RectListNode *)GetBestNodeList(); node != NULL; node = node->next) {
            total = total + 4 + (((node->y1 - node->x0) - node->y0) + node->x1) * 2;
            coord.x = node->x0;
            while (coord.x <= node->x1) {
                coord.y = node->y0 + -1;
                FUN_004598d0(&coord, &total, &matched);
                coord.y = node->y1 + 1;
                FUN_004598d0(&coord, &total, &matched);
                coord.x++;
            }
            coord.y = node->y0;
            while (coord.y <= node->y1) {
                coord.x = node->x0 + -1;
                FUN_004598d0(&coord, &total, &matched);
                coord.x = node->x1 + 1;
                FUN_004598d0(&coord, &total, &matched);
                coord.y++;
            }
        }
        DAT_00667d00 = total;
        DAT_00667d04 = matched;
        if (total != 0) {
            DAT_00667d08 = (matched * 100) / total;
            return;
        }
        DAT_00667d08 = 0;
    }
}

// FUNCTION: LEGOLAND 0x00459ad0
LEGO_EXPORT void PutObjOnMap(struct ObjClass *obj, unsigned int classid, struct Point *pos) {
    int area;
    struct ObjClass *entrance;
    struct MapCell *cell;

    if (classid == CastleObjElem) {
        CastlePlacedFlag = 1;
    }
    obj->method_98(classid, pos);
    if (obj == PathControlObject) {
        DAT_00667cf4 = DAT_00667cf4 + 1;
        DAT_00667ce0 = DAT_00667ce0 + 1;
        DAT_00667d0c = 1;
    } else {
        area = GetRectArea((struct RectNode *)&obj->footprint);
        switch (obj->type) {
        case 1:
            FUN_00489f00(pos);
            DAT_00667ce4 = DAT_00667ce4 + area;
            break;
        case 2:
            DAT_00667d0c = 1;
            DAT_00667cf8 = DAT_00667cf8 + area;
            break;
        case 3:
            DAT_00667cf0 = DAT_00667cf0 + area;
            break;
        case 4:
            FUN_00489f00(pos);
            DAT_00667ce8 = DAT_00667ce8 + area;
            break;
        case 5:
            FUN_00489f00(pos);
            DAT_00667cec = DAT_00667cec + area;
        }
        DAT_00667ce0 = DAT_00667ce0 + area;
        AddObjectsPowerStats(classid, pos);
    }
    if (classid == ElemID("ENTRANCE 1")) {
        entrance = (struct ObjClass *)((struct ClassNode *)ElemID("ENTRANCE 1"))->iface;
        if (pos->x < 0 || pos->x >= (int)lpConfig->width || pos->y < 0 || pos->y >= (int)lpConfig->height) {
            cell = NULL;
        } else {
            cell = (struct MapCell *)((char *)GameMap[pos->y] + pos->x * 0x14);
        }
        DAT_004b8320.x = (cell->src.b.byte_4 + entrance->footprint.v[0]) * 0x100 + -0x100;
        DAT_004b8320.y = ((unsigned int)(entrance->footprint.v[3] - entrance->footprint.v[1]) >> 1) * 0x100 +
            (cell->src.b.byte_5 + entrance->footprint.v[1]) * 0x100;
    }
    DAT_00668610 = DAT_00668610 | 1;
}

// FUNCTION: LEGOLAND 0x00459c90
LEGO_EXPORT void RemObjFromMap(struct ObjClass *obj, unsigned int classid, TileId tile, void *cursor) {
    int area;
    struct MapCell *cell;
    struct Cursor *query;
    int x;
    int y;
    struct RemBlock blk;

    EraseWorkOrdersAtTile(obj, tile);
    if (classid == CastleObjElem) {
        CastlePlacedFlag = 0;
    }
    if (DAT_00667cd8 == 0) {
        blk.f8 = tile.pos.x;
        blk.fc = tile.pos.y;
        blk.f0 = 2;
        PlayInstanceOfSample(GameFX[FX_PUNCH].sample, 0, 1, &blk);
    } else {
        DAT_00667cdc = 1;
    }
    RemoveObjectsPowerStats(classid, tile);
    obj->method_9c(classid, tile, cursor);
    if (obj == PathControlObject) {
        DAT_00667cf4 = DAT_00667cf4 + -1;
        DAT_00667ce0 = DAT_00667ce0 + -1;
        DAT_00667d0c = 1;
    } else {
        area = GetRectArea((struct RectNode *)&obj->footprint);
        switch (obj->type) {
        case 1:
            FUN_00489f50((struct Point *)&blk);
            DAT_00667ce4 = DAT_00667ce4 - area;
            break;
        case 2:
            DAT_00667d0c = 1;
            DAT_00667cf8 = DAT_00667cf8 - area;
            break;
        case 3:
            DAT_00667cf0 = DAT_00667cf0 - area;
            break;
        case 4:
            FUN_00489f50((struct Point *)&blk);
            DAT_00667ce8 = DAT_00667ce8 - area;
            break;
        case 5:
            FUN_00489f50((struct Point *)&blk);
            DAT_00667cec = DAT_00667cec - area;
        }
        DAT_00667ce0 = DAT_00667ce0 - area;
    }
    x = tile.pos.x;
    y = tile.pos.y;
    if (x >= 0 && x < (int)lpConfig->width && y >= 0 && y < (int)lpConfig->height) {
        cell = (struct MapCell *)((char *)GameMap[y] + x * 0x14);
    } else {
        cell = NULL;
    }
    if (cell->flags.bytes[1] & 0x40) {
        RemoveRepairOrderAT(obj, x, y);
        cell->flags.word = cell->flags.word & 0xbfff;
    }
    query = &QueryCursor;
    while (query != NULL) {
        if ((query->field_1828 & 0x1000) != 0) {
            if (query != NULL) {
                for (blk.f4 = query->field_1414[1] + query->tile_y;
                    blk.f4 <= (int)(query->tile_y + query->field_1414[3]); blk.f4 = blk.f4 + 1) {
                    for (blk.f0 = query->tile_x + query->field_1414[0];
                        blk.f0 <= (int)(query->tile_x + query->field_1414[2]); blk.f0 = blk.f0 + 1) {
                        cell = (struct MapCell *)((char *)GameMap[blk.f4] + blk.f0 * 0x14);
                        cell->flags.word = cell->flags.word & 0xffe7;
                        cell = (struct MapCell *)((char *)GameMap[blk.f4] + blk.f0 * 0x14);
                        cell->byte_10 = 0;
                        cell = (struct MapCell *)((char *)GameMap[blk.f4] + blk.f0 * 0x14);
                        cell->word_8 = cell->word_a;
                        FUN_0045d260((struct Point *)&blk);
                        RemovePathSquare((struct Point *)&blk);
                    }
                }
            }
            break;
        }
        query = (struct Cursor *)query->field_1830;
    }
    DAT_00668610 = DAT_00668610 | 4;
}

// GLOBAL: LEGOLAND 0x004b9340
static const struct PowerEntry PTR_s_Small_Power_Station_004b9340[] = {
    {"Small Power Station", 800},
    {"Crystal Power Station", 2500},
    {"Dino Big", -100},
    {"Dino Small", -30},
    {"Dino Mini", -30},
    {"T-Rex", -130},
    {"Fountain 1", -10},
    {"Fountain 2", -10},
    {"Fountain 3", -10},
    {"Foodcart Drink", -2},
    {"FoodCart Food", -2},
    {"Foodcart Icecream", -2},
    {"LEGO Shop 1", -3},
    {"LEGO Shop 2", -3},
    {"LEGO Media Shop", -3},
    {"Octopus Cafe", -5},
    {"Restaurant 1", -10},
    {"Restaurant 2", -60},
    {"Shark Cafe", -10},
    {"Boating School", -200},
    {"Boating School Mermaid", -30},
    {"Copters", -100},
    {"Driving School", -60},
    {"Space Tower Ride", -40},
    {"Spider Ride", -100},
    {"Water Works Entrance", -30},
    {"Water Works Crocodile Fountain", -6},
    {"Water Works Elephant Fountain", -6},
    {"Water Works Water Block", -4},
    {"Water Pump", -15},
    {"Bank", -5},
    {"Chuck Wagon", -6},
    {"General Store", -4},
    {"Jail Cell", -2},
    {"Saloon", -4},
    {"Sheriff", -3},
    {"Carousel", -60},
    {"Fort", -8},
    {"Gold Rush", -30},
    {"Log Flume Entrance", -100},
    {"Spinning Barrels Ride", -90},
    {"Temple", -3},
    {"Explorers Institute", -20},
    {"Balloonz", -140},
    {"Earth Slide Ride", -40},
    {"Jungle Cruise", -100},
    {"Plane Ride", -160},
    {"Safari Ride", -80},
    {"Temple Slide", -50},
    {"Castle BBQ", -20},
    {"Castle Level 1", -8},
    {"Castle Obj", -400},
    {"Catapult", -2},
    {"Joust", -75},
    {"Miniland San Francisco", -40},
    {"Miniland France", -40},
    {"Miniland Washington", -40},
    {"Miniland New York", -40},
    {"Miniland India", -40},
    {"Miniland Holland", -40},
    {"Miniland London", -40},
    {"Miniland Italy", -40},
    {"Miniland Australia", -40},
    {"Miniland Egypt", -40},
    {"Miniland Denmark", -40},
    {"", 0},
};

// FUNCTION: LEGOLAND 0x00459fa0
LEGO_EXPORT int FindObjectsPower(struct Ride *ride) {
    const struct PowerEntry *entry;

    for (entry = PTR_s_Small_Power_Station_004b9340; strlen(entry->name) != 0; entry++) {
        if (_stricmp(ride->element->name, entry->name) == 0) {
            return entry->value;
        }
    }
    return 0;
}

// FUNCTION: LEGOLAND 0x0045a000
void FUN_0045a000(int power, MapElement *object) {
    object->flags &= 0xfeff;
    MapStats.unpowered_demand = MapStats.unpowered_demand - power;
    MapStats.unpowered_count = MapStats.unpowered_count - 1;
}

// FUNCTION: LEGOLAND 0x0045a030
void FUN_0045a030(int power, MapElement *object) {
    object->flags |= 0x100;
    MapStats.unpowered_demand = MapStats.unpowered_demand + power;
    MapStats.unpowered_count = MapStats.unpowered_count + 1;
}

// FUNCTION: LEGOLAND 0x0045a060
void FUN_0045a060(void) {
    MapElement *object = GetFirstRenderObject();
    int power;
    if (object == NULL) {
        return;
    }
    while (object != NULL) {
        if (object->flags & 0x100) {
            power = -FindObjectsPower(object->field_0->data);
            if (MapStats.power_demand - (int)MapStats.unpowered_demand + power <= MapStats.power_supply) {
                FUN_0045a000(power, object);
                if (MapStats.power_demand - (int)MapStats.unpowered_demand == MapStats.power_supply) {
                    return;
                }
            }
        }
        object = GetNextRenderObject(object);
    }
}

// FUNCTION: LEGOLAND 0x0045a0d0
void FUN_0045a0d0(void) {
    MapElement *object = GetFirstRenderObject();
    int power;
    while (object != NULL) {
        if ((object->flags & 0x100) == 0) {
            power = -FindObjectsPower(object->field_0->data);
            if (power > 0) {
                FUN_0045a030(power, object);
                if (MapStats.power_demand - (int)MapStats.unpowered_demand <= MapStats.power_supply) {
                    break;
                }
            }
        }
        object = GetNextRenderObject(object);
    }
}

// FUNCTION: LEGOLAND 0x0045a130
LEGO_EXPORT void AddObjectsPowerStats(unsigned int classid, struct Point *pos) {
    int power;
    struct MapCell *cell;
    int amount;

    power = FindObjectsPower(((struct ClassNode *)classid)->iface);
    if (power != 0) {
        if (0 < power) {
            MapStats.power_supply = MapStats.power_supply + power;
            if (MapStats.unpowered_demand != 0) {
                FUN_0045a060();
            }
            return;
        }
        if (pos->x < 0 || pos->x >= (int)lpConfig->width || pos->y < 0 || pos->y >= (int)lpConfig->height) {
            cell = NULL;
        } else {
            cell = (struct MapCell *)((char *)GameMap[pos->y] + pos->x * 0x14);
        }
        amount = abs(power);
        MapStats.power_demand = MapStats.power_demand + amount;
        if (MapStats.power_supply < MapStats.power_demand - (int)MapStats.unpowered_demand) {
            MapStats.unpowered_demand = MapStats.unpowered_demand + amount;
            MapStats.unpowered_count = MapStats.unpowered_count + 1;
            cell->flags.bytes[1] |= 1;
        } else {
            cell->flags.word = cell->flags.word & 0xfeff;
        }
        if (MapStats.power_supply <= MapStats.power_demand) {
            MapStats.power_spare_percent = 0;
            return;
        }
        MapStats.power_spare_percent = 100 - (MapStats.power_demand * 100) / MapStats.power_supply;
    }
}

// FUNCTION: LEGOLAND 0x0045a230
LEGO_EXPORT void RemoveObjectsPowerStats(unsigned int classid, TileId coords) {
    int power;
    struct MapCell *cell;
    int amount;
    int x;
    int y;

    power = FindObjectsPower(((struct ClassNode *)classid)->iface);
    if (power != 0) {
        if (0 < power) {
            x = coords.pos.x;
            y = coords.pos.y;
            if (x >= 0 && x < (int)lpConfig->width && y >= 0 && y < (int)lpConfig->height) {
                cell = (struct MapCell *)((char *)GameMap[y] + x * 0x14);
            } else {
                cell = NULL;
            }
            if ((cell->flags.bytes[1] & 2) == 0) {
                MapStats.power_supply = MapStats.power_supply - power;
                if (MapStats.power_demand - (int)MapStats.unpowered_demand > MapStats.power_supply) {
                    FUN_0045a0d0();
                }
            }
        } else {
            amount = abs(power);
            MapStats.power_demand = MapStats.power_demand - amount;
            if (MapStats.unpowered_demand != 0) {
                x = coords.pos.x;
                y = coords.pos.y;
                if (x >= 0 && x < (int)lpConfig->width && y >= 0 && y < (int)lpConfig->height) {
                    cell = (struct MapCell *)((char *)GameMap[y] + x * 0x14);
                } else {
                    cell = NULL;
                }
                if ((cell->flags.bytes[1] & 1) != 0) {
                    MapStats.unpowered_demand = MapStats.unpowered_demand - amount;
                    MapStats.unpowered_count = MapStats.unpowered_count + -1;
                }
                FUN_0045a060();
            }
        }
        if (MapStats.power_supply <= MapStats.power_demand) {
            MapStats.power_spare_percent = 0;
            return;
        }
        MapStats.power_spare_percent = 100 - (MapStats.power_demand * 100) / MapStats.power_supply;
    }
}

// FUNCTION: LEGOLAND 0x0045a390
LEGO_EXPORT void DefaultCursor(struct Cursor *cursor) {
    unsigned int saved_1404 = cursor->tile_x;
    unsigned int saved_1408 = cursor->tile_y;
    memset(cursor, 0, 0x1834);
    cursor->tile_x = saved_1404;
    cursor->field_1828 = 0xc00;
    cursor->tile_y = saved_1408;
    FUN_0045f460(cursor);
}

// FUNCTION: LEGOLAND 0x0045a3e0
void FUN_0045a3e0(int *param) {
    struct MapRenderOrderEntry *entry;
    int i;

    for (i = 0, entry = MapRenderOrderList; (int)&entry->x < (int)((char *)&CastleObjElem + 2); entry++, i++) {
        if (entry->flag != 0 && (unsigned int)entry->x == *param) {
            param[1] = MapRenderOrderList[i].height;
            MapRenderOrderList[i].flag = 0;
            return;
        }
    }
    param[1] = 0;
}

// FUNCTION: LEGOLAND 0x0045a430
void FUN_0045a430(short param_1, int *param_2) {
    struct MapRenderOrderEntry *entry;
    int i;

    for (i = 0, entry = MapRenderOrderList; entry < MapRenderOrderList + 4096; entry++, i++) {
        if (entry->flag != 0 && (short)entry->coords == param_1) {
            *param_2 = MapRenderOrderList[i].x;
            param_2[1] = MapRenderOrderList[i].height;
            MapRenderOrderList[i].flag = 0;
            return;
        }
    }
    *param_2 = lpConfig->width;
    param_2[1] = lpConfig->height;
}

// FUNCTION: LEGOLAND 0x0045a4a0
LEGO_EXPORT void CalculateMapRenderOrder(void) {
    unsigned short *out_coords;
    struct MapCell *cell;
    struct MapCell *src;
    struct MapCellObjKind *tile;
    struct MapRenderOrderEntry *entry;
    int sx;
    int sy;
    struct Point pt;

    pt.x = 0;
    pt.y = 0;
    DAT_00801408 = 0;
    out_coords = &DAT_007febb8;
    memset(MapRenderOrderList, 0, sizeof(MapRenderOrderList));
    while (pt.x < lpConfig->width) {
        if (pt.x >= 0 && pt.x < lpConfig->width && pt.y >= 0 && pt.y < lpConfig->height) {
            cell = (struct MapCell *)&GameMap[pt.y][pt.x];
        } else {
            cell = NULL;
        }
        if ((cell->flags.bytes[0] & 0xa0) == 0) {
            pt.y++;
        } else {
            sx = cell->src.b.byte_4;
            sy = cell->src.b.byte_5;
            if (sx >= 0 && sx < lpConfig->width && sy >= 0 && sy < lpConfig->height) {
                src = (struct MapCell *)&GameMap[sy][sx];
            } else {
                src = NULL;
            }
            tile = src->obj->ride;
            entry = &MapRenderOrderList[DAT_00801408];
            DAT_00801408++;
            if (DAT_00801408 == 0x1000) {
                DAT_00801408 = 0;
            }
            entry->coords = cell->src.coords;
            entry->x = (unsigned char)pt.x;
            entry->flag = 1;
            entry->height = tile->footprint_y1 + sy + 1;
            if (pt.x == tile->footprint_x1 + sx || pt.x == lpConfig->width - 1) {
                *out_coords = src->src.coords;
                FUN_0045a430(cell->src.coords, &pt.x);
                out_coords = &src->field_6;
            } else {
                pt.x++;
                FUN_0045a3e0(&pt.x);
            }
        }
        while (pt.y >= lpConfig->height) {
            pt.x++;
            FUN_0045a3e0(&pt.x);
        }
    }
    *out_coords = 0;
}

// FUNCTION: LEGOLAND 0x0045a660
void FUN_0045a660(void) {
    unsigned short *out_coords;
    struct MapCell *cell;
    struct MapCell *src;
    struct MapCellObjKind *tile;
    struct MapRenderOrderEntry *entry;
    int sx;
    int sy;
    unsigned int roads;
    struct Point pt;

    out_coords = &DAT_007febb8;
    pt.x = 0;
    pt.y = 0;
    roads = ElemID("DRIVING SCHOOL ROADS");
    DAT_00801408 = 0;
    memset(MapRenderOrderList, 0, sizeof(MapRenderOrderList));
    while (pt.x < lpConfig->width) {
        if (pt.x >= 0 && pt.x < lpConfig->width && pt.y >= 0 && pt.y < lpConfig->height) {
            cell = (struct MapCell *)&GameMap[pt.y][pt.x];
        } else {
            cell = NULL;
        }
        if ((cell->flags.word & 0xa0) == 0 && ((cell->flags.word & 8) == 0 || cell->obj != (struct MapCellObj *)roads)) {
            pt.y++;
        } else {
            sx = cell->src.b.byte_4;
            sy = cell->src.b.byte_5;
            if (sx >= 0 && sx < lpConfig->width && sy >= 0 && sy < lpConfig->height) {
                src = (struct MapCell *)&GameMap[sy][sx];
            } else {
                src = NULL;
            }
            tile = src->obj->ride;
            entry = &MapRenderOrderList[DAT_00801408];
            DAT_00801408++;
            if (DAT_00801408 == 0x1000) {
                DAT_00801408 = 0;
            }
            entry->coords = cell->src.coords;
            entry->x = (unsigned char)pt.x;
            entry->flag = 1;
            entry->height = tile->footprint_y1 + sy + 1;
            if (pt.x == tile->footprint_x1 + sx || pt.x == lpConfig->width - 1) {
                *out_coords = src->src.coords;
                FUN_0045a430(cell->src.coords, &pt.x);
                out_coords = &src->field_6;
            } else {
                pt.x++;
                FUN_0045a3e0(&pt.x);
            }
        }
        while (pt.y >= lpConfig->height) {
            pt.x++;
            FUN_0045a3e0(&pt.x);
        }
    }
    *out_coords = 0;
}

// FUNCTION: LEGOLAND 0x0045a850
LEGO_EXPORT MapElement *GetFirstRenderObject(void) {
    int x;
    int y;
    MapElement *element;

    x = ((unsigned char *)&DAT_007febb8)[0];
    y = ((unsigned char *)&DAT_007febb8)[1];
    if (x < 0 || x >= (int)lpConfig->width || y < 0 || y >= (int)lpConfig->height) {
        return 0;
    }
    element = &GameMap[y][x];
    if (element == 0) {
        return 0;
    }
    if (DAT_007febb8 == 0 && (element->flags & 0xa8) == 0) {
        return 0;
    }
    return element;
}

// FUNCTION: LEGOLAND 0x0045a8b0
LEGO_EXPORT MapElement *GetNextRenderObject(MapElement *object) {
    if (object != 0 && object->next.id != 0) {
        Point p;
        p.x = object->next.pos.x;
        p.y = object->next.pos.y;
        return GetTileAtPoint(&p);
    }
    return 0;
}

// FUNCTION: LEGOLAND 0x0045a910
LEGO_EXPORT MapElement *GetFirstObjectMatching(Element *cls) {
    MapElement *object = GetFirstRenderObject();
    while (object != NULL) {
        if (object->field_0 == cls) {
            return object;
        }
        object = GetNextRenderObject(object);
    }
    return NULL;
}

// FUNCTION: LEGOLAND 0x0045a940
LEGO_EXPORT MapElement *GetNextObjectMatching(MapElement *object, Element *cls) {
    object = GetNextRenderObject(object);
    while (object != NULL) {
        if (object->field_0 == cls) {
            return object;
        }
        object = GetNextRenderObject(object);
    }
    return NULL;
}

// FUNCTION: LEGOLAND 0x0045a970
LEGO_EXPORT struct Point PlayfieldToMap(struct Point pos) {
    int width;
    int height;
    struct Point result;

    GetTileDimensions(&width, &height);
    result.x = pos.x / width + pos.y / height;
    result.y = pos.y / height - pos.x / width;
    return result;
}
