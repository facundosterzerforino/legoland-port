#include <windows.h>
#include <dinput.h>
#include "debug.h"
#include "legoland.h"

#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include "bricks.h"
#include "controller.h"
#include "debug_alloc.h"
#include "globals.h"
#include "icon.h"
#include "input.h"
#include "llidb.h"
#include "map_object.h"
#include "nerps.h"
#include "screens.h"
#include "sound_sfx.h"
#include "wndenv.h"
#ifdef LEGOLAND_PORT
#include "miniwin/miniwin.h"
#include "port_display.h"
#include "port_input.h"
#endif

struct DInputDeviceVtbl {
    void *pad_0[2];
    void(__stdcall *Release)(void *self);
    void *pad_c[5];
    void(__stdcall *Unacquire)(void *self);
};

struct DInputDevice {
    struct DInputDeviceVtbl *vtable;
};

// FUNCTION: LEGOLAND 0x00473870
LEGO_EXPORT int InitInputSystem(void) {
    HINSTANCE hinst;

#ifdef LEGOLAND_PORT
    /* [library:input] miniwin's DirectInput: the keyboard and mouse from SDL3 */
    hinst = NULL;
    MiniwinDirectInputCreate(hinst, 0x300, (LPDIRECTINPUTA *)&dinput, NULL);
#else
    hinst = (HINSTANCE)GetWindowLongA(WNDENV_Gethwnd(), GWL_HINSTANCE);
    DirectInputCreateA(hinst, 0x300, (LPDIRECTINPUTA *)&dinput, NULL);
#endif
    if (CreateKeyboardDevice() == 0) {
        return 0;
    }
    return CreateMouseDevice() != 0;
}

// FUNCTION: LEGOLAND 0x004738b0
int CreateKeyboardDevice(void) {
    DIDEVCAPS caps;

    if (IDirectInput_CreateDevice((LPDIRECTINPUTA)dinput, &GUID_SysKeyboard, (LPDIRECTINPUTDEVICEA *)&dinput_keyboard, NULL) == 0) {
        IDirectInputDevice_SetDataFormat((LPDIRECTINPUTDEVICEA)dinput_keyboard, &c_dfDIKeyboard);
        /* [library:input] the original never set the keyboard's cooperative level (DirectX 3 on Win9x defaulted
         * it); modern DirectInput refuses Acquire with E_INVALIDARG until it is set, so ScanKeyboard spun forever. */
        IDirectInputDevice_SetCooperativeLevel((LPDIRECTINPUTDEVICEA)dinput_keyboard, WNDENV_Gethwnd(),
            DISCL_NONEXCLUSIVE | DISCL_FOREGROUND);
        caps.dwSize = 0x2c;
        IDirectInputDevice_GetCapabilities((LPDIRECTINPUTDEVICEA)dinput_keyboard, &caps);
        if (!((caps.dwFlags == 0) & 1)) {
            return IDirectInput_GetDeviceStatus((LPDIRECTINPUTA)dinput, &GUID_SysKeyboard) == 0;
        }
    }
    return 0;
}

static int kb_trace; /* [port] trace throttles */
static int ms_trace;

// FUNCTION: LEGOLAND 0x00473930
LEGO_EXPORT void ScanKeyboard(void) {
    HRESULT hr;

    if (dinput_keyboard == NULL) {
        return;
    }
    hr = IDirectInputDevice_GetDeviceState((IDirectInputDeviceA *)dinput_keyboard, 0x100, KeyboardState);
    if (hr == DI_OK) {
        return;
    }
    if (kb_trace < 5) {
        kb_trace++;
        DebugTrace("ScanKeyboard: GetDeviceState hr=%lx", hr);
    }
    /* [library:input] the original retried Acquire until it worked. A foreground device can't be acquired while
     * another window is active, so that loop hung the game whenever it lost focus (or never got it at launch);
     * try once, and read no keys this frame if it still fails. ProcessSystemEvents pauses while inactive. */
    if (hr == DIERR_INPUTLOST || hr == DIERR_NOTACQUIRED) {
        hr = IDirectInputDevice_Acquire((IDirectInputDeviceA *)dinput_keyboard);
        if (hr == DI_OK || hr == S_FALSE) {
            hr = IDirectInputDevice_GetDeviceState((IDirectInputDeviceA *)dinput_keyboard, 0x100, KeyboardState);
        }
    }
    if (hr != DI_OK) {
        memset(KeyboardState, 0, 0x100);
    }
}

// FUNCTION: LEGOLAND 0x00473970
int CreateMouseDevice(void) {
    DIPROPDWORD prop;
    DIDEVCAPS caps;

    if (IDirectInput_CreateDevice((LPDIRECTINPUTA)dinput, &GUID_SysMouse, (LPDIRECTINPUTDEVICEA *)&dintput_mouse, NULL) == 0) {
        IDirectInputDevice_SetCooperativeLevel((LPDIRECTINPUTDEVICEA)dintput_mouse, WNDENV_Gethwnd(), 5);
        IDirectInputDevice_SetDataFormat((LPDIRECTINPUTDEVICEA)dintput_mouse, &c_dfDIMouse);
        prop.diph.dwSize = 0x14;
        prop.diph.dwHeaderSize = 0x10;
        prop.diph.dwObj = 8;
        prop.diph.dwHow = 1;
        if (IDirectInputDevice_GetProperty((LPDIRECTINPUTDEVICEA)dintput_mouse, (REFGUID)3, &prop.diph) == 0) {
            mouse_granularity = prop.dwData;
        }
        caps.dwSize = 0x2c;
        IDirectInputDevice_GetCapabilities((LPDIRECTINPUTDEVICEA)dintput_mouse, &caps);
        if (!((caps.dwFlags == 0) & 1)) {
            return IDirectInput_GetDeviceStatus((LPDIRECTINPUTA)dinput, &GUID_SysMouse) == 0;
        }
    }
    return 0;
}

// FUNCTION: LEGOLAND 0x00473a50
void ReleaseKeyboardDevice(void) {
    if (dinput_keyboard != NULL) {
        IDirectInputDevice_Release((LPDIRECTINPUTDEVICEA)dinput_keyboard);
    }
}

// FUNCTION: LEGOLAND 0x00473a60
void ReleaseMouseDevice(void) {
    if (dintput_mouse != NULL) {
        ((struct DInputDevice *)dintput_mouse)->vtable->Unacquire(dintput_mouse);
        ((struct DInputDevice *)dintput_mouse)->vtable->Release(dintput_mouse);
    }
}

// FUNCTION: LEGOLAND 0x00473a80
LEGO_EXPORT void ScanMouse(void) {
    HRESULT hr;

    if (WinDebugMode != 0) {
        /* [library:input] windowed mode: the exclusive DirectInput mouse reported motion that made the game's
         * cursor drift to the bottom-right. Use the window's own cursor instead (position in
         * UpdateControllerFromMouseData, buttons here, the wheel from WM_MOUSEWHEEL: port_input.h). */
        memset(&MouseState, 0, sizeof(MouseState));
#ifdef LEGOLAND_PORT
        {
            /* [library:input] the buttons from SDL3 (miniwin), while the window has the focus; a finger's taps as
             * clicks where it touched (MiniwinMouseButtons) */
            unsigned int buttons = MiniwinHasFocus() ? MiniwinMouseButtons() : 0;

            PortTouchFrame(); /* [library:input] dragged lists, the two-finger pan (port_input.h) */

            MouseState.rgbButtons[0] = (buttons & 1) != 0 ? 0x80 : 0;
            MouseState.rgbButtons[1] = (buttons & 2) != 0 ? 0x80 : 0;
            MouseState.rgbButtons[2] = (buttons & 4) != 0 ? 0x80 : 0;
        }
#else
        if (GetForegroundWindow() == WNDENV_Gethwnd()) {
            if ((GetAsyncKeyState(GetSystemMetrics(SM_SWAPBUTTON) ? VK_RBUTTON : VK_LBUTTON) & 0x8000) != 0) {
                MouseState.rgbButtons[0] = 0x80;
            }
            if ((GetAsyncKeyState(GetSystemMetrics(SM_SWAPBUTTON) ? VK_LBUTTON : VK_RBUTTON) & 0x8000) != 0) {
                MouseState.rgbButtons[1] = 0x80;
            }
            if ((GetAsyncKeyState(VK_MBUTTON) & 0x8000) != 0) {
                MouseState.rgbButtons[2] = 0x80;
            }
        }
#endif
#ifdef LEGOLAND_PORT
        MouseState.lZ = PortWheelTake(mouse_granularity > 0 ? mouse_granularity : WHEEL_DELTA);
#endif
        if (MouseState.lZ == 0) {
            return; /* no wheel this frame (and no "lZ <= -0" scroll if the granularity were unknown) */
        }
    } else if (dintput_mouse != NULL) {
        hr = IDirectInputDevice_GetDeviceState((IDirectInputDeviceA *)dintput_mouse, 0x10, &MouseState);
        if (hr != DI_OK) {
            if (ms_trace < 5) {
                ms_trace++;
                DebugTrace("ScanMouse: GetDeviceState hr=%lx", hr);
            }
            /* [library:input] as in ScanKeyboard: one Acquire, then no movement or buttons this frame */
            if (hr == DIERR_INPUTLOST || hr == DIERR_NOTACQUIRED) {
                hr = IDirectInputDevice_Acquire((IDirectInputDeviceA *)dintput_mouse);
                if (hr == DI_OK || hr == S_FALSE) {
                    hr = IDirectInputDevice_GetDeviceState((IDirectInputDeviceA *)dintput_mouse, 0x10, &MouseState);
                }
            }
            if (hr != DI_OK) {
                memset(&MouseState, 0, sizeof(MouseState));
            }
        }
    }
    if (MouseState.lZ <= -mouse_granularity) {
        ScrollActiveIconRegionBack();
    } else if (MouseState.lZ >= mouse_granularity) {
        ScrollActiveIconRegionForward();
    }
}

// FUNCTION: LEGOLAND 0x00473ae0
LEGO_EXPORT void KillInputSystem(void) {
    ReleaseKeyboardDevice();
    ReleaseMouseDevice();
    if (dinput != NULL) {
        ((struct DInputDevice *)dinput)->vtable->Release(dinput);
    }
}

// Updates the mouse controller from the DirectInput state: remembers the previous position, adds
// the movement (doubled once past mouse_threshold1, and again past mouse_threshold2 when
// mouse_accel is 2), clamps the pointer to the screen, stores the deltas and the three buttons
// (bit 0 left, bit 1 right, bit 2 middle).
//
// MSVC6 build: exact naked __asm transcription; the C in the #else branch is the same function and
// each asm block is commented with the C it implements (CtrlBuffer offsets are named in the
// comments). Why asm: the C reached 90.83% only with volatile casts and search-tool temporaries,
// and the rest is register choice (the original keeps 0 in edx for the clamps and lpConfig in
// edi). See docs/attempts/0x00473b00-UpdateControllerFromMouseData.md. With the asm matching, the
// C below is the plain version (no volatile, no temporaries).
// clang-format off
#if defined(_MSC_VER) && (_MSC_VER <= 1200) && defined(_M_IX86)
// FUNCTION: LEGOLAND 0x00473b00
__declspec(naked) LEGO_EXPORT void UpdateControllerFromMouseData(struct CtrlBuffer *buffer) {
    __asm {
        // if (buffer == NULL) return;
        mov ecx, dword ptr [esp + 4] // ecx = buffer for the whole function
        test ecx, ecx
        je done
        // buffer->prev_x = buffer->x; buffer->prev_y = buffer->y;
        mov eax, dword ptr [ecx + 8] // buffer->x
        mov edx, dword ptr [ecx + 0xc] // buffer->y
        push ebx
        mov ebx, dword ptr [ecx + 0x24] // ebx = buffer->mouse_accel
        push esi
        push edi
        mov dword ptr [ecx], eax // buffer->prev_x
        mov dword ptr [ecx + 4], edx // buffer->prev_y
        // dx = MouseState.lX; dy = MouseState.lY;   (esi = dx, edi = dy)
        mov esi, dword ptr [MouseState]
        mov edi, dword ptr [MouseState + 0x4]
        // if (buffer->mouse_accel != 0) {
        test ebx, ebx
        je move
        //     if (abs(dx) > buffer->mouse_threshold1 || abs(dy) > buffer->mouse_threshold1) { dx *= 2; dy *= 2; }
        mov eax, esi
        cdq
        xor eax, edx
        sub eax, edx
        mov edx, dword ptr [ecx + 0x1c] // buffer->mouse_threshold1
        cmp eax, edx
        jg accel1
        mov eax, edi
        cdq
        xor eax, edx
        sub eax, edx
        mov edx, dword ptr [ecx + 0x1c]
        cmp eax, edx
        jle accel1_done
    accel1:
        add esi, esi
        add edi, edi
    accel1_done:
        //     if (buffer->mouse_accel == 2 &&
        //         (abs(dx) > buffer->mouse_threshold2 || abs(dy) > buffer->mouse_threshold2)) { dx *= 2; dy *= 2; }
        cmp ebx, 2
        jne move
        mov ebx, dword ptr [ecx + 0x20] // buffer->mouse_threshold2
        mov eax, esi
        cdq
        xor eax, edx
        sub eax, edx
        cmp eax, ebx
        jg accel2
        mov eax, edi
        cdq
        xor eax, edx
        sub eax, edx
        cmp eax, ebx
        jle move
    accel2:
        shl esi, 1
        shl edi, 1
        // }
    move:
        // buffer->x += dx; buffer->y += dy;
        mov eax, dword ptr [ecx + 8]
        mov ebx, dword ptr [ecx + 0xc]
        add eax, esi
        add ebx, edi
        mov dword ptr [ecx + 8], eax
        mov dword ptr [ecx + 0xc], ebx
        // if (buffer->x >= lpConfig->screen_width) buffer->x = lpConfig->screen_width - 1;
        mov edi, dword ptr [lpConfig]
        mov edx, eax
        xor eax, eax
        mov esi, ebx
        mov ax, word ptr [edi] // lpConfig->screen_width
        cmp edx, eax
        jl x_below_max
        dec eax
        mov dword ptr [ecx + 8], eax
    x_below_max:
        // if (buffer->x < 0) buffer->x = 0;   (edx = 0 for the clamps below)
        mov eax, dword ptr [ecx + 8]
        xor edx, edx
        cmp eax, edx
        jge x_not_negative
        mov dword ptr [ecx + 8], edx
    x_not_negative:
        // if (buffer->y >= lpConfig->screen_height) buffer->y = lpConfig->screen_height - 1;
        mov edi, dword ptr [lpConfig]
        xor eax, eax
        mov ax, word ptr [edi + 2] // lpConfig->screen_height
        cmp esi, eax
        jl y_below_max
        dec eax
        mov dword ptr [ecx + 0xc], eax
    y_below_max:
        // if (buffer->y < 0) buffer->y = 0;
        cmp dword ptr [ecx + 0xc], edx
        jge y_not_negative
        mov dword ptr [ecx + 0xc], edx
    y_not_negative:
        // buffer->delta_x = buffer->x - buffer->prev_x; buffer->delta_y = buffer->y - buffer->prev_y;
        // buffer->buttons &= ~7;
        mov eax, dword ptr [ecx + 8]
        mov edi, dword ptr [ecx]
        mov edx, dword ptr [ecx + 0xc]
        mov esi, dword ptr [ecx + 4]
        sub eax, edi
        sub edx, esi
        mov dword ptr [ecx + 0x10], eax // buffer->delta_x
        mov eax, dword ptr [ecx + 0x18] // buffer->buttons
        and al, 0xf8
        mov dword ptr [ecx + 0x14], edx // buffer->delta_y
        mov dword ptr [ecx + 0x18], eax
        // if (MouseState.rgbButtons[0] & 0x80) buffer->buttons |= 1;   (left)
        mov bl, byte ptr [MouseState + 0xc]
        mov dl, 0x80
        pop edi
        pop esi
        // test dl, bl -- emitted as bytes: MSVC's inline assembler always encodes it as
        // `test bl, dl` (84 d3), the original has the other valid encoding (84 da).
        _emit 0x84
        _emit 0xda
        pop ebx
        je left_done
        or al, 1
        mov dword ptr [ecx + 0x18], eax
    left_done:
        // if (MouseState.rgbButtons[2] & 0x80) buffer->buttons |= 4;   (middle)
        test byte ptr [MouseState + 0xe], dl
        je middle_done
        mov eax, dword ptr [ecx + 0x18]
        or al, 4
        mov dword ptr [ecx + 0x18], eax
    middle_done:
        // if (MouseState.rgbButtons[1] & 0x80) buffer->buttons |= 2;   (right)
        test byte ptr [MouseState + 0xd], dl
        je done
        mov eax, dword ptr [ecx + 0x18]
        or al, 2
        mov dword ptr [ecx + 0x18], eax
    done:
        ret
    }
}
// clang-format on
#else
LEGO_EXPORT void UpdateControllerFromMouseData(struct CtrlBuffer *buffer) {
    int dx;
    int dy;
    unsigned int mode; /* [library:input] the acceleration, off in a window (below) */

    if (buffer == NULL) {
        return;
    }
    buffer->prev_x = buffer->x;
    buffer->prev_y = buffer->y;
    mode = buffer->mouse_accel;
    dx = MouseState.lX;
    dy = MouseState.lY;
    if (WinDebugMode != 0) {
        /* [library:input] windowed mode: the game's cursor follows the window's cursor (see ScanMouse) */
        POINT pos;
#ifdef LEGOLAND_PORT
        int mx;
        int my;

        if (MiniwinHasFocus()) {
            /* [library:input] the pointer's position from SDL3 (miniwin), in the window's picture */
            MiniwinMouseState(&mx, &my);
            pos.x = mx;
            pos.y = my;
            PortDisplayClientToGame(&pos); /* [library:video] the frame is scaled in the window */
#else
        if (GetForegroundWindow() == WNDENV_Gethwnd() && GetCursorPos(&pos) && ScreenToClient(WNDENV_Gethwnd(), &pos)) {
#endif
            dx = pos.x - buffer->x;
            dy = pos.y - buffer->y;
        } else {
            dx = 0;
            dy = 0;
        }
        mode = 0;
    }
    if (mode != 0) {
        if (abs(dx) > buffer->mouse_threshold1 || abs(dy) > buffer->mouse_threshold1) {
            dx += dx;
            dy += dy;
        }
        if (buffer->mouse_accel == 2) {
            if (abs(dx) > buffer->mouse_threshold2 || abs(dy) > buffer->mouse_threshold2) {
                dx <<= 1;
                dy <<= 1;
            }
        }
    }
    buffer->x += dx;
    buffer->y += dy;
    if (buffer->x >= lpConfig->screen_width) {
        buffer->x = lpConfig->screen_width - 1;
    }
    if (buffer->x < 0) {
        buffer->x = 0;
    }
    if (buffer->y >= lpConfig->screen_height) {
        buffer->y = lpConfig->screen_height - 1;
    }
    if (buffer->y < 0) {
        buffer->y = 0;
    }
    buffer->delta_x = buffer->x - buffer->prev_x;
    buffer->delta_y = buffer->y - buffer->prev_y;
    buffer->buttons &= 0xfffffff8;
    if ((MouseState.rgbButtons[0] & 0x80) != 0) {
        buffer->buttons |= 1;
    }
    if ((MouseState.rgbButtons[2] & 0x80) != 0) {
        buffer->buttons |= 4;
    }
    if ((MouseState.rgbButtons[1] & 0x80) != 0) {
        buffer->buttons |= 2;
    }
}
#endif

// FUNCTION: LEGOLAND 0x00473c10
LEGO_EXPORT void UpdateControllerFromKeyboardData(struct CtrlBuffer *buffer) {
    char c;
    int arg;

    buffer->buttons = buffer->buttons & 0xfffff807;
    c = ReadCheatKey();
    if (c != 0) {
        memcpy(CheatKeyBuffer, CheatKeyBuffer + 1, 19);
        CheatKeyBuffer[0x13] = c;
        // STRING: LEGOLAND 0x004baf9c
        if (_memicmp(":THEME", &CheatKeyBuffer[0xe], 6) == 0) {
            SetInteractiveMusicTheme(0);
            // STRING: LEGOLAND 0x004baf88
            DBPrintf("CHEATTHEME=THEME\n");
            // STRING: LEGOLAND 0x004baf70
        } else if (_memicmp(":EGYPT", &CheatKeyBuffer[0xe], 6) == 0) {
            SetInteractiveMusicTheme(1);
            // STRING: LEGOLAND 0x004baf58
            DBPrintf("CHEATTHEME=EGYPTIAN\n");
            // STRING: LEGOLAND 0x004baf50
        } else if (_memicmp(":INCA", &CheatKeyBuffer[0xf], 5) == 0) {
            SetInteractiveMusicTheme(2);
            // STRING: LEGOLAND 0x004baf3c
            DBPrintf("CHEATTHEME=INCA\n");
            // STRING: LEGOLAND 0x004baf34
        } else if (_memicmp(":CASTLE", &CheatKeyBuffer[0xd], 7) == 0) {
            SetInteractiveMusicTheme(3);
            // STRING: LEGOLAND 0x004baf20
            DBPrintf("CHEATTHEME=CASTLE\n");
            // STRING: LEGOLAND 0x004baf18
        } else if (_memicmp(":WEST", &CheatKeyBuffer[0xf], 5) == 0) {
            SetInteractiveMusicTheme(4);
            // STRING: LEGOLAND 0x004baf04
            DBPrintf("CHEATTHEME=WEST\n");
            // STRING: LEGOLAND 0x004baefc
        } else if (_memicmp(":STOP", &CheatKeyBuffer[0xf], 5) == 0) {
            StopInteractiveMusic();
            // STRING: LEGOLAND 0x004baee8
            DBPrintf("CHEAT:STOPMUSIC\n");
            // STRING: LEGOLAND 0x004baee0
        } else if (_memicmp("::DIE", &CheatKeyBuffer[0xf], 5) == 0) {
            exit(1);
        }
        if (EditMode.unk4 == 3) {
            // STRING: LEGOLAND 0x004baf78
            if (_memicmp(":ILIKETOTRAVEL", &CheatKeyBuffer[4], 0xe) == 0) {
                if (CheatKeyBuffer[0x12] == '1') {
                    if (CheatKeyBuffer[0x13] == '0') {
                        arg = 0xa;
                        lpConfig->level = 0xf;
                        // STRING: LEGOLAND 0x004baed0
                        DBPrintf("CHEAT:Level %d\n", arg);
                        MapStats.level_end = 2;
                    }
                } else if (CheatKeyBuffer[0x12] == '0') {
                    if (CheatKeyBuffer[0x13] >= '1' && CheatKeyBuffer[0x13] <= '9') {
                        lpConfig->level = CheatKeyBuffer[0x13] - 0x2b;
                        arg = lpConfig->level;
                        DBPrintf("CHEAT:Level %d\n", arg);
                        MapStats.level_end = 2;
                    }
                } else if (CheatKeyBuffer[0x12] == 'T' && CheatKeyBuffer[0x13] >= '1' && CheatKeyBuffer[0x13] <= '5') {
                    lpConfig->level = CheatKeyBuffer[0x13] - 0x30;
                    arg = lpConfig->level;
                    DBPrintf("CHEAT:Level %d\n", arg);
                    MapStats.level_end = 2;
                }
                // STRING: LEGOLAND 0x004baec0
            } else if (_memicmp(":COLDHARDCASH", &CheatKeyBuffer[7], 0xd) == 0) {
                // STRING: LEGOLAND 0x004baeac
                DBPrintf("CHEAT:More Money\n");
                AddBricks(5000);
                // STRING: LEGOLAND 0x004bae9c
            } else if (_memicmp(":HARDASNAILS", &CheatKeyBuffer[8], 0xc) == 0) {
                // STRING: LEGOLAND 0x004bae88
                DBPrintf("CHEAT:No Ride Wear\n");
                MapStats.ride_wear = 0;
                // STRING: LEGOLAND 0x004bae7c
            } else if (_memicmp(":PRAISEME", &CheatKeyBuffer[0xb], 9) == 0) {
                // STRING: LEGOLAND 0x004bae60
                DBPrintf("CHEAT:Instant Appraisal\n");
                AppraisalDeadline = 1;
                // STRING: LEGOLAND 0x004bae50
            } else if (_memicmp(":WELOVELEGOLAND", &CheatKeyBuffer[5], 0xf) == 0) {
                // STRING: LEGOLAND 0x004bae3c
                DBPrintf("CHEAT:Win Level\n", lpConfig->level);
                EndLevel(1);
                // STRING: LEGOLAND 0x004bae30
            } else if (memcmp(":IMPROVISE", &CheatKeyBuffer[10], 10) == 0) {
                // STRING: LEGOLAND 0x004bae1c
                DBPrintf("CHEAT:Stop Script\n", lpConfig->level);
                SetScriptStopped(1);
                // STRING: LEGOLAND 0x004bae14
            } else if (_memicmp(":DIGGER", &CheatKeyBuffer[0xd], 7) == 0) {
                // STRING: LEGOLAND 0x004bae00
                DBPrintf("CHEAT:Set Switch 1\n");
                FUN_00460560(0);
                FUN_00460560(1);
                FUN_00460560(2);
                FUN_00460560(3);
                // STRING: LEGOLAND 0x004badf0
            } else if (memcmp(":SHOWCAPACITY", &CheatKeyBuffer[7], 0xd) == 0) {
                MapStats.show_capacity = 1;
                // STRING: LEGOLAND 0x004badd0
                DBPrintf("CHEAT: Capacity Calcs visible\n");
            }
        }
    }
    if ((KeyboardState[0xcb] & 0x80) != 0) {
        buffer->buttons = buffer->buttons | 8;
    }
    if ((KeyboardState[0xcd] & 0x80) != 0) {
        buffer->buttons = buffer->buttons | 0x10;
    }
    if ((KeyboardState[0xc8] & 0x80) != 0) {
        buffer->buttons = buffer->buttons | 0x20;
    }
    if ((KeyboardState[0xd0] & 0x80) != 0) {
        buffer->buttons = buffer->buttons | 0x40;
    }
    if ((KeyboardState[0x39] & 0x80) != 0) {
        buffer->buttons = buffer->buttons | 0x80;
    }
    if ((KeyboardState[0xf] & 0x80) != 0) {
        buffer->buttons = buffer->buttons | 0x100;
    }
    if ((KeyboardState[1] & 0x80) != 0) {
        buffer->buttons = buffer->buttons | 0x200;
    }
    if ((KeyboardState[0x1c] & 0x80) != 0) {
        buffer->buttons = buffer->buttons | 0x400;
    }
}

// FUNCTION: LEGOLAND 0x00474070
unsigned int IsLeftShiftDown(void) {
    return KeyboardState[0x2a] >> 7;
}

// FUNCTION: LEGOLAND 0x00474080
unsigned int IsRightShiftDown(void) {
    return KeyboardState[0x36] >> 7;
}

// FUNCTION: LEGOLAND 0x004740b0
LEGO_EXPORT char GetInputChar(void) {
    int i;
    int result;
    unsigned int prev;
    unsigned char state;
    int alt;

#ifdef LEGOLAND_PORT
    MiniwinWantText(); /* [library:input] the game waits for a name: the on-screen keyboard on a touch screen */
#endif
    result = 0;
    i = 0;
    do {
        prev = (char)PrevKeyState[i];
        state = KeyboardState[CheatKeyMap[i].flags];
        prev = (prev >> 7) & 1;
        PrevKeyState[i] = state;
        if ((state & 0x80) != 0 && prev == 0) {
#ifdef LEGOLAND_PORT
            alt = MiniwinCapsLock(); /* [library:input] Caps Lock from SDL3 */
#else
            alt = GetKeyState(0x14) & 1;
#endif
            if (IsLeftShiftDown() != 0 || IsRightShiftDown() != 0) {
                alt = (alt == 0);
            }
            result = CheatKeyMap[i].code;
            if (alt == 0) {
                result = tolower(result);
            }
        }
        i = i + 1;
    } while (i < 0x3b);
    return (char)result;
}

// FUNCTION: LEGOLAND 0x00474130
char ReadCheatKey(void) {
    int i;
    int result;
    unsigned char state;
    unsigned int prev;

    result = 0;
    i = 0;
    do {
        prev = (char)CheatKeyState[i];
        state = KeyboardState[CheatKeyMap[i].flags];
        prev = (prev >> 7) & 1;
        CheatKeyState[i] = state;
        if ((state & 0x80) != 0 && prev == 0) {
            result = CheatKeyMap[i].code;
        }
        i = i + 1;
    } while (i < 0x3b);
    if (result == -10) {
        result = 0x3a;
    }
    return (char)result;
}

// FUNCTION: LEGOLAND 0x00474190
void SaveInputState(void) {
    SaveGameWrite(&DAT_004baff8, 4);
    SaveGameWrite(&DAT_00668e34, 4);
    SaveGameWrite(&DAT_007fdd80, 16);
}

// FUNCTION: LEGOLAND 0x004741c0
void LoadInputState(void) {
    SaveGameRead(&DAT_004baff8, 4);
    SaveGameRead(&DAT_00668e34, 4);
    SaveGameRead(&DAT_007fdd80, 16);
}
