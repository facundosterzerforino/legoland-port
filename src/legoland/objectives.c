#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "globals.h"
#include "legoland.h"

#include "bricks.h"
#include "challenge.h"
#include "clipping.h"
#include "controller.h"
#include "debug_alloc.h"
#include "draw.h"
#include "gamemap.h"
#include "help.h"
#include "icon.h"
#include "interface.h"
#include "llidb.h"
#include "map_object.h"
#include "math.h"
#include "nerps.h"
#include "objectives.h"
#include "popupinfo.h"
#include "sound_music.h"
#include "sound_sfx.h"
#include "stream.h"
#include "string.h"
#include "tilemap.h"
#include "timer.h"
#include "title.h"
#include "worker.h"

#pragma intrinsic(strcpy, strlen)

struct ThemeQueryArg {
    unsigned char pad_0[0xc];
    struct ThemeData *theme;
};

struct ThemeData {
    unsigned char pad_0[0x5c];
    unsigned int theme_id;
};

struct RewardArg {
    unsigned char pad_0[0x8];
    const char *field_8;
    int reward_type;
    unsigned char pad_10[0x1c - 0x10];
    int field_1c;
};

struct MapRectArg {
    unsigned char pad_0[0x28];
    int x0;
    int y0;
    int x1;
    int y1;
};

struct NerpsTarget {
    unsigned char pad_0[0x10];
    unsigned char flags_10;
    unsigned char pad_11[0x40 - 0x11];
    unsigned int field_40;
};

struct RewardObject {
    unsigned int id;
    unsigned char pad_4[0x8 - 0x4];
    unsigned int flags;
    struct NewObjInfo *info;
};

struct PlaceObject {
    int field_0;
    unsigned char pad_4[0xc - 0x4];
    struct ObjClass *cls;
};

struct SweepInstance {
    struct PlaceObject *object;
    unsigned char tile_x;
    unsigned char tile_y;
    unsigned char pad_6[0xc - 0x6];
    unsigned char flags_c;
};

// FUNCTION: LEGOLAND 0x00468810
void FUN_00468810(char *name) {
    strncpy(DAT_0066869c, name, 0x80);
    DAT_0066869c[0x7f] = 0;
}

// FUNCTION: LEGOLAND 0x00468830
void FUN_00468830(void) {
    DAT_0066869c[0] = 0;
    DAT_0066861c[0] = 0;
}

// FUNCTION: LEGOLAND 0x00468840
void FUN_00468840(void) {
    *(unsigned int *)ObjectiveCounters = 0;
    *(unsigned int *)(ObjectiveCounters + 4) = 0;
    *(unsigned short *)(ObjectiveCounters + 8) = 0;
}

// FUNCTION: LEGOLAND 0x00468860
void FUN_00468860(int index, signed char value) {
    if (index < 10) {
        ObjectiveCounters[index] = value;
        if (index < 4) {
            FUN_00476140(index, value);
        }
    }
}

// FUNCTION: LEGOLAND 0x00468890
unsigned char FUN_00468890(int index, unsigned char value) {
    if (index < 10) {
        ObjectiveCounters[index] += value;
        return ObjectiveCounters[index];
    }
    return 0;
}

// FUNCTION: LEGOLAND 0x004688c0
char FUN_004688c0(int index) {
    if (index < 10) {
        return ObjectiveCounters[index];
    }
    return 0;
}

// FUNCTION: LEGOLAND 0x004688e0
void FUN_004688e0(void) {}

// FUNCTION: LEGOLAND 0x004688f0
void FUN_004688f0(int index, unsigned char param_2) {
    if (index < 4) {
        FUN_00460560(index);
    }
}

// FUNCTION: LEGOLAND 0x00468910
struct ObjectiveEvent *AllocObjectiveEvent(unsigned int type, int sort_key) {
    struct ObjectiveEvent *event;

    event = (struct ObjectiveEvent *)calloc(1, 0x44);
    if (event != NULL) {
        event->next = NULL;
        event->field_8 = 0;
        event->type = type;
        event->sort_key = sort_key;
    }
    return event;
}

// FUNCTION: LEGOLAND 0x00468940
void FreeObjectiveEvent(struct ObjectiveEvent *event) {
    if (event->flags_10 & 0x20) {
        if (event->field_8 != 0) {
            free((void *)event->field_8);
        }
    }
    free(event);
}

// FUNCTION: LEGOLAND 0x00468970
void FreeObjectiveEventList(struct ObjectiveEvent *event) {
    if (event != NULL) {
        if (event->next != NULL) {
            FreeObjectiveEventList(event->next);
        }
        FreeObjectiveEvent(event);
    }
}

// FUNCTION: LEGOLAND 0x004689a0
void FUN_004689a0(void) {
    int i;
    unsigned int *p;

    i = 0;
    if ((int)ScriptStringCount > 0) {
        p = ScriptStringTable;
        do {
            if (*p != 0) {
                free((void *)*p);
                *p = 0;
            }
            i++;
            p++;
        } while (i < (int)ScriptStringCount);
    }
    ScriptStringCount = 0;
}

// FUNCTION: LEGOLAND 0x004689f0
unsigned int FUN_004689f0(char *param_1, char *param_2, int param_3) {
    void *buffer;

    if (param_3 != 0) {
        if (param_1 != NULL) {
            if (param_2 != NULL) {
                buffer = malloc(strlen(param_1) + strlen(param_2) + 2);
                ScriptStringTable[ScriptStringCount] = (unsigned int)buffer;
                if (buffer != NULL) {
                    // STRING: LEGOLAND 0x004b9f90
                    sprintf(buffer, "%s%c%s", param_1, 0x40, param_2);
                }
            } else {
                buffer = malloc(strlen(param_1) + 1);
                ScriptStringTable[ScriptStringCount] = (unsigned int)buffer;
                if (buffer != NULL) {
                    sprintf(buffer, (char *)PercentSFormat, param_1);
                }
            }
        } else {
            ScriptStringTable[ScriptStringCount] = 0;
        }
    } else {
        ScriptStringTable[ScriptStringCount] = (unsigned int)param_1;
    }
    return ScriptStringCount++;
}

// FUNCTION: LEGOLAND 0x00468b00
void FUN_00468b00(struct ObjectiveEvent *event) {
    struct ObjectiveEvent *node;
    struct ObjectiveEvent *prev;
    int key;

    node = DAT_00668724;
    prev = NULL;
    if (node != NULL) {
        key = event->sort_key;
        while (node != NULL) {
            if (node->sort_key <= key) {
                break;
            }
            prev = node;
            node = node->next;
        }
        if (prev != NULL) {
            event->next = prev->next;
            prev->next = event;
        }
    }
    if (prev == NULL) {
        DAT_00668724 = event;
        event->next = NULL;
    }
}

// FUNCTION: LEGOLAND 0x00468b40
void FUN_00468b40(struct ObjectiveEvent *node, unsigned int param_2, unsigned int param_3) {
    char *buffer;

    if (param_3 != 0) {
        buffer = (char *)malloc(strlen((char *)param_2) + 1);
        node->field_8 = (unsigned int)buffer;
        strcpy(buffer, (char *)param_2);
        node->flags_10 |= 0x20;
    } else {
        node->field_8 = param_2;
        node->flags_10 &= 0xdf;
    }
}

// FUNCTION: LEGOLAND 0x00468bb0
struct ObjectiveEvent *PostObjectiveMessage(const char *format, ...) {
    struct ObjectiveEvent *event;
    va_list args;

    event = AllocObjectiveEvent(0, 1);
    if (event != NULL) {
        va_start(args, format);
        vsprintf(DAT_0066820c, format, args);
        FUN_00468b40(event, (unsigned int)DAT_0066820c, 1);
        FUN_00468b00(event);
    }
    return event;
}

// FUNCTION: LEGOLAND 0x00468c00
void FUN_00468c00(void) {
    struct ObjectiveEvent *event;
    char *at;

    event = DAT_00668724;
    if (event != NULL) {
        at = strchr((char *)event->field_8, 0x40);
        if (at != NULL) {
            *at = 0;
            SpeechCloseFile();
            SpeechLoadWavFile(at + 1);
            SpeechPlay();
            FUN_0046d3a0();
        }
        if (DisplayAdvisorHelp((char *)event->field_8, event->type == 0, 0) != 0) {
            FUN_00444070(5, 0);
            DAT_00668724 = DAT_00668724->next;
            FreeObjectiveEvent(event);
        }
    }
}

// FUNCTION: LEGOLAND 0x00468c80
void InsertObjectiveEventSorted(struct ObjectiveEvent *event) {
    struct ObjectiveEvent *node;
    struct ObjectiveEvent *prev;
    int key;

    node = ObjectiveEventList;
    prev = NULL;
    if (node != NULL) {
        key = event->sort_key;
        while (node != NULL) {
            if (node->sort_key > key) {
                break;
            }
            prev = node;
            node = node->next;
        }
    }
    if (prev != NULL) {
        event->next = prev->next;
        prev->next = event;
    } else {
        ObjectiveEventList = event;
        event->next = NULL;
    }
    DAT_0066872c[event->type] += 1;
}

// FUNCTION: LEGOLAND 0x00468cd0
struct ObjectiveEvent *AllocTimestampedObjectiveEvent(unsigned int type, int sort_key) {
    struct ObjectiveEvent *event;

    event = AllocObjectiveEvent(type, sort_key);
    if (event != NULL) {
        event->timestamp = GetGameTimer();
    }
    return event;
}

// FUNCTION: LEGOLAND 0x00468d00
void FUN_00468d00(void) {
    DAT_00668780 = GetGameTimer();
}

// FUNCTION: LEGOLAND 0x00468d10
int FUN_00468d10(void) {
    if ((int)(GetGameTimer() - DAT_00668780) > 0xc350) {
        FUN_00468d00();
        return 1;
    }
    return 0;
}

// FUNCTION: LEGOLAND 0x00468d30
int FUN_00468d30(struct NerpsArg *object) {
    struct ObjectiveEvent *event;
    struct NerpsTarget *target;

    target = (struct NerpsTarget *)object;
    if (target->field_40 != 0 && ScriptStringTable[target->field_40] != 0) {
        if (target->flags_10 & 4) {
            event = AllocTimestampedObjectiveEvent(0, 0);
        } else {
            event = AllocTimestampedObjectiveEvent(0, 1);
        }
        event->field_40 = target->field_40;
        InsertObjectiveEventSorted(event);
        return 1;
    }
    return 0;
}

// FUNCTION: LEGOLAND 0x00468d80
void FUN_00468d80(struct NerpsArg *object, unsigned int a, int b) {
    struct ObjectiveEvent *event;

    if (FUN_00468d10() == 0) {
        return;
    }
    if (FUN_00468d30(object) != 0) {
        return;
    }
    event = AllocTimestampedObjectiveEvent(1, 1);
    event->field_4 = a;
    event->field_1c = b;
    InsertObjectiveEventSorted(event);
}

// FUNCTION: LEGOLAND 0x00468dc0
void FUN_00468dc0(struct NerpsArg *object, unsigned int a) {
    struct ObjectiveEvent *event;

    if (FUN_00468d10() == 0) {
        return;
    }
    if (FUN_00468d30(object) != 0) {
        return;
    }
    event = AllocTimestampedObjectiveEvent(2, 1);
    event->field_4 = a;
    event->field_1c = 0;
    InsertObjectiveEventSorted(event);
}

// FUNCTION: LEGOLAND 0x00468e00
void FUN_00468e00(struct NerpsArg *object, unsigned int a) {
    struct ObjectiveEvent *event;

    if (FUN_00468d10() == 0) {
        return;
    }
    if (FUN_00468d30(object) != 0) {
        return;
    }
    event = AllocTimestampedObjectiveEvent(2, 1);
    event->field_4 = a;
    event->field_1c = 1;
    InsertObjectiveEventSorted(event);
}

// FUNCTION: LEGOLAND 0x00468e40
void FUN_00468e40(struct NerpsArg *arg, unsigned int class_id, int count, int sum) {
    struct ObjectiveEvent *event;

    if (FUN_00468d10() == 0) {
        return;
    }
    if (FUN_00468d30(arg) != 0) {
        return;
    }
    event = AllocTimestampedObjectiveEvent(3, 1);
    if (count < 0) {
        count = 0;
    }
    if (sum < 0) {
        sum = 0;
    }
    event->field_4 = class_id;
    event->field_14 = count;
    event->field_1c = sum;
    InsertObjectiveEventSorted(event);
}

// FUNCTION: LEGOLAND 0x00468ea0
void FUN_00468ea0(struct NerpsArg *arg, unsigned int class_id, int count, int sum) {
    struct ObjectiveEvent *event;

    if (FUN_00468d10() == 0) {
        return;
    }
    if (FUN_00468d30(arg) != 0) {
        return;
    }
    event = AllocTimestampedObjectiveEvent(4, 1);
    if (count < 0) {
        count = 0;
    }
    if (sum < 0) {
        sum = 0;
    }
    event->field_4 = class_id;
    event->field_14 = count;
    event->field_1c = sum;
    InsertObjectiveEventSorted(event);
}

// FUNCTION: LEGOLAND 0x00468f00
void FUN_00468f00(struct NerpsArg *object, int a) {
    struct ObjectiveEvent *event;

    if (FUN_00468d10() == 0) {
        return;
    }
    if (FUN_00468d30(object) != 0) {
        return;
    }
    event = AllocTimestampedObjectiveEvent(5, 1);
    event->field_1c = a;
    InsertObjectiveEventSorted(event);
}

// FUNCTION: LEGOLAND 0x00468f40
void FUN_00468f40(struct NerpsArg *arg, unsigned int class_id, int count) {
    struct ObjectiveEvent *event;

    if (FUN_00468d10() == 0) {
        return;
    }
    if (FUN_00468d30(arg) != 0) {
        return;
    }
    event = AllocTimestampedObjectiveEvent(6, 1);
    event->field_1c = count;
    event->field_4 = class_id;
    InsertObjectiveEventSorted(event);
}

// FUNCTION: LEGOLAND 0x00468f80
void FUN_00468f80(struct NerpsArg *object, int a) {
    struct ObjectiveEvent *event;

    if (FUN_00468d10() == 0) {
        return;
    }
    if (FUN_00468d30(object) != 0) {
        return;
    }
    event = AllocTimestampedObjectiveEvent(7, 1);
    event->field_1c = a;
    InsertObjectiveEventSorted(event);
}

// FUNCTION: LEGOLAND 0x00468fc0
void FUN_00468fc0(struct NerpsArg *object, int a) {
    struct ObjectiveEvent *event;

    if (FUN_00468d10() == 0) {
        return;
    }
    if (FUN_00468d30(object) != 0) {
        return;
    }
    event = AllocTimestampedObjectiveEvent(8, 1);
    event->field_1c = a;
    InsertObjectiveEventSorted(event);
}

// FUNCTION: LEGOLAND 0x00469000
void FUN_00469000(struct NerpsArg *object, int a) {
    struct ObjectiveEvent *event;

    if (FUN_00468d10() == 0) {
        return;
    }
    if (FUN_00468d30(object) != 0) {
        return;
    }
    event = AllocTimestampedObjectiveEvent(8, 1);
    event->field_1c = -a;
    InsertObjectiveEventSorted(event);
}

// FUNCTION: LEGOLAND 0x00469040
void FUN_00469040(struct NerpsArg *object, unsigned int a) {
    struct ObjectiveEvent *event;

    if (FUN_00468d10() == 0) {
        return;
    }
    if (FUN_00468d30(object) != 0) {
        return;
    }
    event = AllocTimestampedObjectiveEvent(9, 1);
    event->field_1c = a;
    InsertObjectiveEventSorted(event);
}

// FUNCTION: LEGOLAND 0x00469080
void FUN_00469080(struct NerpsArg *object, int a) {
    struct ObjectiveEvent *event;

    if (FUN_00468d10() == 0) {
        return;
    }
    if (FUN_00468d30(object) != 0) {
        return;
    }
    event = AllocTimestampedObjectiveEvent(9, 1);
    event->field_1c = -a;
    InsertObjectiveEventSorted(event);
}

// FUNCTION: LEGOLAND 0x004690c0
void FUN_004690c0(struct NerpsArg *arg, int count) {
    struct ObjectiveEvent *event;

    if (FUN_00468d10() == 0) {
        return;
    }
    if (FUN_00468d30(arg) != 0) {
        return;
    }
    event = AllocTimestampedObjectiveEvent(0xd, 1);
    event->field_1c = count;
    InsertObjectiveEventSorted(event);
}

// FUNCTION: LEGOLAND 0x00469100
void FUN_00469100(struct NerpsArg *object, int a, unsigned int b) {
    struct ObjectiveEvent *event;

    if (FUN_00468d10() == 0) {
        return;
    }
    if (FUN_00468d30(object) != 0) {
        return;
    }
    event = AllocTimestampedObjectiveEvent(0xe, 1);
    event->field_1c = a;
    event->field_14 = b;
    InsertObjectiveEventSorted(event);
}

// FUNCTION: LEGOLAND 0x00469140
void FUN_00469140(struct NerpsArg *object, unsigned int a, unsigned int b) {
    struct ObjectiveEvent *event;

    if (FUN_00468d10() == 0) {
        return;
    }
    if (FUN_00468d30(object) != 0) {
        return;
    }
    event = AllocTimestampedObjectiveEvent(0xf, 1);
    event->field_1c = a;
    event->field_14 = b;
    event->field_18 = 1;
    InsertObjectiveEventSorted(event);
}

// FUNCTION: LEGOLAND 0x00469190
void FUN_00469190(struct NerpsArg *object, unsigned int a, unsigned int b) {
    struct ObjectiveEvent *event;

    if (FUN_00468d10() == 0) {
        return;
    }
    if (FUN_00468d30(object) != 0) {
        return;
    }
    event = AllocTimestampedObjectiveEvent(0xf, 1);
    event->field_1c = a;
    event->field_14 = b;
    event->field_18 = 0;
    InsertObjectiveEventSorted(event);
}

// FUNCTION: LEGOLAND 0x004691e0
void FUN_004691e0(struct NerpsArg *arg, int param_2, unsigned int param_3) {
    struct ObjectiveEvent *event;

    if (FUN_00468d10() == 0) {
        return;
    }
    if (FUN_00468d30(arg) != 0) {
        return;
    }
    event = AllocTimestampedObjectiveEvent(0x10, 1);
    event->field_1c = param_2;
    event->field_14 = param_3;
    InsertObjectiveEventSorted(event);
}

// FUNCTION: LEGOLAND 0x00469220
void FUN_00469220(struct NerpsArg *object, unsigned int a, unsigned int b) {
    struct ObjectiveEvent *event;

    if (FUN_00468d10() == 0) {
        return;
    }
    if (FUN_00468d30(object) != 0) {
        return;
    }
    event = AllocTimestampedObjectiveEvent(0x11, 1);
    event->field_4 = a;
    event->field_1c = b;
    InsertObjectiveEventSorted(event);
}

// FUNCTION: LEGOLAND 0x00469260
void FUN_00469260(struct NerpsArg *arg, unsigned int class_id, int sum, int count) {
    struct ObjectiveEvent *event;

    if (FUN_00468d10() == 0) {
        return;
    }
    if (FUN_00468d30(arg) != 0) {
        return;
    }
    event = AllocTimestampedObjectiveEvent(0x12, 1);
    if (sum < 0) {
        sum = 0;
    }
    if (count < 0) {
        count = 0;
    }
    event->field_4 = class_id;
    event->field_14 = count;
    event->field_1c = sum;
    InsertObjectiveEventSorted(event);
}

// FUNCTION: LEGOLAND 0x00469310
void FUN_00469310(struct NerpsArg *object, unsigned int a, int b) {
    struct ObjectiveEvent *event;

    if (FUN_00468d10() == 0) {
        return;
    }
    if (FUN_00468d30(object) != 0) {
        return;
    }
    event = AllocTimestampedObjectiveEvent(0xa, 1);
    event->field_14 = b;
    event->field_1c = a;
    InsertObjectiveEventSorted(event);
}

// FUNCTION: LEGOLAND 0x00469350
void FUN_00469350(struct NerpsArg *object, int a) {
    struct ObjectiveEvent *event;

    if (FUN_00468d10() == 0) {
        return;
    }
    if (FUN_00468d30(object) != 0) {
        return;
    }
    event = AllocTimestampedObjectiveEvent(0xb, 1);
    event->field_14 = a;
    InsertObjectiveEventSorted(event);
}

// FUNCTION: LEGOLAND 0x00469390
void FUN_00469390(struct NerpsArg *object) {
    if (FUN_00468d10() != 0) {
        FUN_00468d30(object);
    }
}

// FUNCTION: LEGOLAND 0x004693b0
void FUN_004693b0(unsigned int type) {
    struct ObjectiveEvent *node;
    struct ObjectiveEvent *next;
    struct ObjectiveEvent *prev;

    prev = NULL;
    node = ObjectiveEventList;
    while (node != NULL) {
        next = node->next;
        if (node->type == type) {
            if (prev != NULL) {
                prev->next = next;
            } else {
                ObjectiveEventList = next;
            }
            FreeObjectiveEvent(node);
        } else {
            prev = node;
        }
        node = next;
    }
}

// FUNCTION: LEGOLAND 0x00469400
void FUN_00469400(void) {
    struct ObjectiveEvent *node;

    GetGameTimer();
    do {
        node = ObjectiveEventList;
        if (node != NULL) {
            switch (node->type) {
            case 0:
                PostObjectiveMessage((char *)PercentSFormat, ScriptStringTable[node->field_40]);
                DAT_00668614 = node->field_40;
                break;
            case 1:
                // STRING: LEGOLAND 0x004ba6bc
                PostObjectiveMessage("You need to build %d more of object %s", node->field_1c,
                    ((struct PlaceObject *)node->field_4)->cls->name);
                break;
            case 0xc:
                // STRING: LEGOLAND 0x004ba69c
                PostObjectiveMessage("You need to research object %s", ((struct PlaceObject *)node->field_4)->cls->name);
                break;
            case 2:
                if (node->field_1c == 0) {
                    if (node->field_4 != 0) {
                        // STRING: LEGOLAND 0x004ba674
                        PostObjectiveMessage("You need to connect your %s to a path",
                            ((struct PlaceObject *)node->field_4)->cls->name);
                    } else {
                        // STRING: LEGOLAND 0x004ba648
                        PostObjectiveMessage("You need to connect all objects to a path");
                    }
                } else {
                    if (node->field_4 != 0) {
                        // STRING: LEGOLAND 0x004ba60c
                        PostObjectiveMessage("You need to link the path from your %s to the park entrance",
                            ((struct PlaceObject *)node->field_4)->cls->name);
                    } else {
                        // STRING: LEGOLAND 0x004ba5c8
                        PostObjectiveMessage("You need to link the paths from all objects to the park entrance.");
                    }
                }
                break;
            case 3:
                if (node->field_14 != 0) {
                    // STRING: LEGOLAND 0x004ba590
                    PostObjectiveMessage("You need to build %d new attractions from the %s range", node->field_14,
                        *(unsigned int *)node->field_4);
                } else {
                    // STRING: LEGOLAND 0x004ba558
                    PostObjectiveMessage("You need to build %d more attractions from the %s range", node->field_1c,
                        *(unsigned int *)node->field_4);
                }
                break;
            case 4:
                if (node->field_14 != 0) {
                    // STRING: LEGOLAND 0x004ba510
                    PostObjectiveMessage("You need to delete all instances of %d attractions from the %s range", node->field_14,
                        *(unsigned int *)node->field_4);
                } else {
                    // STRING: LEGOLAND 0x004ba4dc
                    PostObjectiveMessage("You need delete %d attractions from the %s range", node->field_1c,
                        *(unsigned int *)node->field_4);
                }
                break;
            case 5:
                // STRING: LEGOLAND 0x004ba4b0
                PostObjectiveMessage("You need to remove %d items from the area", node->field_1c);
                break;
            case 6:
                // STRING: LEGOLAND 0x004ba48c
                PostObjectiveMessage("You need to delete %d of object %s", node->field_1c,
                    ((struct PlaceObject *)node->field_4)->cls->name);
                break;
            case 7:
                // STRING: LEGOLAND 0x004ba45c
                PostObjectiveMessage("You need to attract %d more people to your park", node->field_1c);
                break;
            case 8:
                if ((int)node->field_1c > 0) {
                    // STRING: LEGOLAND 0x004ba428
                    PostObjectiveMessage("You need %d more gardeners to look after your park", node->field_1c);
                } else {
                    // STRING: LEGOLAND 0x004ba3fc
                    PostObjectiveMessage("You need %d fewer gardeners in your park", -(int)node->field_1c);
                }
                break;
            case 9:
                if ((int)node->field_1c > 0) {
                    // STRING: LEGOLAND 0x004ba3cc
                    PostObjectiveMessage("You need %d more mechanics to help in the park", node->field_1c);
                } else {
                    // STRING: LEGOLAND 0x004ba3a0
                    PostObjectiveMessage("You need %d fewer mechanics in your park", -(int)node->field_1c);
                }
                break;
            case 0xa:
                switch (node->field_1c) {
                case 0:
                    // STRING: LEGOLAND 0x004ba370
                    PostObjectiveMessage("You need to cover %d more squares with objects", node->field_14);
                    break;
                case 1:
                    // STRING: LEGOLAND 0x004ba340
                    PostObjectiveMessage("You need to cover %d more squares with rides", node->field_14);
                    break;
                case 4:
                    // STRING: LEGOLAND 0x004ba310
                    PostObjectiveMessage("You need to cover %d more squares with shops", node->field_14);
                    break;
                case 5:
                    // STRING: LEGOLAND 0x004ba2dc
                    PostObjectiveMessage("You need to cover %d more squares with food outlets", node->field_14);
                    break;
                case 2:
                    // STRING: LEGOLAND 0x004ba2ac
                    PostObjectiveMessage("You need to cover %d more squares with scenery", node->field_14);
                    break;
                case 3:
                    // STRING: LEGOLAND 0x004ba26c
                    PostObjectiveMessage("You need to cover %d more squares with stop 'n' wonder objects", node->field_14);
                    break;
                }
                break;
            case 0xb:
                // STRING: LEGOLAND 0x004ba234
                PostObjectiveMessage("You need to line %d%% more of your path with scenery", node->field_14);
                break;
            case 0xd:
                // STRING: LEGOLAND 0x004ba210
                PostObjectiveMessage("You need to save up %d more coins.", node->field_1c);
                break;
            case 0xe:
                // STRING: LEGOLAND 0x004ba1dc
                PostObjectiveMessage("You need make %d people up to happiness level %d", node->field_1c, node->field_14);
                break;
            case 0xf:
                if (node->field_18 != 0) {
                    // STRING: LEGOLAND 0x004ba198
                    PostObjectiveMessage("You need to get %d fewer people with hunger levels greater than %d", node->field_1c,
                        node->field_14);
                } else {
                    // STRING: LEGOLAND 0x004ba15c
                    PostObjectiveMessage("You need to get %d more people with hunger level below %d", node->field_1c,
                        node->field_14);
                }
                break;
            case 0x10:
                // STRING: LEGOLAND 0x004ba110
                PostObjectiveMessage("You need make repairs to %d objects to bring them to above %d%% of health", node->field_1c,
                    node->field_14);
                break;
            case 0x11:
                // STRING: LEGOLAND 0x004ba0e4
                PostObjectiveMessage("You need to get %d more people on the %s", node->field_1c,
                    ((struct PlaceObject *)node->field_4)->cls->name);
                break;
            case 0x12:
                if (node->field_14 != 0) {
                    // STRING: LEGOLAND 0x004ba0b4
                    PostObjectiveMessage("You need to add %d different parts to the %s", node->field_14,
                        ((struct PlaceObject *)node->field_4)->cls->name);
                } else {
                    // STRING: LEGOLAND 0x004ba08c
                    PostObjectiveMessage("You need to add %d more parts to the %s", node->field_1c,
                        ((struct PlaceObject *)node->field_4)->cls->name);
                }
                break;
            case 0x13:
                switch (node->field_18) {
                case 0:
                    // STRING: LEGOLAND 0x004ba050
                    PostObjectiveMessage("You need to improve the LEGOLAND zoning (from %d%% to %d%%)", node->field_1c,
                        node->field_14);
                    break;
                case 1:
                    // STRING: LEGOLAND 0x004ba010
                    PostObjectiveMessage("You need to improve the ADVENTURER zoning (from %d%% to %d%%)", node->field_1c,
                        node->field_14);
                    break;
                case 2:
                    // STRING: LEGOLAND 0x004b9fd4
                    PostObjectiveMessage("You need to  inprove the CASTLE zoning (from %d%% to %d%%)", node->field_1c,
                        node->field_14);
                    break;
                case 3:
                    // STRING: LEGOLAND 0x004b9f98
                    PostObjectiveMessage("You need to improve the WESTERN zoning (from %d%% to %d%%)", node->field_1c,
                        node->field_14);
                    break;
                }
                break;
            }
            FUN_004693b0(node->type);
        }
    } while (node != NULL);
}

// FUNCTION: LEGOLAND 0x00469900
void FUN_00469900(struct NerpsArg *object, unsigned int a, unsigned int b) {
    struct RewardObject *obj;
    unsigned int flags;

    obj = (struct RewardObject *)object;
    if (obj != NULL) {
        flags = obj->flags;
        if ((flags & 1) != 0) {
            if ((flags & 0x10002) != 2) {
                obj->flags = (flags & 0xfffeffff) | 2;
                FUN_0048a6e0((struct ClippedObject *)obj);
                if (a != 0) {
                    AddNewObjectIcon(obj->info);
                }
            } else {
                // STRING: LEGOLAND 0x004ba6e4
                DBPrintf("Not giving %d.. Already got it\n", obj->id);
            }
        }
    }
    DAT_0066871c = 1;
}

// FUNCTION: LEGOLAND 0x00469980
void CountObjectTheme(struct ThemeQueryArg *arg) {
    struct ThemeData *theme;
    unsigned int id;

    theme = arg->theme;
    // STRING: LEGOLAND 0x004ba748
    if (LLIDB_FindElement("LEGOLAND THEME", &id, 0) == 0) {
        if (theme->theme_id == id) {
            LegolandCommonThemeCount++;
            return;
        }
        // STRING: LEGOLAND 0x004ba738
        if (LLIDB_FindElement("COMMON THEME", &id, 0) == 0 && theme->theme_id == id) {
            LegolandCommonThemeCount++;
            return;
        }
    }
    // STRING: LEGOLAND 0x004ba728
    if (LLIDB_FindElement("WESTERN THEME", &id, 0) == 0 && theme->theme_id == id) {
        WesternThemeCount++;
        return;
    }
    // STRING: LEGOLAND 0x004ba718
    if (LLIDB_FindElement("CASTLE THEME", &id, 0) == 0 && theme->theme_id == id) {
        CastleThemeCount++;
        return;
    }
    // STRING: LEGOLAND 0x004ba704
    if (LLIDB_FindElement("ADVENTURERS THEME", &id, 0) == 0 && theme->theme_id == id) {
        AdventurersThemeCount++;
    }
}

// FUNCTION: LEGOLAND 0x00469a80
void FUN_00469a80(struct NerpsArg *object) {
    unsigned int flags;

    if (object != NULL) {
        flags = ((struct ObjectiveEvent *)object)->field_8;
        if (flags & 1) {
            ((struct ObjectiveEvent *)object)->field_8 = (flags & 0xfffffffd) | 0x10000;
            CountObjectTheme((struct ThemeQueryArg *)object);
        }
    }
    DAT_0066871c = 1;
}

// FUNCTION: LEGOLAND 0x00469ab0
void FUN_00469ab0(struct NerpsArg *object) {
    unsigned int flags;

    if (object != NULL) {
        flags = ((struct ObjectiveEvent *)object)->field_8;
        if (flags & 1) {
            ((struct ObjectiveEvent *)object)->field_8 = flags & 0xfffefffd;
        }
    }
    DAT_0066871c = 1;
}

// FUNCTION: LEGOLAND 0x00469ae0
int ProcessUnimplementedReward(struct RewardArg *arg) {
    // STRING: LEGOLAND 0x004ba758
    DBPrintf("Processing unimplemented reward [ Type = %d ]\n", arg->reward_type);
    return 1;
}

// FUNCTION: LEGOLAND 0x00469b00
int ProcessUnimplementedObjective(struct RewardArg *arg) {
    // STRING: LEGOLAND 0x004ba788
    DBPrintf("Processing unimplemented objective [ Type = %d ]\n", arg->reward_type);
    return 1;
}

// FUNCTION: LEGOLAND 0x00469b20
int ScriptEventGive(struct ObjectiveEvent *event) {
    struct NerpsArg *object;

    object = (struct NerpsArg *)event->field_4;
    FUN_00469900(object, event->field_14, 0);
    object = (struct NerpsArg *)event->field_4;
    ((struct ObjectiveEvent *)object)->field_8 |= 0x20000;
    return 1;
}

// FUNCTION: LEGOLAND 0x00469b50
int FUN_00469b50(struct ObjectiveEvent *event) {
    FUN_00469a80((struct NerpsArg *)event->field_4);
    return 1;
}

// FUNCTION: LEGOLAND 0x00469b70
int ScriptEventTake(struct ObjectiveEvent *event) {
    FUN_00469ab0((struct NerpsArg *)event->field_4);
    return 1;
}

// FUNCTION: LEGOLAND 0x00469b90
int ObjectiveEventAddBricks(struct ObjectiveEvent *event) {
    SetBrickCount(GetBrickCount() + event->field_1c);
    return 1;
}

// FUNCTION: LEGOLAND 0x00469bb0
int ObjectiveEventSetBricks(struct ObjectiveEvent *event) {
    SetBrickCount(event->field_1c);
    return 1;
}

// FUNCTION: LEGOLAND 0x00469bd0
void FUN_00469bd0(unsigned int a, void *b) {
    struct PlaceObject *object;
    struct ObjClass *cls;
    struct Point centre;

    object = (struct PlaceObject *)a;
    cls = object->cls;
    SetBricksLimited(0);
    GetTileCentre((struct Point *)b, &centre.x);
    EditCursor.tile_x = centre.x;
    EditCursor.tile_y = centre.y;
    cls->method_90(a, &centre, 0x8f8);
    FUN_0045d770(&EditCursor);
    PutObjOnMap(object->cls, a, (struct Point *)&EditCursor.tile_x);
    SetBricksLimited(1);
}

// FUNCTION: LEGOLAND 0x00469c40
int ScriptEventPlace(struct ObjectiveEvent *event) {
    FUN_00469bd0(event->field_4, &event->field_20);
    return 1;
}

// FUNCTION: LEGOLAND 0x00469c60
int FUN_00469c60(unsigned int handle) {
    SetSampleFade((struct Sample *)handle, -100);
    return 0;
}

// FUNCTION: LEGOLAND 0x00469c80
int ScriptEventClear(struct MapRectArg *arg) {
    struct SweepInstance *next;
    RECT rect;
    short tile_x;
    struct SweepInstance *current;
    struct ObjClass *cls;
    short tile_y;
    struct Cursor saved;
    struct Point point;
    struct Sample *sample;
    void *saved_class;
    int power;
    int phase;

    int right;
    sample = PlayInstanceOfSample(GameFX[FX_INVENTORY_OUT].sample, 1, 1, 0);
    FUN_00496d10(sample);
    AddSFX_Callback((struct CallbackEntry *)sample, 3000, (unsigned int (*)(struct CallbackEntry *))FUN_00469c60);
    for (phase = 0; phase < 3; phase++) {
        current = (struct SweepInstance *)GetFirstRenderObject();
        while (current != NULL) {
            UpdateSound();
            switch (phase) {
            case 0:
                next = current;
                do {
                    next = (struct SweepInstance *)GetNextRenderObject((MapElement *)next);
                } while (next != NULL && (cls = next->object->cls, power = FindObjectsPower(cls), cls->field_58 != NULL) &&
                    (0x10 & cls->field_58[8]) != 0 && power <= 0);
                break;
            case 1:
                next = current;
                do {
                    next = (struct SweepInstance *)GetNextRenderObject((MapElement *)next);
                } while (next != NULL && FindObjectsPower(next->object->cls) <= 0);
                break;
            case 2:
                next = (struct SweepInstance *)GetNextRenderObject((MapElement *)current);
                break;
            }
            if (current->flags_c & 0x80) {
                cls = current->object->cls;
                tile_x = current->tile_x;
                tile_y = current->tile_y;
                rect.top = cls->footprint.v[1];
                rect.bottom = cls->footprint.v[3];
                rect.top += tile_y;
                right = cls->footprint.v[2];
                rect.right = right;
                rect.right += tile_x;
                rect.bottom += tile_y;
                rect.left = cls->footprint.v[0] + tile_x;
                if (rect.left <= arg->x1 && rect.right >= arg->x0 && rect.top <= arg->y1 && rect.bottom >= arg->y0) {
                    point.y = tile_y;
                    saved_class = QueryClass;
                    memcpy(&saved, &QueryCursor, sizeof(struct Cursor));
                    QueryCursor.tile_y = tile_y;
                    point.x = tile_x;
                    QueryCursor.tile_x = point.x;
                    QueryObj.pos.x = (unsigned char)point.x;
                    QueryClass = cls;
                    QueryObj.pos.y = (unsigned char)point.y;
                    cls->method_94(cls->element, &point);
                    BuildCursorPtr(&QueryCursor, 0, 0);
                    if (FUN_0045f4b0(&QueryCursor) != 0) {
                        FUN_0045d3d0(QueryClass, &point.x);
                        RemObjFromMap(QueryClass, (unsigned int)(QueryClass)->element, QueryObj,
                            &QueryCursor);
                    }
                    memcpy(&QueryCursor, &saved, sizeof(struct Cursor));
                    QueryClass = saved_class;
                }
            }
            current = next;
        }
    }
    CalculateMapRenderOrder();
    PlayInstanceOfSample(GameFX[FX_INVENTORY_OUT].sample, 0, 1, 0);
    return 1;
}

// FUNCTION: LEGOLAND 0x00469ed0
int ScriptEventUnglue(struct MapRectArg *arg) {
    int y;
    int x;

    y = arg->y0;
    while (y <= arg->y1) {
        x = arg->x0;
        while (x <= arg->x1) {
            GameMap[y][x].flags &= 0xffbf;
            x++;
        }
        y++;
    }
    return 1;
}

// FUNCTION: LEGOLAND 0x00469f20
int ScriptEventGlue(struct MapRectArg *arg) {
    int y;
    int x;

    y = arg->y0;
    while (y <= arg->y1) {
        x = arg->x0;
        while (x <= arg->x1) {
            GameMap[y][x].flags |= 0x40;
            x++;
        }
        y++;
    }
    return 1;
}

// FUNCTION: LEGOLAND 0x00469f70
int ScriptEventExtendPark(struct RewardArg *arg) {
    return ProcessUnimplementedReward(arg);
}

// FUNCTION: LEGOLAND 0x00469f80
int ScriptEventFmv(struct RewardArg *arg) {
    PauseGameTimer();
    SetPointer(0);
    FUN_00496e60(1, 0xf);
    PlayMovie(arg->field_8, 1, 1);
    ResumeGameTimer();
    ClearAdvisorHelp();
    FUN_0046b760();
    return 1;
}

// FUNCTION: LEGOLAND 0x00469fc0
int ScriptEventInterval(struct RewardArg *arg) {
    int retries;

    FUN_00490600(0);
    if (FUN_004907a0(arg->field_8) != 0) {
        EditMode.unk4 = 2;
        DAT_00668e38 = 1;
        DAT_0080ff80.unk4 = 0xffffffff;
        DAT_0080ff80.unk8 = 7;
        return 1;
    }
    retries = arg->field_1c;
    arg->field_1c = retries + 1;
    if (retries < 3) {
        return 0;
    }
    // STRING: LEGOLAND 0x004ba7bc
    DBPrintf("Giving up trying to load Interval %s\n", arg->field_8);
    return 1;
}

// FUNCTION: LEGOLAND 0x0046a030
int ScriptEventMessage(struct RewardArg *arg) {
    return ProcessUnimplementedReward(arg);
}
