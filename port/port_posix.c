/* [port] The port's pieces that are Windows-only (port_watchdog.c, port_setup_win.c), off Windows: what they keep
 * track of, without the parts still to come (ROADMAP.md phase 8 step 4.5: the crash report from signals, the setup
 * as an SDL screen). And the program's entry point: main calls the game's WinMain, as Windows does. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "port_setup_win.h"
#include "port_watchdog.h"

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

int PortSetupRun(HINSTANCE instance) {
    (void)instance;
    fprintf(stderr,
        "LEGOLAND: the game's data isn't here. Run the game from its folder (an install of the CD), or put this\n"
        "program next to it. (Installing from the CD on Linux is still to come.)\n");
    return 0;
}

int main(int argc, char **argv) {
    /* WinMain's command line: the arguments after the program's name, separated by spaces */
    size_t size = 1;
    char *line;
    int i;
    int result;

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
    result = WinMain(GetModuleHandleA(NULL), NULL, line, SW_SHOWNORMAL);
    free(line);
    return result;
}
