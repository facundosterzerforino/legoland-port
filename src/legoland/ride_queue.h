#pragma once

struct QueueTable;
struct QueueNode;

struct Queue {
    struct QueueTable *count;
    struct QueueNode *head;
    struct QueueNode *tail;
};

unsigned int FUN_004123a0(struct QueueNode *start, struct QueueNode *stop);
void FUN_004123c0(struct QueueNode *start, struct Queue *queue);
struct QueueNode *GetNthNextQueueNode(struct QueueNode *node, int n);
void FUN_00412490(struct QueueNode *start, struct Queue *queue);

struct PathPair {
    /* 0x00 */ int a;
    /* 0x04 */ int b;
};

struct PathTable {
    /* 0x00 */ int count;
    /* 0x04 */ struct PathPair *pairs;
};

int FUN_00412100(struct PathTable *param_1);

struct RideQueueEntry {
    /* 0x00 */ struct RideQueueEntry *next;
    /* 0x04 */ struct RideQueueEntry *field_4;
    /* 0x08 */ unsigned short id;
    /* 0x0a */ unsigned char pad_a[0x2];
    /* 0x0c */ int x;
    /* 0x10 */ int y;
    /* 0x14 */ unsigned char field_14;
    /* 0x15 */ unsigned char field_15;
    /* 0x16 */ unsigned char pad_16[0x2];
    /* 0x18 */ struct RideQueueEntry *field_18;
};

unsigned int FUN_00411e60(struct Queue *queue);
unsigned int QueueHasNodes(struct Queue *queue);
int FUN_00411ea0(struct Queue *queue);
void FreeAllQueueNodes(struct Queue *queue);
void QueueUnlinkHead(struct Queue *queue);
struct QueueItemMid;
void FUN_00412060(struct Queue *queue, struct QueueItemMid **out);
void FUN_004120a0(struct Queue *queue, unsigned int param_2, unsigned int param_3);
void FreeIfNotNull(void *param_1);
struct RideSlotArg;
struct RideSlot;
struct QueueTable;
struct Bloke;
void FUN_004122a0(struct RideSlotArg *param_1, struct RideSlot *slot);
void FUN_004122d0(struct RideSlotArg *param_1, struct RideSlot *slot);
struct RideSlotArg *FUN_004122f0(struct RideSlot *slot);
void FUN_00412300(struct QueueTable *table, int x, int y, struct Bloke *bloke);
void *FUN_004125f0(unsigned int a, unsigned int b);
struct RideQueueEntry *FUN_004125a0(int x, int y);
struct RideQueueEntry *FUN_00412650(unsigned short param_1);
void FUN_00412680(int x, int y, int param_3, int param_4);
struct QueueItemMid;
void FUN_00411f20(struct Queue *queue, struct QueueItemMid *mid);

extern unsigned short DAT_004b4c08[15][4];
