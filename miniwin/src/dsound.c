/* [library:audio] miniwin's DirectSound: the game's sound buffers, mixed on SDL3's audio thread. See miniwin/miniwin.h.
 *
 * Every buffer holds PCM (8-bit unsigned or 16-bit signed, mono or stereo, any rate). The mixer resamples each
 * playing buffer to the device rate (linear interpolation), applies DirectSound's volume (hundredths of a decibel,
 * 0 = full) and pan (the other side attenuated), and adds them up. */
#include <SDL3/SDL.h>
#include <math.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "miniwin/miniwin.h"

#define MIX_RATE 44100
#define MIX_CHUNK 1024 /* frames mixed per round */

struct SoundData {
    int refs;
    unsigned char *bytes;
    DWORD size;
};

struct Buffer {
    IDirectSoundBuffer iface;
    LONG refs;
    struct Buffer *next; /* all buffers, for the mixer */
    struct SoundData *data;
    WAVEFORMATEX format;
    DWORD flags;
    int primary;
    double pos; /* play position, in frames */
    int playing;
    int looping;
    LONG volume;
    LONG pan;
    DWORD frequency; /* 0: the format's own rate */
};

struct DirectSound {
    IDirectSound iface;
    LONG refs;
};

static SDL_Mutex *lock;
static SDL_AudioStream *stream;
static struct Buffer *buffers;
static float mix[MIX_CHUNK * 2];

static struct Buffer *AsBuffer(IDirectSoundBuffer *p) {
    return (struct Buffer *)p;
}

static int SameGuid(REFIID a, const GUID *b) {
    return memcmp(a, b, sizeof(GUID)) == 0;
}

static DWORD Frames(const struct Buffer *b) {
    return b->format.nBlockAlign != 0 ? b->data->size / b->format.nBlockAlign : 0;
}

/* One channel of one frame, -1..1. */
static float SampleAt(const struct Buffer *b, DWORD frame, int channel) {
    const unsigned char *p = b->data->bytes + (size_t)frame * b->format.nBlockAlign;

    if (b->format.nChannels > 1) {
        p += (size_t)channel * (b->format.wBitsPerSample / 8);
    }
    if (b->format.wBitsPerSample == 16) {
        return (short)(p[0] | (p[1] << 8)) / 32768.0f;
    }
    return ((int)p[0] - 128) / 128.0f;
}

static float Gain(LONG hundredths_db) {
    return hundredths_db <= DSBVOLUME_MIN ? 0.0f : (float)pow(10.0, hundredths_db / 2000.0);
}

/* Adds frames of buffer b to mix. Called with the lock held. */
static void MixBuffer(struct Buffer *b, int frames) {
    DWORD total = Frames(b);
    double step = (double)(b->frequency != 0 ? b->frequency : b->format.nSamplesPerSec) / MIX_RATE;
    float volume = Gain(b->volume);
    float left = volume * (b->pan > 0 ? Gain(-b->pan) : 1.0f);
    float right = volume * (b->pan < 0 ? Gain(b->pan) : 1.0f);
    int i;

    if (total == 0) {
        b->playing = 0;
        return;
    }
    for (i = 0; i < frames; i++) {
        DWORD f0 = (DWORD)b->pos;
        DWORD f1 = f0 + 1;
        float t = (float)(b->pos - f0);
        float l;
        float r;

        if (f1 >= total) {
            f1 = b->looping ? 0 : f0;
        }
        l = SampleAt(b, f0, 0) * (1.0f - t) + SampleAt(b, f1, 0) * t;
        r = b->format.nChannels > 1 ? SampleAt(b, f0, 1) * (1.0f - t) + SampleAt(b, f1, 1) * t : l;
        mix[i * 2] += l * left;
        mix[i * 2 + 1] += r * right;
        b->pos += step;
        if (b->pos >= total) {
            if (b->looping) {
                b->pos -= total * floor(b->pos / total);
            } else {
                b->pos = 0; /* DirectSound rewinds a buffer that played to its end */
                b->playing = 0;
                return;
            }
        }
    }
}

static void SDLCALL Feed(void *userdata, SDL_AudioStream *s, int additional, int total) {
    int inactive = MiniwinWindowInactive();

    (void)userdata;
    (void)total;
    while (additional > 0) {
        int frames = additional / (int)(2 * sizeof(float));
        struct Buffer *b;
        int i;

        if (frames > MIX_CHUNK) {
            frames = MIX_CHUNK;
        }
        if (frames <= 0) {
            frames = 1;
        }
        memset(mix, 0, sizeof(float) * 2 * frames);
        SDL_LockMutex(lock);
        for (b = buffers; b != NULL; b = b->next) {
            /* While the game's window doesn't have the focus the game waits (ProcessSystemEvents), and so do its
             * sounds: a buffer stays "playing" with its cursor held, as DirectSound mutes buffers without
             * DSBCAPS_GLOBALFOCUS. Otherwise loops kept repeating and the movie's sound ran past what the waiting
             * movie had written. */
            if (b->playing && !b->primary && (!inactive || (b->flags & DSBCAPS_GLOBALFOCUS) != 0)) {
                MixBuffer(b, frames);
            }
        }
        SDL_UnlockMutex(lock);
        for (i = 0; i < frames * 2; i++) {
            mix[i] = mix[i] > 1.0f ? 1.0f : mix[i] < -1.0f ? -1.0f
                                                           : mix[i];
        }
        SDL_PutAudioStreamData(s, mix, (int)(sizeof(float) * 2 * frames));
        additional -= (int)(sizeof(float) * 2 * frames);
    }
}

/* ---- IDirectSoundBuffer ---- */

static HRESULT STDMETHODCALLTYPE Buffer_QueryInterface(IDirectSoundBuffer *self, REFIID riid, LPVOID *out) {
    if (SameGuid(riid, &IID_IUnknown) || SameGuid(riid, &IID_IDirectSoundBuffer)) {
        self->lpVtbl->AddRef(self);
        *out = self;
        return S_OK;
    }
    *out = NULL;
    return E_NOINTERFACE;
}

static ULONG STDMETHODCALLTYPE Buffer_AddRef(IDirectSoundBuffer *self) {
    return (ULONG)InterlockedIncrement(&AsBuffer(self)->refs);
}

static ULONG STDMETHODCALLTYPE Buffer_Release(IDirectSoundBuffer *self) {
    struct Buffer *b = AsBuffer(self);
    LONG refs = InterlockedDecrement(&b->refs);
    struct Buffer **p;

    if (refs > 0) {
        return (ULONG)refs;
    }
    SDL_LockMutex(lock);
    for (p = &buffers; *p != NULL; p = &(*p)->next) {
        if (*p == b) {
            *p = b->next;
            break;
        }
    }
    if (--b->data->refs == 0) {
        free(b->data->bytes);
        free(b->data);
    }
    SDL_UnlockMutex(lock);
    free(b);
    return 0;
}

static HRESULT STDMETHODCALLTYPE Buffer_GetCaps(IDirectSoundBuffer *self, LPDSBCAPS caps) {
    struct Buffer *b = AsBuffer(self);

    if (caps == NULL || caps->dwSize < sizeof(DSBCAPS)) {
        return DSERR_INVALIDPARAM;
    }
    caps->dwFlags = b->flags | DSBCAPS_LOCSOFTWARE;
    caps->dwBufferBytes = b->data->size;
    caps->dwUnlockTransferRate = 0;
    caps->dwPlayCpuOverhead = 0;
    return DS_OK;
}

static HRESULT STDMETHODCALLTYPE Buffer_GetCurrentPosition(IDirectSoundBuffer *self, LPDWORD play, LPDWORD write) {
    struct Buffer *b = AsBuffer(self);
    DWORD p;

    SDL_LockMutex(lock);
    p = (DWORD)b->pos * b->format.nBlockAlign;
    if (play != NULL) {
        *play = p;
    }
    if (write != NULL) {
        /* DirectSound's write cursor runs a little ahead of the play cursor while playing (about 10 ms) */
        DWORD ahead = b->playing ? (b->format.nAvgBytesPerSec / 100 / b->format.nBlockAlign) * b->format.nBlockAlign : 0;
        *write = b->data->size != 0 ? (p + ahead) % b->data->size : 0;
    }
    SDL_UnlockMutex(lock);
    return DS_OK;
}

static HRESULT STDMETHODCALLTYPE Buffer_GetFormat(IDirectSoundBuffer *self, LPWAVEFORMATEX format, DWORD size,
    LPDWORD written) {
    struct Buffer *b = AsBuffer(self);
    DWORD n = size < sizeof(WAVEFORMATEX) ? size : sizeof(WAVEFORMATEX);

    if (format != NULL) {
        memcpy(format, &b->format, n);
    }
    if (written != NULL) {
        *written = format != NULL ? n : sizeof(WAVEFORMATEX);
    }
    return DS_OK;
}

static HRESULT STDMETHODCALLTYPE Buffer_GetVolume(IDirectSoundBuffer *self, LPLONG volume) {
    *volume = AsBuffer(self)->volume;
    return DS_OK;
}

static HRESULT STDMETHODCALLTYPE Buffer_GetPan(IDirectSoundBuffer *self, LPLONG pan) {
    *pan = AsBuffer(self)->pan;
    return DS_OK;
}

static HRESULT STDMETHODCALLTYPE Buffer_GetFrequency(IDirectSoundBuffer *self, LPDWORD frequency) {
    struct Buffer *b = AsBuffer(self);

    *frequency = b->frequency != 0 ? b->frequency : b->format.nSamplesPerSec;
    return DS_OK;
}

static HRESULT STDMETHODCALLTYPE Buffer_GetStatus(IDirectSoundBuffer *self, LPDWORD status) {
    struct Buffer *b = AsBuffer(self);

    SDL_LockMutex(lock);
    *status = b->playing ? DSBSTATUS_PLAYING | (b->looping ? DSBSTATUS_LOOPING : 0) : 0;
    SDL_UnlockMutex(lock);
    return DS_OK;
}

static HRESULT STDMETHODCALLTYPE Buffer_Initialize(IDirectSoundBuffer *self, LPDIRECTSOUND ds, LPCDSBUFFERDESC desc) {
    (void)self;
    (void)ds;
    (void)desc;
    return DSERR_ALREADYINITIALIZED;
}

static HRESULT STDMETHODCALLTYPE Buffer_Lock(IDirectSoundBuffer *self, DWORD offset, DWORD bytes, LPVOID *p1,
    LPDWORD b1, LPVOID *p2, LPDWORD b2, DWORD flags) {
    struct Buffer *b = AsBuffer(self);
    DWORD size = b->data->size;

    if (b->primary) {
        return DSERR_PRIOLEVELNEEDED;
    }
    if ((flags & DSBLOCK_FROMWRITECURSOR) != 0) {
        Buffer_GetCurrentPosition(self, NULL, &offset);
    }
    if ((flags & DSBLOCK_ENTIREBUFFER) != 0) {
        bytes = size;
    }
    if (bytes == 0 || bytes > size || offset >= size) {
        return DSERR_INVALIDPARAM;
    }
    *p1 = b->data->bytes + offset;
    if (offset + bytes > size) {
        *b1 = size - offset;
        if (p2 != NULL) {
            *p2 = b->data->bytes;
        }
        if (b2 != NULL) {
            *b2 = bytes - (size - offset);
        }
    } else {
        *b1 = bytes;
        if (p2 != NULL) {
            *p2 = NULL;
        }
        if (b2 != NULL) {
            *b2 = 0;
        }
    }
    return DS_OK;
}

static HRESULT STDMETHODCALLTYPE Buffer_Play(IDirectSoundBuffer *self, DWORD reserved, DWORD priority, DWORD flags) {
    struct Buffer *b = AsBuffer(self);

    (void)reserved;
    (void)priority;
    SDL_LockMutex(lock);
    b->playing = 1;
    b->looping = (flags & DSBPLAY_LOOPING) != 0;
    SDL_UnlockMutex(lock);
    return DS_OK;
}

static HRESULT STDMETHODCALLTYPE Buffer_SetCurrentPosition(IDirectSoundBuffer *self, DWORD position) {
    struct Buffer *b = AsBuffer(self);

    if (position > b->data->size) {
        return DSERR_INVALIDPARAM;
    }
    SDL_LockMutex(lock);
    b->pos = b->format.nBlockAlign != 0 ? (double)(position / b->format.nBlockAlign) : 0;
    SDL_UnlockMutex(lock);
    return DS_OK;
}

static HRESULT STDMETHODCALLTYPE Buffer_SetFormat(IDirectSoundBuffer *self, LPCWAVEFORMATEX format) {
    (void)format;
    return AsBuffer(self)->primary ? DS_OK : DSERR_INVALIDCALL;
}

static HRESULT STDMETHODCALLTYPE Buffer_SetVolume(IDirectSoundBuffer *self, LONG volume) {
    if (volume < DSBVOLUME_MIN || volume > DSBVOLUME_MAX) {
        return DSERR_INVALIDPARAM;
    }
    AsBuffer(self)->volume = volume;
    return DS_OK;
}

static HRESULT STDMETHODCALLTYPE Buffer_SetPan(IDirectSoundBuffer *self, LONG pan) {
    if (pan < DSBPAN_LEFT || pan > DSBPAN_RIGHT) {
        return DSERR_INVALIDPARAM;
    }
    AsBuffer(self)->pan = pan;
    return DS_OK;
}

static HRESULT STDMETHODCALLTYPE Buffer_SetFrequency(IDirectSoundBuffer *self, DWORD frequency) {
    if (frequency != DSBFREQUENCY_ORIGINAL && (frequency < DSBFREQUENCY_MIN || frequency > DSBFREQUENCY_MAX)) {
        return DSERR_INVALIDPARAM;
    }
    AsBuffer(self)->frequency = frequency;
    return DS_OK;
}

static HRESULT STDMETHODCALLTYPE Buffer_Stop(IDirectSoundBuffer *self) {
    SDL_LockMutex(lock);
    AsBuffer(self)->playing = 0;
    SDL_UnlockMutex(lock);
    return DS_OK;
}

static HRESULT STDMETHODCALLTYPE Buffer_Unlock(IDirectSoundBuffer *self, LPVOID p1, DWORD b1, LPVOID p2, DWORD b2) {
    (void)self;
    (void)p1;
    (void)b1;
    (void)p2;
    (void)b2;
    return DS_OK;
}

static HRESULT STDMETHODCALLTYPE Buffer_Restore(IDirectSoundBuffer *self) {
    (void)self;
    return DS_OK;
}

static IDirectSoundBufferVtbl BufferVtbl = {
    Buffer_QueryInterface,
    Buffer_AddRef,
    Buffer_Release,
    Buffer_GetCaps,
    Buffer_GetCurrentPosition,
    Buffer_GetFormat,
    Buffer_GetVolume,
    Buffer_GetPan,
    Buffer_GetFrequency,
    Buffer_GetStatus,
    Buffer_Initialize,
    Buffer_Lock,
    Buffer_Play,
    Buffer_SetCurrentPosition,
    Buffer_SetFormat,
    Buffer_SetVolume,
    Buffer_SetPan,
    Buffer_SetFrequency,
    Buffer_Stop,
    Buffer_Unlock,
    Buffer_Restore,
};

static struct Buffer *NewBuffer(struct SoundData *data, const WAVEFORMATEX *format, DWORD flags) {
    struct Buffer *b = (struct Buffer *)calloc(1, sizeof(*b));

    if (b == NULL) {
        return NULL;
    }
    b->iface.lpVtbl = &BufferVtbl;
    b->refs = 1;
    b->data = data;
    b->format = *format;
    b->format.cbSize = 0;
    b->flags = flags;
    SDL_LockMutex(lock);
    data->refs++;
    b->next = buffers;
    buffers = b;
    SDL_UnlockMutex(lock);
    return b;
}

/* ---- IDirectSound ---- */

static HRESULT STDMETHODCALLTYPE Sound_QueryInterface(IDirectSound *self, REFIID riid, LPVOID *out) {
    if (SameGuid(riid, &IID_IUnknown) || SameGuid(riid, &IID_IDirectSound)) {
        self->lpVtbl->AddRef(self);
        *out = self;
        return S_OK;
    }
    *out = NULL;
    return E_NOINTERFACE;
}

static ULONG STDMETHODCALLTYPE Sound_AddRef(IDirectSound *self) {
    return (ULONG)InterlockedIncrement(&((struct DirectSound *)self)->refs);
}

static ULONG STDMETHODCALLTYPE Sound_Release(IDirectSound *self) {
    struct DirectSound *ds = (struct DirectSound *)self;
    LONG refs = InterlockedDecrement(&ds->refs);

    if (refs == 0) {
        free(ds);
    }
    return (ULONG)refs;
}

static HRESULT STDMETHODCALLTYPE Sound_CreateSoundBuffer(IDirectSound *self, LPCDSBUFFERDESC desc,
    LPDIRECTSOUNDBUFFER *out, LPUNKNOWN outer) {
    struct SoundData *data;
    struct Buffer *b;
    WAVEFORMATEX primary_format = {WAVE_FORMAT_PCM, 2, MIX_RATE, MIX_RATE * 4, 4, 16, 0};
    const WAVEFORMATEX *format;
    int primary;

    (void)self;
    if (out == NULL || desc == NULL || outer != NULL ||
        (desc->dwSize != sizeof(DSBUFFERDESC) && desc->dwSize != sizeof(DSBUFFERDESC1))) {
        return DSERR_INVALIDPARAM;
    }
    *out = NULL;
    primary = (desc->dwFlags & DSBCAPS_PRIMARYBUFFER) != 0;
    format = primary ? &primary_format : desc->lpwfxFormat;
    if (format == NULL || (format->wBitsPerSample != 8 && format->wBitsPerSample != 16) || format->nChannels < 1 ||
        format->nChannels > 2 || format->nBlockAlign == 0) {
        return DSERR_INVALIDPARAM; /* as Windows answers, e.g. for 12-bit samples */
    }
    if (format->wFormatTag != WAVE_FORMAT_PCM) {
        return DSERR_BADFORMAT;
    }
    if (!primary && (desc->dwBufferBytes < DSBSIZE_MIN || desc->dwBufferBytes > DSBSIZE_MAX)) {
        return DSERR_INVALIDPARAM;
    }
    data = (struct SoundData *)calloc(1, sizeof(*data));
    if (data == NULL) {
        return DSERR_OUTOFMEMORY;
    }
    data->size = primary ? MIX_RATE * 4 : desc->dwBufferBytes;
    data->bytes = (unsigned char *)calloc(1, data->size);
    if (data->bytes == NULL) {
        free(data);
        return DSERR_OUTOFMEMORY;
    }
    b = NewBuffer(data, format, desc->dwFlags);
    if (b == NULL) {
        free(data->bytes);
        free(data);
        return DSERR_OUTOFMEMORY;
    }
    b->primary = primary;
    *out = &b->iface;
    return DS_OK;
}

static HRESULT STDMETHODCALLTYPE Sound_GetCaps(IDirectSound *self, LPDSCAPS caps) {
    DWORD size;

    (void)self;
    if (caps == NULL || caps->dwSize < offsetof(DSCAPS, dwFlags) + sizeof(DWORD)) {
        return DSERR_INVALIDPARAM;
    }
    size = caps->dwSize < sizeof(DSCAPS) ? caps->dwSize : sizeof(DSCAPS);
    memset(caps, 0, size);
    caps->dwSize = size;
    caps->dwFlags = DSCAPS_PRIMARYMONO | DSCAPS_PRIMARYSTEREO | DSCAPS_PRIMARY8BIT | DSCAPS_PRIMARY16BIT |
        DSCAPS_CONTINUOUSRATE | DSCAPS_SECONDARYMONO | DSCAPS_SECONDARYSTEREO | DSCAPS_SECONDARY8BIT |
        DSCAPS_SECONDARY16BIT;
    if (size >= offsetof(DSCAPS, dwMaxSecondarySampleRate) + sizeof(DWORD)) {
        caps->dwMinSecondarySampleRate = DSBFREQUENCY_MIN;
        caps->dwMaxSecondarySampleRate = DSBFREQUENCY_MAX;
    }
    return DS_OK;
}

static HRESULT STDMETHODCALLTYPE Sound_DuplicateSoundBuffer(IDirectSound *self, LPDIRECTSOUNDBUFFER original,
    LPDIRECTSOUNDBUFFER *out) {
    struct Buffer *src = AsBuffer(original);
    struct Buffer *b;

    (void)self;
    if (original == NULL || out == NULL || src->primary) {
        return DSERR_INVALIDPARAM;
    }
    b = NewBuffer(src->data, &src->format, src->flags);
    if (b == NULL) {
        *out = NULL;
        return DSERR_OUTOFMEMORY;
    }
    /* the duplicate starts stopped at the start, with the original's volume, pan and frequency */
    b->volume = src->volume;
    b->pan = src->pan;
    b->frequency = src->frequency;
    *out = &b->iface;
    return DS_OK;
}

static HRESULT STDMETHODCALLTYPE Sound_SetCooperativeLevel(IDirectSound *self, HWND hwnd, DWORD level) {
    (void)self;
    (void)hwnd;
    (void)level;
    return DS_OK;
}

static HRESULT STDMETHODCALLTYPE Sound_Compact(IDirectSound *self) {
    (void)self;
    return DS_OK;
}

static HRESULT STDMETHODCALLTYPE Sound_GetSpeakerConfig(IDirectSound *self, LPDWORD config) {
    (void)self;
    *config = DSSPEAKER_STEREO;
    return DS_OK;
}

static HRESULT STDMETHODCALLTYPE Sound_SetSpeakerConfig(IDirectSound *self, DWORD config) {
    (void)self;
    (void)config;
    return DS_OK;
}

static HRESULT STDMETHODCALLTYPE Sound_Initialize(IDirectSound *self, LPCGUID guid) {
    (void)self;
    (void)guid;
    return DSERR_ALREADYINITIALIZED;
}

static IDirectSoundVtbl SoundVtbl = {
    Sound_QueryInterface,
    Sound_AddRef,
    Sound_Release,
    Sound_CreateSoundBuffer,
    Sound_GetCaps,
    Sound_DuplicateSoundBuffer,
    Sound_SetCooperativeLevel,
    Sound_Compact,
    Sound_GetSpeakerConfig,
    Sound_SetSpeakerConfig,
    Sound_Initialize,
};

HRESULT WINAPI MiniwinDirectSoundCreate(LPCGUID guid, LPDIRECTSOUND *out, LPUNKNOWN outer) {
    struct DirectSound *ds;

    (void)guid;
    if (out == NULL || outer != NULL) {
        return DSERR_INVALIDPARAM;
    }
    *out = NULL;
    if (stream == NULL) {
        SDL_AudioSpec spec;

        if (!SDL_InitSubSystem(SDL_INIT_AUDIO)) {
            return DSERR_NODRIVER;
        }
        lock = SDL_CreateMutex();
        spec.format = SDL_AUDIO_F32;
        spec.channels = 2;
        spec.freq = MIX_RATE;
        stream = SDL_OpenAudioDeviceStream(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &spec, Feed, NULL);
        if (stream == NULL || lock == NULL) {
            return DSERR_NODRIVER;
        }
        SDL_ResumeAudioStreamDevice(stream);
    }
    ds = (struct DirectSound *)calloc(1, sizeof(*ds));
    if (ds == NULL) {
        return DSERR_OUTOFMEMORY;
    }
    ds->iface.lpVtbl = &SoundVtbl;
    ds->refs = 1;
    *out = &ds->iface;
    return DS_OK;
}
