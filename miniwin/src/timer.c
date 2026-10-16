/* [library:timer] miniwin's clock: the Windows timing functions the game uses, on SDL3. See miniwin/timer.h. */
#include <SDL3/SDL.h>

#include "miniwin/timer.h"

#ifdef LEGOLAND_WEB_BROWSER
#include <emscripten.h>
#include <emscripten/threading.h>

static int page_turn; /* in MiniwinWebSleep: the wasm code running now is the page's callbacks', not the game's */
static double last_turn; /* when the page's last turn ended (ms) */

void MiniwinWebSleep(unsigned int milliseconds) {
    page_turn++;
    emscripten_sleep(milliseconds);
    page_turn--;
    last_turn = emscripten_get_now();
}

/* The game waits by polling: the movie player for its next frame, the frame limiter for 28 ms, the movie's sound for
 * the play cursor to leave where it wants to write. On the page's thread nothing else runs meanwhile, and the sound
 * only moves on when its callback runs: starved, it played slowly and in pieces. Every 10 ms of game code that polls,
 * the page gets a turn (never from within one of the page's own callbacks, which can't sleep). */
void MiniwinWebPoll(void) {
    if (emscripten_is_main_browser_thread() && page_turn == 0 && emscripten_get_now() - last_turn >= 10.0) {
        MiniwinWebSleep(0);
    }
}
#endif

/* Milliseconds in GetTickCount's own steps. Windows advances its tick count once per clock interrupt, every 1/64 s
 * (15.625 ms), so it reads 0, 15, 31, 46, 62... The game depends on that: its frame limiter (BlitFrameToWindow)
 * waits until 28 ms have passed on this clock, which makes every frame two steps, 31.25 ms; with a precise
 * millisecond clock the whole game would run about 10% faster. */
DWORD MiniwinGetTickCount(void) {
    Uint64 steps;

#ifdef LEGOLAND_WEB_BROWSER
    MiniwinWebPoll();
#endif
    steps = SDL_GetTicksNS() / 15625000u;

    return (DWORD)(steps * 125 / 8); /* steps * 15.625 ms */
}

/* Nanoseconds since the program started (SDL's start), not since the machine's: the game turns the count into
 * milliseconds in a float, then an int (GetPerformanceTime). Counted from the boot, the float loses the milliseconds
 * after a day or so and the int overflows after 25 days; in a browser the system's clock counts from 1970, so the
 * movies' clock stood still from the start and the page hung in the first movie. */
BOOL MiniwinQueryPerformanceCounter(LARGE_INTEGER *count) {
#ifdef LEGOLAND_WEB_BROWSER
    MiniwinWebPoll();
#endif
    count->QuadPart = (LONGLONG)SDL_GetTicksNS();
    return TRUE;
}

BOOL MiniwinQueryPerformanceFrequency(LARGE_INTEGER *frequency) {
    frequency->QuadPart = 1000000000;
    return TRUE;
}

void MiniwinSleep(DWORD milliseconds) {
#ifdef LEGOLAND_WEB_BROWSER
    if (emscripten_is_main_browser_thread()) {
        MiniwinWebSleep(milliseconds);
        return;
    }
#endif
    SDL_Delay(milliseconds);
}
