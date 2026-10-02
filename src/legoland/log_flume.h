#pragma once

/* [port] declared at file scope so prototypes below refer to the same type */
struct FlumeMover;
struct Footprint;

struct CallbackTable;
struct ClassNode;
struct FlumeEntry;
struct FlumeDims;
struct ParticleEmitter;
struct Context;
struct LinkList;

void FUN_004113d0(void);
struct FlumeSlot;
int FUN_00411650(struct FlumeSlot *slot);
int FUN_00411680(struct FlumeMover *mover);
void FUN_00411810(struct FlumeMover *m);
void FUN_0040d090(struct FlumeEntry *entry, struct Footprint **out, TileId *tile);

struct FlumeDims FUN_004112c0(void);

void FUN_0040da10(struct Context *a, struct LinkList *list);
void FUN_004119a0(struct ParticleEmitter *param_1, unsigned int param_2);

struct FlumeEntry *FUN_0040d210(int x, int y);

struct Point FUN_0040cfd0(struct FlumeEntry *entry);

int FUN_004119c0(Element *obj, int filter);

void LogFlume_GetInterfaces(struct ClassNode *flume, struct CallbackTable *vtbl);

struct Cursor;
void FUN_0040d520(struct FlumeEntry **list, struct Cursor *first);
