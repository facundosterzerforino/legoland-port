#include <windows.h>

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cdcheck.h"
#include "debug.h"
#include "debug_alloc.h"
#include "draw.h"
#include "globals.h"
#include "image_sprite.h"
#include "input.h"
#include "legoland.h"
#include "llidb.h"
#include "resource.h"
#include "saveload.h"
#include "screens.h"
#include "stream.h"
#include "string.h"
#ifdef LEGOLAND_PORT
#include "port_trace.h"
#endif

// FUNCTION: LEGOLAND 0x0047f870
void DebugTrace(const char *fmt, ...) {
#ifdef LEGOLAND_PORT
    /* [port] the original compiled its traces out; the port writes them to legoland-port-trace.txt */
    va_list ap;

    va_start(ap, fmt);
    PortTraceV(fmt, ap);
    va_end(ap);
#endif
}

// FUNCTION: LEGOLAND 0x0047f880
int GameMain(void) {
    char buffer[1024];
    int i;
    int j;
    struct ResVolume **vol;

    lpConfig->field_1e = 1;

    // STRING: LEGOLAND 0x004bcd44
    if (OpenLogFile("legoland.log") == 0) {
        return 1;
    }
    if (WaitForLegolandCd(1) == 0) {
        DebugTrace("GameMain: no CD");
        return 1;
    }

    for (i = 0; i < 3; i++) {
        ResourceVolumes[i] = RES_OpenVolume(ResourceFileNames[i]);
        if (ResourceVolumes[i] == NULL) {
            // STRING: LEGOLAND 0x004bcd28
            sprintf(buffer, "Failed to open resource %s", ResourceFileNames[i]);
            DebugTrace("GameMain: %s", buffer);
            // STRING: LEGOLAND 0x004bcd18
            MessageBoxA(GetDesktopWindow(), buffer, "LEGOLAND Error", 0x30);
            for (j = 0; j < i; j++) {
                RES_CloseVolume(ResourceVolumes[j]);
            }
            return 1;
        }
    }

    LoadStringTable();
    DebugTrace("GameMain: strings loaded");
    InitHostSystemGPU();
    if (InitScreen() == 0) {
        DebugTrace("GameMain: InitScreen failed");
        MessageBoxA(GetDesktopWindow(), GetString(0xcc), GetString(0xcb), 0x30);
        KillHostSystemGPU();
        for (vol = ResourceVolumes; vol < ResourceVolumes + 3; vol++) {
            RES_CloseVolume(*vol);
        }
        return 1;
    }

    if (InitInputSystem() == 0) {
        DebugTrace("GameMain: InitInputSystem failed");
        MessageBoxA(GetDesktopWindow(), GetString(0x9c4), GetString(0xcb), 0x30);
        KillInputSystem();
        KillHostSystemGPU();
        for (vol = ResourceVolumes; vol < ResourceVolumes + 3; vol++) {
            RES_CloseVolume(*vol);
        }
        return 1;
    }

    DebugTrace("GameMain: input ok");
    PointerSprites[0] = 0;
    // STRING: LEGOLAND 0x004bcd08
    PointerSprites[1] = LoadSprite("erase it.lls", 0);
    DebugTrace("GameMain: cursor 1 loaded");
    // STRING: LEGOLAND 0x004bccf8
    PointerSprites[2] = LoadSprite("erase it2.lls", 0);
    DebugTrace("GameMain: cursor 2 loaded");
    // STRING: LEGOLAND 0x004bcce8
    PointerSprites[3] = LoadSprite("no build.lls", 0);
    DebugTrace("GameMain: cursor 3 loaded");
    // STRING: LEGOLAND 0x004bccd8
    PointerSprites[4] = LoadSprite("yes build.lls", 0);
    DebugTrace("GameMain: cursor 4 loaded");
    // STRING: LEGOLAND 0x004bccc4
    PointerSprites[5] = LoadSprite("rab over icon.lls", 0);
    DebugTrace("GameMain: cursor 5 loaded");
    // STRING: LEGOLAND 0x004bccb0
    PointerSprites[6] = LoadSprite("rab over icon2.lls", 0);
    DebugTrace("GameMain: cursor 6 loaded");
    // STRING: LEGOLAND 0x004bcca0
    PointerSprites[7] = LoadSprite("question it.lls", 0);
    DebugTrace("GameMain: cursor 7 loaded");
    // STRING: LEGOLAND 0x004bcc8c
    PointerSprites[8] = LoadSprite("question it2.lls", 0);
    DebugTrace("GameMain: cursor 8 loaded");

    DebugTrace("GameMain: cursors loaded");
    LLIDB_LoadICM();
    DebugTrace("GameMain: ICM loaded");

    LLIDB_RegisterNewElement("BUILD MENU", 0, 0x200);
    // STRING: LEGOLAND 0x004bcc78
    LLIDB_RegisterNewElement("ATTRACTIONS MENU", 0, 0x200);
    // STRING: LEGOLAND 0x004bcc64
    LLIDB_RegisterNewElement("FOOD STORES MENU", 0, 0x200);
    // STRING: LEGOLAND 0x004bcc54
    LLIDB_RegisterNewElement("SCENERY MENU", 0, 0x200);
    // STRING: LEGOLAND 0x004bcc48
    LLIDB_RegisterNewElement("SHOPS MENU", 0, 0x200);

    DebugTrace("GameMain: init done, entering FUN_00459520");
    FUN_00459520();

    DebugTrace("GameMain: FUN_00459520 returned");
    KillHostSystemGPU();

    for (vol = ResourceVolumes; vol < ResourceVolumes + 3; vol++) {
        if (*vol != 0) {
            RES_CloseVolume(*vol);
        }
    }

    // STRING: LEGOLAND 0x004bcc28
    DebugTrace("Finished shutting stuff down");

    CloseLogFile();
    DeleteStrings();
    LLIDB_CloseICM();

    if (PointerSprites[3] != 0) {
        KillSprite(PointerSprites[3]);
        PointerSprites[3] = 0;
    }
    if (PointerSprites[4] != 0) {
        KillSprite(PointerSprites[4]);
        PointerSprites[4] = 0;
    }
    if (PointerSprites[7] != 0) {
        KillSprite(PointerSprites[7]);
        PointerSprites[7] = 0;
    }
    if (PointerSprites[8] != 0) {
        KillSprite(PointerSprites[8]);
        PointerSprites[8] = 0;
    }
    if (PointerSprites[1] != 0) {
        KillSprite(PointerSprites[1]);
        PointerSprites[1] = 0;
    }
    if (PointerSprites[2] != 0) {
        KillSprite(PointerSprites[2]);
        PointerSprites[2] = 0;
    }
    if (PointerSprites[5] != 0) {
        KillSprite(PointerSprites[5]);
        PointerSprites[5] = 0;
    }
    if (PointerSprites[6] != 0) {
        KillSprite(PointerSprites[6]);
        PointerSprites[6] = 0;
    }

    return 0;
}

// FUNCTION: LEGOLAND 0x0047fc20
LEGO_EXPORT unsigned int mystrlen(const char *s) {
    if (s == NULL) {
        return 0;
    }
    return strlen(s);
}

// FUNCTION: LEGOLAND 0x0047fc40
char *StrIStr(const char *haystack, const char *needle) {
    char *upper_haystack;
    char *upper_needle;
    char *match;

    upper_haystack = (char *)malloc(strlen(haystack) + 1);
    if (upper_haystack == NULL) {
        return NULL;
    }
    strcpy(upper_haystack, haystack);
    _strupr(upper_haystack);

    upper_needle = (char *)malloc(strlen(needle) + 1);
    if (upper_needle == NULL) {
        free(upper_haystack);
        return NULL;
    }
    strcpy(upper_needle, needle);
    _strupr(upper_needle);

    match = strstr(upper_haystack, upper_needle);
    if (match != NULL) {
        match = (char *)(haystack + (match - upper_haystack));
    }
    free(upper_haystack);
    free(upper_needle);
    return match;
}

// FUNCTION: LEGOLAND 0x0047fd10
int __cdecl wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    HANDLE mutex;
    SECURITY_ATTRIBUTES sa;

    sa.nLength = sizeof(SECURITY_ATTRIBUTES);
    sa.lpSecurityDescriptor = NULL;
    sa.bInheritHandle = FALSE;
    // STRING: LEGOLAND 0x004bcdb0
    mutex = CreateMutexA(&sa, TRUE, "LegolandGameMutex");
    if (mutex == NULL) {
        // STRING: LEGOLAND 0x004bcd98
        DBPrintf("Couldn't create Mutex\n");
        return 0;
    }

    if (WaitForSingleObject(mutex, 0) == WAIT_TIMEOUT) {
        // STRING: LEGOLAND 0x004bcd7c
        DBPrintf("Program already running.\n");
        CloseHandle(mutex);
        return 0;
    }

    // STRING: LEGOLAND 0x004bcd70
    if (StrIStr(lpCmdLine, "WINDEBUG")) {
        BlitFrameFunc = BlitFrameToWindow;
        WinDebugMode = 1;
        // STRING: LEGOLAND 0x004bcd6c
    } else if (StrIStr(lpCmdLine, "BLT")) {
        BlitFrameFunc = BlitFrameToWindow;
    }

    // STRING: LEGOLAND 0x004bcd60
    if (StrIStr(lpCmdLine, "-nointro")) {
        lpConfig->field_40 = 1;
    } else {
        lpConfig->field_40 = 0;
    }

    // STRING: LEGOLAND 0x004bcd54
    MusicEnabled = StrIStr(lpCmdLine, "-nomusic") ? 0 : 1;
    g_hInstance = hInstance;
    g_nCmdShow = nCmdShow;

    if (CheckHostSystemGPU() == 0) {
        return 0;
    }
    return GameMain();
}
