#include <stdlib.h>
#include <string.h>
#include "legoland.h"

#include "bricks.h"
#include "gamemap.h"
#include "globals.h"
#include "map_object.h"
#include "obj_instance.h"
#include "objclass.h"
#include "pumps.h"
#include "ride_queue.h"
#include "tilemap.h"

struct PumpSource {
    unsigned char pad_0[0xc];
    unsigned int var_c;
};

// FUNCTION: LEGOLAND 0x00411a10
void InitDrivingSchoolPumps(struct PumpSource *param_1) {
    DrivingSchoolPumpsRide = param_1->var_c;
}

// FUNCTION: LEGOLAND 0x00411a20
void DrivingSchoolPumpsSetEditMode(void) {
    unsigned int eax_temp;

    eax_temp = DrivingSchoolPumpsRide;
    EditMode.unk0 = 1;
    EditMode.unk8 = (void *)eax_temp;
    DefaultCursor(&EditCursor);
    SetEditCursorFootPrint(DAT_004b4bd0);
    EditCursor.field_1828 = EditCursor.field_1828 | 8;
    BuildCursorPtr(&EditCursor, 0x8f8, 0);
    DefaultCursor(&DAT_0082f760);
    memcpy(DAT_0082f760.field_1414, DAT_004b4bd0, 0x14);
    DAT_0082f760.field_1828 = 0x34;
}

// FUNCTION: LEGOLAND 0x00411aa0
struct PumpNode *FUN_00411aa0(unsigned int arg1, unsigned int arg2) {
    struct PumpNode *node;

    node = (struct PumpNode *)PumpList;
    while (node) {
        if (node->var_4 == arg1 && node->var_8 == arg2) {
            return node;
        }
        node = node->next;
    }
    return NULL;
}

// FUNCTION: LEGOLAND 0x00411ad0
void FUN_00411ad0(struct PumpNode *node) {
    struct PumpNode *next;
    struct PumpNode *current;

    if (PumpList) {
        next = node->next;
        free(node);
        if (PumpList == node) {
            PumpList = next;
        } else {
            current = (struct PumpNode *)PumpList;
            if (current->next != node) {
                do {
                    current = current->next;
                } while (current && current->next != node);
            }
            if (current) {
                current->next = next;
            }
        }
    }
}

// FUNCTION: LEGOLAND 0x00411b20
void FUN_00411b20(struct PumpNode *node) {
    struct Cursor cursor;
    TileId tile;
    struct Ride *ride;

    ride = (struct Ride *)DrivingSchoolPumpsRide;
    cursor.footprint = ride->footprint;
    tile.pos.x = (unsigned char)node->var_4;
    tile.pos.y = (unsigned char)node->var_8;
    cursor.tile_x = node->var_4;
    cursor.tile_y = node->var_8;
    StandardRemoveObject(ride->element, tile, &cursor);
    FUN_00411ad0(node);
}

// FUNCTION: LEGOLAND 0x00411ba0
void FUN_00411ba0(unsigned short param) {
    struct PumpNode *cur;
    struct PumpNode *next;

    cur = (struct PumpNode *)PumpList;
    if (cur) {
        do {
            next = cur->next;
            if (cur->var_2 == param) {
                FUN_00411b20(cur);
            }
            cur = next;
        } while (cur);
    }
}

// FUNCTION: LEGOLAND 0x00411bd0
void FUN_00411bd0(void) {
    struct PumpNode *current;
    struct PumpNode *next;

    current = (struct PumpNode *)PumpList;
    if (current == NULL) {
        return;
    }
    while (current != NULL) {
        next = current->next;
        FUN_00411b20(current);
        current = next;
    }
}

// FUNCTION: LEGOLAND 0x00411bf0
void DrivingSchoolPumpsAddObject(Element *obj, int *coords) {
    struct PumpTile *info;
    struct PumpNode *node;
    TileId tile;

    tile.pos.x = (unsigned char)coords[0];
    tile.pos.y = (unsigned char)coords[1];
    info = (struct PumpTile *)FindQueueEntryAtTile(coords[0] + 1, coords[1]);
    if (info) {
        AddBasicObject(obj, coords);
        node = (struct PumpNode *)malloc(sizeof(struct PumpNode));
        node->id = tile.id;
        node->var_2 = info->var_8;
        node->var_4 = tile.pos.x;
        node->var_8 = tile.pos.y;
        node->next = (struct PumpNode *)PumpList;
        PumpList = node;
    }
}

// FUNCTION: LEGOLAND 0x00411c70
void DrivingSchoolPumpsRemoveObject(void *param_1, TileId tile, struct Cursor *cursor) {
    struct PumpNode *node;

    BGFullUpdate = 1;
    node = FUN_00411aa0(tile.pos.x, tile.pos.y);
    if (node) {
        StandardRemoveObject(((struct Ride *)DrivingSchoolPumpsRide)->element, tile, cursor);
        FUN_00411ad0(node);
    }
}

// FUNCTION: LEGOLAND 0x00411cd0
void FUN_00411cd0(Element *obj, int *screen, unsigned int param_3) {
    struct Ride *ride;
    struct PumpTile *tile;

    ride = obj->ride;
    EditCursor.footprint = ride->footprint;
    EditCursor.field_1830 = 0;
    ScreenToMapRef(screen, (int *)&EditCursor.tile_x, param_3);
    tile = FUN_00411dc0(&EditCursor);
    ValidateCursor(&EditCursor, (unsigned int)ride);
    if (GetBrickCount() < GetObjCost(ride)) {
        FUN_0045f480(&EditCursor, 2);
        return;
    }
    if (tile != NULL && FUN_0045f4b0(&EditCursor)) {
        EditCursor.next = &DAT_0082f760;
        memcpy(DAT_0082f760.field_1414, DAT_004b4bf0, 0x14);
        DAT_0082f760.field_1830 = 0;
        DAT_0082f760.tile_x = tile->var_c;
        DAT_0082f760.tile_y = tile->var_10;
        DAT_0082f760.field_1828 = 0x2034;
        return;
    }
    EditCursor.field_1830 = 0;
    FUN_0045f480(&EditCursor, 0xe);
}

// FUNCTION: LEGOLAND 0x00411dc0
struct PumpTile *FUN_00411dc0(struct Cursor *cursor) {
    struct PumpTile *tile;
    int y;

    tile = (struct PumpTile *)FindQueueEntryAtTile(cursor->tile_x + 1, cursor->tile_y);
    if (tile != NULL && tile->var_14 != 0) {
        tile = NULL;
    }
    y = 0;
    if (tile != NULL) {
        cursor->tile_x = tile->var_c - 1;
        y = tile->var_10 - ((struct Ride *)DrivingSchoolPumpsRide)->footprint.y0;
        cursor->tile_y = y;
        y = (int)tile;
    }
    return (struct PumpTile *)y;
}
