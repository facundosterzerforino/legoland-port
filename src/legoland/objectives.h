#pragma once

struct NerpsArg;
struct RewardArg;
struct MapRectArg;

struct Vec4 {
    unsigned int x;
    unsigned int y;
    unsigned int z;
    unsigned int w;
};

/* Canonical objective-event node (~0x44 bytes), allocated by AllocObjectiveEvent.
 * objectives.c owns the allocator and treats it as ObjectiveEvent; nerps.c
 * previously viewed the same object as EventNode/EventNodeVec. Unified here so
 * the boundary casts disappear. */
struct ObjectiveEvent {
    struct ObjectiveEvent *next;
    unsigned int field_4;
    unsigned int field_8;
    unsigned int type;
    unsigned char flags_10;
    unsigned char pad_11[0x14 - 0x11];
    unsigned int field_14;
    unsigned int field_18;
    unsigned int field_1c;
    unsigned int field_20;
    unsigned int field_24;
    struct Vec4 vec_28;
    int sort_key;
    unsigned int timestamp;
    unsigned int field_40;
};

void SetHintsFileName(char *name);
void ClearBriefingAndHintsFileNames(void);
void ClearObjectiveCounters(void);
void SetObjectiveCounter(int index, signed char value);
unsigned char AddObjectiveCounter(int index, unsigned char value);
char GetObjectiveCounter(int index);
void FUN_004688e0(void);
void FUN_004688f0(int index, unsigned char param_2);
struct ObjectiveEvent *AllocObjectiveEvent(unsigned int type, int sort_key);
void FreeObjectiveEvent(struct ObjectiveEvent *node);
void FreeObjectiveEventList(struct ObjectiveEvent *node);
void FreeScriptStrings(void);
unsigned int AddScriptString(char *param_1, char *param_2, int param_3);
void InsertObjectiveMessageSorted(struct ObjectiveEvent *node);
void SetObjectiveEventText(struct ObjectiveEvent *node, unsigned int param_2, unsigned int param_3);
struct ObjectiveEvent *PostObjectiveMessage(const char *format, ...);
void ShowNextObjectiveMessage(void);
void ResetObjectiveHintTimer(void);
int IsObjectiveHintDue(void);
int QueueCustomObjectiveHint(struct NerpsArg *arg);
void QueueBuildMoreObjectsHint(struct NerpsArg *object, unsigned int a, int b);
void QueueConnectToPathHint(struct NerpsArg *object, unsigned int a);
void QueueLinkToEntranceHint(struct NerpsArg *object, unsigned int a);
void QueueClearAreaHint(struct NerpsArg *object, int a);
void QueueBuildRangeAttractionsHint(struct NerpsArg *arg, unsigned int class_id, int count, int sum);
void QueueDeleteRangeAttractionsHint(struct NerpsArg *arg, unsigned int class_id, int count, int sum);
void QueueDeleteObjectsHint(struct NerpsArg *arg, unsigned int class_id, int count);
void QueueAttractVisitorsHint(struct NerpsArg *object, int count);
void QueueMoreGardenersHint(struct NerpsArg *object, int a);
void QueueFewerGardenersHint(struct NerpsArg *object, int a);
void QueueMoreMechanicsHint(struct NerpsArg *object, unsigned int a);
void QueueFewerMechanicsHint(struct NerpsArg *object, int a);
void QueueSaveCoinsHint(struct NerpsArg *arg, int count);
void QueueFewerHungryVisitorsHint(struct NerpsArg *object, unsigned int a, unsigned int b);
void QueueMoreFedVisitorsHint(struct NerpsArg *object, unsigned int a, unsigned int b);
void QueueHappyVisitorsHint(struct NerpsArg *object, int a, unsigned int b);
void QueueRepairObjectsHint(struct NerpsArg *arg, int param_2, unsigned int param_3);
void QueueRideVisitorsHint(struct NerpsArg *object, unsigned int a, unsigned int b);
void QueueAddRidePartsHint(struct NerpsArg *arg, unsigned int class_id, int sum, int count);
void QueueCoverSquaresHint(struct NerpsArg *object, unsigned int param_2, int count);
void QueuePathSceneryHint(struct NerpsArg *object, int count);
void QueueCustomObjectiveHintIfDue(struct NerpsArg *object);
void PostPendingObjectiveHints(void);
void PlaceObjectAtTile(unsigned int a, void *b);
int ScriptEventPlace(struct ObjectiveEvent *event);
int ScriptEventGive(struct ObjectiveEvent *event);
int FUN_00469b50(struct ObjectiveEvent *event);
int ScriptEventTake(struct ObjectiveEvent *event);
int ObjectiveEventAddBricks(struct ObjectiveEvent *event);
int ObjectiveEventSetBricks(struct ObjectiveEvent *event);
int ScriptEventClear(struct MapRectArg *arg);
int ScriptEventUnglue(struct MapRectArg *arg);
int ScriptEventGlue(struct MapRectArg *arg);
int ScriptEventExtendPark(struct RewardArg *arg);
int ScriptEventFmv(struct RewardArg *arg);
int ScriptEventInterval(struct RewardArg *arg);
int ScriptEventMessage(struct RewardArg *arg);
void TakeRewardObject(struct NerpsArg *object);
int ProcessUnimplementedReward(struct RewardArg *arg);
int ProcessUnimplementedObjective(struct RewardArg *arg);
void GiveRewardObject(struct NerpsArg *object, unsigned int a, unsigned int b);
