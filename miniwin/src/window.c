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
 * two-finger press (the secondary button) keeps the primary one up until every finger is lifted. */
static int touch_mode;
static int touch_press;
static int touch_reads;
static int touch_secondary;

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
            fingers++;
            touch_mode = 1;
            if (fingers >= 2) {
                touch_secondary = 1;
                touch_press = 0;
            }
            if (fingers == 3) {
                /* [library:input] three fingers: the on-screen keyboard, for typing names */
                ShowKeyboard(!keyboard_shown);
                keyboard_auto = 0;
            }
            break;
        case SDL_EVENT_FINGER_UP:
        case SDL_EVENT_FINGER_CANCELED:
            fingers = fingers > 0 ? fingers - 1 : 0;
            if (fingers == 0) {
                touch_secondary = 0;
            }
            break;
        case SDL_EVENT_MOUSE_MOTION:
            if (!FromTouch(event.motion.which)) {
                touch_mode = 0; /* a mouse: the game's pointer again */
            }
            break;
        case SDL_EVENT_MOUSE_BUTTON_DOWN:
            if (FromTouch(event.button.which) && event.button.button == SDL_BUTTON_LEFT && !touch_secondary) {
                touch_mode = 1;
                touch_press = 1;
                touch_reads = 0;
            }
            break;
        case SDL_EVENT_MOUSE_BUTTON_UP:
            if (FromTouch(event.button.which) && event.button.button == SDL_BUTTON_LEFT && touch_press == 1) {
                touch_press = 2;
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

    if (x != NULL) {
        *x = (int)(fx * density);
    }
    if (y != NULL) {
        *y = (int)(fy * density);
    }
    if (fingers >= 2) {
        /* [library:input] two fingers on a touch screen: the secondary button */
        return 2;
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

    if (fingers >= 2 || touch_secondary) {
        return buttons & ~1u; /* the secondary button (two fingers), not the first finger's press */
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
    return buttons;
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
