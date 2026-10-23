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
    RegDeletePopUpSprite = LoadSprite("Reg_Delete_PopUp.lls", 4);
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
    AcceptIcon = LoadSpriteIcon("Accept_On_Reg.lls", 4, 0x1ef, 0x14f, 7);
    ((struct IconNode *)AcceptIcon)->string_id = 6;
    ((struct IconNode *)AcceptIcon)->string = GetString(6);
    ((struct IconNode *)AcceptIcon)->flags |= 0x2000;
    ((struct IconNode *)AcceptIcon)->flags |= 0x4002;
    ((struct IconNode *)AcceptIcon)->flags |= 0x400;
    ((struct IconNode *)AcceptIcon)->event_handler = (void *)AcceptProfileClick;
    OkKeyHandler = AcceptProfileClick;
    CancelKeyHandler = FUN_004920a0;
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
    DeleteIcon->event_handler = (void *)DeleteIconHandler;
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
    CancelKeyHandler = PopUpCloseIcon->event_handler;
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
    PopUpCloseIcon->event_handler = (void *)DeletePopUpCancelHandler;
}

// FUNCTION: LEGOLAND 0x0048c860
void InitDeletePopUp(struct IconNode *param_1) {
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
    PopUpOkIcon->event_handler = (void *)DeletePopUpOkHandler;

    PopUpCloseIcon = InsertIcon(PopUpOkIcon->x + 0x24, PopUpOkIcon->y, 0xe, PopUpCloseSprite);
    PopUpCloseIcon->string_id = 4;
    PopUpCloseIcon->string = GetString(4);
    PopUpCloseIcon->flags |= 0x2000;
    PopUpCloseIcon->flags |= 0x4002;
    PopUpCloseIcon->event_handler = (void *)DeletePopUpCancelHandler;
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
    sprite = RegDeletePopUpSprite;
    if (sprite != NULL) {
        KillSprite(sprite);
        RegDeletePopUpSprite = 0;
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
unsigned char DeleteIconHandler(void *param_1, unsigned int param_2) {
    if (DAT_004bef9c != 0 && (param_2 & 2) != 0) {
        if (DAT_0080ff80.unk8 == 0) {
            InitProfileCheckBoxIcons(param_1);
        }
        if (DAT_0080ff80.unk8 == 4) {
            InitDeletePopUp(param_1);
        }
        DeletePopUpShown = 1;
        DAT_004bef9c = 0;
    }
    return 1;
}

// FUNCTION: LEGOLAND 0x0048cd50
LEGO_EXPORT void LightUpthisDeleteIcon(struct IconNode *icon, int param_2) {
    RECT rc;

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
    rc.left = DeleteIcon->x;
    rc.top = DeleteIcon->y;
    rc.right = rc.left + 0x24;
    rc.bottom = rc.top + 0x1b;
    if (MousePos.x < rc.right && rc.left < MousePos.x && MousePos.y < rc.bottom && rc.top < MousePos.y) {
        SetIconSprite(DeleteIcon, RegDeleteOnSprite);
    }
}

// FUNCTION: LEGOLAND 0x0048ce20
LEGO_EXPORT void UpdateProfileCheckBoxIcons(void) {
    RECT rc;

    if (PopUpOkIcon) {
        SetIconSprite(PopUpOkIcon, PuOkSprite);
    }
    if (NewProfilePopUpShown) {
        SetIconSprite(PopUpCloseIcon, ClosePopUpSprite);
    } else {
        SetIconSprite(PopUpCloseIcon, PopUpCloseSprite);
    }
    if (PopUpOkIcon) {
        rc.left = PopUpOkIcon->x;
        rc.top = PopUpOkIcon->y;
        rc.right = rc.left + 0x24;
        rc.bottom = rc.top + 0x1b;
        if (MousePos.x < rc.right && rc.left < MousePos.x && MousePos.y < rc.bottom && rc.top < MousePos.y) {
            SetIconSprite(PopUpOkIcon, PuOkOnSprite);
        }
    }
    rc.left = PopUpCloseIcon->x;
    rc.top = PopUpCloseIcon->y;
    rc.right = rc.left + 0x24;
    rc.bottom = rc.top + 0x1b;
    if (MousePos.x < rc.right && rc.left < MousePos.x && MousePos.y < rc.bottom && rc.top < MousePos.y) {
        if (NewProfilePopUpShown) {
            SetIconSprite(PopUpCloseIcon, ClosePopUpOnSprite);
            return;
        }
        SetIconSprite(PopUpCloseIcon, PuCloseOnSprite);
    }
}

// Draws the profile list: the title, then for every visible profile icon its sprite (selected /
// delete-selected / off) at its row (slot * 0x26), the hover hit test for its delete button, and
// its name. The new-profile entry hands over to EnterNewProfile. With the delete or new-profile
// pop-up open it also draws that pop-up's prompt. Finally it enables the accept icon only when a
// profile is selected (and, for a new profile, it has a name).
//
// MSVC6 build: exact naked __asm transcription; the C in the #else branch is the same function and
// each asm block is commented with the C it implements (IconNode offsets: +0xc x, +0xe y,
// +0x10 width, +0x18 name, +0x1c slot, +0x20 field_20b, +0x34 flags). Why asm: the C was at best
// 96.97% after trying all 720 declaration orders: the `last` and `name` stack slots stay swapped.
// See docs/attempts/0x0048cf10-PrintProfileDetails.md.
// clang-format off
#if defined(_MSC_VER) && (_MSC_VER <= 1200) && defined(_M_IX86)
// FUNCTION: LEGOLAND 0x0048cf10
__declspec(naked) LEGO_EXPORT void PrintProfileDetails(void) {
    __asm {
        // Frame after the pushes: [esp+0xf] sel (byte), [esp+0x10] name, [esp+0x14] last
        // (+4 while ebx is pushed inside the loop). esi = icon, edi = rc.top, ebp = 0x400 or rc.left.
        // DeleteIcon->flags |= 0x400;
        mov eax, dword ptr [DeleteIcon]
        sub esp, 0xc
        push ebp
        push esi
        mov esi, dword ptr [IconListHead] // icon = IconListHead
        push edi
        mov edi, dword ptr [eax + 0x34]
        mov ebp, 0x400
        // PrintTextBox(DAT_007cb340, 0x8d, rc.top = 0x72, 0xf0, 0x2e, 3, 0x25, 0xffffff, 0);
        push 0
        or edi, ebp
        push 0xffffff
        push 0x25
        mov dword ptr [eax + 0x34], edi
        push 3
        mov edi, 0x72
        push 0x2e
        push 0xf0
        push edi
        push 0x8d
        push offset DAT_007cb340
        call PrintTextBox
        add esp, 0x24
        // for (; icon != NULL; icon = icon->next) {
        test esi, esi
        je list_done
        push ebx
        jmp icon_loop_top
    next_icon_reload:
        mov ebp, 0x400
    icon_loop_top:
        //     show = 1 (ebx); sel = 1;
        //     if (!(icon->flags & 0x400) && (icon->field_20b & 1)) {
        mov eax, dword ptr [esi + 0x34]
        mov ebx, 1
        // test ebp, eax -- emitted as bytes: MSVC's inline assembler only produces the other valid encoding
        _emit 0x85
        _emit 0xc5
        mov byte ptr [esp + 0x13], bl // sel
        jne next_icon
        test byte ptr [esi + 0x20], bl
        je next_icon
        //         if (MousePos.x in [icon->x - 0x18, icon->x) &&
        //             MousePos.y in [icon->slot * 0x26 + 0x86, icon->y + icon->width))
        //             { Hover.type = 2; Hover.ptr = icon; }
        movsx eax, word ptr [esi + 0xc]
        mov ecx, dword ptr [MousePos]
        lea edx, [eax - 0x18]
        cmp ecx, edx
        jl hover_done
        cmp ecx, eax
        jge hover_done
        xor eax, eax
        mov al, byte ptr [esi + 0x1c]
        lea ecx, [eax + eax * 8]
        lea edx, [eax + ecx * 2] // slot * 19
        mov eax, dword ptr [MousePos + 0x4]
        lea ecx, [edx + edx + 0x86] // slot * 0x26 + 0x86
        cmp eax, ecx
        jl hover_done
        movsx edx, word ptr [esi + 0x10]
        movsx ecx, word ptr [esi + 0xe]
        add edx, ecx
        cmp eax, edx
        jge hover_done
        mov dword ptr [Hover], 2
        mov dword ptr [Hover + 0x4], esi
    hover_done:
        //         if (CurrentProfile.profile_slot == icon->slot) {
        mov dl, byte ptr [CurrentProfile + 0x43]
        mov al, byte ptr [esi + 0x1c]
        cmp dl, al
        jne not_selected
        //             if (DeletePopUpShown) {
        //                 SetIconSprite(icon, RegDeletePopUpSprite); last = icon;
        //                 icon->y = icon->slot * 0x26 + 0x6b; rc.top = icon->y + 0x22;
        mov eax, dword ptr [DeletePopUpShown]
        test eax, eax
        je not_deleting
        mov eax, dword ptr [RegDeletePopUpSprite]
        push eax
        push esi
        call SetIconSprite
        movzx ax, byte ptr [esi + 0x1c]
        add esp, 8
        mov dword ptr [esp + 0x18], esi // last
        lea ecx, [eax + eax * 8]
        lea eax, [eax + ecx * 2]
        lea eax, [eax + eax + 0x6b]
        movsx edi, ax
        mov word ptr [esi + 0xe], ax
        add edi, 0x22
        jmp text_box
    not_deleting:
        //             } else if (NewProfilePopUpShown) { EnterNewProfile(icon); show = 0; last = icon; }
        mov eax, dword ptr [NewProfilePopUpShown]
        test eax, eax
        je selected_plain
        push esi
        call EnterNewProfile
        add esp, 4
        xor ebx, ebx // show = 0
        mov dword ptr [esp + 0x18], esi
        jmp text_box
    selected_plain:
        //             } else {
        //                 SetIconSprite(icon, RegDiffPopUpSprite);
        //                 icon->y = icon->slot * 0x26 + 0x6b; rc.top = icon->y + 0x22;
        //                 LightUpthisDeleteIcon(icon, 1); last = icon;
        //             }
        mov edx, dword ptr [RegDiffPopUpSprite]
        push edx
        push esi
        call SetIconSprite
        movzx ax, byte ptr [esi + 0x1c]
        push 1
        push esi
        lea ecx, [eax + eax * 8]
        lea eax, [eax + ecx * 2]
        lea eax, [eax + eax + 0x6b]
        movsx edi, ax
        mov word ptr [esi + 0xe], ax
        add edi, 0x22
        call LightUpthisDeleteIcon
        add esp, 0x10
        mov dword ptr [esp + 0x18], esi
        jmp text_box
    not_selected:
        //         } else {
        //             SetIconSprite(icon, GetProfileOffSprite(icon->slot)); sel = 0;
        //             icon->y = icon->slot * 0x26 + 0x86; rc.top = icon->y + 7;
        //         }
        mov dl, byte ptr [esi + 0x1c]
        push edx
        call GetProfileOffSprite
        push eax
        push esi
        call SetIconSprite
        movzx ax, byte ptr [esi + 0x1c]
        add esp, 0xc
        mov byte ptr [esp + 0x13], 0 // sel = 0
        lea ecx, [eax + eax * 8]
        lea eax, [eax + ecx * 2]
        lea eax, [eax + eax + 0x86]
        movsx edi, ax
        mov word ptr [esi + 0xe], ax
        add edi, 7
    text_box:
        //         rc.left = icon->x + 0x14 (ebp); rc.bottom = rc.top + 0x13 (eax); rc.right = rc.left + 0xe0 (ecx);
        //         name = icon->field_18p; if (name == NULL || !show) continue;
        movsx ebp, word ptr [esi + 0xc]
        mov edx, dword ptr [esi + 0x18]
        add ebp, 0x14
        test edx, edx
        lea eax, [edi + 0x13]
        lea ecx, [ebp + 0xe0]
        mov dword ptr [esp + 0x14], edx // name
        je next_icon
        test ebx, ebx
        je next_icon
        //         if (CurrentProfile.profile_slot - 1 == icon->slot && (DeletePopUpShown || NewProfilePopUpShown)) continue;
        xor edx, edx
        xor ebx, ebx
        mov dl, byte ptr [CurrentProfile + 0x43]
        mov bl, byte ptr [esi + 0x1c]
        dec edx
        cmp edx, ebx
        jne draw_name
        mov edx, dword ptr [DeletePopUpShown]
        test edx, edx
        jne next_icon
        mov edx, dword ptr [NewProfilePopUpShown]
        test edx, edx
        jne next_icon
    draw_name:
        //         PrintTextBox(name, rc.left, rc.top, width, height, 2, 0x25, sel ? 0 : 0xffffff, sel ? 0xffffff : 0);
        mov dl, byte ptr [esp + 0x13] // sel
        test dl, dl
        je draw_name_off
        push 0xffffff
        push 0
        push 0x25
        sub eax, edi
        push 2
        sub ecx, ebp
        push eax
        mov eax, dword ptr [esp + 0x28] // name
        push ecx
        push edi
        push ebp
        push eax
        jmp draw_name_call
    draw_name_off:
        push 0
        push 0xffffff
        push 0x25
        sub eax, edi
        push 2
        sub ecx, ebp
        push eax
        push ecx
        mov ecx, dword ptr [esp + 0x2c] // name
        push edi
        push ebp
        push ecx
    draw_name_call:
        call PrintTextBox
        add esp, 0x24
        //     }
    next_icon:
        mov esi, dword ptr [esi] // icon = icon->next
        test esi, esi
        jne next_icon_reload
        pop ebx
        mov ebp, 0x400
        // }
    list_done:
        // if (DeletePopUpShown) {
        //     PrintTextBox(GetString(0x85), last->x + 0x14, last->y + 7, 0x9b, 0x13, 2, 0x25, 0, 0xffffff);
        //     UpdateProfileCheckBoxIcons();
        mov eax, dword ptr [DeletePopUpShown]
        test eax, eax
        je no_delete_prompt
        mov ecx, dword ptr [esp + 0x14] // last
        push 0xffffff
        push 0
        push 0x25
        movsx eax, word ptr [ecx + 0xc]
        movsx ecx, word ptr [ecx + 0xe]
        add ecx, 7
        add eax, 0x14
        push 2
        lea esi, [ecx + 0x13]
        lea edx, [eax + 0x9b]
        sub esi, ecx
        sub edx, eax
        push esi
        push edx
        push ecx
        push eax
        push 0x85
        jmp print_prompt
    no_delete_prompt:
        // } else if (NewProfilePopUpShown) {
        //     PrintTextBox(GetString(0x86), last->x + 0x14, last->y - 0x14, 0xe0, 0x13, 2, 0x25, 0, 0xffffff);
        //     UpdateProfileCheckBoxIcons();
        // }
        mov eax, dword ptr [NewProfilePopUpShown]
        test eax, eax
        je accept_icon
        mov ecx, dword ptr [esp + 0x14]
        push 0xffffff
        push 0
        push 0x25
        movsx eax, word ptr [ecx + 0xc]
        movsx ecx, word ptr [ecx + 0xe]
        sub ecx, 0x14
        add eax, 0x14
        push 2
        lea esi, [ecx + 0x13]
        lea edx, [eax + 0xe0]
        sub esi, ecx
        sub edx, eax
        push esi
        push edx
        push ecx
        push eax
        push 0x86
    print_prompt:
        call GetString
        add esp, 4
        push eax
        call PrintTextBox
        add esp, 0x24
        call UpdateProfileCheckBoxIcons
    accept_icon:
        // AcceptIcon->flags: clear 0x400 (enable) if a profile is selected, no delete pop-up, and
        // (no new-profile pop-up or TempProfileHasName()); otherwise set it.
        mov al, byte ptr [CurrentProfile + 0x43]
        test al, al
        je accept_disable
        mov eax, dword ptr [DeletePopUpShown]
        test eax, eax
        jne accept_disable
        mov eax, dword ptr [NewProfilePopUpShown]
        test eax, eax
        je accept_enable
        call TempProfileHasName
        test eax, eax
        mov eax, dword ptr [AcceptIcon]
        mov ecx, dword ptr [eax + 0x34]
        je accept_disable_have_flags
        and ch, 0xfb // flags &= ~0x400
        pop edi
        pop esi
        mov dword ptr [eax + 0x34], ecx
        pop ebp
        add esp, 0xc
        ret
    accept_enable:
        mov eax, dword ptr [AcceptIcon]
        pop edi
        pop esi
        pop ebp
        mov ecx, dword ptr [eax + 0x34]
        and ch, 0xfb
        mov dword ptr [eax + 0x34], ecx
        add esp, 0xc
        ret
    accept_disable:
        mov eax, dword ptr [AcceptIcon]
        mov ecx, dword ptr [eax + 0x34]
    accept_disable_have_flags:
        or ecx, ebp // flags |= 0x400
        pop edi
        pop esi
        mov dword ptr [eax + 0x34], ecx
        pop ebp
        add esp, 0xc
        ret
    }
}
// clang-format on
#else
LEGO_EXPORT void PrintProfileDetails(void) {
    struct IconNode *icon;
    struct IconNode *last;
    char *name;
    RECT rc; /* text box: left/top/right/bottom, passed as x, y, width, height */
    unsigned char sel;
    int show;

    icon = IconListHead;
    DeleteIcon->flags |= 0x400;
    rc.top = 0x72;
    PrintTextBox(DAT_007cb340, 0x8d, rc.top, 0xf0, 0x2e, 3, 0x25, 0xffffff, 0);
    for (; icon != NULL; icon = icon->next) {
        show = 1;
        sel = 1;
        if ((icon->flags & 0x400) == 0 && (icon->field_20b & 1)) {
            if (MousePos.x >= icon->x - 0x18 && MousePos.x < icon->x && MousePos.y >= icon->slot * 0x26 + 0x86 &&
                MousePos.y < icon->width + icon->y) {
                Hover.type = 2;
                Hover.ptr = (struct Bloke *)icon;
            }
            if (CurrentProfile.profile_slot == (int)icon->slot) {
                if (DeletePopUpShown != 0) {
                    SetIconSprite(icon, RegDeletePopUpSprite);
                    last = icon;
                    icon->y = icon->slot * 0x26 + 0x6b;
                    rc.top = icon->y + 0x22;
                } else if (NewProfilePopUpShown != 0) {
                    EnterNewProfile(icon);
                    show = 0;
                    last = icon;
                } else {
                    SetIconSprite(icon, RegDiffPopUpSprite);
                    icon->y = icon->slot * 0x26 + 0x6b;
                    rc.top = icon->y + 0x22;
                    LightUpthisDeleteIcon(icon, 1);
                    last = icon;
                }
            } else {
                SetIconSprite(icon, GetProfileOffSprite(icon->slot));
                sel = 0;
                icon->y = icon->slot * 0x26 + 0x86;
                rc.top = icon->y + 7;
            }
            rc.bottom = rc.top + 0x13;
            rc.left = icon->x + 0x14;
            rc.right = rc.left + 0xe0;
            name = (char *)icon->field_18p;
            if (name != NULL && show) {
                if (CurrentProfile.profile_slot - 1 != icon->slot || (DeletePopUpShown == 0 && NewProfilePopUpShown == 0)) {
                    if (sel) {
                        PrintTextBox(name, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, 2, 0x25, 0, 0xffffff);
                    } else {
                        PrintTextBox(name, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, 2, 0x25, 0xffffff, 0);
                    }
                }
            }
        }
    }
    if (DeletePopUpShown != 0) {
        rc.left = last->x + 0x14;
        rc.top = last->y + 7;
        rc.right = rc.left + 0x9b;
        rc.bottom = rc.top + 0x13;
        PrintTextBox(GetString(0x85), rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, 2, 0x25, 0, 0xffffff);
        UpdateProfileCheckBoxIcons();
    } else if (NewProfilePopUpShown != 0) {
        rc.left = last->x + 0x14;
        rc.top = last->y - 0x14;
        rc.right = rc.left + 0xe0;
        rc.bottom = rc.top + 0x13;
        PrintTextBox(GetString(0x86), rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, 2, 0x25, 0, 0xffffff);
        UpdateProfileCheckBoxIcons();
    }
    if (CurrentProfile.profile_slot != 0 && DeletePopUpShown == 0) {
        if (NewProfilePopUpShown != 0) {
            if (TempProfileHasName()) {
                ((struct IconNode *)AcceptIcon)->flags &= ~0x400;
            } else {
                ((struct IconNode *)AcceptIcon)->flags |= 0x400;
            }
        } else {
            ((struct IconNode *)AcceptIcon)->flags &= ~0x400;
        }
    } else {
        ((struct IconNode *)AcceptIcon)->flags |= 0x400;
    }
}
#endif

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
    if (DeletePopUpShown == 0 && (arg_0 & 0x2) != 0 && ((((struct IconNode *)AcceptIcon)->flags >> 8) & 0x4) == 0 && CurrentProfile.profile_slot != 0) {
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
unsigned char FUN_0048d390(struct IconNode *profile, unsigned char param_2) {
    if (DAT_004bef9c != 0 && (param_2 & 0x2) != 0) {
        CurrentProfile.profile_slot = profile->field_1cb;
        ClearFreePlaySelection();
    }
    return 1;
}

// FUNCTION: LEGOLAND 0x0048d3c0
unsigned char SelectEmptyProfileSlotClick(struct IconNode *profile, unsigned int param_2) {
    if (DAT_004bef9c != 0) {
        if (param_2 & 0x2) {
            CurrentProfile.profile_slot = profile->field_1cb;
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
unsigned char DeletePopUpCancelHandler(unsigned int param_1, unsigned int param_2) {
    if ((param_2 & 2) != 0) {
        DeletePopUpShown = 0;
        CloseFontEndCheckBox();
    }
    return 1;
}

// FUNCTION: LEGOLAND 0x0048d470
void FUN_0048d470(void) {
    DAT_007986f8 = OkKeyHandler;
    DAT_007986f4 = CancelKeyHandler;
}

// FUNCTION: LEGOLAND 0x0048d490
void FUN_0048d490(void) {
    OkKeyHandler = DAT_007986f8;
    CancelKeyHandler = DAT_007986f4;
}
