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

struct SweepInstance {
    struct Element *object;
    unsigned char tile_x;
    unsigned char tile_y;
    unsigned char pad_6[0xc - 0x6];
    unsigned char flags_c;
};

// FUNCTION: LEGOLAND 0x00468810
void SetHintsFileName(char *name) {
    strncpy(DAT_0066869c, name, 0x80);
    DAT_0066869c[0x7f] = 0;
}

// FUNCTION: LEGOLAND 0x00468830
void ClearBriefingAndHintsFileNames(void) {
    DAT_0066869c[0] = 0;
    DAT_0066861c[0] = 0;
}

// FUNCTION: LEGOLAND 0x00468840
void ClearObjectiveCounters(void) {
    *(unsigned int *)ObjectiveCounters = 0;
    *(unsigned int *)(ObjectiveCounters + 4) = 0;
    *(unsigned short *)(ObjectiveCounters + 8) = 0;
}

// FUNCTION: LEGOLAND 0x00468860
void SetObjectiveCounter(int index, signed char value) {
    if (index < 10) {
        ObjectiveCounters[index] = value;
        if (index < 4) {
            SetObjectiveIconShown(index, value);
        }
    }
}

// FUNCTION: LEGOLAND 0x00468890
unsigned char AddObjectiveCounter(int index, unsigned char value) {
    if (index < 10) {
        ObjectiveCounters[index] += value;
        return ObjectiveCounters[index];
    }
    return 0;
}

// FUNCTION: LEGOLAND 0x004688c0
char GetObjectiveCounter(int index) {
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

    event = (struct ObjectiveEvent *)calloc(1, sizeof(struct ObjectiveEvent));
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
            free(event->text);
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
void FreeScriptStrings(void) {
    int i;
    char **p;

    i = 0;
    if ((int)ScriptStringCount > 0) {
        p = ScriptStringTable;
        do {
            if (*p != 0) {
                free(*p);
                *p = 0;
            }
            i++;
            p++;
        } while (i < (int)ScriptStringCount);
    }
    ScriptStringCount = 0;
}

// FUNCTION: LEGOLAND 0x004689f0
unsigned int AddScriptString(char *param_1, char *param_2, int param_3) {
    void *buffer;

    if (param_3 != 0) {
        if (param_1 != NULL) {
            if (param_2 != NULL) {
                buffer = malloc(strlen(param_1) + strlen(param_2) + 2);
                ScriptStringTable[ScriptStringCount] = (char *)buffer;
                if (buffer != NULL) {
                    // STRING: LEGOLAND 0x004b9f90
                    sprintf(buffer, "%s%c%s", param_1, 0x40, param_2);
                }
            } else {
                buffer = malloc(strlen(param_1) + 1);
                ScriptStringTable[ScriptStringCount] = (char *)buffer;
                if (buffer != NULL) {
                    sprintf(buffer, (char *)PercentSFormat, param_1);
                }
            }
        } else {
            ScriptStringTable[ScriptStringCount] = 0;
        }
    } else {
        ScriptStringTable[ScriptStringCount] = (char *)param_1;
    }
    return ScriptStringCount++;
}

// FUNCTION: LEGOLAND 0x00468b00
void InsertObjectiveMessageSorted(struct ObjectiveEvent *event) {
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
void SetObjectiveEventText(struct ObjectiveEvent *node, char *param_2, unsigned int param_3) {
    char *buffer;

    if (param_3 != 0) {
        buffer = (char *)malloc(strlen(param_2) + 1);
        node->text = buffer;
        strcpy(buffer, param_2);
        node->flags_10 |= 0x20;
    } else {
        node->text = param_2;
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
        SetObjectiveEventText(event, (char *)DAT_0066820c, 1);
        InsertObjectiveMessageSorted(event);
    }
    return event;
}

// FUNCTION: LEGOLAND 0x00468c00
void ShowNextObjectiveMessage(void) {
    struct ObjectiveEvent *event;
    char *at;

    event = DAT_00668724;
    if (event != NULL) {
        at = strchr(event->text, 0x40);
        if (at != NULL) {
            *at = 0;
            SpeechCloseFile();
            SpeechLoadWavFile(at + 1);
            SpeechPlay();
            FUN_0046d3a0();
        }
        if (DisplayAdvisorHelp(event->text, event->type == 0, 0) != 0) {
            SetNextAdvisorAnimState(5, 0);
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
void ResetObjectiveHintTimer(void) {
    DAT_00668780 = GetGameTimer();
}

// FUNCTION: LEGOLAND 0x00468d10
int IsObjectiveHintDue(void) {
    if ((int)(GetGameTimer() - DAT_00668780) > 0xc350) {
        ResetObjectiveHintTimer();
        return 1;
    }
    return 0;
}

// FUNCTION: LEGOLAND 0x00468d30
int QueueCustomObjectiveHint(struct ObjectiveEvent *object) {
    struct ObjectiveEvent *event;
    struct ObjectiveEvent *target;

    target = (struct ObjectiveEvent *)object;
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
void QueueBuildMoreObjectsHint(struct ObjectiveEvent *object, unsigned int a, int b) {
    struct ObjectiveEvent *event;

    if (IsObjectiveHintDue() == 0) {
        return;
    }
    if (QueueCustomObjectiveHint(object) != 0) {
        return;
    }
    event = AllocTimestampedObjectiveEvent(1, 1);
    event->field_4 = a;
    event->field_1c = b;
    InsertObjectiveEventSorted(event);
}

// FUNCTION: LEGOLAND 0x00468dc0
void QueueConnectToPathHint(struct ObjectiveEvent *object, unsigned int a) {
    struct ObjectiveEvent *event;

    if (IsObjectiveHintDue() == 0) {
        return;
    }
    if (QueueCustomObjectiveHint(object) != 0) {
        return;
    }
    event = AllocTimestampedObjectiveEvent(2, 1);
    event->field_4 = a;
    event->field_1c = 0;
    InsertObjectiveEventSorted(event);
}

// FUNCTION: LEGOLAND 0x00468e00
void QueueLinkToEntranceHint(struct ObjectiveEvent *object, unsigned int a) {
    struct ObjectiveEvent *event;

    if (IsObjectiveHintDue() == 0) {
        return;
    }
    if (QueueCustomObjectiveHint(object) != 0) {
        return;
    }
    event = AllocTimestampedObjectiveEvent(2, 1);
    event->field_4 = a;
    event->field_1c = 1;
    InsertObjectiveEventSorted(event);
}

// FUNCTION: LEGOLAND 0x00468e40
void QueueBuildRangeAttractionsHint(struct ObjectiveEvent *arg, unsigned int class_id, int count, int sum) {
    struct ObjectiveEvent *event;

    if (IsObjectiveHintDue() == 0) {
        return;
    }
    if (QueueCustomObjectiveHint(arg) != 0) {
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
void QueueDeleteRangeAttractionsHint(struct ObjectiveEvent *arg, unsigned int class_id, int count, int sum) {
    struct ObjectiveEvent *event;

    if (IsObjectiveHintDue() == 0) {
        return;
    }
    if (QueueCustomObjectiveHint(arg) != 0) {
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
void QueueClearAreaHint(struct ObjectiveEvent *object, int a) {
    struct ObjectiveEvent *event;

    if (IsObjectiveHintDue() == 0) {
        return;
    }
    if (QueueCustomObjectiveHint(object) != 0) {
        return;
    }
    event = AllocTimestampedObjectiveEvent(5, 1);
    event->field_1c = a;
    InsertObjectiveEventSorted(event);
}

// FUNCTION: LEGOLAND 0x00468f40
void QueueDeleteObjectsHint(struct ObjectiveEvent *arg, unsigned int class_id, int count) {
    struct ObjectiveEvent *event;

    if (IsObjectiveHintDue() == 0) {
        return;
    }
    if (QueueCustomObjectiveHint(arg) != 0) {
        return;
    }
    event = AllocTimestampedObjectiveEvent(6, 1);
    event->field_1c = count;
    event->field_4 = class_id;
    InsertObjectiveEventSorted(event);
}

// FUNCTION: LEGOLAND 0x00468f80
void QueueAttractVisitorsHint(struct ObjectiveEvent *object, int a) {
    struct ObjectiveEvent *event;

    if (IsObjectiveHintDue() == 0) {
        return;
    }
    if (QueueCustomObjectiveHint(object) != 0) {
        return;
    }
    event = AllocTimestampedObjectiveEvent(7, 1);
    event->field_1c = a;
    InsertObjectiveEventSorted(event);
}

// FUNCTION: LEGOLAND 0x00468fc0
void QueueMoreGardenersHint(struct ObjectiveEvent *object, int a) {
    struct ObjectiveEvent *event;

    if (IsObjectiveHintDue() == 0) {
        return;
    }
    if (QueueCustomObjectiveHint(object) != 0) {
        return;
    }
    event = AllocTimestampedObjectiveEvent(8, 1);
    event->field_1c = a;
    InsertObjectiveEventSorted(event);
}

// FUNCTION: LEGOLAND 0x00469000
void QueueFewerGardenersHint(struct ObjectiveEvent *object, int a) {
    struct ObjectiveEvent *event;

    if (IsObjectiveHintDue() == 0) {
        return;
    }
    if (QueueCustomObjectiveHint(object) != 0) {
        return;
    }
    event = AllocTimestampedObjectiveEvent(8, 1);
    event->field_1c = -a;
    InsertObjectiveEventSorted(event);
}

// FUNCTION: LEGOLAND 0x00469040
void QueueMoreMechanicsHint(struct ObjectiveEvent *object, unsigned int a) {
    struct ObjectiveEvent *event;

    if (IsObjectiveHintDue() == 0) {
        return;
    }
    if (QueueCustomObjectiveHint(object) != 0) {
        return;
    }
    event = AllocTimestampedObjectiveEvent(9, 1);
    event->field_1c = a;
    InsertObjectiveEventSorted(event);
}

// FUNCTION: LEGOLAND 0x00469080
void QueueFewerMechanicsHint(struct ObjectiveEvent *object, int a) {
    struct ObjectiveEvent *event;

    if (IsObjectiveHintDue() == 0) {
        return;
    }
    if (QueueCustomObjectiveHint(object) != 0) {
        return;
    }
    event = AllocTimestampedObjectiveEvent(9, 1);
    event->field_1c = -a;
    InsertObjectiveEventSorted(event);
}

// FUNCTION: LEGOLAND 0x004690c0
void QueueSaveCoinsHint(struct ObjectiveEvent *arg, int count) {
    struct ObjectiveEvent *event;

    if (IsObjectiveHintDue() == 0) {
        return;
    }
    if (QueueCustomObjectiveHint(arg) != 0) {
        return;
    }
    event = AllocTimestampedObjectiveEvent(0xd, 1);
    event->field_1c = count;
    InsertObjectiveEventSorted(event);
}

// FUNCTION: LEGOLAND 0x00469100
void QueueHappyVisitorsHint(struct ObjectiveEvent *object, int a, unsigned int b) {
    struct ObjectiveEvent *event;

    if (IsObjectiveHintDue() == 0) {
        return;
    }
    if (QueueCustomObjectiveHint(object) != 0) {
        return;
    }
    event = AllocTimestampedObjectiveEvent(0xe, 1);
    event->field_1c = a;
    event->field_14 = b;
    InsertObjectiveEventSorted(event);
}

// FUNCTION: LEGOLAND 0x00469140
void QueueFewerHungryVisitorsHint(struct ObjectiveEvent *object, unsigned int a, unsigned int b) {
    struct ObjectiveEvent *event;

    if (IsObjectiveHintDue() == 0) {
        return;
    }
    if (QueueCustomObjectiveHint(object) != 0) {
        return;
    }
    event = AllocTimestampedObjectiveEvent(0xf, 1);
    event->field_1c = a;
    event->field_14 = b;
    event->field_18 = 1;
    InsertObjectiveEventSorted(event);
}

// FUNCTION: LEGOLAND 0x00469190
void QueueMoreFedVisitorsHint(struct ObjectiveEvent *object, unsigned int a, unsigned int b) {
    struct ObjectiveEvent *event;

    if (IsObjectiveHintDue() == 0) {
        return;
    }
    if (QueueCustomObjectiveHint(object) != 0) {
        return;
    }
    event = AllocTimestampedObjectiveEvent(0xf, 1);
    event->field_1c = a;
    event->field_14 = b;
    event->field_18 = 0;
    InsertObjectiveEventSorted(event);
}

// FUNCTION: LEGOLAND 0x004691e0
void QueueRepairObjectsHint(struct ObjectiveEvent *arg, int param_2, unsigned int param_3) {
    struct ObjectiveEvent *event;

    if (IsObjectiveHintDue() == 0) {
        return;
    }
    if (QueueCustomObjectiveHint(arg) != 0) {
        return;
    }
    event = AllocTimestampedObjectiveEvent(0x10, 1);
    event->field_1c = param_2;
    event->field_14 = param_3;
    InsertObjectiveEventSorted(event);
}

// FUNCTION: LEGOLAND 0x00469220
void QueueRideVisitorsHint(struct ObjectiveEvent *object, unsigned int a, unsigned int b) {
    struct ObjectiveEvent *event;

    if (IsObjectiveHintDue() == 0) {
        return;
    }
    if (QueueCustomObjectiveHint(object) != 0) {
        return;
    }
    event = AllocTimestampedObjectiveEvent(0x11, 1);
    event->field_4 = a;
    event->field_1c = b;
    InsertObjectiveEventSorted(event);
}

// FUNCTION: LEGOLAND 0x00469260
void QueueAddRidePartsHint(struct ObjectiveEvent *arg, unsigned int class_id, int sum, int count) {
    struct ObjectiveEvent *event;

    if (IsObjectiveHintDue() == 0) {
        return;
    }
    if (QueueCustomObjectiveHint(arg) != 0) {
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
void QueueCoverSquaresHint(struct ObjectiveEvent *object, unsigned int a, int b) {
    struct ObjectiveEvent *event;

    if (IsObjectiveHintDue() == 0) {
        return;
    }
    if (QueueCustomObjectiveHint(object) != 0) {
        return;
    }
    event = AllocTimestampedObjectiveEvent(0xa, 1);
    event->field_14 = b;
    event->field_1c = a;
    InsertObjectiveEventSorted(event);
}

// FUNCTION: LEGOLAND 0x00469350
void QueuePathSceneryHint(struct ObjectiveEvent *object, int a) {
    struct ObjectiveEvent *event;

    if (IsObjectiveHintDue() == 0) {
        return;
    }
    if (QueueCustomObjectiveHint(object) != 0) {
        return;
    }
    event = AllocTimestampedObjectiveEvent(0xb, 1);
    event->field_14 = a;
    InsertObjectiveEventSorted(event);
}

// FUNCTION: LEGOLAND 0x00469390
void QueueCustomObjectiveHintIfDue(struct ObjectiveEvent *object) {
    if (IsObjectiveHintDue() != 0) {
        QueueCustomObjectiveHint(object);
    }
}

// FUNCTION: LEGOLAND 0x004693b0
void RemoveObjectiveEventsOfType(unsigned int type) {
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
void PostPendingObjectiveHints(void) {
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
                    ((struct Element *)node->element)->ride->name);
                break;
            case 0xc:
                // STRING: LEGOLAND 0x004ba69c
                PostObjectiveMessage("You need to research object %s", ((struct Element *)node->element)->ride->name);
                break;
            case 2:
                if (node->field_1c == 0) {
                    if (node->field_4 != 0) {
                        // STRING: LEGOLAND 0x004ba674
                        PostObjectiveMessage("You need to connect your %s to a path",
                            ((struct Element *)node->element)->ride->name);
                    } else {
                        // STRING: LEGOLAND 0x004ba648
                        PostObjectiveMessage("You need to connect all objects to a path");
                    }
                } else {
                    if (node->field_4 != 0) {
                        // STRING: LEGOLAND 0x004ba60c
                        PostObjectiveMessage("You need to link the path from your %s to the park entrance",
                            ((struct Element *)node->element)->ride->name);
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
                        *(unsigned int *)node->element);
                } else {
                    // STRING: LEGOLAND 0x004ba558
                    PostObjectiveMessage("You need to build %d more attractions from the %s range", node->field_1c,
                        *(unsigned int *)node->element);
                }
                break;
            case 4:
                if (node->field_14 != 0) {
                    // STRING: LEGOLAND 0x004ba510
                    PostObjectiveMessage("You need to delete all instances of %d attractions from the %s range", node->field_14,
                        *(unsigned int *)node->element);
                } else {
                    // STRING: LEGOLAND 0x004ba4dc
                    PostObjectiveMessage("You need delete %d attractions from the %s range", node->field_1c,
                        *(unsigned int *)node->element);
                }
                break;
            case 5:
                // STRING: LEGOLAND 0x004ba4b0
                PostObjectiveMessage("You need to remove %d items from the area", node->field_1c);
                break;
            case 6:
                // STRING: LEGOLAND 0x004ba48c
                PostObjectiveMessage("You need to delete %d of object %s", node->field_1c,
                    ((struct Element *)node->element)->ride->name);
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
                    ((struct Element *)node->element)->ride->name);
                break;
            case 0x12:
                if (node->field_14 != 0) {
                    // STRING: LEGOLAND 0x004ba0b4
                    PostObjectiveMessage("You need to add %d different parts to the %s", node->field_14,
                        ((struct Element *)node->element)->ride->name);
                } else {
                    // STRING: LEGOLAND 0x004ba08c
                    PostObjectiveMessage("You need to add %d more parts to the %s", node->field_1c,
                        ((struct Element *)node->element)->ride->name);
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
            RemoveObjectiveEventsOfType(node->type);
        }
    } while (node != NULL);
}

// FUNCTION: LEGOLAND 0x00469900
void GiveRewardObject(struct Element *object, unsigned int a, unsigned int b) {
    struct Element *obj;
    unsigned int flags;

    obj = (struct Element *)object;
    if (obj != NULL) {
        flags = obj->flags;
        if ((flags & 1) != 0) {
            if ((flags & 0x10002) != 2) {
                obj->flags = (flags & 0xfffeffff) | 2;
                FUN_0048a6e0((struct Element *)obj);
                if (a != 0) {
                    AddNewObjectIcon(obj->data);
                }
            } else {
                // STRING: LEGOLAND 0x004ba6e4
                DBPrintf("Not giving %d.. Already got it\n", obj->name);
            }
        }
    }
    DAT_0066871c = 1;
}

// FUNCTION: LEGOLAND 0x00469980
void CountObjectTheme(struct Element *arg) {
    struct Ride *theme;
    struct Element *id;

    theme = arg->ride;
    // STRING: LEGOLAND 0x004ba748
    if (LLIDB_FindElement("LEGOLAND THEME", &id, 0) == 0) {
        if (theme->theme_element == id) {
            LegolandCommonThemeCount++;
            return;
        }
        // STRING: LEGOLAND 0x004ba738
        if (LLIDB_FindElement("COMMON THEME", &id, 0) == 0 && theme->theme_element == id) {
            LegolandCommonThemeCount++;
            return;
        }
    }
    // STRING: LEGOLAND 0x004ba728
    if (LLIDB_FindElement("WESTERN THEME", &id, 0) == 0 && theme->theme_element == id) {
        WesternThemeCount++;
        return;
    }
    // STRING: LEGOLAND 0x004ba718
    if (LLIDB_FindElement("CASTLE THEME", &id, 0) == 0 && theme->theme_element == id) {
        CastleThemeCount++;
        return;
    }
    // STRING: LEGOLAND 0x004ba704
    if (LLIDB_FindElement("ADVENTURERS THEME", &id, 0) == 0 && theme->theme_element == id) {
        AdventurersThemeCount++;
    }
}

// FUNCTION: LEGOLAND 0x00469a80
void FUN_00469a80(struct Element *object) {
    unsigned int flags;

    if (object != NULL) {
        flags = object->flags;
        if (flags & 1) {
            object->flags = (flags & 0xfffffffd) | 0x10000;
            CountObjectTheme(object);
        }
    }
    DAT_0066871c = 1;
}

// FUNCTION: LEGOLAND 0x00469ab0
void TakeRewardObject(struct Element *object) {
    unsigned int flags;

    if (object != NULL) {
        flags = object->flags;
        if (flags & 1) {
            object->flags = flags & 0xfffefffd;
        }
    }
    DAT_0066871c = 1;
}

// FUNCTION: LEGOLAND 0x00469ae0
int ProcessUnimplementedReward(struct ObjectiveEvent *arg) {
    // STRING: LEGOLAND 0x004ba758
    DBPrintf("Processing unimplemented reward [ Type = %d ]\n", arg->type);
    return 1;
}

// FUNCTION: LEGOLAND 0x00469b00
int ProcessUnimplementedObjective(struct ObjectiveEvent *arg) {
    // STRING: LEGOLAND 0x004ba788
    DBPrintf("Processing unimplemented objective [ Type = %d ]\n", arg->type);
    return 1;
}

// FUNCTION: LEGOLAND 0x00469b20
int ScriptEventGive(struct ObjectiveEvent *event) {
    struct Element *object;

    object = event->element;
    GiveRewardObject(object, event->field_14, 0);
    object = event->element;
    object->flags |= 0x20000;
    return 1;
}

// FUNCTION: LEGOLAND 0x00469b50
int FUN_00469b50(struct ObjectiveEvent *event) {
    FUN_00469a80(event->element);
    return 1;
}

// FUNCTION: LEGOLAND 0x00469b70
int ScriptEventTake(struct ObjectiveEvent *event) {
    TakeRewardObject(event->element);
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
void PlaceObjectAtTile(struct Element *a, void *b) {
    struct Ride *cls;
    struct Point centre;

    cls = a->ride;
    SetBricksLimited(0);
    GetTileCentre((struct Point *)b, &centre.x);
    EditCursor.tile_x = centre.x;
    EditCursor.tile_y = centre.y;
    cls->cb_90(a, &centre, 0x8f8);
    FUN_0045d770(&EditCursor);
    PutObjOnMap(a->ride, a, (struct Point *)&EditCursor.tile_x);
    SetBricksLimited(1);
}

// FUNCTION: LEGOLAND 0x00469c40
int ScriptEventPlace(struct ObjectiveEvent *event) {
    PlaceObjectAtTile(event->element, &event->field_20);
    return 1;
}

// FUNCTION: LEGOLAND 0x00469c60
int FadeOutSampleCallback(struct Sample *handle) {
    SetSampleFade(handle, -100);
    return 0;
}

// Script "clear" event: removes every object whose footprint overlaps the event's area
// (field_28..field_30 x, field_2c..field_34 y), in three passes over the render list (powered
// parent-flagged objects first, then any powered object, then the rest), each removal done through
// the normal query cursor (saved and restored around it). Plays the inventory sound while it runs.
//
// MSVC6 build: exact naked __asm transcription; the C in the #else branch is the same function and
// each asm block is commented with the C it implements (struct offsets are named in the comments).
// Why asm: the C was at best 94.89%: the original reads tile_x/tile_y zero-extended into 32-bit
// registers, the C's short locals give movzx/movsx pairs and different registers; unsigned char
// and int locals were tried and are worse. See docs/attempts/0x00469c80-ScriptEventClear.md.
// The CRT stack probe is called as _chkstk (declared in objectives.h).
// clang-format off
#if defined(_MSC_VER) && (_MSC_VER <= 1200) && defined(_M_IX86)
// FUNCTION: LEGOLAND 0x00469c80
__declspec(naked) int ScriptEventClear(struct ObjectiveEvent *arg) {
    __asm {
        // Frame (0x1854 bytes, probed by _chkstk): [esp+0x10] phase, [esp+0x14] saved_class,
        // [esp+0x18] point (x, y), [esp+0x24] rect.top, [esp+0x2c] rect.bottom,
        // [esp+0x30] saved (struct Cursor, 0x60d dwords), [esp+0x1868] arg.
        // edi = current, ebx = next.
        mov eax, 0x1854
        call _chkstk
        // sample = PlayInstanceOfSample(GameFX[FX_INVENTORY_OUT].sample, 1, 1, 0); FUN_00496d10(sample);
        // AddSFX_Callback(sample, 3000, FadeOutSampleCallback);
        mov eax, dword ptr [GameFX + 0xd4] // GameFX[FX_INVENTORY_OUT].sample
        push ebx
        push ebp
        push esi
        push edi
        push 0
        push 1
        push 1
        push eax
        call PlayInstanceOfSample
        mov esi, eax
        push esi
        call FUN_00496d10
        push offset FadeOutSampleCallback
        push 0xbb8
        push esi
        call AddSFX_Callback
        mov ebx, dword ptr [esp + 0x34]
        add esp, 0x20
        // for (phase = 0; phase < 3; phase++) {
        mov dword ptr [esp + 0x10], 0
    next_phase:
        //     for (current = GetFirstRenderObject(); current != NULL; current = next) {
        call GetFirstRenderObject
        mov edi, eax
        test edi, edi
        je phase_done
    next_object:
        //         UpdateSound();
        call UpdateSound
        mov eax, dword ptr [esp + 0x10]
        sub eax, 0
        je phase0_next
        dec eax
        je phase1_next
        dec eax
        jne next_chosen
        //         phase 2: next = GetNextRenderObject(current);
        push edi
        call GetNextRenderObject
        add esp, 4
        mov ebx, eax
        jmp next_chosen
    phase1_next:
        //         phase 1: next = the next object after current with FindObjectsPower(...) > 0
        mov ebx, edi
    phase1_skip:
        push ebx
        call GetNextRenderObject
        mov ebx, eax
        add esp, 4
        test ebx, ebx
        je next_chosen
        mov ecx, dword ptr [ebx] // next->object
        mov edx, dword ptr [ecx + 0xc] // ->ride
        push edx
        call FindObjectsPower
        add esp, 4
        test eax, eax
        jle phase1_skip
        jmp next_chosen
    phase0_next:
        //         phase 0: skip while (cls = next->object->ride)->parent_element has flag 0x10
        //                  and FindObjectsPower(cls) <= 0
        mov ebx, edi
    phase0_skip:
        push ebx
        call GetNextRenderObject
        mov ebx, eax
        add esp, 4
        test ebx, ebx
        je next_chosen
        mov eax, dword ptr [ebx]
        mov esi, dword ptr [eax + 0xc] // cls
        push esi
        call FindObjectsPower
        mov esi, dword ptr [esi + 0x58] // cls->parent_element
        add esp, 4
        test esi, esi
        je next_chosen
        test byte ptr [esi + 8], 0x10
        je next_chosen
        test eax, eax
        jle phase0_skip
    next_chosen:
        //         if (current->flags_c & 0x80) {
        test byte ptr [edi + 0xc], 0x80
        je object_done
        //             cls = current->object->ride; rect = cls->footprint offset by (tile_x, tile_y)
        //             (ebp = tile_x, edx = tile_y)
        mov ecx, dword ptr [edi]
        xor edx, edx
        mov dl, byte ptr [edi + 4] // current->tile_x
        mov eax, dword ptr [ecx + 0xc] // cls
        mov ebp, edx
        xor edx, edx
        mov dl, byte ptr [edi + 5] // current->tile_y
        mov esi, dword ptr [eax + 0x40] // footprint.v[1]
        mov edi, dword ptr [eax + 0x48] // footprint.v[3]
        mov ecx, dword ptr [eax + 0x3c] // footprint.v[0]
        add esi, edx
        add edi, edx
        mov dword ptr [esp + 0x24], esi // rect.top
        mov esi, dword ptr [eax + 0x44] // footprint.v[2]
        mov dword ptr [esp + 0x2c], edi // rect.bottom
        mov edi, dword ptr [esp + 0x1868] // arg
        add ecx, ebp // rect.left
        add esi, ebp // rect.right
        //             if (rect.left <= arg->field_30 && rect.right >= arg->field_28 &&
        //                 rect.top <= arg->field_34 && rect.bottom >= arg->field_2c) {
        cmp ecx, dword ptr [edi + 0x30]
        jg object_done
        cmp esi, dword ptr [edi + 0x28]
        jl object_done
        mov ecx, dword ptr [esp + 0x24]
        mov esi, dword ptr [edi + 0x34]
        cmp ecx, esi
        jg object_done
        mov ecx, dword ptr [esp + 0x2c]
        mov esi, dword ptr [edi + 0x2c]
        cmp ecx, esi
        jl object_done
        //                 saved_class = QueryClass; memcpy(&saved, &QueryCursor, sizeof(struct Cursor));
        mov ecx, dword ptr [QueryClass]
        mov esi, offset QueryCursor
        mov dword ptr [esp + 0x14], ecx
        mov ecx, 0x60d
        lea edi, [esp + 0x30]
        mov dword ptr [esp + 0x1c], edx // point.y = tile_y
        rep movsd
        //                 QueryCursor.tile_y = tile_y; QueryCursor.tile_x = point.x = tile_x;
        //                 QueryObj.pos = (tile_x, tile_y); QueryClass = cls;
        mov ecx, ebp
        mov dword ptr [QueryCursor + 0x1408], edx // QueryCursor.tile_y
        mov dl, byte ptr [esp + 0x1c]
        mov dword ptr [esp + 0x18], ecx // point.x
        mov dword ptr [QueryCursor + 0x1404], ecx // QueryCursor.tile_x
        mov byte ptr [QueryObj], cl // QueryObj.pos.x
        mov dword ptr [QueryClass], eax
        mov byte ptr [QueryObj + 0x1], dl // QueryObj.pos.y
        //                 cls->cb_94(cls->element, &point);
        mov edx, dword ptr [eax + 0xc4] // cls->element
        lea ecx, [esp + 0x18]
        push ecx
        push edx
        call dword ptr [eax + 0x94] // cls->cb_94
        //                 BuildCursorPtr(&QueryCursor, 0, 0);
        //                 if (IsCursorBuildable(&QueryCursor)) {
        //                     ClearPathUnderFootprint(QueryClass, &point.x);
        //                     RemObjFromMap(QueryClass, QueryClass->element, QueryObj, &QueryCursor);
        //                 }
        push 0
        push 0
        push offset QueryCursor
        call BuildCursorPtr
        push offset QueryCursor
        call IsCursorBuildable
        add esp, 0x18
        test eax, eax
        je restore_cursor
        mov ecx, dword ptr [QueryClass]
        lea eax, [esp + 0x18]
        push eax
        push ecx
        call ClearPathUnderFootprint
        mov eax, dword ptr [QueryClass]
        mov dx, word ptr [QueryObj]
        push offset QueryCursor
        push edx
        mov ecx, dword ptr [eax + 0xc4]
        push ecx
        push eax
        call RemObjFromMap
        add esp, 0x18
    restore_cursor:
        //                 memcpy(&QueryCursor, &saved, sizeof(struct Cursor)); QueryClass = saved_class;
        mov edx, dword ptr [esp + 0x14]
        mov ecx, 0x60d
        lea esi, [esp + 0x30]
        mov edi, offset QueryCursor
        rep movsd
        mov dword ptr [QueryClass], edx
        //             }
        //         }
    object_done:
        test ebx, ebx
        mov edi, ebx // current = next
        jne next_object
        //     }
    phase_done:
        mov eax, dword ptr [esp + 0x10]
        inc eax
        cmp eax, 3
        mov dword ptr [esp + 0x10], eax
        jl next_phase
        // }
        // CalculateMapRenderOrder(); PlayInstanceOfSample(GameFX[FX_INVENTORY_OUT].sample, 0, 1, 0); return 1;
        call CalculateMapRenderOrder
        mov eax, dword ptr [GameFX + 0xd4]
        push 0
        push 1
        push 0
        push eax
        call PlayInstanceOfSample
        add esp, 0x10
        mov eax, 1
        pop edi
        pop esi
        pop ebp
        pop ebx
        add esp, 0x1854
        ret
    }
}
// clang-format on
#else
int ScriptEventClear(struct ObjectiveEvent *arg) {
    struct SweepInstance *next;
    RECT rect;
    short tile_x;
    struct SweepInstance *current;
    struct Ride *cls;
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
    AddSFX_Callback(sample, 3000, (unsigned int (*)(struct Sample *))FadeOutSampleCallback);
    for (phase = 0; phase < 3; phase++) {
        current = (struct SweepInstance *)GetFirstRenderObject();
        while (current != NULL) {
            UpdateSound();
            switch (phase) {
            case 0:
                next = current;
                do {
                    next = (struct SweepInstance *)GetNextRenderObject((MapElement *)next);
                } while (next != NULL && (cls = next->object->ride, power = FindObjectsPower(cls), cls->parent_element != NULL) &&
                    (0x10 & (unsigned char)cls->parent_element->flags) != 0 && power <= 0);
                break;
            case 1:
                next = current;
                do {
                    next = (struct SweepInstance *)GetNextRenderObject((MapElement *)next);
                } while (next != NULL && FindObjectsPower(next->object->ride) <= 0);
                break;
            case 2:
                next = (struct SweepInstance *)GetNextRenderObject((MapElement *)current);
                break;
            }
            if (current->flags_c & 0x80) {
                cls = current->object->ride;
                tile_y = current->tile_y;
                rect.top = cls->footprint.v[1];
                tile_x = current->tile_x;
                rect.bottom = cls->footprint.v[3];
                rect.top += tile_y;
                right = cls->footprint.v[2];
                rect.right = right;
                rect.right += tile_x;
                rect.bottom += tile_y;
                rect.left = cls->footprint.v[0] + tile_x;
                if (rect.left <= arg->field_30 && rect.right >= arg->field_28 && rect.top <= arg->field_34 && rect.bottom >= arg->field_2c) {
                    point.y = tile_y;
                    saved_class = QueryClass;
                    memcpy(&saved, &QueryCursor, sizeof(struct Cursor));
                    QueryCursor.tile_y = tile_y;
                    point.x = tile_x;
                    QueryCursor.tile_x = point.x;
                    QueryObj.pos.x = (unsigned char)point.x;
                    QueryClass = cls;
                    QueryObj.pos.y = (unsigned char)point.y;
                    cls->cb_94(cls->element, &point);
                    BuildCursorPtr(&QueryCursor, 0, 0);
                    if (IsCursorBuildable(&QueryCursor) != 0) {
                        ClearPathUnderFootprint(QueryClass, &point.x);
                        RemObjFromMap(QueryClass, (QueryClass)->element, QueryObj, &QueryCursor);
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
#endif

// FUNCTION: LEGOLAND 0x00469ed0
int ScriptEventUnglue(struct ObjectiveEvent *arg) {
    int y;
    int x;

    y = arg->field_2c;
    while (y <= arg->field_34) {
        x = arg->field_28;
        while (x <= arg->field_30) {
            GameMap[y][x].flags &= 0xffbf;
            x++;
        }
        y++;
    }
    return 1;
}

// FUNCTION: LEGOLAND 0x00469f20
int ScriptEventGlue(struct ObjectiveEvent *arg) {
    int y;
    int x;

    y = arg->field_2c;
    while (y <= arg->field_34) {
        x = arg->field_28;
        while (x <= arg->field_30) {
            GameMap[y][x].flags |= 0x40;
            x++;
        }
        y++;
    }
    return 1;
}

// FUNCTION: LEGOLAND 0x00469f70
int ScriptEventExtendPark(struct ObjectiveEvent *arg) {
    return ProcessUnimplementedReward(arg);
}

// FUNCTION: LEGOLAND 0x00469f80
int ScriptEventFmv(struct ObjectiveEvent *arg) {
    PauseGameTimer();
    SetPointer(0);
    FUN_00496e60(1, 0xf);
    PlayMovie(arg->text, 1, 1);
    ResumeGameTimer();
    ClearAdvisorHelp();
    RepostScriptIntro();
    return 1;
}

// FUNCTION: LEGOLAND 0x00469fc0
int ScriptEventInterval(struct ObjectiveEvent *arg) {
    int retries;

    FUN_00490600(0);
    if (LoadIntervalReport(arg->text) != 0) {
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
    DBPrintf("Giving up trying to load Interval %s\n", arg->text);
    return 1;
}

// FUNCTION: LEGOLAND 0x0046a030
int ScriptEventMessage(struct ObjectiveEvent *arg) {
    return ProcessUnimplementedReward(arg);
}
