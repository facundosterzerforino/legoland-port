#ifndef PORT_SETUP_WIN_H
#define PORT_SETUP_WIN_H

#include <windows.h>

/* [library:filesystem] The first-run setup window (port_setup.h): shown when the game's data isn't in the current
 * folder or next to the exe. Finds the LEGOLAND CD in a CD drive (a mounted ISO is one too) or lets the player
 * choose a folder (a copy of the CD, or an install), then copies the files next to the exe with a progress bar.
 * Returns 1 when the data is there (and is the current folder), 0 when the player quit. */
int PortSetupRun(HINSTANCE instance);

#endif
