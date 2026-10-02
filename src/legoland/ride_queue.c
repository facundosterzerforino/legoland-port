#include <stdlib.h>
#include "legoland.h"

#include "bloke.h"
#include "globals.h"
#include "llidb.h"
#include "map_object.h"
#include "math.h"
#include "pathfind.h"
#include "ride_queue.h"
#include "tilemap.h"

struct QueueItemInner {
    unsigned char pad_0[0xe];
    unsigned short field_e;
    unsigned char pad_10[0x24 - 0x10];
    int field_24;
    int field_28;
    unsigned char pad_2c[0x38 - 0x2c];
    short field_38;
    unsigned char pad_3a[0x60 - 0x3a];
    unsigned char field_60;
    unsigned char pad_61;
    unsigned short field_62;
    unsigned char pad_64[0x68 - 0x64];
    int field_68;
    int field_6c;
    unsigned char pad_70[0x73 - 0x70];
    unsigned char field_73;
    unsigned char pad_74[0x98 - 0x74];
    struct Navigator field_98;
};

struct QueueItemMid {
    unsigned char pad_0[0x8];
    struct QueueItemInner *field_8;
};

struct QueueNode {
    struct QueueNode *next;
    struct QueueItemMid *field_4;
};

struct QueueStep {
    int dx;
    int dy;
    int pad;
};

struct QueueTable {
    int count;
    struct QueueStep *steps;
};

struct RideSlotArg {
    unsigned short field_0;
};

struct RideSlot {
    unsigned char pad_0[0x34];
    unsigned char field_34;
    unsigned char pad_35[0x3];
    unsigned short field_38;
    unsigned char pad_3a[0x16];
    struct RideSlotArg *field_50;
    unsigned char pad_54[0xc];
    unsigned char field_60;
};

struct RQObjClass {
    unsigned char pad_0[0xc4];
    struct Element *field_c4;
};

static __inline struct MapElement *TileAt(int x, int y) {
    if (x < 0 || x >= (unsigned short)lpConfig->width || y < 0 || y >= (unsigned short)lpConfig->height) {
        return NULL;
    }
    return &GameMap[y][x];
}

// FUNCTION: LEGOLAND 0x00411e30
void QueueAppendNode(struct Queue *queue, struct QueueNode *node) {
    if (queue->head == NULL && queue->tail == NULL) {
        queue->head = node;
        queue->tail = node;
    } else {
        queue->tail->next = node;
        queue->tail = node;
    }
}

// FUNCTION: LEGOLAND 0x00411e60
unsigned int FUN_00411e60(struct Queue *queue) {
    unsigned int count;
    struct QueueNode *node;

    if (queue->head != NULL) {
        count = 0;
        node = queue->head;
        while (node != NULL) {
            count++;
            node = node->next;
        }
        if (count == (unsigned int)queue->count->count) {
            return 1;
        }
    }
    return 0;
}

// FUNCTION: LEGOLAND 0x00411e90
unsigned int QueueHasNodes(struct Queue *queue) {
    return queue->head != NULL;
}

// FUNCTION: LEGOLAND 0x00411ea0
int FUN_00411ea0(struct Queue *queue) {
    struct QueueNode *head = queue->head;
    if (head != NULL) {
        short value = head->field_4->field_8->field_38;
        if (value == (queue->count->count - 1)) {
            return 1;
        }
    }
    return 0;
}

// FUNCTION: LEGOLAND 0x00411ed0
void FreeAllQueueNodes(struct Queue *queue) {
    struct QueueNode *node = queue->head;
    while (node != NULL) {
        QueueUnlinkHead(queue);
        free(node);
        node = queue->head;
    }
}

// FUNCTION: LEGOLAND 0x00411f00
void QueueUnlinkHead(struct Queue *queue) {
    struct QueueNode *head = queue->head;
    if (head != NULL) {
        queue->head = head->next;
        if (queue->tail == head) {
            queue->tail = NULL;
        }
    }
}

// FUNCTION: LEGOLAND 0x00411f20
void FUN_00411f20(struct Queue *queue, struct QueueItemMid *mid) {
    struct QueueNode *node = (struct QueueNode *)malloc(8);
    if (node != NULL) {
        memset(node, 0, sizeof(*node));
        node->field_4 = mid;
        mid->field_8->field_62 |= 0x40;
        mid->field_8->field_38 = 0;
        mid->field_8->field_60++;
        QueueAppendNode(queue, node);
    }
}

// FUNCTION: LEGOLAND 0x00411f70
int FUN_00411f70(struct Queue *queue, struct QueueItemInner *inner) {
    struct QueueNode *head = queue->head;
    if (head != NULL && head->field_4->field_8 == inner) {
        return 1;
    }
    return 0;
}

// FUNCTION: LEGOLAND 0x00411fa0
void FUN_00411fa0(struct Queue *queue, int param_2, int param_3, struct QueueItemInner *inner) {
    struct QueueTable *table = queue->count;
    struct QueueStep *step = &table->steps[inner->field_38];
    struct QueueNode *node;
    short cur;
    char dir;
    int x;
    int y;

    x = (step->dx + param_2) << 8;
    y = (step->dy + param_3) << 8;
    inner->field_24 = x;
    inner->field_28 = y;
    dir = CalcMoveLine(*(struct Point *)&inner->field_68, *(struct Point *)&inner->field_24, &inner->field_98);
    inner->field_e = 7;
    inner->field_73 = dir + 0x10;
    NewDirForAction((Bloke *)inner, (unsigned char)(inner->field_73 >> 5) + 3);
    inner->field_38++;
    cur = inner->field_38;
    if (cur >= table->count) {
        inner->field_38 = (short)(table->count - 1);
        return;
    }
    for (node = queue->head; node != NULL; node = node->next) {
        struct QueueItemInner *other = node->field_4->field_8;
        if (other->field_38 == cur && other != inner) {
            inner->field_38 = cur - 1;
            return;
        }
    }
}

// FUNCTION: LEGOLAND 0x00412060
void FUN_00412060(struct Queue *queue, struct QueueItemMid **out) {
    struct QueueNode *node = queue->head;
    if (node != NULL) {
        struct QueueItemInner *inner = node->field_4->field_8;
        inner->field_62 &= ~0x40;
        inner->field_60++;
        *out = node->field_4;
        QueueUnlinkHead(queue);
        free(node);
    }
}

// FUNCTION: LEGOLAND 0x004120a0
void FUN_004120a0(struct Queue *queue, unsigned int param_2, unsigned int param_3) {
    struct QueueNode *node = queue->head;
    while (node != NULL) {
        struct QueueItemInner *inner = node->field_4->field_8;
        if (inner->field_e == 0) {
            FUN_00411fa0(queue, param_2, param_3, inner);
        }
        node = node->next;
    }
}

// FUNCTION: LEGOLAND 0x00412100
int FUN_00412100(struct PathTable *param_1) {
    int total = 0;
    int px;
    int py;
    int i;
    struct QueueTable *table;
    int k;
    int j;
    int sx;
    int sy;
    int dx;
    int dy;
    int len;
    int cx;
    int cy;

    for (i = 0; i < param_1->count; i++) {
        int x = param_1->pairs[i].a;
        int y = param_1->pairs[i].b;
        total += (int)sqrt((double)(y * y + x * x));
    }
    table = (struct QueueTable *)malloc(total * 12 + 8);
    if (table != NULL) {
        memset(table, 0, total * 12 + 8);
        table->count = total;
        table->steps = (struct QueueStep *)(table + 1);
    }
    sx = 0;
    sy = 0;
    k = 0;
    for (i = 0; i < param_1->count; i++) {
        struct PathPair *p = &param_1->pairs[i];
        dx = p->a;
        dy = p->b;
        len = (int)sqrt((double)(dy * dy + dx * dx));
        px = 0;
        py = 0;
        cx = sx;
        cy = sy;
        for (j = len; j > 0; j--) {
            table->steps[k].dx = cx;
            table->steps[k].dy = cy;
            k++;
            cx = sx + px / len;
            cy = sy + py / len;
            px += dx;
            py += dy;
        }

        sx += dx;
        sy += dy;
    }
    return (int)table;
}

// FUNCTION: LEGOLAND 0x00412290
void FreeIfNotNull(void *param_1) {
    if (param_1 != NULL) {
        free(param_1);
    }
}

// FUNCTION: LEGOLAND 0x004122a0
void FUN_004122a0(struct RideSlotArg *param_1, struct RideSlot *slot) {
    slot->field_50 = param_1;
    slot->field_38 = param_1->field_0 - 1;
    slot->field_34 = 0xff;
    slot->field_60++;
}

// FUNCTION: LEGOLAND 0x004122d0
void FUN_004122d0(struct RideSlotArg *param_1, struct RideSlot *slot) {
    slot->field_50 = param_1;
    slot->field_38 = 0;
    slot->field_34 = 1;
    slot->field_60++;
}

// FUNCTION: LEGOLAND 0x004122f0
struct RideSlotArg *FUN_004122f0(struct RideSlot *slot) {
    return slot->field_50;
}

// FUNCTION: LEGOLAND 0x00412300
void FUN_00412300(struct QueueTable *table, int x, int y, struct Bloke *bloke) {
    struct Point p;
    char dir;

    p.x = x + table->steps[bloke->field_38].dx;
    p.y = table->steps[bloke->field_38].dy + y;
    bloke->dest.x = p.x << 8;
    bloke->dest.y = p.y << 8;
    dir = CalcMoveLine(bloke->pos, bloke->dest, &bloke->nav);
    bloke->field_e = 7;
    bloke->field_73 = dir + 0x10;
    NewDirForAction(bloke, (unsigned char)(bloke->field_73 >> 5) + 3);
    bloke->field_38 += (signed char)bloke->field_34;
    if ((signed char)bloke->field_34 < 0) {
        if (bloke->field_38 < 0) {
            bloke->param_action++;
        }
    } else if (bloke->field_38 >= table->count) {
        bloke->param_action++;
    }
}

// FUNCTION: LEGOLAND 0x004123a0
unsigned int FUN_004123a0(struct QueueNode *start, struct QueueNode *stop) {
    unsigned int count = 0;
    struct QueueNode *current = start;
    if (current != NULL) {
        while (1) {
            if (current == stop) {
                break;
            }
            current = current->next;
            count++;
            if (current == NULL) {
                break;
            }
        }
    }
    return count;
}

// FUNCTION: LEGOLAND 0x004123c0
void FUN_004123c0(struct QueueNode *start, struct Queue *queue) {
    struct QueueNode *node;
    int n;

    SaveGameWrite(queue->count, 4);
    for (n = 0; n < queue->count->count; n++) {
        SaveGameWrite(&queue->count->steps[n], 12);
    }
    n = 0;
    for (node = queue->head; node != NULL; node = node->next) {
        n++;
    }
    SaveGameWrite(&n, 4);
    for (node = queue->head; node != NULL; node = node->next) {
        n = FUN_004123a0(start, (struct QueueNode *)node->field_4);
        SaveGameWrite(&n, 4);
    }
}

// FUNCTION: LEGOLAND 0x00412470
struct QueueNode *GetNthNextQueueNode(struct QueueNode *node, int n) {
    int i = n;
    while (i-- != 0) {
        node = node->next;
    }
    return node;
}

// FUNCTION: LEGOLAND 0x00412490
void FUN_00412490(struct QueueNode *start, struct Queue *queue) {
    int n;
    int idx;

    SaveGameRead(&n, 4);
    queue->count = (struct QueueTable *)malloc(n * 12 + 8);
    queue->count->count = n;
    queue->count->steps = (struct QueueStep *)(queue->count + 1);
    for (n = 0; n < queue->count->count; n++) {
        SaveGameRead(&queue->count->steps[n], 12);
    }
    queue->head = NULL;
    queue->tail = NULL;
    SaveGameRead(&n, 4);
    while (n-- != 0) {
        if (queue->tail == NULL) {
            queue->tail = (struct QueueNode *)malloc(8);
            queue->head = queue->tail;
        } else {
            queue->tail->next = (struct QueueNode *)malloc(8);
            queue->tail = queue->tail->next;
        }
        queue->tail->next = NULL;
        SaveGameRead(&idx, 4);
        queue->tail->field_4 = (struct QueueItemMid *)GetNthNextQueueNode(start, idx);
    }
}

// FUNCTION: LEGOLAND 0x004125a0
struct RideQueueEntry *FUN_004125a0(int x, int y) {
    struct RideQueueEntry *entry = DAT_004cbeac;
    if (x < 0) {
        return NULL;
    }
    if (x >= (unsigned short)lpConfig->width) {
        return NULL;
    }
    if (y < 0) {
        return NULL;
    }
    if (y >= (unsigned short)lpConfig->height) {
        return NULL;
    }
    if (entry == NULL) {
        return NULL;
    }
    while (entry != NULL) {
        if (entry->x == x && entry->y == y) {
            return entry;
        }
        entry = entry->next;
    }
    return NULL;
}

// FUNCTION: LEGOLAND 0x004125f0
void *FUN_004125f0(unsigned int a, unsigned int b) {
    struct RideQueueEntry *entry = DAT_004cbeac;
    int x = a;
    int y = b;
    if (x < 0) {
        return NULL;
    }
    if (x >= (unsigned short)lpConfig->width) {
        return NULL;
    }
    if (y < 0) {
        return NULL;
    }
    if (y >= (unsigned short)lpConfig->height) {
        return NULL;
    }
    if (entry == NULL) {
        return NULL;
    }
    while (entry != NULL) {
        if ((unsigned int)(x - entry->x) < 4 && (unsigned int)(y - entry->y) < 4) {
            return entry;
        }
        entry = entry->next;
    }
    return NULL;
}

// FUNCTION: LEGOLAND 0x00412650
struct RideQueueEntry *FUN_00412650(unsigned short param_1) {
    struct RideQueueEntry *entry = DAT_004cbeac;
    if (entry == NULL) {
        return NULL;
    }
    while (entry != NULL) {
        if (entry->field_8 == param_1 && (entry->field_14 & 0xf) == 6) {
            return entry;
        }
        entry = entry->next;
    }
    return NULL;
}

// FUNCTION: LEGOLAND 0x00412680
void FUN_00412680(int x, int y, int param_3, int param_4) {
    unsigned short tiles[16] = {0};
    int dm2 = (param_4 - 2) & 3;
    int dm1 = (param_4 - 1) & 3;
    int dp1 = (param_4 + 1) & 3;
    struct RideQueueEntry *entry;
    struct MapElement *tile;
    struct Point pos;
    TileId id;
    int i;
    struct FXSpriteList *set1;
    struct FXSpriteList *set2;
    int col;
    int row;

    entry = FUN_004125a0(x, y);
    if (entry == NULL) {
        return;
    }
    BGFullUpdate = 1;
    entry->field_14 = ((param_4 & 3) << 5) | param_3;
    id.pos.x = x;
    id.pos.y = y;
    pos.x = x;
    for (i = 0; i < 4; i++) {
        pos.y = i + y;
        tile = TileAt(pos.x, pos.y);
        tile[0].field_10 = 2;
        tile[0].flags |= 8;
        tile[0].field_0 = ((struct RQObjClass *)DAT_0082c684)->field_c4;
        tile[0].anchor = id;
        tile[1].field_10 = 2;
        tile[1].flags |= 8;
        tile[1].field_0 = ((struct RQObjClass *)DAT_0082c684)->field_c4;
        tile[1].anchor = id;
        tile[2].field_10 = 2;
        tile[2].flags |= 8;
        tile[2].field_0 = ((struct RQObjClass *)DAT_0082c684)->field_c4;
        tile[2].anchor = id;
        tile[3].field_10 = 2;
        tile[3].flags |= 8;
        tile[3].field_0 = ((struct RQObjClass *)DAT_0082c684)->field_c4;
        tile[3].anchor = id;
        RemovePathSquare(&pos);
        pos.x++;
        RemovePathSquare(&pos);
        pos.x++;
        RemovePathSquare(&pos);
        pos.x++;
        RemovePathSquare(&pos);
        pos.x = x;
    }

    switch (param_3 & 0xf) {
    case 1:
        tiles[0] = DAT_004b4c08[2][param_4];
        tiles[1] = DAT_004b4c08[4][param_4];
        tiles[2] = DAT_004b4c08[5][param_4];
        tiles[3] = DAT_004b4c08[3][param_4];
        tiles[4] = DAT_004b4c08[0][param_4];
        tiles[5] = DAT_004b4c08[1][param_4];
        tiles[6] = DAT_004b4c08[1][dm2];
        tiles[7] = DAT_004b4c08[0][dm2];
        tiles[8] = DAT_004b4c08[0][param_4];
        tiles[9] = DAT_004b4c08[1][param_4];
        tiles[10] = DAT_004b4c08[1][dm2];
        tiles[11] = DAT_004b4c08[0][dm2];
        tiles[12] = DAT_004b4c08[0][param_4];
        tiles[13] = DAT_004b4c08[1][param_4];
        tiles[14] = DAT_004b4c08[1][dm2];
        tiles[15] = DAT_004b4c08[0][dm2];
        break;
    case 7:
        tiles[0] = DAT_004b4c08[2][param_4];
        tiles[1] = DAT_004b4c08[4][param_4];
        tiles[2] = DAT_004b4c08[5][param_4];
        tiles[3] = DAT_004b4c08[3][param_4];
        tiles[4] = DAT_004b4c08[0][param_4];
        tiles[5] = DAT_004b4c08[1][param_4];
        tiles[6] = DAT_004b4c08[1][dm2];
        tiles[7] = DAT_004b4c08[0][dm2];
        tiles[8] = DAT_004b4c08[0][param_4];
        tiles[9] = DAT_004b4c08[1][param_4];
        tiles[10] = DAT_004b4c08[1][dm2];
        tiles[11] = DAT_004b4c08[0][dm2];
        tiles[12] = DAT_004b4c08[3][dm2];
        tiles[13] = DAT_004b4c08[5][dm2];
        tiles[14] = DAT_004b4c08[4][dm2];
        tiles[15] = DAT_004b4c08[2][dm2];
        break;
    case 3:
        tiles[0] = DAT_004b4c08[14][param_4];
        tiles[1] = DAT_004b4c08[0][dp1];
        tiles[2] = DAT_004b4c08[0][dp1];
        tiles[3] = DAT_004b4c08[0][dp1];
        tiles[4] = DAT_004b4c08[0][param_4];
        tiles[5] = DAT_004b4c08[11][param_4];
        tiles[6] = DAT_004b4c08[12][param_4];
        tiles[7] = DAT_004b4c08[8][dp1];
        tiles[8] = DAT_004b4c08[0][param_4];
        tiles[9] = DAT_004b4c08[13][param_4];
        tiles[10] = DAT_004b4c08[10][param_4];
        tiles[11] = DAT_004b4c08[8][dm1];
        tiles[12] = DAT_004b4c08[0][param_4];
        tiles[13] = DAT_004b4c08[8][param_4];
        tiles[14] = DAT_004b4c08[8][dm2];
        tiles[15] = DAT_004b4c08[9][param_4];
        break;
    case 4:
        tiles[0] = DAT_004b4c08[9][dm2];
        tiles[1] = DAT_004b4c08[11][param_4];
        tiles[2] = DAT_004b4c08[11][dm2];
        tiles[3] = DAT_004b4c08[9][dm1];
        tiles[4] = DAT_004b4c08[1][dp1];
        tiles[5] = DAT_004b4c08[1][dp1];
        tiles[6] = DAT_004b4c08[1][dp1];
        tiles[7] = DAT_004b4c08[1][dp1];
        tiles[8] = DAT_004b4c08[1][dm1];
        tiles[9] = DAT_004b4c08[1][dm1];
        tiles[10] = DAT_004b4c08[1][dm1];
        tiles[11] = DAT_004b4c08[1][dm1];
        tiles[12] = DAT_004b4c08[0][dm1];
        tiles[13] = DAT_004b4c08[0][dm1];
        tiles[14] = DAT_004b4c08[0][dm1];
        tiles[15] = DAT_004b4c08[0][dm1];
        break;
    case 5:
        tiles[0] = DAT_004b4c08[9][dm2];
        tiles[1] = DAT_004b4c08[11][param_4];
        tiles[2] = DAT_004b4c08[11][param_4];
        tiles[3] = DAT_004b4c08[9][dm1];
        tiles[4] = DAT_004b4c08[11][param_4];
        tiles[5] = DAT_004b4c08[11][param_4];
        tiles[6] = DAT_004b4c08[11][param_4];
        tiles[7] = DAT_004b4c08[11][param_4];
        tiles[8] = DAT_004b4c08[11][param_4];
        tiles[9] = DAT_004b4c08[11][param_4];
        tiles[10] = DAT_004b4c08[11][param_4];
        tiles[11] = DAT_004b4c08[11][param_4];
        tiles[12] = DAT_004b4c08[9][dp1];
        tiles[13] = DAT_004b4c08[11][param_4];
        tiles[14] = DAT_004b4c08[11][param_4];
        tiles[15] = DAT_004b4c08[9][param_4];
        break;
    case 6:
        tiles[0] = DAT_004b4c08[2][param_4];
        tiles[1] = DAT_004b4c08[4][param_4];
        tiles[2] = DAT_004b4c08[5][param_4];
        tiles[3] = DAT_004b4c08[3][param_4];
        tiles[4] = DAT_004b4c08[0][param_4];
        tiles[5] = DAT_004b4c08[1][param_4];
        tiles[6] = DAT_004b4c08[1][dm2];
        tiles[7] = DAT_004b4c08[0][dm2];
        tiles[8] = DAT_004b4c08[0][param_4];
        tiles[9] = DAT_004b4c08[1][param_4];
        tiles[10] = DAT_004b4c08[1][dm2];
        tiles[11] = DAT_004b4c08[0][dm2];
        tiles[12] = DAT_004b4c08[14][dm1];
        tiles[13] = DAT_004b4c08[0][dm1];
        tiles[14] = DAT_004b4c08[0][dm1];
        tiles[15] = DAT_004b4c08[0][dm1];
        break;
    case 2:
    default:
        tiles[0] = DAT_004b4c08[0][param_4];
        tiles[1] = DAT_004b4c08[1][param_4];
        tiles[2] = DAT_004b4c08[1][dm2];
        tiles[3] = DAT_004b4c08[0][dm2];
        tiles[4] = DAT_004b4c08[0][param_4];
        tiles[5] = DAT_004b4c08[1][param_4];
        tiles[6] = DAT_004b4c08[1][dm2];
        tiles[7] = DAT_004b4c08[0][dm2];
        tiles[8] = DAT_004b4c08[0][param_4];
        tiles[9] = DAT_004b4c08[1][param_4];
        tiles[10] = DAT_004b4c08[1][dm2];
        tiles[11] = DAT_004b4c08[0][dm2];
        tiles[12] = DAT_004b4c08[0][param_4];
        tiles[13] = DAT_004b4c08[1][param_4];
        tiles[14] = DAT_004b4c08[1][dm2];
        tiles[15] = DAT_004b4c08[0][dm2];
        break;
    }

    if (param_3 & 0x10) {
        if (param_4 < 2) {
            tiles[4] = DAT_004b4c08[6][param_4];
            tiles[5] = DAT_004b4c08[7][param_4];
            tiles[6] = DAT_004b4c08[7][param_4];
            tiles[7] = DAT_004b4c08[6][dm2];
        } else {
            tiles[8] = DAT_004b4c08[6][param_4];
            tiles[9] = DAT_004b4c08[7][param_4];
            tiles[10] = DAT_004b4c08[7][param_4];
            tiles[11] = DAT_004b4c08[6][dm2];
        }
    }

    switch (param_4) {
    case 0:
        for (col = 0; col < 4; col++) {
            for (row = 0; row < 4; row++) {
                SetMapTile(x + col, y + row, DSchoolMappingData[tiles[row * 4 + col] >> 8].base[0] + (tiles[row * 4 + col] & 0xff));
            }
        }
        break;
    case 1:
        for (col = 0; col < 4; col++) {
            for (row = 0; row < 4; row++) {
                SetMapTile(x + col, y + row, DSchoolMappingData[tiles[(3 - col) * 4 + row] >> 8].base[0] + (tiles[(3 - col) * 4 + row] & 0xff));
            }
        }
        break;
    case 2:
        for (col = 0; col < 4; col++) {
            for (row = 0; row < 4; row++) {
                SetMapTile(x + col, y + row, DSchoolMappingData[tiles[(3 - row) * 4 + (3 - col)] >> 8].base[0] + (tiles[(3 - row) * 4 + (3 - col)] & 0xff));
            }
        }
        break;
    case 3:
        for (col = 0; col < 4; col++) {
            for (row = 0; row < 4; row++) {
                SetMapTile(x + col, y + row, DSchoolMappingData[tiles[col * 4 + (3 - row)] >> 8].base[0] + (tiles[col * 4 + (3 - row)] & 0xff));
            }
        }
        break;
    }

    pos.x = x;
    pos.y = y + 1;
    tile = TileAt(pos.x, pos.y);
    set1 = TileSpriteInfo[tile[1].field_8].src;
    if (tile[1].field_8 - *(int *)set1 == 0x1a) {
        AdjustTileRFFlags((int *)&pos);
        AddPathSquare(&pos);
        tile[0].field_10 = 3;
        pos.x++;
        AdjustTileRFFlags((int *)&pos);
        AddPathSquare(&pos);
        tile[1].field_10 = 3;
        pos.x++;
        AdjustTileRFFlags((int *)&pos);
        AddPathSquare(&pos);
        tile[2].field_10 = 3;
        pos.x++;
        AdjustTileRFFlags((int *)&pos);
        AddPathSquare(&pos);
        tile[3].field_10 = 3;
    } else if (set2 = TileSpriteInfo[tile[2].field_8].src, tile[2].field_8 - *(int *)set2 == 0x1b) {
        pos.x = x + 2;
        pos.y--;
        AdjustTileRFFlags((int *)&pos);
        AddPathSquare(&pos);
        TileAt(pos.x, pos.y)->field_10 = 3;
        pos.y++;
        AdjustTileRFFlags((int *)&pos);
        AddPathSquare(&pos);
        TileAt(pos.x, pos.y)->field_10 = 3;
        pos.y++;
        AdjustTileRFFlags((int *)&pos);
        AddPathSquare(&pos);
        TileAt(pos.x, pos.y)->field_10 = 3;
        pos.y++;
        AdjustTileRFFlags((int *)&pos);
        AddPathSquare(&pos);
        TileAt(pos.x, pos.y)->field_10 = 3;
    }

    pos.x = x - 1;
    pos.y = y + 1;
    tile = TileAt(pos.x, pos.y);
    if (tile != NULL && (tile->field_10 & 1)) {
        AdjustTileRFFlags((int *)&pos);
    }
    pos.x = x + 4;
    pos.y = y + 1;
    tile = TileAt(pos.x, pos.y);
    if (tile != NULL && (tile->field_10 & 1)) {
        AdjustTileRFFlags((int *)&pos);
    }
    pos.x = x + 2;
    pos.y = y - 1;
    tile = TileAt(pos.x, pos.y);
    if (tile != NULL && (tile->field_10 & 1)) {
        AdjustTileRFFlags((int *)&pos);
    }
    pos.x = x + 2;
    pos.y = y + 4;
    tile = TileAt(pos.x, pos.y);
    if (tile != NULL && (tile->field_10 & 1)) {
        AdjustTileRFFlags((int *)&pos);
    }
}
