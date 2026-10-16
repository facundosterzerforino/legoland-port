#ifndef MINIWIN_DMUSIC_H
#define MINIWIN_DMUSIC_H

/* [library:music] miniwin's DirectMusic (miniwin/src/dmusic.c): the performance, loader, segment, port and composer
 * objects the game's music thread creates through CoCreateInstance, played by miniwin's style engine
 * (miniwin/dmengine.h) into the game's own DirectSound buffer. miniwin/redirect.h sends the game's CoInitialize and
 * CoCreateInstance here; while the portable music is off they go on to Windows (and its DirectMusic). */

#include <windows.h>
#include <stdio.h>

/* On (the default): the game's DirectMusic objects are miniwin's; off: Windows'. Before the music thread starts
 * (extensions: -windows-music turns it off). */
void MiniwinUsePortableMusic(int on);
int MiniwinPortableMusic(void);
/* -port-selftest: the synthesizer's instruments (gm.dls or a SoundFont, as the music finds them) loaded on a thread of
 * their own, as the music thread loads them, and timed; and the same file read whole, for comparison (the time spent
 * reading it). Writes them to the report; returns 1 when the file doesn't load. */
int MiniwinMusicSelfTest(FILE *report);

HRESULT WINAPI MiniwinCoInitialize(LPVOID reserved);
HRESULT WINAPI MiniwinCoCreateInstance(REFCLSID cls, LPUNKNOWN outer, DWORD context, REFIID riid, LPVOID *out);

#endif
