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
#include "port_trace.h"
#endif

// FUNCTION: LEGOLAND 0x00453cd0
void LogOutput(char *text) {}

// FUNCTION: LEGOLAND 0x00453ce0
void LogPrintf(const char *format, ...) {
    va_list argptr;

    va_start(argptr, format);
    vsprintf(LogBuffer, format, argptr);
    va_end(argptr);
    LogOutput(LogBuffer);
}

// FUNCTION: LEGOLAND 0x00453d10
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    int result;

    result = -1;
#ifdef LEGOLAND_PORT
    PortLoadData(); /* [port] the original started with its .data already initialized */
    PortTrace("WinMain: cmdline \"%s\"", lpCmdLine);
    if (strstr(lpCmdLine, "-port-selftest") != NULL) {
        /* [port] check the startup data and exit, without starting the game */
        return PortDataSelfTest("port-selftest.txt");
    }
#endif
    __try {
        GetProductVersion(ProductVersionString);
        result = wWinMain(hInstance, hPrevInstance, lpCmdLine, nCmdShow);
        // STRING: LEGOLAND 0x004b8a94
    } __except (stackdump((void *)GetExceptionInformation(), "main thread")) {
    }

#ifdef LEGOLAND_PORT
    PortTrace("WinMain: returning %d", result);
#endif
    return result;
}
