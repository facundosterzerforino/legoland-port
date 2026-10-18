/* Touch screens in miniwin (miniwin/src/window.c): what the game reads, frame by frame, for a finger's tap, a long
 * press, two fingers, a mouse, typed characters (the web page's text box) and the on-screen keyboard while the game
 * waits for a name. Off Windows, with SDL's offscreen video (no screen needed):
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

static void Push(Uint32 type, SDL_MouseID which, Uint8 button) {
    SDL_Event e;

    memset(&e, 0, sizeof(e));
    e.type = type;
    if (type == SDL_EVENT_MOUSE_MOTION) {
        e.motion.which = which;
    } else if (type == SDL_EVENT_FINGER_DOWN || type == SDL_EVENT_FINGER_UP) {
        e.tfinger.fingerID = button;
    } else {
        e.button.which = which;
        e.button.button = button;
        e.button.down = type == SDL_EVENT_MOUSE_BUTTON_DOWN;
    }
    SDL_PushEvent(&e);
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

/* the primary button in each of n frames, as "0110..." */
static void Frames(const char *what, int n, const char *want) {
    char got[32];
    int i;

    for (i = 0; i < n; i++) {
        got[i] = (char)('0' + (Frame() & 3));
    }
    got[n] = 0;
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

int main(void) {
    HWND hwnd = MiniwinCreateWindow("test", 640, 480, MINIWIN_WINDOW);

    if (hwnd == NULL) {
        printf("no window: %s\n", MiniwinError());
        return 1;
    }
    Frames("nothing", 3, "000");

    /* a tap shorter than a frame: the pointer first, then a click two reads long */
    Push(SDL_EVENT_FINGER_DOWN, 0, 1);
    Push(SDL_EVENT_MOUSE_BUTTON_DOWN, SDL_TOUCH_MOUSEID, SDL_BUTTON_LEFT);
    Push(SDL_EVENT_MOUSE_BUTTON_UP, SDL_TOUCH_MOUSEID, SDL_BUTTON_LEFT);
    Push(SDL_EVENT_FINGER_UP, 0, 1);
    Frames("tap within one frame", 5, "01100");
    Expect("touch mode after a tap", MiniwinTouchMode() ? "on" : "off", "on");

    /* a long press: down while the finger stays */
    Push(SDL_EVENT_FINGER_DOWN, 0, 1);
    Push(SDL_EVENT_MOUSE_BUTTON_DOWN, SDL_TOUCH_MOUSEID, SDL_BUTTON_LEFT);
    Frames("long press, held", 6, "011111");
    Push(SDL_EVENT_MOUSE_BUTTON_UP, SDL_TOUCH_MOUSEID, SDL_BUTTON_LEFT);
    Push(SDL_EVENT_FINGER_UP, 0, 1);
    Frames("long press, lifted", 2, "00");

    /* two fingers: the secondary button only, and not the first finger's press after the second is lifted */
    Push(SDL_EVENT_FINGER_DOWN, 0, 1);
    Push(SDL_EVENT_MOUSE_BUTTON_DOWN, SDL_TOUCH_MOUSEID, SDL_BUTTON_LEFT);
    Push(SDL_EVENT_FINGER_DOWN, 0, 2);
    Frames("two fingers", 3, "222");
    Push(SDL_EVENT_FINGER_UP, 0, 2);
    Frames("second finger lifted", 2, "00");
    Push(SDL_EVENT_MOUSE_BUTTON_UP, SDL_TOUCH_MOUSEID, SDL_BUTTON_LEFT);
    Push(SDL_EVENT_FINGER_UP, 0, 1);
    Frames("all lifted", 2, "00");

    /* a mouse: the game's pointer is back */
    Push(SDL_EVENT_MOUSE_MOTION, 1, 0);
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
    Push(SDL_EVENT_FINGER_DOWN, 0, 1);
    Push(SDL_EVENT_FINGER_UP, 0, 1);
    MiniwinWantText();
    MiniwinPumpEvents(Proc);
    Expect("keyboard while a name is wanted", SDL_TextInputActive((SDL_Window *)hwnd) ? "shown" : "hidden", "shown");
    SDL_Delay(600);
    MiniwinPumpEvents(Proc);
    Expect("keyboard after the name", SDL_TextInputActive((SDL_Window *)hwnd) ? "shown" : "hidden", "hidden");
    Push(SDL_EVENT_MOUSE_MOTION, 1, 0);
    MiniwinWantText();
    MiniwinPumpEvents(Proc);
    Expect("keyboard with a mouse", SDL_TextInputActive((SDL_Window *)hwnd) ? "shown" : "hidden", "hidden");

    printf("%d checks, %d failed\n", checks, failed);
    return failed != 0;
}
