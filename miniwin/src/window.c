/* [library:window] miniwin's window: the game's window, renderer and events on SDL3. See miniwin/miniwin.h. */
#include <SDL3/SDL.h>

#include "miniwin/miniwin.h"

static SDL_Window *window;
static SDL_Renderer *renderer;
static SDL_Texture *texture;
static int texture_width;
static int texture_height;
static HWND window_hwnd;

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
    window_hwnd = (HWND)SDL_GetPointerProperty(SDL_GetWindowProperties(window), SDL_PROP_WINDOW_WIN32_HWND_POINTER, NULL);
    return window_hwnd;
}

/* The game's window procedure only reacts to a few messages; these are the SDL events that stand for them. */
void MiniwinPumpEvents(WNDPROC proc) {
    SDL_Event event;

    while (SDL_PollEvent(&event)) {
        switch (event.type) {
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
}

int MiniwinOutputSize(int *width, int *height) {
    return renderer != NULL && SDL_GetCurrentRenderOutputSize(renderer, width, height);
}

int MiniwinHasFocus(void) {
    return window != NULL && (SDL_GetWindowFlags(window) & SDL_WINDOW_INPUT_FOCUS) != 0;
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
