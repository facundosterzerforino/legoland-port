/* The parts of DirectX's dmusici.h the music tools use (the Windows SDK no longer ships it): the interfaces as
 * vtables of the methods called, at their DirectX 6 offsets, and the performance message layouts. */
#ifndef TOOLS_MUSIC_DMI_H
#define TOOLS_MUSIC_DMI_H

#include <windows.h>

typedef long MUSIC_TIME;
typedef LONGLONG REFERENCE_TIME;

static const GUID CLSID_DMPerformance = {0xd2ac2881, 0xb39b, 0x11d1, {0x87, 0x04, 0x00, 0x60, 0x08, 0x93, 0xb1, 0xbd}};
static const GUID IID_DMPerformance = {0x07d43d03, 0x6523, 0x11d2, {0x87, 0x1d, 0x00, 0x60, 0x08, 0x93, 0xb1, 0xbd}};
static const GUID CLSID_DMLoader = {0xd2ac2892, 0xb39b, 0x11d1, {0x87, 0x04, 0x00, 0x60, 0x08, 0x93, 0xb1, 0xbd}};
static const GUID IID_DMLoader = {0x2ffaaca2, 0x5dca, 0x11d2, {0xaf, 0xa6, 0x00, 0xaa, 0x00, 0x24, 0xd8, 0xb6}};
static const GUID CLSID_DMSegment = {0xd2ac2882, 0xb39b, 0x11d1, {0x87, 0x04, 0x00, 0x60, 0x08, 0x93, 0xb1, 0xbd}};
static const GUID IID_DMSegment = {0xf96029a2, 0x4282, 0x11d2, {0x87, 0x17, 0x00, 0x60, 0x08, 0x93, 0xb1, 0xbd}};
static const GUID CLSID_DMGraph = {0xd2ac2884, 0xb39b, 0x11d1, {0x87, 0x04, 0x00, 0x60, 0x08, 0x93, 0xb1, 0xbd}};
static const GUID IID_DMGraph = {0x2befc277, 0x5497, 0x11d2, {0xbc, 0xcb, 0x00, 0xa0, 0xc9, 0x22, 0xe6, 0xeb}};
static const GUID IID_DMTool = {0xd2ac28ba, 0xb39b, 0x11d1, {0x87, 0x04, 0x00, 0x60, 0x08, 0x93, 0xb1, 0xbd}};
static const GUID GUID_DMAllTypes = {0xd2ac2893, 0xb39b, 0x11d1, {0x87, 0x04, 0x00, 0x60, 0x08, 0x93, 0xb1, 0xbd}};
static const GUID GUID_DMDownload = {0xd2ac28a7, 0xb39b, 0x11d1, {0x87, 0x04, 0x00, 0x60, 0x08, 0x93, 0xb1, 0xbd}};
static const GUID GUID_DMMasterTempo = {0xd2ac28b0, 0xb39b, 0x11d1, {0x87, 0x04, 0x00, 0x60, 0x08, 0x93, 0xb1, 0xbd}};
static const GUID GUID_DMMasterGrooveLevel = {0xd2ac28b2, 0xb39b, 0x11d1, {0x87, 0x04, 0x00, 0x60, 0x08, 0x93, 0xb1, 0xbd}};
static const GUID GUID_DMNotifySegment = {0xd2ac2899, 0xb39b, 0x11d1, {0x87, 0x04, 0x00, 0x60, 0x08, 0x93, 0xb1, 0xbd}};
static const GUID GUID_DMNotifyMeasureBeat = {0xd2ac289a, 0xb39b, 0x11d1, {0x87, 0x04, 0x00, 0x60, 0x08, 0x93, 0xb1, 0xbd}};

#define DMUS_PMSGT_MIDI 0
#define DMUS_PMSGT_NOTE 1
#define DMUS_PMSGT_SYSEX 2
#define DMUS_PMSGT_NOTIFICATION 3
#define DMUS_PMSGT_TEMPO 4
#define DMUS_PMSGT_CURVE 5
#define DMUS_PMSGT_TIMESIG 6
#define DMUS_PMSGT_PATCH 7
#define DMUS_PMSGT_TRANSPOSE 8
#define DMUS_PMSGT_CHANNEL_PRIORITY 9
#define DMUS_PMSGT_STOP 10
#define DMUS_PMSGT_DIRTY 11

#define DMUS_S_REQUEUE ((HRESULT)0x08780200)
#define DMUS_S_FREE ((HRESULT)0x08780201)
#define DMUS_PMSGF_TOOL_IMMEDIATE 0x10

struct DMGraph;
struct DMTool;
struct DMPerformance;

/* DMUS_PMSG_PART: 0x38 bytes */
typedef struct DMPMsg {
    DWORD dwSize;
    REFERENCE_TIME rtTime;
    MUSIC_TIME mtTime;
    DWORD dwFlags;
    DWORD dwPChannel;
    DWORD dwVirtualTrackID;
    struct DMTool *pTool;
    struct DMGraph *pGraph;
    DWORD dwType;
    DWORD dwVoiceID;
    DWORD dwGroupID;
    IUnknown *punkUser;
} DMPMsg;

typedef struct DMNotePMsg {
    DMPMsg head;
    MUSIC_TIME mtDuration;
    WORD wMusicValue;
    WORD wMeasure;
    short nOffset;
    BYTE bBeat, bGrid, bVelocity, bFlags, bTimeRange, bDurRange, bVelRange, bPlayModeFlags, bSubChordLevel,
        bMidiValue;
    char cTranspose;
} DMNotePMsg;

typedef struct DMCurvePMsg {
    DMPMsg head;
    MUSIC_TIME mtDuration, mtOriginalStart, mtResetDuration;
    short nStartValue, nEndValue, nResetValue;
    WORD wMeasure;
    short nOffset;
    BYTE bBeat, bGrid, bType, bCurveShape, bCCData, bFlags;
} DMCurvePMsg;

typedef struct DMMidiPMsg {
    DMPMsg head;
    BYTE bStatus, bByte1, bByte2, bPad;
} DMMidiPMsg;

typedef struct DMPatchPMsg {
    DMPMsg head;
    BYTE byInstrument, byMSB, byLSB, byPad;
} DMPatchPMsg;

typedef struct DMTempoPMsg {
    DMPMsg head;
    double dblTempo;
} DMTempoPMsg;

typedef struct DMTimeSigPMsg {
    DMPMsg head;
    BYTE bBeatsPerMeasure, bBeat;
    WORD wGridsPerBeat;
} DMTimeSigPMsg;

typedef struct DMTransposePMsg {
    DMPMsg head;
    short nTranspose;
} DMTransposePMsg;

typedef struct DMNotificationPMsg {
    DMPMsg head;
    GUID guidNotificationType;
    DWORD dwNotificationOption, dwField1, dwField2;
} DMNotificationPMsg;

typedef struct DMObjectDesc {
    DWORD dwSize, dwValidData;
    GUID guidObject, guidClass;
    FILETIME ftDate;
    DWORD vVersion[2];
    WCHAR wszName[64], wszCategory[64], wszFileName[MAX_PATH];
    __int64 llMemLength;
    BYTE *pbMemData;
} DMObjectDesc;

typedef struct DMPortParams {
    DWORD dwSize, dwValidParams, dwVoices, dwChannelGroups, dwAudioChannels, dwSampleRate, dwEffectFlags;
    BOOL fShare;
} DMPortParams;

/* IDirectMusicTool, implemented by the caller */
struct DMToolVtbl {
    HRESULT(__stdcall *QueryInterface)(struct DMTool *, REFIID, void **);
    ULONG(__stdcall *AddRef)(struct DMTool *);
    ULONG(__stdcall *Release)(struct DMTool *);
    HRESULT(__stdcall *Init)(struct DMTool *, struct DMGraph *);
    HRESULT(__stdcall *GetMsgDeliveryType)(struct DMTool *, DWORD *);
    HRESULT(__stdcall *GetMediaTypeArraySize)(struct DMTool *, DWORD *);
    HRESULT(__stdcall *GetMediaTypes)(struct DMTool *, DWORD **, DWORD);
    HRESULT(__stdcall *ProcessPMsg)(struct DMTool *, struct DMPerformance *, DMPMsg *);
    HRESULT(__stdcall *Flush)(struct DMTool *, struct DMPerformance *, DMPMsg *, REFERENCE_TIME);
};
struct DMTool {
    struct DMToolVtbl *v;
};

struct DMGraphVtbl {
    void *QueryInterface, *AddRef;
    ULONG(__stdcall *Release)(struct DMGraph *);
    HRESULT(__stdcall *StampPMsg)(struct DMGraph *, DMPMsg *);
    HRESULT(__stdcall *InsertTool)(struct DMGraph *, struct DMTool *, DWORD *, DWORD, LONG);
};
struct DMGraph {
    struct DMGraphVtbl *v;
};

struct DMPort;
struct DMPortVtbl {
    void *QueryInterface, *AddRef;
    ULONG(__stdcall *Release)(struct DMPort *);
    BYTE pad_c[0x3c - 0xc];
    HRESULT(__stdcall *Activate)(struct DMPort *, BOOL);
    BYTE pad_40[0x48 - 0x40];
    HRESULT(__stdcall *SetDirectSound)(struct DMPort *, void *, void *);
    HRESULT(__stdcall *GetFormat)(struct DMPort *, WAVEFORMATEX *, DWORD *, DWORD *);
};
struct DMPort {
    struct DMPortVtbl *v;
};

struct DMMusic;
struct DMMusicVtbl {
    void *QueryInterface, *AddRef, *Release, *EnumPort, *CreateMusicBuffer;
    HRESULT(__stdcall *CreatePort)(struct DMMusic *, const GUID *, DMPortParams *, struct DMPort **, void *);
};
struct DMMusic {
    struct DMMusicVtbl *v;
};

struct DMSegment;
struct DMSegmentVtbl {
    void *QueryInterface, *AddRef;
    ULONG(__stdcall *Release)(struct DMSegment *);
    HRESULT(__stdcall *GetLength)(struct DMSegment *, MUSIC_TIME *);
    void *SetLength;
    HRESULT(__stdcall *GetRepeats)(struct DMSegment *, DWORD *);
    HRESULT(__stdcall *SetRepeats)(struct DMSegment *, DWORD);
    BYTE pad_1c[0x4c - 0x1c];
    HRESULT(__stdcall *SetParam)(struct DMSegment *, const GUID *, DWORD, DWORD, MUSIC_TIME, void *);
};
struct DMSegment {
    struct DMSegmentVtbl *v;
};

struct DMLoader;
struct DMLoaderVtbl {
    void *QueryInterface, *AddRef;
    ULONG(__stdcall *Release)(struct DMLoader *);
    HRESULT(__stdcall *GetObject)(struct DMLoader *, DMObjectDesc *, const GUID *, void **);
    void *SetObject;
    HRESULT(__stdcall *SetSearchDirectory)(struct DMLoader *, const GUID *, const WCHAR *, BOOL);
    HRESULT(__stdcall *ScanDirectory)(struct DMLoader *, const GUID *, const WCHAR *, const WCHAR *);
    void *CacheObject, *ReleaseObject;
    HRESULT(__stdcall *ClearCache)(struct DMLoader *, const GUID *);
    HRESULT(__stdcall *EnableCache)(struct DMLoader *, const GUID *, BOOL);
};
struct DMLoader {
    struct DMLoaderVtbl *v;
};

/* IDirectMusicPerformance (DirectX 6), by vtable slot */
struct DMPerformanceVtbl {
    void *QueryInterface, *AddRef; /* 0, 1 */
    ULONG(__stdcall *Release)(struct DMPerformance *); /* 2 */
    HRESULT(__stdcall *Init)(struct DMPerformance *, struct DMMusic **, void *, HWND); /* 3 */
    HRESULT(__stdcall *PlaySegment)(struct DMPerformance *, struct DMSegment *, DWORD, __int64, void **); /* 4 */
    HRESULT(__stdcall *Stop)(struct DMPerformance *, struct DMSegment *, void *, MUSIC_TIME, DWORD); /* 5 */
    void *GetSegmentState, *SetPrepareTime, *GetPrepareTime, *SetBumperLength, *GetBumperLength; /* 6-10 */
    void *SendPMsg, *MusicToReferenceTime, *ReferenceToMusicTime; /* 11-13 */
    HRESULT(__stdcall *IsPlaying)(struct DMPerformance *, struct DMSegment *, void *); /* 14 */
    HRESULT(__stdcall *GetTime)(struct DMPerformance *, REFERENCE_TIME *, MUSIC_TIME *); /* 15 */
    void *AllocPMsg; /* 16 */
    HRESULT(__stdcall *FreePMsg)(struct DMPerformance *, DMPMsg *); /* 17 */
    void *GetGraph; /* 18 */
    HRESULT(__stdcall *SetGraph)(struct DMPerformance *, struct DMGraph *); /* 19 */
    HRESULT(__stdcall *SetNotificationHandle)(struct DMPerformance *, HANDLE, REFERENCE_TIME); /* 20 */
    HRESULT(__stdcall *GetNotificationPMsg)(struct DMPerformance *, DMNotificationPMsg **); /* 21 */
    HRESULT(__stdcall *AddNotificationType)(struct DMPerformance *, const GUID *); /* 22 */
    void *RemoveNotificationType; /* 23 */
    HRESULT(__stdcall *AddPort)(struct DMPerformance *, struct DMPort *); /* 24 */
    void *RemovePort; /* 25 */
    HRESULT(__stdcall *AssignPChannelBlock)(struct DMPerformance *, DWORD, struct DMPort *, DWORD); /* 26 */
    void *AssignPChannel, *PChannelInfo, *DownloadInstrument, *Invalidate, *GetParam, *SetParam; /* 27-32 */
    void *GetGlobalParam; /* 33 */
    HRESULT(__stdcall *SetGlobalParam)(struct DMPerformance *, const GUID *, void *, DWORD); /* 34 */
    void *GetLatencyTime, *GetQueueTime, *AdjustTime; /* 35-37 */
    HRESULT(__stdcall *CloseDown)(struct DMPerformance *); /* 38 */
};
struct DMPerformance {
    struct DMPerformanceVtbl *v;
};

#endif
