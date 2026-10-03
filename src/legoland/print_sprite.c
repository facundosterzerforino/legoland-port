#include <stdio.h>
#include <stdlib.h>
#include "legoland.h"

#include "clipping.h"
#include "debug_alloc.h"
#include "draw.h"
#include "globals.h"
#include "image_sprite.h"
#include "man3d.h"
#include "print_sprite.h"
#include "render.h"
#include "timer.h"

struct SortNode {
    /* 0x00 */ struct SortNode *left;
    /* 0x04 */ struct SortNode *right;
    /* 0x08 */ int key;
    /* 0x0c */ unsigned int flags;
    /* 0x10 */ struct HitInfo hit;
    /* 0x1c */ struct Sprite *sprite;
    /* 0x20 */ int x;
    /* 0x24 */ int y;
    /* 0x28 */ int field_28;
    /* 0x2c */ int field_2c;
    /* 0x30 */ int field_30;
    /* 0x34 */ int field_34;
    /* 0x38 */ int field_38;
    /* 0x3c */ int field_3c;
    /* 0x40 */ int field_40;
};

struct SpriteExArg {
    /* 0x00 */ struct Sprite *sprite;
    /* 0x04 */ int xoff;
    /* 0x08 */ int yoff;
    /* 0x0c */ int field_c;
    /* 0x10 */ unsigned int mode;
    /* 0x14 */ unsigned int mask;
};

// FUNCTION: LEGOLAND 0x004853a0
LEGO_EXPORT unsigned int PrintSprite(struct Sprite *sprite, unsigned int x, unsigned int y, unsigned int param_4, int *param_5) {
    int i;
    int xoff;
    int yoff;
    unsigned int result;

    result = 1;
    DAT_007feb14 = 0;
    if ((sprite->flags & 0x8000) == 0) {
        if (param_4 != 0) {
            if (*GetVRAMAddress(sprite) == 0) {
                if (FUN_00499500(sprite) != 0) {
                    result = RenderSpriteX(sprite, x, y, param_4);
                } else {
                    result = 0;
                }
            } else {
                result = RenderSpriteX(sprite, x, y, param_4);
            }
        } else {
            if (*GetVRAMAddress(sprite) == 0) {
                if (FUN_00499500(sprite) != 0) {
                    result = RenderSprite(sprite, x, y);
                } else {
                    result = 0;
                }
            } else {
                result = RenderSprite(sprite, x, y);
            }
        }
    } else {
        i = 0;
        if (sprite->group->count > 0) {
            do {
                if ((sprite->group->subs[i]->flags & 0x4000) == 0) {
                    xoff = sprite->group->xoffs[i];
                    yoff = sprite->group->yoffs[i];
                    if (xoff < 0) {
                        xoff = -(-xoff >> 1);
                    } else {
                        xoff = xoff >> 1;
                    }
                    if (yoff < 0) {
                        yoff = -(-yoff >> 1);
                    } else {
                        yoff = yoff >> 1;
                    }
                    if (param_4 != 0) {
                        if (*GetVRAMAddress(sprite->group->subs[i]) == 0) {
                            if (FUN_00499500(sprite->group->subs[i]) != 0) {
                                RenderSpriteX(sprite->group->subs[i], xoff + x, yoff + y, param_4);
                            }
                        } else {
                            RenderSpriteX(sprite->group->subs[i], xoff + x, yoff + y, param_4);
                        }
                    } else {
                        if (*GetVRAMAddress(sprite->group->subs[i]) == 0) {
                            if (FUN_00499500(sprite->group->subs[i]) != 0) {
                                RenderSprite(sprite->group->subs[i], xoff + x, yoff + y);
                            }
                        } else {
                            RenderSprite(sprite->group->subs[i], xoff + x, yoff + y);
                        }
                    }
                }
                i = i + 1;
            } while (i < sprite->group->count);
        }
    }
    if (param_5 != NULL && DAT_007feb14 != 0 && *param_5 != 0x100) {
        Hover = *(struct HoverInfo *)param_5;
    }
    return result;
}

// FUNCTION: LEGOLAND 0x004855d0
void FUN_004855d0(struct Sprite *sprite, int *out) {
    int xoff;
    int yoff;
    int i;
    int right;
    int bottom;
    struct SpriteGroup *group;
    struct Sprite *sub;

    if ((sprite->flags & 0x8000) == 0) {
        out[0] = 0;
        out[1] = 0;
        if (sprite != NULL) {
            out[2] = (short)sprite->width + -1;
            out[3] = (short)sprite->height + -1;
            return;
        }
        out[2] = 0;
        out[3] = 0;
        return;
    }
    i = 0;
    out[0] = 0x7fffffff;
    out[1] = 0x7fffffff;
    out[2] = -0x80000000;
    out[3] = -0x80000000;
    group = (struct SpriteGroup *)sprite->image;
    if (0 < group->count) {
        do {
            xoff = group->xoffs[i];
            yoff = group->yoffs[i];
            if (xoff < 0) {
                xoff = -(-xoff >> 1);
            } else {
                xoff = xoff >> 1;
            }
            if (yoff < 0) {
                yoff = -(-yoff >> 1);
            } else {
                yoff = yoff >> 1;
            }
            sub = group->subs[i];
            right = (short)sub->width + xoff;
            bottom = (short)sub->height + yoff;
            if (xoff < out[0]) {
                out[0] = xoff;
            }
            if (yoff < out[1]) {
                out[1] = yoff;
            }
            if (right > out[2]) {
                out[2] = right;
            }
            if (bottom > out[3]) {
                out[3] = bottom;
            }
            group = (struct SpriteGroup *)sprite->image;
            i = i + 1;
        } while (i < group->count);
    }
}

// FUNCTION: LEGOLAND 0x004856a0
LEGO_EXPORT unsigned int PrintSpriteEx(struct SpriteExArg *arg, int x, int y) {
    struct Sprite *sprite;
    struct SpriteGroup *group;
    int mask;
    unsigned int result;
    int i;
    int xoff;
    int yoff;

    result = 1;
    sprite = arg->sprite;
    x = x + arg->xoff / 2;
    y = y + arg->yoff / 2;
    if ((sprite->flags & 0x8000) == 0) {
        if (arg->mode != 0) {
            if (*GetVRAMAddress(sprite) == 0) {
                if (FUN_00499500(sprite) != 0) {
                    return RenderSpriteX(sprite, x, y, arg->mode);
                }
            } else {
                return RenderSpriteX(sprite, x, y, arg->mode);
            }
        } else {
            if (*GetVRAMAddress(sprite) == 0) {
                if (FUN_00499500(sprite) != 0) {
                    return RenderSprite(sprite, x, y);
                }
            } else {
                return RenderSprite(sprite, x, y);
            }
        }
        return 0;
    }
    mask = arg->mask;
    group = (struct SpriteGroup *)sprite->image;
    if (group->count <= 0) {
        return result;
    }
    i = 0;
    do {
        if ((mask & 1) != 0) {
            xoff = group->xoffs[i];
            yoff = group->yoffs[i];
            if (xoff < 0) {
                xoff = -(-xoff >> 1);
            } else {
                xoff = xoff >> 1;
            }
            if (yoff < 0) {
                yoff = -(-yoff >> 1);
            } else {
                yoff = yoff >> 1;
            }
            if (arg->mode != 0) {
                if (*GetVRAMAddress(group->subs[i]) == 0) {
                    if (FUN_00499500(sprite->group->subs[i]) != 0) {
                        RenderSpriteX(sprite->group->subs[i], xoff + x, yoff + y, arg->mode);
                    }
                } else {
                    RenderSpriteX(sprite->group->subs[i], xoff + x, yoff + y, arg->mode);
                }
            } else {
                if (*GetVRAMAddress(group->subs[i]) == 0) {
                    if (FUN_00499500(sprite->group->subs[i]) != 0) {
                        RenderSprite(sprite->group->subs[i], xoff + x, yoff + y);
                    }
                } else {
                    RenderSprite(sprite->group->subs[i], xoff + x, yoff + y);
                }
            }
        }
        group = (struct SpriteGroup *)sprite->image;
        i++;
        mask >>= 1;
    } while (i < group->count);
    return 1;
}

// FUNCTION: LEGOLAND 0x004858e0
LEGO_EXPORT unsigned int PrintTiledSprite(struct Sprite *sprite, int param_2, int param_3, int param_4, int param_5, int param_6, int param_7) {
    if ((sprite->flags & 0x8000) == 0) {
        if (*GetVRAMAddress(sprite) != 0 || FUN_00499500(sprite) != 0) {
            return RenderTiledSprite(sprite, param_2, param_3, param_4, param_5, param_6, param_7);
        }
    }
    return 0;
}

// FUNCTION: LEGOLAND 0x00485940
LEGO_EXPORT unsigned int PrintScaledSprite(struct Sprite *sprite, int param_2, int param_3, int param_4, int param_5) {
    if (*GetVRAMAddress(sprite) == 0) {
        if (FUN_00499500(sprite) != 0) {
            return RenderScaledSprite(sprite, param_2, param_3, param_4, param_5);
        }
        return 0;
    }
    return RenderScaledSprite(sprite, param_2, param_3, param_4, param_5);
}

// FUNCTION: LEGOLAND 0x004859b0
LEGO_EXPORT void ClearPrintList(void) {
    struct SortNode *node = PrintListHead;

    while (node != NULL) {
        node = node->right;
    }

    PrintListBytesUsed = 0;
    PrintListHead = NULL;
}

// FUNCTION: LEGOLAND 0x004859d0
LEGO_EXPORT void DrawAndClearPrintList(void) {
    struct SortNode *node;
    struct SortNode *node_save;
    unsigned int saved_pal;
    unsigned int saved_frame;
    void *obj;
    RECT saved_clip;

    node = PrintListHead;
    saved_pal = GetOverridePalette();
    saved_frame = GetOverrideFrame();
    DAT_0066b5ac = 0;
    for (; node != NULL; node = node->right) {
        if ((node->flags & 1) != 0) {
            if (node->field_28 < 0 || node->field_30 > (int)(unsigned int)lpConfig->screen_width || node->field_2c < 0 || node->field_34 > (int)(unsigned int)lpConfig->screen_height) {
                // STRING: LEGOLAND 0x004bdd0c
                printf("error");
            }
            GetClipping(&saved_clip);
            SetClipping(&node->field_28);
            if (node->sprite != NULL) {
                SetOverridePalette(node->field_3c);
                SetOverrideFrame(node->field_40);
                PrintSprite(node->sprite, node->x, node->y, node->field_38, (int *)&node->hit);
                ClearSpriteOverrides();
                if (node->hit.type == 0x103 && node->hit.field_4 != 0 && (((struct Sprite *)node->sprite)->flags & 0x2000) != 0) {
                    obj = (void *)node->hit.field_4;
                    (*(void (**)(void *, int, int, int *, int *, int))((char *)*(void **)((char *)obj + 0xc) + 0xb0))(obj, node->x, node->y, &node->hit.field_8, &node->field_28, node->field_38);
                }
            }
            if ((node->flags & 4) != 0) {
                (*(void (**)(int))&node_save->field_34)(node_save->field_38);
            }
            SetClipping(&saved_clip);
        } else if ((node->flags & 0x2000) != 0) {
            Render3DPerson((struct Person *)node->sprite);
        } else {
            node_save = node;
            if (node->sprite != NULL) {
                SetOverridePalette(node->field_2c);
                SetOverrideFrame(node->field_30);
                PrintSprite(node->sprite, node->x, node->y, node->field_28, (int *)&node->hit);
                ClearSpriteOverrides();
                if (node->hit.type == 0x103 && node->hit.field_4 != 0 && (((struct Sprite *)node->sprite)->flags & 0x2000) != 0) {
                    obj = (void *)node->hit.field_4;
                    (*(void (**)(void *, int, int, int *, int, int))((char *)*(void **)((char *)obj + 0xc) + 0xb0))(obj, node->x, node->y, &node->hit.field_8, 0, node->field_28);
                }
                if ((node->flags & 4) != 0) {
                    (*(void (**)(int))&node->field_34)(node->field_38);
                }
            }
        }
    }
    PrintListBytesUsed = 0;
    PrintListHead = NULL;
    SetOverridePalette(saved_pal);
    SetOverrideFrame(saved_frame);
}

// FUNCTION: LEGOLAND 0x00485bd0
void InsertSortNode(struct SortNode *node) {
    struct SortNode *l;

    if (SortCursor == NULL) {
        // STRING: LEGOLAND 0x004bdd40
        DBPrintf("Oh drat, Bad stuff in the sprite sorter\n");
    }
    if (node == NULL) {
        // STRING: LEGOLAND 0x004bdd14
        DBPrintf("Oh no, Not enough RAM for sprite sort list\n");
    }
    if (PrintListHead == NULL) {
        node->left = NULL;
        node->right = NULL;
        PrintListHead = node;
    } else {
        if (node->key < SortCursor->key) {
            while (node->key < SortCursor->key && SortCursor->left != NULL) {
                SortCursor = SortCursor->left;
            }
        } else {
            while (node->key > SortCursor->key && SortCursor->right != NULL) {
                SortCursor = SortCursor->right;
            }
        }
        if (node->key < SortCursor->key) {
            l = SortCursor->left;
            node->left = l;
            if (l == NULL) {
                PrintListHead = node;
            } else {
                l->right = node;
            }
            node->right = SortCursor;
            SortCursor->left = node;
        } else {
            l = SortCursor->right;
            node->right = l;
            if (l != NULL) {
                l->left = node;
            }
            node->left = SortCursor;
            SortCursor->right = node;
        }
    }
    SortCursor = node;
    if (node == NULL) {
        DBPrintf("Oh drat, Bad stuff in the sprite sorter\n");
    }
}

// FUNCTION: LEGOLAND 0x00485cd0
LEGO_EXPORT void SortSpriteWithCallback(struct Sprite *sprite, unsigned int x, unsigned int y, int key, unsigned int param_5, unsigned int param_6, unsigned int param_7, struct HitInfo *hit) {
    unsigned int original = PrintListBytesUsed;
    struct SortNode *node;

    PrintListBytesUsed += 0x3c;
    node = (struct SortNode *)&PrintListPool[original];
    node->key = key;
    node->sprite = sprite;
    node->x = x;
    node->y = y;
    node->flags = 4;
    node->field_28 = param_5;
    node->field_34 = param_6;
    node->field_38 = param_7;
    node->field_2c = GetOverridePalette();
    node->field_30 = GetOverrideFrame();
    if (hit != NULL) {
        node->hit = *hit;
        InsertSortNode(node);
        return;
    }
    node->hit.type = 0x100;
    InsertSortNode(node);
}

// FUNCTION: LEGOLAND 0x00485d70
LEGO_EXPORT void SortSprite(struct Sprite *sprite, unsigned int x, unsigned int y, int key, unsigned int param_5, struct HitInfo *hit) {
    unsigned int original = PrintListBytesUsed;
    struct SortNode *node;

    PrintListBytesUsed += 0x3c;
    node = (struct SortNode *)&PrintListPool[original];
    node->key = key;
    node->sprite = sprite;
    node->flags = 0;
    node->x = x;
    node->y = y;
    node->field_28 = param_5;
    node->field_2c = GetOverridePalette();
    node->field_30 = GetOverrideFrame();
    if (hit != NULL) {
        node->hit = *hit;
        InsertSortNode(node);
        return;
    }
    node->hit.type = 0x100;
    InsertSortNode(node);
}

// FUNCTION: LEGOLAND 0x00485e00
LEGO_EXPORT void SortPerson(struct Person *person, unsigned int param_2, void *param_3) {
    unsigned int original = PrintListBytesUsed;
    struct SortNode *block;

    PrintListBytesUsed += 0x20;
    block = (struct SortNode *)&PrintListPool[original];
    block->key = param_2;
    block->flags = 0x2000;
    block->sprite = (struct Sprite *)person;
    InsertSortNode(block);
}

// FUNCTION: LEGOLAND 0x00485e40
LEGO_EXPORT void SortClippedSprite(struct Sprite *sprite, unsigned int x, unsigned int y, int key, RECT *clip, unsigned int param_6, struct HitInfo *hit) {
    unsigned int original = PrintListBytesUsed;
    struct SortNode *node;

    PrintListBytesUsed += 0x4c;
    node = (struct SortNode *)&PrintListPool[original];
    node->key = key;
    node->sprite = sprite;
    node->x = x;
    node->flags = 1;
    node->y = y;
    *(RECT *)&node->field_28 = *clip;
    node->field_38 = param_6;
    node->field_3c = GetOverridePalette();
    node->field_40 = GetOverrideFrame();
    if (hit != NULL) {
        node->hit = *hit;
        InsertSortNode(node);
        return;
    }
    node->hit.type = 0x100;
    InsertSortNode(node);
}

// FUNCTION: LEGOLAND 0x00485ef0
LEGO_EXPORT void ResetHitInfo(void) {
    Hover.type = 0x100;
}

// FUNCTION: LEGOLAND 0x00485f00
void PrintSpriteSimple(struct Sprite *param_1, unsigned int param_2, unsigned int param_3) {
    PrintSprite(param_1, param_2, param_3, 0, 0);
}

// FUNCTION: LEGOLAND 0x00485f20
void FUN_00485f20(void *ptr) {
    DAT_0066b630 = ptr;
}

// FUNCTION: LEGOLAND 0x00485f30
void FUN_00485f30(unsigned int param_1, unsigned int param_2, unsigned int param_3, unsigned int param_4) {
    DAT_00797e68 = param_1;
    DAT_00701e58 = param_2;
    DAT_00701e60 = param_3;
    DAT_0066be48 = param_4;
}

// FUNCTION: LEGOLAND 0x00485f60
void FUN_00485f60(void) {
    DAT_0066be40 = 0x80;
    DAT_0066be44 = 0x78;
    DAT_00701e5c = malloc(0xf000);
    DAT_0066be4c = DAT_0066be40 * 4;
}

// FUNCTION: LEGOLAND 0x00485fa0
void FUN_00485fa0(void) {
    if (DAT_00701e5c != NULL) {
        free(DAT_00701e5c);
    }
}

// FUNCTION: LEGOLAND 0x00485fc0
void FUN_00485fc0(unsigned int param_1) {
    *(unsigned int *)&DAT_007cb5e0 = param_1;
    FUN_004860f0();
    FUN_00485f60();
    FUN_00486540();
}

// FUNCTION: LEGOLAND 0x00485fe0
void FUN_00485fe0(struct Sprite *sprite, int x, int y) {
    RECT rect1;
    RECT rect2;
    int offsets[2];
    unsigned int *p;
    int count;

    p = (unsigned int *)DAT_00701e5c;
    for (count = DAT_0066be40 * DAT_0066be44; count != 0; count = count + -1) {
        *p = 0;
        p = p + 1;
    }
    if (sprite == NULL) {
        return;
    }
    rect1.bottom = (short)sprite->height;
    rect1.left = 0;
    rect1.top = 0;
    rect2.right = DAT_0066be40 + x;
    rect1.right = (short)sprite->width;
    rect2.bottom = DAT_0066be44 + y;
    rect2.left = x;
    rect2.top = y;
    if (IntersectRect(&rect1, &rect1, &rect2) == 0) {
        return;
    }
    if (x < 0) {
        offsets[0] = -x;
        rect1.left = 0;
    } else {
        offsets[0] = 0;
    }
    if (y < 0) {
        offsets[1] = -y;
        rect1.top = 0;
    } else {
        offsets[1] = 0;
    }
    ZBufferHelper((struct DrawLLS *)sprite->image->data, &rect1, (struct Point *)offsets, (unsigned int *)DAT_00701e5c);
}
