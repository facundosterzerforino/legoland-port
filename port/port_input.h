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

/* [library:input] Touch gestures for the game (miniwin's MiniwinSetTouchDragArea): a finger dragging an object list
 * (the build menu's) scrolls it as its arrows do, and two fingers moving pan the park. The original only scrolls the
 * park when the pointer touches the screen's edge, and its lists with their arrow buttons or the wheel.
 * PortTouchFrame: each frame, from ScanMouse (scrolls the dragged list, keeps the pan for PortTouchPan). */
void PortTouchFrame(void);

/* The two-finger pan of this frame, applied to the park's view (from UpdateController, where the game scrolls). */
void PortTouchPan(void);

/* 0 when the pointer's place shouldn't scroll the park on a touch screen: no finger down (the pointer stays where
 * the last one lifted, maybe at the edge) or two fingers (a pan). With a finger down the edges scroll as before. */
int PortTouchEdgeScroll(void);

#endif
