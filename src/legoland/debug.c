#include <windows.h>

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

// FUNCTION: LEGOLAND 0x0047f870
void DebugTrace(const char *fmt, ...) {}

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
        return 1;
    }

    for (i = 0; i < 3; i++) {
        ResourceVolumes[i] = RES_OpenVolume(ResourceFileNames[i]);
        if (ResourceVolumes[i] == NULL) {
            // STRING: LEGOLAND 0x004bcd28
            sprintf(buffer, "Failed to open resource %s", ResourceFileNames[i]);
            // STRING: LEGOLAND 0x004bcd18
            MessageBoxA(GetDesktopWindow(), buffer, "LEGOLAND Error", 0x30);
            for (j = 0; j < i; j++) {
                RES_CloseVolume(ResourceVolumes[j]);
            }
            return 1;
        }
    }

    LoadStringTable();
    InitHostSystemGPU();
    if (InitScreen() == 0) {
        MessageBoxA(GetDesktopWindow(), GetString(0xcc), GetString(0xcb), 0x30);
        KillHostSystemGPU();
        for (vol = ResourceVolumes; vol < ResourceVolumes + 3; vol++) {
            RES_CloseVolume(*vol);
        }
        return 1;
    }

    if (InitInputSystem() == 0) {
        MessageBoxA(GetDesktopWindow(), GetString(0x9c4), GetString(0xcb), 0x30);
        KillInputSystem();
        KillHostSystemGPU();
        for (vol = ResourceVolumes; vol < ResourceVolumes + 3; vol++) {
            RES_CloseVolume(*vol);
        }
        return 1;
    }

    DAT_007fe9c0[0] = 0;
    // STRING: LEGOLAND 0x004bcd08
    DAT_007fe9c0[1] = LoadSprite("erase it.lls", 0);
    // STRING: LEGOLAND 0x004bccf8
    DAT_007fe9c0[2] = LoadSprite("erase it2.lls", 0);
    // STRING: LEGOLAND 0x004bcce8
    DAT_007fe9c0[3] = LoadSprite("no build.lls", 0);
    // STRING: LEGOLAND 0x004bccd8
    DAT_007fe9c0[4] = LoadSprite("yes build.lls", 0);
    // STRING: LEGOLAND 0x004bccc4
    DAT_007fe9c0[5] = LoadSprite("rab over icon.lls", 0);
    // STRING: LEGOLAND 0x004bccb0
    DAT_007fe9c0[6] = LoadSprite("rab over icon2.lls", 0);
    // STRING: LEGOLAND 0x004bcca0
    DAT_007fe9c0[7] = LoadSprite("question it.lls", 0);
    // STRING: LEGOLAND 0x004bcc8c
    DAT_007fe9c0[8] = LoadSprite("question it2.lls", 0);

    LLIDB_LoadICM();

    LLIDB_RegisterNewElement("BUILD MENU", 0, 0x200);
    // STRING: LEGOLAND 0x004bcc78
    LLIDB_RegisterNewElement("ATTRACTIONS MENU", 0, 0x200);
    // STRING: LEGOLAND 0x004bcc64
    LLIDB_RegisterNewElement("FOOD STORES MENU", 0, 0x200);
    // STRING: LEGOLAND 0x004bcc54
    LLIDB_RegisterNewElement("SCENERY MENU", 0, 0x200);
    // STRING: LEGOLAND 0x004bcc48
    LLIDB_RegisterNewElement("SHOPS MENU", 0, 0x200);

    FUN_00459520();

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

    if (DAT_007fe9c0[3] != 0) {
        KillSprite(DAT_007fe9c0[3]);
        DAT_007fe9c0[3] = 0;
    }
    if (DAT_007fe9c0[4] != 0) {
        KillSprite(DAT_007fe9c0[4]);
        DAT_007fe9c0[4] = 0;
    }
    if (DAT_007fe9c0[7] != 0) {
        KillSprite(DAT_007fe9c0[7]);
        DAT_007fe9c0[7] = 0;
    }
    if (DAT_007fe9c0[8] != 0) {
        KillSprite(DAT_007fe9c0[8]);
        DAT_007fe9c0[8] = 0;
    }
    if (DAT_007fe9c0[1] != 0) {
        KillSprite(DAT_007fe9c0[1]);
        DAT_007fe9c0[1] = 0;
    }
    if (DAT_007fe9c0[2] != 0) {
        KillSprite(DAT_007fe9c0[2]);
        DAT_007fe9c0[2] = 0;
    }
    if (DAT_007fe9c0[5] != 0) {
        KillSprite(DAT_007fe9c0[5]);
        DAT_007fe9c0[5] = 0;
    }
    if (DAT_007fe9c0[6] != 0) {
        KillSprite(DAT_007fe9c0[6]);
        DAT_007fe9c0[6] = 0;
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
        DAT_004b9ca4 = BlitFrameToWindow;
        DAT_00667d6c = 1;
        // STRING: LEGOLAND 0x004bcd6c
    } else if (StrIStr(lpCmdLine, "BLT")) {
        DAT_004b9ca4 = BlitFrameToWindow;
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
