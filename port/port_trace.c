#include <windows.h>

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "port_trace.h"

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

void PortTraceV(const char *fmt, va_list ap) {
    FILE *f = OpenTraceFile();
    char line[1024];

    vsnprintf(line, sizeof(line), fmt, ap);
    line[sizeof(line) - 1] = 0;
    OutputDebugStringA(line);
    OutputDebugStringA("\n");
    if (f != NULL) {
        fprintf(f, "[%8lu] %s\n", GetTickCount(), line);
        fflush(f);
    }
}

void PortTrace(const char *fmt, ...) {
    va_list ap;

    va_start(ap, fmt);
    PortTraceV(fmt, ap);
    va_end(ap);
}
