#pragma once

#include "llidb.h"

struct BlokeArg;
struct RideNode;
struct EateryObj;
struct EditArg;
struct UserFlagsArg;

struct SaveBlock;
struct SaveBlock *FindSaveBlock(unsigned short *param);

void LoadChuckWagonResources(struct EateryObj *obj);
void UnloadChuckWagonResources();
void RenderChuckWagon(struct BlokeArg *arg, unsigned int param2, unsigned int param3, unsigned short *value);
void FUN_0042e2a0(Element *obj);
void LoadBrollyImages(struct EateryObj *obj);
void UnloadBrollyImages();
void BrollySetEditMode();
void BrollyAddObject(int param_1, unsigned char *param_2);
struct RideSpriteInfo *GetBrollySpriteInfo(int param_1, unsigned int param_2);
void LoadSharkCafeResources(struct EateryObj *obj);
void UnloadSharkCafeResources();
void FUN_0042e610(Element *obj);
void LoadFoodcartDrinkResources(struct EateryObj *obj);
void UnloadFoodcartDrinkResources();
void LoadFoodcartIcecreamResources(struct EateryObj *obj);
void UnloadFoodcartIcecreamResources();
void LoadFoodcartFoodResources(struct EateryObj *obj);
void UnloadFoodcartFoodResources();
void FUN_0042e830(struct BlokeArg *arg, unsigned int param2, unsigned int param3, unsigned short *value);
void LoadCastleBBQResources(struct EateryObj *obj);
void UnloadCastleBBQResources();
void EaterySetEditMode(struct EditArg *arg);
void RenderCastleBBQ(int param_1, unsigned int param_2, unsigned int param_3, short *param_4, unsigned int param_5, unsigned int param_6);
void CastleBBQAddObject(unsigned int param_1, unsigned int *param_2);
void CastleBBQRemoveObject(unsigned int param_1, unsigned int param_2, unsigned int param_3);
void FUN_0042ea60(Element *obj);
void FUN_0042ec10(Element *obj);
void FUN_0042ed70(Element *obj);
void Restaurant1AddObject(unsigned int param_1, unsigned char *param_2);
void Restaurant1RemoveObject(unsigned int param_1, TileId tile, unsigned int param_3);
void LoadRestaurant1Resources(struct EateryObj *obj);
void FUN_0042f1a0(Element *obj);
void RenderRestaurant1(int param_1, unsigned int param_2, unsigned int param_3, short *param_4, unsigned int param_5, unsigned int param_6);
void KillRestMaskSpritesAndMoneySFX();
void LoadRestaurant2Resources(struct EateryObj *obj);
void Restaurant2AddObject(unsigned int param_1, unsigned char *param_2);
void Restaurant2RemoveObject(unsigned int arg1, TileId tile, unsigned int arg3, unsigned int arg4, unsigned int arg5);
void Restaurant2Update(int param_1);
void *FUN_004304a0(struct EateryObj *obj, unsigned short a2);
void RenderRestaurant2(int param_1, unsigned int param_2, unsigned int param_3, short *param_4, unsigned int param_5, unsigned int param_6);
void UnloadRestaurant2Resources();
void FUN_00431170(Element *obj);
void EateryRemoveObject(unsigned int param_1, TileId tile, unsigned int param_3);
void LoadOctopusCafeResources(struct EateryObj *obj);
void OctopusCafeAddObject(unsigned int param_1, struct UserFlagsArg *param_2);
void UnloadOctopusCafeResources();
void OctopusCafeUpdate(int param_1);
void RenderOctopusCafe(int param_1, unsigned int param_2, unsigned int param_3, unsigned char *param_4, unsigned int param_5, unsigned int param_6);
int Restaurant1_Save(void);
int Restaurant1_Load(void);
int Restaurant2_Save(void);
int Restaurant2_Load(void);