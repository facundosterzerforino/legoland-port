/* [library:video] GDI (text on the game's surfaces, the certificate's picture), printing and COM off Windows. GDI
 * text comes with ROADMAP.md phase 8 step 4.4 (stb_truetype and Lego.TTF); until then these fail as on a system
 * without the device. There are no printers here. See posix.h. */
#include <string.h>

#include "posix.h"

#define NOT_YET() PosixSetError(ERROR_CALL_NOT_IMPLEMENTED)

/* ---- GDI: step 4.4 ---- */

int AddFontResourceA(LPCSTR name) {
    (void)name;
    NOT_YET();
    return 0;
}

BOOL RemoveFontResourceA(LPCSTR name) {
    (void)name;
    return FALSE;
}

HDC CreateCompatibleDC(HDC dc) {
    (void)dc;
    NOT_YET();
    return NULL;
}

HDC CreateDCA(LPCSTR driver, LPCSTR device, LPCSTR port, const DEVMODEA *mode) {
    (void)driver;
    (void)device;
    (void)port;
    (void)mode;
    NOT_YET();
    return NULL;
}

BOOL DeleteDC(HDC dc) {
    (void)dc;
    return FALSE;
}

HBITMAP CreateDIBitmap(HDC dc, const BITMAPINFOHEADER *header, DWORD init, const void *bits, const BITMAPINFO *info,
    UINT usage) {
    (void)dc;
    (void)header;
    (void)init;
    (void)bits;
    (void)info;
    (void)usage;
    NOT_YET();
    return NULL;
}

HFONT CreateFontIndirectA(const LOGFONTA *font) {
    (void)font;
    NOT_YET();
    return NULL;
}

HPEN CreatePen(int style, int width, COLORREF color) {
    (void)style;
    (void)width;
    (void)color;
    NOT_YET();
    return NULL;
}

HBRUSH CreateSolidBrush(COLORREF color) {
    (void)color;
    NOT_YET();
    return NULL;
}

HRGN CreateRectRgn(int left, int top, int right, int bottom) {
    (void)left;
    (void)top;
    (void)right;
    (void)bottom;
    NOT_YET();
    return NULL;
}

HRGN CreateRectRgnIndirect(const RECT *r) {
    (void)r;
    NOT_YET();
    return NULL;
}

BOOL DeleteObject(HGDIOBJ object) {
    (void)object;
    return FALSE;
}

HGDIOBJ GetStockObject(int index) {
    (void)index;
    return NULL;
}

HGDIOBJ SelectObject(HDC dc, HGDIOBJ object) {
    (void)dc;
    (void)object;
    return NULL;
}

int GetDeviceCaps(HDC dc, int index) {
    (void)dc;
    (void)index;
    return 0;
}

COLORREF GetNearestColor(HDC dc, COLORREF color) {
    (void)dc;
    return color;
}

COLORREF SetBkColor(HDC dc, COLORREF color) {
    (void)dc;
    (void)color;
    return CLR_INVALID;
}

int SetBkMode(HDC dc, int mode) {
    (void)dc;
    (void)mode;
    return 0;
}

UINT SetTextAlign(HDC dc, UINT align) {
    (void)dc;
    (void)align;
    return GDI_ERROR;
}

COLORREF SetTextColor(HDC dc, COLORREF color) {
    (void)dc;
    (void)color;
    return CLR_INVALID;
}

BOOL TextOutA(HDC dc, int x, int y, LPCSTR text, int count) {
    (void)dc;
    (void)x;
    (void)y;
    (void)text;
    (void)count;
    return FALSE;
}

BOOL MoveToEx(HDC dc, int x, int y, LPPOINT old) {
    (void)dc;
    (void)x;
    (void)y;
    (void)old;
    return FALSE;
}

BOOL LineTo(HDC dc, int x, int y) {
    (void)dc;
    (void)x;
    (void)y;
    return FALSE;
}

int StretchDIBits(HDC dc, int x, int y, int w, int h, int sx, int sy, int sw, int sh, const void *bits,
    const BITMAPINFO *info, UINT usage, DWORD rop) {
    (void)dc;
    (void)x;
    (void)y;
    (void)w;
    (void)h;
    (void)sx;
    (void)sy;
    (void)sw;
    (void)sh;
    (void)bits;
    (void)info;
    (void)usage;
    (void)rop;
    return 0;
}

/* ---- printing: no printers ---- */

BOOL GetDefaultPrinterA(LPSTR name, LPDWORD size) {
    (void)name;
    (void)size;
    PosixSetError(ERROR_FILE_NOT_FOUND);
    return FALSE;
}

BOOL OpenPrinterA(LPSTR name, LPHANDLE printer, LPPRINTER_DEFAULTSA defaults) {
    (void)name;
    (void)defaults;
    *printer = NULL;
    PosixSetError(ERROR_INVALID_PRINTER_NAME);
    return FALSE;
}

BOOL ClosePrinter(HANDLE printer) {
    (void)printer;
    return FALSE;
}

LONG DocumentPropertiesA(HWND w, HANDLE printer, LPSTR name, PDEVMODEA out, PDEVMODEA in, DWORD mode) {
    (void)w;
    (void)printer;
    (void)name;
    (void)out;
    (void)in;
    (void)mode;
    return -1;
}

BOOL EnumPrintersA(DWORD flags, LPSTR name, DWORD level, LPBYTE buffer, DWORD size, LPDWORD needed,
    LPDWORD returned) {
    (void)flags;
    (void)name;
    (void)level;
    (void)buffer;
    (void)size;
    if (needed != NULL) {
        *needed = 0;
    }
    if (returned != NULL) {
        *returned = 0;
    }
    return TRUE; /* none */
}

int StartDocA(HDC dc, const DOCINFOA *info) {
    (void)dc;
    (void)info;
    return 0;
}

int EndDoc(HDC dc) {
    (void)dc;
    return 0;
}

int StartPage(HDC dc) {
    (void)dc;
    return 0;
}

int EndPage(HDC dc) {
    (void)dc;
    return 0;
}

/* ---- COM: no system objects here (the game's DirectMusic is miniwin's) ---- */

HRESULT CoInitialize(LPVOID reserved) {
    (void)reserved;
    return S_OK;
}

HRESULT CoCreateInstance(REFCLSID clsid, LPUNKNOWN outer, DWORD context, REFIID iid, LPVOID *out) {
    (void)clsid;
    (void)outer;
    (void)context;
    (void)iid;
    if (out != NULL) {
        *out = NULL;
    }
    return REGDB_E_CLASSNOTREG;
}
