/* test_seh: win32_prelude.h's __try and __except off Windows (miniwin/src/posix/seh.c): a caught fault, nested
 * frames, a filter of 0 passing the fault out, the exception's code and address, a divide by zero, and the frames
 * left balanced. Prints "OK" and exits 0 when all hold. Linux only (Windows has the real thing). */
#include <windows.h>
#include <stdio.h>

static volatile int *bad = (volatile int *)16;
static volatile int zero;
static volatile int seven = 7; /* a numerator clang can't fold: 1 / x becomes a compare, without a divide */

static int Inner(volatile int *passed) {
    int r = 0;
    __try {
        *bad = 1;
    } __except (*passed = 1, 0) {
        r = 99; /* never: the filter passes it on */
    }
    return r;
}

int main(void) {
    /* volatile: changed after sigsetjmp and read after the jump, as setjmp's rules ask */
    volatile int problems = 0;
    volatile int caught = 0;
    volatile int passed = 0;
    volatile int sink;
    int i;

    __try {
        *bad = 1;
    } __except (1) {
        caught = 1;
    }
    if (!caught) {
        printf("FAIL: a fault in __try wasn't caught\n");
        problems++;
    }
    caught = 0;
    __try {
        Inner(&passed);
    } __except (GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION &&
        ((EXCEPTION_POINTERS *)GetExceptionInformation())->ExceptionRecord->ExceptionInformation[1] == 16) {
        caught = 1;
    }
    if (!passed || !caught) {
        printf("FAIL: the inner filter (%d) didn't pass the fault to the outer frame (%d)\n", passed, caught);
        problems++;
    }
    caught = 0;
    __try {
        sink = seven / zero;
    } __except (GetExceptionCode() == EXCEPTION_INT_DIVIDE_BY_ZERO) {
        caught = 1;
    }
    if (!caught) {
        printf("FAIL: a divide by zero wasn't caught as one\n");
        problems++;
    }
    for (i = 0; i < 100; i++) {
        /* frames that end normally, then a fault: must reach this frame */
        __try {
            caught = i;
        } __except (1) {
            printf("FAIL: a normal __try ran its handler\n");
            problems++;
        }
    }
    caught = 0;
    __try {
        *bad = 2;
    } __except (1) {
        caught = 1;
    }
    if (!caught) {
        printf("FAIL: after 100 normal frames, a fault wasn't caught\n");
        problems++;
    }
    printf(problems == 0 ? "OK\n" : "%d problem(s)\n", problems);
    return problems != 0;
}
