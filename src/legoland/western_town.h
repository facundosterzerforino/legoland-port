#pragma once

#include "legoland.h"
#include "llidb.h"

struct MapObject;
struct EditObject;
struct Cursor;

/* Callback-table entry points for the western-town shops (General Store,
 * Sheriff, Jail Cells, Bank, Saloon), registered by the shops dispatcher
 * (ShopsGetInterfaces). */
void LoadGStoreMatteSpritesAndMoneySFX(struct MapObject *obj);
void KillGStoreMatteSpritesAndMoneySFX(void);
void GeneralStoreSetEditMode(void);
void RenderGeneralStore(Element *obj, unsigned int param_2, unsigned int param_3, unsigned short *tile, unsigned int param_5, unsigned int param_6);
void GeneralStoreUpdate(struct MapObject *param_1);
void LoadSherifshutMatteSpriteAndMoneySFX(struct MapObject *obj);
void KillSherifshutMatteSpriteAndMoneySFX(void);
void SheriffSetEditMode(void);
void RenderSheriff(struct MapObject *param_1, unsigned int param_2, unsigned int param_3, unsigned short *param_4, unsigned int param_5, unsigned int param_6);
void SheriffUpdate(struct MapObject *param_1);
void JailCellAddObject(struct EditObject *editObj, int *coords);
void JailCellRemoveObject(struct MapObject *editObj, TileId coords, struct Cursor *cursor);
void JailCellLoad(struct MapObject *obj);
void JailCellUnload(void);
void JailCellSetEditMode(void);
void RenderJailCell(Element *obj, unsigned int param_2, unsigned int param_3, unsigned short *tile, unsigned int param_5, unsigned int param_6);
void JailCellUpdate(Element *obj);
LEGO_EXPORT unsigned int SaveJailCells(void);
LEGO_EXPORT unsigned int LoadJailCells(void);
void LoadBankMatteSpriteAndMoneySFX(struct MapObject *obj);
void KillBankMatteSpriteAndMoneySFX(void);
void BankSetEditMode(void);
void RenderBank(struct MapObject *param_1, unsigned int param_2, unsigned int param_3, unsigned short *param_4, unsigned int param_5, unsigned int param_6);
void BankUpdate(Element *obj);
void LoadSaloonMatteSpritesAndMoneySFX(struct MapObject *obj);
void KillSaloonMatteSpritesAndMoneySFX(void);
void SaloonSetEditMode(void);
void RenderSaloon(Element *obj, unsigned int param_2, unsigned int param_3, unsigned short *tile, unsigned int param_5, unsigned int param_6);
void SaloonUpdate(Element *obj);
