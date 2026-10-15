#pragma once

/* [port] declared at file scope so prototypes below refer to the same type */
struct FlumeMover;
struct Footprint;

struct CallbackTable;
#include "ride_queue.h"

struct FlumeNode {
    struct FlumeNode *next;
    struct FlumeNode *prev;
    struct FlumeNode *field_8;
    struct FlumeNode *field_c;
    union {
        unsigned int flags;
        struct FlumeEntry *sub; /* as FlumeEntry has it */
    };
    TileId tile;
    unsigned char pad_16[2];
    union {
        int mode;
        struct FlumeEntry *link;
    };
    int submode;
    struct Ride *ride;
    struct FlumeEntry *owner;
    struct FlumeNode *entry;
    struct FlumeNode *field_2c;
    struct FlumeNode *field_30;
    struct FlumeNode *field_34;
};

struct FlumeSlot {
    int timer;
    unsigned int flags;
    struct RideNode *busy; /* the rider in the slot */
    int x;
    int y;
    union {
        struct FlumeEntry *owner; /* the track piece the slot is on */
        struct Element *element; /* particle pieces */
    };
    float weight;
    int field_1c;
    float field_20;
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
    /* 0x0c */ struct FlumeEntry *field_c;
    /* 0x10 */ union {
        struct FlumeEntry *sub;
        struct ListNode *list_head; /* list pieces */
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
    /* 0x20 */ union {
        struct Ride *ride;
        int timer; /* slot sets */
    };
    /* 0x24 */ union {
        struct FlumeSlotSet *slotset;
        struct FlumeEntry *parent;
        struct FlumeMover *field_24;
        int ticks; /* slot sets */
    };
    /* 0x28 */ union {
        int field_28;
        struct FlumeEntry *link28;
    };
    /* 0x2c */ union {
        struct {
            union {
                struct FlumeEntry *sub2;
                struct ListNode *list_tail; /* list pieces */
            };
            union {
                struct FlumeRunNode *first;
                struct FlumeEntry *link30;
                int a_30; /* edges */
            };
            union {
                struct FlumeRunNode *last;
                struct FlumeEntry *link34;
                int b_34;
            };
        };
        struct Queue queue;
    };
    /* 0x38 */ struct FlumeSlot *target; /* one of its slots */
    /* 0x3c */ int count;
    /* 0x40 */ struct FlumeSlot slots[4];
    /* 0xd0 */ int field_d0;
};

struct FlumeEntry;
struct ListNode;
struct FlumeDims;
struct LinkList;

void FUN_004113d0(void);
struct FlumeSlot;
int FUN_00411650(struct FlumeSlot *slot);
struct FlumeMover;
int AdvanceFlumeMover(struct FlumeMover *mover);
void FUN_00411810(struct FlumeMover *m);
void FUN_0040d090(struct FlumeEntry *entry, struct Footprint **out, TileId *tile);

struct FlumeDims FUN_004112c0(void);

void FUN_0040da10(struct FlumeEntry *a, struct LinkList *list);
void FUN_004119a0(struct FlumeEntry *param_1, unsigned int param_2);

struct FlumeEntry *FUN_0040d210(int x, int y);

struct Point FUN_0040cfd0(struct FlumeEntry *entry);

int FUN_004119c0(Element *obj, int filter);

struct Ride;
void LogFlume_GetInterfaces(struct Element *flume, struct Ride *vtbl);

struct Cursor;
void FUN_0040d520(struct FlumeEntry **list, struct Cursor *first);
