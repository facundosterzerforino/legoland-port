/* dbgrun: run the port under a minimal debugger and log every exception with the faulting thread's call stack.
 *
 *   dbgrun.exe <log file> <exe> [args...]      (working directory: the current one)
 *
 * In-process crash reporting can't help when the stack itself is gone (overflow or a corrupted ESP); a debugger
 * sees the first-chance exception from outside. Symbols come from the exe's PDB next to it. */
#include <windows.h>
#include <dbghelp.h>
#include <stdio.h>
#include <string.h>

static FILE *out;
static HANDLE process;
static HANDLE threads[256];
static DWORD thread_ids[256];
static int thread_count;

static HANDLE FindThread(DWORD id) {
    int i;

    for (i = 0; i < thread_count; i++) {
        if (thread_ids[i] == id) {
            return threads[i];
        }
    }
    return NULL;
}

static void Describe(DWORD64 address) {
    char buffer[sizeof(SYMBOL_INFO) + 256];
    SYMBOL_INFO *symbol = (SYMBOL_INFO *)buffer;
    IMAGEHLP_MODULE64 module;
    DWORD64 displacement = 0;
    IMAGEHLP_LINE64 line;
    DWORD line_displacement = 0;

    memset(buffer, 0, sizeof(buffer));
    symbol->SizeOfStruct = sizeof(SYMBOL_INFO);
    symbol->MaxNameLen = 255;
    module.SizeOfStruct = sizeof(module);
    if (!SymGetModuleInfo64(process, address, &module)) {
        strcpy(module.ModuleName, "?");
        module.BaseOfImage = 0;
    }
    fprintf(out, "  %08llx %s+%llx", address, module.ModuleName, address - module.BaseOfImage);
    if (SymFromAddr(process, address, &displacement, symbol)) {
        fprintf(out, "  %s+0x%llx", symbol->Name, displacement);
    }
    line.SizeOfStruct = sizeof(line);
    if (SymGetLineFromAddr64(process, address, &line_displacement, &line)) {
        fprintf(out, "  (%s:%lu)", line.FileName, line.LineNumber);
    }
    fputc('\n', out);
}

static void DumpThread(DWORD thread_id, const char *why) {
    HANDLE thread = FindThread(thread_id);
    CONTEXT context;
    STACKFRAME64 frame;
    DWORD64 previous_sp = 0;
    int n;
    int repeats = 0;
    DWORD64 last_pc = 0;

    fprintf(out, "--- thread %lu (%s)\n", thread_id, why);
    if (thread == NULL) {
        fprintf(out, "  (no handle)\n");
        return;
    }
    context.ContextFlags = CONTEXT_FULL;
    if (!GetThreadContext(thread, &context)) {
        fprintf(out, "  GetThreadContext failed %lu\n", GetLastError());
        return;
    }
    fprintf(out, "  eip %08lx esp %08lx ebp %08lx eax %08lx ebx %08lx ecx %08lx edx %08lx esi %08lx edi %08lx\n",
        context.Eip, context.Esp, context.Ebp, context.Eax, context.Ebx, context.Ecx, context.Edx, context.Esi,
        context.Edi);
    memset(&frame, 0, sizeof(frame));
    frame.AddrPC.Offset = context.Eip;
    frame.AddrPC.Mode = AddrModeFlat;
    frame.AddrFrame.Offset = context.Ebp;
    frame.AddrFrame.Mode = AddrModeFlat;
    frame.AddrStack.Offset = context.Esp;
    frame.AddrStack.Mode = AddrModeFlat;
    for (n = 0; n < 4000; n++) {
        if (!StackWalk64(IMAGE_FILE_MACHINE_I386, process, thread, &frame, &context, NULL, SymFunctionTableAccess64,
                SymGetModuleBase64, NULL)) {
            break;
        }
        if (frame.AddrPC.Offset == 0 || (n > 0 && frame.AddrStack.Offset == previous_sp)) {
            break;
        }
        previous_sp = frame.AddrStack.Offset;
        /* a deep recursion: print the first 60 frames, then only changes */
        if (frame.AddrPC.Offset == last_pc) {
            repeats++;
            continue;
        }
        if (repeats > 0) {
            fprintf(out, "  ... previous frame repeated %d more times\n", repeats);
            repeats = 0;
        }
        last_pc = frame.AddrPC.Offset;
        if (n < 60 || n % 200 == 0) {
            Describe(frame.AddrPC.Offset);
        }
    }
    fprintf(out, "  (%d frames)\n", n);
}

static void LoadModuleSymbols(HANDLE file, void *base) {
    char name[MAX_PATH];
    char *path = name;

    name[0] = 0;
    if (file != NULL && GetFinalPathNameByHandleA(file, name, sizeof(name), 0) > 0) {
        if (strncmp(name, "\\\\?\\", 4) == 0) {
            path = name + 4;
        }
    }
    SymLoadModuleEx(process, file, path[0] ? path : NULL, NULL, (DWORD64)base, 0, NULL, 0);
}

int main(int argc, char **argv) {
    STARTUPINFOA si;
    PROCESS_INFORMATION pi;
    DEBUG_EVENT event;
    char command[4096];
    int i;
    int exceptions = 0;
    DWORD status;

    if (argc < 3) {
        fprintf(stderr, "usage: dbgrun <log> <exe> [args...]\n");
        return 2;
    }
    out = fopen(argv[1], "w");
    if (out == NULL) {
        return 2;
    }
    snprintf(command, sizeof(command), "\"%s\"", argv[2]);
    for (i = 3; i < argc; i++) {
        strncat(command, " ", sizeof(command) - strlen(command) - 1);
        strncat(command, argv[i], sizeof(command) - strlen(command) - 1);
    }
    memset(&si, 0, sizeof(si));
    si.cb = sizeof(si);
    if (!CreateProcessA(NULL, command, NULL, NULL, FALSE, DEBUG_ONLY_THIS_PROCESS, NULL, NULL, &si, &pi)) {
        fprintf(out, "CreateProcess failed %lu\n", GetLastError());
        return 1;
    }
    process = pi.hProcess;
    SymSetOptions(SYMOPT_UNDNAME | SYMOPT_DEFERRED_LOADS | SYMOPT_LOAD_LINES);
    for (;;) {
        if (!WaitForDebugEvent(&event, INFINITE)) {
            break;
        }
        status = DBG_CONTINUE;
        switch (event.dwDebugEventCode) {
        case CREATE_PROCESS_DEBUG_EVENT:
            SymInitialize(process, NULL, FALSE);
            LoadModuleSymbols(event.u.CreateProcessInfo.hFile, event.u.CreateProcessInfo.lpBaseOfImage);
            fprintf(out, "exe loaded at %p\n", event.u.CreateProcessInfo.lpBaseOfImage);
            threads[thread_count] = event.u.CreateProcessInfo.hThread;
            thread_ids[thread_count++] = event.dwThreadId;
            if (event.u.CreateProcessInfo.hFile != NULL) {
                CloseHandle(event.u.CreateProcessInfo.hFile);
            }
            break;
        case CREATE_THREAD_DEBUG_EVENT:
            if (thread_count < 256) {
                threads[thread_count] = event.u.CreateThread.hThread;
                thread_ids[thread_count++] = event.dwThreadId;
            }
            break;
        case LOAD_DLL_DEBUG_EVENT:
            LoadModuleSymbols(event.u.LoadDll.hFile, event.u.LoadDll.lpBaseOfDll);
            if (event.u.LoadDll.hFile != NULL) {
                CloseHandle(event.u.LoadDll.hFile);
            }
            break;
        case EXCEPTION_DEBUG_EVENT: {
            EXCEPTION_RECORD *record = &event.u.Exception.ExceptionRecord;
            DWORD code = record->ExceptionCode;

            if (code == EXCEPTION_BREAKPOINT || code == 0x4000001f /* WOW64 breakpoint */) {
                break; /* the loader's initial breakpoint */
            }
            status = DBG_EXCEPTION_NOT_HANDLED;
            /* ignore the routine first-chance noise (C++/RPC/debug-string exceptions) */
            if (code == 0x406d1388 || code == 0x40010006 || code == 0x4001000a || code == 0xe06d7363 ||
                code == 0x000006ba || code == 0x80000001) {
                break;
            }
            if (exceptions++ < 6) {
                fprintf(out, "=== exception %08lx (%s chance) at %p, info %08lx %08lx\n", code,
                    event.u.Exception.dwFirstChance ? "first" : "second", record->ExceptionAddress,
                    record->NumberParameters > 0 ? (DWORD)record->ExceptionInformation[0] : 0,
                    record->NumberParameters > 1 ? (DWORD)record->ExceptionInformation[1] : 0);
                DumpThread(event.dwThreadId, "faulting");
                if (event.dwThreadId != pi.dwThreadId) {
                    DumpThread(pi.dwThreadId, "main");
                }
                fflush(out);
            }
            break;
        }
        case EXIT_PROCESS_DEBUG_EVENT:
            fprintf(out, "=== process exited, code %08lx\n", event.u.ExitProcess.dwExitCode);
            fclose(out);
            ContinueDebugEvent(event.dwProcessId, event.dwThreadId, DBG_CONTINUE);
            return 0;
        default:
            break;
        }
        ContinueDebugEvent(event.dwProcessId, event.dwThreadId, status);
    }
    fclose(out);
    return 0;
}
