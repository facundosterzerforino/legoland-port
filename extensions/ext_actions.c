/* Action log: see ext_actions.h. */
#include <windows.h>

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "ext_actions.h"

#include "../src/legoland/controller.h"
#include "../src/legoland/debug.h"
#include "../src/legoland/globals.h"
#include "../src/legoland/icon.h"
#include "../src/legoland/image_sprite.h"
#include "../src/legoland/llidb.h"
#include "../src/legoland/obj_instance.h"

int ExtActionLog = 1;

#define LINES_PER_SECOND 40

static DWORD window_start;
static int window_lines;
static int skipped;

/* text for a trace line: control characters (a tooltip's line breaks) as spaces, at most size - 1 characters */
static void Clean(char *out, size_t size, const char *text) {
    size_t n = 0;

    for (; text != NULL && *text != 0 && n + 1 < size; text++) {
        out[n++] = (unsigned char)*text < 0x20 ? ' ' : *text;
    }
    out[n] = 0;
}

static void Emit(const char *prefix, const char *fmt, va_list ap) {
    char line[512];
    char *p;
    DWORD now;

    if (!ExtActionLog) {
        return;
    }
    now = GetTickCount();
    if (now - window_start >= 1000) {
        if (skipped > 0) {
            DebugTrace("action: (%d more lines in a second, not shown)", skipped);
            skipped = 0;
        }
        window_start = now;
        window_lines = 0;
    }
    if (window_lines >= LINES_PER_SECOND) {
        skipped++;
        return;
    }
    window_lines++;
    vsnprintf(line, sizeof(line), fmt, ap);
    line[sizeof(line) - 1] = 0;
    for (p = line; *p != 0; p++) {
        if ((unsigned char)*p < 0x20) {
            *p = ' '; /* one trace line each: the advisor's and the game's messages have line breaks */
        }
    }
    DebugTrace("%s%s", prefix, line);
}

void ExtAction(const char *fmt, ...) {
    va_list ap;

    va_start(ap, fmt);
    Emit("action: ", fmt, ap);
    va_end(ap);
}

/* DBPrintf messages the game prints every frame or for every sound and person: left out */
static const char *const ChattyMessages[] = {
    "[Bloke",
    "Fading",
    "\tStopping",
    "Autokilling",
    "Sample should",
    "Allocated Workorder",
    "unlinking",
    "   ",
    "IMT:",
    "Creating Cell",
    "Deleting Cell",
    "DC=",
    "Processing your bloke",
    "Killing Joust",
    "Picking up worker",
};

void ExtActionGameMessage(const char *format, va_list ap) {
    char text[400];
    size_t i;
    size_t n;

    if (!ExtActionLog || format == NULL) {
        return;
    }
    for (i = 0; i < sizeof(ChattyMessages) / sizeof(ChattyMessages[0]); i++) {
        if (strncmp(format, ChattyMessages[i], strlen(ChattyMessages[i])) == 0) {
            return;
        }
    }
    vsnprintf(text, sizeof(text), format, ap);
    text[sizeof(text) - 1] = 0;
    n = strlen(text);
    while (n > 0 && (text[n - 1] == '\n' || text[n - 1] == '\r')) {
        text[--n] = 0;
    }
    ExtAction("game says \"%s\"", text);
}

/* ---- what things are, in words ---- */

static void DescribeIcon(const struct IconNode *icon, char *out, size_t size) {
    char text[160];
    const struct Sprite *sprite;
    const struct Image *image;

    if (icon == NULL) {
        snprintf(out, size, "a button");
        return;
    }
    if ((icon->flags & 0x1000) != 0 && icon->field_8 != NULL) {
        /* a build menu's object (help.c shows its name the same way) */
        const struct Ride *ride = (const struct Ride *)icon->field_8;
        if (ride->element != NULL && ride->element->name != NULL) {
            Clean(text, sizeof(text), ride->element->name);
            snprintf(out, size, "the build menu's \"%s\"", text);
            return;
        }
    }
    if (icon->string != NULL && icon->string[0] != 0) {
        Clean(text, sizeof(text), icon->string);
        snprintf(out, size, "button \"%s\" (icon %u)", text, (unsigned int)icon->id);
        return;
    }
    /* no tooltip: the picture's file, for a bitmap that kept its name (CreateSourceImage puts it after the image) */
    sprite = icon->sprite;
    if (sprite != NULL && (sprite->flags & 0x8020) == 0) {
        image = sprite->image;
        if (image != NULL && image->field_14 <= 1 && image->name == (const char *)(image + 1)) {
            Clean(text, sizeof(text), image->name);
            snprintf(out, size, "button %s (icon %u)", text, (unsigned int)icon->id);
            return;
        }
    }
    snprintf(out, size, "button (icon %u)", (unsigned int)icon->id);
}

const char *ExtActionHover(int type, const void *ptr) {
    static char out[220];
    char text[160];

    switch (type) {
    case 2:
        DescribeIcon((const struct IconNode *)ptr, out, sizeof(out));
        break;
    case 0x103:
        if (ptr != NULL && ((const struct Element *)ptr)->name != NULL) {
            Clean(text, sizeof(text), ((const struct Element *)ptr)->name);
            snprintf(out, sizeof(out), "object \"%s\"", text);
        } else {
            snprintf(out, sizeof(out), "an object");
        }
        break;
    case 0x306:
        snprintf(out, sizeof(out), "a visitor");
        break;
    case 0x307:
        snprintf(out, sizeof(out), "a gardener");
        break;
    case 0x308:
        snprintf(out, sizeof(out), "a mechanic");
        break;
    case 0x10b:
        snprintf(out, sizeof(out), "a gardener's job");
        break;
    case 0x10c:
        snprintf(out, sizeof(out), "a mechanic's job");
        break;
    case 0x109:
    case 0x10d:
        snprintf(out, sizeof(out), "the ground");
        break;
    case 0x10a:
        snprintf(out, sizeof(out), "outside the park");
        break;
    case 0:
    case 0x100:
        snprintf(out, sizeof(out), "nothing");
        break;
    default:
        snprintf(out, sizeof(out), "hover type 0x%x", (unsigned int)type);
        break;
    }
    return out;
}

static const char *ScreenName(unsigned int screen) {
    switch (screen) {
    case 1:
        return "the map";
    case 2:
        return "the park";
    case 3:
        return "the menus";
    default:
        return "?";
    }
}

static const char *ThemeName(unsigned int theme) {
    static const char *const names[] = {"LEGOLAND", "WESTERN", "CASTLE", "ADVENTURER", "NONE", "closed"};
    return theme < 6 ? names[theme] : "?";
}

static void DescribeMode(char *out, size_t size) {
    char text[120];
    const struct Ride *ride = EditMode.unk8;

    switch (EditMode.unk0) {
    case 0:
        snprintf(out, size, "query");
        break;
    case 2:
        snprintf(out, size, "erase");
        break;
    case 1:
        if ((const void *)ride == PathControlObject) {
            snprintf(out, size, "path");
        } else if (ride != NULL && ride->element != NULL && ride->element->name != NULL) {
            Clean(text, sizeof(text), ride->element->name);
            snprintf(out, size, "build \"%s\"", text);
        } else {
            snprintf(out, size, "build");
        }
        break;
    default:
        snprintf(out, size, "%u", EditMode.unk0);
        break;
    }
}

static const char *KeyName(int code) {
    static char other[16];
    static const char *const names[0x59] = {
        NULL, "Esc", "1", "2", "3", "4", "5", "6", "7", "8", "9", "0", "-", "=", "Backspace", "Tab", "Q", "W", "E", "R",
        "T", "Y", "U", "I", "O", "P", "[", "]", "Enter", "Ctrl", "A", "S", "D", "F", "G", "H", "J", "K", "L", ";", "'",
        "`", "Shift", "\\", "Z", "X", "C", "V", "B", "N", "M", ",", ".", "/", "right Shift", "keypad *", "Alt", "Space",
        "Caps Lock", "F1", "F2", "F3", "F4", "F5", "F6", "F7", "F8", "F9", "F10", "Num Lock", "Scroll Lock", "keypad 7",
        "keypad 8", "keypad 9", "keypad -", "keypad 4", "keypad 5", "keypad 6", "keypad +", "keypad 1", "keypad 2",
        "keypad 3", "keypad 0", "keypad .", NULL, NULL, NULL, "F11", "F12"};

    if (code < 0x59 && names[code] != NULL) {
        return names[code];
    }
    switch (code) {
    case 0x9c:
        return "keypad Enter";
    case 0x9d:
        return "right Ctrl";
    case 0xb5:
        return "keypad /";
    case 0xb8:
        return "right Alt";
    case 0xc7:
        return "Home";
    case 0xc8:
        return "Up";
    case 0xc9:
        return "Page Up";
    case 0xcb:
        return "Left";
    case 0xcd:
        return "Right";
    case 0xcf:
        return "End";
    case 0xd0:
        return "Down";
    case 0xd1:
        return "Page Down";
    case 0xd2:
        return "Insert";
    case 0xd3:
        return "Delete";
    }
    snprintf(other, sizeof(other), "0x%02x", (unsigned int)code);
    return other;
}

/* ---- the per-frame watch ---- */

static int watching;
static unsigned int last_buttons;
static int down_x[3];
static int down_y[3];
static unsigned char last_keys[256];
static unsigned int last_screen;
static unsigned int last_mode;
static const void *last_edit;
static unsigned int last_theme;
static unsigned int last_tab;
static unsigned int last_paused;
static unsigned int last_level;
static unsigned int last_level_end;

static void Where(char *out, size_t size, int x, int y) {
    if (EditMode.unk4 == 2 && Hover.type != 2) {
        snprintf(out, size, "%d,%d (tile %u,%u)", x, y, MouseTileX, MouseTileY);
    } else {
        snprintf(out, size, "%d,%d", x, y);
    }
}

static void WatchMouse(void) {
    static const char *const names[3] = {"left", "right", "middle"};
    struct CtrlBuffer *c = CONTROLLERBUFFER;
    unsigned int buttons;
    char where[64];
    int i;

    if (c == NULL) {
        return;
    }
    buttons = c->buttons & 7;
    for (i = 0; i < 3; i++) {
        unsigned int bit = 1u << i;
        if ((buttons & bit) != 0 && (last_buttons & bit) == 0) {
            const void *what = Hover.type == 2 && FocussedIconPtr != NULL ? FocussedIconPtr : (const void *)Hover.ptr;
            down_x[i] = c->x;
            down_y[i] = c->y;
            Where(where, sizeof(where), c->x, c->y);
            ExtAction("click %s at %s on %s (money %d)", names[i], where, ExtActionHover(Hover.type, what),
                UnlimitedBricks != 0 ? -1 : (int)BrickCount);
        } else if ((buttons & bit) == 0 && (last_buttons & bit) != 0) {
            int dx = c->x - down_x[i];
            int dy = c->y - down_y[i];
            if (dx * dx + dy * dy > 16) {
                Where(where, sizeof(where), c->x, c->y);
                ExtAction("release %s at %s (dragged from %d,%d) on %s", names[i], where, down_x[i], down_y[i],
                    ExtActionHover(Hover.type, Hover.type == 2 && FocussedIconPtr != NULL ? FocussedIconPtr : (const void *)Hover.ptr));
            }
        }
    }
    last_buttons = buttons;
}

static void WatchKeys(void) {
    int i;

    for (i = 1; i < 256; i++) {
        unsigned char down = KeyboardState[i] & 0x80;
        if (down != 0 && last_keys[i] == 0) {
            ExtAction("key %s", KeyName(i));
        }
        last_keys[i] = down;
    }
}

static void WatchState(int first) {
    char mode[160];
    unsigned int level = lpConfig != NULL ? lpConfig->level : 0;

    if (first || EditMode.unk4 != last_screen) {
        ExtAction("screen: %s", ScreenName(EditMode.unk4));
        last_screen = EditMode.unk4;
    }
    if (first || EditMode.unk0 != last_mode || (const void *)EditMode.unk8 != last_edit) {
        DescribeMode(mode, sizeof(mode));
        ExtAction("mode: %s", mode);
        last_mode = EditMode.unk0;
        last_edit = EditMode.unk8;
    }
    if (first || DAT_004baff8 != last_theme) {
        ExtAction("build menu: %s", ThemeName(DAT_004baff8));
        last_theme = DAT_004baff8;
    }
    if (first || DAT_00668e34 != last_tab) {
        ExtAction("build menu tab: %s", DAT_00668e34 != 0 ? "research" : "build");
        last_tab = DAT_00668e34;
    }
    if (first || (GameTimerPaused != 0) != (last_paused != 0)) {
        ExtAction("game timer %s", GameTimerPaused != 0 ? "paused" : "running");
        last_paused = GameTimerPaused;
    }
    if (first || level != last_level) {
        if (level >= 1 && level <= 5) {
            ExtAction("level: tutorial %u", level);
        } else if (level >= 6 && level <= 15) {
            ExtAction("level: game level %u", level - 5);
        } else {
            ExtAction("level: %u", level);
        }
        last_level = level;
    }
    if (first || MapStats.level_end != last_level_end) {
        if (!first || MapStats.level_end != 0) {
            ExtAction("level end state: %u", MapStats.level_end);
        }
        last_level_end = MapStats.level_end;
    }
}

void ExtActionsPoll(void) {
    if (!ExtActionLog) {
        return;
    }
    if (!watching) {
        watching = 1;
        memcpy(last_keys, KeyboardState, sizeof(last_keys));
        last_buttons = CONTROLLERBUFFER != NULL ? CONTROLLERBUFFER->buttons & 7 : 0;
        WatchState(1);
        return;
    }
    WatchMouse();
    WatchKeys();
    WatchState(0);
}
