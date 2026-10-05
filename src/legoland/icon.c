#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "globals.h"
#include "legoland.h"
#include "math.h"

#include "clipping.h"
#include "controller.h"
#include "gfx.h"
#include "icon.h"
#include "interface.h"
#include "llidb.h"
#include "map_object.h"
#include "print_sprite.h"
#include "render.h"
#include "string.h"
#include "timer.h"

struct Sprite;

struct IconAnim {
    /* 0x00 */ struct LLS *lls;
    /* 0x04 */ unsigned char pad_4[0x14 - 0x4];
    /* 0x14 */ int kind;
};

struct IconSprite {
    /* 0x00 */ unsigned char pad_0[8];
    /* 0x08 */ struct IconAnim *anim;
};

struct Rect16 {
    unsigned char pad_0[0xc];
    short x;
    short y;
    short width;
    short height;
};

struct Rect32 {
    int left;
    int top;
    int right;
    int bottom;
};

struct Config {
    unsigned short field_0;
    unsigned short field_2;
};

struct Bbox {
    int min_x;
    int min_y;
    int max_x;
    int max_y;
};

struct Indicator {
    struct Indicator *next;
    unsigned int field_4;
    unsigned int field_8;
    unsigned int field_c;
    unsigned int field_10;
    struct IconNode *field_14;
    unsigned int field_18;
    void (*field_1c)(struct Indicator *);
    unsigned char pad_20[0x22 - 0x20];
    unsigned char field_22;
};

struct MenuGroup {
    /* 0x00 */ short field_0;
    /* 0x02 */ unsigned char pad_2[0x4 - 0x2];
    /* 0x04 */ unsigned int field_4;
    /* 0x08 */ struct IconNode *icon;
    /* 0x0c */ int field_c;
    /* 0x10 */ int field_10;
    /* 0x14 */ int field_14;
    /* 0x18 */ int field_18;
    /* 0x1c */ int field_1c;
    /* 0x20 */ int field_20;
    /* 0x24 */ int field_24;
    /* 0x28 */ int field_28;
};

struct ListElement {
    /* 0x00 */ unsigned char pad_0[8];
    /* 0x08 */ unsigned char field_8;
    /* 0x09 */ unsigned char pad_9[0xc - 0x9];
    /* 0x0c */ int *field_c;
};

struct InfoSource {
    /* 0x00 */ unsigned char pad_0[4];
    /* 0x04 */ void *field_4;
    /* 0x08 */ void *field_8;
    /* 0x0c */ unsigned char pad_c[0x10 - 0xc];
    /* 0x10 */ unsigned int field_10;
    /* 0x14 */ struct Sprite *field_14;
    /* 0x18 */ unsigned char pad_18[0x68 - 0x18];
    /* 0x68 */ struct Sprite *field_68;
    /* 0x6c */ unsigned char pad_6c[0x78 - 0x6c];
    /* 0x78 */ void *field_78;
    /* 0x7c */ void *field_7c;
    /* 0x80 */ unsigned char pad_80[0xc4 - 0x80];
    /* 0xc4 */ unsigned int *field_c4;
};

struct ScrollRegion {
    /* 0x00 */ short field_0;
    /* 0x02 */ unsigned char pad_2[0x4 - 0x2];
    /* 0x04 */ unsigned char field_4;
    /* 0x05 */ unsigned char pad_5[0xc - 0x5];
    /* 0x0c */ int content_left;
    /* 0x10 */ int content_top;
    /* 0x14 */ int content_right;
    /* 0x18 */ int content_bottom;
    /* 0x1c */ RECT clip;
};

struct IndicatorFuncs {
    unsigned char pad_0[4];
    unsigned int field_4;
    unsigned char pad_8[0x18 - 0x8];
    unsigned int check_func;
    unsigned int click_func;
};

struct TimedIndicator {
    struct Indicator *next;
    unsigned int field_4;
    unsigned int start_time;
    unsigned int duration;
    unsigned int field_10;
    struct IconNode *icon;
    unsigned int check_func;
    unsigned int click_func;
};

#include "bricks.h"
#include "freeplay.h"
#include "image_sprite.h"
#include "objclass.h"
#include "popupinfo.h"
#include "render3d.h"
#include "screens.h"
#include "sound_music.h"
#include "text.h"

// FUNCTION: LEGOLAND 0x0046d3a0
void FUN_0046d3a0(void) { DAT_006687a4 = 4; }

// FUNCTION: LEGOLAND 0x0046d3c0
void FUN_0046d3c0(struct IconNode *node) {
    if (!node) {
        return;
    }
    if (node->sprite) {
        KillSprite(node->sprite);
        node->sprite = NULL;
    }
    if (node->flags & 0x80) {
        if (node->field_30) {
            free(node->field_30);
        }
    }
    free(node);
    if (FocussedIconPtr == node) {
        FocussedIconPtr = NULL;
    }
    if (Hover.type == 2 && Hover.ptr == (unsigned int)node) {
        Hover.ptr = 0;
        Hover.type = 0x100;
    }
}

// FUNCTION: LEGOLAND 0x0046d440
void FUN_0046d440(struct IconNode *node) {
    ((struct IconNode *)PTR_some_list_head_004ba87c)->next = node;
    PTR_some_list_head_004ba87c = node;
}

// FUNCTION: LEGOLAND 0x0046d460
void FUN_0046d460(struct IconNode **slot) {
    struct IconNode *node = *slot;
    struct IconNode *next;
    if (FocussedIconPtr == node->next) {
        FocussedIconPtr = NULL;
    }
    if (PTR_some_list_head_004ba87c == *slot) {
        PTR_some_list_head_004ba87c = (struct IconNode *)slot;
    }
    node = *slot;
    next = node->next;
    FUN_0046d3c0(node);
    *slot = next;
}

// FUNCTION: LEGOLAND 0x0046d4a0
void FUN_0046d4a0(struct IconNode **slot) {
    struct IconNode *node = *slot;
    struct IconNode *next;
    if (FocussedIconPtr == node->next) {
        FocussedIconPtr = NULL;
    }
    if (PTR_DAT_004ba880 == *slot) {
        PTR_DAT_004ba880 = slot;
    }
    node = *slot;
    next = node->next;
    FUN_0046d3c0(node);
    *slot = next;
}

// FUNCTION: LEGOLAND 0x0046d4e0
void FUN_0046d4e0(struct IconNode *node) {
    struct IconNode *cur;
    struct IconNode *next;
    if (IconListHead == node) {
        FUN_0046d460(&IconListHead);
    } else {
        cur = IconListHead;
        while (cur != NULL && (next = cur->next) != node) {
            cur = next;
        }
        if (cur != NULL) {
            FUN_0046d460((struct IconNode **)cur);
        }
    }
}

// FUNCTION: LEGOLAND 0x0046d520
LEGO_EXPORT void RemoveIconGroup(unsigned short group) {
    struct IconNode **slot;
    struct IconNode *node;
    int removed = 0;

    slot = &IconListHead;
    node = IconListHead;
    while (node != NULL) {
        if (node->id == group) {
            node = node->next;
            FUN_0046d460(slot);
            removed++;
        } else {
            slot = &node->next;
            node = node->next;
        }
    }
    slot = &DAT_006687cc;
    node = DAT_006687cc;
    while (node != NULL) {
        if (node->id == group) {
            node = node->next;
            FUN_0046d4a0(slot);
            removed++;
        } else {
            slot = &node->next;
            node = node->next;
        }
    }
}

// FUNCTION: LEGOLAND 0x0046d590
void FUN_0046d590(unsigned short val) {
    struct IconNode **slot;
    struct IconNode *node;
    int removed = 0;

    slot = &IconListHead;
    node = IconListHead;
    while (node != NULL) {
        if (node->id < val || (int)node->id >= val + 7) {
            slot = &node->next;
            node = node->next;
        } else {
            node = node->next;
            FUN_0046d460(slot);
            removed++;
        }
    }
    slot = &DAT_006687cc;
    node = DAT_006687cc;
    while (node != NULL) {
        if (node->id < val || (int)node->id >= val + 7) {
            slot = &node->next;
            node = node->next;
        } else {
            node = node->next;
            FUN_0046d4a0(slot);
            removed++;
        }
    }
}

// FUNCTION: LEGOLAND 0x0046d630
LEGO_EXPORT struct IconNode *FindIcon(unsigned short id) {
    struct IconNode *node = (struct IconNode *)IconListHead;
    while (node) {
        if (node->id == id && (node->flags & 0x100) == 0) {
            return node;
        }
        node = node->next;
    }
    node = (struct IconNode *)DAT_006687cc;
    while (node) {
        if (node->id == id && (node->flags & 0x100) == 0) {
            return node;
        }
        node = node->next;
    }
    return NULL;
}

// FUNCTION: LEGOLAND 0x0046d680
void SetIconSprite(struct IconNode *node, struct Sprite *sprite) {
    if (node) {
        struct Sprite *old = node->sprite;
        if (sprite != old) {
            if (old) {
                KillSprite(old);
            }
            if (sprite) {
                ReferenceSprite(sprite);
            }
            node->sprite = sprite;
        }
    }
}

// FUNCTION: LEGOLAND 0x0046d6c0
LEGO_EXPORT struct IconNode *InsertIcon(short a1, short a2, int a3, struct Sprite *sprite) {
    struct IconNode *icon = (struct IconNode *)malloc(sizeof(struct IconNode));
    if (icon != NULL) {
        *icon = DAT_00668858;
        icon->x = a1;
        icon->y = a2;
        icon->id = (unsigned short)a3;
        ReferenceSprite(sprite);
        icon->sprite = sprite;
        if (sprite != NULL) {
            short h = *(volatile short *)&sprite->height;
            short w = *(volatile short *)&sprite->width;
            icon->height = h;
            icon->width = w;
            icon->flags = icon->flags | 0x10;
        }
    }
    FUN_0046d440(icon);
    return icon;
}

// FUNCTION: LEGOLAND 0x0046d740
LEGO_EXPORT void SetNewGroup_Callbacks(void *param_1, void *param_2, void *param_3) {
    DAT_006688a8 = param_1;
    DAT_006688ac = param_2;
    DAT_006688b0 = param_3;
}

// FUNCTION: LEGOLAND 0x0046d760
LEGO_EXPORT struct IconNode *AddFullScreenIcon(void *icon) {
    struct IconNode *result = InsertIcon(0, 0, (int)icon, 0);
    if (result) {
        result->width = ((struct Config *)lpConfig)->field_0;
        result->height = ((struct Config *)lpConfig)->field_2;
        result->render_func = (void *)SetClippingToScreenRect;
        result->flags = result->flags | 0x29;
    }
    return result;
}

// FUNCTION: LEGOLAND 0x0046d7b0
LEGO_EXPORT struct IconNode *LoadSpriteIcon(const char *filename, unsigned int param_2, short param_3, short param_4, int param_5) {
    struct IconNode *result = NULL;
    struct Sprite *sprite = LoadSprite(filename, param_2);
    if (sprite != NULL) {
        result = InsertIcon(param_3, param_4, param_5, sprite);
    }
    KillSprite(sprite);
    return result;
}

// FUNCTION: LEGOLAND 0x0046d850
void ScrollIconRegion(struct ScrollRegion *r, int param_2, int param_3) {
    int dy;
    int dx;
    int l;
    int t;
    int nl;
    int nt;
    int edge;

    if ((r->field_4 & 1) != 0) {
        if (r->content_bottom - r->content_top <= r->clip.bottom - r->clip.top) {
            MoveIcons(0xffff, r->field_0, 0, (short)(r->clip.top - r->content_top));
            return;
        }
    } else {
        if (r->content_right - r->content_left <= r->clip.right - r->clip.left) {
            MoveIcons(0xffff, r->field_0, (short)(r->clip.left - r->content_left), 0);
            return;
        }
    }

    dy = FUN_0046dd10(0xffff, (short)((r->clip.top - r->content_top) - param_3), (short)(r->content_top + param_3), r->field_0, param_3);
    dx = param_2;
    l = r->content_left;
    t = r->content_top;
    nl = l + dx;
    nt = t + dy;
    if ((r->field_4 & 1) != 0) {
        if (nt > r->clip.top) {
            dy = dy + (r->clip.top - nt);
        } else {
            edge = r->content_bottom + dy;
            if (edge < r->clip.bottom) {
                dy = dy + (r->clip.bottom - edge);
            }
        }
    } else {
        if (nl > r->clip.left) {
            dx = dx + (r->clip.left - nl);
        } else {
            edge = r->content_right + dx;
            if (edge < r->clip.right) {
                dx = dx + (r->clip.right - edge);
            }
        }
    }
    r->content_left = l + dx;
    r->content_top = t + dy;
    r->content_right = r->content_right + dx;
    r->content_bottom = r->content_bottom + dy;
    DAT_00668e44[DAT_00668e64] = DAT_00668e44[DAT_00668e64] + dy;
    MoveIcons(0xffff, r->field_0, (short)dx, (short)dy);
}

// FUNCTION: LEGOLAND 0x0046d980
unsigned char FUN_0046d980(struct IconNode *icon, unsigned char buttons, int a3, int a4) {
    struct ScrollRegion *region;
    if ((buttons & 1) != 0) {
        do {
            region = (struct ScrollRegion *)icon->field_30;
            LastScrollIconTick = GetTickCount();
            if ((region->field_4 & 1) != 0) {
                ScrollIconRegion(region, 0, 6);
            } else {
                ScrollIconRegion(region, 0x20, 0);
            }
        } while (0);
        return 2;
    }
    if ((buttons & 4) != 0) {
        do {
            if (GetTickCount() - LastScrollIconTick >= 0xfa) {
                region = (struct ScrollRegion *)icon->field_30;
                LastScrollIconTick = GetTickCount();
                if ((region->field_4 & 1) != 0) {
                    ScrollIconRegion(region, 0, 6);
                } else {
                    ScrollIconRegion(region, 0x20, 0);
                }
            }
        } while (0);
        return 2;
    }
    return 1;
}

// FUNCTION: LEGOLAND 0x0046da20
unsigned char FUN_0046da20(struct IconNode *icon, unsigned char buttons, int a3, int a4) {
    struct ScrollRegion *region;
    if ((buttons & 1) != 0) {
        region = (struct ScrollRegion *)icon->field_30;
        LastScrollIconTick = GetTickCount();
        if ((region->field_4 & 1) != 0) {
            ScrollIconRegion(region, 0, -6);
            return 2;
        }
        ScrollIconRegion(region, -0x20, 0);
        return 2;
    }
    if ((buttons & 4) != 0) {
        if (GetTickCount() - LastScrollIconTick >= 0xfa) {
            region = (struct ScrollRegion *)icon->field_30;
            LastScrollIconTick = GetTickCount();
            if ((region->field_4 & 1) != 0) {
                ScrollIconRegion(region, 0, -6);
            } else {
                ScrollIconRegion(region, -0x20, 0);
            }
        }
        return 2;
    }
    return 1;
}

// FUNCTION: LEGOLAND 0x0046dac0
void FUN_0046dac0(void) {
    unsigned int v;
    struct IconNode *icon;
    if (EditMode.unk4 == 2 && DAT_0080ff80.unk8 == 3) {
        int field = CONTROLLERBUFFER->x;
        if (field < 0xb2) {
            v = 0xc8;
        } else if (field < 0x143) {
            v = 0x1f4;
        } else {
            unsigned int ecx = (field >= 0x1d4) ? 1 : 0;
            ecx = ecx - 1;
            ecx = ecx & 0x64;
            v = ecx + 0x12c;
        }
    } else {
        v = 0xd2;
    }
    v = v + 3;
    icon = FindIcon((unsigned short)v);
    if (icon) {
        FUN_0046d980(icon, 1, 0, 0);
    }
}

// FUNCTION: LEGOLAND 0x0046db40
void FUN_0046db40(void) {
    unsigned int v;
    struct IconNode *icon;
    if (EditMode.unk4 == 2 && DAT_0080ff80.unk8 == 3) {
        int field = CONTROLLERBUFFER->x;
        if (field < 0xb2) {
            v = 0xc8;
        } else if (field < 0x143) {
            v = 0x1f4;
        } else {
            unsigned int ecx = (field >= 0x1d4) ? 1 : 0;
            ecx = ecx - 1;
            ecx = ecx & 0x64;
            v = ecx + 0x12c;
        }
    } else {
        v = 0xd2;
    }
    v = v + 4;
    icon = FindIcon((unsigned short)v);
    if (icon) {
        FUN_0046da20(icon, 1, 0, 0);
    }
}

// FUNCTION: LEGOLAND 0x0046dbc0
LEGO_EXPORT struct IconNode *AddGBarIcons(unsigned int param_1, unsigned int param_2, int param_3, int param_4, int param_5, int param_6) {
    struct IconNode *icon;

    AddFullScreenIcon((void *)(param_6 + 6));
    icon = InsertIcon(param_2, param_3, param_6 + 3, IfSideBUpSprite);
    icon->field_30 = (void *)param_1;
    icon->string_id = 0x95;
    icon->string = GetString(0x95);
    icon->height = 0x1e;
    icon->flags = icon->flags | 0x2013;
    icon->event_handler = (void *)FUN_0046d980;
    icon = InsertIcon(param_2, param_5 - 0x1e + param_3, param_6 + 4, IfSideBDownSprite);
    icon->field_30 = (void *)param_1;
    icon->string_id = 0x94;
    icon->string = GetString(0x94);
    icon->height = 0x1e;
    icon->flags = icon->flags | 0x2013;
    icon->event_handler = (void *)FUN_0046da20;
    icon = InsertIcon(param_2, param_3 + 0x1e, param_6 + 5, 0);
    icon->field_30 = (void *)param_1;
    icon->width = 0x79;
    icon->height = (short)param_5 - 0x3c;
    icon->box.box_color = GetNearestColour(0, 10, 0x14);
    icon->flags = icon->flags | 0x29;
    icon->render_func = (void *)RenderBoxIcon;
    return icon;
}

// FUNCTION: LEGOLAND 0x0046dcd0
LEGO_EXPORT void MoveIcons(unsigned short mask, unsigned short id, short dx, short dy) {
    struct IconNode *node;
    for (node = IconListHead; node != NULL; node = node->next) {
        if ((node->flags & 1) == 0 && (node->id & mask) == id) {
            node->x = node->x + dx;
            node->y = node->y + dy;
        }
    }
}

// FUNCTION: LEGOLAND 0x0046dd10
int FUN_0046dd10(unsigned short param_1, short param_2, short param_3, unsigned short param_4, int param_5) {
    struct IconNode *best = NULL;
    struct IconNode *node = IconListHead;
    int extreme;
    int v;

    if (param_5 > 0) {
        extreme = -100000;
        if (node == NULL) {
            return 0;
        }
        do {
            if ((node->flags & 1) == 0 && (node->id & param_1) == param_4 &&
                (v = (int)node->y, (v - param_3) + param_5 <= (int)param_2 && v > extreme)) {
                best = node;
                extreme = v;
            }
            node = node->next;
        } while (node != NULL);
    } else {
        if (param_5 >= 0) {
            return 0;
        }
        extreme = 100000;
        if (node == NULL) {
            return 0;
        }
        do {
            if ((node->flags & 1) == 0 && (node->id & param_1) == param_4 &&
                (v = (int)node->y, (v - param_3) + param_5 > (int)param_2) && v < extreme) {
                best = node;
                extreme = v;
            }
            node = node->next;
        } while (node != NULL);
    }
    if (best != NULL && best->y != param_2) {
        return ((int)param_2 - (int)best->y) + (int)param_3;
    }
    return 0;
}

// FUNCTION: LEGOLAND 0x0046de50
void FUN_0046de50(struct Rect16 *src, struct Rect32 *dst) {
    dst->left = src->x;
    dst->top = src->y;
    dst->right = src->width + src->x;
    dst->bottom = src->height + src->y;
}

// FUNCTION: LEGOLAND 0x0046de90
void FUN_0046de90(struct IconNode *icon, struct Bbox *bbox) {
    FUN_0046de50((struct Rect16 *)icon, (struct Rect32 *)bbox);
    if (icon->flags & 0x20) {
        bbox->min_x = bbox->min_x - 3;
        bbox->max_x = bbox->max_x + 3;
        bbox->min_y = bbox->min_y - 3;
        bbox->max_y = bbox->max_y + 3;
    }
    if (icon->flags & 0x40) {
        struct Indicator *ind;
        bbox->min_y = bbox->min_y - 0x16;
        ind = (struct Indicator *)icon->field_30;
        if (ind != NULL) {
            if (ind->field_22 & 8) {
                bbox->max_y = bbox->max_y + DAT_00668840;
            }
            if (ind->field_22 & 2) {
                bbox->max_x = bbox->max_x + DAT_0066884c;
            }
        }
    }
    if (icon->flags & 0x200) {
        int v = icon->field_22 + icon->y;
        int top = v + 0x28;
        if (bbox->min_y > v) {
            bbox->min_y = v;
        }
        if (bbox->max_y < top) {
            bbox->max_y = top;
        }
    }
}

// FUNCTION: LEGOLAND 0x0046df30
int FUN_0046df30(struct Rect16 *src) {
    RECT rect;
    FUN_0046de50(src, (struct Rect32 *)&rect);
    SetClipping(&rect);
    return 0;
}

// FUNCTION: LEGOLAND 0x0046df60
int SetClippingToScreenRect(int param) {
    SetClipping(&ScreenRect);
    return 0;
}

// FUNCTION: LEGOLAND 0x0046df70
LEGO_EXPORT int RenderBoxIcon(struct IconNode *node) {
    unsigned short border = node->box.border;
    RenderThickBox(node->x - border, node->y - border,
        node->width + border * 2, node->height + border * 2,
        border, node->box.box_color);
    if ((node->flags & 0x20) != 0) {
        FUN_0046df30((struct Rect16 *)node);
    }
    return 0;
}

// FUNCTION: LEGOLAND 0x0046dfd0
int FUN_0046dfd0(struct IconNode *node) {
    struct PrintCtx ctx;

    ctx.flags = 2;
    ctx.node = node;
    ctx.field_8 = 0;
    if (node->sprite != NULL) {
        PrintSprite(node->sprite, node->x, node->y, 0, (int *)&ctx);
    }
    PrintCent(node->field_20 + node->x, node->field_22 + node->y, 0x40, node->field_18p, 1);
    return 0;
}

// FUNCTION: LEGOLAND 0x0046e040
int FUN_0046e040(struct IconNode *node) {
    struct PrintCtx ctx;

    ctx.flags = 2;
    ctx.node = node;
    ctx.field_8 = 0;
    if (node == (struct IconNode *)FocussedIconPtr) {
        PrintSprite(node->sprite, node->x, node->y, 0, (int *)&ctx);
        return 0;
    }
    PrintSprite(node->alt_sprite, node->x, node->y, 0, (int *)&ctx);
    return 0;
}

// FUNCTION: LEGOLAND 0x0046e0a0
LEGO_EXPORT int RenderBuildObjectIcon(struct IconNode *node) {
    struct PrintCtx ctx = {2, node};
    int cost;
    unsigned int *objsub;

    if (EditMode.unk0 == 1 && EditMode.unk8 != 0) {
        objsub = *(unsigned int **)((char *)node->field_8 + 0xc4);
        if (*(unsigned int **)((char *)EditMode.unk8 + 0xc4) == objsub) {
            objsub[2] = objsub[2] & 0xfffdffff;
        }
    }
    if (node->y > 0 && node->y < 0x174) {
        char buf[12];
        do {
            cost = GetObjCost(node->field_8);
            if (node->sprite != NULL) {
                if (cost <= GetBrickCount()) {
                    PrintSprite(node->sprite, node->x, node->y, 0, (int *)&ctx);
                } else {
                    PrintSprite(node->sprite, node->x, node->y, DAT_004ba884, (int *)&ctx);
                }
            }
            if (node->field_8 != NULL) {
                // STRING: LEGOLAND 0x004b8a80
                sprintf(buf, "%d", cost);
                FUN_00455e50(buf, node->x + 0x3c, node->y + 0x14, 0x41, 0x14, 1, 1, 0, 0xffffff);
            }
            switch (node->field_20b) {
            case 1:
                PrintSprite(LinkMiddleSprite, node->x, node->y, 0, 0);
                break;
            case 2:
                PrintSprite(LinkBottomSprite, node->x, node->y, 0, 0);
                break;
            }
            if (EditMode.unk0 == 1 && EditMode.unk8 != 0) {
                // STRING: LEGOLAND 0x004ba888
                if (strcmp("Path", *(char **)((char *)EditMode.unk8 + 0x78)) != 0 &&
                    *(unsigned int **)((char *)EditMode.unk8 + 0xc4) == *(unsigned int **)((char *)node->field_8 + 0xc4)) {
                    struct Sprite *s = AttractHighlightOnSprite;
                    if (GetBlink() != 0) {
                        s = AttractHighlightOffSprite;
                    }
                    PrintSprite(s, node->x, node->y, 0, 0);
                }
            }
            if ((*(unsigned int **)((char *)node->field_8 + 0xc4))[2] & 0x20000) {
                PrintSprite(GetBlink() != 0 ? AttractNewOffSprite : AttractNewOnSprite, node->x, node->y, 0, 0);
            }
        } while (0);
    }
    if ((*(unsigned int **)((char *)node->field_8 + 0xc4))[2] & 0x20000) {
        if (node->y < 0x3e) {
            DAT_006688b8 = DAT_006688b8 | 1;
            return 0;
        }
        if (0x156 < node->height + node->y) {
            DAT_006688b8 = DAT_006688b8 | 2;
        }
    }
    return 0;
}

// FUNCTION: LEGOLAND 0x0046e300
LEGO_EXPORT int RenderFreePlayIcons(struct IconNode *node) {
    struct PrintCtx ctx;
    short y;
    struct Sprite *sprite;

    ctx.flags = 2;
    ctx.node = node;
    y = node->y;
    ctx.field_8 = 0;
    if (y >= 0 && y <= 0x12c) {
        if ((char)node->field_18 != 1) {
            sprite = node->sprite;
            if (sprite != NULL) {
                FUN_0048a840((unsigned int)node->field_1c, 0);
                if (FUN_0048aef0(node->field_1c, node->field_20p) != 0) {
                    PrintSprite(node->sprite, node->x, node->y, 0, (int *)&ctx);
                    return 0;
                }
                PrintSprite(node->sprite, node->x, node->y, 0xff000000, (int *)&ctx);
                return 0;
            }
        } else {
            sprite = node->sprite;
            if (sprite != NULL) {
                PrintSprite(sprite, node->x, y, 0, (int *)&ctx);
            }
            PrintSprite(FreePlayTickSprite, node->x + 0x46, node->y, 0, (int *)&ctx);
        }
    }
    return 0;
}

// FUNCTION: LEGOLAND 0x0046e400
LEGO_EXPORT int RenderScroll_Icons(struct IconNode *node) {
    struct PrintCtx ctx;
    ctx.flags = 2;
    ctx.node = node;
    ctx.field_8 = 0;
    if (node->sprite != NULL) {
        PrintSprite(node->sprite, node->x, node->y, 0, (int *)&ctx);
    }
    if (GetBlink() != 0) {
        if ((DAT_006688b8 & 2) != 0 && (char)node->field_18 == 2) {
            PrintSprite(SideScrollDownLitSprite, node->x, node->y, 0, (int *)&ctx);
            DAT_006688b8 = DAT_006688b8 & 0xfffffffd;
            return 0;
        }
        if ((DAT_006688b8 & 1) != 0 && (char)node->field_18 == 1) {
            PrintSprite(SideScrollUpLitSprite, node->x, node->y, 0, (int *)&ctx);
            DAT_006688b8 = DAT_006688b8 & 0xfffffffe;
        }
    }
    return 0;
}

// FUNCTION: LEGOLAND 0x0046e4d0
LEGO_EXPORT int RenderEnergyBar(struct IconNode *node) {
    struct PrintCtx ctx = {1};
    int fill1;
    int fill2;
    int scale;
    RECT clip;

    if (MapStats.power_supply == 0 && MapStats.power_demand == 0) {
        fill1 = 0;
        fill2 = 0;
    } else {
        scale = MapStats.power_demand * 2;
        if (MapStats.power_supply > scale) {
            scale = MapStats.power_supply;
        }
        if (scale == 0) {
            scale = 1;
        }
        fill1 = (node->width * MapStats.power_supply) / scale;
        fill2 = (node->width * MapStats.power_demand) / scale;
    }
    if (node->sprite != NULL && MapStats.field_18c != 0) {
        StoreClipping();
        if (fill1 > EnergyBarSupplyWidth) {
            EnergyBarSupplyWidth += 6;
            if (EnergyBarSupplyWidth > fill1) {
                EnergyBarSupplyWidth = fill1;
            }
        } else {
            EnergyBarSupplyWidth -= 6;
            if (EnergyBarSupplyWidth < fill1) {
                EnergyBarSupplyWidth = fill1;
            }
        }
        if (fill2 > PowerDemandBarPos) {
            PowerDemandBarPos += 6;
            if (PowerDemandBarPos > fill2) {
                PowerDemandBarPos = fill2;
            }
        } else {
            PowerDemandBarPos -= 6;
            if (PowerDemandBarPos < fill2) {
                PowerDemandBarPos = fill2;
            }
        }
        clip.left = node->x;
        clip.top = node->y;
        clip.right = EnergyBarSupplyWidth + clip.left;
        clip.bottom = node->height + clip.top;
        SetClipping(&clip);
        PrintSprite(node->sprite, node->x, node->y, 0, (int *)&ctx);
        RestoreClipping();
        PrintSprite(BarPointerSprite, (node->x - BarPointerSprite->width / 2) + PowerDemandBarPos, node->height / 2 + node->y, 0, (int *)&ctx);
        return 0;
    }
    if (MapStats.field_18c == 0) {
        PrintSprite(NoEnergySprite, node->x - 0x15, node->y - 6, 0, (int *)&ctx);
    }
    return 0;
}

// FUNCTION: LEGOLAND 0x0046e670
LEGO_EXPORT int RenderMoneyBar(struct IconNode *node) {
    struct PrintCtx ctx = {1};
    int fill;
    int width;
    int bricks;
    RECT clip;
    char buf[100];

    if (AreBricksLimited() == 0) {
        return 0;
    }
    width = node->width;
    bricks = GetBrickCount();
    fill = (bricks * width) / (int)MapStats.brick_meter_max;
    if (fill < 0) {
        fill = 0;
    }
    if (fill > width) {
        fill = width;
    }
    if (fill > DAT_006688c4) {
        DAT_006688c4 += 6;
        if (DAT_006688c4 > fill) {
            DAT_006688c4 = fill;
        }
    } else {
        DAT_006688c4 -= 6;
        if (DAT_006688c4 < fill) {
            DAT_006688c4 = fill;
        }
    }
    StoreClipping();
    clip.left = node->x;
    clip.right = DAT_006688c4 + clip.left;
    clip.top = node->y;
    clip.bottom = node->height + clip.top;
    SetClipping(&clip);
    if (node->sprite != NULL) {
        PrintSprite(node->sprite, node->x, node->y, 0, (int *)&ctx);
    }
    RestoreClipping();
    bricks = GetBrickCount();
    if (bricks < 0) {
        bricks = 0;
    }
    // STRING: LEGOLAND 0x004ba890
    sprintf(buf, "%5d", bricks);
    FUN_00455e50(buf, node->x + 10 + node->width, node->y - 3, 200, node->height, 0, 0, 0xff0000, 0xffffff);
    return 0;
}

// FUNCTION: LEGOLAND 0x0046e7b0
LEGO_EXPORT int RenderFreePlayBar(struct IconNode *node) {
    struct PrintCtx ctx;
    RECT clip;

    ctx.flags = 2;
    ctx.node = node;
    ctx.field_8 = 0;
    StoreClipping();
    clip.left = node->x;
    clip.right = (int)(node->width * 0.00005f * (int)DAT_007cb3a0);
    clip.right += clip.left;
    clip.top = node->y;
    clip.bottom = node->height + clip.top;
    SetClipping(&clip);
    if (node->sprite != NULL) {
        PrintSprite(node->sprite, node->x, node->y, 0, (int *)&ctx);
    }
    RestoreClipping();
    return 0;
}

// FUNCTION: LEGOLAND 0x0046e850
LEGO_EXPORT int RenderGBarSpriteIcon(struct IconNode *node) {
    struct PrintCtx ctx;
    ctx.flags = 2;
    ctx.node = node;
    ctx.field_8 = 0;
    if (node->sprite != NULL) {
        PrintSprite(node->sprite, node->x, node->y, 0, (int *)&ctx);
    }
    return 0;
}

// FUNCTION: LEGOLAND 0x0046e8a0
LEGO_EXPORT int RenderFlashingSpriteIcon(struct IconNode *node) {
    struct PrintCtx ctx;
    ctx.flags = 2;
    ctx.node = node;
    ctx.field_8 = 0;
    if (node->sprite != NULL) {
        if (GetBlink() != 0) {
            PrintSprite(node->sprite, node->x, node->y, 0, (int *)&ctx);
            return 0;
        }
        PrintSprite(node->sprite, node->x, node->y, 0xff000000, (int *)&ctx);
    }
    return 0;
}

// FUNCTION: LEGOLAND 0x0046e920
int FUN_0046e920(struct IconNode *node) {
    struct PrintCtx ctx;
    struct LLS *lls;
    int frame;

    ctx.flags = 2;
    ctx.node = node;
    ctx.field_8 = 0;
    if (node->sprite != NULL) {
        if (GetBlink() != 0) {
            PrintSprite(node->sprite, node->x, node->y, 0, (int *)&ctx);
        } else {
            struct Sprite *alt = node->alt_sprite;
            if (alt != NULL) {
                lls = (struct LLS *)GetLLSForSprite((struct SpriteLLS *)node->sprite);
                if (lls != NULL) {
                    frame = *(short *)lls;
                } else {
                    frame = 0;
                }
                lls = (struct LLS *)GetLLSForSprite((struct SpriteLLS *)alt);
                if (lls != NULL) {
                    LLSSetFrame(lls, frame);
                }
                PrintSprite(alt, node->x, node->y, 0, (int *)&ctx);
                return 0;
            }
        }
    }
    return 0;
}

// FUNCTION: LEGOLAND 0x0046e9d0
LEGO_EXPORT int RenderGBarSprite(struct IconNode *node) {
    struct PrintCtx ctx = {1};
    if (node->sprite != NULL) {
        PrintSprite(node->sprite, node->x, node->y, 0, (int *)&ctx);
    }
    return 0;
}

// FUNCTION: LEGOLAND 0x0046ea10
int FUN_0046ea10(struct IconNode *node) {
    struct PrintCtx ctx;
    RECT rect;
    RECT out;

    ctx.flags = 2;
    ctx.node = node;
    rect.left = (LONG)node->x;
    rect.top = (LONG)node->y;
    rect.right = node->width + rect.left;
    rect.bottom = node->height + rect.top;
    ctx.field_8 = 0;
    if (IntersectRect(&out, &rect, &((struct ScrollRegion *)node->field_30)->clip) != 0 && node->sprite != NULL) {
        PrintSprite(node->sprite, node->x, node->y, 0, (int *)&ctx);
    }
    return 0;
}

// FUNCTION: LEGOLAND 0x0046eaa0
int FUN_0046eaa0(struct IconNode *node) {
    struct PrintCtx ctx;

    ctx.flags = 2;
    ctx.node = node;
    ctx.field_8 = 0;
    if (GetBlink() != 0) {
        PrintSprite(node->sprite, node->x, node->y, 0, (int *)&ctx);
    }
    return 1;
}

// FUNCTION: LEGOLAND 0x0046eaf0
LEGO_EXPORT struct IconNode *SetupInterfacePanelIcons(unsigned int param_1, int param_2, int param_3, int param_4, int param_5, int param_6) {
    struct IconNode *icon;

    DAT_006688b8 = 0;
    AddFullScreenIcon((void *)(param_6 + 6));
    icon = InsertIcon(param_2 - 3, 0x20, param_6, IfSidebar1Sprite);
    icon->render_func = (void *)RenderGBarSprite;
    icon->field_18 = 10;
    icon->flags = icon->flags | 9;
    icon = InsertIcon(param_2 - 1, param_3, param_6 + 3, IfSideBUpSprite);
    icon->field_30 = (void *)param_1;
    icon->string_id = 0x95;
    icon->string = GetString(0x95);
    icon->width = 0x79;
    icon->height = 0x1e;
    icon->flags = icon->flags | 0x201b;
    icon->render_func = (void *)RenderScroll_Icons;
    icon->event_handler = (void *)FUN_0046d980;
    icon->field_18 = 1;
    icon = InsertIcon(param_2 - 1, param_5 - 0x20 + param_3, param_6 + 4, IfSideBDownSprite);
    icon->field_30 = (void *)param_1;
    icon->string_id = 0x94;
    icon->string = GetString(0x94);
    icon->width = 0x79;
    icon->height = 0x1e;
    icon->flags = icon->flags | 0x201b;
    icon->render_func = (void *)RenderScroll_Icons;
    icon->event_handler = (void *)FUN_0046da20;
    icon->field_18 = 2;
    icon = InsertIcon(param_2, param_3 + 0x1e, param_6 + 5, 0);
    icon->field_30 = (void *)param_1;
    icon->width = 0x79;
    icon->height = (short)param_5 - 0x3e;
    icon->box.box_color = GetNearestColour(0, 10, 0x14);
    icon->flags = icon->flags | 0x29;
    icon->render_func = (void *)RenderBoxIcon;
    return icon;
}

// FUNCTION: LEGOLAND 0x0046ec50
void FUN_0046ec50(char param) {
    char saved_mode = DAT_007fdd80;
    struct IconNode *node = IconListHead;

    if (DAT_007fdd80 == 2 || DAT_007fdd80 == 3) {
        return;
    }
    while (node != NULL) {
        short id = (short)node->id;
        if (id == 0xd2 || id == 0xd5 || id == 0xd6 || id == 0xd7) {
            if (DAT_007fdd80 == 0) {
                int old = node->x;
                node->x += param;
                if ((char)node->field_18 == 0xa && node->x >= 3) {
                    node->x = 0;
                    param = (char)node->x - (char)old;
                    DAT_007fdd8c = 3;
                    saved_mode = 3;
                    lpConfig->view_x = 0x79;
                    ScrollX = ScrollX + 0x7900;
                    lpConfig->view_width = 0x207;
                    BGFullUpdate = 1;
                    DAT_007fdd88 = 1;
                }
            } else if (DAT_007fdd80 == 1) {
                if (DAT_007fdd88 != 0) {
                    lpConfig->view_x = 0;
                    ScrollX = ScrollX - 0x7900;
                    lpConfig->view_width = 0x280;
                    DAT_007fdd88 = 0;
                    BGFullUpdate = 1;
                    return;
                }
                if ((char)node->field_18 == 0xa) {
                    int diff = (int)(short)node->x - (int)param;
                    if (diff < -0x7a) {
                        DAT_007fdd80 = 2;
                        saved_mode = 2;
                        DAT_007fdd8c = 0x86;
                        DestroyIconGroup(0xd2);
                        break;
                    }
                    diff = diff + 0x7a;
                    FUN_00461290((lpConfig->view_width - diff) * 0x100, lpConfig->view_height << 8, diff * 0x100, 0);
                }
                node->x = (short)node->x - (short)param;
            }
        }
        node = node->next;
    }

    DAT_007fdd80 = saved_mode;
}

// FUNCTION: LEGOLAND 0x0046ee00
void FUN_0046ee00(void) {
    struct IconNode *node = IconListHead;
    int mode;

    if (EditMode.unk4 == 1) {
        mode = 4;
    } else {
        switch (EditMode.unk0) {
        case 1:
            if (EditMode.unk8 != 0) {
                if (strcmp("Path", *(char **)((char *)EditMode.unk8 + 0x78)) == 0) {
                    mode = 1;
                    break;
                }
            }
        default:
            mode = 5;
            break;
        case 0:
            mode = 2;
            break;
        case 2:
            mode = 3;
            break;
        case 4:
            mode = 4;
            break;
        }
    }
    for (; node != NULL; node = node->next) {
        if (node->id == 0x93 && node->field_1c != NULL) {
            void *sprite = node->field_20p;
            if (sprite != NULL) {
                if (node->field_18 == mode) {
                    SetIconSprite(node, node->field_1c);
                } else {
                    SetIconSprite(node, sprite);
                }
            }
        }
    }
}

// FUNCTION: LEGOLAND 0x0046eee0
LEGO_EXPORT void RenderIcons(void) {
    struct PrintCtx ctx = {2};
    struct IconNode *node = (struct IconNode *)IconListHead;
    int elapsed;

    StoreClipping();
    SetClippingToScreenRect(0);
    elapsed = GetTicks() - RenderIconsLastTicks;
    if (elapsed > 0x3de) {
        elapsed = 0x3de;
    }
    FUN_0046ec50(elapsed * 5 / 33);
    RenderIconsLastTicks = GetTicks();
    while (node) {
        if ((node->flags & 0x400) == 0) {
            if (node->flags & 0x8) {
                ((void (*)(struct IconNode *))node->render_func)(node);
            } else if (node->sprite) {
                ctx.node = node;
                PrintSprite(node->sprite, node->x, node->y, 0, (int *)&ctx);
            }
            if (node == (struct IconNode *)FocussedIconPtr) {
                if (node->sprite) {
                    struct IconAnim *anim = ((struct IconSprite *)node->sprite)->anim;
                    if (anim->kind == 3 || anim->kind == 2) {
                        LLSPlayOnce(anim->lls, (unsigned int)anim);
                    }
                }
                if (node->flags & 0x8000) {
                    unsigned int colour = GetNearestColour(0, 0xff, 0);
                    RenderThickBox(node->x, node->y, node->width, node->height, 2, colour);
                }
            }
        }
        node = node->next;
    }
    DrawFlashingButtons();
    RestoreClipping();
}

// FUNCTION: LEGOLAND 0x0046f010
LEGO_EXPORT void RenderIcons2(short param_1, short param_2, short param_3) {
    struct PrintCtx ctx = {2};
    struct IconNode *node = (struct IconNode *)IconListHead;

    StoreClipping();
    SetClippingToScreenRect(0);
    while (node) {
        if ((node->flags & 0x400) == 0 && node->y > 0 && node->y < 0x1e0 &&
            ((short)node->id == param_1 || (short)node->id == param_2 || (short)node->id == param_3)) {
            if (node->flags & 0x8) {
                ((void (*)(struct IconNode *))node->render_func)(node);
            } else if (node->sprite) {
                ctx.node = node;
                PrintSprite(node->sprite, node->x, node->y, 0, (int *)&ctx);
            }
        }
        if (node == (struct IconNode *)FocussedIconPtr && node->sprite) {
            struct IconAnim *anim = ((struct IconSprite *)node->sprite)->anim;
            if (anim->kind == 3 || anim->kind == 2) {
                LLSPlayOnce(anim->lls, (unsigned int)anim);
            }
        }
        node = node->next;
    }
    RestoreClipping();
}

// FUNCTION: LEGOLAND 0x0046f100
void FUN_0046f100(short param_1) {
    struct PrintCtx ctx = {2};
    struct IconNode *node;
    int elapsed;

    elapsed = GetTicks() - DAT_006688cc;
    if (elapsed > 0x3de) {
        elapsed = 0x3de;
    }
    FUN_0046ec50(elapsed * 5 / 0x21);
    DAT_006688cc = GetTicks();
    node = IconListHead;
    StoreClipping();
    SetClippingToScreenRect(0);
    for (; node != NULL; node = node->next) {
        if ((node->flags & 0x400) == 0 && (short)node->id != param_1) {
            if (node->flags & 0x8) {
                ((void (*)(struct IconNode *))node->render_func)(node);
            } else if (node->sprite) {
                ctx.node = node;
                PrintSprite(node->sprite, node->x, node->y, 0, (int *)&ctx);
            }
        }
        if (node == (struct IconNode *)FocussedIconPtr && node->sprite) {
            struct IconAnim *anim = ((struct IconSprite *)node->sprite)->anim;
            if (anim->kind == 3 || anim->kind == 2) {
                LLSPlayOnce(anim->lls, (unsigned int)anim);
            }
        }
    }
    SetClippingToScreenRect(0);
    DrawFlashingButtons();
    RestoreClipping();
}

// FUNCTION: LEGOLAND 0x0046f200
LEGO_EXPORT void RenderHelpIcons(void) {
    struct PrintCtx ctx = {2};
    struct IconNode *node = DAT_006687cc;
    StoreClipping();
    SetClippingToScreenRect(0);
    for (; node != NULL; node = node->next) {
        if ((node->flags & 0x400) == 0 && node->x < 0x1e0 && node->x > 0) {
            if ((node->flags & 8) != 0) {
                ((void (*)(struct IconNode *))node->render_func)(node);
            } else if (node->sprite != NULL) {
                ctx.node = node;
                PrintSprite(node->sprite, node->x, node->y, 0, (int *)&ctx);
            }
            if (node == (struct IconNode *)FocussedIconPtr && (node->flags & 0x8000) != 0) {
                unsigned int colour = GetNearestColour(0, 0xff, 0);
                RenderThickBox(node->x, node->y, node->width, node->height, 2, colour);
            }
        }
    }
    RestoreClipping();
}

// FUNCTION: LEGOLAND 0x0046f2e0
unsigned char FUN_0046f2e0(struct IconNode *node, unsigned char buttons, short dx, short dy) {
    unsigned char r = buttons;
    if (r & 1) {
        return 2;
    }
    if (r & 4) {
        r = 2;
    } else {
        r = 1;
    }
    return r;
}

// FUNCTION: LEGOLAND 0x0046f300
int PointInBBox(struct Point *point, struct Bbox *bbox) {
    return point->x >= bbox->min_x && point->x <= bbox->max_x &&
        point->y >= bbox->min_y && point->y <= bbox->max_y;
}

// FUNCTION: LEGOLAND 0x0046f330
int FUN_0046f330(struct Point *point, struct IconNode *icon) {
    struct Bbox bbox;
    FUN_0046de90(icon, &bbox);
    return PointInBBox(point, &bbox);
}

// FUNCTION: LEGOLAND 0x0046f360
LEGO_EXPORT struct IconNode *GetIconAtPos(struct Point *param_1, unsigned char *param_2) {
    short x;
    short y;
    struct IconNode *found;
    struct IconNode *cur;
    int doGeom;
    int i;
    unsigned int flags;
    struct Bbox bbox;

    found = NULL;
    doGeom = 1;
    x = (short)param_1->x;
    y = (short)param_1->y;
    for (i = 0; i < 2; i++) {
        for (cur = i != 0 ? DAT_006687cc : IconListHead; cur != NULL; cur = cur->next) {
            if (doGeom) {
                flags = cur->flags;
                if ((flags & 0x10) != 0 && (flags & 0x400) == 0 && x >= cur->x && y >= cur->y &&
                    x <= cur->width + cur->x && y <= cur->height + cur->y) {
                    found = cur;
                } else if ((flags & 0x40) != 0 && (flags & 0x400) == 0 && FUN_0046f330(param_1, cur) != 0) {
                    *param_2 |= 4;
                    if (found == NULL || (found->flags & 0x800) == 0 ||
                        (cur->id - 1 != found->id - 3 && cur->id - 1 != found->id - 4)) {
                        found = NULL;
                    }
                }
            }
            if ((cur->flags & 0x20) != 0 && (cur->flags & 0x400) == 0) {
                FUN_0046de50((struct Rect16 *)cur, (struct Rect32 *)&bbox);
                if (x < bbox.min_x || x > bbox.max_x || y < bbox.min_y || y > bbox.max_y) {
                    doGeom = 0;
                } else {
                    doGeom = 1;
                }
            }
        }
    }
    return found;
}

// FUNCTION: LEGOLAND 0x0046f4c0
LEGO_EXPORT unsigned char CheckFocussedIcon(void) {
    struct IconNode *icon;

    if (DAT_006687c0 != 0 && (DAT_00813ad4 & 7) != 0) {
        return ((unsigned char (*)(unsigned int, unsigned int, unsigned int, unsigned int))DAT_006687c0)(0, DAT_00813ad4, 0, 0);
    }

    icon = (struct IconNode *)FocussedIconPtr;
    if (icon != NULL) {
        if ((icon->flags & 2) != 0 && icon->event_handler != NULL) {
            unsigned int buttons;
            if ((DAT_00813adc & 7) != 0 && DAT_00668954 == 0) {
                return ((unsigned char (*)(struct IconNode *, unsigned int, short, short))icon->event_handler)(
                    icon, DAT_00813adc, (short)MousePos.x - icon->x, (short)MousePos.y - icon->y);
            }
            buttons = DAT_00813ac4;
            DAT_00667c48 = 1;
            return ((unsigned char (*)(struct IconNode *, unsigned int, short, short))icon->event_handler)(
                icon, buttons, (short)MousePos.x - icon->x, (short)MousePos.y - icon->y);
        }
        {
            unsigned int buttons = DAT_00813adc;
            if ((DAT_00813adc & 7) == 0 || DAT_00668954 != 0) {
                buttons = DAT_00813ac4;
                DAT_00667c48 = 1;
            }
            return FUN_0046f2e0(icon, buttons, (short)MousePos.x - icon->x, (short)MousePos.y - icon->y);
        }
    }

    if (DAT_006687bc != 0 && (DAT_00813adc & 7) != 0) {
        return ((unsigned char (*)(unsigned int, unsigned int, unsigned int, unsigned int))DAT_006687bc)(0, DAT_00813adc, 0, 0);
    }
    return 0;
}

// FUNCTION: LEGOLAND 0x0046f690
LEGO_EXPORT struct IconNode *AddGBarClassIcon(unsigned int param_1, struct InfoSource *src, int a3, int a4, int a5, short a6) {
    struct IconNode *icon = InsertIcon(a3, a4, a5, GBarFrameSprite);
    if (icon != NULL) {
        struct Sprite *sprite = src->field_68;
        void *value;
        unsigned int flags;
        if (sprite != NULL) {
            unsigned short width = sprite->width;
            unsigned short height = sprite->height;
            SetIconSprite(icon, sprite);
            icon->height = height;
            icon->width = width;
        } else {
            SetIconSprite(icon, NULL);
        }
        icon->field_18p = src->field_78;
        if ((src->field_c4[2] & 0x10000) != 0) {
            value = src->field_78;
        } else {
            value = src->field_7c;
        }
        icon->string = value;
        icon->field_1c = value;
        icon->string_id = 0xffffffff;
        if ((src->field_c4[2] & 0x20000) != 0) {
            *(unsigned char *)&icon->field_1c = 0x14;
        }
        flags = icon->flags & 0xfffffdff;
        icon->field_30 = (void *)param_1;
        icon->render_func = (void *)RenderGBarSpriteIcon;
        icon->flags = flags | 0x3008;
        icon->field_8 = src;
        icon->field_16 = a6;
        if (DAT_006688a8 != 0) {
            icon->field_24 = DAT_006688a8;
            icon->flags = icon->flags | 4;
        }
        if (DAT_006688ac != 0) {
            icon->render_func = DAT_006688ac;
            icon->flags = icon->flags | 8;
        }
        if (DAT_006688b0 != 0) {
            icon->event_handler = DAT_006688b0;
            icon->flags = icon->flags | 2;
        }
    }
    return icon;
}

// FUNCTION: LEGOLAND 0x0046f7a0
LEGO_EXPORT struct IconNode *AddFreePlayIcon(unsigned int param_1, struct InfoSource *src, int a3, int a4, int a5, short a6, void *a7) {
    struct IconNode *icon = InsertIcon(a3, a4, a5, GBarFrameSprite);
    if (icon != NULL) {
        struct Sprite *sprite = src->field_14;
        unsigned int flags;
        if (sprite != NULL) {
            SetIconSprite(icon, sprite);
        } else {
            SetIconSprite(icon, NULL);
        }
        flags = icon->flags;
        icon->render_func = (void *)RenderGBarSpriteIcon;
        icon->string = (char *)src->field_10;
        icon->string_id = 0xffffffff;
        icon->flags = flags | 0x3008;
        icon->field_16 = a6;
        icon->field_8 = src->field_8;
        icon->field_18 = 0;
        icon->field_1c = src->field_4;
        icon->field_20p = a7;
        if (DAT_006688a8 != 0) {
            icon->field_24 = DAT_006688a8;
            icon->flags = icon->flags | 4;
        }
        if (DAT_006688ac != 0) {
            icon->render_func = DAT_006688ac;
            icon->flags = icon->flags | 8;
        }
        if (DAT_006688b0 != 0) {
            icon->event_handler = DAT_006688b0;
            icon->flags = icon->flags | 2;
        }
    }
    return icon;
}

// FUNCTION: LEGOLAND 0x0046f890
void LoadGBarSprites(void) {
    if (GBarSpritesLoaded != 0) {
        return;
    }
    if (GBarFrameSprite == 0) {
        // STRING: LEGOLAND 0x004ba8c8
        GBarFrameSprite = LoadSprite("GBarFrame.lls", 4);
    }
    if (IfSideBUpSprite == 0) {
        // STRING: LEGOLAND 0x004ba8b8
        IfSideBUpSprite = LoadSprite("IF_Side_BUp.lls", 4);
    }
    if (IfSideBDownSprite == 0) {
        // STRING: LEGOLAND 0x004ba8a4
        IfSideBDownSprite = LoadSprite("IF_Side_BDown.lls", 4);
    }
    if (IfSidebar1Sprite == 0) {
        // STRING: LEGOLAND 0x004ba894
        IfSidebar1Sprite = LoadSprite("IF_Sidebar1.lls", 4);
    }
    GBarSpritesLoaded = 1;
}

// FUNCTION: LEGOLAND 0x0046f920
void UnloadGBarSprites(void) {
    if (GBarSpritesLoaded != 0) {
        GBarSpritesLoaded = 0;
        if (GBarFrameSprite != 0) {
            KillSprite(GBarFrameSprite);
            GBarFrameSprite = 0;
        }
        if (IfSideBUpSprite != 0) {
            KillSprite(IfSideBUpSprite);
            IfSideBUpSprite = 0;
        }
        if (IfSideBDownSprite != 0) {
            KillSprite(IfSideBDownSprite);
            IfSideBDownSprite = 0;
        }
        if (IfSidebar1Sprite != 0) {
            KillSprite(IfSidebar1Sprite);
            IfSidebar1Sprite = 0;
        }
    }
}

// FUNCTION: LEGOLAND 0x0046f9a0
int FUN_0046f9a0(int param_1, int param_2, int param_3, int param_4, unsigned int param_5, int param_6) {
    int saved = param_1;
    int i = 0;
    int found = 0;
    int total;
    struct MenuGroup *group;
    struct IconNode *icon;
    struct ListElement *elem;

    group = (struct MenuGroup *)malloc(sizeof(struct MenuGroup));
    if (group == NULL) {
        return 0;
    }
    icon = AddGBarIcons((unsigned int)group, param_3, param_4, param_5, param_6, param_1);
    group->icon = icon;
    param_1 = icon->x;
    group->field_1c = param_1;
    group->field_c = param_1;
    param_6 = icon->y;
    group->field_20 = param_6;
    group->field_10 = param_6;
    group->field_24 = icon->width + icon->x;
    group->field_28 = icon->height + icon->y;
    group->field_4 = param_5;
    group->field_0 = (short)saved;
    SetNewGroup_Callbacks(0, 0, (void *)FUN_00470000);
    total = LLIDB_GetCount();
    if (total > 0) {
        do {
            LLIDB_GetElement(i, (int *)&elem);
            if ((elem->field_8 & 0x10) != 0 && elem->field_c[0x16] == param_2) {
                found = found + 1;
                AddGBarClassIcon((unsigned int)group, (struct InfoSource *)elem->field_c, param_1, param_6, saved, (short)i);
                if ((param_5 & 1) == 0) {
                    param_1 = param_1 + 0x79;
                } else {
                    param_6 = param_6 + 0x38;
                }
            }
            i = i + 1;
        } while (i < total);
    }
    AddFullScreenIcon((void *)(saved + 6));
    icon = group->icon;
    group->field_14 = param_1;
    group->field_18 = param_6;
    if ((param_5 & 1) == 0) {
        if (param_1 < icon->x + icon->width) {
            icon->width = (short)(group->field_14 - group->field_c);
            icon = FindIcon((unsigned short)(saved + 4));
            if (icon != NULL) {
                icon->x = (short)group->field_14;
            }
        }
    } else if (param_6 < icon->y + icon->height) {
        icon->height = (short)param_6 - (short)group->field_10;
        icon = FindIcon((unsigned short)(saved + 4));
        if (icon != NULL) {
            icon->y = (short)group->field_18;
        }
    }
    if (found == 0) {
        DestroyIconGroup(saved);
    }
    return found;
}

// FUNCTION: LEGOLAND 0x0046fb40
void DestroyIconGroup(unsigned int group) {
    struct IconNode *icon1;
    struct IconNode *icon2;
    struct IconNode *icon3;
    struct IconNode *icon4;

    icon1 = FindIcon(group + 5);
    if (icon1) {
        icon2 = FindIcon(group + 3);
        if (icon2) {
            FUN_0046d4e0(icon2);
        }
        icon3 = FindIcon(group + 4);
        if (icon3) {
            FUN_0046d4e0(icon3);
        }
        icon4 = FindIcon(group + 6);
        if (icon4) {
            FUN_0046d4e0(icon4);
        }
        free(icon1->field_30);
        FUN_0046d4e0(icon1);
        RemoveIconGroup(group);
    }
}

// FUNCTION: LEGOLAND 0x0046fbc0
unsigned char FUN_0046fbc0(struct IconNode *icon, unsigned char flag) {
    struct Indicator *ind = (struct Indicator *)icon->field_30;
    if (flag & 0x5) {
        if (ind->field_4 & 0x4) {
            ind->field_1c(ind);
        }
        return 2;
    }
    return 1;
}

// FUNCTION: LEGOLAND 0x0046fbf0
LEGO_EXPORT struct TimedIndicator *AllocateTimedIndicator(struct Sprite *sprite, unsigned int a2, unsigned int a3) {
    struct TimedIndicator *ind;
    struct IconNode *icon;

    ind = malloc(40);
    ind->next = DAT_006688d4;
    ind->field_4 = 0;
    ind->start_time = GetGameTimer();
    ind->duration = a2;
    ind->field_10 = a3;

    ReferenceSprite(sprite);
    icon = InsertIcon(0, 0, 0xe000, sprite);
    ind->icon = icon;
    icon->render_func = (void *)FUN_0046eaa0;

    icon = ind->icon;
    icon->event_handler = (void *)FUN_0046fbc0;

    icon = ind->icon;
    icon->flags = icon->flags | 0x40a;

    icon = ind->icon;
    icon->field_30 = ind;

    ind->check_func = 0;
    ind->click_func = 0;

    DAT_006688d4 = (struct Indicator *)ind;
    return ind;
}

// FUNCTION: LEGOLAND 0x0046fc80
LEGO_EXPORT struct TimedIndicator *AllocatePermanentIndicator(struct Sprite *sprite, unsigned int param_2) {
    struct TimedIndicator *ind;
    struct IconNode *icon;

    ind = malloc(40);
    ind->next = DAT_006688d4;
    ind->field_4 = 1;
    ind->start_time = GetGameTimer();
    ind->field_10 = param_2;

    ReferenceSprite(sprite);
    icon = InsertIcon(0, 0, 0xe000, sprite);
    ind->icon = icon;
    icon->render_func = (void *)FUN_0046eaa0;

    icon = ind->icon;
    icon->event_handler = (void *)FUN_0046fbc0;

    icon = ind->icon;
    icon->flags = icon->flags | 0x40a;

    icon = ind->icon;
    icon->field_30 = ind;

    ind->check_func = 0;
    ind->click_func = 0;

    DAT_006688d4 = (struct Indicator *)ind;
    return ind;
}

// FUNCTION: LEGOLAND 0x0046fd00
LEGO_EXPORT void SetCheckFunc(struct IndicatorFuncs *ind, unsigned int func) {
    ind->check_func = func;
    ind->field_4 |= 0x4;
}

// FUNCTION: LEGOLAND 0x0046fd20
LEGO_EXPORT void SetClickFunc(struct IndicatorFuncs *ind, unsigned int func) {
    ind->click_func = func;
    ind->field_4 |= 0x4;
}

// FUNCTION: LEGOLAND 0x0046fd40
LEGO_EXPORT void AddIndicator(struct Indicator *ind) {
    struct Indicator *cur = DAT_006688d4;
    if (cur == ind) {
        DAT_006688d4 = cur->next;
    } else {
        while (cur != NULL) {
            if (cur->next == ind) {
                cur->next = ind->next;
                break;
            }
            cur = cur->next;
        }
    }
    if (cur != NULL) {
        ind->next = ActiveIndicators;
        ActiveIndicators = ind;
        ind->field_4 = ind->field_4 | 8;
        ind->field_14->x = 0xf000;
        ind->field_14->flags = ind->field_14->flags & 0xfffffbff;
    }
}

// FUNCTION: LEGOLAND 0x0046fda0
LEGO_EXPORT void RemoveIndicator(struct Indicator *ind) {
    struct Indicator *cur = ActiveIndicators;
    if (cur == ind) {
        ActiveIndicators = cur->next;
    } else {
        while (cur != NULL) {
            if (cur->next == ind) {
                cur->next = ind->next;
                break;
            }
            cur = cur->next;
        }
    }
    if (cur != NULL) {
        ind->next = DAT_006688d4;
        DAT_006688d4 = ind;
        ind->field_4 = ind->field_4 & 0xfffffff7;
        ind->field_14->flags = ind->field_14->flags | 0x400;
    }
}

// FUNCTION: LEGOLAND 0x0046fe00
LEGO_EXPORT void DeleteIndicator(struct Indicator *ind) {
    struct Indicator *cur;

    FUN_0046d4e0(ind->field_14);

    if ((ind->field_4 & 0x8) != 0) {
        if (ActiveIndicators == ind) {
            ActiveIndicators = ((struct Indicator *)ActiveIndicators)->next;
            free(ind);
            return;
        }
        cur = ActiveIndicators;
        while (cur != 0) {
            if (cur->next == ind) {
                cur->next = ind->next;
                free(ind);
                return;
            }
            cur = cur->next;
        }
        free(ind);
        return;
    }

    if (DAT_006688d4 == ind) {
        DAT_006688d4 = ((struct Indicator *)DAT_006688d4)->next;
        free(ind);
        return;
    }
    cur = DAT_006688d4;
    while (cur != 0) {
        if (cur->next == ind) {
            cur->next = ind->next;
            free(ind);
            return;
        }
        cur = cur->next;
    }
    free(ind);
}

// FUNCTION: LEGOLAND 0x0046feb0
LEGO_EXPORT void ControlIndicators(void) {
    struct Indicator *node = ActiveIndicators;
    int now = GetGameTimer();
    int x = ((struct Config *)lpConfig)->field_0 - 0x50;
    struct Indicator *cur;
    struct Indicator *head;
    struct Indicator *prev;

    ActiveIndicators = NULL;
    while (node != NULL) {
        node->field_10 &= 0xffff;
        if (now - (int)node->field_8 < 0x1388) {
            node->field_10 |= 0x10000;
        } else {
            node->field_14->flags &= ~8;
        }
        if ((node->field_4 & 1) != 0) {
            node->field_10 |= 0x20000;
        } else if (now - (int)node->field_8 >= (int)node->field_c) {
            cur = node;
            node = node->next;
            cur->next = ActiveIndicators;
            ActiveIndicators = cur;
            RemoveIndicator(cur);
            if ((cur->field_4 & 2) != 0) {
                DeleteIndicator(cur);
            }
            continue;
        }
        head = ActiveIndicators;
        cur = node;
        node = node->next;
        if (head == NULL) {
            ActiveIndicators = cur;
            cur->next = NULL;
            continue;
        }
        for (prev = NULL; head != NULL; prev = head, head = head->next) {
            if ((int)head->field_10 < (int)cur->field_10) {
                if (prev != NULL) {
                    prev->next = cur;
                } else {
                    ActiveIndicators = cur;
                }
                cur->next = head;
                break;
            }
        }
        if (head == NULL) {
            prev->next = cur;
            cur->next = NULL;
        }
    }

    for (node = ActiveIndicators; node != NULL; node = node->next) {
        node->field_14->x = (short)x;
        x = x - 0x34;
        node->field_14->y = 8;
        if (x <= (int)(((struct Config *)lpConfig)->field_0 >> 2)) {
            break;
        }
    }
    for (; node != NULL; node = node->next) {
        node->field_14->x = (short)0xf000;
    }
}

// FUNCTION: LEGOLAND 0x00470000
unsigned char FUN_00470000(struct IconNode *node, unsigned char buttons) {
    int cost;
    unsigned int *sub;
    unsigned int flags;

    if ((buttons & 2) != 0) {
        do {
            cost = GetObjCost(node->field_8);
            if (cost <= GetBrickCount()) {
                if ((node->flags & 0x1000) != 0) {
                    SetEditObject(node->field_8);
                }
                sub = *(unsigned int **)((char *)node->field_8 + 0xc4);
                flags = sub[2];
                if ((flags & 0x20000) != 0) {
                    sub[2] = flags & 0xfffdffff;
                    RemoveNewObject(node->field_8);
                }
                PlayInstanceOfSample(GameFX[FX_CLICK1].sample, 0, 1, 0);
            } else {
                PlayInstanceOfSample(GameFX[FX_RASP].sample, 0, 1, 0);
            }
        } while (0);
        return 2;
    }
    return 1;
}

// FUNCTION: LEGOLAND 0x004700a0
LEGO_EXPORT void UpdateFocussedIconPtr(void) {
    if (Hover.type == 2) {
        FocussedIconPtr = (void *)Hover.ptr;
    } else {
        FocussedIconPtr = NULL;
    }
}
