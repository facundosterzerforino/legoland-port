/* [library:timer] [library:music] The multimedia timer (a thread per timer) and MIDI output (no device: the music
 * plays through miniwin's DirectMusic) off Windows. See posix.h. */
#include <errno.h>
#include <stdlib.h>
#include <time.h>

#include "posix.h"

struct Timer {
    UINT id;
    UINT delay;
    UINT flags;
    LPTIMECALLBACK callback;
    DWORD_PTR user;
    pthread_t thread;
    int stop;
    struct Timer *next;
};

static pthread_mutex_t timers_lock = PTHREAD_MUTEX_INITIALIZER;
static struct Timer *timers;
static UINT next_timer_id = 1;

static void *TimerThread(void *arg) {
    struct Timer *t = (struct Timer *)arg;
    struct timespec next;

    clock_gettime(CLOCK_MONOTONIC, &next);
    for (;;) {
        next.tv_nsec += (long)(t->delay % 1000) * 1000000;
        next.tv_sec += t->delay / 1000 + next.tv_nsec / 1000000000;
        next.tv_nsec %= 1000000000;
        while (clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &next, NULL) == EINTR) {
        }
        if (__atomic_load_n(&t->stop, __ATOMIC_ACQUIRE)) {
            break;
        }
        t->callback(t->id, 0, t->user, 0, 0);
        if ((t->flags & TIME_PERIODIC) == 0) {
            break;
        }
        {
            /* fell behind (a slow callback): from now on, as Windows' timer does */
            struct timespec now;
            clock_gettime(CLOCK_MONOTONIC, &now);
            if (now.tv_sec > next.tv_sec + 1) {
                next = now;
            }
        }
    }
    return NULL;
}

MMRESULT timeSetEvent(UINT delay, UINT resolution, LPTIMECALLBACK callback, DWORD_PTR user, UINT flags) {
    struct Timer *t = (struct Timer *)calloc(1, sizeof(struct Timer));

    (void)resolution;
    if (t == NULL || callback == NULL || (flags & TIME_CALLBACK_EVENT_SET) != 0 ||
        (flags & TIME_CALLBACK_EVENT_PULSE) != 0) {
        free(t);
        return 0; /* the code's timers call a function */
    }
    t->delay = delay > 0 ? delay : 1;
    t->flags = flags;
    t->callback = callback;
    t->user = user;
    pthread_mutex_lock(&timers_lock);
    t->id = next_timer_id++;
    if (pthread_create(&t->thread, NULL, TimerThread, t) != 0) {
        pthread_mutex_unlock(&timers_lock);
        free(t);
        return 0;
    }
    t->next = timers;
    timers = t;
    pthread_mutex_unlock(&timers_lock);
    return t->id;
}

MMRESULT timeKillEvent(UINT id) {
    struct Timer **p;
    struct Timer *t = NULL;

    pthread_mutex_lock(&timers_lock);
    for (p = &timers; *p != NULL; p = &(*p)->next) {
        if ((*p)->id == id) {
            t = *p;
            *p = t->next;
            break;
        }
    }
    pthread_mutex_unlock(&timers_lock);
    if (t == NULL) {
        return MMSYSERR_INVALPARAM;
    }
    __atomic_store_n(&t->stop, 1, __ATOMIC_RELEASE);
    if (pthread_equal(t->thread, pthread_self())) {
        pthread_detach(t->thread); /* killed from its own callback: it ends after it */
    } else {
        pthread_join(t->thread, NULL);
        free(t);
    }
    return MMSYSERR_NOERROR;
}

MMRESULT midiOutOpen(LPHMIDIOUT handle, UINT device, DWORD_PTR callback, DWORD_PTR instance, DWORD flags) {
    (void)device;
    (void)callback;
    (void)instance;
    (void)flags;
    if (handle != NULL) {
        *handle = NULL;
    }
    return MMSYSERR_NODRIVER;
}

MMRESULT midiOutShortMsg(HMIDIOUT handle, DWORD message) {
    (void)handle;
    (void)message;
    return MMSYSERR_INVALHANDLE;
}

MMRESULT midiOutClose(HMIDIOUT handle) {
    (void)handle;
    return MMSYSERR_INVALHANDLE;
}
