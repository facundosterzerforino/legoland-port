/* Debug console for playtesting: see ext_console.h. The list of commands for players is in
 * docs/debug-console.md; keep both in step. */
#include <windows.h>

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ext_console.h"

#include "../src/legoland/bricks.h"
#include "../src/legoland/challenge.h"
#include "../src/legoland/clipping.h"
#include "../src/legoland/debug.h"
#include "../src/legoland/globals.h"
#include "../src/legoland/map_object.h"
#include "../src/legoland/nerps.h"
#include "../src/legoland/obj_instance.h"
#include "../src/legoland/objectives.h"
#include "../src/legoland/profile_io.h"
#include "../src/legoland/screens.h"
#include "../src/legoland/sound_sfx.h"
#include "../src/legoland/timer.h"
#include "../src/legoland/wndenv.h"

int ExtConsole = 0;

#define CON_QUEUE 16
#define CON_LINE 256

static HANDLE ConIn;
static HANDLE ConOut;
static CRITICAL_SECTION ConLock;
static char ConQueue[CON_QUEUE][CON_LINE];
static int ConHead;
static int ConCount;

static void ConPrintf(const char *fmt, ...) {
    char buffer[1024];
    va_list ap;
    DWORD written;
    int len;

    va_start(ap, fmt);
    len = vsnprintf(buffer, sizeof(buffer) - 2, fmt, ap);
    va_end(ap);
    if (len < 0) {
        return;
    }
    if (len > (int)sizeof(buffer) - 3) {
        len = (int)sizeof(buffer) - 3;
    }
    buffer[len] = '\r';
    buffer[len + 1] = '\n';
    if (ConOut != NULL && ConOut != INVALID_HANDLE_VALUE) {
        WriteConsoleA(ConOut, buffer, (DWORD)len + 2, &written, NULL);
    }
    buffer[len] = 0;
    DebugTrace("console: %s", buffer);
}

/* Ctrl+C / Ctrl+Break in the console would end the game: ignore them. (Closing the console window still
 * ends it; Windows doesn't let a program refuse that.) */
static BOOL WINAPI ConCtrlHandler(DWORD type) {
    return type == CTRL_C_EVENT || type == CTRL_BREAK_EVENT;
}

static DWORD WINAPI ConReaderThread(LPVOID unused) {
    char line[CON_LINE];
    DWORD got;
    HWND hwnd;

    (void)unused;
    for (;;) {
        if (!ReadConsoleA(ConIn, line, sizeof(line) - 1, &got, NULL)) {
            Sleep(100);
            continue;
        }
        while (got > 0 && (line[got - 1] == '\n' || line[got - 1] == '\r')) {
            got--;
        }
        line[got] = 0;
        EnterCriticalSection(&ConLock);
        if (ConCount < CON_QUEUE) {
            memcpy(ConQueue[(ConHead + ConCount) % CON_QUEUE], line, got + 1);
            ConCount++;
        }
        LeaveCriticalSection(&ConLock);
        /* wake the game if it is waiting for its window to be active again (ProcessSystemEvents) */
        hwnd = WNDENV_Gethwnd();
        if (hwnd != NULL) {
            PostMessageA(hwnd, WM_NULL, 0, 0);
        }
    }
}

void ExtConsoleStart(void) {
    if (!AllocConsole()) {
        DebugTrace("console: AllocConsole failed (%lu)", GetLastError());
        return;
    }
    SetConsoleTitleA("LEGOLAND debug console");
    ConIn = CreateFileA("CONIN$", GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
    ConOut = CreateFileA("CONOUT$", GENERIC_READ | GENERIC_WRITE, FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
    SetConsoleCtrlHandler(ConCtrlHandler, TRUE);
    InitializeCriticalSection(&ConLock);
    ConPrintf("LEGOLAND debug console. Type \"help\" for the commands.");
    ConPrintf("Commands run while the game is running (most need a level to be loaded).");
    ExtConsole = 1;
    CloseHandle(CreateThread(NULL, 0, ConReaderThread, NULL, 0, NULL));
}

/* ---- the game's state ---- */

static int InLevel(void) {
    return EditMode.unk4 == 3;
}

static int NeedLevel(void) {
    if (!InLevel()) {
        ConPrintf("Only works while a level is loaded.");
        return 0;
    }
    return 1;
}

static const char *LevelName(unsigned int level) {
    static char name[32];

    if (level >= 1 && level <= 5) {
        sprintf(name, "tutorial %u", level);
    } else if (level >= 6 && level <= 15) {
        sprintf(name, "game level %u", level - 5);
    } else {
        sprintf(name, "level %u (free play or none)", level);
    }
    return name;
}

/* Event type -> script keyword (from the keyword tables at 0x4bb6f0 and their builders in nerps.c). */
static const char *EventKeyword(unsigned int type) {
    static const struct {
        unsigned int type;
        const char *name;
    } names[] = {
        {0x00, "(done/skipped)"},
        {0x01, "MESSAGE (INTRO/PROMPT)"},
        {0x02, "GIVE"},
        {0x04, "TAKE"},
        {0x05, "ADDBRICKS"},
        {0x06, "CURRENCY"},
        {0x07, "PLACE"},
        {0x08, "CLEAR"},
        {0x09, "UNGLUE"},
        {0x0a, "GLUE"},
        {0x0b, "EXTENDPARK"},
        {0x0c, "FMV"},
        {0x0d, "INTERVAL"},
        {0x0e, "MESSAGE"},
        {0x0f, "FEATURE"},
        {0x10, "GARDENER/MECHANIC"},
        {0x11, "WORKERS"},
        {0x12, "DEGRADE"},
        {0x13, "MAX/MIN VISITORS"},
        {0x14, "CAPACITYSCALE"},
        {0x15, "CAPACITYCAP"},
        {0x16, "ENTRANCEFEE"},
        {0x17, "LOOKAT"},
        {0x18, "REPORT"},
        {0x19, "THEMEICON"},
        {0x1a, "ADDFLAG"},
        {0x1b, "BRIDGES"},
        {0x1c, "BRIEFINGFILE"},
        {0x1d, "HINTSFILE"},
        {0x1e, "FLASHBUTTON"},
        {0x1f, "PURGE"},
        {0x20, "ENDLEVEL"},
        {0x21, "NEED"},
        {0x22, "NEEDAT"},
        {0x23, "NEEDIN"},
        {0x24, "CONNECT"},
        {0x25, "LINK"},
        {0x26, "RANGE"},
        {0x27, "CLEARAREA"},
        {0x28, "REMOVE"},
        {0x29, "REMOVERANGE"},
        {0x2a, "COMPOSITE"},
        {0x2b, "LOOPCOMPOSITE"},
        {0x2c, "TECHLEVEL"},
        {0x2d, "PARKVISITORS"},
        {0x2e, "RIDEVISITORS"},
        {0x2f, "RIDERS"},
        {0x30, "SCENERYCOVERAGE"},
        {0x31, "PATHSCENERY"},
        {0x32, "RIDECOVERAGE"},
        {0x33, "SHOPCOVERAGE"},
        {0x34, "FOODCOVERAGE"},
        {0x35, "TOTCOVERAGE"},
        {0x37, "STUDAREA"},
        {0x38, "SAVE"},
        {0x39, "HAPPINESS"},
        {0x3a, "NEEDGARDENERS"},
        {0x3b, "NEEDMECHANICS"},
        {0x3c, "HUNGER"},
        {0x3d, "FIXRIDES"},
        {0x3e, "POWERRIDES"},
        {0x3f, "ZONING"},
        {0x40, "CHECKFLAG"},
        {0x41, "SELECTTHEME"},
        {0x42, "SELECTTAB"},
        {0x43, "SELECTMODE"},
        {0x45, "FOREVER"},
    };
    static char unknown[16];
    unsigned int i;

    for (i = 0; i < sizeof(names) / sizeof(names[0]); i++) {
        if (names[i].type == type) {
            return names[i].name;
        }
    }
    sprintf(unknown, "type 0x%x", type);
    return unknown;
}

/* Types whose field_4 is an object/theme element (its first field is the name; see SaveObjectiveEventList). */
static int EventHasElement(unsigned int type) {
    switch (type) {
    case 0x02:
    case 0x04:
    case 0x21:
    case 0x22:
    case 0x23:
    case 0x24:
    case 0x25:
    case 0x26:
    case 0x28:
    case 0x29:
    case 0x2a:
    case 0x2b:
    case 0x2e:
    case 0x2f:
        return 1;
    }
    return 0;
}

/* Objectives (conditions the step waits for) are the types from NEED on, plus FOREVER. */
static int EventIsCondition(unsigned int type) {
    return type >= 0x21 && type <= 0x45;
}

/* The steps still to run are ScriptSectionList, sorted, with the current one (CurrentScriptSection) at its head until it
 * is done (RunLevelScript). */
static int IsLastStep(void) {
    struct SortNode *current;

    current = (struct SortNode *)ScriptSectionList;
    return current != NULL && (unsigned int)current == CurrentScriptSection && current->next == NULL;
}

/* What RANGE counts (ScriptEventRange): the object classes of a theme that have objects in the park, and how many
 * objects they have in all. 0x08 is the class's object count, 0x5c its theme (struct ObjectClass in nerps.c). */
static void RangeCount(unsigned int theme, int *kinds, int *placed) {
    struct Ride *cls;
    unsigned int count;

    *kinds = 0;
    *placed = 0;
    for (cls = ObjectClassList; cls != NULL; cls = cls->next) {
        count = *(const unsigned int *)((const char *)cls + 0x08);
        if (*(const unsigned int *)((const char *)cls + 0x5c) == theme && count != 0) {
            *kinds += 1;
            *placed += (int)count;
        }
    }
}

static const char *EventKind(unsigned char flags) {
    if (flags & 2) {
        return "PERMANENT";
    }
    if (flags & 4) {
        return "REMINDER";
    }
    if (flags & 1) {
        return "ongoing";
    }
    return "one-off";
}

/* ---- commands ---- */

static void CmdHelp(void) {
    ConPrintf("Info:");
    ConPrintf("  status               level, money, visitors, script and inspector state");
    ConPrintf("  objectives (obj)     what the level script is waiting for right now");
    ConPrintf("  note <text>          write a marker line into legoland-port-trace.txt");
    ConPrintf("Level flow (need a level loaded):");
    ConPrintf("  skip                 complete the current step (its rewards are given as normal)");
    ConPrintf("  skip perm            also drop unmet PERMANENT objectives that block the script");
    ConPrintf("  win | lose           end the level now, won or lost");
    ConPrintf("  level t1..t5 | 1..10 go to tutorial 1-5 or game level 1-10 (through the progress screen)");
    ConPrintf("  appraise             the inspector comes now");
    ConPrintf("  inspector <minutes>  set the time between appraisals (0 = no inspector)");
    ConPrintf("  stopscript           stop the level script (original cheat IMPROVISE; usually ends the level as won)");
    ConPrintf("Park (need a level loaded):");
    ConPrintf("  money <n>            add n coins (negative takes coins away)");
    ConPrintf("  money set <n>        set the coins to n");
    ConPrintf("  ridewear <n>         ride wear level (0 = rides never wear out; scripts use 0-3)");
    ConPrintf("  capacity             show the capacity calculations (original cheat SHOWCAPACITY)");
    ConPrintf("  switches             original cheat DIGGER (sets map switches 0-3)");
    ConPrintf("Other:");
    ConPrintf("  music theme|egypt|inca|castle|west|stop");
    ConPrintf("  unlock               mark all 15 levels completed and unlock every free play object (saved!)");
}

static void CmdStatus(void) {
    unsigned int now;

    now = GetGameTimer();
    ConPrintf("screen: %s", InLevel() ? "in a level" : "not in a level (menu, briefing or loading)");
    ConPrintf("level: %s", LevelName(lpConfig->level));
    if (!InLevel()) {
        return;
    }
    ConPrintf("game time: %u:%02u%s", now / 60000, (now / 1000) % 60,
        GameTimerPaused == 0                ? ""
            : (lpConfig->field_1c & 1) != 0 ? " (paused while the game window is in the background)"
                                            : " (game clock PAUSED: scripts and the inspector wait)");
    if (AreBricksLimited()) {
        ConPrintf("money: %d coins", GetBrickCount());
    } else {
        ConPrintf("money: unlimited");
    }
    ConPrintf("visitors: %d", ParkVisitorCount);
    ConPrintf("ride wear: %u", MapStats.ride_wear);
    ConPrintf("script: %s", IsScriptStopped() != 0 ? "stopped" : "running");
    if (MapStats.timer_minutes == 0 || AppraisalDeadline == 0) {
        ConPrintf("inspector: off");
    } else {
        int left = (int)(AppraisalDeadline - now);
        if (left < 0) {
            left = 0;
        }
        ConPrintf("inspector: every %u min, next in %d:%02d (game time)", MapStats.timer_minutes, left / 60000,
            (left / 1000) % 60);
    }
    if (MapStats.appraisal_streak > 0) {
        ConPrintf("appraisals: %d passed in a row", MapStats.appraisal_streak);
    } else if (MapStats.appraisal_streak < 0) {
        ConPrintf("appraisals: %d failed in a row, the level is lost at %d", -MapStats.appraisal_streak,
            MapStats.appraisal_fail_limit);
    } else {
        ConPrintf("appraisals: none yet (the level is lost after %d failed in a row)", MapStats.appraisal_fail_limit);
    }
}

static void CmdObjectives(void) {
    struct ObjectiveEvent *node;
    int shown;
    int waiting;

    if (!NeedLevel()) {
        return;
    }
    shown = 0;
    waiting = 0;
    for (node = (struct ObjectiveEvent *)ScriptEventList; node != NULL; node = node->next) {
        char what[160];
        const char *state;

        if (!EventIsCondition(node->type)) {
            continue;
        }
        what[0] = 0;
        if (EventHasElement(node->type)) {
            if (node->field_4 != 0 && *(const char **)node->field_4 != NULL) {
                snprintf(what, sizeof(what), " \"%s\"", *(const char **)node->field_4);
            } else if (node->type == 0x25) {
                snprintf(what, sizeof(what), " ALL");
            }
        }
        if (node->type == 0x26) {
            size_t len = strlen(what);
            int kinds;
            int placed;
            RangeCount(node->field_4, &kinds, &placed);
            snprintf(what + len, sizeof(what) - len, ": %d different kinds (have %d), %d placed in all (have %d)",
                (int)node->field_14, kinds, (int)node->field_1c, placed);
        } else if (node->type != 0x45 && node->type != 0x41 && node->type != 0x42 && node->type != 0x43) {
            size_t len = strlen(what);
            snprintf(what + len, sizeof(what) - len, " %d", (int)node->field_1c);
        }
        if (node->type == 0x45) {
            state = "never done: the level now waits for the inspector";
        } else if (node->flags_10 & 0x80) {
            state = "met";
        } else {
            state = "NOT met";
            if ((node->flags_10 & 4) == 0) {
                waiting++;
            }
        }
        ConPrintf("  %-9s %s%s  -> %s", EventKind(node->flags_10), EventKeyword(node->type), what, state);
        shown++;
    }
    if (shown == 0) {
        ConPrintf("No objectives in the script right now.");
    } else {
        ConPrintf("%d objective(s) not met (PERMANENT and one-off/ongoing ones block the next step).", waiting);
    }
    if (IsLastStep()) {
        ConPrintf("This is the last step of the level script.");
    }
}

static void CmdSkip(int permanent_too) {
    struct ObjectiveEvent *node;
    int skipped;
    int blocking;

    if (!NeedLevel()) {
        return;
    }
    skipped = 0;
    blocking = 0;
    /* Don't free nodes here (a popup may be running the message loop from inside the script runner, which
     * is walking this list): make them type 0, which has no handler. The runner then skips them, doesn't
     * count them, and frees the step's nodes itself when it moves on (RunLevelScript). */
    for (node = (struct ObjectiveEvent *)ScriptEventList; node != NULL; node = node->next) {
        if (!EventIsCondition(node->type) || (node->flags_10 & 0x80) != 0 || (node->flags_10 & 4) != 0) {
            continue;
        }
        if ((node->flags_10 & 2) != 0 && !permanent_too) {
            blocking++;
            continue;
        }
        ConPrintf("  skipped: %s %s", EventKind(node->flags_10), EventKeyword(node->type));
        node->type = 0;
        node->flags_10 |= 0x80;
        skipped++;
    }
    if (skipped == 0) {
        ConPrintf("Nothing to skip.");
    }
    if (blocking != 0) {
        ConPrintf("%d unmet PERMANENT objective(s) still block the script; \"skip perm\" drops them.", blocking);
    }
    if (IsLastStep()) {
        ConPrintf("That was the last step: if the level doesn't end, use \"appraise\" or \"win\".");
    }
}

static void CmdLevel(const char *arg) {
    unsigned int level;

    if (!NeedLevel()) {
        return;
    }
    level = 0;
    if ((arg[0] == 't' || arg[0] == 'T') && arg[1] >= '1' && arg[1] <= '5' && arg[2] == 0) {
        level = arg[1] - '0';
    } else if (atoi(arg) >= 1 && atoi(arg) <= 10) {
        level = atoi(arg) + 5;
    }
    if (level == 0) {
        ConPrintf("usage: level t1..t5 | 1..10");
        return;
    }
    /* as the original cheat ILIKETOTRAVEL (input.c) */
    lpConfig->level = level;
    MapStats.level_end = 2;
    ConPrintf("Going to %s.", LevelName(level));
}

static void CmdMusic(const char *arg) {
    static const char *themes[] = {"theme", "egypt", "inca", "castle", "west"};
    int i;

    if (_stricmp(arg, "stop") == 0) {
        StopInteractiveMusic();
        ConPrintf("Music stopped.");
        return;
    }
    for (i = 0; i < 5; i++) {
        if (_stricmp(arg, themes[i]) == 0) {
            SetInteractiveMusicTheme(i);
            ConPrintf("Music theme: %s.", themes[i]);
            return;
        }
    }
    ConPrintf("usage: music theme|egypt|inca|castle|west|stop");
}

/* Splits off the next word (spaces and tabs separate words); returns "" at the end of the line. */
static char *NextWord(char **line) {
    char *p;
    char *word;

    p = *line;
    while (*p == ' ' || *p == '\t') {
        p++;
    }
    word = p;
    while (*p != 0 && *p != ' ' && *p != '\t') {
        p++;
    }
    if (*p != 0) {
        *p++ = 0;
    }
    *line = p;
    return word;
}

static void RunCommand(char *line) {
    char *cmd;
    char *arg;
    char *arg2;

    cmd = NextWord(&line);
    if (cmd[0] == 0) {
        return;
    }
    arg = NextWord(&line);
    while (*line == ' ' || *line == '\t') {
        line++;
    }
    arg2 = line[0] != 0 ? line : NULL;
    ConPrintf("> %s %s%s%s", cmd, arg, arg2 != NULL ? " " : "", arg2 != NULL ? arg2 : "");

    if (_stricmp(cmd, "help") == 0 || strcmp(cmd, "?") == 0) {
        CmdHelp();
    } else if (_stricmp(cmd, "status") == 0) {
        CmdStatus();
    } else if (_stricmp(cmd, "objectives") == 0 || _stricmp(cmd, "obj") == 0) {
        CmdObjectives();
    } else if (_stricmp(cmd, "note") == 0) {
        ConPrintf("NOTE: %s%s%s", arg, arg2 != NULL ? " " : "", arg2 != NULL ? arg2 : "");
    } else if (_stricmp(cmd, "skip") == 0) {
        CmdSkip(_stricmp(arg, "perm") == 0);
    } else if (_stricmp(cmd, "win") == 0) {
        if (NeedLevel()) {
            EndLevel(1);
            ConPrintf("Level won.");
        }
    } else if (_stricmp(cmd, "lose") == 0) {
        if (NeedLevel()) {
            EndLevel(2);
            ConPrintf("Level lost.");
        }
    } else if (_stricmp(cmd, "level") == 0) {
        CmdLevel(arg);
    } else if (_stricmp(cmd, "appraise") == 0) {
        if (NeedLevel()) {
            AppraisalDeadline = 1; /* as the original cheat PRAISEME */
            ConPrintf("The inspector is coming.");
        }
    } else if (_stricmp(cmd, "inspector") == 0) {
        if (NeedLevel()) {
            if (arg[0] == 0) {
                ConPrintf("usage: inspector <minutes>");
            } else {
                MapStats.timer_minutes = (unsigned int)atoi(arg);
                StartAppraisalTimer();
                ConPrintf("Inspector: %s.", MapStats.timer_minutes != 0 ? "timer restarted" : "off");
            }
        }
    } else if (_stricmp(cmd, "stopscript") == 0) {
        if (NeedLevel()) {
            SetScriptStopped(1); /* as the original cheat IMPROVISE */
            ConPrintf("Script stopped.");
        }
    } else if (_stricmp(cmd, "money") == 0) {
        if (NeedLevel()) {
            if (_stricmp(arg, "set") == 0 && arg2 != NULL) {
                SetBrickCount((unsigned int)atoi(arg2));
            } else if (arg[0] != 0) {
                AddBricks((unsigned int)atoi(arg));
            } else {
                ConPrintf("usage: money <n> | money set <n>");
                return;
            }
            ConPrintf("Money: %d coins.", GetBrickCount());
        }
    } else if (_stricmp(cmd, "ridewear") == 0) {
        if (NeedLevel()) {
            if (arg[0] == 0) {
                ConPrintf("usage: ridewear <n>");
            } else {
                MapStats.ride_wear = (unsigned int)atoi(arg);
                ConPrintf("Ride wear: %u.", MapStats.ride_wear);
            }
        }
    } else if (_stricmp(cmd, "capacity") == 0) {
        if (NeedLevel()) {
            MapStats.show_capacity = 1;
            ConPrintf("Capacity calculations visible.");
        }
    } else if (_stricmp(cmd, "switches") == 0) {
        if (NeedLevel()) {
            FUN_00460560(0);
            FUN_00460560(1);
            FUN_00460560(2);
            FUN_00460560(3);
            ConPrintf("Switches 0-3 set.");
        }
    } else if (_stricmp(cmd, "music") == 0) {
        CmdMusic(arg);
    } else if (_stricmp(cmd, "unlock") == 0) {
        struct ClipQueryResult *entry;
        int unlocked;
        int level;

        for (level = 1; level <= 15; level++) {
            CurrentProfile.flags[3 + level] = 1;
        }
        /* Free play lists an object once its byte in field_46 is set, which the game does the first time the
         * object is built in a level (FUN_0048a6e0): set it for every object in that table. */
        unlocked = 0;
        for (entry = DAT_004bdeb8; entry < DAT_004bdeb8 + 0x86 && entry->name != NULL && entry->name[0] != '\0';
            entry++) {
            if (entry->id < sizeof(CurrentProfile.field_46)) {
                unlocked += CurrentProfile.field_46[entry->id] == 0;
                CurrentProfile.field_46[entry->id] = 1;
            }
        }
        UpDateCurrentProfile();
        ConPrintf("All levels marked as completed and %d more free play objects unlocked in the current profile.",
            unlocked);
    } else {
        ConPrintf("Unknown command \"%s\". Type \"help\".", cmd);
    }
}

void ExtConsolePoll(void) {
    char line[CON_LINE];

    if (!ExtConsole) {
        return;
    }
    for (;;) {
        EnterCriticalSection(&ConLock);
        if (ConCount == 0) {
            LeaveCriticalSection(&ConLock);
            return;
        }
        memcpy(line, ConQueue[ConHead], sizeof(line));
        ConHead = (ConHead + 1) % CON_QUEUE;
        ConCount--;
        LeaveCriticalSection(&ConLock);
        RunCommand(line);
    }
}
