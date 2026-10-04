#ifndef PORT_DISPLAY_H
#define PORT_DISPLAY_H

#include <windows.h>
#include <ddraw.h>

/* [library:video] Output resolution. The game always draws a 640x480 frame (lpConfig->screen_width/height, the
 * interface is laid out for it); the port shows that frame scaled to the chosen size, keeping 4:3 (black bars
 * when the screen or window isn't 4:3). Nearest-neighbour, so pixels stay sharp; 1280x960 and 1920x1440 are
 * exact multiples. Chosen in the launcher (port_launcher.c) or with -res WxH; 640x480 is the original. */

#define PORT_GAME_WIDTH 640
#define PORT_GAME_HEIGHT 480

extern int PortDisplayWidth; /* fullscreen display mode, or the window's client size */
extern int PortDisplayHeight;
extern int PortDisplayWindowed; /* 1: a window (same as the WINDEBUG switch), also for PortDisplayBorderless */
extern int PortDisplayBorderless; /* 1: full screen at the desktop's resolution, a borderless window covering it */
extern int PortDisplayBpp; /* full screen with a mode change: 32 (the game's RGB565 frame is converted when
                                   * shown) or 16 when the display refused 32 (copied as is) */

/* 1 when the frame is shown at another size than 640x480. */
int PortDisplayScaled(void);

/* 1 when frames go through PortPresent (window, scaled, or a 32-bit display); 0: the original Blt. */
int PortDisplayCustomPresent(void);

/* Where the 640x480 frame goes inside an area of the given size (4:3, centred). */
void PortDisplayDestRect(int area_width, int area_height, RECT *dest);

/* Shows the part src (game coordinates; NULL: the whole frame) of the game's frame surface: scaled into the
 * fullscreen primary surface, or with GDI into the window. Returns a DirectDraw result. */
HRESULT PortPresent(HWND hwnd, LPDIRECTDRAWSURFACE frame, LPDIRECTDRAWSURFACE primary, const RECT *src);

/* Windowed: turns a point in the window's client area into game coordinates (0..639, 0..479). */
void PortDisplayClientToGame(HWND hwnd, POINT *pt);

/* The window's outer size for a client area of PortDisplayWidth x PortDisplayHeight with this style, shrunk
 * (keeping 4:3) to fit the desktop's work area. */
void PortDisplayWindowSize(DWORD style, int *width, int *height);

/* Debugging: Ctrl+F12 in the game saves the next 8 presented frames into capture\ next to the exe, each twice:
 * frame-N.bmp (the game's 640x480 picture) and screen-N.bmp (what the window shows). Call after each present. */
void PortDisplayCapture(HWND hwnd, LPDIRECTDRAWSURFACE frame);

#endif
