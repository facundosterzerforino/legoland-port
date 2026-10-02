#include "progress.h"
#include "draw.h"
#include "freeplay.h"
#include "globals.h"
#include "icon.h"
#include "interface.h"
#include "legoland.h"
#include "options.h"
#include "profile_io.h"
#include "screens.h"
#include "sound_music.h"
#include "string.h"

#include "image_sprite.h"
#include "stream.h"
#include "text.h"
#include "timer.h"

// FUNCTION: LEGOLAND 0x0048b7e0
LEGO_EXPORT void InitProgressScreen(void) {
    struct IconNode *icon;
    unsigned int flags;
    int i;

    if ((int)lpConfig->field_28 < 1) {
        lpConfig->field_28 = 1;
    }
    if (DAT_00798660 == 0) {
        FUN_0048b6d0();
        if (MapStats.field_3a0 == 0) {
            if (DAT_00798668 == 0) {
                lpConfig->field_28 = DAT_007cb394 + 1;
            }
        }
        if (MapStats.field_3a0 == 1 && (int)lpConfig->field_28 <= 0xf) {
            CurrentProfile.flags[3 + lpConfig->field_28] = 1;
            UpDateCurrentProfile();
        }
    }
    if ((int)lpConfig->field_28 <= 5) {
        FUN_0048bde0();
        return;
    }
    if ((int)lpConfig->field_28 > 0xf) {
        DAT_00668e38 = 1;
        SPRITE_TitleScreenBk = NULL;
        EditMode.unk4 = 2;
        DAT_0080ff80.unk4 = 0xffffffff;
        DAT_0080ff80.unk8 = 8;
        GamePad &= ~0x20;
        return;
    }
    DAT_00798664 = 0;
    // STRING: LEGOLAND 0x004bef44
    SPRITE_TitleScreenBk = LoadSprite("Progress_ScreenBK.lls", 4);
    flags = 0x6002;
    if (DAT_00798660 == 0) {
        if (MapStats.field_3a0 == 1 && lpConfig->field_28 == 6) {
            MapStats.field_3a0 = 2;
        }
        FUN_0048b700();
        // STRING: LEGOLAND 0x004bef2c
        icon = LoadSpriteIcon("Accept_on_Progress.lls", 4, 0x20e, 0x16f, 0x23);
        icon->string_id = 0x262;
        icon->string = GetString(0x262);
        icon->flags |= flags;
        icon->event_handler = (void *)FUN_0048bc20;
        // STRING: LEGOLAND 0x004bef14
        icon = LoadSpriteIcon("GoBack_on_Progress.lls", 4, 0x208, 0xb, 0x23);
        icon->string_id = 0x26;
        icon->string = GetString(0x26);
        icon->flags |= flags;
        icon->event_handler = (void *)FUN_0048c020;
        if (MapStats.field_3a0 != 1) {
            // STRING: LEGOLAND 0x004beef8
            icon = LoadSpriteIcon("Tutorial_On_Progress.lls", 4, 0x174, 0x16d, 0x23);
            icon->string_id = 0x258;
            icon->string = GetString(0x258);
            icon->flags |= flags;
            icon->event_handler = (void *)FUN_0048c090;
        }
    }
    FUN_0048b740();
    RemoveIconGroup(0x1c);
    if (MapStats.field_3a0 == 1) {
        flags = 0x200a;

        for (i = 0; i < 10; i++) {
            if (i + 5 == (int)lpConfig->field_28 - 1) {
                icon = InsertIcon(DAT_004beb80.levels[i].x, DAT_004beb80.levels[i].y, 0x1c, DAT_004beb80.levels[i].sprite0);
                if (icon) {
                    icon->field_28 = (void *)RenderFlashingSpriteIcon;
                    icon->string_id = DAT_004beb80.levels[i].id;
                    icon->string = GetString(icon->string_id);
                    icon->field_18 = i + 5;
                    icon->event_handler = (void *)FUN_0048bb60;
                    icon->flags |= flags;
                }
            } else if (i + 5 < (int)lpConfig->field_28 - 1) {
                icon = InsertIcon(DAT_004beb80.levels[i].x, DAT_004beb80.levels[i].y, 0x1c, DAT_004beb80.levels[i].sprite1);
                if (icon) {
                    icon->string_id = DAT_004beb80.levels[i].id;
                    icon->string = GetString(icon->string_id);
                    icon->flags |= 0x2000;
                }
            }
        }
    } else {
        for (i = 0; i < 10; i++) {
            if (i + 5 == (int)lpConfig->field_28 - 1) {
                icon = InsertIcon(DAT_004beb80.levels[i].x, DAT_004beb80.levels[i].y, 0x1c, DAT_004beb80.levels[i].sprite0);
                if (icon) {
                    icon->field_28 = (void *)RenderFlashingSpriteIcon;
                    icon->string_id = DAT_004beb80.levels[i].id;
                    icon->string = GetString(icon->string_id);
                    icon->field_18 = i + 5;
                    icon->event_handler = (void *)FUN_0048bb60;
                    icon->flags |= 0x600a;
                }
            } else if (CurrentProfile.flags[4 + i + 5] == 1) {
                icon = InsertIcon(DAT_004beb80.levels[i].x, DAT_004beb80.levels[i].y, 0x1c, DAT_004beb80.levels[i].sprite1);
                if (icon) {
                    icon->string_id = DAT_004beb80.levels[i].id;
                    icon->string = GetString(icon->string_id);
                    icon->field_18 = i + 5;
                    icon->event_handler = (void *)FUN_0048bb60;
                    icon->flags |= flags;
                }
            }
        }
    }
    DAT_006687bc = (unsigned int)FUN_0048bc20;
    DAT_006687c0 = (unsigned int)FUN_0048c020;
}

// FUNCTION: LEGOLAND 0x0048bb60
unsigned char FUN_0048bb60(unsigned char *arg0, unsigned int arg1, unsigned int arg2, unsigned int arg3) {
    DAT_00798660 = 1;
    if ((arg1 & 2) != 0) {
        if ((int)(GetTicks() - DAT_0079866c) < 500 && arg0[0x18] == DAT_004beb80.last_clicked) {
            if (DAT_00798664 != 0) {
                return FUN_0048bf90(arg0, arg1, arg2, arg3);
            }
            return FUN_0048bc20(arg0, arg1, arg2, arg3);
        }
        DAT_0079866c = GetTicks();
        DAT_004beb80.last_clicked = arg0[0x18];
        PlayInstanceOfSample(PTR_004b92c0, 0, 1, 0);
        lpConfig->field_28 = arg0[0x18] + 1;
        DAT_0080ff80.unk4 = 0xffffffff;
    }
    return 1;
}

// FUNCTION: LEGOLAND 0x0048bc20
unsigned char FUN_0048bc20(unsigned char *arg0, unsigned int arg1, unsigned int arg2, unsigned int arg3) {
    if ((arg1 & 2) != 0) {
        do {
            DAT_006687c0 = 0;
            DAT_006687bc = 0;
            SpeechCloseFile();
            DAT_006687b0 = 4;
            PlayInstanceOfSample(PTR_004b92c0, 0, 1, 0);
            FUN_0048b770();
            if ((int)lpConfig->field_28 <= 0xf) {
                LoadWatchSprite(0xfa, 0x181);
                DAT_00668e38 = 0;
                InitGameInterface(1);
                EditMode.unk4 = 3;
                FUN_00474880();
                FUN_00458a50();
                UnloadWatchSprite();
                DAT_00798660 = 0;
                DAT_00798668 = 0;
            } else {
                DAT_00798660 = 0;
                GamePad &= ~0x20;
                DAT_00798668 = 0;
                EditMode.unk4 = 2;
                DAT_0080ff80.unk4 = 0xffffffff;
                DAT_0080ff80.unk8 = 8;
                DAT_00668e38 = 1;
            }
        } while (0);
    }
    return 1;
}

// FUNCTION: LEGOLAND 0x0048bd00
void LoadTutorialSprites(void) {
    struct ProgressEntry *e;

    e = &DAT_004beb80.tutorials[0];
    do {
        e->sprite0 = LoadSprite(e->name0, 4);
        e->sprite1 = LoadSprite(e->name1, 4);
        e++;
    } while ((int)&e->sprite0 < (int)&DAT_004bed40);
}
// FUNCTION: LEGOLAND 0x0048bd40
void FUN_0048bd40(void) {
    int *esi;

    esi = (int *)&DAT_004beb80.tutorials[0].sprite1;
    do {
        ReferenceSprite((struct Sprite *)esi[-1]);
        ReferenceSprite((struct Sprite *)esi[0]);
        esi += 7;
    } while ((int)esi < (int)&DAT_004bed44);
}
// FUNCTION: LEGOLAND 0x0048bd70
void FUN_0048bd70(void) {
    struct FreePlaySpriteSlot *slot;

    RemoveIconGroup(0x1c);
    RemoveIconGroup(0x23);
    slot = (struct FreePlaySpriteSlot *)&DAT_004beb80.tutorials[0].sprite0;
    while ((int)slot < (int)&DAT_004bed40) {
        while (KillSprite(slot->field_0) == 0) {
        }
        while (KillSprite(slot->field_4) == 0) {
        }
        slot->field_4 = NULL;
        slot->field_0 = NULL;
        slot++;
    }
}

// FUNCTION: LEGOLAND 0x0048bde0
void FUN_0048bde0(void) {
    struct IconNode *icon;
    unsigned int *mapping;
    struct ProgressEntry *entry;
    int i;

    // STRING: LEGOLAND 0x004bef88
    SPRITE_TitleScreenBk = LoadSprite("TutorialBK.lls", 0);
    DAT_00798664 = 1;
    if (DAT_00798660 == 0) {
        LoadTutorialSprites();
        // STRING: LEGOLAND 0x004bef70
        icon = LoadSpriteIcon("Accept_on_Report.lls", 4, 0x20a, 0x16c, 0x23);
        icon->string_id = 0x262;
        icon->string = GetString(0x262);
        icon->flags |= 0x6002;
        icon->event_handler = (void *)FUN_0048bf90;
        DAT_006687bc = (unsigned int)FUN_0048bf90;
        // STRING: LEGOLAND 0x004bef5c
        icon = LoadSpriteIcon("GoBack_on_Tut.lls", 4, 0x20a, 0xf5, 0x23);
        icon->string_id = 0x26;
        icon->string = GetString(0x26);
        if (CurrentProfile.flags[9] == 1) {
            icon->string_id = 0x26c;
            icon->string = GetString(0x26c);
            icon->event_handler = (void *)FUN_0048c090;
        } else {
            icon->string_id = 0x26;
            icon->string = GetString(0x26);
            icon->event_handler = (void *)FUN_0048c020;
        }
        icon->flags |= 0x6002;
        DAT_006687c0 = (unsigned int)icon->event_handler;
    }
    FUN_0048bd40();
    RemoveIconGroup(0x1c);
    i = 0;
    mapping = DAT_007cb380;
    entry = &DAT_004beb80.tutorials[0];
    do {
        if (i == lpConfig->field_28 - 1) {
            icon = InsertIcon(entry->x, entry->y, 0x1c, entry->sprite0);
        } else if (CurrentProfile.flags[4 + i] == 1) {
            icon = InsertIcon(entry->x, entry->y, 0x1c, entry->sprite1);
        } else {
            icon = 0;
        }
        *mapping = (unsigned int)icon;
        if (icon) {
            icon->string = GetString(0x276);
            icon->string_id = entry->id;
            icon->field_18 = i;
            icon->event_handler = (void *)FUN_0048bb60;
            icon->flags |= 0x6002;
        }
        i++;
        mapping++;
        entry++;
    } while ((int)mapping < (int)&DAT_007cb394);
}

// FUNCTION: LEGOLAND 0x0048bf90
unsigned char FUN_0048bf90(unsigned char *arg0, unsigned int arg1, unsigned int arg2, unsigned int arg3) {
    if ((arg1 & 2) != 0) {
        DAT_006687c0 = 0;
        DAT_006687bc = 0;
        SpeechCloseFile();
        DAT_006687b0 = 4;
        PlayInstanceOfSample(PTR_004b92c0, 0, 1, 0);
        FUN_0048bd70();
        LoadWatchSprite(0x186, 0x18b);
        DAT_00668e38 = 0;
        InitGameInterface(1);
        EditMode.unk4 = 3;
        FUN_00474880();
        FUN_00458a50();
        UnloadWatchSprite();
        DAT_00798660 = 0;
        DAT_00798668 = 0;
    }
    return 1;
}

// FUNCTION: LEGOLAND 0x0048c020
unsigned char FUN_0048c020(unsigned int a1, unsigned int a2, unsigned int a3, unsigned int a4) {
    unsigned int temp;

    if ((a2 & 2) != 0) {
        DAT_006687c0 = 0;
        DAT_006687bc = 0;
        PlayInstanceOfSample(PTR_004b92c0, 0, 1, 0);
        temp = DAT_00798664;
        DAT_00798660 = 0;
        DAT_00798668 = 0;
        if (temp != 0) {
            FUN_0048bd70();
        } else {
            FUN_0048b770();
        }
        return FUN_0048fb80(a1, a2, a3, a4);
    }
    return 1;
}

// FUNCTION: LEGOLAND 0x0048c090
unsigned char FUN_0048c090(void *param1, unsigned char param2) {
    if (param2 & 2) {
        DAT_00798660 = 0;
        DAT_00798668 = 1;
        DAT_0080ff80.unk4 = 0xffffffff;
        PlayInstanceOfSample(PTR_004b92c0, 0, 1, 0);
        if (DAT_00798664 != 0) {
            FUN_0048bd70();
            lpConfig->field_28 = 6;
        } else {
            FUN_0048b770();
            lpConfig->field_28 = 1;
        }
    }
    return 1;
}

// FUNCTION: LEGOLAND 0x0048c100
void FUN_0048c100(void) {
    RECT rc;
    unsigned int *mapping;
    int *entry;
    int i;
    char *text;

    if (DAT_00798664 != 0) {
        rc.top = 0x45;
        rc.bottom = 0x6c;
        rc.left = 10;
        rc.right = 0x1d6;
        NewPrintCent(GetString(0x28a), 3, rc, 0);
        i = 0;
        mapping = DAT_007cb380;
        entry = &DAT_004beb80.tutorials[0].x;
        do {
            text = GetString(entry[-1]);
            rc.left = entry[0] + 0x32;
            rc.top = entry[1] + 6;
            rc.right = rc.left + 0x190;
            rc.bottom = rc.top + 0x16;
            if (i == (int)lpConfig->field_28 - 1) {
                DrawTextOnRenderSurface(text, 2, rc, 0);
            } else if (CurrentProfile.flags[4 + i] == 1) {
                DrawTextOnRenderSurface(text, 2, rc, 0x323232);
            } else {
                DrawTextOnRenderSurface(text, 2, rc, 0xa0a0a0);
            }
            if (CurrentProfile.flags[4 + i] == 1 && DAT_00813a44.x >= rc.left && DAT_00813a44.x < rc.right && DAT_00813a44.y >= rc.top && DAT_00813a44.y < rc.bottom) {
                Hover.type = 2;
                Hover.ptr = (struct Bloke *)*mapping;
            }
            i++;
            entry += 7;
            mapping++;
        } while ((int)entry < (int)&DAT_004beb80.tail[12]);
    }
}
