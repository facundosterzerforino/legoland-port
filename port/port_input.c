/* [library:input] The mouse wheel in windowed mode, and touch gestures: see port_input.h. */
#include <windows.h>

#include "miniwin/miniwin.h"
#include "port_display.h"
#include "port_input.h"
#include "port_trace.h"

#include "../src/legoland/globals.h"
#include "../src/legoland/icon.h"
#include "../src/legoland/interface.h"
#include "../src/legoland/map_object.h"
#include "../src/legoland/obj_instance.h"

/* the most notches kept waiting, so a fast spin doesn't keep scrolling long after the wheel stops */
#define MAX_PENDING (5 * WHEEL_DELTA)

static int pending;

void PortWheelAdd(int delta) {
    pending += delta;
    if (pending > MAX_PENDING) {
        pending = MAX_PENDING;
    } else if (pending < -MAX_PENDING) {
        pending = -MAX_PENDING;
    }
}

int PortWheelTake(int step) {
    if (pending >= WHEEL_DELTA) {
        pending -= WHEEL_DELTA;
        return step;
    }
    if (pending <= -WHEEL_DELTA) {
        pending += WHEEL_DELTA;
        return -step;
    }
    return 0;
}

/* ---- touch gestures ---- */

/* the scrolling object lists, by their icon group: the build menu's (0xd2) and the ones FUN_0046dac0 picks in the
 * other edit modes; each group's icon +3 is its "scroll up" arrow, whose field_30 is the list's panel (FUN_0046d980) */
static const unsigned short ListGroups[] = {0xd2, 0xc8, 0x1f4, 0x12c, 0x190};

#define LIST_STEP 40.0f /* the game's pixels of a list drag per object scrolled (they're 56 to 66 high) */

static unsigned short drag_arrow; /* the dragged list's arrow icon (looked up again each frame) */
static float drag_rest_x;
static float drag_rest_y;
static float pan_rest_x;
static float pan_rest_y;
static int pan_x;
static int pan_y;

/* the list whose (scrollable: its arrow is shown) panel has the game's point (gx, gy), and its arrow's id */
static struct InterfacePanel *ListAt(int gx, int gy, unsigned short *arrow) {
    int i;

    for (i = 0; i < (int)(sizeof(ListGroups) / sizeof(ListGroups[0])); i++) {
        struct IconNode *icon = FindIcon((unsigned short)(ListGroups[i] + 3));
        struct InterfacePanel *panel;

        if (icon == NULL || (icon->flags & 0x400) != 0 || icon->field_30 == NULL) {
            continue;
        }
        panel = (struct InterfacePanel *)icon->field_30;
        if (gx >= panel->clip_left && gx < panel->clip_right && gy >= panel->clip_top && gy < panel->clip_bottom) {
            *arrow = icon->id;
            return panel;
        }
    }
    return NULL;
}

/* miniwin's drag area: a finger down on a scrollable list (window pixels) */
static int TouchDragArea(int x, int y) {
    POINT pt;

    pt.x = x;
    pt.y = y;
    PortDisplayClientToGame(&pt);
    return ListAt(pt.x, pt.y, &drag_arrow) != NULL;
}

/* an object of the build menu is being placed (EditMode.unk0 1 with an object, only ever in the park; not on the map
 * screen, EditMode.unk4 1), and not a path, a hedge or an object laid by dragging a rectangle (flag 0x2000000), which
 * a finger drags as before */
static const struct Ride *PlacingObject(void) {
    const struct Ride *ride = EditMode.unk8;

    if (EditMode.unk4 == 1 || EditMode.unk0 != 1 || ride == NULL || (const void *)ride == PathControlObject ||
        (const void *)ride == HedgeObjectClass || (ride->flags & 0x2000000) != 0) {
        return NULL;
    }
    return ride;
}

/* miniwin's aim area: the park while placing an object (PlacingObject), not on the game's buttons or panels
 * (GetIconAtPos: a button, or flag 4 for a panel) */
static int TouchAimArea(int x, int y) {
    POINT pt;
    struct Point at;
    unsigned char flags = 0;

    if (PlacingObject() == NULL) {
        return 0;
    }
    pt.x = x;
    pt.y = y;
    PortDisplayClientToGame(&pt);
    if (pt.x < 0 || pt.y < 0 || pt.x >= PORT_GAME_WIDTH || pt.y >= PORT_GAME_HEIGHT) {
        return 0;
    }
    at.x = pt.x;
    at.y = pt.y;
    return GetIconAtPos(&at, &flags) == NULL && (flags & 4) == 0;
}

/* the game's pixels per window pixel (the picture is scaled to the window) */
static float GameScale(void) {
    POINT a;
    POINT b;

    a.x = 0;
    a.y = 0;
    b.x = 1000;
    b.y = 1000;
    PortDisplayClientToGame(&a);
    PortDisplayClientToGame(&b);
    return b.y > a.y ? (float)(b.y - a.y) / 1000.0f : 1.0f;
}

/* whole game pixels out of a movement, the rest kept for the next frame */
static int TakeWhole(float *rest, float add) {
    int whole;

    *rest += add;
    whole = (int)*rest;
    *rest -= (float)whole;
    return whole;
}

void PortTouchFrame(void) {
    float dx;
    float dy;
    float px;
    float py;
    float scale;

    MiniwinSetTouchDragArea(TouchDragArea);
    MiniwinSetTouchAimArea(TouchAimArea);
    if (MiniwinTouchMode()) {
        /* [port] the log ("Save the game's log"): when placing with a finger starts and ends */
        static const struct Ride *placing;
        static int aiming;
        const struct Ride *now = PlacingObject();

        if (now != placing) {
            PortTrace("touch: placing %s (screen %d)", now != NULL ? now->name : "nothing", (int)EditMode.unk4);
            placing = now;
        }
        if (MiniwinTouchAiming() != aiming) {
            aiming = MiniwinTouchAiming();
            PortTrace("touch: %s", aiming ? "aiming" : "aim let go");
        }
    }
    MiniwinTouchTakeDrag(&dx, &dy, &px, &py);
    scale = GameScale();
    if (MiniwinTouchFingers() == 0) {
        drag_rest_x = 0.0f; /* a new drag starts from nothing */
        drag_rest_y = 0.0f;
    }
    drag_rest_x += dx * scale;
    drag_rest_y += dy * scale;
    {
        struct IconNode *arrow = FindIcon(drag_arrow);
        struct InterfacePanel *panel = arrow != NULL ? (struct InterfacePanel *)arrow->field_30 : NULL;

        /* the list follows the finger an object at a time: the game's scrolling snaps to its objects (one per click
         * of its arrows: ScrollIconRegion by 6, or 0x20 across), so one such step per LIST_STEP pixels of the drag */
        if (panel == NULL) {
            drag_rest_x = 0.0f;
            drag_rest_y = 0.0f;
        } else if ((panel->field_4 & 1) != 0) {
            drag_rest_x = 0.0f;
            while (drag_rest_y >= LIST_STEP) {
                ScrollIconRegion(panel, 0, 6);
                drag_rest_y -= LIST_STEP;
            }
            while (drag_rest_y <= -LIST_STEP) {
                ScrollIconRegion(panel, 0, -6);
                drag_rest_y += LIST_STEP;
            }
        } else {
            drag_rest_y = 0.0f;
            while (drag_rest_x >= LIST_STEP) {
                ScrollIconRegion(panel, 0x20, 0);
                drag_rest_x -= LIST_STEP;
            }
            while (drag_rest_x <= -LIST_STEP) {
                ScrollIconRegion(panel, -0x20, 0);
                drag_rest_x += LIST_STEP;
            }
        }
    }
    /* the park follows the fingers: the view moves the other way (ScrollX/Y are the view's place, 1/256 pixels) */
    pan_x = TakeWhole(&pan_rest_x, px * scale);
    pan_y = TakeWhole(&pan_rest_y, py * scale);
}

void PortTouchPan(void) {
    if (pan_x != 0 || pan_y != 0) {
        ProcessScrolling((unsigned int)(-pan_x * 0x100), (unsigned int)(-pan_y * 0x100));
        pan_x = 0;
        pan_y = 0;
    }
}

int PortTouchEdgeScroll(void) {
    return !MiniwinTouchMode() || MiniwinTouchFingers() == 1;
}
