/* Touch screens in miniwin (miniwin/src/window.c): what the game reads, frame by frame, for a finger's tap, a long
 * press, a list dragged with a finger, two fingers (a tap, a hold, a pan), a mouse, typed characters (the web page's
 * text box) and the on-screen keyboard while the game waits for a name. Off Windows, with SDL's offscreen video (no
 * screen needed; a 640x480 window, so window pixels are the game's):
 *
 *     SDL_VIDEO_DRIVER=offscreen build-linux/tests/test_touch
 */
#include <SDL3/SDL.h>
#include <stdio.h>
#include <string.h>

#include "miniwin/miniwin.h"

static int failed;
static int checks;

static LRESULT CALLBACK Proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
    (void)hwnd;
    (void)msg;
    (void)wparam;
    (void)lparam;
    return 0;
}

/* a mouse event: from a finger (SDL_TOUCH_MOUSEID) or a mouse (1), at (x, y) in the window */
static void Mouse(Uint32 type, SDL_MouseID which, float x, float y) {
    SDL_Event e;

    memset(&e, 0, sizeof(e));
    e.type = type;
    if (type == SDL_EVENT_MOUSE_MOTION) {
        e.motion.which = which;
        e.motion.x = x;
        e.motion.y = y;
    } else {
        e.button.which = which;
        e.button.button = SDL_BUTTON_LEFT;
        e.button.down = type == SDL_EVENT_MOUSE_BUTTON_DOWN;
        e.button.x = x;
        e.button.y = y;
    }
    SDL_PushEvent(&e);
}

/* a finger event: finger id at (x, y), from 0 to 1 across the window */
static void Finger(Uint32 type, int id, float x, float y) {
    SDL_Event e;

    memset(&e, 0, sizeof(e));
    e.type = type;
    e.tfinger.fingerID = (SDL_FingerID)id;
    e.tfinger.x = x;
    e.tfinger.y = y;
    SDL_PushEvent(&e);
}

/* a first finger: SDL's mouse events made from it, then the finger's (as SDL sends them) */
static void FirstDown(float x, float y) {
    Mouse(SDL_EVENT_MOUSE_BUTTON_DOWN, SDL_TOUCH_MOUSEID, x * 640, y * 480);
    Finger(SDL_EVENT_FINGER_DOWN, 1, x, y);
}

static void FirstUp(float x, float y) {
    Mouse(SDL_EVENT_MOUSE_BUTTON_UP, SDL_TOUCH_MOUSEID, x * 640, y * 480);
    Finger(SDL_EVENT_FINGER_UP, 1, x, y);
}

static void FirstMove(float x, float y) {
    Mouse(SDL_EVENT_MOUSE_MOTION, SDL_TOUCH_MOUSEID, x * 640, y * 480);
    Finger(SDL_EVENT_FINGER_MOTION, 1, x, y);
}

/* one frame: the game's message loop, then its read of the mouse's buttons */
static unsigned int Frame(void) {
    MiniwinPumpEvents(Proc);
    return MiniwinMouseButtons();
}

static void Expect(const char *what, const char *got, const char *want) {
    checks++;
    if (strcmp(got, want) != 0) {
        failed++;
        printf("FAILED %s: %s, expected %s\n", what, got, want);
    } else {
        printf("ok     %s: %s\n", what, got);
    }
}

/* the buttons in each of n frames, as "0110..." (1 primary, 2 secondary) */
static void Frames(const char *what, int n, const char *want) {
    char got[32];
    int i;

    for (i = 0; i < n; i++) {
        got[i] = (char)('0' + (Frame() & 3));
    }
    got[n] = 0;
    Expect(what, got, want);
}

/* the drag and pan taken since the last call, "dx,dy pan x,y" */
static void Drags(const char *what, const char *want) {
    char got[64];
    float dx;
    float dy;
    float px;
    float py;

    MiniwinTouchTakeDrag(&dx, &dy, &px, &py);
    snprintf(got, sizeof(got), "%.0f,%.0f pan %.0f,%.0f", dx, dy, px, py);
    Expect(what, got, want);
}

/* the keyboard reads that show a typed key: "a", "A" (Shift), "." (none), "<" Backspace, "|" Enter */
static void Typed(const char *what, int n, const char *want) {
    char got[64];
    int i;

    for (i = 0; i < n; i++) {
        int shift;
        int code = MiniwinTypedKey(&shift);

        if (code == 0) {
            got[i] = '.';
        } else if (code == SDL_SCANCODE_BACKSPACE) {
            got[i] = '<';
        } else if (code == SDL_SCANCODE_RETURN) {
            got[i] = '|';
        } else if (code == SDL_SCANCODE_SPACE) {
            got[i] = '_';
        } else {
            SDL_Keycode key = SDL_GetKeyFromScancode((SDL_Scancode)code, SDL_KMOD_NONE, false);
            got[i] = (char)(shift ? key - 'a' + 'A' : key);
        }
    }
    got[n] = 0;
    Expect(what, got, want);
}

/* the left half of the window is a list (a drag area) */
static int LeftHalf(int x, int y) {
    (void)y;
    return x < 320;
}

int main(void) {
    HWND hwnd = MiniwinCreateWindow("test", 640, 480, MINIWIN_WINDOW);

    if (hwnd == NULL) {
        printf("no window: %s\n", MiniwinError());
        return 1;
    }
    Frames("nothing", 3, "000");

    /* a tap shorter than a frame: the pointer first, then a click two reads long */
    FirstDown(0.75f, 0.5f);
    FirstUp(0.75f, 0.5f);
    Frames("tap within one frame", 5, "01100");
    Expect("touch mode after a tap", MiniwinTouchMode() ? "on" : "off", "on");

    /* a long press: down while the finger stays */
    FirstDown(0.75f, 0.5f);
    Frames("long press, held", 6, "011111");
    FirstUp(0.75f, 0.5f);
    Frames("long press, lifted", 2, "00");

    /* lists (the left half): a tap is a click, a drag scrolls without a click, a still press presses */
    MiniwinSetTouchDragArea(LeftHalf);
    FirstDown(0.25f, 0.5f);
    FirstUp(0.25f, 0.5f);
    Frames("tap on a list", 5, "01100");
    FirstDown(0.25f, 0.3f);
    Frames("finger down on a list", 2, "00");
    FirstMove(0.25f, 0.4f); /* 48 pixels: past the slop (9.6), the drag starts here */
    FirstMove(0.25f, 0.5f);
    FirstMove(0.25f, 0.45f);
    Frames("dragging a list", 3, "000");
    Drags("the list's drag", "0,24 pan 0,0");
    FirstUp(0.25f, 0.45f);
    Frames("list drag lifted", 3, "000");
    FirstDown(0.25f, 0.5f);
    Frames("still finger on a list, at first", 1, "0");
    SDL_Delay(450);
    Frames("still finger on a list, held", 3, "111");
    FirstUp(0.25f, 0.5f);
    Frames("lifted", 2, "00");
    FirstDown(0.75f, 0.3f);
    FirstMove(0.75f, 0.5f);
    Frames("a drag elsewhere is a held press", 3, "011");
    Drags("no list drag elsewhere", "0,0 pan 0,0");
    FirstUp(0.75f, 0.5f);
    Frames("lifted", 2, "00");
    MiniwinSetTouchDragArea(NULL);

    /* two fingers: a tap is a secondary click, a still hold keeps it down, a move pans; never the primary */
    FirstDown(0.5f, 0.5f);
    Finger(SDL_EVENT_FINGER_DOWN, 2, 0.6f, 0.5f);
    Frames("two fingers down", 2, "00");
    Finger(SDL_EVENT_FINGER_UP, 2, 0.6f, 0.5f);
    Frames("two-finger tap", 3, "220");
    FirstUp(0.5f, 0.5f);
    Frames("all lifted", 2, "00");

    FirstDown(0.5f, 0.5f);
    Finger(SDL_EVENT_FINGER_DOWN, 2, 0.6f, 0.5f);
    Frames("two fingers down", 1, "0");
    SDL_Delay(450);
    Frames("two fingers held still", 3, "222");
    Finger(SDL_EVENT_FINGER_UP, 2, 0.6f, 0.5f);
    FirstUp(0.5f, 0.5f);
    Frames("all lifted", 2, "00");

    FirstDown(0.5f, 0.5f);
    Finger(SDL_EVENT_FINGER_DOWN, 2, 0.6f, 0.5f);
    FirstMove(0.5f, 0.7f);
    Finger(SDL_EVENT_FINGER_MOTION, 2, 0.6f, 0.7f);
    FirstMove(0.4f, 0.7f);
    Finger(SDL_EVENT_FINGER_MOTION, 2, 0.5f, 0.7f);
    Frames("two fingers moving", 3, "000");
    Drags("the pan (centre moved 64 left, 96 down)", "0,0 pan -64,96");
    Finger(SDL_EVENT_FINGER_UP, 2, 0.5f, 0.7f);
    Frames("pan: one finger lifted, no click", 3, "000");
    FirstUp(0.4f, 0.7f);
    Frames("all lifted", 2, "00");
    Expect("fingers after all lifted", MiniwinTouchFingers() == 0 ? "0" : "not 0", "0");

    /* a mouse: the game's pointer is back */
    Mouse(SDL_EVENT_MOUSE_MOTION, 1, 10, 10);
    Frames("mouse moves", 1, "0");
    Expect("touch mode after a mouse", MiniwinTouchMode() ? "on" : "off", "off");

    /* typed characters: each key down for one read, up for the next */
    MiniwinTypeChar('L');
    MiniwinTypeChar('l');
    MiniwinTypeChar('l');
    MiniwinTypeChar(' ');
    MiniwinTypeChar('7');
    MiniwinTypeChar(8);
    MiniwinTypeChar(13);
    MiniwinTypeChar(0xe9); /* not ASCII: dropped */
    Typed("typed \"Lll 7\", Backspace, Enter, an e-acute", 16, "L.l.l._.7.<.|...");

    /* the on-screen keyboard: opens while the game waits for a name (on a touch screen), closes after */
    Finger(SDL_EVENT_FINGER_DOWN, 1, 0.5f, 0.5f);
    Finger(SDL_EVENT_FINGER_UP, 1, 0.5f, 0.5f);
    MiniwinWantText();
    MiniwinPumpEvents(Proc);
    Expect("keyboard while a name is wanted", SDL_TextInputActive((SDL_Window *)hwnd) ? "shown" : "hidden", "shown");
    SDL_Delay(600);
    MiniwinPumpEvents(Proc);
    Expect("keyboard after the name", SDL_TextInputActive((SDL_Window *)hwnd) ? "shown" : "hidden", "hidden");
    Mouse(SDL_EVENT_MOUSE_MOTION, 1, 10, 10);
    MiniwinWantText();
    MiniwinPumpEvents(Proc);
    Expect("keyboard with a mouse", SDL_TextInputActive((SDL_Window *)hwnd) ? "shown" : "hidden", "hidden");

    printf("%d checks, %d failed\n", checks, failed);
    return failed != 0;
}
