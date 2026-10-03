#pragma once

struct EventNode;
struct GameMainNode;
struct Point;
struct QueryNode;
struct ResFile;

int IsInMapBounds(int a, int b);
void FUN_004776c0(struct QueryNode *node);
void InsertOpenListSorted(struct EventNode *node);
struct GameMainNode *FUN_00477730(struct Point *ctx);
struct GameMainNode *FUN_004777c0(struct Point *arg);
struct GameMainNode *FUN_004777f0(struct Point *pos, int *result);
void RemoveQueryNode(struct QueryNode *ctx);
void RemoveFromOpenList(struct EventNode *param_1);

void FUN_004779d0(struct Point *p);
void FUN_00477bd0(int x, int y, int a, int b);
int FindStringNoCase(const char *param_1, const void *param_2, int param_3);
int ParseScriptFile(const char *name, struct ScriptCommand *commands, int count, int flags);
int ParseScriptResFile(struct ResFile *file, struct ScriptCommand *commands, int count, int flags);
void FUN_004784c0(void);
void FUN_004785d0(char *param_1, unsigned int param_2);
void FUN_00478610(unsigned int param_1);
void FUN_00478650(unsigned int param_1, unsigned int param_2);
int FUN_00478690(unsigned int param_1, unsigned int param_2, unsigned int param_3);
int FUN_004786a0(unsigned int param_1, unsigned int param_2, unsigned int param_3);
void ParseRect(int *param_1, char **param_2, int param_3);
unsigned int FUN_004786c0(unsigned int param_1, unsigned int param_2, unsigned int param_3, unsigned int param_4);
void ParseIntPair(int *param_1, char **param_2, int param_3);
unsigned int FUN_004787a0(unsigned int param_1, unsigned int param_2);
