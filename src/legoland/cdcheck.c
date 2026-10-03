#include <windows.h>
#include "legoland.h"

#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include "cdcheck.h"
#include "debug.h"
#include "globals.h"
#include "saveload.h"
#include "wndenv.h"

#ifdef LEGOLAND_PORT
// [library:filesystem] The port's data folder is a complete install (volumes\, speech\, FMV\ next to
// the exe), so the CD is never needed. Pretend the current folder is the CD drive: every CD fallback
// path is built as "%sfile" from CdDrivePath, and ".\\" makes those point at the data folder.
static int PortDataFolderIsCd(void) {
    static int checked = 0;
    static int present = 0;

    if (!checked) {
        checked = 1;
        present = GetFileAttributesA(".\\volumes\\Legoland.res") != INVALID_FILE_ATTRIBUTES;
        if (present) {
            strcpy(CdDrivePath, ".\\");
        }
    }
    return present;
}
#endif

// FUNCTION: LEGOLAND 0x00450f30
int FindCdDriveWithVolume(char *cd_volume) {
    DWORD drives;
    int result;
    DWORD bit;
    int i;
    char root_path[4];
    char volume_name[256];
    char fs_name[256];
    DWORD serial_number;
    DWORD max_component_length;
    DWORD fs_flags;

#ifdef LEGOLAND_PORT
    if (PortDataFolderIsCd()) { // [library:filesystem]
        return 1;
    }
#endif
    // STRING: LEGOLAND 0x004b8630
    strcpy(root_path, "c:\\");
    result = 0;
    drives = GetLogicalDrives();
    // STRING: LEGOLAND 0x004b8610
    DebugTrace("Checking all drives (Mask = %d)", drives);
    FUN_0047f850();
    if (drives != 0) {
        bit = 1;
        i = 0;
        do {
            if (bit & drives) {
                root_path[0] = (char)('A' + i);
                if (GetDriveTypeA(root_path) == DRIVE_CDROM) {
                    // STRING: LEGOLAND 0x004b85f4
                    DebugTrace("Getting Info on drive %s", root_path);
                    FUN_0047f850();
                    if (GetVolumeInformationA(root_path, volume_name, 256, &serial_number, &max_component_length, &fs_flags, fs_name, 256)) {
                        // STRING: LEGOLAND 0x004b85ec
                        if (strcmp(fs_name, "CDFS") == 0) {
                            if (cd_volume == NULL || strcmp(volume_name, cd_volume) == 0) {
                                result = 1;
                                strcpy(CdDrivePath, root_path);
                                // STRING: LEGOLAND 0x004b85c8
                                DebugTrace("Drive %s contains the correct CD", CdDrivePath);
                                FUN_0047f850();
                            }
                        }
                    }
                }
            }
            i++;
            bit <<= 1;
        } while (i < 32);
    }

    return result;
}

// FUNCTION: LEGOLAND 0x004510e0
int IsCdVolumePresent(char *cd_volume) {
    int result;
    char root_path[4];
    char volume_name[256];
    char fs_name[256];
    DWORD serial_number;
    DWORD max_component_length;
    DWORD fs_flags;

#ifdef LEGOLAND_PORT
    if (PortDataFolderIsCd()) { // [library:filesystem]
        return 1;
    }
#endif
    strcpy(root_path, "c:\\");
    result = 0;
    root_path[0] = CdDrivePath[0];
    toupper(CdDrivePath[0]);
    if (GetVolumeInformationA(root_path, volume_name, 256, &serial_number, &max_component_length, &fs_flags, fs_name, 256)) {
        if (strcmp(fs_name, "CDFS") == 0) {
            if (cd_volume == NULL || strcmp(volume_name, cd_volume) == 0) {
                result = 1;
            }
        }
    }
    return result;
}

// FUNCTION: LEGOLAND 0x004511e0
void FUN_004511e0(void) { return; }

// FUNCTION: LEGOLAND 0x004511f0
void FUN_004511f0(int param_1) { return; }

// FUNCTION: LEGOLAND 0x00451200
void FUN_00451200(void) { return; }

// FUNCTION: LEGOLAND 0x00451210
void FUN_00451210(int param_1) {
    int drive;

    FUN_004511f0(param_1);
    if (DAT_004b85c4 != INVALID_HANDLE_VALUE) {
        *(char *)&drive = (char)toupper((char)param_1) - 0x40;
        FUN_00451280(DAT_004b85c4, drive);
        FUN_00451410(DAT_004b85c4, drive);
        UnlockLogicalVolume(DAT_004b85c4, drive);
        CloseDeviceHandle(DAT_004b85c4);
        DAT_004b85c4 = INVALID_HANDLE_VALUE;
    }
}

// FUNCTION: LEGOLAND 0x00451280
int FUN_00451280(HANDLE h, int drive) {
    DIOC_REGISTERS regs = {0};
    unsigned char buf[8];
    int result;
    int i;
    int drv;

    buf[1] = 0;
    drv = drive & 0xff;
    regs.reg_EBX = drv;
    regs.reg_EDX = (DWORD)buf;
    buf[0] = 2;
    regs.reg_EAX = 0x440d;
    regs.reg_ECX = 0x848;
    result = DeviceIoControl(h, 1, &regs, sizeof(regs), &regs, sizeof(regs), (LPDWORD)&drive, NULL);
    if (!result) {
        return 0;
    }
    if (regs.reg_Flags & 1) {
        if (regs.reg_EAX != 0xb0 && regs.reg_EAX != 1) {
            return 0;
        }
        result = 1;
    }
    i = 0;
    while (buf[1] > 0) {
        regs.reg_EBX = drv;
        regs.reg_EDX = (DWORD)buf;
        buf[0] = 1;
        regs.reg_EAX = 0x440d;
        regs.reg_ECX = 0x848;
        if (!DeviceIoControl(h, 1, &regs, sizeof(regs), &regs, sizeof(regs), (LPDWORD)&drive, NULL) || (regs.reg_Flags & 1)) {
            result = 0;
            break;
        }
        i++;
        result = 1;
        if (i >= buf[1]) {
            break;
        }
    }
    return result;
}

// FUNCTION: LEGOLAND 0x00451390
int FUN_00451390(HANDLE h, int drive) {
    DIOC_REGISTERS regs = {0};
    char buf[2];

    regs.reg_EBX = drive & 0xff;
    buf[1] = 0;
    buf[0] = 0;
    regs.reg_EAX = 0x440d;
    regs.reg_ECX = 0x848;
    regs.reg_EDX = (DWORD)buf;
    if (DeviceIoControl(h, 1, &regs, sizeof(regs), &regs, sizeof(regs), (LPDWORD)&drive, NULL) && !(regs.reg_Flags & 1)) {
        return 1;
    }
    return 0;
}

// FUNCTION: LEGOLAND 0x00451410
int FUN_00451410(HANDLE h, int drive) {
    DIOC_REGISTERS regs = {0};

    regs.reg_EBX = drive & 0xff;
    regs.reg_EAX = 0x440d;
    regs.reg_ECX = 0x849;
    if (DeviceIoControl(h, 1, &regs, sizeof(regs), &regs, sizeof(regs), (LPDWORD)&drive, NULL) && !(regs.reg_Flags & 1)) {
        return 1;
    }
    return 0;
}

// FUNCTION: LEGOLAND 0x00451480
int OpenVWin32Device(void) {
    // STRING: LEGOLAND 0x004b8634
    HANDLE result = CreateFileA("\\\\.\\vwin32", 0, 0, NULL, 0, 0x4000000, NULL);
    return (int)result;
}

// FUNCTION: LEGOLAND 0x004514a0
BOOL __stdcall CloseDeviceHandle(HANDLE param_1) { return CloseHandle(param_1); }

// FUNCTION: LEGOLAND 0x004514b0
BOOL __stdcall FUN_004514b0(HANDLE h, int drive, int param_3, int param_4) {
    DIOC_REGISTERS regs = {0};
    unsigned char category = 0x48;
    BOOL result;
    DWORD cb;

    for (;;) {
        regs.reg_ECX = MAKEWORD(0x4a, category);
        regs.reg_EAX = 0x440d;
        regs.reg_EBX = MAKEWORD(drive, param_3);
        regs.reg_EDX = param_4 & 0xffff;
        if (DeviceIoControl(h, 1, &regs, sizeof(regs), &regs, sizeof(regs), &cb, NULL) && !(regs.reg_Flags & 1)) {
            result = TRUE;
            break;
        }
        result = FALSE;
        if (category == 8) {
            break;
        }
        category = 8;
    }
    return result;
}

// FUNCTION: LEGOLAND 0x00451550
BOOL __stdcall UnlockLogicalVolume(HANDLE h, int drive) {
    DIOC_REGISTERS regs = {0};
    unsigned char category = 0x48;
    BOOL result;
    DWORD cb;

    for (;;) {
        regs.reg_ECX = MAKEWORD(0x6a, category);
        regs.reg_EAX = 0x440d;
        regs.reg_EBX = drive & 0xff;
        if (DeviceIoControl(h, 1, &regs, sizeof(regs), &regs, sizeof(regs), &cb, NULL) && !(regs.reg_Flags & 1)) {
            result = TRUE;
            break;
        }
        result = FALSE;
        if (category == 8) {
            break;
        }
        category = 8;
    }
    return result;
}

// FUNCTION: LEGOLAND 0x004515e0
int WaitForLegolandCd(int param_1) {
    char text[256];
    int minimised;

    minimised = 0;
    if (param_1 != 0) {
        // STRING: LEGOLAND 0x004b86d0
        while (!FindCdDriveWithVolume("LEGOLAND")) {
            if (!minimised) {
                // STRING: LEGOLAND 0x004b86c0
                DebugTrace("Minimising Game");
                FUN_0047f850();
                MinimizeGameWindow();
                minimised = 1;
            }
            if (MessageBoxA(WNDENV_Gethwnd(),
                    // STRING: LEGOLAND 0x004b8680
                    "Please insert the LEGOLAND CD-ROM into the CD drive",
                    // STRING: LEGOLAND 0x004b86b4
                    "CD Missing", 0x50015) == IDCANCEL) {
                // STRING: LEGOLAND 0x004b8670
                DebugTrace("Maximising Game");
                FUN_0047f850();
                RestoreGameWindow();
                return 0;
            }
        }
        if (minimised) {
            DebugTrace("Maximising Game");
            FUN_0047f850();
            RestoreGameWindow();
        }
        return 1;
    } else {
        while (!IsCdVolumePresent("LEGOLAND")) {
            if (!minimised) {
                DebugTrace("Minimising Game");
                FUN_0047f850();
                MinimizeGameWindow();
                minimised = 1;
            }
            // STRING: LEGOLAND 0x004b8640
            sprintf(text, "Please insert the LEGOLAND CD-ROM into drive %s", CdDrivePath);
            if (MessageBoxA(WNDENV_Gethwnd(), text, "CD Missing", 0x50015) == IDCANCEL) {
                DebugTrace("Maximising Game");
                FUN_0047f850();
                RestoreGameWindow();
                return 0;
            }
        }
        if (minimised) {
            DebugTrace("Maximising Game");
            FUN_0047f850();
            RestoreGameWindow();
        }
        return 1;
    }
}
