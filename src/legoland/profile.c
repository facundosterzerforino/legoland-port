#include "globals.h"
#include "legoland.h"

#include "clipping.h"
#include "icon.h"
#include "profile.h"
#include "profile_io.h"
#include "savegame_ui.h"
#include "sound_music.h"
#include "sound_sfx.h"
#include "string.h"
#include "text.h"
#include "title.h"

struct ProfileFlags {
    unsigned char pad_0[0x34];
    unsigned int var_34;
};

struct Profile {
    unsigned char pad_0[0x1c];
    unsigned char var_1c;
};

#include "image_sprite.h"
#include "stream.h"

// FUNCTION: LEGOLAND 0x0048c260
LEGO_EXPORT void InitListProfiles(void) {
    struct ProfileNode *node;
    struct IconNode *icon;
    char *str;

    UpdateSoundVols();
    DeleteProfileList();
    LoadProfilesFormDisk();
    node = (struct ProfileNode *)ProfileListHead;
    // STRING: LEGOLAND 0x004bf124
    SPRITE_TitleScreenBk = LoadSprite("Reg_ScreenBK.lls", 0);
    // STRING: LEGOLAND 0x004bf114
    RegDeleteOnSprite = LoadSprite("RegDeleteOn.lls", 4);
    // STRING: LEGOLAND 0x004bf104
    RegDeleteSprite = LoadSprite("RegDelete.lls", 4);
    // STRING: LEGOLAND 0x004bf0f0
    RegProfileOnSprite = LoadSprite("RegProfileON.lls", 4);
    // STRING: LEGOLAND 0x004bf0dc
    RegProfileOff1Sprite = LoadSprite("RegProfileOff_1.lls", 4);
    // STRING: LEGOLAND 0x004bf0c8
    RegProfileOff2Sprite = LoadSprite("RegProfileOff_2.lls", 4);
    // STRING: LEGOLAND 0x004bf0b4
    RegProfileOff3Sprite = LoadSprite("RegProfileOff_3.lls", 4);
    // STRING: LEGOLAND 0x004bf0a0
    RegProfileOff4Sprite = LoadSprite("RegProfileOff_4.lls", 4);
    // STRING: LEGOLAND 0x004bf08c
    RegProfileOff5Sprite = LoadSprite("RegProfileOff_5.lls", 4);
    // STRING: LEGOLAND 0x004bf078
    RegProfileOff6Sprite = LoadSprite("RegProfileOff_6.lls", 4);
    // STRING: LEGOLAND 0x004bf064
    RegProfileOff7Sprite = LoadSprite("RegProfileOff_7.lls", 4);
    // STRING: LEGOLAND 0x004bf050
    RegProfileOff8Sprite = LoadSprite("RegProfileOff_8.lls", 4);
    // STRING: LEGOLAND 0x004bf038
    DAT_007986b8 = LoadSprite("Reg_Delete_PopUp.lls", 4);
    // STRING: LEGOLAND 0x004bf024
    RegDiffPopUpSprite = LoadSprite("Reg_Diff_PopUp.lls", 4);
    // STRING: LEGOLAND 0x004bf014
    RegEasyOnSprite = LoadSprite("Reg_Easy_On.lls", 4);
    // STRING: LEGOLAND 0x004bf000
    RegEasyOffSprite = LoadSprite("Reg_Easy_Off.lls", 4);
    // STRING: LEGOLAND 0x004beff0
    RegMidOnSprite = LoadSprite("Reg_Mid_On.lls", 4);
    // STRING: LEGOLAND 0x004befe0
    RegMidOffSprite = LoadSprite("Reg_Mid_Off.lls", 4);
    // STRING: LEGOLAND 0x004befd0
    RegHardOnSprite = LoadSprite("Reg_Hard_On.lls", 4);
    // STRING: LEGOLAND 0x004befbc
    RegHardOffSprite = LoadSprite("Reg_Hard_Off.lls", 4);

    // STRING: LEGOLAND 0x004befa8
    AcceptIcon = (unsigned int)LoadSpriteIcon("Accept_On_Reg.lls", 4, 0x1ef, 0x14f, 7);
    ((struct IconNode *)AcceptIcon)->string_id = 6;
    ((struct IconNode *)AcceptIcon)->string = GetString(6);
    ((struct IconNode *)AcceptIcon)->flags |= 0x2000;
    ((struct IconNode *)AcceptIcon)->flags |= 0x4002;
    ((struct IconNode *)AcceptIcon)->flags |= 0x400;
    ((struct IconNode *)AcceptIcon)->event_handler = (void *)AcceptProfileClick;
    DAT_006687bc = (unsigned int)AcceptProfileClick;
    DAT_006687c0 = (unsigned int)FUN_004920a0;
    strcpy(DAT_007cb340, GetString(0x84));

    for (; node != NULL; node = node->next) {
        if (node->has_header) {
            icon = InsertIcon(0x80, node->slot * 0x26 + 0x86, 7, GetProfileOffSprite(node->slot));
            icon->string_id = 0;
            str = GetString(0);
            icon->flags |= 0x6002;
            icon->string = str;
            icon->event_handler = (void *)FUN_0048d390;
            icon->field_18p = &node->data;
            icon->slot = node->slot;
        } else {
            icon = InsertIcon(0x80, node->slot * 0x26 + 0x86, 7, GetProfileOffSprite(node->slot));
            icon->string_id = 1;
            str = GetString(1);
            icon->flags |= 0x6002;
            icon->string = str;
            icon->event_handler = (void *)SelectEmptyProfileSlotClick;
            icon->field_18p = "EMPTY";
            icon->slot = node->slot;
        }
        icon->field_20b |= 1;
    }

    DeleteIcon = InsertIcon(0, 0, 7, RegDeleteSprite);
    DeleteIcon->string_id = 2;
    DeleteIcon->string = GetString(2);
    DeleteIcon->flags |= 0x2000;
    DeleteIcon->flags |= 0x4002;
    DeleteIcon->flags |= 0x400;
    DeleteIcon->event_handler = (void *)FUN_0048cc30;
}

// FUNCTION: LEGOLAND 0x0048c5e0
struct Sprite *GetProfileOffSprite(signed char param_1) {
    switch (param_1) {
    case 1:
        return RegProfileOff1Sprite;
    case 2:
        return RegProfileOff2Sprite;
    case 3:
        return RegProfileOff3Sprite;
    case 4:
        return RegProfileOff4Sprite;
    case 5:
        return RegProfileOff5Sprite;
    case 6:
        return RegProfileOff6Sprite;
    case 7:
        return RegProfileOff7Sprite;
    case 8:
        return RegProfileOff8Sprite;
    default:
        return NULL;
    }
}

// FUNCTION: LEGOLAND 0x0048c650
LEGO_EXPORT void EnterNewProfileCheckBoxIcons(struct IconNode *param_1) {
    // STRING: LEGOLAND 0x004bf148
    PopUpCloseSprite = LoadSprite("RegClose.lls", 4);
    // STRING: LEGOLAND 0x004bf138
    PuCloseOnSprite = LoadSprite("RegCloseON.lls", 4);
    ClosePopUpSprite = LoadSprite("PU_ClosePopUp.lls", 4);
    ClosePopUpOnSprite = LoadSprite("PU_ClosePopUpON.lls", 4);

    PopUpOkIcon = 0;
    PopUpCloseIcon = InsertIcon(param_1->x + 0xe1, param_1->y + 0x1e, 0xe, ClosePopUpSprite);
    PopUpCloseIcon->string_id = 4;
    PopUpCloseIcon->string = GetString(4);
    PopUpCloseIcon->flags |= 0x2000;
    PopUpCloseIcon->flags |= 0x4002;
    PopUpCloseIcon->event_handler = (void *)FUN_004920a0;
    DAT_006687c0 = (unsigned int)PopUpCloseIcon->event_handler;
}

// FUNCTION: LEGOLAND 0x0048c720
LEGO_EXPORT void InitProfileCheckBoxIcons(struct IconNode *param_1) {
    PuOkSprite = LoadSprite("PU_OK.lls", 4);
    PuOkOnSprite = LoadSprite("PU_OKON.lls", 4);
    PopUpCloseSprite = LoadSprite("RegClose.lls", 4);
    PuCloseOnSprite = LoadSprite("RegCloseON.lls", 4);
    ClosePopUpSprite = LoadSprite("PU_ClosePopUp.lls", 4);
    ClosePopUpOnSprite = LoadSprite("PU_ClosePopUpON.lls", 4);

    PopUpOkIcon = InsertIcon(param_1->x - 0x24, param_1->y - 0x18, 0xe, PuOkSprite);
    PopUpOkIcon->string_id = 0x2;
    PopUpOkIcon->string = GetString(0x2);
    PopUpOkIcon->flags |= 0x2000;
    PopUpOkIcon->flags |= 0x4002;
    PopUpOkIcon->event_handler = (void *)ConfirmDeleteProfileClick;

    PopUpCloseIcon = InsertIcon(PopUpOkIcon->x + 0x24, PopUpOkIcon->y, 0xe, PopUpCloseSprite);
    PopUpCloseIcon->string_id = 0x4;
    PopUpCloseIcon->string = GetString(0x4);
    PopUpCloseIcon->flags |= 0x2000;
    PopUpCloseIcon->flags |= 0x4002;
    PopUpCloseIcon->event_handler = (void *)FUN_0048d450;
}

// FUNCTION: LEGOLAND 0x0048c860
void FUN_0048c860(struct IconNode *param_1) {
    PuOkSprite = LoadSprite("PU_OK.lls", 4);
    PuOkOnSprite = LoadSprite("PU_OKON.lls", 4);
    PopUpCloseSprite = LoadSprite("RegClose.lls", 4);
    PuCloseOnSprite = LoadSprite("RegCloseON.lls", 4);
    ClosePopUpSprite = LoadSprite("PU_ClosePopUp.lls", 4);
    ClosePopUpOnSprite = LoadSprite("PU_ClosePopUpON.lls", 4);

    PopUpOkIcon = InsertIcon(param_1->x - 0x42, param_1->y - 0x18, 0xe, PuOkSprite);
    PopUpOkIcon->string_id = 5;
    PopUpOkIcon->string = GetString(5);
    PopUpOkIcon->flags |= 0x2000;
    PopUpOkIcon->flags |= 0x4002;
    PopUpOkIcon->event_handler = (void *)FUN_0048e450;

    PopUpCloseIcon = InsertIcon(PopUpOkIcon->x + 0x24, PopUpOkIcon->y, 0xe, PopUpCloseSprite);
    PopUpCloseIcon->string_id = 4;
    PopUpCloseIcon->string = GetString(4);
    PopUpCloseIcon->flags |= 0x2000;
    PopUpCloseIcon->flags |= 0x4002;
    PopUpCloseIcon->event_handler = (void *)FUN_0048d450;
}

// FUNCTION: LEGOLAND 0x0048c9a0
LEGO_EXPORT void KillFrontEndCheckBoxSprite(void) {
    if (PuOkOnSprite != NULL) {
        KillSprite(PuOkOnSprite);
        PuOkOnSprite = NULL;
    }
    if (PuOkSprite != NULL) {
        KillSprite(PuOkSprite);
        PuOkSprite = NULL;
    }
    if (PuCloseOnSprite != NULL) {
        KillSprite(PuCloseOnSprite);
        PuCloseOnSprite = NULL;
    }
    if (PopUpCloseSprite != NULL) {
        KillSprite(PopUpCloseSprite);
        PopUpCloseSprite = NULL;
    }
    if (ClosePopUpSprite != NULL) {
        KillSprite(ClosePopUpSprite);
        ClosePopUpSprite = NULL;
    }
    if (ClosePopUpOnSprite != NULL) {
        KillSprite(ClosePopUpOnSprite);
        ClosePopUpOnSprite = NULL;
    }
}

// FUNCTION: LEGOLAND 0x0048ca40
LEGO_EXPORT void KillListProfileSprite(void) {
    struct Sprite *sprite;
    sprite = RegDeleteOnSprite;
    if (sprite != NULL) {
        KillSprite(sprite);
        RegDeleteOnSprite = NULL;
    }
    sprite = RegDeleteSprite;
    if (sprite != NULL) {
        KillSprite(sprite);
        RegDeleteSprite = NULL;
    }
    sprite = RegProfileOff1Sprite;
    if (sprite != NULL) {
        KillSprite(sprite);
        RegProfileOff1Sprite = NULL;
    }
    sprite = RegProfileOff2Sprite;
    if (sprite != NULL) {
        KillSprite(sprite);
        RegProfileOff2Sprite = NULL;
    }
    sprite = RegProfileOff3Sprite;
    if (sprite != NULL) {
        KillSprite(sprite);
        RegProfileOff3Sprite = NULL;
    }
    sprite = RegProfileOff4Sprite;
    if (sprite != NULL) {
        KillSprite(sprite);
        RegProfileOff4Sprite = NULL;
    }
    sprite = RegProfileOff5Sprite;
    if (sprite != NULL) {
        KillSprite(sprite);
        RegProfileOff5Sprite = NULL;
    }
    sprite = RegProfileOff6Sprite;
    if (sprite != NULL) {
        KillSprite(sprite);
        RegProfileOff6Sprite = NULL;
    }
    sprite = RegProfileOff7Sprite;
    if (sprite != NULL) {
        KillSprite(sprite);
        RegProfileOff7Sprite = NULL;
    }
    sprite = RegProfileOff8Sprite;
    if (sprite != NULL) {
        KillSprite(sprite);
        RegProfileOff8Sprite = NULL;
    }
    sprite = RegProfileOnSprite;
    if (sprite != NULL) {
        KillSprite(sprite);
        RegProfileOnSprite = NULL;
    }
    sprite = DAT_007986b8;
    if (sprite != NULL) {
        KillSprite(sprite);
        DAT_007986b8 = 0;
    }
    sprite = RegDiffPopUpSprite;
    if (sprite != NULL) {
        KillSprite(sprite);
        RegDiffPopUpSprite = NULL;
    }
    sprite = RegEasyOnSprite;
    if (sprite != NULL) {
        KillSprite(sprite);
        RegEasyOnSprite = NULL;
    }
    sprite = RegEasyOffSprite;
    if (sprite != NULL) {
        KillSprite(sprite);
        RegEasyOffSprite = NULL;
    }
    sprite = RegMidOnSprite;
    if (sprite != NULL) {
        KillSprite(sprite);
        RegMidOnSprite = NULL;
    }
    sprite = RegMidOffSprite;
    if (sprite != NULL) {
        KillSprite(sprite);
        RegMidOffSprite = NULL;
    }
    sprite = RegHardOnSprite;
    if (sprite != NULL) {
        KillSprite(sprite);
        RegHardOnSprite = NULL;
    }
    sprite = RegHardOffSprite;
    if (sprite != NULL) {
        KillSprite(sprite);
        RegHardOffSprite = NULL;
    }
}

// FUNCTION: LEGOLAND 0x0048cc10
LEGO_EXPORT void CloseFontEndCheckBox(void) {
    RemoveIconGroup(0xE);
    KillFrontEndCheckBoxSprite();
    DAT_004bef9c = 1;
}

// FUNCTION: LEGOLAND 0x0048cc30
unsigned char FUN_0048cc30(void *param_1, unsigned int param_2) {
    if (DAT_004bef9c != 0 && (param_2 & 2) != 0) {
        if (DAT_0080ff80.unk8 == 0) {
            InitProfileCheckBoxIcons(param_1);
        }
        if (DAT_0080ff80.unk8 == 4) {
            FUN_0048c860(param_1);
        }
        DeletePopUpShown = 1;
        DAT_004bef9c = 0;
    }
    return 1;
}

// FUNCTION: LEGOLAND 0x0048cd50
LEGO_EXPORT void LightUpthisDeleteIcon(struct IconNode *icon, int param_2) {
    int bx;
    int by;

    DeleteIcon->flags &= ~0x400;
    if (param_2 != 0) {
        DeleteIcon->y = icon->y + 0x1b;
        DeleteIcon->x = icon->x + 0xe1;
    } else {
        if (DeletePopUpShown != 0) {
            DeleteIcon->y = icon->y + 0x1b;
        } else {
            DeleteIcon->y = icon->y;
        }
        DeleteIcon->x = icon->x + 0xff;
    }
    SetIconSprite(DeleteIcon, RegDeleteSprite);
    bx = DeleteIcon->x;
    by = DeleteIcon->y;
    if (MousePos.x < bx + 0x24 && bx < MousePos.x && MousePos.y < by + 0x1b && by < MousePos.y) {
        SetIconSprite(DeleteIcon, RegDeleteOnSprite);
    }
}

// FUNCTION: LEGOLAND 0x0048ce20
LEGO_EXPORT void UpdateProfileCheckBoxIcons(void) {
    struct IconNode *a;
    struct IconNode *b;
    int x;
    int y;

    if (PopUpOkIcon) {
        SetIconSprite(PopUpOkIcon, PuOkSprite);
    }
    if (NewProfilePopUpShown) {
        SetIconSprite(PopUpCloseIcon, ClosePopUpSprite);
    } else {
        SetIconSprite(PopUpCloseIcon, PopUpCloseSprite);
    }
    a = PopUpOkIcon;
    if (a) {
        x = a->x;
        y = a->y;
        if (MousePos.x < x + 0x24 && x < MousePos.x && MousePos.y < y + 0x1b && y < MousePos.y) {
            SetIconSprite(a, PuOkOnSprite);
        }
    }
    b = PopUpCloseIcon;
    x = b->x;
    y = b->y;
    if (MousePos.x < x + 0x24 && x < MousePos.x && MousePos.y < y + 0x1b && y < MousePos.y) {
        if (NewProfilePopUpShown) {
            SetIconSprite(b, ClosePopUpOnSprite);
            return;
        }
        SetIconSprite(b, PuCloseOnSprite);
    }
}

// FUNCTION: LEGOLAND 0x0048cf10
LEGO_EXPORT void PrintProfileDetails(void) {
    struct IconNode *icon;
    struct IconNode *last;
    char *name;
    int y;
    int x;
    unsigned char sel;
    int show;

    y = 0x72;
    DeleteIcon->flags |= 0x400;
    icon = IconListHead;
    FUN_00455e50(DAT_007cb340, 0x8d, y, 0xf0, 0x2e, 3, 0x25, 0xffffff, 0);
    while (icon != NULL) {
        sel = 1;
        show = 1;
        if ((icon->flags & 0x400) == 0 && (icon->field_20b & 1)) {
            if (MousePos.x >= icon->x - 0x18 && MousePos.x < icon->x && MousePos.y >= icon->slot * 0x26 + 0x86 &&
                MousePos.y < icon->width + icon->y) {
                Hover.type = 2;
                Hover.ptr = (struct Bloke *)icon;
            }
            if (CurrentProfile.profile_slot == icon->slot) {
                if (DeletePopUpShown != 0) {
                    SetIconSprite(icon, DAT_007986b8);
                    last = icon;
                    icon->y = icon->slot * 0x26 + 0x6b;
                    y = icon->y + 0x22;
                } else if (NewProfilePopUpShown != 0) {
                    EnterNewProfile(icon);
                    show = 0;
                    last = icon;
                } else {
                    SetIconSprite(icon, RegDiffPopUpSprite);
                    icon->y = icon->slot * 0x26 + 0x6b;
                    y = icon->y + 0x22;
                    LightUpthisDeleteIcon(icon, 1);
                    last = icon;
                }
            } else {
                SetIconSprite(icon, GetProfileOffSprite(icon->slot));
                sel = 0;
                icon->y = icon->slot * 0x26 + 0x86;
                y = icon->y + 7;
            }
            x = icon->x + 0x14;
            name = (char *)icon->field_18p;
            if (name != NULL && show) {
                if (CurrentProfile.profile_slot - 1 != icon->slot || (DeletePopUpShown == 0 && NewProfilePopUpShown == 0)) {
                    if (sel) {
                        FUN_00455e50(name, x, y, 0xe0, 0x13, 2, 0x25, 0, 0xffffff);
                    } else {
                        FUN_00455e50(name, x, y, 0xe0, 0x13, 2, 0x25, 0xffffff, 0);
                    }
                }
            }
        }
        icon = icon->next;
    }
    if (DeletePopUpShown != 0) {
        FUN_00455e50(GetString(0x85), last->x + 0x14, last->y + 7, 0x9b, 0x13, 2, 0x25, 0, 0xffffff);
    } else if (NewProfilePopUpShown != 0) {
        FUN_00455e50(GetString(0x86), last->x + 0x14, last->y - 0x14, 0xe0, 0x13, 2, 0x25, 0, 0xffffff);
        UpdateProfileCheckBoxIcons();
    }
    if (CurrentProfile.profile_slot != 0 && DeletePopUpShown == 0) {
        if (NewProfilePopUpShown != 0) {
            if (TempProfileHasName()) {
                ((struct IconNode *)AcceptIcon)->flags &= ~0x400;
                return;
            }
        } else {
            ((struct IconNode *)AcceptIcon)->flags &= ~0x400;
            return;
        }
    }
    ((struct IconNode *)AcceptIcon)->flags |= 0x400;
}

// FUNCTION: LEGOLAND 0x0048d230
void FUN_0048d230(void) {
    struct ProfileNode *node = (struct ProfileNode *)ProfileListHead;

    while (node != NULL) {
        if (node->slot == CurrentProfile.profile_slot) {
            strcpy((char *)&CurrentProfile, node->data.name);
            CurrentProfile.field_20 = node->data.field_20;
            CurrentProfile.save_slot = 0;
            CurrentProfile.speech_volume = node->data.speech_volume;
            CurrentProfile.music_volume = node->data.music_volume;
            CurrentProfile.fx_volume = node->data.fx_volume;
            CurrentProfile.field_45 = 0;
            memcpy(&CurrentProfile.flags[4], &node->data.field_34, 15);
            memcpy(CurrentProfile.field_46, node->data.field_43, 200);
            *(int *)&CurrentProfile.flags = *(int *)&node->data.field_10b;
            return;
        }
        node = node->next;
    }
}

// FUNCTION: LEGOLAND 0x0048d300
unsigned char AcceptProfileClick(unsigned int dummy, unsigned char arg_0) {
    if (DeletePopUpShown == 0 && (arg_0 & 0x2) != 0 && ((((struct ProfileFlags *)AcceptIcon)->var_34 >> 8) & 0x4) == 0 && CurrentProfile.profile_slot != 0) {
        if (NewProfilePopUpShown != 0) {
            SaveProfileToDisk();
            DeleteProfileList();
            LoadProfilesFormDisk();
            RemoveIconGroup(0x15);
            CloseFontEndCheckBox();
            NewProfilePopUpShown = 0;
        }
        SpeechCloseFile();
        DAT_006687b0 = 4;
        PlayInstanceOfSample(GameFX[FX_BUTTON4].sample, 0, 1, 0);
        DAT_0080ff80.unk8 = 1;
    }
    return 1;
}

// FUNCTION: LEGOLAND 0x0048d390
unsigned char FUN_0048d390(struct Profile *profile, unsigned char param_2) {
    if (DAT_004bef9c != 0 && (param_2 & 0x2) != 0) {
        CurrentProfile.profile_slot = profile->var_1c;
        FUN_0048a800();
    }
    return 1;
}

// FUNCTION: LEGOLAND 0x0048d3c0
unsigned char SelectEmptyProfileSlotClick(struct Profile *profile, unsigned int param_2) {
    if (DAT_004bef9c != 0) {
        if (param_2 & 0x2) {
            CurrentProfile.profile_slot = profile->var_1c;
            NewProfilePopUpShown = 1;
            InitNewProfilePoPUp(profile);
            DAT_004bef9c = 0;
        }
    }
    return 1;
}

// FUNCTION: LEGOLAND 0x0048d400
unsigned char ConfirmDeleteProfileClick(unsigned int arg0, unsigned int arg1) {
    if (arg1 & 0x2) {
        if (CurrentProfile.profile_slot) {
            CloseFontEndCheckBox();
            DeletePopUpShown = 0;
            RemoveProfile(CurrentProfile.profile_slot);
            CurrentProfile.profile_slot = 0;
            DAT_0080ff80.unk4 = 0xffffffff;
            DAT_0080ff80.unk8 = 0;
        }
    }
    return 1;
}

// FUNCTION: LEGOLAND 0x0048d450
unsigned char FUN_0048d450(unsigned int param_1, unsigned int param_2) {
    if ((param_2 & 2) != 0) {
        DeletePopUpShown = 0;
        CloseFontEndCheckBox();
    }
    return 1;
}

// FUNCTION: LEGOLAND 0x0048d470
void FUN_0048d470(void) {
    DAT_007986f8 = DAT_006687bc;
    DAT_007986f4 = DAT_006687c0;
}

// FUNCTION: LEGOLAND 0x0048d490
void FUN_0048d490(void) {
    DAT_006687bc = DAT_007986f8;
    DAT_006687c0 = DAT_007986f4;
}
