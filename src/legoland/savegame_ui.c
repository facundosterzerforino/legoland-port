#include <direct.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "globals.h"
#include "legoland.h"

#include "debug_alloc.h"
#include "draw.h"
#include "icon.h"
#include "input.h"
#include "interface.h"
#include "main.h"
#include "math.h"
#include "options.h"
#include "print_sprite.h"
#include "profile.h"
#include "profile_io.h"
#include "savegame_ui.h"
#include "saveload.h"
#include "screens.h"
#include "sound_music.h"
#include "string.h"
#include "timer.h"
#include "title.h"

#include "image_sprite.h"
#include "stream.h"

#pragma intrinsic(memset, strcpy)

struct SaveNode {
    struct SaveNode *next;
    struct ProfileData data;
    int has_header;
    unsigned char slot;
};

struct SaveScreenIcon {
    unsigned char pad_0[0x18];
    void *field_18;
    unsigned char slot;
    unsigned char pad_1d[0x20 - 0x1d];
    unsigned char field_20;
    unsigned char pad_21[0x2c - 0x21];
    void *event_handler;
    unsigned char pad_30[0x34 - 0x30];
    unsigned int flags;
    unsigned int string;
    unsigned int string_id;
};

struct SaveScreenNode {
    struct SaveScreenNode *next;
    char name[0x24];
    unsigned char field_28;
    unsigned char pad_29[0x114 - 0x29];
    int has_header;
    unsigned char slot;
};

// FUNCTION: LEGOLAND 0x0048d4b0
LEGO_EXPORT void InitSavedGameScreen(void) {
    struct SaveScreenIcon *icon;
    struct SaveScreenNode *node;

    // STRING: LEGOLAND 0x004bf2a0
    SPRITE_TitleScreenBk = LoadSprite("Saved_Game_Screen.lls", 0);
    CurrentProfile.save_slot = 0;
    // STRING: LEGOLAND 0x004bf28c
    RegSaveSlotOnSprite = LoadSprite("RegSaveSlotOn.lls", 4);
    // STRING: LEGOLAND 0x004bf278
    RegSaveOff1Sprite = LoadSprite("RegSaveOff_1.lls", 4);
    // STRING: LEGOLAND 0x004bf264
    RegSaveOff2Sprite = LoadSprite("RegSaveOff_2.lls", 4);
    // STRING: LEGOLAND 0x004bf250
    RegSaveOff3Sprite = LoadSprite("RegSaveOff_3.lls", 4);
    // STRING: LEGOLAND 0x004bf23c
    RegSaveOff4Sprite = LoadSprite("RegSaveOff_4.lls", 4);
    // STRING: LEGOLAND 0x004bf228
    RegSaveOff5Sprite = LoadSprite("RegSaveOff_5.lls", 4);
    // STRING: LEGOLAND 0x004bf214
    RegSaveOff6Sprite = LoadSprite("RegSaveOff_6.lls", 4);
    // STRING: LEGOLAND 0x004bf200
    RegSaveOff7Sprite = LoadSprite("RegSaveOff_7.lls", 4);
    // STRING: LEGOLAND 0x004bf1ec
    RegSaveOff8Sprite = LoadSprite("RegSaveOff_8.lls", 4);
    // STRING: LEGOLAND 0x004bf1d8
    SaveTypeNormalSprite = LoadSprite("SaveType_Normal.lls", 4);
    // STRING: LEGOLAND 0x004bf1c4
    SaveTypeFreeSprite = LoadSprite("SaveType_Free.lls", 4);
    // STRING: LEGOLAND 0x004bf1b4
    RegDeleteOnSprite = LoadSprite("RegDeleteON.lls", 4);
    // STRING: LEGOLAND 0x004bf104
    RegDeleteSprite = LoadSprite("RegDelete.lls", 4);
    // STRING: LEGOLAND 0x004bf19c
    DAT_007986b8 = LoadSprite("RegSaveDouble_PopUp.lls", 4);
    // STRING: LEGOLAND 0x004bf188
    RegCornerMaskSprite = LoadSprite("RegCornerMask.lls", 4);

    // STRING: LEGOLAND 0x004bf170
    icon = (struct SaveScreenIcon *)LoadSpriteIcon("GoBack_on_SavedGame.lls", 4, 0x1c2, 0x13, 7);
    icon->flags |= 0x6002;
    if (DAT_007cb324) {
        icon->event_handler = (void *)FUN_0048fb80;
        icon->string_id = 0x26;
        icon->string = GetString(0x26);
    } else {
        icon->event_handler = (void *)FUN_0048db10;
        icon->string_id = 0x28;
        icon->string = GetString(0x28);
    }
    DAT_006687c0 = (unsigned int)icon->event_handler;

    if (!LoadMode) {
        // STRING: LEGOLAND 0x004bf15c
        AcceptIcon = (unsigned int)LoadSpriteIcon("Accept_On_Save.lls", 4, 0x1dc, 0x142, 7);
        ((struct SaveScreenIcon *)AcceptIcon)->flags |= 0x2000;
        ((struct SaveScreenIcon *)AcceptIcon)->flags |= 0x4002;
        ((struct SaveScreenIcon *)AcceptIcon)->flags |= 0x400;
        if (DAT_007cb310) {
            ((struct SaveScreenIcon *)AcceptIcon)->event_handler = (void *)FUN_0048f5a0;
            ((struct SaveScreenIcon *)AcceptIcon)->string_id = 0x29;
            ((struct SaveScreenIcon *)AcceptIcon)->string = GetString(0x29);
        } else {
            ((struct SaveScreenIcon *)AcceptIcon)->event_handler = (void *)FUN_0048da50;
            ((struct SaveScreenIcon *)AcceptIcon)->string_id = 0x2a;
            ((struct SaveScreenIcon *)AcceptIcon)->string = GetString(0x2a);
        }
    } else {
        AcceptIcon = (unsigned int)LoadSpriteIcon("Accept_On_Save.lls", 4, 0x1dc, 0x142, 7);
        ((struct SaveScreenIcon *)AcceptIcon)->string_id = 0x2b;
        ((struct SaveScreenIcon *)AcceptIcon)->string = GetString(0x2b);
        ((struct SaveScreenIcon *)AcceptIcon)->flags |= 0x2000;
        ((struct SaveScreenIcon *)AcceptIcon)->flags |= 0x4002;
        ((struct SaveScreenIcon *)AcceptIcon)->flags |= 0x400;
        ((struct SaveScreenIcon *)AcceptIcon)->event_handler = (void *)FUN_0048d970;
    }
    DAT_006687bc = (unsigned int)((struct SaveScreenIcon *)AcceptIcon)->event_handler;

    FUN_0048d470();
    DeleteSavedGameList();
    LoadSavedGamesList(CurrentProfile.profile_slot);

    node = (struct SaveScreenNode *)SavedGameList;
    if (node) {
        do {
            if (node->has_header) {
                icon = (struct SaveScreenIcon *)InsertIcon(0x51, (short)(node->slot * 38 + 0x6f), 7, GetSavePanelBK(node->slot));
                icon->event_handler = (void *)FUN_0048e4a0;
                icon->flags |= 0x4002;
                icon->field_18 = node->name;
                icon->slot = node->slot;
                if (node->field_28 == 1) {
                    icon->field_20 |= 3;
                    icon->string_id = 0x2c;
                    icon->string = GetString(0x2c);
                } else {
                    icon->field_20 |= 5;
                    icon->string_id = 0x2d;
                    icon->string = GetString(0x2d);
                }
            } else {
                icon = (struct SaveScreenIcon *)InsertIcon(0x51, (short)(node->slot * 38 + 0x6f), 7, GetSavePanelBK(node->slot));
                icon->string_id = 0x2e;
                icon->string = GetString(0x2e);
                icon->flags |= 0x6002;
                icon->event_handler = (void *)FUN_0048e4f0;
                // STRING: LEGOLAND 0x004befa0
                icon->field_18 = "EMPTY";
                icon->slot = node->slot;
                icon->field_20 |= 1;
            }
            node = node->next;
        } while (node);
    }

    DeleteIcon = InsertIcon(0, 0, 7, RegDeleteSprite);
    DeleteIcon->string_id = 5;
    DeleteIcon->string = GetString(5);
    DeleteIcon->flags |= 0x2000;
    DeleteIcon->flags |= 0x4002;
    DeleteIcon->flags |= 0x400;
    DeleteIcon->event_handler = (void *)FUN_0048cc30;
}

// FUNCTION: LEGOLAND 0x0048d8f0
LEGO_EXPORT int LoadDateIntoTempProfile(int a1, int a2) {
    char buffer[132];
    void *file;

    // STRING: LEGOLAND 0x004bf2bc
    sprintf(buffer, "profiles\\%dsave%d.sh", a1, a2);

    if (!Goto_ProfileDir()) {
        return 0;
    }

    // STRING: LEGOLAND 0x004bf2b8
    file = fopen(buffer, "r");
    if (file != 0) {
        fread(&TempProfile, sizeof(struct ProfileData), 1, file);
        fclose(file);
        ReturnFrom_ProfileDir();
        return 1;
    }

    return 0;
}

// FUNCTION: LEGOLAND 0x0048d970
unsigned char FUN_0048d970(unsigned int a1, unsigned char flags) {
    if (DAT_004bef9c != 0 && (flags & 2) && CurrentProfile.save_slot != 0) {
        SpeechCloseFile();
        DAT_006687b0 = 4;
        PlayInstanceOfSample(GameFX[FX_BUTTON4].sample, 0, 1, 0);
        if (LoadDateIntoTempProfile(CurrentProfile.profile_slot, CurrentProfile.save_slot & 0xff) != 0) {
            CurrentProfile.field_45 = TempProfile.field_24;
            CurrentProfile.field_20 = TempProfile.field_20;
            CurrentProfile.speech_volume = TempProfile.speech_volume;
            CurrentProfile.music_volume = TempProfile.music_volume;
            CurrentProfile.fx_volume = TempProfile.fx_volume;
            DAT_00667c64 = 1;
            DAT_00667c80 = 1;
        }
        RemoveIconGroup(7);
        KillSaveScreenSprites();
        KillTitleScreenSprites();
        DeleteSavedGameList();
        if (DAT_007cb324 == 0) {
            UnloadMap();
        }
    }
    return 1;
}

// FUNCTION: LEGOLAND 0x0048da50
unsigned char FUN_0048da50(unsigned int a1, unsigned int flags, unsigned int a3, unsigned int a4) {
    if (DAT_004bef9c == 0) {
        unsigned char r = FUN_0048e720(0, flags, a3, a4);
        if (DAT_0080ff80.unk4 != 0xffffffff) {
            return r;
        }
        DAT_00668e38 = 0;
        RemoveIconGroup(7);
    } else {
        if (!(flags & 2) || CurrentProfile.save_slot == 0) {
            return 1;
        }
        PlayInstanceOfSample(GameFX[FX_BUTTON4].sample, 0, 1, 0);
        SpeechCloseFile();
        DAT_006687b0 = 4;
        StoreNewSaveGameToDisk();
        DAT_00668e38 = 0;
        RemoveIconGroup(7);
    }
    KillTitleScreenSprites();
    EditMode.unk4 = 3;
    FUN_00474880();
    ResumeGameTimer();
    DAT_004bef9c = 1;
    return 1;
}

// FUNCTION: LEGOLAND 0x0048db10
unsigned char FUN_0048db10(int param1, unsigned char flags) {
    if (DAT_004bef9c != 0 && (flags & 0x2)) {
        PlayInstanceOfSample(GameFX[FX_BUTTON4].sample, 0, 1, 0);
        DAT_0080ff80.unk8 = 5;
    }
    return 1;
}

// FUNCTION: LEGOLAND 0x0048db50
LEGO_EXPORT struct Sprite *GetSavePanelBK(signed char slot) {
    switch (slot) {
    case 1:
        return RegSaveOff1Sprite;
    case 2:
        return RegSaveOff2Sprite;
    case 3:
        return RegSaveOff3Sprite;
    case 4:
        return RegSaveOff4Sprite;
    case 5:
        return RegSaveOff5Sprite;
    case 6:
        return RegSaveOff6Sprite;
    case 7:
        return RegSaveOff7Sprite;
    case 8:
        return RegSaveOff8Sprite;
    default:
        return 0;
    }
}

// FUNCTION: LEGOLAND 0x0048dbc0
LEGO_EXPORT void KillSaveScreenSprites(void) {
    if (RegSaveSlotOnSprite != 0) {
        KillSprite(RegSaveSlotOnSprite);
        RegSaveSlotOnSprite = 0;
    }
    if (RegSaveOff1Sprite != 0) {
        KillSprite(RegSaveOff1Sprite);
        RegSaveOff1Sprite = 0;
    }
    if (RegSaveOff2Sprite != 0) {
        KillSprite(RegSaveOff2Sprite);
        RegSaveOff2Sprite = 0;
    }
    if (RegSaveOff3Sprite != 0) {
        KillSprite(RegSaveOff3Sprite);
        RegSaveOff3Sprite = 0;
    }
    if (RegSaveOff4Sprite != 0) {
        KillSprite(RegSaveOff4Sprite);
        RegSaveOff4Sprite = 0;
    }
    if (RegSaveOff5Sprite != 0) {
        KillSprite(RegSaveOff5Sprite);
        RegSaveOff5Sprite = 0;
    }
    if (RegSaveOff6Sprite != 0) {
        KillSprite(RegSaveOff6Sprite);
        RegSaveOff6Sprite = 0;
    }
    if (RegSaveOff7Sprite != 0) {
        KillSprite(RegSaveOff7Sprite);
        RegSaveOff7Sprite = 0;
    }
    if (RegSaveOff8Sprite != 0) {
        KillSprite(RegSaveOff8Sprite);
        RegSaveOff8Sprite = 0;
    }
    if (SaveTypeNormalSprite != 0) {
        KillSprite(SaveTypeNormalSprite);
        SaveTypeNormalSprite = 0;
    }
    if (SaveTypeFreeSprite != 0) {
        KillSprite(SaveTypeFreeSprite);
        SaveTypeFreeSprite = 0;
    }
    if (RegCornerMaskSprite != 0) {
        KillSprite(RegCornerMaskSprite);
        RegCornerMaskSprite = 0;
    }
    if (RegDeleteOnSprite != 0) {
        KillSprite(RegDeleteOnSprite);
        RegDeleteOnSprite = 0;
    }
}

// FUNCTION: LEGOLAND 0x0048dd00
LEGO_EXPORT void PrintSavedGameDetails(void) {
    struct IconNode *node = IconListHead;
    struct IconNode *selected;
    struct IconNode *btn;
    int by;
    int draw;
    RECT rc;

    if (DeleteIcon != NULL) {
        DeleteIcon->flags |= 0x400;
    }
    rc.left = 0x5c;
    rc.top = 0x24;
    rc.right = 0x14e;
    rc.bottom = 0x56;
    NewPrintCent(GetString(LoadMode != 0 ? 0x4b0 : 0x4ba), 0, rc, 1);
    rc.top = 0x38;
    rc.bottom = 0x6a;
    NewPrintCent((char *)&CurrentProfile, 1, rc, 1);

    while (node != NULL) {
        draw = 1;
        if ((node->flags & 0x1400) == 0 && (node->field_20b & 1) != 0) {
            for (;;) {
                if (CurrentProfile.save_slot == node->field_1cb) {
                    selected = node;
                    if (DeletePopUpShown != 0) {
                        SetIconSprite(node, DAT_007986b8);
                        node->y = (short)(node->field_1cb * 0x26 + 0x54);
                        by = node->y + 0x1b;
                        rc.top = node->y + 0x22;
                        LightUpthisDeleteIcon(node, 0);
                    } else if (DAT_00798700 != 0) {
                        SetIconSprite(node, DAT_007986b8);
                        node->y = (short)(node->field_1cb * 0x26 + 0x54);
                        by = node->y;
                        rc.top = by + 0x22;
                        EnterSaveGameDetails((struct EditSprite *)node);
                        draw = 0;
                        if (CurrentProfile.field_45 == 1) {
                            PrintSprite(SaveTypeNormalSprite, node->x + 7, by + 0x21, 0, 0);
                        }
                        if (CurrentProfile.field_45 == 2) {
                            PrintSprite(SaveTypeFreeSprite, node->x + 7, by + 0x21, 0, 0);
                        }
                        break;
                    } else {
                        SetIconSprite(node, RegSaveSlotOnSprite);
                        node->y = (short)(node->field_1cb * 0x26 + 0x6f);
                        by = node->y;
                        rc.top = by + 7;
                        if (LoadMode != 0) {
                            LightUpthisDeleteIcon(node, 0);
                        }
                    }
                } else {
                    SetIconSprite(node, GetSavePanelBK(node->field_1cb));
                    by = node->y;
                    rc.top = by + 7;
                }
                if ((node->field_20b & 2) != 0) {
                    PrintSprite(SaveTypeNormalSprite, node->x + 7, by + 6, 0, 0);
                }
                if ((node->field_20b & 4) != 0) {
                    PrintSprite(SaveTypeFreeSprite, node->x + 7, by + 6, 0, 0);
                }
                break;
            }
            rc.left = node->x + 0x28;
            rc.right = rc.left + 0xd7;
            rc.bottom = rc.top + 0x13;
            if (node->field_18p != NULL && draw != 0) {
                if (CurrentProfile.save_slot - 1 != node->field_1cb || (DeletePopUpShown == 0 && DAT_00798700 == 0)) {
                    NewPrintCent((char *)node->field_18p, 2, rc, 1);
                }
            }
        }
        node = node->next;
    }

    if (DAT_00798700 != 0) {
        rc.left = selected->x + 8;
        rc.top = selected->y + 7;
        rc.right = rc.left + 0xae;
        rc.bottom = rc.top + 0x13;
        PrintSprite(RegCornerMaskSprite, selected->x, selected->y, 0, 0);
        if (DAT_007986f0 != 0) {
            NewPrintCent(GetString(0xc44), 2, rc, 1);
        } else {
            NewPrintCent(GetString(0x8c), 2, rc, 1);
        }
        UpdateProfileCheckBoxIcons();
    } else if (DeletePopUpShown != 0) {
        rc.left = selected->x + 0x14;
        rc.top = selected->y + 7;
        rc.right = rc.left + 0x9b;
        rc.bottom = rc.top + 0x13;
        PrintSprite(RegCornerMaskSprite, selected->x, selected->y, 0, 0);
        NewPrintCent(GetString(0x85), 2, rc, 1);
        UpdateProfileCheckBoxIcons();
    }

    btn = (struct IconNode *)AcceptIcon;
    if (btn != NULL) {
        if (CurrentProfile.save_slot != 0) {
            btn->flags &= ~0x400;
        } else {
            btn->flags |= 0x400;
        }
    }
}

// FUNCTION: LEGOLAND 0x0048e0c0
void AddSavedGameNode(int has_header, struct ProfileData *header, unsigned char slot) {
    struct SaveNode *node = (struct SaveNode *)malloc(sizeof(struct SaveNode));
    memset(node, 0, sizeof(struct SaveNode));
    if (has_header != 0) {
        node->has_header = 1;
        strcpy(node->data.name, header->name);
        node->slot = slot;
        node->data.field_24 = header->field_24;
        node->next = (struct SaveNode *)SavedGameList;
        SavedGameList = node;
        return;
    }
    node->has_header = 0;
    node->slot = slot;
    node->next = (struct SaveNode *)SavedGameList;
    SavedGameList = node;
}

// FUNCTION: LEGOLAND 0x0048e160
LEGO_EXPORT void DeleteSavedGameList(void) {
    struct SaveNode *current = (struct SaveNode *)SavedGameList;
    while (current != NULL) {
        struct SaveNode *next = current->next;
        free(current);
        current = next;
    }
    SavedGameList = NULL;
}

// FUNCTION: LEGOLAND 0x0048e190
LEGO_EXPORT char LoadSavedGamesList(unsigned char profile) {
    char path[120];
    struct ProfileData header;
    char slot;
    int rc;
    void *file;

    if (!Goto_ProfileDir()) {
        return -1;
    }

    slot = 8;
    do {
        sprintf(path, "profiles\\%dsave%d.sh", profile, slot);
        file = fopen(path, "r");
        if (file == 0) {
            AddSavedGameNode(0, &header, slot);
        } else {
            fread(&header, sizeof(struct ProfileData), 1, file);
            AddSavedGameNode(1, &header, slot);
            fclose(file);
        }
        memset(&header, 0, sizeof(struct ProfileData));
        slot = slot - 1;
    } while (slot != 0);

    rc = ReturnFrom_ProfileDir();
    if (rc) {
        return 0;
    }
    return -1;
}

// FUNCTION: LEGOLAND 0x0048e280
LEGO_EXPORT void InitNewSaveGamePOPUP(struct IconNode *icon) {
    // STRING: LEGOLAND 0x004baa70
    PuOkSprite = LoadSprite("PU_OK.lls", 4);
    // STRING: LEGOLAND 0x004baa64
    PuOkOnSprite = LoadSprite("PU_OKON.lls", 4);
    // STRING: LEGOLAND 0x004bf148
    PopUpCloseSprite = LoadSprite("RegClose.lls", 4);
    // STRING: LEGOLAND 0x004bf2d4
    PuCloseOnSprite = LoadSprite("RegCloseOn.lls", 4);

    PopUpOkIcon = InsertIcon(icon->x + 0xbd, icon->y - 0x18, 7, PuOkSprite);
    PopUpOkIcon->string_id = 0x2f;
    PopUpOkIcon->string = GetString(0x2f);
    PopUpOkIcon->flags |= 0x2000;
    PopUpOkIcon->flags |= 0x4002;
    PopUpOkIcon->event_handler = (void *)FUN_0048e720;
    DAT_006687bc = (unsigned int)PopUpOkIcon->event_handler;

    PopUpCloseIcon = InsertIcon(icon->x + 0xe1, icon->y - 0x18, 7, PopUpCloseSprite);
    PopUpCloseIcon->string_id = 4;
    PopUpCloseIcon->string = GetString(4);
    PopUpCloseIcon->flags |= 0x2000;
    PopUpCloseIcon->flags |= 0x4002;
    PopUpCloseIcon->event_handler = (void *)FUN_0048e810;
    DAT_006687c0 = (unsigned int)PopUpCloseIcon->event_handler;
    DAT_007986f0 = 0;
}

// FUNCTION: LEGOLAND 0x0048e3d0
void FUN_0048e3d0(const char *name) {
    strcpy(TempProfile.name, name);
    TempProfile.name_len = (unsigned char)strlen(name);
    DAT_007986f0 = 1;
}

// FUNCTION: LEGOLAND 0x0048e420
void ResetPopUpIconSprites(void) {
    SetIconSprite((struct IconNode *)PopUpOkIcon, PuOkSprite);
    SetIconSprite((struct IconNode *)PopUpCloseIcon, PopUpCloseSprite);
}

// FUNCTION: LEGOLAND 0x0048e450
unsigned char FUN_0048e450(unsigned int a1, unsigned int a2) {
    if ((a2 & 2) != 0 && CurrentProfile.save_slot != 0) {
        CloseFontEndCheckBox();
        DeletePopUpShown = 0;
        RemoveSaveGame(CurrentProfile.save_slot);
        DAT_0080ff80.unk4 = 0xffffffffu;
        CurrentProfile.save_slot = 0;
    }
    return 1;
}

struct SaveIcon {
    unsigned char pad_0[0x18];
    const char *field_18;
    unsigned char field_1c;
};

// FUNCTION: LEGOLAND 0x0048e4a0
unsigned char FUN_0048e4a0(struct SaveIcon *icon, unsigned int flags, unsigned int a3, unsigned int a4) {
    if (DAT_004bef9c != 0 && (flags & 2)) {
        if (LoadMode != 0) {
            CurrentProfile.save_slot = icon->field_1c;
        } else {
            return FUN_0048e4f0(icon, flags, a3, a4);
        }
    }
    return 1;
}

// FUNCTION: LEGOLAND 0x0048e4f0
unsigned char FUN_0048e4f0(struct SaveIcon *icon, unsigned int a2, unsigned int a3, unsigned int a4) {
    if (DAT_004bef9c != 0) {
        if ((a2 & 2) == 0 || LoadMode != 0) {
            return 1;
        }
        ResetTempProfile();
        CurrentProfile.save_slot = icon->field_1c;
        InitNewSaveGamePOPUP(icon);
        if (icon->field_18 != "EMPTY") {
            FUN_0048e3d0(icon->field_18);
        }
        DAT_00798700 = 1;
        DAT_004bef9c = 0;
    }
    return 1;
}

struct EditSprite {
    unsigned char pad_0[0xc];
    short field_c;
    short field_e;
};

// FUNCTION: LEGOLAND 0x0048e550
LEGO_EXPORT void EnterSaveGameDetails(struct EditSprite *sprite) {
    char cursor_str[2];
    unsigned char count;
    char input;
    int field_e;
    char *name;
    int sprite_y;
    char zero = 0;
    int left;
    register int right;
    unsigned int bottom;
    int center_x;
    int top;
    char *blink;
    struct EditSprite *focus;
    int fx;
    int fy;

    *(short *)cursor_str = *(short *)"|";
    count = TempProfile.name_len;
    input = GetInputChar();
    if (input != zero) {
        if (input == -1 && zero != count) {
            count -= 1;
            TempProfile.name[count] = zero;
        }
        if (count < 0x1f) {
            if (0xcb > DAT_00798738) {
                if (input > '\0') {
                    int index = count;
                    count++;
                    TempProfile.name[index] = input;
                    TempProfile.name[count] = zero;
                } else if (input == ' ') {
                    if (count != zero) {
                        int index = count;
                        count++;
                        TempProfile.name[index] = ' ';
                        TempProfile.name[count] = zero;
                    }
                }
            }
        }
    }
    top = 0x24 + sprite->field_e;
    left = 0x28 + sprite->field_c;
    bottom = top + 0x11;
    right = left + 0xd7;
    if (count != zero) {
        RECT rc;
        rc.left = left;
        rc.top = top;
        rc.right = right;
        rc.bottom = bottom;
        name = TempProfile.name;
        center_x = FUN_00491e40(name, 2, rc, 1);
    } else {
        center_x = (right + left) >> 1;
    }
    sprite_y = sprite->field_e;
    field_e = sprite_y;
    DAT_00798738 = (center_x - ((right + left) >> 1)) * 2;
    top = field_e + 0x20;
    blink = "|";
    if (GetBlink() == 0) {
        blink = " ";
    }
    strcpy(cursor_str, blink);
    {
        register RECT rc;
        rc.left = center_x;
        rc.top = top;
        rc.right = center_x + 100;
        rc.bottom = bottom;
        FUN_00490fa0(cursor_str, 2, rc, 1);
    }
    TempProfile.name_len = count;
    focus = (struct EditSprite *)PopUpOkIcon;
    fy = focus->field_e;
    fx = focus->field_c;
    if (fx + 0x48 < (int)MousePos.x || (int)MousePos.x < fx) {
        ResetPopUpIconSprites();
    }
    if (fy + 0x1b < (int)MousePos.y || (int)MousePos.y < fy) {
        ResetPopUpIconSprites();
    }
}

// FUNCTION: LEGOLAND 0x0048e720
unsigned char FUN_0048e720(struct IconNode *icon, unsigned int a2, unsigned int a3, unsigned int a4) {
    char buf[16];
    if (icon == 0) {
        icon = PopUpOkIcon;
    }
    ResetPopUpIconSprites();
    SetIconSprite(icon, PuOkOnSprite);
    if (a2 & 2) {
        if (TempProfile.name[0] == 0) {
            // STRING: LEGOLAND 0x004bf2e8
            sprintf(buf, "%s%d", GetString(0x87), *(unsigned int *)&CurrentProfile.save_slot & 0xff);
            strcpy(TempProfile.name, buf);
            TempProfile.name_len = (unsigned char)strlen(buf);
            FUN_0048d490();
            return 1;
        }
        SpeechCloseFile();
        DAT_006687b0 = 4;
        StoreNewSaveGameToDisk();
        DAT_0080ff80.unk4 = 0xffffffff;
        DAT_00798700 = 0;
        DAT_004bef9c = 1;
        FUN_0048d490();
    }
    return 1;
}

// FUNCTION: LEGOLAND 0x0048e810
unsigned char FUN_0048e810(struct IconNode *icon, unsigned char flags) {
    ResetPopUpIconSprites();
    if (icon == 0) {
        icon = (struct IconNode *)PopUpCloseIcon;
    }
    SetIconSprite(icon, PuCloseOnSprite);
    if (flags & 2) {
        CurrentProfile.save_slot = 0;
        RemoveIconGroup(7);
        InitSavedGameScreen();
        DAT_00798700 = 0;
        DAT_004bef9c = 1;
        FUN_0048d490();
    }
    return 1;
}

// FUNCTION: LEGOLAND 0x0048e870
LEGO_EXPORT unsigned char StoreNewSaveGameToDisk(void) {
    char header_path[200];
    char save_path[256];
    void *file;

    sprintf(save_path, "%s\\%dsave%d.sav", "profiles", CurrentProfile.profile_slot, *(unsigned int *)&CurrentProfile.save_slot & 0xff);
    LoadWatchSprite(0, 0);
    MarkGameTimer();
    if (SaveGame(save_path) == 0) {
        // STRING: LEGOLAND 0x004bf360
        LogPrintf("Failed to save game %s", save_path);
        CurrentProfile.save_slot = 0;
        remove(save_path);
        UnloadWatchSprite();
        return 0xff;
    }
    UnloadWatchSprite();

    TempProfile.field_24 = CurrentProfile.field_45;
    TempProfile.field_20 = CurrentProfile.field_20;
    TempProfile.speech_volume = CurrentProfile.speech_volume;
    TempProfile.music_volume = CurrentProfile.music_volume;
    TempProfile.fx_volume = CurrentProfile.fx_volume;

    sprintf(header_path, "profiles\\%dsave%d.sh", CurrentProfile.profile_slot, *(unsigned int *)&CurrentProfile.save_slot & 0xff);
    if (!Goto_ProfileDir()) {
        // STRING: LEGOLAND 0x004bf33c
        LogPrintf("Failed to move to profile folder");
        return 0xff;
    }

    // STRING: LEGOLAND 0x004beb70
    file = fopen(header_path, "w+");
    if (file == 0) {
        // STRING: LEGOLAND 0x004bf320
        LogPrintf("\ncannot open output file %s", header_path);
    } else {
        if (fwrite(&TempProfile, sizeof(struct ProfileData), 1, file) == 0) {
            // STRING: LEGOLAND 0x004bf2fc
            LogPrintf("Failed to write to save header %s", header_path);
        }
        fclose(file);
    }

    if (!ReturnFrom_ProfileDir()) {
        return 0xff;
    }
    // STRING: LEGOLAND 0x004bf2f0
    LogPrintf("Saved OK %s", header_path);
    return 1;
}

// FUNCTION: LEGOLAND 0x0048ea10
LEGO_EXPORT void RemoveSaveGame(unsigned char slot) {
    char path[132];

    if (!Goto_ProfileDir()) {
        return;
    }

    // STRING: LEGOLAND 0x004b9164
    sprintf(path, "%s\\%dsave%d.sav", "profiles", CurrentProfile.profile_slot, slot);
    if (remove(path) != 0) {
        // STRING: LEGOLAND 0x004bf3b0
        DBPrintf("Failed to delete saved game %s\n", path);
    }

    // STRING: LEGOLAND 0x004bf3a0
    sprintf(path, "%s\\%dsave%d.sh", "profiles", CurrentProfile.profile_slot, slot);
    if (remove(path) != 0) {
        // STRING: LEGOLAND 0x004bf378
        DBPrintf("Failed to delete saved game Header %s\n", path);
    }

    ReturnFrom_ProfileDir();
}

// FUNCTION: LEGOLAND 0x0048eac0
int FUN_0048eac0(int param) {
    return (100 * (param - 124)) / 241;
}

// FUNCTION: LEGOLAND 0x0048eaf0
int FUN_0048eaf0(int param) {
    return (241 * param) / 100 + 124;
}

// FUNCTION: LEGOLAND 0x0048eb20
void FUN_0048eb20(void) {
    DAT_00798740 = DAT_006687bc;
    DAT_0079873c = DAT_006687c0;
}
