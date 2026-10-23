#include <windows.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include "globals.h"
#include "legoland.h"

#include <float.h>
#include <math.h>
#include "bloke.h"
#include "castle.h"
#include "debug_alloc.h"
#include "draw.h"
#include "gamemap.h"
#include "interface.h"
#include "llidb.h"
#include "man3d.h"
#include "map_object.h"
#include "obj_instance.h"
#include "objclass.h"
#include "path_control.h"
#include "print_sprite.h"
#include "render3d.h"
#include "screens.h"
#include "tilemap.h"
#include "timer.h"

struct ClearRect {
    int left;
    int top;
    int right;
    int bottom;
};

struct EdgeVert {
    int f[5];
};

struct EdgePair {
    int a;
    int b;
};

struct EdgeObj {
    unsigned short color;
    unsigned short pad_2;
    int npts;
    int nedges;
    struct EdgePair edges[12];
    float pts[8][3];
};

struct EdgeSet {
    unsigned char pad_0[8];
    int count;
    unsigned char pad_c[4];
    struct EdgeVert *base;
    struct EdgePair *pairs;
};

#include "../../port/port_asm.h"
#include "image_sprite.h"
// FUNCTION: LEGOLAND 0x0041cc50
unsigned int GetOppositeDirection(unsigned int dir) {
    if (dir == 1) return 4;
    if (dir == 4) return 1;
    if (dir == 2) return 8;
    if (dir == 8) return 2;
    if (dir == 0xffffffff) return 1;
    return 0;
}

// FUNCTION: LEGOLAND 0x0041cc90
unsigned int BitFromIndex(unsigned int shift) {
    return 1U << shift;
}

// FUNCTION: LEGOLAND 0x0041cca0
unsigned int DirMaskToIndex(unsigned int dir) {
    if (dir == 1) return 0;
    if (dir == 4) return 2;
    if (dir == 2) return 1;
    if (dir == 8) return 3;
    return 0;
}

// FUNCTION: LEGOLAND 0x0041cce0
void GetFootprintRectAt(short *pt, struct Ride *ride, int *out) {
    int *rect = ride->footprint.v;
    out[0] = pt[0] + rect[0];
    out[2] = pt[0] + rect[2];
    out[1] = pt[1] + rect[1];
    out[3] = pt[1] + rect[3];
}

// FUNCTION: LEGOLAND 0x0041cd20
unsigned int IsAreaListClearAt(short *a, struct AreaNode *b) {
    return IsAreaListClear(a, b);
}

// FUNCTION: LEGOLAND 0x0041cd40
int FindDoorPointIndex(unsigned int *val, unsigned int *rec) {
    int i;
    unsigned int bit = 1;

    for (i = 0; i <= 3; i++) {
        if ((bit & rec[0]) != 0 && *val == rec[i + 1]) {
            return i;
        }
        bit <<= 1;
    }
    return -1;
}

// FUNCTION: LEGOLAND 0x0041cd80
void FindOpenDoorPoints(short *pos, struct DirPoints *out, unsigned int mask) {
    int i;
    unsigned int bit = 1;

    out->mask = 0;
    for (i = 0; i * 4 + 4 <= 16; i++) {
        if ((mask & bit) != 0) {
            out->pt[i][0] = pos[0] + PieceNeighbourOffsets[i][0];
            out->pt[i][1] = pos[1] + PieceNeighbourOffsets[i][1];
            if (IsAreaListClearAt(&out->pt[i][0], &DAT_004b5570) == 1) {
                out->mask |= bit;
            }
        }
        bit <<= 1;
    }
}

// FUNCTION: LEGOLAND 0x0041ce10
void ResetPieceEnd(struct PieceEnd *anim) {
    anim->link = 0;
    anim->dir = 0xffffffff;
    anim->height = 0;
}

// FUNCTION: LEGOLAND 0x0041ce30
void InitPieceHead(struct PieceHead *pair) {
    pair->flags = 0;
    pair->x = 0;
    pair->y = 0;
    pair->cls = 0;
    pair->owner = 0;
    ResetPieceEnd(&pair->end_b);
    ResetPieceEnd(&pair->end_a);
}

// FUNCTION: LEGOLAND 0x0041ce60
unsigned int GetPieceHeadDirPairIndex(struct PieceHead *pair) {
    unsigned int a = pair->end_a.dir;
    unsigned int lo;
    unsigned int b;

    if (a == 0xffffffff) {
        a = GetOppositeDirection(pair->end_b.dir);
    }
    lo = DirMaskToIndex(a);
    b = pair->end_b.dir;
    if (b == 0xffffffff) {
        b = GetOppositeDirection(pair->end_a.dir);
    }
    return (DirMaskToIndex(b) << 2) | lo;
}

// FUNCTION: LEGOLAND 0x0041ceb0
void AddPathSegMapObject(struct PathSeg *obj) {
    int pt[2];

    pt[0] = obj->x;
    pt[1] = obj->y;
    AddMapObjectIfEnabled(obj->ride->element, pt);
}

// FUNCTION: LEGOLAND 0x0041cee0
unsigned int CountPrevSegs(struct PathSeg *node) {
    unsigned int count = 0;
    while (node != NULL) {
        if (node->flags & 0x6) break;
        node = node->prev;
        count++;
    }
    return count;
}

// FUNCTION: LEGOLAND 0x0041cf00
unsigned int CountNextSegs(struct PathSeg *node) {
    unsigned int count = 0;
    while (node != NULL) {
        if (node->flags & 0x6) break;
        node = node->next;
        count++;
    }
    return count;
}

// FUNCTION: LEGOLAND 0x0041cf20
void CountPathSegsAround(struct PathSeg *src, unsigned int *out_b, unsigned int *out_a) {
    *out_a = 0xffffffff;
    *out_b = 0xffffffff;
    if (src->prev != NULL) {
        *out_a = CountPrevSegs(src->prev);
    }
    if (src->next != NULL) {
        *out_b = CountNextSegs(src->next);
    }
}

// FUNCTION: LEGOLAND 0x0041cf70
void CountCastleCoasterSegs(struct CastleState *src, int *out_a, int *out_b) {
    *out_a = 0;
    *out_b = 0;
    if (src->flags == 2) return;
    *out_b = CountNextSegs(src->f_c0);
    *out_a = CountPrevSegs(src->f_a8);
}

// FUNCTION: LEGOLAND 0x0041cfc0
void PieceFillOpenEnd(struct PathSeg *host) {
    host->cls->method_1c(host);
}

// FUNCTION: LEGOLAND 0x0041cfd0
unsigned int PieceSetHeights(struct PathSeg *host, float arg) {
    return host->cls->method_20(host, arg);
}

// FUNCTION: LEGOLAND 0x0041cff0
struct SegBlob *GetPieceCurve(struct PathSeg *obj, struct FVec3 *pos) {
    if (obj->flags & 1) {
        *pos = obj->vec;
        return &obj->blob;
    }
    MapPosToWorld(&obj->pos, obj->f18, pos);
    return obj->cls->method_24(obj);
}

// FUNCTION: LEGOLAND 0x0041d060
struct PathSeg *FindPathSegByXY(struct CastleState *host, unsigned int *key) {
    if (host->flags & 0x2) {
        struct PathSeg *current = (struct PathSeg *)&host->piece;
        unsigned int search = *key;
        if (current->xy == search) {
            return current;
        }
        while (1) {
            current = current->next;
            if (current == (struct PathSeg *)&host->piece) {
                break;
            }
            if (current->xy == search) {
                return current;
            }
        }
    } else {
        struct PathSeg *current = host->f_c0;
        unsigned int search = *key;
        if (current->xy == search) {
            return current;
        }
        while (1) {
            current = current->next;
            if (current == NULL) {
                break;
            }
            if (current->xy == search) {
                return current;
            }
        }
    }
    return NULL;
}

// FUNCTION: LEGOLAND 0x0041d0b0
int IsPointInPathSegFootprint(struct PathSeg *obj, short *pt) {
    int r[5];

    GetFootprintRectAt(&obj->x, obj->ride, r);
    if (pt[0] >= r[0] && pt[0] <= r[2] && pt[1] >= r[1] && pt[1] <= r[3]) {
        return 1;
    }
    return 0;
}

// FUNCTION: LEGOLAND 0x0041d100
struct PathSeg *FindPathSegAt(struct CastleState *host, struct Ride *ride, short *pt) {
    struct PathSeg *node;

    if (host->flags & 2) {
        node = (struct PathSeg *)&host->piece;
        do {
            if (node->ride == ride && IsPointInPathSegFootprint(node, pt)) {
                return node;
            }
            node = node->next;
        } while (node != (struct PathSeg *)&host->piece);
        return 0;
    }
    node = host->f_c0;
    do {
        if (node->ride == ride && IsPointInPathSegFootprint(node, pt)) {
            return node;
        }
        node = node->next;
    } while (node);
    return 0;
}

// FUNCTION: LEGOLAND 0x0041d170
unsigned int FindPathSegDoorAPoints(struct PathSeg *obj, struct DirPoints *pts) {
    return FindPathSegDoorPoints(obj, &obj->cls->door_a, pts);
}

// FUNCTION: LEGOLAND 0x0041d190
unsigned int FindPathSegDoorBPoints(struct PathSeg *obj, struct DirPoints *pts) {
    return FindPathSegDoorPoints(obj, &obj->cls->door_b, pts);
}

// FUNCTION: LEGOLAND 0x0041d1b0
void ClearDirPoints(struct DirPoints *pts) {
    pts->mask = 0;
}

// FUNCTION: LEGOLAND 0x0041d1c0
void FUN_0041d1c0(void *unused) {
}

// FUNCTION: LEGOLAND 0x0041d1d0
unsigned int FindPathSegDoorPoints(struct PathSeg *obj, struct PieceDoor *door, struct DirPoints *pts) {
    short sum[2];

    ClearDirPoints(pts);
    sum[0] = obj->x + door->x;
    sum[1] = obj->y + door->y;
    FindOpenDoorPoints(sum, pts, door->mask);
}

// FUNCTION: LEGOLAND 0x0041d210
int MatchPieceToCoasterEnds(short *pos, struct SprEnt *ent, struct PieceClass *info) {
    short p[2];
    int r;

    if (CastleCoaster.flags != 2) {
        ent->owner = (struct CastleState *)&CastleCoaster;
        p[0] = info->door_b.x + pos[0];
        p[1] = info->door_b.y + pos[1];
        r = FindDoorPointIndex((unsigned int *)p, &CastleCoaster.pts_a.mask);
        ent->id_a = r;
        if (r != -1) {
            CastleCoaster.f_a8->dir_b = BitFromIndex(r);
        }
        p[0] = info->door_a.x + pos[0];
        p[1] = info->door_a.y + pos[1];
        r = FindDoorPointIndex((unsigned int *)p, &CastleCoaster.pts_b.mask);
        ent->id_b = r;
        if (r != -1) {
            CastleCoaster.f_c0->dir_a = BitFromIndex(r);
        }
        if (ent->id_a != -1 || ent->id_b != -1) {
            return 1;
        }
    }
    return 0;
}

// FUNCTION: LEGOLAND 0x0041d2e0
int CheckAnchorPiecePlacement(struct PieceClass *info, short *pos, struct SprEnt *ent) {
    if (MatchPieceToCoasterEnds(pos, ent, info)) {
        if (ent->id_a != -1) {
            ent->h_a = FindReachableAnchor(ent->owner->f_a8, info->f8);
            if (!ent->h_a) {
                return 0;
            }
        }
        if (ent->id_b != -1) {
            ent->h_b = FindReachableAnchor(ent->owner->f_c0, info->f4);
            if (!ent->h_b) {
                return 0;
            }
        }
        return 1;
    }
    return 0;
}

// FUNCTION: LEGOLAND 0x0041d350
int CheckTrackPiecePlacement(struct PieceClass *info, short *pos, struct SprEnt *ent) {
    if (MatchPieceToCoasterEnds(pos, ent, info)) {
        if (ent->id_a == -1 || ent->id_b == -1) {
            return 1;
        }
        return HasStraightForClosingSlope(info, ent->owner, &ent->h_a, &ent->h_b) != 0;
    }
    return 0;
}

// FUNCTION: LEGOLAND 0x0041d3b0
struct SprEnt *CheckCoasterPiecePlacement(struct PieceClass *cls, struct Int16Pair *pos) {
    int r;

    if (cls->flags & 1) {
        r = CheckAnchorPiecePlacement(cls, &pos->x, &CoasterPlacement);
    } else {
        r = CheckTrackPiecePlacement(cls, &pos->x, &CoasterPlacement);
    }
    CoasterPlacement.field_0 = 0;
    if (r) {
        if (CoasterPlacement.id_a != -1) {
            CoasterPlacement.field_0 = 1;
        }
        if (CoasterPlacement.id_b != -1) {
            CoasterPlacement.field_0 |= 2;
        }
        return &CoasterPlacement;
    }
    CoasterPlacement.field_0 = 0;
    return &CoasterPlacement;
}

// FUNCTION: LEGOLAND 0x0041d430
void LinkPathSegs(struct PathSeg *a, struct PathSeg *b) {
    a->next = b;
    b->prev = a;
}

// FUNCTION: LEGOLAND 0x0041d440
void AttachPathSegToCoaster(struct PathSeg *obj, struct SprEnt *ent) {
    struct CastleState *owner = ent->owner;
    int *r;

    obj->owner = owner;
    if (ent->id_a != -1) {
        LinkPathSegs(owner->f_a8, obj);
        owner->f_a8->dir_b = BitFromIndex(ent->id_a);
        obj->dir_a = GetOppositeDirection(owner->f_a8->dir_b);
        PieceSetHeights(obj, owner->f_a8->f24);
        owner->f_a8 = obj;
        FindPathSegDoorAPoints(obj, &owner->pts_a);
    }
    if (ent->id_b != -1) {
        LinkPathSegs(obj, owner->f_c0);
        owner->f_c0->dir_a = BitFromIndex(ent->id_b);
        obj->dir_b = GetOppositeDirection(owner->f_c0->dir_a);
        PieceSetHeights(obj, owner->f_c0->f18);
        owner->f_c0 = obj;
        FindPathSegDoorBPoints(obj, &owner->pts_b);
    }
    if (ent->id_a != -1 && ent->id_b != -1) {
        owner->flags = 2;
        OnCoasterRingClosed(owner);
    } else {
        PieceFillOpenEnd((struct PathSeg *)obj);
        FUN_0041d1c0(&owner->pts_a);
        FUN_0041d1c0(&owner->pts_b);
    }
    AddPathSegMapObject(obj);
    r = obj->ride->footprint.v;
    obj->fp.v[0] = r[0] - owner->piece.x + obj->x;
    obj->fp.v[2] = r[2] - owner->piece.x + obj->x;
    obj->fp.v[1] = r[1] - owner->piece.y + obj->y;
    obj->fp.v[3] = r[3] - owner->piece.y + obj->y;
    obj->fp.next = 0;
}

// FUNCTION: LEGOLAND 0x0041d5b0
struct PathSeg *NewTrackPiece(struct Ride *ride, struct PieceClass *cls, struct Int16Pair *pos, struct SprEnt *ent) {
    struct PathSeg *obj = MemAlloc(sizeof(struct PathSeg), 0, 0, 0);

    if (obj == NULL) {
        return 0;
    }
    InitPieceHead((struct PieceHead *)obj);
    obj->ride = ride;
    obj->owner = ent->owner;
    obj->pos = *pos;
    obj->cls = cls;
    AttachPathSegToCoaster(obj, ent);
    if (ent->id_a != -1 && ent->id_b != -1) {
        InterpolateCoasterHeights(ent->h_a, ent->h_b);
    }
    obj->cls->method_28(obj);
    return obj;
}

// FUNCTION: LEGOLAND 0x0041d630
struct PathSeg *NewAnchorPiece(struct Ride *ride, struct PieceClass *cls, struct Int16Pair *pos, struct SprEnt *ent) {
    struct PathSeg *obj = MemAlloc(sizeof(struct PathSeg), 0, 0, 0);

    if (obj == NULL) {
        return 0;
    }
    InitPieceHead((struct PieceHead *)obj);
    obj->ride = ride;
    obj->owner = ent->owner;
    obj->pos = *pos;
    obj->cls = cls;
    AttachPathSegToCoaster(obj, ent);
    if (ent->id_a != -1) {
        InterpolateCoasterHeights(ent->h_a, obj);
    }
    if (ent->id_b != -1) {
        InterpolateCoasterHeights(obj, ent->h_b);
    }
    obj->cls->method_28(obj);
    return obj;
}

// FUNCTION: LEGOLAND 0x0041d6c0
void SetCoasterPieceCount(unsigned int value) {
    CoasterPieceCount = value;
}

// FUNCTION: LEGOLAND 0x0041d6d0
unsigned int AddCoasterPieceCount(unsigned int delta) {
    CoasterPieceCount += delta;
    return CoasterPieceCount;
}

// FUNCTION: LEGOLAND 0x0041d6f0
unsigned int GetCoasterPieceCount(void) {
    return CoasterPieceCount;
}

// FUNCTION: LEGOLAND 0x0041d700
struct PathSeg *AddCoasterPiece(struct Ride *ride, struct PieceClass *cls, struct Int16Pair *pos) {
    struct SprEnt *result = CheckCoasterPiecePlacement(cls, pos);
    if (result->field_0 != 0) {
        AddCoasterPieceCount(1);
        if ((cls->flags & 1) != 0) {
            return NewAnchorPiece(ride, cls, pos, &CoasterPlacement);
        } else {
            return NewTrackPiece(ride, cls, pos, &CoasterPlacement);
        }
    }
    return 0;
}

// FUNCTION: LEGOLAND 0x0041d760
void RemovePathSegMapObject(struct PathSeg *obj) {
    TileId tile;
    struct Point pt;

    tile.pos.x = (unsigned char)obj->x;
    pt.x = obj->x;
    tile.pos.y = (unsigned char)obj->y;
    pt.y = obj->y;
    BasicObjectDCalcCursor(obj->ride->element, &pt);
    RemoveMapObjectIfEnabled(obj->ride->element, tile, &QueryCursor);
}

// FUNCTION: LEGOLAND 0x0041d7c0
unsigned int IsCoasterEndPiece(struct PathSeg *value) {
    if (CastleCoaster.flags != 2) {
        if (value != CastleCoaster.f_a8) {
            if (value != CastleCoaster.f_c0) {
                return 0;
            }
        }
    }
    return 1;
}

// FUNCTION: LEGOLAND 0x0041d7f0
void RemoveCoasterPiece(void *arg) {
    struct PathSeg *obj = arg;
    struct CastleState *owner = obj->owner;

    if (obj != NULL) {
        RemovePathSegMapObject(obj);
        AddCoasterPieceCount(-1);
        if (owner->flags == 2) {
            ResetPieceEnd((struct PieceEnd *)&obj->next->dir_a);
            ResetPieceEnd((struct PieceEnd *)&obj->prev->dir_b);
            owner->f_a8 = obj->prev;
            FindPathSegDoorAPoints(obj->prev, &CastleCoaster.pts_a);
            owner->f_c0 = obj->next;
            FindPathSegDoorBPoints(obj->next, &CastleCoaster.pts_b);
            ResetCoasterHeightsBack((struct PathSeg *)owner->f_a8);
            ResetCoasterHeightsForward((struct PathSeg *)owner->f_c0);
            PieceFillOpenEnd((struct PathSeg *)owner->f_a8);
            PieceFillOpenEnd((struct PathSeg *)owner->f_c0);
            owner->flags = 1;
            OnCoasterRingOpened(owner);
        } else if (obj == owner->f_a8) {
            ResetPieceEnd((struct PieceEnd *)&obj->prev->dir_b);
            owner->f_a8 = obj->prev;
            FindPathSegDoorAPoints(obj->prev, &CastleCoaster.pts_a);
            ResetCoasterHeightsBack((struct PathSeg *)owner->f_a8);
            PieceFillOpenEnd((struct PathSeg *)owner->f_a8);
        } else {
            ResetPieceEnd((struct PieceEnd *)&obj->next->dir_a);
            owner->f_c0 = obj->next;
            FindPathSegDoorBPoints(obj->next, &CastleCoaster.pts_b);
            ResetCoasterHeightsForward((struct PathSeg *)owner->f_c0);
            PieceFillOpenEnd((struct PathSeg *)owner->f_c0);
        }
        FUN_0041d1c0(&CastleCoaster.pts_a.mask);
        FUN_0041d1c0(&CastleCoaster.pts_b.mask);
        ((struct PieceClass *)obj->cls)->method_2c(obj);
        MemFree(obj);
    }
}

// FUNCTION: LEGOLAND 0x0041d950
void PositionTrainCarsAndBogies(void *obj, float val, void *b) {
    struct Train *host = obj;
    struct TrainCar *head;
    struct TrainCar *cur;
    struct TrainCar *next;
    struct CurveWalker elem;
    struct CurveWalker out;
    unsigned int tmp[3];
    float t;
    float f;

    next = host->head.next;
    head = &host->head;
    cur = head;
    host->motion.f4 = val;
    host->walker = *(struct CurveWalker *)b;
    PositionTrainCarAndBogies(head, (struct CurveWalker *)b, val);
    while (next != head) {
        t = cur->rear.w0.t;
        elem = cur->rear.w0.walker;
        GetTrainCarRearPos(cur, (float *)tmp);
        FindPathPosAtRangeBehind((float *)tmp, 30.0f, &elem, t, 4.8f, &out, &f);
        PositionTrainCarAndBogies(next, &out, f);
        cur = next;
        next = next->next;
    }
}

// FUNCTION: LEGOLAND 0x0041da10
void PositionTrainCars(void *obj, float val, void *b) {
    struct Train *host = obj;
    struct TrainCar *head;
    struct TrainCar *cur;
    struct TrainCar *next;
    struct CurveWalker elem;
    struct CurveWalker out;
    unsigned int tmp[3];
    float t;
    float f;

    next = host->head.next;
    head = &host->head;
    cur = head;
    host->motion.f4 = val;
    host->walker = *(struct CurveWalker *)b;
    PositionTrainCar(head, (struct CurveWalker *)b, val);
    while (next != head) {
        t = cur->rear.w0.t;
        elem = cur->rear.w0.walker;
        GetTrainCarRearPos(cur, (float *)tmp);
        FindPathPosAtRangeBehind((float *)tmp, 30.0f, &elem, t, 4.8f, &out, &f);
        PositionTrainCar(next, &out, f);
        cur = next;
        next = next->next;
    }
}

// FUNCTION: LEGOLAND 0x0041dad0
void SetTrainEnergy(struct Train *holder, float value) {
    holder->motion.f8 = value;
}

// FUNCTION: LEGOLAND 0x0041dae0
float SumTrainPotential(struct Train *obj) {
    float sum = 0.0f;
    struct TrainCar *head = &obj->head;
    struct TrainCar *cur = head;

    do {
        sum += GetTrainCarPotential(cur);
        cur = cur->next;
    } while (cur != head);
    return sum;
}

// FUNCTION: LEGOLAND 0x0041db20
void EvalTrainPotentialAndCentres(float a, struct FloatArray *out) {
    struct TrainCar *node;
    float *p;
    unsigned int n;

    n = 1;
    node = &DerivTrain->head;
    PositionTrainCars(DerivTrain, a, &DerivTrainWalker);
    out->data[0] = 0.0f;
    p = &out->data[1];
    do {
        out->data[0] += GetTrainCarPotential(node);
        GetTrainCarCenter((unsigned char *)node, (unsigned int *)p);
        node = node->next;
        n += 3;
        p += 3;
    } while (node != &DerivTrain->head);
    out->count = n;
}

// FUNCTION: LEGOLAND 0x0041db90
void GetTrainInertiaAndSlope(struct Train *obj, float *a, float *b) {
    struct TrainCar *node;
    struct CurveWalker saved;
    float savedval;
    struct FloatArray out;
    float *p;
    float sum;
    volatile float x; /* volatile: the original re-reads this from its stack slot */
    int k;

    saved = obj->walker;
    DerivTrain = obj;
    savedval = obj->motion.f4;
    DerivTrainWalker = obj->walker;
    VecDerivative(EvalTrainPotentialAndCentres, &TrainVecMath, savedval, 0.1f, &out);
    *b = out.data[0];
    *a = 0.0f;
    p = &out.data[1];
    node = &obj->head;
    do {
        sum = FLOAT_004ab390;
        for (k = 0; k < 3; k++) {
            x = *p++;
            sum += x * x;
        }
        *a += GetTrainCarMass(node) * sum * 2.5201562e-06f;
        node = node->next;
    } while (node != &obj->head);
    obj->walker = saved;
    obj->motion.f4 = savedval;
    TrainInertiaHistory[TrainInertiaHistoryIndex & 0x3f] = *a;
    TrainInertiaHistoryIndex++;
}

// FUNCTION: LEGOLAND 0x0041dca0
float GetTrainParamSpeed(struct Train *param) {
    float a;
    float b;
    float t;
    float r;

    t = SumTrainPotential(param);
    GetTrainInertiaAndSlope(param, &a, &b);
    r = (param->motion.f8 - t) * 2.0f / a;
    if (r > FLOAT_004ab390) {
        return (float)sqrt(r);
    }
    return FLOAT_004ab390;
}

// FUNCTION: LEGOLAND 0x0041dd00
float ParamSpeedToTrackSpeed(struct Train *param, float result) {
    struct FVec3 v;
    float t;

    GetWalkerTangentBelow(&param->walker, 1, *(unsigned int *)&param->motion.f4, 0.0f, &v);
    t = lego_sqrtf(Vec3Dot(&v, &v));
    t = t * result;
    return t * FLOAT_004ab404;
}

// FUNCTION: LEGOLAND 0x0041dd50
float GetTrainTrackSpeed(struct Train *param) {
    float result = GetTrainParamSpeed(param);
    return ParamSpeedToTrackSpeed(param, result);
}

// FUNCTION: LEGOLAND 0x0041dd70
float SumTrainCarMasses(struct Train *obj) {
    float sum = 0.0f;
    struct TrainCar *head = &obj->head;
    struct TrainCar *cur = head;

    do {
        sum += GetTrainCarMass(cur);
        cur = cur->next;
    } while (cur != head);
    return sum;
}

// FUNCTION: LEGOLAND 0x0041ddb0
float GetTrainKineticEnergy(struct Train *param_1, float param_2) {
    float result = SumTrainCarMasses(param_1);
    result = result * param_2;
    result = result * param_2;
    result = result * 0.5f;
    return result;
}

// FUNCTION: LEGOLAND 0x0041ddd0
void TrainIntegratorStore(struct Integrator *a, struct FloatArray *b) {
    struct Train *e = a->owner;

    e->motion.kind = 2;
    PositionTrainCars(e, b->data[0], &e->walker);
    SetTrainEnergy(e, b->data[1]);
}

// FUNCTION: LEGOLAND 0x0041de10
void TrainIntegratorDeriv(struct Integrator *obj, float unused, struct FloatArray *out) {
    struct Train *e = obj->owner;
    float a;
    float b;
    float t;

    out->count = 2;
    t = SumTrainPotential(e);
    GetTrainInertiaAndSlope(e, &a, &b);
    t = (e->motion.f8 - t) * 2.0f / a;
    if (t > FLOAT_004ab390) {
        t = lego_sqrtf(t);
    } else {
        t = FLOAT_004ab390;
    }
    out->data[0] = t;
    out->data[1] = 0.0f;
    if (b > FLT_MIN) {
        if (ParamSpeedToTrackSpeed(e, t) < 0.05) {
            out->data[1] = b;
        }
    }
}

// FUNCTION: LEGOLAND 0x0041dec0
void InitTrainIntegrator(struct Train *self) {
    InitIntegrator(&self->integ, 2);
    self->integ.store = TrainIntegratorStore;
    self->integ.state = &self->motion;
    self->integ.owner = self;
    self->integ.deriv = TrainIntegratorDeriv;
    self->motion.kind = 2;
}

struct BisectPair {
    float p;
    unsigned int q;
};

struct BisectOut {
    float p;
    unsigned int q;
    unsigned int r;
    unsigned int w;
};

// FUNCTION: LEGOLAND 0x0041df00
float BisectTrainSegmentEnd(struct Train *obj, float t) {
    struct BisectPair s;
    struct AnimOut anim;
    float lo;
    float hi;

    /* bisection on t in [0, t]: t itself holds the midpoint being tried */
    anim = obj->motion;
    hi = t;
    lo = 0.0f;
    s.p = 0.0f;
    s.q = anim.kind;
    while (hi - lo > DAT_004ab418) {
        t = (hi + lo) * 0.5f;
        PositionTrainCars(obj, anim.f4, &obj->walker);
        SetTrainEnergy((struct Train *)obj, anim.f8);
        IntegratorStepRK4(&obj->integ, &s.p, t);
        if (obj->motion.f4 > obj->walker.node->end) {
            hi = t;
        } else {
            lo = t;
        }
    }
    PositionTrainCars(obj, anim.f4, &obj->walker);
    SetTrainEnergy((struct Train *)obj, anim.f8);
    t = (hi + lo) * 0.5f;
    IntegratorStepRK4(&obj->integ, &s.p, t);
    return t;
}

// FUNCTION: LEGOLAND 0x0041e000
float StepTrainToSegmentEnd(struct Train *obj, float total) {
    struct BisectOut s;
    struct AnimOut anim;
    int i = 0;
    int n = (int)ceil(total * 70.0f);
    float acc = 0.0f;
    float dt;

    s.p = 0.0f;
    dt = total / n;
    for (i = 0; i < n; i++) {
        anim = obj->motion;
        s.q = anim.kind;
        IntegratorStepRK4(&obj->integ, &s.p, dt);
        if (obj->motion.f4 > obj->walker.node->end) {
            PositionTrainCars(obj, anim.f4, &obj->walker);
            SetTrainEnergy((struct Train *)obj, anim.f8);
            return BisectTrainSegmentEnd(obj, dt) + acc;
        }
        acc += dt;
    }
    return total;
}

// FUNCTION: LEGOLAND 0x0041e0e0
float StepTrainLeaving(struct Train *self, float time_left) {
    return self->track->method_110(self, time_left);
}

// FUNCTION: LEGOLAND 0x0041e100
float StepTrainArriving(struct Train *self, float time_left) {
    void (*saved)(struct Integrator *, float, struct FloatArray *) = self->integ.deriv;
    float used = self->track->method_114(self, time_left);

    self->integ.deriv = saved;
    return used;
}

// FUNCTION: LEGOLAND 0x0041e130
void StepTrainForFrame(void *host) {
    /* Port [library:asm]: the original is inline asm (rdtsc profiling). Steps the train through the game time
     * elapsed since the last call (in seconds, capped at 0.8), moving on to the next curve stretch whenever a
     * step finishes before the time is used up. */
    struct Train *obj = (struct Train *)host;
    unsigned int now = GetGameTimer();
    float elapsed = ((float)(int)now - (float)obj->field_4) * 0.001f;
    struct CastleState *track = obj->track;
    unsigned int start;
    float done = 0.0f;
    float limit;
    struct PathSeg *key;

    if (elapsed > 0.8f) {
        elapsed = 0.8f;
    }
    start = PortTimestamp();
    limit = elapsed - 1e-6f;
    for (;;) {
        key = obj->walker.piece;
        if (obj->flags & 8) {
            done += StepTrainLeaving(obj, elapsed - done);
        } else if (obj->flags & 0x10) {
            done += StepTrainArriving(obj, elapsed - done);
        } else {
            done += StepTrainToSegmentEnd(obj, elapsed - done);
        }
        if (!(done < limit)) {
            break;
        }
        CurveWalkerNext(&obj->walker);
        obj->motion.f4 = obj->walker.node->start;
        if (key != obj->walker.piece) {
            if (obj->flags & 8) {
                obj->flags = (obj->flags & ~8u) | 4;
            } else if ((obj->flags & 4) && obj->walker.piece == (struct PathSeg *)&track->piece) {
                obj->flags = (obj->flags & ~4u) | 0x10;
            }
        }
    }
    obj->field_4 = now;
    TrainStepCycles += PortTimestamp() - start;
}

// FUNCTION: LEGOLAND 0x0041e240
void UpdateTrainStartDelay(struct Train *timed) {
    int now = GetGameTimer();
    if (now > timed->field_8) {
        timed->flags &= ~0x40;
    }
    timed->field_4 = now;
}

// FUNCTION: LEGOLAND 0x0041e260
unsigned int PlaceInEmptyTrainSeat(struct Train *host, void *arg) {
    struct TrainCar *node = &host->head;

    while (PlaceInEmptySeatOfCar(node, arg) == 0) {
        node = node->next;
        if (node == &host->head) {
            return 0;
        }
    }
    return 1;
}

// FUNCTION: LEGOLAND 0x0041e2b0
struct TrainSeat *FindEmptyTrainSeat(struct Train *host) {
    struct TrainCar *node = &host->head;
    struct TrainSeat *r;

    while ((r = FindEmptySeatInCar(node)) == 0) {
        node = node->next;
        if (node == &host->head) {
            return 0;
        }
    }
    return r;
}

// FUNCTION: LEGOLAND 0x0041e2f0
struct CastleVisitor *FindTrainOccupant(struct Train *host) {
    struct TrainCar *node = &host->head;
    struct CastleVisitor *r;
    while ((r = FindOccupantInCar(node)) == 0) {
        node = node->next;
        if (node == &host->head) {
            return 0;
        }
    }
    return r;
}

// FUNCTION: LEGOLAND 0x0041e330
void ForEachRingNode(struct Train *host, void (*visit)(struct TrainCar *car)) {
    struct TrainCar *start = &host->head;
    struct TrainCar *node = start;
    do {
        visit(node);
        node = node->next;
    } while (node != start);
}

// FUNCTION: LEGOLAND 0x0041e360
void AddAllTrainCarLinks(struct Train *host) {
    ForEachRingNode(host, AddTrainCarLink);
}

// FUNCTION: LEGOLAND 0x0041e380
void UnlinkAllTrainCars(struct Train *host) {
    ForEachRingNode(host, UnlinkTrainCar);
}

// FUNCTION: LEGOLAND 0x0041e3a0
void UpdateAllTrainCarBounds(struct Train *host) {
    ForEachRingNode(host, UpdateTrainCarBounds);
}

// FUNCTION: LEGOLAND 0x0041e3c0
unsigned int DrawTrainCarOnDrawPiece(struct TrainCar *param) {
    return DrawTrainCarOnPiece(param, TrainDrawPiece);
}

// FUNCTION: LEGOLAND 0x0041e3e0
void DrawTrainCarsOnPiece(struct Train *host, struct PathSeg *value) {
    TrainDrawPiece = value;
    ForEachRingNode(host, (void (*)(struct TrainCar *))DrawTrainCarOnDrawPiece);
}

// FUNCTION: LEGOLAND 0x0041e400
void ClearAllTrainCarsDrawn(struct Train *host) {
    ForEachRingNode(host, ClearTrainCarDrawn);
}

// FUNCTION: LEGOLAND 0x0041e420
void AddTrainCar(struct Train *self, unsigned int flag) {
    struct TrainCar *node = AllocTrainCar(flag);
    struct TrainCar *head;

    if (node != NULL) {
        self->head.next->prev = node;
        head = &((struct Train *)self)->head;
        node->next = head->next;
        head->next = node;
        node->prev = head;
    }
}

// FUNCTION: LEGOLAND 0x0041e460
void RemoveTrainCar(struct TrainCar *node, struct Train *host) {
    if (node != &host->head) {
        node->next->prev = node->prev;
        node->prev->next = node->next;
        FreeTrainCar(node);
    }
}

// FUNCTION: LEGOLAND 0x0041e4a0
unsigned int IsTrainRunning(struct Train *obj) {
    if (*(signed char *)&obj->flags & 2) {
        return 1;
    }
    return 0;
}

// FUNCTION: LEGOLAND 0x0041e4b0
unsigned int IsTrainStartDelayed(struct Train *obj) {
    if (*(signed char *)&obj->flags & 0x40) {
        return 1;
    }
    return 0;
}

// FUNCTION: LEGOLAND 0x0041e4c0
unsigned int StartTrain(struct Train *obj, unsigned int delay) {
    obj->flags |= 2;
    if (delay != 0) {
        obj->flags |= 0x40;
        obj->field_8 = GetGameTimer() + delay;
        return obj->field_8;
    }
    obj->flags &= ~0x40;
    return obj->flags;
}

// FUNCTION: LEGOLAND 0x0041e4f0
void StopTrain(struct Train *obj) {
    obj->flags &= 0xfffffffd;
    ResetTrainToStation(obj);
}

// FUNCTION: LEGOLAND 0x0041e500
void ResetTrainToStation(struct Train *obj) {
    struct CurveWalker t;
    float slot;

    obj->flags = 8;
    t.piece = (struct PathSeg *)&obj->track->piece;
    obj->track->method_10c(obj->track, &t.node, &slot, &t.pos);
    PositionTrainCarsAndBogies(obj, slot, &t);
    obj->field_4 = GetGameTimer();
    obj->motion.f8 = SumTrainPotential(obj);
    obj->motion.f8 = GetTrainKineticEnergy(obj, 0.1f) + obj->motion.f8;
}

// FUNCTION: LEGOLAND 0x0041e570
struct Train *CreateTrain(struct CastleState *arg) {
    struct Train *self = MemAlloc(sizeof(struct Train), 0, 0, 0);
    if (self == NULL) {
        return NULL;
    }
    self->track = arg;
    InitTrainCar(&self->head, 0);
    AddTrainCar(self, 2);
    AddTrainCar(self, 1);
    self->flags = 0;
    InitTrainIntegrator((struct Train *)self);
    ResetTrainToStation(self);
    AddAllTrainCarLinks((struct Train *)self);
    return self;
}

// FUNCTION: LEGOLAND 0x0041e5d0
void DestroyTrain(struct Train **param_1) {
    struct Train *host = (struct Train *)*param_1;
    struct TrainCar *node = &host->head;
    struct TrainCar *next;

    UnlinkAllTrainCars(host);
    do {
        next = node->next;
        RemoveTrainCar(node, host);
        node = next;
    } while (next != &host->head);
    MemFree(*param_1);
    *param_1 = NULL;
}

// FUNCTION: LEGOLAND 0x0041e620
void InitTrainVecMath(void) {
    InitVecMath(&TrainVecMath, 10);
}

// FUNCTION: LEGOLAND 0x0041e630
void ClearTrainCarDrawn(struct TrainCar *obj) {
    obj->flags &= 0xfffffffe;
}

// FUNCTION: LEGOLAND 0x0041e640
void SetTrainCarDrawnFlag(unsigned int *flags, unsigned int set) {
    if (set == 1) {
        *flags |= 1;
    } else {
        *flags &= 0xfffffffe;
    }
}

// FUNCTION: LEGOLAND 0x0041e660
unsigned int IsTrainCarDrawn(struct TrainCar *a) {
    if (*(signed char *)&a->flags & 1) {
        return 1;
    }
    return 0;
}

// FUNCTION: LEGOLAND 0x0041e670
unsigned int IsTrainCarOnPiece(struct TrainCar *obj, struct PathSeg *id) {
    if (obj->front.w0.walker.piece == id) {
        return 1;
    }
    return obj->rear.w0.walker.piece == id;
}

// FUNCTION: LEGOLAND 0x0041e6a0
void InitTrainCar(struct TrainCar *param, unsigned int value) {
    float v[3];

    v[1] = 0.0f;
    v[2] = -4.0f;
    param->kind = value;
    v[0] = TrainCarSeatX[value];
    InitTrainSeat(&param->seats[0], (const struct FVec3 *)v);
    v[0] = v[0] - TrainSeatSpacing;
    InitTrainSeat(&param->seats[1], (const struct FVec3 *)v);
    param->next = param;
    param->prev = param;
    param->flags = 0;
}

// FUNCTION: LEGOLAND 0x0041e720
unsigned int PlaceInEmptySeatOfCar(struct TrainCar *host, void *arg) {
    struct TrainSeat *slot = &host->seats[0];
    int i = 0;
    while (i <= 1) {
        if (slot->method_10(slot) == 0) {
            slot->method_14(slot, arg);
            return 1;
        }
        i++;
        slot++;
    }
    return 0;
}

// FUNCTION: LEGOLAND 0x0041e760
struct TrainSeat *FindEmptySeatInCar(struct TrainCar *host) {
    struct TrainSeat *slot = &host->seats[0];
    int i = 0;
    while (i <= 1) {
        if (slot->method_10(slot) == 0) {
            return slot;
        }
        i++;
        slot++;
    }
    return NULL;
}

// FUNCTION: LEGOLAND 0x0041e790
struct CastleVisitor *FindOccupantInCar(struct TrainCar *host) {
    struct TrainSeat *slot = &host->seats[0];
    int i = 0;

    while (i <= 1) {
        if (slot->method_10(slot) != 0) {
            return TakeTrainSeatOccupant(slot);
        }
        i++;
        slot++;
    }
    return 0;
}

// FUNCTION: LEGOLAND 0x0041e7c0
void FreeCarSeatOccupants(struct TrainCar *host) {
    struct TrainSeat *slot = &host->seats[0];
    int i = 2;
    do {
        slot->method_18(slot);
        slot++;
        i--;
    } while (i != 0);
}

// FUNCTION: LEGOLAND 0x0041e7e0
float GetTrainCarMass(struct TrainCar *obj) {
    return TrainCarMass;
}

// FUNCTION: LEGOLAND 0x0041e7f0
void GetTrainCarCenter(unsigned char *obj, unsigned int *out) {
    obj += offsetof(struct TrainCar, center);
    out[0] = *(unsigned int *)obj;
    out[1] = *(unsigned int *)(obj + 4);
    out[2] = *(unsigned int *)(obj + 8);
}

// FUNCTION: LEGOLAND 0x0041e810
float GetTrainCarPotential(struct TrainCar *obj) {
    return obj->weight;
}

// FUNCTION: LEGOLAND 0x0041e820
void PositionTrainCar(struct TrainCar *obj, struct CurveWalker *e, float t) {
    float vA[3];
    float vB[3];
    struct CurveWalker k;
    float f;

    SetTrainWheelPos(&obj->front.w0, e, *(unsigned int *)&t);
    GetTrainWheelPos(&obj->front.w0, 2, vA);
    FindPathPosAtRangeBehind(vA, 30.0f, &obj->front.w0.walker, obj->front.w0.t, 4.8f, &k, &f);
    SetTrainWheelPos(&obj->rear.w0, &k, *(unsigned int *)&f);
    GetTrainWheelPos(&obj->rear.w0, 2, vB);
    obj->center.x = (vA[0] + vB[0]) * 0.5f;
    obj->center.y = (vA[1] + vB[1]) * 0.5f;
    obj->center.z = (vA[2] + vB[2]) * 0.5f;
    obj->weight = GetTrainCarMass(obj) * obj->center.z * -0.00134937500115484f;
}

// FUNCTION: LEGOLAND 0x0041e8f0
void PositionTrainCarAndBogies(struct TrainCar *obj, struct CurveWalker *e, float t) {
    PositionTrainCar(obj, e, t);
    InitTrainBogie(&obj->front, &obj->front.w0.walker, obj->front.w0.t);
    InitTrainBogie(&obj->rear, &obj->rear.w0.walker, obj->rear.w0.t);
}

// FUNCTION: LEGOLAND 0x0041e930
unsigned int GetTrainCarRearPos(struct TrainCar *obj, float *param) {
    return GetTrainWheelPos(&obj->rear.w0, 2, param);
}

// FUNCTION: LEGOLAND 0x0041e950
void AddTrainCarLink(struct TrainCar *obj) {
    AddCastleLink(&obj->link);
}

// FUNCTION: LEGOLAND 0x0041e970
void UnlinkTrainCar(struct TrainCar *param_1) {
    UnlinkListLink(&param_1->link);
}

struct RectI {
    int var_0;
    int var_4;
    int var_8;
    int var_c;
};

// FUNCTION: LEGOLAND 0x0041e990
void UpdateTrainCarBounds(struct TrainCar *obj) {
    struct {
        struct RectI r;
        unsigned int loc1[3];
        unsigned int b[9];
    } s;

    GetTrainCarCenterAndAxis(obj, (float *)s.loc1);
    GetLmsBoxScreenRect(CoasterTrainCarLms[0], s.loc1, s.b, &s.r);
    ClipRectIToCastleView((struct RectI *)&obj->link, &s.r);
}

// FUNCTION: LEGOLAND 0x0041e9e0
void GetTrainCarCenterAndAxis(struct TrainCar *obj, float *out) {
    float a[3];
    float b[3];

    GetTrainWheelPos(&obj->front.w0, 2, a);
    GetTrainWheelPos(&obj->rear.w0, 2, b);
    out[0] = (a[0] + b[0]) * 0.5f;
    out[1] = (a[1] + b[1]) * 0.5f;
    out[2] = (a[2] + b[2]) * 0.5f;
    out[3] = a[0] - b[0];
    out[4] = a[1] - b[1];
    out[5] = a[2] - b[2];
    BuildUprightBasis((struct FVec3 *)(out + 3), (struct FVec3 *)(out + 3));
}

// FUNCTION: LEGOLAND 0x0041ea70
void DrawTrainCar(struct TrainCar *a) {
    struct CarXform x;

    SetModelLighting();
    DrawTrainBogie(&a->front);
    DrawTrainBogie(&a->rear);
    GetTrainCarCenterAndAxis(a, &x.pos.x);
    DrawLmsModel(CoasterTrainCarLms[a->kind], CoasterTrainCarLfm[a->kind], &x.pos, x.m, 0);
    a->seats[0].draw(&a->seats[0], &x);
    a->seats[1].draw(&a->seats[1], &x);
}

// FUNCTION: LEGOLAND 0x0041eaf0
unsigned int DrawTrainCarOnPiece(struct TrainCar *a, struct PathSeg *b) {
    if (IsTrainCarDrawn(a) == 0) {
        if (IsTrainCarOnPiece(a, b) != 0) {
            DrawTrainCar(a);
            SetTrainCarDrawnFlag(&a->flags, 1);
        }
    }
}

// FUNCTION: LEGOLAND 0x0041eb30
struct TrainCar *AllocTrainCar(unsigned int arg) {
    struct TrainCar *obj = MemAlloc(sizeof(struct TrainCar), 0, 0, 0);
    if (obj == NULL) {
        return NULL;
    }
    InitTrainCar(obj, arg);
    return obj;
}

// FUNCTION: LEGOLAND 0x0041eb60
void FreeTrainCar(void *param_1) {
    MemFree(param_1);
}

// FUNCTION: LEGOLAND 0x0041eb70
void LoadCoasterTrainCarModels(void) {
    // STRING: LEGOLAND 0x004b55d8
    CoasterTrainCarLms[0] = GetLmsByName("coastertrain.headcar");
    // STRING: LEGOLAND 0x004b55c4
    CoasterTrainCarLms[1] = GetLmsByName("coastertrain.midcar");
    // STRING: LEGOLAND 0x004b55ac
    CoasterTrainCarLms[2] = GetLmsByName("coastertrain.tailcar");
    CoasterTrainCarLfm[0] = GetLfmByName("coastertrain.headcar");
    CoasterTrainCarLfm[1] = GetLfmByName("coastertrain.midcar");
    CoasterTrainCarLfm[2] = GetLfmByName("coastertrain.tailcar");
}

// FUNCTION: LEGOLAND 0x0041ebd0
unsigned int FindCastleDispatchIndex(struct Ride *arg) {
    unsigned int index = 0;
    struct DispatchRow *row = CastleDispatchTable;
    /* [port] bounded by the table itself: the original stops at the next global, CastleDispatchTableEnd */
    while (row < CastleDispatchTable + sizeof(CastleDispatchTable) / sizeof(CastleDispatchTable[0])) {
        struct Element *target = row->element;
        if (target != NULL && arg == target->ride) {
            return index;
        }
        index++;
        row++;
    }
    return 0;
}

// FUNCTION: LEGOLAND 0x0041ec00
struct Ride *GetCastleDispatchRide(unsigned int param) {
    struct Element *target = CastleDispatchTable[param].element;
    return target->ride;
}

// FUNCTION: LEGOLAND 0x0041ec20
unsigned int CastleDispatchIndex(struct Element *param) {
    unsigned int index = 0;
    struct DispatchRow *row = CastleDispatchTable;
    /* [port] bounded by the table itself: the original stops at the next global, CastleDispatchTableEnd */
    while (row < CastleDispatchTable + sizeof(CastleDispatchTable) / sizeof(CastleDispatchTable[0])) {
        if (param == row->element) {
            return index;
        }
        row++;
        index++;
    }
    return 0;
}

// FUNCTION: LEGOLAND 0x0041ec40
struct Element *GetCastleDispatchElement(unsigned int param) {
    return CastleDispatchTable[param].element;
}

// FUNCTION: LEGOLAND 0x0041ec50
void CastleDispatchSetEditMode(struct Element *obj) {
    CastleDispatchTable[CastleDispatchIndex(obj)].fn_4(obj);
}

// FUNCTION: LEGOLAND 0x0041ec70
void CastleDispatchCalcCursor(struct Element *obj, struct Point *a, unsigned int b) {
    CastleDispatchTable[CastleDispatchIndex(obj)].fn_8(obj, a, b);
}

// FUNCTION: LEGOLAND 0x0041eca0
void CastleDispatchAddObjectByIndex(unsigned int idx, short *p) {
    int v[2];

    v[0] = p[0];
    v[1] = p[1];
    CastleDispatchTable[idx].fn_c(CastleDispatchTable[idx].element, v);
}

// FUNCTION: LEGOLAND 0x0041ece0
void CastleDispatchAddObject(struct Element *obj, int *a) {
    ((void (*)(struct Element *, int *)) * (void (**)())((char *)CastleDispatchTable + CastleDispatchIndex(obj) * sizeof(struct DispatchRow) + offsetof(struct DispatchRow, fn_c)))(obj, a);
}

// FUNCTION: LEGOLAND 0x0041ed00
void CastleDispatchDCalcCursor(struct Element *obj, struct Point *a) {
    ((void (*)(struct Element *, struct Point *)) * (void (**)())((char *)CastleDispatchTable + CastleDispatchIndex(obj) * sizeof(struct DispatchRow) + offsetof(struct DispatchRow, fn_10)))(obj, a);
}

// FUNCTION: LEGOLAND 0x0041ed50
void CastleDispatchRemoveObject(struct Element *obj, TileId a, struct Cursor *b) {
    ((void (*)(struct Element *, TileId, struct Cursor *)) * (void (**)())((char *)CastleDispatchTable + CastleDispatchIndex(obj) * sizeof(struct DispatchRow) + offsetof(struct DispatchRow, fn_14)))(obj, a, b);
}

// FUNCTION: LEGOLAND 0x0041ed80
void SetMapObjectsEnabled(unsigned int param) {
    MapObjectsEnabled = param;
}

// FUNCTION: LEGOLAND 0x0041ed90
void AddMapObjectIfEnabled(struct Element *element, int *pos) {
    if (MapObjectsEnabled != 0) {
        AddBasicObject(element, pos);
    }
}

// FUNCTION: LEGOLAND 0x0041edb0
void RemoveMapObjectIfEnabled(struct Element *element, TileId tile, struct Cursor *cursor) {
    if (MapObjectsEnabled != 0) {
        StandardRemoveObject(element, tile, cursor);
    }
}

struct KindInfo {
    unsigned char pad_0[0x1c];
    unsigned int field_1c;
};

struct KindOwner {
    unsigned char pad_0[0xc];
    struct KindInfo *field_c;
};

struct KindObj {
    struct KindOwner *owner;
    unsigned char pad_4[0x8];
    unsigned short flags;
};

// FUNCTION: LEGOLAND 0x0041ede0
unsigned int IsMapElementBlocking(struct KindObj *obj) {
    struct KindOwner *owner;
    struct KindInfo *info;
    unsigned short flags;

    if (obj == NULL) {
        return 1;
    }
    flags = obj->flags;
    if ((flags & 0x8f8) == 0) {
        return 0;
    }
    if (flags & 0x40) {
        return 1;
    }
    owner = obj->owner;
    if (owner != NULL) {
        info = owner->field_c;
    } else {
        info = NULL;
    }
    if ((flags & 0x8) && owner != NULL && info == PathControlObject) {
        return 0;
    }
    if (flags & 0x800) {
        return 0;
    }
    if ((flags & 0x88) && info != NULL && (info->field_1c & 0x200000)) {
        return 0;
    }
    return 1;
}

// FUNCTION: LEGOLAND 0x0041ee40
unsigned int IsAreaListClear(short *pos, struct AreaNode *b) {
    struct AreaNode *node;
    struct MapElement *elem;
    int x;
    int y;

    if (MapObjectsEnabled != 0) {
        node = b;
        do {
            for (y = node->y + pos[1]; y < node->h + pos[1]; y++) {
                for (x = node->x + pos[0]; x < node->w + pos[0]; x++) {
                    if (x >= 0 && x < lpConfig->width && y >= 0 && y < lpConfig->height) {
                        elem = &GameMap[y][x];
                    } else {
                        elem = NULL;
                    }
                    if (IsMapElementBlocking((struct KindObj *)elem) != 0) {
                        return 0;
                    }
                }
            }
            node = node->next;
        } while (node != NULL);
    }
    return 1;
}

// FUNCTION: LEGOLAND 0x0041ef00
int FUN_0041ef00(void) {
    return 1;
}

struct PlaneSet {
    int count;
    float p[4][3];
};

// FUNCTION: LEGOLAND 0x0041ef20
void SetClipRect(int a, int b, int c, int d) {
    struct RectI r;

    r.var_0 = b;
    r.var_4 = a;
    r.var_c = c;
    r.var_8 = d;
    SetClipPlanesFromRect((int *)&r, ClipPlanes);
}

// FUNCTION: LEGOLAND 0x0041ef60
void *ClipTriangle(void *verts, int *out, unsigned int code, int n) {
    int sel = 0;

    if (code == 0xf) {
        *out = 3;
        return verts;
    }
    SetClipVertexFormat(n + 1);
    if ((char)(code & 3) < 3) {
        sel = 1;
    }
    if ((char)(code & 0xc) < 0xc) {
        sel |= 2;
    }
    switch (sel) {
    case 3:
        return ClipPolygonToPlanes(3, verts, out, 4, ClipPlanes->p[0]);
    case 2:
        return ClipPolygonToPlanes(3, verts, out, 2, ClipPlanes->p[2]);
    case 1:
        return ClipPolygonToPlanes(3, verts, out, 2, ClipPlanes->p[0]);
    }
    return out;
}

// FUNCTION: LEGOLAND 0x0041f030
void SetClipVertexFormat(unsigned int param) {
    ClipVertexStride = 0x1c;
    ClipVertexLastField = param;
}

union FloatBits {
    float f;
    unsigned int u;
};

struct FDist {
    union FloatBits mag;
    unsigned int sign;
};

// FUNCTION: LEGOLAND 0x0041f050
int ClipPolygonToPlane(int n, int **src, int **dst, int **cursor, float *pl) {
    struct FDist m, pm;
    int count = 0;
    int *pool = *cursor;
    int i, k;
    int *prev, *cur;
    float t;
    int code;

    src[n] = src[0];
    cur = src[0];
    m.mag.f = pl[2] - (cur[2] * pl[0] + cur[1] * pl[1]);
    code = m.mag.u & 0x80000000;
    m.mag.u &= 0x7fffffff;
    for (i = 1; i <= n; i++) {
        pm.mag = m.mag;
        prev = cur;
        cur = src[i];
        code = (code >> 1) & 0x40000000;
        m.mag.f = pl[2] - (cur[2] * pl[0] + cur[1] * pl[1]);
        code |= m.mag.u & 0x80000000;
        m.mag.u &= 0x7fffffff;
        switch (code) {
        case 0x40000000:
            t = pm.mag.f / (pm.mag.f + m.mag.f);
            *dst++ = prev;
            for (k = 0; k <= ClipVertexLastField; k++) {
                pool[k] = prev[k] + (int)((cur[k] - prev[k]) * t);
            }
            *dst++ = pool;
            pool = (int *)((char *)pool + ClipVertexStride);
            count += 2;
            break;
        case 0xc0000000:
            *dst++ = prev;
            count++;
            break;
        case 0x80000000:
            t = m.mag.f / (pm.mag.f + m.mag.f);
            for (k = 0; k <= ClipVertexLastField; k++) {
                pool[k] = cur[k] + (int)((prev[k] - cur[k]) * t);
            }
            *dst++ = pool;
            pool = (int *)((char *)pool + ClipVertexStride);
            count++;
            break;
        }
    }
    *cursor = pool;
    return count;
}

// FUNCTION: LEGOLAND 0x0041f2b0
int **ClipPolygonToPlanes(int n, int **verts, int *outCount, int nPlanes, float *planes) {
    int i;

    for (i = 0; i < n; i++) {
        ClipVertsIn[i] = verts[i];
    }
    ClipVertexPoolNext = ClipVertexPool;
    for (i = 0; i < nPlanes; i++) {
        n = ClipPolygonToPlane(n, ClipVertexLists[i & 1], ClipVertexLists[(i - 1) & 1], &ClipVertexPoolNext, planes);
        if (n == 0) {
            *outCount = 0;
            return NULL;
        }
        planes += 3;
    }
    *outCount = n;
    return ClipVertexLists[i & 1];
}

struct Pair {
    unsigned int field_0;
    unsigned int field_4;
};

// FUNCTION: LEGOLAND 0x0041f350
int **ClipPolygonToPlaneSet(int param1, int **param2, int *param3, struct PlaneSet *param4) {
    return ClipPolygonToPlanes(param1, param2, param3, param4->count, param4->p[0]);
}

// FUNCTION: LEGOLAND 0x0041f380
void SetClipPlanesFromRect(const int *r, struct PlaneSet *out) {
    out->count = 4;
    out->p[0][0] = 1.0f;
    out->p[0][1] = 0.0f;
    out->p[0][2] = (float)r[1];
    out->p[1][0] = -1.0f;
    out->p[1][1] = 0.0f;
    out->p[1][2] = -(float)r[3];
    out->p[2][0] = 0.0f;
    out->p[2][1] = 1.0f;
    out->p[2][2] = (float)r[0];
    out->p[3][0] = 0.0f;
    out->p[3][1] = -1.0f;
    out->p[3][2] = -(float)r[2];
}

// FUNCTION: LEGOLAND 0x0041f3e0
struct FloatArray ***BuildVecDifferenceTable(void (*fn)(float, struct FloatArray *), struct VecMath *ar, int n, float a, float b) {
    int cnt = n + 1;
    struct FloatArray **blk;
    int i;
    int j;
    int k;

    blk = ar->alloc((cnt + 1) * cnt >> 1);
    if (blk == NULL) {
        return NULL;
    }
    for (i = 0; i <= n; i++) {
        DifferenceTableRows[i] = blk;
        blk += cnt - i;
    }
    a -= (n >> 1) * b;
    for (i = 0; i <= n; i++) {
        fn(a, DifferenceTableRows[0][i]);
        a += b;
    }
    for (k = 1; k <= n; k++) {
        for (j = 0; j < n - k + 1; j++) {
            ar->sub(DifferenceTableRows[k - 1][j + 1], DifferenceTableRows[k - 1][j], DifferenceTableRows[k][j]);
        }
    }
    return DifferenceTableRows;
}

// FUNCTION: LEGOLAND 0x0041f4c0
unsigned int FreeVecDifferenceTable(struct FloatArray ***param1, struct VecMath *param2, int param3) {
    int n = param3 + 1;
    int tri = ((n + 1) * n) >> 1;
    return ((unsigned int (*)(struct FloatArray **, unsigned int))param2->release)(*param1, tri);
}

// FUNCTION: LEGOLAND 0x0041f4e0
int VecDerivative(void (*fn)(float, struct FloatArray *), struct VecMath *a, float b, float c, struct FloatArray *d) {
    struct VecMath *ar = a;
    struct FloatArray ***t;
    struct FloatArray **h;
    struct FloatArray *p;
    struct FloatArray *q;

    c *= 0.5f;
    t = BuildVecDifferenceTable(fn, ar, 4, b, c);
    h = ar->alloc(2);
    p = h[0];
    q = h[1];
    if (t == NULL) {
        return 0;
    }
    ar->add(t[1][1], t[1][2], p);
    ar->add(t[3][0], t[3][1], q);
    ar->scale(q, DerivThirdDiffWeight);
    ar->add(p, q, d);
    ar->scale(d, 0.5f / c);
    ar->release(h, 2);
    FreeVecDifferenceTable(t, ar, 4);
    return 1;
}

// FUNCTION: LEGOLAND 0x0041f5a0
int VecDerivative4Point(void (*fn)(float, struct FloatArray *), struct VecMath *a, float b, float c, struct FloatArray *d) {
    struct VecMath *ar = a;
    struct FloatArray **h;
    struct FloatArray *q;
    struct FloatArray *r;
    int i;

    c *= 0.5f;
    h = ar->alloc(2);
    q = h[1];
    r = d;
    fn(Deriv4PointFirstOffset * c + b, r);
    ar->scale(r, Deriv4PointFirstWeight);
    for (i = 0; i <= 2; i++) {
        fn(c * Deriv4PointOffsets[i] + b, q);
        ar->scale(q, Deriv4PointWeights[i]);
        ar->add(q, r, r);
    }
    ar->scale(r, 1.0f / c);
    ar->release(h, 2);
    return 1;
}

// FUNCTION: LEGOLAND 0x0041f650
float **BuildDifferenceTable(double (*fn)(float), int n, float a, float b) {
    int cnt = n + 1;
    float **t;
    float *p;
    int i;
    int j;
    int k;

    /* the rows' pointers, then the triangle of cnt * (cnt + 1) / 2 floats */
    t = (float **)MemAlloc(((cnt + 1) * cnt >> 1) * sizeof(float) + cnt * sizeof(float *), 0, 0, 0);
    if (t == NULL) {
        return NULL;
    }
    p = (float *)(t + n + 1);
    for (i = 0; i <= n; i++) {
        t[i] = p;
        p += n + 1 - i;
    }
    a -= (n >> 1) * b;
    for (i = 0; i <= n; i++) {
        t[0][i] = (float)fn(a);
        a += b;
    }
    for (k = 1; k <= n; k++) {
        for (j = 0; j < n - k + 1; j++) {
            t[k][j] = t[k - 1][j + 1] - t[k - 1][j];
        }
    }
    return t;
}

// FUNCTION: LEGOLAND 0x0041f710
void FreeDifferenceTable(void *param) {
    MemFree(param);
}

// FUNCTION: LEGOLAND 0x0041f720
int ScalarDerivative(double (*fn)(float), float p2, float p3, float *out) {
    float **r;

    p3 *= 0.5f;
    r = BuildDifferenceTable(fn, 4, p2, p3);
    if (!r) {
        return 0;
    }
    p3 = ((r[1][2] + r[1][1]) - (r[3][1] + r[3][0]) * ScalarDerivThirdDiffWeight) * 0.5f / p3;
    FreeDifferenceTable(r);
    *out = p3;
    return 1;
}

// FUNCTION: LEGOLAND 0x0041f790
void ExpTestCurve(float x, struct FloatArray *out) {
    double e;
    e = exp(x);
    out->count = 3;
    out->data[0] = (float)e;
    out->data[1] = (float)(2.0 * e);
    out->data[2] = (float)(2.0 * exp(2.0 * x));
}

// FUNCTION: LEGOLAND 0x0041f7e0
double LogFloat(float param) {
    return log(param);
}

// FUNCTION: LEGOLAND 0x0041f7f0
void TestNumericDerivatives(void) {
    float buf;
    struct VecMath x;
    struct FloatArray y;

    ScalarDerivative(LogFloat, 3.0f, 1.0f, &buf);
    InitVecMathOps();
    InitVecMath(&x, 3);
    VecDerivative(ExpTestCurve, &x, 3.0f, 0.01f, &y);
}

// FUNCTION: LEGOLAND 0x0041f850
struct SegBlob *CurveWalkerNext(struct CurveWalker *cursor) {
    struct SegBlob *result = cursor->node->next;
    if (result == 0) {
        struct PathSeg *piece = cursor->piece->next;
        cursor->piece = piece;
        result = GetPieceCurve(piece, &cursor->pos);
    }
    cursor->node = result;
    return result;
}

// FUNCTION: LEGOLAND 0x0041f880
void CurveWalkerPrev(struct CurveWalker *walker) {
    struct SegBlob *node = walker->node->prev;
    if (node != NULL) {
        walker->node = node;
        return;
    }
    walker->piece = walker->piece->prev;
    walker->node = GetPieceCurve(walker->piece, &walker->pos);
    if (walker->node->next == NULL) {
        return;
    }
    do {
        walker->node = walker->node->next;
    } while (walker->node->next != NULL);
}

struct RecIdx {
    int v;
    int k;
};

struct RecSrc {
    int f0;
    int flag;
    int fx;
    int f0c;
    int f10;
    int f14;
    int f18;
    int d;
    int f20;
    int f24;
    int f28;
    int f2c;
};

/* Port [library:asm]: the original fills polygons in inline asm, in three variants. PortFillPolygon fills a flat polygon
 * from its edge table with one 16-bit color: idx[0..n-1] are {first scanline, edge}; between two starts the
 * current left and right edges (16.16 fixed-point x and slope) bound the spans. The left edge also carries a
 * 16.16 depth (f0c, slope f20) that advances by dz per pixel along a span:
 *   PORT_FILL_FLAT     color only
 *   PORT_FILL_WRITE_Z  color, and the depth written to zbuf
 *   PORT_FILL_TEST_Z   only where depth >= zbuf (unsigned), writing both
 * idx[n] receives the last line (the original's bookkeeping). */
#define PORT_FILL_FLAT 0
#define PORT_FILL_WRITE_Z 1
#define PORT_FILL_TEST_Z 2

static void PortFillPolygon(unsigned short *screen, unsigned short *zbuf, unsigned short color, int dz, int mode, int n, struct RecIdx *idx, struct RecSrc *src) {
    struct RecSrc *last = &src[idx[n - 1].k];
    unsigned short *row;
    unsigned short *zrow = NULL;
    unsigned short depth;
    int y = idx[0].v;
    int end;
    int lx = 0;
    int ldx = 0;
    int lz = 0;
    int ldz = 0;
    int rx = 0;
    int rdx = 0;
    int x;
    int z;

    ((short *)&last->f0)[1]++;
    end = ((short *)&last->f0)[1];
    idx[n].v = end;
    FilledTriangleCount++;
    row = screen + CoasterSurfacePitch * y;
    if (mode != PORT_FILL_FLAT) {
        zrow = zbuf + CoasterSurfacePitch * y;
    }
    do {
        struct RecSrc *e = &src[idx->k];

        idx++;
        if (e->flag) {
            rx = e->fx - e->d;
            rdx = e->d;
        } else {
            lx = e->fx - e->d;
            ldx = e->d;
            lz = e->f0c - e->f20;
            ldz = e->f20;
        }
        while (y < idx->v) {
            y++;
            lx += ldx;
            lz += ldz;
            rx += rdx;
            if (rx - lx >= 0x8000) {
                z = lz;
                for (x = lx >> 16; x <= rx >> 16; x++) {
                    depth = (unsigned short)(z >> 16);
                    if (mode != PORT_FILL_TEST_Z || depth >= zrow[x]) {
                        row[x] = color;
                        if (mode != PORT_FILL_FLAT) {
                            zrow[x] = depth;
                        }
                    }
                    z += dz;
                }
            }
            row += CoasterSurfacePitch;
            if (mode != PORT_FILL_FLAT) {
                zrow += CoasterSurfacePitch;
            }
        }
    } while (y < end);
}

// FUNCTION: LEGOLAND 0x0041f8d0
void FillSpansFlat(int palette, int *color_index, int n, struct RecIdx *idx, struct RecSrc *src) {
    /* Port [library:asm]: the original is inline asm. Fills a polygon on the screen buffer with palette color *color_index. */
    PortFillPolygon((unsigned short *)CoasterSurfaceBits, NULL, ((unsigned short *)CoasterShadeTables[palette])[*color_index], 0, PORT_FILL_FLAT, n, idx, src);
}

// FUNCTION: LEGOLAND 0x0041fa10
void FillSpansFlatWriteZ(int palette, int *shade, int n, struct RecIdx *idx, struct RecSrc *src) {
    /* Port [library:asm]: the original is inline asm. Fills a polygon with palette color shade[0] and writes its depth
     * (per-pixel step shade[1]) into the depth buffer CoasterDepthBuffer. */
    PortFillPolygon((unsigned short *)CoasterSurfaceBits, CoasterDepthBuffer, ((unsigned short *)CoasterShadeTables[palette])[shade[0]], shade[1], PORT_FILL_WRITE_Z, n, idx, src);
}

// FUNCTION: LEGOLAND 0x0041fba0
void FillSpansFlatZ(int palette, int *shade, int n, struct RecIdx *idx, struct RecSrc *src) {
    /* Port [library:asm]: the original is inline asm. Like FillSpansFlatWriteZ, but only draws where the polygon's depth is >= the
     * depth buffer. */
    PortFillPolygon((unsigned short *)CoasterSurfaceBits, CoasterDepthBuffer, ((unsigned short *)CoasterShadeTables[palette])[shade[0]], shade[1], PORT_FILL_TEST_Z, n, idx, src);
}

// FUNCTION: LEGOLAND 0x0041fd30
void InitShadeClampTable(void) {
    int i;
    for (i = 0; i < 16; i++) {
        ((unsigned int *)ShadeClampTable)[i] = 0;
    }
    ShadeClamp = &ShadeClampTable[0x40];
    for (i = 0x40; i <= 0x7f; i++) {
        ShadeClampTable[i] = (unsigned char)(i - 0x40);
    }
    for (i = 0; i < 16; i++) {
        ((unsigned int *)(ShadeClampTable + 0x80))[i] = 0x3f3f3f3f;
    }
}

/* Port [library:asm]: the original fills textured polygons in inline asm. Same edge-table walk as PortFillPolygon; the left
 * edge also carries the texture coordinate u (f0c, slope f20) and the depth z (f10, slope f24), both 16.16.
 * Along a span u advances by shade[1] per pixel and z by shade[2]; each pixel is palette[texture[u]], with
 * texture = ShadeClamp. With depth set, a pixel is only drawn where its depth is >= the depth buffer
 * (unsigned), and the depth is written too.
 *
 * The original steps u as a pointer into the texture plus a fraction register, adding the integer part of
 * |du| and the fraction's carry, backwards when du < 0 (so negative steps round oddly; kept as is). In the
 * depth variant the fraction register is shared: bits 0-23 hold the depth (16.8) and bits 24-31 the top 8
 * fraction bits of u, with bit 24 cleared after every step. */
static void PortTexturePolygon(int palette, int *shade, int n, struct RecIdx *idx, struct RecSrc *src, int depth) {
    unsigned short *colors = (unsigned short *)CoasterShadeTables[palette];
    struct RecSrc *last = &src[idx[n - 1].k];
    unsigned short *row;
    unsigned short *zrow;
    unsigned char *texel;
    unsigned int du = shade[1];
    int backwards = 0;
    unsigned int step_int;
    unsigned int step_frac;
    unsigned int acc;
    unsigned int sum;
    int y = idx[0].v;
    int end;
    int lx = 0;
    int ldx = 0;
    int lu = 0;
    int ldu = 0;
    int lz = 0;
    int ldz = 0;
    int rx = 0;
    int rdx = 0;
    int x;

    ((short *)&last->f0)[1]++;
    end = ((short *)&last->f0)[1];
    idx[n].v = end;
    FilledTriangleCount++;
    if ((int)du < 0) {
        du = -(int)du;
        backwards = 1;
    }
    step_int = (int)du >> 16;
    if (depth) {
        step_frac = ((du & 0xffffff00) << 16) | (((unsigned int)shade[2] >> 8) & 0xffffff);
    } else {
        step_frac = du << 16;
    }
    row = (unsigned short *)CoasterSurfaceBits + CoasterSurfacePitch * y;
    zrow = CoasterDepthBuffer + CoasterSurfacePitch * y;
    do {
        struct RecSrc *e = &src[idx->k];

        idx++;
        if (e->flag) {
            rx = e->fx - e->d;
            rdx = e->d;
        } else {
            lx = e->fx - e->d;
            ldx = e->d;
            lu = e->f0c - e->f20;
            ldu = e->f20;
            lz = e->f10 - e->f24;
            ldz = e->f24;
        }
        while (y < idx->v) {
            y++;
            lx += ldx;
            lu += ldu;
            lz += ldz;
            rx += rdx;
            if (rx - lx >= 0x8000) {
                if (depth) {
                    texel = (unsigned char *)ShadeClamp + (lu >> 16);
                    acc = ((unsigned int)lz >> 8) & 0xffffff;
                } else {
                    texel = (unsigned char *)ShadeClamp + ((lu >> 16) & 0xffff);
                    acc = (unsigned int)lu << 16;
                }
                for (x = lx >> 16; x <= rx >> 16; x++) {
                    if (!depth) {
                        row[x] = colors[*texel];
                    } else if ((unsigned short)(acc >> 8) >= zrow[x]) {
                        zrow[x] = (unsigned short)(acc >> 8);
                        row[x] = colors[*texel];
                    }
                    sum = acc + step_frac;
                    if (backwards) {
                        texel -= step_int + (sum < acc);
                    } else {
                        texel += step_int + (sum < acc);
                    }
                    acc = depth ? sum & 0xfeffffff : sum;
                }
            }
            row += CoasterSurfacePitch;
            zrow += CoasterSurfacePitch;
        }
    } while (y < end);
}

// FUNCTION: LEGOLAND 0x0041fd80
void FillSpansGouraud(int palette, int *shade, int n, struct RecIdx *idx, struct RecSrc *src) {
    /* Port [library:asm]: the original is inline asm. Textured polygon fill, no depth buffer. */
    PortTexturePolygon(palette, shade, n, idx, src, 0);
}

// FUNCTION: LEGOLAND 0x0041ff80
void FillSpansGouraudZ(int palette, int *shade, int n, struct RecIdx *idx, struct RecSrc *src) {
    /* Port [library:asm]: the original is inline asm. Textured polygon fill, depth-tested against and written to the depth
     * buffer CoasterDepthBuffer. */
    PortTexturePolygon(palette, shade, n, idx, src, 1);
}

#pragma optimize("", off)
// FUNCTION: LEGOLAND 0x00420200
float IntegrateSimpson(float (*fn)(unsigned int), unsigned int a, unsigned int b, float tol) {
    int it2;
    int iib;
    float hh_;
    float s_;
    float add0;
    float pr0;
    float area0;
    float strideb;
    float cxxb;

    it2 = 1;
    hh_ = *(float *)&b - *(float *)&a;
    s_ = fn(a) + fn(b);
    add0 = 0;
    area0 = s_ * hh_ * 0.5f * FLOAT_004ab43c;
    do {
        pr0 = area0;
        s_ = s_ - 2.0f * add0;
        add0 = 0;
        strideb = hh_;
        hh_ = 0.5f * hh_;
        cxxb = hh_ + *(float *)&a;
        for (iib = 1; iib <= it2; iib++) {
            add0 = fn(*(unsigned int *)&cxxb) + add0;
            cxxb = cxxb + strideb;
        }
        s_ = 4.0f * add0 + s_;
        area0 = s_ * hh_;
        it2 = it2 + it2;
    } while (fabs(area0 - pr0) > tol * fabs(pr0));
    return FLOAT_004ab438 * area0;
}
#pragma optimize("", on)

// FUNCTION: LEGOLAND 0x00420310
void IntegratorStepRK4(struct Integrator *ar, float *p, float c) {
    struct FloatArray **h;
    struct FloatArray *v0;
    struct FloatArray *v1;
    struct FloatArray *v2;
    struct FloatArray *v3;
    int i;

    h = ar->math.alloc(4);
    v0 = h[0];
    v1 = h[1];
    v2 = h[2];
    v3 = h[3];
    ar->math.zero(v2, ar->math.dim);
    ar->load(ar, v0);
    ar->load(ar, v1);
    for (i = 0; i <= 3; i++) {
        ar->math.blend(v0, 1.0f, v2, RK4Coefficients[i][0], v3);
        ar->store(ar, v3);
        ar->deriv(ar, *p + c * RK4Coefficients[i][0], v2);
        ar->math.scale(v2, c);
        ar->math.axpy(v2, RK4Coefficients[i][1], v1);
    }
    ar->store(ar, v1);
    *p += c;
    ar->math.release(h, 4);
}

// FUNCTION: LEGOLAND 0x004203d0
void IntegratorStoreState(struct Integrator *self, struct FloatArray *param) {
    self->math.copy(param, self->state);
}

// FUNCTION: LEGOLAND 0x004203f0
unsigned int IntegratorLoadState(struct Integrator *self, struct FloatArray *param) {
    return ((unsigned int (*)(void *, struct FloatArray *))self->math.copy)(self->state, param);
}

// FUNCTION: LEGOLAND 0x00420410
void InitIntegrator(struct Integrator *param, unsigned int count) {
    struct Integrator *host = param;
    InitVecMath(&host->math, 2);
    host->store = IntegratorStoreState;
    host->load = IntegratorLoadState;
}

// FUNCTION: LEGOLAND 0x00420440
void LoadRollercoasterFiles(void) {
    char buf[256];
    int i;
    int n;

    RollercoasterLpt = LoadRollercoasterLpt();
    // STRING: LEGOLAND 0x004b56a0
    SetCurrentDirectoryIfNotNull("RollerCoaster\\RollerCoaster\\CreatedData");
    // STRING: LEGOLAND 0x004b5690
    LoadObjAndTxtFiles("ROLLERCOASTER");
    // STRING: LEGOLAND 0x004b5684
    SetCurrentDirectoryIfNotNull("..\\..\\..");
    n = GetCoasterObjLineCount();
    for (i = 0; i < n; i++) {
        GetCoasterObjLine(i, buf);
        LmsFileTable[i] = LoadLmsFile(buf);
    }
    n = GetCoasterObjLineCount();
    for (i = 0; i < n; i++) {
        GetCoasterObjLine(i, buf);
        LfmFileTable[i] = LoadLfmFile(buf, &LfmFileSizes[i]);
    }
    n = GetCoasterTxtLineCount();
    for (i = 0; i < n; i++) {
        FormatRollercoasterFileName(i, buf);
        LtxFileTable[i] = LoadLtxFile(buf);
    }
}

// FUNCTION: LEGOLAND 0x00420530
unsigned int SetCurrentDirectoryIfNotNull(const char *param) {
    if (param == NULL) {
        return 0;
    }
    return SetCurrentDirectoryA(param);
}

// FUNCTION: LEGOLAND 0x00420550
void *LoadCreatedDataFile(const char *fileName, unsigned int *sizeOut) {
    HANDLE hFile;
    unsigned int fileSize;
    void *buffer;
    DWORD bytesRead;

    if (fileName == 0) {
        return 0;
    }

    // STRING: LEGOLAND 0x004b56a0
    SetCurrentDirectoryIfNotNull("RollerCoaster\\RollerCoaster\\CreatedData");
    hFile = CreateFileA(fileName, 0x80000000, 0x1, 0, 0x3, 0x8000000, 0);
    if (hFile == (HANDLE)-1) {
        // STRING: LEGOLAND 0x004b5684
        SetCurrentDirectoryIfNotNull("..\\..\\..");
        return 0;
    }

    fileSize = GetFileSize(hFile, 0);
    buffer = MemAlloc(fileSize, 0, (unsigned int)(UINT_PTR)DAT_004d8bb0, 0);
    if (buffer == 0) {
        CloseHandle(hFile);
        SetCurrentDirectoryIfNotNull("..\\..\\..");
        return 0;
    }

    ReadFile(hFile, buffer, fileSize, &bytesRead, 0);
    if (bytesRead != fileSize) {
        MemFree(buffer);
        CloseHandle(hFile);
        SetCurrentDirectoryIfNotNull("..\\..\\..");
        return 0;
    }

    CloseHandle(hFile);
    SetCurrentDirectoryIfNotNull("..\\..\\..");
    if (sizeOut != 0) {
        *sizeOut = fileSize;
    }
    return buffer;
}

// FUNCTION: LEGOLAND 0x00420640
struct LmsFile *LoadLmsFile(const char *name) {
    char buffer[256];
    struct LmsFile *f;
    // STRING: LEGOLAND 0x004b56c8
    wsprintfA(buffer, "%s.lms", name);
#ifdef LEGOLAND_PORT
    {
        /* [port] the file's header holds 32-bit offsets of its tables: the whole file goes after a native header,
           and the header's pointers point into that copy (the same on 32 and 64 bits) */
        static const unsigned char tables[6] = {0x0c, 0x10, 0x14, 0x18, 0x20, 0x28};
        void **pointers[6];
        unsigned int size = 0;
        unsigned char *raw = LoadCreatedDataFile(buffer, &size);
        unsigned char *copy;
        unsigned int off;
        int i;

        if (raw == 0) {
            return 0;
        }
        f = size >= PORT_LMS_FILE_HEADER ? MemAlloc(sizeof(struct LmsFile) + size, 0, 0, 0) : 0;
        if (f == 0) {
            MemFree(raw);
            return 0;
        }
        copy = (unsigned char *)(f + 1);
        memcpy(copy, raw, size);
        MemFree(raw);
        memcpy(f->pad_0, copy, sizeof(f->pad_0));
        memcpy(&f->field_1c, copy + 0x1c, 4);
        memcpy(&f->field_24, copy + 0x24, 4);
        memcpy(&f->field_2c, copy + 0x2c, 4);
        memcpy(f->box, copy + 0x30, sizeof(f->box));
        pointers[0] = &f->field_c;
        pointers[1] = &f->field_10;
        pointers[2] = &f->field_14;
        pointers[3] = (void **)&f->field_18;
        pointers[4] = (void **)&f->field_20;
        pointers[5] = (void **)&f->field_28;
        for (i = 0; i < 6; i++) {
            memcpy(&off, copy + tables[i], 4);
            if (off > size) {
                MemFree(f);
                return 0;
            }
            *pointers[i] = copy + off;
        }
        return f;
    }
#else
    f = LoadCreatedDataFile(buffer, 0);
    if (f != 0) {
        *(UINT_PTR *)&f->field_c += (UINT_PTR)f;
        *(UINT_PTR *)&f->field_14 += (UINT_PTR)f;
        *(UINT_PTR *)&f->field_10 += (UINT_PTR)f;
        *(UINT_PTR *)&f->field_18 += (UINT_PTR)f;
        *(UINT_PTR *)&f->field_20 += (UINT_PTR)f;
        *(UINT_PTR *)&f->field_28 += (UINT_PTR)f;
        return f;
    }
    return 0;
#endif
}

// FUNCTION: LEGOLAND 0x004206b0
struct LmsFile *GetLmsByName(const char *s) {
    unsigned int index = FindCoasterObjLine(s);
    if (index != 0xffffffff) {
        return LmsFileTable[index];
    }
    return 0;
}

// FUNCTION: LEGOLAND 0x004206d0
struct LmsRec *LoadLfmFile(const char *name, unsigned int *param2) {
    char buffer[256];
    // STRING: LEGOLAND 0x004b56d0
    wsprintfA(buffer, "%s.lfm", name);
    return LoadCreatedDataFile(buffer, param2);
}

// FUNCTION: LEGOLAND 0x00420710
struct LmsRec *GetLfmByName(const char *s) {
    unsigned int index = FindCoasterObjLine(s);
    if (index != 0xffffffff) {
        return LfmFileTable[index];
    }
    return 0;
}

// FUNCTION: LEGOLAND 0x00420730
unsigned int GetLfmSizeByName(const char *s) {
    unsigned int index = FindCoasterObjLine(s);
    if (index != 0xffffffff) {
        return LfmFileSizes[index];
    }
    return 0;
}

// FUNCTION: LEGOLAND 0x00420750
void *LoadLtxFile(const char *name) {
    char buffer[256];
    // STRING: LEGOLAND 0x004b56d8
    wsprintfA(buffer, "%s.ltx", name);
    return LoadCreatedDataFile(buffer, 0);
}

// FUNCTION: LEGOLAND 0x00420780
void *GetLtxFileTableEntry(unsigned int param) {
    return LtxFileTable[param];
}

// FUNCTION: LEGOLAND 0x00420790
unsigned int FindLtxIndexByName(const char *param) {
    return FindCoasterTxtLine(param);
}

// FUNCTION: LEGOLAND 0x004207a0
int *LoadRollercoasterLpt(void) {
    // STRING: LEGOLAND 0x004b56e0
    return LoadCreatedDataFile("Rollercoaster.lpt", 0);
}

// FUNCTION: LEGOLAND 0x004207c0
int *GetRollercoasterLpt(void) {
    return RollercoasterLpt;
}

// FUNCTION: LEGOLAND 0x004207d0
int FindRollercoasterLptEntry(unsigned int key) {
    int *table = RollercoasterLpt;
    int count = table[0];
    int i;
    int *p = table + 1;
    for (i = 0; i < count; i++) {
        if (((*p++ ^ key) & 0xffffff) == 0) {
            return i;
        }
    }
    return -1;
}

/* Port: a polygon vertex as RasteriseTriangle reads it: screen y and x, then the interpolated attributes. */
struct PolyVert {
    int pad_0;
    int y;
    int x;
    int attr[4];
};

typedef void (*PolyFiller)(int palette, int *shade, int n, struct RecIdx *idx, struct RecSrc *src);

/* Port: the polygon RasteriseTriangle rasterizes. dx1/dy1 and dx2/dy2 are v[1] - v[0] and v[2] - v[0]; area is
 * their cross product. */
struct PolyArg {
    /* 0x00 */ unsigned int flags; /* 1: add the polygon to the dirty rows (RecordDepthSpans) */
    /* 0x04 */ int palette;
    /* 0x08 */ unsigned int and_codes; /* clip codes of the vertices, and-ed */
    /* 0x0c */ unsigned int or_codes; /* or-ed; 0xf0 = a vertex lies in a dirty rectangle */
    /* 0x10 */ int shade;
    /* 0x14 */ float dx1;
    /* 0x18 */ float dx2;
    /* 0x1c */ float dy1;
    /* 0x20 */ float dy2;
    /* 0x24 */ float area;
    /* 0x28 */ struct PolyVert *v[4]; /* the 3 corners; [3] closes the loop */
    /* 0x38 */ const PolyFiller *fillers; /* [0]: outside dirty rectangles, [1]: inside (depth-buffered) */
};

/* Port: the original's filler table at 0x4b5658. */
static const PolyFiller port_texture_fillers[2] = {FillSpansGouraud, FillSpansGouraudZ};

/* Port: a 3D object as DrawLmsModel draws it. Its vertices are transformed into DAT_004d8bb8 ({x, y, z,
 * clip code} each) before the face passes run. */
struct MeshFace {
    short material;
    short normal; /* face normal (flat and textured faces) */
    short v[3]; /* vertices */
    short vnormal[3]; /* vertex normals (smooth faces) */
};

struct Mesh3D {
    /* 0x00 */ int vert_count;
    /* 0x04 */ int pad_4[2];
    /* 0x0c */ float (*verts)[3];
    /* 0x10 */ float (*face_normals)[3];
    /* 0x14 */ float (*vert_normals)[3];
    /* 0x18 */ struct MeshFace *flat_faces;
    /* 0x1c */ int flat_count;
    /* 0x20 */ struct MeshFace *smooth_faces;
    /* 0x24 */ int smooth_count;
    /* 0x28 */ struct MeshFace *textured_faces;
    /* 0x2c */ int textured_count;
};

struct Material {
    int palette;
    unsigned char uv[3][2]; /* texture coordinates of the face's corners */
    short pad;
};

#define PORT_FACES_FLAT 0
#define PORT_FACES_SMOOTH 1
#define PORT_FACES_TEXTURED 2

/* Port: light level of a normal: the dot product with the light vector DAT_004dcbb8, plus the ambient light. */
static int PortLight(const float *normal) {
    float light = DAT_004dcbb8[2] * normal[2] + DAT_004dcbb8[1] * normal[1] + DAT_004dcbb8[0] * normal[0] + CastleLightBias;

    return PortRound(light);
}

/* Port: the original's three face passes are inline asm (rdtsc profiling) and differ only in the face list and
 * the values interpolated across each triangle. Every front-facing, not trivially clipped face goes to
 * RasteriseTriangle:
 *   flat:     one light level per face; interpolates x, z (fillers FillSpansFlat / FillSpansFlatZ)
 *   smooth:   a light level per corner from the vertex normals; interpolates x, light, z (fillers
 *             FillSpansGouraud / FillSpansGouraudZ, indexing the shade ramp ShadeClamp with the light)
 *   textured: one light level per face; interpolates x, u, v, z (filler FillSpansTexturedZ)
 * flags != 0 adds the drawn faces to the dirty rows. */
static void PortDrawFaces(struct Mesh3D *mesh, struct Material *materials, unsigned int flags, int kind) {
    static const PolyFiller flat_fillers[2] = {FillSpansFlat, FillSpansFlatZ};
    static const PolyFiller textured_fillers[2] = {FillSpansTexturedZ, FillSpansTexturedZ};
    unsigned int start = PortTimestamp();
    struct PolyArg poly;
    struct PolyVert corner[3];
    struct MeshFace *faces;
    struct MeshFace *f;
    int *p[3];
    int count;
    int i;
    int j;

    if (kind == PORT_FACES_FLAT) {
        faces = mesh->flat_faces;
        count = mesh->flat_count;
        poly.fillers = flat_fillers;
    } else if (kind == PORT_FACES_SMOOTH) {
        faces = mesh->smooth_faces;
        count = mesh->smooth_count;
        poly.fillers = port_texture_fillers;
    } else {
        faces = mesh->textured_faces;
        count = mesh->textured_count;
        poly.fillers = textured_fillers;
    }
    poly.flags = flags != 0;
    poly.shade = 0;
    for (j = 0; j < 3; j++) {
        poly.v[j] = &corner[j];
    }
    for (i = 0; i < count; i++) {
        f = &faces[i];
        for (j = 0; j < 3; j++) {
            p[j] = DAT_004d8bb8[f->v[j]];
        }
        poly.dx1 = (float)(p[1][0] - p[0][0]);
        poly.dy1 = (float)(p[1][1] - p[0][1]);
        poly.dx2 = (float)(p[2][0] - p[0][0]);
        poly.dy2 = (float)(p[2][1] - p[0][1]);
        poly.area = poly.dy2 * poly.dx1 - poly.dx2 * poly.dy1;
        if (poly.area <= FLOAT_004ab390) {
            continue; /* back-facing */
        }
        poly.and_codes = p[0][3] & p[1][3] & p[2][3] & 0xff;
        poly.or_codes = p[0][3] | p[1][3] | p[2][3];
        if ((poly.or_codes & 0xf) != 0xf) {
            continue; /* entirely outside one edge of the view */
        }
        poly.palette = materials[f->material].palette;
        for (j = 0; j < 3; j++) {
            corner[j].y = p[j][1];
            corner[j].x = p[j][0];
        }
        if (kind == PORT_FACES_FLAT) {
            poly.shade = PortLight(mesh->face_normals[f->normal]);
            for (j = 0; j < 3; j++) {
                corner[j].attr[0] = p[j][2];
            }
            RasteriseTriangle(2, &poly);
        } else if (kind == PORT_FACES_SMOOTH) {
            for (j = 0; j < 3; j++) {
                corner[j].attr[0] = PortLight(mesh->vert_normals[f->vnormal[j]]);
                corner[j].attr[1] = p[j][2];
            }
            RasteriseTriangle(3, &poly);
        } else {
            poly.shade = PortLight(mesh->face_normals[f->normal]);
            for (j = 0; j < 3; j++) {
                corner[j].attr[0] = materials[f->material].uv[j][0];
                corner[j].attr[1] = materials[f->material].uv[j][1];
                corner[j].attr[2] = p[j][2];
            }
            RasteriseTriangle(4, &poly);
        }
    }
    LmsDrawCycles += PortTimestamp() - start;
}

// FUNCTION: LEGOLAND 0x00420810
void DrawLmsFlatTriangles(struct LmsFile *mesh, struct LmsRec *b, unsigned int flags) {
    /* Port [library:asm]: the original is inline asm. Draws the object's flat-shaded faces. */
    PortDrawFaces((struct Mesh3D *)mesh, (struct Material *)b, flags, PORT_FACES_FLAT);
}

// FUNCTION: LEGOLAND 0x00420a20
void DrawLmsGouraudTriangles(struct LmsFile *mesh, struct LmsRec *b, unsigned int flags) {
    /* Port [library:asm]: the original is inline asm. Draws the object's smooth-shaded faces. */
    PortDrawFaces((struct Mesh3D *)mesh, (struct Material *)b, flags, PORT_FACES_SMOOTH);
}

// FUNCTION: LEGOLAND 0x00420c40
void DrawLmsTexturedTriangles(struct LmsFile *mesh, struct LmsRec *b, unsigned int flags) {
    /* Port [library:asm]: the original is inline asm. Draws the object's textured faces. */
    PortDrawFaces((struct Mesh3D *)mesh, (struct Material *)b, flags, PORT_FACES_TEXTURED);
}

// FUNCTION: LEGOLAND 0x00420e90
unsigned int DrawLmsModel(struct LmsFile *a, struct LmsRec *b, void *c, void *d, unsigned int e) {
    /* Port [library:asm]: the original is inline asm (rdtsc profiling). Draws one 3D object: a = mesh (vertex count at
     * [0], vertices at [3]), c = position, d = orientation. Builds the object-to-screen matrix, transforms
     * the vertices into DAT_004d8bb8 and the light vector into DAT_004dcbb8, then, if the screen can be
     * locked, runs the three drawing passes. Returns 1 when it drew. */
    unsigned int start = PortTimestamp();
    float rot[4][4];
    float m[4][4];
    struct VideoArg video;

    if (a == 0 || b == 0) {
        return 0;
    }
    Mat3ToMat4Transpose((unsigned int *)d, (struct Mat4x4 *)rot);
    TransformPoints3x4((float *)&CastleLightDir, DAT_004dcbb8, rot, 1);
    DAT_004dcbb8[0] *= CastleLightScale;
    DAT_004dcbb8[1] *= CastleLightScale;
    DAT_004dcbb8[2] *= CastleLightScale;
    MakeMat4FromRotTrans((unsigned int *)c, (unsigned int *)d, (unsigned int (*)[4])rot);
    Mat4Multiply(CastleWorldToScreen.m, rot, m);
    ProjectPoints((float (*)[3])a->field_c, (int (*)[4])DAT_004d8bb8, &m[0][0], 0x10, *(unsigned int *)a);
    LmsDrawCycles += PortTimestamp() - start;
    if (GetCoasterSurface(&video) == 0) {
        return 0;
    }
    DrawLmsFlatTriangles(a, b, e);
    DrawLmsGouraudTriangles(a, b, e);
    DrawLmsTexturedTriangles(a, b, e);
    ReleaseCoasterSurface();
    return 1;
}

// FUNCTION: LEGOLAND 0x00420fb0
unsigned int GetLmsBoxScreenRect(struct LmsFile *param_1, unsigned int *param_2, unsigned int *param_3, struct RectI *param_4) {
    return GetBoxScreenRect(param_1->box, param_2, param_3, param_4);
}

// GLOBAL: LEGOLAND 0x004b5648
void *FlatSpanFillers[2] = {FillSpansFlat, FillSpansFlatZ};

// GLOBAL: LEGOLAND 0x004b5658
void *GouraudSpanFillers[2] = {FillSpansGouraud, FillSpansGouraudZ};

// GLOBAL: LEGOLAND 0x004b5700
float BoxFaceNormals[6][3] = {
    {0.0f, 0.0f, -1.0f},
    {0.0f, 0.0f, 1.0f},
    {1.0f, 0.0f, 0.0f},
    {-1.0f, 0.0f, 0.0f},
    {0.0f, 1.0f, 0.0f},
    {0.0f, -1.0f, 0.0f},
};

// GLOBAL: LEGOLAND 0x004b5748
struct BoxFace DAT_004b5748[12] = {
    {0, 0, {0, 1, 2}, {0, 0, 0}},
    {0, 0, {0, 2, 3}, {0, 0, 0}},
    {0, 1, {7, 6, 5}, {0, 0, 0}},
    {0, 1, {7, 5, 4}, {0, 0, 0}},
    {0, 2, {0, 5, 1}, {0, 0, 0}},
    {0, 2, {0, 4, 5}, {0, 0, 0}},
    {0, 3, {2, 7, 3}, {0, 0, 0}},
    {0, 3, {2, 6, 7}, {0, 0, 0}},
    {0, 4, {1, 6, 2}, {0, 0, 0}},
    {0, 3, {1, 5, 6}, {0, 0, 0}},
    {0, 5, {3, 4, 0}, {0, 0, 0}},
    {0, 3, {3, 7, 4}, {0, 0, 0}},
};

// GLOBAL: LEGOLAND 0x004b5808
BoxFace DAT_004b5808[12] = {
    {0, 0, {0, 1, 2}, {0, 0, 0}},
    {0, 0, {0, 2, 3}, {0, 0, 0}},
    {0, 1, {7, 6, 5}, {0, 0, 0}},
    {0, 1, {7, 5, 4}, {0, 0, 0}},
    {0, 2, {0, 5, 1}, {0, 0, 0}},
    {0, 2, {0, 4, 5}, {0, 0, 0}},
    {0, 3, {2, 7, 3}, {0, 0, 0}},
    {0, 3, {2, 6, 7}, {0, 0, 0}},
    {0, 4, {1, 6, 2}, {0, 0, 0}},
    {0, 3, {1, 5, 6}, {0, 0, 0}},
    {0, 5, {3, 4, 0}, {0, 0, 0}},
    {0, 3, {3, 7, 4}, {0, 0, 0}},
};

// GLOBAL: LEGOLAND 0x004b58c8
struct BoxSolid BoxSolidTemplate = {8, 0, 12, NULL, BoxFaceNormals, NULL, DAT_004b5748, 12};

// GLOBAL: LEGOLAND 0x004b5f50
void *TexturedSpanFillers[2] = {FillSpansTexturedZ, FillSpansTexturedZ};

// FUNCTION: LEGOLAND 0x00420fd0
void InitBoxSolidToOrigin(struct BoxSolid *box, struct FVec3 *verts, struct FVec3 *xverts, int unused, float w, float h, float d) {
    int i;

    *box = BoxSolidTemplate;
    box->verts = verts;
    box->xverts = xverts;
    box->verts[0].x = w;
    box->verts[0].y = -h;
    box->verts[0].z = -d;
    box->verts[1].x = w;
    box->verts[1].y = h;
    box->verts[1].z = -d;
    box->verts[2].x = -w;
    box->verts[2].y = h;
    box->verts[2].z = -d;
    box->verts[3].x = -w;
    box->verts[3].y = -h;
    box->verts[3].z = -d;
    for (i = 0; i <= 3; i++) {
        box->verts[i + 4] = box->verts[i];
        box->verts[i + 4].z = 0.0f;
    }
    if (xverts != NULL) {
        for (i = 0; i <= 7; i++) {
            struct FVec3 tmp = box->verts[i];
            Vec3Normalize(&tmp.x);
            box->xverts[i] = tmp;
        }
        for (i = 0; i < 12; i++) {
            DAT_004b5808[i].plane = -1;
            DAT_004b5808[i].u[0] = DAT_004b5808[i].t[0];
            DAT_004b5808[i].u[1] = DAT_004b5808[i].t[1];
            DAT_004b5808[i].u[2] = DAT_004b5808[i].t[2];
        }
        box->nFaceA = 0;
        box->nFaceB = 12;
        box->faceB = DAT_004b5808;
    }
}

// FUNCTION: LEGOLAND 0x00421130
void InitBoxSolidCentred(struct BoxSolid *box, struct FVec3 *verts, struct FVec3 *xverts, int unused, float w, float h, float d) {
    int i;

    *box = BoxSolidTemplate;
    box->verts = verts;
    box->xverts = xverts;
    box->verts[0].x = w;
    box->verts[0].y = -h;
    box->verts[0].z = -d;
    box->verts[1].x = w;
    box->verts[1].y = h;
    box->verts[1].z = -d;
    box->verts[2].x = -w;
    box->verts[2].y = h;
    box->verts[2].z = -d;
    box->verts[3].x = -w;
    box->verts[3].y = -h;
    box->verts[3].z = -d;
    for (i = 0; i <= 3; i++) {
        box->verts[i + 4] = box->verts[i];
        box->verts[i + 4].z = d;
    }
    if (xverts != NULL) {
        for (i = 0; i <= 7; i++) {
            struct FVec3 tmp = box->verts[i];
            Vec3Normalize(&tmp.x);
            box->xverts[i] = tmp;
        }
        for (i = 0; i < 12; i++) {
            DAT_004b5808[i].plane = -1;
            DAT_004b5808[i].u[0] = DAT_004b5808[i].t[0];
            DAT_004b5808[i].u[1] = DAT_004b5808[i].t[1];
            DAT_004b5808[i].u[2] = DAT_004b5808[i].t[2];
        }
        box->nFaceA = 0;
        box->nFaceB = 12;
        box->faceB = DAT_004b5808;
    }
}

// FUNCTION: LEGOLAND 0x004212a0
void FloatArrayAdd(struct FloatArray *a, struct FloatArray *b, struct FloatArray *out) {
    int i;
    out->count = a->count;
    for (i = 0; i < a->count; i++) {
        float t = b->data[i];
        out->data[i] = t + a->data[i];
    }
}

// FUNCTION: LEGOLAND 0x004212e0
void FloatArraySub(struct FloatArray *a, struct FloatArray *b, struct FloatArray *out) {
    int i;
    out->count = a->count;
    for (i = 0; i < a->count; i++) {
        out->data[i] = a->data[i] - b->data[i];
    }
}

// FUNCTION: LEGOLAND 0x00421320
void CopyFloatArray(struct FloatArray *src, struct FloatArray *dst) {
    memcpy(dst, src, sizeof(struct FloatArray));
}

// FUNCTION: LEGOLAND 0x00421340
void FloatArrayScale(struct FloatArray *arr, float f) {
    int i;
    for (i = 0; i < arr->count; i++) {
        arr->data[i] = f * arr->data[i];
    }
}

// FUNCTION: LEGOLAND 0x00421360
float FloatArrayMaxAbs(struct FloatArray *arr) {
    float max = FLOAT_004ab390;
    int i;
    for (i = 0; i < arr->count; i++) {
        float v = (float)fabs(arr->data[i]);
        if (v > max) {
            max = v;
        }
    }
    return max;
}

// FUNCTION: LEGOLAND 0x004213a0
void BlendFloatArrays(struct FloatArray *a, float s, struct FloatArray *b, float t, struct FloatArray *out) {
    int i;

    out->count = a->count;
    for (i = 0; i < a->count; i++) {
        out->data[i] = a->data[i] * s + b->data[i] * t;
    }
    out->count = a->count;
}

// FUNCTION: LEGOLAND 0x00421400
void ZeroIntBuffer(struct FloatArray *buf, int size) {
    int i;
    if (size > 0) {
        for (i = 0; i < size; i++) {
            *(int *)&buf->data[i] = 0;
        }
    }
    buf->count = size;
}

// FUNCTION: LEGOLAND 0x00421430
void FloatArrayAddScaled(struct FloatArray *a, float f, struct FloatArray *out) {
    int i;
    out->count = a->count;
    for (i = 0; i < a->count; i++) {
        out->data[i] += f * a->data[i];
    }
}

// FUNCTION: LEGOLAND 0x00421470
void InitVecMathOps(void) {
    struct FloatArray **table;
    struct FloatArray *cur;

    VecMathOps.add = FloatArrayAdd;
    VecMathOps.sub = FloatArraySub;
    VecMathOps.copy = CopyFloatArray;
    VecMathOps.scale = FloatArrayScale;
    VecMathOps.maxabs = FloatArrayMaxAbs;
    VecMathOps.blend = BlendFloatArrays;
    VecMathOps.zero = ZeroIntBuffer;
    VecMathOps.axpy = FloatArrayAddScaled;
    VecMathOps.alloc = AllocVecScratch;
    VecMathOps.release = FreeVecScratch;

    table = DAT_0082ac60;
    cur = DAT_004dcc00;
    /* [port] bounded by the pool itself: the original stops at the next global, VecScratchTop */
    while (cur < DAT_004dcc00 + sizeof(DAT_004dcc00) / sizeof(DAT_004dcc00[0])) {
        *table = cur;
        cur++;
        table += 1;
    }
}

// FUNCTION: LEGOLAND 0x004214f0
struct FloatArray **AllocVecScratch(unsigned int param_1) {
    unsigned int index = VecScratchTop;
    VecScratchTop = VecScratchTop + param_1;
    return &DAT_0082ac60[index];
}

// FUNCTION: LEGOLAND 0x00421510
void FreeVecScratch(struct FloatArray **param_1, unsigned int param_2) {
    VecScratchTop = VecScratchTop - param_2;
}

// FUNCTION: LEGOLAND 0x00421540
void InitVecMath(struct VecMath *dest, unsigned int value) {
    VecMathOps.dim = value;
    memcpy(dest, &VecMathOps, sizeof(struct VecMath));
}

// FUNCTION: LEGOLAND 0x00421560
unsigned int DrawCastleVisitor(struct CastleVisitor *param_2, struct CarXform *param_1) {
    SetModelLighting();
    return DrawLmsModel(param_2->lms, param_2->field_8, param_1, &param_1->m, 0);
}

// FUNCTION: LEGOLAND 0x00421590
void SeatCastleVisitor(struct CastleVisitor *arg1, struct TrainSeat *arg2) {
    SetTrainSeatOccupant(arg2, arg1);
    arg1->seat = arg2;
}

// FUNCTION: LEGOLAND 0x004215b0
void UnseatCastleVisitor(struct CastleVisitor *param_1) {
    struct TrainSeat *temp = param_1->seat;
    TakeTrainSeatOccupant(temp);
    param_1->seat = 0;
}

// FUNCTION: LEGOLAND 0x004215d0
struct CastleVisitor *CreateCastleVisitor(struct BlokeInfo *a1) {
    struct CastleVisitor *self = MemAlloc(sizeof(struct CastleVisitor), 0, 0, 0);
    if (self == NULL) {
        return NULL;
    }
    self->flags = 0;
    if (a1->sex == 0) {
        self->lms = GetLmsByName(SitLoManSitName);
        self->field_8 = GetLfmByName(SitLoManSitName);
    } else {
        self->lms = GetLmsByName(SitLoGirlSitName);
        self->field_8 = GetLfmByName(SitLoGirlSitName);
    }
    self->field_8 = BuildCastleVisitorModel(a1);
    if (self->field_8 == 0) {
        self->flags = 1;
    }
    self->draw = DrawCastleVisitor;
    self->unlink = FreeCastleVisitor;
    self->bloke = (struct Bloke *)a1->bloke;
    return self;
}

// FUNCTION: LEGOLAND 0x00421660
struct LmsRec *BuildCastleVisitorModel(struct BlokeInfo *p) {
    struct LmsFile *lms;
    struct LmsRec *src;
    struct LmsRec *dst;
    unsigned int *colours;
    unsigned int size;
    const char **names;
    struct KeyPair colPairs[2];
    struct KeyPair namePairs[2];
    char *nm;
    unsigned int *cp;
    int i;
    int k;
    struct LmsRef *r;

    if (p->sex == 0) {
        lms = GetLmsByName(SitLoManSitName);
        src = GetLfmByName(SitLoManSitName);
        size = GetLfmSizeByName(SitLoManSitName);
        names = ManVisitorTextures;
        colours = ManVisitorColours;
    } else {
        lms = GetLmsByName(SitLoGirlSitName);
        src = GetLfmByName(SitLoGirlSitName);
        size = GetLfmSizeByName(SitLoGirlSitName);
        names = GirlVisitorTextures;
        colours = GirlVisitorColours;
    }
    dst = MemAlloc(size, 0, 0, 0);
    if (dst == NULL) {
        return 0;
    }
    for (i = 0, nm = p->chest; i < 2; i++) {
        namePairs[i].key = FindLtxIndexByName(*names++);
        namePairs[i].val = FindLtxIndexByName(nm);
        nm += 0x14;
    }
    cp = &p->leg;
    for (i = 0; i < 2; i++) {
        colPairs[i].key = FindRollercoasterLptEntry(*colours++);
        colPairs[i].val = FindRollercoasterLptEntry(*cp);
        cp++;
    }
    memcpy(dst, src, size);
    for (i = 0; i < lms->field_2c; i++) {
        unsigned int key;
        r = lms->field_28 + i;
        key = src[r->idx].key;
        for (k = 0; k <= 1; k++) {
            if (namePairs[k].key == key) {
                key = namePairs[k].val;
                break;
            }
        }
        dst[r->idx].key = key;
    }
    for (i = 0; i < lms->field_1c; i++) {
        unsigned int key;
        r = lms->field_18 + i;
        key = src[r->idx].key;
        for (k = 0; k <= 1; k++) {
            if (colPairs[k].key == key) {
                key = colPairs[k].val;
                break;
            }
        }
        dst[r->idx].key = key;
    }
    for (i = 0; i < lms->field_24; i++) {
        unsigned int key;
        r = lms->field_20 + i;
        key = src[r->idx].key;
        for (k = 0; k <= 1; k++) {
            if (colPairs[k].key == key) {
                key = colPairs[k].val;
                break;
            }
        }
        dst[r->idx].key = key;
    }
    return dst;
}

// FUNCTION: LEGOLAND 0x00421890
struct BlokeInfo *GetBlokeAppearance(struct BlokeSex0 *bloke) {
    CastleVisitorInfo.bloke = bloke;
    CastleVisitorInfo.sex = GetSexOfBloke(bloke) != 0;
    CastleVisitorInfo.leg = GetLegColourOfBloke(bloke) & 0xffffff;
    CastleVisitorInfo.arm = GetArmColourOfBloke(bloke) & 0xffffff;
    strcpy(CastleVisitorInfo.chest, GetChestTextureNameOfBloke(bloke));
    strcpy(CastleVisitorInfo.face, GetFaceTextureNameOfBloke(bloke));
    return &CastleVisitorInfo;
}

// FUNCTION: LEGOLAND 0x00421930
struct CastleVisitor *AddCastleVisitor(struct Bloke *param, struct CastleState *timer) {
    struct CastleVisitor *node;
    struct CastleVisitor *old_head;

    node = CreateCastleVisitor(GetBlokeAppearance((struct BlokeSex0 *)param));

    old_head = timer->visitors.prev;
    old_head->next = node;
    old_head = timer->visitors.prev;
    node->prev = old_head;
    node->next = (struct CastleVisitor *)&timer->visitors;
    timer->visitors.prev = node;

    node->time = GetGameTimer();
    node->flags = 1;

    return node;
}

// FUNCTION: LEGOLAND 0x00421980
void FreeCastleVisitor(struct CastleVisitor *p) {
    p->prev->next = p->next;
    p->next->prev = p->prev;

    if ((p->flags & 1U) == 0U) {
        MemFree(p->field_8);
    }

    MemFree(p);
}

struct Vec5 {
    float field_0;
    float field_4;
    unsigned int field_8;
    float field_c;
    float field_10;
};

struct Vec3 {
    float field_0;
    float field_4;
    unsigned int field_8;
};

// FUNCTION: LEGOLAND 0x004219c0
void LineCurvePos(struct Vec5 *arg1, float float_arg, struct Vec3 *arg3) {
    arg3->field_0 = arg1->field_0 + (float_arg * arg1->field_c);
    arg3->field_4 = arg1->field_4 + (float_arg * arg1->field_10);
    arg3->field_8 = arg1->field_8;
}

struct Words3 {
    unsigned int field_0;
    unsigned int field_4;
    unsigned int field_8;
};

// FUNCTION: LEGOLAND 0x004219f0
void LineCurveTangent(unsigned char *param_1, unsigned int param_2, struct Words3 *param_3) {
    param_1 = param_1 + 12;
    param_3->field_0 = *(unsigned int *)param_1;
    param_3->field_4 = *(unsigned int *)(param_1 + 4);
    param_3->field_8 = *(unsigned int *)(param_1 + 8);
}

struct Vec7 {
    float field_0;
    float field_4;
    unsigned int field_8;
    float field_c;
    float field_10;
    unsigned char pad_14[4];
    float field_18;
    float field_1c;
};

// FUNCTION: LEGOLAND 0x00421a10
void LineCurveSideAPos(struct Vec7 *s1, float arg2, struct Vec3 *s2) {
    s2->field_0 = arg2 * s1->field_c + s1->field_18 + s1->field_0;
    s2->field_4 = arg2 * s1->field_10 + s1->field_1c + s1->field_4;
    s2->field_8 = s1->field_8;
}

// FUNCTION: LEGOLAND 0x00421a40
void LineCurveSideBPos(struct Vec7 *a, float c, struct Vec3 *b) {
    b->field_0 = c * a->field_c + a->field_0 - a->field_18;
    b->field_4 = c * a->field_10 + a->field_4 - a->field_1c;
    b->field_8 = a->field_8;
}

struct Struct1a70A {
    unsigned char pad_0[0x44];
    unsigned int field_44;
    unsigned int field_48;
};

// FUNCTION: LEGOLAND 0x00421a70
int LineCurveSplitPoints(struct Struct1a70A *param_1, struct Pair *param_2) {
    param_2->field_0 = param_1->field_44;
    param_2->field_4 = param_1->field_48;
    return 2;
}

struct Struct1a90 {
    unsigned int field_0;
    unsigned int field_4;
    float field_8;
};

// FUNCTION: LEGOLAND 0x00421a90
void LineCurveNormal(unsigned int param_1, unsigned int param_2, struct Struct1a90 *param_3) {
    param_3->field_0 = 0;
    param_3->field_4 = 0;
    param_3->field_8 = 1.0f;
}

struct Obj421ab0 {
    struct Words3 field_0;
    float field_c[3];
    struct Words3 field_18;
    float field_24[4];
    unsigned char pad_34[0xc];
    float field_40;
    unsigned int field_44;
    float field_48;
    void **field_4c;
    struct SegBlob *field_50; /* next */
    struct SegBlob *field_54; /* prev */
};

// FUNCTION: LEGOLAND 0x00421ab0
void InitLineCurve(struct Obj421ab0 *obj, float *a, float *b, struct Words3 *c) {
    float sum = FLOAT_004ab390;
    float d;
    int i;

    for (i = 0; i < 3; i++) {
        d = b[i] - a[i];
        obj->field_c[i] = d;
        sum += d * d;
    }
    obj->field_40 = (float)sqrt(sum);
    obj->field_0 = *(struct Words3 *)a;
    obj->field_18 = *c;
    obj->field_4c = &CurveFunctionTable[8];
    obj->field_44 = 0;
    obj->field_48 = 1.0f;
    obj->field_50 = NULL;
    obj->field_54 = NULL;
}

// FUNCTION: LEGOLAND 0x00421b40
void ArcCurvePos(float m[3][3], float angle, float out[3]) {
    int j;
    float s = (float)sin(angle);
    float c = (float)cos(angle);
    for (j = 0; j < 2; j++) {
        out[j] = c * m[0][j] + s * m[1][j] + m[2][j];
    }
    out[2] = m[2][2];
}

// FUNCTION: LEGOLAND 0x00421b90
void ArcCurveTangent(float m[3][3], float angle, float out[3]) {
    int j;
    float s = (float)sin(angle);
    angle = (float)cos(angle);
    s = -s;
    for (j = 0; j < 2; j++) {
        out[j] = angle * m[1][j] + s * m[0][j];
    }
    out[2] = 0.0f;
}

// FUNCTION: LEGOLAND 0x00421be0
void ArcCurveSideAPos(float m[12], float angle, float out[3]) {
    int j;
    float s = (float)sin(angle) * m[9];
    float c = (float)cos(angle) * m[9];
    for (j = 0; j < 2; j++) {
        out[j] = c * m[j] + s * m[j + 3] + m[j + 6];
    }
    out[2] = m[8];
}

// FUNCTION: LEGOLAND 0x00421c30
void ArcCurveSideBPos(float m[12], float angle, float out[3]) {
    int j;
    float s = (float)sin(angle) * m[10];
    float c = (float)cos(angle) * m[10];
    for (j = 0; j < 2; j++) {
        out[j] = c * m[j] + s * m[j + 3] + m[j + 6];
    }
    out[2] = m[8];
}

struct Floats6 {
    float field_0;
    float field_4;
    float field_8;
    float field_c;
    float field_10;
    float field_14;
};

// FUNCTION: LEGOLAND 0x00421c80
unsigned int GetQuarterCircleAngles(unsigned int param_1, struct Floats6 *ptr_0) {
    ptr_0->field_0 = 0.0f;
    ptr_0->field_4 = 0.31415900588035583f;
    ptr_0->field_8 = 0.6283180117607117f;
    ptr_0->field_c = 0.9424769878387451f;
    ptr_0->field_10 = 1.2566360235214233f;
    ptr_0->field_14 = 1.5707950592041016f;
    return 6;
}

// FUNCTION: LEGOLAND 0x00421cc0
void ArcCurveNormal(int param_1, int param_2, struct Struct1a90 *param_3) {
    param_3->field_0 = 0;
    param_3->field_4 = 0;
    param_3->field_8 = 1.0f;
}

struct Obj421ce0 {
    struct Words3 field_0;
    struct Words3 field_c;
    struct Words3 field_18;
    unsigned int field_24;
    unsigned int field_28;
    unsigned char pad_2c[0x18];
    unsigned int field_44;
    float field_48;
    void **field_4c;
    struct SegBlob *field_50; /* next */
    struct SegBlob *field_54; /* prev */
};

// FUNCTION: LEGOLAND 0x00421ce0
void InitArcCurve(struct Words3 *a, struct Words3 *b, struct Words3 *c, struct Obj421ce0 *obj, unsigned int d, unsigned int e) {
    obj->field_0 = *a;
    obj->field_c = *b;
    obj->field_18 = *c;
    obj->field_4c = &CurveFunctionTable[16];
    obj->field_24 = d;
    obj->field_28 = e;
    obj->field_44 = 0;
    obj->field_48 = 1.5707950592041016f;
    obj->field_50 = NULL;
    obj->field_54 = NULL;
}

struct Struct1d60 {
    float field_0;
    float field_4;
    unsigned char pad_8[4];
    float field_c;
    float field_10;
    unsigned char pad_14[16];
    float field_24;
    float field_28;
    float field_2c;
    float field_30;
};

struct Floats3 {
    float field_0;
    float field_4;
    float field_8;
};

// FUNCTION: LEGOLAND 0x00421d60
void SlopeCurvePos(struct Struct1d60 *a, float b, struct Floats3 *out) {
    out->field_0 = a->field_0 + b * a->field_c;
    out->field_4 = a->field_4 + b * a->field_10;
    out->field_8 = a->field_30 + b * (a->field_2c + b * (a->field_28 + b * a->field_24));
}

// FUNCTION: LEGOLAND 0x00421da0
void SlopeCurveTangent(unsigned char *param_1, float t, struct Words3 *out) {
    float *f = (float *)param_1;
    float v;
    *out = *(struct Words3 *)(param_1 + 12);
    v = t * f[9];
    v = v * FLOAT_004ab43c;
    v = v + (f[10] + f[10]);
    *(float *)&out->field_8 = v * t + f[11];
}

// FUNCTION: LEGOLAND 0x00421df0
void SlopeCurveSideAPos(float *a, float x, float *out) {
    out[0] = x * a[3] + a[6] + a[0];
    out[1] = x * a[4] + a[7] + a[1];
    out[2] = ((x * a[9] + a[10]) * x + a[11]) * x + a[12];
}

struct Struct1e40 {
    float field_0;
    float field_4;
    float pad_8;
    float field_c;
    float field_10;
    float pad_14;
    float field_18;
    float field_1c;
    float pad_20;
    float coef_t3;
    float coef_t2;
    float coef_t1;
    float coef_t0;
    unsigned char pad_34[0x44 - 0x34];
    float t_start;
    float t_end;
};

// FUNCTION: LEGOLAND 0x00421e40
void SlopeCurveSideBPos(struct Struct1e40 *ptr1, float multiplier, struct Floats3 *ptr2) {
    float temp;

    ptr2->field_0 = multiplier * ptr1->field_c + ptr1->field_0 - ptr1->field_18;
    ptr2->field_4 = multiplier * ptr1->field_10 + ptr1->field_4 - ptr1->field_1c;

    temp = multiplier * ptr1->coef_t3 + ptr1->coef_t2;
    temp = temp * multiplier + ptr1->coef_t1;
    temp = temp * multiplier + ptr1->coef_t0;
    ptr2->field_8 = temp;
}

// FUNCTION: LEGOLAND 0x00421e90
void SubdivideCurveAtMaxError(float x0, float y0, float x1, float y1) {
    float roots[2];
    float b;
    float by;
    float bx;
    float m;
    float best = 1.17549435e-38f;
    struct Struct1e40 *o = SplitCurve;
    float *r = roots;
    int i = 2;

    m = (y1 - y0) / (x1 - x0);
    b = y0 - m * x0;
    {
        float a;
        volatile float b2;
        volatile float sq;
        float c;
        a = o->coef_t3;
        a = a * FLOAT_004ab43c;
        b2 = o->coef_t2 + o->coef_t2;
        c = o->coef_t1 - m;
        sq = (float)sqrt(b2 * b2 - c * a * 4.0f);
        roots[0] = (sq - b2) / (a + a);
        roots[1] = (-b2 - sq) / (a + a);
    }
    for (; i > 0; i--, r++) {
        float y;
        float d;
        if (*r >= x0 && *r <= x1) {
            y = o->coef_t3;
            y = y * *r + o->coef_t2;
            y = y * *r + o->coef_t1;
            y = y * *r + o->coef_t0;
            d = (float)fabs(y - m * *r - b);
            if (d > best) {
                bx = *r;
                by = y;
                best = d;
            }
        }
    }
    if (fabs(best) > 1.0) {
        *SplitPointsNext++ = bx;
        SplitPointCount++;
        SubdivideCurveAtMaxError(x0, y0, bx, by);
        SubdivideCurveAtMaxError(bx, by, x1, y1);
    }
}

// FUNCTION: LEGOLAND 0x00422000
int BuildCurveSplitPoints(struct Struct1e40 *o, float *out) {
    int i;
    int j;
    float t;

    SplitCurveStart = o;
    SplitCurve = o;
    SplitPointCount = 2;
    SplitPointsNext = out;
    *SplitPointsNext = o->t_start;
    SplitPointsNext++;
    *SplitPointsNext = o->t_end;
    SplitPointsNext++;
    SubdivideCurveAtMaxError(o->t_start, ((o->t_start * o->coef_t3 + o->coef_t2) * o->t_start + o->coef_t1) * o->t_start + o->coef_t0, o->t_end, ((o->t_end * o->coef_t3 + o->coef_t2) * o->t_end + o->coef_t1) * o->t_end + o->coef_t0);
    for (i = SplitPointCount - 1; i >= 0; i--) {
        for (j = 0; j < i; j++) {
            if (out[j] > out[j + 1]) {
                t = out[j];
                out[j] = out[j + 1];
                out[j + 1] = t;
            }
        }
    }
    return SplitPointCount;
}

struct V3 {
    float x, y, z;
};

// FUNCTION: LEGOLAND 0x004220e0
void SlopeCurveNormal(float *obj, float t, float *out) {
    float c, v;

    *(struct V3 *)out = *(struct V3 *)(obj + 3);
    v = t * obj[9];
    out[2] = (v * FLOAT_004ab43c + obj[10] * 2) * t + obj[11];
    Vec3Normalize(out);
    if (fabs(out[1]) < DAT_004ab418) {
        c = out[2];
        if (*(unsigned int *)&out[0] & 0x80000000) {
            out[2] = -out[0];
            out[0] = c;
        } else {
            out[2] = out[0];
            out[0] = -c;
        }
    } else {
        c = out[2];
        if (*(unsigned int *)&out[1] & 0x80000000) {
            out[2] = -out[1];
            out[1] = c;
        } else {
            out[2] = out[1];
            out[1] = -c;
        }
    }
}

// FUNCTION: LEGOLAND 0x00422180
void InitSlopeCurve(float *a1, float *a2, struct Words3 *a3, struct Obj421ab0 *a4) {
    float q[4];
    float sum;
    float *p;
    float *out;
    int j;

    q[0] = a2[2];
    q[1] = a1[2];
    q[2] = 0.0f;
    q[3] = 0.0f;
    a2[2] = 0.0f;
    a1[2] = 0.0f;
    InitLineCurve(a4, a1, a2, a3);
    p = &HermiteBasis[0][0];
    out = a4->field_24;
    do {
        sum = FLOAT_004ab390;
        for (j = 0; j < 4; j++) {
            sum += q[j] * *p++;
        }
        *out++ = sum;
    } while ((INT_PTR)p <= (INT_PTR)&HermiteBasis[3][0]);
    a4->field_4c = &CurveFunctionTable[0];
}

// FUNCTION: LEGOLAND 0x00422210
void InitCurveFunctionTable(void) {
    CurveFunctionTable[0] = SlopeCurveSideAPos;
    CurveFunctionTable[1] = SlopeCurveTangent;
    CurveFunctionTable[3] = SlopeCurveTangent;
    CurveFunctionTable[5] = SlopeCurveTangent;
    CurveFunctionTable[9] = LineCurveTangent;
    CurveFunctionTable[11] = LineCurveTangent;
    CurveFunctionTable[13] = LineCurveTangent;
    CurveFunctionTable[2] = SlopeCurvePos;
    CurveFunctionTable[4] = SlopeCurveSideBPos;
    CurveFunctionTable[6] = BuildCurveSplitPoints;
    CurveFunctionTable[7] = SlopeCurveNormal;
    CurveFunctionTable[8] = LineCurveSideAPos;
    CurveFunctionTable[10] = LineCurvePos;
    CurveFunctionTable[12] = LineCurveSideBPos;
    CurveFunctionTable[14] = LineCurveSplitPoints;
    CurveFunctionTable[15] = LineCurveNormal;
    CurveFunctionTable[16] = ArcCurveSideAPos;
    CurveFunctionTable[17] = ArcCurveTangent;
    CurveFunctionTable[18] = ArcCurvePos;
    CurveFunctionTable[19] = ArcCurveTangent;
    CurveFunctionTable[20] = ArcCurveSideBPos;
    CurveFunctionTable[21] = ArcCurveTangent;
    CurveFunctionTable[22] = GetQuarterCircleAngles;
    CurveFunctionTable[23] = ArcCurveNormal;
}

// FUNCTION: LEGOLAND 0x004222f0
int IsCrLf(unsigned short *param_1) {
    return param_1[0] == 0xa0d;
}

// FUNCTION: LEGOLAND 0x00422300
unsigned char *CopyTextLine(unsigned char *src, unsigned char *dst) {
    if (IsCrLf((unsigned short *)src) == 0) {
        do {
            *dst = *src;
            dst++;
            src++;
        } while (IsCrLf((unsigned short *)src) == 0);
        return dst;
    }
    return dst;
}

// FUNCTION: LEGOLAND 0x00422340
unsigned char *FindLineStart(struct CoasterTextFile *range, int count) {
    int n = 0;
    unsigned char *p = range->data;
    unsigned char *end = p + range->size;

    if (count == 0) {
        return p;
    }
    while (p < end - 1) {
        if (IsCrLf((unsigned short *)p) != 0) {
            n++;
            p += 2;
            if (n == count) {
                return p;
            }
        } else {
            p++;
        }
    }
    return 0;
}

// FUNCTION: LEGOLAND 0x00422390
unsigned int GetTextLine(struct CoasterTextFile *a1, char *a2, unsigned int a3) {
    unsigned char *result1 = FindLineStart(a1, a3);
    if (result1 == 0)
        return 0;
    {
        unsigned char *p = CopyTextLine(result1, (unsigned char *)a2);
        p[0] = 0;
    }
    return 1;
}

// FUNCTION: LEGOLAND 0x004223c0
int CountTextLines(struct CoasterTextFile *range) {
    unsigned char *p = range->data;
    unsigned char *end = p + range->size;
    int n = 0;

    while (p < end - 1) {
        if (IsCrLf((unsigned short *)p) != 0) {
            n++;
            p += 2;
        } else {
            p++;
        }
    }
    return n;
}

// FUNCTION: LEGOLAND 0x00422400
int FindTextLine(struct CoasterTextFile *param_1, const char *param_2) {
    char buf[0x100];
    int i = 0;

    while (GetTextLine(param_1, buf, i) != 0) {
        if (_strcmpi(param_2, buf) == 0) {
            return i;
        }
        i++;
    }
    return -1;
}

// FUNCTION: LEGOLAND 0x00422470
void *ReadFileToBuffer(const char *fileName, unsigned int *bytesReadPtr) {
    HANDLE hFile;
    unsigned int fileSize;
    void *buffer;
    DWORD bytesRead;

    if (fileName == 0) {
        return 0;
    }

    hFile = CreateFileA(fileName, 0x80000000, 0x1, 0, 0x3, 0x8000000, 0);
    if (hFile == (HANDLE)-1) {
        return 0;
    }

    fileSize = GetFileSize(hFile, 0);
    buffer = MemAlloc(fileSize, 0, (unsigned int)(UINT_PTR)DAT_004d8bb0, 0);
    if (buffer == 0) {
        CloseHandle(hFile);
        return 0;
    }

    ReadFile(hFile, buffer, fileSize, &bytesRead, 0);
    if (bytesRead != fileSize) {
        MemFree(buffer);
        CloseHandle(hFile);
        return 0;
    }

    CloseHandle(hFile);
    *bytesReadPtr = fileSize;
    return buffer;
}

// FUNCTION: LEGOLAND 0x00422590
unsigned int FindCoasterObjLine(const char *s) {
    return FindTextLine(&CoasterObjFile, s);
}

// FUNCTION: LEGOLAND 0x004225b0
void GetCoasterObjLine(unsigned int param_1, char *param_2) {
    GetTextLine(&CoasterObjFile, param_2, param_1);
}

// FUNCTION: LEGOLAND 0x004225d0
unsigned int GetCoasterObjLineCount(void) {
    return CoasterObjLineCount;
}

// FUNCTION: LEGOLAND 0x00422600
unsigned int FindCoasterTxtLine(const char *param) {
    return FindTextLine(&CoasterTxtFile, param);
}

// FUNCTION: LEGOLAND 0x00422620
void FormatRollercoasterFileName(int value, char *buffer) {
    // STRING: LEGOLAND 0x004b5afc
    wsprintfA(buffer, "%s%04d", CoasterObjBaseName, value);
}

// FUNCTION: LEGOLAND 0x00422640
unsigned int GetCoasterTxtLineCount(void) {
    return CoasterTxtLineCount;
}

// FUNCTION: LEGOLAND 0x004226c0
int LoadObjAndTxtFiles(const char *name) {
    char buffer[256];

    // STRING: LEGOLAND 0x004b5b04
    wsprintfA(buffer, "%s.obj", name);
    CoasterObjFile.data = ReadFileToBuffer(buffer, &CoasterObjFile.size);
    if (CoasterObjFile.data == 0) {
        return 0;
    }
    // STRING: LEGOLAND 0x004b5b0c
    wsprintfA(buffer, "%s.txt", name);
    CoasterTxtFile.data = ReadFileToBuffer(buffer, &CoasterTxtFile.size);
    if (CoasterTxtFile.data == 0) {
        MemFree(CoasterObjFile.data);
        return 0;
    }
    CoasterObjLineCount = CountTextLines(&CoasterObjFile);
    CoasterTxtLineCount = CountTextLines(&CoasterTxtFile);
    strcpy(CoasterObjBaseName, name);
    return 1;
}

// FUNCTION: LEGOLAND 0x004227a0
void FreeObjAndTxtFiles(void) {
    MemFree(CoasterTxtFile.data);
    MemFree(CoasterObjFile.data);
}

struct MVert {
    int x;
    int y;
    int rest[3];
};
struct MEdge {
    int v0;
    int v1;
};
struct MTri {
    int e[3];
};
struct EdgeMesh {
    int f0;
    int nVerts;
    int nEdges;
    int nTris;
    struct MVert *verts;
    struct MEdge *edges;
    struct MTri *tris;
};

// FUNCTION: LEGOLAND 0x004227c0
struct EdgeMesh *EdgeMeshRemoveBackfaces(struct EdgeMesh *src) {
    int *triFlag;
    volatile int ptmp45;
    int nV = 0;
    int *edgeFlag;
    int j;
    int i;
    int removedTris = 0;
    int *vertFlag;
    int k;
    struct EdgeMesh *out = NULL;
    int nEdges = src->nEdges;
    int removedEdges = 0;
    int removedVerts = 0;

    for (i = 0; i < src->nTris; i++) {
        for (k = 0; k < 3; k++) {
            struct MEdge *ed = &src->edges[src->tris[i].e[k] & 0x7fffffff];
            if (ed->v0 > nV) {
                nV = ed->v0;
            }
            if (ed->v1 > nV) {
                nV = ed->v1;
            }
        }
    }
    nV++;
    triFlag = (int *)malloc(src->nTris * 4);
    edgeFlag = (int *)malloc(nEdges * 4);
    vertFlag = (int *)malloc(nV * 4);
    if (triFlag != NULL) {
        if (edgeFlag != NULL && vertFlag != NULL) {
            memset(triFlag, 0, src->nTris * 4);
            memset(edgeFlag, 0, nEdges * 4);
            memset(vertFlag, 0, nV * 4);
            for (i = 0; i < src->nTris; i++) {
                int v[3];
                float z;
                struct MVert *p0;
                struct MVert *p1;
                struct MVert *p2;
                double dx1;
                double dy1;
                double dx2;
                double dy2;
                unsigned int bits;
                for (k = 0; k < 3; k++) {
                    int e = src->tris[i].e[k];
                    if (e & 0x80000000) {
                        v[k] = src->edges[e & 0x7fffffff].v1;
                    } else {
                        ptmp45 = src->edges[e & 0x7fffffff].v0;
                        v[k] = ptmp45;
                    }
                }
                p0 = &src->verts[v[0]];
                p1 = &src->verts[v[1]];
                dx1 = p1->x - p0->x;
                dy1 = p1->y - p0->y;
                p2 = &src->verts[v[2]];
                dx2 = p2->x - p0->x;
                dy2 = p2->y - p0->y;
                z = dy2 * dx1 - dx2 * dy1;
                bits = *(unsigned int *)&z;
                if (bits & 0x80000000) {
                    triFlag[i] = 1;
                    removedTris++;
                }
            }
            for (i = 0; i < src->nTris; i++) {
                if (triFlag[i] != 0) {
                    for (k = 0; k < 3; k++) {
                        int found = 0;
                        for (j = 0; j < src->nTris; j++) {
                            if (triFlag[j] == 0) {
                                if (((src->tris[j].e[0] ^ ((int *)src->tris)[i * 3 + k]) & 0x7fffffff) == 0 || ((src->tris[j].e[1] ^ ((int *)src->tris)[i * 3 + k]) & 0x7fffffff) == 0 || ((src->tris[j].e[2] ^ ((int *)src->tris)[i * 3 + k]) & 0x7fffffff) == 0) {
                                    found = 1;
                                    break;
                                }
                            }
                        }
                        if (!found) {
                            edgeFlag[((int *)src->tris)[i * 3 + k] & 0x7fffffff] = 1;
                        }
                    }
                }
            }
            i = 0;
            while (i < nEdges) {
                if (edgeFlag[i] != 0) {
                    int *pv = &src->edges[i].v0;
                    for (k = 0; k < 2; k++) {
                        int found = 0;
                        for (j = 0; j < nEdges; j++) {
                            if (edgeFlag[j] == 0) {
                                if (*pv == src->edges[j].v0 || *pv == src->edges[j].v1) {
                                    found = 1;
                                    break;
                                }
                            }
                        }
                        if (!found) {
                            vertFlag[*pv] = 1;
                        }
                        pv++;
                    }
                }
                i++;
            }
            for (i = 0; i < nEdges; i++) {
                if (edgeFlag[i] != 0) {
                    removedEdges++;
                }
            }
            for (i = 0; i < nV; i++) {
                if (vertFlag[i] != 0) {
                    removedVerts++;
                }
            }
            out = (struct EdgeMesh *)malloc((nEdges - removedEdges) * 8 + (src->nTris - removedTris) * 12 + (nV - removedVerts) * 20 + 0x1c);
            if (out != NULL) {
                out->nEdges = nEdges - removedEdges - 1;
                out->nTris = src->nTris - removedTris;
                out->nVerts = nV - removedVerts - 1;
                out->edges = (struct MEdge *)(out + 1);
                out->tris = (struct MTri *)(out->edges + (nEdges - removedEdges));
                out->verts = (struct MVert *)(out->tris + (src->nTris - removedTris));
                j = 0;
                for (i = 0; i < nV; i++) {
                    if (vertFlag[i] == 0) {
                        out->verts[j] = src->verts[i];
                        vertFlag[i] = i - j;
                        j++;
                    }
                }
                for (i = 0, j = 0; nEdges > i; i++) {
                    if (edgeFlag[i] == 0) {
                        out->edges[j].v0 = src->edges[i].v0 - vertFlag[src->edges[i].v0];
                        out->edges[j].v1 = src->edges[i].v1 - vertFlag[src->edges[i].v1];
                        edgeFlag[i] = i - j;
                        j++;
                    }
                }
                j = 0;
                for (i = 0; i < src->nTris; i++) {
                    if (triFlag[i] == 0) {
                        int e = src->tris[i].e[0];
                        if (0x80000000 & e) {
                            out->tris[j].e[0] = (e - edgeFlag[e & 0x7fffffff]) | 0x80000000;
                        } else {
                            out->tris[j].e[0] = e - edgeFlag[e];
                        }
                        e = src->tris[i].e[1];
                        if (e & 0x80000000) {
                            out->tris[j].e[1] = (e - edgeFlag[e & 0x7fffffff]) | 0x80000000;
                        } else {
                            out->tris[j].e[1] = e - edgeFlag[e];
                        }
                        e = src->tris[i].e[2];
                        if (e & 0x80000000) {
                            out->tris[j].e[2] = (e - edgeFlag[e & 0x7fffffff]) | 0x80000000;
                        } else {
                            out->tris[j].e[2] = e - edgeFlag[e];
                        }
                        j++;
                    }
                }
            }
        }
        free(triFlag);
    }
    if (edgeFlag != NULL) {
        free(edgeFlag);
    }
    if (vertFlag != NULL) {
        free(vertFlag);
    }
    return out;
}

// FUNCTION: LEGOLAND 0x00422e10
void SetCoasterShadeTable(int param1, int index) {
    char *p = CoasterShadeTableData + (index << 7);
    CoasterShadeTables[index] = p;
    BuildShadeRamp(param1, p);
}

// FUNCTION: LEGOLAND 0x00422e40
void BuildShadeRamp(int param1, char *entry) {
    unsigned short *out;
    unsigned int gbits;
    unsigned int rshift;
    float rf, gf, bf;
    float dr, dg, db;
    double ar;
    double ag;
    double ab;
    int i;

    if (DisplayPixelFormat == 2) {
        gbits = 6;
        rshift = 11;
    } else {
        gbits = 5;
        rshift = 10;
    }
    ar = FLOAT_004ab390;
    ag = FLOAT_004ab390;
    ab = FLOAT_004ab390;
    rf = (float)(((unsigned int)param1 >> 19) & 0x1f);
    dr = rf * 0.03125f;
    gf = (float)(((unsigned int)param1 & 0xff00) >> (16 - gbits));
    dg = gf * 0.03125f;
    bf = (float)(((unsigned int)param1 >> 3) & 0x1f);
    db = bf * 0.03125f;
    out = (unsigned short *)entry;
    for (i = 0; i < 33; i++) {
        *out++ = (unsigned short)(((short)(int)ar << rshift) | ((int)ag << 5) | (int)ab);
        ar += dr;
        ag += dg;
        ab += db;
    }
    ar = rf;
    ag = gf;
    ab = bf;
    dr = (float)(FLOAT_004ab444 - ar) * 0.032258064f;
    dg = (float)((float)((1 << gbits) - 1) - ag) * 0.032258064f;
    db = (float)(FLOAT_004ab444 - ab) * 0.032258064f;
    out = (unsigned short *)entry + 32;
    for (i = 0; i < 32; i++) {
        *out++ = (unsigned short)(((short)(int)ar << rshift) | ((int)ag << 5) | (int)ab);
        ar += dr;
        ag += dg;
        ab += db;
    }
}

// FUNCTION: LEGOLAND 0x00422fe0
void BuildCoasterShadeTables(void) {
    int *p = GetRollercoasterLpt();
    int n = *p++;
    int i;

    CoasterShadeTableCount = n;
    CoasterShadeTableData = MemAlloc(n << 7, 0, 0, 0);
    if (CoasterShadeTableData == NULL) {
        BuildShadeRamp(0xffffff, DAT_00579878);
        for (i = 0; i < 1024; i++) {
            CoasterShadeTables[i] = DAT_00579878;
        }
        return;
    }
    for (i = 0; i < n; i++) {
        SetCoasterShadeTable(*p++, i);
    }
}

#pragma pack(push, 2)
struct RecEnt {
    short w;
    int d;
    short s0n;
};

struct RecBuf {
    int n;
    int x;
    short s0_first;
    struct RecEnt ent[1];
};
#pragma pack(pop)

// FUNCTION: LEGOLAND 0x00423140
void ClearCoasterDepth(int param_1) {
    /* Port [library:asm]: the original is inline asm. It flushes the queued draw records, timing itself with rdtsc. */
    unsigned int start = PortTimestamp();
    struct ListLink *node;
    struct RecBuf *rec;
    struct RecBuf *next;

    if (DepthSpanRecordsOverflow != 0) {
        /* full redraw requested: clear the whole back buffer */
        memset(DAT_004e3870, 0, 0x25800 * 4);
        CastleTrackCells[0].fp.v[0] = 10;
    } else {
        /* clear the dirty rectangles, then replay the records queued since the last flush */
        for (node = CastleLinkList.var_18; node != &CastleLinkList; node = node->var_18) {
            if (node->kind != 0) {
                ClearCoasterSurfaceRect((struct ClearRect *)node);
            }
        }
        for (rec = (struct RecBuf *)DepthSpanRecords; rec != DepthSpanRecordEnd; rec = next) {
            next = (struct RecBuf *)((char *)rec + rec->n * 8 + 8);
            ClearRecordedDepthSpans(rec);
        }
    }
    DepthSpanRecordEnd = (struct RecBuf *)DepthSpanRecords;
    DepthSpanRecordsOverflow = 0;
    DepthSpanRecordCount = 0;
    DepthClearCycles = PortTimestamp() - start;
}

// FUNCTION: LEGOLAND 0x00423200
void RecordDepthSpans(int n, int x, struct RecIdx *idx, struct RecSrc *src) {
    short *p = (short *)DepthSpanRecordEnd;
    struct RecBuf *saved = (struct RecBuf *)p;
    int i;

    DepthSpanRecordCount++;
    if ((char *)p + 0x10 > DAT_004e3870) {
        DepthSpanRecordsOverflow = 1;
        return;
    }
    if (n) {
        *(int *)p = n;
        saved->x = x;
        i = 0;
        if (n > 0) {
            p = (short *)saved + 5;
            for (; i < n; i++) {
                struct RecSrc *s = &src[idx->k];
                p[-1] = s->fx >> 16;
                *(int *)(p + 1) = s->d;
                p[0] = (short)idx->v;
                idx++;
                if (s->flag == 1) {
                    p[0] = -p[0];
                }
                p += 4;
            }
        }
        DepthSpanRecordEnd = (struct RecBuf *)((char *)saved + i * 8 + 8);
    }
}

// FUNCTION: LEGOLAND 0x004232b0
void ClearRecordedDepthSpans(struct RecBuf *rb) {
    /* The edge tables are one block: the fill reads the last line from the high half of src[n - 1].f0, and
     * writes idx[n].v (src[0] when n == 8). */
    struct {
        struct RecIdx idx[8];
        struct RecSrc src[8];
    } poly;
    int n = rb->n;
    int i;

    for (i = 0; i < n; i++) {
        short w = rb->ent[i].w;
        short w2 = rb->ent[i].w;

        if (w < 0) {
            poly.src[i].flag = 1;
            poly.idx[i].v = -w;
        } else {
            poly.src[i].flag = 0;
            poly.idx[i].v = w2;
        }
        poly.idx[i].k = i;
        poly.src[i].fx = rb->ent[i - 1].s0n << 16;
        poly.src[i].d = rb->ent[i].d;
    }
    ((short *)&poly.src[i - 1].f0)[1] = (short)(rb->x - 1);
    ClearDepthSpans(n, poly.idx, poly.src);
}

// FUNCTION: LEGOLAND 0x00423350
void ClearDepthSpans(int n, struct RecIdx *idx, struct RecSrc *src) {
    /* Port [library:asm]: the original is inline asm. Fills a polygon with 0 in the buffer at CoasterDepthBuffer. */
    PortFillPolygon(CoasterDepthBuffer, NULL, 0, 0, PORT_FILL_FLAT, n, idx, src);
}

// FUNCTION: LEGOLAND 0x00423480
void ClearCoasterSurfaceRect(struct ClearRect *r) {
    unsigned short *row = CoasterDepthBuffer + r->top * CoasterSurfacePitch;
    int y;
    int x;
    unsigned short *p;

    for (y = r->top; y <= r->bottom; y++) {
        p = row + r->left;
        for (x = r->left; x <= r->right; x++) {
            *p++ = 0;
        }
        row += CoasterSurfacePitch;
    }
}

// FUNCTION: LEGOLAND 0x004234e0
int DrawEdgeMeshTriangles(void *param1) {
    /* Port [library:asm]: the original is inline asm (rdtsc profiling). Draws every front-facing, not trivially clipped
     * triangle of a textured mesh through RasteriseTriangle, with the texture coordinate and depth as attributes. */
    struct TexMesh *mesh = (struct TexMesh *)param1;
    struct VideoArg video;
    struct PolyArg poly;
    struct PolyVert corner[3];
    struct MeshVert *mv[3];
    unsigned int start;
    unsigned int f;
    int i;
    int j;

    GetCoasterSurface(&video);
    start = PortTimestamp();
    poly.flags = 1;
    poly.fillers = port_texture_fillers;
    for (j = 0; j < 3; j++) {
        poly.v[j] = &corner[j];
    }
    for (i = 0; i < mesh->face_count; i++) {
        for (j = 0; j < 3; j++) {
            f = mesh->faces[i][j];
            mv[j] = &mesh->verts[mesh->corners[f & 0x7fffffff][f >> 31]];
        }
        poly.dx1 = (float)(mv[1]->x - mv[0]->x);
        poly.dy1 = (float)(mv[1]->y - mv[0]->y);
        poly.dx2 = (float)(mv[2]->x - mv[0]->x);
        poly.dy2 = (float)(mv[2]->y - mv[0]->y);
        poly.area = poly.dy2 * poly.dx1 - poly.dx2 * poly.dy1;
        if (poly.area <= FLOAT_004ab390) {
            continue; /* back-facing */
        }
        poly.and_codes = mv[0]->codes & mv[1]->codes & mv[2]->codes & 0xff;
        poly.or_codes = mv[0]->codes | mv[1]->codes | mv[2]->codes;
        if ((poly.or_codes & 0xf) != 0xf) {
            continue; /* entirely outside one edge of the view */
        }
        for (j = 0; j < 3; j++) {
            corner[j].y = mv[j]->y;
            corner[j].x = mv[j]->x;
            corner[j].attr[0] = mv[j]->u;
            corner[j].attr[1] = mv[j]->z;
        }
        poly.palette = mesh->palette;
        RasteriseTriangle(3, &poly);
    }
    EdgeMeshDrawCycles += PortTimestamp() - start;
    ReleaseCoasterSurface();
    return 1;
}

// FUNCTION: LEGOLAND 0x004236f0
unsigned int FPUSetSinglePrecision(void) {
    /* Port [library:asm]: the original reads the x87 control word with fstcw and, unless it already is, sets single
     * precision, round to nearest and all exceptions masked with fldcw. It returns the old word. */
    unsigned int old = _control87(0, 0);

    _control87(_PC_24 | _RC_NEAR | _MCW_EM, _MCW_PC | _MCW_RC | _MCW_EM);
    return old;
}

// FUNCTION: LEGOLAND 0x00423730
void FPURestoreControlWord(unsigned int control_word) {
    /* Port [library:asm]: the original restores the saved x87 control word with fldcw. */
    _control87(control_word, _MCW_PC | _MCW_RC | _MCW_EM);
}

// FUNCTION: LEGOLAND 0x00423740
void InitCoasterShading(void) {
    BuildCoasterShadeTables();
    InitShadeClampTable();
}

// FUNCTION: LEGOLAND 0x00423760
int GetCoasterSurface(struct VideoArg *arg) {
    int result = GetVideoSurface(arg);
    if (result == 0) {
        return 0;
    }
    CoasterSurfaceBits = arg->bits;
    CoasterSurfacePitch = arg->pitch >> 1;
    return 1;
}

// FUNCTION: LEGOLAND 0x00423790
void ReleaseCoasterSurface(void) {
}

// FUNCTION: LEGOLAND 0x004237a0
void DrawEdgeSet(struct EdgeSet *set) {
    int i;

    for (i = 0; i < set->count; i++) {
        DrawDottedLine(&set->base[set->pairs[i].a], &set->base[set->pairs[i].b], -1);
    }
}

// FUNCTION: LEGOLAND 0x004237f0
void DrawDottedLine(void *a, void *b, short c) {
    int *pa = (int *)a;
    int *pb = (int *)b;
    int pts[30][4];
    float x, y, dx, dy;
    int i;

    x = (float)pa[0];
    y = (float)pa[1];
    dx = (float)(pb[0] - pa[0]) * FLOAT_004ab44c;
    dy = (float)(pb[1] - pa[1]) * FLOAT_004ab44c;
    for (i = 0; i < 30; i++) {
        pts[i][0] = (int)x;
        pts[i][1] = (int)y;
        x += dx;
        y += dy;
    }
    PlotClippedPoints(&pts[0][0], 30, c);
}

// FUNCTION: LEGOLAND 0x004238a0
void PlotClippedPoints(int *points, int count, short c) {
    struct VideoArg surf;
    int x;
    int y;

    if (GetSprite(&surf, NULL)) {
        while (count-- > 0) {
            x = points[0];
            if (x > DAT_008299ac[0] && x < DAT_008299ac[2]) {
                y = points[1];
                if (y > DAT_008299ac[1] && y < DAT_008299ac[3]) {
                    ((unsigned short *)surf.bits)[x + (surf.pitch >> 1) * y] = c;
                }
            }
            points += 4;
        }
        ReleaseSprite(&surf);
    }
}

// FUNCTION: LEGOLAND 0x00423930
unsigned int IsCastleCoasterPlaced(void) {
    return CastleCoasterPlaced;
}

// FUNCTION: LEGOLAND 0x00423940
void CastleFillOpenEnd(struct PathSeg *arg1) {
    arg1->dir_a = 2;
    arg1->f18 = (float)CastlePieceClass.f8;
    arg1->dir_b = 8;
    arg1->f24 = (float)CastlePieceClass.f4;
}

// FUNCTION: LEGOLAND 0x00423970
void CastleSetHeights(struct PathSeg *param_1) {
    param_1->f18 = (float)CastlePieceClass.f8;
    param_1->f24 = (float)CastlePieceClass.f4;
}

// FUNCTION: LEGOLAND 0x00423990
struct SegBlob *CastleGetCurve(void) {
    return CastleCurves;
}

// FUNCTION: LEGOLAND 0x004239a0
void FUN_004239a0(void) {
}

// FUNCTION: LEGOLAND 0x004239b0
void GetCastleCoasterDoorA(struct Int16Pair *a, struct Int16Pair *b) {
    unsigned char *p = (unsigned char *)&CastleCoasterRide->footprint;
    b->x = *(unsigned short *)(p + 8) + a->x;
    b->y = *(unsigned short *)(p + 4) + a->y - 2;
}

// FUNCTION: LEGOLAND 0x004239e0
void GetCastleCoasterDoorB(struct Int16Pair *a, struct Int16Pair *b) {
    unsigned char *global_ptr = (unsigned char *)&CastleCoasterRide->footprint;
    b->x = *(unsigned short *)global_ptr + a->x - 2;
    b->y = *(unsigned short *)(global_ptr + 0xc) + a->y;
}

// FUNCTION: LEGOLAND 0x00423a10
void BuildCastleTrack(void) {
    struct Int16Pair origin;
    struct Int16Pair q;
    struct Int16Pair p;
    struct Int16Pair r;
    struct Int16Pair pt;
    struct Int16Pair pt1;
    struct FVec3 v1;
    struct FVec3 v2;
    struct FVec3 v3;
    struct FVec3 c;
    float t;
    float step;
    int n;
    int i;
    int cnt = 0;

    origin.x = 0;
    origin.y = 0;
    GetCastleCoasterDoorB(&origin, &q);
    GetCastleCoasterDoorA(&origin, &p);
    r.x = q.x;
    r.y = p.y;
    pt.x = r.x + 1;
    pt.y = q.y + 2;
    pt1.x = r.x + 1;
    pt1.y = r.y + 2;
    MapPosToWorld(&pt, 0.0f, &v1);
    MapPosToWorld(&pt1, 0.0f, &v2);
    c.x = -10.0f;
    c.y = 0.0f;
    c.z = 0.0f;
    InitLineCurve((struct Obj421ab0 *)&CastleCurves[0], &v1.x, &v2.x, (struct Words3 *)&c);
    n = (q.y - r.y) >> 1;
    t = CastleCurves[0].start;
    pt = q;
    step = (CastleCurves[0].end - CastleCurves[0].start) / n;
    for (i = 0; i < n; i++) {
        CastleTrackCells[cnt].xy = pt;
        pt.y -= 2;
        CastleTrackCells[cnt].k = 0;
        CastleTrackCells[cnt].a = t;
        t += step;
        CastleTrackCells[cnt].b = t;
        cnt++;
    }
    pt.x = -1;
    pt.y = 0;
    MapPosToWorld(&pt, 0.0f, &v1);
    pt.x = 0;
    pt.y = -1;
    MapPosToWorld(&pt, 0.0f, &v2);
    pt.x = r.x + 2;
    pt.y = r.y + 2;
    MapPosToWorld(&pt, 0.0f, &v3);
    InitArcCurve((struct Words3 *)&v1, (struct Words3 *)&v2, (struct Words3 *)&v3, (struct Obj421ce0 *)&CastleCurves[1], 0x3fc00000, 0x3f000000);
    CastleTrackCells[cnt].xy = r;
    CastleTrackCells[cnt].k = 1;
    CastleTrackCells[cnt].a = CastleCurves[1].start;
    CastleTrackCells[cnt].b = CastleCurves[1].end;
    cnt++;
    pt.x = r.x + 2;
    pt.y = r.y + 1;
    pt1.x = p.x + 2;
    pt1.y = p.y + 1;
    MapPosToWorld(&pt, 0.0f, &v1);
    MapPosToWorld(&pt1, 0.0f, &v2);
    c.x = 0.0f;
    c.y = -10.0f;
    c.z = 0.0f;
    InitLineCurve((struct Obj421ab0 *)&CastleCurves[2], &v1.x, &v2.x, (struct Words3 *)&c);
    CastleCurves[0].prev = 0;
    CastleCurves[0].next = &CastleCurves[1];
    CastleCurves[1].prev = &CastleCurves[0];
    CastleCurves[1].next = &CastleCurves[2];
    CastleCurves[2].prev = &CastleCurves[1];
    CastleCurves[2].next = 0;
    n = (p.x - r.x) >> 1;
    t = CastleCurves[2].start;
    step = (CastleCurves[2].end - CastleCurves[2].start) / n;
    pt = r;
    pt.x = r.x + 2;
    for (i = 0; i < n; i++) {
        CastleTrackCells[cnt].xy = pt;
        pt.x += 2;
        CastleTrackCells[cnt].k = 2;
        CastleTrackCells[cnt].a = t;
        t += step;
        CastleTrackCells[cnt].b = t;
        cnt++;
    }
    for (i = 0; i < cnt; i++) {
        CastleTrackCells[i + 1].fp.v[0] = CastleTrackCells[i].xy.x;
        CastleTrackCells[i + 1].fp.v[1] = CastleTrackCells[i].xy.y;
        CastleTrackCells[i + 1].fp.v[2] = CastleTrackCells[i].xy.x + 1;
        CastleTrackCells[i + 1].fp.v[3] = CastleTrackCells[i].xy.y + 1;
        CastleTrackCells[i + 1].fp.next = &CastleTrackCells[i + 2].fp;
    }
    CastleTrackCellCount = cnt;
    CastleTrackCells[i].fp.next = 0;
}

// FUNCTION: LEGOLAND 0x00423d40
void InitCastleCoasterDoors(void) {
    struct Int16Pair a;
    struct Int16Pair b;
    struct Int16Pair c;

    a.x = 0;
    a.y = 0;
    GetCastleCoasterDoorA(&a, &b);
    GetCastleCoasterDoorB(&a, &c);
    CastlePieceClass.door_a.x = b.x;
    CastlePieceClass.door_a.y = b.y;
    CastlePieceClass.door_b.x = c.x;
    CastlePieceClass.door_b.y = c.y;
    BuildCastleTrack();
}

// FUNCTION: LEGOLAND 0x00423db0
void InitCastleCoaster(void) {
    CastleCoaster.method_10c = GetCastleStationStart;
    CastleCoaster.method_110 = CastleStepLeaving;
    CastleCoaster.method_114 = CastleStepArriving;
    InitCastleCoasterDoors();
}

// FUNCTION: LEGOLAND 0x00423de0
void BuildCastleBaseFootprint(void) {
    struct Ride *v0 = CastleCoasterRide;
    memcpy(&CastleCoasterFootprint, &v0->footprint, sizeof(struct Footprint));
    CastleCoasterFootprint.next = &CastleTrackCells[1].fp;
    CastleTrackCells[CastleTrackCellCount].fp.next = 0;
}

// FUNCTION: LEGOLAND 0x00423e20
void BuildCastleCoasterFootprint(void) {
    struct PathSeg *n;
    struct Footprint *tail;

    CastleCoasterFootprint = CastleCoasterRide->footprint;
    CastleCoasterFootprint.next = &CastleTrackCells[1].fp;
    CastleTrackCells[CastleTrackCellCount].fp.next = 0;
    tail = &CastleTrackCells[CastleTrackCellCount].fp;
    if (CastleCoaster.flags == 2) {
        for (n = (struct PathSeg *)CastleCoaster.piece.next; n != (struct PathSeg *)&CastleCoaster.piece.flags; n = n->next) {
            tail->next = &n->fp;
            tail = &n->fp;
        }
        tail->next = 0;
        return;
    }
    for (n = (struct PathSeg *)CastleCoaster.piece.next; n != 0; n = n->next) {
        tail->next = &n->fp;
        tail = &n->fp;
    }
    for (n = (struct PathSeg *)CastleCoaster.piece.prev; n != 0; n = n->prev) {
        tail->next = &n->fp;
        tail = &n->fp;
    }
    tail->next = 0;
}

// FUNCTION: LEGOLAND 0x00423ec0
void RemoveAllCoasterPieces(struct CastleState *castle) {
    struct PathSeg *n;
    struct PathSeg *next;
    struct PathSeg *end;

    if (CastleCoaster.flags == 2) {
        n = (struct PathSeg *)CastleCoaster.piece.next;
        while (n != (struct PathSeg *)&CastleCoaster.piece.flags) {
            next = n->next;
            RemoveCoasterPiece(n);
            n = next;
        }
        return;
    }
    n = (struct PathSeg *)CastleCoaster.f_c0;
    end = (struct PathSeg *)&castle->piece;
    while (n != end) {
        next = n->next;
        RemoveCoasterPiece(n);
        n = next;
    }
    n = (struct PathSeg *)CastleCoaster.f_a8;
    while (n != end) {
        next = n->prev;
        RemoveCoasterPiece(n);
        n = next;
    }
}

// FUNCTION: LEGOLAND 0x00423f40
int GetPieceMapLine(struct Point *pos, float *a2, struct Point *a3, float *a4, struct PathSeg *node, int *a6) {
    struct FVec3 a;
    struct FVec3 b;
    struct SegBlob *c;
    struct PathSeg *n2;

    *a6 = node->prev == (struct PathSeg *)&CastleCoaster.piece.flags;
    c = GetPieceCurve(node, &a);
    c->vt->method_8(c, (c->end + c->start) * 0.5f, &b.x);
    b.z += a.z;
    *a2 = b.z * -2.0f;
    n2 = node->next;
    if (n2 != 0 && n2 != (struct PathSeg *)&CastleCoaster.piece.flags) {
        c = GetPieceCurve(n2, &a);
        c->vt->method_8(c, (c->end + c->start) * 0.5f, &b.x);
        b.z += a.z;
        *a4 = b.z * -2.0f;
        a3->x = n2->x;
        a3->y = n2->y;
        return 1;
    }
    *a3 = *pos;
    if (node->next == (struct PathSeg *)&CastleCoaster.piece.flags) {
        a3->y -= 16;
    }
    *a4 = 0.0f;
    return 1;
}

// FUNCTION: LEGOLAND 0x00424050
int GetCoasterPieceMapLine(struct Point *pos, void *a2, void *a3, void *a4, void *a5) {
    struct PathSeg *n;
    struct PathSeg *next;

    if (CastleCoaster.flags == 2) {
        n = (struct PathSeg *)CastleCoaster.piece.next;
        while (n != (struct PathSeg *)&CastleCoaster.piece.flags) {
            next = n->next;
            if (n->x == pos->x && n->y == pos->y) {
                return GetPieceMapLine(pos, a2, a3, a4, n, a5);
            }
            n = next;
        }
        return 0;
    }
    n = (struct PathSeg *)CastleCoaster.f_c0;
    while (n != (struct PathSeg *)&CastleCoaster.piece.flags) {
        next = n->next;
        if (n->x == pos->x && n->y == pos->y) {
            return GetPieceMapLine(pos, a2, a3, a4, n, a5);
        }
        n = next;
    }
    n = (struct PathSeg *)CastleCoaster.f_a8;
    while (n != (struct PathSeg *)&CastleCoaster.piece.flags) {
        next = n->prev;
        if (n->x == pos->x && n->y == pos->y) {
            return GetPieceMapLine(pos, a2, a3, a4, n, a5);
        }
        n = next;
    }
    return 0;
}

// FUNCTION: LEGOLAND 0x00424140
void *GetPlacedCastleCoaster(void) {
    return CastleCoasterPlaced ? &CastleCoaster : 0;
}

// FUNCTION: LEGOLAND 0x00424150
void CastleObjLoad(struct Element *param_1) {
    CastleCoasterPlaced = 0;
    FUN_00477400();
    CastleObjElement = param_1;
    CastleCoasterRide = param_1->ride;
    if (CastleCoasterRide->layer != NULL) {
        CastleCoasterRide->layer->flags |= 0x2000;
    }
    CastleCoasterRide->flags |= 0x20;
    CastleMatteSprite = LoadSprite("Castle Matte.lls", 1);
    InitCastleView();
    InitVecMathOps();
    LoadRollercoasterFiles();
    InitCoasterShading();
    InitCurveFunctionTable();
    InitTrackDrawing();
    FUN_0041ef00();
    InitCurveVecMath();
    InitCastleCoaster();
    InitTrainVecMath();
    LoadCoasterTrainCarModels();
    LoadCoasterTrainWheelModel();
}

// FUNCTION: LEGOLAND 0x004241e0
void CastleObjUnload(void) {
    KillSprite(CastleMatteSprite);
    CastleCoaster.pts_a.mask = 0;
    CastleCoaster.pts_b.mask = 0;
    ClearDirPoints(&CastleCoaster.pts_a);
    ClearDirPoints(&CastleCoaster.pts_b);
    CastleCoaster.f_a8 = 0;
    CastleCoaster.f_c0 = 0;
    ResetPieceEnd((struct PieceEnd *)&CastleCoaster.piece.end_a);
    ResetPieceEnd((struct PieceEnd *)&CastleCoaster.piece.end_b);
    CastleCoaster.flags = 0;
}

// FUNCTION: LEGOLAND 0x00424240
unsigned int CastleObjSetEditMode(void) {
    if (CastleCoasterPlaced != 0) {
        return CastleCoasterPlaced;
    }
    EditMode.unk0 = 1;
    EditMode.unk8 = CastleCoasterRide;
    DefaultCursor(&EditCursor);
    BuildCastleBaseFootprint();
    return ((unsigned int (*)(const char *))SetEditCursorFootPrint)((const char *)&CastleCoasterFootprint); /* TODO: fold — SetEditCursorFootPrint leaves memcpy's eax, used here as uint return */
}

// FUNCTION: LEGOLAND 0x00424280
void CastleObjCalcCursor(struct Element *param_1, int *param_2, unsigned int param_3) {
    struct Cursor *cursor;

    BuildCastleBaseFootprint();
    SetEditCursorFootPrint(&CastleCoasterFootprint);
    ScreenToMapRef((int *)param_2, &EditCursor.tile_x, param_3);
    EditCursor.field_1830 = 0;
    cursor = GetOrAddPathCursor(&EditCursor);
    if (cursor != NULL) {
        cursor->field_1414[1]++;
        cursor->field_1414[0]++;
    }
    ResetCursorBuildable(&EditCursor);
    ValidateCursor(&EditCursor, param_1->data);
    if (CastleCoasterPlaced != 0) {
        SetCursorBlocked(&EditCursor, 0xf);
        PropagateCursorBlocked(&EditCursor);
    }
}

// FUNCTION: LEGOLAND 0x00424320
void PlaceCastleCoaster(struct Element *obj, short *pt) {
    short v[2];

    SetCoasterPieceCount(0);
    AddMapObjectIfEnabled(obj, (int *)pt);
    v[0] = pt[0];
    v[1] = pt[2];
    AddCastleCoasterMapObjects(v);
    InitPieceHead(&CastleCoaster.piece);
    CastleCoaster.flags = 1;
    CastleCoaster.piece.x = pt[0];
    CastleCoaster.piece.y = pt[2];
    CastleCoaster.piece.cls = &CastlePieceClass;
    CastleCoaster.piece.owner = &CastleCoaster;
    CastleCoaster.piece.ride = CastleCoasterRide;
    CastleCoaster.f_a8 = (struct PathSeg *)&CastleCoaster.piece;
    CastleCoaster.f_c0 = (struct PathSeg *)&CastleCoaster.piece;
    CastleCoaster.pts_a.mask = 0;
    CastleCoaster.pts_b.mask = 0;
    FindPathSegDoorAPoints((struct PathSeg *)&CastleCoaster.piece, &CastleCoaster.pts_a);
    FindPathSegDoorBPoints((struct PathSeg *)&CastleCoaster.piece, &CastleCoaster.pts_b);
    FUN_0041d1c0(&CastleCoaster.pts_a.mask);
    FUN_0041d1c0(&CastleCoaster.pts_b.mask);
    PieceFillOpenEnd((struct PathSeg *)&CastleCoaster.piece.flags);
    PieceSetHeights((struct PathSeg *)&CastleCoaster.piece, 0);
    CreateCastleTrain((struct CastleState *)&CastleCoaster);
    InitCastleVisitorList((struct CastleState *)&CastleCoaster);
    CastleCoaster.piece.flags |= 6;
    CastleCoasterPlaced = 1;
}

// FUNCTION: LEGOLAND 0x00424440
void CastleObjDCalcCursor(struct Element *param_1, struct Point *param_2) {
    int x;
    int y;

    BuildCastleCoasterFootprint();
    x = CastleCoaster.piece.x;
    memcpy(QueryCursor.field_1414, &CastleCoasterFootprint, sizeof(struct Footprint));
    y = CastleCoaster.piece.y;
    QueryCursor.tile_x = x;
    QueryCursor.tile_y = y;
    // STRING: LEGOLAND 0x004b5b7c
    if ((void *)QueryClass != ElemID("CASTLE_DUMMY")->data) {
        ResetCursorBuildable(&QueryCursor);
        return;
    }
    QueryCursor.field_140c = 0;
}

// FUNCTION: LEGOLAND 0x004244b0
void RemoveCastleCoaster(struct Element *param_1, TileId param_2, struct Cursor *param_3) {
    struct Cursor cursor;
    TileId tile;
    struct Element *handle;

    if (CastleCoasterPlaced != 0) {
        tile.pos.x = CastleCoaster.piece.x;
        tile.pos.y = CastleCoaster.piece.y;
        cursor.tile_x = tile.pos.x;
        cursor.tile_y = tile.pos.y;
        cursor.footprint = CastleCoasterRide->footprint;
        cursor.field_1414[4] = 0;
        StopCastleTrain((struct CastleState *)&CastleCoaster);
        RemoveAllCoasterPieces(&CastleCoaster);
        ClearDirPoints(&CastleCoaster.pts_a);
        ClearDirPoints(&CastleCoaster.pts_b);
        CastleCoaster.f_a8 = 0;
        CastleCoaster.f_c0 = 0;
        DestroyCastleTrain((struct CastleState *)&CastleCoaster);
        FreeAllCastleVisitors((struct CastleState *)&CastleCoaster);
        CastleRemoveAllRiders();
        handle = GetCastleDispatchElement(0);
        RemoveMapObjectIfEnabled(handle, tile, &cursor);
        RemoveCastleCoasterMapObjects(&CastleCoaster.piece.pos);
        CastleCoasterPlaced = 0;
        FUN_004775f0();
        FreeMoviePool();
        SetCoasterPieceCount(0);
    }
}

// FUNCTION: LEGOLAND 0x004245b0
void AddCastleCoasterMapObjects(short *param_1) {
    int pt[2];
    int i;

    for (i = 0; i < (int)CastleTrackCellCount; i++) {
        short *q = &CastleTrackCells[i].xy.y;
        pt[0] = q[-1] + param_1[0];
        pt[1] = param_1[1] + q[0];
        AddMapObjectIfEnabled(CastleDummyElement, pt);
    }
    AddCoasterPieceCount(CastleTrackCellCount);
}

// FUNCTION: LEGOLAND 0x00424620
void RemoveCastleCoasterMapObjects(struct Int16Pair *pos) {
    struct Cursor cursor;
    TileId tile;
    int i;

    for (i = 0; i < (int)CastleTrackCellCount; i++) {
        tile.pos.x = CastleTrackCells[i].xy.x + pos->x;
        tile.pos.y = CastleTrackCells[i].xy.y + pos->y;
        cursor.tile_x = tile.pos.x;
        cursor.tile_y = tile.pos.y;
        cursor.footprint = CastleDummyRide->footprint;
        cursor.field_1414[4] = 0;
        RemoveMapObjectIfEnabled(CastleDummyElement, tile, &cursor);
    }
    AddCoasterPieceCount(-(int)CastleTrackCellCount);
}

// FUNCTION: LEGOLAND 0x004246e0
unsigned int CastleObjPieceCount(struct Element *param_1, unsigned int param_2) {
    if (param_2 != 0 && CastleCoaster.flags != 2) {
        return 0;
    }
    return GetCoasterPieceCount();
}

// FUNCTION: LEGOLAND 0x00424700
void RenderCastleDummy(struct Element *a1, unsigned int a2, unsigned int a3, unsigned char *p) {
    struct FVec3 buf;
    struct SegBlob obj;
    short key[2];
    int want;
    int i;

    MapPosToWorld(&CastleCoaster.piece.pos, (float)CastlePieceClass.f8, &buf);
    key[0] = p[0];
    key[1] = p[1];
    want = *(int *)key;
    for (i = 0; i < (int)CastleTrackCellCount; i++) {
        key[0] = CastleTrackCells[i].xy.x + CastleCoaster.piece.x;
        key[1] = CastleTrackCells[i].xy.y + CastleCoaster.piece.y;
        if (want == *(int *)key) {
            obj = ((struct SegBlob *)&CastleCurves)[CastleTrackCells[i].k];
            obj.start = CastleTrackCells[i].a;
            obj.end = CastleTrackCells[i].b;
            DrawTrackStretch(&obj, (struct FVec3 *)&buf, 1, 0);
            break;
        }
    }
    DrawCastleTrainOnPiece((struct PieceHead *)&CastleCoaster.piece.flags);
}

// FUNCTION: LEGOLAND 0x00424800
void CastleDummyRemoveObject(void) {
    TileId tile;

    tile.pos.x = 0;
    tile.pos.y = 0;
    RemoveCastleCoaster(0, tile, 0);
}

// FUNCTION: LEGOLAND 0x00424820
void CastleDummyDCalcCursor(void) {
    CastleObjDCalcCursor(0, 0);
}

// FUNCTION: LEGOLAND 0x00424830
void CastleDummyLoad(struct Element *param_1) {
    struct Ride *inner;

    inner = param_1->ride;
    CastleDummyElement = param_1;
    CastleDummyRide = inner;
    inner->layer->flags |= 0x2000;
}

// FUNCTION: LEGOLAND 0x00424850
void GetCastleStationStart(struct CastleState *p1, struct SegBlob **p2, float *p3, struct FVec3 *p4) {
    *p2 = &CastleCurves[2];
    *(unsigned int *)p3 = 0x3dcccccd;
    MapPosToWorld(&p1->piece.pos, (float)CastlePieceClass.f8, p4);
}

// FUNCTION: LEGOLAND 0x00424890
float CastleStepLeaving(struct Train *param_1, float param_2) {
    return StepTrainToSegmentEnd(param_1, param_2);
}

// FUNCTION: LEGOLAND 0x004248b0
void CastleBrakingDeriv(struct Integrator *obj, float unused, struct FloatArray *out) {
    struct Train *e = obj->owner;
    struct CastleState *sub = e->track;
    struct FVec3 v;
    void *s[2]; /* the piece and stretch of a CurveWalker (the rest is not read) */
    float t;
    float r;

    TrainIntegratorDeriv(obj, unused, out);
    t = GetTrainTrackSpeed(e);
    MapPosToWorld(&sub->piece.pos, (float)CastlePieceClass.f8, &v);
    r = t * t;
    s[0] = &sub->piece;
    s[1] = &CastleCurves[2];
    r = -(r / (CurveArcLength((struct CurveWalker *)s, 0x3dcccccd, &e->walker, *(unsigned int *)&e->motion.f4, 1, 0) * FLOAT_004ab404 * 2));

    out->data[1] = SumTrainCarMasses(e) * r * t;
}

// FUNCTION: LEGOLAND 0x00424960
int HasTrainReachedStation(struct Train *param_1) {
    if (param_1->walker.node != &CastleCurves[2]) {
        return 0;
    }
    if (param_1->motion.f4 > FLOAT_004ab454) {
        return 1;
    }
    return 0;
}

// FUNCTION: LEGOLAND 0x00424990
float CastleStepArriving(struct Train *self, float value) {
    if (HasTrainReachedStation(self) != 0) {
        StopCastleTrain(self->track);
        self->track->field_e0 = GetGameTimer();
        return value;
    }
    self->integ.deriv = CastleBrakingDeriv;
    return StepTrainToSegmentEnd(self, value);
}

// FUNCTION: LEGOLAND 0x004249e0
void CreateCastleTrain(struct CastleState *param_1) {
    param_1->field_d8 = CreateTrain(param_1);
    param_1->field_e0 = 0;
}

// FUNCTION: LEGOLAND 0x00424a00
void DestroyCastleTrain(struct CastleState *param_1) {
    DestroyTrain(&param_1->field_d8);
}

// FUNCTION: LEGOLAND 0x00424a20
void DrawCastleTrainOnPiece(struct PieceHead *param_1) {
    struct Train *obj = param_1->owner->field_d8;
    if (AnyCastleLinkVisible() != 0) {
        DrawTrainCarsOnPiece(obj, (struct PathSeg *)param_1);
    }
}

// FUNCTION: LEGOLAND 0x00424a50
void UpdateCastleTrain(struct CastleState *param_1) {
    struct Train *host = (struct Train *)param_1->field_d8;

    ClearAllTrainCarsDrawn(host);
    if (IsTrainRunning(param_1->field_d8) != 0) {
        if (IsTrainStartDelayed(param_1->field_d8) != 0) {
            UpdateTrainStartDelay((struct Train *)host);
        }
        StepTrainForFrame(host);
    }
    UpdateAllTrainCarBounds(host);
}

// FUNCTION: LEGOLAND 0x00424ab0
void StopCastleTrain(struct CastleState *this) {
    if (IsTrainRunning(this->field_d8) != 0) {
        StopTrain(this->field_d8);
        UnseatAllCastleVisitors((struct CastleState *)this);
    }
}

// FUNCTION: LEGOLAND 0x00424ae0
void StartCastleTrain(struct CastleState *param_1, unsigned int param_2) {
    ResetTrainToStation(param_1->field_d8);
    StartTrain((struct Train *)param_1->field_d8, param_2);
}

// FUNCTION: LEGOLAND 0x00424b10
void InitCastleVisitorList(struct CastleState *param_1) {
    struct CastleVisitor *addr = &param_1->visitors;
    param_1->visitors.next = addr;
    param_1->visitors.prev = addr;
}

// FUNCTION: LEGOLAND 0x00424b30
struct CastleVisitor *FindWaitingCastleVisitor(struct CastleState *list) {
    struct CastleVisitor *node = list->visitors.next;
    struct CastleVisitor *list_end = (struct CastleVisitor *)&list->visitors;
    while (node != list_end && node->flags != 1) {
        node = node->next;
    }
    return (node == list_end) ? 0 : node;
}

// FUNCTION: LEGOLAND 0x00424b60
struct CastleVisitor *FindSeatedCastleVisitor(struct CastleState *list) {
    struct CastleVisitor *node = list->visitors.next;
    struct CastleVisitor *list_end = (struct CastleVisitor *)&list->visitors;
    while (node != list_end && node->flags != 2) {
        node = node->next;
    }
    return (node == list_end) ? 0 : node;
}

// FUNCTION: LEGOLAND 0x00424b90
void *FindFinishedCastleVisitor(struct CastleState *list) {
    struct CastleVisitor *cur = list->visitors.next;
    struct CastleVisitor *end = (struct CastleVisitor *)&list->visitors;

    while (cur != end && cur->flags != 4) {
        cur = cur->next;
    }
    return (cur == end) ? 0 : cur;
}

// FUNCTION: LEGOLAND 0x00424bc0
unsigned int GetLongestCastleVisitorWait(struct CastleState *ctx) {
    unsigned int result = 0;
    unsigned int t = GetGameTimer();
    struct CastleVisitor *cur = ctx->visitors.next;
    struct CastleVisitor *end = (struct CastleVisitor *)&ctx->visitors;

    if (cur != end) {
        do {
            if (cur->flags == 1) {
                unsigned int v = t - cur->time;
                if ((int)v > (int)result) {
                    result = v;
                }
            }
            cur = cur->next;
        } while (cur != end);
    }
    return result;
}

// FUNCTION: LEGOLAND 0x00424c10
unsigned int CountWaitingCastleVisitors(struct CastleState *ctx) {
    unsigned int result = 0;
    struct CastleVisitor *cur = ctx->visitors.next;
    struct CastleVisitor *end = (struct CastleVisitor *)&ctx->visitors;
    while (cur != end) {
        if (cur->flags == 1) {
            ++result;
        }
        cur = cur->next;
    }
    return result;
}

// FUNCTION: LEGOLAND 0x00424c40
int ShouldCastleTrainDepart(struct CastleState *arg) {
    int temp1 = GetLongestCastleVisitorWait(arg);
    int temp2 = CountWaitingCastleVisitors(arg);
    if (temp1 > 0x1388 || temp2 > 3) {
        return 1;
    }
    return 0;
}

// FUNCTION: LEGOLAND 0x00424c70
void BoardCastleVisitors(struct CastleState *param_1) {
    struct CastleState *host = (struct CastleState *)param_1;
    struct CastleVisitor *node;
    struct CastleVisitor *cur;
    struct TrainSeat *ring;
    int found;
    int now = GetGameTimer();

    if (CastleCoaster.flags == 2) {
        if (IsTrainRunning(param_1->field_d8) != 0) {
            if (IsTrainStartDelayed(param_1->field_d8) == 0) {
                return;
            }
        }
        if (ShouldCastleTrainDepart(host) == 0) {
            return;
        }
        if (FindWaitingCastleVisitor(host) == 0) {
            return;
        }
        found = 0;
        while ((node = FindWaitingCastleVisitor(host)) != 0) {
            ring = FindEmptyTrainSeat(param_1->field_d8);
            if (ring == 0) {
                break;
            }
            SeatCastleVisitor((struct CastleVisitor *)node, ring);
            node->flags = 2;
            found = 1;
        }
        if (found != 0) {
            StartCastleTrain(param_1, 0xbb8);
        }
        return;
    }
    for (cur = (struct CastleVisitor *)host->visitors.next; cur != (struct CastleVisitor *)&host->visitors; cur = cur->next) {
        if (now - (int)cur->time > 0x1388) {
            cur->flags = 4;
        }
    }
}

// FUNCTION: LEGOLAND 0x00424d80
void UnseatAllCastleVisitors(struct CastleState *list) {
    struct CastleVisitor *current = FindSeatedCastleVisitor(list);
    while (current != NULL) {
        UnseatCastleVisitor((struct CastleVisitor *)current);
        current->flags = 4;
        current = FindSeatedCastleVisitor(list);
    }
}

// FUNCTION: LEGOLAND 0x00424dc0
void ReleaseFinishedCastleVisitors(struct CastleState *arg) {
    struct CastleVisitor *current = FindFinishedCastleVisitor(arg);
    if (current != NULL) {
        while (current != NULL) {
            struct Bloke *next = current->bloke;
            next->param_action = 33;
            FreeCastleVisitor((struct CastleVisitor *)current);
            current = FindFinishedCastleVisitor(arg);
        }
    }
}

// FUNCTION: LEGOLAND 0x00424df0
void FreeAllCastleVisitors(struct CastleState *esi) {
    struct CastleVisitor *current = esi->visitors.next;
    struct CastleVisitor *target = (struct CastleVisitor *)&esi->visitors;
    while (current != target) {
        FreeCastleVisitor((struct CastleVisitor *)current);
        current = esi->visitors.next;
    }
}

// FUNCTION: LEGOLAND 0x00424e20
void CastleRemoveAllRiders(void) {
    struct Ride *ride = GetCastleDispatchRide(0);
    struct RideNode *node = ride->riders;
    if (node != 0) {
        do {
            struct RideNode *next_node = node->next;
            node->rider->flags &= 0xfff7;
            RemoveBlokeFromRide(ride, node);
            node = next_node;
        } while (node != 0);
    }
}

// FUNCTION: LEGOLAND 0x00424e60
void OnCoasterRingClosed(struct CastleState *owner) {
}

// FUNCTION: LEGOLAND 0x00424e70
void OnCoasterRingOpened(struct CastleState *owner) {
    StopCastleTrain((struct CastleState *)&CastleCoaster);
}

#pragma optimize("", off)
// FUNCTION: LEGOLAND 0x00424e80
void RecordCastleFrameCounters(void) {
    DAT_00610804[DAT_00610a10 & 0x3f] = DepthSpanRecordCount;
    DAT_00610500[DAT_00610a10 & 0x3f] = FilledTriangleCount;
    DAT_00610400[DAT_00610a10 & 0x3f] = EdgeMeshDrawCycles;
    DAT_00610704[DAT_00610a10 & 0x3f] = DAT_00611644;
    DAT_00610604[DAT_00610a10 & 0x3f] = DAT_00615f68;
    DAT_006100f8[DAT_00610a10 & 0x3f] = LmsDrawCycles;
    DAT_0060fdf8[DAT_00610a10 & 0x3f] = TrainStepCycles;
    DAT_0060fef8[DAT_00610a10 & 0x3f] = DepthClearCycles;
    DAT_0060fbf8[DAT_00610a10 & 0x3f] = DAT_00615fc4;
    DAT_006101f8[DAT_00610a10 & 0x3f] = DAT_00615fc8;
    DAT_0060fcf8[DAT_00610a10 & 0x3f] = DAT_00615fcc;
    DAT_00610904[DAT_00610a10 & 0x3f] = DAT_00610704[DAT_00610a10 & 0x3f] + DAT_00610400[DAT_00610a10 & 0x3f] + DAT_0060fdf8[DAT_00610a10 & 0x3f] + DAT_00610604[DAT_00610a10 & 0x3f] + DAT_006100f8[DAT_00610a10 & 0x3f] + DAT_0060fef8[DAT_00610a10 & 0x3f];
    DepthSpanRecordCount = 0;
    FilledTriangleCount = 0;
    EdgeMeshDrawCycles = 0;
    DAT_00611644 = 0;
    DAT_00615f68 = 0;
    LmsDrawCycles = 0;
    TrainStepCycles = 0;
    DepthClearCycles = 0;
    DAT_00615fc4 = 0;
    DAT_00615fc8 = 0;
    DAT_00615fcc = 0;
}
#pragma optimize("", on)

#pragma optimize("", off)
// FUNCTION: LEGOLAND 0x00425050
void RenderCastleObj(Element *obj, unsigned int param_2, unsigned int param_3, TileId *tile, void *param_5, unsigned int clip) {
    struct Ride *r;
    struct RideNode *walk;
    struct Bloke *rider;
    struct Point off;
    struct Point pos;

    r = obj->ride;
    RenderItems_New();
    CastleRiderRenderList = NULL;
    walk = r->riders;
    while (walk != NULL) {
        if (tile->id == walk->tile.id) {
            rider = walk->rider;
            if (rider->param_action < 0x10) {
                AddBlokeToRenderList(&CastleRiderRenderList, walk, walk->person->field_20);
            }
            if (rider->param_action >= 0x21) {
                AddBlokeToRenderList(&CastleRiderRenderList, walk, walk->person->field_20);
            }
        }
        walk = walk->next;
    }
    RenderBlokeList((struct RenderItemNode **)&CastleRiderRenderList);
    if (CastleMatteSprite != NULL) {
        pos = GetScreenCoordsForObject(tile, r);
        off = GetRenderOffsetForLayer(r->layer, 2);
        AdjustOffsetForViewMode(&off);
        PrintSprite(CastleMatteSprite, pos.x + off.x, pos.y + off.y, clip, 0);
    }
}
#pragma optimize("", on)

#pragma optimize("", off)
// FUNCTION: LEGOLAND 0x00425170
void CastleCoasterUpdate(Element *obj) {
    RecordCastleFrameCounters();
    UpdateCastleView();
    ClearCoasterDepth(0);
    if (CastleCoasterPlaced != 0) {
        BoardCastleVisitors((struct CastleState *)&CastleCoaster);
        UpdateCastleTrain((struct CastleState *)&CastleCoaster);
        ReleaseFinishedCastleVisitors((struct CastleState *)&CastleCoaster);
    }
}
#pragma optimize("", on)

#pragma optimize("", off)
// FUNCTION: LEGOLAND 0x004251c0
void CastleObjUpdate(Element *obj) {
    /* /Od build: stack-slot order follows the (hashed) local names; these single-letter
     * names reproduce the original layout (a at ebp-4 ... i at ebp-0x24). */
    RideNode *a; /* current rider node */
    TileId *b; /* &a->tile */
    struct CastleState *c; /* CastleCoaster */
    Ride *d; /* the castle ride */
    unsigned int e; /* tile y */
    unsigned int f; /* tile x */
    Bloke *g; /* rider */
    RideNode *h; /* next rider node */
    struct CastleVisitor *i; /* timer node (unused) */

    d = obj->ride;
    a = d->riders;
    c = &CastleCoaster;
    if (c->f_a8 != NULL) {
        FindPathSegDoorAPoints(c->f_a8, &c->pts_a);
    }
    if (c->f_c0 != NULL) {
        FindPathSegDoorBPoints(c->f_c0, &c->pts_b);
    }
    while (a != NULL) {
        h = a->next;
        b = &a->tile;
        f = b->pos.x + d->x;
        e = b->pos.y + d->y;
        g = a->rider;
        switch (g->param_action) {
        case 0:
            g->flags |= 8;
            if (g->low_level_action == 0) {
                g->dest.x = (f - 8) << 8;
                g->dest.y = (e - 1) << 8;
                g->field_73 = ((CalcMoveLineIntFn)CalcMoveLine)(g->pos, g->dest, &g->nav) + 0x10;
                g->low_level_action = 7;
                NewDirForAction(g, (g->field_73 >> 5) + 3);
                g->param_action++;
            }
            break;
        case 1:
            if (g->low_level_action == 0) {
                g->param_action = 0x10;
            }
            break;
        case 0x10:
            i = AddCastleVisitor(g, &CastleCoaster);
            g->param_action = 0x20;
            break;
        case 0x21:
            if (g->low_level_action == 0) {
                g->dest.x = (f << 8) + 0x80;
                g->dest.y = (e << 8) + 0x80;
                g->field_73 = ((CalcMoveLineIntFn)CalcMoveLine)(g->pos, g->dest, &g->nav) + 0x10;
                g->low_level_action = 7;
                NewDirForAction(g, (g->field_73 >> 5) + 3);
                g->param_action++;
            }
            break;
        case 0x22:
            if (g->low_level_action == 0) {
                g->param_action = 0x40;
            }
            break;
        case 0x40:
            g->flags &= 0xfff7;
            RemoveBlokeFromRide(d, a);
            break;
        }
        a = h;
    }
    CastleCoasterUpdate(obj);
}
#pragma optimize("", on)

#pragma optimize("", off)
// FUNCTION: LEGOLAND 0x004254d0
void CastleGetInterfaces(struct Element *head, struct Ride *obj) {
    // STRING: LEGOLAND 0x004b5c0c
    if (_stricmp("CASTLE OBJ", head->name) == 0) {
        obj->cb_8c = CastleDispatchSetEditMode;
        obj->cb_90 = CastleDispatchCalcCursor;
        obj->cb_94 = CastleDispatchDCalcCursor;
        obj->cb_98 = CastleDispatchAddObject;
        obj->cb_9c = CastleDispatchRemoveObject;
        obj->cb_a4 = CastleObjLoad;
        obj->cb_ac = CastleObjUnload;
        obj->cb_pre_render = CastleObjUpdate;
        obj->cb_b0 = RenderCastleObj;
        obj->save_hook = CastleObj_Save;
        obj->load_hook = CastleObj_Load;
        obj->cb_c0 = (int (*)())CastleObjPieceCount;
        CastleDispatchTable[0].element = head;
        CastleDispatchTable[0].fn_4 = (void (*)())CastleObjSetEditMode;
        CastleDispatchTable[0].fn_8 = CastleObjCalcCursor;
        CastleDispatchTable[0].fn_10 = CastleObjDCalcCursor;
        CastleDispatchTable[0].fn_c = PlaceCastleCoaster;
        CastleDispatchTable[0].fn_14 = RemoveCastleCoaster;
        CastleDispatchTableEnd[2] = head;
    }
    // STRING: LEGOLAND 0x004b5b7c
    else if (_stricmp("CASTLE_DUMMY", head->name) == 0) {
        obj->cb_8c = CastleDispatchSetEditMode;
        obj->cb_90 = CastleDispatchCalcCursor;
        obj->cb_94 = CastleDispatchDCalcCursor;
        obj->cb_98 = CastleDispatchAddObject;
        obj->cb_9c = CastleDispatchRemoveObject;
        obj->cb_a4 = CastleDummyLoad;
        obj->cb_b0 = RenderCastleDummy;
        CastleDispatchTable[1].fn_10 = CastleDummyDCalcCursor;
        CastleDispatchTable[1].fn_14 = CastleDummyRemoveObject;
        CastleDispatchTableEnd[3] = head;
    }
    // STRING: LEGOLAND 0x004b5bfc
    else if (_stricmp("SQUARE_TRACK", head->name) == 0) {
        obj->cb_8c = CastleDispatchSetEditMode;
        obj->cb_90 = CastleDispatchCalcCursor;
        obj->cb_94 = CastleDispatchDCalcCursor;
        obj->cb_98 = CastleDispatchAddObject;
        obj->cb_9c = CastleDispatchRemoveObject;
        obj->cb_a4 = CastleLoadBasicTiles;
        obj->cb_ac = CastleUnloadBasicTiles;
        obj->cb_b0 = RenderSquareTrack;
        obj->cb_90 = SquareTrackCalcCursor;
        CastleDispatchTable[2].element = head;
        CastleDispatchTable[2].fn_4 = TrackPieceSetEditMode;
        CastleDispatchTable[2].fn_8 = SquareTrackPieceCalcCursor;
        CastleDispatchTable[2].fn_10 = CastleRemovePieceCalcCursor;
        CastleDispatchTable[2].fn_c = SquareTrackAddObject;
        CastleDispatchTable[2].fn_14 = RemoveQueriedCoasterPiece;
        CastleDispatchTableEnd[0] = head;
    }
    // STRING: LEGOLAND 0x004b5be8
    else if (_stricmp("SQUARE_TRACK_HEIGHT", head->name) == 0) {
        obj->cb_8c = CastleDispatchSetEditMode;
        obj->cb_90 = CastleDispatchCalcCursor;
        obj->cb_94 = CastleDispatchDCalcCursor;
        obj->cb_98 = CastleDispatchAddObject;
        obj->cb_9c = CastleDispatchRemoveObject;
        obj->cb_a4 = SquareTrackHeightLoad;
        obj->cb_b0 = RenderSquareTrack;
        CastleDispatchTable[3].element = head;
        CastleDispatchTable[3].fn_4 = TrackPieceSetEditMode;
        CastleDispatchTable[3].fn_8 = HeightTrackCalcCursor;
        CastleDispatchTable[3].fn_10 = CastleRemovePieceCalcCursor;
        CastleDispatchTable[3].fn_c = HeightTrackAddObject;
        CastleDispatchTable[3].fn_14 = RemoveQueriedCoasterPiece;
        CastleDispatchTableEnd[5] = head;
    }
    // STRING: LEGOLAND 0x004b5bd0
    else if (_stricmp("SQUARE_TRACK_HEIGHT_0", head->name) == 0) {
        obj->cb_8c = CastleDispatchSetEditMode;
        obj->cb_90 = CastleDispatchCalcCursor;
        obj->cb_94 = CastleDispatchDCalcCursor;
        obj->cb_98 = CastleDispatchAddObject;
        obj->cb_9c = CastleDispatchRemoveObject;
        obj->cb_a4 = SquareTrackHeight0Load;
        obj->cb_b0 = RenderSquareTrack;
        CastleDispatchTable[4].element = head;
        CastleDispatchTable[4].fn_4 = TrackPieceSetEditMode;
        CastleDispatchTable[4].fn_8 = HeightTrackCalcCursor;
        CastleDispatchTable[4].fn_10 = CastleRemovePieceCalcCursor;
        CastleDispatchTable[4].fn_c = HeightTrackAddObject;
        CastleDispatchTable[4].fn_14 = RemoveQueriedCoasterPiece;
        CastleDispatchTableEnd[4] = head;
    }
    // STRING: LEGOLAND 0x004b5bb4
    else if (_stricmp("SQUARE_TRACK_HEIGHT_PATH", head->name) == 0) {
        obj->cb_8c = CastleDispatchSetEditMode;
        obj->cb_90 = CastleDispatchCalcCursor;
        obj->cb_94 = CastleDispatchDCalcCursor;
        obj->cb_98 = CastleDispatchAddObject;
        obj->cb_9c = CastleDispatchRemoveObject;
        obj->cb_a4 = SquareTrackHeightPathLoad;
        obj->cb_b0 = RenderSquareTrack;
        CastleDispatchTable[5].element = head;
        CastleDispatchTable[5].fn_4 = TrackPieceSetEditMode;
        CastleDispatchTable[5].fn_8 = HeightTrackPathCalcCursor;
        CastleDispatchTable[5].fn_10 = CastleRemovePieceCalcCursor;
        CastleDispatchTable[5].fn_c = HeightTrackPathAddObject;
        CastleDispatchTable[5].fn_14 = RemoveQueriedCoasterPiece;
        CastleDispatchTableEnd[1] = head;
    }
    // STRING: LEGOLAND 0x004b5ba0
    else if (_stricmp("ROLLER_COASTER_LOAD", head->name) == 0) {
        obj->cb_8c = CastleDispatchSetEditMode;
        obj->cb_90 = CastleDispatchCalcCursor;
        obj->cb_94 = CastleDispatchDCalcCursor;
        obj->cb_98 = CastleDispatchAddObject;
        obj->cb_9c = CastleDispatchRemoveObject;
        obj->cb_8c = (void (*)())CastleObj_Load;
    }
    // STRING: LEGOLAND 0x004b5b8c
    else if (_stricmp("ROLLER_COASTER_SAVE", head->name) == 0) {
        obj->cb_8c = CastleDispatchSetEditMode;
        obj->cb_90 = CastleDispatchCalcCursor;
        obj->cb_94 = CastleDispatchDCalcCursor;
        obj->cb_98 = CastleDispatchAddObject;
        obj->cb_9c = CastleDispatchRemoveObject;
        obj->cb_8c = (void (*)())CastleObj_Save;
    }
}
#pragma optimize("", on)

// FUNCTION: LEGOLAND 0x00425a50
void InitCastleView(void) {
    CastleIsoProjections[0].m[0][0] = 1.60334f;
    CastleIsoProjections[0].m[0][1] = -1.60334f;
    CastleIsoProjections[0].m[0][2] = 0.0f;
    CastleIsoProjections[0].m[0][3] = 0.0f;
    CastleIsoProjections[0].m[1][0] = 0.801688f;
    CastleIsoProjections[0].m[1][1] = 0.801688f;
    CastleIsoProjections[0].m[1][2] = 1.96416f;
    CastleIsoProjections[0].m[1][3] = 0.0f;
    CastleIsoProjections[0].m[2][0] = 3.19995f;
    CastleIsoProjections[0].m[2][1] = 3.19995f;
    CastleIsoProjections[0].m[2][2] = 0.0f;
    CastleIsoProjections[0].m[2][3] = 32767.5f;
    CastleIsoProjections[0].m[3][0] = 0.0f;
    CastleIsoProjections[0].m[3][1] = 0.0f;
    CastleIsoProjections[0].m[3][2] = 0.0f;
    CastleIsoProjections[0].m[3][3] = 1.0f;
    CastleIsoProjections[1] = CastleIsoProjections[0];
    *(volatile float *)&CastleIsoProjections[1].m[0][0] *= 0.5f;
    CastleIsoProjections[1].m[0][1] *= 0.5f;
    CastleIsoProjections[1].m[1][0] *= 0.5f;
    CastleIsoProjections[1].m[1][1] *= 0.5f;
    CastleIsoProjections[1].m[1][2] *= 0.5f;
    lego_sqrtf_init();
    lego_rsqrtf_init();
    CastleViewDir[0] = 1.0f;
    CastleViewDir[1] = 1.0f;
    CastleViewDir[2] = CastleIsoProjections[0].m[1][0] * -2.0f / CastleIsoProjections[0].m[1][2];
    Vec3Normalize(CastleViewDir);
    Vec3Normalize((float *)&TrackLightDir);
    Vec3Normalize((float *)&ModelLightDir);
    SetTrackLighting();
    InitCastleLinkList();
}

// FUNCTION: LEGOLAND 0x00425bd0
void SetTrackLighting(void) {
    float f;
    unsigned int i1;
    unsigned int i2;
    unsigned int i3;

    f = DAT_004b5cac;
    f = f + DAT_0061164c;
    f = f * 0.5f;

    i1 = TrackLightDir;
    i2 = DAT_004b5cb4;
    i3 = DAT_004b5cb8;
    CastleLightDir = i1;

    memcpy(&i1, &DAT_0061164c, sizeof(i1));
    DAT_004b5ca4 = i2;
    memcpy(&i2, &DAT_004b5cac, sizeof(i2));
    DAT_004b5ca8 = i3;

    DAT_00611648 = i1;
    DAT_004b5c9c = i2;
    CastleLightBias = f;

    f = DAT_004b5cac;
    f = f - DAT_0061164c;
    f = f * 0.5f;
    CastleLightScale = f;
}

// FUNCTION: LEGOLAND 0x00425c40
void SetModelLighting(void) {
    float f;
    unsigned int i1;
    unsigned int i2;
    unsigned int i3;

    f = DAT_004b5cbc;
    f = f + DAT_00611650;
    f = f * 0.5f;

    i1 = ModelLightDir;
    i2 = DAT_004b5cc4;
    i3 = DAT_004b5cc8;
    CastleLightDir = i1;

    memcpy(&i1, &DAT_00611650, sizeof(i1));
    DAT_004b5ca4 = i2;
    memcpy(&i2, &DAT_004b5cbc, sizeof(i2));
    DAT_004b5ca8 = i3;

    if (f == f) {
        DAT_00611648 = i1;
        DAT_004b5c9c = i2;
        CastleLightBias = f;
    }

    f = DAT_004b5cbc;
    f = f - DAT_00611650;
    f = f * 0.5f;
    CastleLightScale = f;
}

// FUNCTION: LEGOLAND 0x00425cb0
void MapPosToWorld(const struct Int16Pair *in, float f, struct FVec3 *out) {
    short temp;
    float temp_f;
    temp = in->x;
    out->x = (float)temp * FLOAT_004ab45c;
    temp = in->y;
    out->y = (float)temp * FLOAT_004ab45c;
    temp_f = f;
    out->z = temp_f * FLOAT_004ab458;
}

// FUNCTION: LEGOLAND 0x00425cf0
void Vec3Cross(const struct FVec3 *a, const struct FVec3 *b, struct FVec3 *result) {
    result->x = b->z * a->y - a->z * b->y;
    result->y = a->z * b->x - a->x * b->z;
    result->z = a->x * b->y - a->y * b->x;
}

// FUNCTION: LEGOLAND 0x00425d30
float Vec3Dot(struct FVec3 *param_1, struct FVec3 *param_2) {
    return param_1->z * param_2->z + param_1->y * param_2->y + param_1->x * param_2->x;
}

// FUNCTION: LEGOLAND 0x00425d50
void Vec3Normalize(float *v) {
    float sum = FLOAT_004ab390;
    float s;
    int i;

    for (i = 0; i < 3; i++) {
        sum += v[i] * v[i];
    }
    s = lego_rsqrtf(sum);
    for (i = 0; i < 3; i++) {
        v[i] *= s;
    }
}

// FUNCTION: LEGOLAND 0x00425da0
unsigned int Vec3Equal(const float *a, const float *b) {
    if (a[0] != b[0]) {
        return 0;
    }
    if (a[1] != b[1]) {
        return 0;
    }
    if (a[2] != b[2]) {
        return 0;
    }
    return 1;
}

// FUNCTION: LEGOLAND 0x00425de0
void Mat2Invert(float *a) {
    float inv = 1.0f / (a[0] * a[3] - a[1] * a[2]);
    float t = a[0];

    a[0] = inv * a[3];
    a[3] = t * inv;
    a[1] = -(inv * a[1]);
    a[2] = -(inv * a[2]);
}

// FUNCTION: LEGOLAND 0x00425e20
void UpdateCastleView(void) {
    struct FMat4 m;
    float a[4];
    struct Point pt;
    struct Point ref;
    int bounds[4];

    int mid;
    float dx;
    float fx, fy, fm, fb;
    float dy;

    CastleViewMatrix = CastleIsoProjections[1];
    a[0] = CastleViewMatrix.m[0][0];
    a[1] = CastleViewMatrix.m[0][1];
    a[2] = CastleViewMatrix.m[1][0];
    a[3] = CastleViewMatrix.m[1][1];
    Mat2Invert(a);
    pt.x = (lpConfig->view_width >> 1) + lpConfig->view_x;
    pt.y = (lpConfig->view_height >> 1) + lpConfig->view_y;
    CastleViewMatrix.m[0][3] = (float)pt.x;
    CastleViewMatrix.m[1][3] = (float)pt.y;
    ScreenToMapRef(&pt.x, &ref.x, 0);
    GetTileBounds(&ref, bounds);
    CastleCameraPos[2] = 0.0f;
    mid = (bounds[2] + bounds[0]) >> 1;
    fx = (float)pt.x;
    fm = (float)mid;
    dx = fx - fm;
    fy = (float)pt.y;
    fb = (float)bounds[1];
    dy = fy - fb;
    CastleCameraPos[0] = dy * a[1] + dx * a[0];
    CastleCameraPos[1] = dy * a[3] + dx * a[2];
    CastleCameraPos[0] += (float)ref.x * FLOAT_004ab45c;
    CastleCameraPos[1] += (float)ref.y * FLOAT_004ab45c;
    DAT_008299ac[0] = lpConfig->view_x;
    DAT_008299ac[1] = lpConfig->view_y;
    DAT_008299ac[2] = lpConfig->view_width + lpConfig->view_x;
    DAT_008299ac[3] = lpConfig->view_height + lpConfig->view_y;
    Mat4Identity(m.m);
    m.m[0][3] = -CastleCameraPos[0];
    m.m[1][3] = -CastleCameraPos[1];
    m.m[2][3] = -CastleCameraPos[2];
    Mat4Multiply(CastleViewMatrix.m, m.m, CastleWorldToScreen.m);
    SetClipRect(DAT_008299ac[0], DAT_008299ac[1], DAT_008299ac[2], DAT_008299ac[3]);
}

// FUNCTION: LEGOLAND 0x00426000
void SetCastleViewAtOrigin(int flag) {
    struct FMat4 m;

    if (flag) {
        CastleViewMatrix = CastleIsoProjections[1];
    } else {
        CastleViewMatrix = CastleIsoProjections[0];
    }
    CastleViewMatrix.m[0][3] = 0.0f;
    CastleViewMatrix.m[1][3] = 0.0f;
    DAT_008299ac[0] = lpConfig->view_x;
    DAT_008299ac[1] = lpConfig->view_y;
    DAT_008299ac[2] = lpConfig->view_width + lpConfig->view_x;
    DAT_008299ac[3] = lpConfig->view_height + lpConfig->view_y;
    CastleCameraPos[0] = 0.0f;
    CastleCameraPos[1] = 0.0f;
    CastleCameraPos[2] = 0.0f;
    Mat4Identity(m.m);
    m.m[0][3] = 0.0f;
    m.m[1][3] = 0.0f;
    m.m[2][3] = 0.0f;
    Mat4Multiply(CastleViewMatrix.m, m.m, CastleWorldToScreen.m);
}

// FUNCTION: LEGOLAND 0x004260e0
float GetCastleViewHeightScale(void) {
    return CastleViewMatrix.m[1][2];
}

// FUNCTION: LEGOLAND 0x004260f0
void Mat4Identity(float m[4][4]) {
    int i;
    int j;

    for (i = 0; i <= 3; i++) {
        for (j = 0; j <= 3; j++) {
            m[i][j] = 0.0f;
            if (i == j) {
                m[i][j] = 1.0f;
            }
        }
    }
}

// FUNCTION: LEGOLAND 0x00426120
void Mat4Multiply(float a[4][4], float b[4][4], float out[4][4]) {
    int i;
    int j;
    int k;

    for (i = 0; i < 4; i++) {
        for (j = 0; j < 4; j++) {
            float sum = FLOAT_004ab390;
            for (k = 0; k < 4; k++) {
                sum += a[i][k] * b[k][j];
            }
            out[i][j] = sum;
        }
    }
}

struct Mat4x4 {
    unsigned int m[4][4];
};

// FUNCTION: LEGOLAND 0x00426190
void Mat4Transpose(struct Mat4x4 *param_2, struct Mat4x4 *param_1) {
    int i;
    int j;
    for (i = 0; i < 4; ++i) {
        for (j = 0; j < 4; ++j) {
            param_1->m[i][j] = param_2->m[j][i];
        }
    }
}

// FUNCTION: LEGOLAND 0x004261c0
void TransformPoints3x4(float *in, float *out, float m[4][4], int n) {
    int j;
    int k;

    while (n-- > 0) {
        for (j = 0; j < 3; j++) {
            float sum = FLOAT_004ab390;
            for (k = 0; k < 3; k++) {
                sum += in[k] * m[j][k];
            }
            out[j] = sum + m[j][3];
        }
        out += 3;
        in += 3;
    }
}

// FUNCTION: LEGOLAND 0x00426230
unsigned int ProjectCastlePoints(float in[][3], int out[][4], int n) {
    return ProjectPoints(in, out, &CastleWorldToScreen.m[0][0], 0x10, n);
}

/* Port: clip code of (x, y) against a rectangle: 1 = x >= left, 2 = x <= right, 4 = y >= top,
 * 8 = y <= bottom; 0xf means inside. */
static int PortOutCode(int x, int y, int left, int top, int right, int bottom) {
    int code;

    if (x < left) {
        code = 2;
    } else if (x > right) {
        code = 1;
    } else {
        code = 3;
    }
    if (y < top) {
        code |= 8;
    } else if (y > bottom) {
        code |= 4;
    } else {
        code |= 0xc;
    }
    return code;
}

// FUNCTION: LEGOLAND 0x00426250
unsigned int ProjectPoints(float in[][3], int out[][4], float *m, unsigned int stride, int n) {
    /* Port [library:asm]: the original is inline asm (fistp). Projects n points with the 4x4 float matrix m into records
     * stride bytes apart: x, y, z as rounded ints and [3] = the clip code against the view rectangle, with 0xf0
     * added when the point lies inside a dirty rectangle whose mask accepts it. */
    float (*mat)[4] = (float (*)[4])m;
    int *rec = (int *)out;
    struct ListLink *node;
    struct ClearRect *r;
    int i;
    int k;

    for (i = 0; i < n; i++) {
        for (k = 0; k < 3; k++) {
            rec[k] = PortRound(FLOAT_004ab390 + in[i][0] * mat[k][0] + in[i][1] * mat[k][1] + in[i][2] * mat[k][2] + mat[k][3]);
        }
        rec[3] = PortOutCode(rec[0], rec[1], DAT_008299ac[0], DAT_008299ac[1], DAT_008299ac[2], DAT_008299ac[3]);
        for (node = CastleLinkList.var_18; node != &CastleLinkList; node = node->var_18) {
            r = (struct ClearRect *)node;
            if ((node->kind & PortOutCode(rec[0], rec[1], r->left, r->top, r->right, r->bottom)) == 0xf) {
                rec[3] |= 0xf0;
                break;
            }
        }
        rec = (int *)((char *)rec + stride);
    }
    return 0;
}

// FUNCTION: LEGOLAND 0x004263a0
void GetPointsScreenRect(void *pts, unsigned int m[4][4], int n, struct RectI *out) {
    /* Port [library:asm]: the original is inline asm. Projects n points (3 floats each) with the first two rows of the
     * 4x4 float matrix m and grows the integer bounding box out = {min x, min y, max x, max y}. */
    float (*mat)[4] = (float (*)[4])m;
    float *p = (float *)pts;
    int *box = (int *)out;
    int xy[2];
    int row;

    box[0] = 0x7fffffff;
    box[1] = 0x7fffffff;
    box[2] = (int)0x80000000;
    box[3] = (int)0x80000000;
    for (; n > 0; n--) {
        for (row = 0; row < 2; row++) {
            xy[row] = PortRound(FLOAT_004ab390 + p[0] * mat[row][0] + p[1] * mat[row][1] + p[2] * mat[row][2] + mat[row][3]);
        }
        if (xy[0] > box[2]) {
            box[2] = xy[0];
        }
        if (xy[0] < box[0]) {
            box[0] = xy[0];
        }
        if (xy[1] > box[3]) {
            box[3] = xy[1];
        }
        if (xy[1] < box[1]) {
            box[1] = xy[1];
        }
        p += 3;
    }
}

// FUNCTION: LEGOLAND 0x00426460
void Mat4ToMat3Transpose(unsigned int *dst, void *src) {
    unsigned int i;
    unsigned int j;
    for (i = 0; i < 3; ++i) {
        for (j = 0; j < 3; ++j) {
            dst[j] = *(unsigned int *)((unsigned char *)src + i * 4 + j * 16);
        }
        dst += 3;
    }
}

// FUNCTION: LEGOLAND 0x00426490
void Mat3ToMat4Transposed(unsigned int *src, unsigned int dst[4][4]) {
    int i;
    int j;

    for (i = 0; i < 3; i++) {
        for (j = 0; j < 3; j++) {
            dst[j][i] = src[i * 3 + j];
        }
        dst[3][i] = 0;
    }
    dst[2][3] = 0;
    dst[1][3] = 0;
    dst[0][3] = 0;
    dst[3][3] = 0x3f800000;
}

// FUNCTION: LEGOLAND 0x004264e0
void MakeMat4FromRotTrans(unsigned int *t, unsigned int *m3, unsigned int dst[4][4]) {
    Mat3ToMat4Transposed(m3, dst);
    dst[0][3] = t[0];
    dst[1][3] = t[1];
    dst[2][3] = t[2];
}

// FUNCTION: LEGOLAND 0x00426510
void Mat3ToMat4Transpose(unsigned int *m3, struct Mat4x4 *out) {
    float t[3];
    unsigned int dst[4][4];
    t[0] = 0.0f;
    t[1] = 0.0f;
    t[2] = 0.0f;
    MakeMat4FromRotTrans((unsigned int *)t, m3, dst);
    Mat4Transpose((struct Mat4x4 *)dst, out);
}

// FUNCTION: LEGOLAND 0x00426560
void BuildBasisFromDir(struct FVec3 *dir, struct FVec3 *basis) {
    int i;

    basis[2].x = 0.0f;
    basis[2].y = 0.0f;
    basis[2].z = 1.0f;
    Vec3Cross(&basis[2], dir, &basis[1]);
    Vec3Cross(dir, &basis[1], &basis[2]);
    basis[0] = *dir;
    for (i = 0; i < 3; i++) {
        Vec3Normalize(&basis[i].x);
    }
}

// FUNCTION: LEGOLAND 0x004265d0
int ClipRectIToRect(struct RectI *a, struct RectI *b) {
    if (a->var_0 > b->var_8) {
        return 0;
    }
    if (a->var_8 < b->var_0) {
        return 0;
    }
    if (a->var_4 > b->var_c) {
        return 0;
    }
    if (a->var_c < b->var_4) {
        return 0;
    }
    if (a->var_0 < b->var_0) {
        a->var_0 = b->var_0;
    }
    if (a->var_8 > b->var_8) {
        a->var_8 = b->var_8;
    }
    if (a->var_4 < b->var_4) {
        a->var_4 = b->var_4;
    }
    if (a->var_c > b->var_c) {
        a->var_c = b->var_c;
    }
    return 0xF;
}

// FUNCTION: LEGOLAND 0x00426650
unsigned int AnyCastleLinkVisible(void) {
    struct ListLink *n = (struct ListLink *)CastleLinkList.var_18;

    while (n != (struct ListLink *)&CastleLinkList) {
        if (n->kind == 0xf) {
            return 1;
        }
        n = n->var_18;
    }
    return 0;
}

// FUNCTION: LEGOLAND 0x004266b0
void AddCastleLink(struct ListLink *param_1) {
    CastleLinkList.var_18->var_14 = param_1;
    param_1->var_18 = CastleLinkList.var_18;
    CastleLinkList.var_18 = param_1;
    param_1->var_14 = &CastleLinkList;
}

// FUNCTION: LEGOLAND 0x004266e0
void UnlinkListLink(struct ListLink *param_1) {
    struct ListLink *temp_ptr_1;
    struct ListLink *temp_ptr_2;

    temp_ptr_1 = param_1->var_18;
    temp_ptr_2 = param_1->var_14;
    temp_ptr_1->var_14 = temp_ptr_2;

    temp_ptr_1 = param_1->var_14;
    temp_ptr_2 = param_1->var_18;
    temp_ptr_1->var_18 = temp_ptr_2;
}

// FUNCTION: LEGOLAND 0x00426700
void ClipRectIToCastleView(struct RectI *dst, struct RectI *src) {
    *dst = *src;
    ((int *)dst)[4] = ClipRectIToRect(dst, (struct RectI *)&DAT_008299ac);
}

// FUNCTION: LEGOLAND 0x00426740
void InitCastleLinkList(void) {
    CastleLinkList.var_18 = &CastleLinkList;
    CastleLinkList.var_14 = &CastleLinkList;
}

// FUNCTION: LEGOLAND 0x00426750
unsigned int GetBoxScreenRect(void *ptr, unsigned int *a, unsigned int *b, struct RectI *c) {
    unsigned int m[4][4];
    unsigned int m2[4][4];

    MakeMat4FromRotTrans(a, b, m);
    Mat4Multiply(CastleWorldToScreen.m, (float (*)[4])m, (float (*)[4])m2);
    GetPointsScreenRect(ptr, m2, 8, c);
}

// FUNCTION: LEGOLAND 0x004267b0
void DrawEdgeObj(unsigned int *t, unsigned int *m3, struct EdgeObj *obj) {
    unsigned int m[4][4];
    int xf[8][4];
    float pts[8][3];
    int i;

    MakeMat4FromRotTrans(t, m3, m);
    TransformPoints3x4(&obj->pts[0][0], &pts[0][0], (float (*)[4])m, obj->npts);
    ProjectCastlePoints(pts, xf, obj->npts);
    for (i = 0; i < obj->nedges; i++) {
        DrawDottedLine(xf[obj->edges[i].a], xf[obj->edges[i].b], obj->color);
    }
}

// FUNCTION: LEGOLAND 0x00426850
void InitEdgeObjBox(float x, float y, float z, struct EdgeObj *obj) {
    int i;
    obj->npts = 8;
    obj->nedges = 12;
    obj->pts[0][0] = x;
    obj->pts[0][1] = -y;
    obj->pts[0][2] = 0.0f;
    obj->pts[1][0] = x;
    obj->pts[1][1] = y;
    obj->pts[1][2] = 0.0f;
    obj->pts[2][0] = -x;
    obj->pts[2][1] = y;
    obj->pts[3][0] = -x;
    obj->pts[3][1] = -y;
    obj->pts[3][2] = 0.0f;
    for (i = 0; i < 4; i++) {
        *(struct FVec3 *)obj->pts[i + 4] = *(struct FVec3 *)obj->pts[i];
        obj->pts[i + 4][2] = -z;
    }
    obj->edges[0].a = 0;
    obj->edges[0].b = 1;
    obj->edges[1].a = 1;
    obj->edges[1].b = 2;
    obj->edges[2].a = 2;
    obj->edges[2].b = 3;
    obj->edges[3].a = 3;
    obj->edges[3].b = 0;
    for (i = 0; i < 4; i++) {
        obj->edges[i + 4].a = obj->edges[i].a + 4;
        obj->edges[i + 4].b = obj->edges[i].b + 4;
    }
    obj->edges[8].a = 0;
    obj->edges[8].b = 4;
    obj->edges[9].a = 1;
    obj->edges[9].b = 5;
    obj->edges[10].a = 1;
    obj->edges[10].b = 5;
    obj->edges[11].a = 3;
    obj->edges[11].b = 7;
}

#ifdef LEGOLAND_PORT
/* [library:asm] HASM_lego_rsqrtf and HASM_lego_sqrtf in C: the top 6 bits of the mantissa pick a linear piece
 * (a pair in the table), applied to the mantissa scaled to [1, 2), and the exponent picks a scale. The original
 * calls them through lego_rsqrtf_impl / lego_sqrtf_impl with the argument and the result on the x87 stack. */
static float PortTableRoot(float x, const float *table, const float *exp_table) {
    unsigned int bits;
    unsigned int m_bits;
    unsigned int piece;
    float m;

    memcpy(&bits, &x, sizeof(bits));
    m_bits = (bits & 0x007FFFFFu) | 0x3F800000u;
    memcpy(&m, &m_bits, sizeof(m));
    piece = (bits >> 17) & 0x3F;
    return (float)(((double)m * table[piece * 2 + 1] + table[piece * 2]) * exp_table[(bits >> 23) & 0xFF]);
}
#endif

// FUNCTION: LEGOLAND 0x00426960
float lego_rsqrtf(float x) {
#ifdef LEGOLAND_PORT
    return PortTableRoot(x, rsqrtf_mantissa_table, rsqrtf_exp_table);
#else
    __asm {
        fld x
        call dword ptr [lego_rsqrtf_impl]
        fstp x
    }
    return x;
#endif
}

#ifndef LEGOLAND_PORT /* [library:asm] see PortTableRoot */
// FUNCTION: LEGOLAND 0x00426980
__declspec(naked) void HASM_lego_rsqrtf(void) {
    __asm {
        fstp dword ptr [esp - 10h]
        mov dword ptr [esp - 8], ebx
        mov dword ptr [esp - 0Ch], edx
        mov edx, dword ptr [esp - 10h]
        mov ebx, 007FFFFFh
        mov dword ptr [esp - 4], eax
        mov eax, edx
        shr eax, 11h
        and ebx, edx
        and eax, 3Fh
        or  ebx, 3F800000h
        mov dword ptr [esp - 10h], ebx
        mov ebx, dword ptr [esp - 8]
        fld dword ptr [esp - 10h]
        fmul dword ptr [eax*8 + rsqrtf_mantissa_table + 4]
        shr edx, 17h
        fadd dword ptr [eax*8 + rsqrtf_mantissa_table]
        mov eax, dword ptr [esp - 4]
        fmul dword ptr [edx*4 + rsqrtf_exp_table]
        mov edx, dword ptr [esp - 0Ch]
        ret
    }
}
#endif

// FUNCTION: LEGOLAND 0x004269e0
void lego_rsqrtf_init(void) {
    int i;
    float x;

    for (i = 0; i < 64; i++) {
        if (i == 29) {
            i = 29;
        }
        x = (float)sqrt((float)i * 0.015625f + 1.0f);
        /* the original addresses the table from 8 bytes before it (CastleRiderRenderList) with index i + 1 */
        rsqrtf_mantissa_table[i * 2] = FLOAT_004ab43c / (x + x);
        rsqrtf_mantissa_table[i * 2 + 1] = FLOAT_004ab468 / (x * x * x * 2.0f);
    }
    rsqrtf_exp_table[0] = 1.0f;
    for (i = 1; i <= 255; i++) {
        rsqrtf_exp_table[i] = 1.0f / (float)sqrt(pow(2.0, i - 127));
    }
#ifndef LEGOLAND_PORT /* [library:asm] the port calls PortTableRoot directly */
    __asm {
        push eax
        lea eax, HASM_lego_rsqrtf
        mov lego_rsqrtf_impl, eax
        pop eax
    }
#endif
}

// FUNCTION: LEGOLAND 0x00426a90
float lego_sqrtf(float x) {
#ifdef LEGOLAND_PORT
    return PortTableRoot(x, sqrtf_mantissa_table, sqrtf_exp_table);
#else
    __asm {
        fld x
        call dword ptr [lego_sqrtf_impl]
        fstp x
    }
    return x;
#endif
}

#ifndef LEGOLAND_PORT /* [library:asm] see PortTableRoot */
// FUNCTION: LEGOLAND 0x00426ab0
__declspec(naked) void HASM_lego_sqrtf(void) {
    __asm {
        fstp dword ptr [esp - 10h]
        mov dword ptr [esp - 8], ebx
        mov dword ptr [esp - 0Ch], edx
        mov edx, dword ptr [esp - 10h]
        mov ebx, 007FFFFFh
        mov dword ptr [esp - 4], eax
        mov eax, edx
        shr eax, 11h
        and ebx, edx
        and eax, 3Fh
        or  ebx, 3F800000h
        mov dword ptr [esp - 10h], ebx
        mov ebx, dword ptr [esp - 8]
        fld dword ptr [esp - 10h]
        fmul dword ptr [eax*8 + sqrtf_mantissa_table + 4]
        shr edx, 17h
        fadd dword ptr [eax*8 + sqrtf_mantissa_table]
        mov eax, dword ptr [esp - 4]
        fmul dword ptr [edx*4 + sqrtf_exp_table]
        mov edx, dword ptr [esp - 0Ch]
        ret
    }
}
#endif

// FUNCTION: LEGOLAND 0x00426b10
void lego_sqrtf_init(void) {
    int i;
    int j;
    float x;

    for (i = 0; i < 64; i++) {
        x = (float)sqrt((float)i * 0.015625f + 1.0f);
        sqrtf_mantissa_table[i * 2] = x * 0.5f;
        sqrtf_mantissa_table[i * 2 + 1] = 0.5f / x;
    }
    for (j = 0; j <= 255; j++) {
        sqrtf_exp_table[j] = (float)sqrt(pow(2.0, j - 127));
    }
#ifndef LEGOLAND_PORT /* [library:asm] the port calls PortTableRoot directly */
    __asm {
        push eax
        lea eax, HASM_lego_sqrtf
        mov lego_sqrtf_impl, eax
        pop eax
    }
#endif
}

// FUNCTION: LEGOLAND 0x00426ba0
void RelocatePointer(UINT_PTR *param_1, UINT_PTR param_2) {
    if (*param_1 != 0) {
        *param_1 = *param_1 + param_2;
    }
}

// FUNCTION: LEGOLAND 0x00426bc0
void UnrelocatePointer(UINT_PTR *param_1, UINT_PTR param_2) {
    UINT_PTR var_0 = *param_1;
    if (var_0 != 0) {
        *param_1 = var_0 - param_2;
    }
}

struct TrackEntry {
    unsigned int var_0;
    short var_4;
    short var_6;
};

struct TrackList {
    unsigned int var_0;
    unsigned int var_4;
    unsigned int var_8;
    struct TrackEntry *var_c;
};

// FUNCTION: LEGOLAND 0x00426be0
void PrintTrackList(struct TrackList *edi) {
    int i;
    for (i = 0; i < (int)edi->var_8; i += 1) {
        struct TrackEntry *ptr = edi->var_c + i;
        // STRING: LEGOLAND 0x004b5cd0
        DBPrintf("Track %2x, Type %2x at (%2x, %2x)\n", i, ptr->var_0, ptr->var_4, ptr->var_6);
    }
}

struct Struct426d80X {
    struct CastleState *field_0;
    unsigned int field_4;
    unsigned int field_8;
    unsigned int field_c;
    unsigned int field_10;
};

struct Struct426e80Dst {
    unsigned int field_0;
    unsigned int field_4;
};

struct Struct426f40Dst {
    unsigned int field_0;
    float field_4;
    float field_8;
    unsigned int field_c;
    unsigned int field_10;
    unsigned int field_14;
    unsigned int field_18;
    unsigned int field_1c;
};

struct SaveRec {
    unsigned int size;
    unsigned int field_4;
    int pairCount;
    struct Struct426e80Dst *pairs;
    unsigned int timerCount;
    struct Struct426f40Dst *timer;
    unsigned int recCount;
    struct Struct427070Src *recs;
};

// FUNCTION: LEGOLAND 0x00426c20
int CastleObj_Load(struct Element *element) {
    struct SaveRec *rec = ReadSaveRec();
    void *obj;
    int i;

    if (rec == 0) {
        return 0;
    }
    if (rec == (struct SaveRec *)-1) {
        return 1;
    }
    RelocatePointer((UINT_PTR *)&rec->pairs, (UINT_PTR)rec);
    RelocatePointer((UINT_PTR *)&rec->timer, (UINT_PTR)rec);
    RelocatePointer((UINT_PTR *)&rec->recs, (UINT_PTR)rec);
    SetMapObjectsEnabled(0);
    for (i = 0; i < rec->pairCount; i++) {
        CastleDispatchAddObjectByIndex(rec->pairs[i].field_0, (short *)&rec->pairs[i].field_4);
    }
    obj = GetPlacedCastleCoaster();
    if (rec->timer != 0) {
        LoadCastleTrain(rec->timer, obj);
    }
    if (rec->recs != 0) {
        LoadCastleVisitors(rec->recs, rec->recCount, obj);
    }
    MemFree(rec);
    SetMapObjectsEnabled(1);
    return 1;
}

// FUNCTION: LEGOLAND 0x00426ce0
int CastleObj_Save(struct Element *element) {
    struct Struct426d80X info;
    unsigned int zero;
    struct SaveRec *rec;
    struct CastleState *obj = GetPlacedCastleCoaster();

    if (obj != 0) {
        MeasureCastleSave(obj, &info);
        rec = MemAlloc(info.field_4, 0, 0, 0);
        if (rec == 0) {
            return 0;
        }
        WriteCastleSave(&info, rec);
        UnrelocatePointer((UINT_PTR *)&rec->pairs, (UINT_PTR)rec);
        UnrelocatePointer((UINT_PTR *)&rec->timer, (UINT_PTR)rec);
        UnrelocatePointer((UINT_PTR *)&rec->recs, (UINT_PTR)rec);
        WriteSaveRec((unsigned int *)rec);
        MemFree(rec);
        return 1;
    }
    zero = 0;
    SaveGameWrite(&zero, 4);
    return 1;
}

// FUNCTION: LEGOLAND 0x00426d80
void MeasureCastleSave(struct CastleState *param1, struct Struct426d80X *param2) {
    param2->field_0 = param1;
    param2->field_8 = CountCoasterPieces(param1);
    if (IsTrainRunning(param1->field_d8) != 0) {
        param2->field_c = 1;
    } else {
        param2->field_c = 0;
    }
    param2->field_10 = CountCastleVisitors((struct CastleState *)param1);
    param2->field_4 = (param2->field_8 + param2->field_c * 4 + 4 + param2->field_10) * 8;
}

// FUNCTION: LEGOLAND 0x00426de0
void WriteCastleSave(struct Struct426d80X *src, struct SaveRec *dst) {
    unsigned char *end;

    dst->size = src->field_4;
    dst->pairCount = src->field_8;
    dst->pairs = (struct Struct426e80Dst *)&dst[1];
    SaveCoasterPieces(src->field_0, dst->pairs);
    end = (unsigned char *)(dst->pairs + dst->pairCount);
    if (IsTrainRunning(src->field_0->field_d8) != 0) {
        struct Train *t = (struct Train *)src->field_0->field_d8;
        dst->timerCount = 1;
        dst->timer = (struct Struct426f40Dst *)end;
        SaveCastleTrain(t, (struct Struct426f40Dst *)end);
        end = (unsigned char *)(dst->timer + dst->timerCount);
    } else {
        dst->timer = 0;
    }
    dst->recCount = src->field_10;
    if (dst->recCount != 0) {
        dst->recs = (struct Struct427070Src *)end;
        SaveCastleVisitors((struct CastleState *)src->field_0, (struct Struct427050Dst *)end);
    } else {
        dst->recs = 0;
    }
}

// FUNCTION: LEGOLAND 0x00426e80
void SavePathSegRef(struct PathSeg *src, struct Struct426e80Dst *dst) {
    dst->field_0 = FindCastleDispatchIndex(src->ride);
    dst->field_4 = src->xy;
}

// FUNCTION: LEGOLAND 0x00426ea0
struct PathSeg *LoadPathSegRef(unsigned char *param_1, struct CastleState *param_2) {
    return FindPathSegByXY(param_2, (unsigned int *)(param_1 + 4));
}

// FUNCTION: LEGOLAND 0x00426ec0
void SaveCurveWalker(struct CurveWalker *arg0, unsigned int *arg1) {
    struct FVec3 local;
    struct SegBlob *cur;
    struct SegBlob *end;
    unsigned int count = 0;

    SavePathSegRef(arg0->piece, (struct Struct426e80Dst *)arg1);
    cur = GetPieceCurve(arg0->piece, &local);
    end = arg0->node;
    while (cur != end) {
        cur = cur->next;
        if (cur == NULL)
            break;
        count++;
    }
    arg1[2] = count;
}

// FUNCTION: LEGOLAND 0x00426f10
void LoadCurveWalker(unsigned char *a, struct CurveWalker *b, struct CastleState *c) {
    b->piece = LoadPathSegRef(a, c);
    b->node = GetPieceCurve(b->piece, &b->pos);
}

// FUNCTION: LEGOLAND 0x00426f40
void SaveCastleTrain(struct Train *src, struct Struct426f40Dst *dst) {
    SaveCurveWalker(&src->walker, &dst->field_14);
    dst->field_0 = src->flags;
    dst->field_4 = src->motion.f4;
    dst->field_8 = src->motion.f8;
    dst->field_c = GetGameTimer() - src->field_4;
    dst->field_10 = src->field_8 - GetGameTimer();
}

// FUNCTION: LEGOLAND 0x00426f90
void LoadCastleTrain(struct Struct426f40Dst *src, struct CastleState *obj) {
    struct Train *dst = obj->field_d8;

    dst->flags = src->field_0;
    LoadCurveWalker((unsigned char *)&src->field_14, (struct CurveWalker *)&dst->walker, (struct CastleState *)obj);
    PositionTrainCars(dst, src->field_4, &dst->walker);
    SetTrainEnergy(dst, src->field_8);
    dst->field_4 = GetGameTimer() - src->field_c;
    dst->field_8 = GetGameTimer() + src->field_10;
}

// FUNCTION: LEGOLAND 0x00426ff0
int FindCastleRiderIndex(struct Bloke *param) {
    int i = 0;
    struct RideNode *n = GetCastleDispatchRide(0)->riders;

    while (n != 0) {
        if (param == n->rider) {
            return i;
        }
        n = n->next;
        i++;
    }
    return -1;
}

// FUNCTION: LEGOLAND 0x00427020
struct Bloke *GetCastleRiderByIndex(unsigned int index) {
    unsigned int i = 0;
    struct RideNode *n = GetCastleDispatchRide(0)->riders;

    while (n != 0) {
        if (index == i) {
            return n->rider;
        }
        n = n->next;
        i++;
    }
    return 0;
}

struct Struct427050Dst {
    unsigned int field_0;
    unsigned int field_4;
};

// FUNCTION: LEGOLAND 0x00427050
void SaveCastleVisitor(struct CastleVisitor *param1, struct Struct427050Dst *param2) {
    param2->field_0 = param1->flags;
    param2->field_4 = FindCastleRiderIndex(param1->bloke);
}

struct Struct427070Src {
    unsigned int field_0;
    unsigned int field_4;
};

// FUNCTION: LEGOLAND 0x00427070
void LoadCastleVisitor(struct Struct427070Src *param_1, struct CastleState *param_2) {
    struct CastleVisitor *node;
    unsigned int kind;
    struct Bloke *index = GetCastleRiderByIndex(param_1->field_4);
    node = AddCastleVisitor(index, param_2);
    kind = param_1->field_0;
    node->flags = kind;
    if (kind == 2) {
        SeatCastleVisitor((struct CastleVisitor *)node, FindEmptyTrainSeat((struct Train *)param_2->field_d8));
    }
}

// FUNCTION: LEGOLAND 0x004270c0
void SaveCastleVisitors(struct CastleState *a1, struct Struct427050Dst *a2) {
    struct CastleVisitor *node = a1->visitors.next;
    struct CastleVisitor *end = &a1->visitors;
    if (node != end) {
        struct Struct427050Dst *out = a2;
        do {
            struct Struct427050Dst *cur = out;
            out = out + 1;
            SaveCastleVisitor(node, cur);
            node = node->next;
        } while (node != end);
    }
}

// FUNCTION: LEGOLAND 0x00427100
void LoadCastleVisitors(struct Struct427070Src *arr, int count, struct CastleState *obj) {
    for (; count > 0; count--) {
        LoadCastleVisitor(arr++, obj);
    }
}

// FUNCTION: LEGOLAND 0x00427130
unsigned int CountCastleVisitors(struct CastleState *main) {
    unsigned int counter = 0;
    struct CastleVisitor *node = main->visitors.next;
    struct CastleVisitor *end = &main->visitors;
    while (node != end) {
        node = node->next;
        counter++;
    }
    return counter;
}

// FUNCTION: LEGOLAND 0x00427150
unsigned int CountCoasterPieces(struct CastleState *arg) {
    unsigned int count = 1;

    if (arg->flags == 2) {
        struct PathSeg *next = arg->piece.next;
        struct PathSeg *end = (struct PathSeg *)&arg->piece;
        if (next != end) {
            do {
                next = next->next;
                count++;
            } while (next != end);
        }
        return count;
    }

    if (arg->piece.next != NULL) {
        struct PathSeg *curr = arg->piece.next;
        do {
            curr = curr->next;
            count++;
        } while (curr != NULL);
    }

    if (arg->piece.prev != NULL) {
        struct PathSeg *curr = arg->piece.prev;
        do {
            curr = curr->prev;
            count++;
        } while (curr != NULL);
    }

    return count;
}

// FUNCTION: LEGOLAND 0x00427190
void SaveCoasterPieces(struct CastleState *obj, struct Struct426e80Dst *out) {
    struct PathSeg *cur;

    out->field_0 = FindCastleDispatchIndex(obj->piece.ride);
    out->field_4 = obj->piece.xy;
    cur = obj->piece.next;
    out++;
    if (obj->flags == 2) {
        struct PathSeg *end = (struct PathSeg *)&obj->piece;
        if (cur != end) {
            do {
                SavePathSegRef(cur, out++);
                cur = cur->next;
            } while (cur != end);
        }
        return;
    }
    while (cur != 0) {
        SavePathSegRef(cur, out++);
        cur = cur->next;
    }
    cur = obj->piece.prev;
    while (cur != 0) {
        SavePathSegRef(cur, out++);
        cur = cur->prev;
    }
}

// FUNCTION: LEGOLAND 0x00427220
void WriteSaveRec(unsigned int *param_1) {
    SaveGameWrite(param_1, *param_1);
}

// FUNCTION: LEGOLAND 0x00427240
struct SaveRec *ReadSaveRec(void) {
    unsigned int size;
    struct SaveRec *rec;

    SaveGameRead(&size, 4);
    if (size == 0) {
        return (struct SaveRec *)-1;
    }
    rec = MemAlloc(size, 0, 0, 0);
    if (rec == 0) {
        return rec;
    }
    rec->size = size;
    SaveGameRead(&rec->field_4, size - 4);
    return rec;
}

// FUNCTION: LEGOLAND 0x004272a0
int SaveRollerCoasterFile(unsigned int *data) {
    unsigned int size = *data;
    DWORD written;
    HANDLE hFile;

    if (RollerCoasterSavePath && data) {
        hFile = CreateFileA(RollerCoasterSavePath, 0x40000000, 0, 0, 2, 0x8000000, 0);
        if (hFile != (HANDLE)-1) {
            WriteFile(hFile, data, size, &written, 0);
            if (written != size) {
                CloseHandle(hFile);
            } else {
                CloseHandle(hFile);
                return 1;
            }
        }
    }
    return 0;
}

// FUNCTION: LEGOLAND 0x00427310
void *LoadRollerCoasterFile(void) {
    DWORD bytesRead;
    HANDLE hFile;
    unsigned int fileSize;
    void *buffer;

    if (!RollerCoasterSavePath) {
        return 0;
    }
    hFile = CreateFileA(RollerCoasterSavePath, 0x80000000, 1, 0, 3, 0x8000000, 0);
    if (hFile == (HANDLE)-1) {
        return 0;
    }
    fileSize = GetFileSize(hFile, 0);
    buffer = MemAlloc(fileSize, 0, (unsigned int)(UINT_PTR)DAT_004d8bb0, 0);
    if (buffer == 0) {
        CloseHandle(hFile);
        return 0;
    }
    ReadFile(hFile, buffer, fileSize, &bytesRead, 0);
    if (bytesRead != fileSize) {
        MemFree(buffer);
        CloseHandle(hFile);
        return 0;
    }
    CloseHandle(hFile);
    return buffer;
}

// FUNCTION: LEGOLAND 0x004273c0
unsigned char IsTrainSeatOccupied(struct TrainSeat *param_1) {
    return (unsigned char)(param_1->occupant != 0);
}

// FUNCTION: LEGOLAND 0x004273d0
void SetTrainSeatOccupant(struct TrainSeat *param_1, void *param_2) {
    param_1->occupant = param_2;
}

// FUNCTION: LEGOLAND 0x004273e0
struct CastleVisitor *TakeTrainSeatOccupant(struct TrainSeat *self) {
    struct CastleVisitor *value = self->occupant;
    self->occupant = 0;
    return value;
}

// FUNCTION: LEGOLAND 0x004273f0
void UnlinkSeatOccupant(struct TrainSeat *param_1) {
    struct CastleVisitor *obj = param_1->occupant;
    if (obj != NULL) {
        obj->unlink(obj);
    }
}

// FUNCTION: LEGOLAND 0x00427410
void DrawTrainSeat(struct TrainSeat *obj, struct CarXform *xf) {
    struct FVec3 old;

    if (obj->occupant != 0) {
        old = xf->pos;
        xf->pos.x = xf->m[6] * obj->pos.z + xf->m[0] * obj->pos.x + xf->m[3] * obj->pos.y + old.x;
        xf->pos.y = xf->m[7] * obj->pos.z + xf->m[1] * obj->pos.x + xf->m[4] * obj->pos.y + old.y;
        xf->pos.z = xf->m[8] * obj->pos.z + xf->m[2] * obj->pos.x + xf->m[5] * obj->pos.y + old.z;
        obj->occupant->draw(obj->occupant, xf);
        xf->pos = old;
    }
}

// FUNCTION: LEGOLAND 0x004274b0
void InitTrainSeat(struct TrainSeat *obj, const struct FVec3 *src) {
    obj->pos = *src;
    obj->occupant = 0;
    /* the original stores this byte-returning function in an unsigned int slot (callers test eax) */
    obj->method_10 = (unsigned int (*)(struct TrainSeat *))IsTrainSeatOccupied;
    obj->method_14 = SetTrainSeatOccupant;
    obj->method_18 = UnlinkSeatOccupant;
    obj->draw = DrawTrainSeat;
}

struct MapHeader {
    unsigned short field_0;
};

struct Struct4274f0Coord {
    short field_0;
    short field_2;
};

// FUNCTION: LEGOLAND 0x004274f0
void SetCell2x2ToBasicTile(struct Struct4274f0Coord *coord) {
    if (MapObjectsEnabled != 0) {
        SetMapTile(coord->field_0, coord->field_2, ((struct MapHeader *)BasicTilesData)->field_0 + 1);
        SetMapTile(coord->field_0 + 1, coord->field_2, ((struct MapHeader *)BasicTilesData)->field_0 + 1);
        SetMapTile(coord->field_0, coord->field_2 + 1, ((struct MapHeader *)BasicTilesData)->field_0 + 1);
        SetMapTile(coord->field_0 + 1, coord->field_2 + 1, ((struct MapHeader *)BasicTilesData)->field_0 + 1);
    }
}

// FUNCTION: LEGOLAND 0x004275b0
void TrackPieceOnRemove(void) {}

// FUNCTION: LEGOLAND 0x004275c0
void TrackPieceOnAdd(void) {}

// FUNCTION: LEGOLAND 0x004275d0
void SquareTrackCalcCursor(Element *obj, int *x, unsigned int y) {
    struct SprEnt *result;
    struct PieceClass *key;
    unsigned int bit;
    unsigned int *mask;
    struct Cursor *c;
    struct Ride *ride = obj->ride;
    struct Footprint *fp = &ride->footprint;
    int n = 0;
    int i;
    int found = 0;

    EditCursor.footprint = *fp;
    EditCursor.field_1830 = 0;
    ScreenToMapRef((int *)x, &EditCursor.tile_x, y);
    if (CastleCoaster.flags != 2) {
        bit = 1;
        for (i = 0; i <= 3; i++) {
            if ((bit & CastleCoaster.pts_a.mask) && EditCursor.tile_x - CastleCoaster.pts_a.pt[i][0] >= -1 && EditCursor.tile_x - CastleCoaster.pts_a.pt[i][0] <= 1 && (int)EditCursor.tile_y - CastleCoaster.pts_a.pt[i][1] >= -1 && (int)EditCursor.tile_y - CastleCoaster.pts_a.pt[i][1] <= 1) {
                EditCursor.tile_x = CastleCoaster.pts_a.pt[i][0];
                EditCursor.tile_y = CastleCoaster.pts_a.pt[i][1];
                found = 1;
                break;
            }
            bit <<= 1;
        }
        if (!found) {
            bit = 1;
            for (i = 0; i <= 3; i++) {
                if ((bit & CastleCoaster.pts_b.mask) && EditCursor.tile_x - CastleCoaster.pts_b.pt[i][0] >= -1 && EditCursor.tile_x - CastleCoaster.pts_b.pt[i][0] <= 1 && (int)EditCursor.tile_y - CastleCoaster.pts_b.pt[i][1] >= -1 && (int)EditCursor.tile_y - CastleCoaster.pts_b.pt[i][1] <= 1) {
                    EditCursor.tile_x = CastleCoaster.pts_b.pt[i][0];
                    EditCursor.tile_y = CastleCoaster.pts_b.pt[i][1];
                    found = 1;
                    break;
                }
                bit <<= 1;
            }
        }
        ValidateCursor(&EditCursor, ride);
        bit = 1;
        for (i = 0; i <= 3; i++) {
            if (bit & CastleCoaster.pts_a.mask) {
                DefaultCursor(&FreeSpotCursors[n]);
                FreeSpotCursors[n].next = EditCursor.next;
                EditCursor.next = &FreeSpotCursors[n];
                FreeSpotCursors[n].tile_x = CastleCoaster.pts_a.pt[i][0];
                FreeSpotCursors[n].tile_y = CastleCoaster.pts_a.pt[i][1];
                FreeSpotCursors[n].footprint = *fp;
                FreeSpotCursors[n].field_1828 = 0x2032;
                ResetCursorBuildable(&FreeSpotCursors[n]);
                n++;
            }
            bit <<= 1;
        }
        bit = 1;
        for (i = 0; i <= 3; i++) {
            if (bit & CastleCoaster.pts_b.mask) {
                DefaultCursor(&FreeSpotCursors[n]);
                FreeSpotCursors[n].next = EditCursor.next;
                EditCursor.next = &FreeSpotCursors[n];
                FreeSpotCursors[n].tile_x = CastleCoaster.pts_b.pt[i][0];
                FreeSpotCursors[n].tile_y = CastleCoaster.pts_b.pt[i][1];
                FreeSpotCursors[n].footprint = *fp;
                FreeSpotCursors[n].field_1828 = 0x2032;
                ResetCursorBuildable(&FreeSpotCursors[n]);
                n++;
            }
            bit <<= 1;
        }
        {
            struct Int16Pair pair;
            pair.x = (unsigned short)EditCursor.tile_x;
            pair.y = (unsigned short)EditCursor.tile_y;
            key = FindPieceClassForElement(obj);
            if (key == 0) {
                key = &SquareTrackPieceClass;
            }
            result = (struct SprEnt *)CheckCoasterPiecePlacement(key, &pair);
            if (result->field_0 == 0) {
                SetCursorBlocked(&EditCursor, 0xe);
            }
        }
        if (IsCursorBuildable(&EditCursor)) {
            SetCursorBlocked(&EditCursor, 0xe);
            bit = 1;
            mask = &CastleCoaster.pts_a.mask;
            c = FreeSpotCursors;
            for (i = 0; i < 8; i++) {
                if (i == 4) {
                    bit = 1;
                    mask = &CastleCoaster.pts_b.mask;
                }
                if (bit & *mask) {
                    if (EditCursor.tile_x == c->tile_x && EditCursor.tile_y == c->tile_y) {
                        ResetCursorBuildable(&EditCursor);
                        return;
                    }
                    c++;
                }
                bit <<= 1;
            }
        }
    } else {
        SetCursorBlocked(&EditCursor, 0xe);
    }
}

// FUNCTION: LEGOLAND 0x00427940
void TrackPieceSetEditMode(struct Element *param_1) {
    struct Ride *value = param_1->ride;
    EditMode.unk0 = 1;
    EditMode.unk8 = value;
    SetEditCursorFootPrint(&value->footprint);
    EditCursor.field_1830 = 0;
}

struct Struct427970Src {
    short field_0;
    short pad_2;
    short field_4;
};

// FUNCTION: LEGOLAND 0x00427970
void CastleRemovePieceCalcCursor(Element *obj, const struct Struct427970Src *src) {
    struct Ride *key = obj->ride;
    struct Int16Pair pair;
    struct PathSeg *node;

    BasicObjectDCalcCursor(obj, (struct Point *)src);
    pair.x = src->field_0;
    pair.y = src->field_4;
    node = FindPathSegAt((struct CastleState *)&CastleCoaster, key, (short *)&pair);
    if (IsCoasterEndPiece(node) == 0) {
        SetCursorBlocked(&QueryCursor, 1);
        CoasterPieceToRemove = 0;
    } else {
        ResetCursorBuildable(&QueryCursor);
        CoasterPieceToRemove = node;
    }
}

// FUNCTION: LEGOLAND 0x004279f0
void RemoveQueriedCoasterPiece(void) {
    RemoveCoasterPiece(CoasterPieceToRemove);
}

// FUNCTION: LEGOLAND 0x00427a00
void RenderSquareTrack(struct Element *unused0, unsigned int unused1, unsigned int unused2, const unsigned char *src) {
    struct PieceHead *node;
    struct Int16Pair tile;
    tile.x = src[0];
    tile.y = src[1];
    node = (struct PieceHead *)FindPathSegByXY((struct CastleState *)&CastleCoaster, (unsigned int *)&tile);
    DrawTrackPiece(node);
    DrawCastleTrainOnPiece(node);
}

struct Struct427a40 {
    unsigned char pad_0[0x14];
    unsigned int field_14;
    unsigned int field_18;
    unsigned int field_1c;
    unsigned int field_20;
    unsigned int field_24;
};

// FUNCTION: LEGOLAND 0x00427a40
void SquareTrackFillOpenEnd(struct Struct427a40 *param_1) {
    if (param_1->field_20 == 0xffffffff) {
        param_1->field_20 = GetOppositeDirection(param_1->field_14);
        param_1->field_24 = param_1->field_18;
    } else if (param_1->field_14 == 0xffffffff) {
        param_1->field_14 = GetOppositeDirection(param_1->field_20);
        param_1->field_18 = param_1->field_24;
    }
}

struct Struct427a80 {
    unsigned char pad_0[0x18];
    unsigned int field_18;
    unsigned char pad_1c[0x24 - 0x1c];
    float field_24;
};

// FUNCTION: LEGOLAND 0x00427a80
void SquareTrackSetHeights(struct Struct427a80 *param_1, float param_2) {
    param_1->field_24 = param_2;
    param_1->field_18 = *(unsigned int *)&param_2;
}

// FUNCTION: LEGOLAND 0x00427aa0
void CastleLoadBasicTiles(Element *obj) {
    struct Element *handle;

    SquareTrackRide = obj->ride;
    SquareTrackRide->layer->flags |= 0x2000;
    // STRING: LEGOLAND 0x004b5f34
    if (LLIDB_FindElement("BASIC TILES 1", &handle, 0) == 0) {
        BasicTilesData = LLIDB_LoadData((void *)handle);
    }
    BuildTrackPieceCurves();
    InitRailDrawOrder();
}

// FUNCTION: LEGOLAND 0x00427af0
void CastleUnloadBasicTiles(void) {
    struct Element *handle;
    unsigned int packed;
    // STRING: LEGOLAND 0x004b5f34
    if (LLIDB_FindElement("BASIC TILES 1", &handle, 0) == 0) {
        LLIDB_UnLoadData(handle);
    }
}

// FUNCTION: LEGOLAND 0x00427b20
void SquareTrackPieceCalcCursor(Element *obj, int *x, unsigned int y) {
    struct Int16Pair tile;
    struct SprEnt *result;

    SetEditCursorFootPrint(&obj->ride->footprint);
    ScreenToMapRef((int *)x, &EditCursor.tile_x, y);
    ValidateCursor(&EditCursor, obj->data);
    EditCursor.field_1828 |= 8;
    tile.x = (unsigned short)EditCursor.tile_x;
    tile.y = (unsigned short)EditCursor.tile_y;
    result = CheckCoasterPiecePlacement(&SquareTrackPieceClass, &tile);
    if (result->field_0 != 0) {
        ResetCursorBuildable(&EditCursor);
    } else {
        SetCursorBlocked(&EditCursor, 0xe);
    }
}

struct Struct427bc0Src {
    short field_0;
    short pad_2;
    short field_4;
};

// FUNCTION: LEGOLAND 0x00427bc0
void SquareTrackAddObject(struct Element *unused, const struct Struct427bc0Src *src) {
    struct Int16Pair pair;
    struct PathSeg *entry;
    pair.x = src->field_0;
    pair.y = src->field_4;
    entry = AddCoasterPiece(SquareTrackRide, &SquareTrackPieceClass, &pair);
    if (entry != NULL) {
        entry->flags |= 6;
    }
}

// FUNCTION: LEGOLAND 0x00427c00
struct PieceClass *FindPieceClassForElement(struct Element *key) {
    int i;
    for (i = 0; i < (int)ElementPieceClassCount; i++) {
        if (ElementPieceClasses[i].key == key)
            return ElementPieceClasses[i].value;
    }
    return 0;
}

struct Struct427c30 {
    unsigned char pad_0[0x14];
    unsigned int field_14;
    unsigned int field_18;
    unsigned char pad_1c[0x20 - 0x1c];
    unsigned int field_20;
    unsigned int field_24;
};

// FUNCTION: LEGOLAND 0x00427c30
void HeightTrackFillOpenEnd(struct Struct427c30 *param_1) {
    if (param_1->field_20 == 0xffffffff) {
        param_1->field_20 = GetOppositeDirection(param_1->field_14);
        param_1->field_24 = param_1->field_18;
        return;
    }
    if (param_1->field_14 == 0xffffffff) {
        param_1->field_14 = GetOppositeDirection(param_1->field_20);
        param_1->field_18 = param_1->field_24;
    }
}

struct Struct427c70Inner {
    unsigned char pad_0[4];
    int field_4;
    int field_8;
};

struct Struct427c70 {
    unsigned char pad_0[0xc];
    struct Struct427c70Inner *field_c;
    unsigned char pad_10[0x18 - 0x10];
    float field_18;
    unsigned char pad_1c[0x24 - 0x1c];
    float field_24;
};

// FUNCTION: LEGOLAND 0x00427c70
void HeightTrackSetHeights(struct Struct427c70 *param_1) {
    struct Struct427c70Inner *inner = param_1->field_c;
    param_1->field_24 = (float)inner->field_4;
    param_1->field_18 = (float)inner->field_8;
}

// FUNCTION: LEGOLAND 0x00427c90
void HeightTrackCalcCursor(Element *obj, int *x, unsigned int y) {
    struct Int16Pair pair;
    struct SprEnt *result;
    struct PieceClass *key;
    unsigned int bit;
    int n;
    struct Ride *ride = obj->ride;
    int i;

    SetEditCursorFootPrint(&ride->footprint);
    EditCursor.field_1830 = 0;
    ScreenToMapRef((int *)x, &EditCursor.tile_x, y);
    n = 0;
    bit = 1;
    for (i = 0; i <= 3; i++) {
        if (bit & CastleCoaster.pts_a.mask) {
            DefaultCursor(&FreeSpotCursors[n]);
            FreeSpotCursors[n].next = EditCursor.next;
            EditCursor.next = &FreeSpotCursors[n];
            FreeSpotCursors[n].tile_x = CastleCoaster.pts_a.pt[i][0];
            FreeSpotCursors[n].tile_y = CastleCoaster.pts_a.pt[i][1];
            FreeSpotCursors[n].footprint = ride->footprint;
            FreeSpotCursors[n].field_1828 = 0x2032;
            ResetCursorBuildable(&FreeSpotCursors[n]);
            n++;
        }
        bit <<= 1;
    }
    bit = 1;
    for (i = 0; i <= 3; i++) {
        if (bit & CastleCoaster.pts_b.mask) {
            DefaultCursor(&FreeSpotCursors[n]);
            FreeSpotCursors[n].next = EditCursor.next;
            EditCursor.next = &FreeSpotCursors[n];
            FreeSpotCursors[n].tile_x = CastleCoaster.pts_b.pt[i][0];
            FreeSpotCursors[n].tile_y = CastleCoaster.pts_b.pt[i][1];
            FreeSpotCursors[n].footprint = ride->footprint;
            FreeSpotCursors[n].field_1828 = 0x2032;
            ResetCursorBuildable(&FreeSpotCursors[n]);
            n++;
        }
        bit <<= 1;
    }
    ValidateCursor(&EditCursor, obj->data);
    pair.x = (unsigned short)EditCursor.tile_x;
    pair.y = (unsigned short)EditCursor.tile_y;
    EditCursor.field_1828 |= 8;
    key = FindPieceClassForElement(obj);
    if (key != 0) {
        result = CheckCoasterPiecePlacement(key, &pair);
        if (result->field_0 != 0) {
            ResetCursorBuildable(&EditCursor);
        } else {
            SetCursorBlocked(&EditCursor, 0xe);
        }
    }
}

// FUNCTION: LEGOLAND 0x00427ea0
void HeightTrackAddObject(Element *obj, const struct Struct427bc0Src *src) {
    struct Int16Pair pair;
    struct Ride *ride = obj->ride;
    struct PieceClass *key;

    pair.x = src->field_0;
    pair.y = src->field_4;
    key = FindPieceClassForElement(obj);
    if (key != 0) {
        AddCoasterPiece(ride, key, &pair)->flags |= 6;
    }
}

// FUNCTION: LEGOLAND 0x00427ef0
void SquareTrackHeightLoad(Element *param_1) {
    struct Ride *a = param_1->ride;
    HeightTrack25Ride = a;
    a->layer->flags |= 0x2000;
    ElementPieceClasses[ElementPieceClassCount].key = param_1;
    ElementPieceClasses[ElementPieceClassCount].value = &HeightTrack25PieceClass;
    ElementPieceClassCount++;
}

// FUNCTION: LEGOLAND 0x00427f30
void SquareTrackHeight0Load(Element *param_1) {
    struct Ride *a = param_1->ride;
    HeightTrack2Ride = a;
    a->layer->flags |= 0x2000;
    ElementPieceClasses[ElementPieceClassCount].key = param_1;
    ElementPieceClasses[ElementPieceClassCount].value = &HeightTrack2PieceClass;
    ElementPieceClassCount++;
}

struct Struct427f70Arg {
    unsigned char pad_0[4];
    short field_4;
    short field_6;
};

// FUNCTION: LEGOLAND 0x00427f70
void RemoveRollerCoasterPath2x2(const struct Struct427f70Arg *arg) {
    int coords[2];

    coords[0] = arg->field_4;
    coords[1] = arg->field_6;
    RemoveRollerCoasterPath(coords);

    coords[0] = arg->field_4 + 1;
    coords[1] = arg->field_6;
    RemoveRollerCoasterPath(coords);

    coords[0] = arg->field_4 + 1;
    coords[1] = arg->field_6 + 1;
    RemoveRollerCoasterPath(coords);

    coords[0] = arg->field_4;
    coords[1] = arg->field_6 + 1;
    RemoveRollerCoasterPath(coords);
}

struct Struct427ff0Point {
    unsigned short pad_0;
    unsigned short pad_2;
    short x;
    short y;
};

// FUNCTION: LEGOLAND 0x00427ff0
void AddRollerCoasterPath2x2(struct Struct427ff0Point *point) {
    int coords[2];

    coords[0] = point->x;
    coords[1] = point->y;
    AddRollerCoasterPath(coords);

    coords[0] = point->x + 1;
    coords[1] = point->y;
    AddRollerCoasterPath(coords);

    coords[0] = point->x + 1;
    coords[1] = point->y + 1;
    AddRollerCoasterPath(coords);

    coords[0] = point->x;
    coords[1] = point->y + 1;
    AddRollerCoasterPath(coords);
}

// FUNCTION: LEGOLAND 0x00428070
void SquareTrackHeightPathLoad(Element *param_1) {
    struct Ride *a = param_1->ride;
    a->layer->flags |= 0x2000;
    ElementPieceClasses[ElementPieceClassCount].key = param_1;
    ElementPieceClasses[ElementPieceClassCount].value = &HeightTrackPathPieceClass;
    ElementPieceClassCount++;
}

// FUNCTION: LEGOLAND 0x004280b0
void HeightTrackPathCalcCursor(Element *obj, int *x, unsigned int y) {
    int total = 0;
    struct Int16Pair pair;
    int a;
    int b;
    struct SprEnt *result;
    struct PieceClass *key;
    unsigned int bit;
    int n;
    struct Footprint *fp = &obj->ride->footprint;
    int i;

    SetEditCursorFootPrint(fp);
    EditCursor.field_1830 = 0;
    ScreenToMapRef((int *)x, &EditCursor.tile_x, y);
    n = 0;
    bit = 1;
    for (i = 0; i <= 3; i++) {
        if (bit & CastleCoaster.pts_a.mask) {
            DefaultCursor(&FreeSpotCursors[n]);
            FreeSpotCursors[n].next = EditCursor.next;
            EditCursor.next = &FreeSpotCursors[n];
            FreeSpotCursors[n].tile_x = CastleCoaster.pts_a.pt[i][0];
            FreeSpotCursors[n].tile_y = CastleCoaster.pts_a.pt[i][1];
            FreeSpotCursors[n].footprint = *fp;
            FreeSpotCursors[n].field_1828 = 0x2032;
            ResetCursorBuildable(&FreeSpotCursors[n]);
            n++;
        }
        bit <<= 1;
    }
    bit = 1;
    for (i = 0; i <= 3; i++) {
        if (bit & CastleCoaster.pts_b.mask) {
            DefaultCursor(&FreeSpotCursors[n]);
            FreeSpotCursors[n].next = EditCursor.next;
            EditCursor.next = &FreeSpotCursors[n];
            FreeSpotCursors[n].tile_x = CastleCoaster.pts_b.pt[i][0];
            FreeSpotCursors[n].tile_y = CastleCoaster.pts_b.pt[i][1];
            FreeSpotCursors[n].footprint = *fp;
            FreeSpotCursors[n].field_1828 = 0x2032;
            ResetCursorBuildable(&FreeSpotCursors[n]);
            n++;
        }
        bit <<= 1;
    }
    ValidateCursor(&EditCursor, obj->data);
    pair.x = (unsigned short)EditCursor.tile_x;
    pair.y = (unsigned short)EditCursor.tile_y;
    EditCursor.field_1828 |= 8;
    key = FindPieceClassForElement(obj);
    if (key != 0) {
        result = CheckCoasterPiecePlacement(key, &pair);
        if (result->field_0 == 0) {
            SetCursorBlocked(&EditCursor, 0xe);
        } else {
            CountCastleCoasterSegs(result->owner, &a, &b);
            if (a >= 0) {
                total = a;
            }
            if (b >= 0) {
                total += b;
            }
            if (total >= 2) {
                SetCursorBlocked(&EditCursor, 0xb);
            } else {
                ResetCursorBuildable(&EditCursor);
            }
        }
    }
}

// FUNCTION: LEGOLAND 0x00428300
void HeightTrackPathAddObject(Element *obj, const struct Struct427bc0Src *src) {
    struct Int16Pair pair;
    struct Ride *ride = obj->ride;
    struct PieceClass *key;

    pair.x = src->field_0;
    pair.y = src->field_4;
    key = FindPieceClassForElement(obj);
    if (key != 0) {
        AddCoasterPiece(ride, key, &pair)->flags &= ~6;
    }
}

// FUNCTION: LEGOLAND 0x00428350
void InitStraightPieceCurve(int a, int b, int c, struct Obj421ab0 *obj, int d) {
    Vector3 pt;

    pt.x = DAT_006117c0[b].x;
    pt.y = DAT_006117c0[b].y;
    pt.z = DAT_006117c0[b].z - (float)(c * 6);
    InitLineCurve(obj, &DAT_006117c0[a].x, &pt.x, (struct Words3 *)&DAT_00611658[d]);
}

// FUNCTION: LEGOLAND 0x004283c0
unsigned int GetTrackPieceCurveIndex(struct PieceHead *pair) {
    unsigned int kind = GetPieceHeadDirPairIndex(pair);
    int v = (((int)pair->end_b.height - (int)pair->end_a.height) >> 1) + 2;

    if (pair->end_a.dir == GetOppositeDirection(pair->end_b.dir)) {
        if (kind == 8) {
            v += kind;
        } else if (kind == 2) {
            v += 0xd;
        } else if (kind == 0xd) {
            v += 0x12;
        } else if (kind == 7) {
            v += 0x17;
        }
        return v;
    }
    switch (kind) {
    case 12:
        return 0;
    case 3:
        return 1;
    case 4:
        return 2;
    case 1:
        return 3;
    case 6:
        return 4;
    case 9:
        return 5;
    case 14:
        return 6;
    case 11:
        return 7;
    }
    return 0xffffffff;
}

// FUNCTION: LEGOLAND 0x004284d0
void BuildTrackPieceCurves(void) {
    struct Obj421ce0 *base;
    register struct Obj421ce0 *cur;
    struct Words3 *p;
    struct Words3 *q;
    int x;
    unsigned int y;
    int j;
    register int qi;
    register unsigned int i;
    unsigned int k;
    int n;
    int ptmp22;
    n = 0;

    for (i = 0; i <= 3; i++) {
        DAT_006117c0[i].x = DAT_004b5e00[i][0] * 20.0f;
        DAT_006117c0[i].y = DAT_004b5e00[i][1] * 20.0f;
        DAT_006117c0[i].z = DAT_004b5e00[i][2] * 20.0f;
    }
    for (i = 0; i <= 3; i++) {
        DAT_00611658[i].x = DAT_004b5e30[i][0] * 20.0f;
        DAT_00611658[i].y = DAT_004b5e30[i][1] * 20.0f;
        DAT_00611658[i].z = DAT_004b5e30[i][2] * 20.0f;
    }
    i = 0;
    while (3 >= i) {
        DAT_00611750[i].x = DAT_004b5e60[i][0] * 20.0f;
        DAT_00611750[i].y = DAT_004b5e60[i][1] * 20.0f;
        DAT_00611750[i].z = DAT_004b5e60[i][2] * 20.0f;
        i++;
    }
    i = 0;
    while (i <= 3) {
        DAT_006116e0[i].x = DAT_004b5e90[i][0] * 20.0f;
        DAT_006116e0[i].y = DAT_004b5e90[i][1] * 20.0f;
        { DAT_006116e0[i].z = DAT_004b5e90[i][2] * 20.0f; }
        ++i;
    }
    i = 0;
    while (3 >= i) {
        ptmp22 = DAT_004b5ef4[i][1];
        qi = ptmp22;
        q = (struct Words3 *)&DAT_00611750[qi];
        y = DAT_004b5f14[i][1];
        p = (struct Words3 *)&DAT_00611750[DAT_004b5ef4[i][0]];
        x = DAT_004b5f14[i][0];
        InitArcCurve(p, q, (struct Words3 *)&DAT_006116e0[i], &((struct Obj421ce0 *)TrackPieceCurves)[n++], x, y);
        if (5 == n) {
            n = 5;
        }
        InitArcCurve(q, p, (struct Words3 *)&DAT_006116e0[i], &((struct Obj421ce0 *)TrackPieceCurves)[n++], y, x);
        i++;
    }
    j = 0;
    base = &((struct Obj421ce0 *)TrackPieceCurves)[n];
    while (j <= 3) {
        cur = base;
        base = 5 + base;
        for (k = 0; k < 5; k++) {
            InitStraightPieceCurve(DAT_004b5ec0[j], (DAT_004b5ec0[j] - 2) & 3, DAT_004b5ee0[k], (struct Obj421ab0 *)cur++, j);
        }
        j++;
    }
}

struct Struct4286e0Element {
    unsigned int field_0;
    unsigned int field_4;
};

// FUNCTION: LEGOLAND 0x004286e0
struct Struct4286e0Element *GetTrackPieceCurve(void *param1) {
    unsigned int result = GetTrackPieceCurveIndex(param1);
    unsigned int index = result + (result + result * 4) * 2;
    return &((struct Struct4286e0Element *)TrackPieceCurves)[index];
}

// FUNCTION: LEGOLAND 0x00428700
void DrawTrackPiece(void *node) {
    struct FVec3 buf;
    unsigned int idx = GetPieceHeadDirPairIndex((struct PieceHead *)node);
    struct SegBlob *r = GetPieceCurve(node, &buf);
    if (*(unsigned char *)node & 6) {
        DrawTrackStretch(r, &buf, RailDrawOrder[idx], 0);
    } else {
        DrawTrackStretch(r, &buf, RailDrawOrder[idx], 1);
    }
}

struct DirRule {
    unsigned int from;
    unsigned int to;
    unsigned int kind;
};

struct AnimPairBuf {
    struct PieceHead pair;
    unsigned char pad_2c[0xa4 - 0x2c];
};

// FUNCTION: LEGOLAND 0x00428750
void InitRailDrawOrder(void) {
    int n;
    struct DirRule table[6];
    struct AnimPairBuf buf;

    table[0].from = 1;
    table[0].to = 4;
    table[0].kind = 2;
    table[1].from = 2;
    table[1].to = 8;
    table[1].kind = 2;
    table[2].from = 1;
    table[2].to = 2;
    table[2].kind = 1;
    table[3].from = 2;
    table[3].to = 4;
    table[3].kind = 2;
    table[4].from = 4;
    table[4].to = 8;
    table[4].kind = 1;
    table[5].from = 8;
    table[5].to = 1;
    table[5].kind = 1;
    for (n = 0; n < 6; n++) {
        unsigned int idx;

        buf.pair.end_a.dir = table[n].from;
        buf.pair.end_b.dir = table[n].to;
        RailDrawOrder[GetPieceHeadDirPairIndex(&buf.pair)] = table[n].kind;
        buf.pair.end_a.dir = table[n].to;
        buf.pair.end_b.dir = table[n].from;
        idx = GetPieceHeadDirPairIndex(&buf.pair);
        if (table[n].kind == 2) {
            RailDrawOrder[idx] = 1;
        } else {
            RailDrawOrder[idx] = 2;
        }
    }
}

// FUNCTION: LEGOLAND 0x00428840
unsigned int LowestSetBitIndex(unsigned int param_1) {
    unsigned int mask = 1;
    unsigned int result = 0;
    if ((param_1 & 1) == 0) {
        while ((param_1 & mask) == 0) {
            mask = mask * 2;
            result = result + 1;
        }
    }
    return result;
}

/* Port: a texture as GetLtxFileTableEntry returns it: power-of-two sizes, then one palette index per texel. */
struct Texture {
    int width;
    int height;
    unsigned char texels[1];
};

/* Port: the color of each palette at the current light level (the original's table at 0x611960). */
static unsigned short port_texture_colors[1024];

// FUNCTION: LEGOLAND 0x00428860
void FillSpansTexturedZ(int palette, int *shade, int n, struct RecIdx *idx, struct RecSrc *src) {
    /* Port [library:asm]: the original is inline asm. Texture-mapped, depth-tested polygon fill. palette selects the texture
     * (GetLtxFileTableEntry); its texels are palette numbers, drawn in the color of light level shade[0]. The left
     * edge carries u (f0c), v (f10) and z (f14); along a span they step by shade[1], shade[2] and shade[3].
     * u and v are rescaled so that or-ing them and shifting right by 16 gives the texel index (v * width + u),
     * which wraps around the texture. */
    struct RecSrc *last = &src[idx[n - 1].k];
    struct Texture *tex;
    unsigned short *row;
    unsigned short *zrow;
    unsigned short depth;
    int wbits;
    int hbits;
    int ushift;
    int vshift;
    unsigned int vmask;
    unsigned int index_mask;
    int du;
    int dv;
    int dz;
    int y = idx[0].v;
    int end;
    int lx = 0;
    int ldx = 0;
    int lu = 0;
    int ldu = 0;
    int lv = 0;
    int ldv = 0;
    int lz = 0;
    int ldz = 0;
    int rx = 0;
    int rdx = 0;
    int u;
    unsigned int v;
    int z;
    int x;
    int i;

    ((short *)&last->f0)[1]++;
    end = ((short *)&last->f0)[1];
    tex = (struct Texture *)GetLtxFileTableEntry(palette);
    if (tex == NULL) {
        return;
    }
    wbits = LowestSetBitIndex(tex->width);
    hbits = LowestSetBitIndex(tex->height);
    ushift = 8 - wbits;
    vshift = 16 - wbits - hbits;
    vmask = (unsigned int)((int)0xff000000 >> ushift);
    du = shade[1] >> ushift;
    dv = (shade[2] << 8) >> vshift;
    dz = shade[3];
    index_mask = (1 << (wbits + hbits)) - 1;
    FilledTriangleCount++;
    idx[n].v = end;
    row = (unsigned short *)CoasterSurfaceBits + CoasterSurfacePitch * y;
    zrow = CoasterDepthBuffer + CoasterSurfacePitch * y;
    for (i = 0; i < CoasterShadeTableCount; i++) {
        port_texture_colors[i] = ((unsigned short *)CoasterShadeTables[i])[shade[0]];
    }
    do {
        struct RecSrc *e = &src[idx->k];

        idx++;
        if (e->flag) {
            rx = e->fx - e->d;
            rdx = e->d;
        } else {
            lx = e->fx - e->d;
            ldx = e->d;
            lu = e->f0c - e->f20;
            ldu = e->f20;
            lv = e->f10 - e->f24;
            ldv = e->f24;
            lz = e->f14 - e->f28;
            ldz = e->f28;
        }
        while (y < idx->v) {
            y++;
            lx += ldx;
            rx += rdx;
            lu += ldu;
            lv += ldv;
            lz += ldz;
            if (rx - lx >= 0x8000) {
                u = lu >> ushift;
                v = (unsigned int)(lv << 8) >> vshift;
                z = lz;
                for (x = lx >> 16; x <= rx >> 16; x++) {
                    depth = (unsigned short)(z >> 16);
                    if (depth >= zrow[x]) {
                        zrow[x] = depth;
                        row[x] = port_texture_colors[tex->texels[(((v & vmask) | (unsigned int)u) >> 16) & index_mask]];
                    }
                    u += du;
                    v += dv;
                    z += dz;
                }
            }
            row += CoasterSurfacePitch;
            zrow += CoasterSurfacePitch;
        }
    } while (y < end);
}

// FUNCTION: LEGOLAND 0x00428b70
void InitTrackDrawing(void) {
    InitRailTubeMesh();
    InitSupportPostShape();
    InitTrackTieShape();
}

// FUNCTION: LEGOLAND 0x00428b80
void PlotCurvePoints(struct SegBlob *curve, float *off) {
    float step = (curve->end - curve->start) * FLOAT_004ab44c;
    float t = curve->start;
    int i;
    int k = 0;

    for (i = 0; i < 30; i++) {
        curve->vt->method_8(curve, t, DAT_006122a0[k]);
        DAT_006122a0[k][0] += off[0];
        DAT_006122a0[k][1] += off[1];
        DAT_006122a0[k][2] += off[2];
        k++;
        curve->vt->method_0(curve, t, DAT_006122a0[k]);
        DAT_006122a0[k][0] += off[0];
        DAT_006122a0[k][1] += off[1];
        DAT_006122a0[k][2] += off[2];
        k++;
        curve->vt->method_10(curve, t, DAT_006122a0[k]);
        DAT_006122a0[k][0] += off[0];
        DAT_006122a0[k][1] += off[1];
        DAT_006122a0[k][2] += off[2];
        k++;
        t = t + step;
    }
    ProjectCastlePoints(DAT_006122a0, DAT_006159c8, 0x5a);
    PlotClippedPoints(&DAT_006159c8[0][0], 0x5a, -1);
}

/* One entry of the curve function table at obj+0x4c: where the track is, and which way it points. */
// FUNCTION: LEGOLAND 0x00428cb0
struct TexMesh *BuildRailTubeMesh(struct SegBlob *obj, const struct FVec3 *offset, int fn, int count, const int *keys) {
    /* Port [library:asm]: the original is inline asm (rdtsc, x87). Builds the roller-coaster track tube into
     * RailTubeMesh: for each of the count keys it asks curve function pair fn for the track's direction and
     * position, turns them into a frame (basis), transforms a ring of 6 vertices (DAT_006121c8) by it into
     * DAT_006139c8, and lights each vertex (texture coordinate u) from the light direction CastleLightDir. */
    const float *light = (const float *)&CastleLightDir;
    struct FVec3 dir;
    struct FVec3 basis[3];
    struct FVec3 pos;
    float m4[4][4];
    float out4[4][4];
    float side;
    float up;
    unsigned int start;
    int i;
    int j;

    start = PortTimestamp();
    for (i = 0; i < count; i++) {
        struct MeshVert *ring = (struct MeshVert *)DAT_006139c8[i];

        obj->vt->pair[fn].fn(obj, keys[i], &dir);
        BuildBasisFromDir(&dir, basis);
        obj->vt->pair[fn].func(obj, keys[i], &pos);
        pos.x += offset->x;
        pos.y += offset->y;
        pos.z += offset->z;
        MakeMat4FromRotTrans((unsigned int *)&pos, (unsigned int *)basis, (unsigned int (*)[4])m4);
        Mat4Multiply(CastleWorldToScreen.m, m4, out4);
        ProjectPoints(DAT_006121c8, (int (*)[4])ring, &out4[0][0], 0x14, 6);
        /* how much the light lines up with the frame's second and third axes */
        side = (light[2] * basis[1].z + light[1] * basis[1].y + light[0] * basis[1].x) * CastleLightScale;
        up = (light[2] * basis[2].z + light[1] * basis[2].y + light[0] * basis[2].x) * CastleLightScale;
        for (j = 0; j < 6; j++) {
            ring[j].u = (int)(up * DAT_006126d8[j][1] + side * DAT_006126d8[j][0] + CastleLightBias);
        }
    }
    RailTubeMesh.corner_count = 18 * (count - 1) + 6;
    RailTubeMesh.face_count = 12 * (count - 1);
    DAT_00615f68 += PortTimestamp() - start;
    return &RailTubeMesh;
}

// FUNCTION: LEGOLAND 0x00428e70
void DrawTrackRail(struct SegBlob *p, const struct FVec3 *a, unsigned int b) {
    unsigned int handle = FPUSetSinglePrecision();
    unsigned int result = ((unsigned int (**)(void *, const char *))p->vt)[6](p, (const char *)RailSplitPoints);
    RailSplitCount = result;
    BuildRailTubeMesh(p, (const struct FVec3 *)a, b, result, RailSplitPoints);
    DrawEdgeMeshTriangles(&RailTubeMesh);
    FPURestoreControlWord(handle);
}

// FUNCTION: LEGOLAND 0x00428ec0
void DrawTrackRailSameSplit(struct SegBlob *p, const struct FVec3 *a, unsigned int b) {
    unsigned int handle = FPUSetSinglePrecision();
    BuildRailTubeMesh(p, a, b, RailSplitCount, RailSplitPoints);
    DrawEdgeMeshTriangles(&RailTubeMesh);
    FPURestoreControlWord(handle);
}

// FUNCTION: LEGOLAND 0x00428f00
void InitRailTubeMesh(void) {
    unsigned int *volatile ptmp7;
    unsigned int *ptmp8;
    int i, j, k, m, n, t, e, v;
    register float ang;
    register int cnt;
    float s;
    unsigned int *g, *out;
    int (*d)[2];
    int (*sp)[2];
    int k6, k18;

    for (i = 0; 4 >= i; i++) {
        DAT_00613908[i][0] = i;
        DAT_00613908[i][1] = i + 1;
    }
    DAT_00613908[5][0] = i;
    DAT_00613908[5][1] = 0;
    n = 6;
    for (j = 0; j <= 4; j++) {
        DAT_00613908[n][0] = j;
        DAT_00613908[n][1] = j + 6;
        n++;
        DAT_00613908[n][0] = j;
        DAT_00613908[n][1] = 7 + j;
        n++;
    }
    DAT_00613908[n][0] = j;
    DAT_00613908[n][1] = j + 6;
    n++;
    DAT_00613908[n][0] = j;
    DAT_00613908[n][1] = 6;
    n++;
    for (j = 0; j <= 4; j++) {
        DAT_00613908[n][0] = j + 6;
        DAT_00613908[n][1] = 7 + j;
        n++;
    }
    DAT_00613908[n][0] = j + 6;
    DAT_00613908[n][1] = 6;

    t = 0;
    v = 6;
    for (e = 18; 23 > e; e++) {
        DAT_00613878[t * 3] = e - 18 + 0x80000000;
        DAT_00613878[t * 3 + 1] = v + 1;
        DAT_00613878[t * 3 + 2] = v + 2 + 0x80000000;
        t++;
        DAT_00613878[t * 3] = v;
        DAT_00613878[t * 3 + 1] = e;
        DAT_00613878[t * 3 + 2] = v + 1 + 0x80000000;
        t++;
        v += 2;
    }
    DAT_00613878[t * 3] = 0x80000005;
    DAT_00613878[t * 3 + 1] = v + 1;
    DAT_00613878[t * 3 + 2] = 0x80000006;
    DAT_00613878[t * 3 + 3] = v;
    DAT_00613878[t * 3 + 4] = e;
    DAT_00613878[t * 3 + 5] = v + 1 + 0x80000000;

    n = 0;
    ptmp8 = DAT_00612708[0];
    ptmp7 = ptmp8;
    g = ptmp7;
    k = 0;
    while (k < 30) {
        k6 = k * 6;
        k18 = k * 18;
        cnt = 18;
        if (k == 29) {
            cnt = 24;
        }
        d = &DAT_006148b8[n];
        sp = DAT_00613908;
        n += cnt;
        do {
            (*d)[0] = (*sp)[0] + k6;
            (*d)[1] = (*sp)[1] + k6;
            d++;
            sp++;
        } while (--cnt);
        out = g;
        g = g + 36;
        for (i = 0; i < 12; i++) {
            for (j = 0; 3 > j; j++) {
                unsigned int x = DAT_00613878[i * 3 + j];
                *out++ = ((x & 0x7fffffff) + k18) | (x & 0x80000000);
            }
        }
        k++;
    }

    ang = FLOAT_004ab390;
    for (i = 0; i < (unsigned)6; i++) {
        DAT_006121c8[i][0] = 0.0f;
        DAT_006121c8[i][1] = (float)cos(ang) * 1.8f;
        s = (float)sin(ang);
        DAT_006121c8[i][2] = s * 1.8f;
        DAT_006126d8[i][0] = (float)cos(ang);
        DAT_006126d8[i][1] = s;
        ang += 1.0471976f;
    }
}

// FUNCTION: LEGOLAND 0x00429150
void DrawTrackSupport(struct SegBlob *curve, float *off, int flag) {
    struct FVec3 pos;
    struct FVec3 dir;
    struct FVec3 basis[3];
    float t = (curve->end + curve->start) * 0.5f;

    curve->vt->method_c(curve, t, &dir.x);
    BuildBasisFromDir(&dir, basis);
    curve->vt->method_8(curve, t, &pos.x);
    pos.x += off[0];
    pos.y += off[1];
    pos.z += off[2];
    if (pos.z > DOUBLE_004ab478) {
        DrawTrackTie(&pos, basis);
        return;
    }
    if (Vec3Dot((struct FVec3 *)CastleViewDir, &basis[2]) < FLOAT_004ab390) {
        if (flag != 1) {
            DrawSupportPost(&pos, basis);
        }
        DrawTrackTie(&pos, basis);
    } else {
        DrawTrackTie(&pos, basis);
        if (flag != 1) {
            DrawSupportPost(&pos, basis);
        }
    }
}

// FUNCTION: LEGOLAND 0x00429270
void InitSupportPostShape(void) {
    int i;

    for (i = 0; i <= 3; i++) {
        DAT_006121c8[6 + i][1] = DAT_006121c8[6 + i][0] = DAT_004b5f80[0][i].a * 5.0f;
    }
    for (i = 0; i <= 3; i++) {
        DAT_00612210[1][i].a = DAT_004b5f80[1][i].a * 1.5f;
        DAT_00612210[1][i].b = DAT_004b5f80[1][i].b * 1.5f;
    }
    for (i = 0; i <= 3; i++) {
        DAT_00612210[2][i].a = DAT_004b5f80[2][i].a * 1.5f;
        DAT_00612210[2][i].b = DAT_004b5f80[2][i].b * 1.5f;
    }
}

// FUNCTION: LEGOLAND 0x004292f0
void DrawSupportPost(struct FVec3 *pos, struct FVec3 *basis) {
    /* Port [library:asm]: the original is inline asm (rdtsc, x87). Builds the 3-part marker mesh (3 groups of 4
     * vertices in DAT_006137e8) at pos and draws it through DrawLmsModel. Part 0 is a quad snapped to a grid of
     * 1.0 (pos / 5 truncated, times 1.0), part 1 a flat quad at pos, part 2 a quad lying in the plane through
     * pos whose normal is basis[2]. */
    struct FVec3 *normal = &basis[2];
    float zero_pos[3];
    float identity[9];
    double gx;
    double gy;
    float x;
    float y;
    float d;
    unsigned int start;
    int i;

    start = PortTimestamp();
    gx = (double)(int)pos->x * 0.2 * 5.0f;
    gy = (double)(int)pos->y * 0.2 * 5.0f;
    for (i = 0; i < 4; i++) {
        DAT_006137e8[0][i][0] = (float)(gx + DAT_00612210[0][i].a);
        DAT_006137e8[0][i][1] = (float)(gy + DAT_00612210[0][i].b);
        DAT_006137e8[0][i][2] = DAT_00612210[0][i].c;
    }
    for (i = 0; i < 4; i++) {
        DAT_006137e8[1][i][0] = DAT_00612210[1][i].a + pos->x;
        DAT_006137e8[1][i][1] = DAT_00612210[1][i].b + pos->y;
        DAT_006137e8[1][i][2] = 0.0f;
    }
    d = Vec3Dot(pos, normal);
    for (i = 0; i < 4; i++) {
        x = DAT_00612210[2][i].a + pos->x;
        y = DAT_00612210[2][i].b + pos->y;
        DAT_006137e8[2][i][0] = x;
        DAT_006137e8[2][i][1] = y;
        DAT_006137e8[2][i][2] = (d - x * normal->x - y * normal->y) / normal->z;
    }
    DAT_00615f68 += PortTimestamp() - start;
    zero_pos[0] = 0.0f;
    zero_pos[1] = 0.0f;
    zero_pos[2] = 0.0f;
    identity[0] = 1.0f;
    identity[1] = 0.0f;
    identity[2] = 0.0f;
    identity[3] = 0.0f;
    identity[4] = 1.0f;
    identity[5] = 0.0f;
    identity[6] = 0.0f;
    identity[7] = 0.0f;
    identity[8] = 1.0f;
    DrawLmsModel(&SupportPostModel, (struct LmsRec *)DAT_00615f70, zero_pos, identity, 1);
}

// FUNCTION: LEGOLAND 0x00429490
void DrawTrackTie(struct FVec3 *param_1, struct FVec3 *param_2) {
    DrawLmsModel(&TrackTieModel, (struct LmsRec *)DAT_004b62f0, param_1, param_2, 1);
}

// FUNCTION: LEGOLAND 0x004294b0
void InitTrackTieShape(void) {
    int i;

    for (i = 0; i < 8; i++) {
        TrackTieVerts[i].a = TrackTieShape[i].a * 2.5f;
        TrackTieVerts[i].b = TrackTieShape[i].b * 8.2f;
        TrackTieVerts[i].c = TrackTieShape[i].c;
    }
}

// FUNCTION: LEGOLAND 0x004294f0
void DrawTrackStretch(struct SegBlob *a, struct FVec3 *b, unsigned int c, unsigned int d) {
    SetTrackLighting();
    if (c == 1) {
        DrawTrackRail(a, b, 0);
        DrawTrackSupport(a, &b->x, d);
        DrawTrackRailSameSplit(a, b, 2);
    } else {
        DrawTrackRail(a, b, 2);
        DrawTrackSupport(a, &b->x, d);
        DrawTrackRailSameSplit(a, b, 0);
    }
}

// FUNCTION: LEGOLAND 0x00429560
void BuildSlopeRun(struct PathSeg *seg, struct PathSeg *end, int count, float h, float dh) {
    float t;
    float step;
    struct FVec3 b;
    struct FVec3 a;
    struct SegBlob blob;
    unsigned int di = DirMaskToIndex(seg->dir_a);
    unsigned int dout = DirMaskToIndex(end->dir_b);
    int i;

    step = 1.0f / (float)count;
    t = 0.0f;
    MapPosToWorld(&seg->pos, h, &a);
    a.x += DAT_004b6398[di][0];
    a.y += DAT_004b6398[di][1];
    MapPosToWorld(&end->pos, h + dh, &b);
    b.x += DAT_004b6398[dout][0];
    b.y += DAT_004b6398[dout][1];
    InitSlopeCurve(&a.x, &b.x, (struct Words3 *)DAT_004b63c8[di], (struct Obj421ab0 *)&blob);
    for (i = 0; i < count; i++) {
        seg->flags |= 1;
        seg->blob = blob;
        seg->blob.start = t;
        t += step;
        seg->blob.end = t;
        seg->field_40 = 0;
        seg->field_44 = 0;
        seg->field_48 = 0;
        seg = seg->next;
    }
}

// FUNCTION: LEGOLAND 0x00429690
struct PathSeg *SetHeightUntilStraight(struct PathSeg *n, float v, struct PathSeg *end) {
    while (!IsStraightSegment(n->cls, n->dir_a, n->dir_b) && n != end) {
        n->f24 = n->f18 = v;
        n = n->next;
    }
    return n;
}

// FUNCTION: LEGOLAND 0x004296f0
struct PathSeg *FindStraightRunEnd(struct PathSeg *param_1, int *param_2) {
    struct PathSeg *node = param_1->next;
    *param_2 = 1;

    if (!IsStraightSegment(node->cls, node->dir_a, node->dir_b)) {
        return node->prev;
    }

    do {
        ++(*param_2);
        node = node->next;
    } while (IsStraightSegment(node->cls, node->dir_a, node->dir_b));

    return node->prev;
}

// FUNCTION: LEGOLAND 0x00429750
void InterpolateCoasterHeights(struct PathSeg *seg, struct PathSeg *end) {
    struct PathSeg *n;
    struct PathSeg *node;
    unsigned int count;
    float base;
    float step;

    count = CountStraightSegsForward(seg->next, &n);
    base = (float)(int)seg->cls->f4;
    if (count) {
        step = (float)(int)(end->cls->f8 - seg->cls->f4) / (int)count;
        n = seg->next;
        while (n != end) {
            if (IsStraightSegment(n->cls, n->dir_a, n->dir_b)) {
                int k;
                float h;
                node = FindStraightRunEnd(n, &k);
                h = (float)k * step;
                BuildSlopeRun(n, node, k, base, h);
                base = h + base;
                n = node->next;
            } else {
                n = SetHeightUntilStraight(n, base, end);
            }
        }
    }
}

// FUNCTION: LEGOLAND 0x00429840
struct PathSeg *FindReachableAnchor(struct PathSeg *n, int x) {
    struct PathSeg *r;
    unsigned int ok;

    if (n->next == 0) {
        ok = CountStraightSegsBack(n, &n);
        r = n;
        if (ok || (float)x == r->f24) {
            return r;
        }
        return 0;
    } else {
        ok = CountStraightSegsForward(n, &n);
        r = n;
        if (ok || (float)x == r->f18) {
            return r;
        }
        return 0;
    }
}

// FUNCTION: LEGOLAND 0x004298a0
int HasStraightForClosingSlope(struct PieceClass *info, struct CastleState *owner, struct PathSeg **a, struct PathSeg **b) {
    int n = CountStraightSegsBack(owner->f_a8, a) + CountStraightSegsForward(owner->f_c0, b);
    unsigned int da = GetOppositeDirection(owner->f_a8->dir_b);
    unsigned int db = GetOppositeDirection(owner->f_c0->dir_a);

    if (IsStraightSegment(info, da, db)) {
        n++;
    }
    return n > 0;
}

// FUNCTION: LEGOLAND 0x00429910
unsigned int IsStraightSegment(struct PieceClass *s, unsigned int v, unsigned int c) {
    unsigned int flag = (s->flags == 0);
    flag &= 1;
    if (flag) {
        if (c == GetOppositeDirection(v)) {
            return 1;
        }
    }
    return 0;
}

// FUNCTION: LEGOLAND 0x00429940
unsigned int CountStraightSegsBack(struct PathSeg *n, struct PathSeg **out) {
    unsigned int count = 0;

    while (!(n->cls->flags & 1)) {
        if (IsStraightSegment(n->cls, n->dir_a, n->dir_b)) {
            count++;
        }
        n = n->prev;
    }
    *out = n;
    return count;
}

// FUNCTION: LEGOLAND 0x00429990
unsigned int CountStraightSegsForward(struct PathSeg *n, struct PathSeg **out) {
    unsigned int count = 0;

    while (!(n->cls->flags & 1)) {
        if (IsStraightSegment(n->cls, n->dir_a, n->dir_b)) {
            count++;
        }
        n = n->next;
    }
    *out = n;
    return count;
}

// FUNCTION: LEGOLAND 0x004299e0
void ResetCoasterHeightsBack(struct PathSeg *n) {
    struct PathSeg *p = n;
    float v;

    while (!(p->cls->flags & 1)) {
        p = p->prev;
    }
    v = (float)(int)p->cls->f4;
    p = p->next;
    while (p != n->next) {
        unsigned int fl = p->flags & ~1u;
        p->f18 = v;
        p->f24 = v;
        p->flags = fl;
        p = p->next;
    }
}

// FUNCTION: LEGOLAND 0x00429a30
void ResetCoasterHeightsForward(struct PathSeg *n) {
    struct PathSeg *p = n;
    float v;

    while (!(p->cls->flags & 1)) {
        p = p->next;
    }
    v = (float)(int)p->cls->f8;
    p = p->prev;
    while (p != n->prev) {
        unsigned int fl = p->flags & ~1u;
        p->f18 = v;
        p->f24 = v;
        p->flags = fl;
        p = p->prev;
    }
}

// FUNCTION: LEGOLAND 0x00429a80
void GetWalkerPoint(struct CurveWalker *a, int param0, int param2, struct FVec3 *b) {
    struct SegBlob *base = a->node;
    base->vt->pair[param0].func(base, param2, b);
    b->x += a->pos.x;
    b->y += a->pos.y;
    b->z += a->pos.z;
}

// FUNCTION: LEGOLAND 0x00429ac0
void GetWalkerTangent(struct CurveWalker *a, int b, int c, void *d) {
    struct SegBlob *obj = a->node;
    obj->vt->pair[b].fn(obj, c, d);
}

// FUNCTION: LEGOLAND 0x00429af0
void BuildUprightBasis(struct FVec3 *a, struct FVec3 *b) {
    struct FVec3 *m = b;
    struct FVec3 *v = a;
    int i;

    m[2].x = 0.0f;
    m[2].y = 0.0f;
    m[2].z = 1.0f;
    Vec3Cross(&m[2], v, &m[1]);
    Vec3Cross(v, &m[1], &m[2]);
    m[0] = *v;
    for (i = 0; i < 3; i++) {
        Vec3Normalize(&m[i].x);
    }
}

// FUNCTION: LEGOLAND 0x00429b60
void GetWalkerBasis(struct CurveWalker *a, int b, void *out) {
    int local[3];
    GetWalkerTangent(a, 1, b, local);
    BuildUprightBasis((struct FVec3 *)&local[0], out);
}

// FUNCTION: LEGOLAND 0x00429b90
unsigned int GetWalkerNormal(struct CurveWalker *param_1, unsigned int param_2, unsigned int param_3, float *param_4) {
    struct SegBlob *inner = param_1->node;
    return inner->vt->method_1c(inner, param_3, param_4);
}

// FUNCTION: LEGOLAND 0x00429bb0
void GetWalkerPointBelow(struct CurveWalker *a, int b, int c, float scale, float *d) {
    float v[3];

    GetWalkerPoint(a, b, c, (struct FVec3 *)d);
    GetWalkerNormal(a, b, c, v);
    d[0] = d[0] - v[0] * scale;
    d[1] = d[1] - v[1] * scale;
    d[2] = d[2] - v[2] * scale;
}

struct Out429c60 {
    int type;
    struct FVec3 v;
    unsigned char pad[0x54 - 0x10];
};

// FUNCTION: LEGOLAND 0x00429c10
void WalkerPointBelowFn(float t, struct FloatArray *out) {
    struct FVec3 v;

    /* the integrator's time comes in as a float; its bits are the walker's position (GetWalkerTangentBelow passed an int) */
    GetWalkerPointBelow(RangeWalker, DAT_00615f90, *(int *)&t, RangeDepth, (float *)&v);
    out->data[0] = v.x;
    out->data[1] = v.y;
    out->data[2] = v.z;
    out->count = 3;
}

// FUNCTION: LEGOLAND 0x00429c60
void GetWalkerTangentBelow(struct CurveWalker *obj, int a, unsigned int b, float c, struct FVec3 *out) {
    struct Out429c60 local;

    if (c != FLOAT_004ab390) {
        RangeWalker = obj;
        DAT_00615f90 = a;
        RangeDepth = c;
        VecDerivative(WalkerPointBelowFn, &CurveVecMath, *(float *)&b, 0.01f, (struct FloatArray *)&local);
        out->x = local.v.x;
        out->y = local.v.y;
        out->z = local.v.z;
    } else {
        GetWalkerTangent(obj, a, b, out);
    }
}

// FUNCTION: LEGOLAND 0x00429cf0
float WalkerRangeFn(unsigned int x) {
    float v[3];
    float w[3];
    float *c;
    float d;

    DAT_00615fcc++;
    GetWalkerPoint(RangeWalker, 1, x, (struct FVec3 *)v);
    c = RangeCentre;
    v[0] = v[0] - c[0];
    v[1] = v[1] - c[1];
    v[2] = v[2] - c[2];
    if ((d = v[2] * v[2] + v[1] * v[1] + v[0] * v[0]) > RangeOuterSq) {
        DAT_00615fe4++;
        return 1.0f;
    }
    if (RangeRadius > RangeDepth && d < RangeInnerSq) {
        return FLOAT_004ab468;
    }
    GetWalkerNormal(RangeWalker, 1, x, w);
    DAT_00615fe8++;
    v[0] = v[0] - RangeDepth * w[0];
    v[1] = v[1] - RangeDepth * w[1];
    v[2] = v[2] - RangeDepth * w[2];
    return v[2] * v[2] + v[1] * v[1] + v[0] * v[0] - RangeRadiusSq;
}

// FUNCTION: LEGOLAND 0x00429e20
int BisectRoot(float (*fn)(unsigned int), float lo, float hi, float *out) {
    float fa;
    int n;

    DAT_00615fc4++;
    DAT_00615fc8++;
    n = 1;
    fa = fn(*(unsigned int *)&lo);
    {
        float fb = fn(*(unsigned int *)&hi);
        if (!((*(unsigned int *)&fa ^ *(unsigned int *)&fb) & 0x80000000)) {
            return 0;
        }
    }
    while (hi - lo > 0.005f) {
        float mid = (lo + hi) * 0.5f;
        float fb = fn(*(unsigned int *)&mid);
        if ((*(unsigned int *)&fb ^ *(unsigned int *)&fa) & 0x80000000) {
            hi = mid;
        } else {
            lo = mid;
            fa = fb;
        }
        DAT_00615fc8++;
        n++;
    }
    *out = (lo + hi) * 0.5f;
    if (n > DAT_00615fec) {
        DAT_00615fec = n;
    }
    return 1;
}

// FUNCTION: LEGOLAND 0x00429f30
void FindPathPosAtRangeBehind(float *center, float r, struct CurveWalker *src, float hi, float x, struct CurveWalker *dst, float *out) {
    struct CurveWalker cur;
    float lo;
    int found;

    cur = *src;
    lo = *(float *)&cur.node->start_bits;
    RangeRadiusSq = r * r;
    RangeCentre = center;
    RangeRadius = r;
    RangeDepth = x;
    RangeWalker = &cur;
    RangeOuterSq = (r + x) * (r + x);
    RangeInnerSq = (r - x) * (r - x);
    found = RangeRootFinder(WalkerRangeFn, lo, hi, out);
    while (!found) {
        float hi2;
        CurveWalkerPrev(&cur);
        lo = *(float *)&cur.node->start_bits;
        hi2 = *(float *)&cur.node->end_bits;
        found = RangeRootFinder(WalkerRangeFn, lo, hi2, out);
    }
    *dst = cur;
}

// FUNCTION: LEGOLAND 0x0042a020
void FindPathPosAtRangeAhead(float *center, float r, struct CurveWalker *src, float start, float x, struct CurveWalker *dst, float *out) {
    struct CurveWalker cur;
    float end;
    int found;

    cur = *src;
    end = *(float *)&cur.node->end_bits;
    RangeRadiusSq = r * r;
    RangeCentre = center;
    RangeRadius = r;
    RangeDepth = x;
    RangeWalker = &cur;
    RangeOuterSq = (r + x) * (r + x);
    RangeInnerSq = (r - x) * (r - x);
    found = RangeRootFinder(WalkerRangeFn, start, end, out);
    while (!found) {
        CurveWalkerNext(&cur);
        start = *(float *)&cur.node->start_bits;
        end = *(float *)&cur.node->end_bits;
        found = RangeRootFinder(WalkerRangeFn, start, end, out);
    }
    *dst = cur;
}

// FUNCTION: LEGOLAND 0x0042a110
int CurveWalkersEqual(struct CurveWalker *p1, struct CurveWalker *p2) {
    if (p1->piece != p2->piece) {
        return 0;
    }
    if (p1->node != p2->node) {
        return 0;
    }
    return Vec3Equal(&p1->pos.x, &p2->pos.x) != 0;
}

// FUNCTION: LEGOLAND 0x0042a150
float CurveTangentLength(unsigned int b) {
    float sum = 0.0f;
    float v[3];
    int i;

    GetWalkerTangentBelow(ArcLengthWalker, ArcLengthCurvePair, b, ArcLengthDepth, (struct FVec3 *)v);
    for (i = 0; i < 3; i++) {
        sum += v[i] * v[i];
    }
    return (float)sqrt(sum);
}

// FUNCTION: LEGOLAND 0x0042a1b0
float CurveArcLength(struct CurveWalker *a, unsigned int hi, struct CurveWalker *b, unsigned int lo, int idx, float c) {
    struct CurveWalker cur;
    float acc;

    ArcLengthCurvePair = idx;
    ArcLengthDepth = c;
    if (CurveWalkersEqual(a, b)) {
        ArcLengthWalker = a;
        return IntegrateSimpson(CurveTangentLength, lo, hi, 0.01f);
    }
    cur = *b;
    ArcLengthWalker = &cur;
    acc = IntegrateSimpson(CurveTangentLength, lo, cur.node->end_bits, 0.01f);
    CurveWalkerNext(&cur);
    while (!CurveWalkersEqual(&cur, a)) {
        ArcLengthWalker = &cur;
        acc = IntegrateSimpson(CurveTangentLength, cur.node->start_bits, cur.node->end_bits, 0.01f) + acc;
        CurveWalkerNext(&cur);
    }
    ArcLengthWalker = &cur;
    return IntegrateSimpson(CurveTangentLength, cur.node->start_bits, hi, 0.01f) + acc;
}

// FUNCTION: LEGOLAND 0x0042a2e0
void InitCurveVecMath(void) {
    InitVecMath(&CurveVecMath, 3);
}

// FUNCTION: LEGOLAND 0x0042a2f0
void RasteriseTriangle(int n, struct PolyArg *poly) {
    /* Port [library:asm]: the original is inline asm (fistp). Rasterizes a triangle with n interpolated values (x, then
     * attr[0..n-2]): clips it to the view (ClipTriangle) unless all corners are inside, computes the per-pixel
     * gradients of the attributes, builds the edge table sorted by first scanline and hands it to a filler.
     * Outside dirty rectangles the last attribute (the depth) is dropped and fillers[0] is used. */
    struct PolyVert **v = poly->v;
    struct RecSrc edges[9];
    struct RecIdx order[9];
    struct RecIdx tmp;
    struct PolyVert *a;
    struct PolyVert *b;
    struct RecSrc *e;
    int shade[4];
    int count = 3;
    int inside;
    int edge_count = 0;
    int dy;
    float scale;
    float grad;
    float inv;
    int i;
    int k;

    if ((poly->or_codes & 0xf0) == 0xf0) {
        inside = 1;
    } else {
        inside = 0;
        n--;
    }
    if ((poly->and_codes & 0xf) != 0xf) {
        count = 0;
        v = (struct PolyVert **)ClipTriangle(v, &count, poly->and_codes & 0xf, n);
        if (count == 0) {
            return;
        }
    }
    v[count] = v[0];

    shade[0] = poly->shade;
    if (n > 1) {
        scale = 65536.0f / poly->area;
        for (k = 0; k < n - 1; k++) {
            int d1 = poly->v[1]->attr[k] - poly->v[0]->attr[k];
            int d2 = poly->v[2]->attr[k] - poly->v[0]->attr[k];

            grad = ((float)d1 * poly->dy2 - (float)d2 * poly->dy1) * scale;
            shade[k + 1] = PortRound(grad);
        }
    }

    for (i = 0; i < count; i++) {
        a = v[i];
        b = v[i + 1];
        dy = b->y - a->y;
        if (dy == 0) {
            continue;
        }
        inv = 65536.0f / (float)dy;
        e = &edges[edge_count];
        order[edge_count].k = edge_count;
        edge_count++;
        if (dy < 0) {
            /* upward edge: the left side; runs from b down to a, carrying all n values */
            order[edge_count - 1].v = b->y;
            e->flag = 0;
            ((short *)&e->f0)[0] = (short)b->y;
            ((short *)&e->f0)[1] = (short)a->y;
            for (k = 0; k < n; k++) {
                (&e->fx)[k] = (&b->x)[k] << 16;
                (&e->d)[k] = PortRound((float)((&b->x)[k] - (&a->x)[k]) * inv);
            }
        } else {
            /* downward edge: the right side; only x */
            order[edge_count - 1].v = a->y;
            e->flag = 1;
            ((short *)&e->f0)[0] = (short)a->y;
            ((short *)&e->f0)[1] = (short)b->y;
            e->fx = a->x << 16;
            e->d = PortRound((float)(b->x - a->x) * inv);
        }
    }
    if (edge_count == 0) {
        return;
    }

    /* bubble sort by first scanline */
    for (i = edge_count - 1; i >= 0; i--) {
        for (k = 1; k <= i; k++) {
            if (order[k - 1].v > order[k].v) {
                tmp = order[k - 1];
                order[k - 1] = order[k];
                order[k] = tmp;
            }
        }
    }
    if ((poly->flags & 1) && inside == 1) {
        RecordDepthSpans(edge_count, ((short *)&edges[order[edge_count - 1].k].f0)[1], order, edges);
    }
    poly->fillers[inside](poly->palette, shade, edge_count, order, edges);
}

static __inline void Init42a5e0(struct TrainWheel *id, struct CurveWalker *src, float v) {
    id->walker = *src;
    id->t = v;
}

// FUNCTION: LEGOLAND 0x0042a5e0
void InitTrainBogie(struct TrainBogie *o, struct CurveWalker *src, float param_5) {
    Init42a5e0(&o->w0, src, param_5);
    Init42a5e0(&o->w1, src, param_5);
}

// FUNCTION: LEGOLAND 0x0042a620
void SetTrainWheelPos(struct TrainWheel *param_1, struct CurveWalker *param_2, unsigned int param_5) {
    memcpy(&param_1->walker, param_2, sizeof(struct CurveWalker));
    *(unsigned int *)&param_1->t = param_5;
}

// FUNCTION: LEGOLAND 0x0042a640
unsigned int GetTrainWheelPos(struct TrainWheel *ptr, unsigned int a, float *b) {
    GetWalkerPointBelow(&ptr->walker, WheelCurvePairs[a], *(int *)&ptr->t, 4.8f, b);
}

// FUNCTION: LEGOLAND 0x0042a670
float GetWheelSpinStep(struct TrainBogie *p, int i) {
    return FLOAT_004ab390;
}

// FUNCTION: LEGOLAND 0x0042a680
void DrawTrainBogie(struct TrainBogie *p) {
    struct TrainBogie *o = p;
    float E[4][4];
    unsigned int B[4][4];
    unsigned int A[9];
    unsigned int G[9];
    float D[4][4];
    float C[3];
    int i;

    SetModelLighting();
    GetWalkerBasis(&o->w0.walker, *(int *)&o->w0.t, A);
    Mat3ToMat4Transposed(A, B);
    for (i = 0; i <= 1; i++) {
        o->ang[i] = GetWheelSpinStep(p, i) + o->ang[i];
        GetTrainWheelPos(&p->w0, i, C);
        Mat4Identity(D);
        D[0][0] = (float)sin(o->ang[i]);
        D[2][0] = -(D[0][2] = (float)cos(o->ang[i]));
        D[2][2] = (float)sin(o->ang[i]);
        Mat4Multiply((float (*)[4])B, D, E);
        Mat4ToMat3Transpose(G, E);
        DrawLmsModel(CoasterTrainWheelLms, CoasterTrainWheelLfm, C, G, 0);
    }
    o->w1.t = o->w0.t;
    o->w1.walker = o->w0.walker;
}

// FUNCTION: LEGOLAND 0x0042a780
void LoadCoasterTrainWheelModel(void) {
    // STRING: LEGOLAND 0x004b6414
    const char *str = "coastertrain.wheel01";
    CoasterTrainWheelLms = GetLmsByName(str);
    CoasterTrainWheelLfm = GetLfmByName(str);
}
