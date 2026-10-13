/* [library:timer] miniwin's clock: the Windows timing functions the game uses, on SDL3. See miniwin/timer.h. */
#include <SDL3/SDL.h>

#include "miniwin/timer.h"

/* Milliseconds in GetTickCount's own steps. Windows advances its tick count once per clock interrupt, every 1/64 s
 * (15.625 ms), so it reads 0, 15, 31, 46, 62... The game depends on that: its frame limiter (BlitFrameToWindow)
 * waits until 28 ms have passed on this clock, which makes every frame two steps, 31.25 ms; with a precise
 * millisecond clock the whole game would run about 10% faster. */
DWORD MiniwinGetTickCount(void) {
    Uint64 steps = SDL_GetTicksNS() / 15625000u;

    return (DWORD)(steps * 125 / 8); /* steps * 15.625 ms */
}

BOOL MiniwinQueryPerformanceCounter(LARGE_INTEGER *count) {
    count->QuadPart = (LONGLONG)SDL_GetPerformanceCounter();
    return TRUE;
}

BOOL MiniwinQueryPerformanceFrequency(LARGE_INTEGER *frequency) {
    frequency->QuadPart = (LONGLONG)SDL_GetPerformanceFrequency();
    return TRUE;
}

void MiniwinSleep(DWORD milliseconds) {
    SDL_Delay(milliseconds);
}
