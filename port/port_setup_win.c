/* [library:filesystem] The first-run setup window (Windows): when the game's data isn't found, it asks for the
 * LEGOLAND CD (or its ISO image, or a folder) and builds the data folder next to the exe with port_setup.c. */
#include <windows.h>
#include <commctrl.h>
#include <commdlg.h>
#include <shlobj.h>
#include <stdio.h>
#include <string.h>

#include "port_setup.h"
#include "port_setup_win.h"
#include "port_trace.h"

#define ID_INSTALL 201
#define ID_CHOOSE 202
#define ID_SOURCE 203
#define ID_PROGRESS 204
#define ID_FILE 205
#define ID_CHOOSE_ISO 206
#define TIMER_PROGRESS 1
#define NEEDED_MB 450 /* the data folder is about 400 MB */

static HWND window;
static HWND source_text;
static HWND install_button;
static HWND choose_button;
static HWND iso_button;
static HWND quit_button;
static HWND progress_bar;
static HWND file_text;
static int dpi = 96;
static int finished; /* 1 installed, -1 quit */
static char source[MAX_PATH];

/* the install, on its own thread */
static CRITICAL_SECTION lock;
static HANDLE worker;
static volatile LONG cancel;
static unsigned long long progress_done, progress_total;
static char progress_file[MAX_PATH];
static int result;
static char error[512];

static int S(int v) {
    return MulDiv(v, dpi, 96);
}

static void ShowSource(void) {
    char text[MAX_PATH + 64];

    if (source[0] == '\0') {
        SetWindowTextA(source_text, "No LEGOLAND CD found. Insert it, or choose its ISO image or a folder.");
    } else {
        int kind = PortSetupKind(source);

        snprintf(text, sizeof(text), "Install from: %s (%s)", source,
            kind == PORT_SETUP_CD          ? "the LEGOLAND CD"
                : kind == PORT_SETUP_IMAGE ? "an image of the LEGOLAND CD"
                                           : "an installed LEGOLAND");
        SetWindowTextA(source_text, text);
    }
    EnableWindow(install_button, source[0] != '\0');
}

/* a CD drive (a mounted ISO is one too) with the LEGOLAND CD */
static void FindCd(void) {
    DWORD drives = GetLogicalDrives();
    char root[4] = "A:\\";
    int i;

    source[0] = '\0';
    for (i = 0; i < 26; i++) {
        if (drives & (1u << i)) {
            root[0] = (char)('A' + i);
            if (GetDriveTypeA(root) == DRIVE_CDROM && PortSetupKind(root) == PORT_SETUP_CD) {
                strcpy(source, root);
                return;
            }
        }
    }
}

static void ChooseFolder(void) {
    BROWSEINFOA bi;
    LPITEMIDLIST item;
    char path[MAX_PATH];

    memset(&bi, 0, sizeof(bi));
    bi.hwndOwner = window;
    bi.lpszTitle = "Choose the LEGOLAND CD, a copy of it, or a folder with LEGOLAND installed:";
    bi.ulFlags = BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE;
    item = SHBrowseForFolderA(&bi);
    if (item == NULL) {
        return;
    }
    if (SHGetPathFromIDListA(item, path)) {
        if (PortSetupKind(path) == PORT_SETUP_NONE) {
            MessageBoxA(window,
                "That isn't the LEGOLAND CD or a LEGOLAND folder.\n\nThe CD has main.z, Legoland.res and the "
                "folder Speech; an installed LEGOLAND has IMusic, FMV, speech and volumes. For an ISO image, use "
                "Choose ISO.",
                "LEGOLAND", MB_OK | MB_ICONWARNING);
        } else {
            strcpy(source, path);
        }
    }
    CoTaskMemFree(item);
    ShowSource();
}

/* an image file of the CD, read as it is (no need to mount it) */
static void ChooseIso(void) {
    OPENFILENAMEA ofn;
    char path[MAX_PATH] = "";

    memset(&ofn, 0, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = window;
    ofn.lpstrFilter = "CD images (*.iso, *.bin)\0*.iso;*.bin\0All files (*.*)\0*.*\0";
    ofn.lpstrFile = path;
    ofn.nMaxFile = sizeof(path);
    ofn.lpstrTitle = "Choose the LEGOLAND CD's ISO image";
    ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY | OFN_NOCHANGEDIR;
    if (!GetOpenFileNameA(&ofn)) {
        return;
    }
    if (PortSetupKind(path) != PORT_SETUP_IMAGE) {
        MessageBoxA(window,
            "That isn't an image of the LEGOLAND CD.\n\nThe CD's image is an .iso (or .bin) file with main.z, "
            "Legoland.res and the folder Speech in it.",
            "LEGOLAND", MB_OK | MB_ICONWARNING);
    } else {
        strcpy(source, path);
    }
    ShowSource();
}

static void Report(void *ctx, unsigned long long done, unsigned long long total, const char *file) {
    (void)ctx;
    EnterCriticalSection(&lock);
    progress_done = done;
    progress_total = total;
    strncpy(progress_file, file, sizeof(progress_file) - 1);
    LeaveCriticalSection(&lock);
}

static int Cancelled(void *ctx) {
    (void)ctx;
    return cancel != 0;
}

static DWORD WINAPI Work(LPVOID param) {
    PortSetupProgress progress = {Report, Cancelled, NULL};
    char target[MAX_PATH];

    (void)param;
    strncpy(target, PortSetupExeFolder(), sizeof(target) - 1);
    result = PortSetupInstall(source, target, &progress, error, sizeof(error));
    return 0;
}

/* the exe's folder can take the data: writable, with room */
static int TargetUsable(void) {
    char path[MAX_PATH + 32];
    char text[MAX_PATH + 256];
    ULARGE_INTEGER free_bytes;
    HANDLE h;

    snprintf(path, sizeof(path), "%ssetup-test.tmp", PortSetupExeFolder());
    h = CreateFileA(path, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_FLAG_DELETE_ON_CLOSE, NULL);
    if (h == INVALID_HANDLE_VALUE) {
        snprintf(text, sizeof(text),
            "The game's files go next to this program, in\n%s\nbut that folder can't be written to.\n\nMove the "
            "LEGOLAND folder somewhere you can write, such as Documents, and start it again.",
            PortSetupExeFolder());
        MessageBoxA(window, text, "LEGOLAND", MB_OK | MB_ICONWARNING);
        return 0;
    }
    CloseHandle(h);
    if (GetDiskFreeSpaceExA(PortSetupExeFolder(), &free_bytes, NULL, NULL) &&
        free_bytes.QuadPart < (ULONGLONG)NEEDED_MB << 20) {
        snprintf(text, sizeof(text), "The game's files need about %d MB, and the disk with\n%s\nhas less free space.",
            NEEDED_MB, PortSetupExeFolder());
        MessageBoxA(window, text, "LEGOLAND", MB_OK | MB_ICONWARNING);
        return 0;
    }
    return 1;
}

static void StartInstall(void) {
    if (!TargetUsable()) {
        return;
    }
    PortTrace("setup: installing from %s into %s", source, PortSetupExeFolder());
    cancel = 0;
    progress_done = progress_total = 0;
    progress_file[0] = '\0';
    EnableWindow(install_button, FALSE);
    EnableWindow(choose_button, FALSE);
    EnableWindow(iso_button, FALSE);
    SetWindowTextA(quit_button, "Cancel");
    ShowWindow(progress_bar, SW_SHOW);
    ShowWindow(file_text, SW_SHOW);
    SetWindowTextA(source_text, "Copying the game's files...");
    worker = CreateThread(NULL, 0, Work, NULL, 0, NULL);
    SetTimer(window, TIMER_PROGRESS, 100, NULL);
}

static void Poll(void) {
    char text[MAX_PATH + 32];
    unsigned long long done, total;

    EnterCriticalSection(&lock);
    done = progress_done;
    total = progress_total;
    snprintf(text, sizeof(text), "%s", progress_file);
    LeaveCriticalSection(&lock);
    SendMessageA(progress_bar, PBM_SETPOS, total ? (WPARAM)(done * 1000 / total) : 0, 0);
    SetWindowTextA(file_text, text);
    if (WaitForSingleObject(worker, 0) != WAIT_OBJECT_0) {
        return;
    }
    KillTimer(window, TIMER_PROGRESS);
    CloseHandle(worker);
    worker = NULL;
    if (result) {
        PortTrace("setup: done");
        finished = 1;
        return;
    }
    PortTrace("setup: failed: %s", error);
    if (!cancel) {
        snprintf(text, sizeof(text), "The game's files couldn't be copied.\n\n%s", error);
        MessageBoxA(window, text, "LEGOLAND", MB_OK | MB_ICONWARNING);
    }
    ShowWindow(progress_bar, SW_HIDE);
    ShowWindow(file_text, SW_HIDE);
    SetWindowTextA(quit_button, "Quit");
    EnableWindow(choose_button, TRUE);
    EnableWindow(iso_button, TRUE);
    ShowSource();
}

static LRESULT CALLBACK SetupProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
    switch (msg) {
    case WM_COMMAND:
        switch (LOWORD(wparam)) {
        case ID_INSTALL:
            StartInstall();
            return 0;
        case ID_CHOOSE:
            ChooseFolder();
            return 0;
        case ID_CHOOSE_ISO:
            ChooseIso();
            return 0;
        case IDCANCEL:
            if (worker != NULL) {
                cancel = 1;
            } else {
                finished = -1;
            }
            return 0;
        }
        break;
    case WM_TIMER:
        if (wparam == TIMER_PROGRESS) {
            Poll();
        }
        return 0;
    case WM_DEVICECHANGE:
        /* a CD inserted or an ISO mounted while the window is open (a chosen image stays chosen) */
        if (worker == NULL && PortSetupKind(source) != PORT_SETUP_IMAGE) {
            FindCd();
            ShowSource();
        }
        break;
    case WM_CLOSE:
        if (worker != NULL) {
            cancel = 1;
        } else {
            finished = -1;
        }
        return 0;
    }
    return DefWindowProcA(hwnd, msg, wparam, lparam);
}

static HWND Control(const char *cls, const char *text, DWORD style, int x, int y, int w, int h, int id, HFONT font) {
    HWND hwnd = CreateWindowExA(0, cls, text, WS_CHILD | WS_VISIBLE | style, S(x), S(y), S(w), S(h), window,
        (HMENU)(INT_PTR)id, GetModuleHandleA(NULL), NULL);
    SendMessageA(hwnd, WM_SETFONT, (WPARAM)font, TRUE);
    return hwnd;
}

int PortSetupRun(HINSTANCE instance) {
    INITCOMMONCONTROLSEX icc = {sizeof(icc), ICC_PROGRESS_CLASS};
    NONCLIENTMETRICSA metrics;
    WNDCLASSEXA wc;
    HFONT font;
    RECT frame;
    MSG msg;
    DWORD style = WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX;
    HDC screen = GetDC(NULL);

    if (screen != NULL) {
        dpi = GetDeviceCaps(screen, LOGPIXELSY);
        ReleaseDC(NULL, screen);
    }
    InitCommonControlsEx(&icc);
    CoInitializeEx(NULL, COINIT_APARTMENTTHREADED); /* the folder dialog */
    InitializeCriticalSection(&lock);
    memset(&metrics, 0, sizeof(metrics));
    metrics.cbSize = sizeof(metrics);
    SystemParametersInfoA(SPI_GETNONCLIENTMETRICS, sizeof(metrics), &metrics, 0);
    font = CreateFontIndirectA(&metrics.lfMessageFont);

    memset(&wc, 0, sizeof(wc));
    wc.cbSize = sizeof(wc);
    wc.lpfnWndProc = SetupProc;
    wc.hInstance = instance;
    wc.hIcon = LoadIconA(instance, (LPCSTR)0x65);
    wc.hIconSm = wc.hIcon;
    wc.hCursor = LoadCursorA(NULL, (LPCSTR)IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
    wc.lpszClassName = "LEGOLANDSETUP";
    RegisterClassExA(&wc);

    SetRect(&frame, 0, 0, S(420), S(220));
    AdjustWindowRect(&frame, style, FALSE);
    window = CreateWindowExA(WS_EX_DLGMODALFRAME, "LEGOLANDSETUP", "LEGOLAND", style, CW_USEDEFAULT, CW_USEDEFAULT,
        frame.right - frame.left, frame.bottom - frame.top, NULL, NULL, instance, NULL);
    if (window == NULL) {
        DeleteObject(font);
        return 0;
    }
    {
        RECT work;
        if (SystemParametersInfoA(SPI_GETWORKAREA, 0, &work, 0)) {
            SetWindowPos(window, NULL, work.left + (work.right - work.left - (frame.right - frame.left)) / 2,
                work.top + (work.bottom - work.top - (frame.bottom - frame.top)) / 2, 0, 0,
                SWP_NOSIZE | SWP_NOZORDER);
        }
    }
    Control("STATIC",
        "LEGOLAND needs its game files from the CD. Insert the LEGOLAND CD, or choose its ISO image (no need "
        "to mount it), or a folder with a copy of the CD or with LEGOLAND installed.\n\nThe files (about "
        "400 MB) are copied next to this program, once.",
        0, 16, 14, 388, 80, -1, font);
    source_text = Control("STATIC", "", SS_PATHELLIPSIS, 16, 102, 388, 18, ID_SOURCE, font);
    progress_bar = Control(PROGRESS_CLASSA, "", 0, 16, 126, 388, 18, ID_PROGRESS, font);
    SendMessageA(progress_bar, PBM_SETRANGE32, 0, 1000);
    file_text = Control("STATIC", "", SS_PATHELLIPSIS, 16, 148, 388, 18, ID_FILE, font);
    ShowWindow(progress_bar, SW_HIDE);
    ShowWindow(file_text, SW_HIDE);
    install_button = Control("BUTTON", "Install", BS_DEFPUSHBUTTON | WS_GROUP | WS_TABSTOP, 44, 178, 86, 28,
        ID_INSTALL, font);
    iso_button = Control("BUTTON", "Choose ISO...", WS_TABSTOP, 136, 178, 86, 28, ID_CHOOSE_ISO, font);
    choose_button = Control("BUTTON", "Choose folder...", WS_TABSTOP, 228, 178, 86, 28, ID_CHOOSE, font);
    quit_button = Control("BUTTON", "Quit", WS_TABSTOP, 320, 178, 84, 28, IDCANCEL, font);
    FindCd();
    ShowSource();
    ShowWindow(window, SW_SHOW);
    SetForegroundWindow(window);

    finished = 0;
    while (finished == 0 && GetMessageA(&msg, NULL, 0, 0) > 0) {
        if (!IsDialogMessageA(window, &msg)) {
            TranslateMessage(&msg);
            DispatchMessageA(&msg);
        }
    }
    DestroyWindow(window);
    window = NULL;
    UnregisterClassA("LEGOLANDSETUP", instance);
    DeleteObject(font);
    DeleteCriticalSection(&lock);
    CoUninitialize();
    return finished == 1 && PortSetupFindData();
}
