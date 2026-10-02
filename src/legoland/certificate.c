#include "legoland.h"

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
    if (EnumPrintersA(1, NULL, 2, printers, 0x540, &needed, &returned) <= 0)
        return 0;
    if (returned <= 0)
        return 0;
    memset(&dm, 0, 0x94);
    dm.dmSize = 0x94;
    dm.dmFields = DM_ORIENTATION;
    dm.dmOrientation = DMORIENT_LANDSCAPE;
    hDC = CreateDCA(NULL, ((char **)printers)[1], NULL, &dm);
    if (hDC == NULL)
        return 0;
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
            if (pInfo != NULL) {
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
                    return 0;
                }
                pBits = GlobalLock(hBits);
                if (pBits == NULL) {
                    GlobalUnlock(hInfo);
                    GlobalFree(hInfo);
                    GlobalFree(hBits);
                    _close(fd);
                    DeleteDC(hDC);
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
                    return 0;
                }
                if (StartPage(hDC) <= 0) {
                    GlobalFree(hInfo);
                    GlobalFree(hBits);
                    DeleteObject(hBitmap);
                    EndDoc(hDC);
                    DeleteDC(hDC);
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
                    return 0;
                }
                if (SelectObject(hMemDC, hBitmap) == NULL) {
                    GlobalFree(hInfo);
                    GlobalFree(hBits);
                    DeleteObject(hBitmap);
                    EndPage(hDC);
                    EndDoc(hDC);
                    DeleteDC(hDC);
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
                    return 0;
                }
                EndDoc(hDC);
                DeleteDC(hDC);
                GlobalFree(hInfo);
                GlobalFree(hBits);
                DeleteObject(hBitmap);
                return 1;
            }
            GlobalFree(hInfo);
        }
        _close(fd);
    }
    DeleteDC(hDC);
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
