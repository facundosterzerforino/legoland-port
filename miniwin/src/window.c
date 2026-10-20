/* [library:window] miniwin's window: the game's window, renderer and events on SDL3. See miniwin/miniwin.h. */
#include <SDL3/SDL.h>

#include "miniwin/miniwin.h"
#include "miniwin/timer.h"

#ifdef LEGOLAND_WEB_BROWSER
#include <emscripten.h>
#endif

static SDL_Window *window;
static SDL_Renderer *renderer;
static SDL_Texture *texture;
static int texture_width;
static int texture_height;
static HWND window_hwnd;
static int fingers; /* touching the screen now */
static bool latched[SDL_SCANCODE_COUNT]; /* pressed since the last keyboard read (MiniwinKeyLatched) */

/* [library:input] touch screens (MiniwinTouchMode, MiniwinMouseButtons): the last pointer input was a finger or a pen;
 * a finger's press the game hasn't finished seeing (1 down, 2 lifted) and the button reads since it began; a
 * two-finger touch keeps the primary button up until every finger is lifted. */
static int touch_mode;
static int touch_press;
static int touch_reads;
static int touch_secondary;

/* [library:input] one finger in a drag area (MiniwinSetTouchDragArea: a list): DRAG_WAIT while it may still be a tap
 * or a press (the press is held back), DRAG_MOVING once it moved (no press at all); where it started and last was,
 * in window pixels, and the movement not taken yet (MiniwinTouchTakeDrag) */
enum {
    DRAG_NONE,
    DRAG_WAIT,
    DRAG_MOVING
};
static int (*drag_area)(int x, int y);
static int drag_state;
static float drag_start_x;
static float drag_start_y;
static float drag_last_x;
static float drag_last_y;
static Uint64 drag_start_tick;
static float drag_dx;
static float drag_dy;

/* [library:input] placing an object (MiniwinSetTouchAimArea): one finger aims and a tap on the aim builds. A finger
 * that goes down near the aim moves it by as much as the finger moves, one that goes down elsewhere brings it there;
 * either way no press. Lifted without moving (AIM_WAIT) on the aim, it clicks there. While aim_held the game's pointer
 * is held at aim_x, aim_y (window pixels), not where SDL's touch pointer last was; aim_click: the tap's click is under
 * way, and the hold ends with it. */
enum {
    AIM_NONE,
    AIM_WAIT,
    AIM_MOVING
};
static int (*aim_area)(int x, int y);
static int aim_state;
static int aim_near;
static int aim_held;
static int aim_click;
static float aim_x;
static float aim_y;
static float aim_from_x;
static float aim_from_y;
static float aim_start_x;
static float aim_start_y;

/* [library:input] two fingers: TWO_WAIT until they move (a pan) or stay (the secondary button: a tap clicks it, a
 * still hold keeps it down); the tracked fingers (window pixels), their last centre, the pan not taken yet, and a
 * two-finger tap's click still to give (reads) */
enum {
    TWO_NONE,
    TWO_WAIT,
    TWO_PAN,
    TWO_HOLD,
    TWO_DONE
};
static int two_state;
static SDL_FingerID finger_id[2];
static float finger_x[2];
static float finger_y[2];
static int finger_count;
static float two_start_x;
static float two_start_y;
static float two_last_x;
static float two_last_y;
static Uint64 two_start_tick;
static float pan_dx;
static float pan_dy;
static int secondary_click_reads;

#define TOUCH_HOLD_MS 400 /* a still finger (or two) this long presses instead of dragging */
/* [library:input] a tap lifted this long before the game first read the mouse is dropped (MiniwinMouseButtons) */
#define TOUCH_STALE_NS (300 * SDL_NS_PER_MS)
static Uint64 touch_up_ns;

/* [library:input] presses so far: buttons, fingers and keys (MiniwinPressCount) */
static unsigned int press_count;

/* [library:input] the on-screen keyboard (MiniwinWantText): shown until this tick, because the game waits for a
 * name; shown now (by the game's wait, or three fingers) */
static Uint64 text_wanted_until;
static int keyboard_auto;
static int keyboard_shown;

/* [library:input] typed characters as key taps (MiniwinTypeChar, MiniwinTypedKey): SDL scancode, plus TYPED_SHIFT
 * with Shift held; the first is down for one keyboard read (typed_down), then up for one */
#define TYPED_MAX 64
#define TYPED_SHIFT 0x10000
static int typed[TYPED_MAX];
static int typed_head;
static int typed_count;
static int typed_down;

const char *MiniwinError(void) {
    return SDL_GetError();
}

const char *MiniwinRendererName(void) {
    const char *name = renderer != NULL ? SDL_GetRendererName(renderer) : NULL;
    return name != NULL ? name : "none";
}

HWND MiniwinCreateWindow(const char *title, int width, int height, int fullscreen) {
    SDL_PropertiesID props;

    /* the exe's own icon (resource 0x65, as the game's window class used) */
    SDL_SetHint(SDL_HINT_WINDOWS_INTRESOURCE_ICON, "101");
    SDL_SetHint(SDL_HINT_WINDOWS_INTRESOURCE_ICON_SMALL, "101");
    if (!SDL_InitSubSystem(SDL_INIT_VIDEO)) {
        return NULL;
    }
#ifdef LEGOLAND_WEB_BROWSER
    /* [library:window] the page's canvas is the window: resizable, sized by the page's CSS (the whole page), and the
     * page itself goes full screen (its button). An SDL full-screen window kept the screen's size after the browser
     * left full screen (Esc), taller than the page: the picture's top was cut off. */
    fullscreen = MINIWIN_WINDOW;
#endif
    props = SDL_CreateProperties();
    SDL_SetStringProperty(props, SDL_PROP_WINDOW_CREATE_TITLE_STRING, title);
    SDL_SetNumberProperty(props, SDL_PROP_WINDOW_CREATE_WIDTH_NUMBER, width);
    SDL_SetNumberProperty(props, SDL_PROP_WINDOW_CREATE_HEIGHT_NUMBER, height);
    SDL_SetNumberProperty(props, SDL_PROP_WINDOW_CREATE_X_NUMBER, SDL_WINDOWPOS_CENTERED);
    SDL_SetNumberProperty(props, SDL_PROP_WINDOW_CREATE_Y_NUMBER, SDL_WINDOWPOS_CENTERED);
    SDL_SetBooleanProperty(props, SDL_PROP_WINDOW_CREATE_RESIZABLE_BOOLEAN, fullscreen == MINIWIN_WINDOW);
    SDL_SetBooleanProperty(props, SDL_PROP_WINDOW_CREATE_FULLSCREEN_BOOLEAN, fullscreen != MINIWIN_WINDOW);
    window = SDL_CreateWindowWithProperties(props);
    SDL_DestroyProperties(props);
    if (window == NULL) {
        return NULL;
    }
    if (fullscreen == MINIWIN_FULLSCREEN_MODE) {
        SDL_DisplayMode mode;
        if (SDL_GetClosestFullscreenDisplayMode(SDL_GetDisplayForWindow(window), width, height, 0.0f, false, &mode)) {
            SDL_SetWindowFullscreenMode(window, &mode);
        }
    }
    renderer = SDL_CreateRenderer(window, NULL);
    if (renderer == NULL) {
        SDL_DestroyWindow(window);
        window = NULL;
        return NULL;
    }
    SDL_HideCursor(); /* the game draws its own pointer */
#if defined(__ANDROID__)
    touch_mode = 1; /* [library:input] a touch screen: the game's pointer hidden until a mouse moves */
#elif defined(LEGOLAND_WEB_BROWSER)
    /* [library:input] a phone's or tablet's browser (its main pointer is a finger): the same */
    touch_mode = EM_ASM_INT({ return window.matchMedia && window.matchMedia('(pointer: coarse)').matches ? 1 : 0; });
#endif
#ifdef _WIN32
    window_hwnd = (HWND)SDL_GetPointerProperty(SDL_GetWindowProperties(window), SDL_PROP_WINDOW_WIN32_HWND_POINTER, NULL);
#else
    /* no native window the code could use: the SDL window stands for it (MiniwinWindowOf) */
    window_hwnd = (HWND)window;
#endif
    return window_hwnd;
}

/* [library:input] a mouse event SDL made from a finger or a pen */
static int FromTouch(SDL_MouseID which) {
    return which == SDL_TOUCH_MOUSEID || which == SDL_PEN_MOUSEID;
}

/* [library:input] how far a finger moves before it's a drag: a fiftieth of the window's height (about 10 of the
 * game's pixels) */
static float TouchSlop(void) {
    int w = 0;
    int h = 0;

    if (window != NULL) {
        SDL_GetWindowSizeInPixels(window, &w, &h);
    }
    return h > 0 ? (float)h / 50.0f : 10.0f;
}

static float Distance(float dx, float dy) {
    return SDL_sqrtf(dx * dx + dy * dy);
}

/* [library:input] how near the aim a tap builds: six slops, about 58 of the game's pixels */
static float AimRadius(void) {
    return TouchSlop() * 6.0f;
}

/* [library:input] a finger's place in window pixels (SDL gives it from 0 to 1 across the window) */
static void FingerPixels(const SDL_TouchFingerEvent *f, float *x, float *y) {
    int w = 0;
    int h = 0;

    if (window != NULL) {
        SDL_GetWindowSizeInPixels(window, &w, &h);
    }
    *x = f->x * (float)w;
    *y = f->y * (float)h;
}

/* the two tracked fingers' centre */
static void TwoCentre(float *x, float *y) {
    *x = (finger_x[0] + finger_x[1]) / 2.0f;
    *y = (finger_y[0] + finger_y[1]) / 2.0f;
}

static void TrackFinger(const SDL_TouchFingerEvent *f, int down) {
    int i;
    float x;
    float y;

    FingerPixels(f, &x, &y);
    for (i = 0; i < finger_count; i++) {
        if (finger_id[i] == f->fingerID) {
            if (down < 0) {
                /* lifted: the other one moves to the first place */
                finger_count--;
                if (i == 0 && finger_count == 1) {
                    finger_id[0] = finger_id[1];
                    finger_x[0] = finger_x[1];
                    finger_y[0] = finger_y[1];
                }
                return;
            }
            finger_x[i] = x;
            finger_y[i] = y;
            return;
        }
    }
    if (down > 0 && finger_count < 2) {
        finger_id[finger_count] = f->fingerID;
        finger_x[finger_count] = x;
        finger_y[finger_count] = y;
        finger_count++;
    }
}

void MiniwinSetTouchDragArea(int (*area)(int x, int y)) {
    drag_area = area;
}

void MiniwinSetTouchAimArea(int (*area)(int x, int y)) {
    aim_area = area;
}

int MiniwinTouchAiming(void) {
    return aim_held;
}

void MiniwinTouchTakeDrag(float *dx, float *dy, float *pan_x, float *pan_y) {
    *dx = drag_dx;
    *dy = drag_dy;
    *pan_x = pan_dx;
    *pan_y = pan_dy;
    drag_dx = 0.0f;
    drag_dy = 0.0f;
    pan_dx = 0.0f;
    pan_dy = 0.0f;
}

int MiniwinTouchFingers(void) {
    return fingers;
}

/* [library:input] the on-screen keyboard: Android's (SDL), or the page's text box on the web (port/web/shell.html,
 * Module.legolandKeyboard), whose characters come back through MiniwinTypeChar */
static void ShowKeyboard(int show) {
    keyboard_shown = show;
#ifdef LEGOLAND_WEB_BROWSER
    EM_ASM({ if (Module.legolandKeyboard) Module.legolandKeyboard($0); }, show);
#else
    if (window == NULL) {
        return;
    }
    if (show) {
        SDL_StartTextInput(window);
    } else {
        SDL_StopTextInput(window);
    }
#endif
}

void MiniwinWantText(void) {
    text_wanted_until = SDL_GetTicks() + 500;
}

/* The game's window procedure only reacts to a few messages; these are the SDL events that stand for them. */
void MiniwinPumpEvents(WNDPROC proc) {
    SDL_Event event;
    int wanted;

#ifdef LEGOLAND_WEB_BROWSER
    /* [library:window] the game's loop runs on the page's thread: give the page its turn (drawing the frame, the
     * input, the sound), every frame (ASYNCIFY) */
    MiniwinWebSleep(0);
#endif
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
        case SDL_EVENT_FINGER_DOWN:
            press_count++;
            fingers++;
            touch_mode = 1;
            TrackFinger(&event.tfinger, 1);
            if (fingers == 2) {
                /* [library:input] two fingers: a pan or the secondary button (MiniwinMouseButtons), never the first
                 * finger's press or drag */
                touch_secondary = 1;
                touch_press = 0;
                drag_state = DRAG_NONE;
                aim_state = AIM_NONE; /* (the aim stays where it is) */
                two_state = TWO_WAIT;
                two_start_tick = SDL_GetTicks();
                TwoCentre(&two_start_x, &two_start_y);
                two_last_x = two_start_x;
                two_last_y = two_start_y;
            }
            if (fingers == 3) {
                /* [library:input] three fingers: the on-screen keyboard, for typing names */
                two_state = TWO_DONE;
                ShowKeyboard(!keyboard_shown);
                keyboard_auto = 0;
            }
            break;
        case SDL_EVENT_FINGER_MOTION:
            TrackFinger(&event.tfinger, 0);
            if (fingers == 2 && finger_count == 2 && (two_state == TWO_WAIT || two_state == TWO_PAN)) {
                float x;
                float y;

                TwoCentre(&x, &y);
                if (two_state == TWO_WAIT && Distance(x - two_start_x, y - two_start_y) > TouchSlop()) {
                    two_state = TWO_PAN; /* [library:input] two fingers moving: the park follows them */
                }
                if (two_state == TWO_PAN) {
                    pan_dx += x - two_last_x;
                    pan_dy += y - two_last_y;
                }
                two_last_x = x;
                two_last_y = y;
            }
            break;
        case SDL_EVENT_FINGER_UP:
        case SDL_EVENT_FINGER_CANCELED:
            fingers = fingers > 0 ? fingers - 1 : 0;
            TrackFinger(&event.tfinger, -1);
            if (two_state == TWO_WAIT) {
                /* [library:input] two fingers lifted without moving: a click of the secondary button */
                if (event.type == SDL_EVENT_FINGER_UP) {
                    secondary_click_reads = 2;
                }
                two_state = TWO_DONE;
            } else if (two_state == TWO_PAN || two_state == TWO_HOLD) {
                two_state = TWO_DONE;
            }
            if (fingers == 0) {
                touch_secondary = 0;
                two_state = TWO_NONE;
                finger_count = 0;
            }
            break;
        case SDL_EVENT_MOUSE_MOTION:
            if (!FromTouch(event.motion.which)) {
                touch_mode = 0; /* a mouse: the game's pointer again */
                aim_held = 0;
                aim_state = AIM_NONE;
            } else if (aim_state != AIM_NONE && touch_press == 1) {
                float density = window != NULL ? SDL_GetWindowPixelDensity(window) : 1.0f;
                float x = event.motion.x * density;
                float y = event.motion.y * density;

                if (aim_state == AIM_WAIT && Distance(x - aim_start_x, y - aim_start_y) > TouchSlop()) {
                    aim_state = AIM_MOVING; /* [library:input] the finger moves the aim: no click */
                }
                if (aim_state == AIM_MOVING) {
                    aim_x = aim_from_x + (x - aim_start_x);
                    aim_y = aim_from_y + (y - aim_start_y);
                }
            } else if (drag_state != DRAG_NONE && touch_press == 1) {
                float density = window != NULL ? SDL_GetWindowPixelDensity(window) : 1.0f;
                float x = event.motion.x * density;
                float y = event.motion.y * density;

                if (drag_state == DRAG_WAIT && Distance(x - drag_start_x, y - drag_start_y) > TouchSlop()) {
                    drag_state = DRAG_MOVING; /* [library:input] a finger dragging a list: it scrolls, no click */
                    drag_last_x = x;
                    drag_last_y = y;
                }
                if (drag_state == DRAG_MOVING) {
                    drag_dx += x - drag_last_x;
                    drag_dy += y - drag_last_y;
                    drag_last_x = x;
                    drag_last_y = y;
                }
            }
            break;
        case SDL_EVENT_MOUSE_BUTTON_DOWN:
            press_count++;
            if (FromTouch(event.button.which) && event.button.button == SDL_BUTTON_LEFT && !touch_secondary) {
                float density = window != NULL ? SDL_GetWindowPixelDensity(window) : 1.0f;

                touch_mode = 1;
                touch_press = 1;
                touch_reads = 0;
                drag_start_x = event.button.x * density;
                drag_start_y = event.button.y * density;
                drag_start_tick = SDL_GetTicks();
                drag_state = drag_area != NULL && drag_area((int)drag_start_x, (int)drag_start_y) ? DRAG_WAIT : DRAG_NONE;
                aim_state = AIM_NONE;
                aim_click = 0;
                if (drag_state == DRAG_NONE && aim_area != NULL && aim_area((int)drag_start_x, (int)drag_start_y)) {
                    /* [library:input] placing an object: the finger aims (see aim_state) */
                    aim_near = aim_held && Distance(drag_start_x - aim_x, drag_start_y - aim_y) <= AimRadius();
                    aim_from_x = aim_near ? aim_x : drag_start_x;
                    aim_from_y = aim_near ? aim_y : drag_start_y;
                    aim_start_x = drag_start_x;
                    aim_start_y = drag_start_y;
                    aim_x = aim_from_x;
                    aim_y = aim_from_y;
                    aim_held = 1;
                    aim_state = AIM_WAIT;
                } else {
                    aim_held = 0;
                }
            }
            break;
        case SDL_EVENT_MOUSE_BUTTON_UP:
            if (FromTouch(event.button.which) && event.button.button == SDL_BUTTON_LEFT && touch_press == 1) {
                touch_press = 2;
                touch_up_ns = event.button.timestamp != 0 ? event.button.timestamp : SDL_GetTicksNS();
            }
            break;
        case SDL_EVENT_WINDOW_FOCUS_LOST:
            proc(window_hwnd, WM_KILLFOCUS, 0, 0);
            break;
        case SDL_EVENT_WINDOW_FOCUS_GAINED:
            proc(window_hwnd, WM_SETFOCUS, 0, 0);
            break;
        case SDL_EVENT_MOUSE_WHEEL: {
            float y = event.wheel.direction == SDL_MOUSEWHEEL_FLIPPED ? -event.wheel.y : event.wheel.y;
            proc(window_hwnd, WM_MOUSEWHEEL, MAKEWPARAM(0, (WORD)(short)(y * WHEEL_DELTA)), 0);
            break;
        }
        case SDL_EVENT_KEY_DOWN:
            if (!event.key.repeat) {
                press_count++;
            }
            if (event.key.scancode < SDL_SCANCODE_COUNT) {
                latched[event.key.scancode] = true;
            }
            if (event.key.key == SDLK_BACKSPACE) {
                proc(window_hwnd, WM_CHAR, 8, 0);
            }
            break;
        case SDL_EVENT_WINDOW_CLOSE_REQUESTED:
            proc(window_hwnd, WM_CLOSE, 0, 0);
            break;
        default:
            break;
        }
    }
    /* [library:input] on a touch screen the keyboard opens while the game waits for a name, and closes after */
    wanted = touch_mode && SDL_GetTicks() < text_wanted_until;
    if (wanted && !keyboard_auto && !keyboard_shown) {
        ShowKeyboard(1);
        keyboard_auto = 1;
    } else if (!wanted && keyboard_auto) {
        if (keyboard_shown) {
            ShowKeyboard(0);
        }
        keyboard_auto = 0;
    }
}

void *MiniwinWindowOf(HWND hwnd) {
    return window != NULL && hwnd == window_hwnd ? (void *)window : NULL;
}

int MiniwinOutputSize(int *width, int *height) {
    return renderer != NULL && SDL_GetCurrentRenderOutputSize(renderer, width, height);
}

int MiniwinHasFocus(void) {
    return window != NULL && (SDL_GetWindowFlags(window) & SDL_WINDOW_INPUT_FOCUS) != 0;
}

int MiniwinWindowInactive(void) {
    return window != NULL && (SDL_GetWindowFlags(window) & SDL_WINDOW_INPUT_FOCUS) == 0;
}

unsigned int MiniwinMouseState(int *x, int *y) {
    float fx = 0.0f;
    float fy = 0.0f;
    SDL_MouseButtonFlags flags = SDL_GetMouseState(&fx, &fy);
    float density = window != NULL ? SDL_GetWindowPixelDensity(window) : 1.0f;
    unsigned int buttons = 0;

    if (aim_held) {
        /* [library:input] placing an object: the pointer stays on the aim (MiniwinSetTouchAimArea) */
        fx = aim_x / density;
        fy = aim_y / density;
    }
    if (x != NULL) {
        *x = (int)(fx * density);
    }
    if (y != NULL) {
        *y = (int)(fy * density);
    }
    if ((flags & SDL_BUTTON_LMASK) != 0) {
        buttons |= 1;
    }
    if ((flags & SDL_BUTTON_RMASK) != 0) {
        buttons |= 2;
    }
    if ((flags & SDL_BUTTON_MMASK) != 0) {
        buttons |= 4;
    }
    return buttons;
}

unsigned int MiniwinMouseButtons(void) {
    unsigned int buttons = MiniwinMouseState(NULL, NULL);

    if (secondary_click_reads > 0) {
        /* [library:input] a two-finger tap: the secondary button for two reads */
        secondary_click_reads--;
        buttons |= 2;
    }
    if (fingers >= 2 || touch_secondary) {
        /* [library:input] two fingers held still: the secondary button down while they stay; never the primary */
        if (two_state == TWO_WAIT && SDL_GetTicks() - two_start_tick >= TOUCH_HOLD_MS) {
            two_state = TWO_HOLD;
        }
        if (two_state == TWO_HOLD) {
            buttons |= 2;
        }
        return buttons & ~1u;
    }
    if (touch_press == 2 && touch_reads == 0 && SDL_GetTicksNS() - touch_up_ns > TOUCH_STALE_NS) {
        /* [library:input] a tap made while the game wasn't reading the mouse (loading): lost, as a mouse's click
         * then is. Kept, it clicked whatever came next: the first movie (skipped) or a button. */
        touch_press = 0;
        drag_state = DRAG_NONE;
        aim_state = AIM_NONE;
    }
    if (aim_held && aim_state == AIM_NONE && !aim_click &&
        (aim_area == NULL || !aim_area((int)aim_x, (int)aim_y))) {
        aim_held = 0; /* [library:input] not placing an object any more (or the aim left the park): let it go */
    }
    if (touch_press != 0 && aim_state != AIM_NONE) {
        if (touch_press == 1) {
            touch_reads++;
            return buttons & ~1u; /* [library:input] the finger aims: no press while it's down */
        }
        if (aim_state == AIM_WAIT && aim_near) {
            /* [library:input] a tap on the aim: a click there, two reads long (the pointer is there already) */
            aim_state = AIM_NONE;
            aim_click = 1;
            touch_reads = 1;
        } else {
            /* [library:input] the aim moved or was brought here: it stays, no click */
            aim_state = AIM_NONE;
            touch_press = 0;
            return buttons & ~1u;
        }
    }
    if (touch_press != 0 && drag_state == DRAG_WAIT) {
        if (touch_press == 1 && SDL_GetTicks() - drag_start_tick < TOUCH_HOLD_MS) {
            /* [library:input] in a list: a tap or a drag? The press waits (the pointer is there already) */
            touch_reads++;
            return buttons & ~1u;
        }
        drag_state = DRAG_NONE; /* lifted without moving (a click) or held still (a press): as below */
    }
    if (touch_press != 0 && drag_state == DRAG_MOVING) {
        if (touch_press == 2) {
            touch_press = 0;
            drag_state = DRAG_NONE;
        }
        return buttons & ~1u; /* the list was dragged: no click */
    }
    if (touch_press != 0) {
        /* [library:input] a finger: the pointer jumps to it in the first read and the press follows in the next (as
         * a mouse moves over a button before clicking it), down for at least two reads even for a tap shorter than
         * a frame, and as long as the finger stays */
        touch_reads++;
        buttons &= ~1u;
        if (touch_reads >= 2 && (touch_press == 1 || touch_reads <= 3)) {
            buttons |= 1;
        } else if (touch_reads > 3) {
            touch_press = 0;
        }
    }
    if (aim_click && touch_press == 0) {
        aim_click = 0; /* [library:input] built: the next tap aims again */
        aim_held = 0;
    }
    return buttons;
}

unsigned int MiniwinPressCount(void) {
    return press_count;
}

int MiniwinTouchMode(void) {
    return touch_mode;
}

#ifdef LEGOLAND_WEB_BROWSER
EMSCRIPTEN_KEEPALIVE
#endif
void MiniwinTypeChar(int ch) {
    SDL_Keymod mod = SDL_KMOD_NONE;
    int code;

    if (ch == 8) {
        code = SDL_SCANCODE_BACKSPACE;
    } else if (ch == '\r' || ch == '\n') {
        code = SDL_SCANCODE_RETURN;
    } else if (ch >= 'A' && ch <= 'Z') {
        code = SDL_GetScancodeFromKey((SDL_Keycode)(ch - 'A' + 'a'), NULL) | TYPED_SHIFT;
    } else if (ch >= ' ' && ch < 0x7f) {
        code = SDL_GetScancodeFromKey((SDL_Keycode)ch, &mod);
        if ((mod & SDL_KMOD_SHIFT) != 0) {
            code |= TYPED_SHIFT;
        }
    } else {
        return; /* the game's names are ASCII (GetInputChar) */
    }
    if ((code & ~TYPED_SHIFT) == SDL_SCANCODE_UNKNOWN || typed_count == TYPED_MAX) {
        return;
    }
    typed[(typed_head + typed_count) % TYPED_MAX] = code;
    typed_count++;
}

int MiniwinTypedKey(int *shift) {
    int code;

    *shift = 0;
    if (typed_count == 0) {
        return 0;
    }
    if (typed_down) {
        /* the key's up read: the next character's tap begins at the read after it */
        typed_down = 0;
        typed_head = (typed_head + 1) % TYPED_MAX;
        typed_count--;
        return 0;
    }
    typed_down = 1;
    code = typed[typed_head];
    *shift = (code & TYPED_SHIFT) != 0;
    return code & ~TYPED_SHIFT;
}

int MiniwinKeyLatched(int scancode) {
    int was;

    if (scancode < 0 || scancode >= SDL_SCANCODE_COUNT) {
        return 0;
    }
    was = latched[scancode];
    latched[scancode] = false;
    return was;
}

int MiniwinCapsLock(void) {
    return (SDL_GetModState() & SDL_KMOD_CAPS) != 0;
}

void MiniwinPresent(const void *pixels, int pitch, int width, int height, const RECT *dest) {
    SDL_FRect rect;

    if (renderer == NULL) {
        return;
    }
    if (texture == NULL || texture_width != width || texture_height != height) {
        if (texture != NULL) {
            SDL_DestroyTexture(texture);
        }
        texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGB565, SDL_TEXTUREACCESS_STREAMING, width, height);
        if (texture == NULL) {
            return;
        }
        SDL_SetTextureScaleMode(texture, SDL_SCALEMODE_NEAREST);
        texture_width = width;
        texture_height = height;
    }
    SDL_UpdateTexture(texture, NULL, pixels, pitch);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);
    rect.x = (float)dest->left;
    rect.y = (float)dest->top;
    rect.w = (float)(dest->right - dest->left);
    rect.h = (float)(dest->bottom - dest->top);
    SDL_RenderTexture(renderer, texture, NULL, &rect);
    SDL_RenderPresent(renderer);
}
