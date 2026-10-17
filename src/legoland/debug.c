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
#include "port_display.h"
#include "port_trace.h"
#include "port_watchdog.h"
#endif

// FUNCTION: LEGOLAND 0x0047f870
void DebugTrace(const char *fmt, ...) {
#ifdef LEGOLAND_PORT
    /* [port] the original compiled its traces out; the port writes them to legoland-port-trace.txt */
    va_list ap;

    PortSetPhase(fmt);
    va_start(ap, fmt);
    PortTraceV(fmt, ap);
    va_end(ap);
#endif
}

// Strings used by GameMain. They are named arrays rather than literals so the MSVC6 __asm version
// below can push their addresses (inline asm cannot take the address of a string literal).
// GLOBAL: LEGOLAND 0x004bcd44
static const char GameLogFileName[] = "legoland.log";
// GLOBAL: LEGOLAND 0x004bcd28
static const char ResourceOpenErrorFormat[] = "Failed to open resource %s";
// GLOBAL: LEGOLAND 0x004bcd18
static const char ErrorBoxTitle[] = "LEGOLAND Error";
// GLOBAL: LEGOLAND 0x004bcd08
static const char EraseItSpriteName[] = "erase it.lls";
// GLOBAL: LEGOLAND 0x004bccf8
static const char EraseIt2SpriteName[] = "erase it2.lls";
// GLOBAL: LEGOLAND 0x004bcce8
static const char NoBuildSpriteName[] = "no build.lls";
// GLOBAL: LEGOLAND 0x004bccd8
static const char YesBuildSpriteName[] = "yes build.lls";
// GLOBAL: LEGOLAND 0x004bccc4
static const char RabOverIconSpriteName[] = "rab over icon.lls";
// GLOBAL: LEGOLAND 0x004bccb0
static const char RabOverIcon2SpriteName[] = "rab over icon2.lls";
// GLOBAL: LEGOLAND 0x004bcca0
static const char QuestionItSpriteName[] = "question it.lls";
// GLOBAL: LEGOLAND 0x004bcc8c
static const char QuestionIt2SpriteName[] = "question it2.lls";
// GLOBAL: LEGOLAND 0x004bcc78
static const char AttractionsMenuName[] = "ATTRACTIONS MENU";
// GLOBAL: LEGOLAND 0x004bcc64
static const char FoodStoresMenuName[] = "FOOD STORES MENU";
// GLOBAL: LEGOLAND 0x004bcc54
static const char SceneryMenuName[] = "SCENERY MENU";
// GLOBAL: LEGOLAND 0x004bcc48
static const char ShopsMenuName[] = "SHOPS MENU";
// GLOBAL: LEGOLAND 0x004bcc28
static const char ShutdownTraceMessage[] = "Finished shutting stuff down";

// GameMain: the game's top level. Opens the log, checks the CD, opens the three resource volumes,
// starts graphics and input, loads the shared pointer sprites and registers the build-menu
// elements, runs the game (FUN_00459520), then shuts everything down again.
// Returns 0 on a normal exit, 1 if any start-up step failed.
//
// The MSVC6 build uses an exact __asm transcription of the original. The C below (the #else
// branch, built by the port) is the same function, and the asm is commented block by block
// against it. Why asm: every C form of the "open the three volumes" loop compiles to a signed
// `jl` at its bottom, but the original has an unsigned `jb`; tried int/unsigned/char/short
// counters, sizeof and 3u bounds, casts, do/while, pointer loops and per-file compiler options
// (/O1 /Ox /Os /Ot /Ob0 /Ob1 /Oy- /Op /Oa /Ow /G3-/G6 /GB /Gy). That one instruction is the only
// difference (99.62% in C).

// clang-format off
#if defined(_MSC_VER) && (_MSC_VER <= 1200) && defined(_M_IX86)
// FUNCTION: LEGOLAND 0x0047f880
__declspec(naked) int GameMain(void) {
    __asm {
        // char buffer[1024] sits at [esp + 8] once esi and edi are pushed.
        // lpConfig->field_1e = 1; if (OpenLogFile("legoland.log") == 0) return 1;
        mov eax, lpConfig
        sub esp, 0x400
        push esi
        push edi
        push offset GameLogFileName
        mov word ptr [eax].field_1e, 1
        call OpenLogFile
        add esp, 4
        test eax, eax
        jne log_opened
        pop edi
        mov eax, 1
        pop esi
        add esp, 0x400
        ret

    log_opened:
        // if (WaitForLegolandCd(1) == 0) return 1;
        push 1
        call WaitForLegolandCd
        add esp, 4
        test eax, eax
        jne cd_found
        pop edi
        mov eax, 1
        pop esi
        add esp, 0x400
        ret

    cd_found:
        // edi = 0 (NULL) for the rest of the function; esi = i.
        // for (i = 0; i < 3; i++) {
        //     ResourceVolumes[i] = RES_OpenVolume(ResourceFileNames[i]);
        //     if (ResourceVolumes[i] == NULL) goto open_failed;
        // }
        xor edi, edi
        xor esi, esi
    open_next_volume:
        mov ecx, dword ptr ResourceFileNames[esi * 4]
        push ecx
        call RES_OpenVolume
        add esp, 4
        cmp eax, edi
        mov dword ptr ResourceVolumes[esi * 4], eax
        je open_failed
        inc esi
        cmp esi, 3
        jb open_next_volume // the unsigned compare that C cannot produce here

        // LoadStringTable(); InitHostSystemGPU();
        // if (InitScreen() == 0) {
        call LoadStringTable
        call InitHostSystemGPU
        call InitScreen
        test eax, eax
        jne screen_ready
        //     MessageBoxA(GetDesktopWindow(), GetString(0xcc), GetString(0xcb), 0x30);
        push 0x30
        push 0xcb
        call GetString
        add esp, 4
        push eax
        push 0xcc
        call GetString
        add esp, 4
        push eax
        call dword ptr [GetDesktopWindow]
        push eax
        call dword ptr [MessageBoxA]
        //     KillHostSystemGPU();
        //     for (vol = ResourceVolumes; vol < ResourceVolumes + 3; vol++) RES_CloseVolume(*vol);
        //     return 1;
        call KillHostSystemGPU
        mov esi, offset ResourceVolumes
    screen_failed_close_next:
        mov eax, dword ptr [esi]
        push eax
        call RES_CloseVolume
        add esi, 4
        add esp, 4
        cmp esi, offset ResourceVolumes + 3 * 4
        jb screen_failed_close_next
        pop edi
        mov eax, 1
        pop esi
        add esp, 0x400
        ret
        // }

    open_failed:
        // sprintf(buffer, "Failed to open resource %s", ResourceFileNames[i]);
        // MessageBoxA(GetDesktopWindow(), buffer, "LEGOLAND Error", 0x30);
        mov edx, dword ptr ResourceFileNames[esi * 4]
        lea eax, [esp + 8]
        push edx
        push offset ResourceOpenErrorFormat
        push eax
        call sprintf
        add esp, 0xc
        lea ecx, [esp + 8]
        push 0x30
        push offset ErrorBoxTitle
        push ecx
        call dword ptr [GetDesktopWindow]
        push eax
        call dword ptr [MessageBoxA]
        // for (j = 0; j < i; j++) RES_CloseVolume(ResourceVolumes[j]);  (esi counts down)
        // return 1;
        cmp esi, edi
        jle open_failed_done
        mov edi, offset ResourceVolumes
    open_failed_close_next:
        mov edx, dword ptr [edi]
        push edx
        call RES_CloseVolume
        add esp, 4
        add edi, 4
        dec esi
        jne open_failed_close_next
    open_failed_done:
        pop edi
        mov eax, 1
        pop esi
        add esp, 0x400
        ret

    screen_ready:
        // if (InitInputSystem() == 0) {
        //     MessageBoxA(GetDesktopWindow(), GetString(0x9c4), GetString(0xcb), 0x30);
        //     KillInputSystem(); KillHostSystemGPU();
        //     close the three volumes; return 1;
        // }
        call InitInputSystem
        test eax, eax
        jne input_ready
        push 0x30
        push 0xcb
        call GetString
        add esp, 4
        push eax
        push 0x9c4
        call GetString
        add esp, 4
        push eax
        call dword ptr [GetDesktopWindow]
        push eax
        call dword ptr [MessageBoxA]
        call KillInputSystem
        call KillHostSystemGPU
        mov esi, offset ResourceVolumes
    input_failed_close_next:
        mov ecx, dword ptr [esi]
        push ecx
        call RES_CloseVolume
        add esi, 4
        add esp, 4
        cmp esi, offset ResourceVolumes + 3 * 4
        jb input_failed_close_next
        pop edi
        mov eax, 1
        pop esi
        add esp, 0x400
        ret

    input_ready:
        // PointerSprites[0] = 0; PointerSprites[1..8] = LoadSprite(<name>, 0);
        // (the eight calls share one stack cleanup; PointerSprites + 4 * n is PointerSprites[n])
        push edi
        push offset EraseItSpriteName
        mov dword ptr PointerSprites, edi
        call LoadSprite
        push edi
        push offset EraseIt2SpriteName
        mov dword ptr PointerSprites + 4 * 1, eax
        call LoadSprite
        push edi
        push offset NoBuildSpriteName
        mov dword ptr PointerSprites + 4 * 2, eax
        call LoadSprite
        push edi
        push offset YesBuildSpriteName
        mov dword ptr PointerSprites + 4 * 3, eax
        call LoadSprite
        push edi
        push offset RabOverIconSpriteName
        mov dword ptr PointerSprites + 4 * 4, eax
        call LoadSprite
        push edi
        push offset RabOverIcon2SpriteName
        mov dword ptr PointerSprites + 4 * 5, eax
        call LoadSprite
        push edi
        push offset QuestionItSpriteName
        mov dword ptr PointerSprites + 4 * 6, eax
        call LoadSprite
        push edi
        push offset QuestionIt2SpriteName
        mov dword ptr PointerSprites + 4 * 7, eax
        call LoadSprite
        add esp, 0x40
        mov dword ptr PointerSprites + 4 * 8, eax

        // LLIDB_LoadICM();
        // LLIDB_RegisterNewElement(<menu name>, 0, 0x200) for the five build menus
        call LLIDB_LoadICM
        push 0x200
        push edi
        push offset BuildMenuName
        call LLIDB_RegisterNewElement
        push 0x200
        push edi
        push offset AttractionsMenuName
        call LLIDB_RegisterNewElement
        push 0x200
        push edi
        push offset FoodStoresMenuName
        call LLIDB_RegisterNewElement
        push 0x200
        push edi
        push offset SceneryMenuName
        call LLIDB_RegisterNewElement
        push 0x200
        push edi
        push offset ShopsMenuName
        call LLIDB_RegisterNewElement
        add esp, 0x3c

        // Run the game, then shut down:
        // FUN_00459520(); KillHostSystemGPU();
        // for (vol = ResourceVolumes; vol < ResourceVolumes + 3; vol++) if (*vol != 0) RES_CloseVolume(*vol);
        call FUN_00459520
        call KillHostSystemGPU
        mov esi, offset ResourceVolumes
    shutdown_close_next:
        mov eax, dword ptr [esi]
        cmp eax, edi
        je shutdown_volume_closed
        push eax
        call RES_CloseVolume
        add esp, 4
    shutdown_volume_closed:
        add esi, 4
        cmp esi, offset ResourceVolumes + 3 * 4
        jb shutdown_close_next

        // DebugTrace("Finished shutting stuff down"); CloseLogFile(); DeleteStrings(); LLIDB_CloseICM();
        push offset ShutdownTraceMessage
        call DebugTrace
        add esp, 4
        call CloseLogFile
        call DeleteStrings
        call LLIDB_CloseICM

        // Free the pointer sprites: if (PointerSprites[n] != 0) { KillSprite(PointerSprites[n]); PointerSprites[n] = 0; }
        // in the order 3, 4, 7, 8, 1, 2, 5, 6.
        mov eax, dword ptr PointerSprites + 4 * 3
        cmp eax, edi
        je sprite3_freed
        push eax
        call KillSprite
        add esp, 4
        mov dword ptr PointerSprites + 4 * 3, edi
    sprite3_freed:
        mov eax, dword ptr PointerSprites + 4 * 4
        cmp eax, edi
        je sprite4_freed
        push eax
        call KillSprite
        add esp, 4
        mov dword ptr PointerSprites + 4 * 4, edi
    sprite4_freed:
        mov eax, dword ptr PointerSprites + 4 * 7
        cmp eax, edi
        je sprite7_freed
        push eax
        call KillSprite
        add esp, 4
        mov dword ptr PointerSprites + 4 * 7, edi
    sprite7_freed:
        mov eax, dword ptr PointerSprites + 4 * 8
        cmp eax, edi
        je sprite8_freed
        push eax
        call KillSprite
        add esp, 4
        mov dword ptr PointerSprites + 4 * 8, edi
    sprite8_freed:
        mov eax, dword ptr PointerSprites + 4 * 1
        cmp eax, edi
        je sprite1_freed
        push eax
        call KillSprite
        add esp, 4
        mov dword ptr PointerSprites + 4 * 1, edi
    sprite1_freed:
        mov eax, dword ptr PointerSprites + 4 * 2
        cmp eax, edi
        je sprite2_freed
        push eax
        call KillSprite
        add esp, 4
        mov dword ptr PointerSprites + 4 * 2, edi
    sprite2_freed:
        mov eax, dword ptr PointerSprites + 4 * 5
        cmp eax, edi
        je sprite5_freed
        push eax
        call KillSprite
        add esp, 4
        mov dword ptr PointerSprites + 4 * 5, edi
    sprite5_freed:
        mov eax, dword ptr PointerSprites + 4 * 6
        cmp eax, edi
        je sprite6_freed
        push eax
        call KillSprite
        add esp, 4
        mov dword ptr PointerSprites + 4 * 6, edi
    sprite6_freed:
        // return 0;
        pop edi
        xor eax, eax
        pop esi
        add esp, 0x400
        ret
    }
}
// clang-format on
#else
int GameMain(void) {
    char buffer[1024];
    int i;
    int j;
    struct ResVolume **vol;

    lpConfig->field_1e = 1;

    if (OpenLogFile(GameLogFileName) == 0) {
        return 1;
    }
    if (WaitForLegolandCd(1) == 0) {
        DebugTrace("GameMain: no CD");
        return 1;
    }

    for (i = 0; i < 3; i++) {
        ResourceVolumes[i] = RES_OpenVolume(ResourceFileNames[i]);
        if (ResourceVolumes[i] == NULL) {
            sprintf(buffer, ResourceOpenErrorFormat, ResourceFileNames[i]);
            DebugTrace("GameMain: %s", buffer);
            MessageBoxA(GetDesktopWindow(), buffer, ErrorBoxTitle, 0x30);
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
    PointerSprites[1] = LoadSprite(EraseItSpriteName, 0);
    DebugTrace("GameMain: cursor 1 loaded");
    PointerSprites[2] = LoadSprite(EraseIt2SpriteName, 0);
    DebugTrace("GameMain: cursor 2 loaded");
    PointerSprites[3] = LoadSprite(NoBuildSpriteName, 0);
    DebugTrace("GameMain: cursor 3 loaded");
    PointerSprites[4] = LoadSprite(YesBuildSpriteName, 0);
    DebugTrace("GameMain: cursor 4 loaded");
    PointerSprites[5] = LoadSprite(RabOverIconSpriteName, 0);
    DebugTrace("GameMain: cursor 5 loaded");
    PointerSprites[6] = LoadSprite(RabOverIcon2SpriteName, 0);
    DebugTrace("GameMain: cursor 6 loaded");
    PointerSprites[7] = LoadSprite(QuestionItSpriteName, 0);
    DebugTrace("GameMain: cursor 7 loaded");
    PointerSprites[8] = LoadSprite(QuestionIt2SpriteName, 0);
    DebugTrace("GameMain: cursor 8 loaded");

    DebugTrace("GameMain: cursors loaded");
    LLIDB_LoadICM();
    DebugTrace("GameMain: ICM loaded");

    LLIDB_RegisterNewElement(BuildMenuName, 0, 0x200);
    LLIDB_RegisterNewElement(AttractionsMenuName, 0, 0x200);
    LLIDB_RegisterNewElement(FoodStoresMenuName, 0, 0x200);
    LLIDB_RegisterNewElement(SceneryMenuName, 0, 0x200);
    LLIDB_RegisterNewElement(ShopsMenuName, 0, 0x200);

    DebugTrace("GameMain: init done, entering FUN_00459520");
    FUN_00459520();

    DebugTrace("GameMain: FUN_00459520 returned");
    KillHostSystemGPU();

    for (vol = ResourceVolumes; vol < ResourceVolumes + 3; vol++) {
        if (*vol != 0) {
            RES_CloseVolume(*vol);
        }
    }

    DebugTrace(ShutdownTraceMessage);

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
#endif

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
#ifdef LEGOLAND_PORT
    if (TRUE) { /* [library:video] always: the SDL3 window is shown in a window or full screen (port_display.h) */
#else
    if (StrIStr(lpCmdLine, "WINDEBUG")) {
#endif
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
