#ifndef PORT_DISPLAY_H
#define PORT_DISPLAY_H

#include <windows.h>
#include <ddraw.h>

/* [library:video] How the game's picture is shown. The game always draws a 640x480 RGB565 frame (lpConfig->
 * screen_width/height; the interface is laid out for it) into miniwin's memory surfaces, and always takes its
 * windowed code path (WinDebugMode). The port shows the frame in its SDL3 window (miniwin/miniwin.h), scaled to the
 * window or screen keeping 4:3 (black bars around it), nearest neighbour so pixels stay sharp; 1280x960 and
 * 1920x1440 are exact multiples. The window and its size are chosen in the launcher (port_launcher.c) or with
 * -res WxH; 640x480 is the original. */

#define PORT_GAME_WIDTH 640
#define PORT_GAME_HEIGHT 480

extern int PortDisplayWidth; /* the window's client size, or the full-screen display mode */
extern int PortDisplayHeight;
extern int PortDisplayFullscreen; /* MINIWIN_WINDOW, MINIWIN_FULLSCREEN_DESKTOP or MINIWIN_FULLSCREEN_MODE */

/* Where the 640x480 frame goes inside an area of the given size (4:3, centred). */
void PortDisplayDestRect(int area_width, int area_height, RECT *dest);

/* Shows the game's frame surface in the window. Returns a DirectDraw result. */
HRESULT PortPresent(LPDIRECTDRAWSURFACE frame);

/* Turns a point in the window's picture (pixels) into game coordinates (0..639, 0..479). */
void PortDisplayClientToGame(POINT *pt);

/* Debugging: Ctrl+F12 in the game saves the next 4 frames (the game's 640x480 picture) as capture\frame-N.bmp next
 * to the exe. Call it after each present. */
void PortDisplayCapture(LPDIRECTDRAWSURFACE frame);

#endif
