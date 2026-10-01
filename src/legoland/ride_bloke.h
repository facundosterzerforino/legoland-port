#pragma once

/* [port] declared at file scope so prototypes below refer to the same type */
struct RideBloke;

struct HistPair {
    int a;
    int b;
};

struct HistBuf {
    unsigned char pad_00[0x30];
    /* 0x30 */ struct HistPair e[17];
    unsigned char pad_b8[3];
    /* 0xbb */ unsigned char count;
};

void FUN_00401e00(struct HistBuf *p);
struct PathPair;
int FUN_00401f30(unsigned short id, struct PathPair *p, int dir);
int FUN_00402150(unsigned short id, struct PathPair *p, int dir);
int FUN_00401ae0(unsigned short id, int bloke);
unsigned int FUN_00401c40(unsigned short arg0);
void FUN_00401c60(struct RideBloke *b);
void FUN_00402c10(void);
