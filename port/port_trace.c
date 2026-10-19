#include <windows.h>

#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "port_trace.h"

/* the game's string table (src/legoland/string.c, globals.c) */
extern void *strings[10];
char *GetString(int n);

static FILE *trace_file;
static int trace_tried;

static FILE *OpenTraceFile(void) {
    char path[MAX_PATH];
    char prev[MAX_PATH];
    char *slash;

    if (trace_tried) {
        return trace_file;
    }
    trace_tried = 1;
    if (GetModuleFileNameA(NULL, path, sizeof(path)) == 0) {
        return NULL;
    }
    slash = strrchr(path, '\\');
    if (slash == NULL || (strrchr(path, '/') != NULL && strrchr(path, '/') > slash)) {
        slash = strrchr(path, '/'); /* off Windows the program's path has slashes */
    }
    if (slash == NULL) {
        return NULL;
    }
    lstrcpyA(slash + 1, "legoland-port-trace-prev.txt");
    lstrcpyA(prev, path);
    strcpy(slash + 1, "legoland-port-trace.txt");
    /* keep the previous run's trace (a crash report is often in it) */
    MoveFileExA(path, prev, MOVEFILE_REPLACE_EXISTING);
    trace_file = fopen(path, "w");
    return trace_file;
}

/* The game's string table, watched: whole after LoadStringTable (the selftest checks it), and texts went missing in
 * the browser (GetString returning NULL: the exit dialog, the notebooks, the info popups' "(null)"). After each line
 * the trace checks it, and reports once where it was damaged: the line before is the last step it was whole. */
static int strings_whole;
static int strings_reported;

static void WatchStrings(FILE *f) {
    int slots = 0;
    int i;
    int whole;
    char line[200];

    for (i = 0; i < 10; i++) {
        slots += strings[i] != NULL;
    }
    whole = slots == 10 && GetString(0x89) != NULL && GetString(0x28a) != NULL;
    if (whole) {
        strings_whole = 1;
        return;
    }
    if (!strings_whole || strings_reported || slots == 0) { /* none: freed at the end (DeleteStrings) */
        return;
    }
    strings_reported = 1;
    snprintf(line, sizeof(line), "strings: the table was damaged since the line before (%d of 10 slots, EXIT %s, tutorial "
                                 "heading %s)",
        slots, GetString(0x89) != NULL ? "there" : "missing", GetString(0x28a) != NULL ? "there" : "missing");
    if (f != NULL) {
        fprintf(f, "[%8lu] %s\n", (unsigned long)GetTickCount(), line);
        fflush(f);
    }
#ifdef __EMSCRIPTEN__
    fprintf(stderr, "[%8lu] %s\n", GetTickCount(), line);
#endif
}

void PortTraceV(const char *fmt, va_list ap) {
    FILE *f = OpenTraceFile();
    char line[1024];

    vsnprintf(line, sizeof(line), fmt, ap);
    line[sizeof(line) - 1] = 0;
    OutputDebugStringA(line);
    OutputDebugStringA("\n");
    if (f != NULL) {
        fprintf(f, "[%8lu] %s\n", (unsigned long)GetTickCount(), line);
        fflush(f);
    }
#ifdef __EMSCRIPTEN__
    /* the web: the file is in the page's storage, out of sight; the browser's console shows the lines too */
    fprintf(stderr, "[%8lu] %s\n", GetTickCount(), line);
#endif
    WatchStrings(f);
}

void PortTrace(const char *fmt, ...) {
    va_list ap;

    va_start(ap, fmt);
    PortTraceV(fmt, ap);
    va_end(ap);
}

unsigned long PortRva(const void *address) {
    return (unsigned long)((uintptr_t)address - (uintptr_t)GetModuleHandleA(NULL));
}
