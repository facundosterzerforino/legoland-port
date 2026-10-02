#include <stdlib.h>

#include "legoland.h"

#include "bloke.h"
#include "draw.h"
#include "globals.h"
#include "image_sprite.h"
#include "man3d.h"
#include "map_object.h"
#include "math.h"
#include "print_sprite.h"
#include "render3d.h"
#include "ride_bloke.h"
#include "ride_queue.h"
#include "sound_music.h"

struct CountNode {
    struct CountNode *next;
    unsigned short var_4;
};

struct RideFX {
    unsigned char pad_0[0x60];
    unsigned char refcount;
};

struct RideMover {
    unsigned char pad_0[0x10];
    int x;
    int y;
    unsigned char pad_18[0x28 - 0x18];
    int velX;
    int velY;
    int destX;
    int destY;
    unsigned char pad_38[0xb0 - 0x38];
    float dirX;
    float dirY;
    unsigned char dir;
    unsigned char pad_b9[2];
    unsigned char moving;
    unsigned char pad_bc[0xc8 - 0xbc];
    unsigned short speed;
};

struct NewBloke {
    struct NewBloke *next;
    unsigned short id;
    unsigned char pad_6[0x8 - 0x6];
    int sx;
    int sy;
    union {
        struct Point f;
        struct {
            int fx;
            int fy;
        };
    };
    union {
        struct Point p;
        struct {
            int px;
            int py;
        };
    };
    union {
        struct Point t;
        struct PathPair tp;
        struct {
            int tx;
            int ty;
        };
    };
    int velX;
    int velY;
    struct Point wp[17];
    unsigned char f_b8;
    unsigned char f_b9;
    unsigned char f_ba;
    unsigned char f_bb;
    short f_bc;
    unsigned short f_be;
    unsigned short f_c0;
    unsigned char f_c2;
    unsigned char f_c3;
    unsigned char f_c4;
    unsigned char pad_c5;
    unsigned short f_c6;
    unsigned short f_c8;
    unsigned char pad_ca[2];
    union {
        int owner;
        struct Bloke *bloke;
    };
};

struct RideBloke {
    struct RideBloke *next;
    unsigned char pad_4[0xcc - 0x4];
    struct RideFX *fx;
};

struct DrivingBloke {
    struct DrivingBloke *next;
    unsigned char pad_4[0x18 - 0x4];
    int px;
    int py;
    unsigned char pad_20[0xbe - 0x20];
    unsigned short field_be;
    unsigned short field_c0;
    unsigned char field_c2;
};

struct NearBloke {
    struct NearBloke *next;
    unsigned char pad_4[0x10 - 0x4];
    int px;
    int py;
    unsigned char pad_18[0xb0 - 0x18];
    float fx;
    float fy;
};

struct SPHandlers {
    unsigned char pad_0[0x18];
    unsigned int (*get_rf_flags)(int x, int y);
    void (*sp_enter)(unsigned int x, unsigned int y);
    void (*sp_leave)(unsigned int x, unsigned int y);
};

struct TimerStruct {
    unsigned char pad_0[0x30];
    unsigned int var_30[34];
    unsigned char pad_b8[3];
    unsigned char var_bb;
};

struct PairArg {
    unsigned int var_0;
    unsigned int var_4;
};

struct TileInfo {
    unsigned char pad_0[0x14];
    unsigned char var_14;
    unsigned char pad_15[0x1c - 0x15];
    unsigned char var_1c;
    unsigned char var_1d;
};

struct BlokeSprite {
    unsigned char pad_0[8];
    unsigned int var_8;
    unsigned int var_c;
    unsigned char pad_10[0xa9];
    unsigned char var_b9;
    unsigned char pad_ba[9];
    unsigned char var_c3;
    unsigned char pad_c4[8];
    unsigned int var_cc;
};

// FUNCTION: LEGOLAND 0x00401000
__int64 FUN_00401000(int x, int y, int rot) {
    union {
        __int64 i;
        struct {
            int lo;
            int hi;
        } p;
    } r;
    switch (rot) {
    case 1:
        r.p.lo = y;
        r.p.hi = -x;
        return r.i;
    case 3:
        r.p.lo = x;
        r.p.hi = y;
        return r.i;
    case 5:
        r.p.lo = -y;
        r.p.hi = x;
        return r.i;
    case 7:
        r.p.lo = -x;
        r.p.hi = -y;
        return r.i;
    default:
        return r.i;
    }
}

// FUNCTION: LEGOLAND 0x00401080
void FUN_00401080(struct NewBloke *b) {
    struct Point local;
    unsigned int frame;
    union {
        __int64 i;
        struct {
            int lo;
            int hi;
        } p;
    } r;

    local.x = b->tx;
    local.y = b->ty;
    frame = b->f_bb;
    if (DAT_004c11c0 != 0) {
        FUN_00480840(&local, &local, b->f_ba);
        r.i = FUN_00401000(104, 0, b->f_ba);
        b->wp[frame].x = ((local.x << 8) + r.p.lo) << 8;
        b->wp[frame].y = ((local.y << 8) + r.p.hi) << 8;
        frame++;
        r.i = FUN_00401000(268, 88, b->f_ba);
        b->wp[frame].x = ((local.x << 8) + r.p.lo) << 8;
        b->wp[frame].y = ((local.y << 8) + r.p.hi) << 8;
        frame++;
        r.i = FUN_00401000(424, 244, b->f_ba);
        b->wp[frame].x = ((local.x << 8) + r.p.lo) << 8;
        b->wp[frame].y = ((local.y << 8) + r.p.hi) << 8;
        frame++;
        r.i = FUN_00401000(512, 408, b->f_ba);
        b->wp[frame].x = ((local.x << 8) + r.p.lo) << 8;
        b->wp[frame].y = ((local.y << 8) + r.p.hi) << 8;
        frame++;
        FUN_00480840(&local, &local, b->f_ba);
        FUN_00480840(&local, &local, b->f_ba = (b->f_ba + 2) & 7);
        b->wp[frame].x = local.x << 16;
        b->wp[frame].y = local.y << 16;
    } else {
        r.i = FUN_00401000(104, 0, b->f_ba);
        b->wp[frame].x = ((local.x << 8) + r.p.lo) << 8;
        b->wp[frame].y = ((local.y << 8) + r.p.hi) << 8;
        frame++;
        r.i = FUN_00401000(268, 44, b->f_ba);
        b->wp[frame].x = ((local.x << 8) + r.p.lo) << 8;
        b->wp[frame].y = ((local.y << 8) + r.p.hi) << 8;
        frame++;
        r.i = FUN_00401000(424, 122, b->f_ba);
        b->wp[frame].x = ((local.x << 8) + r.p.lo) << 8;
        b->wp[frame].y = ((local.y << 8) + r.p.hi) << 8;
        frame++;
        r.i = FUN_00401000(512, 204, b->f_ba);
        b->wp[frame].x = ((local.x << 8) + r.p.lo) << 8;
        b->wp[frame].y = ((local.y << 8) + r.p.hi) << 8;
        FUN_00480840(&local, &local, b->f_ba);
        b->f_ba = (b->f_ba + 2) & 7;
    }
    b->tx = local.x;
    b->ty = local.y;
    b->f_bb = frame + 1;
}

// FUNCTION: LEGOLAND 0x00401320
void FUN_00401320(struct NewBloke *b) {
    struct Point local;
    unsigned int frame;
    union {
        __int64 i;
        struct {
            int lo;
            int hi;
        } p;
    } r;

    local.x = b->tx;
    local.y = b->ty;
    frame = b->f_bb;
    if (DAT_004c11c0 != 0) {
        r.i = FUN_00401000(104, 0, b->f_ba);
        b->wp[frame].x = ((local.x << 8) + r.p.lo) << 8;
        b->wp[frame].y = ((local.y << 8) + r.p.hi) << 8;
        frame++;
        r.i = FUN_00401000(268, -44, b->f_ba);
        b->wp[frame].x = ((local.x << 8) + r.p.lo) << 8;
        b->wp[frame].y = ((local.y << 8) + r.p.hi) << 8;
        frame++;
        r.i = FUN_00401000(424, -122, b->f_ba);
        b->wp[frame].x = ((local.x << 8) + r.p.lo) << 8;
        b->wp[frame].y = ((local.y << 8) + r.p.hi) << 8;
        frame++;
        r.i = FUN_00401000(512, -204, b->f_ba);
        b->wp[frame].x = ((local.x << 8) + r.p.lo) << 8;
        b->wp[frame].y = ((local.y << 8) + r.p.hi) << 8;
        FUN_00480840(&local, &local, b->f_ba);
        b->f_ba = (b->f_ba - 2) & 7;
    } else {
        FUN_00480840(&local, &local, b->f_ba);
        r.i = FUN_00401000(104, 0, b->f_ba);
        b->wp[frame].x = ((local.x << 8) + r.p.lo) << 8;
        b->wp[frame].y = ((local.y << 8) + r.p.hi) << 8;
        frame++;
        r.i = FUN_00401000(268, -88, b->f_ba);
        b->wp[frame].x = ((local.x << 8) + r.p.lo) << 8;
        b->wp[frame].y = ((local.y << 8) + r.p.hi) << 8;
        frame++;
        r.i = FUN_00401000(424, -244, b->f_ba);
        b->wp[frame].x = ((local.x << 8) + r.p.lo) << 8;
        b->wp[frame].y = ((local.y << 8) + r.p.hi) << 8;
        frame++;
        r.i = FUN_00401000(512, -408, b->f_ba);
        b->wp[frame].x = ((local.x << 8) + r.p.lo) << 8;
        b->wp[frame].y = ((local.y << 8) + r.p.hi) << 8;
        frame++;
        FUN_00480840(&local, &local, b->f_ba);
        FUN_00480840(&local, &local, b->f_ba = (b->f_ba - 2) & 7);
        b->wp[frame].x = local.x << 16;
        b->wp[frame].y = local.y << 16;
    }
    b->tx = local.x;
    b->ty = local.y;
    b->f_bb = frame + 1;
}

// FUNCTION: LEGOLAND 0x004015c0
LEGO_EXPORT void GetSpriteSize(struct Sprite *sprite, unsigned short *pWidth, unsigned short *pHeight) {
    *pWidth = sprite->width;
    *pHeight = sprite->height;
}

// FUNCTION: LEGOLAND 0x004015e0
void FUN_004015e0(unsigned char *param_1) {
    struct Point local;
    unsigned int frame;

    local.x = *(int *)(param_1 + 0x20);
    local.y = *(int *)(param_1 + 0x24);
    frame = *(unsigned char *)(param_1 + 0xbb);
    FUN_00480840(&local, &local, *(unsigned char *)(param_1 + 0xba));
    FUN_00480840(&local, &local, *(unsigned char *)(param_1 + 0xba));
    *(int *)(param_1 + 0x20) = local.x;
    *(int *)(param_1 + 0x24) = local.y;
    *(int *)(param_1 + 0x30 + frame * 8) = local.x << 16;
    *(int *)(param_1 + 0x34 + frame * 8) = local.y << 16;
    *(unsigned char *)(param_1 + 0xbb) = frame + 1;
}

// FUNCTION: LEGOLAND 0x00401660
void FUN_00401660(struct NewBloke *b) {
    struct Point local;
    unsigned int frame;
    unsigned char d;
    union {
        __int64 i;
        struct {
            int lo;
            int hi;
        } p;
    } r;

    local.x = b->tx;
    local.y = b->ty;
    frame = b->f_bb;
    if (DAT_004c11c0 != 0) {
        r.i = FUN_00401000(13, 40, b->f_ba);
        b->wp[frame].x = ((local.x << 8) + r.p.lo) << 8;
        b->wp[frame].y = ((local.y << 8) + r.p.hi) << 8;
        frame++;
        r.i = FUN_00401000(33, 84, b->f_ba);
        b->wp[frame].x = ((local.x << 8) + r.p.lo) << 8;
        b->wp[frame].y = ((local.y << 8) + r.p.hi) << 8;
        frame++;
        d = b->f_ba + 2;
        FUN_00480840(&local, &local, b->f_ba = d & 7);
        b->wp[frame].x = local.x << 16;
        b->wp[frame].y = local.y << 16;
        b->tx = local.x;
        b->ty = local.y;
        b->f_bb = frame + 1;
        b->f_c2 = 1;
    } else {
        r.i = FUN_00401000(13, -40, b->f_ba);
        b->wp[frame].x = ((local.x << 8) + r.p.lo) << 8;
        b->wp[frame].y = ((local.y << 8) + r.p.hi) << 8;
        frame++;
        r.i = FUN_00401000(33, -84, b->f_ba);
        b->wp[frame].x = ((local.x << 8) + r.p.lo) << 8;
        b->wp[frame].y = ((local.y << 8) + r.p.hi) << 8;
        frame++;
        d = b->f_ba - 2;
        FUN_00480840(&local, &local, b->f_ba = d & 7);
        b->wp[frame].x = local.x << 16;
        b->wp[frame].y = local.y << 16;
        b->tx = local.x;
        b->ty = local.y;
        b->f_bb = frame + 1;
        b->f_c2 = 1;
    }
}

// FUNCTION: LEGOLAND 0x004017c0
LEGO_EXPORT __int64 MapToPlayfield(int param_1, int param_2) {
    int w;
    int h;
    union {
        __int64 i;
        struct {
            int lo;
            int hi;
        } p;
    } r;
    GetTileDimensions(&w, &h);
    r.p.lo = (param_1 - param_2) * w >> 9;
    r.p.hi = (param_1 + param_2) * h >> 9;
    return r.i;
}

static __inline __int64 MapToPlayfieldInl(int param_1, int param_2) {
    int w;
    int h;
    union {
        __int64 i;
        struct {
            int lo;
            int hi;
        } p;
    } r;
    GetTileDimensions(&w, &h);
    r.p.lo = (param_1 - param_2) * w >> 9;
    r.p.hi = (param_1 + param_2) * h >> 9;
    return r.i;
}

// FUNCTION: LEGOLAND 0x00401800
LEGO_EXPORT unsigned int IsSemiPermiable(int param_1, int param_2) {
    unsigned char rf_flags = Get_RFFlags(param_1, param_2);
    return (rf_flags & 0x3) == 0x3;
}

// FUNCTION: LEGOLAND 0x00401820
LEGO_EXPORT unsigned char SPGetRFFlags(int param_1, int param_2) {
    struct SPHandlers *src;
    int tile_ptr;
    int tx, ty;
    unsigned int tile_id;

    tx = param_1 >> 8;
    ty = param_2 >> 8;
    if (tx < 0 || tx >= (int)(unsigned int)lpConfig->width ||
        ty < 0 || ty >= (int)(unsigned int)lpConfig->height) {
        tile_ptr = 0;
    } else {
        tile_ptr = (int)GameMap[ty] + tx * 0x14;
    }
    tile_id = *(unsigned short *)(tile_ptr + 8);
    src = (struct SPHandlers *)TileSpriteInfo[tile_id].src;
    if (src->get_rf_flags != 0) {
        return src->get_rf_flags(param_1, param_2);
    }
    return 2;
}

// FUNCTION: LEGOLAND 0x00401890
LEGO_EXPORT void SPEnter(unsigned int param_1, unsigned int param_2) {
    struct SPHandlers *src;
    int tile_ptr;
    int tx, ty;
    unsigned int tile_id;

    tx = param_1 >> 8;
    ty = param_2 >> 8;
    if (tx < 0 || tx >= (int)(unsigned int)lpConfig->width ||
        ty < 0 || ty >= (int)(unsigned int)lpConfig->height) {
        tile_ptr = 0;
    } else {
        tile_ptr = (int)GameMap[ty] + tx * 0x14;
    }
    tile_id = *(unsigned short *)(tile_ptr + 8);
    src = (struct SPHandlers *)TileSpriteInfo[tile_id].src;
    if (src->sp_enter != 0) {
        src->sp_enter(param_1, param_2);
    }
}

// FUNCTION: LEGOLAND 0x00401900
LEGO_EXPORT void SPLeave(unsigned int param_1, unsigned int param_2) {
    struct SPHandlers *src;
    int tile_ptr;
    int tx, ty;
    unsigned int tile_id;

    tx = param_1 >> 8;
    ty = param_2 >> 8;
    if (tx < 0 || tx >= (int)(unsigned int)lpConfig->width ||
        ty < 0 || ty >= (int)(unsigned int)lpConfig->height) {
        tile_ptr = 0;
    } else {
        tile_ptr = (int)GameMap[ty] + tx * 0x14;
    }
    tile_id = *(unsigned short *)(tile_ptr + 8);
    src = (struct SPHandlers *)TileSpriteInfo[tile_id].src;
    if (src->sp_enter != 0) {
        src->sp_leave(param_1, param_2);
    }
}

// FUNCTION: LEGOLAND 0x00401970
int *FUN_00401970(int *param_1, int param_2, int param_3) {
    int *piVar1;

    if (DAT_004c10d4 == 0) {
        return 0;
    }
    piVar1 = (int *)DAT_004c10d4;
    do {
        if ((piVar1[6] <= param_2 + 1) && (param_2 <= piVar1[6] + 1) &&
            (piVar1[7] <= param_3 + 1) && (param_3 <= piVar1[7] + 1) &&
            (piVar1 != param_1)) {
            return piVar1;
        }
        piVar1 = (int *)*piVar1;
    } while (piVar1 != 0);
    return 0;
}

// FUNCTION: LEGOLAND 0x004019c0
void FUN_004019c0(struct RideMover *m) {
    unsigned char oldDir = m->dir;
    unsigned char moving = m->moving;

    if (!moving) {
        m->velX = 0;
        m->velY = 0;
    } else {
        int dy, dx, dist, diff;
        unsigned char newDir;

        dx = m->destX - m->x;
        dy = m->destY - m->y;
        dist = (int)sqrt((float)dx * (float)dx + (float)dy * (float)dy);
        if (dist != 0) {
            m->dirX = (float)dx / dist;
            m->dirY = (float)dy / dist;
            newDir = (ArcTan256(dx, dy) + 8) >> 4 & 15;
            m->dir = newDir;
            diff = (newDir - oldDir) & 15;
            if (diff & 8)
                diff |= -16;
            if (diff < -2 || diff > 2) {
                if (diff & 8)
                    oldDir--;
                else
                    oldDir++;
                m->dir = oldDir;
                m->dir = oldDir & 15;
            }
            m->velX = m->speed * dx / dist;
            m->velY = m->speed * dy / dist;
        } else {
            m->velX = 0;
            m->dirX = 0;
            m->dirY = 0;
            m->velY = 0;
        }
    }
}

// FUNCTION: LEGOLAND 0x00401ae0
int FUN_00401ae0(unsigned short id, int bloke) {
    struct RideQueueEntry *e;
    struct NewBloke *b;
    int t;

    e = FUN_00412650(id);
    b = (struct NewBloke *)malloc(0xd0);
    if (b == 0) {
        return -1;
    }
    b->id = id;
    b->owner = bloke;
    if (DAT_004c11c0 != 0) {
        b->px = e->x;
        b->fx = e->x << 16;
    } else {
        b->px = e->x + 2;
        b->fx = (e->x + 2) << 16;
    }
    b->py = e->y + 4;
    b->t = b->p;
    b->fy = (e->y + 4) << 16;
    if (FUN_00401970((int *)b, b->px, b->ty) != 0) {
        free(b);
        return -2;
    }
    b->next = (struct NewBloke *)DAT_004c10d4;
    DAT_004c10d4 = b;
    b->f_bb = 0;
    b->f_b8 = 0;
    b->f_c2 = 1;
    b->f_ba = 1;
    t = rand() & 15;
    b->f_c8 = 0x1000;
    t += 16;
    b->f_bc = 0;
    b->f_c6 = t << 8;
    switch (rand() % 3) {
    case 0:
        b->f_c3 = 3;
        break;
    case 1:
        b->f_c3 = 1;
        break;
    case 2:
        b->f_c3 = 2;
        break;
    }
    b->f_c4 = 0;
    b->f_be = 9000;
    b->f_c0 = 1200;
    FUN_00401080(b);
    FUN_004019c0((struct RideMover *)b);
    return 0;
}

// FUNCTION: LEGOLAND 0x00401c40
unsigned int FUN_00401c40(unsigned short arg0) {
    struct CountNode *current = (struct CountNode *)DAT_004c10d4;
    unsigned int count = 0;

    while (current != NULL) {
        if (current->var_4 == arg0) {
            count++;
        }
        current = current->next;
    }

    return count;
}

// FUNCTION: LEGOLAND 0x00401c60
void FUN_00401c60(struct RideBloke *b) {
    struct SampleSource ss;
    struct RideBloke *next;
    struct RideBloke *cur;

    if (b != 0) {
        next = b->next;
        ss.type = 1;
        b->fx->refcount++;
        ss.bloke = b->fx;
        KillAllSamplesFromSource(&ss);
        free(b);

        if (b == (struct RideBloke *)DAT_004c10d4) {
            DAT_004c10d4 = next;
        } else {
            cur = (struct RideBloke *)DAT_004c10d4;
            while (cur->next != b) {
                cur = cur->next;
            }
            cur->next = next;
        }
    }
}

// FUNCTION: LEGOLAND 0x00401cd0
void FUN_00401cd0(struct TimerStruct *p) {
    unsigned char temp_bb = p->var_bb;
    if (temp_bb == 0) {
        return;
    }
    temp_bb--;
    p->var_30[0] = p->var_30[2];
    p->var_30[1] = p->var_30[3];
    p->var_30[2] = p->var_30[4];
    p->var_30[3] = p->var_30[5];
    p->var_30[4] = p->var_30[6];
    p->var_30[5] = p->var_30[7];
    p->var_30[6] = p->var_30[8];
    p->var_30[7] = p->var_30[9];
    p->var_30[8] = p->var_30[10];
    p->var_30[9] = p->var_30[11];
    p->var_30[10] = p->var_30[12];
    p->var_30[11] = p->var_30[13];
    p->var_30[12] = p->var_30[14];
    p->var_30[13] = p->var_30[15];
    p->var_30[14] = p->var_30[16];
    p->var_30[15] = p->var_30[17];
    p->var_30[16] = p->var_30[18];
    p->var_30[17] = p->var_30[19];
    p->var_30[18] = p->var_30[20];
    p->var_30[19] = p->var_30[21];
    p->var_30[20] = p->var_30[22];
    p->var_30[21] = p->var_30[23];
    p->var_30[22] = p->var_30[24];
    p->var_30[23] = p->var_30[25];
    p->var_30[24] = p->var_30[26];
    p->var_30[25] = p->var_30[27];
    p->var_30[26] = p->var_30[28];
    p->var_30[27] = p->var_30[29];
    p->var_30[28] = p->var_30[30];
    p->var_30[29] = p->var_30[31];
    p->var_30[30] = p->var_30[32];
    p->var_30[31] = p->var_30[33];
    p->var_bb = temp_bb;
}

// FUNCTION: LEGOLAND 0x00401e00
void FUN_00401e00(struct HistBuf *p) {
    p->e[16] = p->e[15];
    p->e[15] = p->e[14];
    p->e[14] = p->e[13];
    p->e[13] = p->e[12];
    p->e[12] = p->e[11];
    p->e[11] = p->e[10];
    p->e[10] = p->e[9];
    p->e[9] = p->e[8];
    p->e[8] = p->e[7];
    p->e[7] = p->e[6];
    p->e[6] = p->e[5];
    p->e[5] = p->e[4];
    p->e[4] = p->e[3];
    p->e[3] = p->e[2];
    p->e[2] = p->e[1];
    p->e[1] = p->e[0];
    p->count++;
}

// FUNCTION: LEGOLAND 0x00401f30
int FUN_00401f30(unsigned short id, struct PathPair *p, int dir) {
    struct RideQueueEntry *e;
    struct RideQueueEntry *q;
    struct RideQueueEntry *t1;
    struct RideQueueEntry *t2;
    struct RideQueueEntry *t3;
    struct PathPair pt;
    char r;
    int mask;

    e = (struct RideQueueEntry *)FUN_004125f0(p->a, p->b);
    r = rand() & 3;
    mask = 0;
    if (e == NULL) {
        return 0;
    }
    FUN_004808d0(&e->x, &pt.a, dir);
    q = FUN_004125a0(pt.a, pt.b);
    if (q != NULL && q->id == id && (q->field_14 & 0xf) != 6) {
        FUN_004808d0(&q->x, &pt.a, dir);
        t1 = FUN_004125a0(pt.a, pt.b);
        if (t1 != NULL && (t1->id != id || (t1->field_14 & 0xf) == 6)) {
            t1 = NULL;
        }
        FUN_004808d0(&q->x, &pt.a, (dir - 2) & 7);
        t2 = FUN_004125a0(pt.a, pt.b);
        if (t2 != NULL && (t2->id != id || (t2->field_14 & 0xf) == 6)) {
            t2 = NULL;
        }
        FUN_004808d0(&q->x, &pt.a, (dir + 2) & 7);
        t3 = FUN_004125a0(pt.a, pt.b);
        if (t3 != NULL && (t3->id != id || (t3->field_14 & 0xf) == 6)) {
            t3 = NULL;
        }
        if (t2 != NULL) {
            mask = 1;
        }
        if (t3 != NULL) {
            mask |= 4;
        }
        if (t1 != NULL) {
            mask |= 2;
        }
        switch (mask) {
        case 1:
            return 1;
        case 4:
            return 3;
        case 5:
            return (~r & 2) | 1;
        case 3:
            return ((r & 2) != 0) + 1;
        case 6:
            return ((~r & 2) | 4) >> 1;
        case 7:
            if ((r & 2) != 0) {
                return 2;
            }
            return ((~r & 1) << 1) | 1;
        default:
            return 2;
        }
    }
    return 4;
}

// FUNCTION: LEGOLAND 0x00402150
int FUN_00402150(unsigned short id, struct PathPair *p, int dir) {
    struct RideQueueEntry *e;
    struct RideQueueEntry *q;
    struct RideQueueEntry *s;
    struct PathPair pt;

    e = (struct RideQueueEntry *)FUN_004125f0(p->a, p->b);
    if (e != NULL) {
        if (e->field_18 == NULL) {
            return FUN_00401f30(id, p, dir);
        }
        FUN_004808d0(&e->x, &pt.a, dir);
        q = FUN_004125a0(pt.a, pt.b);
        if (q == NULL) {
            return FUN_00401f30(id, p, dir);
        }
        if (q->id != id) {
            return FUN_00401f30(id, p, dir);
        }
        if ((q->field_14 & 0xf) == 6) {
            return 5;
        }
        s = FUN_00412650(id);
        if ((s->y != q->y || s->x + 4 != q->x) && (q->field_14 & 0xf) != 4 && (q->field_14 & 0xf) != 5) {
            return FUN_00401f30(id, p, dir);
        }
        s = q->field_18;
        FUN_004808d0(&e->x, &pt.a, dir);
        FUN_004808d0(&pt.a, &pt.a, (dir - 2) & 7);
        if (s->x == pt.a && s->y == pt.b) {
            return 1;
        }
        FUN_004808d0(&e->x, &pt.a, dir);
        FUN_004808d0(&pt.a, &pt.a, (dir + 2) & 7);
        if (s->x == pt.a && s->y == pt.b) {
            return 3;
        }
        FUN_004808d0(&e->x, &pt.a, dir);
        FUN_004808d0(&pt.a, &pt.a, dir);
        if (s->x == pt.a && s->y == pt.b) {
            return 2;
        }
        if (s->x == e->x && s->y == e->y) {
            return 2;
        }
    }
    return 0;
}

// FUNCTION: LEGOLAND 0x00402340
int FUN_00402340(int a1) {
    if (DAT_004cbeb0 != 0) {
        if (a1 == 7 || a1 == 3)
            return 0;
    }
    if (DAT_004cbeb4 != 0) {
        if (a1 == 1 || a1 == 5)
            return 0;
    }
    if (DAT_004cbeb0 == 0 && DAT_004cbeb4 == 0)
        return 0;
    return 1;
}

// FUNCTION: LEGOLAND 0x00402390
int FUN_00402390(unsigned char *param_1) {
    void *iVar1;
    struct Point local;

    iVar1 = FUN_004125f0(*(unsigned int *)(param_1 + 0x20), *(unsigned int *)(param_1 + 0x24));
    if (iVar1 == 0) {
        return 0;
    }
    local.x = *(int *)((char *)iVar1 + 0xc);
    local.y = *(int *)((char *)iVar1 + 0x10);
    FUN_004808d0((int *)&local, (int *)&local,
        DAT_004b4034[(unsigned int)*(unsigned char *)(param_1 + 0xb8)]);
    iVar1 = FUN_004125a0(local.x, local.y);
    if ((iVar1 != 0) && ((*(unsigned char *)((char *)iVar1 + 0x14) & 0xf) == 5)) {
        return FUN_00402340(DAT_004b4034[(unsigned int)*(unsigned char *)(param_1 + 0xb8)]);
    }
    return 1;
}

// FUNCTION: LEGOLAND 0x00402430
int FUN_00402430(struct PairArg *a, struct PairArg *b) {
    struct TileInfo *p1 = FUN_004125f0(a->var_0, a->var_4);
    struct TileInfo *p2 = FUN_004125f0(b->var_0, b->var_4);

    if (p2 != p1) {
        if (p1 != 0) {
            if ((p1->var_14 & 0x10) && p1->var_1c == 0 && p1->var_1d != 0) {
                return 0;
            }

            if (p2 != 0 && p2->var_1c != 0) {
                p2->var_1c--;
            }

            p1->var_1c++;
        }
    }

    return 1;
}

// FUNCTION: LEGOLAND 0x00402490
struct NearBloke *FUN_00402490(struct NearBloke *b) {
    struct NearBloke *cur;
    struct NearBloke *result = NULL;
    int dx, dy, x, y;

    for (cur = (struct NearBloke *)DAT_004c10d4; cur != NULL; cur = cur->next) {
        if (cur == b) {
            continue;
        }
        dx = (b->px - cur->px) >> 8;
        dy = (b->py - cur->py) >> 8;
        if (dx * dx + dy * dy > 0x40000) {
            continue;
        }
        x = b->px - (int)(b->fx * -65536.0f);
        y = b->py - (int)(b->fy * -65536.0f);
        dx = (x - cur->px) >> 8;
        dy = (y - cur->py) >> 8;
        if (dx * dx + dy * dy <= 0x10000) {
            result = cur;
        }
    }
    return result;
}

// FUNCTION: LEGOLAND 0x00402550
void FUN_00402550(struct BlokeSprite *arg) {
    IP_RenderBlokeIn3DNow((struct Bloke *)arg->var_cc);

    switch (arg->var_c3) {
    case 1:
        SetOverridePalette(DSchoolBluePalette);
        break;
    case 2:
        SetOverridePalette(DSchoolYellowPalette);
        break;
    case 3:
        SetOverridePalette(DSchoolRedPalette);
        break;
    }

    SetOverrideFrame(arg->var_b9 + 16);
    PrintSprite(DSCarSprite, arg->var_8, arg->var_c, 0, 0);
    ClearOverrideFrame();
    ClearOverridePalette();
}

// FUNCTION: LEGOLAND 0x004025d0
void FUN_004025d0(struct Person *person, unsigned int direction) {
    person->field_48 = person->field_40 = 0.0f;
    switch (direction) {
    case 0:
        person->field_44 = -0.78539794683456421f;
        SetPersonRotation(person, &person->field_40);
        return;
    case 1:
        person->field_44 = -1.1780968904495239f;
        SetPersonRotation(person, &person->field_40);
        return;
    case 2:
        person->field_44 = 4.7123875617980957f;
        SetPersonRotation(person, &person->field_40);
        return;
    case 3:
        person->field_44 = 4.3196887969970703f;
        SetPersonRotation(person, &person->field_40);
        return;
    case 4:
        person->field_44 = 3.9269897937774658f;
        SetPersonRotation(person, &person->field_40);
        return;
    case 5:
        person->field_44 = 3.5342907905578613f;
        SetPersonRotation(person, &person->field_40);
        return;
    case 6:
        person->field_44 = 3.1415917873382568f;
        SetPersonRotation(person, &person->field_40);
        return;
    case 7:
        person->field_44 = 2.7488927841186523f;
        SetPersonRotation(person, &person->field_40);
        return;
    case 8:
        person->field_44 = 2.3561937808990479f;
        SetPersonRotation(person, &person->field_40);
        return;
    case 9:
        person->field_44 = 1.9634948968887329f;
        SetPersonRotation(person, &person->field_40);
        return;
    case 10:
        person->field_44 = 1.5707958936691284f;
        SetPersonRotation(person, &person->field_40);
        return;
    case 11:
        person->field_44 = 1.1780968904495239f;
        SetPersonRotation(person, &person->field_40);
        return;
    case 12:
        person->field_44 = 0.78539794683456421f;
        SetPersonRotation(person, &person->field_40);
        return;
    case 13:
        person->field_44 = 0.3926989734172821f;
        SetPersonRotation(person, &person->field_40);
        return;
    case 14:
        person->field_44 = 0.0f;
        SetPersonRotation(person, &person->field_40);
        return;
    case 15:
        person->field_44 = -0.3926989734172821f;
    }
    SetPersonRotation(person, &person->field_40);
}

// FUNCTION: LEGOLAND 0x00402780
void FUN_00402780(struct NewBloke *b) {
    int w2, h2;
    struct Point of;
    struct Point off;
    struct Point op;
    struct Point wp0;
    struct HitInfo cfg;
    union {
        __int64 i;
        struct {
            int lo;
            int hi;
        } p;
    } r;
    int sx, sy, key, v;
    struct Person *person;
    struct Point *scr;

    cfg.type = 0x306;
    cfg.field_4 = b->owner;
    cfg.field_8 = 0;
    r.i = MapToPlayfieldInl(b->fx >> 8, b->fy >> 8);
    of = b->f;
    wp0 = b->wp[0];
    op = b->p;
    v = 0;
    FUN_004125f0(b->px, b->py);
    GetTileDimensions(&w2, &h2);
    sx = r.p.lo - ((w2 + 1) >> 1) - (ScrollX >> 8);
    sy = r.p.hi - (ScrollY >> 8);
    off.x = DSchoolBlueCarData->x[b->f_b8] >> 1;
    off.y = DSchoolBlueCarData->y[b->f_b8] >> 1;
    AdjustOffsetForViewMode(&off);
    b->sx = lpConfig->view_x + off.x + sx;
    b->sy = lpConfig->view_y + off.y + sy;
    key = h2 + sy;
    switch (b->f_c3) {
    case 1:
        SetOverridePalette((unsigned int)DSchoolBluePalette);
        break;
    case 2:
        SetOverridePalette((unsigned int)DSchoolYellowPalette);
        break;
    case 3:
        SetOverridePalette((unsigned int)DSchoolRedPalette);
        break;
    }
    SetOverrideFrame(b->f_b9 = b->f_b8);
    SortSpriteWithCallback(DSCarSprite, b->sx, b->sy, key, 0, (unsigned int)FUN_00402550, (unsigned int)b, &cfg);
    ClearOverridePalette();
    ClearOverrideFrame();
    b->bloke->pos.x = sx;
    b->bloke->pos.y = sy;
    b->bloke->dir = (b->f_b8 + 6) & 15;
    person = Find3DPersonFromBloke(b->bloke);
    person->sort_id = wp0.x;
    scr = &person->screen;
    scr->x = lpConfig->view_x + b->bloke->pos.x + 0x10;
    scr->y = lpConfig->view_y + b->bloke->pos.y + 8;
    AdjustBlokePosition(scr);
    FUN_004025d0(person, b->bloke->dir);
    for (;;) {
        if (FUN_00402490((struct NearBloke *)b) != NULL) {
            b->f_c8 = b->f_c6 >> 1;
            b->f_bc++;
            if (b->f_bc <= 0x200) {
                return;
            }
            if (b->f_c4 != 0) {
                return;
            }
            break;
        }
        b->f_bc = 0;
        if (b->f_c8 < b->f_c6) {
            b->f_c8 += 0x40;
        }
        FUN_004019c0((struct RideMover *)b);
        b->fx += b->velX;
        b->fy += b->velY;
        b->px = (b->fx + 0x10000) >> 16;
        b->py = (b->fy + 0x10000) >> 16;
        if (FUN_00402430((struct PairArg *)&b->p, (struct PairArg *)&op) != 0) {
            break;
        }
        b->p = op;
        b->f = of;
        return;
    }
    if ((((wp0.x - b->fx) ^ (wp0.x - of.x)) | ((wp0.y - b->fy) ^ (wp0.y - of.y))) & 0x80000000) {
    } else if (b->f_bb != 0) {
        if (wp0.x != b->fx || wp0.y != b->fy) {
            return;
        }
    }
    if (b->f_bb == 0) {
        v = 1;
        if (b->f_c4 == 0) {
            b->f_c2 = 0;
            if (b->f_c0 != 0) {
                b->f_c4 = FUN_00401f30(b->id, &b->tp, b->f_ba);
                if (b->f_c4 == 0) {
                    b->f_c8 = 0;
                }
            } else {
                b->f_c4 = FUN_00402150(b->id, &b->tp, b->f_ba);
                if (b->f_c4 == 0) {
                    b->f_c8 = 0;
                }
            }
        }
    }
    switch (b->f_c4) {
    case 1:
        if (FUN_00402390((unsigned char *)b) != 0) {
            FUN_00401320(b);
            b->f_c4 = 0;
        }
        break;
    case 2:
        if (FUN_00402390((unsigned char *)b) != 0) {
            b->f_c4 = 0;
            FUN_004015e0((unsigned char *)b);
        }
        break;
    case 3:
        if (FUN_00402390((unsigned char *)b) != 0) {
            b->f_c4 = 0;
            FUN_00401080(b);
        }
        break;
    case 4:
        FUN_00401660(b);
        b->f_c4 = 0;
        break;
    case 5:
        FUN_00401320(b);
        FUN_004015e0((unsigned char *)b);
        b->f_c4 = 0;
        break;
    }
    if (v == 0) {
        FUN_00401cd0((struct TimerStruct *)b);
    }
}

// FUNCTION: LEGOLAND 0x00402c10
void FUN_00402c10(void) {
    struct DrivingBloke *cur = (struct DrivingBloke *)DAT_004c10d4;
    struct DrivingBloke *next;
    struct TileInfo *tile;

    while (cur != 0) {
        next = cur->next;

        if (cur->field_be != 0) {
            cur->field_be--;
        }
        if (cur->field_c0 != 0) {
            cur->field_c0--;
        }

        tile = (struct TileInfo *)FUN_004125f0(cur->px, cur->py);

        if ((tile == 0 && cur->field_c2 == 0) || cur->field_be == 0) {
            FUN_00401c60((struct RideBloke *)cur);
            if (tile != 0) {
                tile->var_1c--;
            }
        } else {
            FUN_00402780((struct NewBloke *)cur);
        }

        cur = next;
    }
}
