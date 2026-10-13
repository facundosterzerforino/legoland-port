#ifndef PORT_AUDIO_H
#define PORT_AUDIO_H

#include <windows.h>
#include <dsound.h>

/* [library:music] Until the music moves off DirectMusic (ROADMAP.md phase 6, step 3), DirectMusic still renders
 * through Windows' own DirectSound: this is that object, created the first time it is asked for (NULL if Windows has
 * no sound). The game's own sounds (effects, speech, movie sound) use miniwin's DirectSound (miniwin/miniwin.h). */
LPDIRECTSOUND PortMusicDirectSound(HWND hwnd);

#endif
