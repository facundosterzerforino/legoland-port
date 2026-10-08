#include "legoland.h"
#ifdef LEGOLAND_PORT
#include "debug.h"
#endif

#define _WINSPOOL_
#include <windows.h>
#include <fcntl.h>
#include <io.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include "certificate.h"
#include "globals.h"
#include "imports.h"

// FUNCTION: LEGOLAND 0x00451740
int PrintCertificate(char *param_1, char *param_2, char *param_3) {
    BYTE printers[0xa80];
    DEVMODEA dm;
    DOCINFOA di;
    BITMAPFILEHEADER bmfh;
    BITMAPINFOHEADER bmih;
    HDC hMemDC;
    DWORD needed;
    void *pBits;
    DWORD returned;
    BITMAPINFO *pInfo;
    HGLOBAL hInfo;
    HGLOBAL hBits;
    HBITMAP hBitmap;
    HFONT hFont;
    HFONT hOldFont;
    LOGFONTA *plf;
    HDC hDC;
    int hres;
    int vres;
    int ncolors;
    int fd;

    needed = 0;
    returned = 0;
    pInfo = NULL;
#ifdef LEGOLAND_PORT
    {
        /* [library:print] the original asks EnumPrinters for PRINTER_ENUM_DEFAULT (flags 1), which only Windows
         * 95/98/Me support: on NT it lists no printer and printing always failed. Ask for the default printer by
         * name instead (with "Microsoft Print to PDF" as the default, Windows asks where to save a PDF). */
        char printer_name[256];
        DWORD name_size = sizeof(printer_name);

        (void)printers;
        (void)needed;
        (void)returned;
        if (!GetDefaultPrinterA(printer_name, &name_size)) {
            DebugTrace("PrintCertificate: no default printer (%lu)", GetLastError());
            return 0;
        }
        DebugTrace("PrintCertificate: printing to \"%s\"", printer_name);
        /* the original's DEVMODE is a 0x94-byte Windows 9x one with only the orientation set; current drivers
         * refuse it (CreateDC fails with ERROR_ACCESS_DENIED). Ask the driver for its full settings and turn
         * them to landscape; without them, print with the driver's defaults. */
        (void)dm;
        {
            HANDLE spool = NULL;
            DEVMODEA *full = NULL;
            LONG size;

            if (OpenPrinterA(printer_name, &spool, NULL)) {
                size = DocumentPropertiesA(NULL, spool, printer_name, NULL, NULL, 0);
                if (size > 0) {
                    full = (DEVMODEA *)malloc(size);
                }
                if (full != NULL && DocumentPropertiesA(NULL, spool, printer_name, full, NULL, DM_OUT_BUFFER) == IDOK) {
                    full->dmFields |= DM_ORIENTATION;
                    full->dmOrientation = DMORIENT_LANDSCAPE;
                    DocumentPropertiesA(NULL, spool, printer_name, full, full, DM_IN_BUFFER | DM_OUT_BUFFER);
                } else {
                    free(full);
                    full = NULL;
                }
                ClosePrinter(spool);
            }
            hDC = CreateDCA(NULL, printer_name, NULL, full);
            free(full);
        }
        if (hDC == NULL) {
            DebugTrace("PrintCertificate: CreateDC failed (%lu)", GetLastError());
        }
    }
#else
    if (EnumPrintersA(1, NULL, 2, printers, 0x540, &needed, &returned) <= 0) {
#ifdef LEGOLAND_PORT
        DebugTrace("PrintCertificate: failed before line %d (%lu)", __LINE__, GetLastError());
#endif
        return 0;
    }
    if (returned <= 0) {
#ifdef LEGOLAND_PORT
        DebugTrace("PrintCertificate: failed before line %d (%lu)", __LINE__, GetLastError());
#endif
        return 0;
    }
    memset(&dm, 0, 0x94);
    dm.dmSize = 0x94;
    dm.dmFields = DM_ORIENTATION;
    dm.dmOrientation = DMORIENT_LANDSCAPE;
    hDC = CreateDCA(NULL, ((char **)printers)[1], NULL, &dm);
#endif
    if (hDC == NULL) {
#ifdef LEGOLAND_PORT
        DebugTrace("PrintCertificate: failed before line %d (%lu)", __LINE__, GetLastError());
#endif
        return 0;
    }
    fd = _open(param_1, _O_RDONLY | _O_BINARY, _S_IREAD);
    if (fd >= 0) {
        _read(fd, &bmfh, 14);
        _read(fd, &bmih, 40);
        if (bmih.biBitCount > 8)
            ncolors = 0;
        else
            ncolors = 1 << bmih.biBitCount;
        hInfo = GlobalAlloc(GHND, ncolors * 4 + 40);
        if (hInfo != NULL) {
            pInfo = (BITMAPINFO *)GlobalLock(hInfo);
            if (pInfo == NULL) {
                GlobalFree(hInfo);
            } else {
                pInfo->bmiHeader.biSize = bmih.biSize;
                pInfo->bmiHeader.biWidth = bmih.biWidth;
                pInfo->bmiHeader.biHeight = bmih.biHeight;
                pInfo->bmiHeader.biPlanes = bmih.biPlanes;
                pInfo->bmiHeader.biBitCount = bmih.biBitCount;
                pInfo->bmiHeader.biCompression = bmih.biCompression;
                pInfo->bmiHeader.biSizeImage = bmih.biSizeImage;
                pInfo->bmiHeader.biXPelsPerMeter = bmih.biXPelsPerMeter;
                pInfo->bmiHeader.biYPelsPerMeter = bmih.biYPelsPerMeter;
                pInfo->bmiHeader.biClrUsed = bmih.biClrUsed;
                pInfo->bmiHeader.biClrImportant = bmih.biClrImportant;
                if (pInfo->bmiHeader.biBitCount < 9)
                    _read(fd, pInfo->bmiColors, (1 << bmih.biBitCount) * 4);
                hBits = GlobalAlloc(GHND, bmfh.bfSize - bmfh.bfOffBits);
                if (hBits == NULL) {
                    GlobalUnlock(hInfo);
                    GlobalFree(hInfo);
                    _close(fd);
                    DeleteDC(hDC);
#ifdef LEGOLAND_PORT
                    DebugTrace("PrintCertificate: failed before line %d (%lu)", __LINE__, GetLastError());
#endif
                    return 0;
                }
                pBits = GlobalLock(hBits);
                if (pBits == NULL) {
                    GlobalUnlock(hInfo);
                    GlobalFree(hInfo);
                    GlobalFree(hBits);
                    _close(fd);
                    DeleteDC(hDC);
#ifdef LEGOLAND_PORT
                    DebugTrace("PrintCertificate: failed before line %d (%lu)", __LINE__, GetLastError());
#endif
                    return 0;
                }
                _read(fd, pBits, bmfh.bfSize - bmfh.bfOffBits);
                hBitmap = CreateDIBitmap(hDC, &bmih, CBM_INIT, pBits, pInfo, 0);
                if (hBitmap == NULL) {
                    GlobalUnlock(hInfo);
                    GlobalUnlock(hBits);
                    GlobalFree(hInfo);
                    GlobalFree(hBits);
                    DeleteObject(NULL);
                    DeleteDC(hDC);
#ifdef LEGOLAND_PORT
                    DebugTrace("PrintCertificate: failed before line %d (%lu)", __LINE__, GetLastError());
#endif
                    return 0;
                }
                GlobalUnlock(hInfo);
                GlobalUnlock(hBits);
                _close(fd);
                if (!(GetDeviceCaps(hDC, RASTERCAPS) & RC_BITBLT)) {
                    GlobalFree(hInfo);
                    GlobalFree(hBits);
                    DeleteObject(hBitmap);
                    DeleteDC(hDC);
#ifdef LEGOLAND_PORT
                    DebugTrace("PrintCertificate: failed before line %d (%lu)", __LINE__, GetLastError());
#endif
                    return 0;
                }
                di.cbSize = sizeof(DOCINFOA);
                // STRING: LEGOLAND 0x004b86e8
                di.lpszDocName = "Lego certificate";
                di.lpszOutput = NULL;
                di.lpszDatatype = NULL;
                di.fwType = 0;
                if (StartDocA(hDC, &di) == -1) {
                    GlobalFree(hInfo);
                    GlobalFree(hBits);
                    DeleteObject(hBitmap);
                    DeleteDC(hDC);
#ifdef LEGOLAND_PORT
                    DebugTrace("PrintCertificate: failed before line %d (%lu)", __LINE__, GetLastError());
#endif
                    return 0;
                }
                if (StartPage(hDC) <= 0) {
                    GlobalFree(hInfo);
                    GlobalFree(hBits);
                    DeleteObject(hBitmap);
                    EndDoc(hDC);
                    DeleteDC(hDC);
#ifdef LEGOLAND_PORT
                    DebugTrace("PrintCertificate: failed before line %d (%lu)", __LINE__, GetLastError());
#endif
                    return 0;
                }
                hMemDC = CreateCompatibleDC(hDC);
                if (hMemDC == NULL) {
                    GlobalFree(hInfo);
                    GlobalFree(hBits);
                    DeleteObject(hBitmap);
                    EndPage(hDC);
                    EndDoc(hDC);
                    DeleteDC(hDC);
#ifdef LEGOLAND_PORT
                    DebugTrace("PrintCertificate: failed before line %d (%lu)", __LINE__, GetLastError());
#endif
                    return 0;
                }
                if (SelectObject(hMemDC, hBitmap) == NULL) {
                    GlobalFree(hInfo);
                    GlobalFree(hBits);
                    DeleteObject(hBitmap);
                    EndPage(hDC);
                    EndDoc(hDC);
                    DeleteDC(hDC);
#ifdef LEGOLAND_PORT
                    DebugTrace("PrintCertificate: failed before line %d (%lu)", __LINE__, GetLastError());
#endif
                    return 0;
                }
                hres = GetDeviceCaps(hDC, HORZRES);
                vres = GetDeviceCaps(hDC, VERTRES);
                if (!StretchDIBits(hDC, hres / 8, vres / 64, hres - (hres / 8) * 2, vres - (vres / 64) * 2, 0, 0, bmih.biWidth, bmih.biHeight, pBits, pInfo, DIB_RGB_COLORS, SRCCOPY)) {
                    GlobalFree(hInfo);
                    GlobalFree(hBits);
                    DeleteObject(hBitmap);
                    EndPage(hDC);
                    EndDoc(hDC);
                    DeleteDC(hDC);
#ifdef LEGOLAND_PORT
                    DebugTrace("PrintCertificate: failed before line %d (%lu)", __LINE__, GetLastError());
#endif
                    return 0;
                }
                plf = (LOGFONTA *)LocalAlloc(LPTR, sizeof(LOGFONTA));
                plf->lfHeight = -MulDiv(20, GetDeviceCaps(hDC, LOGPIXELSY), 72);
                plf->lfWeight = 400;
                // STRING: LEGOLAND 0x004b86e0
                lstrcpyA(plf->lfFaceName, "Lego");
                hFont = CreateFontIndirectA(plf);
                if (hFont == NULL) {
                    GlobalFree(hInfo);
                    GlobalFree(hBits);
                    DeleteObject(hBitmap);
                    DeleteDC(hDC);
                    LocalFree(plf);
#ifdef LEGOLAND_PORT
                    DebugTrace("PrintCertificate: failed before line %d (%lu)", __LINE__, GetLastError());
#endif
                    return 0;
                }
                SetBkMode(hDC, TRANSPARENT);
                hOldFont = (HFONT)SelectObject(hDC, hFont);
                SetBkMode(hDC, TRANSPARENT);
                SetTextAlign(hDC, TA_CENTER | TA_TOP);
                TextOutA(hDC, hres / 2, vres * 678 / bmih.biHeight, param_2, strlen(param_2));
                plf->lfHeight = -MulDiv(8, GetDeviceCaps(hDC, LOGPIXELSY), 72);
                plf->lfWeight = 300;
                lstrcpyA(plf->lfFaceName, "Lego");
                hFont = CreateFontIndirectA(plf);
                if (hFont == NULL) {
                    GlobalFree(hInfo);
                    GlobalFree(hBits);
                    DeleteObject(hBitmap);
                    DeleteDC(hDC);
                    LocalFree(plf);
#ifdef LEGOLAND_PORT
                    DebugTrace("PrintCertificate: failed before line %d (%lu)", __LINE__, GetLastError());
#endif
                    return 0;
                }
                SetBkMode(hDC, TRANSPARENT);
                SelectObject(hDC, hFont);
                SetBkMode(hDC, TRANSPARENT);
                SetTextAlign(hDC, TA_LEFT);
                TextOutA(hDC, 0, 0, param_3, strlen(param_3));
                LocalFree(plf);
                SelectObject(hDC, hOldFont);
                DeleteObject(hFont);
                DeleteDC(hMemDC);
                if (EndPage(hDC) <= 0) {
                    GlobalFree(hInfo);
                    GlobalFree(hBits);
                    DeleteObject(hBitmap);
                    EndDoc(hDC);
                    DeleteDC(hDC);
#ifdef LEGOLAND_PORT
                    DebugTrace("PrintCertificate: failed before line %d (%lu)", __LINE__, GetLastError());
#endif
                    return 0;
                }
                EndDoc(hDC);
                DeleteDC(hDC);
                GlobalFree(hInfo);
                GlobalFree(hBits);
                DeleteObject(hBitmap);
                return 1;
            }
        }
        _close(fd);
    }
    DeleteDC(hDC);
#ifdef LEGOLAND_PORT
    DebugTrace("PrintCertificate: failed before line %d (%lu)", __LINE__, GetLastError());
#endif
    return 0;
}

// FUNCTION: LEGOLAND 0x00451e20
int PrintCertificateWithCurrentDate(void) {
    time_t now;
    char *str;

    time(&now);
    str = asctime(localtime(&now));
    str[strlen(str) - 1] = '\0';
    // STRING: LEGOLAND 0x004b86fc
    return PrintCertificate("EGC.bmp", (char *)&CurrentProfile, str) != 0;
}
