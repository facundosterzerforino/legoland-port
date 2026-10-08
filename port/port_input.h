#ifndef PORT_INPUT_H
#define PORT_INPUT_H

/* [library:input] The mouse wheel in windowed mode. There ScanMouse reads the window's own cursor instead of the
 * DirectInput mouse, which also carried the wheel (MouseState.lZ: the original scrolls the build menu one step per
 * frame while it is at least mouse_granularity). The window procedure collects WM_MOUSEWHEEL here and ScanMouse
 * hands it back as lZ, one notch per frame, so every notch scrolls one step as in the original. */

/* Adds a WM_MOUSEWHEEL delta (WHEEL_DELTA = 120 per notch; positive = away from the player). */
void PortWheelAdd(int delta);

/* One notch of the collected movement as an lZ value: +step, -step or 0. */
int PortWheelTake(int step);

#endif
