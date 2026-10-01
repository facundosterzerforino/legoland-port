#pragma once

/* Port only: small helpers for the C versions of functions the original wrote in inline assembly. */

#include <windows.h>
#include <math.h>

/* fistp: convert to int by rounding to nearest, ties to even (the x87 default mode), not C's truncation. */
static __inline int PortRound(double v) {
    double r = floor(v);
    double frac = v - r;

    if (frac > 0.5 || (frac == 0.5 && fmod(r, 2.0) != 0.0)) {
        r += 1.0;
    }
    return (int)r;
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
