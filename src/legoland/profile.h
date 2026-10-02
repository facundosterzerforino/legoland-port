#pragma once

#include "legoland.h"

struct IconNode;
struct Sprite;
struct Profile;

LEGO_EXPORT void CloseFontEndCheckBox(void);
LEGO_EXPORT void InitListProfiles(void);
LEGO_EXPORT void PrintProfileDetails(void);
LEGO_EXPORT void EnterNewProfileCheckBoxIcons(struct IconNode *param_1);
LEGO_EXPORT void InitProfileCheckBoxIcons(struct IconNode *param_1);
LEGO_EXPORT void LightUpthisDeleteIcon(struct IconNode *icon, int param_2);
LEGO_EXPORT void UpdateProfileCheckBoxIcons(void);
struct Sprite *GetProfileOffSprite(signed char param_1);
void FUN_0048c860(struct IconNode *param_1);
unsigned char FUN_0048cc30(void *param_1, unsigned int param_2);
unsigned char AcceptProfileClick(unsigned int dummy, unsigned char arg_0);
unsigned char FUN_0048d390(struct Profile *profile, unsigned char param_2);
unsigned char SelectEmptyProfileSlotClick(struct Profile *profile, unsigned int param_2);
unsigned char ConfirmDeleteProfileClick(unsigned int arg0, unsigned int arg1);
unsigned char FUN_0048d450(unsigned int param_1, unsigned int param_2);
void FUN_0048d470(void);
void FUN_0048d490(void);
void FUN_0048d230(void);
LEGO_EXPORT void KillListProfileSprite(void);
