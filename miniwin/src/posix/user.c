/* [library:window] Windows' user interface functions off Windows. Rectangles and message boxes are done; windows,
 * their messages and dialogs come with ROADMAP.md phase 8 step 4.3 (the game's message loop on SDL's events) and
 * fail with ERROR_CALL_NOT_IMPLEMENTED until then. See posix.h. */
#include <SDL3/SDL.h>
#include <string.h>

#include "posix.h"

#define NOT_YET() PosixSetError(ERROR_CALL_NOT_IMPLEMENTED)

/* ---- rectangles ---- */

BOOL SetRect(LPRECT r, int left, int top, int right, int bottom) {
    if (r == NULL) {
        return FALSE;
    }
    r->left = left;
    r->top = top;
    r->right = right;
    r->bottom = bottom;
    return TRUE;
}

BOOL OffsetRect(LPRECT r, int dx, int dy) {
    if (r == NULL) {
        return FALSE;
    }
    r->left += dx;
    r->right += dx;
    r->top += dy;
    r->bottom += dy;
    return TRUE;
}

BOOL IsRectEmpty(const RECT *r) {
    return r == NULL || r->right <= r->left || r->bottom <= r->top;
}

BOOL PtInRect(const RECT *r, POINT p) {
    return p.x >= r->left && p.x < r->right && p.y >= r->top && p.y < r->bottom;
}

BOOL IntersectRect(LPRECT out, const RECT *a, const RECT *b) {
    RECT r;

    r.left = a->left > b->left ? a->left : b->left;
    r.top = a->top > b->top ? a->top : b->top;
    r.right = a->right < b->right ? a->right : b->right;
    r.bottom = a->bottom < b->bottom ? a->bottom : b->bottom;
    if (IsRectEmpty(a) || IsRectEmpty(b) || r.left >= r.right || r.top >= r.bottom) {
        SetRect(out, 0, 0, 0, 0);
        return FALSE;
    }
    *out = r;
    return TRUE;
}

BOOL AdjustWindowRect(LPRECT r, DWORD style, BOOL menu) {
    (void)r;
    (void)style;
    (void)menu;
    return TRUE; /* SDL's windows are sized by their inside: no frame to add */
}

/* ---- message boxes ---- */

int MessageBoxA(HWND owner, LPCSTR text, LPCSTR caption, UINT type) {
    SDL_MessageBoxButtonData buttons[3];
    SDL_MessageBoxData box;
    int count = 0;
    int pressed = IDOK;

    (void)owner;
    memset(&box, 0, sizeof(box));
    memset(buttons, 0, sizeof(buttons));
    switch (type & MB_TYPEMASK) {
    case MB_OKCANCEL:
        buttons[count++] = (SDL_MessageBoxButtonData){SDL_MESSAGEBOX_BUTTON_RETURNKEY_DEFAULT, IDOK, "OK"};
        buttons[count++] = (SDL_MessageBoxButtonData){SDL_MESSAGEBOX_BUTTON_ESCAPEKEY_DEFAULT, IDCANCEL, "Cancel"};
        break;
    case MB_YESNO:
        buttons[count++] = (SDL_MessageBoxButtonData){SDL_MESSAGEBOX_BUTTON_RETURNKEY_DEFAULT, IDYES, "Yes"};
        buttons[count++] = (SDL_MessageBoxButtonData){SDL_MESSAGEBOX_BUTTON_ESCAPEKEY_DEFAULT, IDNO, "No"};
        break;
    case MB_YESNOCANCEL:
        buttons[count++] = (SDL_MessageBoxButtonData){SDL_MESSAGEBOX_BUTTON_RETURNKEY_DEFAULT, IDYES, "Yes"};
        buttons[count++] = (SDL_MessageBoxButtonData){0, IDNO, "No"};
        buttons[count++] = (SDL_MessageBoxButtonData){SDL_MESSAGEBOX_BUTTON_ESCAPEKEY_DEFAULT, IDCANCEL, "Cancel"};
        break;
    default:
        buttons[count++] = (SDL_MessageBoxButtonData){
            SDL_MESSAGEBOX_BUTTON_RETURNKEY_DEFAULT | SDL_MESSAGEBOX_BUTTON_ESCAPEKEY_DEFAULT, IDOK, "OK"};
        break;
    }
    box.flags = (type & MB_ICONMASK) == MB_ICONERROR ? SDL_MESSAGEBOX_ERROR
        : (type & MB_ICONMASK) == MB_ICONEXCLAMATION ? SDL_MESSAGEBOX_WARNING
                                                     : SDL_MESSAGEBOX_INFORMATION;
    box.title = caption != NULL ? caption : "Error";
    box.message = text != NULL ? text : "";
    box.numbuttons = count;
    box.buttons = buttons;
    if (!SDL_ShowMessageBox(&box, &pressed)) {
        /* no display: the terminal */
        SDL_Log("%s: %s", box.title, box.message);
        pressed = buttons[0].buttonID;
    }
    return pressed;
}

/* ---- the keyboard ---- */

SHORT GetAsyncKeyState(int key) {
    const bool *state = SDL_GetKeyboardState(NULL);
    SDL_Scancode code = SDL_SCANCODE_UNKNOWN;

    if (key >= 'A' && key <= 'Z') {
        code = (SDL_Scancode)(SDL_SCANCODE_A + (key - 'A'));
    } else if (key >= '1' && key <= '9') {
        code = (SDL_Scancode)(SDL_SCANCODE_1 + (key - '1'));
    } else if (key == '0') {
        code = SDL_SCANCODE_0;
    } else if (key >= VK_F1 && key <= VK_F12) {
        code = (SDL_Scancode)(SDL_SCANCODE_F1 + (key - VK_F1));
    } else {
        switch (key) {
        case VK_SHIFT:
            return (SHORT)(state[SDL_SCANCODE_LSHIFT] || state[SDL_SCANCODE_RSHIFT] ? 0x8000 : 0);
        case VK_CONTROL:
            return (SHORT)(state[SDL_SCANCODE_LCTRL] || state[SDL_SCANCODE_RCTRL] ? 0x8000 : 0);
        case VK_MENU:
            return (SHORT)(state[SDL_SCANCODE_LALT] || state[SDL_SCANCODE_RALT] ? 0x8000 : 0);
        case VK_ESCAPE:
            code = SDL_SCANCODE_ESCAPE;
            break;
        case VK_RETURN:
            code = SDL_SCANCODE_RETURN;
            break;
        case VK_SPACE:
            code = SDL_SCANCODE_SPACE;
            break;
        case VK_LEFT:
            code = SDL_SCANCODE_LEFT;
            break;
        case VK_RIGHT:
            code = SDL_SCANCODE_RIGHT;
            break;
        case VK_UP:
            code = SDL_SCANCODE_UP;
            break;
        case VK_DOWN:
            code = SDL_SCANCODE_DOWN;
            break;
        case VK_LBUTTON:
            return (SHORT)((SDL_GetMouseState(NULL, NULL) & SDL_BUTTON_LMASK) != 0 ? 0x8000 : 0);
        case VK_RBUTTON:
            return (SHORT)((SDL_GetMouseState(NULL, NULL) & SDL_BUTTON_RMASK) != 0 ? 0x8000 : 0);
        default:
            return 0;
        }
    }
    return (SHORT)(state != NULL && state[code] ? 0x8000 : 0);
}

/* ---- windows, messages, dialogs: step 4.3 ---- */

ATOM RegisterClassExA(const WNDCLASSEXA *c) {
    (void)c;
    NOT_YET();
    return 0;
}

BOOL UnregisterClassA(LPCSTR name, HINSTANCE instance) {
    (void)name;
    (void)instance;
    return TRUE;
}

HWND CreateWindowExA(DWORD ex_style, LPCSTR class_name, LPCSTR title, DWORD style, int x, int y, int w, int h,
    HWND parent, HMENU menu, HINSTANCE instance, LPVOID param) {
    (void)ex_style;
    (void)class_name;
    (void)title;
    (void)style;
    (void)x;
    (void)y;
    (void)w;
    (void)h;
    (void)parent;
    (void)menu;
    (void)instance;
    (void)param;
    NOT_YET();
    return NULL;
}

BOOL DestroyWindow(HWND w) {
    (void)w;
    NOT_YET();
    return FALSE;
}

LRESULT DefWindowProcA(HWND w, UINT message, WPARAM wparam, LPARAM lparam) {
    (void)w;
    (void)message;
    (void)wparam;
    (void)lparam;
    return 0;
}

HWND FindWindowA(LPCSTR class_name, LPCSTR title) {
    (void)class_name;
    (void)title;
    return NULL;
}

HWND GetDesktopWindow(void) {
    return NULL;
}

BOOL ShowWindow(HWND w, int show) {
    (void)w;
    (void)show;
    return FALSE;
}

BOOL SetWindowPos(HWND w, HWND after, int x, int y, int cx, int cy, UINT flags) {
    (void)w;
    (void)after;
    (void)x;
    (void)y;
    (void)cx;
    (void)cy;
    (void)flags;
    NOT_YET();
    return FALSE;
}

BOOL IsIconic(HWND w) {
    (void)w;
    return FALSE;
}

BOOL SetForegroundWindow(HWND w) {
    (void)w;
    return FALSE;
}

HWND SetFocus(HWND w) {
    (void)w;
    return NULL;
}

BOOL EnableWindow(HWND w, BOOL enable) {
    (void)w;
    (void)enable;
    return FALSE;
}

BOOL GetMessageA(LPMSG msg, HWND w, UINT min, UINT max) {
    (void)w;
    (void)min;
    (void)max;
    memset(msg, 0, sizeof(*msg));
    msg->message = WM_QUIT;
    NOT_YET();
    return FALSE;
}

BOOL PeekMessageA(LPMSG msg, HWND w, UINT min, UINT max, UINT remove) {
    (void)msg;
    (void)w;
    (void)min;
    (void)max;
    (void)remove;
    return FALSE;
}

BOOL TranslateMessage(const MSG *msg) {
    (void)msg;
    return FALSE;
}

LRESULT DispatchMessageA(const MSG *msg) {
    (void)msg;
    return 0;
}

BOOL PostMessageA(HWND w, UINT message, WPARAM wparam, LPARAM lparam) {
    (void)w;
    (void)message;
    (void)wparam;
    (void)lparam;
    NOT_YET();
    return FALSE;
}

LRESULT SendMessageA(HWND w, UINT message, WPARAM wparam, LPARAM lparam) {
    (void)w;
    (void)message;
    (void)wparam;
    (void)lparam;
    return 0;
}

void PostQuitMessage(int code) {
    (void)code;
}

BOOL WaitMessage(void) {
    SDL_Delay(1);
    return TRUE;
}

INT_PTR DialogBoxParamA(HINSTANCE instance, LPCSTR name, HWND parent, DLGPROC proc, LPARAM param) {
    (void)instance;
    (void)name;
    (void)parent;
    (void)proc;
    (void)param;
    NOT_YET();
    return -1;
}

BOOL EndDialog(HWND w, INT_PTR result) {
    (void)w;
    (void)result;
    return FALSE;
}

BOOL IsDialogMessageA(HWND w, LPMSG msg) {
    (void)w;
    (void)msg;
    return FALSE;
}

HWND GetDlgItem(HWND w, int id) {
    (void)w;
    (void)id;
    return NULL;
}

BOOL CheckDlgButton(HWND w, int id, UINT check) {
    (void)w;
    (void)id;
    (void)check;
    return FALSE;
}

UINT IsDlgButtonChecked(HWND w, int id) {
    (void)w;
    (void)id;
    return BST_UNCHECKED;
}

HCURSOR LoadCursorA(HINSTANCE instance, LPCSTR name) {
    (void)instance;
    (void)name;
    return NULL;
}

HICON LoadIconA(HINSTANCE instance, LPCSTR name) {
    (void)instance;
    (void)name;
    return NULL;
}

HCURSOR SetCursor(HCURSOR cursor) {
    (void)cursor;
    return NULL;
}

int ShowCursor(BOOL show) {
    return show ? 0 : -1;
}

int GetSystemMetrics(int index) {
    (void)index;
    return 0;
}

BOOL SystemParametersInfoA(UINT action, UINT param, PVOID value, UINT ini) {
    (void)action;
    (void)param;
    (void)value;
    (void)ini;
    NOT_YET();
    return FALSE;
}

BOOL EnumDisplaySettingsA(LPCSTR device, DWORD mode, DEVMODEA *out) {
    (void)device;
    (void)mode;
    (void)out;
    return FALSE;
}

HDC GetDC(HWND w) {
    (void)w;
    NOT_YET();
    return NULL;
}

int ReleaseDC(HWND w, HDC dc) {
    (void)w;
    (void)dc;
    return 0;
}

int FillRect(HDC dc, const RECT *r, HBRUSH brush) {
    (void)dc;
    (void)r;
    (void)brush;
    return 0;
}

int DrawTextA(HDC dc, LPCSTR text, int count, LPRECT r, UINT format) {
    (void)dc;
    (void)text;
    (void)count;
    (void)r;
    (void)format;
    return 0;
}
