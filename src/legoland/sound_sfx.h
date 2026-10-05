#pragma once

#include "legoland.h"

#include <windows.h>
#include <mmsystem.h>

struct SampleBuffer;

struct SampleDef {
    /* 0x00 */ unsigned char pad_0[4];
    /* 0x04 */ int refcount;
    /* 0x08 */ unsigned char pad_8[0x10 - 0x8];
    /* 0x10 */ void *name;
    /* 0x14 */ unsigned char pad_14[0x28 - 0x14];
    /* 0x28 */ struct SampleDef *parent;
    /* 0x2c */ struct SampleBuffer *buffer;
    /* 0x30 */ void *block_30;
    /* 0x34 */ void *block_34;
};

/* Sound-buffer object behind a playable sample. sound_sfx.c drives playback
 * through method_0x20..0x48; sound_music.c reuses the same vtable slot at 0x3c
 * (Apply) to push the sample's source config. One object, one vtable. */
struct SampleBufferVtbl {
    unsigned char pad_0[0x18];
    int(__stdcall *method_0x18)(struct SampleBuffer *self, int *out);
    unsigned char pad_1c[0x20 - 0x1c];
    int(__stdcall *method_0x20)(struct SampleBuffer *self, struct Sample **out);
    int(__stdcall *method_0x24)(struct SampleBuffer *self, unsigned int *out);
    unsigned char pad_28[0x30 - 0x28];
    int(__stdcall *method_0x30)(struct SampleBuffer *self, int arg1, int arg2, int arg3);
    int(__stdcall *method_0x34)(struct SampleBuffer *self, int arg1);
    unsigned char pad_38[0x3c - 0x38];
    int(__stdcall *method_0x3c)(struct SampleBuffer *self, int arg1);
    int(__stdcall *method_0x40)(struct SampleBuffer *self, int arg1);
    int(__stdcall *method_0x44)(struct SampleBuffer *self, int arg1);
    int(__stdcall *method_0x48)(struct SampleBuffer *self);
};

struct SampleBuffer {
    struct SampleBufferVtbl *vtable;
};

/* Canonical playable-sample object. sound_sfx.c manipulates it through the
 * "Sample" field view (next/fade/flags/buffer); sound_music.c through the
 * "PlayableSample" field view (field_c..field_18/flags/buffer). Same heap
 * object, same offsets. */
struct Sample {
    struct Sample *next;
    int refcount;
    unsigned int fade;
    unsigned int source_type;
    void *bloke;
    unsigned int source_x;
    unsigned int source_y;
    unsigned short flags;
    unsigned char pad_1e[0x28 - 0x1e];
    struct SampleDef *active;
    struct SampleBuffer *buffer;
};

/* DirectMusic (DirectX 6) objects used by the interactive music thread (IMT, MusicThreadProc).
 * MSVC6 ships no DirectMusic headers: these follow dmusici.h/dmusicc.h, typing only the
 * methods the game calls, at their vtable offsets. */
struct DirectMusic;
struct DirectMusicComposer;
struct DirectMusicLoader;
struct DirectMusicPerformance;
struct DirectMusicPort;
struct DirectMusicSegment;

/* DMUS_OBJECTDESC */
struct DirectMusicObjectDesc {
    /* 0x000 */ unsigned int dwSize;
    /* 0x004 */ unsigned int dwValidData;
    /* 0x008 */ GUID guidObject;
    /* 0x018 */ GUID guidClass;
    /* 0x028 */ FILETIME ftDate;
    /* 0x030 */ unsigned int vVersion[2];
    /* 0x038 */ WCHAR wszName[64];
    /* 0x0b8 */ WCHAR wszCategory[64];
    /* 0x138 */ WCHAR wszFileName[MAX_PATH];
    /* 0x340 */ __int64 llMemLength;
    /* 0x348 */ unsigned char *pbMemData;
};

/* DMUS_PORTPARAMS */
struct DirectMusicPortParams {
    /* 0x00 */ unsigned int dwSize;
    /* 0x04 */ unsigned int dwValidParams;
    /* 0x08 */ unsigned int dwVoices;
    /* 0x0c */ unsigned int dwChannelGroups;
    /* 0x10 */ unsigned int dwAudioChannels;
    /* 0x14 */ unsigned int dwSampleRate;
    /* 0x18 */ unsigned int dwEffectFlags;
    /* 0x1c */ BOOL fShare;
};

/* DMUS_NOTIFICATION_PMSG */
struct DirectMusicNotification {
    /* 0x00 */ unsigned char pmsg[0x38];
    /* 0x38 */ GUID guidNotificationType;
    /* 0x48 */ unsigned int dwNotificationOption;
    /* 0x4c */ unsigned int dwField1;
    /* 0x50 */ unsigned int dwField2;
};

struct DirectMusicVtbl {
    void *QueryInterface;
    void *AddRef;
    unsigned int(__stdcall *Release)(struct DirectMusic *self);
    void *EnumPort;
    void *CreateMusicBuffer;
    HRESULT(__stdcall *CreatePort)(struct DirectMusic *self, const GUID *clsid, struct DirectMusicPortParams *params, struct DirectMusicPort **port, void *outer);
};

struct DirectMusic {
    struct DirectMusicVtbl *vtable;
};

struct DirectMusicPortVtbl {
    void *QueryInterface;
    void *AddRef;
    unsigned int(__stdcall *Release)(struct DirectMusicPort *self);
    unsigned char pad_c[0x3c - 0xc];
    HRESULT(__stdcall *Activate)(struct DirectMusicPort *self, BOOL active);
    unsigned char pad_40[0x48 - 0x40];
    HRESULT(__stdcall *SetDirectSound)(struct DirectMusicPort *self, void *dsound, void *buffer);
    HRESULT(__stdcall *GetFormat)(struct DirectMusicPort *self, WAVEFORMATEX *format, unsigned int *formatSize, unsigned int *bufferSize);
};

struct DirectMusicPort {
    struct DirectMusicPortVtbl *vtable;
};

struct DirectMusicSegmentVtbl {
    void *QueryInterface;
    void *AddRef;
    unsigned int(__stdcall *Release)(struct DirectMusicSegment *self);
    unsigned char pad_c[0x18 - 0xc];
    HRESULT(__stdcall *SetRepeats)(struct DirectMusicSegment *self, unsigned int repeats);
    unsigned char pad_1c[0x4c - 0x1c];
    HRESULT(__stdcall *SetParam)(struct DirectMusicSegment *self, const GUID *type, unsigned int groupBits, unsigned int index, int time, void *param);
};

struct DirectMusicSegment {
    struct DirectMusicSegmentVtbl *vtable;
};

struct DirectMusicLoaderVtbl {
    void *QueryInterface;
    void *AddRef;
    unsigned int(__stdcall *Release)(struct DirectMusicLoader *self);
    HRESULT(__stdcall *GetObject)(struct DirectMusicLoader *self, struct DirectMusicObjectDesc *desc, const GUID *iid, void **out);
    void *SetObject;
    HRESULT(__stdcall *SetSearchDirectory)(struct DirectMusicLoader *self, const GUID *cls, const WCHAR *path, BOOL clear);
    HRESULT(__stdcall *ScanDirectory)(struct DirectMusicLoader *self, const GUID *cls, const WCHAR *ext, const WCHAR *cacheFile);
    void *CacheObject;
    void *ReleaseObject;
    HRESULT(__stdcall *ClearCache)(struct DirectMusicLoader *self, const GUID *cls);
    HRESULT(__stdcall *EnableCache)(struct DirectMusicLoader *self, const GUID *cls, BOOL enable);
    HRESULT(__stdcall *EnumObject)(struct DirectMusicLoader *self, const GUID *cls, unsigned int index, struct DirectMusicObjectDesc *desc);
};

struct DirectMusicLoader {
    struct DirectMusicLoaderVtbl *vtable;
};

struct DirectMusicPerformanceVtbl {
    void *QueryInterface;
    void *AddRef;
    unsigned int(__stdcall *Release)(struct DirectMusicPerformance *self);
    HRESULT(__stdcall *Init)(struct DirectMusicPerformance *self, struct DirectMusic **music, void *dsound, HWND hwnd);
    HRESULT(__stdcall *PlaySegment)(struct DirectMusicPerformance *self, struct DirectMusicSegment *segment, unsigned int flags, __int64 startTime, void **segmentState);
    HRESULT(__stdcall *Stop)(struct DirectMusicPerformance *self, struct DirectMusicSegment *segment, void *segmentState, int time, unsigned int flags);
    unsigned char pad_18[0x44 - 0x18];
    HRESULT(__stdcall *FreePMsg)(struct DirectMusicPerformance *self, struct DirectMusicNotification *msg);
    unsigned char pad_48[0x50 - 0x48];
    HRESULT(__stdcall *SetNotificationHandle)(struct DirectMusicPerformance *self, HANDLE event, __int64 minimum);
    HRESULT(__stdcall *GetNotificationPMsg)(struct DirectMusicPerformance *self, struct DirectMusicNotification **msg);
    HRESULT(__stdcall *AddNotificationType)(struct DirectMusicPerformance *self, const GUID *type);
    void *RemoveNotificationType;
    HRESULT(__stdcall *AddPort)(struct DirectMusicPerformance *self, struct DirectMusicPort *port);
    void *RemovePort;
    HRESULT(__stdcall *AssignPChannelBlock)(struct DirectMusicPerformance *self, unsigned int block, struct DirectMusicPort *port, unsigned int group);
    unsigned char pad_6c[0x88 - 0x6c];
    HRESULT(__stdcall *SetGlobalParam)(struct DirectMusicPerformance *self, const GUID *type, void *data, unsigned int size);
    unsigned char pad_8c[0x98 - 0x8c];
    HRESULT(__stdcall *CloseDown)(struct DirectMusicPerformance *self);
};

struct DirectMusicPerformance {
    struct DirectMusicPerformanceVtbl *vtable;
};

struct DirectMusicComposerVtbl {
    void *QueryInterface;
    void *AddRef;
    unsigned int(__stdcall *Release)(struct DirectMusicComposer *self);
};

struct DirectMusicComposer {
    struct DirectMusicComposerVtbl *vtable;
};

void *ConvertWaveToPcm16(void *data, WAVEFORMATEX *has, unsigned int *size);
LEGO_EXPORT struct SampleDef *CreateSampleFromWAV(const char *path);
LEGO_EXPORT struct Sample *CreatePlayableSample(struct SampleDef *def);
LEGO_EXPORT int PlaySample(struct Sample *sample, unsigned int looping, unsigned int oneshot);
LEGO_EXPORT int PauseSingleSample(struct Sample *sample);
void PauseAllSamples(void);
void ResumeAllSamples(void);
LEGO_EXPORT void DeletePlayableSamples(unsigned int param_1);
LEGO_EXPORT int UpdateSoundVols(void);
LEGO_EXPORT int ResumeSinglyPausedSample(struct Sample *sample);
void FUN_00492980(void);
void FUN_00492990(void);
LEGO_EXPORT int SetSampleFade(struct Sample *sample, unsigned int fade);
LEGO_EXPORT void KillPlayableSample(struct Sample *sample);
LEGO_EXPORT void DeleteSampleDef(struct SampleDef *def);
LEGO_EXPORT int KillSoundSampleSystem(void);
void FreeSample(struct Sample *sample);
void SetInteractiveMusicTheme(int param_1);
BOOL StopInteractiveMusic(void);
void FUN_00492da0(void);
int FUN_00495a50(int param_1);
int StartMusicThread(void *hwnd);
int ShutDownDirectMusic(void);
LEGO_EXPORT void AdjustPSampleFreq(struct Sample *sample, unsigned int param_2);
void SuspendMusicThread(void);
void ResumeMusicThread(void);
void FUN_00492ca0(int param_1);
