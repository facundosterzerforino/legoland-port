#pragma once
#include <windows.h>

typedef struct DIOC_REGISTERS {
    DWORD reg_EBX;
    DWORD reg_EDX;
    DWORD reg_ECX;
    DWORD reg_EAX;
    DWORD reg_EDI;
    DWORD reg_ESI;
    DWORD reg_Flags;
} DIOC_REGISTERS;

int WaitForLegolandCd(int param_1);

void FUN_004511f0(int param_1);
int UnlockMedia(HANDLE h, int drive);
int EjectMedia(HANDLE h, int drive);
BOOL __stdcall UnlockLogicalVolume(HANDLE h, int drive);
BOOL __stdcall CloseDeviceHandle(HANDLE param_1);
