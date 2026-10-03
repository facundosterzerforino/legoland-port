#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "globals.h"
#include "legoland.h"

#include "bricks.h"
#include "clipping.h"
#include "draw.h"
#include "freeplay.h"
#include "game_util.h"
#include "icon.h"
#include "interface.h"
#include "llidb.h"
#include "map_object.h"
#include "obj_instance.h"
#include "objclass.h"
#include "objectives.h"
#include "options.h"
#include "saveload.h"
#include "screens.h"
#include "sound_music.h"
#include "sound_sfx.h"
#include "stream.h"
#include "string.h"
#include "title.h"

struct PanelNode {
    struct PanelNode *next;
    void *buffer1;
    char **name;
    char **after;
    void *buffer2;
    int a;
    int key;
};

struct FreePlayGroup {
    short group;
    unsigned char pad_2[2];
    unsigned int field_4;
    struct IconNode *icon;
    int content_left;
    int content_top;
    int field_14;
    int content_bottom;
    int clip_left;
    int clip_top;
    int clip_right;
    int clip_bottom;
};

struct GameListNode {
    struct GameListNode *next;
    unsigned char pad_4[0x14];
    unsigned char field_18;
    unsigned char pad_19[3];
    unsigned int field_1c;
};

// LLIDB element fields read by InitFreePlayScreen (real Element is opaque).
struct FreePlayElement {
    unsigned char pad_0[8];
    unsigned int flags;
    unsigned char pad_c[4];
    unsigned int counter;
};

#include "image_sprite.h"

// FUNCTION: LEGOLAND 0x0048a8a0
LEGO_EXPORT void InitFreePlayScreen(void) {
    struct IconNode *icon;
    struct IconNode *bar;
    struct FreePlayElement *elem;
    unsigned int path_idx;
    unsigned int count;
    unsigned int i;

    DAT_00668e38 = 0;
    FUN_0048b6c0();
    DAT_007cb3a0 = 0;
    DAT_00798650 = 0;

    // STRING: LEGOLAND 0x004beb34
    SPRITE_TitleScreenBk = LoadSprite("FreePlayScreenBK.lls", 0);
    // STRING: LEGOLAND 0x004beb24
    FreePlayDown1Sprite = LoadSprite("FP_Down1.lls", 4);
    // STRING: LEGOLAND 0x004beb14
    FreePlayDown2Sprite = LoadSprite("FP_Down2.lls", 4);
    // STRING: LEGOLAND 0x004beb04
    FreePlayDown3Sprite = LoadSprite("FP_Down3.lls", 4);
    // STRING: LEGOLAND 0x004beaf4
    FreePlayDown4Sprite = LoadSprite("FP_Down4.lls", 4);
    // STRING: LEGOLAND 0x004beae8
    FreePlayUp1Sprite = LoadSprite("FP_Up1.lls", 4);
    // STRING: LEGOLAND 0x004beadc
    FreePlayUp2Sprite = LoadSprite("FP_Up2.lls", 4);
    // STRING: LEGOLAND 0x004bead0
    FreePlayUp3Sprite = LoadSprite("FP_Up3.lls", 4);
    // STRING: LEGOLAND 0x004beac4
    FreePlayUp4Sprite = LoadSprite("FP_Up4.lls", 4);
    // STRING: LEGOLAND 0x004beab0
    FreePlayTickSprite = LoadSprite("FreePlay_Tick.lls", 4);
    // STRING: LEGOLAND 0x004beaa0
    FreePlayCoverSprite = LoadSprite("FP_Cover.lls", 4);

    // STRING: LEGOLAND 0x004bea88
    icon = LoadSpriteIcon("GoBack_on_FreePlay.lls", 4, 0xd, 0x137, 7);
    icon->string_id = 0x26;
    icon->string = GetString(0x26);
    icon->flags |= 0x6002;
    icon->event_handler = (void *)FUN_0048fb80;
    DAT_006687c0 = (unsigned int)FUN_0048fb80;

    // STRING: LEGOLAND 0x004bea70
    icon = LoadSpriteIcon("Accept_On_FreePlay.lls", 4, 0x1e4, 0x14b, 7);
    icon->string_id = 0x32;
    icon->string = GetString(0x32);
    icon->flags |= 0x6002;
    icon->event_handler = (void *)FUN_0048ac60;
    DAT_006687bc = (unsigned int)FUN_0048ac60;
    icon->flags |= 0x400;
    DAT_0079864c = icon;

    // STRING: LEGOLAND 0x004bea60
    bar = LoadSpriteIcon("Bar_Rides.lls", 4, 0xd1, 0x156, 7);
    bar->render_func = (void *)RenderFreePlayBar;
    bar->flags |= 0x400a;

    count = LLIDB_GetCount();
    // STRING: LEGOLAND 0x004b8a70
    LLIDB_FindElement("PATH CONTROL", &path_idx, 0);

    for (i = 0; (int)i < (int)count; i++) {
        LLIDB_GetElement(i, (struct Element **)&elem);
        if ((elem->flags & 0x10) == 0) continue;
        if ((elem->flags & 0x1) == 0) continue;
        if (path_idx == (unsigned int)elem) continue;
        if (elem->counter == 0) continue;
        elem->counter--;
        if (elem->counter != 0) continue;
        if ((elem->flags & 0x1) == 0) continue;
        elem->flags &= 0xfffcfff0;
        if ((elem->flags & 0xfff0) == 0x10 || (elem->flags & 0xfff0) == 0x1010) {
            LLIDB_UnLoadODF((struct LLIDBHead *)elem);
        }
    }

    LLIDB_ClearOnLevel();
    InitFreePlayLists();

    FreePlayObjectList(0xc8, 0x2a, 0x41, 0xec, 0xc8);
    FreePlayObjectList(0x1f4, 0xbb, 0x41, 0xec, 0x1f4);
    FreePlayObjectList(0x190, 0x14c, 0x41, 0xec, 0x190);
    FreePlayObjectList(0x12c, 0x1dd, 0x41, 0xec, 0x12c);

    FUN_0048a790();
}

// FUNCTION: LEGOLAND 0x0048ab60
void FUN_0048ab60(void) {
    struct GameListNode *node;

    node = (struct GameListNode *)IconListHead;
    if (node == 0) {
        return;
    }
    do {
        DrawWatchSprite();
        if (node->field_18 == 1) {
            if (FUN_00478b20(node->field_1c) != 0) {
                FUN_00469900((struct NerpsArg *)ElemID((const char *)node->field_1c), 0, 1);
            }
        }
        node = node->next;
    } while (node != 0);
}

// FUNCTION: LEGOLAND 0x0048abb0
void FUN_0048abb0(void) {
    char buf[0x34];

    QueryClass = 0;
    // STRING: LEGOLAND 0x004beb4c
    sprintf(buf, "FreePlayTest.txt");
    PauseGameTimer();
    ResetGameTimer();
    MarkGameTimer();
    CastlePlacedFlag = 0;
    ResetMapAI();
    DAT_00667c4c = FUN_0047afb0(buf);
    SetBricksLimited(0);
    FUN_0048ab60();
    AllocBlokeCounters(lpConfig->max_blokes);
    FUN_00458940();
    MapStats.field_3a0 = 0;
    FUN_00489ee0();
    UpdateMenu();
    UnloadWatchSprite();
    FUN_00490600(1);
    FUN_004911c0(DAT_0066861c, 0);
    SetMapLoaded(1);
    ResumeGameTimer();
    UpdateSoundVols();
}

// FUNCTION: LEGOLAND 0x0048ac60
unsigned char FUN_0048ac60(unsigned int param_1, unsigned int param_2) {
    if (DAT_0079864c != 0 && (DAT_0079864c->flags & 0x400) != 0) {
        return 0;
    }

    if (DAT_004bef9c != 0 && (param_2 & 0x2) != 0) {
        CurrentProfile.field_45 = 2;
        LoadWatchSprite(0x127, 0x170);
        SpeechCloseFile();
        DAT_006687b0 = 0x4;
        PlayInstanceOfSample(GameFX[FX_BUTTON4].sample, 0, 1, 0);
        FUN_0048abb0();
        CleanUpFreePlay();
        KillTitleScreenSprites();
        RemoveIconGroup(0x7);
        EditMode.unk4 = 0x3;
        InitGameInterface(0x1);
        FUN_00474880();
        CurrentProfile.field_45 = 2;
    }
    return 1;
}

// FUNCTION: LEGOLAND 0x0048ad00
LEGO_EXPORT void InitFreePlayLists(void) {
    int t2 = 0;
    struct Element *t1 = 0;
    char **t3;
    char *t4;
    struct Element *build = 0;
    struct Element *common = 0;
    struct Element *legoland = 0;
    struct Element *adventurers = 0;
    struct Element *castle = 0;
    struct Element *western = 0;
    unsigned char i;
    unsigned char *p;
    int idx;
    struct Sprite *sprite;
    struct ClipQueryResult *entry;

    if (LLIDB_FindElement("BUILD MENU", (unsigned int *)&build, 0) != 0) {
        exit(1);
    }
    if (LLIDB_FindElement("COMMON THEME", (unsigned int *)&common, 0) != 0) {
        exit(1);
    }
    if (LLIDB_FindElement("LEGOLAND THEME", (unsigned int *)&legoland, 0) != 0) {
        exit(1);
    }
    if (LLIDB_FindElement("ADVENTURERS THEME", (unsigned int *)&adventurers, 0) != 0) {
        exit(1);
    }
    if (LLIDB_FindElement("CASTLE THEME", (unsigned int *)&castle, 0) != 0) {
        exit(1);
    }
    if (LLIDB_FindElement("WESTERN THEME", (unsigned int *)&western, 0) != 0) {
        exit(1);
    }
    for (i = 0, p = CurrentProfile.field_46; i < 200; i++, p++) {
        if (*p == 0) {
            continue;
        }
        idx = 0;
        for (entry = DAT_004bdeb8; (int)entry < (int)&DAT_004bdeb8[0x86]; entry++, idx++) {
            if (entry->id == i && LLIDB_FindElement(entry->name, (unsigned int *)&t1, 0) == 0) {
                break;
            }
        }
        if (idx == 0x86) {
            continue;
        }
        sprite = FUN_0047c7f0(t1, &t4, &t2, &t3);
        if (sprite == 0) {
            continue;
        }
        if (DAT_007cb3bc->name == legoland->name || DAT_007cb3bc->name == common->name) {
            Add2FreePlayPanelLists((int)sprite, &t1->name, t4, t2, t3, 200);
        } else if (DAT_007cb3bc->name == adventurers->name) {
            Add2FreePlayPanelLists((int)sprite, &t1->name, t4, t2, t3, 300);
        } else if (DAT_007cb3bc->name == castle->name) {
            Add2FreePlayPanelLists((int)sprite, &t1->name, t4, t2, t3, 400);
        } else if (DAT_007cb3bc->name == western->name) {
            Add2FreePlayPanelLists((int)sprite, &t1->name, t4, t2, t3, 500);
        } else {
            KillSprite(sprite);
        }
        free(t4);
    }
}

// FUNCTION: LEGOLAND 0x0048aef0
unsigned int FUN_0048aef0(unsigned int arg1, struct Element *arg2) {
    unsigned int result;
    struct Element *elem;

    result = FUN_0048a840(arg1, 0);
    if ((int)(DAT_007cb3a0 + result) <= 0x4e20) {
        if (arg2) {
            // STRING: LEGOLAND 0x004bb49c
            elem = ElemID("BUILD MENU");
            if (arg2 != elem) {
                if ((arg2->flags & 0x4) == 0) {
                    return 0;
                }
            }
        }
        return 1;
    }
    return 0;
}

// FUNCTION: LEGOLAND 0x0048af40
void FUN_0048af40(unsigned int param) {
    struct ClipQueryResult *node;

    node = 0;
    DAT_007cb3a0 += FUN_0048a840(param, &node);
    if (node != 0) {
        node->field_c = 1;
    }

    if (DAT_00798650++ == 0) {
        DAT_0079864c->flags &= ~0x400u;
    }
}

// FUNCTION: LEGOLAND 0x0048afa0
void FUN_0048afa0(unsigned int param) {
    struct ClipQueryResult *node;

    node = 0;
    DAT_007cb3a0 -= FUN_0048a840(param, &node);
    if (node != 0) {
        node->field_c = 0;
    }

    if (--DAT_00798650 == 0) {
        DAT_0079864c->flags |= 0x400;
    }
}

// FUNCTION: LEGOLAND 0x0048b000
unsigned char FUN_0048b000(struct IconNode *icon, unsigned int param_2) {
    struct IconNode *node;
    struct Element *elem;

    if ((param_2 & 0x2) != 0) {
        do {
            if (icon->field_18 == 0) {
                if (FUN_0048aef0((unsigned int)icon->field_1c, (struct Element *)icon->field_20p) != 0) {
                    if (DAT_00798648 == 0) {
                        PlayInstanceOfSample(GameFX[FX_CLICK1].sample, 0, 1, 0);
                    }
                    FUN_0048af40((unsigned int)icon->field_1c);
                    icon->field_18 = 1;
                    ElemID((const char *)icon->field_1c)->flags |= 0x4;
                } else {
                    PlayInstanceOfSample(GameFX[FX_RASP].sample, 0, 1, 0);
                }
            } else {
                elem = ElemID((const char *)icon->field_1c);
                for (node = IconListHead; node != 0; node = node->next) {
                    if (node->field_20p != 0 && node->field_18 == 1 && (struct Element *)node->field_20p == elem) {
                        FUN_0048afa0((unsigned int)node->field_1c);
                        node->field_18 = 0;
                    }
                }
                PlayInstanceOfSample(GameFX[FX_BUTTON14].sample, 0, 1, 0);
                FUN_0048afa0((unsigned int)icon->field_1c);
                icon->field_18 = 0;
                ElemID((const char *)icon->field_1c)->flags &= ~0x4u;
            }
        } while (0);
    }
    return 1;
}

// FUNCTION: LEGOLAND 0x0048b110
LEGO_EXPORT void Add2FreePlayPanelLists(int a, char **name, char *name2, int key, char **after, int type) {
    struct PanelNode *cur = 0;
    struct PanelNode *prev = 0;
    struct PanelNode **head = 0;
    struct PanelNode *node = (struct PanelNode *)malloc(sizeof(struct PanelNode));

    if (type == 200) {
        cur = DAT_007cb3d0;
        head = &DAT_007cb3d0;
    } else if (type == 300) {
        cur = DAT_007cb39c;
        head = &DAT_007cb39c;
    } else if (type == 400) {
        cur = DAT_007cb3b8;
        head = &DAT_007cb3b8;
    } else if (type == 500) {
        cur = DAT_007cb3a4;
        head = &DAT_007cb3a4;
    }
    node->buffer2 = malloc(strlen(name2) + 1);
    strcpy(node->buffer2, name2);
    node->buffer1 = malloc(strlen(*name) + 1);
    strcpy(node->buffer1, *name);
    node->name = name;
    node->a = a;
    node->after = after;
    node->key = key;
    node->next = 0;
    if (after != 0) {
        while (cur != 0) {
            if (cur->name == after) {
                break;
            }
            cur = cur->next;
        }
        if (cur == 0) {
            free(node->buffer1);
            free(node->buffer2);
            free(node);
            return;
        }
        prev = cur;
        cur = cur->next;
    } else {
        while (cur != 0) {
            if (cur->key > key) {
                break;
            }
            prev = cur;
            cur = cur->next;
        }
    }
    if (prev != 0) {
        prev->next = node;
        node->next = cur;
    } else {
        *head = node;
        node->next = cur;
    }
}

// FUNCTION: LEGOLAND 0x0048b2a0
LEGO_EXPORT unsigned int FreePlayObjectList(int a, int b, int c, int d, int e) {
    struct FreePlayGroup *group;
    struct IconNode *icon;
    struct PanelNode *list;
    struct Sprite *sprite2;
    struct Sprite *sprite;
    int y;

    group = (struct FreePlayGroup *)malloc(sizeof(struct FreePlayGroup));
    if (group == NULL) {
        return 0;
    }
    if (e == 0xc8) {
        sprite = FreePlayDown1Sprite;
        sprite2 = FreePlayUp1Sprite;
        list = DAT_007cb3d0;
    } else if (e == 0x1f4) {
        sprite = FreePlayDown2Sprite;
        sprite2 = FreePlayUp2Sprite;
        list = DAT_007cb3a4;
    } else if (e == 0x190) {
        sprite = FreePlayDown3Sprite;
        sprite2 = FreePlayUp3Sprite;
        list = DAT_007cb3b8;
    } else if (e == 0x12c) {
        sprite = FreePlayDown4Sprite;
        sprite2 = FreePlayUp4Sprite;
        list = DAT_007cb39c;
    } else {
        list = (struct PanelNode *)b;
    }
    if (list == NULL) {
        InsertIcon(b - 3, 0x15, 7, FreePlayCoverSprite);
        return 0;
    }
    icon = AddGBarIcons((unsigned int)group, b, c, 1, d, a);
    group->icon = icon;
    b = icon->x;
    group->clip_left = b;
    group->content_left = b;
    y = icon->y;
    group->clip_top = y;
    group->content_top = y;
    group->clip_right = icon->width + icon->x;
    group->clip_bottom = icon->height + icon->y;
    group->field_4 = 1;
    group->group = (short)a;
    SetNewGroup_Callbacks(0, (void *)RenderFreePlayIcons, (void *)FUN_0048b000);
    do {
        AddFreePlayIcon((unsigned int)group, (struct InfoSource *)list, b, y, a, 1, list->after);
        list = list->next;
        y += 0x42;
    } while (list != NULL);
    AddFullScreenIcon((void *)(a + 6));
    group->field_14 = b;
    group->content_bottom = y;
    icon = FindIcon(a + 4);
    if (icon != NULL) {
        SetIconSprite(icon, sprite);
        icon->x -= 9;
        icon->string_id = 0x94;
        icon->string = GetString(0x94);
    }
    icon = FindIcon(a + 3);
    if (icon != NULL) {
        SetIconSprite(icon, sprite2);
        icon->x -= 9;
        icon->string_id = 0x95;
        icon->string = GetString(0x95);
    }
    return 1;
}

// FUNCTION: LEGOLAND 0x0048b4a0
void FUN_0048b4a0(int arg) {
    struct PanelNode **head;
    struct PanelNode *cur;
    struct PanelNode *next;

    RemoveIconGroup((unsigned short)(arg + 6));

    if (arg == 0xc8) {
        cur = DAT_007cb3d0;
        head = &DAT_007cb3d0;
    } else if (arg == 0x12c) {
        cur = DAT_007cb39c;
        head = &DAT_007cb39c;
    } else if (arg == 0x190) {
        cur = DAT_007cb3b8;
        head = &DAT_007cb3b8;
    } else if (arg == 0x1f4) {
        cur = DAT_007cb3a4;
        head = &DAT_007cb3a4;
    } else {
        return;
    }

    if (cur == 0) {
        return;
    }
    if (cur->next != 0) {
        do {
            next = cur->next;
            free(cur->buffer1);
            free(cur->buffer2);
            free(cur);
            cur = next;
        } while (next != 0);
    }
    *head = 0;
}

// FUNCTION: LEGOLAND 0x0048b540
LEGO_EXPORT void CleanUpFreePlay(void) {
    if (FreePlayDown1Sprite) {
        KillSprite(FreePlayDown1Sprite);
        FreePlayDown1Sprite = NULL;
    }
    if (FreePlayDown2Sprite) {
        KillSprite(FreePlayDown2Sprite);
        FreePlayDown2Sprite = NULL;
    }
    if (FreePlayDown3Sprite) {
        KillSprite(FreePlayDown3Sprite);
        FreePlayDown3Sprite = NULL;
    }
    if (FreePlayDown4Sprite) {
        KillSprite(FreePlayDown4Sprite);
        FreePlayDown4Sprite = NULL;
    }
    if (FreePlayUp1Sprite) {
        KillSprite(FreePlayUp1Sprite);
        FreePlayUp1Sprite = NULL;
    }
    if (FreePlayUp2Sprite) {
        KillSprite(FreePlayUp2Sprite);
        FreePlayUp2Sprite = NULL;
    }
    if (FreePlayUp3Sprite) {
        KillSprite(FreePlayUp3Sprite);
        FreePlayUp3Sprite = NULL;
    }
    if (FreePlayUp4Sprite) {
        KillSprite(FreePlayUp4Sprite);
        FreePlayUp4Sprite = NULL;
    }
    if (FreePlayTickSprite) {
        KillSprite(FreePlayTickSprite);
        FreePlayTickSprite = NULL;
    }
    if (FreePlayCoverSprite) {
        KillSprite(FreePlayCoverSprite);
        FreePlayCoverSprite = NULL;
    }
    DestroyIconGroup(0xc8);
    DestroyIconGroup(0x12c);
    DestroyIconGroup(0x190);
    DestroyIconGroup(0x1f4);
    FUN_0048b4a0(0xc8);
    FUN_0048b4a0(0x12c);
    FUN_0048b4a0(0x190);
    FUN_0048b4a0(0x1f4);
}

// FUNCTION: LEGOLAND 0x0048b6c0
void FUN_0048b6c0(void) {
    return;
}

// FUNCTION: LEGOLAND 0x0048b6d0
void FUN_0048b6d0(void) {
    int i;

    CurrentProfile.flags[4] = 1;
    for (i = 0; i < 15; i++) {
        if (CurrentProfile.flags[4 + i] != 0) {
            CurrentProfile.flags[4 + i] = 1;
            DAT_007cb394 = i;
        }
    }
}

struct FreePlayLoadSlot {
    const char *name0;
    const char *name1;
    int pad_8[3];
    struct Sprite *sprite0;
    struct Sprite *sprite1;
};

// FUNCTION: LEGOLAND 0x0048b700
void FUN_0048b700(void) {
    int i;

    for (i = 0; i < 10; i++) {
        ProgressScreenTables.levels[i].sprite0 = LoadSprite(ProgressScreenTables.levels[i].name0, 4);
        ProgressScreenTables.levels[i].sprite1 = LoadSprite(ProgressScreenTables.levels[i].name1, 4);
    }
}

// FUNCTION: LEGOLAND 0x0048b740
void FUN_0048b740(void) {
    int *esi;

    esi = (int *)&ProgressScreenTables.levels[0].sprite1;
    do {
        ReferenceSprite((struct Sprite *)esi[-1]);
        ReferenceSprite((struct Sprite *)esi[0]);
        esi += 7;
    } while ((long)esi < (long)&ProgressScreenTables.levels[10].sprite1);
}

// FUNCTION: LEGOLAND 0x0048b770
void FUN_0048b770(void) {
    struct FreePlaySpriteSlot *slot;

    RemoveIconGroup(0x1c);
    RemoveIconGroup(0x23);
    slot = (struct FreePlaySpriteSlot *)&ProgressScreenTables.levels[0].sprite0;
    while ((int)slot < (int)&ProgressScreenTables.levels[10].sprite0) {
        while (KillSprite(slot->sprite0) == 0) {
        }
        while (KillSprite(slot->sprite1) == 0) {
        }
        slot->sprite1 = NULL;
        slot->sprite0 = NULL;
        slot++;
    }
}
