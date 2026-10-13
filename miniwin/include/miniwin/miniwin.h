#ifndef MINIWIN_MINIWIN_H
#define MINIWIN_MINIWIN_H

/* miniwin: the Windows APIs the game calls, reimplemented so that it can run without DirectX (PORTING.md).
 * For now it is built on Windows only, next to the real SDK headers, and switched on at run time (the launcher's
 * SDL3 box, -sdl): the game creates its DirectDraw object and window here instead of with DirectDrawCreate and
 * CreateWindowEx, and every other call reaches this code through the same COM interfaces as before. */

#include <windows.h>
#include <ddraw.h>

/* [library:video] DirectDraw on memory surfaces (miniwin/src/ddraw.c), called like DirectDrawCreate. It has what
 * the game uses: 16-bit RGB565 surfaces; Blt copies, stretches and colour fills, with source colour keys and clip
 * lists; Lock; GetDC (GDI drawing on the surface, Windows only). Nothing reaches the screen by itself: the frame is
 * shown with MiniwinPresent. */
HRESULT WINAPI MiniwinDirectDrawCreate(GUID *guid, LPDIRECTDRAW *out, IUnknown *outer);

/* [library:window] The game's window, made by SDL3 (miniwin/src/window.c). */
enum {
    MINIWIN_WINDOW, /* a window with a client area of width x height */
    MINIWIN_FULLSCREEN_DESKTOP, /* full screen at the desktop's resolution */
    MINIWIN_FULLSCREEN_MODE /* full screen in the display mode closest to width x height */
};

/* Creates the window and its renderer; returns its HWND (which DirectInput and DirectSound still need), or NULL
 * (MiniwinError says why). The system cursor is hidden over it: the game draws its own. */
HWND MiniwinCreateWindow(const char *title, int width, int height, int fullscreen);

/* SDL's last error message. */
const char *MiniwinError(void);

/* The renderer SDL chose (e.g. "direct3d11"). */
const char *MiniwinRendererName(void);

/* Hands SDL's window events to the game's window procedure as the window messages it handles: focus lost and
 * gained (WM_KILLFOCUS, WM_SETFOCUS), the mouse wheel (WM_MOUSEWHEEL), Backspace (WM_CHAR 8) and the close button
 * (WM_CLOSE). Call it from the game's message loop. */
void MiniwinPumpEvents(WNDPROC proc);

/* The size of the window's picture in pixels. Returns 0 if there is no window. */
int MiniwinOutputSize(int *width, int *height);

/* Shows a frame: width x height RGB565 pixels, scaled (nearest neighbour) into dest, black around it. */
void MiniwinPresent(const void *pixels, int pitch, int width, int height, const RECT *dest);

#endif
