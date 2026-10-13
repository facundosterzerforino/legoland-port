#ifndef MINIWIN_TIMER_H
#define MINIWIN_TIMER_H

/* [library:timer] The Windows clock functions the game calls, on SDL3 (miniwin/src/timer.c). In the port build this
 * header is included first into every game source (CMakeLists.txt, /FI), so the decompiled calls reach these
 * versions under their Windows names and stay unchanged. */

#include <windows.h>

DWORD MiniwinGetTickCount(void);
BOOL MiniwinQueryPerformanceCounter(LARGE_INTEGER *count);
BOOL MiniwinQueryPerformanceFrequency(LARGE_INTEGER *frequency);
void MiniwinSleep(DWORD milliseconds);

#define GetTickCount MiniwinGetTickCount
#define QueryPerformanceCounter MiniwinQueryPerformanceCounter
#define QueryPerformanceFrequency MiniwinQueryPerformanceFrequency
#define Sleep MiniwinSleep

#endif
