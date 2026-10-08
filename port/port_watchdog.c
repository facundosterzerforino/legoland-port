#include <windows.h>

#include <dbghelp.h>
#include <stdio.h>
#include <string.h>

#include "port_trace.h"
#include "port_watchdog.h"

#define STACK_WORDS 2048
#define MAX_FRAMES 48

static volatile LONG heartbeat;
static const char *volatile current_phase = "startup";
static volatile LONG paused;

static HANDLE main_thread;
static DWORD main_thread_id;
static HANDLE crash_event;
static HANDLE crash_done;
static EXCEPTION_POINTERS *volatile crash_pointers;
static volatile DWORD crash_thread_id;
static volatile LONG crash_claimed;

static char exe_dir[MAX_PATH];
static DWORD module_base;
static DWORD module_size;
static int symbols_ready;

/* Copied from the stopped thread, examined after it runs again: nothing here may take a lock it might hold. */
static CONTEXT report_context;
static DWORD report_stack[STACK_WORDS];
static int report_stack_words;

/* Hardware write watchpoints (debug registers DR0/DR1) on globals that only level loading should write. A write
 * from anywhere else is logged with the writing code's rva and the stack's exe addresses, to find code that
 * overruns its own buffer into them (the map state was wiped this way while placing rides). */
extern void *GameMapRawBlock;
extern unsigned int DAT_00667ca4;
static struct {
    void *address;
    const char *name;
} watches[2];
static volatile LONG watch_reports;
#define MAX_WATCH_REPORTS 40

void PortHeartbeat(void) { InterlockedIncrement(&heartbeat); }

void PortSetPhase(const char *phase) {
    current_phase = phase;
    InterlockedIncrement(&heartbeat);
}

void PortSetPaused(int value) {
    InterlockedExchange(&paused, value);
    InterlockedIncrement(&heartbeat);
}

static int InModule(DWORD address) { return address >= module_base && address < module_base + module_size; }

static void Describe(DWORD address, char *out, size_t size) {
    char buffer[sizeof(SYMBOL_INFO) + 256];
    SYMBOL_INFO *symbol = (SYMBOL_INFO *)buffer;
    DWORD64 displacement = 0;

    out[0] = 0;
    if (!symbols_ready) {
        return;
    }
    memset(buffer, 0, sizeof(buffer));
    symbol->SizeOfStruct = sizeof(SYMBOL_INFO);
    symbol->MaxNameLen = 255;
    if (SymFromAddr(GetCurrentProcess(), address, &displacement, symbol)) {
        snprintf(out, size, "%s+0x%lx", symbol->Name, (unsigned long)displacement);
    }
}

/* Copy the thread's stack from its ESP, stopping at the first unreadable page. */
static void CopyStack(DWORD esp) {
    SIZE_T got = 0;
    int words = STACK_WORDS;

    report_stack_words = 0;
    while (words > 0) {
        if (ReadProcessMemory(GetCurrentProcess(), (void *)esp, report_stack, words * 4, &got)) {
            report_stack_words = words;
            return;
        }
        words /= 2;
    }
}

static DWORD StackWord(DWORD address) {
    DWORD esp = report_context.Esp;

    if (address < esp || address + 4 > esp + (DWORD)report_stack_words * 4 || (address & 3) != 0) {
        return 0;
    }
    return report_stack[(address - esp) / 4];
}

static void TraceAddress(const char *label, DWORD address) {
    char name[300];

    Describe(address, name, sizeof(name));
    if (InModule(address)) {
        PortTrace("  %s rva %08lx %s", label, address - module_base, name);
    } else {
        PortTrace("  %s %08lx %s", label, address, name);
    }
}

static void Report(const char *what) {
    DWORD ebp = report_context.Ebp;
    DWORD ret;
    char label[32];
    int i;

    PortTrace("%s: last phase \"%s\", eip %08lx esp %08lx ebp %08lx", what, current_phase, report_context.Eip,
        report_context.Esp, report_context.Ebp);
    TraceAddress("at  ", report_context.Eip);
    /* frame-pointer chain (the port builds keep frame pointers in most functions) */
    for (i = 0; i < MAX_FRAMES; i++) {
        ret = StackWord(ebp + 4);
        if (ret == 0) {
            break;
        }
        snprintf(label, sizeof(label), "#%-2d ", i + 1);
        TraceAddress(label, ret);
        if (StackWord(ebp) <= ebp) {
            break;
        }
        ebp = StackWord(ebp);
    }
    /* every stack word that points into the exe, like the crash handler prints */
    PortTrace("  stack words into the exe:");
    for (i = 0; i < report_stack_words; i++) {
        if (InModule(report_stack[i])) {
            snprintf(label, sizeof(label), "stack+%04x", i * 4);
            TraceAddress(label, report_stack[i]);
        }
    }
}

static void WriteDump(const char *prefix, DWORD thread_id, EXCEPTION_POINTERS *pointers) {
    MINIDUMP_EXCEPTION_INFORMATION info;
    SYSTEMTIME now;
    char path[MAX_PATH];
    HANDLE file;
    BOOL ok;

    GetLocalTime(&now);
    snprintf(path, sizeof(path), "%s%s-%04d%02d%02d-%02d%02d%02d.dmp", exe_dir, prefix, now.wYear, now.wMonth,
        now.wDay, now.wHour, now.wMinute, now.wSecond);
    file = CreateFileA(path, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (file == INVALID_HANDLE_VALUE) {
        PortTrace("  could not create %s", path);
        return;
    }
    info.ThreadId = thread_id;
    info.ExceptionPointers = pointers;
    info.ClientPointers = FALSE;
    ok = MiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(), file,
        (MINIDUMP_TYPE)(MiniDumpWithDataSegs | MiniDumpWithIndirectlyReferencedMemory | MiniDumpWithThreadInfo |
            MiniDumpWithHandleData),
        pointers != NULL ? &info : NULL, NULL, NULL);
    CloseHandle(file);
    PortTrace("  minidump %s: %s", ok ? "written" : "FAILED", path);
}

/* Set the debug registers on the main thread (a thread can't reliably set its own): DR0/DR1 = the watched
 * addresses, break on 4-byte writes. */
static void ArmWatches(void) {
    CONTEXT ctx;
    int i;

    watches[0].address = &GameMapRawBlock;
    watches[0].name = "GameMapRawBlock";
    watches[1].address = &DAT_00667ca4;
    watches[1].name = "DAT_00667ca4";
    if (SuspendThread(main_thread) == (DWORD)-1) {
        return;
    }
    memset(&ctx, 0, sizeof(ctx));
    ctx.ContextFlags = CONTEXT_DEBUG_REGISTERS;
    if (GetThreadContext(main_thread, &ctx)) {
        ctx.Dr0 = (DWORD)watches[0].address;
        ctx.Dr1 = (DWORD)watches[1].address;
        ctx.Dr7 = (ctx.Dr7 & ~0x00ff000fu) | 0x1 | 0x4 /* L0, L1 */ | (0x1u << 16) | (0x3u << 18) /* DR0: write, 4 bytes */ |
            (0x1u << 20) | (0x3u << 22) /* DR1: write, 4 bytes */;
        if (SetThreadContext(main_thread, &ctx)) {
            for (i = 0; i < 2; i++) {
                PortTrace("watch: armed on %s (%p)", watches[i].name, watches[i].address);
            }
        }
    }
    ResumeThread(main_thread);
}

/* A watched global was written: log the value, the writing code and the stack, then carry on. */
static LONG WatchHit(EXCEPTION_POINTERS *pointers) {
    CONTEXT *ctx = pointers->ContextRecord;
    DWORD *sp = (DWORD *)ctx->Esp;
    int i;
    int shown = 0;

    for (i = 0; i < 2; i++) {
        if ((ctx->Dr6 & (1u << i)) != 0 && InterlockedIncrement(&watch_reports) <= MAX_WATCH_REPORTS) {
            PortTrace("watch: %s = %08lx, written by the instruction before rva %08lx (phase \"%s\")", watches[i].name,
                *(DWORD *)watches[i].address, ctx->Eip - module_base, current_phase);
            for (shown = 0; shown < 16 && (DWORD)(sp + 1) < (DWORD)ctx->Esp + 0x4000; sp++) {
                if (!IsBadReadPtr(sp, 4) && InModule(*sp)) {
                    PortTrace("  stack+%04lx rva %08lx", (DWORD)sp - ctx->Esp, *sp - module_base);
                    shown++;
                }
            }
        }
    }
    ctx->Dr6 = 0;
    return EXCEPTION_CONTINUE_EXECUTION;
}

/* First-chance access violation or stack overflow in the exe: hand it to the watchdog thread, which has a
 * healthy stack, and wait until it has written the report. Then let the game's own handler run as before. */
static LONG CALLBACK FirstChanceHandler(EXCEPTION_POINTERS *pointers) {
    DWORD code = pointers->ExceptionRecord->ExceptionCode;

    if (code == EXCEPTION_SINGLE_STEP && (pointers->ContextRecord->Dr6 & 3) != 0) {
        return WatchHit(pointers);
    }
    if (code != EXCEPTION_ACCESS_VIOLATION && code != EXCEPTION_STACK_OVERFLOW) {
        return EXCEPTION_CONTINUE_SEARCH;
    }
    if (code == EXCEPTION_ACCESS_VIOLATION && !InModule(pointers->ContextRecord->Eip)) {
        return EXCEPTION_CONTINUE_SEARCH;
    }
    if (InterlockedExchange(&crash_claimed, 1) != 0) {
        return EXCEPTION_CONTINUE_SEARCH;
    }
    crash_pointers = pointers;
    crash_thread_id = GetCurrentThreadId();
    SetEvent(crash_event);
    WaitForSingleObject(crash_done, 30000);
    return EXCEPTION_CONTINUE_SEARCH;
}

static BOOL CALLBACK FindVisibleWindow(HWND window, LPARAM out) {
    if (IsWindowVisible(window)) {
        *(HWND *)out = window;
        return FALSE;
    }
    return TRUE;
}

/* Windows' own "not responding": the main thread hasn't taken messages for 5 s. Catches loops that keep the
 * heartbeat going (DrawWatchSprite) but never pump messages. */
static int WindowHung(void) {
    HWND window = NULL;

    EnumThreadWindows(main_thread_id, FindVisibleWindow, (LPARAM)&window);
    return window != NULL && IsHungAppWindow(window);
}

static int SampleMainThread(void) {
    if (SuspendThread(main_thread) == (DWORD)-1) {
        return 0;
    }
    report_context.ContextFlags = CONTEXT_FULL;
    GetThreadContext(main_thread, &report_context);
    CopyStack(report_context.Esp);
    ResumeThread(main_thread);
    return 1;
}

static DWORD WINAPI WatchdogThread(LPVOID unused) {
    DWORD gdi_logged = GetTickCount() - 300000; /* log once at start */
    LONG last = heartbeat;
    DWORD since = GetTickCount();
    int reported = 0;
    int dumped = 0;
    int hung;
    int i;
    DWORD stalled;
    EXCEPTION_RECORD *record;

    (void)unused;
    ArmWatches();
    SymSetOptions(SYMOPT_UNDNAME | SYMOPT_DEFERRED_LOADS);
    symbols_ready = SymInitialize(GetCurrentProcess(), exe_dir, TRUE);
    for (;;) {
        /* every 5 minutes: the game's GDI and USER object counts, to catch handle leaks (Windows refuses new GDI
         * objects at 10000; the info popup's text measuring leaked two DCs per popup in the original) */
        if (GetTickCount() - gdi_logged >= 300000) {
            gdi_logged = GetTickCount();
            PortTrace("gdi: %lu GDI objects, %lu USER objects", GetGuiResources(GetCurrentProcess(), GR_GDIOBJECTS),
                GetGuiResources(GetCurrentProcess(), GR_USEROBJECTS));
        }
        if (WaitForSingleObject(crash_event, 250) == WAIT_OBJECT_0) {
            record = crash_pointers->ExceptionRecord;
            report_context = *crash_pointers->ContextRecord;
            CopyStack(report_context.Esp);
            PortTrace("CRASH: exception %08lx at %08lx (thread %lu%s), address %08lx", record->ExceptionCode,
                (DWORD)record->ExceptionAddress, crash_thread_id, crash_thread_id == main_thread_id ? ", main" : "",
                record->NumberParameters > 1 ? (DWORD)record->ExceptionInformation[1] : 0);
            Report("CRASH");
            WriteDump("crash", crash_thread_id, crash_pointers);
            SetEvent(crash_done);
            continue;
        }
        hung = !paused && WindowHung();
        if (!hung && (paused || heartbeat != last)) {
            last = heartbeat;
            since = GetTickCount();
            reported = 0;
            dumped = 0;
            continue;
        }
        stalled = GetTickCount() - since;
        if (!hung && stalled < PORT_WATCHDOG_FREEZE_MS) {
            continue;
        }
        if (!reported) {
            reported = 1;
            if (hung) {
                PortTrace("FREEZE detected: the window stopped responding (heartbeat %s)",
                    heartbeat != last ? "still moving: a loop that doesn't pump messages" : "stopped");
            } else {
                PortTrace("FREEZE detected: no heartbeat for %lu ms", stalled);
            }
            /* three samples, so a loop shows where it spins */
            for (i = 0; i < 3; i++) {
                if (!SampleMainThread()) {
                    break;
                }
                Report(i == 0 ? "FREEZE" : "FREEZE (again)");
                Sleep(300);
            }
        }
        if (!dumped && (hung || stalled >= PORT_WATCHDOG_DUMP_MS)) {
            dumped = 1;
            WriteDump("freeze", main_thread_id, NULL);
        }
    }
    return 0;
}

void PortWatchdogStart(void) {
    IMAGE_DOS_HEADER *dos;
    IMAGE_NT_HEADERS *nt;
    char *slash;

    module_base = (DWORD)GetModuleHandleA(NULL);
    dos = (IMAGE_DOS_HEADER *)module_base;
    nt = (IMAGE_NT_HEADERS *)(module_base + dos->e_lfanew);
    module_size = nt->OptionalHeader.SizeOfImage;
    GetModuleFileNameA(NULL, exe_dir, sizeof(exe_dir));
    slash = strrchr(exe_dir, '\\');
    if (slash != NULL) {
        slash[1] = 0;
    }
    DuplicateHandle(GetCurrentProcess(), GetCurrentThread(), GetCurrentProcess(), &main_thread, 0, FALSE,
        DUPLICATE_SAME_ACCESS);
    main_thread_id = GetCurrentThreadId();
    crash_event = CreateEventA(NULL, FALSE, FALSE, NULL);
    crash_done = CreateEventA(NULL, FALSE, FALSE, NULL);
    AddVectoredExceptionHandler(1, FirstChanceHandler);
    CloseHandle(CreateThread(NULL, 0, WatchdogThread, NULL, 0, NULL));
}
