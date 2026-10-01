#pragma once

/* Port only: small helpers for the C versions of functions the original wrote in inline assembly. */

#include <windows.h>
#include <math.h>

/* fistp: convert to int by rounding to nearest (the x87 default), not C's truncation. */
static __inline int PortRound(double v) {
    return (int)floor(v + 0.5);
}

/* 16.16 fixed-point multiply with a 64-bit intermediate (imul + shrd 16). */
static __inline int PortFixMul(int a, int b) {
    return (int)(((__int64)a * b) >> 16);
}

/* rdtsc stand-in: a free-running counter. The game only uses it for profiling statistics. */
static __inline unsigned int PortTimestamp(void) {
    LARGE_INTEGER t;
    QueryPerformanceCounter(&t);
    return t.LowPart;
}
