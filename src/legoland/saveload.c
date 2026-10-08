#include "legoland.h"

#include <errno.h>
#include <fcntl.h>
#include <io.h>
#include <stdlib.h>
#include <string.h>
#include "bloke.h"
#include "bloke_ai.h"
#include "bricks.h"
#include "challenge.h"
#include "debug_alloc.h"
#include "draw.h"
#include "gamemap.h"
#include "globals.h"
#include "input.h"
#include "interface.h"
#include "llidb.h"
#include "main.h"
#include "man3d.h"
#include "map_object.h"
#include "nerps.h"
#include "obj_instance.h"
#include "pathfind.h"
#include "render3d.h"
#include "saveload.h"
#include "screens.h"
#include "tilemap.h"
#include "timer.h"
#include "worker.h"

/* Abort a load: close the file and report failure. */
#pragma auto_inline(off)
static int LoadAbort(void) {
    _close(SaveFileHandle);
    LoadInProgress = 0;
    return 0;
}
#pragma auto_inline(on)
#define LOAD_FAIL() return LoadAbort()

/* A node of OverlayList (the overlays saved at the end of a save game). */
struct OverlayNode {
    struct OverlayParam param;
    unsigned int pad_14[2];
    struct OverlayNode *next;
};

// FUNCTION: LEGOLAND 0x0047d790
LEGO_EXPORT int BeginMeasuredBlock(void) {
    int pos;
    int marker;

    pos = _tell(SaveFileHandle);
    marker = 0;
    if (pos == -1) {
        return 0;
    }
    MeasuredBlockStarts[MeasuredBlockDepth] = pos;
    MeasuredBlockDepth = MeasuredBlockDepth + 1;
    return SaveGameWrite(&marker, 4) != 0;
}

// FUNCTION: LEGOLAND 0x0047d7e0
int SkipSaveGameDword(void) {
    unsigned int buffer;

    return SaveGameRead(&buffer, 4);
}

// FUNCTION: LEGOLAND 0x0047d800
LEGO_EXPORT int EndMeasuredBlock(void) {
    int end_pos;

    end_pos = _tell(SaveFileHandle);
    if (end_pos == -1) {
        return 0;
    }
    MeasuredBlockDepth = MeasuredBlockDepth - 1;
    if (_lseek(SaveFileHandle, MeasuredBlockStarts[MeasuredBlockDepth], 0) == -1) {
        return 0;
    }
    if (SaveGameWrite(&end_pos, 4) == 0) {
        return 0;
    }
    return _lseek(SaveFileHandle, end_pos, 0) != -1;
}

// FUNCTION: LEGOLAND 0x0047d880
LEGO_EXPORT int FindeIneList(union SavedElement *handle) {
    int i;

    for (i = 0; i < SavedElementCount; i++) {
        if (handle->element == SavedElementTable[i]) {
            handle->index = i;
            return 1;
        }
    }
    handle->index = -1;
    return 0;
}

// FUNCTION: LEGOLAND 0x0047d8c0
LEGO_EXPORT struct Element *GeteListPtr(int idx) {
    if (idx == -1) {
        return 0;
    }
    return SavedElementTable[idx];
}

// FUNCTION: LEGOLAND 0x0047d8e0
LEGO_EXPORT int SaveGame(char *filename) {
    // STRING: LEGOLAND 0x004bcb6c
    char header[0x21] = "00002 LEGOLAND Save Game V0.02 \x1a";
    int i;
    int k;
    int x;
    int n_elems;
    int tab_count;
    struct MapElement tile;
    struct FXSpriteList *tab[256];

    SaveFileHandle = _open(filename, _O_RDWR | _O_CREAT | _O_TRUNC | _O_BINARY, 0x180);
    if (SaveFileHandle == -1) {
        switch (errno) {
        case EACCES:
            // STRING: LEGOLAND 0x004bcb14
            LogPrintf("can't open read-only file for writing, or file\x92s sharing mode forbids operations (%s)\n", filename);
            return 0;
        case EINVAL:
            // STRING: LEGOLAND 0x004bcaec
            LogPrintf("Invalid oflag or pmode argument (%s)\n", filename);
            return 0;
        case EMFILE:
            // STRING: LEGOLAND 0x004bcab0
            LogPrintf("No more file handles available (too many open files) (%s)\n", filename);
            return 0;
        case ENOENT:
            // STRING: LEGOLAND 0x004bca90
            LogPrintf("File or path not found (%s)\n", filename);
            return 0;
        default:
            // STRING: LEGOLAND 0x004bca68
            LogPrintf("Unknown error (%d) openning file %s\n", errno, filename);
            return 0;
        }
    }
    MeasuredBlockDepth = 0;
    if (SaveGameWrite(header, 0x20) == 0) {
        // STRING: LEGOLAND 0x004bca54
        LogPrintf("Header write failed");
        goto fail;
    }
    if (BeginMeasuredBlock() == 0) {
        goto fail;
    }
    if (BeginMeasuredBlock() == 0) {
        // STRING: LEGOLAND 0x004bca38
        LogPrintf("Measured block begin failed");
        goto fail;
    }
    {
        struct Element *elem;
        int flags;
        int len;
        unsigned int *fl;
        n_elems = LLIDB_GetCount();
        SavedElementCount = 0;
        // STRING: LEGOLAND 0x004b8a70
        fl = &ElemID("PATH CONTROL")->flags;
        *fl |= 4;
        for (i = 0; i < n_elems; i++) {
            LLIDB_GetElement(i, &elem);
            if ((elem->flags & 4) != 0) {
                SavedElementCount++;
            }
        }
        if (SaveGameWrite(&SavedElementCount, 4) == 0) {
            // STRING: LEGOLAND 0x004bca24
            LogPrintf("Num elements failed");
            goto fail;
        }
        if (SavedElementTable != 0) {
            free(SavedElementTable);
        }
        SavedElementTable = malloc(SavedElementCount * 4);
        k = 0;
        for (i = 0; i < n_elems; i++) {
            DrawWatchSprite();
            LLIDB_GetElement(i, &elem);
            if ((elem->flags & 4) != 0) {
                len = strlen(elem->name);
                if (SaveGameWrite(&len, 4) == 0) {
                    goto fail;
                }
                if (SaveGameWrite(elem->name, len) == 0) {
                    // STRING: LEGOLAND 0x004bc9e8
                    {
                        LogPrintf("Element name write failed %s", elem->name);
                        goto fail;
                    }
                }
                SavedElementTable[k++] = elem;
                flags = elem->flags & 0x3000e;
                if (SaveGameWrite(&flags, 4) == 0) {
                    // STRING: LEGOLAND 0x004bc9c4
                    {
                        LogPrintf("Flags of interest write failed %s", elem->name);
                        goto fail;
                    }
                }
            }
        }
        tab_count = FUN_0045aa50(tab);
        if (SaveGameWrite(&tab_count, 4) == 0) {
            // STRING: LEGOLAND 0x004bca08
            LogPrintf("TSF Pointers write failed");
            goto fail;
        }
        for (i = 0; i < tab_count; i++) {
            DrawWatchSprite();
            LLIDB_FindElementFromDataPtr(tab[i], (unsigned int *)&elem, 0);
            len = strlen(elem->name);
            if (SaveGameWrite(&len, 4) == 0) {
                // STRING: LEGOLAND 0x004bc980
                {
                    LogPrintf("TSF element length write failed %s, %d", elem->name, len);
                    goto fail;
                }
            }
            if (SaveGameWrite(elem->name, len) == 0) {
                // STRING: LEGOLAND 0x004bc95c
                {
                    LogPrintf("TSF Element name writer failed %s", elem->name);
                    goto fail;
                }
            }
        }
    }
    if (EndMeasuredBlock() == 0) {
        // STRING: LEGOLAND 0x004bc9a8
        LogPrintf("End measured block1 failed");
        goto fail;
    }
    if (BeginMeasuredBlock() == 0) {
        // STRING: LEGOLAND 0x004bc93c
        LogPrintf("Begin Measured block2 failed");
        goto fail;
    }
    if (SaveGameWrite(lpConfig, 0x44) == 0) {
        // STRING: LEGOLAND 0x004bc920
        LogPrintf("Host Config write failed");
        goto fail;
    }
    {
        int y;
        for (y = 0; y < lpConfig->height; y++) {
            for (x = 0; x < lpConfig->width; x++) {
                struct FXSpriteList *src;
                tile = GameMap[y][x];
                DrawWatchSprite();
                if ((tile.flags & 0x8a8) != 0) {
                    if (FindeIneList((union SavedElement *)&tile) == 0) {
                        // STRING: LEGOLAND 0x004bc8d8
                        {
                            LogPrintf("Failed to locate Object instance at (%d,%d)", x, y);
                            goto fail;
                        }
                    }
                } else {
                    tile.field_0 = 0;
                }
                src = TileSpriteInfo[tile.field_8].src;
                for (k = 0; k < tab_count && src != tab[k]; k++) {
                }
                if (k != tab_count) {
                    tile.field_8 = (tile.field_8 - *(unsigned short *)src) | ((k + 1) << 8);
                } else {
                    tile.field_8 = 0;
                }
                src = TileSpriteInfo[tile.field_a].src;
                for (k = 0; k < tab_count && src != tab[k]; k++) {
                }
                if (k != tab_count) {
                    tile.field_a = (tile.field_a - *(unsigned short *)src) | ((k + 1) << 8);
                } else {
                    tile.field_a = 0;
                }
                if (SaveGameWrite(&tile, 0x14) == 0) {
                    // STRING: LEGOLAND 0x004bc8ac
                    {
                        LogPrintf("Failed to write mapinfo struct at (%d,%d)", x, y);
                        goto fail;
                    }
                }
            }
        }
    }
    if (EndMeasuredBlock() == 0) {
        // STRING: LEGOLAND 0x004bc904
        LogPrintf("EndMeasured Block2 failed");
        goto fail;
    }
    if (BeginMeasuredBlock() == 0) {
        // STRING: LEGOLAND 0x004bc88c
        LogPrintf("Begin Measured block3 failed");
        goto fail;
    }
    if (SaveGameWrite(&MapStats, 0x3f0) == 0) {
        // STRING: LEGOLAND 0x004bc874
        LogPrintf("Mapstats write failed");
        goto fail;
    }
    if (SaveGameWrite(&ScrollX, 4) == 0) {
        // STRING: LEGOLAND 0x004bc860
        LogPrintf("Scrollx (%d) Failed", ScrollX);
        goto fail;
    }
    if (SaveGameWrite(&ScrollY, 4) == 0) {
        // STRING: LEGOLAND 0x004bc84c
        LogPrintf("Scrolly (%d) Failed", ScrollY);
        goto fail;
    }
    if (SaveGameWrite(&EditMode, 0xc) == 0) {
        // STRING: LEGOLAND 0x004bc83c
        LogPrintf("EditMode Failed");
        goto fail;
    }
    FUN_00474190();
    if (SaveScripts() == 0) {
        // STRING: LEGOLAND 0x004bc828
        LogPrintf("Scripts Save Failed");
        goto fail;
    }
    if (SaveReport() == 0) {
        // STRING: LEGOLAND 0x004bc814
        LogPrintf("Report Save Failed");
        goto fail;
    }
    if (SaveCurrency() == 0) {
        // STRING: LEGOLAND 0x004bc7fc
        LogPrintf("Currency Save Failed");
        goto fail;
    }
    if (SaveGameWrite(ButtonFlashStates, 0x24) == 0) {
        // STRING: LEGOLAND 0x004bc7dc
        LogPrintf("Button flash states Save Failed");
        goto fail;
    }
    DrawWatchSprite();
    if (EndMeasuredBlock() == 0) {
        // STRING: LEGOLAND 0x004bc7c8
        LogPrintf("EndMeasured Block 3");
        goto fail;
    }
    if (BeginMeasuredBlock() == 0) {
        // STRING: LEGOLAND 0x004bc7b0
        LogPrintf("Begin Measured Block 4");
        goto fail;
    }
    {
        struct Bloke *bloke;
        int n;
        int num;
        n = 0;
        for (bloke = FirstBloke; bloke != 0; bloke = bloke->next) {
            n++;
        }
        if (SaveGameWrite(&n, 4) == 0) {
            // STRING: LEGOLAND 0x004bc794
            LogPrintf("NumBlokes (%d) save Failed", n);
            goto fail;
        }
        for (bloke = FirstBloke; bloke != 0; bloke = bloke->next) {
            DrawWatchSprite();
            num = GetBlokeNum(bloke);
            if (SaveGameWrite(&num, 4) == 0) {
                // STRING: LEGOLAND 0x004bc764
                {
                    LogPrintf("Bloke Num (d) Save Failed", num);
                    goto fail;
                }
            }
            BlokeSaveBuffer.action = bloke->action;
            BlokeSaveBuffer.low_level_action = bloke->low_level_action;
            BlokeSaveBuffer.pending_action = bloke->pending_action;
            if (bloke->target != 0) {
                BlokeSaveBuffer.target = (int)bloke->target;
                FindeIneList((union SavedElement *)&BlokeSaveBuffer.target);
            } else {
                BlokeSaveBuffer.target = -1;
            }
            if (bloke->last_ride != 0) {
                BlokeSaveBuffer.last_ride = (int)bloke->last_ride;
                FindeIneList((union SavedElement *)&BlokeSaveBuffer.last_ride);
            } else {
                BlokeSaveBuffer.last_ride = -1;
            }
            BlokeSaveBuffer.field_1c = bloke->field_1c;
            BlokeSaveBuffer.field_20 = bloke->field_20;
            BlokeSaveBuffer.dest.x = bloke->dest.x;
            BlokeSaveBuffer.dest.y = bloke->dest.y;
            BlokeSaveBuffer.goal.x = bloke->goal.x;
            BlokeSaveBuffer.goal.y = bloke->goal.y;
            memcpy(BlokeSaveBuffer.block_34, &bloke->field_34, sizeof(BlokeSaveBuffer.block_34));
            BlokeSaveBuffer.field_5c = bloke->field_5c;
            BlokeSaveBuffer.param_action = bloke->param_action;
            BlokeSaveBuffer.flags = bloke->flags;
            BlokeSaveBuffer.field_64 = bloke->field_64;
            BlokeSaveBuffer.field_78 = bloke->field_78;
            BlokeSaveBuffer.mood = bloke->mood;
            BlokeSaveBuffer.field_7c = bloke->field_7c;
            BlokeSaveBuffer.field_7e = bloke->field_7e;
            BlokeSaveBuffer.speed = bloke->speed;
            BlokeSaveBuffer.field_80 = bloke->field_80;
            BlokeSaveBuffer.field_81 = bloke->field_81;
            BlokeSaveBuffer.field_82 = bloke->field_82;
            BlokeSaveBuffer.favourite[0] = (int)bloke->favourite_attraction_0;
            FindeIneList((union SavedElement *)&BlokeSaveBuffer.favourite[0]);
            BlokeSaveBuffer.favourite[1] = (int)bloke->favourite_attraction_1;
            FindeIneList((union SavedElement *)&BlokeSaveBuffer.favourite[1]);
            BlokeSaveBuffer.favourite[2] = (int)bloke->favourite_attraction_2;
            FindeIneList((union SavedElement *)&BlokeSaveBuffer.favourite[2]);
            BlokeSaveBuffer.favourite[3] = (int)bloke->favourite_food;
            FindeIneList((union SavedElement *)&BlokeSaveBuffer.favourite[3]);
            BlokeSaveBuffer.pos.x = bloke->pos.x;
            BlokeSaveBuffer.pos.y = bloke->pos.y;
            BlokeSaveBuffer.height = bloke->height;
            BlokeSaveBuffer.dir = bloke->dir;
            BlokeSaveBuffer.field_73 = bloke->field_73;
            BlokeSaveBuffer.frame = bloke->frame;
            BlokeSaveBuffer.field_75 = bloke->field_75;
            BlokeSaveBuffer.nav = bloke->nav;
            BlokeSaveBuffer.person_8 = bloke->person->character;
            BlokeSaveBuffer.scale = bloke->person->scale;
            BlokeSaveBuffer.screen = bloke->person->screen;
            BlokeSaveBuffer.offset = bloke->person->offset;
            BlokeSaveBuffer.field_34 = bloke->person->field_34;
            BlokeSaveBuffer.field_38 = bloke->person->field_38;
            BlokeSaveBuffer.depth = bloke->person->depth;
            BlokeSaveBuffer.rotation = bloke->person->rotation;
            BlokeSaveBuffer.person_4c = bloke->person->frame;
            BlokeSaveBuffer.anim = (unsigned int)bloke->person->anim;
            BlokeSaveBuffer.sort_id = bloke->person->sort_id;
            BlokeSaveBuffer.field_38 = bloke->person->field_38;
            memcpy(BlokeSaveBuffer.m, bloke->person->m, sizeof(BlokeSaveBuffer.m));
            BlokeSaveBuffer.field_7c_p = bloke->person->field_7c;
            BlokeSaveBuffer.field_80_p = bloke->person->field_80;
            BlokeSaveBuffer.field_8c_p = bloke->person->field_8c;
            BlokeSaveBuffer.field_90_p = bloke->person->field_90;
            BlokeSaveBuffer.random = bloke->person->random;
            BlokeSaveBuffer.prev_param = bloke->prev_param;
            BlokeSaveBuffer.prev_action = bloke->prev_action;
            BlokeSaveBuffer.field_30 = bloke->person->field_30;
            if (SaveGameWrite(&BlokeSaveBuffer, sizeof(BlokeSaveBuffer)) == 0) {
                // STRING: LEGOLAND 0x004bc74c
                {
                    LogPrintf("Bloke data failed (%d)", num);
                    goto fail;
                }
            }
            if (BlokeSaveBuffer.block_34[8] != 0) {
                if (SaveGameWrite((void *)BlokeSaveBuffer.block_34[8], 0x48) == 0) {
                    // STRING: LEGOLAND 0x004bc730
                    {
                        LogPrintf("Bloke BNV path data (%d)", num);
                        goto fail;
                    }
                }
            }
        }
    }
    if (EndMeasuredBlock() == 0) {
        // STRING: LEGOLAND 0x004bc780
        LogPrintf("EndMeasured Block 4");
        goto fail;
    }
    if (BeginMeasuredBlock() == 0) {
        // STRING: LEGOLAND 0x004bc718
        LogPrintf("Begin Measured Block 5");
        goto fail;
    }
    SaveGardeners();
    if (EndMeasuredBlock() == 0) {
        // STRING: LEGOLAND 0x004bc704
        LogPrintf("EndMeasuredBlock5");
        goto fail;
    }
    if (BeginMeasuredBlock() == 0) {
        // STRING: LEGOLAND 0x004bc6ec
        LogPrintf("Begin Measured Block 6");
        goto fail;
    }
    DrawWatchSprite();
    SaveMechanics();
    if (EndMeasuredBlock() == 0) {
        // STRING: LEGOLAND 0x004bc6d4
        LogPrintf("End Measured Block 6");
        goto fail;
    }
    if (BeginMeasuredBlock() == 0) {
        // STRING: LEGOLAND 0x004bc6bc
        LogPrintf("Begin measured block 7");
        goto fail;
    }
    SaveGardenerOrders();
    if (EndMeasuredBlock() == 0) {
        // STRING: LEGOLAND 0x004bc6a4
        LogPrintf("End Measured Block 7");
        goto fail;
    }
    DrawWatchSprite();
    if (BeginMeasuredBlock() == 0) {
        // STRING: LEGOLAND 0x004bc68c
        LogPrintf("Begin Measured Block 8");
        goto fail;
    }
    SaveMechanicOrders();
    if (EndMeasuredBlock() == 0) {
        // STRING: LEGOLAND 0x004bc674
        LogPrintf("End Measured Block 8");
        goto fail;
    }
    if (BeginMeasuredBlock() == 0) {
        // STRING: LEGOLAND 0x004bc65c
        LogPrintf("Begin Measured Block 9");
        goto fail;
    }
    if (SaveGameWrite(&CastlePlacedFlag, 4) == 0) {
        // STRING: LEGOLAND 0x004bc634
        LogPrintf("Castle Placed Flag %s",
            // STRING: LEGOLAND 0x004bc654
            CastlePlacedFlag != 0 ? "TRUE" :
                                  // STRING: LEGOLAND 0x004bc64c
                "FALSE");
        goto fail;
    }
    if (SaveGameWrite(&BuildObjCount, 4) == 0) {
        // STRING: LEGOLAND 0x004bc614
        LogPrintf("Num Build Objs (%d) Save Failed", BuildObjCount);
        goto fail;
    }
    if (SaveGameWrite(BuildObjArray, sizeof(BuildObjArray)) == 0) {
        // STRING: LEGOLAND 0x004bc5f0
        LogPrintf("BuildObjList (size %dS) Save Failed", BuildObjCount);
        goto fail;
    }
    for (i = 0; i < SavedElementCount; i++) {
        struct Ride *ride;
        struct ObjInstance *inst;
        struct RideNode *rnode;
        int count;
        DrawWatchSprite();
        if ((SavedElementTable[i]->flags & 0x10) != 0) {
            ride = SavedElementTable[i]->ride;
            SaveGameWrite(ride->element->name, 8);
            if (ride->type != 2 && ride->type != 0) {
                if (SaveGameWrite(ride->counters, lpConfig->max_blokes) == 0) {
                    // STRING: LEGOLAND 0x004bc5c4
                    {
                        LogPrintf("BeenOn Flags for %s Save Failed", ride->element->name);
                        goto fail;
                    }
                }
            }
            if (SaveGameWrite(&ride->field_8, 4) == 0) {
                // STRING: LEGOLAND 0x004bc5b0
                {
                    LogPrintf("Count for Object %s", ride->element->name);
                    goto fail;
                }
            }
            count = 0;
            for (inst = ride->instances; inst != 0; inst = inst->next) {
                count++;
            }
            if (SaveGameWrite(&count, 4) == 0) {
                // STRING: LEGOLAND 0x004bc598
                {
                    LogPrintf("Instance Count for %s", ride->element->name);
                    goto fail;
                }
            }
            for (inst = ride->instances; inst != 0; inst = inst->next) {
                if (SaveGameWrite(&inst->flags, 4) == 0) {
                    // STRING: LEGOLAND 0x004bc57c
                    {
                        LogPrintf("Flags for instance of %s", ride->element->name);
                        goto fail;
                    }
                }
                if (SaveGameWrite(&inst->uid, 4) == 0) {
                    // STRING: LEGOLAND 0x004bc560
                    {
                        LogPrintf("Objuid for instance of %s", ride->element->name);
                        goto fail;
                    }
                }
                if (SaveGameWrite(&inst->field_10, 4) == 0) {
                    // STRING: LEGOLAND 0x004bc540
                    {
                        LogPrintf("TickCount for instance of %s", ride->element->name);
                        goto fail;
                    }
                }
            }
            count = 0;
            for (rnode = ride->riders; rnode != 0; rnode = rnode->next) {
                count++;
            }
            if (SaveGameWrite(&count, 4) == 0) {
                // STRING: LEGOLAND 0x004bc51c
                {
                    LogPrintf("Num Blokes On Ride for object %s", ride->element->name);
                    goto fail;
                }
            }
            for (rnode = ride->riders; rnode != 0; rnode = rnode->next) {
                int num;
                num = GetBlokeNum(rnode->rider);
                if (SaveGameWrite(&num, 4) == 0) {
                    // STRING: LEGOLAND 0x004bc500
                    {
                        LogPrintf("Bloke Num for bloke on %s", ride->element->name);
                        goto fail;
                    }
                }
                if (SaveGameWrite(&rnode->tile, 2) == 0) {
                    // STRING: LEGOLAND 0x004bc4e8
                    {
                        LogPrintf("Ride ID for bloke on %s", ride->element->name);
                        goto fail;
                    }
                }
            }
            if (ride->save_hook != 0) {
                if (ride->save_hook(ride->element) == 0) {
                    // STRING: LEGOLAND 0x004bc4c8
                    {
                        LogPrintf("Ride specific save data for %s", ride->element->name);
                        goto fail;
                    }
                }
            }
        }
    }
    if (SaveGameWrite(ObjInstanceTable, sizeof(ObjInstanceTable)) == 0) {
        // STRING: LEGOLAND 0x004bc5e4
        LogPrintf("RideTotal");
        goto fail;
    }
    if (EndMeasuredBlock() == 0) {
        // STRING: LEGOLAND 0x004bc4b0
        LogPrintf("End Measured VBlock 9");
        goto fail;
    }
    DrawWatchSprite();
    if (BeginMeasuredBlock() == 0) {
        // STRING: LEGOLAND 0x004bc498
        LogPrintf("Begin Measured Block 10");
        goto fail;
    }
    if (FUN_00482860() == 0) {
        // STRING: LEGOLAND 0x004bc48c
        LogPrintf("Path Rects");
        goto fail;
    }
    if (EndMeasuredBlock() == 0) {
        // STRING: LEGOLAND 0x004bc474
        LogPrintf("End Measured Block 10");
        goto fail;
    }
    if (BeginMeasuredBlock() == 0) {
        // STRING: LEGOLAND 0x004bc458
        LogPrintf("Begin Measured VBlock 11");
        goto fail;
    }
    FUN_00450a80();
    if (EndMeasuredBlock() == 0) {
        // STRING: LEGOLAND 0x004bc440
        LogPrintf("End Measured Block 11");
        goto fail;
    }
    DrawWatchSprite();
    if (BeginMeasuredBlock() == 0) {
        // STRING: LEGOLAND 0x004bc428
        LogPrintf("Begin Measured Block 12");
        goto fail;
    }
    {
        struct OverlayNode *ov;
        int n;
        n = strlen(((struct Element *)OverlayILFHandle)->name);
        SaveGameWrite(&n, 4);
        SaveGameWrite(((struct Element *)OverlayILFHandle)->name, n);
        if (BridgesHandle != 0) {
            n = strlen(((struct Element *)BridgesHandle)->name);
        } else {
            n = 0;
        }
        SaveGameWrite(&n, 4);
        if (n != 0) {
            SaveGameWrite(((struct Element *)BridgesHandle)->name, n);
        }
        n = 0;
        for (ov = OverlayList; ov != 0; ov = ov->next) {
            n++;
        }
        SaveGameWrite(&n, 4);
        for (ov = OverlayList; ov != 0; ov = ov->next) {
            SaveGameWrite(ov, 0x14);
        }
    }
    if (EndMeasuredBlock() == 0) {
        // STRING: LEGOLAND 0x004bc410
        LogPrintf("End Measured Block 12");
        goto fail;
    }
    DrawWatchSprite();
    if (EndMeasuredBlock() == 0) {
        // STRING: LEGOLAND 0x004bc3f8
        LogPrintf("End Measured Block 13");
        goto fail;
    }
    _close(SaveFileHandle);
    return 1;
fail:
    _close(SaveFileHandle);
    return 0;
}

// FUNCTION: LEGOLAND 0x0047e980
LEGO_EXPORT int LoadGame(char *path) {
    int i;
    int x;
    int y;
    struct Ride *ride;
    struct Element *e;
    struct MapElement *tile;
    struct Bloke *bloke;
    struct Anim3D *anim;
    struct ObjInstance *inst;
    struct ObjInstance *inst_prev;
    struct RideNode *rnode;
    struct RideNode *rnode_prev;
    struct Ride *entrance;
    struct MapElement *first;
    int n;
    unsigned int len;
    int m;
    char header[0x20];
    char name[0x200];

    SaveFileHandle = _open(path, _O_BINARY);
    if (SaveFileHandle == -1) {
        return 0;
    }
    MeasuredBlockDepth = 0;
    LoadInProgress = 1;
    for (;;) {
        if (SaveGameRead(header, 0x20) == 0) {
            break;
        }
        header[5] = 0;
        if (atoi(header) != 2) {
            return 0;
        }
        if (SkipSaveGameDword() == 0) {
            break;
        }
        if (SkipSaveGameDword() == 0) {
            break;
        }
        if (SaveGameRead(&SavedElementCount, 4) == 0) {
            break;
        }
        if (SavedElementTable != 0) {
            free(SavedElementTable);
            SavedElementTable = 0;
        }
        SavedElementTable = malloc(SavedElementCount * 4);
        for (i = 0; i < SavedElementCount; i++) {
            DrawWatchSprite();
            if (SaveGameRead(&len, 4) == 0) {
                LOAD_FAIL();
            }
            if (SaveGameRead(name, len) == 0) {
                LOAD_FAIL();
            }
            name[len] = 0;
            if (LLIDB_FindElement(name, (unsigned int *)&n, 0) != 0) {
                LOAD_FAIL();
            }
            SavedElementTable[i] = (struct Element *)n;
            if (SaveGameRead(&m, 4) == 0) {
                LOAD_FAIL();
            }
            LLIDB_LoadData((void *)n);
            ((struct Element *)n)->flags &= 0xfffcfff1;
            ((struct Element *)n)->flags |= m;
        }
        {
            unsigned short *tab[256];
            if (SaveGameRead(&DAT_007fdb84, 4) == 0) {
                break;
            }
            for (i = 0; i < DAT_007fdb84; i++) {
                DrawWatchSprite();
                if (SaveGameRead(&len, 4) == 0) {
                    LOAD_FAIL();
                }
                if (SaveGameRead(name, len) == 0) {
                    LOAD_FAIL();
                }
                name[len] = 0;
                if (LLIDB_FindElement(name, (unsigned int *)&n, 0) != 0) {
                    LOAD_FAIL();
                }
                DAT_007fd660[i] = n;
                LLIDB_LoadData((void *)n);
                tab[i] = ((struct Element *)n)->data;
            }
            if (SkipSaveGameDword() == 0) {
                break;
            }
            if (SaveGameRead(lpConfig, 0x44) == 0) {
                break;
            }
            ClearGameMap();
            for (y = 0; y < lpConfig->height; y++) {
                DrawWatchSprite();
                for (x = 0; x < lpConfig->width; x++) {
                    tile = &GameMap[y][x];
                    if (SaveGameRead(tile, 0x14) == 0) {
                        LOAD_FAIL();
                    }
                    if ((tile->flags & 0x8a8) != 0) {
                        tile->field_0 = SavedElementTable[(int)tile->field_0];
                    } else {
                        tile->field_0 = 0;
                    }
                    if (tile->field_8 != 0) {
                        tile->field_8 = *tab[(tile->field_8 >> 8) - 1] + (tile->field_8 & 0xff);
                    }
                    if (tile->field_a != 0) {
                        tile->field_a = *tab[(tile->field_a >> 8) - 1] + (tile->field_a & 0xff);
                    }
                }
            }
        }
        if (SkipSaveGameDword() == 0) {
            break;
        }
        if (SaveGameRead(&MapStats, 0x3f0) == 0) {
            break;
        }
        if (SaveGameRead(&ScrollX, 4) == 0) {
            break;
        }
        if (SaveGameRead(&ScrollY, 4) == 0) {
            break;
        }
        if (SaveGameRead(&EditMode, 0xc) == 0) {
            break;
        }
        EditMode.unk4 = 3;
        EditMode.unk8 = 0;
        FUN_004741c0();
        FUN_00474880();
        DrawWatchSprite();
        if (LoadScripts() == 0) {
            break;
        }
        if (LoadReport() == 0) {
            break;
        }
        if (LoadCurrency() == 0) {
            break;
        }
        if (SaveGameRead(ButtonFlashStates, 0x24) == 0) {
            break;
        }
        DrawWatchSprite();
        MapDataLoaded = 1;
        if (SkipSaveGameDword() == 0) {
            break;
        }
        while (FirstBloke != 0) {
            DestroyBloke(FirstBloke);
        }
        n = 0;
        DrawWatchSprite();
        if (SaveGameRead(&n, 4) == 0) {
            break;
        }
        while (n-- != 0) {
            if (SaveGameRead(&m, 4) == 0) {
                LOAD_FAIL();
            }
            bloke = &BlokePool[m];
            bloke->next = FirstBloke;
            FirstBloke = bloke;
            if (SaveGameRead(&BlokeSaveBuffer, sizeof(BlokeSaveBuffer)) == 0) {
                LOAD_FAIL();
            }
            if (BlokeSaveBuffer.block_34[8] != 0) {
                BlokeSaveBuffer.block_34[8] = (unsigned int)malloc(0x48);
                if (SaveGameRead((void *)BlokeSaveBuffer.block_34[8], 0x48) == 0) {
                    LOAD_FAIL();
                }
            }
            bloke->action = BlokeSaveBuffer.action;
            bloke->low_level_action = BlokeSaveBuffer.low_level_action;
            bloke->pending_action = BlokeSaveBuffer.pending_action;
            if (BlokeSaveBuffer.target != -1) {
                bloke->target = SavedElementTable[BlokeSaveBuffer.target];
            } else {
                bloke->target = 0;
            }
            if (BlokeSaveBuffer.last_ride != -1) {
                bloke->last_ride = SavedElementTable[BlokeSaveBuffer.last_ride];
            } else {
                bloke->last_ride = 0;
            }
            bloke->field_1c = BlokeSaveBuffer.field_1c;
            bloke->field_20 = BlokeSaveBuffer.field_20;
            bloke->dest.x = BlokeSaveBuffer.dest.x;
            bloke->dest.y = BlokeSaveBuffer.dest.y;
            bloke->goal.x = BlokeSaveBuffer.goal.x;
            bloke->goal.y = BlokeSaveBuffer.goal.y;
            memcpy(&bloke->field_34, BlokeSaveBuffer.block_34, sizeof(BlokeSaveBuffer.block_34));
            bloke->field_5c = BlokeSaveBuffer.field_5c;
            bloke->param_action = BlokeSaveBuffer.param_action;
            bloke->flags = BlokeSaveBuffer.flags;
            bloke->field_64 = BlokeSaveBuffer.field_64;
            bloke->field_78 = BlokeSaveBuffer.field_78;
            bloke->mood = BlokeSaveBuffer.mood;
            bloke->field_7c = BlokeSaveBuffer.field_7c;
            bloke->field_7e = BlokeSaveBuffer.field_7e;
            bloke->speed = BlokeSaveBuffer.speed;
            bloke->field_80 = BlokeSaveBuffer.field_80;
            bloke->field_81 = BlokeSaveBuffer.field_81;
            bloke->field_82 = BlokeSaveBuffer.field_82;
            if (BlokeSaveBuffer.favourite[0] < SavedElementCount) {
                bloke->favourite_attraction_0 = SavedElementTable[BlokeSaveBuffer.favourite[0]];
            } else {
                bloke->favourite_attraction_0 = 0;
            }
            if (BlokeSaveBuffer.favourite[1] < SavedElementCount) {
                bloke->favourite_attraction_1 = SavedElementTable[BlokeSaveBuffer.favourite[1]];
            } else {
                bloke->favourite_attraction_1 = 0;
            }
            if (BlokeSaveBuffer.favourite[2] < SavedElementCount) {
                bloke->favourite_attraction_2 = SavedElementTable[BlokeSaveBuffer.favourite[2]];
            } else {
                bloke->favourite_attraction_2 = 0;
            }
            if (BlokeSaveBuffer.favourite[3] < SavedElementCount) {
                bloke->favourite_food = SavedElementTable[BlokeSaveBuffer.favourite[3]];
            } else {
                bloke->favourite_food = 0;
            }
            bloke->pos.x = BlokeSaveBuffer.pos.x;
            bloke->pos.y = BlokeSaveBuffer.pos.y;
            bloke->height = BlokeSaveBuffer.height;
            bloke->dir = BlokeSaveBuffer.dir;
            bloke->field_73 = BlokeSaveBuffer.field_73;
            bloke->frame = BlokeSaveBuffer.frame;
            bloke->field_75 = BlokeSaveBuffer.field_75;
            bloke->nav = BlokeSaveBuffer.nav;
            bloke->person = malloc(sizeof(Person));
            AddPersonToList(bloke->person);
            bloke->person->bloke = bloke;
            bloke->person->character = BlokeSaveBuffer.person_8;
            bloke->person->scale = BlokeSaveBuffer.scale;
            bloke->person->screen = BlokeSaveBuffer.screen;
            bloke->person->offset = BlokeSaveBuffer.offset;
            bloke->person->field_34 = BlokeSaveBuffer.field_34;
            bloke->person->field_38 = BlokeSaveBuffer.field_38;
            bloke->person->depth = BlokeSaveBuffer.depth;
            bloke->person->rotation = BlokeSaveBuffer.rotation;
            bloke->person->frame = BlokeSaveBuffer.person_4c;
            bloke->person->anim = BlokeSaveBuffer.anim;
            bloke->person->sort_id = BlokeSaveBuffer.sort_id;
            bloke->person->field_38 = BlokeSaveBuffer.field_38;
            memcpy(bloke->person->m, BlokeSaveBuffer.m, sizeof(BlokeSaveBuffer.m));
            bloke->person->field_7c = BlokeSaveBuffer.field_7c_p;
            bloke->person->field_80 = BlokeSaveBuffer.field_80_p;
            bloke->person->field_8c = BlokeSaveBuffer.field_8c_p;
            bloke->person->field_90 = BlokeSaveBuffer.field_90_p;
            bloke->person->random = BlokeSaveBuffer.random;
            bloke->prev_param = BlokeSaveBuffer.prev_param;
            bloke->prev_action = BlokeSaveBuffer.prev_action;
            bloke->person->field_2c = 0;
            bloke->person->field_30 = BlokeSaveBuffer.field_30;
            anim = GetBlokeAnim3DFromPerson(bloke->person);
            bloke->person->field_50 = FUN_00442580(bloke->person, VisitorLocData, (unsigned int)anim->field_8, anim->elems->shared->count, bloke->person->random);
        }
        if (SkipSaveGameDword() == 0) {
            break;
        }
        LoadGardeners();
        if (SkipSaveGameDword() == 0) {
            break;
        }
        LoadMechanics();
        DrawWatchSprite();
        if (SkipSaveGameDword() == 0) {
            break;
        }
        LoadGardenerOrders();
        if (SkipSaveGameDword() == 0) {
            break;
        }
        FUN_0049ce00();
        DrawWatchSprite();
        if (SkipSaveGameDword() == 0) {
            break;
        }
        if (SaveGameRead(&CastlePlacedFlag, 4) == 0) {
            break;
        }
        if (SaveGameRead(&BuildObjCount, 4) == 0) {
            break;
        }
        if (SaveGameRead(BuildObjArray, sizeof(BuildObjArray)) == 0) {
            break;
        }
        for (i = 0; i < SavedElementCount; i++) {
            DrawWatchSprite();
            e = SavedElementTable[i];
            if ((e->flags & 0x10) != 0) {
                // STRING: LEGOLAND 0x004bcb94
                char label[9] = "xxxxxxxx";
                ride = SavedElementTable[i]->ride;
                e->flags = e->flags | 4;
                if (SaveGameRead(label, 8) == 0) {
                    LOAD_FAIL();
                }
                // STRING: LEGOLAND 0x004bcb90
                DBPrintf("%s\n", label);
                if (ride->type != 2) {
                    if (ride->type != 0) {
                        ride->counters = malloc(lpConfig->max_blokes);
                        if (SaveGameRead(ride->counters, lpConfig->max_blokes) == 0) {
                            LOAD_FAIL();
                        }
                    }
                }
                if (SaveGameRead(&ride->field_8, 4) == 0) {
                    LOAD_FAIL();
                }
                inst_prev = 0;
                n = 0;
                if (SaveGameRead(&n, 4) == 0) {
                    LOAD_FAIL();
                }
                while (n-- != 0) {
                    inst = malloc(sizeof(struct ObjInstance));
                    inst->next = 0;
                    inst->field_8 = (unsigned int)ride;
                    if (inst_prev == 0) {
                        ride->instances = inst;
                        inst->field_4 = (unsigned int)inst_prev;
                    } else {
                        inst_prev->next = inst;
                        inst->field_4 = (unsigned int)inst_prev;
                    }
                    inst_prev = inst;
                    if (SaveGameRead(&inst->flags, 4) == 0) {
                        LOAD_FAIL();
                    }
                    if (SaveGameRead(&inst->uid, 4) == 0) {
                        LOAD_FAIL();
                    }
                    if (SaveGameRead(&inst->field_10, 4) == 0) {
                        LOAD_FAIL();
                    }
                }
                rnode_prev = 0;
                n = 0;
                if (SaveGameRead(&n, 4) == 0) {
                    LOAD_FAIL();
                }
                while (n-- != 0) {
                    rnode = malloc(sizeof(struct RideNode));
                    rnode->next = 0;
                    if (rnode_prev == 0) {
                        ride->riders = rnode;
                        rnode->prev = rnode_prev;
                    } else {
                        rnode_prev->next = rnode;
                        rnode->prev = rnode_prev;
                    }
                    rnode_prev = rnode;
                    if (SaveGameRead(&m, 4) == 0) {
                        LOAD_FAIL();
                    }
                    if (SaveGameRead(&rnode->tile, 2) == 0) {
                        LOAD_FAIL();
                    }
                    rnode->rider = GetBlokePtr(m);
                    rnode->person = rnode->rider->person;
                }
                if (ride->load_hook != 0) {
                    if (ride->load_hook(ride->element) == 0) {
                        LOAD_FAIL();
                    }
                }
            }
        }
        if (SaveGameRead(ObjInstanceTable, sizeof(ObjInstanceTable)) == 0) {
            break;
        }
        DrawWatchSprite();
        if (SkipSaveGameDword() == 0) {
            break;
        }
        if (FUN_00482920() == 0) {
            break;
        }
        if (SkipSaveGameDword() == 0) {
            break;
        }
        LoadBuildObjArray();
        DrawWatchSprite();
        if (SkipSaveGameDword() == 0) {
            break;
        }
        {
            char name2[0x200];
            struct OverlayParam ov;
            SaveGameRead(&n, 4);
            SaveGameRead(name2, n);
            name2[n] = 0;
            LLIDB_FindElement(name2, (unsigned int *)&OverlayILFHandle, 0);
            OverlayILF = (unsigned int)LLIDB_LoadData(OverlayILFHandle);
            SaveGameRead(&n, 4);
            if (n != 0) {
                SaveGameRead(name2, n);
                name2[n] = 0;
                LLIDB_FindElement(name2, (unsigned int *)&BridgesHandle, 0);
                BridgesData = LLIDB_LoadData(BridgesHandle);
                SetBridgeParamsByName(name2);
            } else {
                BridgesHandle = 0;
                BridgesData = 0;
            }
            SaveGameRead(&n, 4);
            for (i = 0; i < n; i++) {
                SaveGameRead(&ov, 0x14);
                AddOvSav(&ov);
            }
        }
        _close(SaveFileHandle);
        DrawWatchSprite();
        LoadInProgress = 0;
        EditMode.unk0 = 0;
        GamePad |= 0x20;
        CalculateMapRenderOrder();
        // STRING: LEGOLAND 0x004b83d0
        Entrance1Elem = ElemID("ENTRANCE 1");
        entrance = Entrance1Elem->ride;
        first = GetFirstObjectMatching(Entrance1Elem);
        if (first != 0) {
            DAT_004b8320.x = (first->field_4 + entrance->footprint.v[0]) * 0x100 + -0x100;
            DAT_004b8320.y = (((entrance->footprint.v[3] - entrance->footprint.v[1]) << 7) & ~0xff) + ((first->field_5 + entrance->footprint.v[1]) << 8);
        }
        FUN_00475f10();
        SetMapLoaded(1);
        DAT_00810140 = 1;
        DAT_00667c48 = 1;
        GamePad &= ~0x1000;
        FUN_0046b760();
        return 1;
    }
    return LoadAbort();
}

// FUNCTION: LEGOLAND 0x0047f760
LEGO_EXPORT void UnloadSaveGameMap(void) {
    int i;

    if (SavedElementTable != 0) {
        for (i = 0; i < SavedElementCount; i++) {
            LLIDB_UnLoadData(SavedElementTable[i]);
        }
        free(SavedElementTable);
        SavedElementTable = 0;
    }

    for (i = 0; i < DAT_007fdb84; i++) {
        LLIDB_UnLoadData(DAT_007fd660[i]);
    }

    ClearGameMap();
    LLIDB_UnLoadData((unsigned int)OverlayILFHandle);
    if (BridgesHandle != 0) {
        LLIDB_UnLoadData((unsigned int)BridgesHandle);
    }
    ClearOverlays();
    FUN_004828f0();
    MapDataLoaded = 0;
}

// FUNCTION: LEGOLAND 0x0047f810
void MarkGameTimer(void) {
    GameTimerMark = GetGameTimer();
}

// FUNCTION: LEGOLAND 0x0047f820
int GetGameTimerSinceMark(void) {
    return GetGameTimer() - GameTimerMark;
}

// FUNCTION: LEGOLAND 0x0047f830
unsigned int OpenLogFile(const char *path) {
    return 1;
}

// FUNCTION: LEGOLAND 0x0047f840
int CloseLogFile(void) {
    return 1;
}

// FUNCTION: LEGOLAND 0x0047f850
void FUN_0047f850(void) {
    return;
}
