/* [library:input] miniwin's DirectInput: the keyboard and mouse devices the game reads, on SDL3. See
 * miniwin/miniwin.h.
 *
 * The keyboard's state is DirectInput's: 256 bytes indexed by DIK code (the PC scancode, plus 0x80 for the
 * extended keys), 0x80 for a key that is down. SDL keeps the same physical-key state by its own scancodes; the
 * table below translates them. */
#include <SDL3/SDL.h>
#include <stdlib.h>
#include <string.h>

#include "miniwin/miniwin.h"

/* The data formats the game passes to SetDataFormat (they came from dinput.lib). Only their identity matters
 * here: the device's GUID already says what it is. */
const DIDATAFORMAT c_dfDIKeyboard = {sizeof(DIDATAFORMAT), sizeof(DIOBJECTDATAFORMAT), DIDF_RELAXIS, 256, 0, NULL};
const DIDATAFORMAT c_dfDIMouse = {
    sizeof(DIDATAFORMAT), sizeof(DIOBJECTDATAFORMAT), DIDF_RELAXIS, sizeof(DIMOUSESTATE), 0, NULL};

/* SDL scancode -> DIK code: the inverse of SDL's own Windows table (src/events/scancodes_windows.h, which is
 * indexed by exactly the DIK value), generated from SDL 3.4.18. */
static const struct {
    SDL_Scancode sdl;
    unsigned char dik;
} KeyTable[] = {
    {SDL_SCANCODE_ESCAPE, 0x01},
    {SDL_SCANCODE_1, 0x02},
    {SDL_SCANCODE_2, 0x03},
    {SDL_SCANCODE_3, 0x04},
    {SDL_SCANCODE_4, 0x05},
    {SDL_SCANCODE_5, 0x06},
    {SDL_SCANCODE_6, 0x07},
    {SDL_SCANCODE_7, 0x08},
    {SDL_SCANCODE_8, 0x09},
    {SDL_SCANCODE_9, 0x0a},
    {SDL_SCANCODE_0, 0x0b},
    {SDL_SCANCODE_MINUS, 0x0c},
    {SDL_SCANCODE_EQUALS, 0x0d},
    {SDL_SCANCODE_BACKSPACE, 0x0e},
    {SDL_SCANCODE_TAB, 0x0f},
    {SDL_SCANCODE_Q, 0x10},
    {SDL_SCANCODE_W, 0x11},
    {SDL_SCANCODE_E, 0x12},
    {SDL_SCANCODE_R, 0x13},
    {SDL_SCANCODE_T, 0x14},
    {SDL_SCANCODE_Y, 0x15},
    {SDL_SCANCODE_U, 0x16},
    {SDL_SCANCODE_I, 0x17},
    {SDL_SCANCODE_O, 0x18},
    {SDL_SCANCODE_P, 0x19},
    {SDL_SCANCODE_LEFTBRACKET, 0x1a},
    {SDL_SCANCODE_RIGHTBRACKET, 0x1b},
    {SDL_SCANCODE_RETURN, 0x1c},
    {SDL_SCANCODE_LCTRL, 0x1d},
    {SDL_SCANCODE_A, 0x1e},
    {SDL_SCANCODE_S, 0x1f},
    {SDL_SCANCODE_D, 0x20},
    {SDL_SCANCODE_F, 0x21},
    {SDL_SCANCODE_G, 0x22},
    {SDL_SCANCODE_H, 0x23},
    {SDL_SCANCODE_J, 0x24},
    {SDL_SCANCODE_K, 0x25},
    {SDL_SCANCODE_L, 0x26},
    {SDL_SCANCODE_SEMICOLON, 0x27},
    {SDL_SCANCODE_APOSTROPHE, 0x28},
    {SDL_SCANCODE_GRAVE, 0x29},
    {SDL_SCANCODE_LSHIFT, 0x2a},
    {SDL_SCANCODE_BACKSLASH, 0x2b},
    {SDL_SCANCODE_Z, 0x2c},
    {SDL_SCANCODE_X, 0x2d},
    {SDL_SCANCODE_C, 0x2e},
    {SDL_SCANCODE_V, 0x2f},
    {SDL_SCANCODE_B, 0x30},
    {SDL_SCANCODE_N, 0x31},
    {SDL_SCANCODE_M, 0x32},
    {SDL_SCANCODE_COMMA, 0x33},
    {SDL_SCANCODE_PERIOD, 0x34},
    {SDL_SCANCODE_SLASH, 0x35},
    {SDL_SCANCODE_RSHIFT, 0x36},
    {SDL_SCANCODE_KP_MULTIPLY, 0x37},
    {SDL_SCANCODE_LALT, 0x38},
    {SDL_SCANCODE_SPACE, 0x39},
    {SDL_SCANCODE_CAPSLOCK, 0x3a},
    {SDL_SCANCODE_F1, 0x3b},
    {SDL_SCANCODE_F2, 0x3c},
    {SDL_SCANCODE_F3, 0x3d},
    {SDL_SCANCODE_F4, 0x3e},
    {SDL_SCANCODE_F5, 0x3f},
    {SDL_SCANCODE_F6, 0x40},
    {SDL_SCANCODE_F7, 0x41},
    {SDL_SCANCODE_F8, 0x42},
    {SDL_SCANCODE_F9, 0x43},
    {SDL_SCANCODE_F10, 0x44},
    {SDL_SCANCODE_NUMLOCKCLEAR, 0x45},
    {SDL_SCANCODE_SCROLLLOCK, 0x46},
    {SDL_SCANCODE_KP_7, 0x47},
    {SDL_SCANCODE_KP_8, 0x48},
    {SDL_SCANCODE_KP_9, 0x49},
    {SDL_SCANCODE_KP_MINUS, 0x4a},
    {SDL_SCANCODE_KP_4, 0x4b},
    {SDL_SCANCODE_KP_5, 0x4c},
    {SDL_SCANCODE_KP_6, 0x4d},
    {SDL_SCANCODE_KP_PLUS, 0x4e},
    {SDL_SCANCODE_KP_1, 0x4f},
    {SDL_SCANCODE_KP_2, 0x50},
    {SDL_SCANCODE_KP_3, 0x51},
    {SDL_SCANCODE_KP_0, 0x52},
    {SDL_SCANCODE_KP_PERIOD, 0x53},
    {SDL_SCANCODE_NONUSBACKSLASH, 0x56},
    {SDL_SCANCODE_F11, 0x57},
    {SDL_SCANCODE_F12, 0x58},
    {SDL_SCANCODE_KP_EQUALS, 0x59},
    {SDL_SCANCODE_INTERNATIONAL6, 0x5c},
    {SDL_SCANCODE_F13, 0x64},
    {SDL_SCANCODE_F14, 0x65},
    {SDL_SCANCODE_F15, 0x66},
    {SDL_SCANCODE_F16, 0x67},
    {SDL_SCANCODE_F17, 0x68},
    {SDL_SCANCODE_F18, 0x69},
    {SDL_SCANCODE_F19, 0x6a},
    {SDL_SCANCODE_F20, 0x6b},
    {SDL_SCANCODE_F21, 0x6c},
    {SDL_SCANCODE_F22, 0x6d},
    {SDL_SCANCODE_F23, 0x6e},
    {SDL_SCANCODE_INTERNATIONAL2, 0x70},
    {SDL_SCANCODE_LANG2, 0x71},
    {SDL_SCANCODE_LANG1, 0x72},
    {SDL_SCANCODE_INTERNATIONAL1, 0x73},
    {SDL_SCANCODE_F24, 0x76},
    {SDL_SCANCODE_LANG4, 0x77},
    {SDL_SCANCODE_LANG3, 0x78},
    {SDL_SCANCODE_INTERNATIONAL4, 0x79},
    {SDL_SCANCODE_INTERNATIONAL5, 0x7b},
    {SDL_SCANCODE_INTERNATIONAL3, 0x7d},
    {SDL_SCANCODE_KP_COMMA, 0x7e},
    {SDL_SCANCODE_AGAIN, 0x87},
    {SDL_SCANCODE_UNDO, 0x88},
    {SDL_SCANCODE_PASTE, 0x8a},
    {SDL_SCANCODE_MEDIA_PREVIOUS_TRACK, 0x90},
    {SDL_SCANCODE_CUT, 0x97},
    {SDL_SCANCODE_COPY, 0x98},
    {SDL_SCANCODE_MEDIA_NEXT_TRACK, 0x99},
    {SDL_SCANCODE_KP_ENTER, 0x9c},
    {SDL_SCANCODE_RCTRL, 0x9d},
    {SDL_SCANCODE_MUTE, 0xa0},
    {SDL_SCANCODE_MEDIA_PLAY_PAUSE, 0xa2},
    {SDL_SCANCODE_MEDIA_STOP, 0xa4},
    {SDL_SCANCODE_MEDIA_EJECT, 0xac},
    {SDL_SCANCODE_VOLUMEDOWN, 0xae},
    {SDL_SCANCODE_VOLUMEUP, 0xb0},
    {SDL_SCANCODE_AC_HOME, 0xb2},
    {SDL_SCANCODE_KP_DIVIDE, 0xb5},
    {SDL_SCANCODE_PRINTSCREEN, 0xb7},
    {SDL_SCANCODE_RALT, 0xb8},
    {SDL_SCANCODE_HELP, 0xbb},
    {SDL_SCANCODE_AC_NEW, 0xbe},
    {SDL_SCANCODE_AC_OPEN, 0xbf},
    {SDL_SCANCODE_AC_CLOSE, 0xc0},
    {SDL_SCANCODE_PAUSE, 0xc5}, /* DIK_PAUSE; SDL packs it as 0xc6 */
    {SDL_SCANCODE_HOME, 0xc7},
    {SDL_SCANCODE_UP, 0xc8},
    {SDL_SCANCODE_PAGEUP, 0xc9},
    {SDL_SCANCODE_LEFT, 0xcb},
    {SDL_SCANCODE_RIGHT, 0xcd},
    {SDL_SCANCODE_END, 0xcf},
    {SDL_SCANCODE_DOWN, 0xd0},
    {SDL_SCANCODE_PAGEDOWN, 0xd1},
    {SDL_SCANCODE_INSERT, 0xd2},
    {SDL_SCANCODE_DELETE, 0xd3},
    {SDL_SCANCODE_AC_SAVE, 0xd7},
    {SDL_SCANCODE_AC_PRINT, 0xd8},
    {SDL_SCANCODE_LGUI, 0xdb},
    {SDL_SCANCODE_RGUI, 0xdc},
    {SDL_SCANCODE_APPLICATION, 0xdd},
    {SDL_SCANCODE_POWER, 0xde},
    {SDL_SCANCODE_SLEEP, 0xdf},
    {SDL_SCANCODE_WAKE, 0xe3},
    {SDL_SCANCODE_AC_SEARCH, 0xe5},
    {SDL_SCANCODE_AC_BOOKMARKS, 0xe6},
    {SDL_SCANCODE_AC_REFRESH, 0xe7},
    {SDL_SCANCODE_AC_STOP, 0xe8},
    {SDL_SCANCODE_AC_FORWARD, 0xe9},
    {SDL_SCANCODE_AC_BACK, 0xea},
    {SDL_SCANCODE_MEDIA_SELECT, 0xed},
};

static unsigned char DikFromSdl[SDL_SCANCODE_COUNT];
static int table_ready;

/* The keyboard as DirectInput's 256 bytes; on Android, with latched, a key that went down and up since the last
 * such read too (the game's reads, one per frame: the on-screen keyboard's taps are shorter). */
static void KeyboardBytes(unsigned char *state, int latched) {
    int count = 0;
    const bool *keys = SDL_GetKeyboardState(&count);
    int i;

    if (!table_ready) {
        size_t k;
        for (k = 0; k < sizeof(KeyTable) / sizeof(KeyTable[0]); k++) {
            DikFromSdl[KeyTable[k].sdl] = KeyTable[k].dik;
        }
        table_ready = 1;
    }
    memset(state, 0, 256);
    for (i = 0; i < count && i < SDL_SCANCODE_COUNT; i++) {
#ifdef __ANDROID__
        int down = keys[i] || (latched && MiniwinKeyLatched(i)); /* the on-screen keyboard's taps */
#else
        int down = keys[i];
#endif
        (void)latched;
        if (down && DikFromSdl[i] != 0) {
            state[DikFromSdl[i]] = 0x80;
        }
    }
#ifdef __ANDROID__
    /* [library:input] the back key (or gesture): Escape, the game's way back */
    if (state[DikFromSdl[SDL_SCANCODE_AC_BACK]] != 0) {
        state[0x01] = 0x80;
    }
#endif
}

int MiniwinKeyDown(int dik) {
    unsigned char state[256];

    if (dik <= 0 || dik > 255 || !MiniwinHasFocus()) {
        return 0;
    }
    KeyboardBytes(state, 0);
    return state[dik] != 0;
}

/* ---- devices ---- */

enum {
    DEVICE_KEYBOARD,
    DEVICE_MOUSE
};

struct Device {
    IDirectInputDeviceA iface;
    LONG refs;
    int kind;
};

static struct Device *AsDevice(IDirectInputDeviceA *p) {
    return (struct Device *)p;
}

static int SameGuid(REFGUID a, const GUID *b) {
    return memcmp(a, b, sizeof(GUID)) == 0;
}

static HRESULT STDMETHODCALLTYPE Device_QueryInterface(IDirectInputDeviceA *self, REFIID riid, LPVOID *out) {
    if (SameGuid(riid, &IID_IUnknown) || SameGuid(riid, &IID_IDirectInputDeviceA)) {
        self->lpVtbl->AddRef(self);
        *out = self;
        return S_OK;
    }
    *out = NULL;
    return E_NOINTERFACE;
}

static ULONG STDMETHODCALLTYPE Device_AddRef(IDirectInputDeviceA *self) {
    return (ULONG)++AsDevice(self)->refs;
}

static ULONG STDMETHODCALLTYPE Device_Release(IDirectInputDeviceA *self) {
    struct Device *d = AsDevice(self);
    LONG refs = --d->refs;

    if (refs == 0) {
        free(d);
    }
    return (ULONG)refs;
}

static HRESULT STDMETHODCALLTYPE Device_GetCapabilities(IDirectInputDeviceA *self, LPDIDEVCAPS caps) {
    struct Device *d = AsDevice(self);
    DWORD size;

    if (caps == NULL || caps->dwSize < sizeof(DIDEVCAPS_DX3)) {
        return DIERR_INVALIDPARAM;
    }
    size = caps->dwSize < sizeof(DIDEVCAPS) ? caps->dwSize : sizeof(DIDEVCAPS);
    memset(caps, 0, size);
    caps->dwSize = size;
    caps->dwFlags = DIDC_ATTACHED;
    if (d->kind == DEVICE_KEYBOARD) {
        caps->dwDevType = DIDEVTYPE_KEYBOARD;
        caps->dwButtons = 128;
    } else {
        caps->dwDevType = DIDEVTYPE_MOUSE;
        caps->dwAxes = 3;
        caps->dwButtons = 3;
    }
    return DI_OK;
}

static HRESULT STDMETHODCALLTYPE Device_EnumObjects(IDirectInputDeviceA *self, LPDIENUMDEVICEOBJECTSCALLBACKA callback,
    LPVOID context, DWORD flags) {
    (void)self;
    (void)callback;
    (void)context;
    (void)flags;
    return DIERR_UNSUPPORTED;
}

/* The mouse's granularity: a wheel notch is WHEEL_DELTA (120), as Windows' DirectInput reports for the Z axis. */
static HRESULT STDMETHODCALLTYPE Device_GetProperty(IDirectInputDeviceA *self, REFGUID prop, LPDIPROPHEADER header) {
    if (header == NULL || header->dwSize != sizeof(DIPROPDWORD)) {
        return DIERR_INVALIDPARAM;
    }
    if (prop == DIPROP_GRANULARITY && AsDevice(self)->kind == DEVICE_MOUSE) {
        ((DIPROPDWORD *)header)->dwData = header->dwObj == DIMOFS_Z ? WHEEL_DELTA : 1;
        return DI_OK;
    }
    return DIERR_UNSUPPORTED;
}

static HRESULT STDMETHODCALLTYPE Device_SetProperty(IDirectInputDeviceA *self, REFGUID prop, LPCDIPROPHEADER header) {
    (void)self;
    (void)prop;
    (void)header;
    return DI_OK;
}

static HRESULT STDMETHODCALLTYPE Device_Acquire(IDirectInputDeviceA *self) {
    (void)self;
    return DI_OK;
}

static HRESULT STDMETHODCALLTYPE Device_Unacquire(IDirectInputDeviceA *self) {
    (void)self;
    return DI_OK;
}

/* Like a foreground device: nothing is down while the window doesn't have the focus. */
static HRESULT STDMETHODCALLTYPE Device_GetDeviceState(IDirectInputDeviceA *self, DWORD size, LPVOID data) {
    struct Device *d = AsDevice(self);

    if (d->kind == DEVICE_KEYBOARD) {
        if (size != 256) {
            return DIERR_INVALIDPARAM;
        }
        if (MiniwinHasFocus()) {
            KeyboardBytes((unsigned char *)data, 1);
        } else {
            memset(data, 0, 256);
        }
        return DI_OK;
    }
    if (size != sizeof(DIMOUSESTATE)) {
        return DIERR_INVALIDPARAM;
    }
    memset(data, 0, size);
    if (MiniwinHasFocus()) {
        DIMOUSESTATE *state = (DIMOUSESTATE *)data;
        float dx = 0.0f;
        float dy = 0.0f;
        unsigned int buttons = MiniwinMouseState(NULL, NULL);

        SDL_GetRelativeMouseState(&dx, &dy);
        state->lX = (LONG)dx;
        state->lY = (LONG)dy;
        state->rgbButtons[0] = (buttons & 1) != 0 ? 0x80 : 0;
        state->rgbButtons[1] = (buttons & 2) != 0 ? 0x80 : 0;
        state->rgbButtons[2] = (buttons & 4) != 0 ? 0x80 : 0;
    }
    return DI_OK;
}

static HRESULT STDMETHODCALLTYPE Device_GetDeviceData(IDirectInputDeviceA *self, DWORD size, LPDIDEVICEOBJECTDATA data,
    LPDWORD count, DWORD flags) {
    (void)self;
    (void)size;
    (void)data;
    (void)flags;
    if (count != NULL) {
        *count = 0;
    }
    return DIERR_UNSUPPORTED;
}

static HRESULT STDMETHODCALLTYPE Device_SetDataFormat(IDirectInputDeviceA *self, LPCDIDATAFORMAT format) {
    (void)self;
    (void)format;
    return DI_OK;
}

static HRESULT STDMETHODCALLTYPE Device_SetEventNotification(IDirectInputDeviceA *self, HANDLE event) {
    (void)self;
    (void)event;
    return DIERR_UNSUPPORTED;
}

static HRESULT STDMETHODCALLTYPE Device_SetCooperativeLevel(IDirectInputDeviceA *self, HWND hwnd, DWORD flags) {
    (void)self;
    (void)hwnd;
    (void)flags;
    return DI_OK;
}

static HRESULT STDMETHODCALLTYPE Device_GetObjectInfo(IDirectInputDeviceA *self, LPDIDEVICEOBJECTINSTANCEA info,
    DWORD obj, DWORD how) {
    (void)self;
    (void)info;
    (void)obj;
    (void)how;
    return DIERR_UNSUPPORTED;
}

static HRESULT STDMETHODCALLTYPE Device_GetDeviceInfo(IDirectInputDeviceA *self, LPDIDEVICEINSTANCEA info) {
    (void)self;
    (void)info;
    return DIERR_UNSUPPORTED;
}

static HRESULT STDMETHODCALLTYPE Device_RunControlPanel(IDirectInputDeviceA *self, HWND hwnd, DWORD flags) {
    (void)self;
    (void)hwnd;
    (void)flags;
    return DIERR_UNSUPPORTED;
}

static HRESULT STDMETHODCALLTYPE Device_Initialize(IDirectInputDeviceA *self, HINSTANCE instance, DWORD version,
    REFGUID guid) {
    (void)self;
    (void)instance;
    (void)version;
    (void)guid;
    return S_FALSE; /* already initialized */
}

static IDirectInputDeviceAVtbl DeviceVtbl = {
    Device_QueryInterface,
    Device_AddRef,
    Device_Release,
    Device_GetCapabilities,
    Device_EnumObjects,
    Device_GetProperty,
    Device_SetProperty,
    Device_Acquire,
    Device_Unacquire,
    Device_GetDeviceState,
    Device_GetDeviceData,
    Device_SetDataFormat,
    Device_SetEventNotification,
    Device_SetCooperativeLevel,
    Device_GetObjectInfo,
    Device_GetDeviceInfo,
    Device_RunControlPanel,
    Device_Initialize,
};

/* ---- IDirectInputA ---- */

struct DirectInput {
    IDirectInputA iface;
    LONG refs;
};

static int DeviceKind(REFGUID guid) {
    if (SameGuid(guid, &GUID_SysKeyboard)) {
        return DEVICE_KEYBOARD;
    }
    if (SameGuid(guid, &GUID_SysMouse)) {
        return DEVICE_MOUSE;
    }
    return -1;
}

static HRESULT STDMETHODCALLTYPE Input_QueryInterface(IDirectInputA *self, REFIID riid, LPVOID *out) {
    if (SameGuid(riid, &IID_IUnknown) || SameGuid(riid, &IID_IDirectInputA)) {
        self->lpVtbl->AddRef(self);
        *out = self;
        return S_OK;
    }
    *out = NULL;
    return E_NOINTERFACE;
}

static ULONG STDMETHODCALLTYPE Input_AddRef(IDirectInputA *self) {
    return (ULONG)++((struct DirectInput *)self)->refs;
}

static ULONG STDMETHODCALLTYPE Input_Release(IDirectInputA *self) {
    struct DirectInput *di = (struct DirectInput *)self;
    LONG refs = --di->refs;

    if (refs == 0) {
        free(di);
    }
    return (ULONG)refs;
}

static HRESULT STDMETHODCALLTYPE Input_CreateDevice(IDirectInputA *self, REFGUID guid, LPDIRECTINPUTDEVICEA *out,
    LPUNKNOWN outer) {
    struct Device *d;
    int kind = DeviceKind(guid);

    (void)self;
    (void)outer;
    *out = NULL;
    if (kind < 0) {
        return DIERR_DEVICENOTREG;
    }
    d = (struct Device *)calloc(1, sizeof(*d));
    if (d == NULL) {
        return DIERR_OUTOFMEMORY;
    }
    d->iface.lpVtbl = &DeviceVtbl;
    d->refs = 1;
    d->kind = kind;
    *out = &d->iface;
    return DI_OK;
}

static HRESULT STDMETHODCALLTYPE Input_EnumDevices(IDirectInputA *self, DWORD type, LPDIENUMDEVICESCALLBACKA callback,
    LPVOID context, DWORD flags) {
    (void)self;
    (void)type;
    (void)callback;
    (void)context;
    (void)flags;
    return DIERR_UNSUPPORTED;
}

static HRESULT STDMETHODCALLTYPE Input_GetDeviceStatus(IDirectInputA *self, REFGUID guid) {
    (void)self;
    return DeviceKind(guid) >= 0 ? DI_OK : DIERR_DEVICENOTREG;
}

static HRESULT STDMETHODCALLTYPE Input_RunControlPanel(IDirectInputA *self, HWND hwnd, DWORD flags) {
    (void)self;
    (void)hwnd;
    (void)flags;
    return DIERR_UNSUPPORTED;
}

static HRESULT STDMETHODCALLTYPE Input_Initialize(IDirectInputA *self, HINSTANCE instance, DWORD version) {
    (void)self;
    (void)instance;
    (void)version;
    return S_FALSE; /* already initialized */
}

static IDirectInputAVtbl InputVtbl = {
    Input_QueryInterface,
    Input_AddRef,
    Input_Release,
    Input_CreateDevice,
    Input_EnumDevices,
    Input_GetDeviceStatus,
    Input_RunControlPanel,
    Input_Initialize,
};

HRESULT WINAPI MiniwinDirectInputCreate(HINSTANCE instance, DWORD version, LPDIRECTINPUTA *out, LPUNKNOWN outer) {
    struct DirectInput *di;

    (void)instance;
    (void)version;
    if (out == NULL || outer != NULL) {
        return DIERR_INVALIDPARAM;
    }
    di = (struct DirectInput *)calloc(1, sizeof(*di));
    if (di == NULL) {
        *out = NULL;
        return DIERR_OUTOFMEMORY;
    }
    di->iface.lpVtbl = &InputVtbl;
    di->refs = 1;
    *out = &di->iface;
    return DI_OK;
}
