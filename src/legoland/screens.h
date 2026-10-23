#pragma once

void GetProductVersion(char *param_1);
// FUN_00458930 is the game's own __ftol (compiler float->int conversion helper).
int _ftol();
#define FUN_00458930 _ftol
void ResetEditState(void);
void StartLevelGame(void);
void UnloadMap(void);
void SetMapLoaded(unsigned int param_1);
void MarkLevelComplete(void);
void FUN_00459520(void);
void FUN_004597e0(int param0, const char *param1);
void EndLevel(unsigned int a1);
void RenderFrontEndScreen(unsigned char param_1);
void GameProc(void);
void RenderMapScreenFrame(void);
