/* [library:timer] miniwin's clock: the Windows timing functions the game uses, on SDL3. See miniwin/timer.h. */
#include <SDL3/SDL.h>

#include "miniwin/timer.h"

/* Milliseconds in GetTickCount's own steps. Windows advances its tick count once per clock interrupt, every 1/64 s
 * (15.625 ms), so it reads 0, 15, 31, 46, 62... The game depends on that: its frame limiter (BlitFrameToWindow)
 * waits until 28 ms have passed on this clock, which makes every frame two steps, 31.25 ms; with a precise
 * millisecond clock the whole game would run about 10% faster. */
DWORD MiniwinGetTickCount(void) {
    Uint64 steps;

    steps = SDL_GetTicksNS() / 15625000u;

    return (DWORD)(steps * 125 / 8); /* steps * 15.625 ms */
}

/* Nanoseconds since the program started (SDL's start), not since the machine's: the game turns the count into
 * milliseconds in a float, then an int (GetPerformanceTime). Counted from the boot, the float loses the milliseconds
 * after a day or so and the int overflows after 25 days; in a browser the system's clock counts from 1970, so the
 * movies' clock stood still from the start and the page hung in the first movie. */
BOOL MiniwinQueryPerformanceCounter(LARGE_INTEGER *count) {
    count->QuadPart = (LONGLONG)SDL_GetTicksNS();
    return TRUE;
}

BOOL MiniwinQueryPerformanceFrequency(LARGE_INTEGER *frequency) {
    frequency->QuadPart = 1000000000;
    return TRUE;
}

/* The game waits for its next frame by reading the clock in a loop: the frame limiter (BlitFrameToWindow) for 28 ms
 * on GetTickCount, the movie player for the next frame's time. That kept a core busy all the time, which on a phone
 * is heat and battery, and in a browser time the page doesn't get. A nap of a millisecond per pass leaves the loops
 * as they were: they still read the clock and stop at the same value (GetTickCount moves in 15.625 ms steps; a movie
 * frame is 66 ms), just up to about a millisecond later. Windows, the play-tested build, spins as the original. */
void MiniwinIdle(void) {
#ifndef _WIN32
    SDL_DelayNS(1000000);
#endif
}

void MiniwinSleep(DWORD milliseconds) {
    SDL_Delay(milliseconds);
}
