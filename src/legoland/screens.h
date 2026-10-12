#pragma once

void GetProductVersion(char *param_1);
// FUN_00458930 is the game's own __ftol (compiler float->int conversion helper).
int _ftol();
#define FUN_00458930 _ftol
void FUN_00458940(void);
void FUN_00458a50(void);
void UnloadMap(void);
void SetMapLoaded(unsigned int param_1);
void FUN_00458be0(void);
void FUN_00459520(void);
void FUN_004597e0(int param0, const char *param1);
void EndLevel(unsigned int a1);
void RenderFrontEndScreen(unsigned char param_1);
void GameProc(void);
void FUN_00459360(void);
