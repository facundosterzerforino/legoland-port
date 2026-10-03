#include <string.h>
#include "legoland.h"

#include "gamemap.h"
#include "globals.h"
#include "map_object.h"
#include "obj_instance.h"
#include "objclass.h"
#include "temple.h"

#include "bloke.h"
#include "image_sprite.h"
#include "man3d.h"
#include "math.h"
#include "print_sprite.h"
#include "render3d.h"
#include "ride_queue.h"

// FUNCTION: LEGOLAND 0x004169c0
void LoadTempleMatteSprites(Element *obj) {
    TempleBuildingRide = obj->ride;
    if (TempleBuildingRide != NULL) {
        TempleBuildingRide->flags |= 0x20;
        if (TempleBuildingRide->layer != NULL) {
            TempleBuildingRide->layer->flags |= 0x2000;
            TempleLayer = TempleBuildingRide->layer;
        }
    }
    // STRING: LEGOLAND 0x004b4edc
    TempleMatte1Sprite = LoadSprite("temple_matte1.lls", 1);
    // STRING: LEGOLAND 0x004b4ec8
    TempleMatte2Sprite = LoadSprite("temple_matte2.lls", 1);
}

// FUNCTION: LEGOLAND 0x00416a30
void KillTempleMatteSprites(void) {
    if (TempleMatte1Sprite != 0) {
        KillSprite(TempleMatte1Sprite);
    }
    if (TempleMatte2Sprite != 0) {
        KillSprite(TempleMatte2Sprite);
    }
}

// FUNCTION: LEGOLAND 0x00416a60
void RenderTemple(Element *obj, unsigned int param_2, unsigned int param_3, unsigned short *coords, unsigned int param_5, unsigned int clip) {
    struct Ride *ride = obj->ride;
    struct RideNode *node;
    struct Point pos;
    struct Point offset;

    RenderItems_New();
    DAT_004cbf70 = NULL;
    for (node = ride->riders; node != NULL; node = node->next) {
        if (*coords == node->tile.id) {
            AddBlokeToRenderList(&DAT_004cbf70, (struct BlokeRenderSrc *)node, node->person->field_20);
        }
    }
    RenderBlokeList((struct BlokeListHead *)&DAT_004cbf70);
    pos = GetScreenCoordsForObject((unsigned char *)coords, ride);
    offset = GetRenderOffsetForLayer(TempleLayer, 0);
    AdjustOffsetForViewMode(&offset);
    PrintSprite(TempleMatte1Sprite, offset.x + pos.x, offset.y + pos.y, clip, 0);
    offset = GetRenderOffsetForLayer(TempleLayer, 3);
    AdjustOffsetForViewMode(&offset);
    PrintSprite(TempleMatte2Sprite, offset.x + pos.x, offset.y + pos.y, clip, 0);
}

// FUNCTION: LEGOLAND 0x00416b50
void FUN_00416b50(Element *obj) {
    struct Ride *ride = obj->ride;
    struct RideNode *node;
    struct RideNode *next;
    struct Bloke *bloke;
    unsigned int x;
    unsigned int y;
    char dir;

    for (node = ride->riders; node != NULL; node = next) {
        next = node->next;
        bloke = node->rider;
        x = ride->x + node->tile.pos.x;
        y = ride->y + node->tile.pos.y;
        if (bloke->low_level_action == 0) {
            switch (bloke->param_action) {
            case 0:
                bloke->flags |= 8;
                bloke->dest.x = (x - 2) << 8;
                bloke->dest.y = (y - 4) << 8;
                dir = CalcMoveLine(bloke->pos, bloke->dest, &bloke->nav);
                bloke->field_73 = dir + 0x10;
                bloke->low_level_action = 7;
                NewDirForAction(bloke, ((unsigned char)(dir + 0x10) >> 5) + 3);
                bloke->param_action++;
                break;
            case 1:
                bloke->dest.x = (x - 2) << 8;
                bloke->dest.y = (y << 8) - 0x680;
                dir = CalcMoveLine(bloke->pos, bloke->dest, &bloke->nav);
                bloke->field_73 = dir + 0x10;
                bloke->low_level_action = 7;
                NewDirForAction(bloke, ((unsigned char)(dir + 0x10) >> 5) + 3);
                bloke->param_action++;
                break;
            case 2:
                bloke->dest.x = (x << 8) - 0x260;
                bloke->dest.y = (y - 8) << 8;
                dir = CalcMoveLine(bloke->pos, bloke->dest, &bloke->nav);
                bloke->field_73 = dir + 0x10;
                bloke->low_level_action = 7;
                NewDirForAction(bloke, ((unsigned char)(dir + 0x10) >> 5) + 3);
                bloke->param_action++;
                break;
            case 3:
                bloke->dest.x = (x - 2) << 8;
                bloke->dest.y = (y << 8) - 0x980;
                dir = CalcMoveLine(bloke->pos, bloke->dest, &bloke->nav);
                bloke->field_73 = dir + 0x10;
                bloke->low_level_action = 7;
                NewDirForAction(bloke, ((unsigned char)(dir + 0x10) >> 5) + 3);
                bloke->param_action++;
                break;
            case 4:
                bloke->dest.x = (x << 8) - 0x260;
                bloke->dest.y = (y - 12) << 8;
                dir = CalcMoveLine(bloke->pos, bloke->dest, &bloke->nav);
                bloke->field_73 = dir + 0x10;
                bloke->low_level_action = 7;
                NewDirForAction(bloke, ((unsigned char)(dir + 0x10) >> 5) + 3);
                bloke->param_action++;
                break;
            case 5:
                bloke->dest.x = (x - 2) << 8;
                bloke->dest.y = (y << 8) - 0x980;
                dir = CalcMoveLine(bloke->pos, bloke->dest, &bloke->nav);
                bloke->field_73 = dir + 0x10;
                bloke->low_level_action = 7;
                NewDirForAction(bloke, ((unsigned char)(dir + 0x10) >> 5) + 3);
                bloke->param_action++;
                break;
            case 6:
                bloke->dest.x = (x << 8) - 0x260;
                bloke->dest.y = (y - 8) << 8;
                dir = CalcMoveLine(bloke->pos, bloke->dest, &bloke->nav);
                bloke->field_73 = dir + 0x10;
                bloke->low_level_action = 7;
                NewDirForAction(bloke, ((unsigned char)(dir + 0x10) >> 5) + 3);
                bloke->param_action++;
                break;
            case 7:
                bloke->dest.x = (x - 2) << 8;
                bloke->dest.y = (y << 8) - 0x680;
                dir = CalcMoveLine(bloke->pos, bloke->dest, &bloke->nav);
                bloke->field_73 = dir + 0x10;
                bloke->low_level_action = 7;
                NewDirForAction(bloke, ((unsigned char)(dir + 0x10) >> 5) + 3);
                bloke->param_action++;
                break;
            case 8:
                bloke->dest.y = (y - 4) << 8;
                bloke->dest.x = (x - 2) << 8;
                dir = CalcMoveLine(bloke->pos, bloke->dest, &bloke->nav);
                bloke->field_73 = dir + 0x10;
                bloke->low_level_action = 7;
                NewDirForAction(bloke, ((unsigned char)(dir + 0x10) >> 5) + 3);
                bloke->param_action++;
                break;
            case 9:
                bloke->dest.x = (x << 8) + 0x80;
                bloke->dest.y = (y << 8) + 0x80;
                dir = CalcMoveLine(bloke->pos, bloke->dest, &bloke->nav);
                bloke->field_73 = dir + 0x10;
                bloke->low_level_action = 7;
                NewDirForAction(bloke, ((unsigned char)(dir + 0x10) >> 5) + 3);
                bloke->param_action++;
                break;
            case 10:
                RemoveBlokeFromRide(ride, node);
                bloke->flags &= ~8;
                break;
            }
        }
    }
}

// FUNCTION: LEGOLAND 0x00416dc0
void TempleSetEditMode(void) {
    void *temp = TempleBuildingRide;
    EditMode.unk0 = 1;
    EditMode.unk8 = temp;
    DefaultCursor(&EditCursor);
    SetEditCursorFootPrint((void *)((unsigned int)EditMode.unk8 + 0x3c));
}

// FUNCTION: LEGOLAND 0x00416e00
void TempleAddObject(unsigned int param_1, unsigned int param_2) {
    AddBasicObject(param_1, param_2);
}

// FUNCTION: LEGOLAND 0x00416e20
void TempleRemoveObject(Element *a1, TileId tile, unsigned int a3) {
    StandardRemoveObject((unsigned int)a1, tile, a3);
    RemoveAllBlokesFromRide(a1->ride, tile);
}

// FUNCTION: LEGOLAND 0x00416e50
void Temple_GetInterfaces(struct ClassNode *str, struct CallbackTable *obj) {
    // STRING: LEGOLAND 0x004b4ef0
    if (_stricmp("TEMPLE", str->name) == 0) {
        obj->cb_a4 = LoadTempleMatteSprites;
        obj->cb_ac = KillTempleMatteSprites;
        obj->cb_8c = TempleSetEditMode;
        obj->cb_a8 = FUN_00416b50;
        obj->cb_b0 = RenderTemple;
        obj->cb_9c = TempleRemoveObject;
        obj->cb_98 = TempleAddObject;
    }
}
