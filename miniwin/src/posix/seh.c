/* [port] Structured exception handling off Windows (win32_prelude.h's __try and __except): each __try pushes a
 * sigsetjmp'd frame onto its thread's stack of them; a fault (SIGSEGV, SIGBUS, SIGFPE, SIGILL) goes back to the
 * innermost one with Windows' description of it (the exception record, and the context from the signal's), and the
 * __except filter decides: handle it there, or pass it to the next frame out, and with none left the program dies
 * of the signal as it would have. Each thread gets an alternate signal stack, so that a stack overflow is reported
 * too. A __try body must not leave with return, break or goto (none of the code's does). */
#include <setjmp.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#ifndef __EMSCRIPTEN__
#include <ucontext.h>
#endif

#include "posix.h"

#define MAX_FRAMES 32

struct TryState {
    sigjmp_buf frames[MAX_FRAMES];
    int depth;
    int signal;
    EXCEPTION_RECORD record;
    CONTEXT context;
    EXCEPTION_POINTERS pointers;
};

static __thread struct TryState *state;
static pthread_once_t handlers_once = PTHREAD_ONCE_INIT;

static struct TryState *State(void) {
    if (state == NULL) {
        state = (struct TryState *)calloc(1, sizeof(struct TryState));
        if (state == NULL) {
            abort();
        }
    }
    return state;
}

#ifndef __EMSCRIPTEN__
static DWORD Code(int sig, const siginfo_t *info) {
    switch (sig) {
    case SIGFPE:
        return info->si_code == FPE_INTDIV ? EXCEPTION_INT_DIVIDE_BY_ZERO
            : info->si_code == FPE_INTOVF  ? EXCEPTION_INT_OVERFLOW
                                           : EXCEPTION_FLT_INVALID_OPERATION;
    case SIGILL:
        return EXCEPTION_ILLEGAL_INSTRUCTION;
    default:
        return EXCEPTION_ACCESS_VIOLATION;
    }
}

static void Describe(struct TryState *s, int sig, siginfo_t *info, void *uc) {
    memset(&s->record, 0, sizeof(s->record));
    memset(&s->context, 0, sizeof(s->context));
    s->signal = sig;
    s->record.ExceptionCode = Code(sig, info);
    s->record.ExceptionAddress = info->si_addr;
#if defined(__x86_64__)
    {
        const mcontext_t *m = &((const ucontext_t *)uc)->uc_mcontext;
        s->context.Rip = (DWORD64)m->gregs[REG_RIP];
        s->context.Rsp = (DWORD64)m->gregs[REG_RSP];
        s->context.Rbp = (DWORD64)m->gregs[REG_RBP];
        s->context.Rax = (DWORD64)m->gregs[REG_RAX];
        s->context.Rbx = (DWORD64)m->gregs[REG_RBX];
        s->context.Rcx = (DWORD64)m->gregs[REG_RCX];
        s->context.Rdx = (DWORD64)m->gregs[REG_RDX];
        s->context.Rsi = (DWORD64)m->gregs[REG_RSI];
        s->context.Rdi = (DWORD64)m->gregs[REG_RDI];
        s->context.EFlags = (DWORD)m->gregs[REG_EFL];
        s->record.ExceptionAddress = (PVOID)m->gregs[REG_RIP];
        if (sig == SIGSEGV || sig == SIGBUS) {
            /* the page fault's error code: bit 1 is a write */
            s->record.NumberParameters = 2;
            s->record.ExceptionInformation[0] = (m->gregs[REG_ERR] & 2) != 0;
            s->record.ExceptionInformation[1] = (ULONG_PTR)info->si_addr;
        }
    }
#else
    (void)uc;
    if (sig == SIGSEGV || sig == SIGBUS) {
        s->record.NumberParameters = 2;
        s->record.ExceptionInformation[1] = (ULONG_PTR)info->si_addr;
    }
#endif
    s->pointers.ExceptionRecord = &s->record;
    s->pointers.ContextRecord = &s->context;
}

static void Die(int sig) {
    struct sigaction sa;

    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = SIG_DFL;
    sigemptyset(&sa.sa_mask);
    sigaction(sig, &sa, NULL);
    {
        sigset_t set;
        sigemptyset(&set);
        sigaddset(&set, sig);
        pthread_sigmask(SIG_UNBLOCK, &set, NULL);
    }
    raise(sig);
    abort();
}

static void OnFault(int sig, siginfo_t *info, void *uc) {
    struct TryState *s = state;

    if (s == NULL || s->depth == 0) {
        Die(sig); /* no __try around it: the signal's own end */
    }
    Describe(s, sig, info, uc);
    siglongjmp(s->frames[s->depth - 1], 1);
}

#endif

static void InstallHandlers(void) {
#ifdef __EMSCRIPTEN__
    /* WebAssembly: a bad access or a divide by zero traps, and the program ends; there's nothing to catch */
#else
    static const int signals[] = {SIGSEGV, SIGBUS, SIGFPE, SIGILL};
    struct sigaction sa;
    size_t i;

    memset(&sa, 0, sizeof(sa));
    sa.sa_sigaction = OnFault;
    sa.sa_flags = SA_SIGINFO | SA_ONSTACK | SA_NODEFER;
    sigemptyset(&sa.sa_mask);
    for (i = 0; i < sizeof(signals) / sizeof(signals[0]); i++) {
        sigaction(signals[i], &sa, NULL);
    }
#endif
}

void PosixThreadSignalStack(void) {
#ifndef __EMSCRIPTEN__
    stack_t ss;
    stack_t old;

    if (sigaltstack(NULL, &old) == 0 && (old.ss_flags & SS_DISABLE) == 0) {
        return;
    }
    ss.ss_size = 0x10000; /* the crash report's locals are 5 KB; as Windows' stack guarantee (main.c) */
    ss.ss_sp = malloc(ss.ss_size);
    ss.ss_flags = 0;
    if (ss.ss_sp != NULL) {
        sigaltstack(&ss, NULL);
    }
#endif
}

int PosixTryPush(void) {
    struct TryState *s = State();

    pthread_once(&handlers_once, InstallHandlers);
    PosixThreadSignalStack();
    if (s->depth >= MAX_FRAMES) {
        abort(); /* deeper than the code ever nests */
    }
    s->depth++;
    return 1;
}

void *PosixTryTop(void) {
    return &state->frames[state->depth - 1];
}

void PosixTryPop(void) {
    if (state != NULL && state->depth > 0) {
        state->depth--;
    }
}

/* the __except filter's verdict (its frame already popped): nonzero handles it here; 0 (EXCEPTION_CONTINUE_SEARCH)
 * passes it out to the next frame, or ends the program with its signal */
int PosixTryFilter(int verdict) {
    struct TryState *s = state;

    if (verdict != 0) {
        return 1;
    }
    if (s != NULL && s->depth > 0) {
        siglongjmp(s->frames[s->depth - 1], 1);
    }
#ifdef __EMSCRIPTEN__
    abort();
#else
    Die(s != NULL && s->signal != 0 ? s->signal : SIGSEGV);
#endif
    return 0;
}

void *PosixExceptionInformation(void) {
    return &State()->pointers;
}

unsigned int PosixExceptionCode(void) {
    return State()->record.ExceptionCode;
}
