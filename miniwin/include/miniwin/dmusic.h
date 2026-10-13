#ifndef MINIWIN_DMUSIC_H
#define MINIWIN_DMUSIC_H

/* [library:music] miniwin's DirectMusic (miniwin/src/dmusic.c): the performance, loader, segment, port and composer
 * objects the game's music thread creates through CoCreateInstance, played by miniwin's style engine
 * (miniwin/dmengine.h) into the game's own DirectSound buffer. miniwin/redirect.h sends the game's CoInitialize and
 * CoCreateInstance here; while the portable music is off they go on to Windows (and its DirectMusic). */

#include <windows.h>

/* On: the game's DirectMusic objects are miniwin's. Before the music thread starts (extensions: -portable-music). */
void MiniwinUsePortableMusic(int on);
int MiniwinPortableMusic(void);

HRESULT WINAPI MiniwinCoInitialize(LPVOID reserved);
HRESULT WINAPI MiniwinCoCreateInstance(REFCLSID cls, LPUNKNOWN outer, DWORD context, REFIID riid, LPVOID *out);

#endif
