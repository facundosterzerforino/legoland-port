#include "sound_sfx.h"
#include <windows.h>
#include <fcntl.h>
#include <io.h>
#include <stdlib.h>
#include <string.h>
#include "debug_alloc.h"
#include "globals.h"
#include "imports.h"
#include "legoland.h"
#include "profile_io.h"
#include "resource.h"
#include "sound_music.h"
#include "stream.h"

struct DirectSoundObj;

struct SampleCounter {
    unsigned char pad_0[4];
    int count;
};

struct SampleDefVtbl {
    void *func_0;
    void *func_4;
    void(__stdcall *func_8)(struct SampleBuffer *self);
};

struct DirectSoundVtbl {
    void *QueryInterface;
    void *AddRef;
    void(__stdcall *Release)(struct DirectSoundObj *self);
};

struct DirectSoundObj {
    struct DirectSoundVtbl *vtable;
};

struct AcmHeader {
    unsigned int cbStruct;
    unsigned int fdwStatus;
    unsigned int dwUser;
    unsigned char *pbSrc;
    unsigned int cbSrcLength;
    unsigned int cbSrcLengthUsed;
    unsigned int dwSrcUser;
    unsigned char *pbDst;
    unsigned int cbDstLength;
    unsigned int cbDstLengthUsed;
    unsigned int dwDstUser;
    unsigned int reserved[10];
};

// FUNCTION: LEGOLAND 0x004921c0
void *ConvertWaveToPcm16(void *data, WAVEFORMATEX *has, unsigned int *size) {
    WAVEFORMATEX *src = has;
    WAVEFORMATEX dst;
    struct AcmHeader hdr;
    unsigned int outSize;
    void *buf;

    dst = *src;
    dst.nBlockAlign = src->nChannels * 2;
    dst.wFormatTag = 1;
    dst.nAvgBytesPerSec = src->nSamplesPerSec * dst.nBlockAlign;
    dst.wBitsPerSample = 16;
    dst.cbSize = 0;
    if (acmStreamOpen((void **)&has, NULL, src, &dst, NULL, 0, 0, 4) != 0) {
        return NULL;
    }
    if (acmStreamSize(has, *size, &outSize, 0) != 0) {
        return NULL;
    }
    buf = malloc(outSize);
    if (buf == NULL) {
        return NULL;
    }
    memset(&hdr, 0, sizeof(hdr));
    hdr.cbStruct = sizeof(hdr);
    hdr.pbSrc = (unsigned char *)data;
    hdr.cbSrcLength = *size;
    hdr.pbDst = (unsigned char *)buf;
    hdr.cbDstLength = outSize;
    if (acmStreamPrepareHeader(has, &hdr, 0) != 0) {
        free(buf);
        return NULL;
    }
    if (acmStreamConvert(has, &hdr, 0x10) != 0) {
        free(buf);
        return NULL;
    }
    free(data);
    *size = hdr.cbDstLengthUsed;
    *src = dst;
    acmStreamUnprepareHeader(has, &hdr, 0);
    return buf;
}

struct WaveBufferDesc {
    unsigned int dwSize;
    unsigned int dwFlags;
    unsigned int dwBufferBytes;
    unsigned int dwReserved;
    WAVEFORMATEX *lpwfxFormat;
    GUID guid3DAlgorithm;
};

// FUNCTION: LEGOLAND 0x00492380
LEGO_EXPORT struct SampleDef *CreateSampleFromWAV(const char *path) {
    LPDIRECTSOUNDBUFFER buffer = NULL;
    struct ResFile *file;
    unsigned int chunk;
    unsigned int size;
    WAVEFORMATEX *format;
    void *data;
    void *converted;
    void *locked;
    DWORD lockedSize;
    struct WaveBufferDesc desc;
    struct SampleDef *sample;

    if (SoundAvailable == 0) {
        return NULL;
    }
    file = RES_OpenFile(path);
    if (file != NULL) {
        do {
            if (RES_ReadFile(file, &chunk, 4) != 4 || chunk != 0x46464952) {
                break;
            }
            if (RES_ReadFile(file, &size, 4) != 4) {
                break;
            }
            if (RES_ReadFile(file, &chunk, 4) != 4 || chunk != 0x45564157) {
                break;
            }
            if (RES_ReadFile(file, &chunk, 4) != 4) {
                break;
            }
            if (RES_ReadFile(file, &size, 4) != 4) {
                break;
            }
            if (size < sizeof(WAVEFORMATEX)) {
                format = malloc(sizeof(WAVEFORMATEX));
            } else {
                format = malloc(size);
            }
            do {
                if (RES_ReadFile(file, format, size) != size) {
                    break;
                }
                if (size <= sizeof(WAVEFORMATEX)) {
                    format->cbSize = 0;
                }
                if (RES_ReadFile(file, &chunk, 4) != 4) {
                    break;
                }
                for (;;) {
                    if (chunk == 0x61746164) {
                        if (RES_ReadFile(file, &size, 4) != 4) {
                            break;
                        }
                        data = malloc(size);
                        if (RES_ReadFile(file, data, size) == size) {
                            converted = ConvertWaveToPcm16(data, format, &size);
                            if (converted != NULL) {
                                data = converted;
                                desc.dwSize = sizeof(desc);
                                desc.dwFlags = 0xe0;
                                desc.dwBufferBytes = size;
                                desc.dwReserved = 0;
                                desc.lpwfxFormat = format;
                                if (((LPDIRECTSOUND)DSound)->lpVtbl->CreateSoundBuffer((LPDIRECTSOUND)DSound, (LPCDSBUFFERDESC)&desc, &buffer, NULL) == 0) {
                                    if (buffer->lpVtbl->Lock(buffer, 0, 0, &locked, &lockedSize, NULL, NULL, 2) == 0) {
                                        memcpy(locked, converted, lockedSize);
                                        buffer->lpVtbl->Unlock(buffer, locked, lockedSize, NULL, 0);
                                        sample = (struct SampleDef *)FUN_004920e0();
                                        if (sample != NULL) {
                                            sample->refcount++;
                                            sample->parent = NULL;
                                            sample->buffer = (struct SampleBuffer *)buffer;
                                            sample->block_30 = format;
                                            sample->block_34 = converted;
                                            RES_CloseFile(file);
                                            return sample;
                                        }
                                    }
                                    buffer->lpVtbl->Release(buffer);
                                }
                            }
                        }
                        free(data);
                        break;
                    }
                    if (RES_ReadFile(file, &size, 4) != 4) {
                        break;
                    }
                    data = malloc(size);
                    if (RES_ReadFile(file, data, size) != size) {
                        free(data);
                        break;
                    }
                    free(data);
                    if (RES_ReadFile(file, &chunk, 4) != 4) {
                        break;
                    }
                }
            } while (0);
            free(format);
        } while (0);
        RES_CloseFile(file);
    }
    return NULL;
}

// FUNCTION: LEGOLAND 0x00492690
LEGO_EXPORT struct Sample *CreatePlayableSample(struct SampleDef *def) {
    struct SampleDef *src = def;
    struct Sample *sample;

    if (SoundAvailable == 0) {
        return 0;
    }
    if (src == 0) {
        return 0;
    }
    while (src->parent != 0) {
        src = src->parent;
    }
    for (;;) {
        if (((LPDIRECTSOUND)DSound)
                ->lpVtbl->DuplicateSoundBuffer((LPDIRECTSOUND)DSound, (LPDIRECTSOUNDBUFFER)src->buffer,
                    (LPDIRECTSOUNDBUFFER *)&def) != 0) {
            break;
        }
        sample = FUN_00492110();
        if (sample == 0) {
            ((LPDIRECTSOUNDBUFFER)def)->lpVtbl->Release((LPDIRECTSOUNDBUFFER)def);
            break;
        }
        src->refcount++;
        sample->refcount++;
        sample->active = src;
        sample->buffer = (struct SampleBuffer *)def;
        return sample;
    }
    return 0;
}

// FUNCTION: LEGOLAND 0x00492710
LEGO_EXPORT int PlaySample(struct Sample *sample, unsigned int looping, unsigned int oneshot) {
    if (SoundAvailable == 0) {
        return 0;
    }
    if (sample == 0) {
        return 0;
    }
    if (sample->active == 0) {
        return 0;
    }

    if (sample->buffer->vtable->method_0x34(sample->buffer, 0) != 0) {
        return 0;
    }

    if (looping != 0) {
        if (sample->buffer->vtable->method_0x30(sample->buffer, 0, 0, 1) != 0) {
            return 0;
        }
        sample->flags |= 4;
    } else {
        if (sample->buffer->vtable->method_0x30(sample->buffer, 0, 0, 0) != 0) {
            return 0;
        }
        sample->flags &= 0xfffb;
    }

    if (oneshot != 0) {
        sample->flags |= 8;
    } else {
        sample->flags &= 0xfff7;
    }
    return 1;
}

// FUNCTION: LEGOLAND 0x004927b0
int StopSampleBuffer(struct Sample *sample) {
    if (SoundAvailable == 0) {
        return 0;
    }
    if (sample == 0) {
        return 0;
    }
    if (sample->active == 0) {
        return 0;
    }
    if (sample->buffer->vtable->method_0x48(sample->buffer) != 0) {
        return 0;
    }
    sample->flags |= 2;
    return 1;
}
// FUNCTION: LEGOLAND 0x00492800
LEGO_EXPORT int PauseSingleSample(struct Sample *sample) {
    if ((sample->flags & 1) == 0 && StopSampleBuffer(sample) != 0) {
        sample->flags |= 1;
        return 1;
    }
    return 0;
}
// FUNCTION: LEGOLAND 0x00492830
void PauseAllSamples(void) {
    struct Sample *sample;

    sample = SampleListHead;
    if (sample != 0) {
        do {
            PauseSingleSample(sample);
            sample = sample->next;
        } while (sample != 0);
    }
}

// FUNCTION: LEGOLAND 0x00492850
void ResumeAllSamples(void) {
    struct Sample *sample;

    sample = SampleListHead;
    if (sample != 0) {
        do {
            ResumeSinglyPausedSample(sample);
            sample = sample->next;
        } while (sample != 0);
    }
}

// FUNCTION: LEGOLAND 0x00492870
LEGO_EXPORT void Mute_SFX(void) {
    struct Sample *sample;

    sample = SampleListHead;
    if (sample != 0) {
        do {
            StopSampleBuffer(sample);
            sample = sample->next;
        } while (sample != 0);
    }
    SfxMuted = 1;
}

// FUNCTION: LEGOLAND 0x004928a0
int FUN_004928a0(struct Sample *sample) {
    if (SoundAvailable == 0) {
        return 0;
    }
    if (sample == 0) {
        return 0;
    }
    if (sample->active == 0) {
        return 0;
    }
    if ((sample->flags & 4) != 0) {
        if (sample->buffer->vtable->method_0x30(sample->buffer, 0, 0, 1) != 0) {
            return 0;
        }
    } else {
        if (sample->buffer->vtable->method_0x30(sample->buffer, 0, 0, 0) != 0) {
            return 0;
        }
    }
    sample->flags &= 0xfffd;
    return 1;
}

// FUNCTION: LEGOLAND 0x00492910
LEGO_EXPORT int ResumeSinglyPausedSample(struct Sample *sample) {
    if (SoundAvailable != 0 && sample != 0 && (sample->flags & 1) != 0) {
        sample->flags &= 0xfffe;
        if (SfxMuted == 0) {
            FUN_004928a0(sample);
        }
        return 1;
    }
    return 0;
}

// FUNCTION: LEGOLAND 0x00492950
LEGO_EXPORT void UnMute_FX(void) {
    struct Sample *sample;

    sample = SampleListHead;
    while (sample != 0) {
        FUN_004928a0(sample);
        sample = sample->next;
    }
    SfxMuted = 0;
}

// FUNCTION: LEGOLAND 0x00492980
void FUN_00492980(void) {
    DAT_007988c8 = 1;
}

// FUNCTION: LEGOLAND 0x00492990
void FUN_00492990(void) {
    DAT_007988c8 = 0;
}

// FUNCTION: LEGOLAND 0x004929a0
LEGO_EXPORT int SetSampleVolume(struct Sample *sample, int volume) {
    if (SoundAvailable == 0) {
        return 0;
    }
    if (sample == 0) {
        return 0;
    }
    if (sample->active == 0) {
        return 0;
    }

    volume -= 100;
    volume <<= 5;
    return sample->buffer->vtable->method_0x3c(sample->buffer, volume) == 0;
}

// FUNCTION: LEGOLAND 0x004929e0
LEGO_EXPORT int SetSamplePan(struct Sample *sample, int pan) {
    if (SoundAvailable == 0) {
        return 0;
    }
    if (sample == 0) {
        return 0;
    }
    if (sample->active == 0) {
        return 0;
    }

    pan = pan * 5 * 5 * 4;
    return sample->buffer->vtable->method_0x40(sample->buffer, pan) == 0;
}

// FUNCTION: LEGOLAND 0x00492a20
LEGO_EXPORT int SetSampleFrequency(struct Sample *sample, int frequency) {
    if (SoundAvailable == 0) {
        return 0;
    }
    if (sample == 0) {
        return 0;
    }
    if (sample->active == 0) {
        return 0;
    }
    return sample->buffer->vtable->method_0x44(sample->buffer, frequency) == 0;
}

// FUNCTION: LEGOLAND 0x00492a60
struct Sample *GetSampleFrequency(struct Sample *sample) {
    struct SampleBuffer *buffer;

    if (SoundAvailable == 0) {
        return 0;
    }
    if (sample == 0) {
        return 0;
    }
    if (sample->active == 0) {
        return 0;
    }

    buffer = sample->buffer;
    return buffer->vtable->method_0x20(buffer, &sample) == 0 ? sample : 0;
}

// FUNCTION: LEGOLAND 0x00492aa0
LEGO_EXPORT void AdjustPSampleFreq(struct Sample *sample, unsigned int param_2) {
    unsigned short range = (unsigned short)param_2;
    int percent = rand() % (range * 2) - range + 100;

    SetSampleFrequency(sample, (int)GetSampleFrequency(sample) * percent / 100);
}

// FUNCTION: LEGOLAND 0x00492af0
LEGO_EXPORT int SetSampleFade(struct Sample *sample, unsigned int fade) {
    if (SoundAvailable == 0) {
        return 0;
    }
    if (sample == 0) {
        return 0;
    }
    if (sample->active == 0) {
        return 0;
    }
    sample->fade = fade;
    return 1;
}

// FUNCTION: LEGOLAND 0x00492b20
void FUN_00492b20(struct Sample *sample) {
    if (sample != 0) {
        ((struct SampleDefVtbl *)sample->buffer->vtable)->func_8(sample->buffer);
        ((struct SampleCounter *)sample->active)->count--;
        free(sample);
    }
}

// FUNCTION: LEGOLAND 0x00492b50
LEGO_EXPORT void KillPlayableSample(struct Sample *sample) {
    struct Sample *node;

    if (SampleListHead == sample) {
        SampleListHead = sample->next;
    } else if (SampleListHead != 0) {
        node = SampleListHead;
        while (node != 0) {
            if (node->next == sample) {
                node->next = sample->next;
                break;
            }
            node = node->next;
        }
    }
    FUN_00492b20(sample);
}

// FUNCTION: LEGOLAND 0x00492b90
LEGO_EXPORT void DeletePlayableSamples(unsigned int param_1) {
    struct Sample *current;
    struct Sample *previous;
    struct Sample *next;

    current = SampleListHead;
    previous = 0;
    if (current == 0) {
        return;
    }
    do {
        next = current->next;
        if (param_1 == 0 || (unsigned int)current->active == param_1) {
            if (previous != 0) {
                previous->next = next;
            } else {
                SampleListHead = next;
            }
            FUN_00492b20(current);
            current = next;
        } else {
            previous = current;
            current = next;
        }
    } while (current != 0);
}

// FUNCTION: LEGOLAND 0x00492be0
LEGO_EXPORT void DeleteSampleDef(struct SampleDef *def) {
    if (def == 0) {
        return;
    }
    DeletePlayableSamples((unsigned int)def);
    free(def->block_30);
    free(def->block_34);
    ((struct SampleDefVtbl *)def->buffer->vtable)->func_8(def->buffer);
    free(def);
}

// FUNCTION: LEGOLAND 0x00492c20
LEGO_EXPORT int KillSoundSampleSystem(void) {
    if (SoundAvailable == 0) {
        return 0;
    }
    DeletePlayableSamples(0);
    ((struct DirectSoundObj *)DSound)->vtable->Release(DSound);
    DSound = 0;
    SoundAvailable = 0;
    return 1;
}

// FUNCTION: LEGOLAND 0x00492c60
void SuspendMusicThread(void) {
    if (MusicEnabled != 0) {
        SuspendThread(MusicThread);
    }
}

// FUNCTION: LEGOLAND 0x00492c80
void ResumeMusicThread(void) {
    if (MusicEnabled != 0) {
        ResumeThread(MusicThread);
    }
}

// FUNCTION: LEGOLAND 0x00492ca0
void FUN_00492ca0(int param_1) {
    if (DAT_004bf778 == 1 || DAT_004bf778 == 2) {
        DAT_0079a6a4 = 3;
        DAT_0079a6a8 = param_1 % 5;
        SetEvent(DAT_0079a6a0);
    }
}

// FUNCTION: LEGOLAND 0x00492ce0
void FUN_00492ce0(int theme) {
    if (DAT_004bf778 == 1 || DAT_004bf778 == 2) {
        FUN_00492ca0(theme);
    } else if (theme == DAT_0079a6ac) {
        // STRING: LEGOLAND 0x004bf7cc
        DBPrintf("IMT:Theme was same %d, continuing\n", DAT_0079a6ac);
    } else if (DAT_004bf778 == 5 || DAT_004bf778 == 6) {
        // STRING: LEGOLAND 0x004bf77c
        DBPrintf("IMT:Changing theme before transition\n");
        DAT_0079a6a8 = theme;
    } else if (DAT_004bf778 == 7) {
        // STRING: LEGOLAND 0x004bf7a4
        DBPrintf("IMT:Already in transition, continuing\n");
    } else {
        DAT_0079a6a4 = 4;
        DAT_0079a6a8 = theme % 5;
        SetEvent(DAT_0079a6a0);
    }
}

// FUNCTION: LEGOLAND 0x00492d80
BOOL FUN_00492d80(void) {
    DAT_0079a6a4 = 1;
    return SetEvent(DAT_0079a6a0);
}

// FUNCTION: LEGOLAND 0x00492da0
void FUN_00492da0(void) {
    FUN_00492ce0(DAT_0079a6ac);
}

#define DMUS_OBJ_CLASS 0x2
#define DMUS_OBJ_NAME 0x4
#define DMUS_OBJ_MEMORY 0x400

// STRING: LEGOLAND 0x004bfd30
#define IMT_MSG_SHORT_READ "Not Enough Data %s (wanted %d, got %d)\n"
// STRING: LEGOLAND 0x004bfd0c
#define IMT_MSG_NO_MEMORY "Failed to allocate Music Object %s\n"
// STRING: LEGOLAND 0x004bfa78
#define IMT_MSG_GET_FAILED "Error Getting music object (Call %d) (Ret = %d) (Error = %d)\n"

/* Reads music file n into DAT_00799c1c[n] (DAT_0079a608[n] bytes). As in the original,
 * "got" is the result of `read != size`, not the number of bytes read. */
#define IMT_LOAD(n, file) \
    fd = _open(file, _O_BINARY); \
    DAT_0079a608[n] = _filelength(fd); \
    DAT_00799c1c[n] = malloc(DAT_0079a608[n]); \
    if (DAT_00799c1c[n] != NULL) { \
        got = _read(fd, DAT_00799c1c[n], DAT_0079a608[n]) != DAT_0079a608[n]; \
        if (got != 0) { \
            DBPrintf(IMT_MSG_SHORT_READ, file, DAT_0079a608[n], got); \
        } \
    } else { \
        DBPrintf(IMT_MSG_NO_MEMORY, file); \
    } \
    _close(fd)

/* Creates segment n from the file IMT_LOAD read into memory, then frees the file. */
#define IMT_GET(n, name) \
    DAT_0079a6b0++; \
    desc.guidClass = CLSID_DirectMusicSegment; \
    desc.dwSize = sizeof(desc); \
    desc.dwValidData = DMUS_OBJ_CLASS | DMUS_OBJ_NAME | DMUS_OBJ_MEMORY; \
    desc.llMemLength = DAT_0079a608[n]; \
    desc.pbMemData = DAT_00799c1c[n]; \
    wcscpy(desc.wszName, name); \
    hr = DMusicLoader->vtable->GetObject(DMusicLoader, &desc, &IID_IDirectMusicSegment, (void **)&DAT_00799230[n]); \
    if (hr != S_OK) { \
        DBPrintf(IMT_MSG_GET_FAILED, DAT_0079a6b0, hr, GetLastError()); \
    } \
    free(DAT_00799c1c[n])

/* Interactive music thread (IMT). Sets up DirectMusic, loads the five themes (two segments each)
 * and the 5x5 transition segments, then serves the commands posted by FUN_00492ca0/FUN_00492ce0/
 * FUN_00492d80 (DAT_0079a6a4, signalled through DAT_0079a6a0) and DirectMusic notifications.
 * Setup failures jump into one release chain, as in the original. */
// FUNCTION: LEGOLAND 0x00492db0
DWORD WINAPI MusicThreadProc(LPVOID param) {
    unsigned char groove;
    int beat = -1;
    struct DirectMusicPort *port = NULL;
    int theme;
    struct DirectMusicNotification *msg;
    struct DirectMusic *music = NULL;
    unsigned int bufferSize;
    unsigned int formatSize;
    struct DirectMusicObjectDesc desc;
    HANDLE events[2];
    struct DirectMusicPortParams params = {sizeof(params), 0, 24, 1, 2, 22050, 0, 0};
    struct WaveBufferDesc bufferDesc;
    WAVEFORMATEX *format;
    HRESULT hr;
    int fd;
    int got;
    int i;
    int k;

    if (MusicEnabled == 0) {
        DAT_007988bc = 1;
        return 0;
    }
    desc.dwSize = sizeof(desc);
    CoInitialize(NULL);
    if (FAILED(CoCreateInstance(&CLSID_DirectMusicComposer, NULL, CLSCTX_INPROC, &IID_IDirectMusicComposer, (void **)&DMusicComposer))) {
        goto fail;
    }
    if (FAILED(CoCreateInstance(&CLSID_DirectMusicPerformance, NULL, CLSCTX_INPROC, &IID_IDirectMusicPerformance, (void **)&DMusicPerformance))) {
        goto release_composer;
    }
    if (FAILED(DMusicPerformance->vtable->Init(DMusicPerformance, &music, DSound, NULL))) {
        goto release_perf;
    }
    music->vtable->CreatePort(music, &NullGuid, &params, &port, NULL);
    port->vtable->GetFormat(port, NULL, &formatSize, &bufferSize);
    format = malloc(formatSize < sizeof(WAVEFORMATEX) ? sizeof(WAVEFORMATEX) : formatSize);
    port->vtable->GetFormat(port, format, &formatSize, &bufferSize);
    bufferDesc.dwSize = sizeof(bufferDesc);
    bufferDesc.dwFlags = DSBCAPS_CTRLVOLUME;
    bufferDesc.dwBufferBytes = bufferSize;
    bufferDesc.dwReserved = 0;
    bufferDesc.lpwfxFormat = format;
    if (IDirectSound_CreateSoundBuffer((LPDIRECTSOUND)DSound, (LPCDSBUFFERDESC)&bufferDesc, (LPDIRECTSOUNDBUFFER *)&DMusicSoundBuffer, NULL) != DS_OK) {
        goto fail;
    }
    free(format);
    port->vtable->SetDirectSound(port, DSound, DMusicSoundBuffer);
    port->vtable->Activate(port, TRUE);
    DMusicPerformance->vtable->AddPort(DMusicPerformance, port);
    DMusicPerformance->vtable->AssignPChannelBlock(DMusicPerformance, 0, port, 1);
    if (port != NULL) {
        port->vtable->Release(port);
    }
    if (FAILED(CoCreateInstance(&CLSID_DirectMusicLoader, NULL, CLSCTX_INPROC, &IID_IDirectMusicLoader, (void **)&DMusicLoader))) {
        goto stop_perf;
    }
    UpdateSoundVols();
    DAT_0079a69c = CreateEvent(NULL, TRUE, FALSE, NULL);
    DAT_0079a6a0 = CreateEvent(NULL, TRUE, FALSE, NULL);
    if (DAT_0079a6a0 == NULL) {
        goto clear_loader;
    }
    // STRING: LEGOLAND 0x004bfda4
    DMusicLoader->vtable->SetSearchDirectory(DMusicLoader, &GUID_DirectMusicAllTypes, L"imusic", TRUE);
    DMusicLoader->vtable->EnableCache(DMusicLoader, &GUID_DirectMusicAllTypes, TRUE);
    // STRING: LEGOLAND 0x004bfd9c
    if (DMusicLoader->vtable->ScanDirectory(DMusicLoader, &CLSID_DirectMusicSegment, L"sgt", NULL) == S_OK) {
        // STRING: LEGOLAND 0x004bfd94
        if (DMusicLoader->vtable->ScanDirectory(DMusicLoader, &CLSID_DirectMusicStyle, L"sty", NULL) == S_OK) {
            // STRING: LEGOLAND 0x004bfd84
            DBPrintf("Loading Styles\n");
            for (i = 0; DMusicLoader->vtable->EnumObject(DMusicLoader, &CLSID_DirectMusicStyle, i, &desc) == S_OK; i++) {
                DMusicLoader->vtable->GetObject(DMusicLoader, &desc, &IID_IDirectMusicStyle, &DAT_007988d0[i]);
            }
        }
        // STRING: LEGOLAND 0x004bfd70
        DBPrintf("Loading Segments\n");
        // STRING: LEGOLAND 0x004bfd58
        IMT_LOAD(0, "imusic\\segtheme1.sgt");
        // STRING: LEGOLAND 0x004bfcf4
        IMT_LOAD(1, "imusic\\segegypt1.sgt");
        // STRING: LEGOLAND 0x004bfce0
        IMT_LOAD(2, "imusic\\seginca1.sgt");
        // STRING: LEGOLAND 0x004bfccc
        IMT_LOAD(3, "imusic\\segmed1.sgt");
        // STRING: LEGOLAND 0x004bfcb8
        IMT_LOAD(4, "imusic\\segwest1.sgt");
        // STRING: LEGOLAND 0x004bfca0
        IMT_LOAD(5, "imusic\\segtheme2.sgt");
        // STRING: LEGOLAND 0x004bfc88
        IMT_LOAD(6, "imusic\\segegypt2.sgt");
        // STRING: LEGOLAND 0x004bfc74
        IMT_LOAD(7, "imusic\\seginca2.sgt");
        // STRING: LEGOLAND 0x004bfc60
        IMT_LOAD(8, "imusic\\segmed2.sgt");
        // STRING: LEGOLAND 0x004bfc4c
        IMT_LOAD(9, "imusic\\segwest2.sgt");
        // STRING: LEGOLAND 0x004bfc38
        IMT_LOAD(11, "imusic\\letran2.sgt");
        // STRING: LEGOLAND 0x004bfc24
        IMT_LOAD(12, "imusic\\litran2.sgt");
        // STRING: LEGOLAND 0x004bfc10
        IMT_LOAD(13, "imusic\\lmtran2.sgt");
        // STRING: LEGOLAND 0x004bfbfc
        IMT_LOAD(14, "imusic\\lwtran2.sgt");
        // STRING: LEGOLAND 0x004bfbe8
        IMT_LOAD(15, "imusic\\eltran2.sgt");
        // STRING: LEGOLAND 0x004bfbd4
        IMT_LOAD(17, "imusic\\ietran2.sgt");
        // STRING: LEGOLAND 0x004bfbc0
        IMT_LOAD(18, "imusic\\emtran2.sgt");
        // STRING: LEGOLAND 0x004bfbac
        IMT_LOAD(19, "imusic\\ewtran2.sgt");
        // STRING: LEGOLAND 0x004bfb98
        IMT_LOAD(20, "imusic\\iltran2.sgt");
        IMT_LOAD(21, "imusic\\ietran2.sgt");
        // STRING: LEGOLAND 0x004bfb84
        IMT_LOAD(23, "imusic\\imtran2.sgt");
        // STRING: LEGOLAND 0x004bfb70
        IMT_LOAD(24, "imusic\\iwtran2.sgt");
        // STRING: LEGOLAND 0x004bfb5c
        IMT_LOAD(25, "imusic\\mltran2.sgt");
        // STRING: LEGOLAND 0x004bfb48
        IMT_LOAD(26, "imusic\\metran2.sgt");
        // STRING: LEGOLAND 0x004bfb34
        IMT_LOAD(27, "imusic\\mitran2.sgt");
        // STRING: LEGOLAND 0x004bfb20
        IMT_LOAD(29, "imusic\\mwtran2.sgt");
        // STRING: LEGOLAND 0x004bfb0c
        IMT_LOAD(30, "imusic\\wltran2.sgt");
        // STRING: LEGOLAND 0x004bfaf8
        IMT_LOAD(31, "imusic\\wetran2.sgt");
        // STRING: LEGOLAND 0x004bfae4
        IMT_LOAD(32, "imusic\\witran2.sgt");
        // STRING: LEGOLAND 0x004bfad0
        IMT_LOAD(33, "imusic\\wmtran2.sgt");
        // STRING: LEGOLAND 0x004bfab8
        IMT_GET(0, L"themeintro");
        // STRING: LEGOLAND 0x004bfa6c
        IMT_GET(5, L"theme");
        // STRING: LEGOLAND 0x004bfa58
        IMT_GET(1, L"segegypt1");
        // STRING: LEGOLAND 0x004bfa44
        IMT_GET(2, L"seginca1");
        // STRING: LEGOLAND 0x004bfa34
        IMT_GET(3, L"segmed1");
        // STRING: LEGOLAND 0x004bfa20
        IMT_GET(4, L"segwest1");
        // STRING: LEGOLAND 0x004bfa0c
        IMT_GET(6, L"segegypt2");
        // STRING: LEGOLAND 0x004bf9f8
        IMT_GET(7, L"seginca2");
        // STRING: LEGOLAND 0x004bf9e8
        IMT_GET(8, L"segmed2");
        // STRING: LEGOLAND 0x004bf9d4
        IMT_GET(9, L"segwest2");
        // STRING: LEGOLAND 0x004bf9c4
        IMT_GET(11, L"letran2");
        // STRING: LEGOLAND 0x004bf9b4
        IMT_GET(12, L"litran2");
        // STRING: LEGOLAND 0x004bf9a4
        IMT_GET(13, L"lmtran2");
        // STRING: LEGOLAND 0x004bf994
        IMT_GET(14, L"lwtran2");
        // STRING: LEGOLAND 0x004bf984
        IMT_GET(15, L"eltran2");
        // STRING: LEGOLAND 0x004bf974
        IMT_GET(17, L"eitran2");
        // STRING: LEGOLAND 0x004bf964
        IMT_GET(18, L"emtran2");
        // STRING: LEGOLAND 0x004bf954
        IMT_GET(19, L"ewtran2");
        // STRING: LEGOLAND 0x004bf944
        IMT_GET(20, L"iltran2");
        // STRING: LEGOLAND 0x004bf934
        IMT_GET(21, L"ietran2");
        // STRING: LEGOLAND 0x004bf924
        IMT_GET(23, L"imtran2");
        // STRING: LEGOLAND 0x004bf914
        IMT_GET(24, L"iwtran2");
        // STRING: LEGOLAND 0x004bf904
        IMT_GET(25, L"mltran2");
        // STRING: LEGOLAND 0x004bf8f4
        IMT_GET(26, L"metran2");
        // STRING: LEGOLAND 0x004bf8e4
        IMT_GET(27, L"mitran2");
        // STRING: LEGOLAND 0x004bf8d4
        IMT_GET(29, L"mwtran2");
        // STRING: LEGOLAND 0x004bf8c4
        IMT_GET(30, L"wltran2");
        // STRING: LEGOLAND 0x004bf8b4
        IMT_GET(31, L"wetran2");
        // STRING: LEGOLAND 0x004bf8a4
        IMT_GET(32, L"witran2");
        // STRING: LEGOLAND 0x004bf894
        IMT_GET(33, L"wmtran2");
    }
    DMusicPerformance->vtable->SetNotificationHandle(DMusicPerformance, DAT_0079a69c, 0);
    DMusicPerformance->vtable->AddNotificationType(DMusicPerformance, &GUID_NOTIFICATION_MEASUREANDBEAT);
    DMusicPerformance->vtable->AddNotificationType(DMusicPerformance, &GUID_NOTIFICATION_SEGMENT);
    for (theme = 1; theme < 5; theme++) {
        DAT_00799230[theme]->vtable->SetParam(DAT_00799230[theme], &GUID_Download, 0xffffffff, 0, 0, DMusicPerformance);
        DAT_00799230[theme]->vtable->SetRepeats(DAT_00799230[theme], 0);
        DAT_00799230[theme + 5]->vtable->SetParam(DAT_00799230[theme + 5], &GUID_Download, 0xffffffff, 0, 0, DMusicPerformance);
        DAT_00799230[theme + 5]->vtable->SetRepeats(DAT_00799230[theme + 5], 0);
        for (k = 0; k < 5; k++) {
            if (k != theme) {
                DAT_00799230[10 + theme * 5 + k]->vtable->SetParam(DAT_00799230[10 + theme * 5 + k], &GUID_Download, 0xffffffff, 0, 0, DMusicPerformance);
                DAT_00799230[10 + theme * 5 + k]->vtable->SetRepeats(DAT_00799230[10 + theme * 5 + k], 0);
            }
        }
    }
    DAT_007988bc = 1;
    DMusicInitialised = 1;
    events[0] = DAT_0079a69c;
    events[1] = DAT_0079a6a0;
    // STRING: LEGOLAND 0x004bf87c
    DBPrintf("Entering IMT Control\n");
    for (;;) {
        WaitForMultipleObjects(2, events, FALSE, INFINITE);
        if (WaitForSingleObject(DAT_0079a6a0, 0) == WAIT_OBJECT_0) {
            ResetEvent(DAT_0079a6a0);
            if (DAT_0079a6a4 != 0) {
                switch (DAT_0079a6a4) {
                case 4:
                    if (DAT_0079a6ac == DAT_0079a6a8) {
                        DAT_0079a6ac = DAT_0079a6a8;
                        groove = 0;
                        DMusicPerformance->vtable->SetGlobalParam(DMusicPerformance, &GUID_PerfMasterGrooveLevel, &groove, 1);
                        DMusicPerformance->vtable->PlaySegment(DMusicPerformance, DAT_00799230[DAT_0079a6ac + 5], 0x2000, 0, NULL);
                    } else {
                        groove = 1;
                        DAT_0079a6a4 = 5;
                        beat = -1;
                        DMusicPerformance->vtable->SetGlobalParam(DMusicPerformance, &GUID_PerfMasterGrooveLevel, &groove, 1);
                    }
                    break;
                case 3:
                    DAT_0079a6ac = DAT_0079a6a8;
                    groove = 0;
                    DMusicPerformance->vtable->SetGlobalParam(DMusicPerformance, &GUID_PerfMasterGrooveLevel, &groove, 1);
                    DMusicPerformance->vtable->PlaySegment(DMusicPerformance, DAT_00799230[DAT_0079a6ac], 0x2000, 0, NULL);
                    break;
                case 1:
                    DMusicPerformance->vtable->Stop(DMusicPerformance, NULL, NULL, 0, 0);
                    break;
                }
                DAT_004bf778 = DAT_0079a6a4;
                DAT_0079a6a4 = 0;
            }
        }
        if (WaitForSingleObject(DAT_0079a69c, 0) == WAIT_OBJECT_0) {
            ResetEvent(DAT_0079a69c);
            while (DMusicPerformance->vtable->GetNotificationPMsg(DMusicPerformance, &msg) == S_OK) {
                if (IsEqualGUID(&msg->guidNotificationType, &GUID_NOTIFICATION_SEGMENT)) {
                    switch (msg->dwNotificationOption) {
                    case 4: /* DMUS_NOTIFICATION_SEGABORT */
                        // STRING: LEGOLAND 0x004bf864
                        DBPrintf("IMT:Segment stopped\n");
                        if (DAT_004bf778 == 6) {
                            DAT_004bf778 = 7;
                        }
                        break;
                    case 2: /* DMUS_NOTIFICATION_SEGALMOSTEND */
                        // STRING: LEGOLAND 0x004bf84c
                        DBPrintf("IMT:Segment almost end\n");
                        if (DAT_004bf778 == 3 || DAT_004bf778 == 4) {
                            DAT_0079a6a4 = 4;
                            DAT_0079a6a8 = DAT_0079a6ac;
                            SetEvent(DAT_0079a6a0);
                        }
                        break;
                    case 1: /* DMUS_NOTIFICATION_SEGEND */
                        // STRING: LEGOLAND 0x004bf838
                        DBPrintf("IMT:Segment end\n");
                        break;
                    case 3: /* DMUS_NOTIFICATION_SEGLOOP */
                        // STRING: LEGOLAND 0x004bf824
                        DBPrintf("IMT:Segment looped\n");
                        break;
                    case 0: /* DMUS_NOTIFICATION_SEGSTART */
                        // STRING: LEGOLAND 0x004bf80c
                        DBPrintf("IMT:Segment started\n");
                        break;
                    }
                } else if (DAT_004bf778 == 7) {
                    if (msg->dwField1 == 3 && msg->dwField2 == 1) {
                        DAT_0079a6ac = DAT_0079a6a8;
                        DMusicPerformance->vtable->PlaySegment(DMusicPerformance, DAT_00799230[DAT_0079a6ac + 5], 0x2000, 0, NULL);
                        groove = 0;
                        DMusicPerformance->vtable->SetGlobalParam(DMusicPerformance, &GUID_PerfMasterGrooveLevel, &groove, 1);
                        DAT_004bf778 = 4;
                        // STRING: LEGOLAND 0x004bf7f0
                        DBPrintf("IMT_INTERACTIVE command\n");
                    }
                } else {
                    if (DAT_004bf778 == 5 && beat == -1) {
                        beat = msg->dwField1;
                    }
                    if (msg->dwField1 == 0 && DAT_004bf778 == 5) {
                        if (beat != 0) {
                            DAT_004bf778 = 6;
                            DMusicPerformance->vtable->PlaySegment(DMusicPerformance, DAT_00799230[10 + DAT_0079a6ac * 5 + DAT_0079a6a8], 0x2000, 0, NULL);
                        } else {
                            beat = 1;
                        }
                    }
                }
                DMusicPerformance->vtable->FreePMsg(DMusicPerformance, msg);
            }
        }
    }
clear_loader:
    DMusicLoader->vtable->ClearCache(DMusicLoader, &GUID_DirectMusicAllTypes);
    DMusicLoader->vtable->Release(DMusicLoader);
stop_perf:
    DMusicPerformance->vtable->Stop(DMusicPerformance, NULL, NULL, 0, 0);
    DMusicPerformance->vtable->CloseDown(DMusicPerformance);
release_perf:
    DMusicPerformance->vtable->Release(DMusicPerformance);
release_composer:
    DMusicComposer->vtable->Release(DMusicComposer);
fail:
    DMusicInitialised = 0;
    DAT_007988bc = 1;
    return 0;
}

// FUNCTION: LEGOLAND 0x00495a10
int StartMusicThread(void *hwnd) {
    if (MusicEnabled != 0) {
        MusicThread = CreateThread(0, 0x4000, MusicThreadProc, 0, 0, (LPDWORD)&MusicThreadId);
        return 1;
    }
    DAT_007988bc = 1;
    return 0;
}

// FUNCTION: LEGOLAND 0x00495a50
int FUN_00495a50(int param_1) {
    int v;

    v = param_1 * 4000 / 100 - 4000;
    if (v == -4000) {
        v = -10000;
    }
    return v;
}
// FUNCTION: LEGOLAND 0x00495a90
LEGO_EXPORT int UpdateSoundVols(void) {
    int vol;

    if (SoundAvailable != 0) {
        if (MusicEnabled != 0 && DMusicInitialised != 0) {
            vol = FUN_00495a50(CurrentProfile.field_28);
            ((struct SampleBuffer *)DMusicSoundBuffer)->vtable->method_0x3c((struct SampleBuffer *)DMusicSoundBuffer, vol);
        }
        DAT_007988a0 = FUN_00495a50(CurrentProfile.field_2c);
        FUN_004967b0();
        SpeechSetVolume(FUN_00495a50(CurrentProfile.field_24));
    }
    return 0;
}
// FUNCTION: LEGOLAND 0x00495b00
int ShutDownDirectMusic(void) {
    if (MusicEnabled != 0 && DMusicInitialised != 0) {
        TerminateThread(MusicThread, 0);

        DMusicLoader->vtable->ClearCache(DMusicLoader, &GUID_DirectMusicAllTypes);
        DMusicLoader->vtable->Release(DMusicLoader);

        DMusicPerformance->vtable->Stop(DMusicPerformance, NULL, NULL, 0, 0);
        DMusicPerformance->vtable->CloseDown(DMusicPerformance);
        DMusicPerformance->vtable->Release(DMusicPerformance);

        DMusicComposer->vtable->Release(DMusicComposer);

        DMusicInitialised = 0;
    }
    return 1;
}
