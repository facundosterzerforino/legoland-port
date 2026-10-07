#ifndef PORT_LAUNCHER_H
#define PORT_LAUNCHER_H

#include <windows.h>

/* [library:config] The launcher: a small window before the game starts to choose how to show it
 * (port_display.h):
 *   - full screen at the desktop's resolution (default): a borderless window over the screen, no mode change;
 *   - full screen changing the resolution: a 4:3 display mode (32-bit; 16-bit if refused);
 *   - in a window of a chosen 4:3 size.
 * It also has the box for the level music extension (extensions/ext_music.h, off by default).
 * The choice is saved next to the exe (<exe name>.ini) and the launcher can be skipped from then on ("Don't show
 * this again"; hold Shift while starting to see it again).
 *
 * Command line: -fullscreen (desktop resolution), -res WxH (full screen in that mode, e.g. -res 1024x768),
 * -windowed or WINDEBUG (a window; with -res, of that size) choose without the launcher; -launcher always shows it.
 *
 * Sets PortDisplayWidth/Height/Windowed/Borderless. Returns 0 when the player closed it with Quit. */
int PortLauncherRun(HINSTANCE instance, const char *cmdline);

#endif
