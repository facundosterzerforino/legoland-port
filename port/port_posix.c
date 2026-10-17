/* [port] The port's pieces that are Windows-only (port_watchdog.c, port_setup_win.c), off Windows: what they keep
 * track of, without its thread (a freeze isn't reported off Windows). And the program's entry point: main calls the
 * game's WinMain, as Windows does. (The setup off Windows: port_setup_sdl.c; the crash report: the game's own,
 * through miniwin/src/posix/seh.c.) */
#include <windows.h>
#include <direct.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#if defined(__ANDROID__) || defined(LEGOLAND_WEB_BROWSER)
#include <sys/stat.h>
#include <unistd.h>

#include "port_setup.h"
#endif
#ifdef LEGOLAND_WEB_BROWSER
#include <emscripten/wasmfs.h>
#endif

#include <SDL3/SDL_main.h> /* Android: main is the SDL_main its Java side calls */

#include "miniwin/miniwin.h"
#include "port_trace.h"
#include "port_watchdog.h"

static void TraceLine(const char *line) {
    PortTrace("%s", line);
}

void PosixThreadSignalStack(void); /* miniwin/src/posix/seh.c */

static volatile int heartbeat;
static const char *volatile current_phase;
static volatile int paused;

void PortWatchdogStart(void) {
    /* no watchdog thread yet: a freeze isn't reported off Windows */
}

void PortHeartbeat(void) {
    __atomic_add_fetch(&heartbeat, 1, __ATOMIC_RELAXED);
}

void PortSetPhase(const char *phase) {
    current_phase = phase;
    __atomic_add_fetch(&heartbeat, 1, __ATOMIC_RELAXED);
}

void PortSetPaused(int value) {
    __atomic_store_n(&paused, value, __ATOMIC_RELAXED);
    __atomic_add_fetch(&heartbeat, 1, __ATOMIC_RELAXED);
}

void PortWatchRequest(int slot, void *address, const char *name) {
    (void)slot;
    (void)address;
    (void)name; /* debug registers: Windows only */
}

int main(int argc, char **argv) {
    /* WinMain's command line: the arguments after the program's name, separated by spaces */
    size_t size = 1;
    char *line;
    int i;
    int result;

    PosixThreadSignalStack(); /* the crash report runs on it after a stack overflow (main.c's __except) */
#ifdef LEGOLAND_WEB_BROWSER
    /* the page's origin's private storage, where the game's files stay between visits: /opfs */
    wasmfs_create_directory("/opfs", 0777, wasmfs_create_opfs_backend());
#endif
#if defined(__ANDROID__) || defined(LEGOLAND_WEB_BROWSER)
    /* an app (or a page) starts in /: the game's folder is its own (port_setup.c), as the current one is the game's
     * folder when it's started from it elsewhere */
    if (_chdir(PortSetupExeFolder()) != 0) {
        mkdir(PortSetupExeFolder(), 0755);
        _chdir(PortSetupExeFolder());
    }
#endif
    for (i = 1; i < argc; i++) {
        size += strlen(argv[i]) + 1;
    }
    line = (char *)malloc(size);
    if (line == NULL) {
        return 1;
    }
    line[0] = '\0';
    for (i = 1; i < argc; i++) {
        if (i > 1) {
            strcat(line, " ");
        }
        strcat(line, argv[i]);
    }
    MiniwinTrace = TraceLine; /* miniwin's reports in the port's trace */
    result = WinMain(GetModuleHandleA(NULL), NULL, line, SW_SHOWNORMAL);
    free(line);
#ifdef LEGOLAND_WEB_BROWSER
    /* the page goes on after the game: its sound with it, unless SDL lets go of the browser's audio (the music played
     * on after the game ended) */
    SDL_Quit();
#endif
    return result;
}
