#include <stdlib.h>
#include <string.h>
#include "globals.h"
#include "legoland.h"

#include "bloke.h"
#include "bricks.h"
#include "driving_school.h"
#include "gamemap.h"
#include "llidb.h"
#include "map_object.h"
#include "objclass.h"
#include "print_sprite.h"
#include "pumps.h"
#include "render3d.h"
#include "ride_queue.h"
#include "roads.h"
#include "tilemap.h"

struct RoadQueueEntry {
    struct RoadQueueEntry *next;
    unsigned char pad_4[0x4];
    unsigned short field_8;
    unsigned char pad_a[0x2];
    int x;
    int y;
    unsigned char field_14;
    unsigned char pad_15[0x7];
    unsigned char field_1c;
    unsigned char field_1d;
    unsigned char pad_1e[0x2];
};

struct RoadTile {
    unsigned char pad_0[0xc];
    unsigned int x;
    unsigned int y;
    unsigned char flags;
    unsigned char pad_15[0x7];
    unsigned char field_1c;
    unsigned char field_1d;
};

struct RoadCallbacks {
    unsigned char pad_0[0x18];
    void *field_18;
    void *field_1c;
    void *field_20;
};

struct LLIDB_Head {
    unsigned char pad_0[0xc];
    struct RoadCallbacks *callbacks;
};

struct RoadEditArg {
    unsigned char pad_0[0xc];
    void *ride;
};

struct RoadPlaceArg {
    unsigned int x;
    unsigned int y;
};

struct NeighborResult {
    struct RoadQueueEntry *field_0;
    struct RoadQueueEntry *field_4;
    struct RoadQueueEntry *field_8;
    struct RoadQueueEntry *field_c;
    struct RoadQueueEntry *field_10;
    struct RoadQueueEntry *field_14;
    struct RoadQueueEntry *field_18;
    struct RoadQueueEntry *field_1c;
};

// FUNCTION: LEGOLAND 0x004132a0
void FUN_004132a0(TileId tile, int param_2, int param_3, unsigned int param_4, unsigned int param_5) {
    struct RoadQueueEntry *entry = malloc(0x20);

    entry->field_8 = tile.id;
    entry->x = param_2;
    entry->y = param_3;
    entry->field_14 = (unsigned char)param_4;
    entry->next = (struct RoadQueueEntry *)DAT_004cbeac;
    entry->field_1c = 0;
    entry->field_1d = 0;
    DAT_004cbeac = (struct RideQueueEntry *)entry;

    FUN_00412680(param_2, param_3, param_4, param_5);

    Set_UserFlags(param_2 << 8, param_3 << 8, 0);
    Set_UserFlags((param_2 + 1) << 8, param_3 << 8, 0);
    Set_UserFlags((param_2 + 2) << 8, param_3 << 8, 0);
    Set_UserFlags((param_2 + 3) << 8, param_3 << 8, 0);

    Set_UserFlags(param_2 << 8, (param_3 + 1) << 8, 0);
    Set_UserFlags((param_2 + 1) << 8, (param_3 + 1) << 8, 0);
    Set_UserFlags((param_2 + 2) << 8, (param_3 + 1) << 8, 0);
    Set_UserFlags((param_2 + 3) << 8, (param_3 + 1) << 8, 0);

    Set_UserFlags(param_2 << 8, (param_3 + 2) << 8, 0);
    Set_UserFlags((param_2 + 1) << 8, (param_3 + 2) << 8, 0);
    Set_UserFlags((param_2 + 2) << 8, (param_3 + 2) << 8, 0);
    Set_UserFlags((param_2 + 3) << 8, (param_3 + 2) << 8, 0);

    Set_UserFlags(param_2 << 8, (param_3 + 3) << 8, 0);
    Set_UserFlags((param_2 + 1) << 8, (param_3 + 3) << 8, 0);
    Set_UserFlags((param_2 + 2) << 8, (param_3 + 3) << 8, 0);
    Set_UserFlags((param_2 + 3) << 8, (param_3 + 3) << 8, 0);
}

// FUNCTION: LEGOLAND 0x004133e0
void FUN_004133e0(int param_1, int param_2) {
    struct RoadQueueEntry *entry = (struct RoadQueueEntry *)FUN_004125a0(param_1, param_2);
    struct PumpNode *pump;
    struct RoadQueueEntry *next;
    struct RoadQueueEntry *cur;

    if (entry == NULL) {
        return;
    }

    next = entry->next;
    pump = FUN_00411aa0(entry->x - 1, entry->y + 1);
    if (pump != NULL) {
        FUN_00411b20(pump);
    }

    free(entry);

    if (entry == (struct RoadQueueEntry *)DAT_004cbeac) {
        DAT_004cbeac = (struct RideQueueEntry *)next;
        return;
    }

    cur = (struct RoadQueueEntry *)DAT_004cbeac;
    while (cur->next != entry) {
        cur = cur->next;
    }
    cur->next = next;
}

// FUNCTION: LEGOLAND 0x00413450
int FUN_00413450(int x, int y, struct RideQueueEntry **out) {
    int count = 0;
    struct RideQueueEntry *e;
    int x1 = x + 4;
    int y0 = y - 4;
    e = FUN_004125a0(x1, y0);
    if (e != NULL) {
        count = 1;
    }
    if (out != NULL) {
        out[1] = e;
    }
    y += 4;
    e = FUN_004125a0(x1, y);
    if (e != NULL) {
        count++;
    }
    if (out != NULL) {
        out[3] = e;
    }
    x1 = x - 4;
    e = FUN_004125a0(x1, y);
    if (e != NULL) {
        count++;
    }
    if (out != NULL) {
        out[5] = e;
    }
    e = FUN_004125a0(x1, y0);
    if (e != NULL) {
        count++;
    }
    if (out != NULL) {
        out[7] = e;
    }
    return count;
}

// FUNCTION: LEGOLAND 0x004134f0
struct RoadTile *FUN_004134f0(int arg1, int arg2, struct RoadTile *tile) {
    if (tile != NULL) {
        if ((tile->flags & 0xf) != 6) {
            return tile;
        }
        if (tile->x + 4 == arg1) {
            if (tile->y == arg2) {
                return tile;
            }
        }
    }
    return 0;
}

// FUNCTION: LEGOLAND 0x00413520
int FUN_00413520(int x, int y, struct NeighborResult *out) {
    struct NeighborResult r;
    struct RoadTile *t3;
    int count = 0;

    FUN_004135d0(x, y, &r);
    r.field_0 = (struct RoadQueueEntry *)FUN_004134f0(x, y, (struct RoadTile *)r.field_0);
    if (r.field_0 != NULL) {
        count = 1;
    }
    r.field_8 = (struct RoadQueueEntry *)FUN_004134f0(x, y, (struct RoadTile *)r.field_8);
    if (r.field_8 != NULL) {
        count++;
    }
    r.field_10 = (struct RoadQueueEntry *)FUN_004134f0(x, y, (struct RoadTile *)r.field_10);
    if (r.field_10 != NULL) {
        count++;
    }
    t3 = FUN_004134f0(x, y, (struct RoadTile *)r.field_18);
    if (t3 != NULL) {
        count++;
    }
    if (out != NULL) {
        out->field_0 = r.field_0;
        out->field_8 = r.field_8;
        out->field_10 = r.field_10;
        out->field_18 = (struct RoadQueueEntry *)t3;
    }
    return count;
}

// FUNCTION: LEGOLAND 0x004135d0
int FUN_004135d0(int x, int y, struct NeighborResult *out) {
    struct RoadQueueEntry *e;
    int count = 0;

    e = (struct RoadQueueEntry *)FUN_004125a0(x, y - 4);
    if (e != NULL) {
        count = 1;
    }
    if (out != NULL) {
        out->field_0 = e;
    }

    e = (struct RoadQueueEntry *)FUN_004125a0(x + 4, y);
    if (e != NULL) {
        count++;
    }
    if (out != NULL) {
        out->field_8 = e;
    }

    e = (struct RoadQueueEntry *)FUN_004125a0(x, y + 4);
    if (e != NULL) {
        count++;
    }
    if (out != NULL) {
        out->field_10 = e;
    }

    e = (struct RoadQueueEntry *)FUN_004125a0(x - 4, y);
    if (e != NULL) {
        count++;
    }
    if (out != NULL) {
        out->field_18 = e;
    }

    return count;
}

// FUNCTION: LEGOLAND 0x00413650
void FUN_00413650(unsigned short param_1, int param_2, int param_3) {
    struct NeighborResult r;
    struct RideQueueEntry *e;
    int count = 0;
    int bits = 0;
    int mask = 0;
    int flag = 0;
    unsigned int m;
    unsigned short id;

    id = param_1;
    e = FUN_004125a0(param_2, param_3);
    if (e != NULL) {
        flag = e->field_14 & 0x10;
    }
    FUN_00413520(param_2, param_3, &r);
    if (r.field_0 != NULL) {
        if (r.field_0->field_8 == id) {
            mask = count = 1;
        }
    }
    if (r.field_8 != NULL && r.field_8->field_8 == id) {
        count++;
        mask |= 2;
    }
    if (r.field_10 != NULL && r.field_10->field_8 == id) {
        count++;
        mask |= 4;
    }
    if (r.field_18 != NULL && r.field_18->field_8 == id) {
        count++;
        mask |= 8;
    }
    switch (count) {
    case 0:
        return;
    case 2:
        if ((mask & 5) != 5 && (mask & 0xa) != 0xa) {
            switch (mask) {
            case 6:
                FUN_00412680(param_2, param_3, 3, 0);
                break;
            case 12:
                FUN_00412680(param_2, param_3, 3, 1);
                break;
            case 9:
                FUN_00412680(param_2, param_3, 3, 2);
                break;
            case 3:
                FUN_00412680(param_2, param_3, 3, 3);
                break;
            }
            break;
        }
    case 1:
        FUN_00413450(param_2, param_3, (struct RideQueueEntry **)&r);
        if (r.field_0 != NULL && r.field_0->field_8 == id) {
            bits = 1;
        }
        if (r.field_4 != NULL && r.field_4->field_8 == id) {
            bits |= 2;
        }
        if (r.field_8 != NULL && r.field_8->field_8 == id) {
            bits |= 4;
        }
        if (r.field_c != NULL && r.field_c->field_8 == id) {
            bits |= 8;
        }
        if (r.field_10 != NULL && r.field_10->field_8 == id) {
            bits |= 0x10;
        }
        if (r.field_14 != NULL && r.field_14->field_8 == id) {
            bits |= 0x20;
        }
        if (r.field_18 != NULL && r.field_18->field_8 == id) {
            bits |= 0x40;
        }
        if (r.field_1c != NULL && r.field_1c->field_8 == id) {
            bits |= 0x80;
        }
        m = bits;
        if ((m & 0x11) != 0) {
            if ((m & 0x83) == 0x83) {
                if ((m & 0x38) == 0x38) {
                    FUN_00412680(param_2, param_3, flag | 7, 0);
                } else {
                    FUN_00412680(param_2, param_3, flag | 1, 0);
                }
            } else {
                if ((m & 0x38) == 0x38) {
                    FUN_00412680(param_2, param_3, flag | 1, 2);
                } else {
                    FUN_00412680(param_2, param_3, flag, 0);
                }
            }
        } else if ((m & 0xe0) == 0xe0) {
            if ((m & 0xe) == 0xe) {
                FUN_00412680(param_2, param_3, flag | 7, 1);
            } else {
                FUN_00412680(param_2, param_3, flag | 1, 3);
            }
        } else if ((m & 0xe) == 0xe) {
            FUN_00412680(param_2, param_3, flag | 1, 1);
        } else {
            FUN_00412680(param_2, param_3, flag, 1);
        }
        return;
    case 3:
        switch (mask) {
        case 11:
            FUN_00412680(param_2, param_3, 4, 0);
            break;
        case 7:
            FUN_00412680(param_2, param_3, 4, 1);
            break;
        case 14:
            FUN_00412680(param_2, param_3, 4, 2);
            break;
        case 13:
            FUN_00412680(param_2, param_3, 4, 3);
            break;
        }
        break;
    case 4:
        FUN_00412680(param_2, param_3, 5, 0);
        break;
    }
    if (flag != 0) {
        AddBricks(GetObjCost(ZebraCrossingRide));
    }
}

// FUNCTION: LEGOLAND 0x00413970
unsigned int FUN_00413970(unsigned short param_1) {
    struct RoadQueueEntry *entry = (struct RoadQueueEntry *)DAT_004cbeac;
    unsigned int count = 0;

    while (entry != NULL) {
        if (entry->field_8 == param_1) {
            count++;
        }
        entry = entry->next;
    }
    return count;
}

// FUNCTION: LEGOLAND 0x00413990
unsigned char FUN_00413990(unsigned int param_1, unsigned int param_2) {
    struct RoadTile *tile = FindQueueEntryAtTile(param_1 >> 8, param_2 >> 8);
    if (tile == NULL) {
        return 2;
    }
    if (tile->field_1c == 0) {
        return 1;
    }
    return 3;
}

// FUNCTION: LEGOLAND 0x004139c0
void FUN_004139c0(unsigned int param_1, unsigned int param_2) {
    struct RoadTile *tile = FindQueueEntryAtTile(param_1 >> 8, param_2 >> 8);
    if (tile != NULL) {
        tile->field_1d++;
    }
}

// FUNCTION: LEGOLAND 0x004139e0
void FUN_004139e0(unsigned int param_1, unsigned int param_2) {
    struct RoadTile *tile = FindQueueEntryAtTile(param_1 >> 8, param_2 >> 8);
    if (tile != NULL && tile->field_1d != 0) {
        tile->field_1d--;
    }
}

// FUNCTION: LEGOLAND 0x00413a10
void LoadDrivingSchoolRoadsResources(struct LLIDB_Head *head) {
    struct LLIDB_Head **handle = &head;

    DAT_0082c684 = head->callbacks;
    // STRING: LEGOLAND 0x004b4c94
    if (LLIDB_FindElement("DSCHOOL LIGHTS", (unsigned int *)handle, 0) == 0) {
        DrivingSchoolLightsData = LLIDB_LoadData(head);
    }
    // STRING: LEGOLAND 0x004b4c80
    if (LLIDB_FindElement("TILES FOR DSCHOOL", (unsigned int *)handle, 0) == 0) {
        struct RoadCallbacks *callbacks = head->callbacks;
        callbacks->field_18 = FUN_00413990;
        callbacks->field_1c = FUN_004139c0;
        callbacks->field_20 = FUN_004139e0;
    }
}

// FUNCTION: LEGOLAND 0x00413a80
void UnloadDrivingSchoolRoadsResources(void) {
    struct RoadQueueEntry *entry = (struct RoadQueueEntry *)DAT_004cbeac;
    unsigned int handle;
    struct RoadQueueEntry *next;

    if (LLIDB_FindElement("DSCHOOL LIGHTS", &handle, 0) == 0) {
        LLIDB_UnLoadData(handle);
    }

    while (entry != NULL) {
        next = entry->next;
        free(entry);
        entry = next;
    }
}

// FUNCTION: LEGOLAND 0x00413ad0
void DrivingSchoolRoadsSetEditMode(void) {
    EditMode.unk0 = 1;
    EditMode.unk8 = DAT_0082c684;
    DefaultCursor(&EditCursor);
    SetEditCursorFootPrint(&DAT_004b4bf0);
    EditCursor.field_1828 |= 8;
    BuildCursorPtr(&EditCursor, 0x8f8, 0);
    DefaultCursor(&DAT_0082f760);
    memcpy(DAT_0082f760.field_1414, &DAT_004b4bf0, 20);
    DAT_0082f760.field_1828 = 0x34;
}

// FUNCTION: LEGOLAND 0x00413b50
void FUN_00413b50(Element *obj, int *param_2, unsigned int param_3) {
    unsigned int bits;
    struct Ride *ride = obj->ride;
    struct RoadTile *t;

    struct NeighborResult r;
    struct MapRect rect;
    struct RoadQueueEntry **q = (struct RoadQueueEntry **)&r;
    unsigned short type;
    int result;
    int cost;

    memcpy(EditCursor.field_1414, &ride->footprint, 20);
    bits = 0;
    EditCursor.field_1830 = bits;
    ScreenToMapRef(param_2, (int *)&EditCursor.tile_x, param_3);
    t = FUN_00413e30(&EditCursor);
    ValidateCursor(&EditCursor, (unsigned int)ride);
    if (FUN_0045f4b0(&EditCursor) == 0) {
        return;
    }
    rect.x0 = EditCursor.field_1414[0] + EditCursor.tile_x;
    rect.y0 = EditCursor.field_1414[1] + EditCursor.tile_y;
    rect.x1 = EditCursor.field_1414[2] + EditCursor.tile_x;
    rect.y1 = EditCursor.field_1414[3] + EditCursor.tile_y;
    result = CheckForPeople(&rect);
    switch (result) {
    case -1:
        FUN_0045f480(&EditCursor, 4);
        break;
    case 1:
        FUN_0045f480(&EditCursor, 3);
        break;
    default:
        cost = GetObjCost(ride);
        if (GetBrickCount() < cost) {
            FUN_0045f480(&EditCursor, 2);
        } else {
            if (FUN_0045f4b0(&EditCursor) != 0) {
                if (FUN_00413520(EditCursor.tile_x, EditCursor.tile_y, &r) == 0) {
                    FUN_0045f480(&EditCursor, 0xe);
                } else {
                    if (q[0] != NULL) {
                        type = q[0]->field_8;
                    } else if (q[2] != NULL) {
                        type = q[2]->field_8;
                    } else if (q[4] != NULL) {
                        type = q[4]->field_8;
                    } else if (q[6] != NULL) {
                        type = q[6]->field_8;
                    } else {
                        type = (unsigned short)(unsigned int)t;
                    }
                    FUN_004135d0(EditCursor.tile_x, EditCursor.tile_y, &r);
                    FUN_00413450(EditCursor.tile_x, EditCursor.tile_y, (struct RideQueueEntry **)&r);
                    if (q[0] != NULL && q[0]->field_8 == type) bits = 1;
                    if (q[1] != NULL && q[1]->field_8 == type) bits |= 2;
                    if (q[2] != NULL && q[2]->field_8 == type) bits |= 4;
                    if (q[3] != NULL && q[3]->field_8 == type) bits |= 8;
                    if (q[4] != NULL && q[4]->field_8 == type) bits |= 0x10;
                    if (q[5] != NULL && q[5]->field_8 == type) bits |= 0x20;
                    if (q[6] != NULL && q[6]->field_8 == type) bits |= 0x40;
                    if (q[7] != NULL && q[7]->field_8 == type) bits |= 0x80;
                    if ((bits & 7) == 7 || (bits & 0x1c) == 0x1c || (bits & 0x70) == 0x70 || (bits & 0xc1) == 0xc1) {
                        FUN_0045f480(&EditCursor, 0xe);
                    }
                }
            }
            if (t != NULL && FUN_0045f4b0(&EditCursor) != 0) {
                memcpy(DAT_0082f760.field_1414, DAT_004b4bf0, 20);
                EditCursor.field_1830 = (unsigned int)&DAT_0082f760;
                DAT_0082f760.tile_x = t->x;
                DAT_0082f760.tile_y = t->y;
                DAT_0082f760.field_1830 = 0;
                DAT_0082f760.field_1828 = 0x2034;
            } else {
                EditCursor.field_1830 = 0;
            }
        }
    }
}

// FUNCTION: LEGOLAND 0x00413e30
struct RoadTile *FUN_00413e30(struct Cursor *cur) {
    struct RoadTile *t = FindQueueEntryAtTile(cur->tile_x, cur->tile_y);
    struct RoadTile *a;
    struct RoadTile *b;
    struct RoadTile *c;
    struct RoadTile *d;
    struct RoadTile *r;
    if (t) {
        cur->tile_x = t->x;
        cur->tile_y = t->y;
    }
    a = FindQueueEntryAtTile(cur->tile_x, cur->tile_y - 4);
    if (a && (a->flags & 0xf) == 6) a = NULL;
    b = FindQueueEntryAtTile(cur->tile_x + 4, cur->tile_y);
    if (b && (b->flags & 0xf) == 6) b = NULL;
    c = FindQueueEntryAtTile(cur->tile_x, cur->tile_y + 4);
    if (c && (c->flags & 0xf) == 6) c = NULL;
    d = FindQueueEntryAtTile(cur->tile_x - 4, cur->tile_y);
    if (d && (d->flags & 0xf) == 6) d = NULL;
    r = NULL;
    if (a) {
        cur->tile_x = a->x;
        cur->tile_y = a->y + 4;
        r = a;
    } else if (b) {
        cur->tile_x = b->x - 4;
        cur->tile_y = b->y;
        r = b;
    } else if (c) {
        cur->tile_x = c->x;
        cur->tile_y = c->y - 4;
        r = c;
    } else if (d) {
        cur->tile_x = d->x + 4;
        cur->tile_y = d->y;
        r = d;
    }
    return r;
}

// FUNCTION: LEGOLAND 0x00413fa0
void FUN_00413fa0(unsigned int dummy, struct RoadPlaceArg *param) {
    struct RoadTile *tile = FindQueueEntryAtTile(param->x, param->y);
    unsigned int *src;
    unsigned int *dst;
    unsigned int count;

    if (tile != NULL) {
        QueryCursor.tile_x = tile->x;
        QueryCursor.tile_y = tile->y;

        src = &DAT_004b4bf0[0];
        dst = &QueryCursor.field_1414[0];
        count = 5;
        while (count != 0) {
            *dst = *src;
            src++;
            dst++;
            count--;
        }

        QueryCursor.field_1828 = 8;
        FUN_0045f480(&QueryCursor, 1);

        if ((tile->flags & 0xf) != 6) {
            FUN_0045f460(&QueryCursor);
        }
    }
}

// FUNCTION: LEGOLAND 0x00414020
void FUN_00414020(struct RoadEditArg *edit, struct RoadPlaceArg *place) {
    struct NeighborResult r;
    struct RoadQueueEntry **q = (struct RoadQueueEntry **)&r;
    TileId id;
    int x = place->x;
    int y = place->y;

    FUN_00413520(x, y, &r);
    if (r.field_0) {
        id.id = r.field_0->field_8;
    } else if (r.field_8) {
        id.id = r.field_8->field_8;
    } else if (r.field_10) {
        id.id = r.field_10->field_8;
    } else if (r.field_18) {
        id.id = r.field_18->field_8;
    }
    FUN_004132a0(id, x, y, 0, 0);
    FUN_00413650(id.id, x, y);
    FUN_00405310(id);
    FUN_00413450(x, y, (struct RideQueueEntry **)&r);
    if (q[0] && q[0]->field_8 == id.id && (q[0]->field_14 & 0xf) != 6) FUN_00413650(id.id, x, y - 4);
    if (q[1] && q[1]->field_8 == id.id && (q[1]->field_14 & 0xf) != 6) FUN_00413650(id.id, x + 4, y - 4);
    if (q[2] && q[2]->field_8 == id.id && (q[2]->field_14 & 0xf) != 6) FUN_00413650(id.id, x + 4, y);
    if (q[3] && q[3]->field_8 == id.id && (q[3]->field_14 & 0xf) != 6) FUN_00413650(id.id, x + 4, y + 4);
    if (q[4] && q[4]->field_8 == id.id && (q[4]->field_14 & 0xf) != 6) FUN_00413650(id.id, x, y + 4);
    if (q[5] && q[5]->field_8 == id.id && (q[5]->field_14 & 0xf) != 6) FUN_00413650(id.id, x - 4, y + 4);
    if (q[6] && q[6]->field_8 == id.id && (q[6]->field_14 & 0xf) != 6) FUN_00413650(id.id, x - 4, y);
    if (q[7] && q[7]->field_8 == id.id && (q[7]->field_14 & 0xf) != 6) FUN_00413650(id.id, x - 4, y - 4);
    FUN_00406020(id.id, 1);
    IncrementObjectCount((struct ObjectCount *)edit->ride);
}

// FUNCTION: LEGOLAND 0x00414220
void FUN_00414220(Element *edit, TileId tile, struct Cursor *cursor) {
    struct NeighborResult r;
    struct RoadQueueEntry **q = (struct RoadQueueEntry **)&r;
    int x = cursor->tile_x;
    int y = cursor->tile_y;
    struct RoadQueueEntry *entry;

    BGFullUpdate = 1;
    StandardRemoveObject(edit, tile, cursor);
    entry = (struct RoadQueueEntry *)FUN_004125a0(x, y);
    tile.id = entry->field_8;
    if (entry->field_14 & 0x10) {
        IncrementObjectCount((struct ObjectCount *)edit->data);
        DecrementObjectCount((struct ObjectCount *)ZebraCrossingRide);
        entry->field_14 &= 0xef;
        FUN_00413650(tile.id, x, y);
        AddBricks(GetObjCost((struct Ride *)ZebraCrossingRide));
        return;
    }
    FUN_00406020(tile.id, -1);
    FUN_004133e0(x, y);
    FUN_00405310(tile);
    FUN_004135d0(x, y, &r);
    FUN_00413450(x, y, (struct RideQueueEntry **)&r);
    if (q[0] && q[0]->field_8 == tile.id && (q[0]->field_14 & 0xf) != 6) FUN_00413650(tile.id, x, y - 4);
    if (q[1] && q[1]->field_8 == tile.id && (q[1]->field_14 & 0xf) != 6) FUN_00413650(tile.id, x + 4, y - 4);
    if (q[2] && q[2]->field_8 == tile.id && (q[2]->field_14 & 0xf) != 6) FUN_00413650(tile.id, x + 4, y);
    if (q[3] && q[3]->field_8 == tile.id && (q[3]->field_14 & 0xf) != 6) FUN_00413650(tile.id, x + 4, y + 4);
    if (q[4] && q[4]->field_8 == tile.id && (q[4]->field_14 & 0xf) != 6) FUN_00413650(tile.id, x, y + 4);
    if (q[5] && q[5]->field_8 == tile.id && (q[5]->field_14 & 0xf) != 6) FUN_00413650(tile.id, x - 4, y + 4);
    if (q[6] && q[6]->field_8 == tile.id && (q[6]->field_14 & 0xf) != 6) FUN_00413650(tile.id, x - 4, y);
    if (q[7] && q[7]->field_8 == tile.id && (q[7]->field_14 & 0xf) != 6) FUN_00413650(tile.id, x - 4, y - 4);
}

// FUNCTION: LEGOLAND 0x00414440
void FUN_00414440(void) {
    struct RideQueueEntry *e = DAT_004cbeac;
    int ia;
    int ib;
    int ic;
    int id;
    int w;
    int h;
    struct Point pt;
    struct Point ref;
    int b[4];

    DAT_004b4c04--;
    if (DAT_004b4c04 <= 0) {
        switch (DAT_004cbeb8) {
        case 0:
            DAT_004cbeb0 = 0;
            DAT_004b4c04 = 0x28;
            break;
        case 1:
            DAT_004cbeb4 = 1;
            DAT_004b4c04 = 0x8c;
            break;
        case 2:
            DAT_004cbeb4 = 0;
            DAT_004b4c04 = 0x28;
            break;
        case 3:
            DAT_004cbeb0 = 1;
            DAT_004b4c04 = 0x8c;
            break;
        }
        DAT_004cbeb8 = (DAT_004cbeb8 + 1) & 3;
    }
    if (DAT_004c11c0 != 0) {
        if (DAT_004cbeb0 != 0) {
            id = 6;
            ib = 1;
            ic = 2;
            ia = 5;
        } else if (DAT_004cbeb4 != 0) {
            id = 7;
            ib = 0;
            ic = 3;
            ia = 4;
        } else {
            id = 7;
            ib = 1;
            ic = 3;
            ia = 5;
        }
    } else if (DAT_004cbeb0 != 0) {
        id = 9;
        ib = 10;
        ic = 13;
        ia = 14;
    } else if (DAT_004cbeb4 != 0) {
        id = 8;
        ib = 11;
        ic = 12;
        ia = 15;
    } else {
        id = 9;
        ib = 11;
        ic = 13;
        ia = 15;
    }
    for (; e != NULL; e = e->next) {
        if ((e->field_14 & 0xf) != 5) {
            continue;
        }
        GetTileDimensions(&w, &h);
        pt.x = DrivingSchoolLightsData->off_x[ia & 0xff] >> 1;
        pt.y = DrivingSchoolLightsData->off_y[ia & 0xff] >> 1;
        AdjustOffsetForViewMode(&pt);
        ref.x = e->x;
        ref.y = e->y;
        GetTileBounds(&ref, b);
        SortSprite(DrivingSchoolLightsData->sprites[ia & 0xff], b[0] + pt.x, b[1] + pt.y, b[1] - lpConfig->view_y, 0, 0);

        ref.x = e->x + 3;
        ref.y = e->y;
        GetTileBounds(&ref, b);
        pt.x = DrivingSchoolLightsData->off_x[id & 0xff] >> 1;
        pt.y = DrivingSchoolLightsData->off_y[id & 0xff] >> 1;
        AdjustOffsetForViewMode(&pt);
        SortSprite(DrivingSchoolLightsData->sprites[id & 0xff], b[0] + pt.x, b[1] + pt.y, ((b[1] + b[3]) >> 1) - lpConfig->view_y, 0, 0);

        ref.x = e->x + 3;
        ref.y = e->y + 3;
        GetTileBounds(&ref, b);
        pt.x = DrivingSchoolLightsData->off_x[ib & 0xff] >> 1;
        pt.y = DrivingSchoolLightsData->off_y[ib & 0xff] >> 1;
        AdjustOffsetForViewMode(&pt);
        SortSprite(DrivingSchoolLightsData->sprites[ib & 0xff], b[0] + pt.x, b[1] + pt.y, b[3] - lpConfig->view_y, 0, 0);

        ref.x = e->x;
        ref.y = e->y + 3;
        GetTileBounds(&ref, b);
        pt.x = DrivingSchoolLightsData->off_x[ic & 0xff] >> 1;
        pt.y = DrivingSchoolLightsData->off_y[ic & 0xff] >> 1;
        AdjustOffsetForViewMode(&pt);
        SortSprite(DrivingSchoolLightsData->sprites[ic & 0xff], b[0] + pt.x, b[1] + pt.y, ((b[1] + b[3]) >> 1) - lpConfig->view_y, 0, 0);
    }
}

// FUNCTION: LEGOLAND 0x00414830
void ZebraCrossingSetEditMode(void) {
    EditMode.unk0 = 1;
    EditMode.unk8 = ZebraCrossingRide;
    DefaultCursor(&EditCursor);
    SetEditCursorFootPrint(&DAT_004b4bf0);
    EditCursor.field_1828 |= 8;
    BuildCursorPtr(&EditCursor, 0, 0);
}

// FUNCTION: LEGOLAND 0x00414880
void FUN_00414880(struct RoadEditArg *param_1, unsigned int param_2, unsigned int param_3) {
    void *obj = param_1->ride;
    struct RoadTile *tile;
    int cost;

    memcpy(EditCursor.field_1414, (char *)obj + 0x3c, 20);
    EditCursor.field_1830 = 0;
    ScreenToMapRef(param_2, &EditCursor.tile_x, param_3);
    tile = FindQueueEntryAtTile(EditCursor.tile_x, EditCursor.tile_y);
    FUN_0045f480(&EditCursor, 0xe);
    cost = GetObjCost(obj);
    if (GetBrickCount() < cost) {
        return;
    }
    if (tile == NULL) {
        return;
    }
    EditCursor.tile_x = tile->x;
    EditCursor.tile_y = tile->y;
    if (tile->flags & 0x10) {
        return;
    }
    if ((tile->flags & 0xf) == 3) return;
    if ((tile->flags & 0xf) == 4) return;
    if ((tile->flags & 0xf) == 5) return;
    if ((tile->flags & 0xf) == 6) return;
    if (FUN_00411aa0(tile->x - 1, tile->y + 1) != 0) {
        return;
    }
    FUN_0045f460(&EditCursor);
}

// FUNCTION: LEGOLAND 0x00414940
void InitZebraCrossing(struct RoadEditArg *param_1) {
    ZebraCrossingRide = param_1->ride;
}

// FUNCTION: LEGOLAND 0x00414950
void FUN_00414950(struct RoadEditArg *edit, struct RoadPlaceArg *place) {
    struct NeighborResult r;
    struct RoadTile *tile = (struct RoadTile *)FUN_004125a0(place->x, place->y);

    if (tile != NULL) {
        FUN_004135d0(place->x, place->y, &r);
        tile->flags |= 0x10;
        FUN_00413650(*(short *)((char *)tile + 8), place->x, place->y);
        tile->field_1d = 0;
        tile->field_1c = 0;
        IncrementObjectCount((struct ObjectCount *)edit->ride);
    }
}
