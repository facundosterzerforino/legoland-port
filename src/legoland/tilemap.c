#include <stdlib.h>
#include <string.h>
#include "legoland.h"
#include "math.h"

#include "bloke.h"
#include "build.h"
#include "clipping.h"
#include "draw.h"
#include "gamemain.h"
#include "gamemap.h"
#include "globals.h"
#include "llidb.h"
#include "map_object.h"
#include "obj_instance.h"
#include "pathfind.h"
#include "print_sprite.h"
#include "tilemap.h"
#include "timer.h"
#include "worker.h"

struct MapTile {
    /* 0x00 */ unsigned char pad_0[8];
    /* 0x08 */ unsigned short tile;
    /* 0x0a */ unsigned short base_id;
    /* 0x0c */ unsigned short flags_c;
    /* 0x0e */ unsigned char pad_e[2];
    /* 0x10 */ unsigned char flags_10;
    /* 0x11 */ unsigned char pad_11[3];
};

struct TileSprite {
    unsigned char pad_0[0x16];
    short size;
};

#include "image_sprite.h"

// FUNCTION: LEGOLAND 0x0045a9b0
LEGO_EXPORT unsigned int *AllocTileSpace(void *manager, int count, unsigned int *out) {
    struct FXSpriteList *src = (struct FXSpriteList *)manager;
    int base;
    int run;
    unsigned int *slot;
    unsigned int *cursor;
    unsigned int i;
    int slot_value;

    count = count & 0xffff;
    base = 0;
    while (count + base <= 0x800) {
        slot = (unsigned int *)&TileSpriteArray[base];
        run = 0;
        slot_value = (int)TileSpriteArray[base];
        while (slot_value == -1) {
            run++;
            slot++;
            if (run >= count) {
                cursor = (unsigned int *)&TileSpriteArray[base];
                memset(cursor, 0, (unsigned int)count * 4);
                for (i = 0; (int)i < count; i++) {
                    TileSpriteInfo[base + i].src = src;
                    if (src != NULL) {
                        TileSpriteInfo[base + i].sprite = (unsigned short)src->sprite_ids[i];
                    } else {
                        TileSpriteInfo[base + i].sprite = 0;
                    }
                }
                *(unsigned short *)out = (unsigned short)base;
                return (unsigned int *)&TileSpriteArray[base];
            }
            slot_value = (int)*slot;
        }
        base = run + base + 1;
    }
    return NULL;
}

// FUNCTION: LEGOLAND 0x0045aa50
int FUN_0045aa50(struct FXSpriteList **out) {
    int count = 0;
    struct FXSpriteList *prev = NULL;
    struct FXSpriteList *src;
    struct FXSpriteList **dst = out;
    struct TileSpriteEntry *info = TileSpriteInfo;
    void **slot = TileSpriteArray;

    do {
        if (*slot != (void *)-1) {
            src = info->src;
            if (src != NULL && src != prev) {
                *dst = src;
                count++;
                prev = src;
                dst++;
            }
        }
        slot++;
        info++;
    } while ((int)slot < (int)MapRenderOrderList);
    return count;
}

// FUNCTION: LEGOLAND 0x0045aa90
LEGO_EXPORT void FreeTileSpace(unsigned short index, unsigned short count) {
    memset(&TileSpriteArray[index], 0xFF, count * 4);
}

// FUNCTION: LEGOLAND 0x0045aad0
LEGO_EXPORT unsigned int LoadMapTiles(void) {
    unsigned short alloc_index;
    unsigned int handle;
    int offset;
    unsigned int row;
    unsigned int p;

    if (GameMapRawBlock == 0) {
        p = (unsigned int)calloc(0x14041f, 1);
        GameMap = (struct MapElement **)((p + 0x1f) & 0xffffffe0);
        row = (p + 0x41f) & 0xffffffe0;
        GameMapRawBlock = (void *)p;
        offset = 0;
        do {
            *(unsigned int *)((char *)GameMap + offset) = row;
            offset += 4;
            row += 0x1400;
        } while (offset < 0x400);
    }
    FreeTileSpace(0, 0x800);
    AllocTileSpace(0, 1, (unsigned int *)&alloc_index);
    LLIDB_FindElement("MAPPING 1", &handle, 0);
    LLIDB_LoadData((void *)handle);
    LLIDB_FindElement("BASIC TILES 1", &handle, 0);
    DAT_00801a6c = *(void **)(handle + 0xc);
    DAT_00667ca4 = *(unsigned int *)DAT_00801a6c;
    DAT_008003f8 = 0;
    DAT_00801b20 = 1;
    DAT_0080ff60 = 2;
    DAT_00805f48 = 3;
    DAT_0080ff68 = 4;
    LLIDB_FindElement("NORMAL PATH TILES", &handle, 0);
    PathSprite = *(void **)(handle + 0xc);
    // STRING: LEGOLAND 0x004b9bc4
    Arrow01Sprite = LoadSprite("arrow01.lls", 1);
    // STRING: LEGOLAND 0x004b9bb8
    Arrow02Sprite = LoadSprite("arrow02.lls", 1);
    // STRING: LEGOLAND 0x004b9bac
    Arrow03Sprite = LoadSprite("arrow03.lls", 1);
    // STRING: LEGOLAND 0x004b9ba0
    Arrow04Sprite = LoadSprite("arrow04.lls", 1);
    return 1;
}

// FUNCTION: LEGOLAND 0x0045ac20
unsigned int UnloadMapTiles(void) {
    unsigned int handle;

    if (GameMapRawBlock != 0) {
        free(GameMapRawBlock);
    }
    // STRING: LEGOLAND 0x004b9be4
    LLIDB_FindElement("MAPPING 1", &handle, 0);
    LLIDB_UnLoadData(handle);
    if (Arrow01Sprite != 0) {
        KillSprite(Arrow01Sprite);
        Arrow01Sprite = 0;
    }
    if (Arrow02Sprite != 0) {
        KillSprite(Arrow02Sprite);
        Arrow02Sprite = 0;
    }
    if (Arrow03Sprite != 0) {
        KillSprite(Arrow03Sprite);
        Arrow03Sprite = 0;
    }
    if (Arrow04Sprite != 0) {
        KillSprite(Arrow04Sprite);
        Arrow04Sprite = 0;
    }
    return 1;
}

// FUNCTION: LEGOLAND 0x0045acc0
LEGO_EXPORT void GetTileBounds(struct Point *ref, int *out) {
    struct TileSprite *sprite = (struct TileSprite *)TileSpriteArray[DAT_00667ca4];
    short size = sprite->size;
    int top;

    out[0] = (short)(((short)(size * 2) + 1) >> 1) * ((ref->x - ref->y) - 1) + lpConfig->view_x - (ScrollX >> 8);
    top = (short)((size + 1) >> 1) * (ref->x + ref->y) + lpConfig->view_y - (ScrollY >> 8);
    out[1] = top;
    out[2] = (short)(size * 2) - 1 + out[0];
    out[3] = top - 1 + size;
}

// FUNCTION: LEGOLAND 0x0045ad60
LEGO_EXPORT void GetTileCentre(struct Point *ref, int *out) {
    struct TileSprite *sprite = (struct TileSprite *)TileSpriteArray[DAT_00667ca4];
    short size = sprite->size;
    int y;

    out[0] = (short)(((short)(size * 2) + 1) >> 1) * (ref->x - ref->y) + lpConfig->view_x - (ScrollX >> 8);
    y = (short)((size + 1) >> 1) * (ref->y + 1 + ref->x) + lpConfig->view_y;
    out[1] = y - (ScrollY >> 8);
}

// FUNCTION: LEGOLAND 0x0045ade0
void FUN_0045ade0(void) {
    int iVar1;
    int iVar2;
    int iVar3;
    char cVar4;
    int iVar7;
    int iVar8;
    int iVar9;
    int iVar10;
    int iVar11;
    int iVar13;
    int local_50;
    int local_4c;
    int local_2c;
    RECT local_24;
    short size;
    struct MapTile tile;

    local_24.left = lpConfig->view_x;
    local_24.top = lpConfig->view_y;
    local_24.right = lpConfig->screen_width;
    local_24.bottom = lpConfig->screen_height;
    SetClipping(&local_24);
    size = ((struct TileSprite *)TileSpriteArray[DAT_00667ca4])->size;
    iVar9 = (int)size;
    iVar13 = (short)(size * 2);
    iVar1 = iVar9 + 1 >> 1;
    local_4c = (ScrollY >> 8) - iVar1;
    iVar2 = (ScrollX >> 8) / iVar13;
    iVar7 = iVar13 + 1 >> 1;
    local_50 = (ScrollX >> 8) % iVar13;
    iVar3 = local_4c / iVar9;
    local_4c = local_4c % iVar9;
    local_2c = iVar3 + -3 + iVar2;
    iVar3 = iVar3 - iVar2;
    cVar4 = (iVar7 <= local_50) + '\x01';
    if (iVar1 < local_4c) {
        cVar4 = (iVar7 <= local_50) + '\x03';
    }
    switch (cVar4) {
    case '\x01':
        if (local_50 < iVar7 + local_4c * -2) {
            local_50 = local_50 + iVar7;
            local_2c = local_2c + -1;
            local_4c = local_4c + iVar1;
        }
        break;
    case '\x02':
        if (iVar7 + local_4c * 2 <= local_50) {
            local_50 = local_50 - iVar7;
            iVar3 = iVar3 + -1;
            local_4c = local_4c + iVar1;
        }
        break;
    case '\x03':
        if (iVar7 + (local_4c - iVar9) * 2 <= local_50) {
            break;
        }
        iVar3 = iVar3 + 1;
        iVar2 = iVar7;
        local_50 = local_50 + iVar2;
        local_4c = local_4c - iVar1;
        break;
    case '\x04':
        if (local_50 < iVar7 + (iVar9 - local_4c) * 2) {
            break;
        }
        local_2c = local_2c + 1;
        iVar2 = -iVar7;
        local_50 = local_50 + iVar2;
        local_4c = local_4c - iVar1;
    }
    local_4c = (local_24.top + iVar9 * -2) - local_4c;
    if (local_4c < (int)(iVar9 * 2 + local_24.bottom)) {
        do {
            iVar2 = local_2c;
            iVar8 = (local_24.left + iVar13 * -2) - local_50;
            if (iVar8 < (int)(iVar13 * 2 + local_24.right)) {
                iVar10 = iVar3;
                do {
                    if (local_2c >= 0 && local_2c < (int)lpConfig->width && iVar10 >= 0 &&
                        iVar10 < (int)lpConfig->height) {
                        tile = *(struct MapTile *)&GameMap[iVar10][local_2c];
                    } else {
                        tile.flags_10 = 0;
                    }
                    if ((tile.flags_10 & 2) != 0) {
                        PrintSprite((struct Sprite *)TileSpriteArray[(DAT_00805f48 & 0xff) + *(unsigned int *)DAT_00801a6c], iVar8, local_4c, 0xff6868, 0);
                    }
                    local_2c = local_2c + 1;
                    iVar8 = iVar8 + iVar13;
                    iVar10 = iVar10 + -1;
                } while (iVar8 < (int)(iVar13 * 2 + local_24.right));
            }
            iVar2 = iVar2 + 1;
            iVar8 = (local_24.left + iVar13 * -2) - local_50;
            if (iVar8 < (int)(iVar13 * 2 + local_24.right)) {
                iVar10 = iVar8 + iVar7;
                iVar11 = iVar3;
                local_2c = iVar2;
                do {
                    if (local_2c >= 0 && local_2c < (int)lpConfig->width && iVar11 >= 0 &&
                        iVar11 < (int)lpConfig->height) {
                        tile = *(struct MapTile *)&GameMap[iVar11][local_2c];
                    } else {
                        tile.flags_10 = 0;
                    }
                    if ((tile.flags_10 & 2) != 0) {
                        PrintSprite((struct Sprite *)TileSpriteArray[(DAT_00805f48 & 0xff) + *(unsigned int *)DAT_00801a6c], iVar10, local_4c + iVar1, 0xff6868, 0);
                    }
                    local_2c = local_2c + 1;
                    iVar8 = iVar8 + iVar13;
                    iVar11 = iVar11 + -1;
                    iVar10 = iVar10 + iVar13;
                } while (iVar8 < (int)(iVar13 * 2 + local_24.right));
            }
            iVar3 = iVar3 + 1;
            local_4c = local_4c + iVar9;
            local_2c = iVar2;
        } while (local_4c < (int)(iVar9 * 2 + local_24.bottom));
    }
}

// FUNCTION: LEGOLAND 0x0045b170
void FUN_0045b170(struct Point *pt) {
}

/* RideSpriteInfo followed by one more zeroed word (RenderView local copy). */
struct RenderSprite {
    RideSpriteInfo info;
    unsigned int field_14;
};

#define HALF(v) ((v) < 0 ? -(-(v) >> 1) : (v) >> 1)

static __inline struct MapElement *GetTile(int x, int y) {
    if (x < 0 || x >= (int)lpConfig->width || y < 0 || y >= (int)lpConfig->height) {
        return 0;
    }
    return &GameMap[y][x];
}

// FUNCTION: LEGOLAND 0x0045b180
LEGO_EXPORT void RenderView(void) {
    struct MapElement *list[3000];
    RECT clip;
    struct HitInfo hit_a;
    struct HitInfo hit_b;
    int bounds[4];
    /* iVar1..iVar8 are scratch: tile metrics while scanning the map, strip maths while drawing. */
    int b[6];
    struct Ride *rides;
    RECT rect;
    int ya;
    int savey;
    int iVar5;
    int iVar4;
    int iVar3;
    int xlim;
    int q;
    int count;
    TileId uid;
    struct Point pt;
    int rx;
    int h;
    int iVar1;
    int iVar2;
    int hw;
    int ry;
    unsigned char sel;
    int sx;
    struct MapElement *tile;
    struct MapElement *anchor;
    int i;
    struct Ride *ride;
    RideSpriteInfo *info;

    rect.left = lpConfig->view_x;
    rect.top = lpConfig->view_y;
    rect.right = lpConfig->view_width + lpConfig->view_x;
    rect.bottom = lpConfig->view_height + lpConfig->view_y;
    count = 0;
    rides = ObjectClassList;
    BGFullUpdate = 1;
    DAT_00667cc4 = 0;
    GetClipping(&clip);
    FUN_00460e00();
    PrintBackground(DAT_00667cd0, DAT_00667cd4);
    if (EditMode.unk0 == 2 || (EditMode.unk0 == 1 && EditMode.unk8 == PathControlObject)) {
        DAT_00667d40 = 1;
        DAT_00667d48 = (GetTickCount() & 0x100) ? 0xff : 0;
    } else {
        DAT_00667d40 = 0;
    }
    SetClipping(&rect);

    /* Scroll position -> first map tile of the view (same maths as FUN_0045ade0). */
    h = ((struct TileSprite *)TileSpriteArray[DAT_00667ca4])->size;
    iVar1 = (short)(((struct TileSprite *)TileSpriteArray[DAT_00667ca4])->size * 2);
    iVar2 = (h + 1) >> 1;
    ry = (ScrollY >> 8) - iVar2;
    q = (ScrollX >> 8) / iVar1;
    hw = (iVar1 + 1) >> 1;
    rx = (ScrollX >> 8) % iVar1;
    pt.y = ry / h;
    ry = ry % h;
    pt.x = pt.y + -3 + q;
    pt.y = pt.y - q;
    sel = (rx >= hw) + '\x01';
    if (ry > iVar2) {
        sel = (rx >= hw) + '\x03';
    }
    switch (sel) {
    case 1:
        if (rx < hw + ry * -2) {
            pt.x = pt.x + -1;
            rx = rx + hw;
            ry = ry + iVar2;
        }
        break;
    case 2:
        if (hw + ry * 2 <= rx) {
            rx = rx - hw;
            pt.y = pt.y + -1;
            ry = ry + iVar2;
        }
        break;
    case 3:
        if (hw + (ry - h) * 2 <= rx) {
            break;
        }
        pt.y = pt.y + 1;
        rx = rx + hw;
        ry = ry - iVar2;
        break;
    case 4:
        if (rx < hw + (h - ry) * 2) {
            break;
        }
        pt.x = pt.x + 1;
        rx = rx - hw;
        ry = ry - iVar2;
    }

    /* Walk the visible tiles in diagonal rows; list each object once (0x400 marks it as listed). */
    xlim = iVar1 * 2 + rect.right;
    iVar3 = lpConfig->view_height + rect.bottom;
    for (iVar5 = (rect.top - h * 2) - ry; iVar5 < iVar3; iVar5 += h) {
        q = pt.x;
        savey = pt.y;
        for (sx = (rect.left - iVar1 * 2) - rx; sx < xlim; sx += iVar1) {
            tile = GetTile(pt.x, pt.y);
            if (tile != NULL) {
                FUN_0045b170(&pt);
                if (tile->flags & 0xa0) {
                    anchor = GetTile(tile->field_4, tile->field_5);
                    if ((anchor->flags & 0x400) == 0) {
                        list[count++] = anchor;
                        anchor->flags |= 0x400;
                    }
                }
            }
            pt.x++;
            if (tile != NULL) {
                if (pt.x == (int)lpConfig->width) {
                    break;
                }
                tile++;
            }
            if (tile == NULL) {
                tile = GetTile(pt.x, pt.y);
            }
            if (tile != NULL) {
                FUN_0045b170(&pt);
                if (tile->flags & 0xa0) {
                    anchor = GetTile(tile->field_4, tile->field_5);
                    if ((anchor->flags & 0x400) == 0) {
                        list[count++] = anchor;
                        anchor->flags |= 0x400;
                    }
                }
            }
            pt.y--;
        }
        pt.x = q + 1;
        pt.y = savey + 1;
    }

    /* Pre-render callback of each object class. */
    for (; rides != NULL; rides = rides->next) {
        if ((rides->flags & 0x20) && rides->cb_pre_render != NULL) {
            rides->cb_pre_render(rides->element);
        }
    }

    /* Draw every listed object, sliced into vertical strips for depth sorting. */
    list[count] = NULL;
    if (count > 0) {
        for (i = 0; i < count; i++) {
            struct RenderSprite loc = {0};
            int step;
            int iVar6;
            int iVar7;
            int iVar8;
            RECT *part;
            RECT *prev;
            unsigned int k;

            tile = list[i];
            tile->flags &= 0xfbff;
            if (tile->field_0 == NULL) {
                continue;
            }
            ride = tile->field_0->ride;
            ObjectPartCount = 0;
            uid.id = tile->anchor.id;
            pt.x = tile->field_4 + ride->footprint.x0;
            pt.y = tile->field_5 + ride->footprint.y1;
            GetTileBounds(&pt, b);
            iVar1 = b[0];
            ya = ((b[1] + b[3]) >> 1) - lpConfig->view_y;
            pt.x = tile->field_4 + ride->footprint.x1;
            pt.y = tile->field_5 + ride->footprint.y0;
            GetTileBounds(&pt, b);
            b[4] = b[2];
            b[5] = ((b[1] + b[3]) >> 1) - lpConfig->view_y;
            if (ya != b[5]) {
                pt.x = tile->field_4 + ride->footprint.x0;
                pt.y = tile->field_5 + ride->footprint.y0;
                GetTileBounds(&pt, b);
                iVar7 = (b[0] + b[2]) >> 1;
                pt.x = tile->field_4 + ride->footprint.x1;
                pt.y = tile->field_5 + ride->footprint.y1;
                GetTileBounds(&pt, b);
                iVar7 = iVar7 - iVar1;
                iVar6 = ((b[0] + b[2]) >> 1) - iVar1;
                if (iVar6 < iVar7) {
                    iVar2 = iVar6 * 2;
                    step = -(iVar2 / 4);
                } else {
                    iVar2 = iVar7 * 2;
                    step = iVar2 / 4;
                }
                iVar3 = ya;
                ObjectPartKey[ObjectPartCount] = iVar3;
                iVar3 += step;
                part = &ObjectPartArray[ObjectPartCount++];
                part->top = rect.top;
                part->bottom = rect.bottom;
                part->left = rect.left;
                iVar8 = iVar2 * 3 / 4;
                part->right = iVar1 + iVar8;
                if (part->right + iVar8 < b[4]) {
                    ObjectPartKey[ObjectPartCount] = iVar3;
                    iVar3 += step;
                    part = &ObjectPartArray[ObjectPartCount++];
                    part->top = rect.top;
                    part->left = iVar1 + iVar8;
                    part->bottom = rect.bottom;
                    part->right = iVar2 * 5 / 4 + iVar1;
                }
                if (part->right + iVar8 < b[4]) {
                    iVar2 = iVar2 / 2;
                    do {
                        prev = part;
                        ObjectPartKey[ObjectPartCount] = iVar3;
                        iVar3 += step;
                        part = &ObjectPartArray[ObjectPartCount++];
                        part->top = rect.top;
                        part->bottom = rect.bottom;
                        part->left = prev->left + iVar2;
                        part->right = prev->right + iVar2;
                    } while (part->right + iVar8 < b[4]);
                }
                ObjectPartKey[ObjectPartCount] = b[5];
                prev = part;
                part = &ObjectPartArray[ObjectPartCount++];
                part->top = rect.top;
                part->bottom = rect.bottom;
                part->left = prev->right;
                part->right = rect.right;
            }
            if (ride->flags & 0x400) {
                if (ride->cb_sprite == NULL) {
                    continue;
                }
                info = ride->cb_sprite(ride->element, uid);
                if (info == NULL) {
                    continue;
                }
            } else {
                loc.info.sprite = ride->layer;
                loc.info.x = ride->field_14;
                loc.info.y = ride->field_18;
                loc.info.field_10 = 0;
                info = &loc.info;
            }
            pt.x = tile->field_4;
            pt.y = tile->field_5;
            GetTileBounds(&pt, bounds);
            if (tile->flags & 0x20) {
                iVar1 = info->field_10;
                hit_a.type = 0x104;
                hit_a.element = tile->field_0;
                hit_a.coords = tile->anchor.id;
                if (ride->anim != NULL) {
                    iVar4 = HALF(ride->anim_dx + info->x);
                    iVar5 = HALF(ride->anim_dy + info->y);
                    iVar4 = bounds[0] + iVar4;
                    iVar5 = bounds[1] + iVar5;
                    info->sprite = ride->anim;
                    SetOverrideFrame(GetBuildAnimFrame(ride, uid));
                } else {
                    iVar4 = HALF(info->x);
                    iVar5 = HALF(info->y);
                    iVar4 = iVar4 + bounds[0];
                    iVar5 = iVar5 + bounds[1];
                    iVar1 = 0xff00;
                }
                if (ObjectPartCount != 0) {
                    for (k = 0; k < ObjectPartCount; k++) {
                        SortClippedSprite(info->sprite, iVar4, iVar5, ObjectPartKey[k], &ObjectPartArray[k], iVar1, &hit_a);
                    }
                } else {
                    SortSprite(info->sprite, iVar4, iVar5, ya, iVar1, &hit_a);
                }
            } else {
                iVar1 = info->field_10;
                hit_b.type = 0x103;
                hit_b.element = tile->field_0;
                hit_b.coords = tile->anchor.id;
                iVar4 = HALF(info->x);
                iVar5 = HALF(info->y);
                iVar4 = bounds[0] + iVar4;
                iVar5 = bounds[1] + iVar5;
                if ((tile->flags & 4) == 0) {
                    if ((tile->flags & 0x200) && GetBlink()) {
                        iVar1 = 0xff0000;
                    } else if (MapStats.field_18c != 0 && (tile->flags & 0x100) && !GetBlink()) {
                        iVar1 = 0xffff;
                    }
                }
                if (ObjectPartCount != 0) {
                    for (k = 0; k < ObjectPartCount; k++) {
                        SortClippedSprite(info->sprite, iVar4, iVar5, ObjectPartKey[k], &ObjectPartArray[k], iVar1, &hit_b);
                    }
                } else {
                    SortSprite(info->sprite, iVar4, iVar5, ya, iVar1, &hit_b);
                }
            }
            ClearOverrideFrame();
        }
    }
    /* People and workers are sorted in with the objects. */
    RenderPeople();
    RenderWorkers();
    DrawAndClearPrintList();
    for (i = 0; i < count; i++) {
        tile = list[i];
        if (tile->field_0 != NULL && (tile->flags & 0x20)) {
            uid.id = tile->anchor.id;
            DoBuildEffects(tile->field_0->ride, uid);
        }
    }
    if (DAT_00667d40 != 0) {
        PushRenderingStatusAndLockVideoSurface();
        FUN_00461220();
        FUN_00461020();
        PopRenderingStatus();
    }
    RenderWorkerInterfaceGFX();
    SetClipping(&clip);
}

// FUNCTION: LEGOLAND 0x0045bcd0
LEGO_EXPORT void PointToIsoPlane(int *param_1, int *out) {
    int iVar1;
    int iVar2;
    int iVar3;
    int iVar4;
    int iVar5;
    int iVar6;
    char cVar7;
    int iVar8;
    int iVar9;
    int x;
    int y;
    short size;

    size = ((struct TileSprite *)TileSpriteArray[DAT_00667ca4])->size;
    iVar9 = (int)size;
    iVar8 = (short)(size * 2);
    iVar4 = iVar8 + 1 >> 1;
    x = *param_1;
    y = param_1[1];
    iVar1 = (x + iVar4) / iVar8;
    iVar5 = (x + iVar4) % iVar8;
    iVar2 = y / iVar9;
    iVar6 = y % iVar9;
    iVar3 = iVar2 + iVar1;
    *out = iVar3;
    iVar2 = iVar2 - iVar1;
    out[1] = iVar2;
    if (iVar5 < 0) {
        iVar5 = iVar5 + -2 + iVar8;
        out[1] = iVar2 + 1;
        *out = iVar3 + -1;
    }
    if (iVar6 < 0) {
        iVar6 = iVar6 + -1 + iVar9;
        out[1] = out[1] + -1;
        *out = *out + -1;
    }
    cVar7 = (iVar4 <= iVar5) + '\x01';
    if (iVar9 + 1 >> 1 < iVar6) {
        cVar7 = (iVar4 <= iVar5) + '\x03';
    }
    switch (cVar7) {
    case '\x01':
        if (iVar5 < iVar4 + iVar6 * -2) {
            *out = *out + -1;
            return;
        }
        break;
    case '\x02':
        if (iVar4 + iVar6 * 2 <= iVar5) {
            out[1] = out[1] + -1;
            return;
        }
        break;
    case '\x03':
        if (iVar5 < iVar4 + (iVar6 - iVar9) * 2) {
            out[1] = out[1] + 1;
            return;
        }
        break;
    case '\x04':
        if (iVar4 + (iVar9 - iVar6) * 2 <= iVar5) {
            *out = *out + 1;
        }
    }
}

// FUNCTION: LEGOLAND 0x0045be00
LEGO_EXPORT unsigned int ScreenToMapRef2(struct Point *screen, struct Point *out) {
    struct TileSprite *sprite;
    short size;
    short twice;
    int ix;
    int iy;

    sprite = (struct TileSprite *)TileSpriteArray[DAT_00667ca4];
    if (sprite == NULL) {
        return 0xffffffff;
    }
    size = sprite->size;
    twice = size * 2;
    ix = ((twice + 1 >> 1) - lpConfig->view_x);
    ix += ScrollX >> 8;
    ix = (ix + screen->x) * (0x100 / twice);
    iy = (((ScrollY >> 8) - lpConfig->view_y) + screen->y) * (0x100 / (int)size);
    out->x = iy + ix;
    out->y = iy - ix;
    return 0;
}

// FUNCTION: LEGOLAND 0x0045be90
LEGO_EXPORT unsigned int ScreenToMapRef(int *param_1, int *out, unsigned int param_3) {
    struct TileSprite *sprite;
    short size;
    short twice;
    int w;
    int hy;
    int half;
    int sx;
    int sy;
    int qx;
    int rx;
    int qy;
    int ry;
    int sel;

    sprite = (struct TileSprite *)TileSpriteArray[DAT_00667ca4];
    if (sprite == NULL) {
        return 0xffffffff;
    }
    do {
        size = sprite->size;
        w = (int)size;
        twice = size * 2;
        hy = w + 1 >> 1;
        half = twice + 1 >> 1;
        sx = ((ScrollX >> 8) - lpConfig->view_x) + *param_1 + half;
        sy = ((ScrollY >> 8) - lpConfig->view_y) + param_1[1];
        qx = sx / twice;
        rx = sx % twice;
        qy = sy / w;
        ry = sy % w;
        *out = qx + qy;
        out[1] = qy - qx;
        if (rx < 0) {
            rx = rx + -2 + twice;
            out[1] = (qy - qx) + 1;
            *out = (qx + qy) + -1;
        }
        if (ry < 0) {
            out[1] = out[1] + -1;
            *out = *out + -1;
            ry = ry + -1 + w;
        }
        sel = (rx >= half) + 1;
        if (ry > hy) {
            sel += 2;
        }
        switch (sel) {
        case 1:
            if (rx < half + ry * -2) {
                *out = *out + -1;
            }
            break;
        case 2:
            if (rx >= half + ry * 2) {
                out[1] = out[1] + -1;
            }
            break;
        case 3:
            if (rx < half + (ry - w) * 2) {
                out[1] = out[1] + 1;
            }
            break;
        case 4:
            if (rx >= half + (w - ry) * 2) {
                *out = *out + 1;
            }
        }
    } while (0);
    return 1;
}

// FUNCTION: LEGOLAND 0x0045c010
LEGO_EXPORT unsigned char Dir_To_Bit(unsigned char param) {
    return DAT_004b9550[param & 7];
}

// FUNCTION: LEGOLAND 0x0045c020
LEGO_EXPORT unsigned char Bit_To_Dir(unsigned char bit) {
    unsigned char result;

    for (result = 0; result < 8; result++) {
        if (DAT_004b9550[result] & bit) {
            return result;
        }
    }
    return 8;
}

// FUNCTION: LEGOLAND 0x0045c050
LEGO_EXPORT unsigned char Get_Path_Directions(struct Point *pos, char *param_2, char *param_3) {
    int x;
    int y;
    int yp1;
    int xp1;
    int xm1;
    char local_15;
    char local_16;
    unsigned char local_17;
    struct MapTile tile;

    y = pos->y;
    x = pos->x;
    pos = (struct Point *)(y + -1);
    local_15 = '\0';
    local_16 = '\0';
    local_17 = 0;
    if (x < 0 || x >= (int)lpConfig->width || (int)pos < 0 || (int)pos >= (int)lpConfig->height) {
        tile.flags_c = 0x40;
        tile.flags_10 = 0;
    } else {
        tile = *(struct MapTile *)((char *)GameMap[(int)pos] + x * 0x14);
    }
    if ((tile.flags_10 & 1) != 0 || ((tile.flags_c & 0x10) != 0 && (tile.flags_10 & 2) == 0)) {
        local_16 = '\x01';
        local_17 = 1;
    }
    yp1 = y + 1;
    if (x < 0 || x >= (int)lpConfig->width || yp1 < 0 || yp1 >= (int)lpConfig->height) {
        tile.flags_c = 0x40;
        tile.flags_10 = 0;
    } else {
        tile = *(struct MapTile *)((char *)GameMap[yp1] + x * 0x14);
    }
    if ((tile.flags_10 & 1) != 0 || ((tile.flags_c & 0x10) != 0 && (tile.flags_10 & 2) == 0)) {
        local_16 = local_16 + '\x01';
        local_17 = local_17 | 0x10;
    }
    xp1 = x + 1;
    if (xp1 < 0 || xp1 >= (int)lpConfig->width || y < 0 || y >= (int)lpConfig->height) {
        tile.flags_c = 0x40;
        tile.flags_10 = 0;
    } else {
        tile = *(struct MapTile *)((char *)GameMap[y] + xp1 * 0x14);
    }
    if ((tile.flags_10 & 1) != 0 || ((tile.flags_c & 0x10) != 0 && (tile.flags_10 & 2) == 0)) {
        local_16 = local_16 + '\x01';
        local_17 = local_17 | 4;
    }
    xm1 = x + -1;
    if (xm1 < 0 || xm1 >= (int)lpConfig->width || y < 0 || y >= (int)lpConfig->height) {
        tile.flags_c = 0x40;
        tile.flags_10 = 0;
    } else {
        tile = *(struct MapTile *)((char *)GameMap[y] + xm1 * 0x14);
    }
    if ((tile.flags_10 & 1) != 0 || ((tile.flags_c & 0x10) != 0 && (tile.flags_10 & 2) == 0)) {
        local_16 = local_16 + '\x01';
        local_17 = local_17 | 0x40;
    }
    if (xp1 < 0 || xp1 >= (int)lpConfig->width || (int)pos < 0 || (int)pos >= (int)lpConfig->height) {
        tile.flags_c = 0x40;
        tile.flags_10 = 0;
    } else {
        tile = *(struct MapTile *)((char *)GameMap[(int)pos] + xp1 * 0x14);
    }
    if ((tile.flags_10 & 1) != 0 || ((tile.flags_c & 0x10) != 0 && (tile.flags_10 & 2) == 0)) {
        local_15 = '\x01';
        local_17 = local_17 | 2;
    }
    if (xp1 < 0 || xp1 >= (int)lpConfig->width || yp1 < 0 || yp1 >= (int)lpConfig->height) {
        tile.flags_c = 0x40;
        tile.flags_10 = 0;
    } else {
        tile = *(struct MapTile *)((char *)GameMap[yp1] + xp1 * 0x14);
    }
    if ((tile.flags_10 & 1) != 0 || ((tile.flags_c & 0x10) != 0 && (tile.flags_10 & 2) == 0)) {
        local_15 = local_15 + '\x01';
        local_17 = local_17 | 8;
    }
    if (xm1 < 0 || xm1 >= (int)lpConfig->width || yp1 < 0 || yp1 >= (int)lpConfig->height) {
        tile.flags_c = 0x40;
        tile.flags_10 = 0;
    } else {
        tile = *(struct MapTile *)((char *)GameMap[yp1] + xm1 * 0x14);
    }
    if ((tile.flags_10 & 1) != 0 || ((tile.flags_c & 0x10) != 0 && (tile.flags_10 & 2) == 0)) {
        local_15 = local_15 + '\x01';
        local_17 = local_17 | 0x20;
    }
    if (xm1 < 0 || xm1 >= (int)lpConfig->width || (int)pos < 0 || (int)pos >= (int)lpConfig->height) {
        tile.flags_c = 0x40;
        tile.flags_10 = 0;
    } else {
        tile = *(struct MapTile *)((char *)GameMap[(int)pos] + xm1 * 0x14);
    }
    if ((tile.flags_10 & 1) != 0 || ((tile.flags_c & 0x10) != 0 && (tile.flags_10 & 2) == 0)) {
        local_15 = local_15 + '\x01';
        local_17 = local_17 | 0x80;
    }
    if (param_2 != NULL) {
        *param_2 = local_16;
    }
    if (param_3 != NULL) {
        *param_3 = local_15;
    }
    return local_17;
}

// FUNCTION: LEGOLAND 0x0045c440
unsigned char FUN_0045c440(int *param_1, char *param_2, char *param_3) {
    int x;
    int y;
    int yp1;
    int xp1;
    int xm1;
    char local_15;
    char local_16;
    unsigned char local_17;
    struct MapTile tile;

    y = param_1[1];
    x = *param_1;
    param_1 = (int *)(y + -1);
    local_15 = '\0';
    local_16 = '\0';
    local_17 = 0;
    if (x < 0 || x >= (int)lpConfig->width || (int)param_1 < 0 || (int)param_1 >= (int)lpConfig->height) {
        tile.flags_c = 0x40;
        tile.flags_10 = 0;
    } else {
        tile = *(struct MapTile *)((char *)GameMap[(int)param_1] + x * 0x14);
    }
    if ((tile.flags_10 & 1) != 0 || ((tile.flags_c & 0x10) != 0 && (tile.flags_10 & 2) == 0)) {
        local_16 = '\x01';
        local_17 = 1;
    }
    yp1 = y + 1;
    if (x < 0 || x >= (int)lpConfig->width || yp1 < 0 || yp1 >= (int)lpConfig->height) {
        tile.flags_c = 0x40;
        tile.flags_10 = 0;
    } else {
        tile = *(struct MapTile *)((char *)GameMap[yp1] + x * 0x14);
    }
    if ((tile.flags_10 & 1) != 0 || ((tile.flags_c & 0x10) != 0 && (tile.flags_10 & 2) == 0)) {
        local_16 = local_16 + '\x01';
        local_17 = local_17 | 0x10;
    }
    xp1 = x + 1;
    if (xp1 < 0 || xp1 >= (int)lpConfig->width || y < 0 || y >= (int)lpConfig->height) {
        tile.flags_c = 0x40;
        tile.flags_10 = 0;
    } else {
        tile = *(struct MapTile *)((char *)GameMap[y] + xp1 * 0x14);
    }
    if ((tile.flags_10 & 1) != 0 || ((tile.flags_c & 0x10) != 0 && (tile.flags_10 & 2) == 0)) {
        local_16 = local_16 + '\x01';
        local_17 = local_17 | 4;
    }
    xm1 = x + -1;
    if (xm1 < 0 || xm1 >= (int)lpConfig->width || y < 0 || y >= (int)lpConfig->height) {
        tile.flags_c = 0x40;
        tile.flags_10 = 0;
    } else {
        tile = *(struct MapTile *)((char *)GameMap[y] + xm1 * 0x14);
    }
    if ((tile.flags_10 & 1) != 0 || ((tile.flags_c & 0x10) != 0 && (tile.flags_10 & 2) == 0)) {
        local_16 = local_16 + '\x01';
        local_17 = local_17 | 0x40;
    }
    if (xp1 < 0 || xp1 >= (int)lpConfig->width || (int)param_1 < 0 || (int)param_1 >= (int)lpConfig->height) {
        tile.flags_c = 0x40;
        tile.flags_10 = 0;
    } else {
        tile = *(struct MapTile *)((char *)GameMap[(int)param_1] + xp1 * 0x14);
    }
    if ((tile.flags_10 & 1) != 0 || ((tile.flags_c & 0x10) != 0 && (tile.flags_10 & 2) == 0)) {
        local_15 = '\x01';
        local_17 = local_17 | 2;
    }
    if (xp1 < 0 || xp1 >= (int)lpConfig->width || yp1 < 0 || yp1 >= (int)lpConfig->height) {
        tile.flags_c = 0x40;
        tile.flags_10 = 0;
    } else {
        tile = *(struct MapTile *)((char *)GameMap[yp1] + xp1 * 0x14);
    }
    if ((tile.flags_10 & 1) != 0 || ((tile.flags_c & 0x10) != 0 && (tile.flags_10 & 2) == 0)) {
        local_15 = local_15 + '\x01';
        local_17 = local_17 | 8;
    }
    if (xm1 < 0 || xm1 >= (int)lpConfig->width || yp1 < 0 || yp1 >= (int)lpConfig->height) {
        tile.flags_c = 0x40;
        tile.flags_10 = 0;
    } else {
        tile = *(struct MapTile *)((char *)GameMap[yp1] + xm1 * 0x14);
    }
    if ((tile.flags_10 & 1) != 0 || ((tile.flags_c & 0x10) != 0 && (tile.flags_10 & 2) == 0)) {
        local_15 = local_15 + '\x01';
        local_17 = local_17 | 0x20;
    }
    if (xm1 < 0 || xm1 >= (int)lpConfig->width || (int)param_1 < 0 || (int)param_1 >= (int)lpConfig->height) {
        tile.flags_c = 0x40;
        tile.flags_10 = 0;
    } else {
        tile = *(struct MapTile *)((char *)GameMap[(int)param_1] + xm1 * 0x14);
    }
    if ((tile.flags_10 & 1) != 0 || ((tile.flags_c & 0x10) != 0 && (tile.flags_10 & 2) == 0)) {
        local_15 = local_15 + '\x01';
        local_17 = local_17 | 0x80;
    }
    if (param_2 != NULL) {
        *param_2 = local_16;
    }
    if (param_3 != NULL) {
        *param_3 = local_15;
    }
    return local_17;
}

// FUNCTION: LEGOLAND 0x0045c830
LEGO_EXPORT unsigned char ExcludeIsolatedDiags(unsigned char param) {
    unsigned char result = param;

    if ((result & 0x5) != 0x5) {
        result &= 0xfd;
    }
    if ((result & 0x41) != 0x41) {
        result &= 0x7f;
    }
    if ((result & 0x14) != 0x14) {
        result &= 0xf7;
    }
    if ((result & 0x50) != 0x50) {
        result &= 0xdf;
    }
    return result;
}

// FUNCTION: LEGOLAND 0x0045c870
LEGO_EXPORT void AdjustTileRFFlags(int *param_1) {
    struct MapTile *tile;
    unsigned char flags;
    char local_1;
    char dir;

    tile = (struct MapTile *)((char *)GameMap[param_1[1]] + *param_1 * 0x14);
    tile->flags_10 = tile->flags_10 & 0xc3;
    flags = FUN_0045c440(param_1, &dir, &local_1);
    if (dir == '\0') {
        tile->flags_10 = tile->flags_10 | 0x10;
        return;
    }
    if (dir == '\x01') {
        tile->flags_10 = tile->flags_10 | 0x10;
        return;
    }
    if (dir == '\x02') {
        switch (flags & 0x11) {
        case 1:
        case 0x10:
            tile->flags_10 = tile->flags_10 | 8;
            return;
        }
    } else {
        if (dir == '\x03') {
            tile->flags_10 = tile->flags_10 | 4;
            return;
        }
        if (dir == '\x04') {
            tile->flags_10 = tile->flags_10 | 0x20;
        }
    }
}

// FUNCTION: LEGOLAND 0x0045c900
int FUN_0045c900(struct MapRect *param_1) {
    int x;
    unsigned int y;
    struct MapTile tile;

    x = param_1->x0;
    while (x <= param_1->x1) {
        for (y = param_1->y0; y <= param_1->y1; y++) {
            if (x >= 0 && x < (int)lpConfig->width && y >= 0 && y < (int)lpConfig->height) {
                tile = *(struct MapTile *)&GameMap[y][x];
            } else {
                tile.flags_c = 0x40;
                tile.flags_10 = 0;
            }
            if ((tile.flags_c & 0x10) == 0 || (tile.flags_10 & 2) != 0) {
                return 0;
            }
        }
        x++;
    }
    return 1;
}

// FUNCTION: LEGOLAND 0x0045c9c0
unsigned int FUN_0045c9c0(int *param_1) {
    int y;
    int yend;
    unsigned int result;
    unsigned int bit;
    int x;
    int xoff;
    int xi;
    struct MapTile tile;

    y = param_1[1];
    result = 0;
    bit = 0x1000000;
    yend = y + 5;
    for (; y < yend; y++) {
        x = *param_1;
        xoff = x * 0x14;
        for (xi = x; xi < x + 5; xi++) {
            if (xoff < 0 || xi >= (int)lpConfig->width || y < 0 || y >= (int)lpConfig->height) {
                tile.flags_c = 0x40;
                tile.flags_10 = 0;
            } else {
                tile = *(struct MapTile *)((char *)GameMap[y] + xoff);
            }
            if ((tile.flags_c & 0x10) != 0 && (tile.flags_10 & 2) == 0) {
                result |= bit;
            }
            xoff += 0x14;
            bit >>= 1;
        }
    }
    return result;
}

// FUNCTION: LEGOLAND 0x0045ca90
int FUN_0045ca90(int *param_1, int *param_2) {
    int dx;
    int x;
    unsigned int mask;
    unsigned int *tbl;
    unsigned int v;
    int i;
    struct Point pt;

    pt.x = *param_1 + -2;
    pt.y = param_1[1] + -2;
    mask = FUN_0045c9c0((int *)&pt);
    i = 0;
    tbl = DAT_004b9558;
    v = *tbl;
    do {
        if ((v & mask) == v) {
            dx = DAT_004b957c[i];
            x = *param_1;
            param_2[0] = dx + x;
            i = DAT_004b95a0[i] + param_1[1];
            param_2[1] = i;
            param_2[2] = dx + x + 2;
            param_2[3] = i + 2;
            return 1;
        }
        tbl = tbl + 1;
        i = i + 1;
    } while ((int)tbl < (int)&DAT_004b957c);
    return 0;
}

// FUNCTION: LEGOLAND 0x0045cb20
void FUN_0045cb20(struct MapRect *param_1) {
    int x;
    int y;

    for (y = param_1->y0; y <= param_1->y1; y++) {
        for (x = param_1->x0; x <= param_1->x1; x++) {
            GameMap[y][x].field_8 = ((((x + y) & 1) ? 1 : 2) & 0xff) + *(unsigned short *)PathSprite;
        }
    }
}

// FUNCTION: LEGOLAND 0x0045cb90
void FUN_0045cb90(struct Point *param) {
    struct MapTile *tile;
    unsigned short value;

    tile = (struct MapTile *)GameMap[param->y];
    value = *(unsigned short *)PathSprite;
    *(unsigned short *)((unsigned char *)tile + param->x * 0x14 + 8) = value;
}

// FUNCTION: LEGOLAND 0x0045cbc0
int FUN_0045cbc0(int *param_1, int param_2) {
    int result;
    struct MapRect r;

    switch (param_2) {
    case 2:
        r.y0 = r.y1 = param_1[1] + -1;
        r.x0 = *param_1;
        r.x1 = param_1[2];
        result = FUN_0045c900(&r);
        if (result != 0) {
            param_1[1] = param_1[1] + -1;
            return 1;
        }
        break;
    case 0:
        r.y0 = r.y1 = param_1[3] + 1;
        r.x0 = *param_1;
        r.x1 = param_1[2];
        result = FUN_0045c900(&r);
        if (result != 0) {
            param_1[3] = param_1[3] + 1;
            return 1;
        }
        break;
    case 1:
        r.y0 = param_1[1];
        r.y1 = param_1[3];
        r.x1 = param_1[2] + 1;
        r.x0 = r.x1;
        result = FUN_0045c900(&r);
        if (result != 0) {
            param_1[2] = param_1[2] + 1;
            return 1;
        }
        break;
    case 3:
        r.y0 = param_1[1];
        r.y1 = param_1[3];
        r.x1 = *param_1 + -1;
        r.x0 = r.x1;
        result = FUN_0045c900(&r);
        if (result != 0) {
            *param_1 = *param_1 + -1;
            return 1;
        }
    }
    return 0;
}

// FUNCTION: LEGOLAND 0x0045cd00
void FUN_0045cd00(int *buf) {
    int zero_count = 0;
    int count = 0;

    do {
        if (FUN_0045cbc0(buf, count) != 0) {
            zero_count = 0;
        } else {
            zero_count++;
        }
        count++;
        count = count & 3;
    } while (zero_count < 4);
}

// FUNCTION: LEGOLAND 0x0045cd30
void FUN_0045cd30(int *arg) {
    struct MapRect buf;

    if (FUN_0045ca90(arg, (int *)&buf) != 0) {
        FUN_0045cd00((int *)&buf);
        FUN_0045cb20(&buf);
    }
}

// FUNCTION: LEGOLAND 0x0045cd70
void FUN_0045cd70(int *param_1) {
    struct Point pt;
    struct MapRect rect;

    pt.x = *param_1 - 2;
    while (pt.x <= *param_1 + 2) {
        pt.y = param_1[1] - 2;
        while (pt.y <= param_1[1] + 2) {
            if ((pt.x != *param_1 || pt.y != param_1[1]) && FUN_0045ce30((int *)&pt) != 0 &&
                FUN_0045ca90((int *)&pt, (int *)&rect) == 0) {
                FUN_0045cb90(&pt);
            }
            pt.y++;
        }
        pt.x++;
    }
}

// FUNCTION: LEGOLAND 0x0045ce10
int FUN_0045ce10(struct MapTile *tile) {
    if ((tile->flags_10 & 0x1) == 0) {
        if ((tile->flags_c & 0x10) != 0 && (tile->flags_10 & 0x2) == 0) {
            return 1;
        }
        return 0;
    }
    return 1;
}

// FUNCTION: LEGOLAND 0x0045ce30
int FUN_0045ce30(int *param_1) {
    int x;
    int y;
    struct MapTile *tile;

    x = *param_1;
    if (x >= 0 && x < (int)lpConfig->width && (y = param_1[1], y >= 0) &&
        y < (int)lpConfig->height &&
        (tile = (struct MapTile *)((char *)GameMap[y] + x * 0x14), tile != NULL)) {
        if (FUN_0045ce10(tile) != 0) {
            tile = (struct MapTile *)((char *)GameMap[param_1[1]] + *param_1 * 0x14);
            return tile->tile != *(unsigned int *)PathSprite;
        }
    }
    return 0;
}

// FUNCTION: LEGOLAND 0x0045ceb0
unsigned char FUN_0045ceb0(int *coords) {
    int x;
    int y;
    unsigned char local_15;
    struct MapTile tile;

    local_15 = 0;
    x = *coords;
    y = coords[1] + -1;
    if (x < 0 || x >= (int)lpConfig->width || y < 0 || y >= (int)lpConfig->height) {
        tile.tile = 0;
        tile.flags_c = 0x40;
        tile.flags_10 = 0;
    } else {
        tile = *(struct MapTile *)((char *)GameMap[y] + x * 0x14);
    }
    if (FUN_0045ce10(&tile) != 0) {
        local_15 = 1;
    }
    y = coords[1];
    x = *coords + 1;
    if (x < 0 || x >= (int)lpConfig->width || y < 0 || y >= (int)lpConfig->height) {
        tile.tile = 0;
        tile.flags_c = 0x40;
        tile.flags_10 = 0;
    } else {
        tile = *(struct MapTile *)((char *)GameMap[y] + x * 0x14);
    }
    if (FUN_0045ce10(&tile) != 0) {
        local_15 = local_15 | 2;
    }
    x = *coords;
    y = coords[1] + 1;
    if (x < 0 || x >= (int)lpConfig->width || y < 0 || y >= (int)lpConfig->height) {
        tile.tile = 0;
        tile.flags_c = 0x40;
        tile.flags_10 = 0;
    } else {
        tile = *(struct MapTile *)((char *)GameMap[y] + x * 0x14);
    }
    if (FUN_0045ce10(&tile) != 0) {
        local_15 = local_15 | 4;
    }
    x = *coords + -1;
    y = coords[1];
    if (x < 0 || x >= (int)lpConfig->width || y < 0 || y >= (int)lpConfig->height) {
        tile.tile = 0;
        tile.flags_c = 0x40;
        tile.flags_10 = 0;
    } else {
        tile = *(struct MapTile *)((char *)GameMap[y] + x * 0x14);
    }
    if (FUN_0045ce10(&tile) != 0) {
        local_15 = local_15 | 8;
    }
    return local_15;
}

// FUNCTION: LEGOLAND 0x0045d080
unsigned char FUN_0045d080(unsigned char flags, int *coords) {
    unsigned char result = 0;
    unsigned int tile_data[5];
    unsigned int *map_row;
    int x, y;

    if ((flags & 0xc) == 0xc) {
        x = coords[0] - 1;
        y = coords[1] + 1;
        if (x >= 0 && x < lpConfig->width && y >= 0 && y < lpConfig->height) {
            map_row = (unsigned int *)GameMap[y];
            memcpy(tile_data, map_row + x * 5, sizeof(tile_data));
        } else {
            *(unsigned short *)((unsigned char *)tile_data + 0x8) = 0;
            *(unsigned short *)((unsigned char *)tile_data + 0xc) = 0x40;
            *(unsigned char *)((unsigned char *)tile_data + 0x10) = 0;
        }
        if (!FUN_0045ce10((struct MapTile *)tile_data)) {
            result = 1;
        }
    }
    if ((flags & 0x3) == 0x3) {
        x = coords[0] + 1;
        y = coords[1] - 1;
        if (x >= 0 && x < lpConfig->width && y >= 0 && y < lpConfig->height) {
            map_row = (unsigned int *)GameMap[y];
            memcpy(tile_data, map_row + x * 5, sizeof(tile_data));
        } else {
            *(unsigned short *)((unsigned char *)tile_data + 0x8) = 0;
            *(unsigned short *)((unsigned char *)tile_data + 0xc) = 0x40;
            *(unsigned char *)((unsigned char *)tile_data + 0x10) = 0;
        }
        if (!FUN_0045ce10((struct MapTile *)tile_data)) {
            result |= 2;
        }
    }
    return result;
}

// FUNCTION: LEGOLAND 0x0045d1a0
LEGO_EXPORT unsigned short *AdjustPathTile(struct Point *p, unsigned short a) {
    unsigned short *flags;
    int x;
    int y;
    struct MapTile tile;

    BGFullUpdate = 1;
    x = p->x;
    if (x < 0 || x >= (int)lpConfig->width || (y = p->y, y < 0) || y >= (int)lpConfig->height) {
        tile.tile = 0;
        tile.flags_c = 0x40;
        tile.flags_10 = 0;
    } else {
        tile = *(struct MapTile *)((char *)GameMap[y] + x * 0x14);
    }
    if ((tile.flags_10 & 1) != 0 || ((tile.flags_c & 0x10) != 0 && (tile.flags_10 & 2) == 0)) {
        AdjustTileRFFlags((int *)p);
    }
    flags = (unsigned short *)FUN_0045ce10(&tile);
    if (flags != NULL) {
        flags = (unsigned short *)((char *)GameMap[p->y] + 0xc + p->x * 0x14);
        *flags = *flags & 0xfffc;
    }
    return flags;
}

// FUNCTION: LEGOLAND 0x0045d260
void FUN_0045d260(struct Point *param) {
    unsigned int edi = *(unsigned int *)PathSprite;
    struct Point point;

    AdjustPathTile(param, edi);

    point.x = param->x;
    point.y = param->y - 1;
    AdjustPathTile(&point, edi);

    point.x = param->x + 1;
    point.y = param->y;
    AdjustPathTile(&point, edi);

    point.x = param->x;
    point.y = param->y + 1;
    AdjustPathTile(&point, edi);

    point.x = param->x - 1;
    point.y = param->y;
    AdjustPathTile(&point, edi);

    point.x = param->x - 1;
    point.y = param->y + 1;
    AdjustPathTile(&point, edi);

    point.x = param->x + 1;
    point.y = param->y - 1;
    AdjustPathTile(&point, edi);

    point.x = param->x + 1;
    point.y = param->y + 1;
    AdjustPathTile(&point, edi);

    point.x = param->x - 1;
    point.y = param->y - 1;
    AdjustPathTile(&point, edi);
}

// FUNCTION: LEGOLAND 0x0045d350
LEGO_EXPORT void AddPathTileGFX(struct Point *p, unsigned short param1) {
    unsigned short *pb;

    pb = (unsigned short *)((char *)GameMap[p->y] + 0xc + p->x * 0x14);
    *pb |= 0x10;
    *(unsigned short *)((char *)GameMap[p->y] + 8 + p->x * 0x14) = param1;
    FUN_0045d260(p);
    if (MapStats.field_184 != 0) {
        FUN_0045cd30((int *)p);
    }
}

// FUNCTION: LEGOLAND 0x0045d3b0
LEGO_EXPORT void AddPathTile(struct Point *p, unsigned short param1) {
    AddPathTileGFX(p, param1);
    /* struct Point and struct Point are identical {x,y} layouts; cast
       bridges tilemap's local Point to pathfind's InstancePos (no shared type). */
    AddPathSquare((struct Point *)p);
}

struct PathFootprint {
    /* 0x00 */ unsigned char pad_0[0x20];
    /* 0x20 */ short type;
    /* 0x22 */ unsigned char pad_22[0x3c - 0x22];
    /* 0x3c */ int x_lo;
    /* 0x40 */ int y_lo;
    /* 0x44 */ int x_hi;
    /* 0x48 */ int y_hi;
};

// FUNCTION: LEGOLAND 0x0045d3d0
void FUN_0045d3d0(struct PathFootprint *param_1, int *param_2) {
    int x;
    int y;
    struct Point pt;

    if (param_1->type != 0 && param_1->type != 2) {
        for (y = param_2[1] + param_1->y_lo - 1; y <= param_2[1] + 1 + param_1->y_hi; y++) {
            for (x = *param_2 + param_1->x_lo - 1; x <= *param_2 + param_1->x_hi + 1; x++) {
                GameMap[y][x].flags &= 0xffe7;
                GameMap[y][x].field_10 = 0;
                GameMap[y][x].field_8 = GameMap[y][x].field_a;
                pt.x = x;
                pt.y = y;
                FUN_0045d260(&pt);
                RemovePathSquare(&pt);
            }
        }
        for (y = param_2[1] + param_1->y_lo; y <= param_2[1] + param_1->y_hi; y++) {
            for (x = *param_2 + param_1->x_lo; x <= *param_2 + param_1->x_hi; x++) {
                GameMap[y][x].flags &= 0xffe7;
                GameMap[y][x].field_10 = 0;
                GameMap[y][x].field_8 = GameMap[y][x].field_a;
                pt.x = x;
                pt.y = y;
                FUN_0045d260(&pt);
                RemovePathSquare(&pt);
            }
        }
    }
}

// FUNCTION: LEGOLAND 0x0045d560
int FUN_0045d560(struct MapRect *out, struct MapRect *a, struct MapRect *b) {
    if (a->x0 > b->x0) {
        out->x0 = a->x0;
    } else {
        out->x0 = b->x0;
    }
    if (a->x1 < b->x1) {
        out->x1 = a->x1;
    } else {
        out->x1 = b->x1;
    }
    if (a->y0 > b->y0) {
        out->y0 = a->y0;
    } else {
        out->y0 = b->y0;
    }
    if (a->y1 < b->y1) {
        out->y1 = a->y1;
    } else {
        out->y1 = b->y1;
    }
    if (out->x0 <= out->x1 && out->y0 <= out->y1) {
        return 1;
    }
    return 0;
}

// FUNCTION: LEGOLAND 0x0045d5d0
void FUN_0045d5d0(struct MapRect *param) {
    struct MapRect r;
    struct MapRect *cur;
    struct MapRect *slot;
    int i;
    struct MapRect old;

    i = 0;
    if (i < DAT_00667d3c) {
        cur = DAT_00801a80;
        do {
            if (FUN_0045d560(&r, param, cur) != 0) {
                old = *cur;
                DAT_00667d3c--;
                if (i < DAT_00667d3c) {
                    *cur = DAT_00801a80[DAT_00667d3c];
                }
                i--;
                cur--;
                if (old.y0 < r.y0) {
                    slot = &DAT_00801a80[DAT_00667d3c++];
                    slot->x0 = old.x0;
                    slot->y0 = old.y0;
                    slot->x1 = old.x1;
                    slot->y1 = r.y0 - 1;
                }
                if (old.x0 < r.x0) {
                    slot = &DAT_00801a80[DAT_00667d3c++];
                    slot->x0 = old.x0;
                    slot->y0 = r.y0;
                    slot->x1 = r.x0 - 1;
                    slot->y1 = r.y1;
                }
                if (r.x1 < old.x1) {
                    slot = &DAT_00801a80[DAT_00667d3c++];
                    slot->x0 = r.x1 + 1;
                    slot->y0 = r.y0;
                    slot->x1 = old.x1;
                    slot->y1 = r.y1;
                }
                if (r.y1 < old.y1) {
                    slot = &DAT_00801a80[DAT_00667d3c++];
                    slot->x0 = old.x0;
                    slot->y0 = r.y1 + 1;
                    slot->x1 = old.x1;
                    slot->y1 = old.y1;
                }
            }
            i++;
            cur++;
        } while (i < DAT_00667d3c);
    }
}

// FUNCTION: LEGOLAND 0x0045d730
int FUN_0045d730(struct MapRect *param) {
    int idx;
    struct MapRect *cur;

    FUN_0045d5d0(param);
    idx = DAT_00667d3c;
    cur = &DAT_00801a80[idx];
    cur->x0 = param->x0;
    cur->y0 = param->y0;
    cur->x1 = param->x1;
    idx = idx + 1;
    DAT_00667d3c = idx;
    cur->y1 = param->y1;
    return idx;
}

struct PathItem {
    /* 0x00 */ int x0;
    /* 0x04 */ int y0;
    /* 0x08 */ int x1;
    /* 0x0c */ int y1;
    /* 0x10 */ struct PathItem *next;
};

// FUNCTION: LEGOLAND 0x0045d770
void FUN_0045d770(struct Cursor *param_1) {
    unsigned char *pb;
    struct Cursor *cur;
    struct Cursor *chain;
    struct PathItem *item;
    int i;
    int x;
    int y;
    struct MapRect rect;
    struct Point local_18;

    cur = param_1;
    if (param_1 != NULL) {
        while ((cur->field_1828 & 0x1000) == 0) {
            cur = (struct Cursor *)cur->field_1830;
            if (NULL == cur) {
                return;
            }
        }
        if (cur != NULL) {
            DAT_00667d3c = 0;
            rect.x0 = cur->field_1414[0] + cur->tile_x;
            rect.y0 = cur->field_1414[1] + cur->tile_y;
            rect.x1 = cur->field_1414[2] + cur->tile_x;
            rect.y1 = cur->field_1414[3] + cur->tile_y;
            FUN_0045d730(&rect);
            do {
                chain = param_1;
                if ((param_1->field_1828 & 0x1000) != 0) {
                    break;
                }
                rect.x0 = param_1->tile_x + param_1->field_1414[0];
                rect.y0 = param_1->field_1414[1] + param_1->tile_y;
                rect.x1 = param_1->field_1414[2] + param_1->tile_x;
                rect.y1 = param_1->field_1414[3] + param_1->tile_y;
                FUN_0045d5d0(&rect);
                for (item = (struct PathItem *)&param_1->field_1414; item != NULL; item = item->next) {
                    rect.x0 = param_1->tile_x + item->x0;
                    rect.y0 = item->y0 + param_1->tile_y;
                    rect.x1 = item->x1 + param_1->tile_x;
                    rect.y1 = item->y1 + param_1->tile_y;
                    FUN_0045d5d0(&rect);
                }
                param_1 = (struct Cursor *)param_1->field_1830;
            } while (param_1 != NULL);
            if (0 < DAT_00667d3c) {
                i = 0;
                do {
                    local_18.y = DAT_00801a80[i].y0;
                    if (local_18.y <= DAT_00801a80[i].y1) {
                        do {
                            local_18.x = DAT_00801a80[i].x0;
                            if (local_18.x <= DAT_00801a80[i].x1) {
                                do {
                                    FUN_004779d0(&local_18);
                                    pb = (unsigned char *)((char *)GameMap[local_18.y] + 0x10 + local_18.x * 0x14);
                                    *pb = *pb & 0xfc;
                                    AddPathTileGFX(&local_18, *(unsigned short *)PathSprite);
                                    ScriptDirtyCategories = ScriptDirtyCategories | 0x10;
                                    AddPathSquare((struct Point *)&local_18);
                                    local_18.x = local_18.x + 1;
                                } while (local_18.x <= DAT_00801a80[i].x1);
                            }
                            local_18.y = local_18.y + 1;
                        } while (local_18.y <= (unsigned int)DAT_00801a80[i].y1);
                    }
                    i = i + 1;
                } while (i < DAT_00667d3c);
            }
            y = chain->field_1414[1] + 1 + chain->tile_y;
            PathUpdateNeeded = 1;
            if (y <= chain->tile_y + -1 + chain->field_1414[3]) {
                do {
                    x = chain->field_1414[0] + 1 + chain->tile_x;
                    if (x <= chain->field_1414[2] + -1 + chain->tile_x) {
                        do {
                            local_18.x = x;
                            local_18.y = y;
                            AddPathTileGFX(&local_18, *(unsigned short *)PathSprite);
                            x = x + 1;
                        } while (x <= chain->field_1414[2] + -1 + chain->tile_x);
                    }
                    y = y + 1;
                } while (y <= chain->tile_y + -1 + chain->field_1414[3]);
            }
            PathUpdateNeeded = 1;
        }
    }
}

// FUNCTION: LEGOLAND 0x0045da60
LEGO_EXPORT void RestoreBaseMap(int tile_x, int row_y) {
    struct MapTile *tile = (struct MapTile *)((char *)GameMap[row_y] + tile_x * 0x14);
    unsigned short id = tile->base_id;
    unsigned short flags = *(volatile unsigned short *)&TileSpriteInfo[id].sprite;

    tile->tile = id;
    (void)(flags & 0x20);
}

// FUNCTION: LEGOLAND 0x0045daa0
LEGO_EXPORT void RemovePathTile(int *param_1, unsigned short param_2) {
    unsigned short *flags;
    struct Point local_8;

    RestoreBaseMap(*param_1, param_1[1]);
    flags = (unsigned short *)((char *)GameMap[param_1[1]] + 0xc + *param_1 * 0x14);
    *flags = *flags & 0xffe4;
    *(unsigned char *)((char *)GameMap[param_1[1]] + 0x10 + *param_1 * 0x14) = 0;
    local_8.x = *param_1;
    local_8.y = param_1[1] + -1;
    AdjustPathTile(&local_8, param_2);
    local_8.x = *param_1 + 1;
    local_8.y = param_1[1];
    AdjustPathTile(&local_8, param_2);
    local_8.x = *param_1;
    local_8.y = param_1[1] + 1;
    AdjustPathTile(&local_8, param_2);
    local_8.x = *param_1 + -1;
    local_8.y = param_1[1];
    AdjustPathTile(&local_8, param_2);
    local_8.x = *param_1 + -1;
    local_8.y = param_1[1] + 1;
    AdjustPathTile(&local_8, param_2);
    local_8.x = *param_1 + 1;
    local_8.y = param_1[1] + -1;
    AdjustPathTile(&local_8, param_2);
    local_8.x = *param_1 + 1;
    local_8.y = param_1[1] + 1;
    AdjustPathTile(&local_8, param_2);
    local_8.x = *param_1 + -1;
    local_8.y = param_1[1] + -1;
    AdjustPathTile(&local_8, param_2);
    RemovePathSquare((struct Point *)param_1);
    if (MapStats.field_184 != 0) {
        FUN_0045cd70(param_1);
    }
}
