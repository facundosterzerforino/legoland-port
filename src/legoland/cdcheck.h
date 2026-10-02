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
int FUN_00451280(HANDLE h, int drive);
int FUN_00451410(HANDLE h, int drive);
BOOL __stdcall FUN_00451550(HANDLE h, int drive);
BOOL __stdcall CloseDeviceHandle(HANDLE param_1);
