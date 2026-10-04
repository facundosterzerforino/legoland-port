/* [library:config] The launcher: resolution and window mode before the game starts. See port_launcher.h. */
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "port_display.h"
#include "port_launcher.h"
#include "port_trace.h"

#define ID_RESOLUTION 101
#define ID_DESKTOP 102
#define ID_CHANGE 103
#define ID_WINDOWED 104
#define ID_DONT_ASK 105

/* the three ways to show the game */
enum {
    MODE_DESKTOP,
    MODE_CHANGE,
    MODE_WINDOW
};
static const char *const mode_names[] = {"desktop", "change", "window"};

struct Resolution {
    int width;
    int height;
    const char *note;
};

/* 4:3 sizes. The game's frame is 640x480; the exact multiples are the sharpest. */
static const struct Resolution resolutions[] = {
    {640, 480, "original"},
    {800, 600, NULL},
    {1024, 768, NULL},
    {1152, 864, NULL},
    {1280, 960, "2x, sharpest"},
    {1400, 1050, NULL},
    {1600, 1200, NULL},
    {1920, 1440, "3x, sharpest"},
    {2048, 1536, NULL},
    {2560, 1920, "4x, sharpest"},
};
#define RESOLUTION_COUNT ((int)(sizeof(resolutions) / sizeof(resolutions[0])))

static char ini_path[MAX_PATH];
static HWND launcher;
static HWND combo;
static int done;
static int accepted;
static int chosen_width;
static int chosen_height;
static int chosen_mode;
static int listed[RESOLUTION_COUNT]; /* combo item -> resolutions[] index */

/* Fullscreen: the display offers this size (in any colour depth; Windows adds 16-bit itself). */
static int DisplayHasMode(int width, int height) {
    DEVMODEA mode;
    DWORD i;

    if (width == PORT_GAME_WIDTH && height == PORT_GAME_HEIGHT) {
        return 1;
    }
    memset(&mode, 0, sizeof(mode));
    mode.dmSize = sizeof(mode);
    for (i = 0; EnumDisplaySettingsA(NULL, i, &mode); i++) {
        if ((int)mode.dmPelsWidth == width && (int)mode.dmPelsHeight == height) {
            return 1;
        }
    }
    return 0;
}

/* Windowed: the window fits on the desktop. */
static int WindowFits(int width, int height) {
    RECT work;
    RECT frame;

    if (width == PORT_GAME_WIDTH && height == PORT_GAME_HEIGHT) {
        return 1;
    }
    SetRect(&frame, 0, 0, width, height);
    AdjustWindowRect(&frame, WS_OVERLAPPEDWINDOW, FALSE);
    if (!SystemParametersInfoA(SPI_GETWORKAREA, 0, &work, 0)) {
        return 1;
    }
    return frame.right - frame.left <= work.right - work.left && frame.bottom - frame.top <= work.bottom - work.top;
}

static int Available(int i, int mode) {
    return mode == MODE_WINDOW ? WindowFits(resolutions[i].width, resolutions[i].height)
                               : DisplayHasMode(resolutions[i].width, resolutions[i].height);
}

static void FillCombo(int mode) {
    int count = 0;
    int select = -1;
    int fallback = 0;
    int i;

    SendMessageA(combo, CB_RESETCONTENT, 0, 0);
    if (mode == MODE_DESKTOP) {
        /* the picture fills the screen (4:3, centred); no resolution to choose */
        char text[96];
        RECT pic;
        PortDisplayDestRect(GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN), &pic);
        sprintf(text, "%d x %d  (fills the screen)", (int)(pic.right - pic.left), (int)(pic.bottom - pic.top));
        SendMessageA(combo, CB_ADDSTRING, 0, (LPARAM)text);
        SendMessageA(combo, CB_SETCURSEL, 0, 0);
        EnableWindow(combo, FALSE);
        return;
    }
    EnableWindow(combo, TRUE);
    for (i = 0; i < RESOLUTION_COUNT; i++) {
        char text[64];

        if (!Available(i, mode)) {
            continue;
        }
        if (resolutions[i].note != NULL) {
            sprintf(text, "%d x %d  (%s)", resolutions[i].width, resolutions[i].height, resolutions[i].note);
        } else {
            sprintf(text, "%d x %d", resolutions[i].width, resolutions[i].height);
        }
        SendMessageA(combo, CB_ADDSTRING, 0, (LPARAM)text);
        listed[count] = i;
        if (resolutions[i].width == chosen_width && resolutions[i].height == chosen_height) {
            select = count;
        }
        if (resolutions[i].width <= chosen_width) {
            fallback = count; /* the largest listed size not above the choice */
        }
        count++;
    }
    SendMessageA(combo, CB_SETCURSEL, select >= 0 ? select : fallback, 0);
}

static void ReadChoice(void) {
    int item = (int)SendMessageA(combo, CB_GETCURSEL, 0, 0);

    if (chosen_mode != MODE_DESKTOP && item >= 0 && item < RESOLUTION_COUNT) {
        chosen_width = resolutions[listed[item]].width;
        chosen_height = resolutions[listed[item]].height;
    }
    chosen_mode = IsDlgButtonChecked(launcher, ID_WINDOWED) == BST_CHECKED ? MODE_WINDOW
        : IsDlgButtonChecked(launcher, ID_CHANGE) == BST_CHECKED           ? MODE_CHANGE
                                                                           : MODE_DESKTOP;
}

static LRESULT CALLBACK LauncherProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
    switch (msg) {
    case WM_COMMAND:
        switch (LOWORD(wparam)) {
        case ID_DESKTOP:
        case ID_CHANGE:
        case ID_WINDOWED:
            ReadChoice();
            FillCombo(chosen_mode);
            return 0;
        case IDOK:
            ReadChoice();
            accepted = 1;
            done = 1;
            return 0;
        case IDCANCEL:
            done = 1;
            return 0;
        }
        break;
    case WM_CLOSE:
        done = 1;
        return 0;
    }
    return DefWindowProcA(hwnd, msg, wparam, lparam);
}

static int dpi = 96;

/* layout in 96-DPI units, scaled to the screen's DPI (the process is DPI-aware) */
static int S(int v) {
    return MulDiv(v, dpi, 96);
}

static HWND Control(const char *cls, const char *text, DWORD style, int x, int y, int w, int h, int id, HFONT font) {
    HWND hwnd = CreateWindowExA(0, cls, text, WS_CHILD | WS_VISIBLE | style, S(x), S(y), S(w), S(h), launcher,
        (HMENU)(INT_PTR)id, GetModuleHandleA(NULL), NULL);
    SendMessageA(hwnd, WM_SETFONT, (WPARAM)font, TRUE);
    return hwnd;
}

/* Shows the launcher; returns 1 for Play, 0 for Quit. */
static int ShowLauncher(HINSTANCE instance, int *dont_ask) {
    NONCLIENTMETRICSA metrics;
    WNDCLASSEXA wc;
    HFONT font;
    RECT frame;
    MSG msg;
    DWORD style = WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX;
    int cw;
    int ch;
    HDC screen = GetDC(NULL);

    if (screen != NULL) {
        dpi = GetDeviceCaps(screen, LOGPIXELSY);
        ReleaseDC(NULL, screen);
    }
    cw = S(330);
    ch = S(268);

    memset(&metrics, 0, sizeof(metrics));
    metrics.cbSize = sizeof(metrics);
    SystemParametersInfoA(SPI_GETNONCLIENTMETRICS, sizeof(metrics), &metrics, 0);
    font = CreateFontIndirectA(&metrics.lfMessageFont);

    memset(&wc, 0, sizeof(wc));
    wc.cbSize = sizeof(wc);
    wc.lpfnWndProc = LauncherProc;
    wc.hInstance = instance;
    wc.hIcon = LoadIconA(instance, (LPCSTR)0x65);
    wc.hIconSm = wc.hIcon;
    wc.hCursor = LoadCursorA(NULL, (LPCSTR)IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
    wc.lpszClassName = "LEGOLANDLAUNCHER";
    RegisterClassExA(&wc);

    SetRect(&frame, 0, 0, cw, ch);
    AdjustWindowRect(&frame, style, FALSE);
    launcher = CreateWindowExA(WS_EX_DLGMODALFRAME, "LEGOLANDLAUNCHER", "LEGOLAND", style, CW_USEDEFAULT,
        CW_USEDEFAULT, frame.right - frame.left, frame.bottom - frame.top, NULL, NULL, instance, NULL);
    if (launcher == NULL) {
        DeleteObject(font);
        return 1; /* no launcher: play with the defaults */
    }
    {
        /* centred on the screen */
        RECT work;
        if (SystemParametersInfoA(SPI_GETWORKAREA, 0, &work, 0)) {
            SetWindowPos(launcher, NULL, work.left + (work.right - work.left - (frame.right - frame.left)) / 2,
                work.top + (work.bottom - work.top - (frame.bottom - frame.top)) / 2, 0, 0,
                SWP_NOSIZE | SWP_NOZORDER);
        }
    }

    Control("STATIC", "Show the game:", 0, 16, 14, 298, 18, -1, font);
    Control("BUTTON", "Full screen (keep the desktop's resolution)", BS_AUTORADIOBUTTON | WS_GROUP | WS_TABSTOP, 16,
        34, 298, 20, ID_DESKTOP, font);
    Control("BUTTON", "Full screen (change the resolution)", BS_AUTORADIOBUTTON, 16, 56, 298, 20, ID_CHANGE, font);
    Control("BUTTON", "In a window", BS_AUTORADIOBUTTON, 16, 78, 298, 20, ID_WINDOWED, font);
    Control("STATIC", "Resolution:", 0, 16, 110, 298, 18, -1, font);
    combo = Control("COMBOBOX", "", CBS_DROPDOWNLIST | WS_VSCROLL | WS_TABSTOP | WS_GROUP, 16, 130, 298, 300,
        ID_RESOLUTION, font);
    Control("BUTTON", "Don't show this again", BS_AUTOCHECKBOX | WS_GROUP | WS_TABSTOP, 16, 170, 298, 20, ID_DONT_ASK,
        font);
    Control("STATIC", "(hold Shift while starting the game to see it again)", 0, 34, 190, 290, 18, -1, font);
    Control("BUTTON", "Play", BS_DEFPUSHBUTTON | WS_GROUP | WS_TABSTOP, 136, 226, 86, 28, IDOK, font);
    Control("BUTTON", "Quit", WS_TABSTOP, 228, 226, 86, 28, IDCANCEL, font);

    CheckDlgButton(launcher,
        chosen_mode == MODE_WINDOW ? ID_WINDOWED : chosen_mode == MODE_CHANGE ? ID_CHANGE
                                                                              : ID_DESKTOP,
        BST_CHECKED);
    FillCombo(chosen_mode);
    ShowWindow(launcher, SW_SHOW);
    SetForegroundWindow(launcher);
    SetFocus(GetDlgItem(launcher, IDOK));

    done = 0;
    accepted = 0;
    while (!done && GetMessageA(&msg, NULL, 0, 0) > 0) {
        if (!IsDialogMessageA(launcher, &msg)) {
            TranslateMessage(&msg);
            DispatchMessageA(&msg);
        }
    }
    *dont_ask = IsDlgButtonChecked(launcher, ID_DONT_ASK) == BST_CHECKED;
    DestroyWindow(launcher);
    launcher = NULL;
    UnregisterClassA("LEGOLANDLAUNCHER", instance);
    DeleteObject(font);
    return accepted;
}

static int ParseResolution(const char *cmdline, int *width, int *height) {
    const char *p = strstr(cmdline, "-res ");
    int w;
    int h;

    if (p == NULL || sscanf(p + 5, "%dx%d", &w, &h) != 2 || w < PORT_GAME_WIDTH || h < PORT_GAME_HEIGHT || w > 8192 ||
        h > 8192) {
        return 0;
    }
    *width = w;
    *height = h;
    return 1;
}

static void WriteInt(const char *key, int value) {
    char text[16];
    sprintf(text, "%d", value);
    WritePrivateProfileStringA("display", key, text, ini_path);
}

int PortLauncherRun(HINSTANCE instance, const char *cmdline) {
    int from_cmdline;
    int ask;
    int dont_ask = 0;
    char *dot;

    GetModuleFileNameA(NULL, ini_path, sizeof(ini_path) - 4);
    dot = strrchr(ini_path, '.');
    strcpy(dot != NULL && strchr(dot, '\\') == NULL ? dot : ini_path + strlen(ini_path), ".ini");

    /* saved choice; the first time full screen at the desktop's resolution (an older .ini has only "windowed") */
    chosen_width = GetPrivateProfileIntA("display", "width", 1024, ini_path);
    chosen_height = GetPrivateProfileIntA("display", "height", 768, ini_path);
    {
        char mode[16];
        int i;
        chosen_mode = GetPrivateProfileIntA("display", "windowed", 0, ini_path) != 0 ? MODE_WINDOW : MODE_DESKTOP;
        GetPrivateProfileStringA("display", "mode", "", mode, sizeof(mode), ini_path);
        for (i = 0; i < 3; i++) {
            if (strcmp(mode, mode_names[i]) == 0) {
                chosen_mode = i;
            }
        }
    }
    ask = GetPrivateProfileIntA("display", "show_launcher", 1, ini_path) != 0;

    from_cmdline = ParseResolution(cmdline, &chosen_width, &chosen_height);
    if (strstr(cmdline, "WINDEBUG") != NULL || strstr(cmdline, "-windowed") != NULL) {
        chosen_mode = MODE_WINDOW;
        from_cmdline = 1;
    } else if (from_cmdline) {
        chosen_mode = MODE_CHANGE; /* -res WxH: full screen in that mode */
    } else if (strstr(cmdline, "-fullscreen") != NULL) {
        chosen_mode = MODE_DESKTOP;
        from_cmdline = 1;
    }
    if (strstr(cmdline, "-launcher") != NULL || (GetAsyncKeyState(VK_SHIFT) & 0x8000) != 0) {
        ask = 1;
    } else if (from_cmdline) {
        ask = 0;
    }

    if (ask) {
        if (!ShowLauncher(instance, &dont_ask)) {
            PortTrace("launcher: quit");
            return 0;
        }
        WriteInt("width", chosen_width);
        WriteInt("height", chosen_height);
        WritePrivateProfileStringA("display", "mode", mode_names[chosen_mode], ini_path);
        WritePrivateProfileStringA("display", "windowed", NULL, ini_path); /* replaced by "mode" */
        WriteInt("show_launcher", !dont_ask);
    }
    if (chosen_mode == MODE_DESKTOP) {
        /* a borderless window over the whole (primary) screen; the display mode isn't touched */
        chosen_width = GetSystemMetrics(SM_CXSCREEN);
        chosen_height = GetSystemMetrics(SM_CYSCREEN);
    } else if (!(chosen_mode == MODE_WINDOW ? WindowFits(chosen_width, chosen_height)
                                            : DisplayHasMode(chosen_width, chosen_height))) {
        PortTrace("launcher: %dx%d isn't available %s, using 640x480", chosen_width, chosen_height,
            chosen_mode == MODE_WINDOW ? "in a window" : "on this display");
        chosen_width = PORT_GAME_WIDTH;
        chosen_height = PORT_GAME_HEIGHT;
    }
    PortDisplayWidth = chosen_width;
    PortDisplayHeight = chosen_height;
    PortDisplayWindowed = chosen_mode != MODE_CHANGE;
    PortDisplayBorderless = chosen_mode == MODE_DESKTOP;
    {
        static const char *const described[] = {"full screen at the desktop's resolution",
            "full screen, display mode changed", "windowed"};
        HDC screen = GetDC(NULL);
        PortTrace("launcher: %dx%d %s (screen %d dpi)", PortDisplayWidth, PortDisplayHeight, described[chosen_mode],
            screen != NULL ? GetDeviceCaps(screen, LOGPIXELSY) : 0);
        if (screen != NULL) {
            ReleaseDC(NULL, screen);
        }
    }
    return 1;
}
