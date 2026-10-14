#pragma once

#include "llidb.h"

struct RideNode;
struct Element;
struct EditArg;
struct UserFlagsArg;

struct SaveBlock;
struct SaveBlock *FindSaveBlock(unsigned short *param);

void LoadChuckWagonResources(struct Element *obj);
void UnloadChuckWagonResources();
void RenderChuckWagon(struct Element *arg, unsigned int param2, unsigned int param3, unsigned short *value);
void ChuckWagonUpdate(Element *obj);
void LoadBrollyImages(struct Element *obj);
void UnloadBrollyImages();
void BrollySetEditMode();
void BrollyAddObject(struct Element *param_1, unsigned char *param_2);
struct RideSpriteInfo *GetBrollySpriteInfo(struct Element *param_1, unsigned int param_2);
void LoadSharkCafeResources(struct Element *obj);
void UnloadSharkCafeResources();
void SharkCafeUpdate(Element *obj);
void LoadFoodcartDrinkResources(struct Element *obj);
void UnloadFoodcartDrinkResources();
void LoadFoodcartIcecreamResources(struct Element *obj);
void UnloadFoodcartIcecreamResources();
void LoadFoodcartFoodResources(struct Element *obj);
void UnloadFoodcartFoodResources();
void FUN_0042e830(struct Element *arg, unsigned int param2, unsigned int param3, unsigned short *value);
void LoadCastleBBQResources(struct Element *obj);
void UnloadCastleBBQResources();
void EaterySetEditMode(struct EditArg *arg);
void RenderCastleBBQ(struct Element *param_1, unsigned int param_2, unsigned int param_3, short *param_4, void *param_5, unsigned int param_6);
void CastleBBQAddObject(struct Element *param_1, unsigned int *param_2);
struct Cursor;
void CastleBBQRemoveObject(struct Element *param_1, unsigned int param_2, struct Cursor *param_3);
void CastleBBQUpdate(Element *obj);
void FoodcartDrinkUpdate(Element *obj);
void FoodcartFoodUpdate(Element *obj);
void Restaurant1AddObject(struct Element *param_1, unsigned char *param_2);
void Restaurant1RemoveObject(struct Element *param_1, TileId tile, struct Cursor *param_3);
void LoadRestaurant1Resources(struct Element *obj);
void Restaurant1Update(Element *obj);
void RenderRestaurant1(struct Element *param_1, unsigned int param_2, unsigned int param_3, short *param_4, void *param_5, unsigned int param_6);
void KillRestMaskSpritesAndMoneySFX();
void LoadRestaurant2Resources(struct Element *obj);
void Restaurant2AddObject(struct Element *param_1, unsigned char *param_2);
void Restaurant2RemoveObject(struct Element *arg1, TileId tile, struct Cursor *arg3, unsigned int arg4, unsigned int arg5);
void Restaurant2Update(Element *obj);
void *GetRestaurant2SpriteInfo(struct Element *obj, unsigned short a2);
void RenderRestaurant2(struct Element *param_1, unsigned int param_2, unsigned int param_3, short *param_4, void *param_5, unsigned int param_6);
void UnloadRestaurant2Resources();
void FoodcartIcecreamUpdate(Element *obj);
void EateryRemoveObject(struct Element *param_1, TileId tile, struct Cursor *param_3);
void LoadOctopusCafeResources(struct Element *obj);
void OctopusCafeAddObject(struct Element *param_1, struct UserFlagsArg *param_2);
void UnloadOctopusCafeResources();
void OctopusCafeUpdate(Element *obj);
void RenderOctopusCafe(struct Element *param_1, unsigned int param_2, unsigned int param_3, unsigned char *param_4, void *param_5, unsigned int param_6);
int Restaurant1_Save(struct Element *element);
int Restaurant1_Load(struct Element *element);
int Restaurant2_Save(struct Element *element);
int Restaurant2_Load(struct Element *element);