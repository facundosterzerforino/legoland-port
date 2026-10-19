#ifndef MINIWIN_TIMER_H
#define MINIWIN_TIMER_H

/* [library:timer] The Windows clock functions the game calls, on SDL3 (miniwin/src/timer.c). miniwin/redirect.h
 * sends the game's calls here under their Windows names. */

#include <windows.h>

DWORD MiniwinGetTickCount(void);
BOOL MiniwinQueryPerformanceCounter(LARGE_INTEGER *count);
BOOL MiniwinQueryPerformanceFrequency(LARGE_INTEGER *frequency);
void MiniwinSleep(DWORD milliseconds);
/* One pass of a loop where the game waits for the clock (the frame limiter, the movie player): a nap of about a
 * millisecond instead of a spin (miniwin/src/timer.c) */
void MiniwinIdle(void);

#ifdef LEGOLAND_WEB_BROWSER
/* [library:timer] In a browser the game runs on the page's thread, which also plays the sound (SDL's audio callback
 * feeds miniwin's DirectSound mixer) and shows each picture: those happen only while the game is in emscripten_sleep
 * (ASYNCIFY). MiniwinWebSleep is that sleep (every one goes through it); MiniwinWebPoll, called where the game polls
 * the clock or a sound's play cursor, gives the page its turn when it hasn't had one for a few milliseconds. */
void MiniwinWebSleep(unsigned int milliseconds);
void MiniwinWebPoll(void);
#endif

#endif
