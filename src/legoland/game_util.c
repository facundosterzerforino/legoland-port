#include <stdlib.h>
#include <string.h>
#include "legoland.h"

#include "bloke.h"
#include "bricks.h"
#include "challenge.h"
#include "draw.h"
#include "game_util.h"
#include "gamemain.h"
#include "gamemap.h"
#include "globals.h"
#include "interface.h"
#include "llidb.h"
#include "map_object.h"
#include "nerps.h"
#include "objclass.h"
#include "objectives.h"
#include "screens.h"
#include "sound_sfx.h"
#include "title.h"
#include "worker.h"

struct CommandArgs {
    unsigned char pad_0[4];
    unsigned int arg1;
    char *arg2;
    char *arg3;
};

// FUNCTION: LEGOLAND 0x004787b0
int IsStringEmpty(char **pp_str) {
    if (strlen(*pp_str) == 0) {
        return 1;
    }
    return 0;
}

// FUNCTION: LEGOLAND 0x004787d0
void FUN_004787d0(void) {
    if (CurrentScriptSection != 0) {
        /* TODO: fold into struct SortNode/NerpsListNode views of CurrentScriptSection */
        InsertScriptSection((struct SortNode *)CurrentScriptSection);
    }
    CurrentScriptSection = 0;
}

// FUNCTION: LEGOLAND 0x004787f0
void FUN_004787f0(void) {
    unsigned int id = DAT_00669098;
    DAT_00669098 = DAT_00669098 + 1;
    CurrentScriptSection = (unsigned int)FUN_0046b4f0(id);
    DAT_007fdca4 = 0;
}

// FUNCTION: LEGOLAND 0x00478820
int FUN_00478820(void) {
    // STRING: LEGOLAND 0x004bc0a4
    FUN_004785d0("Uninitialised", 0);
    return 1;
}

// FUNCTION: LEGOLAND 0x00478840
int ScriptCmdEnd(void) {
    // STRING: LEGOLAND 0x004bc0b4
    FUN_004785d0("Closed", 0);
    FUN_004787d0();
    CurrentScriptSection = 0;
    return 1;
}

// FUNCTION: LEGOLAND 0x00478870
int ScriptCmdInit(unsigned int *param_1) {
    FUN_004785d0((char *)*param_1, 1);
    return 1;
}

// FUNCTION: LEGOLAND 0x00478890
int ScriptCmdAges(struct CommandArgs *arg, int argc) {
    unsigned int n1;
    unsigned int n2;

    if (argc == 0) {
        ScriptConditionActive = 1;
        return 1;
    }
    if (argc == 1) {
        n1 = atoi((char *)arg->arg1);
        ScriptConditionActive = n1 <= CurrentProfile.field_20;
        return 1;
    }
    n1 = atoi((char *)arg->arg1);
    n2 = atoi(arg->arg2);
    if (n1 > n2) {
        return 0;
    }
    if (CurrentProfile.field_20 < n1 || (ScriptConditionActive = 1, CurrentProfile.field_20 > n2)) {
        ScriptConditionActive = 0;
    }
    return 1;
}

// FUNCTION: LEGOLAND 0x00478930
int ScriptCmdObjective(unsigned int param_1, unsigned int param_2) {
    if (ScriptConditionActive != 0) {
        if (FUN_004786a0(param_1, param_2, 7) == 0) {
            return 0;
        }
        FUN_004785d0(*(char **)param_1, 2);
        SetScriptObjectiveKind(1);
        FUN_004787d0();
        FUN_004787f0();
    }
    return 1;
}

// FUNCTION: LEGOLAND 0x00478980
int ScriptCmdOneOff(unsigned int param_1, unsigned int param_2) {
    if (ScriptConditionActive == 0) {
        return 1;
    }
    if (FUN_004786a0(param_1, param_2, 2) == 0) {
        return 0;
    }
    SetScriptObjectiveKind(0);
    return 1;
}

// FUNCTION: LEGOLAND 0x004789c0
int ScriptCmdOngoing(unsigned int param_1, unsigned int param_2) {
    if (ScriptConditionActive == 0) {
        return 1;
    }
    if (FUN_004786a0(param_1, param_2, 2) == 0) {
        return 0;
    }
    SetScriptObjectiveKind(1);
    return 1;
}

// FUNCTION: LEGOLAND 0x00478a00
int ScriptCmdPermanent(unsigned int param_1, unsigned int param_2) {
    if (ScriptConditionActive == 0) {
        return 1;
    }
    if (FUN_004786a0(param_1, param_2, 2) == 0) {
        return 0;
    }
    SetScriptObjectiveKind(2);
    FUN_00478650(param_1, param_2);
    return 1;
}

// FUNCTION: LEGOLAND 0x00478a40
int ScriptCmdReminder(unsigned int param_1, unsigned int param_2) {
    if (ScriptConditionActive == 0) {
        return 1;
    }
    if (FUN_004786a0(param_1, param_2, 2) == 0) {
        return 0;
    }
    SetScriptObjectiveKind(3);
    FUN_00478650(param_1, param_2);
    return 1;
}

// FUNCTION: LEGOLAND 0x00478a80
int ScriptCmdReward(unsigned int param_1, unsigned int param_2) {
    if (ScriptConditionActive != 0) {
        if (FUN_004786a0(param_1, param_2, 2) == 0) {
            return 0;
        }
        FUN_004785d0(*(char **)param_1, 4);
    }
    return 1;
}

// FUNCTION: LEGOLAND 0x00478ac0
int ScriptCmdMap(unsigned int param_1, unsigned int param_2) {
    unsigned int map;

    if (ScriptConditionActive == 0) {
        return 1;
    }
    if (FUN_00478690(param_1, param_2, 1) == 0) {
        return 0;
    }
    map = FUN_004787a0(param_1, param_2);
    ClearObjectCounters();
    if (LoadBaseMap(map) == 0) {
        return -1;
    }
    CalculateMapRenderOrder();
    FUN_0045a060();
    return 1;
}

// FUNCTION: LEGOLAND 0x00478b20
int FUN_00478b20(unsigned int arg) {
    struct GameObject *obj = (struct GameObject *)ElemID((const char *)arg);

    if (obj != NULL) {
        if ((obj->flags & 4) == 0) {
            if (LoadObjectClass(obj) == 0) {
                return 0;
            }
            obj->flags |= 4;
            FUN_00469ab0(obj);
        }
        return 1;
    }
    return 0;
}

// FUNCTION: LEGOLAND 0x00478b70
int ScriptCmdLoad(unsigned int param_1, unsigned int param_2) {
    if (ScriptConditionActive == 0) {
        return 1;
    }
    if (FUN_004786c0(param_1, param_2, 1, 1) == 0) {
        return 0;
    }
    return FUN_00478b20(FUN_004787a0(param_1, param_2)) != 0;
}

// FUNCTION: LEGOLAND 0x00478bc0
unsigned int ScriptCmdBlueprint(unsigned int param_1, unsigned int param_2, unsigned int param_3) {
    return ScriptCmdEnable(param_1, param_2, param_3);
}

// FUNCTION: LEGOLAND 0x00478be0
unsigned int ScriptCmdEnable(unsigned int param_1, unsigned int param_2, unsigned int param_3) {
    unsigned int name;

    if (ScriptConditionActive == 0) {
        return 1;
    }
    if (FUN_004786c0(param_1, param_2, 5, 1) == 0) {
        return 0;
    }
    if (DAT_00669054 == 1) {
        name = FUN_004787a0(param_1, param_2);
        if (FUN_00478b20(name) != 0) {
            FUN_00469900((struct NerpsArg *)ElemID((const char *)name), 0, 1);
        }
        return 1;
    }
    return ScriptCmdGive(param_1, param_2); /* [port] the original also pushed param_3, which ScriptCmdGive never reads */
}

// FUNCTION: LEGOLAND 0x00478c60
int ScriptCmdCurrency(struct CommandArgs *arg, int argc) {
    if (ScriptConditionActive != 0) {
        if (FUN_004786c0((unsigned int)arg, argc, 5, 1) == 0) {
            return 0;
        }
        if (DAT_00669054 == 1) {
            SetBrickCount(atoi((char *)arg->arg1));
            return 1;
        }
        NewScriptCurrencyEvent(atoi((char *)arg->arg1));
    }
    return 1;
}

// FUNCTION: LEGOLAND 0x00478cd0
int ScriptCmdHappinessEnv(char **argv, int argc) {
    int *out;

    if (ScriptConditionActive != 0) {
        if (FUN_004786c0((unsigned int)argv, argc, 5, 5) == 0) {
            return 0;
        }
        if (DAT_00669054 == 1) {
            out = &MapStats.mood_threshold0;
            argv++;
            do {
                *out = atoi(*argv);
                argv++;
                out++;
            } while ((int)out < (int)MapStats.mood_delta);
        }
    }
    return 1;
}

// FUNCTION: LEGOLAND 0x00478d30
int ScriptCmdLookAt(int param_1, int param_2) {
    int local[2];
    char **args;
    int a;
    int b;

    if (ScriptConditionActive == 0) {
        return 1;
    }
    args = (char **)param_1;
    if (FUN_004786c0((int)args, param_2, 5, 2) == 0) {
        return 0;
    }
    ParseIntPair(local, args, 1);
    local[0] <<= 8;
    local[1] <<= 8;
    if (DAT_00669054 == 1) {
        b = local[1];
        a = local[0];
        GetTileDimensions(&param_2, &param_1);
        local[0] = ((a - b) * param_2) >> 9;
        local[1] = ((a + b) * param_1) >> 9;
        ScrollX = (local[0] - (lpConfig->screen_width >> 1)) << 8;
        ScrollY = (local[1] - (lpConfig->screen_height >> 1)) << 8;
        return 1;
    }
    NewScriptLookAtEvent((unsigned int *)local);
    return 1;
}

// FUNCTION: LEGOLAND 0x00478e20
int ScriptCmdBriefingFile(int param_1, int param_2) {
    char *name = DAT_004d8bb0;
    if (ScriptConditionActive != 0) {
        if (FUN_004786c0(param_1, param_2, 5, 0) == 0) {
            return 0;
        }
        if (param_2 >= 1) {
            name = *(char **)(param_1 + 4);
        }
        if (DAT_00669054 == 1) {
            FUN_004687f0(name);
            return 1;
        }
        NewScriptBriefingFileEvent((unsigned int)name);
    }
    return 1;
}

// FUNCTION: LEGOLAND 0x00478e90
int ScriptCmdHintsFile(int param_1, int param_2) {
    char *name = DAT_004d8bb0;
    if (ScriptConditionActive != 0) {
        if (FUN_004786c0(param_1, param_2, 5, 0) == 0) {
            return 0;
        }
        if (param_2 >= 1) {
            name = *(char **)(param_1 + 4);
        }
        if (DAT_00669054 == 1) {
            FUN_00468810(name);
            return 1;
        }
        NewScriptHintsFileEvent((unsigned int)name);
    }
    return 1;
}

// FUNCTION: LEGOLAND 0x00478f00
int ScriptCmdWorkers(struct CommandArgs *arg, int argc) {
    int v1;
    int v2;

    if (ScriptConditionActive == 0) {
        return 1;
    }
    if (FUN_004786c0((unsigned int)arg, argc, 5, 2) == 0) {
        return 0;
    }
    v1 = atoi((char *)arg->arg1);
    v2 = atoi(arg->arg2);
    if (DAT_00669054 == 1) {
        if (v1 >= 0) {
            lpConfig->gardeners_enabled = v1 != 0;
        }
        if (v2 >= 0) {
            lpConfig->mechanics_enabled = v2 != 0;
        }
    } else {
        NewScriptWorkersEvent(v1, v2);
    }
    return 1;
}

// FUNCTION: LEGOLAND 0x00478fa0
int ScriptCmdGardener(struct CommandArgs *arg, int argc) {
    int coords[2];
    int count;

    if (ScriptConditionActive != 0) {
        if (FUN_004786c0((unsigned int)arg, argc, 5, 2) == 0) {
            return 0;
        }
        coords[0] = atoi((char *)arg->arg1);
        coords[1] = atoi(arg->arg2);
        if (argc < 3 || (count = atoi(arg->arg3)) < 1) {
            count = 1;
        }
        if (DAT_00669054 == 1) {
            while (count != 0) {
                GenerateGardener(coords, 0);
                count--;
            }
        } else {
            FUN_0046bad0(1, count, (unsigned int *)coords);
        }
    }
    return 1;
}

// FUNCTION: LEGOLAND 0x00479060
int ScriptCmdMechanic(struct CommandArgs *arg, int argc) {
    int coords[2];
    int count;

    if (ScriptConditionActive != 0) {
        if (FUN_004786c0((unsigned int)arg, argc, 5, 2) == 0) {
            return 0;
        }
        coords[0] = atoi((char *)arg->arg1);
        coords[1] = atoi(arg->arg2);
        if (argc < 3 || (count = atoi(arg->arg3)) < 1) {
            count = 1;
        }
        if (DAT_00669054 == 1) {
            while (count != 0) {
                GenerateMechanic(coords, 0);
                count--;
            }
        } else {
            FUN_0046bad0(0, count, (unsigned int *)coords);
        }
    }
    return 1;
}

// FUNCTION: LEGOLAND 0x00479120
int ScriptCmdPrompt(struct CommandArgs *arg, int argc) {
    if (ScriptConditionActive != 0) {
        if (FUN_004786c0((unsigned int)arg, argc, 2, 0) == 0) {
            return 0;
        }
        if (argc > 0) {
            if (argc > 1) {
                DAT_007fdca4 = FUN_004689f0((char *)arg->arg1, arg->arg2, 1);
                return 1;
            }
            DAT_007fdca4 = FUN_004689f0((char *)arg->arg1, NULL, 1);
            return 1;
        }
        DAT_007fdca4 = 0;
    }
    return 1;
}

// FUNCTION: LEGOLAND 0x004791a0
int ScriptCmdIntro(struct CommandArgs *arg, int argc) {
    if (ScriptConditionActive != 0) {
        if (FUN_004786c0((unsigned int)arg, argc, 2, 1) == 0) {
            return 0;
        }
        if (CurrentScriptSection != 0) {
            FUN_0046b650((const char *)arg->arg1, (struct StringHolder *)CurrentScriptSection);
        }
    }
    return 1;
}

// FUNCTION: LEGOLAND 0x004791f0
int ScriptCmdNeed(struct CommandArgs *arg, int argc) {
    unsigned int id;
    unsigned int count;

    if (ScriptConditionActive == 0) {
        return 1;
    }
    if (FUN_004786c0((unsigned int)arg, argc, 2, 1) == 0) {
        return 0;
    }
    id = ElemID((const char *)arg->arg1);
    if (argc <= 1) {
        count = 1;
    } else {
        count = atoi(arg->arg2);
        if (count == 0) {
            count = 1;
        }
    }
    if (id != 0) {
        NewScriptNeedEvent(CurrentObjectiveEventFlags, id, count);
    }
    return 1;
}

// FUNCTION: LEGOLAND 0x00479270
int ScriptCmdNeedAt(struct CommandArgs *arg, int argc) {
    unsigned int id;
    unsigned int vals[2];

    if (ScriptConditionActive == 0) {
        return 1;
    }
    if (FUN_004786c0((unsigned int)arg, argc, 2, 3) == 0) {
        return 0;
    }
    id = ElemID((const char *)arg->arg1);
    vals[0] = atoi(arg->arg2);
    vals[1] = atoi(arg->arg3);
    if (id != 0) {
        NewScriptNeedAtEvent(CurrentObjectiveEventFlags, id, vals);
    }
    return 1;
}

// FUNCTION: LEGOLAND 0x00479300
int ScriptCmdNeedIn(struct CommandArgs *arg, int argc) {
    unsigned int id;
    unsigned int v;
    struct Vec4 vec;

    if (ScriptConditionActive == 0) {
        return 1;
    }
    if (FUN_004786c0((unsigned int)arg, argc, 2, 6) == 0) {
        return 0;
    }
    id = ElemID((const char *)arg->arg1);
    v = atoi(arg->arg2);
    ParseRect((int *)&vec, (char **)arg, 3);
    if (id != 0) {
        NewScriptNeedInEvent(CurrentObjectiveEventFlags, id, v, &vec);
    }
    return 1;
}

// FUNCTION: LEGOLAND 0x00479390
int ScriptCmdConnect(struct CommandArgs *arg, int argc) {
    unsigned int id;

    if (ScriptConditionActive != 0) {
        if (FUN_004786c0((unsigned int)arg, argc, 2, 1) == 0) {
            return 0;
        }
        id = ElemID((const char *)arg->arg1);
        if (id != 0) {
            NewScriptConnectEvent(CurrentObjectiveEventFlags, id);
        }
    }
    return 1;
}

// FUNCTION: LEGOLAND 0x004793e0
int ScriptCmdLink(struct CommandArgs *arg, int argc) {
    unsigned int id;

    if (ScriptConditionActive != 0) {
        if (FUN_004786c0((unsigned int)arg, argc, 2, 1) == 0) {
            return 0;
        }
        // STRING: LEGOLAND 0x004bc0bc
        if (_stricmp((char *)arg->arg1, "ALL") == 0) {
            id = 0;
        } else {
            id = ElemID((const char *)arg->arg1);
            if (id == 0) {
                return 0;
            }
        }
        NewScriptLinkEvent(CurrentObjectiveEventFlags, id);
    }
    return 1;
}

// FUNCTION: LEGOLAND 0x00479450
int ScriptCmdRange(struct CommandArgs *arg, int argc) {
    unsigned int id;
    int v1;
    unsigned int v2;

    if (ScriptConditionActive == 0) {
        return 1;
    }
    if (FUN_004786c0((unsigned int)arg, argc, 2, 2) == 0) {
        return 0;
    }
    id = ElemID((const char *)arg->arg1);
    v1 = atoi(arg->arg2);
    if (argc >= 3) {
        v2 = atoi(arg->arg3);
    } else {
        v2 = 0;
    }
    if (id != 0) {
        NewScriptRangeEvent(CurrentObjectiveEventFlags, id, v2, v1);
    }
    return 1;
}

// FUNCTION: LEGOLAND 0x004794d0
int ScriptCmdClearArea(struct CommandArgs *arg, int argc) {
    struct Vec4 vec;
    unsigned int v;

    if (ScriptConditionActive != 0) {
        if (FUN_004786c0((unsigned int)arg, argc, 2, 4) == 0) {
            return 0;
        }
        ParseRect((int *)&vec, (char **)arg, 1);
        if (argc >= 5) {
            v = atoi(((char **)arg)[5]);
        } else {
            v = 0;
        }
        NewScriptClearAreaEvent(CurrentObjectiveEventFlags, &vec, v);
    }
    return 1;
}

// FUNCTION: LEGOLAND 0x00479550
int ScriptCmdRemove(struct CommandArgs *arg, int argc) {
    unsigned int id;
    unsigned int v;

    if (ScriptConditionActive != 0) {
        if (FUN_004786c0((unsigned int)arg, argc, 2, 2) == 0) {
            return 0;
        }
        if (FUN_00478690((unsigned int)arg, argc, 2) == 0) {
            return 0;
        }
        id = ElemID((const char *)arg->arg1);
        v = atoi(arg->arg2);
        if (id != 0) {
            NewScriptRemoveEvent(CurrentObjectiveEventFlags, id, v);
        }
    }
    return 1;
}

// FUNCTION: LEGOLAND 0x004795c0
int ScriptCmdRemoveRange(struct CommandArgs *arg, int argc) {
    unsigned int id;
    unsigned int v2;
    unsigned int v3;

    if (ScriptConditionActive == 0) {
        return 1;
    }
    if (FUN_004786c0((unsigned int)arg, argc, 2, 2) == 0) {
        return 0;
    }
    id = ElemID((const char *)arg->arg1);
    v2 = atoi(arg->arg2);
    if (argc >= 3) {
        v3 = atoi(arg->arg3);
    } else {
        v3 = 0xffffffff;
    }
    if (id != 0) {
        NewScriptRemoveRangeEvent(CurrentObjectiveEventFlags, id, v3, v2);
    }
    return 1;
}

// FUNCTION: LEGOLAND 0x00479640
int ScriptCmdComposite(struct CommandArgs *arg, int argc) {
    unsigned int id;
    int n;
    unsigned int v;

    if (ScriptConditionActive == 0) {
        return 1;
    }
    if (FUN_004786c0((unsigned int)arg, argc, 2, 2) == 0) {
        return 0;
    }
    id = ElemID((const char *)arg->arg1);
    n = atoi(arg->arg2);
    if (n == 0) {
        n = 1;
    }
    if (argc >= 3) {
        v = atoi(arg->arg3);
    } else {
        v = 0;
    }
    if (id != 0) {
        NewScriptCompositeEvent(CurrentObjectiveEventFlags, id, n, v);
    }
    return 1;
}

// FUNCTION: LEGOLAND 0x004796d0
int ScriptCmdLoopComposite(struct CommandArgs *arg, int argc) {
    unsigned int id;
    int count;

    if (ScriptConditionActive == 0) {
        return 1;
    }
    if (FUN_004786c0((unsigned int)arg, argc, 2, 2) == 0) {
        return 0;
    }
    id = ElemID((const char *)arg->arg1);
    count = atoi(arg->arg2);
    if (count == 0) {
        count = 1;
    }
    if (id != 0) {
        NewScriptLoopCompositeEvent(CurrentObjectiveEventFlags, id, count);
    }
    return 1;
}

// FUNCTION: LEGOLAND 0x00479740
int ScriptCmdTechLevel(struct CommandArgs *arg, int argc) {
    unsigned int id;
    unsigned int v;

    if (ScriptConditionActive == 0) {
        return 1;
    }
    if (FUN_004786c0((unsigned int)arg, argc, 2, 2) == 0) {
        return 0;
    }
    id = ElemID((const char *)arg->arg1);
    v = atoi(arg->arg2);
    if (id != 0) {
        NewScriptTechLevelEvent(CurrentObjectiveEventFlags, id, v);
    }
    return 1;
}

// FUNCTION: LEGOLAND 0x004797b0
int ScriptCmdResearch(struct CommandArgs *arg, int argc) {
    if (ScriptConditionActive != 0) {
        if (FUN_004786c0((unsigned int)arg, argc, 2, 1) == 0) {
            return 0;
        }
        ElemID((const char *)arg->arg1);
        if (argc >= 2) {
            atoi(arg->arg2);
        }
    }
    return 1;
}

// FUNCTION: LEGOLAND 0x00479800
int ScriptCmdParkVisitors(struct CommandArgs *arg, int argc) {
    if (ScriptConditionActive == 0) {
        return 1;
    }
    if (FUN_004786c0((unsigned int)arg, argc, 2, 1) == 0) {
        return 0;
    }
    NewScriptParkVisitorsEvent(CurrentObjectiveEventFlags, atoi((char *)arg->arg1));
    return 1;
}

// FUNCTION: LEGOLAND 0x00479850
int ScriptCmdRideVisitors(struct CommandArgs *arg, int argc) {
    unsigned int id;
    int value;

    if (ScriptConditionActive == 0) {
        return 1;
    }
    if (FUN_004786c0((unsigned int)arg, argc, 2, 2) == 0) {
        return 0;
    }
    id = ElemID((const char *)arg->arg1);
    value = atoi(arg->arg2);
    if (id != 0) {
        NewScriptRideVisitorsEvent(CurrentObjectiveEventFlags, id, value);
    }
    return 1;
}

// FUNCTION: LEGOLAND 0x004798c0
int ScriptCmdRiders(struct CommandArgs *arg, int argc) {
    unsigned int id;
    unsigned int v;

    if (ScriptConditionActive == 0) {
        return 1;
    }
    if (FUN_004786c0((unsigned int)arg, argc, 2, 2) == 0) {
        return 0;
    }
    id = ElemID((const char *)arg->arg1);
    v = atoi(arg->arg2);
    if (id != 0) {
        NewScriptRidersEvent(CurrentObjectiveEventFlags, id, v);
    }
    return 1;
}

// FUNCTION: LEGOLAND 0x00479930
int ScriptCmdSceneryCoverage(struct CommandArgs *arg, int argc) {
    int value;

    if (ScriptConditionActive == 0) {
        return 1;
    }
    if (FUN_004786c0((unsigned int)arg, argc, 2, 1) == 0) {
        return 0;
    }
    value = atoi((char *)arg->arg1);
    NewScriptSceneryCoverageEvent(CurrentObjectiveEventFlags, value);
    return 1;
}

// FUNCTION: LEGOLAND 0x00479980
int ScriptCmdPathScenery(struct CommandArgs *arg, int argc) {
    if (ScriptConditionActive != 0) {
        if (FUN_004786c0((unsigned int)arg, argc, 2, 1) == 0) {
            return 0;
        }
        NewScriptPathSceneryEvent(CurrentObjectiveEventFlags, atoi((char *)arg->arg1));
    }
    return 1;
}

// FUNCTION: LEGOLAND 0x004799d0
int ScriptCmdRideCoverage(struct CommandArgs *arg, int argc) {
    if (ScriptConditionActive != 0) {
        if (FUN_004786c0((unsigned int)arg, argc, 2, 1) == 0) {
            return 0;
        }
        NewScriptRideCoverageEvent(CurrentObjectiveEventFlags, atoi((char *)arg->arg1));
    }
    return 1;
}

// FUNCTION: LEGOLAND 0x00479a20
int ScriptCmdShopCoverage(struct CommandArgs *arg, int argc) {
    if (ScriptConditionActive != 0) {
        if (FUN_004786c0((unsigned int)arg, argc, 2, 1) == 0) {
            return 0;
        }
        NewScriptShopCoverageEvent(CurrentObjectiveEventFlags, atoi((char *)arg->arg1));
    }
    return 1;
}

// FUNCTION: LEGOLAND 0x00479a70
int ScriptCmdFoodCoverage(struct CommandArgs *arg, int argc) {
    if (ScriptConditionActive != 0) {
        if (FUN_004786c0((unsigned int)arg, argc, 2, 1) == 0) {
            return 0;
        }
        NewScriptFoodCoverageEvent(CurrentObjectiveEventFlags, atoi((char *)arg->arg1));
    }
    return 1;
}

// FUNCTION: LEGOLAND 0x00479ac0
int ScriptCmdTotalCoverage(struct CommandArgs *arg, int argc) {
    if (ScriptConditionActive != 0) {
        if (FUN_004786c0((unsigned int)arg, argc, 2, 1) == 0) {
            return 0;
        }
        NewScriptTotalCoverageEvent(CurrentObjectiveEventFlags, atoi((char *)arg->arg1));
    }
    return 1;
}

// FUNCTION: LEGOLAND 0x00479b10
int ScriptCmdAppraisal(struct CommandArgs *arg, int argc) {
    char buf[0x200];
    int v;

    buf[0] = DAT_004d8bb0[0];
    memset(buf + 1, 0, sizeof(buf) - 1);
    if (ScriptConditionActive == 0) {
        return 1;
    }
    if (FUN_004786c0((unsigned int)arg, argc, 1, 0) == 0) {
        return 0;
    }
    if (argc >= 1) {
        v = atoi((char *)arg->arg1);
    } else {
        v = 0;
    }
    if (argc >= 2) {
        strcpy(buf, arg->arg2);
    }
    // STRING: LEGOLAND 0x004bc0c0
    strcat(buf, ";");
    if (argc >= 3) {
        strcat(buf, arg->arg3);
    }
    FUN_0044dc70(v, (unsigned int)buf);
    return 1;
}

// FUNCTION: LEGOLAND 0x00479c40
int ScriptCmdStudArea(struct CommandArgs *arg, int argc) {
    struct Vec4 vec;
    unsigned int v;

    if (ScriptConditionActive != 0) {
        if (FUN_004786c0((unsigned int)arg, argc, 2, 5) == 0) {
            return 0;
        }
        ParseRect((int *)&vec, (char **)arg, 1);
        v = atoi(((char **)arg)[5]);
        NewScriptStudAreaEvent(CurrentObjectiveEventFlags, &vec, v);
    }
    return 1;
}

// FUNCTION: LEGOLAND 0x00479cb0
int ScriptCmdSave(struct CommandArgs *arg, int argc) {
    if (ScriptConditionActive == 0) {
        return 1;
    }
    if (FUN_004786c0((unsigned int)arg, argc, 2, 1) == 0) {
        return 0;
    }
    NewScriptSaveEvent(CurrentObjectiveEventFlags, atoi((char *)arg->arg1));
    return 1;
}

// FUNCTION: LEGOLAND 0x00479d00
int ScriptCmdHappiness(struct CommandArgs *arg, int argc) {
    unsigned int v1;
    unsigned int v2;

    if (ScriptConditionActive == 0) {
        return 1;
    }
    if (FUN_004786c0((unsigned int)arg, argc, 2, 2) == 0) {
        return 0;
    }
    v1 = atoi((char *)arg->arg1);
    v2 = atoi(arg->arg2);
    NewScriptHappinessEvent(CurrentObjectiveEventFlags, v1, v2);
    return 1;
}

// FUNCTION: LEGOLAND 0x00479d60
int ScriptCmdNeedGardeners(struct CommandArgs *arg, int argc) {
    if (ScriptConditionActive == 0) {
        return 1;
    }
    if (FUN_004786c0((unsigned int)arg, argc, 2, 1) == 0) {
        return 0;
    }
    NewScriptNeedGardenersEvent(CurrentObjectiveEventFlags, atoi((char *)arg->arg1));
    return 1;
}

// FUNCTION: LEGOLAND 0x00479db0
int ScriptCmdNeedMechanics(struct CommandArgs *arg, int argc) {
    if (ScriptConditionActive == 0) {
        return 1;
    }
    if (FUN_004786c0((unsigned int)arg, argc, 2, 1) == 0) {
        return 0;
    }
    NewScriptNeedMechanicsEvent(CurrentObjectiveEventFlags, atoi((char *)arg->arg1));
    return 1;
}

// FUNCTION: LEGOLAND 0x00479e00
int ScriptCmdHunger(struct CommandArgs *arg, int argc) {
    unsigned int v1;
    unsigned int v2;
    int b;

    if (ScriptConditionActive == 0) {
        return 1;
    }
    if (FUN_004786c0((unsigned int)arg, argc, 2, 2) == 0) {
        return 0;
    }
    v1 = atoi((char *)arg->arg1);
    v2 = atoi(arg->arg2);
    if (argc >= 3) {
        b = *arg->arg3 == '+';
    } else {
        b = 0;
    }
    NewScriptHungerEvent(CurrentObjectiveEventFlags, v1, v2, b);
    return 1;
}

// FUNCTION: LEGOLAND 0x00479e80
int ScriptCmdFixRides(struct CommandArgs *arg, int argc) {
    int n1;
    int n2;

    if (ScriptConditionActive == 0) {
        return 1;
    }
    if (FUN_004786c0((unsigned int)arg, argc, 2, 2) == 0) {
        return 0;
    }
    n1 = atoi((char *)arg->arg1);
    n2 = atoi(arg->arg2);
    NewScriptFixRidesEvent(CurrentObjectiveEventFlags, n1, n2);
    return 1;
}

// FUNCTION: LEGOLAND 0x00479ee0
int ScriptCmdPowerRides(struct CommandArgs *arg, int argc) {
    if (ScriptConditionActive == 0) {
        return 1;
    }
    if (FUN_004786c0((unsigned int)arg, argc, 2, 1) == 0) {
        return 0;
    }
    NewScriptPowerRidesEvent(CurrentObjectiveEventFlags, atoi((char *)arg->arg1));
    return 1;
}

// FUNCTION: LEGOLAND 0x00479f30
int ScriptCmdZoning(struct CommandArgs *arg, int argc) {
    int index;
    unsigned int v;

    if (ScriptConditionActive != 0) {
        if (FUN_004786c0((unsigned int)arg, argc, 2, 2) == 0) {
            return 0;
        }
        index = FindStringNoCase((char *)arg->arg1, &DAT_004bb5b4, 4);
        if (index == -1) {
            return 0;
        }
        v = atoi(arg->arg2);
        NewScriptZoningEvent(CurrentObjectiveEventFlags, index, v);
    }
    return 1;
}

// FUNCTION: LEGOLAND 0x00479fa0
int ScriptCmdCheckFlag(struct CommandArgs *arg, int argc) {
    int n1;
    int n2;

    if (ScriptConditionActive == 0) {
        return 1;
    }
    if (FUN_004786c0((unsigned int)arg, argc, 2, 1) == 0) {
        return 0;
    }
    n1 = atoi((char *)arg->arg1);
    if (argc >= 2) {
        n2 = atoi(arg->arg2);
    } else {
        n2 = 1;
    }
    NewScriptCheckFlagEvent(CurrentObjectiveEventFlags, n1, n2);
    return 1;
}

// FUNCTION: LEGOLAND 0x0047a020
int ScriptCmdThemeIcon(struct CommandArgs *arg, int argc) {
    int v2;
    int v3;

    if (ScriptConditionActive == 0) {
        return 1;
    }
    if (FUN_004786c0((unsigned int)arg, argc, 5, 1) == 0) {
        return 0;
    }
    v2 = atoi((char *)arg->arg1);
    if (argc >= 2) {
        v3 = atoi(arg->arg2);
    } else {
        v3 = 1;
    }
    if (DAT_00669054 == 1) {
        FUN_00468860(v2, v3);
    } else {
        NewScriptThemeIconEvent(v2, v3);
    }
    return 1;
}

// FUNCTION: LEGOLAND 0x0047a0b0
int ScriptCmdAddFlag(char **argv, int argc) {
    int v1;
    int v2;

    if (ScriptConditionActive == 0) {
        return 1;
    }
    if (FUN_004786c0((unsigned int)argv, argc, 5, 1) == 0) {
        return 0;
    }
    v1 = atoi(argv[1]);
    if (argc >= 2) {
        v2 = atoi(argv[2]);
    } else {
        v2 = 1;
    }
    if (DAT_00669054 == 1) {
        FUN_00468890(v1, v2);
    } else {
        NewScriptAddFlagEvent(v1, v2);
    }
    return 1;
}

// FUNCTION: LEGOLAND 0x0047a140
int ScriptCmdBridges(struct CommandArgs *arg, int argc) {
    int v1;
    int v2;

    if (ScriptConditionActive == 0) {
        return 1;
    }
    if (FUN_004786c0((unsigned int)arg, argc, 5, 1) == 0) {
        return 0;
    }
    v1 = atoi((char *)arg->arg1);
    if (v1 > 0) {
        v1 = v1 - 1;
    }
    if (argc >= 2) {
        v2 = atoi(arg->arg2);
    } else {
        v2 = 1;
    }
    if (DAT_00669054 == 1) {
        FUN_004688f0(v1, v2);
    } else {
        NewScriptBridgesEvent(v1, v2);
    }
    return 1;
}

// FUNCTION: LEGOLAND 0x0047a1d0
int ScriptCmdEndScreens(struct CommandArgs *arg, int argc) {
    char buf[0x200];
    int v;

    buf[0] = DAT_004d8bb0[0];
    memset(buf + 1, 0, sizeof(buf) - 1);
    if (ScriptConditionActive != 0) {
        if (FUN_004786c0((unsigned int)arg, argc, 5, 1) == 0) {
            return 0;
        }
        v = atoi((char *)arg->arg1);
        if (argc >= 2) {
            strcpy(buf, arg->arg2);
        }
        strcat(buf, ";");
        if (argc >= 3) {
            strcat(buf, arg->arg3);
        }
        if (DAT_00669054 == 1) {
            FUN_004597e0(v, buf);
        }
    }
    return 1;
}

// FUNCTION: LEGOLAND 0x0047a2f0
int ScriptCmdSelectTheme(struct CommandArgs *arg, int argc) {
    int index;

    if (ScriptConditionActive == 0) {
        return 1;
    }
    if (FUN_004786c0((unsigned int)arg, argc, 2, 0) != 0) {
        if (argc != 0) {
            index = FindStringNoCase((char *)arg->arg1, &DAT_004bb5c4, 5);
        } else {
            index = 0;
        }
        if (index != -1) {
            NewScriptSelectThemeEvent(CurrentObjectiveEventFlags, index);
            return 1;
        }
    }
    return 0;
}

// FUNCTION: LEGOLAND 0x0047a360
int ScriptCmdSelectTab(struct CommandArgs *arg, int argc) {
    int index;

    if (ScriptConditionActive == 0) {
        return 1;
    }
    if (FUN_004786c0((unsigned int)arg, argc, 2, 0) != 0) {
        if (argc != 0) {
            index = FindStringNoCase((char *)arg->arg1, &DAT_004bb5d8, 2);
        } else {
            index = 0;
        }
        if (index != -1) {
            NewScriptSelectTabEvent(CurrentObjectiveEventFlags, index);
            return 1;
        }
    }
    return 0;
}

// FUNCTION: LEGOLAND 0x0047a3d0
int ScriptCmdSelectMode(struct CommandArgs *arg, int argc) {
    int index;

    if (ScriptConditionActive == 0) {
        return 1;
    }
    if (FUN_004786c0((unsigned int)arg, argc, 2, 1) != 0) {
        if (argc != 0) {
            index = FindStringNoCase((char *)arg->arg1, &DAT_004bb5e0, 5);
        }
        if (index != -1) {
            NewScriptSelectModeEvent(CurrentObjectiveEventFlags, index);
            return 1;
        }
    }
    return 0;
}

// FUNCTION: LEGOLAND 0x0047a440
int ScriptCmdForever(struct CommandArgs *arg, int argc) {
    if (ScriptConditionActive != 0) {
        if (FUN_004786c0((unsigned int)arg, argc, 2, 0) == 0) {
            return 0;
        }
        NewScriptForeverEvent(CurrentObjectiveEventFlags);
    }
    return 1;
}

// FUNCTION: LEGOLAND 0x0047a480
int ScriptCmdGive(struct CommandArgs *arg, int argc) {
    unsigned int id;
    int popup = 1;

    if (ScriptConditionActive == 0) {
        return 1;
    }
    if (FUN_004786c0((unsigned int)arg, argc, 4, 1) == 0) {
        return 0;
    }
    id = ElemID((const char *)arg->arg1);
    if (argc >= 2) {
        // STRING: LEGOLAND 0x004bc0c4
        if (_stricmp(arg->arg2, "NOPOPUP") == 0) {
            popup = 0;
        }
    }
    if (id != 0) {
        NewScriptGiveEvent(id, popup);
    }
    return 1;
}

// FUNCTION: LEGOLAND 0x0047a500
int ScriptCmdTake(struct CommandArgs *arg, int argc) {
    unsigned int id;

    if (ScriptConditionActive == 0) {
        return 1;
    }
    if (FUN_004786c0((unsigned int)arg, argc, 4, 1) == 0) {
        return 0;
    }
    id = ElemID((const char *)arg->arg1);
    if (id != 0) {
        NewScriptTakeEvent(id);
    }
    return 1;
}

// FUNCTION: LEGOLAND 0x0047a550
int ScriptCmdAddBricks(struct CommandArgs *arg, int argc) {
    int v;

    if (ScriptConditionActive != 0) {
        if (FUN_004786c0((unsigned int)arg, argc, 4, 1) == 0) {
            return 0;
        }
        v = atoi((char *)arg->arg1);
        if (v > 0) {
            NewScriptAddBricksEvent(v);
        }
    }
    return 1;
}

// FUNCTION: LEGOLAND 0x0047a5a0
int ScriptCmdPlace(struct CommandArgs *arg, int argc) {
    unsigned int id;
    unsigned int coords[2];
    unsigned int v;

    if (ScriptConditionActive == 0) {
        return 1;
    }
    if (FUN_004786c0((unsigned int)arg, argc, 5, 3) == 0) {
        return 0;
    }
    id = ElemID((const char *)arg->arg1);
    ParseIntPair((int *)coords, (char **)arg, 2);
    if (argc >= 4) {
        v = atoi(((char **)arg)[4]);
    } else {
        v = 0;
    }
    if (id != 0) {
        if (DAT_00669054 == 1) {
            FUN_00469bd0(id, coords);
        } else {
            NewScriptPlaceEvent(id, coords, v);
        }
    }
    return 1;
}

// FUNCTION: LEGOLAND 0x0047a650
int ScriptCmdClear(struct CommandArgs *arg, int argc) {
    struct Vec4 vec;

    if (ScriptConditionActive != 0) {
        if (FUN_004786c0((unsigned int)arg, argc, 4, 4) == 0) {
            return 0;
        }
        ParseRect((int *)&vec, (char **)arg, 1);
        NewScriptClearEvent(&vec);
    }
    return 1;
}

// FUNCTION: LEGOLAND 0x0047a6a0
int ScriptCmdUnglue(struct CommandArgs *arg, int argc) {
    struct Vec4 vec;

    if (ScriptConditionActive != 0) {
        if (FUN_004786c0((unsigned int)arg, argc, 4, 4) == 0) {
            return 0;
        }
        ParseRect((int *)&vec, (char **)arg, 1);
        NewScriptUnglueEvent(&vec);
    }
    return 1;
}

// FUNCTION: LEGOLAND 0x0047a6f0
int ScriptCmdGlue(struct CommandArgs *arg, int argc) {
    int rect[4];
    int x;
    int y;

    if (ScriptConditionActive != 0) {
        if (FUN_004786c0((unsigned int)arg, argc, 5, 4) == 0) {
            return 0;
        }
        ParseRect(rect, (char **)arg, 1);
        if (DAT_00669054 == 4) {
            NewScriptGlueEvent((struct Vec4 *)rect);
            return 1;
        }
        for (y = rect[1]; y <= rect[3]; y++) {
            for (x = rect[0]; x <= rect[2]; x++) {
                GameMap[y][x].flags |= 0x40;
            }
        }
    }
    return 1;
}

// FUNCTION: LEGOLAND 0x0047a7b0
int ScriptCmdExtendPark(struct CommandArgs *arg, int argc) {
    struct Vec4 vec;

    if (ScriptConditionActive != 0) {
        if (FUN_004786c0((unsigned int)arg, argc, 4, 4) == 0) {
            return 0;
        }
        ParseRect((int *)&vec, (char **)arg, 1);
        NewScriptExtendParkEvent(&vec);
    }
    return 1;
}

// FUNCTION: LEGOLAND 0x0047a800
int ScriptCmdFmv(struct CommandArgs *arg, int argc) {
    if (ScriptConditionActive == 0) {
        return 1;
    }
    if (FUN_004786c0((unsigned int)arg, argc, 5, 1) == 0) {
        return 0;
    }
    if (DAT_00669054 == 4) {
        NewScriptFmvEvent(arg->arg1);
    } else {
        FUN_00490610((const char *)arg->arg1);
    }
    return 1;
}

// FUNCTION: LEGOLAND 0x0047a860
int ScriptCmdInterval(struct CommandArgs *arg, int argc) {
    if (ScriptConditionActive != 0) {
        if (FUN_004786c0((unsigned int)arg, argc, 4, 1) == 0) {
            return 0;
        }
        NewScriptIntervalEvent(arg->arg1);
    }
    return 1;
}

// FUNCTION: LEGOLAND 0x0047a8a0
int ScriptCmdMessage(struct CommandArgs *arg, int argc) {
    if (ScriptConditionActive != 0) {
        if (FUN_004786c0((unsigned int)arg, argc, 4, 1) == 0) {
            return 0;
        }
        NewScriptMessageEvent(arg->arg1);
    }
    return 1;
}

// FUNCTION: LEGOLAND 0x0047a8e0
int ScriptCmdFeature(struct CommandArgs *arg, int argc) {
    int index;
    unsigned int v;

    if (ScriptConditionActive != 0) {
        if (FUN_004786c0((unsigned int)arg, argc, 5, 2) == 0) {
            return 0;
        }
        index = FindStringNoCase((char *)arg->arg1, &DAT_004bb5f4, 0xc);
        v = atoi(arg->arg2);
        if (index == -1) {
            return 0;
        }
        if (DAT_00669054 == 4) {
            NewScriptFeatureEvent(index, v);
            return 1;
        }
        SetScriptFeature(index, v);
    }
    return 1;
}

// FUNCTION: LEGOLAND 0x0047a960
int ScriptCmdReport(struct CommandArgs *arg, int argc) {
    int is_off;
    int index;
    unsigned int v2;
    unsigned int v3;

    if (ScriptConditionActive != 0) {
        if (FUN_004786c0((unsigned int)arg, argc, 5, 2) == 0) {
            return 0;
        }
        // STRING: LEGOLAND 0x004bc0e4
        if (_stricmp(arg->arg2, "off") != 0) {
            is_off = 0;
            if (FUN_004786c0((unsigned int)arg, argc, 5, 3) == 0) {
                return 0;
            }
            v2 = atoi(arg->arg2);
            v3 = atoi(arg->arg3);
        } else {
            is_off = 1;
        }
        // STRING: LEGOLAND 0x004bc0d8
        if (_stricmp((char *)arg->arg1, "HAPPY_VIS") == 0) {
            // STRING: LEGOLAND 0x004bc0cc
            index = FindStringNoCase("Happpy_Vis", &DAT_004bb624, 0x19);
        } else {
            index = FindStringNoCase((char *)arg->arg1, &DAT_004bb624, 0x19);
        }
        if (index == -1) {
            return 0;
        }
        if (DAT_00669054 == 4) {
            if (is_off == 0) {
                NewScriptReportEvent(index, v2, v3);
                return 1;
            }
            NewScriptReportEvent(index, 0, 0);
            return 1;
        }
        if (is_off == 0) {
            FUN_0046a140(index, v2, v3);
            return 1;
        }
        FUN_0046a140(index, 0, 0);
    }
    return 1;
}

// FUNCTION: LEGOLAND 0x0047aa90
int ScriptCmdHapFactor(struct CommandArgs *arg, int argc) {
    int n;
    int r;

    if (ScriptConditionActive == 0) {
        return 1;
    }
    if (FUN_004786c0((unsigned int)arg, argc, 1, 2) == 0) {
        return 0;
    }
    n = atoi(arg->arg2);
    r = FindStringNoCase((char *)arg->arg1, &DAT_004bb688, 13);
    if (r == -1) {
        return 0;
    }
    SetMoodDelta(r, n);
    return 1;
}

// FUNCTION: LEGOLAND 0x0047ab00
int ScriptCmdCapacityScale(struct CommandArgs *arg, int argc) {
    int index;
    int value;

    if (ScriptConditionActive == 0) {
        return 1;
    }
    if (FUN_004786c0((unsigned int)arg, argc, 5, 2) == 0) {
        return 0;
    }
    index = FindStringNoCase((char *)arg->arg1, &DAT_004bb6bc, 6);
    value = atoi(arg->arg2);
    if (index == -1) {
        return 0;
    }
    if (DAT_00669054 == 1) {
        SetClassPercent(index, value);
    } else {
        NewScriptCapacityScaleEvent(index, value);
    }
    return 1;
}

// FUNCTION: LEGOLAND 0x0047ab80
int ScriptCmdCapacityCap(struct CommandArgs *arg, int argc) {
    int index;
    unsigned int v;

    if (ScriptConditionActive != 0) {
        if (FUN_004786c0((unsigned int)arg, argc, 5, 2) == 0) {
            return 0;
        }
        index = FindStringNoCase((char *)arg->arg1, &DAT_004bb6bc, 6);
        v = atoi(arg->arg2);
        if (index == -1) {
            return 0;
        }
        if (DAT_00669054 == 1) {
            SetClassLimit(index, v);
            return 1;
        }
        NewScriptCapacityCapEvent(index, v);
    }
    return 1;
}

// FUNCTION: LEGOLAND 0x0047ac00
int ScriptCmdDegrade(struct CommandArgs *arg, int argc) {
    unsigned int id;
    unsigned int v2;
    unsigned int v3;

    if (ScriptConditionActive == 0) {
        return 1;
    }
    if (FUN_004786c0((unsigned int)arg, argc, 4, 2) == 0) {
        return 0;
    }
    id = ElemID((const char *)arg->arg1);
    v2 = atoi(arg->arg2);
    if (argc >= 3) {
        v3 = atoi(arg->arg3);
    } else {
        v3 = 0;
    }
    if (id != 0) {
        NewScriptDegradeEvent(id, v2, v3);
    }
    return 1;
}

// FUNCTION: LEGOLAND 0x0047ac80
int ScriptCmdMaxBlokes(struct CommandArgs *arg, int argc) {
    if (ScriptConditionActive != 0) {
        if (FUN_004786c0((unsigned int)arg, argc, 1, 1) == 0) {
            return 0;
        }
        lpConfig->max_blokes = (unsigned short)atoi((char *)arg->arg1);
        MapStats.capacity_max = lpConfig->max_blokes;
        MapStats.capacity_min = 0;
    }
    return 1;
}

// FUNCTION: LEGOLAND 0x0047ace0
int ScriptCmdMaxCapacity(struct CommandArgs *arg, int argc) {
    unsigned int v;

    if (ScriptConditionActive != 0) {
        if (FUN_004786c0((unsigned int)arg, argc, 5, 1) == 0) {
            return 0;
        }
        v = atoi((char *)arg->arg1);
        if (DAT_00669054 == 1) {
            MapStats.capacity_max = v;
            return 1;
        }
        FUN_0046bb80(1, v);
    }
    return 1;
}

// FUNCTION: LEGOLAND 0x0047ad40
int ScriptCmdMinCapacity(struct CommandArgs *arg, int argc) {
    unsigned int value;

    if (ScriptConditionActive == 0) {
        return 1;
    }
    if (FUN_004786c0((unsigned int)arg, argc, 5, 1) == 0) {
        return 0;
    }
    value = atoi((char *)arg->arg1);
    if (DAT_00669054 == 1) {
        MapStats.capacity_min = value;
    } else {
        FUN_0046bb80(0, value);
    }
    return 1;
}

// FUNCTION: LEGOLAND 0x0047ada0
int ScriptCmdEntranceFee(struct CommandArgs *arg, int argc) {
    unsigned int v;

    if (ScriptConditionActive != 0) {
        if (FUN_004786c0((unsigned int)arg, argc, 5, 1) == 0) {
            return 0;
        }
        v = atoi((char *)arg->arg1);
        if (DAT_00669054 == 1) {
            MapStats.entrance_fee = v;
            return 1;
        }
        NewScriptEntranceFeeEvent(v);
    }
    return 1;
}

// FUNCTION: LEGOLAND 0x0047ae00
int ScriptCmdFlashButton(char **argv, int argc) {
    int index;
    int i;
    unsigned int mask;

    mask = 0;
    if (ScriptConditionActive != 0) {
        if (FUN_004786c0((unsigned int)argv, argc, 5, 1) == 0) {
            return 0;
        }
        for (i = 1; i <= argc; i++) {
            index = FindStringNoCase(argv[i], &DAT_004bb6d4, 9);
            if (index != -1) {
                mask |= 1 << index;
            } else {
                mask |= atoi(argv[i]);
            }
        }
        if (DAT_00669054 == 1) {
            FUN_00476070(mask, 1);
        } else {
            FUN_0046bd70(mask, 1);
        }
    }
    return 1;
}

// FUNCTION: LEGOLAND 0x0047aea0
int ScriptCmdFlashButtonOff(char **argv, int argc) {
    int index;
    int i;
    unsigned int mask;

    mask = 0;
    if (ScriptConditionActive != 0) {
        if (FUN_004786c0((unsigned int)argv, argc, 5, 0) == 0) {
            return 0;
        }
        if (argc == 0) {
            mask = 0xffffffff;
        }
        for (i = 1; i <= argc; i++) {
            index = FindStringNoCase(argv[i], &DAT_004bb6d4, 9);
            if (index != -1) {
                mask |= 1 << index;
            } else {
                mask |= atoi(argv[i]);
            }
        }
        if (DAT_00669054 == 1) {
            FUN_00476070(mask, 0);
        } else {
            FUN_0046bd70(mask, 0);
        }
    }
    return 1;
}

// FUNCTION: LEGOLAND 0x0047af50
int ScriptCmdPurge(unsigned int param_1, unsigned int param_2) {
    if (ScriptConditionActive == 0) {
        return 1;
    }
    if (FUN_004786a0(param_1, param_2, 4) == 0) {
        return 0;
    }
    NewScriptPurgeEvent();
    return 1;
}

// FUNCTION: LEGOLAND 0x0047af80
int ScriptCmdEndLevel(unsigned int param_1, unsigned int param_2) {
    if (ScriptConditionActive != 0) {
        if (FUN_004786a0(param_1, param_2, 4) == 0) {
            return 0;
        }
        NewScriptEndLevelEvent();
        return 1;
    }
    return 1;
}

// FUNCTION: LEGOLAND 0x0047afb0
int FUN_0047afb0(const char *param_1) {
    int result;
    int v1;

    FUN_00492980();
    LLIDB_ClearOnLevel();
    FUN_004784c0();
    result = ParseScriptFile(param_1, DAT_004bb6f8, 0x5d, 0);
    FUN_00492990();
    v1 = (result >= 0) - 1;
    return v1 & 2;
}
