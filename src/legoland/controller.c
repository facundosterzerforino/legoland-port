#include <stdlib.h>
#include "legoland.h"

#include "controller.h"
#include "globals.h"
#include "map_object.h"
#include "math.h"
#include "tilemap.h"
#include "timer.h"

#include <windows.h>

// FUNCTION: LEGOLAND 0x00451e70
LEGO_EXPORT int SetupControllers(void) {
    if (ControllersInitialized != 0) {
        return 1;
    }
    CONTROLLERBUFFER = malloc(sizeof(struct CtrlBuffer));
    CONTROLLERBUFFER->buttons = 0;
    DAT_00813a5c = 2;
    DAT_00813a4c = 1;
    DAT_00813ac8 = 2;
    GamePad = 0;
    DAT_00813a54 = 4;
    DAT_00813ac0 = 1;
    DAT_00813ab8 = 0x100;
    DAT_00813ad0 = 0x200;
    DAT_00813a98 = 0x20;
    DAT_00813aa0 = 0x10;
    DAT_00813aa8 = 0x40;
    DAT_00813ab0 = 8;
    DAT_00813ad8 = 0x400;
    MouseTileX = 10;
    MouseTileY = 4;
    FootprintWidth = 1;
    FootprintHeight = 1;
    if ((unsigned int)NULL != CONTROLLERBUFFER) {
        ControllersInitialized = 1;
        return 1;
    }
    return 0;
}

// FUNCTION: LEGOLAND 0x00451f40
void FreeControllers(void) {
    if (ControllersInitialized != 0) {
        free(CONTROLLERBUFFER);
        ControllersInitialized = 0;
    }
}

// FUNCTION: LEGOLAND 0x00451f70
int FUN_00451f70(void) {
    int dx;
    unsigned int dy;
    int result;

    result = 0;
    if ((GamePad & 0x20) == 0) {
        return 0;
    }
    if ((DAT_00813ab4 & 4) != 0) {
        dx = (unsigned int)lpConfig->field_24 * -2;
        ProcessScrolling(dx, 0);
    } else if ((DAT_00813aa4 & 4) != 0) {
        dx = (unsigned int)lpConfig->field_24 << 1;
        ProcessScrolling(dx, 0);
    }
    if ((DAT_00813a9c & 4) != 0) {
        dy = -(unsigned int)lpConfig->field_24;
        ProcessScrolling(0, dy);
    } else if ((DAT_00813aac & 4) != 0) {
        dy = (unsigned int)lpConfig->field_24;
        ProcessScrolling(0, dy);
    }
    if ((DAT_00813ab4 & 4) != 0 || (DAT_00813aa4 & 4) != 0 ||
        (DAT_00813a9c & 4) != 0 || (DAT_00813aac & 4) != 0) {
        result = 1;
    }
    return result;
}

// FUNCTION: LEGOLAND 0x00452030
void FUN_00452030(void) {
    int validate;
    int min_x;
    int min_y;
    int max_x;
    int max_y;
    struct Ride *cls;

    validate = 0;
    /* constrain the drag to its dominant axis */
    if ((GamePad & 0x800) != 0) {
        if (abs(DAT_00813a74 - DAT_00813a7c) > abs(DAT_00813a78 - DAT_00813a80)) {
            DAT_00813a80 = DAT_00813a78;
        } else {
            DAT_00813a7c = DAT_00813a74;
        }
    }
    if (EditMode.unk0 == 2) {
        min_x = DAT_00813a74 < DAT_00813a7c ? DAT_00813a74 : DAT_00813a7c;
        min_y = DAT_00813a78 < DAT_00813a80 ? DAT_00813a78 : DAT_00813a80;
        max_x = DAT_00813a74 > DAT_00813a7c ? DAT_00813a74 : DAT_00813a7c;
        max_y = DAT_00813a80;
        if (DAT_00813a78 > DAT_00813a80) {
            max_y = DAT_00813a78;
        }
        /* both arms end in the same stores; the compiler merges them */
        if (DAT_0080ff6c != NULL) {
            cls = DAT_0080ff6c;
            FootprintWidth = cls->footprint.x1 - cls->footprint.x0 + 1;
            FootprintHeight = cls->footprint.y1 - cls->footprint.y0 + 1;
            DAT_00813a90 = max_y;
            DAT_00813a8c = max_x;
            DAT_00813a88 = min_y;
            DAT_00813a84 = min_x;
        } else {
            FootprintWidth = 1;
            FootprintHeight = 1;
            DAT_00813a90 = max_y;
            DAT_00813a8c = max_x;
            DAT_00813a88 = min_y;
            DAT_00813a84 = min_x;
        }
    } else {
        min_x = DAT_00813a74 < DAT_00813a7c ? DAT_00813a74 : DAT_00813a7c;
        min_y = DAT_00813a78 < DAT_00813a80 ? DAT_00813a78 : DAT_00813a80;
        max_x = DAT_00813a74 > DAT_00813a7c ? DAT_00813a74 : DAT_00813a7c;
        max_y = DAT_00813a78 > DAT_00813a80 ? DAT_00813a78 : DAT_00813a80;
        cls = EditMode.unk8;
        FootprintWidth = cls->footprint.x1 - cls->footprint.x0 + 1;
        FootprintHeight = cls->footprint.y1 - cls->footprint.y0 + 1;
        if (memcmp(&DAT_00813a74, &DAT_00813a7c, 8) != 0) {
            /* snap the drag rectangle to whole footprints, anchored at the drag start */
            if (FootprintWidth > 1 && min_x == DAT_00813a7c && max_x == DAT_00813a74) {
                min_x = max_x - (FootprintWidth - min_x + max_x) / FootprintWidth * FootprintWidth;
                max_x = FootprintWidth + DAT_00813a74 - 1;
            } else {
                max_x = (FootprintWidth - min_x + max_x) / FootprintWidth * FootprintWidth + min_x - 1;
            }
            if (FootprintHeight > 1 && min_y == DAT_00813a80 && max_y == DAT_00813a78) {
                min_y = max_y - (FootprintHeight - min_y + max_y) / FootprintHeight * FootprintHeight;
                max_y = DAT_00813a78 + FootprintHeight - 1;
            } else {
                max_y = (FootprintHeight - min_y + max_y) / FootprintHeight * FootprintHeight + min_y - 1;
            }
            if (abs(min_x - max_x) <= FootprintWidth && abs(min_y - max_y) <= FootprintHeight) {
                validate = 1;
            }
        } else {
            /* single click: one footprint */
            validate = 1;
            max_x = (FootprintWidth - min_x + max_x) / FootprintWidth * FootprintWidth + min_x - 1;
            max_y = (FootprintHeight - min_y + max_y) / FootprintHeight * FootprintHeight + min_y - 1;
        }
        if (EditMode.unk0 == 1 && EditMode.unk8 != NULL) {
            DAT_00813a84 = EditMode.unk8->footprint.x0 + min_x;
            DAT_00813a88 = EditMode.unk8->footprint.y0 + min_y;
            DAT_00813a8c = EditMode.unk8->footprint.x0 + max_x;
            DAT_00813a90 = EditMode.unk8->footprint.y0 + max_y;
        }
    }
    EditCursor.field_1414[0] = DAT_00813a84 - min_x;
    EditCursor.field_1414[1] = DAT_00813a88 - min_y;
    EditCursor.field_1414[2] = DAT_00813a8c - min_x;
    EditCursor.field_1414[3] = DAT_00813a90 - min_y;
    EditCursor.field_1414[4] = 0;
    EditCursor.tile_x = min_x;
    EditCursor.tile_y = min_y;
    EditCursor.field_1830 = 0;
    if (validate) {
        ValidateCursor(&EditCursor, (unsigned int)EditMode.unk8);
    } else {
        ResetCursorBuildable(&EditCursor);
    }
}

// FUNCTION: LEGOLAND 0x00452390
void FUN_00452390(void) {
    struct ObjClass *cls;

    if (EditMode.unk0 == 2 && QueryClass != NULL) {
        cls = QueryClass;
        DAT_00813a34 = QueryObj.id;
        DAT_00813af0 = cls->footprint.v[0];
        DAT_00813af8 = cls->footprint.v[2];
        DAT_00813af4 = cls->footprint.v[1];
        DAT_00813afc = cls->footprint.v[3];
        DAT_00813a38 = (DAT_00813af8 - DAT_00813af0) + 1;
        DAT_00813a3c = (DAT_00813afc - DAT_00813af4) + 1;
        DAT_00813a74 = QueryObj.pos.x + DAT_00813af0;
        DAT_00813a78 = QueryObj.pos.y + DAT_00813af4;
        DAT_00813a7c = MouseTileX;
        DAT_00813a80 = MouseTileY;
    } else {
        DAT_00813a74 = MouseTileX;
        DAT_00813a78 = MouseTileY;
        DAT_00813a7c = MouseTileX;
        DAT_00813a80 = MouseTileY;
    }
    if ((DAT_00813ac4 & 1) != 0) {
        GamePad = GamePad | 0x1000;
        DAT_00813ac4 = DAT_00813ac4 & 0xfffffffe;
    }
}

// FUNCTION: LEGOLAND 0x00452460
LEGO_EXPORT void ReadGameButtons(void) {
    volatile unsigned int saved;
    unsigned int held;
    DWORD now;
    int scroll;
    unsigned int released;
    unsigned int repeat;
    unsigned int pressed;
    struct Point mp;

    DAT_00667c48 = 0;
    held = CONTROLLERBUFFER->buttons;
    pressed = (DAT_00813a94 ^ held) & held;
    released = ~pressed & (DAT_00813a94 ^ held);
    if (released != 0 && DAT_00667108 != 0) {
        released = 0;
        DAT_00667108 = 0;
    }
    if (pressed != 0) {
        ButtonRepeatLastTick = GetTickCount();
        ButtonRepeatDelay = 200;
    }
    now = GetTickCount();
    if (now - ButtonRepeatLastTick > ButtonRepeatDelay) {
        saved = held;
        ButtonRepeatLastTick = GetTickCount();
        ButtonRepeatDelay = 0x32;
        repeat = held;
    } else {
        repeat = 0;
    }
    GamePad = GamePad & 0xffffffe1;
    DAT_00813a94 = held;

    DAT_00813abc = 0;
    if ((pressed & DAT_00813ab8) != 0) DAT_00813abc = 1;
    if ((released & DAT_00813ab8) != 0) DAT_00813abc |= 2;
    if ((held & DAT_00813ab8) != 0) DAT_00813abc |= 4;
    if ((repeat & DAT_00813ab8) != 0) DAT_00813abc |= 8;

    DAT_00813ad4 = 0;
    if ((pressed & DAT_00813ad0) != 0) DAT_00813ad4 = 1;
    if ((released & DAT_00813ad0) != 0) DAT_00813ad4 |= 2;
    if ((held & DAT_00813ad0) != 0) DAT_00813ad4 |= 4;
    if ((repeat & DAT_00813ad0) != 0) DAT_00813ad4 |= 8;

    DAT_00813a9c = 0;
    if ((pressed & DAT_00813a98) != 0) DAT_00813a9c = 1;
    if ((released & DAT_00813a98) != 0) DAT_00813a9c |= 2;
    if ((held & DAT_00813a98) != 0) DAT_00813a9c |= 4;
    if ((repeat & DAT_00813a98) != 0) DAT_00813a9c |= 8;

    DAT_00813aa4 = 0;
    if ((pressed & DAT_00813aa0) != 0) DAT_00813aa4 = 1;
    if ((released & DAT_00813aa0) != 0) DAT_00813aa4 |= 2;
    if ((held & DAT_00813aa0) != 0) DAT_00813aa4 |= 4;
    if ((repeat & DAT_00813aa0) != 0) DAT_00813aa4 |= 8;

    DAT_00813aac = 0;
    if ((pressed & DAT_00813aa8) != 0) DAT_00813aac = 1;
    if ((released & DAT_00813aa8) != 0) DAT_00813aac |= 2;
    if ((held & DAT_00813aa8) != 0) DAT_00813aac |= 4;
    if ((repeat & DAT_00813aa8) != 0) DAT_00813aac |= 8;

    DAT_00813ab4 = 0;
    if ((pressed & DAT_00813ab0) != 0) DAT_00813ab4 = 1;
    if ((released & DAT_00813ab0) != 0) DAT_00813ab4 |= 2;
    if ((held & DAT_00813ab0) != 0) DAT_00813ab4 |= 4;
    if ((repeat & DAT_00813ab0) != 0) DAT_00813ab4 |= 8;

    DAT_00813ac4 = 0;
    if ((pressed & DAT_00813ac0) != 0) DAT_00813ac4 = 1;
    if ((released & DAT_00813ac0) != 0) DAT_00813ac4 |= 2;
    if ((held & DAT_00813ac0) != 0) DAT_00813ac4 |= 4;
    if ((repeat & DAT_00813ac0) != 0) DAT_00813ac4 |= 8;

    DAT_00813acc = 0;
    if ((pressed & DAT_00813ac8) != 0) DAT_00813acc = 1;
    if ((released & DAT_00813ac8) != 0) DAT_00813acc |= 2;
    if ((held & DAT_00813ac8) != 0) DAT_00813acc |= 4;
    if ((repeat & DAT_00813ac8) != 0) DAT_00813acc |= 8;

    DAT_00813adc = 0;
    if ((pressed & DAT_00813ad8) != 0) DAT_00813adc = 1;
    if ((released & DAT_00813ad8) != 0) DAT_00813adc |= 2;
    if ((held & DAT_00813ad8) != 0) DAT_00813adc |= 4;
    if ((repeat & DAT_00813ad8) != 0) DAT_00813adc |= 8;

    if (lpConfig->field_1e != 0) {
        MousePos.x = CONTROLLERBUFFER->x;
        MousePos.y = CONTROLLERBUFFER->y;
        if (MapLoaded != 0) {
            scroll = FUN_00451f70();
            if ((GamePad & 0x1000) != 0 || FocussedIconPtr == 0) {
                if (scroll == 0 && (GamePad & 0x20) != 0) {
                    MouseScrollMap();
                }
                if (CONTROLLERBUFFER->delta_x != 0 ||
                    CONTROLLERBUFFER->delta_y != 0 || (GamePad & 0x10) != 0) {
                    GamePad = GamePad | 8;
                    DAT_00813ae0 = GetTicks();
                }
                ScreenToMapRef((unsigned int)&MousePos, (int *)&mp, 0);
                if (MouseTileX != mp.x || MouseTileY != mp.y) {
                    MouseTileY = mp.y;
                    MouseTileX = mp.x;
                    if ((GamePad & 0x1000) == 0) {
                        GamePad = GamePad | 4;
                    }
                    GamePad = GamePad | 2;
                }
            }
        }

        DAT_00813a50 = 0;
        if ((pressed & DAT_00813a4c) != 0) DAT_00813a50 = 1;
        if ((released & DAT_00813a4c) != 0) DAT_00813a50 |= 2;
        if ((held & DAT_00813a4c) != 0) DAT_00813a50 |= 4;
        if ((repeat & DAT_00813a4c) != 0) DAT_00813a50 |= 8;

        DAT_00813a60 = 0;
        if ((pressed & DAT_00813a5c) != 0) DAT_00813a60 = 1;
        if ((released & DAT_00813a5c) != 0) DAT_00813a60 |= 2;
        if ((held & DAT_00813a5c) != 0) DAT_00813a60 |= 4;
        if ((repeat & DAT_00813a5c) != 0) DAT_00813a60 |= 8;

        if ((GamePad & 4) != 0 && (DAT_00813ac4 & 4) != 0) {
            DAT_00813ac4 |= 0x10;
        }
        if ((GamePad & 0x400) != 0 && (Hover.type & 0x100) != 0) {
            if ((GamePad & 0x1000) != 0) {
                DAT_00813a7c = MouseTileX;
                DAT_00813a80 = MouseTileY;
                if ((DAT_00813ac4 & 2) != 0) {
                    DAT_00667c48 = 1;
                    DAT_00813ac4 = DAT_00813ac4 | 0x11;
                    GamePad = GamePad & 0xffffefff;
                } else {
                    DAT_00813ac4 = DAT_00813ac4 & 0xffffffee;
                }
            } else {
                FUN_00452390();
            }
            FUN_00452030();
        }
    }
    now = GetTicks();
    if (now - DAT_00813ae0 >= 2000) {
        GamePad = GamePad | 0x200;
        return;
    }
    GamePad = GamePad & 0xfffffdff;
}
