#ifndef PORT_LAUNCHER_H
#define PORT_LAUNCHER_H

#include <windows.h>

/* [library:config] The launcher: a small window before the game starts to choose the resolution and fullscreen
 * or windowed (port_display.h). The choice is saved next to the exe (<exe name>.ini) and the launcher can be
 * skipped from then on ("Don't show this again"; hold Shift while starting to see it again).
 *
 * Command line: -res WxH (e.g. -res 1024x768) and -fullscreen / -windowed (or WINDEBUG) choose without the
 * launcher; -launcher always shows it.
 *
 * Sets PortDisplayWidth/Height/Windowed. Returns 0 when the player closed it with Quit. */
int PortLauncherRun(HINSTANCE instance, const char *cmdline);

#endif
