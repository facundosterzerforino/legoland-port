/* [library:music] Windows' DirectSound for DirectMusic only. See port_audio.h. */
#include "port_audio.h"

LPDIRECTSOUND PortMusicDirectSound(HWND hwnd) {
    static LPDIRECTSOUND ds;
    static int tried;

    if (!tried) {
        tried = 1;
        if (DirectSoundCreate(NULL, &ds, NULL) != DS_OK) {
            ds = NULL;
        } else if (IDirectSound_SetCooperativeLevel(ds, hwnd, DSSCL_NORMAL) != DS_OK) {
            IDirectSound_Release(ds);
            ds = NULL;
        }
    }
    return ds;
}
