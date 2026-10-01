#include <windows.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include "legoland.h"

#include "debug.h"
#include "exceptlog.h"
#include "globals.h"
#include "main.h"
#include "screens.h"
#ifdef LEGOLAND_PORT
#include "port_data.h"
#endif

// FUNCTION: LEGOLAND 0x00453cd0
void FUN_00453cd0(char *text) {}

// FUNCTION: LEGOLAND 0x00453ce0
void FUN_00453ce0(const char *format, ...) {
    va_list argptr;

    va_start(argptr, format);
    vsprintf(DAT_00667128, format, argptr);
    va_end(argptr);
    FUN_00453cd0(DAT_00667128);
}

// FUNCTION: LEGOLAND 0x00453d10
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    int result;

    result = -1;
#ifdef LEGOLAND_PORT
    PortLoadData(); /* [port] the original started with its .data already initialized */
    if (strstr(lpCmdLine, "-port-selftest") != NULL) {
        /* [port] check the startup data and exit, without starting the game */
        return PortDataSelfTest("port-selftest.txt");
    }
#endif
    __try {
        FUN_00458830((char *)&DAT_0066752c);
        result = wWinMain(hInstance, hPrevInstance, lpCmdLine, nCmdShow);
        // STRING: LEGOLAND 0x004b8a94
    } __except (stackdump((void *)GetExceptionInformation(), "main thread")) {
    }

    return result;
}
