#include "popupinfo.h"
#include <windows.h>
#include <ddraw.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "globals.h"
#include "icon.h"
#include "legoland.h"
#include "math.h"
#include "text.h"
#include "worker_mouse.h"

#pragma intrinsic(strlen, strcpy, strcat, memcpy)

struct Sprite;
struct Element;

struct InfoObjData {
    /* 0x00 */ unsigned char pad_0[0x4];
    /* 0x04 */ struct Element *field_4;
    /* 0x08 */ unsigned int field_8;
    /* 0x0c */ unsigned int field_c;
    /* 0x10 */ unsigned char pad_10[0x18 - 0x10];
    /* 0x18 */ int field_18;
    /* 0x1c */ struct InfoObjInner *field_1c;
};

struct InfoObjInner {
    /* 0x00 */ unsigned char pad_0[0x60];
    /* 0x60 */ unsigned char field_60;
};

#include "bloke.h"
#include "bloke_ai.h"
#include "build.h"
#include "controller.h"
#include "debug_alloc.h"
#include "draw.h"
#include "gamemap.h"
#include "gfx.h"
#include "help.h"
#include "image_sprite.h"
#include "llidb.h"
#include "map_object.h"
#include "obj_instance.h"
#include "objclass.h"
#include "print_sprite.h"
#include "render.h"
#include "sound_music.h"
#include "stream.h"
#include "string.h"
#include "tilemap.h"
#include "timer.h"
#include "worker.h"

struct NewObjInfo {
    /* 0x00 */ unsigned char pad_0[0x26];
    /* 0x26 */ short field_26;
    /* 0x28 */ unsigned char pad_28[0x78 - 0x28];
    /* 0x78 */ char *field_78;
    /* 0x7c */ unsigned char pad_7c[0x80 - 0x7c];
    /* 0x80 */ char *field_80;
    /* 0x84 */ unsigned char pad_84[0xc4 - 0x84];
    /* 0xc4 */ char **name;
};

// FUNCTION: LEGOLAND 0x00470bb0
LEGO_EXPORT void InitPopUpInfo(void) {
    unsigned int *puVar3;
    int iVar2;

    puVar3 = (unsigned int *)&DAT_007fdec0;
    for (iVar2 = 0x40; iVar2 != 0; iVar2 = iVar2 + -1) {
        *puVar3 = 0;
        puVar3 = puVar3 + 1;
    }
    // STRING: LEGOLAND 0x004b89ac
    if (LLIDB_FindElement("POTTING SHED", &PottingShedHandle, 0) != 0) {
        exit(1);
    }
    // STRING: LEGOLAND 0x004b899c
    if (LLIDB_FindElement("MECHANICS HUT", &MechanicsHutHandle, 0) != 0) {
        exit(1);
    }
    if (LLIDB_FindElement("PATH CONTROL", &PathControlHandle, 0) != 0) {
        exit(1);
    }
    // STRING: LEGOLAND 0x004b83d0
    if (LLIDB_FindElement("ENTRANCE 1", &Entrance1Handle, 0) != 0) {
        exit(1);
    }
    if (PopUpSpritesLoaded == NULL) {
        PopUpSpritesLoaded = (struct Sprite *)1;
        // STRING: LEGOLAND 0x004baca4
        PuBgMainSprite = LoadSprite("PU_BGMain.lls", 4);
        // STRING: LEGOLAND 0x004bac90
        PuBgCentreTopSprite = LoadSprite("PU_BGCentreTop.lls", 4);
        // STRING: LEGOLAND 0x004bac7c
        PuBgRightTopSprite = LoadSprite("PU_BGRightTop.lls", 4);
        // STRING: LEGOLAND 0x004bac68
        PuBgLeftMidSprite = LoadSprite("PU_BGLeftMid.lls", 4);
        // STRING: LEGOLAND 0x004bac54
        PuBgCentreMidSprite = LoadSprite("PU_BGCentreMid.lls", 4);
        // STRING: LEGOLAND 0x004bac40
        PuBgRightMidSprite = LoadSprite("PU_BGRightMid.lls", 4);
        // STRING: LEGOLAND 0x004bac2c
        PuBgLeftBtmSprite = LoadSprite("PU_BGLeftBtm.lls", 4);
        // STRING: LEGOLAND 0x004bac18
        PuBgCentreBtmSprite = LoadSprite("Pu_BGCentreBtm.lls", 4);
        // STRING: LEGOLAND 0x004bac04
        PuBgRightBtmSprite = LoadSprite("PU_BGRightBtm.lls", 4);
        // STRING: LEGOLAND 0x004babf4
        NewPopMockSprite = LoadSprite("NewPopMock.lls", 4);
        // STRING: LEGOLAND 0x004babe0
        ObjectRepairOkSprite = LoadSprite("ObjectRepairOK.lls", 4);
        // STRING: LEGOLAND 0x004babcc
        ObjectNoRepair1Sprite = LoadSprite("ObjectNoRepair1.lls", 4);
        // STRING: LEGOLAND 0x004babb4
        PuDeleteObjectOnSprite = LoadSprite("PU_DeleteObjectON.lls", 4);
        // STRING: LEGOLAND 0x004baba0
        PuDeleteObjectSprite = LoadSprite("PU_DeleteObject.lls", 4);
        // STRING: LEGOLAND 0x004bab8c
        PuClosePopUpOnSprite = LoadSprite("PU_ClosePopUpON.lls", 4);
        // STRING: LEGOLAND 0x004bab78
        PuClosePopUpSprite = LoadSprite("PU_ClosePopUp.lls", 4);
        // STRING: LEGOLAND 0x004bab60
        PuAddGardenerOnSprite = LoadSprite("PU_AddGardenerON.lls", 4);
        // STRING: LEGOLAND 0x004bab4c
        PuAddGardenerSprite = LoadSprite("PU_AddGardener.lls", 4);
        // STRING: LEGOLAND 0x004bab34
        PuAddMechanicsOnSprite = LoadSprite("PU_AddMechanicsON.lls", 4);
        // STRING: LEGOLAND 0x004bab20
        PuAddMechanicsSprite = LoadSprite("PU_AddMechanics.lls", 4);
        // STRING: LEGOLAND 0x004bab10
        NextIconSprite = LoadSprite("NextIcon.lls", 4);
        // STRING: LEGOLAND 0x004bab00
        NextIconOnSprite = LoadSprite("NextIconOn.lls", 4);
        // STRING: LEGOLAND 0x004baaf0
        PrevIconSprite = LoadSprite("PrevIcon.lls", 4);
        // STRING: LEGOLAND 0x004baae0
        PrevIconOnSprite = LoadSprite("PrevIconOn.lls", 4);
        // STRING: LEGOLAND 0x004baad4
        ISadSprite = LoadSprite("i_sad.lls", 0);
        // STRING: LEGOLAND 0x004baac8
        INormSprite = LoadSprite("i_norm.lls", 0);
        // STRING: LEGOLAND 0x004baabc
        IHappySprite = LoadSprite("i_happy.lls", 0);
        // STRING: LEGOLAND 0x004baaac
        IHungrySprite = LoadSprite("i_hungry.lls", 0);
        // STRING: LEGOLAND 0x004baa9c
        IPeckishSprite = LoadSprite("i_peckish.lls", 0);
        // STRING: LEGOLAND 0x004baa90
        IFullSprite = LoadSprite("i_full.lls", 0);
    }
    AddGardenerIcon = InsertIcon(0, 0, 0x2c3, PuAddGardenerSprite);
    AddGardenerIcon->string_id = 0x6e;
    AddGardenerIcon->string = GetString(0x6e);
    AddGardenerIcon->flags |= 0x2000;
    AddGardenerIcon->flags |= 0x4002;
    AddGardenerIcon->flags |= 0x400;
    AddGardenerIcon->event_handler = (void *)FUN_004733f0;
    AddMechanicsIcon = InsertIcon(0, 0, 0x2c3, PuAddMechanicsSprite);
    AddMechanicsIcon->string_id = 0x6f;
    AddMechanicsIcon->string = GetString(0x6f);
    AddMechanicsIcon->flags |= 0x2000;
    AddMechanicsIcon->flags |= 0x4002;
    AddMechanicsIcon->flags |= 0x400;
    AddMechanicsIcon->event_handler = (void *)FUN_00473460;
    DeleteObjectIcon = InsertIcon(0, 0, 0x2c3, PuDeleteObjectSprite);
    DeleteObjectIcon->string_id = 0x70;
    DeleteObjectIcon->string = GetString(0x70);
    DeleteObjectIcon->flags |= 0x2000;
    DeleteObjectIcon->flags |= 0x4002;
    DeleteObjectIcon->flags |= 0x400;
    DeleteObjectIcon->event_handler = (void *)FUN_004731a0;
    ClosePopUpIcon = InsertIcon(0, 0, 0x2c3, PuClosePopUpSprite);
    ClosePopUpIcon->string_id = 0x73;
    ClosePopUpIcon->string = GetString(0x73);
    ClosePopUpIcon->flags |= 0x2000;
    ClosePopUpIcon->flags |= 0x4002;
    ClosePopUpIcon->flags |= 0x400;
    ClosePopUpIcon->event_handler = (void *)FUN_004730f0;
    NextPopUpIcon = InsertIcon(0, 0, 0x2c3, NextIconSprite);
    NextPopUpIcon->string_id = 0x88e;
    NextPopUpIcon->string = GetString(0x88e);
    NextPopUpIcon->flags |= 0x2000;
    NextPopUpIcon->flags |= 0x4002;
    NextPopUpIcon->flags |= 0x400;
    NextPopUpIcon->event_handler = (void *)FUN_00473360;
    PrevPopUpIcon = InsertIcon(0, 0, 0x2c3, PrevIconSprite);
    PrevPopUpIcon->string_id = 0x88f;
    PrevPopUpIcon->string = GetString(0x88f);
    PrevPopUpIcon->flags |= 0x2000;
    PrevPopUpIcon->flags |= 0x4002;
    PrevPopUpIcon->flags |= 0x400;
    PrevPopUpIcon->event_handler = (void *)FUN_004733b0;
    // STRING: LEGOLAND 0x004baa7c
    PuCornerMaskIcon = LoadSpriteIcon("PU_CornerMask.lls", 4, 0, 0, 0x2c3);
    PuCornerMaskIcon->flags |= 0x400;
    DAT_007fdfcc = InsertIcon(0, 0, 0x2c3, PuDeleteObjectSprite);
    DAT_007fdfcc->string_id = 0xa1;
    DAT_007fdfcc->string = GetString(0xa1);
    DAT_007fdfcc->flags |= 0x2000;
    DAT_007fdfcc->flags |= 0x4002;
    DAT_007fdfcc->flags |= 0x400;
    DAT_007fdfcc->event_handler = (void *)FUN_004734d0;
    FUN_00470950(FUN_004731e0, FUN_00473310);
}

// FUNCTION: LEGOLAND 0x00471170
int UnloadPopUpSprites(void) {
    if (PopUpSpritesLoaded != NULL) {
        PopUpSpritesLoaded = NULL;

        if (PuBgMainSprite != NULL) {
            KillSprite(PuBgMainSprite);
            PuBgMainSprite = NULL;
        }
        if (PuBgCentreTopSprite != NULL) {
            KillSprite(PuBgCentreTopSprite);
            PuBgCentreTopSprite = NULL;
        }
        if (PuBgRightTopSprite != NULL) {
            KillSprite(PuBgRightTopSprite);
            PuBgRightTopSprite = NULL;
        }
        if (PuBgCentreMidSprite != NULL) {
            KillSprite(PuBgCentreMidSprite);
            PuBgCentreMidSprite = NULL;
        }
        if (PuBgLeftMidSprite != NULL) {
            KillSprite(PuBgLeftMidSprite);
            PuBgLeftMidSprite = NULL;
        }
        if (PuBgRightMidSprite != NULL) {
            KillSprite(PuBgRightMidSprite);
            PuBgRightMidSprite = NULL;
        }
        if (PuBgLeftBtmSprite != NULL) {
            KillSprite(PuBgLeftBtmSprite);
            PuBgLeftBtmSprite = NULL;
        }
        if (PuBgCentreBtmSprite != NULL) {
            KillSprite(PuBgCentreBtmSprite);
            PuBgCentreBtmSprite = NULL;
        }
        if (PuBgRightBtmSprite != NULL) {
            KillSprite(PuBgRightBtmSprite);
            PuBgRightBtmSprite = NULL;
        }
        if (ObjectRepairOkSprite != NULL) {
            KillSprite(ObjectRepairOkSprite);
            ObjectRepairOkSprite = NULL;
        }
        if (ObjectNoRepair1Sprite != NULL) {
            KillSprite(ObjectNoRepair1Sprite);
            ObjectNoRepair1Sprite = NULL;
        }
        if (PuDeleteObjectOnSprite != NULL) {
            KillSprite(PuDeleteObjectOnSprite);
            PuDeleteObjectOnSprite = NULL;
        }
        if (PuDeleteObjectSprite != NULL) {
            KillSprite(PuDeleteObjectSprite);
            PuDeleteObjectSprite = NULL;
        }
        if (PuClosePopUpOnSprite != NULL) {
            KillSprite(PuClosePopUpOnSprite);
            PuClosePopUpOnSprite = NULL;
        }
        if (PuClosePopUpSprite != NULL) {
            KillSprite(PuClosePopUpSprite);
            PuClosePopUpSprite = NULL;
        }
        if (NextIconSprite != NULL) {
            KillSprite(NextIconSprite);
            NextIconSprite = NULL;
        }
        if (NextIconOnSprite != NULL) {
            KillSprite(NextIconOnSprite);
            NextIconOnSprite = NULL;
        }
        if (PrevIconSprite != NULL) {
            KillSprite(PrevIconSprite);
            PrevIconSprite = NULL;
        }
        if (PrevIconOnSprite != NULL) {
            KillSprite(PrevIconOnSprite);
            PrevIconOnSprite = NULL;
        }
        if (PuAddGardenerOnSprite != NULL) {
            KillSprite(PuAddGardenerOnSprite);
            PuAddGardenerOnSprite = NULL;
        }
        if (PuAddGardenerSprite != NULL) {
            KillSprite(PuAddGardenerSprite);
            PuAddGardenerSprite = NULL;
        }
        if (PuAddMechanicsOnSprite != NULL) {
            KillSprite(PuAddMechanicsOnSprite);
            PuAddMechanicsOnSprite = NULL;
        }
        if (PuAddMechanicsSprite != NULL) {
            KillSprite(PuAddMechanicsSprite);
            PuAddMechanicsSprite = NULL;
        }
        if (ISadSprite != NULL) {
            KillSprite(ISadSprite);
            ISadSprite = NULL;
        }
        if (INormSprite != NULL) {
            KillSprite(INormSprite);
            INormSprite = NULL;
        }
        if (IHappySprite != NULL) {
            KillSprite(IHappySprite);
            IHappySprite = NULL;
        }
        if (IHungrySprite != NULL) {
            KillSprite(IHungrySprite);
            IHungrySprite = NULL;
        }
        if (IPeckishSprite != NULL) {
            KillSprite(IPeckishSprite);
            IPeckishSprite = NULL;
        }
        if (IFullSprite != NULL) {
            KillSprite(IFullSprite);
            IFullSprite = NULL;
        }
        KillPUOKAndCBSprites();
    }
}

// FUNCTION: LEGOLAND 0x00471450
LEGO_EXPORT int UnLoad_PopUpInfo(void) {
    RemoveIconGroup(0x2c3);
    return UnloadPopUpSprites();
}

// FUNCTION: LEGOLAND 0x00471470
void FUN_00471470(void) {
    DeleteObjectIcon->event_handler = NULL;
    AddGardenerIcon->event_handler = NULL;
    AddMechanicsIcon->event_handler = NULL;
    DAT_007fdfcc->event_handler = NULL;
}

// FUNCTION: LEGOLAND 0x004714a0
void FUN_004714a0(void) {
    DeleteObjectIcon->event_handler = (void *)FUN_004731a0;
    ClosePopUpIcon->event_handler = (void *)FUN_004730f0;
    AddGardenerIcon->event_handler = (void *)FUN_004733f0;
    AddMechanicsIcon->event_handler = (void *)FUN_00473460;
    DAT_007fdfcc->event_handler = (void *)FUN_004734d0;
}

// FUNCTION: LEGOLAND 0x004714e0
void RemoveAllNewObjects(void) {
    while (NewObjects.count != 0) {
        RemoveNewObject(NewObjects.objs[0]);
    }
}

// FUNCTION: LEGOLAND 0x00471510
LEGO_EXPORT void ResetInfoStruct(void) {
    DAT_007fdf7c = 0;
    DAT_007fdf8c = 0;
    DAT_007fdf84 = 0;
    DAT_007fdf9c = 0;
    DAT_007fdfa0 = 0;
    DAT_007fdfa4 = 0;
    DAT_007fdfa8 = 0;
    DAT_007fdf98 = 0;
    DAT_007fdfac = 0;
    DisableInfoPopUPIcons();
    FUN_004714a0();
}

// FUNCTION: LEGOLAND 0x00471550
LEGO_EXPORT void PopInfoSizeMayChange(void) {
    if (DAT_007fdfa0 != 0) {
        DAT_007fdfa8 = 1;
    }
}

// FUNCTION: LEGOLAND 0x00471570
LEGO_EXPORT void StopFollowingBloke(void) {
    if (DAT_007fdf98 == 0) {
        return;
    }
    ResetInfoStruct();
}

// FUNCTION: LEGOLAND 0x00471580
LEGO_EXPORT void DisableInfoPopUPIcons(void) {
    DeleteObjectIcon->flags |= 0x400;
    ClosePopUpIcon->flags |= 0x400;
    PuCornerMaskIcon->flags |= 0x400;
    DAT_007fdea8->flags |= 0x400;
    DAT_007fe000->flags |= 0x400;
    AddGardenerIcon->flags |= 0x400;
    AddMechanicsIcon->flags |= 0x400;
    DAT_007fdfcc->flags |= 0x400;
    NextPopUpIcon->flags |= 0x400;
    PrevPopUpIcon->flags |= 0x400;
}

// FUNCTION: LEGOLAND 0x00471610
void FUN_00471610(void) {
    SetIconSprite((struct IconNode *)DeleteObjectIcon, PuDeleteObjectSprite);
    SetIconSprite((struct IconNode *)PrevPopUpIcon, PrevIconSprite);
    SetIconSprite((struct IconNode *)NextPopUpIcon, NextIconSprite);
    SetIconSprite((struct IconNode *)ClosePopUpIcon, PuClosePopUpSprite);
    SetIconSprite((struct IconNode *)AddGardenerIcon, PuAddGardenerSprite);
    SetIconSprite((struct IconNode *)AddMechanicsIcon, PuAddMechanicsSprite);
    SetIconSprite((struct IconNode *)DAT_007fdfcc, PuDeleteObjectSprite);
}

// FUNCTION: LEGOLAND 0x004716a0
LEGO_EXPORT void InfoPrintCent(int len, char *text, int font, RECT rc, int flag) {
    HRGN region = CreateRectRgn(SPRITE_ClipRect.left, SPRITE_ClipRect.top, SPRITE_ClipRect.right, SPRITE_ClipRect.bottom);
    HDC hdc;
    HGDIOBJ old_region;
    HGDIOBJ old_font;

    ((LPDIRECTDRAWSURFACE)renderEngine)->lpVtbl->GetDC((LPDIRECTDRAWSURFACE)renderEngine, &hdc);
    SetBkMode(hdc, 1);
    SetTextColor(hdc, 0xffffff);
    old_region = SelectObject(hdc, region);
    old_font = (HGDIOBJ)SelectFont(hdc, font);
    if (flag != 0) {
        DrawTextA(hdc, text, strlen(text), &rc, 0x11);
    } else {
        DrawTextA(hdc, text, strlen(text), &rc, 0x10);
    }
    SelectObject(hdc, old_font);
    SelectObject(hdc, old_region);
    ((LPDIRECTDRAWSURFACE)renderEngine)->lpVtbl->ReleaseDC((LPDIRECTDRAWSURFACE)renderEngine, hdc);
    DeleteObject(region);
}

// FUNCTION: LEGOLAND 0x004717a0
int FUN_004717a0(const char *param_1, int param_2, int param_3, int param_4, int param_5, int param_6) {
    HDC hdc;
    int iVar2;
    int iVar4;
    RECT local_10;

    local_10.left = 0;
    local_10.top = 0;
    local_10.right = 0;
    local_10.bottom = 0;
    hdc = CreateCompatibleDC(NULL);
    iVar4 = param_2;
    param_2 = 0;
    local_10.right = param_4;
    SelectFont(hdc, param_6);
    while (1) {
        iVar2 = DrawTextA(hdc, param_1, strlen(param_1), &local_10, 0x411);
        if (iVar2 <= iVar4) {
            return param_2;
        }
        iVar4 = iVar4 + param_3;
        local_10.right = local_10.right + param_5;
        param_2 = param_2 + 1;
        if (iVar2 <= iVar4) {
            return param_2;
        }
    }
}

// FUNCTION: LEGOLAND 0x00471840
int FUN_00471840(const char *param_1, int param_2, int param_3, int param_4, int param_5, int param_6) {
    HDC hdc;
    RECT local_10;

    local_10.left = 0;
    local_10.top = 0;
    local_10.right = 0;
    local_10.bottom = 0;
    hdc = CreateCompatibleDC(NULL);
    local_10.right = param_4;
    SelectFont(hdc, param_6);
    DrawTextA(hdc, param_1, strlen(param_1), &local_10, 0x401);
    return (((local_10.right - local_10.left) - param_4) + 0x1f) >> 5;
}

// FUNCTION: LEGOLAND 0x004718c0
void FUN_004718c0(int param_1) {
    int x;
    int y;
    int width;
    int height;

    x = DAT_007fdecc;
    y = DAT_007fded0;
    width = param_1 * 0x20 + 0xc8;
    height = param_1 * 0x14 + 0x96;
    if (x > 0x27b - width) {
        x = x + (-5 - width);
        if (x < 0) {
            x = 5;
        }
    }
    if (x < 0x82 && DAT_007fdd80 != 2) {
        x = 0x82;
    }
    if (y < 0x25) {
        y = 0x25;
    } else if (y > 0x16f - height) {
        y = 0x16f - height;
    }
    DAT_007fdecc = x;
    DAT_007fded0 = y;
}

// FUNCTION: LEGOLAND 0x00471950
LEGO_EXPORT void PopUpInfoSetUp(struct HoverInfo t, unsigned int param_4, unsigned int param_5) {
    short sVar1;
    int iVar2;
    int x;
    int y;
    unsigned int uVar3;
    unsigned char *iVar4;
    int iVar5;

    uVar3 = t.data.value & 0xffff;
    x = uVar3 & 0xff;
    y = uVar3 >> 8;
    if ((x < 0) || (lpConfig->width <= x) || (y < 0) || (lpConfig->height <= y)) {
        iVar4 = NULL;
    } else {
        iVar4 = (unsigned char *)GameMap[y] + x * 0x14;
    }
    ResetInfoStruct();
    DAT_007fdfa0 = 1;
    DAT_007fdfa8 = 1;
    DAT_007fdecc = param_4;
    DAT_007fded0 = param_5;
    DAT_007fdf88 = (unsigned short)t.data.value;
    DAT_007fdec0.type = t.type;
    DAT_007fdec0.ptr = t.ptr;
    DAT_007fdec0.data.value = t.data.value;
    if (t.type < 0x308) {
        if (t.type == 0x307) {
            if (*(short *)((char *)t.ptr + 0xc) != 5) {
                PlayInstanceOfSample(DAT_004b92e4, 0, 1, 0);
                PickUpWorker(0x307, t.ptr);
                ResetInfoStruct();
                return;
            }
        } else if (t.type == 0x103) {
            if (t.ptr == NULL) {
                DAT_007fdfa0 = 1;
                DAT_007fdfa8 = 1;
                return;
            }
            if (((unsigned int)t.ptr != PathControlHandle) && ((unsigned int)t.ptr != Entrance1Handle)) {
                DAT_007fdf7c = *(unsigned int *)((char *)t.ptr + 0xc);
                DAT_007fdf84 = iVar4;
                if (*(unsigned int *)(DAT_007fdf7c + 0xc4) == PottingShedHandle) {
                    DAT_007fdfa0 = 0;
                    DAT_007fdf9c = 10;
                    param_4 = iVar4[4];
                    param_5 = iVar4[5];
                    if (BuyGardener() == 0) {
                        return;
                    }
                    GenerateGardener(&param_4, 1);
                    return;
                }
                if (*(unsigned int *)(DAT_007fdf7c + 0xc4) != MechanicsHutHandle) {
                    DAT_007fdf9c = 0x103;
                    return;
                }
                DAT_007fdfa0 = 0;
                DAT_007fdf9c = 0x14;
                param_4 = iVar4[4];
                param_5 = iVar4[5];
                if (BuyMechanic() == 0) {
                    return;
                }
                GenerateMechanic(&param_4, 1);
                return;
            }
        } else if (t.type == 0x306) {
            DAT_007fdf9c = t.type;
            DAT_007fdf8c = t.ptr;
            DAT_007fdf90 = *(unsigned int *)(*(int *)((char *)t.ptr + 4) + 0x1c);
            DAT_007fdf94 = *(unsigned int *)(*(int *)((char *)t.ptr + 4) + 0x20);
            return;
        }
    } else if ((t.type == 0x308) && (sVar1 = *(short *)((char *)t.ptr + 0xc), sVar1 != 5)) {
        if (((sVar1 == 0x13) && (0x6a < *(unsigned char *)((char *)t.ptr + 0x60))) ||
            ((sVar1 == 0x16) && (0x6a < *(unsigned char *)((char *)t.ptr + 0x60)))) {
            iVar4 = *(unsigned char **)((char *)t.ptr + 0x50);
            iVar5 = *(int *)(iVar4 + 8);
            if ((iVar5 < 0) ||
                (((int)(unsigned int)lpConfig->width <= iVar5 ||
                     (iVar2 = *(int *)(iVar4 + 0xc), iVar2 < 0)) ||
                    ((int)(unsigned int)lpConfig->height <= iVar2))) {
                iVar5 = 0;
            } else {
                iVar5 = (int)GameMap[iVar2] + iVar5 * 0x14;
            }
            *(unsigned short *)(iVar5 + 0xc) = *(unsigned short *)(iVar5 + 0xc) & 0xbfff;
            FreeMechanicWorkOrder(iVar4);
        }
        PlayInstanceOfSample(DAT_004b9308, 0, 1, 0);
        PickUpWorker(0x308, t.ptr);
    }
    ResetInfoStruct();
}

// FUNCTION: LEGOLAND 0x00471bf0
void FUN_00471bf0(void) {
    if (DAT_007fdfa0 != 2) {
        NewObjects.current = 0;
        NewObjects.count = 0;
    }
}

// FUNCTION: LEGOLAND 0x00471c10
void AddNewObjectIcon(struct NewObjInfo *param_1) {
    struct Sprite *sprite;
    char local_200[512];

    if ((int)NewObjects.count < 0x14) {
        // STRING: LEGOLAND 0x004bacd8
        sprintf(local_200, "NewObjIcons\\%s.bmp", *param_1->name);
        sprite = LoadSprite(local_200, 0);
        NewObjects.sprites[NewObjects.count] = sprite;
        if (NewObjects.sprites[NewObjects.count] != NULL) {
            NewObjects.objs[NewObjects.count] = param_1;
            NewObjects.count = NewObjects.count + 1;
            return;
        }
        // STRING: LEGOLAND 0x004bacb4
        DBPrintf("Failied to open New Obj graphic %s\n", local_200);
    }
}

// FUNCTION: LEGOLAND 0x00471ca0
void RemoveNewObject(void *param) {
    struct Sprite **puVar2;
    int iVar3;
    int iVar4;
    int iVar5;
    struct Sprite **puVar6;
    int iVar7;

    if (0 < (int)NewObjects.count) {
        puVar6 = NewObjects.sprites;
        iVar5 = NewObjects.count;
        iVar7 = 1;
        do {
            if (param == ((void **)puVar6)[-0x14]) {
                if (*puVar6 != NULL) {
                    KillSprite(*puVar6);
                    *puVar6 = NULL;
                    iVar5 = NewObjects.count;
                }
                puVar2 = puVar6;
                iVar3 = iVar7;
                iVar4 = iVar5;
                if (iVar7 < iVar5) {
                    do {
                        iVar3 = iVar3 + 1;
                        *puVar2 = puVar2[1];
                        ((void **)puVar2)[-0x14] = ((void **)puVar2)[-0x13];
                        puVar2 = puVar2 + 1;
                        iVar4 = NewObjects.count;
                    } while (iVar3 < NewObjects.count);
                }
                iVar5 = iVar4 + -1;
                if (iVar5 <= NewObjects.current) {
                    NewObjects.current = iVar4 + -2;
                }
                NewObjects.count = iVar5;
                if ((iVar5 == 0) && (DAT_007fdfa0 == 2)) {
                    DAT_007fdfa0 = 0;
                }
            }
            puVar6 = puVar6 + 1;
            iVar7 = iVar7 + 1;
        } while (iVar7 + -1 < iVar5);
    }
}

// FUNCTION: LEGOLAND 0x00471d40
void FUN_00471d40(void) {
    if (NewObjects.count != 0) {
        ResetInfoStruct();
        DAT_007fdfa0 = 2;
    }
}

// FUNCTION: LEGOLAND 0x00471d60
void FUN_00471d60(void) {
    SetIconSprite((struct IconNode *)DAT_007fdea8, PUOKSprite);
    SetIconSprite((struct IconNode *)DAT_007fe000, CBCloseSprite);
}

// FUNCTION: LEGOLAND 0x00471d90
void FUN_00471d90(void) {
    int iVar1;
    short sVar2;
    char *str;
    int uVar4;
    int iVar5;
    int iVar6;
    int iVar7;
    int iVar8;
    struct PrintCtx ctx;
    char local_14[20];

    iVar6 = DAT_007fdecc;
    uVar4 = *(unsigned int *)&DAT_007fdfac & 0xff;
    iVar5 = uVar4 * 0x14;
    ctx.node = 0;
    ctx.field_8 = 0;
    ctx.flags = 1;
    iVar1 = DAT_007fded0 + 0x48 + iVar5;
    PrintSprite(CBBGLeftSprite, DAT_007fdecc, iVar1, 0, (int *)&ctx);
    iVar6 = iVar6 + 0x7a;
    for (; uVar4 > 0; uVar4 = uVar4 - 1) {
        PrintSprite(CBBGCentreSprite, iVar6, iVar1, 0, (int *)&ctx);
        iVar6 = iVar6 + 0x20;
    }
    PrintSprite(CBBGRightSprite, iVar6, iVar1, 0, (int *)&ctx);
    DAT_007fdea8->flags = DAT_007fdea8->flags & 0xfffffbff;
    iVar6 = iVar6 + 0x4e;
    DAT_007fdea8->x = (short)iVar6 - 0x4b;
    sVar2 = (short)iVar1 + 3;
    DAT_007fdea8->y = sVar2;
    DAT_007fe000->flags = DAT_007fe000->flags & 0xfffffbff;
    DAT_007fe000->x = (short)iVar6 - 0x27;
    DAT_007fe000->y = sVar2;
    str = GetString(0xa2);
    sprintf(local_14, (char *)DAT_004b8bbc, str);
    iVar7 = DAT_007fdecc + 0xc;
    iVar8 = iVar1 + 6;
    FUN_00455e50(local_14, iVar7, iVar8, (DAT_007fdecc + 0x86 + iVar5) - iVar7, (iVar1 + 0x21) - iVar8, 1, 5, 0xff0000, 0xffffff);
    if ((DAT_007fe000->x + 0x24 < (int)DAT_00813a44.x) || ((int)DAT_00813a44.x < DAT_007fdea8->x)) {
        FUN_00471d60();
    }
    if ((iVar1 + 0x1b < (int)DAT_00813a44.y) || ((int)DAT_00813a44.y < iVar1)) {
        FUN_00471d60();
    }
    FUN_00471470();
}

// FUNCTION: LEGOLAND 0x00471f10
void FUN_00471f10(void) {
    int iVar1;
    int iVar2;
    int uVar3;
    int uVar4;
    int iVar5;
    int iVar6;
    int iVar7;
    int local_1c;
    struct PrintCtx ctx;

    iVar2 = DAT_007fded0;
    iVar1 = DAT_007fdecc;
    ctx.node = 0;
    ctx.flags = 1;
    ctx.field_8 = 0;
    uVar4 = *(unsigned int *)&DAT_007fdfac & 0xff;
    PrintSprite(PuBgMainSprite, DAT_007fdecc, DAT_007fded0, 0, (int *)&ctx);
    iVar6 = iVar1 + 0xbc;
    iVar7 = iVar6;
    for (uVar3 = uVar4; uVar3 > 0; uVar3 = uVar3 - 1) {
        PrintSprite(PuBgCentreTopSprite, iVar7, iVar2, 0, (int *)&ctx);
        iVar7 = iVar7 + 0x20;
    }
    PrintSprite(PuBgRightTopSprite, iVar7, iVar2, 0, (int *)&ctx);
    if (uVar4 != 0) {
        iVar7 = iVar2 + 99;
        local_1c = uVar4;
        do {
            PrintSprite(PuBgLeftMidSprite, iVar1, iVar7, 0, (int *)&ctx);
            uVar3 = uVar4;
            iVar5 = iVar6;
            do {
                PrintSprite(PuBgCentreMidSprite, iVar5, iVar7, 0, (int *)&ctx);
                iVar5 = iVar5 + 0x20;
                uVar3 = uVar3 - 1;
            } while (uVar3 != 0);
            PrintSprite(PuBgRightMidSprite, iVar5, iVar7, 0, (int *)&ctx);
            iVar7 = iVar7 + 0x14;
            local_1c = local_1c - 1;
        } while (local_1c != 0);
    }
    iVar7 = iVar2 + 99 + uVar4 * 0x14;
    PrintSprite(PuBgRightBtmSprite, iVar1, iVar7, 0, (int *)&ctx);
    for (; uVar4 > 0; uVar4 = uVar4 - 1) {
        PrintSprite(PuBgCentreBtmSprite, iVar6, iVar7, 0, (int *)&ctx);
        iVar6 = iVar6 + 0x20;
    }
    PrintSprite(PuBgLeftBtmSprite, iVar6, iVar7, 0, (int *)&ctx);
}

// FUNCTION: LEGOLAND 0x00472090
void FUN_00472090(void) {
}

// FUNCTION: LEGOLAND 0x004720a0
void DrawNewObjectPopup(void) {
    struct NewObjInfo *obj;
    struct PrintCtx ctx = {1, 0, 0};
    char local_80[128];

    PrintSprite(NewPopMockSprite, 0xbe, 0x28, 0, (int *)&ctx);
    PrevPopUpIcon->flags = PrevPopUpIcon->flags & 0xfffffbff;
    PrevPopUpIcon->x = 0xc1;
    PrevPopUpIcon->y = 0x46;
    NextPopUpIcon->flags = NextPopUpIcon->flags & 0xfffffbff;
    NextPopUpIcon->x = NewPopMockSprite->width - ClosePopUpIcon->width + 0xbb;
    NextPopUpIcon->y = 0x46;
    ClosePopUpIcon->flags = ClosePopUpIcon->flags & 0xfffffbff;
    ClosePopUpIcon->x = NewPopMockSprite->width - ClosePopUpIcon->width + 0xbb;
    ClosePopUpIcon->y = NewPopMockSprite->height - ClosePopUpIcon->height + 0x25;
    PushRenderingStatusAndUnlockVideoSurface();
    if (NewObjects.count == 1) {
        // STRING: LEGOLAND 0x004bad04
        sprintf(local_80, "You have a new object");
    } else {
        // STRING: LEGOLAND 0x004bacec
        sprintf(local_80, "You have %d new objects", NewObjects.count);
    }
    FUN_00455e50(local_80, 0xc1, 0x30, NewPopMockSprite->width + 0xbb - 0xc1, 0x14, 2, 5, 0xff0000, 0xffffff);
    obj = (struct NewObjInfo *)NewObjects.objs[NewObjects.current];
    FUN_00455e50(obj->field_78, 0xc1, 0x4b, NewPopMockSprite->width + 0xbb - 0xc1, 0x14, 2, 5, 0xff0000, 0xffffff);
    obj = (struct NewObjInfo *)NewObjects.objs[NewObjects.current];
    FUN_00455e50(obj->field_80, 0x13e, 0x68, 0xfc, 0x77, 2, 0x10, 0xff0000, 0xffffff);
    obj = (struct NewObjInfo *)NewObjects.objs[NewObjects.current];
    sprintf(local_80, "%d", obj->field_26);
    FUN_00455e50(local_80, 0xf0, 0xd1, 0x43, 0x12, 2, 1, 0xff0000, 0xffffff);
    PopRenderingStatus();
    PrintSprite(NewObjects.sprites[NewObjects.current], 0xc4, 100, 0, 0);
    if (((unsigned int)(int)ClosePopUpIcon->width <= (unsigned int)(DAT_00813a44.x - ClosePopUpIcon->x)) ||
        ((unsigned int)(int)ClosePopUpIcon->height <= (unsigned int)(DAT_00813a44.y - ClosePopUpIcon->y))) {
        SetIconSprite((struct IconNode *)ClosePopUpIcon, PuClosePopUpSprite);
    }
    if (((unsigned int)(int)NextPopUpIcon->width <= (unsigned int)(DAT_00813a44.x - NextPopUpIcon->x)) ||
        ((unsigned int)(int)NextPopUpIcon->height <= (unsigned int)(DAT_00813a44.y - NextPopUpIcon->y))) {
        SetIconSprite((struct IconNode *)NextPopUpIcon, NextIconSprite);
    }
    if (((unsigned int)(int)PrevPopUpIcon->width <= (unsigned int)(DAT_00813a44.x - PrevPopUpIcon->x)) ||
        ((unsigned int)(int)PrevPopUpIcon->height <= (unsigned int)(DAT_00813a44.y - PrevPopUpIcon->y))) {
        SetIconSprite((struct IconNode *)PrevPopUpIcon, PrevIconSprite);
    }
    if (NewObjects.current == 0) {
        PrevPopUpIcon->flags = PrevPopUpIcon->flags | 0x400;
    } else {
        PrevPopUpIcon->flags = PrevPopUpIcon->flags & 0xfffffbff;
    }
    if (NewObjects.current == (int)(NewObjects.count + -1)) {
        NextPopUpIcon->flags = NextPopUpIcon->flags | 0x400;
        return;
    }
    NextPopUpIcon->flags = NextPopUpIcon->flags & 0xfffffbff;
}

// FUNCTION: LEGOLAND 0x004723f0
int FUN_004723f0(void) {
    unsigned int local_8[2];
    struct Cursor local_cursor;
    void *saved_class;
    struct ObjClass *cls;
    unsigned int v;

    saved_class = QueryClass;
    v = DAT_007fdec0.data.value & 0xffff;
    memcpy(&local_cursor, &QueryCursor, sizeof(struct Cursor));
    local_8[0] = v & 0xff;
    local_8[1] = v >> 8;
    cls = *(struct ObjClass **)((char *)DAT_007fdec0.ptr + 0xc);
    QueryCursor.tile_y = v >> 8;
    QueryClass = cls;
    QueryCursor.tile_x = v & 0xff;
    cls->method_94(cls->element, local_8);
    BuildCursorPtr(&QueryCursor, 0, 0);
    FUN_0045f4b0(&QueryCursor);
    memcpy(&QueryCursor, &local_cursor, sizeof(struct Cursor));
    QueryClass = saved_class;
}

/* Draws the info popup: the title/info text for the selected object or bloke, the mood
   sprites, the build/repair progress bar and the icons (repair, delete, add gardener/mechanic). */
// FUNCTION: LEGOLAND 0x004724a0
LEGO_EXPORT void DrawPopUpInfo(void) {
    char buf_b[256];
    char buf_a[256];
    char tmp[512];
    struct Ride *ride;
    struct Bloke *bloke;
    int repairable = 0;
    int can_delete = 0;
    int add_mechanic = 0;
    int add_gardener = 0;
    int show_close = 0;
    int size;
    int x;
    int y;
    int left;
    int right;
    int width;
    int bar_x;
    int bar_y;
    volatile int mid; /* kept in memory, as in the original frame */
    int ty;
    int bottom;
    int top;
    int mood;
    int hunger;
    int power;
    int name_lines;
    int info_lines;
    int entry_index;
    int icon_x;
    int icon_y;
    int last_x;
    int left_bound;
    float frac;
    struct BuildObj *entry;
    struct InfoObjData *info;

    buf_a[0] = 0;
    memset(buf_a + 1, 0, 255);
    buf_b[0] = 0;
    memset(buf_b + 1, 0, 255);
    ride = (struct Ride *)DAT_007fdf7c;
    bloke = (struct Bloke *)DAT_007fdf8c;
    GetNearestColour(0xda, 0xc6, 0x96);
    DAT_0066895c = 0;
    if (DAT_007fdfa0 == 2) {
        DrawNewObjectPopup();
        return;
    }
    if (EditMode.unk0 != 0) {
        ResetInfoStruct();
        return;
    }
    if (DAT_007fdfa0 == 0) {
        return;
    }
    if (DAT_00813a60 & 2) {
        ResetInfoStruct();
        return;
    }
    switch (DAT_007fdf9c) {
    case 0x103:
        sprintf(buf_a, (char *)DAT_004b8bbc, ride->name);
        // STRING: LEGOLAND 0x004bad44
        sprintf(buf_b, "%s %d\n%s %d", GetString(0x76), GetObjRepairCost(ride, DAT_007fdf84[0x11]), GetString(0x77), GetObjSalvageValue(ride, DAT_007fdf84[0x11]));
        if (MapStats.field_18c != 0) {
            power = FindObjectsPower(ride);
            if (power < 0) {
                // STRING: LEGOLAND 0x004bad3c
                sprintf(tmp, "\n%s %d", GetString(0x78), -power);
                if (DAT_007fdf84[0xd] & 1) {
                    // STRING: LEGOLAND 0x004bad38
                    strcat(tmp, "\n");
                    strcat(tmp, GetString(0x7a));
                }
            } else if (power != 0) {
                if (DAT_007fdf84[0x11] >= ride->durability >> 2) {
                    sprintf(tmp, "\n%s %d", GetString(0x79), power);
                } else {
                    // STRING: LEGOLAND 0x004bad34
                    sprintf(tmp, "\n%s", GetString(0x7b));
                }
            }
            if (power != 0) {
                strcat(buf_b, tmp);
            }
        }
        if (ride->durability != 0) {
            repairable = 1;
        }
        if (!(DAT_007fdf84[0xc] & 0x40)) {
            can_delete = 1;
        }
        break;
    case 0x14:
        sprintf(buf_a, (char *)DAT_004b8bbc, ride->name);
        // STRING: LEGOLAND 0x004bad2c
        sprintf(buf_b, "%s : %d", GetString(0x93), GetMechanicCount());
        // STRING: LEGOLAND 0x004bad1c
        sprintf(tmp, "\n%s %d\n%s %d", GetString(0x76), GetObjRepairCost(ride, DAT_007fdf84[0x11]), GetString(0x77), GetObjSalvageValue(ride, DAT_007fdf84[0x11]));
        strcat(buf_b, tmp);
        add_mechanic = 1;
        if (!(DAT_007fdf84[0xc] & 0x40)) {
            can_delete = 1;
        }
        break;
    case 0xa:
        sprintf(buf_a, (char *)DAT_004b8bbc, ride->name);
        sprintf(buf_b, "%s : %d", GetString(0x91), GetGardenerCount());
        sprintf(tmp, "\n%s %d\n%s %d", GetString(0x76), GetObjRepairCost(ride, DAT_007fdf84[0x11]), GetString(0x77), GetObjSalvageValue(ride, DAT_007fdf84[0x11]));
        strcat(buf_b, tmp);
        add_gardener = 1;
        if (!(DAT_007fdf84[0xc] & 0x40)) {
            can_delete = 1;
        }
        break;
    case 0x104:
        sprintf(buf_a, (char *)DAT_004b8bbc, ride->name);
        sprintf(buf_b, (char *)DAT_004b8bbc, GetString(0xa0));
        DAT_0066895c = 1;
        break;
    case 0x10b:
        info = DAT_007fdf80;
        if (info->field_18 != 0 && info->field_1c->field_60 >= 0x6b) {
            DAT_007fdec0.type = 0x104;
            PopUpInfoSetUp(DAT_007fdec0, DAT_007fdecc, DAT_007fded0);
            return;
        }
        sprintf(buf_a, (char *)DAT_004b8bbc, *(char **)info->field_4);
        sprintf(buf_b, (char *)DAT_004b8bbc, GetString(0xd2));
        show_close = 1;
        break;
    case 0x10c:
        info = DAT_007fdf80;
        if (info->field_18 != 0 && info->field_1c->field_60 >= 0x6b) {
            DAT_007fdec0.type = 0x104;
            PopUpInfoSetUp(DAT_007fdec0, DAT_007fdecc, DAT_007fded0);
            return;
        }
        sprintf(buf_a, (char *)DAT_004b8bbc, *(char **)info->field_4);
        sprintf(buf_b, (char *)DAT_004b8bbc, GetString(0xd3));
        show_close = 1;
        break;
    case 0x306:
        if (bloke->action == 3 && bloke->param_action >= 0xc) {
            ResetInfoStruct();
            return;
        }
        sprintf(buf_a, GetVisitorName(DAT_007fdf8c));
        sprintf(buf_b, DAT_004d8bb0);
        DAT_007fdf98 = 1;
        break;
    }
    if (DAT_007fdfa8 != 0) {
        if (DAT_007fdf9c == 0x306) {
            DAT_007fdfac = 2;
        } else {
            name_lines = FUN_00471840(buf_a, 0x40, 0x14, 0xb0, 0x20, 1);
            info_lines = FUN_004717a0(buf_b, 0x40, 0x14, 0xb0, 0x20, 2);
            DAT_007fdfac = info_lines;
            if (info_lines <= name_lines) {
                DAT_007fdfac = name_lines;
            }
        }
        DAT_007fdfa8 = 0;
    }
    FUN_004718c0(*(unsigned int *)&DAT_007fdfac & 0xff);
    size = *(unsigned int *)&DAT_007fdfac & 0xff;
    FUN_00471f10();
    PushRenderingStatusAndUnlockVideoSurface();
    x = DAT_007fdecc;
    y = DAT_007fded0;
    if (buf_a) {
        struct Point tl;
        struct Point br;
        tl.x = x + 0xc;
        tl.y = y + 6;
        br.x = size * 32 + x + 0xbc;
        br.y = y + 0x19;
        FUN_00455e50(buf_a, tl.x, tl.y, br.x - tl.x, br.y - tl.y, 1, 1, 0xff0000, 0xffffff);
    }
    if (buf_b) {
        struct Point tl;
        struct Point br;
        tl.x = x + 0xc;
        tl.y = y + 0x23;
        br.x = size * 32 + x + 0xbc;
        br.y = y + size * 20 + 0x63;
        FUN_00455e50(buf_b, tl.x, tl.y, br.x - tl.x, br.y - tl.y, 2, 0x10, 0xff0000, 0xffffff);
    }
    PopRenderingStatus();
    if (DAT_007fdf9c == 0x306) {
        mood = GetBlokeMood(bloke);
        hunger = FUN_0044eb10(bloke);
        left = x + 0xc;
        right = size * 32 + x + 0xb0;
        width = right - left;
        top = y + 0x23;
        bottom = y + size * 20 + 0x63;
        mid = (top + bottom) / 2;
        ty = mid + 0x22;
        FUN_00455e50(GetString(0x8e), left, ty, width / 2, 0x14, 2, 0x11, 0xff0000, 0xffffff);
        FUN_00455e50(GetString(0x8f), (right + left) / 2, ty, width / 2, 0x14, 2, 0x11, 0xff0000, 0xffffff);
        if (mood == 3) {
            mid -= 0x20;
            PrintSprite(ISadSprite, width / 4 + left - 0x20, mid, 0, 0);
        } else if (mood == 2) {
            mid -= 0x20;
            PrintSprite(IHappySprite, width / 4 + left - 0x20, mid, 0, 0);
        } else {
            mid -= 0x20;
            PrintSprite(INormSprite, width / 4 + left - 0x20, mid, 0, 0);
        }
        if (hunger == 0) {
            PrintSprite(IFullSprite, right - width / 4 - 0x20, mid, 0, 0);
        } else if (hunger == 1) {
            PrintSprite(IPeckishSprite, right - width / 4 - 0x20, mid, 0, 0);
        } else {
            PrintSprite(IHungrySprite, right - width / 4 - 0x20, mid, 0, 0);
        }
    }
    if (repairable != 0 || DAT_0066895c != 0) {
        if (repairable != 0) {
            frac = (float)DAT_007fdf84[0x11] / ride->durability;
        } else {
            entry_index = 0;
            for (entry = BuildObjArray;; entry++, entry_index++) {
                if ((int)&entry->coords >= (int)&ButtonRepeatDelay) {
                    return;
                }
                if (entry->coords.id == (unsigned short)DAT_007fdec0.data.value) {
                    break;
                }
            }
            if (entry_index >= 0x100) {
                return;
            }
            frac = (float)BuildObjArray[entry_index].elapsed / GetBuildTime((struct Ride *)DAT_007fdf7c);
            if (frac == 1.0f) {
                DAT_007fdec0.type = 0x103;
                PopUpInfoSetUp(DAT_007fdec0, DAT_007fdecc, DAT_007fded0);
                return;
            }
        }
        bar_x = x + 6;
        bar_y = y + size * 20 + 0x6f;
        width = size * 32 + 0xbc;
        RenderBlock(bar_x, bar_y, width, 6, 0);
        RenderBlock(bar_x, bar_y, (int)(width * frac), 6, (frac < 0.25 && repairable != 0) ? GetNearestColour(0xff, 0, 0) : GetNearestColour(0, 0xff, 0));
    }
    icon_x = size * 32 + x + 0xc8;
    icon_y = y + (size * 5 + 0x1e) * 4;
    ClosePopUpIcon->flags = ClosePopUpIcon->flags & 0xfffffbff;
    ClosePopUpIcon->x = icon_x - 0x27;
    ClosePopUpIcon->y = icon_y;
    last_x = ClosePopUpIcon->x;
    if (show_close != 0) {
        DAT_007fdfcc->flags = DAT_007fdfcc->flags & 0xfffffbff;
        DAT_007fdfcc->x = icon_x - 0x4e;
        DAT_007fdfcc->y = icon_y;
        last_x = DAT_007fdfcc->x;
    }
    if (can_delete != 0) {
        if (FUN_004723f0() != 0) {
            DeleteObjectIcon->flags = DeleteObjectIcon->flags & 0xfffffbff;
            DeleteObjectIcon->x = icon_x - 0x4e;
            DeleteObjectIcon->y = icon_y;
            last_x = DeleteObjectIcon->x;
        }
    }
    if (add_gardener != 0) {
        AddGardenerIcon->flags = AddGardenerIcon->flags & 0xfffffbff;
        AddGardenerIcon->y = icon_y;
        if (can_delete != 0) {
            AddGardenerIcon->x = icon_x - 0x75;
        } else {
            AddGardenerIcon->x = icon_x - 0x4e;
        }
        last_x = AddGardenerIcon->x;
    }
    if (add_mechanic != 0) {
        AddMechanicsIcon->flags = AddMechanicsIcon->flags & 0xfffffbff;
        AddMechanicsIcon->y = icon_y;
        if (can_delete != 0) {
            AddMechanicsIcon->x = icon_x - 0x75;
        } else {
            AddMechanicsIcon->x = icon_x - 0x4e;
        }
        last_x = AddMechanicsIcon->x;
    }
    PuCornerMaskIcon->x = last_x;
    PuCornerMaskIcon->y = icon_y;
    PuCornerMaskIcon->flags = PuCornerMaskIcon->flags & 0xfffffbff;
    if (DAT_007fdfa4 != 0) {
        left_bound = ClosePopUpIcon->x;
    } else {
        left_bound = PuCornerMaskIcon->x;
    }
    if (ClosePopUpIcon->x + 0x24 < DAT_00813a44.x || DAT_00813a44.x < left_bound) {
        FUN_00471610();
    }
    if (icon_y + 0x1b < DAT_00813a44.y || DAT_00813a44.y < icon_y) {
        FUN_00471610();
    }
    if (DAT_007fdfa4 != 0) {
        FUN_00471d90();
    }
    FUN_00472090();
}

// FUNCTION: LEGOLAND 0x004730f0
unsigned char FUN_004730f0(void *param1, unsigned char param2, unsigned int param3, unsigned int param4) {
    FUN_00471610();

    if (param1) {
        SetIconSprite(param1, PuClosePopUpOnSprite);
    }

    if (param2 & 0x2) {
        RemoveAllNewObjects();
        ResetInfoStruct();
    }

    return 1;
}

// FUNCTION: LEGOLAND 0x00473130
unsigned int FUN_00473130(void) {
    if (DAT_007fdfa0 != 0) {
        FUN_004730f0(NULL, 0x2, 0, 0);
        return 1;
    } else {
        return 0;
    }
}

// FUNCTION: LEGOLAND 0x00473160
unsigned int FUN_00473160(void) {
    unsigned int state = DAT_007fdfa0;

    if (state == 1) {
        ResetInfoStruct();
        return 1;
    }

    if (state == 2) {
        FUN_00473360(NULL, state, 0, 0);
        return 1;
    }

    return 0;
}

// FUNCTION: LEGOLAND 0x004731a0
unsigned char FUN_004731a0(void *param_1, unsigned char param_2) {
    FUN_00471610();
    SetIconSprite(param_1, PuDeleteObjectOnSprite);
    if ((param_2 & 2) != 0) {
        FUN_00471470();
        DAT_007fdfa4 = 1;
        return 1;
    }
    return 1;
}

// FUNCTION: LEGOLAND 0x004731e0
unsigned char FUN_004731e0(void *param_1, unsigned char flags) {
    unsigned int local_8[2];
    struct Cursor local_cursor;
    void *saved_class;
    struct ObjClass *cls;
    unsigned int v;

    FUN_00471d60();
    SetIconSprite(param_1, PUOKOnSprite);
    if ((flags & 2) != 0) {
        v = DAT_007fdec0.data.value & 0xffff;
        saved_class = QueryClass;
        memcpy(&local_cursor, &QueryCursor, sizeof(struct Cursor));
        local_8[0] = v & 0xff;
        local_8[1] = v >> 8;
        cls = *(struct ObjClass **)((char *)DAT_007fdec0.ptr + 0xc);
        QueryCursor.tile_y = local_8[1];
        QueryObj.pos.y = (unsigned char)local_8[1];
        QueryClass = cls;
        QueryCursor.tile_x = local_8[0];
        QueryObj.pos.x = (unsigned char)local_8[0];
        cls->method_94(cls->element, local_8);
        BuildCursorPtr(&QueryCursor, 0, 0);
        if ((int)FUN_0045f4b0(&QueryCursor) != 0) {
            FUN_0045d3d0(QueryClass, local_8);
            cls = QueryClass;
            RemObjFromMap(cls, cls->element, QueryObj, &QueryCursor);
        }
        memcpy(&QueryCursor, &local_cursor, sizeof(struct Cursor));
        QueryClass = saved_class;
        ResetInfoStruct();
    }
    return 1;
}

// FUNCTION: LEGOLAND 0x00473310
unsigned char FUN_00473310(void *param1, unsigned char param2) {
    FUN_00471d60();
    SetIconSprite(param1, CBCloseOnSprite);

    if (param2 & 0x2) {
        DAT_007fdea8->flags |= 0x400;
        DAT_007fe000->flags |= 0x400;
        FUN_004714a0();
        DAT_007fdfa4 = 0;
    }

    return 1;
}

// FUNCTION: LEGOLAND 0x00473360
unsigned char FUN_00473360(void *arg1, unsigned char flags, unsigned int arg3, unsigned int arg4) {
    FUN_00471d60();
    if (arg1) {
        SetIconSprite(arg1, NextIconOnSprite);
    }
    if (flags & 2) {
        if (NewObjects.current < (int)(NewObjects.count - 1)) {
            NewObjects.current++;
        } else {
            ResetInfoStruct();
        }
    }
    return 1;
}

// FUNCTION: LEGOLAND 0x004733b0
unsigned char FUN_004733b0(void *arg0, unsigned char flags) {
    FUN_00471d60();
    SetIconSprite(arg0, PrevIconOnSprite);

    if (flags & 0x2) {
        if (NewObjects.current > 0) {
            NewObjects.current--;
        }
    }

    return 1;
}

// FUNCTION: LEGOLAND 0x004733f0
unsigned char FUN_004733f0(void *param_1, unsigned char param_2) {
    unsigned int local_8[2];

    FUN_00471610();
    if (GardenerCount < 0xf) {
        SetIconSprite(param_1, PuAddGardenerOnSprite);
        if ((param_2 & 2) != 0) {
            local_8[0] = DAT_007fdf84[4];
            local_8[1] = DAT_007fdf84[5];
            if (BuyGardener() != 0) {
                GenerateGardener(local_8, 1);
            }
        }
    }
    return 1;
}

// FUNCTION: LEGOLAND 0x00473460
unsigned char FUN_00473460(void *param_1, unsigned char param_2) {
    unsigned int local_8[2];

    FUN_00471610();
    if (MechanicCount < 0xf) {
        SetIconSprite(param_1, PuAddMechanicsOnSprite);
        if ((param_2 & 2) != 0) {
            local_8[0] = DAT_007fdf84[4];
            local_8[1] = DAT_007fdf84[5];
            if (BuyMechanic() != 0) {
                GenerateMechanic(local_8, 1);
            }
        }
    }
    return 1;
}

// FUNCTION: LEGOLAND 0x004734d0
unsigned char FUN_004734d0(void *param_1, unsigned char param_2) {
    FUN_00471610();
    SetIconSprite(param_1, PuDeleteObjectOnSprite);
    if ((param_2 & 2) != 0) {
        if (DAT_007fdf9c == 0x10b) {
            SetObjRectFlags(DAT_007fdf80->field_4, &DAT_007fdf80->field_8, 0);
            RemoveGardenersWorkOrderAt(DAT_007fdf80->field_8, DAT_007fdf80->field_c);
            ResetInfoStruct();
            return 1;
        }
        SetObjRectFlags(DAT_007fdf80->field_4, &DAT_007fdf80->field_8, 0);
        RemoveMechanicsWorkOrderAt(DAT_007fdf80->field_8, DAT_007fdf80->field_c);
        ResetInfoStruct();
    }
    return 1;
}

// FUNCTION: LEGOLAND 0x004735b0
void FUN_004735b0(void) {
    DAT_00668960 = 0;
}

// FUNCTION: LEGOLAND 0x004735e0
unsigned int FUN_004735e0(unsigned int param) {
    struct InfoTimedEntry *entry;
    unsigned long now;

    entry = &DAT_004ba8e0[param];
    if ((int)DAT_00668960 < (int)param) {
        now = GetTicks();
        if ((int)(entry->interval + entry->last_time) < (int)now) {
            if (FUN_0046d280(entry->sample) != 0) {
                DAT_00668960 = param;
                entry->last_time = now;
                return 1;
            }
        }
    }
    return 0;
}

// FUNCTION: LEGOLAND 0x00473640
unsigned int FUN_00473640(unsigned int param_1) {
    return FUN_004735e0(DAT_004ba9ac[param_1]);
}

// FUNCTION: LEGOLAND 0x00473660
void FUN_00473660(void) {
    if (DAT_00668960 == 0) {
        return;
    }
    if (FUN_00498cf0() != 0) {
        return;
    }
    FUN_004735b0();
}

// FUNCTION: LEGOLAND 0x00473680
void FUN_00473680(int param_1, int param_2, char *param_3, unsigned int param_4, unsigned int param_5) {
    strcpy(DAT_00668968, param_3);
    DAT_007fe010 = param_1;
    DAT_007fe014 = param_2;
    FUN_00470950(param_4, param_5);
    DAT_00668d68 = 1;
}

// FUNCTION: LEGOLAND 0x004736e0
void FUN_004736e0(void) {
    KillPUOKAndCBSprites();
    DAT_00668d68 = 0;
}

// FUNCTION: LEGOLAND 0x004736f0
void FUN_004736f0(void) {
    int iVar1;
    int iVar2;
    int iVar3;
    int iVar4;
    struct PrintCtx ctx;

    iVar1 = DAT_007fe014;
    iVar3 = DAT_007fe010;
    if (DAT_00668d68 != 0) {
        ctx.node = 0;
        ctx.flags = 1;
        ctx.field_8 = 0;
        PrintSprite(CBBGLeftSprite, iVar3, iVar1, 0, (int *)&ctx);
        iVar3 = iVar3 + 0x7a;
        iVar2 = 0;
        if (0 < DAT_00668964) {
            do {
                PrintSprite(CBBGCentreSprite, iVar3, iVar1, 0, (int *)&ctx);
                iVar3 = iVar3 + 0x20;
                iVar2 = iVar2 + 1;
            } while (iVar2 < DAT_00668964);
        }
        PrintSprite(CBBGRightSprite, iVar3, iVar1, 0, (int *)&ctx);
        iVar3 = iVar3 + 0x4e;
        DAT_007fdea8->flags = DAT_007fdea8->flags & 0xfffffbff;
        DAT_007fdea8->x = (short)(iVar3 - 0x4b);
        DAT_007fdea8->y = (short)DAT_007fe014 + 3;
        DAT_007fe000->flags = DAT_007fe000->flags & 0xfffffbff;
        DAT_007fe000->x = (short)(iVar3 - 0x27);
        DAT_007fe000->y = (short)DAT_007fe014 + 3;
        iVar2 = DAT_007fe010 + 0xc;
        iVar4 = DAT_007fe014 + 6;
        FUN_00455e50(DAT_00668968, iVar2, iVar4, (iVar2 + DAT_00668964 * 0x14 + 0x7a) - iVar2,
            (iVar4 + 0x1b) - iVar4, 1, 5, 0xff0000, 0xffffff);
        if ((DAT_007fe000->x + 0x24 < (int)DAT_00813a44.x) || ((int)DAT_00813a44.x < DAT_007fdea8->x)) {
            FUN_00471d60();
        }
        if ((iVar1 + 0x1b < (int)DAT_00813a44.y) || ((int)DAT_00813a44.y < iVar1)) {
            FUN_00471d60();
        }
    }
}
