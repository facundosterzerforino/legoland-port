/* test_timer: miniwin's clock waits (miniwin/src/timer.c), off Windows. The game's frame limiter (BlitFrameToWindow:
 * read GetTickCount until it is 28 past the last frame's) with MiniwinIdle in the loop must stop at the same clock
 * values as the spinning original (two of GetTickCount's 15.625 ms steps, 31 or 32 ms), only a little after the step,
 * and use far less of the processor. Built by build_linux.sh; run it with SDL_VIDEO_DRIVER=offscreen. */
#include <windows.h>
#include <SDL3/SDL.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include "miniwin/timer.h"

#define FRAMES 64
#define STEP_NS 15625000u

static int checks;
static int failed;

static void Check(int ok, const char *what) {
    checks++;
    if (!ok) {
        failed++;
        printf("FAILED: %s\n", what);
    }
}

static double CpuMs(void) {
    struct timespec t;

    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &t);
    return t.tv_sec * 1e3 + t.tv_nsec / 1e6;
}

struct Run {
    int other; /* frames that took other than two clock steps */
    double cpu; /* processor time, ms */
    double wall; /* ms */
    double late_max; /* how long after the clock's step the loop stopped, ms */
    double late_sum;
};

static void Frames(int nap, struct Run *r) {
    DWORD last;
    double cpu0;
    Uint64 wall0;
    int i;

    memset(r, 0, sizeof(*r));
    last = MiniwinGetTickCount();
    cpu0 = CpuMs();
    wall0 = SDL_GetTicksNS();
    for (i = 0; i < FRAMES; i++) {
        DWORD tick = MiniwinGetTickCount();
        double late;

        while (tick - last < 0x1c) {
            if (nap) {
                MiniwinIdle();
            }
            tick = MiniwinGetTickCount();
        }
        late = (double)(SDL_GetTicksNS() % STEP_NS) / 1e6;
        if (late > r->late_max) {
            r->late_max = late;
        }
        r->late_sum += late;
        if (tick - last != 31 && tick - last != 32) {
            r->other++;
        }
        last = MiniwinGetTickCount();
    }
    r->cpu = CpuMs() - cpu0;
    r->wall = (double)(SDL_GetTicksNS() - wall0) / 1e6;
}

int main(void) {
    struct Run spin;
    struct Run nap;
    Uint64 start;
    double took;
    int i;

    SDL_Init(0);
    Frames(0, &spin);
    Frames(1, &nap);
    printf("spin: %d frames in %.0f ms, %d not two steps, processor %.0f ms, stopped %.3f ms after the step at most\n",
        FRAMES, spin.wall, spin.other, spin.cpu, spin.late_max);
    printf("nap:  %d frames in %.0f ms, %d not two steps, processor %.0f ms, stopped %.3f ms after the step at most "
           "(%.3f on average)\n",
        FRAMES, nap.wall, nap.other, nap.cpu, nap.late_max, nap.late_sum / FRAMES);
    Check(spin.other == 0, "the spinning limiter: every frame two clock steps");
    Check(nap.other == 0, "the napping limiter: every frame two clock steps, as the spinning one");
    Check(nap.wall > FRAMES * 31.25 - 20.0 && nap.wall < FRAMES * 31.25 + 20.0, "the napping limiter: as many frames a second");
    Check(nap.late_max < 5.0, "the napping limiter stops within 5 ms of the clock's step");
    Check(nap.cpu < nap.wall * 0.25, "the napping limiter leaves the processor free most of the time");

    start = SDL_GetTicksNS();
    for (i = 0; i < 100; i++) {
        MiniwinIdle();
    }
    took = (double)(SDL_GetTicksNS() - start) / 1e6;
    printf("MiniwinIdle: 100 naps in %.1f ms\n", took);
    Check(took >= 100.0 && took < 300.0, "MiniwinIdle naps about a millisecond");

    printf("%d checks, %d failed\n", checks, failed);
    SDL_Quit();
    return failed != 0;
}
