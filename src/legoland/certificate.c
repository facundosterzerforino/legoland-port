#include "legoland.h"
#ifdef LEGOLAND_PORT
#include "debug.h"
#include "port_language.h"
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

// Print-job name; a named array so the asm version of PrintCertificate can push its address.
// GLOBAL: LEGOLAND 0x004b86e8
static const char CertificateDocName[] = "Lego certificate";

// Prints the end-of-game certificate: bitmap_path (EGC.bmp) stretched over a landscape page,
// player_name centred on the certificate's name line and date_text in the top-left corner.
// Returns 1 if the page was printed, 0 if any step failed (each failure frees what it made).
//  1. Take the FIRST local printer from EnumPrintersA (not necessarily the default; no dialog).
//  2. Create a printer DC in landscape orientation.
//  3. Read the BMP by hand (file header, info header, palette for <= 8 bpp, pixels) and build
//     a device bitmap with CreateDIBitmap.
//  4. Give up if the printer cannot print bitmaps (RASTERCAPS & RC_BITBLT).
//  5. StartDoc("Lego certificate") / StartPage.
//  6. StretchDIBits the image over the page, with margins of 1/8 of the width left and right
//     and 1/64 of the height top and bottom.
//  7. Print the name with the 20 pt "Lego" font, centred, at page height * 678 / image height
//     (row 678 of the bitmap is the certificate's name line).
//  8. Print the date with the 8 pt light "Lego" font, left-aligned at (0, 0).
//  9. EndPage / EndDoc and free everything.
// Quirks kept from the original: the first font is never deleted (only the second), the
// memory DC and the bitmap selected into it are never drawn with, and the LocalAlloc'd
// LOGFONT is not checked for NULL.
//
// MSVC6 build: exact naked __asm transcription; the C in the #else branch is the same function
// (the plain version, without the volatile temporaries the C needed to get close) and each asm
// block is commented with the C it implements. Why asm: the C reaches 89.11% at best. With the
// real fix (the TextOutA y divides by pInfo->bmiHeader.biHeight) the frame layout is exactly
// the original's, but MSVC6 gives the zero constant ebx and fd ebp where the original has
// zero in ebp and fd in ebx, which also changes which error-cleanup blocks it merges; about 25
// natural variants did not move that. See docs/attempts/0x00451740-PrintCertificate.md.
// clang-format off
#if defined(_MSC_VER) && (_MSC_VER <= 1200) && defined(_M_IX86)
// FUNCTION: LEGOLAND 0x00451740
__declspec(naked) int PrintCertificate(char *bitmap_path, char *player_name, char *date_text) {
    __asm {
        // Frame after the four pushes (F = esp):
        //   F+0x10 fd (later hres)      F+0x14 hFont (later hOldFont)  F+0x18 hBitmap
        //   F+0x1c hBits                F+0x20 hInfo                   F+0x24 pInfo
        //   F+0x28 returned             F+0x2c pBits                   F+0x30 needed
        //   F+0x34 hMemDC               F+0x38 bmih (BITMAPINFOHEADER) F+0x60 bmfh (BITMAPFILEHEADER)
        //   F+0x70 di (DOCINFOA)        F+0x84 dm (DEVMODEA)           F+0x118 printers[0xa80]
        //   F+0xb9c bitmap_path         F+0xba0 player_name            F+0xba4 date_text
        // Registers: esi = hDC; edi = hInfo; ebx = fd, then hBits, then plf;
        //            ebp = 0, then pInfo, then hBitmap, then vres, then the second font.

        // needed = 0; returned = 0;
        // if (EnumPrintersA(PRINTER_ENUM_LOCAL, NULL, 2, printers, 0x540, &needed, &returned) <= 0) return 0;
        sub esp, 0xb88
        push ebx
        push ebp
        push esi
        lea eax, [esp + 0x24] // &returned
        push edi
        lea ecx, [esp + 0x30] // &needed
        push eax
        push ecx
        lea edx, [esp + 0x120] // printers
        push 0x540
        xor ebp, ebp // ebp = 0
        push edx
        push 2
        push ebp
        push 1
        mov dword ptr [esp + 0x4c], ebp // needed = 0
        mov dword ptr [esp + 0x44], ebp // returned = 0
        call EnumPrintersA
        test eax, eax
        jle fail_no_cleanup
        // if (returned <= 0) return 0;
        cmp dword ptr [esp + 0x28], ebp
        jbe fail_no_cleanup

        // memset(&dm, 0, sizeof(dm)); dm.dmSize = 0x94; dm.dmFields = DM_ORIENTATION;
        // dm.dmOrientation = DMORIENT_LANDSCAPE;
        // hDC = CreateDCA(NULL, ((PRINTER_INFO_2A *)printers)->pPrinterName, NULL, &dm);
        mov ecx, 0x25
        xor eax, eax
        lea edi, [esp + 0x84] // &dm
        rep stosd
        mov ecx, dword ptr [esp + 0x11c] // printers[0].pPrinterName
        lea eax, [esp + 0x84]
        push eax
        push ebp
        push ecx
        push ebp
        mov word ptr [esp + 0xb8], 0x94 // dm.dmSize
        mov dword ptr [esp + 0xbc], 1 // dm.dmFields = DM_ORIENTATION
        mov word ptr [esp + 0xc0], 2 // dm.dmOrientation = DMORIENT_LANDSCAPE
        call dword ptr [CreateDCA]
        mov esi, eax // esi = hDC
        // if (!hDC) return 0;
        cmp esi, ebp
        je fail_no_cleanup

        // fd = _open(bitmap_path, _O_RDONLY | _O_BINARY, _S_IREAD); if (fd < 0) { DeleteDC(hDC); return 0; }
        mov edx, dword ptr [esp + 0xb9c] // bitmap_path
        push 0x100
        push 0x8000
        push edx
        call _open
        mov ebx, eax // ebx = fd
        add esp, 0xc
        cmp ebx, ebp
        mov dword ptr [esp + 0x10], ebx
        jl fail_delete_dc

        // _read(fd, &bmfh, 14); _read(fd, &bmih, 40);
        lea eax, [esp + 0x60]
        push 0xe
        push eax
        push ebx
        call _read
        lea ecx, [esp + 0x44]
        push 0x28
        push ecx
        push ebx
        call _read
        // hInfo = GlobalAlloc(GHND, (bmih.biBitCount > 8 ? 0 : 1 << bmih.biBitCount) * 4 + 40);
        mov ecx, dword ptr [esp + 0x5e] // bmih.biBitCount (in cx)
        add esp, 0x18
        cmp cx, 8
        jbe palette_sized
        xor eax, eax
        jmp alloc_info
    palette_sized:
        mov eax, 1
        shl eax, cl
    alloc_info:
        lea edx, [eax * 4 + 0x28]
        push edx
        push 0x42 // GHND
        call dword ptr [GlobalAlloc]
        mov edi, eax // edi = hInfo
        // if (hInfo == NULL) goto close_file;
        cmp edi, ebp
        mov dword ptr [esp + 0x20], edi
        je close_file
        // pInfo = GlobalLock(hInfo); if (pInfo == NULL) { GlobalFree(hInfo); goto close_file; }
        push edi
        call dword ptr [GlobalLock]
        mov ebp, eax // ebp = pInfo
        test ebp, ebp
        mov dword ptr [esp + 0x24], ebp
        jne copy_header
        push edi
        call dword ptr [GlobalFree]
    close_file:
        // _close(fd);
        push ebx
        call _close
        add esp, 4
    fail_delete_dc:
        // DeleteDC(hDC); return 0;
        push esi
        call dword ptr [DeleteDC]
        pop edi
        pop esi
        pop ebp
        xor eax, eax
        pop ebx
        add esp, 0xb88
        ret

    copy_header:
        // pInfo->bmiHeader = bmih, field by field (biSize .. biClrImportant)
        mov eax, dword ptr [esp + 0x38]
        mov dword ptr [ebp], eax
        mov ecx, dword ptr [esp + 0x3c]
        mov dword ptr [ebp + 4], ecx
        mov edx, dword ptr [esp + 0x40]
        mov dword ptr [ebp + 8], edx
        mov ax, word ptr [esp + 0x44]
        mov word ptr [ebp + 0xc], ax
        mov cx, word ptr [esp + 0x46]
        mov word ptr [ebp + 0xe], cx
        mov edx, dword ptr [esp + 0x48]
        mov dword ptr [ebp + 0x10], edx
        mov eax, dword ptr [esp + 0x4c]
        mov dword ptr [ebp + 0x14], eax
        mov ecx, dword ptr [esp + 0x50]
        mov dword ptr [ebp + 0x18], ecx
        mov edx, dword ptr [esp + 0x54]
        cmp word ptr [ebp + 0xe], 9 // pInfo->bmiHeader.biBitCount < 9 ?
        mov dword ptr [ebp + 0x1c], edx
        mov eax, dword ptr [esp + 0x58]
        mov dword ptr [ebp + 0x20], eax
        mov ecx, dword ptr [esp + 0x5c]
        mov dword ptr [ebp + 0x24], ecx
        // if (pInfo->bmiHeader.biBitCount < 9) _read(fd, pInfo->bmiColors, (1 << bmih.biBitCount) * 4);
        jae palette_done
        mov cl, byte ptr [esp + 0x46]
        mov edx, 1
        shl edx, cl
        lea eax, [ebp + 0x28] // pInfo->bmiColors
        shl edx, 2
        push edx
        push eax
        push ebx
        call _read
        add esp, 0xc
    palette_done:
        // hBits = GlobalAlloc(GHND, bmfh.bfSize - bmfh.bfOffBits);   (ebx = hBits from here)
        mov ecx, dword ptr [esp + 0x62] // bmfh.bfSize
        mov ebx, dword ptr [esp + 0x6a] // bmfh.bfOffBits
        sub ecx, ebx
        push ecx
        push 0x42
        call dword ptr [GlobalAlloc]
        mov ebx, eax
        test ebx, ebx
        mov dword ptr [esp + 0x1c], ebx
        jne lock_bits
        // if (hBits == NULL) { GlobalUnlock(hInfo); GlobalFree(hInfo); _close(fd); DeleteDC(hDC); return 0; }
        push edi
        call dword ptr [GlobalUnlock]
        push edi
        call dword ptr [GlobalFree]
        mov edx, dword ptr [esp + 0x10] // fd
        push edx
        call _close
        add esp, 4
        push esi
        call dword ptr [DeleteDC]
        pop edi
        pop esi
        pop ebp
        xor eax, eax
        pop ebx
        add esp, 0xb88
        ret
    lock_bits:
        // pBits = GlobalLock(hBits);
        // if (pBits == NULL) { GlobalUnlock(hInfo); GlobalFree(hInfo); GlobalFree(hBits); _close(fd); DeleteDC(hDC); return 0; }
        push ebx
        call dword ptr [GlobalLock]
        test eax, eax
        mov dword ptr [esp + 0x2c], eax // pBits
        jne read_bits
        push edi
        call dword ptr [GlobalUnlock]
        push edi
        mov edi, dword ptr [GlobalFree]
        call edi
        push ebx
        call edi
        mov eax, dword ptr [esp + 0x10]
        push eax
        call _close
        add esp, 4
        push esi
        call dword ptr [DeleteDC]
        pop edi
        pop esi
        pop ebp
        xor eax, eax
        pop ebx
        add esp, 0xb88
        ret
    read_bits:
        // _read(fd, pBits, bmfh.bfSize - bmfh.bfOffBits);
        mov ecx, dword ptr [esp + 0x62]
        mov edx, dword ptr [esp + 0x6a]
        sub ecx, edx
        mov edx, dword ptr [esp + 0x10]
        push ecx
        push eax
        push edx
        call _read
        // hBitmap = CreateDIBitmap(hDC, &bmih, CBM_INIT, pBits, pInfo, DIB_RGB_COLORS);   (ebp = hBitmap)
        mov eax, dword ptr [esp + 0x38] // pBits
        add esp, 0xc
        lea ecx, [esp + 0x38] // &bmih
        push 0
        push ebp
        push eax
        push 4 // CBM_INIT
        push ecx
        push esi
        call dword ptr [CreateDIBitmap]
        mov ebp, eax
        push edi
        test ebp, ebp
        mov dword ptr [esp + 0x1c], ebp
        jne bitmap_made
        // if (hBitmap == NULL) {
        //     GlobalUnlock(hInfo); GlobalUnlock(hBits); GlobalFree(hInfo); GlobalFree(hBits);
        //     DeleteObject(NULL); DeleteDC(hDC); return 0;
        // }
        mov ebp, dword ptr [GlobalUnlock]
        call ebp
        push ebx
        call ebp
        push edi
        mov edi, dword ptr [GlobalFree]
        call edi
        push ebx
        call edi
        push 0
        call dword ptr [DeleteObject]
        push esi
        call dword ptr [DeleteDC]
        pop edi
        pop esi
        pop ebp
        xor eax, eax
        pop ebx
        add esp, 0xb88
        ret
    bitmap_made:
        // GlobalUnlock(hInfo); GlobalUnlock(hBits); _close(fd);
        call dword ptr [GlobalUnlock]
        push ebx
        call dword ptr [GlobalUnlock]
        mov edx, dword ptr [esp + 0x10]
        push edx
        call _close
        add esp, 4
        // if (!(GetDeviceCaps(hDC, RASTERCAPS) & RC_BITBLT)) {
        //     GlobalFree(hInfo); GlobalFree(hBits); DeleteObject(hBitmap); DeleteDC(hDC); return 0;
        // }
        push 0x26 // RASTERCAPS
        push esi
        call dword ptr [GetDeviceCaps]
        test al, 1 // RC_BITBLT
        jne start_doc
        push edi
        mov edi, dword ptr [GlobalFree]
        call edi
        push ebx
        call edi
        push ebp
        call dword ptr [DeleteObject]
        push esi
        call dword ptr [DeleteDC]
        pop edi
        pop esi
        pop ebp
        xor eax, eax
        pop ebx
        add esp, 0xb88
        ret
    start_doc:
        // di.cbSize = sizeof(DOCINFOA); di.lpszOutput = NULL; di.lpszDatatype = NULL; di.fwType = 0;
        // di.lpszDocName = "Lego certificate";
        // if (StartDocA(hDC, &di) == -1) { GlobalFree x2; DeleteObject(hBitmap); DeleteDC(hDC); return 0; }
        xor eax, eax
        mov dword ptr [esp + 0x70], 0x14 // di.cbSize
        mov dword ptr [esp + 0x78], eax // di.lpszOutput
        mov dword ptr [esp + 0x7c], eax // di.lpszDatatype
        mov dword ptr [esp + 0x80], eax // di.fwType
        lea eax, [esp + 0x70]
        push eax
        push esi
        mov dword ptr [esp + 0x7c], offset CertificateDocName // di.lpszDocName
        call dword ptr [StartDocA]
        cmp eax, -1
        jne start_page
        push edi
        mov edi, dword ptr [GlobalFree]
        call edi
        push ebx
        call edi
        push ebp
        call dword ptr [DeleteObject]
        push esi
        call dword ptr [DeleteDC]
        pop edi
        pop esi
        pop ebp
        xor eax, eax
        pop ebx
        add esp, 0xb88
        ret
    start_page:
        // if (StartPage(hDC) <= 0) { GlobalFree x2; DeleteObject(hBitmap); EndDoc(hDC); DeleteDC(hDC); return 0; }
        push esi
        call dword ptr [StartPage]
        test eax, eax
        jg make_mem_dc
        push edi
        mov edi, dword ptr [GlobalFree]
        call edi
        push ebx
        call edi
        push ebp
        call dword ptr [DeleteObject]
        push esi
        call dword ptr [EndDoc]
        push esi
        call dword ptr [DeleteDC]
        pop edi
        pop esi
        pop ebp
        xor eax, eax
        pop ebx
        add esp, 0xb88
        ret
    make_mem_dc:
        // hMemDC = CreateCompatibleDC(hDC);
        // if (hMemDC == NULL) { GlobalFree x2; DeleteObject(hBitmap); EndPage; EndDoc; DeleteDC(hDC); return 0; }
        push esi
        call dword ptr [CreateCompatibleDC]
        test eax, eax
        mov dword ptr [esp + 0x34], eax // hMemDC
        jne select_bitmap
        push edi
        mov edi, dword ptr [GlobalFree]
        call edi
        push ebx
        call edi
        push ebp
        call dword ptr [DeleteObject]
        push esi
        call dword ptr [EndPage]
        push esi
        call dword ptr [EndDoc]
        push esi
        call dword ptr [DeleteDC]
        pop edi
        pop esi
        pop ebp
        xor eax, eax
        pop ebx
        add esp, 0xb88
        ret
    select_bitmap:
        // if (SelectObject(hMemDC, hBitmap) == NULL) { same cleanup as above; return 0; }
        push ebp
        push eax
        call dword ptr [SelectObject]
        test eax, eax
        jne draw_image
        push edi
        mov edi, dword ptr [GlobalFree]
        call edi
        push ebx
        call edi
        push ebp
        call dword ptr [DeleteObject]
        push esi
        call dword ptr [EndPage]
        push esi
        call dword ptr [EndDoc]
        push esi
        call dword ptr [DeleteDC]
        pop edi
        pop esi
        pop ebp
        xor eax, eax
        pop ebx
        add esp, 0xb88
        ret
    draw_image:
        // hres = GetDeviceCaps(hDC, HORZRES); vres = GetDeviceCaps(hDC, VERTRES);   (ebp = vres)
        mov ebp, dword ptr [GetDeviceCaps]
        push 8 // HORZRES
        push esi
        call ebp
        push 0xa // VERTRES
        push esi
        mov dword ptr [esp + 0x18], eax // hres
        call ebp
        mov ebp, eax
        // StretchDIBits(hDC, hres / 8, vres / 64, hres - (hres / 8) * 2, vres - (vres / 64) * 2,
        //               0, 0, bmih.biWidth, bmih.biHeight, pBits, pInfo, DIB_RGB_COLORS, SRCCOPY)
        // (the signed divisions round towards zero: cdq / and / add / sar)
        push 0xcc0020 // SRCCOPY
        cdq
        and edx, 0x3f
        push 0 // DIB_RGB_COLORS
        add eax, edx
        mov ecx, eax
        mov eax, dword ptr [esp + 0x18] // hres
        cdq
        and edx, 7
        add eax, edx
        mov edx, dword ptr [esp + 0x2c] // pInfo
        sar eax, 3
        mov dword ptr [esp + 0x1c], eax // hres / 8, kept in the hFont slot
        mov eax, dword ptr [esp + 0x34] // pBits
        push edx
        mov edx, dword ptr [esp + 0x4c] // bmih.biHeight
        push eax
        mov eax, dword ptr [esp + 0x4c] // bmih.biWidth
        sar ecx, 6 // vres / 64
        push edx
        push eax
        lea edx, [ecx + ecx]
        mov eax, ebp
        sub eax, edx // vres - (vres / 64) * 2
        mov edx, dword ptr [esp + 0x2c] // hres / 8
        push 0
        push 0
        push eax
        lea eax, [edx + edx]
        mov edx, dword ptr [esp + 0x34] // hres
        sub edx, eax // hres - (hres / 8) * 2
        mov eax, dword ptr [esp + 0x38] // hres / 8
        push edx
        push ecx
        push eax
        push esi
        call dword ptr [StretchDIBits]
        test eax, eax
        jne make_name_font
        // if it failed: GlobalFree x2; DeleteObject(hBitmap); EndPage; EndDoc; DeleteDC(hDC); return 0;
        push edi
        mov edi, dword ptr [GlobalFree]
        call edi
        push ebx
        call edi
        mov ecx, dword ptr [esp + 0x18] // hBitmap
        push ecx
        call dword ptr [DeleteObject]
        push esi
        call dword ptr [EndPage]
        push esi
        call dword ptr [EndDoc]
        push esi
        call dword ptr [DeleteDC]
        pop edi
        pop esi
        pop ebp
        xor eax, eax
        pop ebx
        add esp, 0xb88
        ret
    make_name_font:
        // plf = LocalAlloc(LPTR, sizeof(LOGFONTA));   (ebx = plf from here)
        // plf->lfHeight = -MulDiv(20, GetDeviceCaps(hDC, LOGPIXELSY), 72); plf->lfWeight = 400;
        // lstrcpyA(plf->lfFaceName, "Lego"); hFont = CreateFontIndirectA(plf);
        push 0x3c
        push 0x40 // LPTR
        call dword ptr [LocalAlloc]
        push 0x48
        push 0x5a // LOGPIXELSY
        push esi
        mov ebx, eax
        call dword ptr [GetDeviceCaps]
        push eax
        push 0x14
        call dword ptr [MulDiv]
        neg eax
        mov dword ptr [ebx], eax // plf->lfHeight
        lea eax, [ebx + 0x1c] // plf->lfFaceName
        push offset LegoFontName
        push eax
        mov dword ptr [ebx + 0x10], 0x190 // plf->lfWeight = 400
        call dword ptr [lstrcpyA]
        push ebx
        call dword ptr [CreateFontIndirectA]
        test eax, eax
        mov dword ptr [esp + 0x14], eax // hFont
        jne draw_name
        // if (!hFont) { GlobalFree(hInfo); GlobalFree(hBits); DeleteObject(hBitmap); DeleteDC(hDC); LocalFree(plf); return 0; }
        push edi
        mov edi, dword ptr [GlobalFree]
        call edi
        mov edx, dword ptr [esp + 0x1c] // hBits
        push edx
        call edi
        mov eax, dword ptr [esp + 0x18] // hBitmap
        push eax
    font_failed:
        call dword ptr [DeleteObject]
        push esi
        call dword ptr [DeleteDC]
        push ebx
        call dword ptr [LocalFree]
    fail_no_cleanup:
        pop edi
        pop esi
        pop ebp
        xor eax, eax
        pop ebx
        add esp, 0xb88
        ret
    draw_name:
        // SetBkMode(hDC, TRANSPARENT); hOldFont = SelectObject(hDC, hFont); SetBkMode(hDC, TRANSPARENT);
        // SetTextAlign(hDC, TA_CENTER | TA_TOP);
        mov edi, dword ptr [SetBkMode]
        push 1
        push esi
        call edi
        mov ecx, dword ptr [esp + 0x14] // hFont
        push ecx
        push esi
        call dword ptr [SelectObject]
        push 1
        push esi
        mov dword ptr [esp + 0x1c], eax // hOldFont (reuses hFont's slot)
        call edi
        push 6 // TA_CENTER | TA_TOP
        push esi
        call dword ptr [SetTextAlign]
        // TextOutA(hDC, hres / 2, vres * 678 / pInfo->bmiHeader.biHeight, player_name, strlen(player_name));
        mov edx, dword ptr [esp + 0xba0] // player_name
        or ecx, 0xffffffff // inlined strlen
        mov edi, edx
        xor eax, eax
        repne scasb
        lea eax, [ebp * 8] // vres * 678, as ((vres * 7) * 16 + vres) * 3 * 2
        sub eax, ebp
        shl eax, 4
        add eax, ebp
        not ecx
        dec ecx
        lea eax, [eax + eax * 2]
        push ecx
        mov ecx, dword ptr [esp + 0x28] // pInfo
        shl eax, 1
        push edx
        cdq
        idiv dword ptr [ecx + 8] // / pInfo->bmiHeader.biHeight
        push eax
        mov eax, dword ptr [esp + 0x1c] // hres
        cdq
        sub eax, edx
        sar eax, 1 // hres / 2
        push eax
        push esi
        call dword ptr [TextOutA]
        // plf->lfHeight = -MulDiv(8, GetDeviceCaps(hDC, LOGPIXELSY), 72); plf->lfWeight = 300;
        // lstrcpyA(plf->lfFaceName, "Lego"); hFont = CreateFontIndirectA(plf);   (ebp = hFont from here)
        push 0x48
        push 0x5a
        push esi
        call dword ptr [GetDeviceCaps]
        push eax
        push 8
        call dword ptr [MulDiv]
        neg eax
        mov dword ptr [ebx], eax
        lea eax, [ebx + 0x1c]
        push offset LegoFontName
        push eax
        mov dword ptr [ebx + 0x10], 0x12c // plf->lfWeight = 300
        call dword ptr [lstrcpyA]
        push ebx
        call dword ptr [CreateFontIndirectA]
        mov ebp, eax
        test ebp, ebp
        jne draw_date
        // if (hFont == NULL) same cleanup as the first font (shares its tail)
        mov edx, dword ptr [esp + 0x20] // hInfo
        mov edi, dword ptr [GlobalFree]
        push edx
        call edi
        mov eax, dword ptr [esp + 0x1c] // hBits
        push eax
        call edi
        mov ecx, dword ptr [esp + 0x18] // hBitmap
        push ecx
        jmp font_failed
    draw_date:
        // SetBkMode(hDC, TRANSPARENT); SelectObject(hDC, hFont); SetBkMode(hDC, TRANSPARENT);
        // SetTextAlign(hDC, TA_LEFT); TextOutA(hDC, 0, 0, date_text, strlen(date_text));
        mov edi, dword ptr [SetBkMode]
        push 1
        push esi
        call edi
        push ebp
        push esi
        call dword ptr [SelectObject]
        push 1
        push esi
        call edi
        push 0 // TA_LEFT
        push esi
        call dword ptr [SetTextAlign]
        mov edx, dword ptr [esp + 0xba4] // date_text
        or ecx, 0xffffffff
        mov edi, edx
        xor eax, eax
        repne scasb
        not ecx
        dec ecx
        push ecx
        push edx
        push eax // y = 0
        push eax // x = 0
        push esi
        call dword ptr [TextOutA]
        // LocalFree(plf); SelectObject(hDC, hOldFont); DeleteObject(hFont); DeleteDC(hMemDC);
        push ebx
        call dword ptr [LocalFree]
        mov edx, dword ptr [esp + 0x14] // hOldFont
        push edx
        push esi
        call dword ptr [SelectObject]
        mov edi, dword ptr [DeleteObject]
        push ebp
        call edi
        mov eax, dword ptr [esp + 0x34] // hMemDC
        mov ebx, dword ptr [DeleteDC]
        push eax
        call ebx
        // if (EndPage(hDC) <= 0) { GlobalFree x2; DeleteObject(hBitmap); EndDoc(hDC); DeleteDC(hDC); return 0; }
        push esi
        call dword ptr [EndPage]
        test eax, eax
        jg finish
        mov ecx, dword ptr [esp + 0x20]
        mov ebp, dword ptr [GlobalFree]
        push ecx
        call ebp
        mov edx, dword ptr [esp + 0x1c]
        push edx
        call ebp
        mov eax, dword ptr [esp + 0x18]
        push eax
        call edi
        push esi
        call dword ptr [EndDoc]
        push esi
        call ebx
        pop edi
        pop esi
        pop ebp
        xor eax, eax
        pop ebx
        add esp, 0xb88
        ret
    finish:
        // EndDoc(hDC); DeleteDC(hDC); GlobalFree(hInfo); GlobalFree(hBits); DeleteObject(hBitmap); return 1;
        push esi
        call dword ptr [EndDoc]
        push esi
        call ebx
        mov ecx, dword ptr [esp + 0x20]
        mov esi, dword ptr [GlobalFree]
        push ecx
        call esi
        mov edx, dword ptr [esp + 0x1c]
        push edx
        call esi
        mov eax, dword ptr [esp + 0x18]
        push eax
        call edi
        pop edi
        pop esi
        pop ebp
        mov eax, 1
        pop ebx
        add esp, 0xb88
        ret
    }
}
// clang-format on
#else
int PrintCertificate(char *bitmap_path, char *player_name, char *date_text) {
    DEVMODEA dm;
    BYTE printers[0xa80];
    DOCINFOA di;
    BITMAPFILEHEADER bmfh;
    HDC hMemDC;
    BITMAPINFOHEADER bmih;
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
    int hres;
    HDC hDC;
    int vres;
    int fd;

    needed = 0;
    returned = 0;
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
    }
#else
    if (EnumPrintersA(1, NULL, 2, printers, 0x540, &needed, &returned) <= 0)
        return 0;
    if (returned <= 0)
        return 0;
    memset(&dm, 0, 0x94);
    dm.dmSize = 0x94;
    dm.dmFields = DM_ORIENTATION;
    dm.dmOrientation = DMORIENT_LANDSCAPE;
    hDC = CreateDCA(NULL, ((char **)printers)[1], NULL, &dm);
#endif
    if (!hDC) {
#ifdef LEGOLAND_PORT
        DebugTrace("PrintCertificate: CreateDC failed (%lu)", GetLastError());
#endif
        return 0;
    }
    fd = _open(bitmap_path, _O_RDONLY | _O_BINARY, _S_IREAD);
    if (fd >= 0) {
        _read(fd, &bmfh, 14);
        _read(fd, &bmih, 40);
        hInfo = GlobalAlloc(GHND, (bmih.biBitCount > 8 ? 0 : 1 << bmih.biBitCount) * 4 + 40);
        if (hInfo != NULL) {
            pInfo = (BITMAPINFO *)GlobalLock(hInfo);
            if (pInfo == NULL) {
                GlobalFree(hInfo);
            } else {
                pInfo->bmiHeader = bmih;
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
                di.lpszOutput = NULL;
                di.lpszDatatype = NULL;
                di.fwType = 0;
                di.lpszDocName = CertificateDocName;
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
                if (hMemDC == NULL || SelectObject(hMemDC, hBitmap) == NULL) {
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
                lstrcpyA(plf->lfFaceName, LegoFontName);
#ifdef LEGOLAND_PORT
                plf->lfCharSet = (BYTE)PortGameCharset(plf->lfCharSet); /* [port] port_language.h */
#endif
                hFont = CreateFontIndirectA(plf);
                if (!hFont) {
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
                TextOutA(hDC, hres / 2, vres * 678 / pInfo->bmiHeader.biHeight, player_name, strlen(player_name));
                plf->lfHeight = -MulDiv(8, GetDeviceCaps(hDC, LOGPIXELSY), 72);
                plf->lfWeight = 300;
                lstrcpyA(plf->lfFaceName, LegoFontName);
#ifdef LEGOLAND_PORT
                plf->lfCharSet = (BYTE)PortGameCharset(plf->lfCharSet); /* [port] port_language.h */
#endif
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
                TextOutA(hDC, 0, 0, date_text, strlen(date_text));
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
#endif

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
