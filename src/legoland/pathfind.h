#pragma once

#include "legoland.h"

struct Point;

struct Point;

int FUN_00482710(int *a, int *b, int *out);
LEGO_EXPORT int SuggestNextMove(struct Point *pos, struct Point *goal, struct Point *out);
LEGO_EXPORT int PTPSuggestNextMove(struct Point *pos, struct Point *goal, struct Point *out);
LEGO_EXPORT void AddPathSquare(struct Point *pos);
LEGO_EXPORT void RemovePathSquare(struct Point *pos);
void FUN_00482a40(struct Point *pos);
void FUN_00482a80(void);
void FUN_00482a90(void);
void FUN_004828f0(void);
struct Point *GetEntrance1Point(void);
unsigned int FUN_00482860(void);
int FUN_00482920(void);
