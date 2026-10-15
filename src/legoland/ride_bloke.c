#include <stdlib.h>

#include <stddef.h>
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

struct PairArg {
    unsigned int var_0;
    unsigned int var_4;
};

// FUNCTION: LEGOLAND 0x00401000
__int64 RotateOffset(int x, int y, int rot) {
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
        MovePointInDirection(&local, &local, b->f_ba);
        r.i = RotateOffset(104, 0, b->f_ba);
        b->wp[frame].x = ((local.x << 8) + r.p.lo) << 8;
        b->wp[frame].y = ((local.y << 8) + r.p.hi) << 8;
        frame++;
        r.i = RotateOffset(268, 88, b->f_ba);
        b->wp[frame].x = ((local.x << 8) + r.p.lo) << 8;
        b->wp[frame].y = ((local.y << 8) + r.p.hi) << 8;
        frame++;
        r.i = RotateOffset(424, 244, b->f_ba);
        b->wp[frame].x = ((local.x << 8) + r.p.lo) << 8;
        b->wp[frame].y = ((local.y << 8) + r.p.hi) << 8;
        frame++;
        r.i = RotateOffset(512, 408, b->f_ba);
        b->wp[frame].x = ((local.x << 8) + r.p.lo) << 8;
        b->wp[frame].y = ((local.y << 8) + r.p.hi) << 8;
        frame++;
        MovePointInDirection(&local, &local, b->f_ba);
        MovePointInDirection(&local, &local, b->f_ba = (b->f_ba + 2) & 7);
        b->wp[frame].x = local.x << 16;
        b->wp[frame].y = local.y << 16;
    } else {
        r.i = RotateOffset(104, 0, b->f_ba);
        b->wp[frame].x = ((local.x << 8) + r.p.lo) << 8;
        b->wp[frame].y = ((local.y << 8) + r.p.hi) << 8;
        frame++;
        r.i = RotateOffset(268, 44, b->f_ba);
        b->wp[frame].x = ((local.x << 8) + r.p.lo) << 8;
        b->wp[frame].y = ((local.y << 8) + r.p.hi) << 8;
        frame++;
        r.i = RotateOffset(424, 122, b->f_ba);
        b->wp[frame].x = ((local.x << 8) + r.p.lo) << 8;
        b->wp[frame].y = ((local.y << 8) + r.p.hi) << 8;
        frame++;
        r.i = RotateOffset(512, 204, b->f_ba);
        b->wp[frame].x = ((local.x << 8) + r.p.lo) << 8;
        b->wp[frame].y = ((local.y << 8) + r.p.hi) << 8;
        MovePointInDirection(&local, &local, b->f_ba);
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
        r.i = RotateOffset(104, 0, b->f_ba);
        b->wp[frame].x = ((local.x << 8) + r.p.lo) << 8;
        b->wp[frame].y = ((local.y << 8) + r.p.hi) << 8;
        frame++;
        r.i = RotateOffset(268, -44, b->f_ba);
        b->wp[frame].x = ((local.x << 8) + r.p.lo) << 8;
        b->wp[frame].y = ((local.y << 8) + r.p.hi) << 8;
        frame++;
        r.i = RotateOffset(424, -122, b->f_ba);
        b->wp[frame].x = ((local.x << 8) + r.p.lo) << 8;
        b->wp[frame].y = ((local.y << 8) + r.p.hi) << 8;
        frame++;
        r.i = RotateOffset(512, -204, b->f_ba);
        b->wp[frame].x = ((local.x << 8) + r.p.lo) << 8;
        b->wp[frame].y = ((local.y << 8) + r.p.hi) << 8;
        MovePointInDirection(&local, &local, b->f_ba);
        b->f_ba = (b->f_ba - 2) & 7;
    } else {
        MovePointInDirection(&local, &local, b->f_ba);
        r.i = RotateOffset(104, 0, b->f_ba);
        b->wp[frame].x = ((local.x << 8) + r.p.lo) << 8;
        b->wp[frame].y = ((local.y << 8) + r.p.hi) << 8;
        frame++;
        r.i = RotateOffset(268, -88, b->f_ba);
        b->wp[frame].x = ((local.x << 8) + r.p.lo) << 8;
        b->wp[frame].y = ((local.y << 8) + r.p.hi) << 8;
        frame++;
        r.i = RotateOffset(424, -244, b->f_ba);
        b->wp[frame].x = ((local.x << 8) + r.p.lo) << 8;
        b->wp[frame].y = ((local.y << 8) + r.p.hi) << 8;
        frame++;
        r.i = RotateOffset(512, -408, b->f_ba);
        b->wp[frame].x = ((local.x << 8) + r.p.lo) << 8;
        b->wp[frame].y = ((local.y << 8) + r.p.hi) << 8;
        frame++;
        MovePointInDirection(&local, &local, b->f_ba);
        MovePointInDirection(&local, &local, b->f_ba = (b->f_ba - 2) & 7);
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
    MovePointInDirection(&local, &local, *(unsigned char *)(param_1 + 0xba));
    MovePointInDirection(&local, &local, *(unsigned char *)(param_1 + 0xba));
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
        r.i = RotateOffset(13, 40, b->f_ba);
        b->wp[frame].x = ((local.x << 8) + r.p.lo) << 8;
        b->wp[frame].y = ((local.y << 8) + r.p.hi) << 8;
        frame++;
        r.i = RotateOffset(33, 84, b->f_ba);
        b->wp[frame].x = ((local.x << 8) + r.p.lo) << 8;
        b->wp[frame].y = ((local.y << 8) + r.p.hi) << 8;
        frame++;
        d = b->f_ba + 2;
        MovePointInDirection(&local, &local, b->f_ba = d & 7);
        b->wp[frame].x = local.x << 16;
        b->wp[frame].y = local.y << 16;
        b->tx = local.x;
        b->ty = local.y;
        b->f_bb = frame + 1;
        b->f_c2 = 1;
    } else {
        r.i = RotateOffset(13, -40, b->f_ba);
        b->wp[frame].x = ((local.x << 8) + r.p.lo) << 8;
        b->wp[frame].y = ((local.y << 8) + r.p.hi) << 8;
        frame++;
        r.i = RotateOffset(33, -84, b->f_ba);
        b->wp[frame].x = ((local.x << 8) + r.p.lo) << 8;
        b->wp[frame].y = ((local.y << 8) + r.p.hi) << 8;
        frame++;
        d = b->f_ba - 2;
        MovePointInDirection(&local, &local, b->f_ba = d & 7);
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
    struct FXSpriteList *src;
    INT_PTR tile_ptr;
    int tx, ty;
    unsigned int tile_id;

    tx = param_1 >> 8;
    ty = param_2 >> 8;
    if (tx < 0 || tx >= (int)(unsigned int)lpConfig->width ||
        ty < 0 || ty >= (int)(unsigned int)lpConfig->height) {
        tile_ptr = 0;
    } else {
        tile_ptr = (INT_PTR)GameMap[ty] + tx * sizeof(struct MapElement);
    }
    tile_id = *(unsigned short *)(tile_ptr + offsetof(struct MapElement, field_8));
    src = TileSpriteInfo[tile_id].src;
    if (src->get_rf_flags != 0) {
        /* the handlers return the flags in the whole register; this caller reads all of it */
        return ((unsigned int (*)(int, int))src->get_rf_flags)(param_1, param_2);
    }
    return 2;
}

// FUNCTION: LEGOLAND 0x00401890
LEGO_EXPORT void SPEnter(unsigned int param_1, unsigned int param_2) {
    struct FXSpriteList *src;
    INT_PTR tile_ptr;
    int tx, ty;
    unsigned int tile_id;

    tx = param_1 >> 8;
    ty = param_2 >> 8;
    if (tx < 0 || tx >= (int)(unsigned int)lpConfig->width ||
        ty < 0 || ty >= (int)(unsigned int)lpConfig->height) {
        tile_ptr = 0;
    } else {
        tile_ptr = (INT_PTR)GameMap[ty] + tx * sizeof(struct MapElement);
    }
    tile_id = *(unsigned short *)(tile_ptr + offsetof(struct MapElement, field_8));
    src = TileSpriteInfo[tile_id].src;
    if (src->on_enter != 0) {
        struct Point pos;

        pos.x = param_1;
        pos.y = param_2;
        src->on_enter(pos);
    }
}

// FUNCTION: LEGOLAND 0x00401900
LEGO_EXPORT void SPLeave(unsigned int param_1, unsigned int param_2) {
    struct FXSpriteList *src;
    INT_PTR tile_ptr;
    int tx, ty;
    unsigned int tile_id;

    tx = param_1 >> 8;
    ty = param_2 >> 8;
    if (tx < 0 || tx >= (int)(unsigned int)lpConfig->width ||
        ty < 0 || ty >= (int)(unsigned int)lpConfig->height) {
        tile_ptr = 0;
    } else {
        tile_ptr = (INT_PTR)GameMap[ty] + tx * sizeof(struct MapElement);
    }
    tile_id = *(unsigned short *)(tile_ptr + offsetof(struct MapElement, field_8));
    src = TileSpriteInfo[tile_id].src;
    if (src->on_enter != 0) { /* the original tests the enter handler */
        src->on_leave(param_1, param_2);
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
        piVar1 = *(int **)piVar1;
    } while (piVar1 != 0);
    return 0;
}

// FUNCTION: LEGOLAND 0x004019c0
void FUN_004019c0(struct NewBloke *m) {
    unsigned char oldDir = m->f_b8;
    volatile unsigned char moving = m->f_bb;
    int dy, dx, dist, diff, vx, vy;
    unsigned char newDir;

    if (!moving) {
        m->velX = 0;
        m->velY = 0;
    } else {
        do {
            dx = m->dest.x - m->fx;
            dy = m->dest.y - m->fy;
            dist = (int)sqrt((float)dx * (float)dx + (float)dy * (float)dy);
            if (dist != 0) {
                m->dirX = (float)dx / dist;
                m->dirY = (float)dy / dist;
                newDir = (ArcTan256(dx, dy) + 8) >> 4 & 15;
                m->f_b8 = newDir;
                diff = (newDir - oldDir) & 15;
                if (diff & 8)
                    diff |= -16;
                if (diff < -2 || (unsigned)diff > 2) {
                    if (diff & 8)
                        oldDir--;
                    else
                        oldDir++;
                    m->f_b8 = oldDir;
                    m->f_b8 = oldDir & 15;
                }
                vx = m->f_c8 * dx / dist;
                vy = m->f_c8 * dy / dist;
            } else {
                vx = 0;
                m->dirX = 0;
                m->dirY = 0;
                vy = 0;
            }
            m->velX = vx;
            m->velY = vy;
        } while (0);
    }
}

// FUNCTION: LEGOLAND 0x00401ae0
int FUN_00401ae0(unsigned short id, struct Bloke *bloke) {
    struct RideQueueEntry *e;
    struct NewBloke *b;
    int t;

    e = FUN_00412650(id);
    b = (struct NewBloke *)malloc(sizeof(struct NewBloke));
    if (b == 0) {
        return -1;
    }
    b->id = id;
    b->bloke = bloke;
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
    FUN_004019c0((struct NewBloke *)b);
    return 0;
}

// FUNCTION: LEGOLAND 0x00401c40
unsigned int FUN_00401c40(unsigned short arg0) {
    struct NewBloke *current = DAT_004c10d4;
    unsigned int count = 0;

    while (current != NULL) {
        if (current->id == arg0) {
            count++;
        }
        current = current->next;
    }

    return count;
}

// FUNCTION: LEGOLAND 0x00401c60
void FUN_00401c60(struct NewBloke *b) {
    struct SampleSource ss;
    struct NewBloke *next;
    struct NewBloke *cur;

    if (b != 0) {
        next = b->next;
        ss.type = 1;
        b->bloke->param_action++;
        ss.bloke = b->bloke;
        KillAllSamplesFromSource(&ss);
        free(b);

        if (b == DAT_004c10d4) {
            DAT_004c10d4 = next;
        } else {
            cur = DAT_004c10d4;
            while (cur->next != b) {
                cur = cur->next;
            }
            cur->next = next;
        }
    }
}

// FUNCTION: LEGOLAND 0x00401cd0
void FUN_00401cd0(struct NewBloke *p) {
    unsigned char temp_bb = p->f_bb;
    if (temp_bb == 0) {
        return;
    }
    temp_bb--;
    p->timers[0] = p->timers[2];
    p->timers[1] = p->timers[3];
    p->timers[2] = p->timers[4];
    p->timers[3] = p->timers[5];
    p->timers[4] = p->timers[6];
    p->timers[5] = p->timers[7];
    p->timers[6] = p->timers[8];
    p->timers[7] = p->timers[9];
    p->timers[8] = p->timers[10];
    p->timers[9] = p->timers[11];
    p->timers[10] = p->timers[12];
    p->timers[11] = p->timers[13];
    p->timers[12] = p->timers[14];
    p->timers[13] = p->timers[15];
    p->timers[14] = p->timers[16];
    p->timers[15] = p->timers[17];
    p->timers[16] = p->timers[18];
    p->timers[17] = p->timers[19];
    p->timers[18] = p->timers[20];
    p->timers[19] = p->timers[21];
    p->timers[20] = p->timers[22];
    p->timers[21] = p->timers[23];
    p->timers[22] = p->timers[24];
    p->timers[23] = p->timers[25];
    p->timers[24] = p->timers[26];
    p->timers[25] = p->timers[27];
    p->timers[26] = p->timers[28];
    p->timers[27] = p->timers[29];
    p->timers[28] = p->timers[30];
    p->timers[29] = p->timers[31];
    p->timers[30] = p->timers[32];
    p->timers[31] = p->timers[33];
    p->f_bb = temp_bb;
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
int PickQueueTurn(unsigned short id, struct PathPair *p, int dir) {
    struct RideQueueEntry *e;
    struct RideQueueEntry *q;
    struct RideQueueEntry *t1;
    struct RideQueueEntry *t2;
    struct RideQueueEntry *t3;
    struct PathPair pt;
    unsigned char r;
    int mask;

    int pb;
    int pa;
    pb = p->b;
    pa = p->a;
    e = (struct RideQueueEntry *)FindQueueEntryAtTile(pa, pb);
    r = rand() & 3;
    mask = 0;
    if (NULL == e) {
        return 0;
    }
    StepPointByDirection(&e->x, &pt.a, dir);
    q = FUN_004125a0(pt.a, pt.b);
    if (q != NULL && q->id == id && (q->field_14 & 0xf) != 6) {
        StepPointByDirection(&q->x, &pt.a, dir);
        t1 = FUN_004125a0(pt.a, pt.b);
        if (t1 != NULL && (t1->id != id || (0xf & t1->field_14) == 6)) {
            t1 = NULL;
        }
        StepPointByDirection(&q->x, &pt.a, (dir - 2) & 7);
        t2 = FUN_004125a0(pt.a, pt.b);
        if (t2 != NULL && (t2->id != id || (t2->field_14 & 0xf) == 6)) {
            t2 = NULL;
        }
        StepPointByDirection(&q->x, &pt.a, (dir + 2) & 7);
        t3 = FUN_004125a0(pt.a, pt.b);
        if (t3 != NULL && (t3->id != id || (0xf & t3->field_14) == 6)) {
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
int GetQueueTurn(unsigned short id, struct PathPair *p, int dir) {
    struct RideQueueEntry *q;
    struct RideQueueEntry *s;
    struct RideQueueEntry *e;
    struct PathPair pt;

    unsigned int ptmp99;
    volatile int ptmp102;
    volatile int ptmp310;
    e = (struct RideQueueEntry *)FindQueueEntryAtTile(p->a, p->b);
    if (e) {
        if (e->field_18 == NULL) {
            return PickQueueTurn(id, p, dir);
        }
        StepPointByDirection(&e->x, &pt.a, dir);
        ptmp102 = pt.b;
        { ptmp99 = ptmp102; }
        ptmp310 = pt.a;
        q = FUN_004125a0(ptmp310, ptmp99);
        if (q == NULL) {
            return PickQueueTurn(id, p, dir);
        }
        if (q->id != id) {
            return PickQueueTurn(id, p, dir);
        }
        if ((q->field_14 & 0xf) == 6) {
            return 5;
        }
        s = FUN_00412650(id);
        if ((s->y != q->y || s->x + 4 != q->x) && (q->field_14 & 0xf) != 4) {
            if ((q->field_14 & 0xf) != 5) {
                return PickQueueTurn(id, p, dir);
            }
        }
        s = q->field_18;
        { StepPointByDirection(&e->x, &pt.a, dir); }
        StepPointByDirection(&pt.a, &pt.a, (dir - 2) & 7);
        if (s->x == (unsigned)pt.a && s->y == pt.b) {
            return 1;
        }
        StepPointByDirection(&e->x, &pt.a, dir);
        StepPointByDirection(&pt.a, &pt.a, (dir + 2) & 7);
        if (s->x == pt.a && s->y == pt.b) {
            return 3;
        }
        StepPointByDirection(&e->x, &pt.a, dir);
        StepPointByDirection(&pt.a, &pt.a, dir);
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

    iVar1 = FindQueueEntryAtTile(*(unsigned int *)(param_1 + 0x20), *(unsigned int *)(param_1 + 0x24));
    if (iVar1 == 0) {
        return 0;
    }
    local.x = *(int *)((char *)iVar1 + 0xc);
    local.y = *(int *)((char *)iVar1 + 0x10);
    StepPointByDirection((int *)&local, (int *)&local,
        DAT_004b4034[(unsigned int)*(unsigned char *)(param_1 + 0xb8)]);
    iVar1 = FUN_004125a0(local.x, local.y);
    if ((iVar1 != 0) && ((*(unsigned char *)((char *)iVar1 + 0x14) & 0xf) == 5)) {
        return FUN_00402340(DAT_004b4034[(unsigned int)*(unsigned char *)(param_1 + 0xb8)]);
    }
    return 1;
}

// FUNCTION: LEGOLAND 0x00402430
int FUN_00402430(struct PairArg *a, struct PairArg *b) {
    struct RideQueueEntry *p1 = FindQueueEntryAtTile(a->var_0, a->var_4);
    struct RideQueueEntry *p2 = FindQueueEntryAtTile(b->var_0, b->var_4);

    if (p2 != p1) {
        if (p1 != 0) {
            if ((p1->field_14 & 0x10) && p1->field_1c == 0 && p1->field_1d != 0) {
                return 0;
            }

            if (p2 != 0 && p2->field_1c != 0) {
                p2->field_1c--;
            }

            p1->field_1c++;
        }
    }

    return 1;
}

// FUNCTION: LEGOLAND 0x00402490
struct NewBloke *FUN_00402490(struct NewBloke *b) {
    struct NewBloke *cur;
    struct NewBloke *result = NULL;
    int dx, dy, x, y;

    for (cur = (struct NewBloke *)DAT_004c10d4; cur != NULL; cur = cur->next) {
        if (cur == b) {
            continue;
        }
        dx = (b->fx - cur->fx) >> 8;
        dy = (b->fy - cur->fy) >> 8;
        if (dx * dx + dy * dy > 0x40000) {
            continue;
        }
        x = b->fx - (int)(b->dirX * -65536.0f);
        y = b->fy - (int)(b->dirY * -65536.0f);
        dx = (x - cur->fx) >> 8;
        dy = (y - cur->fy) >> 8;
        if (dx * dx + dy * dy <= 0x10000) {
            result = cur;
        }
    }
    return result;
}

// FUNCTION: LEGOLAND 0x00402550
void FUN_00402550(struct NewBloke *arg) {
    IP_RenderBlokeIn3DNow(arg->bloke);

    switch (arg->f_c3) {
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

    SetOverrideFrame(arg->f_b9 + 16);
    PrintSprite(DSCarSprite, arg->sx, arg->sy, 0, 0);
    ClearOverrideFrame();
    ClearOverridePalette();
}

// FUNCTION: LEGOLAND 0x004025d0
void SetPersonYawFromDir16(struct Person *person, unsigned int direction) {
    person->field_48 = person->field_40 = 0.0f;
    switch (direction) {
    case 0:
        person->field_44 = -0.78539794683456421f;
        break;
    case 1:
        person->field_44 = -1.1780968904495239f;
        break;
    case 2:
        person->field_44 = 4.7123875617980957f;
        break;
    case 3:
        person->field_44 = 4.3196887969970703f;
        break;
    case 4:
        person->field_44 = 3.9269897937774658f;
        break;
    case 5:
        person->field_44 = 3.5342907905578613f;
        break;
    case 6:
        person->field_44 = 3.1415917873382568f;
        break;
    case 7:
        person->field_44 = 2.7488927841186523f;
        break;
    case 8:
        person->field_44 = 2.3561937808990479f;
        break;
    case 9:
        person->field_44 = 1.9634948968887329f;
        break;
    case 10:
        person->field_44 = 1.5707958936691284f;
        break;
    case 11:
        person->field_44 = 1.1780968904495239f;
        break;
    case 12:
        person->field_44 = 0.78539794683456421f;
        break;
    case 13:
        person->field_44 = 0.3926989734172821f;
        break;
    case 14:
        person->field_44 = 0.0f;
        break;
    case 15:
        person->field_44 = -0.3926989734172821f;
    }
    SetPersonRotation(person, &person->field_40);
}

// FUNCTION: LEGOLAND 0x00402780
void FUN_00402780(struct NewBloke *b) {
    struct HitInfo hit;
    struct Point pos;
    struct Point old_f;
    struct Point old_p;
    struct Point wp0;
    struct Point off;
    int w, h;
    int sx, sy, key;
    int queued;
    struct Person *person;

    hit.type = 0x306;
    hit.ptr = b->bloke;
    hit.field_8 = 0;
    /* car position on the playfield (x in the low dword, y in the high dword) */
    *(__int64 *)&pos = MapToPlayfieldInl(b->fx >> 8, b->fy >> 8);
    old_f = b->f;
    old_p = b->p;
    wp0 = b->wp[0];
    queued = 0;
    FindQueueEntryAtTile(b->px, b->py);
    GetTileDimensions(&w, &h);
    sx = pos.x - ((w + 1) >> 1) - (ScrollX >> 8);
    sy = pos.y - (ScrollY >> 8);
    off.x = DSchoolBlueCarData->x[b->f_b8] >> 1;
    off.y = DSchoolBlueCarData->y[b->f_b8] >> 1;
    AdjustOffsetForViewMode(&off);
    b->sx = lpConfig->view_x + off.x + sx;
    b->sy = lpConfig->view_y + off.y + sy;
    key = h + sy;
    switch (b->f_c3) {
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
    SetOverrideFrame(b->f_b9 = b->f_b8);
    SortSpriteWithCallback(DSCarSprite, b->sx, b->sy, key, 0, (void (*)(void *))FUN_00402550, b, &hit);
    ClearOverridePalette();
    ClearOverrideFrame();

    /* place the driver in the car */
    b->bloke->pos.x = sx;
    b->bloke->pos.y = sy;
    b->bloke->dir = (b->f_b8 + 6) & 15;
    person = Find3DPersonFromBloke(b->bloke);
    person->sort_id = key;
    person->screen.x = lpConfig->view_x + b->bloke->pos.x + 0x10;
    person->screen.y = lpConfig->view_y + b->bloke->pos.y + 8;
    AdjustBlokePosition(&person->screen);
    SetPersonYawFromDir16(person, b->bloke->dir);

    if (FUN_00402490((struct NewBloke *)b) != NULL) {
        /* blocked by another car: slow down and wait */
        b->f_c8 = b->f_c6 >> 1;
        b->f_bc++;
        if (b->f_bc <= 0x200 || b->f_c4 != 0) {
            return;
        }
    } else {
        b->f_bc = 0;
        if (b->f_c8 < b->f_c6) {
            b->f_c8 += 0x40;
        }
        FUN_004019c0((struct NewBloke *)b);
        b->fx += b->velX;
        b->fy += b->velY;
        b->px = (b->fx + 0x10000) >> 16;
        b->py = (b->fy + 0x10000) >> 16;
        if (FUN_00402430((struct PairArg *)&b->p, (struct PairArg *)&old_p) == 0) {
            b->p = old_p;
            b->f = old_f;
            return;
        }
    }

    /* not yet crossed the waypoint (sign of both deltas unchanged) */
    if (((((wp0.x - b->fx) ^ (wp0.x - old_f.x)) | ((wp0.y - b->fy) ^ (wp0.y - old_f.y))) & 0x80000000) == 0) {
        if (b->f_bb != 0 && (wp0.x != b->fx || wp0.y != b->fy)) {
            return;
        }
    }
    if (b->f_bb == 0) {
        queued = 1;
        if (b->f_c4 == 0) {
            b->f_c2 = 0;
            if (b->f_c0 != 0) {
                b->f_c4 = PickQueueTurn(b->id, &b->tp, b->f_ba);
            } else {
                b->f_c4 = GetQueueTurn(b->id, &b->tp, b->f_ba);
            }
            if (b->f_c4 == 0) {
                b->f_c8 = 0;
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
    if (queued == 0) {
        FUN_00401cd0((struct NewBloke *)b);
    }
}

// FUNCTION: LEGOLAND 0x00402c10
void FUN_00402c10(void) {
    struct NewBloke *cur = (struct NewBloke *)DAT_004c10d4;
    struct NewBloke *next;
    struct RideQueueEntry *tile;

    while (cur != 0) {
        next = cur->next;

        if (cur->f_be != 0) {
            cur->f_be--;
        }
        if (cur->f_c0 != 0) {
            cur->f_c0--;
        }

        tile = (struct RideQueueEntry *)FindQueueEntryAtTile(cur->px, cur->py);

        if ((tile == 0 && cur->f_c2 == 0) || cur->f_be == 0) {
            FUN_00401c60((struct NewBloke *)cur);
            if (tile != 0) {
                tile->field_1c--;
            }
        } else {
            FUN_00402780((struct NewBloke *)cur);
        }

        cur = next;
    }
}
