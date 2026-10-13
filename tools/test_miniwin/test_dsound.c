/* test_dsound: checks miniwin's DirectSound (miniwin/src/dsound.c) against Windows' own, without starting the game:
 * the same calls on both, comparing results that don't depend on timing (return codes, lock pointers and sizes,
 * cursors while stopped, status, volume / pan / frequency, duplicates) and the state after a short silent buffer has
 * played to its end. Everything played is silence.
 *
 *   tools/test_miniwin/build.sh && tools/test_miniwin/test_dsound.exe      (prints FAIL lines, exits 1 on any) */
#include <windows.h>
#include <dsound.h>
#include <stdio.h>
#include <string.h>

#include "miniwin/miniwin.h"

enum {
    REAL,
    MINI,
    SIDES
};
static const char *const side_names[SIDES] = {"DirectSound", "miniwin"};

static LPDIRECTSOUND ds[SIDES];
static int failures;
static int checks;

static void Same(const char *what, long a, long b) {
    checks++;
    if (a != b) {
        printf("FAIL %s: %ld (DirectSound) vs %ld (miniwin)\n", what, a, b);
        failures++;
    }
}

static LPDIRECTSOUNDBUFFER Make(int side, int bits, int channels, int rate, DWORD bytes, DWORD flags, HRESULT *hr) {
    WAVEFORMATEX f;
    DSBUFFERDESC d;
    LPDIRECTSOUNDBUFFER b = NULL;

    memset(&f, 0, sizeof(f));
    f.wFormatTag = WAVE_FORMAT_PCM;
    f.nChannels = (WORD)channels;
    f.nSamplesPerSec = rate;
    f.wBitsPerSample = (WORD)bits;
    f.nBlockAlign = (WORD)(channels * bits / 8);
    f.nAvgBytesPerSec = rate * f.nBlockAlign;
    memset(&d, 0, sizeof(d));
    d.dwSize = sizeof(d);
    d.dwFlags = DSBCAPS_CTRLFREQUENCY | DSBCAPS_CTRLPAN | DSBCAPS_CTRLVOLUME | DSBCAPS_GETCURRENTPOSITION2 | flags;
    d.dwBufferBytes = bytes;
    d.lpwfxFormat = &f;
    *hr = IDirectSound_CreateSoundBuffer(ds[side], &d, &b, NULL);
    return b;
}

int main(void) {
    LPDIRECTSOUNDBUFFER b[SIDES], dup[SIDES];
    HRESULT hr[SIDES];
    int s;

    if (DirectSoundCreate(NULL, &ds[REAL], NULL) != DS_OK) {
        printf("no Windows DirectSound (no audio device?): nothing to compare\n");
        return 0;
    }
    if (MiniwinDirectSoundCreate(NULL, &ds[MINI], NULL) != DS_OK) {
        printf("FAIL: MiniwinDirectSoundCreate\n");
        return 1;
    }
    for (s = 0; s < SIDES; s++) {
        IDirectSound_SetCooperativeLevel(ds[s], GetDesktopWindow(), DSSCL_NORMAL);
    }

    /* bad formats and sizes */
    for (s = 0; s < SIDES; s++) {
        b[s] = Make(s, 12, 1, 22050, 4096, 0, &hr[s]);
    }
    Same("create with 12-bit samples", hr[REAL], hr[MINI]);
    for (s = 0; s < SIDES; s++) {
        b[s] = Make(s, 16, 1, 22050, 2, 0, &hr[s]);
    }
    Same("create with a 2-byte buffer", hr[REAL], hr[MINI]);

    /* a 16-bit stereo buffer: locks, cursors, status, controls */
    for (s = 0; s < SIDES; s++) {
        b[s] = Make(s, 16, 2, 22050, 8000, 0, &hr[s]);
    }
    Same("create 16-bit stereo", hr[REAL], hr[MINI]);
    if (b[REAL] == NULL || b[MINI] == NULL) {
        printf("%d checks, %d failed\n", checks, failures + 1);
        return 1;
    }
    {
        LPVOID p1[SIDES], p2[SIDES];
        DWORD n1[SIDES], n2[SIDES];
        DWORD play[SIDES], write[SIDES], status[SIDES], value[SIDES];
        LONG lvalue[SIDES];

        for (s = 0; s < SIDES; s++) {
            hr[s] = IDirectSoundBuffer_Lock(b[s], 6000, 4000, &p1[s], &n1[s], &p2[s], &n2[s], 0);
        }
        Same("lock across the end: result", hr[REAL], hr[MINI]);
        Same("lock across the end: first part", (long)n1[REAL], (long)n1[MINI]);
        Same("lock across the end: second part", (long)n2[REAL], (long)n2[MINI]);
        for (s = 0; s < SIDES; s++) {
            IDirectSoundBuffer_Unlock(b[s], p1[s], n1[s], p2[s], n2[s]);
            hr[s] = IDirectSoundBuffer_Lock(b[s], 0, 0, &p1[s], &n1[s], NULL, NULL, DSBLOCK_ENTIREBUFFER);
        }
        Same("lock the entire buffer: result", hr[REAL], hr[MINI]);
        Same("lock the entire buffer: size", (long)n1[REAL], (long)n1[MINI]);
        for (s = 0; s < SIDES; s++) {
            memset(p1[s], 0, n1[s]);
            IDirectSoundBuffer_Unlock(b[s], p1[s], n1[s], NULL, 0);
            hr[s] = IDirectSoundBuffer_Lock(b[s], 0, 0, &p1[s], &n1[s], NULL, NULL, 0);
        }
        Same("lock 0 bytes", hr[REAL], hr[MINI]);
        for (s = 0; s < SIDES; s++) {
            hr[s] = IDirectSoundBuffer_SetCurrentPosition(b[s], 1003);
            IDirectSoundBuffer_GetCurrentPosition(b[s], &play[s], &write[s]);
        }
        Same("set position 1003: result", hr[REAL], hr[MINI]);
        Same("set position 1003: play cursor while stopped", (long)play[REAL], (long)play[MINI]);
        Same("set position 1003: write cursor while stopped", (long)write[REAL], (long)write[MINI]);
        for (s = 0; s < SIDES; s++) {
            IDirectSoundBuffer_GetFrequency(b[s], &value[s]);
        }
        Same("default frequency", (long)value[REAL], (long)value[MINI]);
        for (s = 0; s < SIDES; s++) {
            IDirectSoundBuffer_GetVolume(b[s], &lvalue[s]);
        }
        Same("default volume", lvalue[REAL], lvalue[MINI]);
        for (s = 0; s < SIDES; s++) {
            IDirectSoundBuffer_GetPan(b[s], &lvalue[s]);
        }
        Same("default pan", lvalue[REAL], lvalue[MINI]);
        for (s = 0; s < SIDES; s++) {
            hr[s] = IDirectSoundBuffer_SetVolume(b[s], 100);
        }
        Same("volume above 0", hr[REAL], hr[MINI]);
        for (s = 0; s < SIDES; s++) {
            hr[s] = IDirectSoundBuffer_SetFrequency(b[s], 50);
        }
        Same("frequency 50 Hz", hr[REAL], hr[MINI]);
        for (s = 0; s < SIDES; s++) {
            IDirectSoundBuffer_SetVolume(b[s], -1234);
            IDirectSoundBuffer_SetPan(b[s], 777);
            IDirectSoundBuffer_SetFrequency(b[s], 30000);
            hr[s] = IDirectSound_DuplicateSoundBuffer(ds[s], b[s], &dup[s]);
        }
        Same("duplicate", hr[REAL], hr[MINI]);
        for (s = 0; s < SIDES; s++) {
            IDirectSoundBuffer_GetVolume(dup[s], &lvalue[s]);
        }
        Same("duplicate's volume", lvalue[REAL], lvalue[MINI]);
        for (s = 0; s < SIDES; s++) {
            IDirectSoundBuffer_GetPan(dup[s], &lvalue[s]);
        }
        Same("duplicate's pan", lvalue[REAL], lvalue[MINI]);
        for (s = 0; s < SIDES; s++) {
            IDirectSoundBuffer_GetFrequency(dup[s], &value[s]);
        }
        Same("duplicate's frequency", (long)value[REAL], (long)value[MINI]);
        for (s = 0; s < SIDES; s++) {
            IDirectSoundBuffer_GetCurrentPosition(dup[s], &play[s], NULL);
        }
        Same("duplicate's position", (long)play[REAL], (long)play[MINI]);
        for (s = 0; s < SIDES; s++) {
            IDirectSoundBuffer_Play(b[s], 0, 0, DSBPLAY_LOOPING);
            IDirectSoundBuffer_GetStatus(b[s], &status[s]);
        }
        Same("status while looping", (long)status[REAL], (long)status[MINI]);
        for (s = 0; s < SIDES; s++) {
            IDirectSoundBuffer_Stop(b[s]);
            IDirectSoundBuffer_GetStatus(b[s], &status[s]);
        }
        Same("status after stop", (long)status[REAL], (long)status[MINI]);
        for (s = 0; s < SIDES; s++) {
            IDirectSoundBuffer_SetFrequency(b[s], DSBFREQUENCY_ORIGINAL);
            IDirectSoundBuffer_GetFrequency(b[s], &value[s]);
        }
        Same("frequency after setting the original again", (long)value[REAL], (long)value[MINI]);
        /* A new short buffer (about 90 ms) played once, then left to finish, as the game plays each sample instance.
         * (Run from WSL without a window, Windows' DirectSound sometimes leaves a buffer that was already played,
         * stopped or duplicated stuck near its start; a fresh one finishes reliably.) */
        for (s = 0; s < SIDES; s++) {
            b[s] = Make(s, 16, 2, 22050, 8000, 0, &hr[s]);
            IDirectSoundBuffer_Play(b[s], 0, 0, 0);
        }
        Sleep(400);
        for (s = 0; s < SIDES; s++) {
            IDirectSoundBuffer_GetStatus(b[s], &status[s]);
            IDirectSoundBuffer_GetCurrentPosition(b[s], &play[s], &write[s]);
        }
        Same("status after playing to the end", (long)status[REAL], (long)status[MINI]);
        Same("play cursor after playing to the end", (long)play[REAL], (long)play[MINI]);
        printf("   (after the end: DirectSound play %lu write %lu, miniwin play %lu write %lu)\n", play[REAL], write[REAL],
            play[MINI], write[MINI]);
    }
    printf("%d checks, %d failed\n", checks, failures);
    return failures != 0;
}
