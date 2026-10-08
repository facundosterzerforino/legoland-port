#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "bloke.h"
#include "bricks.h"
#include "debug_alloc.h"
#include "globals.h"
#include "legoland.h"
#include "math.h"

#include "bricks.h"
#include "clipping.h"
#include "gamemap.h"
#include "llidb.h"
#include "log_flume.h"
#include "man3d.h"
#include "map_object.h"
#include "objclass.h"
#include "print_sprite.h"
#include "render3d.h"
#include "ride_queue.h"
#include "tilemap.h"

struct Sprite;

#include "image_sprite.h"

struct EmitNode {
    unsigned char pad_0[0xc];
    struct EmitNode *next;
};

struct Particle {
    int x;
    int y;
    struct EmitNode *node;
    float scale;
    int z;
    float w;
    int a;
    int b;
    int c;
};

struct ParticleEmitter {
    unsigned char pad_0[8];
    struct EmitNode *head;
    unsigned char pad_c[0x30];
    unsigned int var_3c;
    unsigned char pad_40[0xd0 - 0x40];
    unsigned int var_d0;
};

struct CursorSource {
    unsigned char pad_0[0x3c];
    unsigned int var_3c[5];
};

struct FlumeOut {
    int kind;
    unsigned char pad_4[0xc - 0x4];
    int f_0c;
    int f_10;
    unsigned char pad_14[0x1c - 0x14];
    int f_1c;
    int f_20;
};

struct Obj {
    unsigned char pad_0[0xc];
    void *ride;
};

struct ResA {
    unsigned char pad_0[0x1c];
    unsigned int flags_1c;
    unsigned char pad_20[0x64 - 0x20];
    struct ResB *ptr_64;
};

struct ResB {
    unsigned char pad_0[0x10];
    unsigned int flags_10;
};

struct WalkNode {
    struct WalkNode *var_0;
    unsigned int var_4;
    struct WalkNode *var_8;
};

struct FlumeRideSrc {
    unsigned char pad_0[0x14];
    unsigned int field_14;
    unsigned int field_18;
    unsigned char pad_1c[0x64 - 0x1c];
    void *field_64;
};

struct FlumeRideArg {
    unsigned char pad_0[0xc];
    struct FlumeRideSrc *field_c;
};

struct PairHolder {
    unsigned char pad_0[0x2c];
    unsigned int var_2c;
    struct StateNode *var_30;
    struct StateNode *var_34;
};

struct ModeNode {
    unsigned char pad_0[0x18];
    unsigned int flag_18;
    unsigned int mode_1c;
};

struct EdgeNode {
    unsigned char pad_0[0x1c];
    unsigned int mode_1c;
    unsigned char pad_20[0x10];
    int a_30;
    int b_34;
};

struct SubBuf {
    unsigned char b0;
    unsigned char b1;
};

struct LinkNode {
    unsigned int field_0;
    unsigned int field_4;
    struct Context *field_8;
    struct Context *field_c;
};

struct Context {
    unsigned char pad_0[0x2c];
    unsigned int var_2c;
    struct Context *var_30;
    struct Context *var_34;
};

struct LinkList {
    struct LinkNode *field_0;
    struct LinkNode *field_4;
    struct LinkNode *field_8;
    struct LinkNode *field_c;
};

struct InputBuffer {
    unsigned char flags;
    unsigned char pad_0[3];
    unsigned char var_4;
    unsigned char pad_4[3];
    unsigned char var_8;
    unsigned char pad_8[3];
    unsigned char var_c;
    unsigned char pad_c[3];
    unsigned char var_10;
    unsigned char pad_10[3];
    unsigned char var_14;
    unsigned char pad_14[3];
    unsigned char var_18;
    unsigned char pad_18[3];
    unsigned char var_1c;
    unsigned char pad_1c[3];
    unsigned char var_20;
};

struct Node {
    unsigned char pad_0[8];
    struct Node *next;
    struct Node *prev;
    struct ListNode *head_10;
    unsigned char pad_14[0x24 - 0x14];
    struct Node *head_24;
    unsigned char pad_28[0x2c - 0x28];
    struct ListNode *tail_2c;
};

struct ListNode {
    struct ListNode *prev;
    struct ListNode *next;
    struct ListNode *up;
    struct ListNode *down;
};

struct Slot {
    unsigned char pad_0[0x24];
    unsigned int key;
};

struct StateNode {
    unsigned char pad_0[0x18];
    unsigned int state;
    unsigned int phase;
};

struct StateSlots {
    struct StateNode *slot0;
    struct StateNode *slot1;
    struct StateNode *slot2;
    struct StateNode *slot3;
};

struct FlumeXY {
    unsigned char x;
    unsigned char y;
};

typedef void (*FlumeCallback)(struct FlumeXY tile, unsigned int *res);

struct FlumePos {
    unsigned char pad_0[0x30];
    unsigned int var_30;
    unsigned int var_34;
};

struct FlumeRect {
    unsigned int var_0;
    unsigned int var_4;
    unsigned int var_8;
    unsigned int var_c;
};

struct FlumeSlot0 {
    unsigned int field_0;
    unsigned char pad_4[4];
    unsigned int var_8;
};

struct FlumeSlot1 {
    unsigned int field_0;
    unsigned char pad_4[4];
    unsigned int field_8;
    unsigned int var_c;
};

struct FlumeInput {
    struct FlumeSlot0 *var_0;
    unsigned char pad_4[4];
    struct FlumeSlot1 *var_8;
};

struct FlumeNode {
    struct FlumeNode *next;
    struct FlumeNode *prev;
    unsigned int field_8;
    unsigned int field_c;
    unsigned int flags;
    TileId tile;
    unsigned char pad_16[2];
    int mode;
    int submode;
    struct Ride *ride;
    struct FlumeEntry *owner;
    struct FlumeNode *entry;
    struct FlumeNode *field_2c;
    unsigned int field_30;
    unsigned int field_34;
};

struct FlumeBytes {
    unsigned char b0;
    unsigned char pad_1[3];
    unsigned char b4;
};

struct FlumeWeighted {
    unsigned char pad_0[8];
    struct FlumeWeighted *a;
    struct FlumeWeighted *b;
    unsigned char pad_10[4];
    struct FlumeWeighted *link;
    float weight;
};

struct FlumeCounter {
    unsigned char pad_0[0x60];
    unsigned char count;
};

struct FlumeStatHolder {
    unsigned char pad_0[8];
    struct FlumeCounter *counter;
};

struct FlumeSlot {
    int timer;
    unsigned int flags;
    struct FlumeStatHolder *busy;
    int x;
    int y;
    struct FlumeWeighted *owner;
    float weight;
    int field_1c;
    unsigned char pad_20[4];
};

struct FlumeQueueItem {
    unsigned char pad_0[8];
    struct FlumeSlot *slot;
};

struct FlumeSlotSet {
    unsigned char pad_0[4];
    unsigned int flags;
    union {
        struct FlumeWeighted *inner;
        struct FlumeInner *inner3;
    };
    struct FlumeWeighted *field_c;
    unsigned char pad_10[4];
    TileId tile;
    unsigned char pad_16[6];
    int dir;
    int timer;
    int ticks;
    int limit;
    unsigned int queue[3];
    struct FlumeSlot *target;
    int count;
    struct FlumeSlot slots[1];
};

struct FlumeRunNode {
    struct FlumeRunNode *prev;
    struct FlumeRunNode *next;
};

struct FlumeStageList {
    unsigned char pad_0[4];
    unsigned char *stages;
};

struct FlumeNode;

struct FlumeMover {
    unsigned char pad_0[4];
    unsigned int flags;
    unsigned char pad_8[4];
    int x;
    int y;
    struct FlumeNode *node;
    float f18;
    int i1c;
    float f20;
    int i24;
};

struct FlumeInner {
    unsigned char pad_0[8];
    int id;
};

struct FlumeEntry {
    /* 0x00 */ struct FlumeEntry *next;
    /* 0x04 */ union {
        struct FlumeEntry *alt;
        unsigned int flags;
    };
    /* 0x08 */ union {
        struct FlumeEntry *parent8;
        void *field_8;
    };
    /* 0x0c */ void *field_c;
    /* 0x10 */ union {
        struct FlumeEntry *sub;
        unsigned char flags10;
        unsigned int flags10d;
    };
    /* 0x14 */ TileId tile;
    /* 0x16 */ unsigned char pad_16[2];
    /* 0x18 */ union {
        int mode;
        struct FlumeEntry *link;
    };
    /* 0x1c */ int submode;
    /* 0x20 */ struct Ride *ride;
    /* 0x24 */ union {
        struct FlumeSlotSet *slotset;
        struct FlumeEntry *parent;
        struct FlumeMover *field_24;
    };
    /* 0x28 */ union {
        int field_28;
        struct FlumeEntry *link28;
    };
    /* 0x2c */ union {
        struct {
            struct FlumeEntry *sub2;
            union {
                struct FlumeRunNode *first;
                struct FlumeEntry *link30;
            };
            union {
                struct FlumeRunNode *last;
                struct FlumeEntry *link34;
            };
        };
        struct Queue queue;
    };
    /* 0x38 */ void *target;
    /* 0x3c */ int count;
    /* 0x40 */ struct FlumeSlot slots[4];
    /* 0xd0 */ int field_d0;
};

struct FlumeHolder {
    unsigned char pad_0[0x14];
    struct FlumeEntry *entry;
};

// GLOBAL: LEGOLAND 0x004b4798
static struct PathPair Flume_PathPairs0[4] = {{0, -1}, {3, 0}, {0, -2}, {-4, 0}};
// GLOBAL: LEGOLAND 0x004b47b8
struct PathTable DAT_004b47b8 = {4, Flume_PathPairs0};
// GLOBAL: LEGOLAND 0x004b47c0
static struct PathPair Flume_PathPairs1[5] = {{0, 0}, {0, 1}, {3, 0}, {0, 3}, {-4, 0}};
// GLOBAL: LEGOLAND 0x004b47e8
struct PathTable DAT_004b47e8 = {5, Flume_PathPairs1};

// GLOBAL: LEGOLAND 0x004b4768
const char *LogFlumeTrackSpriteFiles[10] = {
    // STRING: LEGOLAND 0x004b48d4
    "fc1a_m.lls",
    // STRING: LEGOLAND 0x004b48c8
    "fc2a_m.lls",
    // STRING: LEGOLAND 0x004b48bc
    "fc3a_m.lls",
    // STRING: LEGOLAND 0x004b48b0
    "fc4a_m.lls",
    // STRING: LEGOLAND 0x004b48a4
    "fs1_m.lls",
    // STRING: LEGOLAND 0x004b4898
    "fs2_m.lls",
    // STRING: LEGOLAND 0x004b488c
    "fe2_m.lls",
    // STRING: LEGOLAND 0x004b4880
    "fe3_m.lls",
    // STRING: LEGOLAND 0x004b4874
    "fe4_m.lls",
    // STRING: LEGOLAND 0x004b4868
    "fe1_m.lls",
};

// FUNCTION: LEGOLAND 0x00408e40
void FUN_00408e40(TileId tile) {
    struct FlumeEntry *entry = malloc(sizeof(struct FlumeEntry));
    if (entry != NULL) {
        memset(entry, 0, sizeof(struct FlumeEntry));
        entry->tile = tile;
        entry->next = FlumeEntryList;
        FlumeEntryList = entry;
    }
}

// FUNCTION: LEGOLAND 0x00408e80
void FUN_00408e80(struct FlumeEntry *entry) {
    struct FlumeEntry *prev;
    struct FlumeEntry *cur;

    if (FlumeEntryList == entry) {
        FlumeEntryList = entry->next;
    } else {
        cur = FlumeEntryList->next;
        prev = FlumeEntryList;
        while (cur != entry) {
            prev = prev->next;
            if (prev == NULL) {
                break;
            }
            cur = prev->next;
        }
        if (prev != NULL) {
            prev->next = entry->next;
        }
    }
    free(entry);
}

// FUNCTION: LEGOLAND 0x00408ec0
struct FlumeEntry *FindFlumeEntryByTile(TileId *tile) {
    struct FlumeEntry *cur = FlumeEntryList;
    if (cur == NULL) {
        return NULL;
    }
    while (memcmp(&cur->tile, tile, sizeof(TileId)) != 0) {
        cur = cur->next;
        if (cur == NULL) {
            return NULL;
        }
    }
    return cur;
}

// FUNCTION: LEGOLAND 0x00408ef0
struct FlumeEntry *FindFlumeSubEntryByTile(TileId *tile) {
    struct FlumeEntry *outer = FlumeEntryList;
    struct FlumeEntry *cur;

    if (outer == NULL) {
        return NULL;
    }
    do {
        cur = outer->sub;
        while (cur != NULL) {
            if (memcmp(&cur->tile, tile, sizeof(TileId)) == 0) {
                return cur;
            }
            cur = cur->next;
        }
        outer = outer->next;
    } while (outer != NULL);
    return NULL;
}

// FUNCTION: LEGOLAND 0x00408f30
unsigned int FUN_00408f30(struct SubBuf *buf) {
    struct FlumeEntry *outer;
    struct FlumeEntry *mid;
    struct FlumeEntry *cur;

    outer = FlumeEntryList;
    if (FlumeEntryList != NULL) {
        for (; outer != NULL; outer = outer->next) {
            for (mid = outer->sub; mid != NULL; mid = mid->next) {
                cur = mid->sub2;
                if (cur == NULL) {
                    if (memcmp(&mid->tile, buf, sizeof(TileId)) == 0) {
                        return (unsigned int)mid;
                    }
                } else {
                    do {
                        if (memcmp(&cur->tile, buf, sizeof(TileId)) == 0) {
                            return (unsigned int)cur;
                        }
                        cur = cur->next;
                    } while (cur != NULL);
                }
            }
        }
    }
    return 0;
}

// FUNCTION: LEGOLAND 0x00408f90
struct FlumeEntry *FUN_00408f90(int x, int y) {
    struct FlumeEntry *outer;
    struct FlumeEntry *cur;
    int h = LogFlumeFootprint.y1 - LogFlumeFootprint.y0;
    int w = LogFlumeFootprint.x1 - LogFlumeFootprint.x0;

    outer = FlumeEntryList;
    while (outer != NULL) {
        cur = outer->sub;
        while (cur != NULL) {
            if (x >= cur->tile.pos.x && x <= cur->tile.pos.x + w && y >= cur->tile.pos.y && y <= cur->tile.pos.y + h) {
                return cur;
            }
            cur = cur->next;
        }
        outer = outer->next;
    }
    return NULL;
}

// FUNCTION: LEGOLAND 0x00409010
void *FUN_00409010(void) {
    void *block = malloc(0x38);
    if (block != NULL) {
        memset(block, 0, 0x38);
    }
    return block;
}

// FUNCTION: LEGOLAND 0x00409040
void InsertNodeBefore(struct Node *node, struct Node *insert) {
    if (node->prev != NULL) {
        node->prev->next = insert;
    }
    insert->next = node;
    if (node->prev != NULL) {
        // STRING: LEGOLAND 0x004b48e0
        printf("bug");
    }
    node->prev = insert;
}

// FUNCTION: LEGOLAND 0x00409080
void LinkNodeAfter(struct Node *node, struct Node *insert) {
    if (node->next != NULL) {
        node->next->prev = insert;
    }
    insert->prev = node;
    if (node->next != NULL) {
        printf("bug");
    }
    node->next = insert;
}

// FUNCTION: LEGOLAND 0x004090c0
void InsertNodeBetween(struct Node *node1, struct Node *node2, struct Node *node3) {
    node3->prev = node1;
    node3->next = node2;
    node1->next = node3;
    node2->prev = node3;
}

// FUNCTION: LEGOLAND 0x004090e0
struct Node *FUN_004090e0(struct Node *node) {
    struct Node *start = node;

    while (node->prev != NULL) {
        node = node->prev;
        if (node == start) {
            return start;
        }
        if (node == NULL) {
            return NULL;
        }
    }
    return node;
}

// FUNCTION: LEGOLAND 0x00409110
void ReverseNodeList(struct Node *container) {
    struct Node *node = FUN_004090e0(container);
    struct Node *next;
    struct Node *prev;
    if (node != NULL) {
        do {
            next = node->next;
            prev = node->prev;
            node->next = prev;
            node->prev = next;
            node = next;
        } while (next != NULL);
    }
}

// FUNCTION: LEGOLAND 0x00409140
int FUN_00409140(struct Node *container) {
    struct Node *node = FUN_004090e0(container);
    struct Node *head;
    if (node == NULL) {
        return 0;
    }
    head = container->head_24->next;
    while (node != NULL) {
        if (node == head) {
            return 1;
        }
        node = node->next;
    }
    return 0;
}

// FUNCTION: LEGOLAND 0x00409170
void AppendListNode(struct Node *container, struct ListNode *node) {
    node->prev = container->tail_2c;
    node->next = NULL;
    if (container->tail_2c != NULL) {
        container->tail_2c->next = node;
    }
    container->tail_2c = node;
}

// FUNCTION: LEGOLAND 0x004091a0
void FUN_004091a0(struct Node *container, struct ListNode *node) {
    if (node->next != NULL) {
        node->next->prev = node->prev;
    }
    if (node->prev != NULL) {
        node->prev->next = node->next;
    }
    if (node == container->tail_2c) {
        container->tail_2c = node->prev;
    }
    if (node->down != NULL) {
        node->down->up = NULL;
    }
    if (node->up != NULL) {
        node->up->down = NULL;
    }
}

// FUNCTION: LEGOLAND 0x004091f0
void FUN_004091f0(struct Node *container, struct ListNode *node) {
    struct ListNode *head = container->head_10;
    node->next = NULL;
    node->prev = head;
    head = container->head_10;
    if (head != NULL) {
        head->next = node;
    }
    container->head_10 = node;
}

// FUNCTION: LEGOLAND 0x00409220
void FUN_00409220(struct Node *container, struct ListNode *node) {
    if (node->next != NULL) {
        node->next->prev = node->prev;
    }
    if (node->prev != NULL) {
        node->prev->next = node->next;
    }
    if (node == container->head_10) {
        container->head_10 = node->prev;
    }
    if (node->down != NULL) {
        node->down->up = NULL;
    }
    if (node->up != NULL) {
        node->up->down = NULL;
    }
}

// FUNCTION: LEGOLAND 0x00409270
void FUN_00409270(struct Node *other, struct Node *container) {
    struct ListNode *node = container->tail_2c;
    struct ListNode *prev;
    while (node != NULL) {
        prev = node->prev;
        FUN_004091a0(container, node);
        free(node);
        node = prev;
    }
    FUN_00409220(other, (struct ListNode *)container);
    free(container);
}

// FUNCTION: LEGOLAND 0x004092b0
int FUN_004092b0(struct FlumeHolder *holder) {
    struct FlumeEntry *e = holder->entry;
    int mode = e->mode;
    TileId t;
    int dx;
    int dy;

    if (mode > 0) {
        if (mode > 2) {
            if (mode == 3) {
                switch (e->submode) {
                case 0:
                    return 1;
                case 1:
                    return 3;
                case 2:
                    return 5;
                case 3:
                    return 7;
                }
            }
        } else {
            t = e->parent8->tile;
            dx = t.pos.x - e->tile.pos.x;
            dy = t.pos.y - e->tile.pos.y;
            if (dx < 0) {
                return 7;
            } else if (dx > 0) {
                return 3;
            } else if (dy >= 0 && dy > 0) {
                return 5;
            }
        }
    }
    return 1;
}

// FUNCTION: LEGOLAND 0x00409360
void FUN_00409360(TileId tile) {
    struct SubBuf buf;
    int w = LogFlumeFootprint.x1 - LogFlumeFootprint.x0;
    int h = LogFlumeFootprint.y1 - LogFlumeFootprint.y0;

    buf.b0 = tile.pos.x;
    buf.b1 = tile.pos.y - h;
    DAT_004cbe20 = FUN_00408f30(&buf);
    buf.b0 = tile.pos.x + w;
    buf.b1 = tile.pos.y;
    DAT_004cbe24 = FUN_00408f30(&buf);
    buf.b0 = tile.pos.x;
    buf.b1 = tile.pos.y + h;
    DAT_004cbe28 = FUN_00408f30(&buf);
    buf.b0 = tile.pos.x - w;
    buf.b1 = tile.pos.y;
    DAT_004cbe2c = FUN_00408f30(&buf);
}

// FUNCTION: LEGOLAND 0x00409410
int FUN_00409410(unsigned int *ctx) {
    int result = 0;
    if (ctx[0] != 0) {
        result = 1;
    }
    if (ctx[1] != 0) {
        result |= 0x4;
    }
    if (ctx[2] != 0) {
        result |= 0x10;
    }
    if (ctx[3] != 0) {
        result |= 0x40;
    }
    return result;
}

// FUNCTION: LEGOLAND 0x00409440
void FUN_00409440(TileId param_1, void **param_2) {
    DAT_004cbe20 = 0;
    DAT_004cbe24 = 0;
    DAT_004cbe28 = 0;
    DAT_004cbe2c = 0;
    FUN_00409360(param_1);
    *param_2 = &DAT_004cbe20;
}

// FUNCTION: LEGOLAND 0x00409470
unsigned int FUN_00409470(unsigned int *list) {
    unsigned int result = 0;
    if (DAT_004cbe20 != 0) {
        result = 1;
    }
    if (DAT_004cbe24 != 0) {
        result++;
    }
    if (DAT_004cbe28 != 0) {
        result++;
    }
    if (DAT_004cbe2c != 0) {
        result++;
    }
    return result;
}

// FUNCTION: LEGOLAND 0x004094b0
void FUN_004094b0(unsigned int arg, struct Slot **slots) {
    if (slots[0] != NULL && slots[0]->key != arg) {
        slots[0] = NULL;
    }
    if (slots[1] != NULL && slots[1]->key != arg) {
        slots[1] = NULL;
    }
    if (slots[2] != NULL && slots[2]->key != arg) {
        slots[2] = NULL;
    }
    if (slots[3] != NULL && slots[3]->key != arg) {
        slots[3] = NULL;
    }
}

// FUNCTION: LEGOLAND 0x00409510
void FUN_00409510(struct StateSlots *slots) {
    if (slots->slot0 != NULL && slots->slot0->state != 3 && slots->slot0->state != 4) {
        slots->slot0 = NULL;
    }
    if (slots->slot1 != NULL && slots->slot1->state != 3 && slots->slot1->state != 4) {
        slots->slot1 = NULL;
    }
    if (slots->slot2 != NULL && slots->slot2->state != 3 && slots->slot2->state != 4) {
        slots->slot2 = NULL;
    }
    if (slots->slot3 != NULL && slots->slot3->state != 3 && slots->slot3->state != 4) {
        slots->slot3 = NULL;
    }
}

// FUNCTION: LEGOLAND 0x00409580
int FUN_00409580(int code, struct StateSlots *slots) {
    int result = 0;
    if (code == 1 || code == 4 || code == 0x10 || code == 0x40 || code == 0x11 || code == 0x44 || code == 5 || code == 0x14 || code == 0x50 || code == 0x41) {
        result = 1;
        if (slots->slot0 != NULL && slots->slot0->state != 3 && slots->slot0->state != 4) {
            result = 0;
        }
        if (slots->slot1 != NULL && slots->slot1->state != 3 && slots->slot1->state != 4) {
            result = 0;
        }
        if (slots->slot2 != NULL && slots->slot2->state != 3 && slots->slot2->state != 4) {
            result = 0;
        }
        if (slots->slot3 != NULL && slots->slot3->state != 3 && slots->slot3->state != 4) {
            result = 0;
        }
    }
    return result;
}

// FUNCTION: LEGOLAND 0x00409620
void FUN_00409620(struct StateNode *node) {
    if (node->state == 3) {
        if (node->phase == 2) {
            node->state = 1;
            node->phase = 0;
            return;
        }
        if (node->phase == 1) {
            node->state = 2;
            node->phase = 2;
            return;
        }
        if (node->phase == 3) {
            node->state = 2;
            node->phase = 1;
            return;
        }
    }
    if (node->state == 4) {
        node->state = 3;
        node->phase = 0;
    }
}

// FUNCTION: LEGOLAND 0x00409680
void FUN_00409680(struct StateNode *node) {
    if (node->state == 3) {
        if (node->phase == 0) {
            node->state = 1;
            node->phase = 0;
            return;
        }
        if (node->phase == 1) {
            node->state = 2;
            node->phase = 3;
            return;
        }
        if (node->phase == 3) {
            node->state = 2;
            node->phase = 0;
            return;
        }
    }
    if (node->state == 4) {
        node->state = 3;
        node->phase = 2;
    }
}

// FUNCTION: LEGOLAND 0x004096e0
void FUN_004096e0(struct StateNode *node) {
    if (node->state == 3) {
        if (node->phase == node->state) {
            node->state = 1;
            node->phase = 1;
            return;
        }
        if (node->phase == 0) {
            node->state = 2;
            node->phase = 2;
            return;
        }
        if (node->phase == 2) {
            node->state = node->phase;
            node->phase = 3;
            return;
        }
    }
    if (node->state == 4) {
        node->state = 3;
        node->phase = 1;
    }
}

// FUNCTION: LEGOLAND 0x00409740
void FUN_00409740(struct StateNode *node) {
    if (node->state == 3) {
        if (node->phase == 1) {
            node->state = 1;
            node->phase = 1;
            return;
        }
        if (node->phase == 0) {
            node->state = 2;
            node->phase = 1;
            return;
        }
        if (node->phase == 2) {
            node->state = 2;
            node->phase = 0;
            return;
        }
    }
    if (node->state == 4) {
        node->state = 3;
        node->phase = 3;
    }
}

// FUNCTION: LEGOLAND 0x004097a0
void FUN_004097a0(struct FlumeEntry *entry, struct StateSlots *slots) {
    int mask = FUN_00409410((unsigned int *)slots);
    struct FlumeEntry *sub;

    if (entry != NULL) {
        sub = entry->sub2;
        if (sub == NULL) {
            switch (mask) {
            case 0x01:
            case 0x04:
            case 0x10:
            case 0x40:
                entry->mode = 3;
                break;
            case 0x11:
            case 0x44:
                entry->mode = 1;
                break;
            case 0x05:
            case 0x14:
            case 0x41:
            case 0x50:
                entry->mode = 2;
                break;
            }
        }
        switch (mask) {
        case 0x01:
            if (sub == NULL) {
                entry->submode = 2;
            }
            FUN_00409620(slots->slot0);
            break;
        case 0x10:
            if (sub == NULL) {
                entry->submode = 0;
            }
            FUN_00409680(slots->slot2);
            break;
        case 0x04:
            if (sub == NULL) {
                entry->submode = 3;
            }
            FUN_004096e0(slots->slot1);
            break;
        case 0x40:
            if (sub == NULL) {
                entry->submode = 1;
            }
            FUN_00409740(slots->slot3);
            break;
        case 0x11:
            if (sub == NULL) {
                entry->submode = 0;
            }
            FUN_00409620(slots->slot0);
            FUN_00409680(slots->slot2);
            break;
        case 0x44:
            if (sub == NULL) {
                entry->submode = 1;
            }
            FUN_004096e0(slots->slot1);
            FUN_00409740(slots->slot3);
            break;
        case 0x05:
            if (sub == NULL) {
                entry->submode = 0;
            }
            FUN_00409620(slots->slot0);
            FUN_004096e0(slots->slot1);
            break;
        case 0x14:
            if (sub == NULL) {
                entry->submode = 1;
            }
            FUN_004096e0(slots->slot1);
            FUN_00409680(slots->slot2);
            break;
        case 0x50:
            if (sub == NULL) {
                entry->submode = 2;
            }
            FUN_00409680(slots->slot2);
            FUN_00409740(slots->slot3);
            break;
        case 0x41:
            if (sub == NULL) {
                entry->submode = 3;
            }
            FUN_00409740(slots->slot3);
            FUN_00409620(slots->slot0);
            break;
        }
    }
}

// FUNCTION: LEGOLAND 0x00409a50
void FUN_00409a50(struct ModeNode *node) {
    switch (node->mode_1c) {
    case 0:
    case 2:
        node->mode_1c = 0;
        break;
    case 1:
    case 3:
        node->mode_1c = 1;
        break;
    }
    node->flag_18 = 1;
}

// FUNCTION: LEGOLAND 0x00409a90
void FUN_00409a90(void *edi_ptr[4], struct StateNode *esi_ptr[4]) {
    int flags = FUN_00409410((unsigned int *)esi_ptr);
    if (flags & 0x1) {
        FUN_00409a50(edi_ptr[0]);
        FUN_00409620(esi_ptr[0]);
    }
    if (flags & 0x4) {
        FUN_00409a50(edi_ptr[1]);
        FUN_004096e0(esi_ptr[1]);
    }
    if (flags & 0x10) {
        FUN_00409a50(edi_ptr[2]);
        FUN_00409680(esi_ptr[2]);
    }
    if (flags & 0x40) {
        FUN_00409a50(edi_ptr[3]);
        FUN_00409740(esi_ptr[3]);
    }
}

// FUNCTION: LEGOLAND 0x00409b10
void FUN_00409b10(struct Node *a, struct Node *b) {
    int a_ok = FUN_00409140(a);
    int b_ok = FUN_00409140(b);

    if (a_ok != 0 && b_ok != 0) {
        // STRING: LEGOLAND 0x004b48e4
        printf("both pieces are joined to log flume - error");
    } else if (a_ok == 0 && b_ok == 0) {
        ReverseNodeList(a);
    } else if (a_ok != 0 && b_ok == 0) {
        ReverseNodeList(b);
    } else {
        ReverseNodeList(a);
    }
}

// FUNCTION: LEGOLAND 0x00409b70
void JoinOppositeFlumeNodes(int i, int j, struct Node **nodes, struct Node *insert) {
    int a_ok;
    int b_ok;

    // STRING: LEGOLAND 0x004b4910
    printf("going opposite directions!!!!");
    a_ok = FUN_00409140(nodes[i]);
    b_ok = FUN_00409140(nodes[j]);
    if (a_ok != 0 && b_ok != 0) {
        // STRING: LEGOLAND 0x004b48e4
        printf("both pieces are joined to log flume - error");
        return;
    } else if (a_ok == 0 && b_ok == 0) {
        ReverseNodeList(nodes[i]);
    } else if (a_ok != 0 && b_ok == 0) {
        ReverseNodeList(nodes[j]);
    } else {
        ReverseNodeList(nodes[i]);
    }
    if (nodes[i]->next == NULL && nodes[j]->prev == NULL) {
        InsertNodeBetween(nodes[i], nodes[j], insert);
    } else {
        InsertNodeBetween(nodes[j], nodes[i], insert);
    }
}

// FUNCTION: LEGOLAND 0x00409c20
void FUN_00409c20(struct FlumeEntry *entry, unsigned int *list) {
    struct Node *insert = (struct Node *)entry;
    struct Node **nodes = (struct Node **)list;
    struct Node *a;

    switch (FUN_00409410(list)) {
    case 0x01:
        a = nodes[0];
        if (a->prev == NULL) {
            InsertNodeBefore(a, insert);
        } else {
            LinkNodeAfter(a, insert);
        }
        break;
    case 0x10:
        a = nodes[2];
        if (a->prev == NULL) {
            InsertNodeBefore(a, insert);
        } else {
            LinkNodeAfter(a, insert);
        }
        break;
    case 0x04:
        a = nodes[1];
        if (a->prev == NULL) {
            InsertNodeBefore(a, insert);
        } else {
            LinkNodeAfter(a, insert);
        }
        break;
    case 0x40:
        a = nodes[3];
        if (a->prev == NULL) {
            InsertNodeBefore(a, insert);
        } else {
            LinkNodeAfter(a, insert);
        }
        break;
    case 0x11:
        a = nodes[0];
        if ((a->next != NULL && nodes[2]->next != NULL && a->prev == NULL && nodes[2]->prev == NULL) || (a->next == NULL && nodes[2]->next == NULL && a->prev != NULL && nodes[2]->prev != NULL)) {
            JoinOppositeFlumeNodes(0, 2, nodes, insert);
            return;
        }
        if (a->next == NULL && nodes[2]->prev == NULL) {
            InsertNodeBetween(a, nodes[2], insert);
        } else {
            InsertNodeBetween(nodes[2], a, insert);
        }
        return;
    case 0x44:
        a = nodes[1];
        if ((a->next != NULL && nodes[3]->next != NULL && a->prev == NULL && nodes[3]->prev == NULL) || (a->next == NULL && nodes[3]->next == NULL && a->prev != NULL && nodes[3]->prev != NULL)) {
            JoinOppositeFlumeNodes(1, 3, nodes, insert);
            return;
        }
        if (a->prev == NULL && nodes[3]->next == NULL) {
            InsertNodeBetween(nodes[3], a, insert);
        } else {
            InsertNodeBetween(a, nodes[3], insert);
        }
        return;
    case 0x05:
        a = nodes[0];
        if ((a->next != NULL && nodes[1]->next != NULL && a->prev == NULL && nodes[1]->prev == NULL) || (a->next == NULL && nodes[1]->next == NULL && a->prev != NULL && nodes[1]->prev != NULL)) {
            JoinOppositeFlumeNodes(0, 1, nodes, insert);
            return;
        }
        if (nodes[1]->prev == NULL && a->next == NULL) {
            InsertNodeBetween(a, nodes[1], insert);
        } else {
            InsertNodeBetween(nodes[1], a, insert);
        }
        return;
    case 0x14:
        a = nodes[1];
        if ((a->next != NULL && nodes[2]->next != NULL && a->prev == NULL && nodes[2]->prev == NULL) || (a->next == NULL && nodes[2]->next == NULL && a->prev != NULL && nodes[2]->prev != NULL)) {
            JoinOppositeFlumeNodes(1, 2, nodes, insert);
            return;
        }
        if (nodes[2]->prev == NULL && a->next == NULL) {
            InsertNodeBetween(a, nodes[2], insert);
        } else {
            InsertNodeBetween(nodes[2], a, insert);
        }
        return;
    case 0x50:
        a = nodes[2];
        if ((a->next != NULL && nodes[3]->next != NULL && a->prev == NULL && nodes[3]->prev == NULL) || (a->next == NULL && nodes[3]->next == NULL && a->prev != NULL && nodes[3]->prev != NULL)) {
            JoinOppositeFlumeNodes(2, 3, nodes, insert);
            return;
        }
        if (nodes[3]->prev == NULL && a->next == NULL) {
            InsertNodeBetween(a, nodes[3], insert);
        } else {
            InsertNodeBetween(nodes[3], a, insert);
        }
        return;
    case 0x41:
        a = nodes[3];
        if ((a->next != NULL && nodes[0]->next != NULL && a->prev == NULL && nodes[0]->prev == NULL) || (a->next != NULL && nodes[0]->next != NULL && a->prev == NULL && nodes[0]->prev == NULL)) {
            JoinOppositeFlumeNodes(3, 0, nodes, insert);
            return;
        }
        if (nodes[0]->prev == NULL && a->next == NULL) {
            InsertNodeBetween(a, nodes[0], insert);
        } else {
            InsertNodeBetween(nodes[0], a, insert);
        }
        return;
    }
}

// FUNCTION: LEGOLAND 0x0040a010
void FUN_0040a010(struct Node *a, struct Node *b) {
    if ((a->next != NULL && b->next != NULL && a->prev == NULL && b->prev == NULL) ||
        (a->next == NULL && b->next == NULL && a->prev != NULL && b->prev != NULL)) {
        FUN_00409b10(a, b);
    }
    if (a->next == NULL) {
        LinkNodeAfter(a, b);
    } else {
        InsertNodeBefore(a, b);
    }
}

// FUNCTION: LEGOLAND 0x0040a080
void FUN_0040a080(struct Node **b, struct Node **a) {
    int flags = FUN_00409410((unsigned int *)a);

    if (flags & 1) {
        FUN_0040a010(a[0], b[0]);
    }
    if (flags & 4) {
        FUN_0040a010(a[1], b[1]);
    }
    if (flags & 0x10) {
        FUN_0040a010(a[2], b[2]);
    }
    if (flags & 0x40) {
        FUN_0040a010(a[3], b[3]);
    }
}

// FUNCTION: LEGOLAND 0x0040a0f0
void FUN_0040a0f0(struct StateNode *node) {
    if (node == NULL) {
        return;
    }
    if (node->state == 3 && node->phase == 0) {
        node->state = 4;
        return;
    }
    if (node->state == 1 && node->phase == 0) {
        node->state = 3;
        node->phase = 2;
        return;
    }
    if (node->state == 2) {
        if (node->phase == 2) {
            node->state = 3;
            node->phase = 1;
            return;
        } else if (node->phase == 1) {
            node->state = 3;
            node->phase = 3;
        }
    }
}

// FUNCTION: LEGOLAND 0x0040a160
void FUN_0040a160(struct StateNode *node) {
    if (node != NULL) {
        if (node->state == 3 && node->phase == 1) {
            node->state = 4;
        } else if (node->state == 1 && node->phase == 1) {
            node->state = 3;
            node->phase = 3;
        } else if (node->state == 2) {
            if (node->phase == 3) {
                node->state = 3;
                node->phase = 2;
            } else if (node->phase == 2) {
                node->state = 3;
                node->phase = 0;
            }
        }
    }
}

// FUNCTION: LEGOLAND 0x0040a1d0
void FUN_0040a1d0(struct StateNode *node) {
    if (node == NULL) {
        return;
    }
    if (node->state == 3 && node->phase == 2) {
        node->state = 4;
        return;
    }
    if (node->state == 1 && node->phase == 0) {
        node->state = 3;
        node->phase = 0;
        return;
    }
    if (node->state == 2) {
        if (node->phase == 3) {
            node->state = 3;
            node->phase = 1;
            return;
        }
        if (node->phase == 0) {
            node->state = 3;
            node->phase = 3;
            return;
        }
    }
}

// FUNCTION: LEGOLAND 0x0040a230
void FUN_0040a230(struct StateNode *node) {
    if (node == NULL) {
        return;
    }
    if (node->state == 3 && node->phase == 3) {
        node->state = 4;
    } else if (node->state == 1 && node->phase == 1) {
        node->state = 3;
        node->phase = 1;
    } else if (node->state == 2) {
        if (node->phase == 0) {
            node->state = 3;
            node->phase = 2;
        } else if (node->phase == 1) {
            node->state = 3;
            node->phase = 0;
        }
    }
}

// FUNCTION: LEGOLAND 0x0040a2a0
void FUN_0040a2a0(void *other, struct StateNode **ctx) {
    FUN_00409410((unsigned int *)ctx);
    if (other != NULL) {
        FUN_0040a0f0(ctx[0]);
        FUN_0040a160(ctx[1]);
        FUN_0040a1d0(ctx[2]);
        FUN_0040a230(ctx[3]);
    }
}

// FUNCTION: LEGOLAND 0x0040a2e0
void LogFlumeEntranceLoad(Element *elem) {
    struct LLS *lls;

    LogFlumeEntranceRide = elem->ride;
    LogFlumeEntranceRide->flags |= 0x20;
    LogFlumeEntranceLayer = LogFlumeEntranceRide->layer;
    if (LogFlumeEntranceLayer != NULL) {
        LogFlumeEntranceLayer->flags |= 0x2000;
    }
    DAT_004c2ae8 = (void *)FUN_00412100(&DAT_004b47b8);
    DAT_004c2af8 = (void *)FUN_00412100(&DAT_004b47e8);
    // STRING: LEGOLAND 0x004b49c8
    LogFlumeBarrelSprite = LoadSprite("lf_barrel.lls", 1);
    // STRING: LEGOLAND 0x004b49b8
    LogFlumeBarrelMSprite = LoadSprite("lf_barrel_m.lls", 1);
    // STRING: LEGOLAND 0x004b49a8
    LogFlumeBarrel1Sprite = LoadSprite("lf_barrel1.lls", 1);
    // STRING: LEGOLAND 0x004b4998
    LogFlumeBarrelMatteSprite = LoadSprite("barrelmatte.lls", 1);
    if (LogFlumeBarrelSprite != NULL) {
        lls = (struct LLS *)GetLLSForSprite((struct SpriteLLS *)LogFlumeBarrelSprite);
        if (lls != NULL) {
            LLSPlay(lls, (unsigned int)LogFlumeBarrelSprite->image);
        }
    }
    // STRING: LEGOLAND 0x004b4984
    LogFlumeEnta1Matte2Sprite = LoadSprite("lf_enta1_matte2.lls", 1);
    // STRING: LEGOLAND 0x004b4978
    LogFlumeSignSprite = LoadSprite("lf_sign.lls", 1);
    // STRING: LEGOLAND 0x004b4964
    LogFlumeEntrance1Sprite = LoadSprite("lf_entrance1.lls", 1);
    // STRING: LEGOLAND 0x004b4950
    LogFlumeEntrance2Sprite = LoadSprite("lf_entrance2.lls", 1);
    // STRING: LEGOLAND 0x004b493c
    LogFlumeEntrance3Sprite = LoadSprite("lf_entrance3.lls", 1);
    // STRING: LEGOLAND 0x004b4930
    LogFlumeEnta3MSprite = LoadSprite("enta3_m.lls", 1);
}

// FUNCTION: LEGOLAND 0x0040a410
void LogFlumeEntranceUnload(void) {
    struct FlumeEntry *current;

    if (LogFlumeEnta3MSprite) {
        KillSprite(LogFlumeEnta3MSprite);
    }
    if (LogFlumeEntrance3Sprite) {
        KillSprite(LogFlumeEntrance3Sprite);
    }
    if (LogFlumeEntrance2Sprite) {
        KillSprite(LogFlumeEntrance2Sprite);
    }
    if (LogFlumeEntrance1Sprite) {
        KillSprite(LogFlumeEntrance1Sprite);
    }
    if (LogFlumeSignSprite) {
        KillSprite(LogFlumeSignSprite);
    }
    if (LogFlumeEnta1Matte2Sprite) {
        KillSprite(LogFlumeEnta1Matte2Sprite);
    }
    if (LogFlumeBarrelSprite) {
        KillSprite(LogFlumeBarrelSprite);
    }
    if (LogFlumeBarrelMSprite) {
        KillSprite(LogFlumeBarrelMSprite);
    }
    if (DAT_004c2af8) {
        FreeIfNotNull(DAT_004c2af8);
    }
    if (DAT_004c2ae8) {
        FreeIfNotNull(DAT_004c2ae8);
    }
    if (LogFlumeBarrel1Sprite) {
        KillSprite(LogFlumeBarrel1Sprite);
    }
    if (LogFlumeBarrelMatteSprite) {
        KillSprite(LogFlumeBarrelMatteSprite);
    }

    current = FlumeEntryList;
    if (current != NULL) {
        while (current != NULL) {
            struct FlumeEntry *next = current->next;
            struct FlumeEntry *chain = current->sub;
            while (chain != NULL) {
                struct FlumeEntry *next_chain = chain->next;
                free(chain);
                chain = next_chain;
            }
            free(current);
            current = next;
        }
        FlumeEntryList = current;
        return;
    }
    FlumeEntryList = NULL;
}

// FUNCTION: LEGOLAND 0x0040a540
void LogFlumeEntranceSetEditMode(void) {
    EditMode.unk0 = 1;
    EditMode.unk8 = LogFlumeEntranceRide;
    DefaultCursor(&EditCursor);
    SetEditCursorFootPrint(&EditMode.unk8->footprint);
}

// FUNCTION: LEGOLAND 0x0040a580
void FUN_0040a580(struct ParticleEmitter *param) {
    int i;
    struct Particle *p;
    struct EmitNode *node;
    struct FlumeDims d;

    node = param->head;
    i = 0;
    if ((int)param->var_3c > 0) {
        p = (struct Particle *)(param->pad_40 + 0xc);
        do {
            p->node = node;
            d = FUN_004112c0();
            p->x = d.field1;
            p->y = d.field2;
            p->z = 0;
            p->scale = 1.0f;
            p->w = 0.1f;
            p[-1].a = 0;
            p[-1].b = 0;
            node = node->next;
            i++;
            p++;
        } while (i < (int)param->var_3c);
    }
}

// FUNCTION: LEGOLAND 0x0040a5d0
void FUN_0040a5d0(struct ParticleEmitter *param) {
    if (param != NULL) {
        param->var_3c = 4;
        param->var_d0 = 6;
        FUN_0040a580(param);
    }
}

// FUNCTION: LEGOLAND 0x0040a600
void LogFlumeEntranceAddObject(Element *elem, int *pt) {
    TileId t;
    register struct FlumeEntry *entry;
    struct FlumeEntry *mid;
    struct FlumeEntry *node;
    struct FlumeEntry *prev;
    register struct Ride *ride;
    int pos[2];
    int coords[2];
    int h;
    int midY;
    register int last;
    int count;
    int i;

    int fy0;
    int fy1;
    int node_y;
    int fx0;
    t.pos.x = pt[0];
    t.pos.y = pt[1];
    ride = elem->ride;
    AddBasicObject(elem, pt);
    FUN_00408e40(t);
    entry = FindFlumeEntryByTile(&t);
    if (entry != NULL) {
        fy1 = LogFlumeFootprint.y1;
        h = fy1 - LogFlumeFootprint.y0;
        midY = t.pos.y + ride->footprint.y0 + ((ride->footprint.y1 - ride->footprint.y0) >> 1);
        pos[0] = t.pos.x + ride->footprint.x0 + 1;
        entry->sub2 = DAT_004c2ae8;
        pos[1] = t.pos.y + ride->footprint.y0 - h;
        node = FUN_00409010();
        if (NULL != node) {
            node->mode = 3;
            node->submode = 0;
            node->ride = LogFlumeTrackRide;
            node->parent = entry;
            node->link28 = NULL;
            node->flags10d |= 3;
            fx0 = LogFlumeFootprint.x0;
            node->tile.pos.x = pos[0] - fx0;
            node->tile.pos.y = pos[1] - LogFlumeFootprint.y0;
        }
        FUN_004091f0((struct Node *)entry, (struct ListNode *)node);
        coords[0] = node->tile.pos.x;
        coords[1] = node->tile.pos.y;
        memcpy(&LogFlumeTrackRide->footprint, &LogFlumeFootprint, sizeof(struct Footprint));
        LogFlumeTrackRide->footprint.x1--;
        LogFlumeTrackRide->footprint.y1--;
        AddBasicObject(DAT_004c74f4, coords);
        mid = FUN_00409010();
        mid->parent = entry;
        mid->ride = LogFlumeEntranceRide;
        mid->tile = entry->tile;
        FUN_004091f0((struct Node *)entry, (struct ListNode *)mid);
        entry->link = mid;
        LinkNodeAfter((struct Node *)node, (struct Node *)mid);
        count = (ride->footprint.y1 - ride->footprint.y0 + 1) / h;
        prev = node;
        coords[1] = pos[1] + h;
        last = count - 1;
        for (i = 0; i < count; i++) {
            node = FUN_00409010();
            if (node != NULL) {
                node->mode = 1;
                node->submode = 0;
                node->link28 = mid;
                node->ride = LogFlumeTrackRide;
                node->parent = entry;
                node->flags10d = node->flags10d | 7;
                node->tile.pos.x = pos[0] - LogFlumeFootprint.x0;
                fy0 = LogFlumeFootprint.y0;
                node->tile.pos.y = coords[1] - fy0;
            }
            if (coords[1] <= midY) {
                if (coords[1] + h >= midY) {
                    entry->field_8 = node;
                    entry->field_c = prev;
                }
            }
            AppendListNode((struct Node *)mid, (struct ListNode *)node);
            if (0 == i) {
                mid->link30 = node;
            }
            if (i == last) {
                mid->link34 = node;
            }
            LinkNodeAfter((struct Node *)prev, (struct Node *)node);
            prev = node;
            coords[1] += h;
        }
        coords[0] = t.pos.x + ride->footprint.x0 + 1;
        coords[1] = t.pos.y + ride->footprint.y1 - 1 + h;
        node = FUN_00409010();
        if (node != NULL) {
            node->mode = 3;
            node->submode = 2;
            node->flags10d |= 3;
            node->link28 = NULL;
            node->ride = LogFlumeTrackRide;
            node->parent = entry;
            node->tile.pos.x = coords[0] - LogFlumeFootprint.x0;
            node->tile.pos.y = coords[1] - LogFlumeFootprint.y0;
        }
        FUN_004091f0((struct Node *)entry, (struct ListNode *)node);
        node_y = node->tile.pos.y;
        coords[0] = node->tile.pos.x;
        coords[1] = node_y;
        AddBasicObject(DAT_004c74f4, coords);
        LinkNodeAfter((struct Node *)prev, (struct Node *)node);
        FUN_0040a5d0((struct ParticleEmitter *)entry);
    }
}

// FUNCTION: LEGOLAND 0x0040a930
void LogFlumeEntranceCalcCursor(Element *elem, int *param_2, unsigned int param_3) {
    struct Ride *ride;
    register int x;
    int h;
    unsigned int y;

    unsigned int fy0;
    int fx0;
    int fy1;
    ride = elem->ride;
    fy1 = LogFlumeFootprint.y1;
    h = fy1 - LogFlumeFootprint.y0;

    ScreenToMapRef(param_2, &EditCursor.tile_x, param_3);
    DefaultCursor(&EditCursor);
    SetEditCursorFootPrint(&ride->footprint);
    PathCursor.tile_x = EditCursor.tile_x;
    PathCursor.tile_y = EditCursor.tile_y;
    PathCursor.footprint.y0 = EditCursor.footprint.y0 - 1;
    PathCursor.footprint.x0 = EditCursor.footprint.x0 + 3;
    EditCursor.next = &DAT_004c8d78;
    DAT_004c8d78.next = &DAT_004c2c18;
    DAT_004c2c18.next = &PathCursor;
    PathCursor.next = NULL;
    PathCursor.footprint.x1 = EditCursor.footprint.x1 + 1;
    PathCursor.footprint.y1 = 1 + EditCursor.footprint.y1;
    PathCursor.footprint.next = NULL;
    PathCursor.field_1828 = 0x1000;
    FUN_0045f460(&PathCursor);
    memcpy(DAT_004c8d78.footprint.v, &LogFlumeFootprint, 20);
    DAT_004c8d78.footprint.x1 = LogFlumeFootprint.x1 - 1;
    DAT_004c8d78.footprint.y1 = DAT_004c8d78.footprint.y1 - 1;
    FUN_0045f460(&DAT_004c8d78);
    y = EditCursor.tile_y;
    x = EditCursor.tile_x;
    fx0 = LogFlumeEntranceRide->footprint.x0;
    DAT_004c8d78.tile_x = fx0 + x;
    fy0 = LogFlumeEntranceRide->footprint.y0;
    DAT_004c8d78.tile_y = fy0 + y - h;
    DAT_004c8d78.tile_x++;
    DAT_004c2c18.tile_x = LogFlumeEntranceRide->footprint.x0 + x;
    DAT_004c2c18.tile_y = LogFlumeEntranceRide->footprint.y1 + y - 1 + h;
    DAT_004c2c18.tile_x++;
    memcpy(DAT_004c2c18.footprint.v, DAT_004c8d78.footprint.v, 20);
    FUN_0045f460(&DAT_004c2c18);
    ValidateCursor(&EditCursor, (unsigned int)ride);
    FUN_0045f4d0(&EditCursor);
}

// FUNCTION: LEGOLAND 0x0040aac0
void LogFlumeEntranceDCalcCursor(unsigned int param_1, struct Point *param_2) {
    struct FlumeEntry *entry;
    struct FlumeEntry *cur;

    BasicObjectDCalcCursor(param_1, param_2);
    DefaultCursor(&LogFlumeCursor);
    entry = FindFlumeEntryByTile(&QueryObj);
    if (entry != NULL) {
        for (cur = entry->sub; cur != NULL; cur = cur->next) {
            if (cur->field_28 != -1) {
                FUN_0040d090(cur, &param_1, &param_2);
                if (param_1 == 0) {
                    continue;
                }
                memcpy(LogFlumeCursor.footprint.v, (void *)param_1, 20);
                LogFlumeCursor.tile_x = cur->tile.pos.x;
                LogFlumeCursor.tile_y = cur->tile.pos.y;
            }
            LogFlumeCursor.field_1828 = 0x18;
            FUN_0045f460(&LogFlumeCursor);
            BuildCursorPtr(&LogFlumeCursor, 0, 0);
            RenderCursor(&LogFlumeCursor);
        }
        PathCursor.tile_x = LogFlumeCursor.tile_x;
        PathCursor.tile_y = LogFlumeCursor.tile_y;
        PathCursor.footprint.y0 = LogFlumeCursor.footprint.y0 - 1;
        PathCursor.footprint.x0 = LogFlumeCursor.footprint.x0 + 3;
        PathCursor.footprint.x1 = LogFlumeCursor.footprint.x1 + 1;
        PathCursor.footprint.y1 = LogFlumeCursor.footprint.y1 + 1;
        PathCursor.footprint.next = NULL;
        PathCursor.field_1828 = 0x1000;
        PathCursor.next = LogFlumeCursor.next;
        LogFlumeCursor.next = &PathCursor;
    }
}

// FUNCTION: LEGOLAND 0x0040abf0
void LogFlumeEntranceRemoveObject(Element *obj, TileId tile, struct Cursor *cursor_arg) {
    struct Cursor cursor;
    struct FlumeEntry *entry;
    struct FlumeEntry *cur;
    struct FlumeEntry *next;

    StandardRemoveObject(obj, tile, cursor_arg);
    entry = FindFlumeEntryByTile(&tile);
    if (entry != NULL) {
        cur = entry->sub;
        if (cur != NULL) {
            do {
                next = cur->next;
                memcpy(&LogFlumeTrackRide->footprint, &LogFlumeFootprint, 20);
                LogFlumeTrackRide->footprint.x1--;
                LogFlumeTrackRide->footprint.y1--;
                cursor.tile_x = cur->tile.pos.x + LogFlumeFootprint.x0;
                cursor.tile_y = cur->tile.pos.y + LogFlumeFootprint.x1;
                memcpy(cursor.footprint.v, &LogFlumeTrackRide->footprint, 20);
                StandardRemoveObject(LogFlumeTrackRide->element, cur->tile, &cursor);
                if (cur->flags10 & 2) {
                    UseBricks(GetObjCost(LogFlumeTrackRide));
                }
                FUN_00409270((struct Node *)entry, (struct Node *)cur);
                cur = next;
            } while (next != NULL);
        }
        FreeAllQueueNodes((struct Queue *)&entry->sub2);
        FUN_00408e80(entry);
        LogFlumeTrackRide->field_8++;
    }
    RemoveAllBlokesFromRide(obj->ride, tile);
}

// FUNCTION: LEGOLAND 0x0040ad50
int FUN_0040ad50(struct StateNode *node) {
    int result = 0;

    if (node != NULL) {
        switch (node->state) {
        case 1:
            return DAT_004b474c[node->phase];
        case 2:
            return DAT_004b4754[node->phase];
        case 3:
            return DAT_004b473c[node->phase];
        case 4:
            result = DAT_004b4764;
        }
    }
    return result;
}

// FUNCTION: LEGOLAND 0x0040adb0
int FUN_0040adb0(TileId tile, struct FlumeRect *rect, int param_3, int y, float scale) {
    int bounds[4];
    struct Point pt;
    int top;
    float f;
    int x0;
    int y0;
    int x1;
    int y1;

    x0 = rect->var_0 + tile.pos.x;
    x1 = rect->var_8 + tile.pos.x;
    y1 = rect->var_c + tile.pos.y;
    y0 = rect->var_4 + tile.pos.y;
    pt.x = x0;
    pt.y = y0;
    GetTileBounds(&pt, bounds);
    top = bounds[1];
    pt.x = x1;
    pt.y = y1;
    GetTileBounds(&pt, bounds);
    f = (float)(y - top) / ((float)(bounds[3] - top + 1) * scale);
    if (f < FLOAT_004ab390) {
        f = FLOAT_004ab390;
    } else if (f > 1.0f) {
        f = 1.0f;
    }
    return 32 - (int)(f * -192.0f);
}

// FUNCTION: LEGOLAND 0x0040ae90
void FUN_0040ae90(unsigned int param_1, int param_2, int param_3) {
    struct FlumeSlot *slot = (struct FlumeSlot *)param_1;
    struct FlumeEntry *other = (struct FlumeEntry *)param_2;
    struct FlumeEntry *owner = (struct FlumeEntry *)slot->owner;
    struct Sprite *spriteA;
    struct Sprite *spriteB;
    RECT saved;
    RECT clip;
    struct Point p;
    struct Point q;
    struct Point bp;
    int out[4];
    int out2[4];
    int w;
    int h;
    int frame;
    struct LLS *lls;

    if (slot->flags & 4) {
        spriteA = LogFlumeBarrelMatteSprite;
        spriteB = LogFlumeBarrel1Sprite;
    } else {
        spriteA = LogFlumeBarrelMSprite;
        spriteB = LogFlumeBarrelSprite;
    }
    GetClipping(&saved);
    clip = saved;
    if (other != owner) {
        GetTileDimensions(&w, &h);
        if (owner->tile.pos.x == other->tile.pos.x) {
            p.x = other->tile.pos.x + 1;
            p.y = other->tile.pos.y;
            GetTileBounds(&p, out);
            if (out[2] < clip.right) {
                clip.right = out[2];
            }
            p.x = other->tile.pos.x + 1;
            p.y = other->tile.pos.y + 1;
            GetTileBounds(&p, out);
            if ((w >> 1) + out[0] > clip.left) {
                clip.left = (w >> 1) + out[0];
            }
        } else {
            p.x = other->tile.pos.x;
            p.y = other->tile.pos.y + 1;
            GetTileBounds(&p, out);
            if (out[0] + 1 > clip.left) {
                clip.left = out[0] + 1;
            }
            p.x = other->tile.pos.x + 1;
            p.y = other->tile.pos.y + 1;
            GetTileBounds(&p, out);
            if ((w >> 1) + out[0] + 1 < clip.right) {
                clip.right = (w >> 1) + out[0] + 1;
            }
        }
    }
    SetClipping(&clip);
    if (spriteB != NULL) {
        p.y = slot->y;
        p.x = slot->x;
        GetTileDimensions(&w, &h);
        w <<= 1;
        h <<= 1;
        p.x -= w >> 1;
        out[0] = owner->tile.pos.x;
        out[1] = owner->tile.pos.y;
        GetTileBounds((struct Point *)out, out2);
        q.x = out2[0];
        q.y = out2[1];
        if (owner->link28 != NULL) {
            FUN_0040cfd0(owner->link28);
        }
        if (owner->link28 == NULL) {
            FUN_0040cfd0(owner);
        }
        AdjustOffsetForViewMode(&p);
        p.x -= spriteB->width >> 1;
        p.y -= (int)((float)(short)spriteB->height * 0.75f + (slot->field_1c >> 1));
        if (param_3 != 0) {
            PrintSprite(spriteB, p.x + q.x, p.y + q.y, 0, 0);
            if (slot->busy != NULL) {
                if (((struct RideNode *)slot->busy)->rider->flags & 0x80) {
                    if (((struct RideNode *)slot->busy)->person != NULL) {
                        bp.x = p.x + (spriteB->width >> 1);
                        bp.y = p.y + ((short)spriteB->height >> 2) + ((short)spriteB->height >> 1);
                        AdjustBlokePosition(&bp);
                        SetPersonPosition(((struct RideNode *)slot->busy)->person, bp.x + q.x, bp.y + q.y);
                        SetPersonDirection(((struct RideNode *)slot->busy)->person, FUN_004092b0((struct FlumeHolder *)slot));
                        IP_RenderBlokeIn3DNow(((struct RideNode *)slot->busy)->rider);
                    }
                }
            }
        }
        if (spriteA != NULL) {
            frame = 0;
            lls = (struct LLS *)GetLLSForSprite((struct SpriteLLS *)spriteB);
            if (lls != NULL) {
                frame = lls->frame;
            }
            lls = (struct LLS *)GetLLSForSprite((struct SpriteLLS *)spriteA);
            if (lls != NULL) {
                LLSSetFrame(lls, frame);
            }
            PrintSprite(spriteA, p.x + q.x, p.y + q.y, 0, 0);
        }
    }
    SetClipping(&saved);
}

// FUNCTION: LEGOLAND 0x0040b210
int FUN_0040b210(struct FlumeWeighted *self, struct FlumeWeighted *other) {
    struct FlumeWeighted *link = self->link;
    if (link == other) {
        return 1;
    }
    if (self->weight >= DOUBLE_004ab398 && link->a == other) {
        return 1;
    }
    if (self->weight < DOUBLE_004ab398 && link->b == other) {
        return 1;
    }
    return 0;
}

// FUNCTION: LEGOLAND 0x0040b270
int FUN_0040b270(unsigned int *param_1, unsigned int param_2) {
    return param_1[5] == param_2;
}

// FUNCTION: LEGOLAND 0x0040b290
void FUN_0040b290(struct FlumeEntry *entry, int x, int y, struct FlumeStageList *list, int reverse) {
    struct FlumeSlotSet *set = entry->slotset;
    int i = 0;
    int j;
    int off;
    struct FlumeRunNode *node;
    struct FlumeStage *stage;

    if (reverse == 0) {
        node = entry->first;
    } else {
        node = entry->last;
    }
    stage = (struct FlumeStage *)list->stages;
    if (node != NULL) {
        off = 0;
        do {
            for (j = 0; j < set->count; j++) {
                if (FUN_0040b210((struct FlumeWeighted *)&set->slots[j], (struct FlumeWeighted *)node)) {
                    FUN_0040ae90((unsigned int)&set->slots[j], (int)node, 1);
                }
            }
            if (reverse == 0) {
                node = node->next;
            } else {
                node = node->prev;
            }
            i++;
            if (i >= stage->count) {
                if (stage->sprite != NULL) {
                    PrintSprite(stage->sprite, x, y, 0, 0);
                }
                off += 12;
                stage = (struct FlumeStage *)(list->stages + off);
            }
        } while (node != NULL);
    }
    if (stage->sprite != NULL) {
        PrintSprite(stage->sprite, x, y, 0, 0);
    }
}

// FUNCTION: LEGOLAND 0x0040b390
int FUN_0040b390(struct FlumeEntry *entry) {
    struct FlumeSlotSet *set = entry->slotset;
    int matched = 0;
    int i;
    struct FlumeEntry *node;

    for (i = 0; i < set->count; i++) {
        node = entry->sub2;
        if (node != NULL) {
            do {
                if (FUN_0040b210((struct FlumeWeighted *)&set->slots[i], (struct FlumeWeighted *)node)) {
                    FUN_0040ae90((unsigned int)&set->slots[i], (int)node, 1);
                    matched++;
                }
                node = node->next;
            } while (node != NULL);
        } else {
            if (FUN_0040b210((struct FlumeWeighted *)&set->slots[i], (struct FlumeWeighted *)entry)) {
                FUN_0040ae90((unsigned int)&set->slots[i], (int)entry, 1);
                matched++;
            }
        }
    }
    return matched;
}

// FUNCTION: LEGOLAND 0x0040b420
void RenderLogFlumeEntrance(Element *elem, unsigned int param_2, unsigned int param_3, TileId *tile, unsigned int param_5, unsigned int clip) {
    struct Ride *ride = elem->ride;
    struct Point base;
    struct FlumeEntry *entry;
    struct Point coords;
    struct Point pos;
    struct RideNode *node;
    struct LLS *lls;
    struct Sprite *sprite;
    int ok;

    base.x = ride->x + tile->pos.x;
    base.y = ride->y + tile->pos.y;
    entry = FindFlumeEntryByTile(tile);
    coords = GetScreenCoordsForObject(tile, ride);
    if (entry != NULL) {
        ok = FUN_0040b390(entry->link);
    }
    if (ok) {
        struct Point off;
        int frame;
        lls = (struct LLS *)GetLLSForSprite((struct SpriteLLS *)GetSpriteForLayer(ride->layer, 0));
        if (lls != NULL) {
            frame = lls->frame;
        }
        lls = (struct LLS *)GetLLSForSprite((struct SpriteLLS *)LogFlumeEnta3MSprite);
        if (lls != NULL) {
            LLSSetFrame(lls, frame);
        }
        pos = GetScreenCoordsForObject(tile, ride);
        off = GetRenderOffsetForLayer(ride->layer, 0);
        AdjustOffsetForViewMode(&off);
        if (LogFlumeEnta3MSprite != NULL) {
            PrintSprite(LogFlumeEnta3MSprite, pos.x + off.x, pos.y + off.y, clip, 0);
        }
    }
    {
        struct Point off;
        sprite = GetSpriteForLayer(ride->layer, 1);
        pos = GetScreenCoordsForObject(tile, ride);
        off = GetRenderOffsetForLayer(ride->layer, 1);
        AdjustOffsetForViewMode(&off);
        PrintSprite(sprite, pos.x + off.x, pos.y + off.y, clip, 0);
    }

    RenderItems_New();
    DAT_004cbe70 = NULL;
    for (node = ride->riders; node != NULL; node = node->next) {
        if (tile->id == node->tile.id && !(node->rider->flags & 0x80) && node->rider->pos.x <= (base.x << 8) - 0x280) {
            AddBlokeToRenderList(&DAT_004cbe70, (struct BlokeRenderSrc *)node, node->person->field_20);
        }
    }
    RenderBlokeList((struct BlokeListHead *)&DAT_004cbe70);

    RenderItems_New();
    DAT_004cbe70 = NULL;
    for (node = ride->riders; node != NULL; node = node->next) {
        if (tile->id == node->tile.id && !(node->rider->flags & 0x80) && node->rider->pos.x > (base.x << 8) - 0x280 && (node->rider->pos.y >> 8) <= base.y - 9) {
            AddBlokeToRenderList(&DAT_004cbe70, (struct BlokeRenderSrc *)node, node->person->field_20);
        }
    }
    RenderBlokeList((struct BlokeListHead *)&DAT_004cbe70);
    if (LogFlumeEntrance3Sprite != NULL) {
        struct Point off;
        int frame;
        off.x = -100;
        off.y = -0xd1;
        lls = NULL;
        sprite = GetSpriteForLayer(ride->layer, 2);
        if (sprite != NULL) {
            lls = (struct LLS *)GetLLSForSprite((struct SpriteLLS *)sprite);
        }
        if (lls != NULL) {
            frame = lls->frame;
        }
        AdjustOffsetForViewMode(&off);
        lls = (struct LLS *)GetLLSForSprite((struct SpriteLLS *)LogFlumeEntrance3Sprite);
        if (lls != NULL) {
            LLSSetFrame(lls, frame);
        }
        PrintSprite(LogFlumeEntrance3Sprite, coords.x + off.x, coords.y + off.y, clip, 0);
    }

    RenderItems_New();
    DAT_004cbe70 = NULL;
    for (node = ride->riders; node != NULL; node = node->next) {
        if (tile->id == node->tile.id && !(node->rider->flags & 0x80) && node->rider->pos.x > (base.x << 8) - 0x280) {
            int y = node->rider->pos.y >> 8;
            if (y == base.y - 6 || y == base.y - 7 || y == base.y - 8) {
                AddBlokeToRenderList(&DAT_004cbe70, (struct BlokeRenderSrc *)node, node->person->field_20);
            }
        }
    }
    RenderBlokeList((struct BlokeListHead *)&DAT_004cbe70);
    if (LogFlumeEntrance1Sprite != NULL) {
        struct Point off;
        int frame;
        off.x = 8;
        off.y = -0x71;
        lls = NULL;
        sprite = GetSpriteForLayer(ride->layer, 2);
        if (sprite != NULL) {
            lls = (struct LLS *)GetLLSForSprite((struct SpriteLLS *)sprite);
        }
        if (lls != NULL) {
            frame = lls->frame;
        }
        AdjustOffsetForViewMode(&off);
        lls = (struct LLS *)GetLLSForSprite((struct SpriteLLS *)LogFlumeEntrance1Sprite);
        if (lls != NULL) {
            LLSSetFrame(lls, frame);
        }
        PrintSprite(LogFlumeEntrance1Sprite, coords.x + off.x, coords.y + off.y, clip, 0);
    }

    RenderItems_New();
    DAT_004cbe70 = NULL;
    for (node = ride->riders; node != NULL; node = node->next) {
        if (tile->id == node->tile.id && !(node->rider->flags & 0x80) && node->rider->pos.x > (base.x << 8) - 0x280 && (node->rider->pos.y >> 8) == base.y - 3) {
            AddBlokeToRenderList(&DAT_004cbe70, (struct BlokeRenderSrc *)node, node->person->field_20);
        }
    }
    RenderBlokeList((struct BlokeListHead *)&DAT_004cbe70);
    if (LogFlumeEntrance2Sprite != NULL) {
        struct Point off;
        int frame;
        off.x = -100;
        off.y = -0xd1;
        lls = NULL;
        sprite = GetSpriteForLayer(ride->layer, 2);
        if (sprite != NULL) {
            lls = (struct LLS *)GetLLSForSprite((struct SpriteLLS *)sprite);
        }
        if (lls != NULL) {
            frame = lls->frame;
        }
        AdjustOffsetForViewMode(&off);
        lls = (struct LLS *)GetLLSForSprite((struct SpriteLLS *)LogFlumeEntrance2Sprite);
        if (lls != NULL) {
            LLSSetFrame(lls, frame);
        }
        PrintSprite(LogFlumeEntrance2Sprite, coords.x + off.x, coords.y + off.y, clip, 0);
    }

    RenderItems_New();
    DAT_004cbe70 = NULL;
    for (node = ride->riders; node != NULL; node = node->next) {
        if (tile->id == node->tile.id && !(node->rider->flags & 0x80) && node->rider->pos.x > (base.x << 8) - 0x280) {
            int y = node->rider->pos.y >> 8;
            if (y == base.y - 2 || y == base.y - 1 || y == base.y) {
                AddBlokeToRenderList(&DAT_004cbe70, (struct BlokeRenderSrc *)node, node->person->field_20);
            }
        }
    }
    RenderBlokeList((struct BlokeListHead *)&DAT_004cbe70);
    if (LogFlumeSignSprite != NULL) {
        struct Point off;
        int frame;
        off.x = -100;
        off.y = -0xd1;
        AdjustOffsetForViewMode(&off);
        lls = NULL;
        sprite = GetSpriteForLayer(ride->layer, 2);
        if (sprite != NULL) {
            lls = (struct LLS *)GetLLSForSprite((struct SpriteLLS *)sprite);
        }
        if (lls != NULL) {
            frame = lls->frame;
        }
        lls = (struct LLS *)GetLLSForSprite((struct SpriteLLS *)LogFlumeSignSprite);
        if (lls != NULL) {
            LLSSetFrame(lls, frame);
        }
        PrintSprite(LogFlumeSignSprite, coords.x + off.x, coords.y + off.y, clip, 0);
    }
    if (LogFlumeEnta1Matte2Sprite != NULL) {
        struct Point off;
        off = GetRenderOffsetForLayer(ride->layer, 2);
        AdjustOffsetForViewMode(&off);
        PrintSprite(LogFlumeEnta1Matte2Sprite, coords.x + off.x, coords.y + off.y, clip, 0);
    }
}

// FUNCTION: LEGOLAND 0x0040ba80
int FUN_0040ba80(struct Node *arg) {
    struct Node *current = arg->next;
    if (current != NULL) {
        struct Node *target = arg->prev;
        while (current != NULL) {
            current = current->next;
            if (current == target) {
                return 1;
            }
        }
    }
    return 0;
}

// FUNCTION: LEGOLAND 0x0040bab0
int FUN_0040bab0(struct FlumeSlotSet *set, int index) {
    struct FlumeSlot *slot = &set->slots[index];
    struct FlumeSlot *other;
    struct FlumeWeighted *link;
    float weight;
    float f;
    int i;

    if (slot != NULL) {
        link = slot->owner;
        if (link->a != NULL) {
            weight = slot->weight;
            f = weight;
            for (i = 0, other = set->slots; i < set->count; i++, other++) {
                if (i != index) {
                    if (other->owner == link || other->owner == link->a) {
                        if (other->owner == link) {
                            f = other->weight - weight;
                        }
                        if (other->owner == link->a) {
                            f = (float)(other->weight + 1.0 - weight);
                        }
                        if (f > FLOAT_004ab390 && f < 0.8) {
                            return 0;
                        }
                    }
                }
            }
            return 1;
        }
    }
    return 0;
}

// FUNCTION: LEGOLAND 0x0040bb50
int FUN_0040bb50(struct FlumeSlotSet *set, struct FlumeSlot **out) {
    int i;
    int count = set->count;
    struct FlumeSlot *slot;

    for (i = 0; i < count; i++) {
        slot = &set->slots[i];
        if (slot->owner == set->inner->a && slot->busy == 0 && !(slot->flags & 1)) {
            *out = slot;
            return 1;
        }
    }
    *out = NULL;
    return 0;
}

// FUNCTION: LEGOLAND 0x0040bbb0
void FUN_0040bbb0(struct FlumeSlotSet *set, int index) {
    struct FlumeSlot *slot = &set->slots[index];

    if (slot != NULL) {
        if (slot->flags & 2) {
            FUN_00411810(slot);
            return;
        }
        if (slot->flags & 1) {
            if (slot->owner->a != NULL) {
                if (FUN_0040bab0(set, index)) {
                    if (AdvanceFlumeMover(slot)) {
                        slot->owner = slot->owner->a;
                        if (FUN_00411650(slot)) {
                            slot->flags |= 2;
                            return;
                        }
                        if (slot->owner == set->field_c->a) {
                            if (slot->busy != NULL) {
                                slot->busy->counter->count++;
                                slot->busy = NULL;
                                slot->timer = 0x32;
                                slot->flags &= ~1;
                            }
                        }
                        if (slot->owner == set->inner->a) {
                            slot->timer = 0x32;
                            slot->flags &= ~1;
                        }
                    }
                }
            }
        } else {
            slot->timer--;
            if (slot->timer < 0) {
                if (!(set->flags & 1)) {
                    if (slot->owner->a != NULL) {
                        if (FUN_0040bab0(set, index)) {
                            slot->flags |= 1;
                            if (AdvanceFlumeMover(slot)) {
                                slot->owner = slot->owner->a;
                                if (slot->owner == set->field_c->a) {
                                    if (slot->busy != NULL) {
                                        slot->busy->counter->count++;
                                        slot->busy = NULL;
                                        slot->timer = 0x32;
                                        slot->flags &= ~1;
                                    }
                                }
                                if (slot->owner == set->inner->a) {
                                    slot->timer = 0x32;
                                    slot->flags &= ~1;
                                }
                            }
                        }
                    }
                }
                slot->timer = 1;
            }
        }
    }
}

// FUNCTION: LEGOLAND 0x0040bd40
void FUN_0040bd40(struct FlumeSlotSet *set, int index) {
    struct FlumeSlot *slot = &set->slots[index];

    if (slot != NULL) {
        if (slot->flags & 1) {
            if (slot->owner == set->field_c) {
                if (slot->busy != NULL) {
                    slot->busy->counter->count++;
                    slot->busy = NULL;
                }
            }
            if (slot->owner == set->inner) {
                slot->timer = 0x32;
                slot->flags &= ~1;
            } else if (slot->owner->a != NULL) {
                if (FUN_0040bab0(set, index)) {
                    slot->owner = slot->owner->a;
                }
            }
        } else {
            slot->timer--;
            if (slot->timer < 0) {
                if (!(set->flags & 1)) {
                    if (slot->owner->a != NULL) {
                        if (FUN_0040bab0(set, index)) {
                            slot->flags |= 1;
                            slot->owner = slot->owner->a;
                        }
                    }
                }
                slot->timer = 1;
            }
        }
    }
}

// FUNCTION: LEGOLAND 0x0040be00
void FUN_0040be00(struct FlumeSlotSet *set) {
    struct FlumeSlot *slot;
    struct FlumeQueueItem *item;
    int i;

    if (set->flags & 2) {
        if (++set->ticks >= set->limit) {
            set->flags &= ~2;
        }
    }
    if (FUN_0040ba80((struct Node *)set)) {
        set->timer--;
        if (set->timer < 0) {
            set->timer = 2;
            for (i = 0; i < set->count; i++) {
                FUN_0040bbb0(set, i);
            }
            if (!(set->flags & 1)) {
                slot = NULL;
                if (FUN_0040bb50(set, &slot)) {
                    item = NULL;
                    set->target = slot;
                    if (QueueHasNodes((struct Queue *)set->queue)) {
                        if (FUN_00411ea0((struct Queue *)set->queue)) {
                            set->flags |= 1;
                            FUN_00412060((struct Queue *)set->queue, (struct QueueItemMid **)&item);
                            slot->busy = (struct FlumeStatHolder *)item;
                        }
                    }
                }
            }
        }
    } else {
        FUN_0040a580((struct ParticleEmitter *)set);
    }
    FUN_004120a0((struct Queue *)set->queue, LogFlumeEntranceRide->x + set->tile.pos.x, LogFlumeEntranceRide->y + set->tile.pos.y);
    if (FUN_00411e60((struct Queue *)set->queue)) {
        Ride_SetFlagToNotLetAnyoneOn(&set->tile);
    } else {
        Ride_ClearFlagToNotLetAnyoneOn(&set->tile);
    }
    set->dir = (set->dir + 1) & 0xf;
}

// FUNCTION: LEGOLAND 0x0040bf50
void FUN_0040bf50(void) {
    struct FlumeEntry *current = FlumeEntryList;
    while (current != NULL) {
        FUN_0040be00((struct FlumeSlotSet *)current);
        current = current->next;
    }
}

// FUNCTION: LEGOLAND 0x0040bf70
void LogFlumeEntranceUpdate(Element *obj) {
    int field_73;
    Ride *ride = obj->ride;
    RideNode *elem = ride->riders;
    RideNode *next;
    Bloke *bloke;
    TileId *tile;
    struct FlumeEntry *entry;
    unsigned int x;
    unsigned int y;
    char dir;
    int v;

    FUN_0040bf50();
    while (elem != NULL) {
        tile = &elem->tile;
        x = ride->field_24 + tile->pos.x;
        next = elem->next;
        bloke = elem->rider;
        y = ride->field_25 + tile->pos.y;
        if (bloke->low_level_action == 0) {
            switch (bloke->param_action) {
            case 0:
                bloke->flags |= 8;
                entry = FindFlumeEntryByTile(tile);
                if (entry != NULL) {
                    FUN_00411f20(&entry->queue, (struct QueueItemMid *)elem);
                    if (FUN_00411e60(&entry->queue)) {
                        Ride_SetFlagToNotLetAnyoneOn(&entry->tile);
                    }
                }
                break;
            case 2:
                v = (tile->pos.x - 2) << 8;
                bloke->dest.x = v;
                v = tile->pos.y << 8;
                bloke->dest.y = v;
                dir = CalcMoveLine(bloke->pos, bloke->dest, &bloke->nav);
                bloke->low_level_action = 7;
                bloke->field_73 = dir + 0x10;
                NewDirForAction(bloke, (bloke->field_73 >> 5) + 3);
                bloke->param_action++;
                break;
            case 3:
                v = (tile->pos.x << 8) - 0x280;
                bloke->dest.x = v;
                v = (tile->pos.y << 8) + 0x80;
                bloke->dest.y = v;
                dir = CalcMoveLine(bloke->pos, bloke->dest, &bloke->nav);
                bloke->low_level_action = 7;
                bloke->field_73 = dir + 0x10;
                NewDirForAction(bloke, (bloke->field_73 >> 5) + 3);
                bloke->param_action++;
                break;
            case 4:
                bloke->flags = bloke->flags | 0x80;
                BlokeWalkAnim(bloke);
                BlokeSetFrame(bloke, 0);
                entry = FindFlumeEntryByTile(tile);
                if (entry != NULL) {
                    if (NULL != entry->target) {
                        *(int *)entry->target = 1;
                        entry->target = NULL;
                    }
                    entry->flags &= ~1;
                }
                bloke->param_action++;
                break;
            case 6:
                bloke->flags &= ~0x80;
                BlokeWalkAnim(bloke);
                BlokeSetFrame(bloke, 0);
                bloke->pos.x = (tile->pos.x << 8) - 0x280;
                bloke->pos.y = (tile->pos.y << 8) - 0x80;
                bloke->dir = 3;
                bloke->dest.x = (tile->pos.x - 2) << 8;
                bloke->dest.y = (tile->pos.y << 8) - 0x100;
                dir = CalcMoveLine(bloke->pos, bloke->dest, &bloke->nav);
                bloke->low_level_action = 7;
                bloke->field_73 = dir + 0x10;
                field_73 = bloke->field_73;
                NewDirForAction(bloke, (field_73 >> 5) + 3);
                bloke->param_action++;
                break;
            case 7:
                v = tile->pos.x << 8;
                bloke->dest.x = v;
                v = (tile->pos.y << 8) - 0x100;
                bloke->dest.y = v;
                dir = CalcMoveLine(bloke->pos, bloke->dest, &bloke->nav);
                bloke->low_level_action = 7;
                bloke->field_73 = 0x10 + dir;
                NewDirForAction(bloke, (bloke->field_73 >> 5) + 3);
                bloke->param_action++;
                break;
            case 8:
                FUN_004122a0((struct RideSlotArg *)DAT_004c2af8, (struct RideSlot *)bloke);
                break;
            case 9:
                FUN_00412300((struct QueueTable *)DAT_004c2af8, x, y, bloke);
                break;
            case 10:
                bloke->dest.x = (x << 8) + 0x80;
                bloke->dest.y = ((tile->pos.y + ride->field_25) << 8) + 0x80;
                dir = CalcMoveLine(bloke->pos, bloke->dest, &bloke->nav);
                bloke->low_level_action = 7;
                bloke->field_73 = dir + 0x10;
                NewDirForAction(bloke, (bloke->field_73 >> 5) + 3);
                bloke->param_action++;
                break;
            case 11:
                RemoveBlokeFromRide(ride, elem);
                bloke->flags &= ~8;
                break;
            }
        }
        elem = next;
    }
}

// FUNCTION: LEGOLAND 0x0040c250
int FUN_0040c250(struct FlumeEntry *entry) {
    struct FlumeSlotSet *set = entry->slotset;
    struct FlumeEntry *param_1 = entry;
    struct FlumeEntry *tmp;
    struct FlumeEntry *node;
    int i;

    tmp = entry->alt;
    if (tmp == NULL) {
        tmp = entry->next;
    }
    if (tmp != NULL) {
        param_1 = tmp;
    }
    if (param_1 == NULL) {
        return 0;
    }
    tmp = param_1->sub2;
    if (tmp != NULL) {
        param_1 = tmp;
    }
    for (i = 0; i < set->count; i++) {
        node = entry->sub2;
        if (node == NULL) {
            if (FUN_0040b270((unsigned int *)&set->slots[i], (unsigned int)entry)) {
                set->slots[i].owner = (struct FlumeWeighted *)param_1;
            }
        } else {
            do {
                if (FUN_0040b270((unsigned int *)&set->slots[i], (unsigned int)node)) {
                    set->slots[i].owner = (struct FlumeWeighted *)param_1;
                    break;
                }
                node = node->next;
            } while (node != NULL);
        }
    }
    return 1;
}

struct FlumeChainNode {
    struct FlumeChainNode *next;
};

struct FlumeChainOwner {
    unsigned char pad_0[0x24];
    struct FlumeSlotSet *slots;
    unsigned char pad_28[4];
    struct FlumeChainNode *chain;
};

// FUNCTION: LEGOLAND 0x0040c2e0
int FUN_0040c2e0(struct FlumeChainOwner *owner) {
    struct FlumeSlotSet *set = owner->slots;
    int i;
    struct FlumeChainNode *node;

    for (i = 0; i < set->count; i++) {
        node = owner->chain;
        if (node == NULL) {
            if (FUN_0040b210((struct FlumeWeighted *)&set->slots[i], (struct FlumeWeighted *)owner)) {
                return 1;
            }
        } else {
            do {
                if (FUN_0040b210((struct FlumeWeighted *)&set->slots[i], (struct FlumeWeighted *)node)) {
                    return 1;
                }
                node = node->next;
            } while (node != NULL);
        }
    }
    return 0;
}

// FUNCTION: LEGOLAND 0x0040c350
void LogFlumeTrackLoad(Element *elem) {
    int i;

    LogFlumeTrackRide = elem->ride;
    LogFlumeTrackRide->flags |= 0x400;
    LogFlumeTrackRide->field_18 = 0;
    LogFlumeTrackRide->field_14 = 0;
    DAT_004c74f4 = elem;
    // STRING: LEGOLAND 0x004b4a0c
    if (LLIDB_FindElement("LOG FLUME IMAGE LIST", &LogFlumeImageListId, 0) == 0) {
        LogFlumeImageList = LLIDB_LoadData((void *)LogFlumeImageListId);
    }
    // STRING: LEGOLAND 0x004b49f0
    if (LLIDB_FindElement("LOG FLUME TRACK ENDY LIST", &LogFlumeTrackEndyListId, 0) == 0) {
        LogFlumeTrackEndyList = LLIDB_LoadData((void *)LogFlumeTrackEndyListId);
    }
    FUN_004113d0();
    for (i = 0; i < 10; i++) {
        LogFlumeTrackSprites[i] = LoadSprite(LogFlumeTrackSpriteFiles[i], 1);
    }
    // STRING: LEGOLAND 0x004b49e4
    LogFlumeFc1M3Sprite = LoadSprite("fc1_m3.lls", 1);
    // STRING: LEGOLAND 0x004b49d8
    LogFlumeFc3M3Sprite = LoadSprite("fc3_m3.lls", 1);
}

// FUNCTION: LEGOLAND 0x0040c430
void LogFlumeTrackUnload(void) {
    struct Sprite **ptr;

    LLIDB_UnLoadData(LogFlumeImageListId);

    ptr = LogFlumeTrackSprites;
    while (ptr < LogFlumeTrackSprites + 10) {
        if (*ptr != NULL) {
            KillSprite(*ptr);
        }
        ptr++;
    }

    if (LogFlumeFc1M3Sprite != NULL) {
        KillSprite(LogFlumeFc1M3Sprite);
    }
    if (LogFlumeFc3M3Sprite != NULL) {
        KillSprite(LogFlumeFc3M3Sprite);
    }

    LLIDB_UnLoadData(LogFlumeTrackEndyListId);
}

// FUNCTION: LEGOLAND 0x0040c4a0
void LogFlumeTrackCalcCursor(Element *elem, int *param_2, unsigned int param_3) {
    struct Ride *ride = elem->ride;
    struct MapRect rect;
    TileId t;
    struct Slot **list;
    struct Slot *p;
    unsigned int key;
    int r;
    int cost;
    int code;

    memcpy(&EditCursor.footprint, &LogFlumeFootprint, sizeof(struct Footprint));
    EditCursor.footprint.x1 = LogFlumeFootprint.x1 - 1;
    EditCursor.footprint.y1--;
    ScreenToMapRef(param_2, &EditCursor.tile_x, param_3);
    FUN_0045f460(&EditCursor);
    EditCursor.next = NULL;
    ValidateCursor(&EditCursor, (unsigned int)ride);
    if (FUN_0045f4b0(&EditCursor)) {
        rect.x0 = EditCursor.footprint.x0 + EditCursor.tile_x;
        rect.y0 = EditCursor.tile_y + EditCursor.footprint.y0;
        rect.x1 = EditCursor.footprint.x1 + EditCursor.tile_x;
        rect.y1 = EditCursor.footprint.y1 + EditCursor.tile_y;
        r = CheckForPeople(&rect);
        switch (r) {
        case -1:
            FUN_0045f480(&EditCursor, 4);
            break;
        case 1:
            FUN_0045f480(&EditCursor, 3);
            break;
        }
    }
    cost = GetObjCost(ride);
    if (GetBrickCount() < cost) {
        FUN_0045f480(&EditCursor, 2);
    }
    if (FUN_0045f4b0(&EditCursor)) {
        t.pos.x = LogFlumeFootprint.x0 + EditCursor.tile_x;
        t.pos.y = LogFlumeFootprint.y0 + EditCursor.tile_y;
        FUN_00409440(t, (void **)&list);
        FUN_00409510((struct StateSlots *)list);
        if (FUN_00409470((unsigned int *)list) == 0) {
            FUN_0045f480(&EditCursor, 0xe);
        } else {
            FUN_0045f480(&EditCursor, 0xe);
            p = list[0];
            if (p != NULL) {
                key = p->key;
            } else {
                p = list[1];
                if (p != NULL) {
                    key = p->key;
                } else {
                    p = list[2];
                    if (p != NULL) {
                        key = p->key;
                    } else {
                        p = list[3];
                        if (p != NULL) {
                            key = p->key;
                        }
                    }
                }
            }
            FUN_004094b0(key, list);
            code = FUN_00409410((unsigned int *)list);
            if (FUN_00409580(code, (struct StateSlots *)list)) {
                FUN_0040d520((struct FlumeEntry **)list, &EditCursor);
                FUN_0045f460(&EditCursor);
            }
        }
    }
    FUN_0045f4d0(&EditCursor);
}

// FUNCTION: LEGOLAND 0x0040c6c0
void LogFlumeTrackDCalcCursor(int unused, struct Point *pt) {
    struct FlumeEntry *entry = FUN_0040d210(pt->x, pt->y);
    unsigned int v;

    if (entry != NULL) {
        FUN_0045f480(&QueryCursor, 1);
        QueryCursor.tile_x = entry->tile.pos.x;
        QueryCursor.tile_y = entry->tile.pos.y;
        v = LogFlumeFootprint.x1;
        memcpy(&QueryCursor.footprint, &LogFlumeFootprint, sizeof(struct Footprint));
        QueryCursor.footprint.x1 = v - 1;
        QueryCursor.footprint.y1 = QueryCursor.footprint.y1 - 1;
        QueryCursor.field_1828 = 8;
        if (FUN_00409140((struct Node *)entry)) {
            if ((entry->field_c == NULL || entry->field_8 == NULL || FUN_0040ba80((struct Node *)entry->parent)) && !(entry->flags10d & 1)) {
                FUN_0045f460(&QueryCursor);
            }
        }
    }
}

// FUNCTION: LEGOLAND 0x0040c780
void LogFlumeTrackAddObject(int unused, struct Point *pt) {
    TileId t;
    int coords[2];
    struct FlumeEntry *entry;
    struct Slot **list;
    struct Slot *p;
    unsigned int key;

    t.pos.x = pt->x;
    t.pos.y = pt->y;
    coords[0] = t.pos.x;
    coords[1] = t.pos.y;
    entry = FUN_00409010();
    if (entry != NULL) {
        entry->ride = LogFlumeTrackRide;
        entry->link28 = NULL;
        entry->tile.pos.x = LogFlumeFootprint.x0 + coords[0];
        entry->tile.pos.y = LogFlumeFootprint.y0 + coords[1];
        FUN_00409440(t, (void **)&list);
        FUN_00409510((struct StateSlots *)list);
        p = list[0];
        if (p != NULL) {
            key = p->key;
        } else {
            p = list[1];
            if (p != NULL) {
                key = p->key;
            } else {
                p = list[2];
                if (p != NULL) {
                    key = p->key;
                } else {
                    p = list[3];
                    if (p != NULL) {
                        key = p->key;
                    }
                }
            }
        }
        FUN_004119a0((struct ParticleEmitter *)key, 1);
        if (key != 0) {
            entry->parent = (struct FlumeEntry *)key;
            FUN_004094b0(key, list);
            FUN_004091f0((struct Node *)key, (struct ListNode *)entry);
            memcpy(&LogFlumeTrackRide->footprint, &LogFlumeFootprint, sizeof(struct Footprint));
            LogFlumeTrackRide->footprint.x1--;
            LogFlumeTrackRide->footprint.y1--;
            AddBasicObject(DAT_004c74f4, coords);
            FUN_00409c20(entry, (unsigned int *)list);
            FUN_004097a0(entry, (struct StateSlots *)list);
        }
    }
}

// FUNCTION: LEGOLAND 0x0040c8d0
void LogFlumeTrackRemoveObject(Element *elem, TileId tile, struct Cursor *cursor) {
    struct FlumeEntry *entry;

    memcpy(&LogFlumeTrackRide->footprint, &LogFlumeFootprint, sizeof(struct Footprint));
    --LogFlumeTrackRide->footprint.x1;
    --LogFlumeTrackRide->footprint.y1;
    StandardRemoveObject(elem, tile, cursor);
    entry = FindFlumeSubEntryByTile(&tile);
    FUN_004119a0((struct ParticleEmitter *)entry->parent, -1);
    if (entry != NULL) {
        FUN_00409440(tile, (void **)&cursor);
        FUN_0040da10((struct Context *)entry, (struct LinkList *)cursor);
        FUN_0040a2a0(entry, (struct StateNode **)cursor);
        FUN_00409270((struct Node *)entry->parent, (struct Node *)entry);
    }
}

// FUNCTION: LEGOLAND 0x0040c970
struct RideSpriteInfo *GetLogFlumeTrackSpriteInfo(int unused, TileId tile) {
    struct FlumeEntry *entry;
    struct Sprite *spr;
    unsigned int lls;
    int idx;

    DAT_004c74d8.id = tile.id;
    entry = (struct FlumeEntry *)FUN_00408f30((struct SubBuf *)&tile);
    if (entry != NULL) {
        if (entry->flags10d & 4) {
            return NULL;
        }
        idx = FUN_0040ad50((struct StateNode *)entry);
        spr = LogFlumeImageList->sprites[(unsigned char)idx];
        DAT_004c74d8.sprite = spr;
        DAT_004c74d8.x = LogFlumeImageList->offset_x[(unsigned char)idx] >> 1;
        DAT_004c74d8.y = LogFlumeImageList->offset_y[(unsigned char)idx] >> 1;
        DAT_004c74d8.field_10 = 0;
        lls = GetLLSForSprite((struct SpriteLLS *)spr);
        if (lls != 0) {
            LLSStop(lls);
            LLSSetFrame((struct LLS *)lls, entry->parent->submode);
        }
        ((struct Sprite *)DAT_004c74d8.sprite)->flags |= 0x2000;
        DAT_004c74d8.field_10 = 0;
    }
    return &DAT_004c74d8;
}

// FUNCTION: LEGOLAND 0x0040ca30
void FUN_0040ca30(void *a1, int a2) {
    struct WalkNode *node = *(struct WalkNode **)a1;
    if (node != NULL) {
        do {
            if (node->var_4 != 0) {
                FUN_0040ae90(node->var_4, a2, 1);
            }
            node = node->var_8;
        } while (node != NULL);
    }
}

/* A corner of the log flume (track sprites fc1-fc4; this draws corners 0 and 2, which have the extra matte
 * fc1_m3/fc3_m3): the boats on the corner go behind or in front of the matte by their weight along the track,
 * then the corner track sprite. arg (the caller's clip) is used only for corner 0 when the next piece is in
 * another column. */
// FUNCTION: LEGOLAND 0x0040ca60
int RenderLogFlumeCorner(struct FlumeEntry *entry, int arg) {
    struct FlumeEntry *par = entry->parent;
    int count = 0;
    int i;
    struct FlumeSlot *slot;
    double w;
    struct Sprite *spr;
    struct Point pos;
    unsigned char px;
    unsigned char ex;
    int idx;

    RenderItems2_New();
    DAT_004c8d74 = NULL;
    DAT_004ca5ac = NULL;
    for (i = 0; i < entry->parent->count; i++) {
        slot = &par->slots[i];
        if (FUN_0040b210((struct FlumeWeighted *)slot, (struct FlumeWeighted *)entry)) {
            w = slot->weight;
            if ((struct FlumeEntry *)slot->owner != entry) {
                if (entry->field_c == slot->owner) {
                    w = slot->weight - 1.0;
                }
                if (entry->field_8 == slot->owner) {
                    w = slot->weight + 1.0;
                }
            }
            if (w <= DOUBLE_004ab398) {
                RenderItem2_AddItem(&DAT_004c8d74, (unsigned int)slot, 0);
            } else {
                RenderItem2_AddItem(&DAT_004ca5ac, (unsigned int)slot, 0);
            }
            count++;
        }
    }
    if (count != 0) {
        par = entry->parent8;
        pos = FUN_0040cfd0(entry);
        spr = LogFlumeFc1M3Sprite;
        if (entry->submode != 0) {
            spr = LogFlumeFc3M3Sprite;
        }
        px = par->tile.pos.x;
        ex = entry->tile.pos.x;
        if (entry->submode == 0) {
            if (px != ex) {
                FUN_0040ca30(&DAT_004c8d74, (int)entry);
                if (spr != NULL) {
                    PrintSprite(spr, pos.x, pos.y, arg, 0);
                }
                FUN_0040ca30(&DAT_004ca5ac, (int)entry);
                goto track;
            }
        } else if (px == ex) {
            FUN_0040ca30(&DAT_004c8d74, (int)entry);
            if (spr != NULL) {
                PrintSprite(spr, pos.x, pos.y, 0, 0);
            }
            FUN_0040ca30(&DAT_004ca5ac, (int)entry);
            goto track;
        }
        FUN_0040ca30(&DAT_004ca5ac, (int)entry);
        if (spr != NULL) {
            PrintSprite(spr, pos.x, pos.y, 0, 0);
        }
        FUN_0040ca30(&DAT_004c8d74, (int)entry);
    track:
        idx = FUN_0040ad50((struct StateNode *)entry);
        pos = FUN_0040cfd0(entry);
        spr = LogFlumeTrackSprites[idx];
        if (spr != NULL) {
            PrintSprite(spr, pos.x, pos.y, 0, 0);
        }
    }
}

// FUNCTION: LEGOLAND 0x0040cc00
void FUN_0040cc00(struct FlumeEntry *entry, int arg) {
    int idx;
    struct Point pos;
    struct Sprite *spr;

    if (FUN_0040b390(entry)) {
        idx = FUN_0040ad50((struct StateNode *)entry);
        pos = FUN_0040cfd0(entry);
        spr = LogFlumeTrackSprites[idx];
        if (spr != NULL) {
            PrintSprite(spr, pos.x, pos.y, arg, 0);
        }
    }
}

// FUNCTION: LEGOLAND 0x0040cc50
int RenderLogFlumeTrack(int a, int b, int c, TileId *tile, int e, int arg) {
    struct FlumeEntry *entry = FindFlumeSubEntryByTile(tile);
    if (entry != NULL) {
        if (entry->mode == 2 && (entry->submode == 0 || entry->submode == 2)) {
            return RenderLogFlumeCorner(entry, arg);
        }
        FUN_0040cc00(entry, arg);
    }
}

// FUNCTION: LEGOLAND 0x0040cca0
void FUN_0040cca0(struct StateNode *node) {
    struct FlumeEntry *entry = (struct FlumeEntry *)node;
    struct Point pos;
    struct Point off;
    struct Sprite *spr;
    int idx;

    entry->ride->field_18 = 0;
    entry->ride->field_14 = 0;
    pos = GetScreenCoordsForObject(&entry->tile, entry->ride);
    switch (entry->submode) {
    case 0:
        idx = 0;
        break;
    case 1:
        idx = 1;
        break;
    case 2:
        idx = 2;
        break;
    case 3:
        idx = 3;
        break;
    }
    spr = LogFlumeTrackEndyList->sprites[(unsigned char)idx];
    off.x = LogFlumeTrackEndyList->offset_x[(unsigned char)idx] >> 1;
    off.y = LogFlumeTrackEndyList->offset_y[(unsigned char)idx] >> 1;
    AdjustOffsetForViewMode(&off);
    if (spr != NULL) {
        PrintSprite(spr, pos.x + off.x, pos.y + off.y, 0, 0);
    }
}

// FUNCTION: LEGOLAND 0x0040cd70
void FUN_0040cd70(struct PairHolder *p, int param1) {
    struct StateNode *first;
    struct StateNode *second;
    unsigned int phase;

    if (p->var_2c == 0) {
        return;
    }

    first = p->var_30;
    second = p->var_34;

    if (first->state == 3) {
        phase = first->phase;
        if (param1 != 0) {
            if (phase != 0 && phase != 3) {
                FUN_0040cca0(first);
            }
        } else {
            if (phase != 1 && phase != 2) {
                FUN_0040cca0(first);
            }
        }
    }

    if (second->state == 3) {
        phase = second->phase;
        if (param1 != 0) {
            if (phase != 0 && phase != 3) {
                FUN_0040cca0(second);
            }
        } else {
            if (phase != 1 && phase != 2) {
                FUN_0040cca0(second);
            }
        }
    }
}

// FUNCTION: LEGOLAND 0x0040cdf0
int FUN_0040cdf0(TileId *tile) {
    int result = 0;
    struct FlumeEntry *entry = FindFlumeSubEntryByTile(tile);
    if (entry != NULL) {
        result = FUN_0040b390(entry);
    }
    return result;
}

// FUNCTION: LEGOLAND 0x0040ce20
void FUN_0040ce20(struct InputBuffer *esi) {
    unsigned int eax = LogFlumeFootprint.x1;
    unsigned int edx = LogFlumeFootprint.y0;
    unsigned int ebx = LogFlumeFootprint.y1;
    unsigned int local_8 = eax - LogFlumeFootprint.x0;
    struct SubBuf buf;

    ebx = ebx - edx;

    DAT_004cbe20 = 0;
    DAT_004cbe24 = 0;
    DAT_004cbe28 = 0;
    DAT_004cbe2c = 0;

    if (esi->flags & 1) {
        buf.b0 = esi->var_4;
        buf.b1 = esi->var_8 - (unsigned char)ebx;
        DAT_004cbe20 = FUN_00408f30(&buf);
    }
    if (esi->flags & 2) {
        buf.b0 = esi->var_c + (unsigned char)local_8;
        buf.b1 = esi->var_10;
        DAT_004cbe24 = FUN_00408f30(&buf);
    }
    if (esi->flags & 4) {
        buf.b0 = esi->var_14;
        buf.b1 = esi->var_18 + (unsigned char)ebx;
        DAT_004cbe28 = FUN_00408f30(&buf);
    }
    if (esi->flags & 8) {
        buf.b0 = esi->var_1c - (unsigned char)local_8;
        buf.b1 = esi->var_20;
        DAT_004cbe2c = FUN_00408f30(&buf);
    }
}

// FUNCTION: LEGOLAND 0x0040cf10
void FUN_0040cf10(struct InputBuffer *param_1, unsigned int **param_2) {
    FUN_0040ce20(param_1);
    *param_2 = &DAT_004cbe20;
}

// FUNCTION: LEGOLAND 0x0040cf30
unsigned int FUN_0040cf30(unsigned int *param_1) {
    unsigned int count = 0;
    int i;
    for (i = 0; i < 4; i++) {
        if (param_1[i] != 0) {
            count++;
        }
    }
    return count;
}

// FUNCTION: LEGOLAND 0x0040cf50
void FUN_0040cf50(unsigned int arg, struct Slot **slots) {
    int i;
    for (i = 0; i < 4; i++) {
        if (slots[i] != NULL && slots[i]->key != arg) {
            slots[i] = NULL;
        }
    }
}

// FUNCTION: LEGOLAND 0x0040cf80
unsigned int FUN_0040cf80(struct Slot **arg) {
    int i;
    for (i = 0; i < 4; i++) {
        if (arg[i] != NULL) {
            return arg[i]->key;
        }
    }
    return 0;
}

// FUNCTION: LEGOLAND 0x0040cfa0
void FUN_0040cfa0(struct StateNode *(*arr)[4]) {
    int i;
    for (i = 0; i < 4; i++) {
        struct StateNode *p = (*arr)[i];
        if (p != NULL) {
            if (p->state != 3 && p->state != 4) {
                (*arr)[i] = NULL;
            }
        }
    }
}

// FUNCTION: LEGOLAND 0x0040cfd0
struct Point FUN_0040cfd0(struct FlumeEntry *entry) {
    struct Point pos;
    struct Point off;
    int idx;

    if (entry->ride == LogFlumeEntranceRide) {
        return GetScreenCoordsForObject(&entry->parent->tile, LogFlumeEntranceRide);
    }
    if (entry->ride == LogFlumeTrackRide) {
        entry->ride->field_18 = 0;
        entry->ride->field_14 = 0;
        pos = GetScreenCoordsForObject(&entry->tile, entry->ride);
        idx = FUN_0040ad50((struct StateNode *)entry);
        off.x = LogFlumeImageList->offset_x[(unsigned char)idx] >> 1;
        off.y = LogFlumeImageList->offset_y[(unsigned char)idx] >> 1;
        AdjustOffsetForViewMode(&off);
        pos.x += off.x;
        pos.y += off.y;
        return pos;
    }
    return GetScreenCoordsForObject(&entry->tile, entry->ride);
}

// FUNCTION: LEGOLAND 0x0040d090
void FUN_0040d090(struct FlumeEntry *entry, struct Footprint **out, TileId *tile) {
    struct FlumeEntry *cur = entry->link28;
    void *r;
    unsigned int v;

    if (cur == NULL || cur == (struct FlumeEntry *)-1) {
        cur = entry;
    }
    tile->pos.x = cur->tile.pos.x;
    tile->pos.y = cur->tile.pos.y;
    if (cur->link28 == (struct FlumeEntry *)-1) {
        tile->pos.x = cur->parent->tile.pos.x;
        tile->pos.y = cur->parent->tile.pos.y;
        DAT_004c2aa8 = LogFlumeEntranceRide->footprint;
        DAT_004c2aa8.x1 = DAT_004c2aa8.x0 + (LogFlumeFootprint.x1 - LogFlumeFootprint.x0) * 2;
        *out = &DAT_004c2aa8;
        return;
    }
    r = cur->ride;
    if (r == LogFlumeTrackRide) {
        v = LogFlumeFootprint.x1;
        memcpy(&DAT_004c8d38, &LogFlumeFootprint, sizeof(struct Footprint));
        DAT_004c8d38.x1 = v - 1;
        DAT_004c8d38.y1 = DAT_004c8d38.y1 - 1;
        *out = &DAT_004c8d38;
    } else if (r == DAT_004c8d6c) {
        *out = (struct Footprint *)DAT_004c8d6c->var_3c;
    } else if (r == DAT_004c2b60) {
        *out = (struct Footprint *)DAT_004c2b60->var_3c;
    } else if (r == DAT_004c445c) {
        *out = (struct Footprint *)DAT_004c445c->var_3c;
    } else if (r == DAT_004c2aa0) {
        *out = (struct Footprint *)DAT_004c2aa0->var_3c;
    } else if (r == DAT_004c2b0c) {
        *out = (struct Footprint *)DAT_004c2b0c->var_3c;
    } else if (r == DAT_004c74d4) {
        *out = (struct Footprint *)DAT_004c74d4->var_3c;
    } else if (r == DAT_004cbe18) {
        *out = (struct Footprint *)DAT_004cbe18->var_3c;
    } else if (r == DAT_004c2bf0) {
        *out = (struct Footprint *)DAT_004c2bf0->var_3c;
    } else {
        *out = NULL;
    }
}

// FUNCTION: LEGOLAND 0x0040d210
struct FlumeEntry *FUN_0040d210(int x, int y) {
    struct FlumeEntry *outer;
    struct FlumeEntry *cur;
    struct Footprint *fp;
    TileId t;
    int tx;
    int ty;

    outer = FlumeEntryList;
    if (FlumeEntryList != NULL) {
        for (; outer != NULL; outer = outer->next) {
            cur = outer->sub;
            while (cur != NULL) {
                FUN_0040d090(cur, &fp, &t);
                if (fp != NULL) {
                    tx = t.pos.x;
                    ty = t.pos.y;
                    if (x >= fp->x0 + tx && x <= fp->x1 + tx && y >= fp->y0 + ty && y <= fp->y1 + ty) {
                        return cur;
                    }
                } else {
                    // STRING: LEGOLAND 0x004b4a24
                    DBPrintf("Something wrong in the log flume track\n");
                }
                cur = cur->next;
            }
        }
    }
    return NULL;
}

// FUNCTION: LEGOLAND 0x0040d2d0
void FUN_0040d2d0(struct Point *pt) {
    struct FlumeEntry *entry = FUN_0040d210(pt->x, pt->y);
    struct Footprint *fp;
    TileId t;

    if (entry != NULL) {
        QueryCursor.tile_x = entry->tile.pos.x;
        QueryCursor.tile_y = entry->tile.pos.y;
        FUN_0040d090(entry, &fp, &t);
        QueryCursor.footprint = *fp;
        QueryCursor.field_1828 = 8;
        FUN_0045f480(&QueryCursor, 1);
        if (FUN_00409140((struct Node *)entry->sub2)) {
            if (entry->link30->field_c == NULL || entry->link30->field_8 == NULL || entry->link34->field_c == NULL || entry->link34->field_8 == NULL || FUN_0040ba80((struct Node *)entry->parent)) {
                if (!(entry->flags10d & 1) && !FUN_0040c2e0((struct FlumeChainOwner *)entry)) {
                    FUN_0045f460(&QueryCursor);
                }
            }
        }
    }
}

// FUNCTION: LEGOLAND 0x0040d3b0
unsigned int FUN_0040d3b0(void *param_1, unsigned int *param_2) {
    EditMode.unk0 = 1;
    EditMode.unk8 = param_1;
    DefaultCursor(&EditCursor);
    EditCursor.field_1828 |= 8;
    BuildCursorPtr(&EditCursor, 0x8f8, 0);
    SetEditCursorFootPrint((void *)param_2);
    DAT_004ca5b0.field_1828 = 0x2034;
    DAT_004c1260.field_1828 = 0x2034;
    DAT_004c4468.field_1828 = 0x2034;
    DAT_004c5ca0.field_1828 = 0x2034;
    return 0x2034;
}

// FUNCTION: LEGOLAND 0x0040d420
void FUN_0040d420(unsigned int *res) {
    unsigned int flags = res[0];
    struct Cursor *cur = NULL;
    int n;

    DAT_004c4468.next = &DAT_004c5ca0;
    memcpy(&DAT_004c4468.footprint, &LogFlumeFootprint, sizeof(struct Footprint));
    DAT_004c4468.footprint.x1 = LogFlumeFootprint.x1 - 1;
    DAT_004c4468.footprint.y1 = DAT_004c4468.footprint.y1 - 1;
    DAT_004c5ca0.next = NULL;
    DAT_004c5ca0.footprint = DAT_004c4468.footprint;
    for (n = 2; n != 0; n--) {
        if (cur == NULL) {
            cur = &DAT_004c4468;
        } else {
            cur = &DAT_004c5ca0;
        }
        if (flags & 1) {
            cur->tile_x = res[1];
            cur->tile_y = res[2];
            flags &= ~1;
        } else if (flags & 2) {
            cur->tile_x = res[3];
            cur->tile_y = res[4];
            flags &= ~2;
        } else if (flags & 4) {
            cur->tile_x = res[5];
            cur->tile_y = res[6];
            flags &= ~4;
        } else if (flags & 8) {
            cur->tile_x = res[7];
            cur->tile_y = res[8];
            flags &= ~8;
        }
        FUN_0045f460(cur);
    }
}

// FUNCTION: LEGOLAND 0x0040d520
void FUN_0040d520(struct FlumeEntry **list, struct Cursor *first) {
    int flags;
    struct Cursor *cur;
    int i;

    cur = NULL;
    flags = FUN_00409410((unsigned int *)list);
    DAT_004ca5b0.next = NULL;
    DAT_004c1260.next = NULL;
    first->next = &DAT_004ca5b0;
    i = 0;
    do {
        if (cur == NULL) {
            cur = &DAT_004ca5b0;
        } else {
            cur->next = &DAT_004c1260;
            cur = &DAT_004c1260;
        }
        if (flags & 1) {
            struct Footprint *fp;
            TileId t;
            FUN_0040d090(list[0], &fp, &t);
            cur->tile_x = t.pos.x;
            cur->tile_y = t.pos.y;
            cur->footprint = *fp;
            flags &= ~1;
        } else if (flags & 4) {
            struct Footprint *fp;
            TileId t;
            FUN_0040d090(list[1], &fp, &t);
            cur->tile_x = t.pos.x;
            cur->tile_y = t.pos.y;
            cur->footprint = *fp;
            flags &= ~4;
        } else if (flags & 0x10) {
            struct Footprint *fp;
            TileId t;
            FUN_0040d090(list[2], &fp, &t);
            cur->tile_x = t.pos.x;
            cur->tile_y = t.pos.y;
            cur->footprint = *fp;
            flags &= ~0x10;
        } else if (flags & 0x40) {
            struct Footprint *fp;
            TileId t;
            FUN_0040d090(list[3], &fp, &t);
            cur->tile_x = t.pos.x;
            cur->tile_y = t.pos.y;
            cur->footprint = *fp;
            flags &= ~0x40;
        }
        cur->field_1828 = 0x2010;
        FUN_0045f460(cur);
    } while (flags != 0 && ++i < 2);
}

// FUNCTION: LEGOLAND 0x0040d6f0
unsigned int FUN_0040d6f0(struct CursorSource *param_1, unsigned int param_2, unsigned int param_3, unsigned int *param_4, FlumeCallback param_5, int (*param_6)(unsigned int *)) {
    union {
        struct FlumeXY t;
        unsigned int *list;
    } u;

    unsigned int buf[9];

    struct MapRect rect;
    int cost;
    unsigned int key;
    int r;

    EditCursor.footprint = *(struct Footprint *)param_4;
    ScreenToMapRef((int *)param_2, &EditCursor.tile_x, param_3);
    EditCursor.field_1830 = 0;
    FUN_0045f460(&EditCursor);
    ValidateCursor(&EditCursor, (unsigned int)param_1);
    u.t.x = (unsigned char)EditCursor.tile_x;
    u.t.y = (unsigned char)EditCursor.tile_y;
    param_5(u.t, buf);
    FUN_0040d420(buf);
    EditCursor.next = &DAT_004c4468;
    cost = GetObjCost((struct Ride *)param_1);
    if (GetBrickCount() < cost) {
        FUN_0045f480(&EditCursor, 2);
    }
    if (FUN_0045f4b0(&EditCursor)) {
        u.t.x = (unsigned char)EditCursor.tile_x;
        u.t.y = (unsigned char)EditCursor.tile_y;
        param_5(u.t, buf);
        FUN_0040cf10((struct InputBuffer *)buf, &u.list);
        FUN_0040cfa0((struct StateNode * (*)[4]) u.list);
        if (FUN_0040cf30(u.list) == 0) {
            FUN_0045f480(&EditCursor, 0xe);
        } else {
            FUN_0045f460(&EditCursor);
            key = FUN_0040cf80((struct Slot **)u.list);
            FUN_0040cf50(key, (struct Slot **)u.list);
            if (FUN_0040cf30(u.list) == 0) {
                FUN_0045f480(&EditCursor, 0xe);
            } else if (param_6(u.list)) {
                FUN_0045f460(&EditCursor);
                FUN_0040d520((struct FlumeEntry **)u.list, EditCursor.next->next);
            } else {
                FUN_0045f480(&EditCursor, 0xd);
            }
        }
    }
    if (FUN_0045f4b0(&EditCursor)) {
        int cx = EditCursor.tile_x;
        int cy = EditCursor.tile_y;
        rect.x0 = EditCursor.footprint.x0 + cx;
        rect.y0 = EditCursor.footprint.y0 + cy;
        rect.x1 = EditCursor.footprint.x1 + cx;
        rect.y1 = EditCursor.footprint.y1 + cy;
        r = CheckForPeople(&rect);
        switch (r) {
        case -1:
            FUN_0045f480(&EditCursor, 4);
            break;
        case 1:
            FUN_0045f480(&EditCursor, 3);
            break;
        }
    }
    FUN_0045f4d0(&EditCursor);
}

// FUNCTION: LEGOLAND 0x0040d900
void FUN_0040d900(unsigned int param_1, unsigned int *param_2, int param_3, void (*param_4)(), FlumeCallback param_5, void (*param_6)(struct EdgeNode *, int *)) {
    struct FlumeEntry *entry = FUN_00409010();
    unsigned int buf[9];
    unsigned int *list;
    int coords[4];
    unsigned int key;

    if (entry != NULL) {
        entry->tile.pos.x = (unsigned char)param_1;
        entry->tile.pos.y = ((unsigned char *)&param_1)[1];
        entry->ride = ((Element *)param_3)->ride;
        entry->link28 = NULL;
        ((void (*)(unsigned int, unsigned int *))param_5)(param_1, buf);
        FUN_0040cf10((struct InputBuffer *)buf, &list);
        FUN_0040cfa0((struct StateNode * (*)[4]) list);
        key = FUN_0040cf80((struct Slot **)list);
        FUN_004119a0((struct ParticleEmitter *)key, 3);
        entry->parent = (struct FlumeEntry *)key;
        FUN_0040cf50(key, (struct Slot **)list);
        ((void (*)(struct FlumeEntry *))param_4)(entry);
        FUN_004091f0((struct Node *)key, (struct ListNode *)entry);
        coords[1] = ((unsigned char *)&param_1)[1];
        coords[0] = (unsigned char)param_1;
        memcpy(&((Element *)param_3)->ride->footprint, param_2, sizeof(struct Footprint));
        AddBasicObject((Element *)param_3, coords);
        param_6((struct EdgeNode *)entry, coords);
        FUN_0040a080((struct Node **)coords, (struct Node **)list);
        FUN_00409a90((void **)coords, (struct StateNode **)list);
    }
}

// FUNCTION: LEGOLAND 0x0040da10
void FUN_0040da10(struct Context *a, struct LinkList *list) {
    struct LinkNode *node;

    if (list->field_0 != NULL) {
        node = list->field_0;
        if (a->var_2c == 0) {
            if (node->field_8 != a && node->field_c != a) {
                list->field_0 = NULL;
            }
        } else {
            if (node->field_8 != a->var_30 && node->field_c != a->var_30 &&
                node->field_8 != a->var_34 && node->field_c != a->var_34) {
                list->field_0 = NULL;
            }
        }
    }

    if (list->field_4 != NULL) {
        node = list->field_4;
        if (a->var_2c == 0) {
            if (node->field_8 != a && node->field_c != a) {
                list->field_4 = NULL;
            }
        } else {
            if (node->field_8 != a->var_30 && node->field_c != a->var_30 &&
                node->field_8 != a->var_34 && node->field_c != a->var_34) {
                list->field_4 = NULL;
            }
        }
    }

    if (list->field_c != NULL) {
        node = list->field_c;
        if (a->var_2c == 0) {
            if (node->field_8 != a && node->field_c != a) {
                list->field_c = NULL;
            }
        } else {
            if (node->field_8 != a->var_30 && node->field_c != a->var_30 &&
                node->field_8 != a->var_34 && node->field_c != a->var_34) {
                list->field_c = NULL;
            }
        }
    }

    if (list->field_8 != NULL) {
        node = list->field_8;
        if (a->var_2c == 0) {
            if (node->field_8 != a && node->field_c != a) {
                list->field_8 = NULL;
            }
        } else {
            if (node->field_8 != a->var_30 && node->field_c != a->var_30 &&
                node->field_8 != a->var_34 && node->field_c != a->var_34) {
                list->field_8 = NULL;
            }
        }
    }
}

// FUNCTION: LEGOLAND 0x0040db00
void FUN_0040db00(unsigned int param_1, unsigned int param_2, unsigned int param_3, FlumeCallback param_4) {
    struct FlumeEntry *entry = FindFlumeSubEntryByTile((TileId *)&param_2);
    struct Footprint *fp;
    TileId t;
    struct InputBuffer buf;
    unsigned int *list;

    if (entry != NULL) {
        FUN_004119a0((struct ParticleEmitter *)entry->parent, -3);
        FUN_0040d090(entry, &fp, &t);
        ((struct Cursor *)param_3)->footprint = *fp;
        StandardRemoveObject((Element *)param_1, t, (struct Cursor *)param_3);
        param_4(*(struct FlumeXY *)&t, (unsigned int *)&buf);
        FUN_0040cf10(&buf, &list);
        FUN_0040da10((struct Context *)entry, (struct LinkList *)list);
        FUN_0040a2a0(entry, (struct StateNode **)list);
        FUN_00409270((struct Node *)entry->parent, (struct Node *)entry);
    }
}

// FUNCTION: LEGOLAND 0x0040dbb0
void LogFlumeTrackSetEditMode(void) {
    unsigned int local[5];
    unsigned int v = LogFlumeFootprint.x1;

    memcpy(local, &LogFlumeFootprint, sizeof(local));
    local[2] = v - 1;
    local[3] = local[3] - 1;
    FUN_0040d3b0(LogFlumeTrackRide, local);
}

// FUNCTION: LEGOLAND 0x0040dc00
void FUN_0040dc00(struct FlumeEntry *entry) {
    unsigned int w = LogFlumeFootprint.x1 - LogFlumeFootprint.x0;
    unsigned int h = LogFlumeFootprint.y1 - LogFlumeFootprint.y0;
    struct FlumeXY t;
    struct FlumeEntry *n;
    struct FlumeEntry *prev;

    t.x = entry->tile.pos.x;
    t.y = entry->tile.pos.y;

    if (entry != NULL) {
        entry->submode = DAT_004c2af4;
        switch (DAT_004c2af4) {
        case 0:
            t.x += DAT_004c445c->var_3c[0];
            t.y += DAT_004c445c->var_3c[1];
            t.x += 3;
            n = FUN_00409010();
            if (n != NULL) {
                n->mode = 3;
                n->submode = 0;
                n->ride = LogFlumeTrackRide;
                n->slotset = entry->slotset;
                n->flags10d |= 4;
                n->link28 = entry;
                n->tile.pos.x = t.x;
                n->tile.pos.y = t.y;
            }
            AppendListNode((struct Node *)entry, (struct ListNode *)n);
            entry->link30 = n;
            prev = n;
            t.y += h;
            n = FUN_00409010();
            if (n != NULL) {
                n->mode = 1;
                n->submode = 0;
                n->slotset = entry->slotset;
                n->link28 = entry;
                n->ride = LogFlumeTrackRide;
                n->flags10d |= 4;
                n->tile.pos.x = t.x;
                n->tile.pos.y = t.y;
            }
            AppendListNode((struct Node *)entry, (struct ListNode *)n);
            LinkNodeAfter((struct Node *)prev, (struct Node *)n);
            prev = n;
            t.y += h;
            n = FUN_00409010();
            if (n != NULL) {
                n->mode = 2;
                n->submode = 0;
                n->slotset = entry->slotset;
                n->link28 = entry;
                n->ride = LogFlumeTrackRide;
                n->flags10d |= 4;
                n->tile.pos.x = t.x;
                n->tile.pos.y = t.y;
            }
            AppendListNode((struct Node *)entry, (struct ListNode *)n);
            LinkNodeAfter((struct Node *)prev, (struct Node *)n);
            prev = n;
            t.x += w;
            n = FUN_00409010();
            if (n != NULL) {
                n->mode = 1;
                n->submode = 1;
                n->slotset = entry->slotset;
                n->link28 = entry;
                n->ride = LogFlumeTrackRide;
                n->flags10d |= 4;
                n->tile.pos.x = t.x;
                n->tile.pos.y = t.y;
            }
            AppendListNode((struct Node *)entry, (struct ListNode *)n);
            LinkNodeAfter((struct Node *)prev, (struct Node *)n);
            prev = n;
            t.x += w;
            n = FUN_00409010();
            if (n != NULL) {
                n->mode = 3;
                n->submode = 1;
                n->slotset = entry->slotset;
                n->link28 = entry;
                n->ride = LogFlumeTrackRide;
                n->flags10d |= 4;
                n->tile.pos.x = t.x;
                n->tile.pos.y = t.y;
            }
            break;
        case 1:
            t.x += DAT_004c2aa0->var_3c[0];
            t.y += DAT_004c2aa0->var_3c[1];
            t.x += DAT_004c2aa0->var_3c[2] - DAT_004c2aa0->var_3c[0] - w + 1;
            t.y += 2;
            n = FUN_00409010();
            if (n != NULL) {
                n->mode = 3;
                n->submode = 1;
                n->slotset = entry->slotset;
                n->link28 = entry;
                n->ride = LogFlumeTrackRide;
                n->flags10d |= 4;
                n->tile.pos.x = t.x;
                n->tile.pos.y = t.y;
            }
            AppendListNode((struct Node *)entry, (struct ListNode *)n);
            prev = n;
            entry->link30 = n;
            t.x -= w;
            n = FUN_00409010();
            if (n != NULL) {
                n->mode = 1;
                n->submode = 1;
                n->slotset = entry->slotset;
                n->link28 = entry;
                n->ride = LogFlumeTrackRide;
                n->flags10d |= 4;
                n->tile.pos.x = t.x;
                n->tile.pos.y = t.y;
            }
            AppendListNode((struct Node *)entry, (struct ListNode *)n);
            LinkNodeAfter((struct Node *)prev, (struct Node *)n);
            prev = n;
            t.x -= w;
            n = FUN_00409010();
            if (n != NULL) {
                n->mode = 2;
                n->submode = 1;
                n->slotset = entry->slotset;
                n->link28 = entry;
                n->ride = LogFlumeTrackRide;
                n->flags10d |= 4;
                n->tile.pos.x = t.x;
                n->tile.pos.y = t.y;
            }
            AppendListNode((struct Node *)entry, (struct ListNode *)n);
            LinkNodeAfter((struct Node *)prev, (struct Node *)n);
            prev = n;
            t.y += h;
            n = FUN_00409010();
            if (n != NULL) {
                n->mode = 1;
                n->submode = 0;
                n->slotset = entry->slotset;
                n->link28 = entry;
                n->ride = LogFlumeTrackRide;
                n->flags10d |= 4;
                n->tile.pos.x = t.x;
                n->tile.pos.y = t.y;
            }
            AppendListNode((struct Node *)entry, (struct ListNode *)n);
            LinkNodeAfter((struct Node *)prev, (struct Node *)n);
            prev = n;
            t.y += h;
            n = FUN_00409010();
            if (n != NULL) {
                n->mode = 3;
                n->submode = 2;
                n->slotset = entry->slotset;
                n->link28 = entry;
                n->ride = LogFlumeTrackRide;
                n->flags10d |= 4;
                n->tile.pos.x = t.x;
                n->tile.pos.y = t.y;
            }
            break;
        case 2:
            t.x += DAT_004c2b0c->var_3c[0];
            t.y += DAT_004c2b0c->var_3c[1];
            t.x += 4;
            t.y += DAT_004c2b0c->var_3c[3] - DAT_004c2b0c->var_3c[1] - h + 1;
            n = FUN_00409010();
            if (n != NULL) {
                n->mode = 3;
                n->submode = 2;
                n->slotset = entry->slotset;
                n->link28 = entry;
                n->ride = LogFlumeTrackRide;
                n->flags10d |= 4;
                n->tile.pos.x = t.x;
                n->tile.pos.y = t.y;
            }
            AppendListNode((struct Node *)entry, (struct ListNode *)n);
            prev = n;
            entry->link30 = n;
            t.y -= h;
            n = FUN_00409010();
            if (n != NULL) {
                n->mode = 1;
                n->submode = 0;
                n->slotset = entry->slotset;
                n->link28 = entry;
                n->ride = LogFlumeTrackRide;
                n->flags10d |= 4;
                n->tile.pos.x = t.x;
                n->tile.pos.y = t.y;
            }
            AppendListNode((struct Node *)entry, (struct ListNode *)n);
            LinkNodeAfter((struct Node *)prev, (struct Node *)n);
            prev = n;
            t.y -= h;
            n = FUN_00409010();
            if (n != NULL) {
                n->mode = 2;
                n->submode = 2;
                n->slotset = entry->slotset;
                n->link28 = entry;
                n->ride = LogFlumeTrackRide;
                n->flags10d |= 4;
                n->tile.pos.x = t.x;
                n->tile.pos.y = t.y;
            }
            AppendListNode((struct Node *)entry, (struct ListNode *)n);
            LinkNodeAfter((struct Node *)prev, (struct Node *)n);
            prev = n;
            t.x -= w;
            n = FUN_00409010();
            if (n != NULL) {
                n->mode = 1;
                n->submode = 1;
                n->slotset = entry->slotset;
                n->link28 = entry;
                n->ride = LogFlumeTrackRide;
                n->flags10d |= 4;
                n->tile.pos.x = t.x;
                n->tile.pos.y = t.y;
            }
            AppendListNode((struct Node *)entry, (struct ListNode *)n);
            LinkNodeAfter((struct Node *)prev, (struct Node *)n);
            prev = n;
            t.x -= w;
            n = FUN_00409010();
            if (n != NULL) {
                n->mode = 3;
                n->submode = 3;
                n->slotset = entry->slotset;
                n->link28 = entry;
                n->ride = LogFlumeTrackRide;
                n->flags10d |= 4;
                n->tile.pos.x = t.x;
                n->tile.pos.y = t.y;
            }
            break;
        case 3:
            t.x += DAT_004c74d4->var_3c[0];
            t.y += DAT_004c74d4->var_3c[1] + 4;
            n = FUN_00409010();
            if (n != NULL) {
                n->mode = 3;
                n->submode = 3;
                n->slotset = entry->slotset;
                n->link28 = entry;
                n->ride = LogFlumeTrackRide;
                n->flags10d |= 4;
                n->tile.pos.x = t.x;
                n->tile.pos.y = t.y;
            }
            AppendListNode((struct Node *)entry, (struct ListNode *)n);
            entry->link30 = n;
            prev = n;
            t.x += w;
            n = FUN_00409010();
            if (n != NULL) {
                n->mode = 1;
                n->submode = 1;
                n->slotset = entry->slotset;
                n->link28 = entry;
                n->ride = LogFlumeTrackRide;
                n->flags10d |= 4;
                n->tile.pos.x = t.x;
                n->tile.pos.y = t.y;
            }
            AppendListNode((struct Node *)entry, (struct ListNode *)n);
            LinkNodeAfter((struct Node *)prev, (struct Node *)n);
            prev = n;
            t.x += w;
            n = FUN_00409010();
            if (n != NULL) {
                n->mode = 2;
                n->submode = 3;
                n->slotset = entry->slotset;
                n->link28 = entry;
                n->ride = LogFlumeTrackRide;
                n->flags10d |= 4;
                n->tile.pos.x = t.x;
                n->tile.pos.y = t.y;
            }
            AppendListNode((struct Node *)entry, (struct ListNode *)n);
            LinkNodeAfter((struct Node *)prev, (struct Node *)n);
            prev = n;
            t.y -= h;
            n = FUN_00409010();
            if (n != NULL) {
                n->mode = 1;
                n->submode = 0;
                n->slotset = entry->slotset;
                n->link28 = entry;
                n->ride = LogFlumeTrackRide;
                n->flags10d |= 4;
                n->tile.pos.x = t.x;
                n->tile.pos.y = t.y;
            }
            AppendListNode((struct Node *)entry, (struct ListNode *)n);
            LinkNodeAfter((struct Node *)prev, (struct Node *)n);
            prev = n;
            t.y -= h;
            n = FUN_00409010();
            if (n != NULL) {
                n->mode = 3;
                n->submode = 0;
                n->slotset = entry->slotset;
                n->link28 = entry;
                n->ride = LogFlumeTrackRide;
                n->flags10d |= 4;
                n->tile.pos.x = t.x;
                n->tile.pos.y = t.y;
            }
            break;
        default:
            return;
        }
        AppendListNode((struct Node *)entry, (struct ListNode *)n);
        LinkNodeAfter((struct Node *)prev, (struct Node *)n);
        entry->link34 = n;
    }
}

// FUNCTION: LEGOLAND 0x0040e340
void FUN_0040e340(struct EdgeNode *node, int *out) {
    out[0] = 0;
    out[1] = 0;
    out[2] = 0;
    out[3] = 0;
    switch (node->mode_1c) {
    case 0:
        out[0] = node->a_30;
        out[1] = node->b_34;
        break;
    case 1:
        out[1] = node->a_30;
        out[2] = node->b_34;
        break;
    case 2:
        out[2] = node->a_30;
        out[3] = node->b_34;
        break;
    case 3:
        out[3] = node->a_30;
        out[0] = node->b_34;
        break;
    }
}

// FUNCTION: LEGOLAND 0x0040e3b0
int FUN_0040e3b0(unsigned int *ctx) {
    int flags = FUN_00409410(ctx);

    switch (DAT_004c2af4) {
    case 0:
        if (flags == 5 || flags == 1 || flags == 4) {
            return 1;
        }
        break;
    case 1:
        if (flags == 0x14 || flags == 4 || flags == 0x10) {
            return 1;
        }
        break;
    case 2:
        if (flags == 0x50 || flags == 0x10 || flags == 0x40) {
            return 1;
        }
        break;
    case 3:
        if (flags == 0x41 || flags == 0x40 || flags == 1) {
            return 1;
        }
        break;
    }
    return 0;
}

// FUNCTION: LEGOLAND 0x0040e440
void FUN_0040e440(struct FlumeXY p, unsigned int *result) {
    int px;
    unsigned int v3;
    int v1;
    unsigned int h = LogFlumeFootprint.y1 - LogFlumeFootprint.y0;
    unsigned int w = LogFlumeFootprint.x1 - LogFlumeFootprint.x0;

    switch (DAT_004c2af4) {
    case 0:
        p.x = p.x + ((unsigned char)DAT_004c445c->var_3c[0]);
        p.y += (unsigned char)DAT_004c445c->var_3c[1];
        result[0] = 3;
        result[1] = p.x + 3;
        result[2] = p.y;
        result[3] = DAT_004c445c->var_3c[2] - DAT_004c445c->var_3c[0] - w + p.x + 1;
        result[4] = p.y + 4;
        break;
    case 1:
        p.x += (unsigned char)DAT_004c2aa0->var_3c[0];
        v1 = DAT_004c2aa0->var_3c[1];
        result[0] = 6;
        p.y += (unsigned char)v1;
        result[3] = DAT_004c2aa0->var_3c[2] - DAT_004c2aa0->var_3c[0] - w + p.x + 1;
        result[5] = 2 + p.x;
        result[4] = p.y + 2;
        result[6] = DAT_004c2aa0->var_3c[3] - DAT_004c2aa0->var_3c[1] - h + p.y + 1;
        break;
    case 2:
        p.x += (unsigned char)DAT_004c2b0c->var_3c[0];
        p.y = p.y + ((unsigned char)DAT_004c2b0c->var_3c[1]);
        result[0] = 12;
        px = p.x;
        result[5] = 4 + px;
        v3 = DAT_004c2b0c->var_3c[3];
        result[7] = p.x;
        result[6] = v3 - DAT_004c2b0c->var_3c[1] - h + p.y + 1;
        result[8] = p.y + 3;
        break;
    case 3:
        p.x = p.x + ((unsigned char)DAT_004c74d4->var_3c[0]);
        p.y += (unsigned char)DAT_004c74d4->var_3c[1];
        result[7] = p.x;
        result[1] = p.x + 4;
        result[0] = 9;
        result[8] = p.y + 4;
        result[2] = p.y;
        break;
    }
}

// FUNCTION: LEGOLAND 0x0040e630
void LogFlumeSpecialCorner1SetEditMode(void) {
    unsigned int local[5];
    memcpy(local, DAT_004c445c->var_3c, 20);
    FUN_0040d3b0(DAT_004c445c, local);
}

// FUNCTION: LEGOLAND 0x0040e660
void LogFlumeSpecialCorner2SetEditMode(void) {
    struct CursorSource *src = DAT_004c2aa0;
    unsigned int local[5];
    memcpy(local, src->var_3c, sizeof(local));
    FUN_0040d3b0(src, local);
}

// FUNCTION: LEGOLAND 0x0040e690
void LogFlumeSpecialCorner3SetEditMode(void) {
    struct CursorSource *src = DAT_004c2b0c;
    unsigned int local[5];
    memcpy(local, src->var_3c, sizeof(local));
    FUN_0040d3b0(src, local);
}

// FUNCTION: LEGOLAND 0x0040e6c0
void LogFlumeSpecialCorner4SetEditMode(void) {
    struct CursorSource *src = DAT_004c74d4;
    unsigned int local[5];
    memcpy(local, src->var_3c, sizeof(local));
    FUN_0040d3b0(src, local);
}

// FUNCTION: LEGOLAND 0x0040e6f0
void LogFlumeSpecialCorner1CalcCursor(unsigned int param_1, unsigned int param_2, unsigned int param_3) {
    struct CursorSource *src = DAT_004c445c;
    unsigned int local[5];
    DAT_004c2af4 = 0;
    memcpy(local, src->var_3c, 20);
    FUN_0040d6f0(src, param_2, param_3, local, FUN_0040e440, FUN_0040e3b0);
}

// FUNCTION: LEGOLAND 0x0040e740
void LogFlumeSpecialCorner2CalcCursor(unsigned int param_1, unsigned int param_2, unsigned int param_3) {
    struct CursorSource *src = DAT_004c2aa0;
    unsigned int local[5];
    DAT_004c2af4 = 1;
    memcpy(local, src->var_3c, 20);
    FUN_0040d6f0(src, param_2, param_3, local, FUN_0040e440, FUN_0040e3b0);
}

// FUNCTION: LEGOLAND 0x0040e790
void LogFlumeSpecialCorner3CalcCursor(unsigned int param_1, unsigned int param_2, unsigned int param_3) {
    struct CursorSource *src = DAT_004c2b0c;
    unsigned int local[5];
    DAT_004c2af4 = 2;
    memcpy(local, src->var_3c, 20);
    FUN_0040d6f0(src, param_2, param_3, local, FUN_0040e440, FUN_0040e3b0);
}

// FUNCTION: LEGOLAND 0x0040e7e0
void LogFlumeSpecialCorner4CalcCursor(unsigned int param_1, unsigned int param_2, unsigned int param_3) {
    struct CursorSource *src = DAT_004c74d4;
    unsigned int local[5];
    DAT_004c2af4 = 3;
    memcpy(local, src->var_3c, 20);
    FUN_0040d6f0(src, param_2, param_3, local, FUN_0040e440, FUN_0040e3b0);
}

// FUNCTION: LEGOLAND 0x0040e830
void LogFlumeSpecialCorner1DCalcCursor(unsigned int param_1, struct Point *param_2) {
    DAT_004c2af4 = 0;
    FUN_0040d2d0(param_2);
}

// FUNCTION: LEGOLAND 0x0040e850
void LogFlumeSpecialCorner2DCalcCursor(unsigned int param_1, struct Point *param_2) {
    DAT_004c2af4 = 1;
    FUN_0040d2d0(param_2);
}

// FUNCTION: LEGOLAND 0x0040e870
void LogFlumeSpecialCorner3DCalcCursor(unsigned int dummy, struct Point *param_1) {
    DAT_004c2af4 = 2;
    FUN_0040d2d0(param_1);
}

// FUNCTION: LEGOLAND 0x0040e890
void LogFlumeSpecialCorner4DCalcCursor(unsigned int param_1, struct Point *param_2) {
    DAT_004c2af4 = 3;
    FUN_0040d2d0(param_2);
}

// FUNCTION: LEGOLAND 0x0040e8b0
void LogFlumeSpecialCorner1LoadSprites(struct Obj *obj_ptr) {
    struct ResA *resA;
    struct ResB *resB;

    DAT_004c445c = (struct CursorSource *)obj_ptr->ride;
    resA = (struct ResA *)DAT_004c445c;
    resA->flags_1c |= 0x400;

    resA = (struct ResA *)DAT_004c445c;
    if (resA != NULL) {
        resB = resA->ptr_64;
        if (resB != NULL) {
            resB->flags_10 |= 0x2000;
        }
    }

    DAT_004c2b98 = (unsigned int)obj_ptr;

    // STRING: LEGOLAND 0x004b4a58
    LogFlumeFc1M1Sprite = LoadSprite("fc1_m1.lls", 1);
    // STRING: LEGOLAND 0x004b4a4c
    LogFlumeFc1M2Sprite = LoadSprite("fc1_m2.lls", 1);

    FlumeStages_004b47f0[0].sprite = LogFlumeFc1M1Sprite;
    FlumeStages_004b47f0[1].sprite = LogFlumeFc1M2Sprite;
}

// FUNCTION: LEGOLAND 0x0040e920
void LogFlumeSpecialCorner2LoadSprites(struct Obj *obj_ptr) {
    struct ResA *resA;
    struct ResB *resB;

    DAT_004c2aa0 = (struct CursorSource *)obj_ptr->ride;
    resA = (struct ResA *)DAT_004c2aa0;
    resA->flags_1c |= 0x400;

    resA = (struct ResA *)DAT_004c2aa0;
    if (resA != NULL) {
        resB = resA->ptr_64;
        if (resB != NULL) {
            resB->flags_10 |= 0x2000;
        }
    }

    DAT_004cbe10 = obj_ptr;
    // STRING: LEGOLAND 0x004b4a64
    LogFlumeFc2M1Sprite = LoadSprite("fc2_m1.lls", 1);
}

// FUNCTION: LEGOLAND 0x0040e970
void LogFlumeSpecialCorner3LoadSprites(struct Obj *obj_ptr) {
    struct ResA *resA;
    struct ResB *resB;

    DAT_004c2b0c = (struct CursorSource *)obj_ptr->ride;
    resA = (struct ResA *)DAT_004c2b0c;
    resA->flags_1c |= 0x400;

    resA = (struct ResA *)DAT_004c2b0c;
    if (resA != NULL) {
        resB = resA->ptr_64;
        if (resB != NULL) {
            resB->flags_10 |= 0x2000;
        }
    }

    DAT_004c8d50 = (unsigned int)obj_ptr;

    // STRING: LEGOLAND 0x004b4a7c
    LogFlumeFc3M1Sprite = LoadSprite("fc3_m1.lls", 1);
    // STRING: LEGOLAND 0x004b4a70
    LogFlumeFc3M2Sprite = LoadSprite("fc3_m2.lls", 1);

    FlumeStages_004b4810[0].sprite = LogFlumeFc3M1Sprite;
    FlumeStages_004b4810[1].sprite = LogFlumeFc3M2Sprite;
}

// FUNCTION: LEGOLAND 0x0040e9e0
void LogFlumeSpecialCorner4LoadSprites(struct Obj *obj_ptr) {
    struct ResA *resA;
    struct ResB *resB;

    DAT_004c74d4 = (struct CursorSource *)obj_ptr->ride;
    resA = (struct ResA *)DAT_004c74d4;
    resA->flags_1c |= 0x400;
    DAT_004c2ba0 = obj_ptr;

    resA = (struct ResA *)DAT_004c74d4;
    if (resA != NULL) {
        resB = resA->ptr_64;
        if (resB != NULL) {
            resB->flags_10 |= 0x2000;
        }
    }

    // STRING: LEGOLAND 0x004b4a88
    LogFlumeFc4MSprite = LoadSprite("fc4_m.lls", 1);
}

// FUNCTION: LEGOLAND 0x0040ea30
void LogFlumeSpecialCorner1UnloadSprites(void) {
    if (LogFlumeFc1M1Sprite != 0) {
        KillSprite(LogFlumeFc1M1Sprite);
    }
    if (LogFlumeFc1M2Sprite != 0) {
        KillSprite(LogFlumeFc1M2Sprite);
    }
}

// FUNCTION: LEGOLAND 0x0040ea60
void LogFlumeSpecialCorner2UnloadSprites(void) {
    if (LogFlumeFc2M1Sprite != NULL) {
        KillSprite(LogFlumeFc2M1Sprite);
    }
}

// FUNCTION: LEGOLAND 0x0040ea80
void LogFlumeSpecialCorner3UnloadSprites(void) {
    if (LogFlumeFc3M1Sprite != NULL) {
        KillSprite(LogFlumeFc3M1Sprite);
    }
    if (LogFlumeFc3M2Sprite != NULL) {
        KillSprite(LogFlumeFc3M2Sprite);
    }
}

// FUNCTION: LEGOLAND 0x0040eab0
void LogFlumeSpecialCorner4UnloadSprites(void) {
    if (LogFlumeFc4MSprite != NULL) {
        KillSprite(LogFlumeFc4MSprite);
    }
}

// FUNCTION: LEGOLAND 0x0040ead0
void LogFlumeSpecialCorner1AddObject(unsigned int param_1, struct FlumeBytes *param_2) {
    unsigned int packed;
    unsigned int local[5];
    *((unsigned char *)&packed) = param_2->b0;
    *((unsigned char *)&packed + 1) = param_2->b4;
    memcpy(local, DAT_004c445c->var_3c, 20);
    DAT_004c2af4 = 0;
    FUN_0040d900(packed, local, (int)DAT_004c2b98, FUN_0040dc00, FUN_0040e440, FUN_0040e340);
}

// FUNCTION: LEGOLAND 0x0040eb40
void LogFlumeSpecialCorner2AddObject(unsigned int param_1, struct FlumeBytes *param_2) {
    unsigned int packed;
    unsigned int local[5];
    *((unsigned char *)&packed) = param_2->b0;
    *((unsigned char *)&packed + 1) = param_2->b4;
    memcpy(local, DAT_004c2aa0->var_3c, 20);
    DAT_004c2af4 = 1;
    FUN_0040d900(packed, local, (int)DAT_004cbe10, FUN_0040dc00, FUN_0040e440, FUN_0040e340);
}

// FUNCTION: LEGOLAND 0x0040ebb0
void LogFlumeSpecialCorner3AddObject(unsigned int param_1, struct FlumeBytes *param_2) {
    unsigned int packed;
    unsigned int local[5];
    *((unsigned char *)&packed) = param_2->b0;
    *((unsigned char *)&packed + 1) = param_2->b4;
    memcpy(local, DAT_004c2b0c->var_3c, 20);
    DAT_004c2af4 = 2;
    FUN_0040d900(packed, local, (int)DAT_004c8d50, FUN_0040dc00, FUN_0040e440, FUN_0040e340);
}

// FUNCTION: LEGOLAND 0x0040ec20
void LogFlumeSpecialCorner4AddObject(unsigned int param_1, struct FlumeBytes *param_2) {
    unsigned int packed;
    unsigned int local[5];
    *((unsigned char *)&packed) = param_2->b0;
    *((unsigned char *)&packed + 1) = param_2->b4;
    memcpy(local, DAT_004c74d4->var_3c, 20);
    DAT_004c2af4 = 3;
    FUN_0040d900(packed, local, (int)DAT_004c2ba0, FUN_0040dc00, FUN_0040e440, FUN_0040e340);
}

// FUNCTION: LEGOLAND 0x0040ec90
void LogFlumeSpecialCorner1RemoveObject(unsigned int param_1, unsigned int param_2, unsigned int param_3) {
    DAT_004c2af4 = 0;
    FUN_0040db00(param_1, param_2, param_3, FUN_0040e440);
}

// FUNCTION: LEGOLAND 0x0040ecc0
unsigned int LogFlumeSpecialCorner2RemoveObject(unsigned int param_1, unsigned int param_2, unsigned int param_3) {
    DAT_004c2af4 = 1;
    FUN_0040db00(param_1, param_2, param_3, FUN_0040e440);
    return 0; /* [port] the original returned whatever the call left in eax; callers ignore it */
}

// FUNCTION: LEGOLAND 0x0040ecf0
unsigned int LogFlumeSpecialCorner3RemoveObject(unsigned int param_1, unsigned int param_2, unsigned int param_3) {
    DAT_004c2af4 = 2;
    FUN_0040db00(param_1, param_2, param_3, FUN_0040e440);
    return 0; /* [port] the original returned whatever the call left in eax; callers ignore it */
}

// FUNCTION: LEGOLAND 0x0040ed20
void LogFlumeSpecialCorner4RemoveObject(unsigned int param_1, unsigned int param_2, unsigned int param_3) {
    DAT_004c2af4 = 3;
    FUN_0040db00(param_1, param_2, param_3, FUN_0040e440);
}

// FUNCTION: LEGOLAND 0x0040ed50
struct RideSpriteInfo *FUN_0040ed50(struct FlumeRideArg *arg1, TileId arg2) {
    struct FlumeRideSrc *src = arg1->field_c;
    struct FlumeEntry *entry = FindFlumeSubEntryByTile(&arg2);
    if (entry != NULL) {
        FUN_0040cd70((struct PairHolder *)entry, 0);
        DAT_004cbe58.id = arg2.id;
        DAT_004cbe58.sprite = src->field_64;
        DAT_004cbe58.x = src->field_14;
        DAT_004cbe58.y = src->field_18;
        DAT_004cbe58.field_10 = 0;
    }
    return &DAT_004cbe58;
}

// FUNCTION: LEGOLAND 0x0040edb0
void RenderLogFlumeSpecialCorner1(Element *elem, unsigned int param_2, unsigned int param_3, TileId *tile) {
    struct Ride *ride = elem->ride;
    struct FlumeEntry *entry;
    struct LLS *lls;
    struct Point pos;
    int frame = 0;

    DAT_004c2af4 = 0;
    pos = GetScreenCoordsForObject(tile, ride);
    lls = (struct LLS *)GetLLSForSprite((struct SpriteLLS *)ride->layer);
    if (lls != NULL) {
        frame = lls->frame;
    }
    lls = (struct LLS *)GetLLSForSprite((struct SpriteLLS *)LogFlumeFc1M1Sprite);
    if (lls != NULL) {
        LLSSetFrame(lls, frame);
    }
    lls = (struct LLS *)GetLLSForSprite((struct SpriteLLS *)LogFlumeFc1M2Sprite);
    if (lls != NULL) {
        LLSSetFrame(lls, frame);
    }
    FUN_0040cdf0(tile);
    entry = FindFlumeSubEntryByTile(tile);
    if (entry != NULL) {
        FUN_0040b290(entry, pos.x, pos.y, (struct FlumeStageList *)&DAT_004b4808, 0);
        FUN_0040cd70((struct PairHolder *)entry, 1);
    }
}

// FUNCTION: LEGOLAND 0x0040ee60
void RenderLogFlumeSpecialCorner2(Element *elem, unsigned int param_2, unsigned int param_3, TileId *tile, unsigned int param_5, unsigned int clip) {
    struct Ride *ride = elem->ride;
    struct FlumeEntry *entry;
    struct LLS *lls;
    struct Point pos;
    int frame;

    DAT_004c2af4 = 1;
    entry = FindFlumeSubEntryByTile(tile);
    if (FUN_0040cdf0(tile)) {
        frame = 0;
        lls = (struct LLS *)GetLLSForSprite((struct SpriteLLS *)ride->layer);
        if (lls != NULL) {
            frame = lls->frame;
        }
        lls = (struct LLS *)GetLLSForSprite((struct SpriteLLS *)LogFlumeFc2M1Sprite);
        if (lls != NULL) {
            LLSSetFrame(lls, frame);
        }
        pos = GetScreenCoordsForObject(tile, ride);
        if (LogFlumeFc2M1Sprite != NULL) {
            PrintSprite(LogFlumeFc2M1Sprite, pos.x, pos.y, clip, 0);
        }
    }
    FUN_0040cd70((struct PairHolder *)entry, 1);
}

// FUNCTION: LEGOLAND 0x0040ef00
void RenderLogFlumeSpecialCorner3(Element *elem, unsigned int param_2, unsigned int param_3, TileId *tile) {
    struct Ride *ride = elem->ride;
    struct FlumeEntry *entry;
    struct LLS *lls;
    struct Point pos;
    int frame;

    DAT_004c2af4 = 2;
    frame = 0;
    lls = (struct LLS *)GetLLSForSprite((struct SpriteLLS *)ride->layer);
    if (lls != NULL) {
        frame = lls->frame;
    }
    lls = (struct LLS *)GetLLSForSprite((struct SpriteLLS *)LogFlumeFc3M1Sprite);
    if (lls != NULL) {
        LLSSetFrame(lls, frame);
    }
    lls = (struct LLS *)GetLLSForSprite((struct SpriteLLS *)LogFlumeFc3M2Sprite);
    if (lls != NULL) {
        LLSSetFrame(lls, frame);
    }
    pos = GetScreenCoordsForObject(tile, ride);
    FUN_0040cdf0(tile);
    entry = FindFlumeSubEntryByTile(tile);
    if (entry != NULL) {
        FUN_0040b290(entry, pos.x, pos.y, &DAT_004b4828, 1);
        FUN_0040cd70((struct PairHolder *)entry, 1);
    }
}

// FUNCTION: LEGOLAND 0x0040efb0
void RenderLogFlumeSpecialCorner4(Element *elem, unsigned int param_2, unsigned int param_3, TileId *tile, unsigned int param_5, unsigned int clip) {
    struct Ride *ride = elem->ride;
    struct FlumeEntry *entry;
    struct LLS *lls;
    struct Point pos;
    int frame;

    DAT_004c2af4 = 3;
    entry = FindFlumeSubEntryByTile(tile);
    if (FUN_0040cdf0(tile)) {
        frame = 0;
        lls = (struct LLS *)GetLLSForSprite((struct SpriteLLS *)ride->layer);
        if (lls != NULL) {
            frame = lls->frame;
        }
        lls = (struct LLS *)GetLLSForSprite((struct SpriteLLS *)LogFlumeFc4MSprite);
        if (lls != NULL) {
            LLSSetFrame(lls, frame);
        }
        pos = GetScreenCoordsForObject(tile, ride);
        if (LogFlumeFc4MSprite != NULL) {
            PrintSprite(LogFlumeFc4MSprite, pos.x, pos.y, clip, 0);
        }
        FUN_0040cd70((struct PairHolder *)entry, 1);
    }
}

static __inline struct FlumeNode *NewFlumeNode(struct FlumeNode *parent, int mode, int submode, unsigned char y, unsigned char x) {
    struct FlumeNode *node = FUN_00409010();
    if (node != NULL) {
        node->mode = mode;
        node->submode = submode;
        node->owner = parent->owner;
        node->entry = parent;
        node->ride = (struct Ride *)LogFlumeTrackRide;
        node->flags |= 4;
        node->tile.pos.x = x;
        node->tile.pos.y = y;
    }
    return node;
}

// FUNCTION: LEGOLAND 0x0040f050
void FUN_0040f050(struct FlumeNode *parent) {
    struct FlumeNode *node;
    struct FlumeNode *prev;
    TileId pos;
    int w = LogFlumeFootprint.x1 - LogFlumeFootprint.x0;
    int h = LogFlumeFootprint.y1 - LogFlumeFootprint.y0;

    pos.pos.x = ((struct Ride *)DAT_004cbe18)->footprint.x0 + parent->tile.pos.x + 6;
    pos.pos.y = ((struct Ride *)DAT_004cbe18)->footprint.y0 + parent->tile.pos.y;
    node = NewFlumeNode(parent, 3, 0, pos.pos.y, pos.pos.x);
    AppendListNode((struct Node *)parent, (struct ListNode *)node);
    parent->field_30 = (unsigned int)node;
    pos.pos.y += h;
    prev = NewFlumeNode(parent, 1, 0, pos.pos.y, pos.pos.x);
    AppendListNode((struct Node *)parent, (struct ListNode *)prev);
    LinkNodeAfter((struct Node *)node, (struct Node *)prev);
    pos.pos.y += h;
    node = NewFlumeNode(parent, 2, 3, pos.pos.y, pos.pos.x);
    AppendListNode((struct Node *)parent, (struct ListNode *)node);
    LinkNodeAfter((struct Node *)prev, (struct Node *)node);
    pos.pos.x -= w;
    prev = node;
    node = NewFlumeNode(parent, 1, 1, pos.pos.y, pos.pos.x);
    AppendListNode((struct Node *)parent, (struct ListNode *)node);
    LinkNodeAfter((struct Node *)prev, (struct Node *)node);
    pos.pos.x -= w;
    prev = node;
    node = NewFlumeNode(parent, 2, 1, pos.pos.y, pos.pos.x);
    AppendListNode((struct Node *)parent, (struct ListNode *)node);
    LinkNodeAfter((struct Node *)prev, (struct Node *)node);
    pos.pos.y += h;
    prev = node;
    node = NewFlumeNode(parent, 1, 0, pos.pos.y, pos.pos.x);
    AppendListNode((struct Node *)parent, (struct ListNode *)node);
    LinkNodeAfter((struct Node *)prev, (struct Node *)node);
    pos.pos.y += h;
    prev = node;
    node = NewFlumeNode(parent, 3, 2, pos.pos.y, pos.pos.x);
    AppendListNode((struct Node *)parent, (struct ListNode *)node);
    LinkNodeAfter((struct Node *)prev, (struct Node *)node);
    parent->field_34 = (unsigned int)node;
}

// FUNCTION: LEGOLAND 0x0040f300
void FUN_0040f300(struct FlumePos *src, struct FlumeRect *dst) {
    dst->var_0 = 0;
    dst->var_4 = 0;
    dst->var_8 = 0;
    dst->var_c = 0;
    dst->var_0 = src->var_30;
    dst->var_8 = src->var_34;
}

// FUNCTION: LEGOLAND 0x0040f330
int FUN_0040f330(unsigned int *param_1) {
    int flags = FUN_00409410(param_1);
    if (flags == 0x11 || flags == 0x1 || flags == 0x10) {
        return 1;
    }
    return 0;
}

// FUNCTION: LEGOLAND 0x0040f360
void FUN_0040f360(struct FlumeXY p, unsigned int *result) {
    unsigned int w = LogFlumeFootprint.y1 - LogFlumeFootprint.y0;

    p.x += (unsigned char)DAT_004cbe18->var_3c[0];
    p.y += (unsigned char)DAT_004cbe18->var_3c[1];
    result[0] = 5;
    result[1] = p.x + 6;
    result[2] = p.y;
    result[5] = p.x + 2;
    result[6] = DAT_004cbe18->var_3c[3] - DAT_004cbe18->var_3c[1] - w + p.y + 1;
}

// FUNCTION: LEGOLAND 0x0040f3e0
void LogFlumeTunnelLoadSprites(struct Obj *obj_ptr) {
    struct ResA *resA;
    struct ResB *resB;

    DAT_004cbe18 = (struct CursorSource *)obj_ptr->ride;
    resA = (struct ResA *)DAT_004cbe18;
    resA->flags_1c |= 0x400;
    DAT_004cbe48 = obj_ptr;

    resA = (struct ResA *)DAT_004cbe18;
    if (resA != NULL) {
        resB = resA->ptr_64;
        if (resB != NULL) {
            resB->flags_10 |= 0x2000;
        }
    }

    // STRING: LEGOLAND 0x004b4a94
    LogFlumeTunelMSprite = LoadSprite("tunel_m.lls", 1);
}

// FUNCTION: LEGOLAND 0x0040f430
void LogFlumeTunnelUnloadSprites(void) {
    if (LogFlumeTunelMSprite != NULL) {
        KillSprite(LogFlumeTunelMSprite);
    }
}

// FUNCTION: LEGOLAND 0x0040f450
void RenderLogFlumeTunnel(Element *elem, unsigned int param_2, unsigned int param_3, TileId *tile, unsigned int param_5, unsigned int clip) {
    struct Ride *ride = elem->ride;
    struct FlumeEntry *entry;
    struct LLS *lls;
    struct Point pos;
    int frame;

    entry = FindFlumeSubEntryByTile(tile);
    if (FUN_0040cdf0(tile)) {
        if (LogFlumeTunelMSprite != NULL) {
            lls = (struct LLS *)GetLLSForSprite((struct SpriteLLS *)ride->layer);
            frame = lls != NULL ? lls->frame : frame;
            lls = (struct LLS *)GetLLSForSprite((struct SpriteLLS *)LogFlumeTunelMSprite);
            if (lls != NULL) {
                LLSSetFrame(lls, frame);
            }
        }
        pos = FUN_0040cfd0(entry);
        if (LogFlumeTunelMSprite != NULL) {
            PrintSprite(LogFlumeTunelMSprite, pos.x, pos.y, clip, 0);
        }
    }
    FUN_0040cd70((struct PairHolder *)entry, 1);
}

// FUNCTION: LEGOLAND 0x0040f4f0
void LogFlumeTunnelSetEditMode(void) {
    FUN_0040d3b0(DAT_004cbe18, DAT_004cbe18->var_3c);
}

// FUNCTION: LEGOLAND 0x0040f510
void LogFlumeTunnelCalcCursor(unsigned int param_1, unsigned int param_2, unsigned int param_3) {
    FUN_0040d6f0(DAT_004cbe18, param_2, param_3, DAT_004cbe18->var_3c, FUN_0040f360, FUN_0040f330);
}

// FUNCTION: LEGOLAND 0x0040f540
void LogFlumeTunnelAddObject(unsigned int param_1, struct FlumeBytes *param_2) {
    unsigned int packed;
    *((unsigned char *)&packed) = param_2->b0;
    *((unsigned char *)&packed + 1) = param_2->b4;
    FUN_0040d900(packed, DAT_004cbe18->var_3c, (int)DAT_004cbe48, FUN_0040f050, FUN_0040f360, FUN_0040f300);
}

// FUNCTION: LEGOLAND 0x0040f580
void LogFlumeTunnelRemoveObject(unsigned int param_1, unsigned int param_2, unsigned int param_3) {
    FUN_0040db00(param_1, param_2, param_3, FUN_0040f360);
}

// FUNCTION: LEGOLAND 0x0040f5a0
void LogFlumeTunnelDCalcCursor(unsigned int param_1, struct Point *param_2) {
    FUN_0040d2d0(param_2);
}

// FUNCTION: LEGOLAND 0x0040f5b0
void FUN_0040f5b0(struct FlumeNode *param_1) {
    struct FlumeNode *parent = param_1;
    struct FlumeNode *node;
    struct FlumeNode *prev;
    struct FlumeNode *last;
    TileId pos;
    int w = LogFlumeFootprint.x1 - LogFlumeFootprint.x0;
    int h = LogFlumeFootprint.y1 - LogFlumeFootprint.y0;
    int i;

    pos.pos.x = ((struct Ride *)DAT_004c2bf0)->footprint.x0 + parent->tile.pos.x;
    pos.pos.y = ((struct Ride *)DAT_004c2bf0)->footprint.y0 + parent->tile.pos.y + 3;
    node = NewFlumeNode(parent, 3, 3, pos.pos.y, pos.pos.x);
    AppendListNode((struct Node *)parent, (struct ListNode *)node);
    parent->field_30 = (unsigned int)node;
    prev = node;
    for (i = 4; i != 0; i--) {
        pos.pos.x += w;
        node = NewFlumeNode(parent, 1, 1, pos.pos.y, pos.pos.x);
        AppendListNode((struct Node *)parent, (struct ListNode *)node);
        LinkNodeAfter((struct Node *)prev, (struct Node *)node);
        prev = node;
    }
    pos.pos.x += w;
    node = NewFlumeNode(parent, 2, 3, pos.pos.y, pos.pos.x);
    AppendListNode((struct Node *)parent, (struct ListNode *)node);
    LinkNodeAfter((struct Node *)prev, (struct Node *)node);
    prev = node;
    pos.pos.y -= h;
    node = NewFlumeNode(parent, 2, 1, pos.pos.y, pos.pos.x);
    AppendListNode((struct Node *)parent, (struct ListNode *)node);
    LinkNodeAfter((struct Node *)prev, (struct Node *)node);
    prev = node;
    pos.pos.x += w;
    last = NewFlumeNode(parent, 3, 1, pos.pos.y, pos.pos.x);
    AppendListNode((struct Node *)parent, (struct ListNode *)last);
    LinkNodeAfter((struct Node *)prev, (struct Node *)last);
    parent->field_34 = (unsigned int)last;
}

// FUNCTION: LEGOLAND 0x0040f7d0
void FUN_0040f7d0(struct FlumePos *src, struct FlumeRect *dst) {
    dst->var_0 = 0;
    dst->var_4 = 0;
    dst->var_8 = 0;
    dst->var_c = 0;
    dst->var_c = src->var_30;
    dst->var_4 = src->var_34;
}

// FUNCTION: LEGOLAND 0x0040f800
unsigned int FUN_0040f800(unsigned int *param_1) {
    int flags = FUN_00409410(param_1);
    if (flags == 0x44 || flags == 0x4 || flags == 0x40) {
        return 1;
    }
    return 0;
}

// FUNCTION: LEGOLAND 0x0040f830
void FUN_0040f830(struct FlumeXY p, unsigned int *result) {
    unsigned int w = LogFlumeFootprint.x1 - LogFlumeFootprint.x0;

    p.x += (unsigned char)DAT_004c2bf0->var_3c[0];
    p.y += (unsigned char)DAT_004c2bf0->var_3c[1];
    result[0] = 10;
    result[7] = p.x;
    result[8] = p.y + 3;
    result[3] = DAT_004c2bf0->var_3c[2] - DAT_004c2bf0->var_3c[0] - w + p.x + 1;
    result[4] = p.y + 1;
}

// FUNCTION: LEGOLAND 0x0040f8b0
struct Sprite *FUN_0040f8b0(struct Obj *obj_ptr) {
    struct ResA *resA;
    struct ResB *resB;

    DAT_004c2bf0 = (struct CursorSource *)obj_ptr->ride;
    resA = (struct ResA *)DAT_004c2bf0;
    resA->flags_1c |= 0x400;
    DAT_004c4460 = obj_ptr;

    resA = (struct ResA *)DAT_004c2bf0;
    if (resA != NULL) {
        resB = resA->ptr_64;
        if (resB != NULL) {
            resB->flags_10 |= 0x2000;
        }
    }

    // STRING: LEGOLAND 0x004b4aa0
    LogFlumeCsaw2MSprite = LoadSprite("csaw2_m.lls", 1);
    return LogFlumeCsaw2MSprite;
}

// FUNCTION: LEGOLAND 0x0040f900
void LogFlumeCsawUnloadSprites(void) {
    if (LogFlumeCsaw2MSprite != 0) {
        KillSprite(LogFlumeCsaw2MSprite);
    }
}

// FUNCTION: LEGOLAND 0x0040f920
void RenderLogFlumeCsaw(Element *elem, unsigned int param_2, unsigned int param_3, TileId *tile, unsigned int param_5, unsigned int clip) {
    struct Ride *ride = elem->ride;
    struct FlumeEntry *entry;
    struct LLS *lls;
    struct Point off;
    struct Point pos;
    int frame;

    entry = FindFlumeSubEntryByTile(tile);
    if (FUN_0040cdf0(tile)) {
        if (LogFlumeCsaw2MSprite != NULL) {
            lls = (struct LLS *)GetLLSForSprite((struct SpriteLLS *)GetSpriteForLayer(ride->layer, 1));
            if (lls != NULL) {
                frame = lls->frame;
            }
            lls = (struct LLS *)GetLLSForSprite((struct SpriteLLS *)LogFlumeCsaw2MSprite);
            if (lls != NULL) {
                LLSSetFrame(lls, frame);
            }
        }
        off = GetRenderOffsetForLayer(ride->layer, 1);
        pos = FUN_0040cfd0(entry);
        AdjustOffsetForViewMode(&off);
        if (LogFlumeCsaw2MSprite != NULL) {
            PrintSprite(LogFlumeCsaw2MSprite, pos.x + off.x, pos.y + off.y, clip, 0);
        }
    }
    FUN_0040cd70((struct PairHolder *)entry, 1);
}

// FUNCTION: LEGOLAND 0x0040fa00
void LogFlumeCsawSetEditMode(void) {
    FUN_0040d3b0(DAT_004c2bf0, DAT_004c2bf0->var_3c);
}

// FUNCTION: LEGOLAND 0x0040fa20
void LogFlumeCsawCalcCursor(unsigned int param_1, unsigned int param_2, unsigned int param_3) {
    struct CursorSource *src = DAT_004c2bf0;
    FUN_0040d6f0(src, param_2, param_3, src->var_3c, FUN_0040f830, FUN_0040f800);
}

// FUNCTION: LEGOLAND 0x0040fa50
void LogFlumeCsawDCalcCursor(unsigned int param_1, struct Point *param_2) {
    FUN_0040d2d0(param_2);
}

// FUNCTION: LEGOLAND 0x0040fa60
void LogFlumeCsawAddObject(unsigned int param_1, struct FlumeBytes *param_2) {
    unsigned int packed;
    *((unsigned char *)&packed) = param_2->b0;
    *((unsigned char *)&packed + 1) = param_2->b4;
    DAT_004c2af4 = 0;
    FUN_0040d900(packed, DAT_004c2bf0->var_3c, (int)DAT_004c4460, FUN_0040f5b0, FUN_0040f830, FUN_0040f7d0);
}

// FUNCTION: LEGOLAND 0x0040fab0
unsigned int LogFlumeCsawRemoveObject(unsigned int param_1, unsigned int param_2, unsigned int param_3) {
    FUN_0040db00(param_1, param_2, param_3, FUN_0040f830);
    return 0; /* [port] the original returned whatever the call left in eax; callers ignore it */
}

#define NEW_FLUME_NODE(node, parent, mode_, submode_, y_, x_) \
    do { \
        (node) = FUN_00409010(); \
        if ((node) != NULL) { \
            (node)->mode = (mode_); \
            (node)->submode = (submode_); \
            (node)->owner = (parent)->owner; \
            (node)->entry = (parent); \
            (node)->ride = (struct Ride *)LogFlumeTrackRide; \
            (node)->flags |= 4; \
            (node)->tile.pos.x = (x_); \
            (node)->tile.pos.y = (y_); \
        } \
    } while (0)

// FUNCTION: LEGOLAND 0x0040fad0
void FUN_0040fad0(struct FlumeNode *parent) {
    struct FlumeNode *node;
    struct FlumeNode *prev;
    TileId pos;
    int w = LogFlumeFootprint.x1 - LogFlumeFootprint.x0;
    int h = LogFlumeFootprint.y1 - LogFlumeFootprint.y0;
    int i;

    pos.pos.x = ((struct Ride *)DAT_004c2b60)->footprint.x0 + parent->tile.pos.x;
    pos.pos.y = ((struct Ride *)DAT_004c2b60)->footprint.y0 + parent->tile.pos.y + 9;
    NEW_FLUME_NODE(node, parent, 3, 3, pos.pos.y, pos.pos.x);
    AppendListNode((struct Node *)parent, (struct ListNode *)node);
    parent->field_30 = (unsigned int)node;
    prev = node;
    pos.pos.x += w;
    NEW_FLUME_NODE(node, parent, 1, 1, pos.pos.y, pos.pos.x);
    AppendListNode((struct Node *)parent, (struct ListNode *)node);
    LinkNodeAfter((struct Node *)prev, (struct Node *)node);
    prev = node;
    pos.pos.x += w;
    NEW_FLUME_NODE(node, parent, 2, 3, pos.pos.y, pos.pos.x);
    AppendListNode((struct Node *)parent, (struct ListNode *)node);
    LinkNodeAfter((struct Node *)prev, (struct Node *)node);
    prev = node;
    pos.pos.y -= h;
    NEW_FLUME_NODE(node, parent, 1, 0, pos.pos.y, pos.pos.x);
    AppendListNode((struct Node *)parent, (struct ListNode *)node);
    LinkNodeAfter((struct Node *)prev, (struct Node *)node);
    prev = node;
    pos.pos.y -= h;
    NEW_FLUME_NODE(node, parent, 2, 1, pos.pos.y, pos.pos.x);
    AppendListNode((struct Node *)parent, (struct ListNode *)node);
    LinkNodeAfter((struct Node *)prev, (struct Node *)node);
    prev = node;
    for (i = 2; i != 0; i--) {
        pos.pos.x += w;
        NEW_FLUME_NODE(node, parent, 1, 1, pos.pos.y, pos.pos.x);
        AppendListNode((struct Node *)parent, (struct ListNode *)node);
        LinkNodeAfter((struct Node *)prev, (struct Node *)node);
        prev = node;
    }
    pos.pos.x += w;
    NEW_FLUME_NODE(node, parent, 2, 2, pos.pos.y, pos.pos.x);
    AppendListNode((struct Node *)parent, (struct ListNode *)node);
    LinkNodeAfter((struct Node *)prev, (struct Node *)node);
    prev = node;
    pos.pos.y += h;
    NEW_FLUME_NODE(node, parent, 2, 0, pos.pos.y, pos.pos.x);
    AppendListNode((struct Node *)parent, (struct ListNode *)node);
    LinkNodeAfter((struct Node *)prev, (struct Node *)node);
    prev = node;
    pos.pos.x += w;
    NEW_FLUME_NODE(node, parent, 3, 1, pos.pos.y, pos.pos.x);
    AppendListNode((struct Node *)parent, (struct ListNode *)node);
    LinkNodeAfter((struct Node *)prev, (struct Node *)node);
    parent->field_34 = (unsigned int)node;
}

// FUNCTION: LEGOLAND 0x0040fe50
void FUN_0040fe50(struct FlumePos *src, struct FlumeRect *dst) {
    dst->var_0 = 0;
    dst->var_4 = 0;
    dst->var_8 = 0;
    dst->var_c = 0;
    dst->var_c = src->var_30;
    dst->var_4 = src->var_34;
}

// FUNCTION: LEGOLAND 0x0040fe80
unsigned int FUN_0040fe80(unsigned int *param_1) {
    int flags = FUN_00409410(param_1);
    if (flags == 0x44 || flags == 0x4 || flags == 0x40) {
        return 1;
    }
    return 0;
}

// FUNCTION: LEGOLAND 0x0040feb0
void FUN_0040feb0(TileId pos, struct FlumeOut *out) {
    struct Ride *ride = (struct Ride *)DAT_004c2b60;
    unsigned char x = pos.pos.x + (unsigned char)ride->footprint.x0;
    unsigned char y = pos.pos.y + (unsigned char)ride->footprint.y0;
    int span = LogFlumeFootprint.x1 - LogFlumeFootprint.x0;
    pos.pos.x = x;
    pos.pos.y = y;
    out->kind = 10;
    out->f_1c = pos.pos.x;
    out->f_20 = pos.pos.y + 9;
    out->f_10 = pos.pos.y + 7;
    ride = (struct Ride *)DAT_004c2b60;
    out->f_0c = ride->footprint.x1 - ride->footprint.x0 - span + pos.pos.x + 1;
}

// FUNCTION: LEGOLAND 0x0040ff30
void LogFlumeHoldUpLoadSprites(struct Obj *obj_ptr) {
    struct ResA *resA;
    struct ResB *resB;

    DAT_004c2b60 = (struct CursorSource *)obj_ptr->ride;
    resA = (struct ResA *)DAT_004c2b60;
    resA->flags_1c |= 0x400;

    resA = (struct ResA *)DAT_004c2b60;
    DAT_004c2b50 = obj_ptr;
    if (resA != NULL) {
        resB = resA->ptr_64;
        if (resB != NULL) {
            resB->flags_10 |= 0x2000;
        }
    }

    // STRING: LEGOLAND 0x004b4ab8
    LogFlumeHup1M1Sprite = LoadSprite("hup1_m1.lls", 1);
    // STRING: LEGOLAND 0x004b4aac
    LogFlumeHup1M2Sprite = LoadSprite("hup1_m2.lls", 1);
    FlumeStages_004b4830[0].sprite = LogFlumeHup1M1Sprite;
    FlumeStages_004b4830[2].sprite = LogFlumeHup1M2Sprite;
}

// FUNCTION: LEGOLAND 0x0040ffa0
void LogFlumeHoldUpUnloadSprites(void) {
    if (LogFlumeHup1M1Sprite != 0) {
        KillSprite(LogFlumeHup1M1Sprite);
    }
    if (LogFlumeHup1M2Sprite != 0) {
        KillSprite(LogFlumeHup1M2Sprite);
    }
}

// FUNCTION: LEGOLAND 0x0040ffd0
void RenderLogFlumeHoldUp(Element *elem, unsigned int param_2, unsigned int param_3, TileId *tile) {
    struct Ride *ride = elem->ride;
    struct FlumeEntry *entry;
    struct LLS *lls;
    struct Sprite *sprite;
    struct Point pos;
    struct Point off;
    int frame;

    frame = 0;
    sprite = GetSpriteForLayer(ride->layer, 1);
    if (sprite != NULL) {
        lls = (struct LLS *)GetLLSForSprite((struct SpriteLLS *)sprite);
        if (lls != NULL) {
            frame = lls->frame;
        }
    }
    lls = (struct LLS *)GetLLSForSprite((struct SpriteLLS *)LogFlumeHup1M1Sprite);
    if (lls != NULL) {
        LLSSetFrame(lls, frame);
    }
    lls = (struct LLS *)GetLLSForSprite((struct SpriteLLS *)LogFlumeHup1M2Sprite);
    if (lls != NULL) {
        LLSSetFrame(lls, frame);
    }
    pos = GetScreenCoordsForObject(tile, ride);
    off = GetRenderOffsetForLayer(ride->layer, 1);
    AdjustOffsetForViewMode(&off);
    pos.x += off.x;
    pos.y += off.y;
    FUN_0040cdf0(tile);
    entry = FindFlumeSubEntryByTile(tile);
    if (entry != NULL) {
        FUN_0040b290(entry, pos.x, pos.y, &DAT_004b4858, 0);
        FUN_0040cd70((struct PairHolder *)entry, 1);
    }
}

// FUNCTION: LEGOLAND 0x004100b0
void LogFlumeHoldUpSetEditMode(void) {
    FUN_0040d3b0(DAT_004c2b60, DAT_004c2b60->var_3c);
}

// FUNCTION: LEGOLAND 0x004100d0
unsigned int LogFlumeHoldUpCalcCursor(unsigned int param_1, unsigned int param_2, unsigned int param_3) {
    struct CursorSource *src = DAT_004c2b60;
    return FUN_0040d6f0(src, param_2, param_3, src->var_3c, FUN_0040feb0, FUN_0040fe80);
}

// FUNCTION: LEGOLAND 0x00410100
void LogFlumeHoldUpDCalcCursor(unsigned int param_1, struct Point *param_2) {
    FUN_0040d2d0(param_2);
}

// FUNCTION: LEGOLAND 0x00410110
void LogFlumeHoldUpAddObject(unsigned int param_1, struct FlumeBytes *param_2) {
    unsigned int packed;
    *((unsigned char *)&packed) = param_2->b0;
    *((unsigned char *)&packed + 1) = param_2->b4;
    DAT_004c2af4 = 0;
    FUN_0040d900(packed, DAT_004c2b60->var_3c, (int)DAT_004c2b50, FUN_0040fad0, FUN_0040feb0, FUN_0040fe50);
}

// FUNCTION: LEGOLAND 0x00410160
unsigned int LogFlumeHoldUpRemoveObject(unsigned int param_1, unsigned int param_2, unsigned int param_3) {
    FUN_0040db00(param_1, param_2, param_3, FUN_0040feb0);
    return 0; /* [port] the original returned whatever the call left in eax; callers ignore it */
}

// FUNCTION: LEGOLAND 0x00410180
void FUN_00410180(struct FlumeNode *param_1) {
    struct FlumeNode *parent = param_1;
    struct FlumeNode *node;
    struct FlumeNode *prev;
    TileId pos;
    int w = LogFlumeFootprint.y1 - LogFlumeFootprint.y0;
    int n;

    pos.pos.y = ((struct Ride *)DAT_004c8d6c)->footprint.y0 + parent->tile.pos.y;
    pos.pos.x = ((struct Ride *)DAT_004c8d6c)->footprint.x0 + parent->tile.pos.x + 2;
    node = NewFlumeNode(parent, 3, 0, pos.pos.y, pos.pos.x);
    AppendListNode((struct Node *)parent, (struct ListNode *)node);
    n = 0x18 / w;
    n += -2;
    parent->field_30 = (unsigned int)node;
    prev = node;
    for (; n > 0; n--) {
        pos.pos.y += w;
        node = NewFlumeNode(parent, 1, 0, pos.pos.y, pos.pos.x);
        AppendListNode((struct Node *)parent, (struct ListNode *)node);
        LinkNodeAfter((struct Node *)prev, (struct Node *)node);
        prev = node;
    }
    pos.pos.y += w;
    node = NewFlumeNode(parent, 3, 2, pos.pos.y, pos.pos.x);
    AppendListNode((struct Node *)parent, (struct ListNode *)node);
    LinkNodeAfter((struct Node *)prev, (struct Node *)node);
    parent->field_34 = (unsigned int)node;
}

// FUNCTION: LEGOLAND 0x004102e0
void FUN_004102e0(struct FlumePos *src, struct FlumeRect *dst) {
    dst->var_0 = 0;
    dst->var_4 = 0;
    dst->var_8 = 0;
    dst->var_c = 0;
    dst->var_0 = src->var_30;
    dst->var_8 = src->var_34;
}

// FUNCTION: LEGOLAND 0x00410310
int FUN_00410310(struct FlumeInput *param) {
    int result = FUN_00409410((unsigned int *)param);
    if (result != 0x11 && result != 0x1 && result != 0x10) {
        return 0;
    }
    if (result & 0x1) {
        if (param->var_0->var_8 != 0) {
            return 0;
        }
    }
    if (result & 0x10) {
        if (param->var_8->var_c != 0) {
            return 0;
        }
    }
    return 1;
}

// FUNCTION: LEGOLAND 0x00410360
void FUN_00410360(struct FlumeXY p, unsigned int *result) {
    unsigned int w = LogFlumeFootprint.y1 - LogFlumeFootprint.y0;

    p.x += (unsigned char)DAT_004c8d6c->var_3c[0];
    p.y += (unsigned char)DAT_004c8d6c->var_3c[1];
    result[0] = 5;
    result[1] = p.x + 2;
    result[2] = p.y;
    result[5] = p.x + 2;
    result[6] = DAT_004c8d6c->var_3c[3] - DAT_004c8d6c->var_3c[1] - w + p.y + 1;
}

// FUNCTION: LEGOLAND 0x004103e0
struct Sprite *FUN_004103e0(struct Obj *obj_ptr) {
    struct ResA *resA;
    struct ResB *resB;

    DAT_004c8d6c = (struct CursorSource *)obj_ptr->ride;
    resA = (struct ResA *)DAT_004c8d6c;
    resA->flags_1c |= 0x400;

    resA = (struct ResA *)DAT_004c8d6c;
    DAT_004c8d4c = obj_ptr;
    if (resA != NULL) {
        resB = resA->ptr_64;
        if (resB != NULL) {
            resB->flags_10 |= 0x2000;
        }
    }

    // STRING: LEGOLAND 0x004b4ae0
    LogFlumeDrop1MSprite = LoadSprite("drop1_m.lls", 1);
    // STRING: LEGOLAND 0x004b4ad4
    LogFlumeDrop2MSprite = LoadSprite("drop2_m.lls", 1);
    // STRING: LEGOLAND 0x004b4ac4
    LogFlumeSplashSprite = LoadSprite("lf_splash.lls", 1);

    return LogFlumeSplashSprite;
}

// FUNCTION: LEGOLAND 0x00410450
void LogFlumeDropUnloadSprites(void) {
    if (LogFlumeSplashSprite != 0) {
        KillSprite(LogFlumeSplashSprite);
        LogFlumeSplashSprite = 0;
    }
    if (LogFlumeDrop1MSprite != 0) {
        KillSprite(LogFlumeDrop1MSprite);
        LogFlumeDrop1MSprite = 0;
    }
    if (LogFlumeDrop2MSprite != 0) {
        KillSprite(LogFlumeDrop2MSprite);
        LogFlumeDrop2MSprite = 0;
    }
}

// FUNCTION: LEGOLAND 0x004104b0
void RenderLogFlumeDrop(Element *elem, unsigned int param_2, unsigned int param_3, TileId *tile, unsigned int param_5, unsigned int clip) {
    struct Ride *ride = elem->ride;
    struct FlumeEntry *entry;
    struct Sprite *sprite;
    struct LLS *lls;
    struct Point off;
    struct Point pos;
    int frame;

    entry = FindFlumeSubEntryByTile(tile);
    if (FUN_0040cdf0(tile) && entry != NULL) {
        if (LogFlumeDrop1MSprite != NULL && LogFlumeDrop2MSprite != NULL) {
            frame = 0;
            sprite = GetSpriteForLayer(ride->layer, 0);
            if (sprite != NULL) {
                lls = (struct LLS *)GetLLSForSprite((struct SpriteLLS *)sprite);
                if (lls != NULL) {
                    frame = lls->frame;
                }
            }
            lls = (struct LLS *)GetLLSForSprite((struct SpriteLLS *)LogFlumeDrop1MSprite);
            if (lls != NULL) {
                LLSSetFrame(lls, frame);
            }
            sprite = GetSpriteForLayer(ride->layer, 1);
            if (sprite != NULL) {
                lls = (struct LLS *)GetLLSForSprite((struct SpriteLLS *)sprite);
                if (lls != NULL) {
                    frame = lls->frame;
                }
            }
            lls = (struct LLS *)GetLLSForSprite((struct SpriteLLS *)LogFlumeDrop2MSprite);
            if (lls != NULL) {
                LLSSetFrame(lls, frame);
            }
        }
        pos = FUN_0040cfd0(entry);
        if (LogFlumeDrop1MSprite != NULL) {
            off = GetRenderOffsetForLayer(ride->layer, 0);
            AdjustOffsetForViewMode(&off);
            PrintSprite(LogFlumeDrop1MSprite, pos.x + off.x, pos.y + off.y, clip, 0);
        }
        if (LogFlumeDrop2MSprite != NULL) {
            off = GetRenderOffsetForLayer(ride->layer, 1);
            AdjustOffsetForViewMode(&off);
            PrintSprite(LogFlumeDrop2MSprite, pos.x + off.x, pos.y + off.y, clip, 0);
        }
        if (entry->field_24 != NULL && (entry->field_24->flags & 2)) {
            pos = FUN_0040cfd0(entry);
            off = GetRenderOffsetForLayer(ride->layer, 1);
            off.x += DAT_004b4860.x;
            off.y += DAT_004b4860.y;
            AdjustOffsetForViewMode(&off);
            if (LogFlumeSplashSprite != NULL) {
                frame = entry->field_24->i24;
                lls = (struct LLS *)GetLLSForSprite((struct SpriteLLS *)LogFlumeSplashSprite);
                if (lls != NULL) {
                    LLSStop((unsigned int)lls);
                    LLSSetFrame(lls, frame);
                }
                PrintSprite(LogFlumeSplashSprite, pos.x + off.x, pos.y + off.y, clip, 0);
            }
        }
    }
    FUN_0040cd70((struct PairHolder *)entry, 1);
}

// FUNCTION: LEGOLAND 0x004106e0
unsigned int LogFlumeDropSetEditMode(void) {
    return FUN_0040d3b0(DAT_004c8d6c, DAT_004c8d6c->var_3c);
}

// FUNCTION: LEGOLAND 0x00410700
unsigned int LogFlumeDropCalcCursor(unsigned int param_1, unsigned int param_2, unsigned int param_3) {
    struct CursorSource *src = DAT_004c8d6c;
    return FUN_0040d6f0(src, param_2, param_3, src->var_3c, FUN_00410360, FUN_00410310);
}

// FUNCTION: LEGOLAND 0x00410730
void LogFlumeDropDCalcCursor(unsigned int param_1, struct Point *param_2) {
    FUN_0040d2d0(param_2);
}

// FUNCTION: LEGOLAND 0x00410740
void LogFlumeDropAddObject(unsigned int param_1, struct FlumeBytes *param_2) {
    unsigned int packed;
    *((unsigned char *)&packed) = param_2->b0;
    *((unsigned char *)&packed + 1) = param_2->b4;
    DAT_004c2af4 = 0;
    FUN_0040d900(packed, DAT_004c8d6c->var_3c, (int)DAT_004c8d4c, FUN_00410180, FUN_00410360, FUN_004102e0);
}

// FUNCTION: LEGOLAND 0x00410790
unsigned int LogFlumeDropRemoveObject(unsigned int param_1, unsigned int param_2, unsigned int param_3) {
    FUN_0040db00(param_1, param_2, param_3, FUN_00410360);
    return 0; /* [port] the original returned whatever the call left in eax; callers ignore it */
}

// FUNCTION: LEGOLAND 0x004107b0
int FUN_004107b0(struct FlumeNode *node, struct FlumeNode *target) {
    int idx = 1;
    int r;

    while (node != NULL) {
        if (node == target) {
            return idx;
        }
        r = FUN_004107b0(node->field_2c, target);
        if (r != -1) {
            return (r << 16) | idx;
        }
        node = node->next;
        idx++;
    }
    return -1;
}

// FUNCTION: LEGOLAND 0x00410800
void FUN_00410800(struct FlumeEntry *owner, struct FlumeNode *node) {
    int marker = 1;
    int end = 0;

    if (node != NULL) {
        do {
            struct FlumeNode buf;
            int len;

            SaveGameWrite(&marker, 4);
            buf = *node;
            buf.field_8 = FUN_004107b0((struct FlumeNode *)owner->sub, (struct FlumeNode *)buf.field_8);
            buf.field_c = FUN_004107b0((struct FlumeNode *)owner->sub, (struct FlumeNode *)buf.field_c);
            buf.field_30 = FUN_004107b0((struct FlumeNode *)owner->sub, (struct FlumeNode *)buf.field_30);
            buf.field_34 = FUN_004107b0((struct FlumeNode *)owner->sub, (struct FlumeNode *)buf.field_34);
            SaveGameWrite(&buf, sizeof(buf));
            len = strlen(buf.ride->element->name);
            SaveGameWrite(&len, 4);
            SaveGameWrite(buf.ride->element->name, len);
            FUN_00410800(owner, node->field_2c);
            node = node->next;
        } while (node != NULL);
    }
    SaveGameWrite(&end, 4);
}

// FUNCTION: LEGOLAND 0x00410910
int FUN_00410910(struct FlumeEntry *entry) {
    int count = 0;
    void *target = entry->target;
    char *current = (char *)entry->slots;

    while (count < 4) {
        if (target == (void *)current) {
            return count;
        }
        count = count + 1;
        current = current + 0x24;
    }
    return -1;
}

// FUNCTION: LEGOLAND 0x00410930
int LogFlumeEntrance_Save(void) {
    struct FlumeEntry *entry = FlumeEntryList;
    int marker = 1;
    int end = 0;
    int i;

    if (entry != NULL) {
        do {
            struct FlumeEntry buf;
            SaveGameWrite(&marker, 4);
            FUN_00410800(entry, entry->sub);
            buf = *entry;
            buf.field_8 = (struct FlumeNode *)FUN_004107b0((struct FlumeNode *)buf.sub, buf.field_8);
            buf.field_c = (struct FlumeNode *)FUN_004107b0((struct FlumeNode *)buf.sub, buf.field_c);
            buf.link = (struct FlumeEntry *)FUN_004107b0((struct FlumeNode *)buf.sub, (struct FlumeNode *)buf.link);
            buf.target = (void *)FUN_00410910(entry);
            SaveQueue((struct QueueNode *)((struct Ride *)LogFlumeEntranceRide)->riders, &buf.queue);
            for (i = 0; i < 4; i++) {
                buf.slots[i].owner = FUN_004107b0((struct FlumeNode *)buf.sub, (struct FlumeNode *)buf.slots[i].owner);
                buf.slots[i].busy = FUN_004123a0((struct QueueNode *)((struct Ride *)LogFlumeEntranceRide)->riders, (struct QueueNode *)buf.slots[i].busy);
            }
            SaveGameWrite(&buf, sizeof(buf));
            entry = entry->next;
        } while (entry != NULL);
    }
    SaveGameWrite(&end, 4);
    return 1;
}

// FUNCTION: LEGOLAND 0x00410a50
struct FlumeNode *FUN_00410a50(struct FlumeEntry *owner, struct FlumeEntry *entry) {
    struct FlumeNode *head = NULL;
    struct FlumeNode *cur = NULL;
    struct FlumeNode *prev = NULL;
    int marker = 1;
    int len;
    unsigned int elem;
    char name[0x200];

    SaveGameRead(&marker, 4);
    while (marker != 0) {
        if (cur == NULL) {
            cur = malloc(0x38);
            head = cur;
        } else {
            cur->next = malloc(0x38);
            cur = cur->next;
        }
        SaveGameRead(cur, 0x38);
        cur->prev = prev;
        cur->owner = owner;
        cur->entry = entry;
        SaveGameRead(&len, 4);
        SaveGameRead(name, len);
        name[len] = 0;
        if (name[0] != 0) {
            LLIDB_FindElement(name, &elem, 0);
            cur->ride = ((struct Element *)elem)->ride;
        }
        cur->field_2c = FUN_00410a50(owner, (struct FlumeEntry *)cur);
        SaveGameRead(&marker, 4);
        prev = cur;
    }
    return head;
}

// FUNCTION: LEGOLAND 0x00410b60
void *IndexToFlumeNode(void *base, unsigned int packed) {
    unsigned int hi;
    unsigned int lo;
    unsigned int i;
    struct FlumeNode *n;

    lo = packed & 0xffff;
    hi = packed >> 16;

    if (lo == 0) {
        return NULL;
    }
    if (lo == 0xffff) {
        return NULL;
    }
    lo--;
    if (lo != 0) {
        i = lo;
        n = (struct FlumeNode *)base;
        do {
            n = n->next;
        } while (--i != 0);
    } else {
        n = (struct FlumeNode *)base;
    }
    if (hi == 0 || hi == 0xffff) {
        return n;
    }
    n = n->field_2c;
    while (--hi != 0) {
        n = n->next;
    }
    return n;
}

// FUNCTION: LEGOLAND 0x00410bb0
void FUN_00410bb0(void *arg0, struct FlumeNode *arg1) {
    struct FlumeNode *current;

    if (arg1 == NULL) {
        return;
    }

    current = arg1;
    do {
        current->field_8 = (unsigned int)IndexToFlumeNode(arg0, current->field_8);
        current->field_c = (unsigned int)IndexToFlumeNode(arg0, current->field_c);
        current->field_30 = (unsigned int)IndexToFlumeNode(arg0, current->field_30);
        current->field_34 = (unsigned int)IndexToFlumeNode(arg0, current->field_34);
        FUN_00410bb0(arg0, current->field_2c);
        current = current->next;
    } while (current != NULL);
}

// FUNCTION: LEGOLAND 0x00410c10
int LogFlumeEntrance_Load(void) {
    struct FlumeEntry *cur = NULL;
    struct FlumeNode *head;
    struct Queue queue;
    struct FlumeSlot *slot;
    int marker = 1;
    int i;

    SaveGameRead(&marker, 4);
    while (marker != 0) {
        if (cur == NULL) {
            cur = malloc(0xd4);
            FlumeEntryList = cur;
            cur->next = NULL;
        } else {
            cur->next = malloc(0xd4);
            cur = cur->next;
        }
        head = FUN_00410a50(cur, NULL);
        FUN_00410bb0(head, head);
        FUN_00412490((struct QueueNode *)((struct Ride *)LogFlumeEntranceRide)->riders, &queue);
        SaveGameRead(cur, 0xd4);
        cur->sub = (struct FlumeEntry *)head;
        cur->queue = queue;
        cur->field_8 = (struct FlumeNode *)IndexToFlumeNode(head, (unsigned int)cur->field_8);
        cur->field_c = (struct FlumeNode *)IndexToFlumeNode(cur->sub, (unsigned int)cur->field_c);
        cur->link = (struct FlumeEntry *)IndexToFlumeNode(cur->sub, (unsigned int)cur->link);
        cur->target = &cur->slots[(int)cur->target];
        slot = cur->slots;
        for (i = 4; i != 0; i--) {
            slot->owner = (int)IndexToFlumeNode(cur->sub, slot->owner);
            slot->busy = (int)GetNthNextQueueNode((struct QueueNode *)((struct Ride *)LogFlumeEntranceRide)->riders, slot->busy);
            slot++;
        }
        SaveGameRead(&marker, 4);
    }
    return 1;
}

// FUNCTION: LEGOLAND 0x00410d60
void LogFlume_GetInterfaces(struct ClassNode *flume, struct CallbackTable *vtbl) {
    // STRING: LEGOLAND 0x004b4bb4
    if (_stricmp("LOG FLUME ENTRANCE", flume->name) == 0) {
        vtbl->cb_a4 = LogFlumeEntranceLoad;
        vtbl->cb_8c = LogFlumeEntranceSetEditMode;
        vtbl->cb_90 = LogFlumeEntranceCalcCursor;
        vtbl->cb_94 = LogFlumeEntranceDCalcCursor;
        vtbl->cb_98 = LogFlumeEntranceAddObject;
        vtbl->cb_9c = LogFlumeEntranceRemoveObject;
        vtbl->cb_a8 = LogFlumeEntranceUpdate;
        vtbl->cb_b0 = RenderLogFlumeEntrance;
        vtbl->cb_ac = LogFlumeEntranceUnload;
        vtbl->cb_bc = LogFlumeEntrance_Save;
        vtbl->cb_b8 = LogFlumeEntrance_Load;
        vtbl->cb_c0 = FUN_004119c0;
        return;
    }
    // STRING: LEGOLAND 0x004b4ba4
    if (_stricmp("LOG FLUME TRACK", flume->name) == 0) {
        DAT_0082c688 = flume->iface;
        vtbl->cb_a4 = LogFlumeTrackLoad;
        vtbl->cb_8c = LogFlumeTrackSetEditMode;
        vtbl->cb_a0 = GetLogFlumeTrackSpriteInfo;
        vtbl->cb_b0 = RenderLogFlumeTrack;
        vtbl->cb_90 = LogFlumeTrackCalcCursor;
        vtbl->cb_94 = LogFlumeTrackDCalcCursor;
        vtbl->cb_98 = LogFlumeTrackAddObject;
        vtbl->cb_9c = LogFlumeTrackRemoveObject;
        vtbl->cb_ac = LogFlumeTrackUnload;
        return;
    }
    // STRING: LEGOLAND 0x004b4b88
    if (_stricmp("LOG FLUME SPECIAL CORNER 1", flume->name) == 0) {
        vtbl->cb_a4 = LogFlumeSpecialCorner1LoadSprites;
        vtbl->cb_8c = LogFlumeSpecialCorner1SetEditMode;
        vtbl->cb_a0 = FUN_0040ed50;
        vtbl->cb_b0 = RenderLogFlumeSpecialCorner1;
        vtbl->cb_90 = LogFlumeSpecialCorner1CalcCursor;
        vtbl->cb_94 = LogFlumeSpecialCorner1DCalcCursor;
        vtbl->cb_98 = LogFlumeSpecialCorner1AddObject;
        vtbl->cb_9c = LogFlumeSpecialCorner1RemoveObject;
        vtbl->cb_ac = LogFlumeSpecialCorner1UnloadSprites;
        return;
    }
    // STRING: LEGOLAND 0x004b4b6c
    if (_stricmp("LOG FLUME SPECIAL CORNER 2", flume->name) == 0) {
        vtbl->cb_a4 = LogFlumeSpecialCorner2LoadSprites;
        vtbl->cb_8c = LogFlumeSpecialCorner2SetEditMode;
        vtbl->cb_a0 = FUN_0040ed50;
        vtbl->cb_b0 = RenderLogFlumeSpecialCorner2;
        vtbl->cb_90 = LogFlumeSpecialCorner2CalcCursor;
        vtbl->cb_94 = LogFlumeSpecialCorner2DCalcCursor;
        vtbl->cb_98 = LogFlumeSpecialCorner2AddObject;
        vtbl->cb_9c = LogFlumeSpecialCorner2RemoveObject;
        vtbl->cb_ac = LogFlumeSpecialCorner2UnloadSprites;
        return;
    }
    // STRING: LEGOLAND 0x004b4b50
    if (_stricmp("LOG FLUME SPECIAL CORNER 3", flume->name) == 0) {
        vtbl->cb_a4 = LogFlumeSpecialCorner3LoadSprites;
        vtbl->cb_8c = LogFlumeSpecialCorner3SetEditMode;
        vtbl->cb_a0 = FUN_0040ed50;
        vtbl->cb_b0 = RenderLogFlumeSpecialCorner3;
        vtbl->cb_90 = LogFlumeSpecialCorner3CalcCursor;
        vtbl->cb_94 = LogFlumeSpecialCorner3DCalcCursor;
        vtbl->cb_98 = LogFlumeSpecialCorner3AddObject;
        vtbl->cb_9c = LogFlumeSpecialCorner3RemoveObject;
        vtbl->cb_ac = LogFlumeSpecialCorner3UnloadSprites;
        return;
    }
    // STRING: LEGOLAND 0x004b4b34
    if (_stricmp("LOG FLUME SPECIAL CORNER 4", flume->name) == 0) {
        vtbl->cb_a4 = LogFlumeSpecialCorner4LoadSprites;
        vtbl->cb_8c = LogFlumeSpecialCorner4SetEditMode;
        vtbl->cb_a0 = FUN_0040ed50;
        vtbl->cb_b0 = RenderLogFlumeSpecialCorner4;
        vtbl->cb_90 = LogFlumeSpecialCorner4CalcCursor;
        vtbl->cb_94 = LogFlumeSpecialCorner4DCalcCursor;
        vtbl->cb_98 = LogFlumeSpecialCorner4AddObject;
        vtbl->cb_9c = LogFlumeSpecialCorner4RemoveObject;
        vtbl->cb_ac = LogFlumeSpecialCorner4UnloadSprites;
        return;
    }
    // STRING: LEGOLAND 0x004b4b24
    if (_stricmp("LOG FLUME CSAW", flume->name) == 0) {
        vtbl->cb_a4 = (RideCallback)FUN_0040f8b0;
        vtbl->cb_8c = LogFlumeCsawSetEditMode;
        vtbl->cb_a0 = FUN_0040ed50;
        vtbl->cb_b0 = RenderLogFlumeCsaw;
        vtbl->cb_90 = LogFlumeCsawCalcCursor;
        vtbl->cb_94 = LogFlumeCsawDCalcCursor;
        vtbl->cb_98 = LogFlumeCsawAddObject;
        vtbl->cb_9c = LogFlumeCsawRemoveObject;
        vtbl->cb_ac = LogFlumeCsawUnloadSprites;
        return;
    }
    // STRING: LEGOLAND 0x004b4b10
    if (_stricmp("LOG FLUME TUNNEL", flume->name) == 0) {
        vtbl->cb_a4 = LogFlumeTunnelLoadSprites;
        vtbl->cb_8c = LogFlumeTunnelSetEditMode;
        vtbl->cb_a0 = FUN_0040ed50;
        vtbl->cb_b0 = RenderLogFlumeTunnel;
        vtbl->cb_90 = LogFlumeTunnelCalcCursor;
        vtbl->cb_94 = LogFlumeTunnelDCalcCursor;
        vtbl->cb_98 = LogFlumeTunnelAddObject;
        vtbl->cb_9c = LogFlumeTunnelRemoveObject;
        vtbl->cb_ac = LogFlumeTunnelUnloadSprites;
        return;
    }
    // STRING: LEGOLAND 0x004b4b00
    if (_stricmp("LOG FLUME DROP", flume->name) == 0) {
        vtbl->cb_a4 = (RideCallback)FUN_004103e0;
        vtbl->cb_8c = LogFlumeDropSetEditMode;
        vtbl->cb_a0 = FUN_0040ed50;
        vtbl->cb_b0 = RenderLogFlumeDrop;
        vtbl->cb_90 = LogFlumeDropCalcCursor;
        vtbl->cb_94 = LogFlumeDropDCalcCursor;
        vtbl->cb_98 = LogFlumeDropAddObject;
        vtbl->cb_9c = LogFlumeDropRemoveObject;
        vtbl->cb_ac = LogFlumeDropUnloadSprites;
        return;
    }
    // STRING: LEGOLAND 0x004b4aec
    if (_stricmp("LOG FLUME HOLD UP", flume->name) == 0) {
        vtbl->cb_a4 = LogFlumeHoldUpLoadSprites;
        vtbl->cb_8c = LogFlumeHoldUpSetEditMode;
        vtbl->cb_a0 = FUN_0040ed50;
        vtbl->cb_b0 = RenderLogFlumeHoldUp;
        vtbl->cb_90 = LogFlumeHoldUpCalcCursor;
        vtbl->cb_94 = LogFlumeHoldUpDCalcCursor;
        vtbl->cb_98 = LogFlumeHoldUpAddObject;
        vtbl->cb_9c = LogFlumeHoldUpRemoveObject;
        vtbl->cb_ac = LogFlumeHoldUpUnloadSprites;
    }
}

// FUNCTION: LEGOLAND 0x00411220
struct FlumeDims FUN_00411220(void) {
    int dim1;
    int dim2;
    struct FlumeDims result;

    GetTileDimensions(&dim1, &dim2);

    dim1 = dim1 << 1;
    dim2 = dim2 << 1;

    result.field1 = (dim1 >> 1) + dim1;
    result.field2 = dim2 >> 1;

    return result;
}

// FUNCTION: LEGOLAND 0x00411250
struct FlumeDims FUN_00411250(void) {
    int dim1;
    int dim2;
    int val1;
    int val2;
    struct FlumeDims result;

    GetTileDimensions(&dim1, &dim2);

    dim1 = dim1 << 1;
    dim2 = dim2 << 1;

    val1 = dim1 >> 1;
    val2 = dim2 >> 1;

    val1 = val1 + dim1;
    val2 = val2 + dim2;

    result.field1 = val1;
    result.field2 = val2;

    return result;
}

// FUNCTION: LEGOLAND 0x00411290
struct FlumeDims FUN_00411290(void) {
    int dim1;
    int dim2;
    struct FlumeDims result;

    GetTileDimensions(&dim1, &dim2);

    dim1 = dim1 << 1;
    dim2 = dim2 << 1;

    result.field1 = dim1 >> 1;
    result.field2 = dim2 >> 1;

    return result;
}

// FUNCTION: LEGOLAND 0x004112c0
struct FlumeDims FUN_004112c0(void) {
    int dim1;
    int dim2;
    struct FlumeDims result;

    GetTileDimensions(&dim1, &dim2);

    dim1 = dim1 << 1;
    dim2 = dim2 << 1;

    result.field1 = dim1 >> 1;
    result.field2 = (dim2 >> 1) + dim2;

    return result;
}

// FUNCTION: LEGOLAND 0x004112f0
struct FlumeDims FUN_004112f0(struct FlumeShape *shape, float t, int reverse) {
    float step = 1.0f / (shape->count - 1);
    int idx = (int)floor(t / step);
    int next = idx + 1;
    struct FlumeDims p0;
    struct FlumeDims p1;
    struct FlumeDims result;

    if (reverse == 0) {
        p0 = shape->pts[idx];
        p1 = shape->pts[next];
    } else {
        p0 = shape->pts[shape->count - idx - 1];
        p1 = shape->pts[shape->count - next - 1];
    }
    result.field1 = (int)((p1.field1 - p0.field1) * ((t - idx * step) / step) + p0.field1);
    result.field2 = (int)((p1.field2 - p0.field2) * ((t - idx * step) / step) + p0.field2);
    return result;
}

// FUNCTION: LEGOLAND 0x004113d0
void FUN_004113d0(void) {
    int w;
    int h;
    int dx;
    int dy;

    GetTileDimensions(&w, &h);
    w = w << 1;
    h = h << 1;
    dy = h >> 2;
    dx = w >> 2;

    DAT_004c2b58.count = 2;
    DAT_004c2b58.pts = DAT_004cbe38;
    DAT_004c2b58.pts[0] = FUN_00411220();
    DAT_004c2b58.pts[1] = FUN_004112c0();

    DAT_004c2b00.count = 2;
    DAT_004c2b00.pts = DAT_004c8d58;
    DAT_004c2b00.pts[0] = FUN_00411250();
    DAT_004c2b00.pts[1] = FUN_00411290();

    DAT_004c2be8.count = 4;
    DAT_004c2be8.pts = DAT_004cbde8;
    DAT_004c2be8.pts[0] = FUN_00411220();
    DAT_004c2be8.pts[3] = FUN_00411250();
    DAT_004c2be8.pts[1].field1 = DAT_004c2be8.pts[0].field1 - dx;
    DAT_004c2be8.pts[1].field2 = DAT_004c2be8.pts[0].field2 + dy;
    DAT_004c2be8.pts[2].field1 = DAT_004c2be8.pts[3].field1 - dx;
    DAT_004c2be8.pts[2].field2 = DAT_004c2be8.pts[3].field2 - dy;

    DAT_004c2bc0.count = 4;
    DAT_004c2bc0.pts = DAT_004c2b30;
    DAT_004c2bc0.pts[0] = FUN_00411250();
    DAT_004c2bc0.pts[3] = FUN_004112c0();
    DAT_004c2bc0.pts[1].field1 = DAT_004c2bc0.pts[0].field1 - dx;
    DAT_004c2bc0.pts[1].field2 = DAT_004c2bc0.pts[0].field2 - dy;
    DAT_004c2bc0.pts[2].field1 = DAT_004c2bc0.pts[3].field1 + dx;
    DAT_004c2bc0.pts[2].field2 = DAT_004c2bc0.pts[3].field2 - dy;

    DAT_004c2c10.count = 4;
    DAT_004c2c10.pts = DAT_004c2bc8;
    DAT_004c2c10.pts[0] = FUN_004112c0();
    DAT_004c2c10.pts[3] = FUN_00411290();
    DAT_004c2c10.pts[1].field1 = DAT_004c2c10.pts[0].field1 + dx;
    DAT_004c2c10.pts[1].field2 = DAT_004c2c10.pts[0].field2 - dy;
    DAT_004c2c10.pts[2].field1 = DAT_004c2c10.pts[3].field1 + dx;
    DAT_004c2c10.pts[2].field2 = DAT_004c2c10.pts[3].field2 + dy;

    DAT_004c2c08.count = 4;
    DAT_004c2c08.pts = DAT_004c2b78;
    DAT_004c2c08.pts[0] = FUN_00411290();
    DAT_004c2c08.pts[3] = FUN_00411220();
    DAT_004c2c08.pts[1].field1 = DAT_004c2c08.pts[0].field1 + dx;
    DAT_004c2c08.pts[1].field2 = DAT_004c2c08.pts[0].field2 + dy;
    DAT_004c2c08.pts[2].field1 = DAT_004c2c08.pts[3].field1 - dx;
    DAT_004c2c08.pts[2].field2 = DAT_004c2c08.pts[3].field2 + dy;
}

struct FlumeObjA {
    unsigned char pad_0[0x28];
    struct FlumeObjB *var_28;
};
struct FlumeObjB {
    unsigned char pad_0[0x20];
    struct CursorSource *var_20;
};
// FUNCTION: LEGOLAND 0x00411650
int FUN_00411650(struct FlumeSlot *slot) {
    struct FlumeObjB *p = ((struct FlumeObjA *)slot->owner)->var_28;

    if (p != NULL && p != (struct FlumeObjB *)-1 && p->var_20 == DAT_004c8d6c) {
        return 1;
    }
    return 0;
}

// FUNCTION: LEGOLAND 0x00411680
int AdvanceFlumeMover(struct FlumeMover *mover) {
    volatile int next_y;
    struct FlumeNode *node = mover->node;
    struct FlumeNode *next;
    int rev;
    int flag;
    struct FlumeShape *shape = NULL;
    struct FlumeDims r;
    register TileId tile;
    float f;
    int dir;
    int dx;
    int dy;
    volatile int sub;

    flag = 0;
    rev = 0;
    f = mover->f18 + mover->f20;
    if (f > 1.0) {
        f = f - 1.0f;
        node = (struct FlumeNode *)node->field_8;
        flag = 1;
    }
    next = (struct FlumeNode *)node->field_8;
    if (NULL == next) {
        return 0;
    }
    tile = next->tile;
    dx = tile.pos.x - node->tile.pos.x;
    next_y = tile.pos.y;
    dy = next_y - node->tile.pos.y;
    if (0 > dx) {
        dir = 7;
    }
    if (dx > 0) {
        dir = 3;
    }
    if (0 > dy) {
        dir = 1;
    }
    if (dy > 0) {
        dir = 5;
    }
    switch (node->mode) {
    case 1:
        sub = node->submode;
        if (sub == 1) {
            shape = &DAT_004c2b00;
            if (dir == 3) {
                rev = 1;
            }
        }
        if (sub == 0) {
            shape = &DAT_004c2b58;
            if (dir == 1) {
                rev = 1;
            }
        }
        break;
    case 2:
        sub = node->submode;
        if (sub == 3) {
            shape = &DAT_004c2c08;
            if (7 == dir) {
                rev = 1;
            }
        }
        if (!sub) {
            shape = &DAT_004c2be8;
            if (dir == 1) {
                rev = 1;
            }
        }
        if (1 == sub) {
            shape = &DAT_004c2bc0;
            if (dir == 3) {
                rev = 1;
            }
        }
        if (sub == 2) {
            shape = &DAT_004c2c10;
            if (dir == 5) {
                rev = 1;
            }
        }
        break;
    }
    if (shape != NULL) {
        r = FUN_004112f0(shape, f, rev);
    }
    mover->x = r.field1;
    mover->y = r.field2;
    mover->f18 = f;
    return flag;
}

struct FlumeLink {
    unsigned char pad_0[0x8];
    struct FlumeLink *next;
};
struct FlumeHead {
    unsigned char pad_0[0x28];
    struct FlumeHeadB *var_28;
};
struct FlumeHeadB {
    unsigned char pad_0[0x30];
    struct FlumeLink *var_30;
};
struct FlumeObjD {
    unsigned char pad_0[0x14];
    struct FlumeHead *var_14;
};
// FUNCTION: LEGOLAND 0x004117e0
int FUN_004117e0(struct FlumeObjD *obj) {
    int idx = 0;
    struct FlumeHead *head = obj->var_14;
    struct FlumeLink *cur = head->var_28->var_30;

    while (cur != NULL) {
        if ((struct FlumeHead *)cur == head) {
            return idx;
        }
        cur = cur->next;
        idx++;
    }
    return -1;
}

// FUNCTION: LEGOLAND 0x00411810
void FUN_00411810(struct FlumeMover *m) {
    struct FlumeNode *entry;
    struct FlumeEntry *owner;
    struct LLS *lls;
    float f;
    int idx;

    if (!(m->flags & 2)) {
        return;
    }
    entry = m->node->entry;
    if (AdvanceFlumeMover(m)) {
        m->node = (struct FlumeNode *)m->node->field_8;
        if (m->node == (struct FlumeNode *)((struct FlumeNode *)entry->field_34)->field_8) {
            m->flags &= ~2;
            return;
        }
    }
    idx = FUN_004117e0((struct FlumeObjD *)m);
    f = (float)idx + m->f18;
    if (f < 2.0f) {
        m->i1c = 0;
    } else if (f < 4.0f) {
        m->i1c = (int)(120.0f * ((f - 2.0f) * 0.5f));
        m->f20 = 0.05f;
    }
    if (f >= 4.0f && f <= 5.0f) {
        m->i1c = 120;
        m->f20 = 0.1f;
    }
    if (f >= 5.0f && f <= 10.0f) {
        m->i1c = 120 - (int)(120.0f * ((f - 5.0f) * 0.2f));
        m->f20 += 0.05f;
        if (f >= 9.0f) {
            m->flags &= ~4;
        } else {
            m->flags |= 4;
        }
    }
    if (f >= 10.0f) {
        m->i1c = 0;
        m->f20 = 0.1f;
        if (f < 10.5) {
            owner = m->node->owner;
            if (owner != NULL && !(owner->flags & 2)) {
                owner->flags |= 2;
                owner->field_24 = 0;
                lls = (struct LLS *)GetLLSForSprite((struct SpriteLLS *)LogFlumeSplashSprite);
                if (lls != NULL) {
                    owner->field_28 = lls->frame_count;
                }
            }
        }
    }
}

// FUNCTION: LEGOLAND 0x004119a0
void FUN_004119a0(struct ParticleEmitter *param_1, unsigned int param_2) {
    if (param_1 != NULL) {
        param_1->var_d0 += param_2;
    }
}

// FUNCTION: LEGOLAND 0x004119c0
int FUN_004119c0(Element *obj, int filter) {
    struct FlumeEntry *entry = FlumeEntryList;
    int best = 0;

    while (entry != NULL) {
        if (entry->field_d0 > best) {
            if (filter == 0 || FUN_0040ba80((struct Node *)entry) != 0) {
                best = entry->field_d0;
            }
        }
        entry = entry->next;
    }
    return best;
}
