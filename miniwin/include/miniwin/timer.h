#ifndef MINIWIN_TIMER_H
#define MINIWIN_TIMER_H

/* [library:timer] The Windows clock functions the game calls, on SDL3 (miniwin/src/timer.c). miniwin/redirect.h
 * sends the game's calls here under their Windows names. */

#include <windows.h>

DWORD MiniwinGetTickCount(void);
BOOL MiniwinQueryPerformanceCounter(LARGE_INTEGER *count);
BOOL MiniwinQueryPerformanceFrequency(LARGE_INTEGER *frequency);
void MiniwinSleep(DWORD milliseconds);

#endif
