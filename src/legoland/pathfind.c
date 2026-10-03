#include <stdlib.h>
#include "legoland.h"

#include "globals.h"
#include "llidb.h"
#include "objclass.h"
#include "pathfind.h"

struct PathQuery {
    int x;
    int y;
};

struct PathBox {
    unsigned char pad_0[8];
    int x_min;
    int y_min;
    int x_max;
    int y_max;
    unsigned char pad_18[4];
    int dist_sq;
};

struct DirNode {
    struct DirNode *next;
    void *parent;
    int x;
    int y;
};

struct PathLink {
    unsigned char pad_0[4];
    struct PathLink *prev;
    int x;
    int y;
};

struct ObjData {
    unsigned char pad_0[0x3c];
    int field_3c;
    int field_40;
    unsigned char pad_44[4];
    int field_48;
};

struct ElemInfo {
    unsigned char pad_0[0xc];
    struct ObjData *obj;
};

struct MatchResult {
    unsigned char pad_0[4];
    unsigned char anchor_x;
    unsigned char anchor_y;
};

// FUNCTION: LEGOLAND 0x00481c50
LEGO_EXPORT void AddPathSquare(struct Point *pos) {
    struct BestNode *node;
    unsigned int x;
    unsigned int y;

    if (FindBestNodeAtPoint(pos)) {
        return;
    }

    node = AddBestNode();
    x = pos->x;
    node->x_max = x;
    node->x_min = x;
    y = pos->y;
    node->y_max = y;
    node->y_min = y;
    MergeBestNode(node);
}

// FUNCTION: LEGOLAND 0x00481c90
LEGO_EXPORT void RemovePathSquare(struct Point *pos) {
    struct BestNode *found;
    struct BestNode *node;
    struct BestBox box;

    found = FindBestNodeAtPoint(pos);
    if (found == NULL) {
        return;
    }
    box = found->box;
    RemoveBestNode(found);

    if (box.y_min < pos->y) {
        node = AddBestNode();
        node->y_min = box.y_min;
        node->y_max = pos->y - 1;
        node->x_min = box.x_min;
        node->x_max = box.x_max;
        MergeBestNode(node);
    }
    if (box.y_max > pos->y) {
        node = AddBestNode();
        node->y_min = pos->y + 1;
        node->y_max = box.y_max;
        node->x_min = box.x_min;
        node->x_max = box.x_max;
        MergeBestNode(node);
    }
    if (box.x_min < pos->x) {
        node = AddBestNode();
        node->y_min = node->y_max = pos->y;
        node->x_min = box.x_min;
        node->x_max = pos->x - 1;
        MergeBestNode(node);
    }
    if (box.x_max > pos->x) {
        node = AddBestNode();
        node->y_min = node->y_max = pos->y;
        node->x_min = pos->x + 1;
        node->x_max = box.x_max;
        MergeBestNode(node);
    }
}

// FUNCTION: LEGOLAND 0x00481e60
void FUN_00481e60(struct PathQuery *query, struct PathBox *box) {
    int px = query->x;
    int lower_x = (box->x_min << 8) + 128;
    int upper_x = (box->x_max << 8) + 128;
    int lower_y = (box->y_min << 8) + 128;
    int upper_y = (box->y_max << 8) + 128;
    int clamped_x;
    int clamped_y;
    int py;
    int dx;
    int dy;

    if (px < lower_x) {
        clamped_x = lower_x;
    } else if (px > upper_x) {
        clamped_x = upper_x;
    } else {
        clamped_x = px;
    }

    py = query->y;
    if (py < lower_y) {
        clamped_y = lower_y;
    } else if (py > upper_y) {
        clamped_y = upper_y;
    } else {
        clamped_y = py;
    }

    dx = clamped_x - px;
    dy = clamped_y - query->y;
    box->dist_sq = (dx * dx) + (dy * dy);
}

// FUNCTION: LEGOLAND 0x00481ee0
void FUN_00481ee0(void) {
    struct BestNode *node;

    node = (struct BestNode *)DAT_0066b44c;
    if (node) {
        while (node) {
            node->field_20 = node->field_20 & 0xfffffffe;
            node = node->next;
        }
    }
}

// FUNCTION: LEGOLAND 0x00481f00
int FUN_00481f00(struct BestNode *target, struct BestNode *start, struct BestNode **out) {
    struct BestNode *a[1020];
    struct BestNode *b[1020];
    struct BestNode **cur = a;
    struct BestNode **next = b;
    int count;
    int n;
    int i;
    int j;
    struct BestNode *p;

    if (target->field_20 & 1) {
        return 0;
    }
    if (target == start) {
        *out = target;
        return 1;
    }
    a[0] = start;
    count = 1;
    while (count != 0) {
        n = 0;
        for (i = 0; i < count; i++) {
            FUN_004819a0((int *)&cur[i]->x_min);
            for (j = 0; j < (int)DAT_00669254; j++) {
                p = DAT_0066a45c[j];
                if (!(p->field_20 & 1)) {
                    next[n++] = p;
                    if (p == target) {
                        *out = cur[i];
                        return 1;
                    }
                    p->field_20 |= 1;
                }
            }
        }
        if (cur == a) {
            cur = b;
            next = a;
        } else {
            cur = a;
            next = b;
        }
        count = n;
    }
    return 0;
}

// FUNCTION: LEGOLAND 0x00482050
LEGO_EXPORT int SuggestNextMove(struct Point *pos, struct Point *goal, struct Point *out) {
    struct BestNode *start;
    struct BestNode *end;
    struct BestNode *prev;
    int lo_x;
    int hi_x;
    int lo_y;
    int hi_y;

    DAT_004bcec0.x = goal->x >> 8;
    DAT_004bcec0.y = goal->y >> 8;
    start = FUN_004817d0((int *)pos);
    end = FUN_004817d0((int *)goal);
    if (start == 0) {
        return -2;
    }
    if (end == 0) {
        return -1;
    }
    if (start == end) {
        *out = *goal;
        out->x += 0x80;
        out->y += 0x80;
        return 2;
    }
    FUN_00481ee0();
    if (!FUN_00481f00(start, end, &prev)) {
        return -1;
    }
    FUN_00481e60((struct PathQuery *)pos, (struct PathBox *)prev);
    lo_x = prev->x_min << 8;
    hi_x = prev->x_max << 8;
    lo_y = prev->y_min << 8;
    hi_y = prev->y_max << 8;
    if (pos->x < lo_x) {
        out->x = lo_x;
    } else if (pos->x > hi_x) {
        out->x = hi_x;
    } else {
        out->x = pos->x;
    }
    if (pos->y < lo_y) {
        out->y = lo_y;
    } else if (pos->y > hi_y) {
        out->y = hi_y;
    } else {
        out->y = pos->y;
    }
    if ((int)prev->field_1c > 0x18000) {
        lo_x = start->x_min << 8;
        hi_x = start->x_max << 8;
        lo_y = start->y_min << 8;
        hi_y = start->y_max << 8;
        if (out->x < lo_x) {
            out->x = lo_x;
        } else if (out->x > hi_x) {
            out->x = hi_x;
        }
        if (out->y < lo_y) {
            out->y = lo_y;
        } else if (out->y > hi_y) {
            out->y = hi_y;
        }
    }
    out->x += 0x80;
    out->y += 0x80;
    return 1;
}

// FUNCTION: LEGOLAND 0x004821c0
void FUN_004821c0(void) {
    int i;

    for (i = 0; i < 1152; i = i + 1) {
        DAT_00669258[i] = 0;
    }
}

// FUNCTION: LEGOLAND 0x004821e0
void FreeDirNodeList(void) {
    struct DirNode *node;
    struct DirNode *next;

    node = DirSearchNodeList;
    while (node != NULL) {
        next = node->next;
        free(node);
        node = next;
    }
    DirSearchNodeList = NULL;
}

// FUNCTION: LEGOLAND 0x00482210
void FreeDirPathNodes(void) {
    struct DirNode *node;
    struct DirNode *next;

    node = DAT_0066b458;
    while (node != NULL) {
        next = node->next;
        free(node);
        node = next;
    }
    DAT_0066b458 = NULL;
}

// FUNCTION: LEGOLAND 0x00482240
void FUN_00482240(int x, int y, struct DirNode *parent) {
    struct MapElement *tile;
    struct DirNode *node;
    int word_index;

    if (x < 0 || x >= lpConfig->width) {
        return;
    }
    if (y < 0 || y >= lpConfig->height) {
        return;
    }

    tile = &GameMap[y][x];
    if (tile->field_10 & 0x2) {
        return;
    }

    word_index = (x >> 5) + y * 6;
    if ((1u << (x & 0x1f)) & DAT_00669258[word_index]) {
        return;
    }

    node = malloc(16);
    if (node == 0) {
        return;
    }

    node->next = DirSearchNodeList;
    DirSearchNodeList = node;
    node->parent = parent;
    node->x = x;
    node->y = y;
    DAT_00669250++;
    DAT_00669258[word_index] |= 1u << (x & 0x1f);
}

// FUNCTION: LEGOLAND 0x00482300
struct DirNode *FUN_00482300(unsigned int x, unsigned int y) {
    struct DirNode *node;

    node = malloc(16);
    if (node != NULL) {
        node->next = DAT_0066b458;
        DAT_0066b458 = node;
        node->x = x;
        node->y = y;
    }
    return node;
}

// FUNCTION: LEGOLAND 0x00482330
int FUN_00482330(struct PathLink *a, struct PathLink *b, struct PathLink *c, struct PathLink *d) {
    int cx, cy;
    int dx0, dy0, dx1, dy1;
    struct MapElement *e;

    if (b == 0) {
        return 0;
    }
    if (c == 0) {
        return 0;
    }
    cx = c->x;
    dx0 = b->x - a->x;
    dy0 = b->y - a->y;
    dx1 = cx - b->x;
    cy = c->y;
    dy1 = cy - b->y;
    if ((dx1 != 0 && dy0 != 0) || (dy1 != 0 && dx0 != 0)) {
        e = &GameMap[a->y + dy1][a->x + dx1];
        if ((e->field_10 & 2) && !(e->flags & 0x800)) {
            return 0;
        }
        return 1;
    }
    if (d == 0) {
        return 0;
    }
    dx1 = d->x - cx;
    dy1 = d->y - cy;
    if ((dx1 != 0 && dy0 != 0) || (dy1 != 0 && dx0 != 0)) {
        e = &GameMap[a->y + dy1][a->x + dx1];
        if ((e->field_10 & 2) && !(e->flags & 0x800)) {
            return 0;
        }
        return 2;
    }
    return 1;
}

// FUNCTION: LEGOLAND 0x00482430
int FUN_00482430(void) {
    struct PathLink *cur;
    struct PathLink *p2;
    struct PathLink *p3;
    struct PathLink *p4;
    int dir;

    cur = (struct PathLink *)DAT_0066b454;
    p3 = 0;
    p2 = 0;
    p4 = 0;
    if (cur == 0) {
        return 0;
    }
    while (cur->prev != 0) {
        p4 = p3;
        p3 = p2;
        p2 = cur;
        cur = cur->prev;
    }
    dir = FUN_00482330(cur, p2, p3, p4);
    switch (dir) {
    case 0:
        if (p2 != 0) {
            FUN_00482300(p2->x, p2->y);
        }
        return p4 == 0;
    case 1:
        FUN_00482300(p3->x, p3->y);
        return p4 == 0;
    case 2:
        FUN_00482300(p4->x, p4->y);
        break;
    }
    return p4 == 0;
}

// FUNCTION: LEGOLAND 0x004824d0
LEGO_EXPORT int PTPSuggestNextMove(struct Point *pos, struct Point *goal, struct Point *out) {
    int start_x;
    int start_y;
    int goal_x;
    int goal_y;
    int wave;
    struct DirNode *node;

    start_x = pos->x >> 8;
    start_y = pos->y >> 8;
    goal_x = goal->x >> 8;
    goal_y = goal->y >> 8;

    FreeDirNodeList();
    FreeDirPathNodes();
    FUN_004821c0();

    DAT_00669250 = 0;
    DAT_0066b454 = 0;
    FUN_00482240(start_x, start_y, 0);

    while (DAT_00669250 != 0) {
        wave = DAT_00669250;
        node = DirSearchNodeList;
        DAT_00669250 = 0;
        while (wave-- != 0) {
            if (node->x == goal_x && node->y == goal_y) {
                DAT_0066b454 = node;
                if (FUN_00482430()) {
                    out->x = goal->x;
                    out->y = goal->y;
                    FreeDirNodeList();
                    FreeDirPathNodes();
                    return 2;
                }
                out->x = (DAT_0066b458->x << 8) + 0x80;
                out->y = (DAT_0066b458->y << 8) + 0x80;
                FreeDirNodeList();
                FreeDirPathNodes();
                return 1;
            }
            FUN_00482240(node->x, node->y - 1, node);
            FUN_00482240(node->x + 1, node->y, node);
            FUN_00482240(node->x, node->y + 1, node);
            FUN_00482240(node->x - 1, node->y, node);
            node = node->next;
        }
    }

    FreeDirNodeList();
    FreeDirPathNodes();
    return 0;
}

// FUNCTION: LEGOLAND 0x00482620
void FUN_00482620(int x, int y, struct DirNode *parent) {
    struct MapElement *tile;
    struct DirNode *node;
    int word_index;

    if (x < 0 || x >= lpConfig->width) {
        return;
    }
    if (y < 0 || y >= lpConfig->height) {
        return;
    }

    tile = &GameMap[y][x];
    if (tile->field_10 & 0x2) {
        if (!(((unsigned char *)&tile->flags)[1] & 0x8)) {
            return;
        }
    }

    word_index = (x >> 5) + y * 6;
    if ((1u << (x & 0x1f)) & DAT_00669258[word_index]) {
        return;
    }

    node = malloc(16);
    node->next = DirSearchNodeList;
    DirSearchNodeList = node;
    node->parent = parent;
    node->x = x;
    node->y = y;
    DAT_00669250++;
    DAT_00669258[word_index] |= 1u << (x & 0x1f);
}

// FUNCTION: LEGOLAND 0x00482710
int FUN_00482710(int *a, int *b, int *out) {
    int start_x;
    int start_y;
    int goal_x;
    int goal_y;
    int wave;
    struct DirNode *node;

    start_x = a[0] >> 8;
    start_y = a[1] >> 8;
    goal_x = b[0] >> 8;
    goal_y = b[1] >> 8;

    FreeDirNodeList();
    FreeDirPathNodes();
    FUN_004821c0();

    DAT_00669250 = 0;
    DAT_0066b454 = 0;
    FUN_00482620(start_x, start_y, 0);

    while (DAT_00669250 != 0) {
        wave = DAT_00669250;
        node = DirSearchNodeList;
        DAT_00669250 = 0;
        while (wave-- != 0) {
            if (node->x == goal_x && node->y == goal_y) {
                DAT_0066b454 = node;
                if (FUN_00482430()) {
                    out[0] = b[0];
                    out[1] = b[1];
                    FreeDirNodeList();
                    FreeDirPathNodes();
                    return 2;
                }
                out[0] = (DAT_0066b458->x << 8) + 0x80;
                out[1] = (DAT_0066b458->y << 8) + 0x80;
                FreeDirNodeList();
                FreeDirPathNodes();
                return 1;
            }
            FUN_00482620(node->x, node->y - 1, node);
            FUN_00482620(node->x + 1, node->y, node);
            FUN_00482620(node->x, node->y + 1, node);
            FUN_00482620(node->x - 1, node->y, node);
            node = node->next;
        }
    }

    FreeDirNodeList();
    FreeDirPathNodes();
    return 0;
}

// FUNCTION: LEGOLAND 0x00482860
unsigned int FUN_00482860(void) {
    unsigned int count;
    struct BestNode *node;

    count = 0;
    if (DAT_0066b44c != NULL) {
        node = (struct BestNode *)DAT_0066b44c;
        while (node != NULL) {
            count++;
            node = node->next;
        }
    }

    if (SaveGameWrite(&count, 4) == 0) {
        return 0;
    }

    node = (struct BestNode *)DAT_0066b44c;
    while (node != NULL) {
        if (SaveGameWrite(&node->x_min, 20) == 0) {
            return 0;
        }
        if (SaveGameWrite(&node->field_1c, 4) == 0) {
            return 0;
        }
        if (SaveGameWrite(&node->field_20, 4) == 0) {
            return 0;
        }
        node = node->next;
    }

    return 1;
}

// FUNCTION: LEGOLAND 0x004828f0
void FUN_004828f0(void) {
    struct BestNode *next;
    while (DAT_0066b44c != 0) {
        next = ((struct BestNode *)DAT_0066b44c)->next;
        free(DAT_0066b44c);
        DAT_0066b44c = next;
    }
    FUN_00482a80();
}

// FUNCTION: LEGOLAND 0x00482920
int FUN_00482920(void) {
    unsigned int count;
    struct BestNode *node;

    FUN_004828f0();

    if (SaveGameRead(&count, 4) == 0) {
        return 0;
    }

    while (count-- != 0) {
        node = malloc(0x24);
        node->next = DAT_0066b44c;
        DAT_0066b44c = node;
        if (SaveGameRead(&node->x_min, 0x14) == 0) {
            return 0;
        }
        if (SaveGameRead(&node->field_1c, 4) == 0) {
            return 0;
        }
        if (SaveGameRead(&node->field_20, 4) == 0) {
            return 0;
        }
    }

    return 1;
}

// FUNCTION: LEGOLAND 0x004829c0
void FUN_004829c0(struct BestNode *node) {
    int count;
    struct BestNode **copy;
    struct BestNode **p;

    node->field_20 |= 2;
    FUN_004819a0((int *)&node->x_min);
    count = DAT_00669254;
    if (count) {
        copy = (struct BestNode **)malloc(count * 4);
        if (copy != NULL) {
            memcpy(copy, DAT_0066a45c, count * 4);
            p = copy;
            for (; count > 0; count--) {
                if (!((*p)->field_20 & 2)) {
                    FUN_004829c0(*p);
                }
                p++;
            }
            free(copy);
        }
    }
}

// FUNCTION: LEGOLAND 0x00482a40
void FUN_00482a40(struct Point *pos) {
    struct BestNode *node;
    struct BestNode *found;

    if (DAT_0066b44c != NULL) {
        node = (struct BestNode *)DAT_0066b44c;
        while (node != NULL) {
            node->field_20 &= 0xfffffffd;
            node = node->next;
        }
    }

    found = FindBestNodeAtPoint(pos);
    if (found != NULL) {
        FUN_004829c0(found);
    }
}

// FUNCTION: LEGOLAND 0x00482a80
void FUN_00482a80(void) {
    Entrance1Point.x = 0;
    Entrance1Elem = 0;
}

// FUNCTION: LEGOLAND 0x00482a90
void InitEntrance1Point(void) {
    struct MatchResult *match;
    struct ObjData *obj;

    if (Entrance1Point.x != 0) {
        return;
    }

    if (Entrance1Elem == 0) {
        Entrance1Elem = ElemID("ENTRANCE 1");
    }

    match = (struct MatchResult *)GetFirstObjectMatching(
        (Element *)Entrance1Elem);
    obj = ((struct ElemInfo *)Entrance1Elem)->obj;

    Entrance1Point.x = match->anchor_x + obj->field_3c - 1;
    Entrance1Point.y = ((obj->field_48 + obj->field_40) / 2) + match->anchor_y;
}

// FUNCTION: LEGOLAND 0x00482b00
struct Point *GetEntrance1Point(void) {
    return &Entrance1Point;
}
