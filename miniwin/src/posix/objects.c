/* [library:thread] Kernel objects off Windows: events, mutexes, threads, waits, critical sections. See posix.h. */
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "miniwin/timer.h"
#include "posix.h"

#ifdef LEGOLAND_WEB_BROWSER
#include <emscripten.h>
#include <emscripten/threading.h>
#endif

pthread_mutex_t posix_objects_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t posix_objects_changed;
static pthread_once_t init_once = PTHREAD_ONCE_INIT;

struct Event {
    struct PosixObject base;
    int manual_reset;
    int signaled;
};

struct Mutex {
    struct PosixObject base;
    char name[64];
    pthread_t owner;
    int count; /* 0: not owned */
    struct Mutex *next_named;
};

struct Thread {
    struct PosixObject base;
    pthread_t id;
    LPTHREAD_START_ROUTINE start;
    void *parameter;
    int finished; /* signaled: ended, or terminated */
    int exited; /* ThreadMain has returned: the object can go */
    DWORD exit_code;
};

static struct Mutex *named_mutexes;
static DWORD next_thread_id = 1;

static void Init(void) {
    pthread_condattr_t attr;

    pthread_condattr_init(&attr);
    pthread_condattr_setclock(&attr, CLOCK_MONOTONIC);
    pthread_cond_init(&posix_objects_changed, &attr);
    pthread_condattr_destroy(&attr);
}

void *PosixNewObject(enum PosixKind kind, size_t size) {
    struct PosixObject *o;

    pthread_once(&init_once, Init);
    o = (struct PosixObject *)calloc(1, size);
    if (o != NULL) {
        o->kind = kind;
    }
    return o;
}

void *PosixObjectOf(HANDLE handle, enum PosixKind kind) {
    struct PosixObject *o = (struct PosixObject *)handle;

    if (o == NULL || handle == INVALID_HANDLE_VALUE || o->kind != kind) {
        return NULL;
    }
    return o;
}

unsigned long long PosixMilliseconds(void) {
    struct timespec ts;

    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (unsigned long long)ts.tv_sec * 1000 + (unsigned long long)ts.tv_nsec / 1000000;
}

/* ---- events ---- */

HANDLE CreateEventA(LPSECURITY_ATTRIBUTES attributes, BOOL manual_reset, BOOL initial_state, LPCSTR name) {
    struct Event *e = (struct Event *)PosixNewObject(POSIX_EVENT, sizeof(struct Event));

    (void)attributes;
    (void)name; /* named events would be shared between processes; the code's are unnamed */
    if (e == NULL) {
        PosixSetError(ERROR_NOT_ENOUGH_MEMORY);
        return NULL;
    }
    e->manual_reset = manual_reset != 0;
    e->signaled = initial_state != 0;
    return (HANDLE)e;
}

BOOL SetEvent(HANDLE handle) {
    struct Event *e = (struct Event *)PosixObjectOf(handle, POSIX_EVENT);

    if (e == NULL) {
        PosixSetError(ERROR_INVALID_HANDLE);
        return FALSE;
    }
    pthread_mutex_lock(&posix_objects_lock);
    e->signaled = 1;
    pthread_cond_broadcast(&posix_objects_changed);
    pthread_mutex_unlock(&posix_objects_lock);
    return TRUE;
}

BOOL ResetEvent(HANDLE handle) {
    struct Event *e = (struct Event *)PosixObjectOf(handle, POSIX_EVENT);

    if (e == NULL) {
        PosixSetError(ERROR_INVALID_HANDLE);
        return FALSE;
    }
    pthread_mutex_lock(&posix_objects_lock);
    e->signaled = 0;
    pthread_mutex_unlock(&posix_objects_lock);
    return TRUE;
}

/* ---- mutexes (within this process: a second copy of the game doesn't see the first's) ---- */

static struct Mutex *FindNamed(const char *name) {
    struct Mutex *m;

    for (m = named_mutexes; m != NULL; m = m->next_named) {
        if (strcmp(m->name, name) == 0) {
            return m;
        }
    }
    return NULL;
}

HANDLE CreateMutexA(LPSECURITY_ATTRIBUTES attributes, BOOL initial_owner, LPCSTR name) {
    struct Mutex *m;

    (void)attributes;
    pthread_mutex_lock(&posix_objects_lock);
    if (name != NULL && (m = FindNamed(name)) != NULL) {
        pthread_mutex_unlock(&posix_objects_lock);
        PosixSetError(ERROR_ALREADY_EXISTS);
        return (HANDLE)m;
    }
    pthread_mutex_unlock(&posix_objects_lock);
    m = (struct Mutex *)PosixNewObject(POSIX_MUTEX, sizeof(struct Mutex));
    if (m == NULL) {
        PosixSetError(ERROR_NOT_ENOUGH_MEMORY);
        return NULL;
    }
    pthread_mutex_lock(&posix_objects_lock);
    if (initial_owner) {
        m->owner = pthread_self();
        m->count = 1;
    }
    if (name != NULL) {
        strncpy(m->name, name, sizeof(m->name) - 1);
        m->next_named = named_mutexes;
        named_mutexes = m;
    }
    pthread_mutex_unlock(&posix_objects_lock);
    PosixSetError(ERROR_SUCCESS);
    return (HANDLE)m;
}

HANDLE OpenMutexA(DWORD access, BOOL inherit, LPCSTR name) {
    struct Mutex *m;

    (void)access;
    (void)inherit;
    pthread_mutex_lock(&posix_objects_lock);
    m = name != NULL ? FindNamed(name) : NULL;
    pthread_mutex_unlock(&posix_objects_lock);
    if (m == NULL) {
        PosixSetError(ERROR_FILE_NOT_FOUND);
    }
    return (HANDLE)m;
}

/* ---- threads ---- */

static void FreeObject(struct PosixObject *o) {
    if (o->kind == POSIX_MUTEX) {
        struct Mutex **p;

        for (p = &named_mutexes; *p != NULL; p = &(*p)->next_named) {
            if (*p == (struct Mutex *)o) {
                *p = (*p)->next_named;
                break;
            }
        }
    }
    o->kind = 0;
    free(o);
}

static void *ThreadMain(void *arg) {
    struct Thread *t = (struct Thread *)arg;
    DWORD code;

    PosixThreadSignalStack();
    code = t->start(t->parameter);

    pthread_mutex_lock(&posix_objects_lock);
    if (!t->finished) {
        t->exit_code = code;
    }
    t->finished = 1;
    t->exited = 1;
    pthread_cond_broadcast(&posix_objects_changed);
    if (t->base.closed) {
        FreeObject(&t->base);
    }
    pthread_mutex_unlock(&posix_objects_lock);
    return NULL;
}

HANDLE CreateThread(LPSECURITY_ATTRIBUTES attributes, SIZE_T stack_size, LPTHREAD_START_ROUTINE start,
    LPVOID parameter, DWORD flags, LPDWORD thread_id) {
    struct Thread *t = (struct Thread *)PosixNewObject(POSIX_THREAD, sizeof(struct Thread));
    pthread_attr_t attr;
    int failed;

    (void)attributes;
    (void)flags; /* CREATE_SUSPENDED isn't used */
    if (t == NULL) {
        PosixSetError(ERROR_NOT_ENOUGH_MEMORY);
        return NULL;
    }
    t->start = start;
    t->parameter = parameter;
    t->exit_code = STILL_ACTIVE;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    /* Windows' default is 1 MB; the game's threads were written for that */
    pthread_attr_setstacksize(&attr, stack_size > 0x100000 ? stack_size : 0x100000);
    pthread_mutex_lock(&posix_objects_lock);
    failed = pthread_create(&t->id, &attr, ThreadMain, t);
    if (thread_id != NULL) {
        *thread_id = next_thread_id++;
    }
    pthread_mutex_unlock(&posix_objects_lock);
    pthread_attr_destroy(&attr);
    if (failed) {
        free(t);
        PosixSetError(ERROR_NOT_ENOUGH_MEMORY);
        return NULL;
    }
    return (HANDLE)t;
}

BOOL TerminateThread(HANDLE handle, DWORD exit_code) {
    struct Thread *t = (struct Thread *)PosixObjectOf(handle, POSIX_THREAD);

    if (t == NULL) {
        PosixSetError(ERROR_INVALID_HANDLE);
        return FALSE;
    }
    pthread_mutex_lock(&posix_objects_lock);
    if (!t->finished) {
#ifndef __ANDROID__
        /* it stops at its next wait or sleep (a cancellation point), as close as POSIX comes */
        pthread_cancel(t->id);
#endif
        /* (Android's C library can't cancel a thread: it runs on, unseen, until the program ends) */
        t->exit_code = exit_code;
        t->finished = 1;
        pthread_cond_broadcast(&posix_objects_changed);
    }
    pthread_mutex_unlock(&posix_objects_lock);
    return TRUE;
}

/* ---- waits ---- */

static int Signaled(struct PosixObject *o) {
    switch (o->kind) {
    case POSIX_EVENT:
        return ((struct Event *)o)->signaled;
    case POSIX_THREAD:
        return ((struct Thread *)o)->finished;
    case POSIX_MUTEX: {
        struct Mutex *m = (struct Mutex *)o;
        return m->count == 0 || pthread_equal(m->owner, pthread_self());
    }
    default:
        return 0;
    }
}

static void Acquire(struct PosixObject *o) {
    if (o->kind == POSIX_EVENT && !((struct Event *)o)->manual_reset) {
        ((struct Event *)o)->signaled = 0;
    } else if (o->kind == POSIX_MUTEX) {
        struct Mutex *m = (struct Mutex *)o;
        m->owner = pthread_self();
        m->count++;
    }
}

DWORD WaitForMultipleObjects(DWORD count, const HANDLE *handles, BOOL wait_all, DWORD milliseconds) {
    struct timespec deadline;
    DWORD i;
    DWORD result = WAIT_TIMEOUT;

    if (count == 0 || count > MAXIMUM_WAIT_OBJECTS) {
        PosixSetError(ERROR_INVALID_PARAMETER);
        return WAIT_FAILED;
    }
    for (i = 0; i < count; i++) {
        struct PosixObject *o = (struct PosixObject *)handles[i];
        if (o == NULL || handles[i] == INVALID_HANDLE_VALUE ||
            (o->kind != POSIX_EVENT && o->kind != POSIX_THREAD && o->kind != POSIX_MUTEX)) {
            PosixSetError(ERROR_INVALID_HANDLE);
            return WAIT_FAILED;
        }
    }
    pthread_once(&init_once, Init);
    if (milliseconds != INFINITE) {
        clock_gettime(CLOCK_MONOTONIC, &deadline);
        deadline.tv_sec += milliseconds / 1000;
        deadline.tv_nsec += (long)(milliseconds % 1000) * 1000000;
        if (deadline.tv_nsec >= 1000000000) {
            deadline.tv_sec++;
            deadline.tv_nsec -= 1000000000;
        }
    }
    pthread_mutex_lock(&posix_objects_lock);
    for (;;) {
        int timed_out;

        if (wait_all) {
            for (i = 0; i < count && Signaled((struct PosixObject *)handles[i]); i++) {
            }
            if (i == count) {
                for (i = 0; i < count; i++) {
                    Acquire((struct PosixObject *)handles[i]);
                }
                result = WAIT_OBJECT_0;
                break;
            }
        } else {
            for (i = 0; i < count && !Signaled((struct PosixObject *)handles[i]); i++) {
            }
            if (i < count) {
                Acquire((struct PosixObject *)handles[i]);
                result = WAIT_OBJECT_0 + i;
                break;
            }
        }
        if (milliseconds == 0) {
            break;
        }
#ifdef LEGOLAND_WEB_BROWSER
        if (emscripten_is_main_browser_thread()) {
            /* the page's thread mustn't block: look again in a millisecond, the page running meanwhile */
            struct timespec now;
            pthread_mutex_unlock(&posix_objects_lock);
            MiniwinWebSleep(1);
            pthread_mutex_lock(&posix_objects_lock);
            clock_gettime(CLOCK_MONOTONIC, &now);
            if (milliseconds != INFINITE &&
                (now.tv_sec > deadline.tv_sec || (now.tv_sec == deadline.tv_sec && now.tv_nsec >= deadline.tv_nsec))) {
                milliseconds = 0;
            }
            continue;
        }
#endif
        if (milliseconds == INFINITE) {
            timed_out = pthread_cond_wait(&posix_objects_changed, &posix_objects_lock) == ETIMEDOUT;
        } else {
            timed_out = pthread_cond_timedwait(&posix_objects_changed, &posix_objects_lock, &deadline) == ETIMEDOUT;
        }
        if (timed_out) {
            milliseconds = 0; /* one last look, then WAIT_TIMEOUT */
        }
    }
    pthread_mutex_unlock(&posix_objects_lock);
    return result;
}

DWORD WaitForSingleObject(HANDLE handle, DWORD milliseconds) {
    return WaitForMultipleObjects(1, &handle, FALSE, milliseconds);
}

/* the closing of events, mutexes and threads (files and searches: fileio.c) */
BOOL PosixCloseObject(struct PosixObject *o) {
    pthread_mutex_lock(&posix_objects_lock);
    if (o->kind == POSIX_THREAD && !((struct Thread *)o)->exited) {
        o->closed = 1; /* freed when it ends (a terminated thread that never reaches a cancellation point: never) */
    } else {
        FreeObject(o);
    }
    pthread_mutex_unlock(&posix_objects_lock);
    return TRUE;
}

/* ---- critical sections: a recursive pthread mutex, kept in the structure's LockSemaphore ---- */

void InitializeCriticalSection(LPCRITICAL_SECTION section) {
    pthread_mutex_t *m = (pthread_mutex_t *)malloc(sizeof(pthread_mutex_t));
    pthread_mutexattr_t attr;

    memset(section, 0, sizeof(*section));
    pthread_mutexattr_init(&attr);
    pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
    pthread_mutex_init(m, &attr);
    pthread_mutexattr_destroy(&attr);
    section->LockSemaphore = (HANDLE)m;
}

void EnterCriticalSection(LPCRITICAL_SECTION section) {
    pthread_mutex_lock((pthread_mutex_t *)section->LockSemaphore);
}

void LeaveCriticalSection(LPCRITICAL_SECTION section) {
    pthread_mutex_unlock((pthread_mutex_t *)section->LockSemaphore);
}
