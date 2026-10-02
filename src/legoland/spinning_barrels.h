#pragma once

struct CallbackTable;
struct ClassNode;
struct Element;
struct RideNode;

struct BarrelNode {
    struct BarrelNode *next;
    unsigned short tile_id;
    unsigned char seated_count;
    unsigned char leaving_count;
    signed char frame;
    unsigned char pad_9[3];
    unsigned int flags;
    unsigned char cycles_left;
    unsigned char pad_11[3];
    int frame_ticks;
    unsigned char boarding_count;
    unsigned char pad_19[3];
    unsigned int boarding_timer;
    signed char layer2_frame;
    signed char slots[0x34 - 0x21];
};

void FUN_0043c2f0(struct BarrelNode *node);
void FUN_0043c950(struct Element *elem);
unsigned int FUN_0043ce10(struct RideNode *rn, struct BarrelNode *node, signed char n);

void SpinningBarrelsGetInterfaces(struct ClassNode *str, struct CallbackTable *ride);
