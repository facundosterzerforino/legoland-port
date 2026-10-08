/* [library:input] The mouse wheel in windowed mode: see port_input.h. */
#include <windows.h>

#include "port_input.h"

/* the most notches kept waiting, so a fast spin doesn't keep scrolling long after the wheel stops */
#define MAX_PENDING (5 * WHEEL_DELTA)

static int pending;

void PortWheelAdd(int delta) {
    pending += delta;
    if (pending > MAX_PENDING) {
        pending = MAX_PENDING;
    } else if (pending < -MAX_PENDING) {
        pending = -MAX_PENDING;
    }
}

int PortWheelTake(int step) {
    if (pending >= WHEEL_DELTA) {
        pending -= WHEEL_DELTA;
        return step;
    }
    if (pending <= -WHEEL_DELTA) {
        pending += WHEEL_DELTA;
        return -step;
    }
    return 0;
}
