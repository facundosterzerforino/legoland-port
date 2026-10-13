/* test_dmusic: plays the game's music through Windows' DirectMusic on miniwin's DirectSound (miniwin/src/dsound.c),
 * set up the way the game's music thread does it (MusicThreadProc), without starting the game. It checks every step
 * succeeds, that DirectMusic only asks for what miniwin has (unknown interfaces are reported), and that music
 * reaches miniwin's buffer.
 *
 *   tools/test_miniwin/build.sh && tools/test_miniwin/test_dmusic.exe <data folder>   (FAIL lines; exits 1 on any) */
#include <windows.h>
#include <dsound.h>
#include <stdio.h>
#include <string.h>

#include "miniwin/miniwin.h"

static const GUID CLSID_Performance = {0xd2ac2881, 0xb39b, 0x11d1, {0x87, 0x04, 0x00, 0x60, 0x08, 0x93, 0xb1, 0xbd}};
static const GUID IID_Performance = {0x07d43d03, 0x6523, 0x11d2, {0x87, 0x1d, 0x00, 0x60, 0x08, 0x93, 0xb1, 0xbd}};
static const GUID CLSID_Loader = {0xd2ac2892, 0xb39b, 0x11d1, {0x87, 0x04, 0x00, 0x60, 0x08, 0x93, 0xb1, 0xbd}};
static const GUID IID_Loader = {0x2ffaaca2, 0x5dca, 0x11d2, {0xaf, 0xa6, 0x00, 0xaa, 0x00, 0x24, 0xd8, 0xb6}};
static const GUID CLSID_Segment = {0xd2ac2882, 0xb39b, 0x11d1, {0x87, 0x04, 0x00, 0x60, 0x08, 0x93, 0xb1, 0xbd}};
static const GUID IID_Segment = {0xf96029a2, 0x4282, 0x11d2, {0x87, 0x17, 0x00, 0x60, 0x08, 0x93, 0xb1, 0xbd}};
static const GUID GUID_AllTypes = {0xd2ac2893, 0xb39b, 0x11d1, {0x87, 0x04, 0x00, 0x60, 0x08, 0x93, 0xb1, 0xbd}};
static const GUID GUID_Download = {0xd2ac28a7, 0xb39b, 0x11d1, {0x87, 0x04, 0x00, 0x60, 0x08, 0x93, 0xb1, 0xbd}};
static const GUID NullGuid = {0};

/* The game's declarations (src/legoland/sound_sfx.h), trimmed to what this test calls. */
struct ObjectDesc {
    DWORD dwSize, dwValidData;
    GUID guidObject, guidClass;
    FILETIME ftDate;
    DWORD vVersion[2];
    WCHAR wszName[64], wszCategory[64], wszFileName[MAX_PATH];
    __int64 llMemLength;
    BYTE *pbMemData;
};
struct PortParams {
    DWORD dwSize, dwValidParams, dwVoices, dwChannelGroups, dwAudioChannels, dwSampleRate, dwEffectFlags;
    BOOL fShare;
};
struct Port;
struct PortVtbl {
    void *QueryInterface, *AddRef;
    ULONG(__stdcall *Release)(struct Port *);
    BYTE pad_c[0x3c - 0xc];
    HRESULT(__stdcall *Activate)(struct Port *, BOOL);
    BYTE pad_40[0x48 - 0x40];
    HRESULT(__stdcall *SetDirectSound)(struct Port *, void *, void *);
    HRESULT(__stdcall *GetFormat)(struct Port *, WAVEFORMATEX *, DWORD *, DWORD *);
};
struct Port {
    struct PortVtbl *v;
};
struct Music;
struct MusicVtbl {
    void *QueryInterface, *AddRef, *Release, *EnumPort, *CreateMusicBuffer;
    HRESULT(__stdcall *CreatePort)(struct Music *, const GUID *, struct PortParams *, struct Port **, void *);
};
struct Music {
    struct MusicVtbl *v;
};
struct Segment;
struct SegmentVtbl {
    void *QueryInterface, *AddRef;
    ULONG(__stdcall *Release)(struct Segment *);
    BYTE pad_c[0x18 - 0xc];
    HRESULT(__stdcall *SetRepeats)(struct Segment *, DWORD);
    BYTE pad_1c[0x4c - 0x1c];
    HRESULT(__stdcall *SetParam)(struct Segment *, const GUID *, DWORD, DWORD, int, void *);
};
struct Segment {
    struct SegmentVtbl *v;
};
struct Loader;
struct LoaderVtbl {
    void *QueryInterface, *AddRef;
    ULONG(__stdcall *Release)(struct Loader *);
    HRESULT(__stdcall *GetObject)(struct Loader *, struct ObjectDesc *, const GUID *, void **);
    void *SetObject;
    HRESULT(__stdcall *SetSearchDirectory)(struct Loader *, const GUID *, const WCHAR *, BOOL);
    HRESULT(__stdcall *ScanDirectory)(struct Loader *, const GUID *, const WCHAR *, const WCHAR *);
    void *CacheObject, *ReleaseObject;
    HRESULT(__stdcall *ClearCache)(struct Loader *, const GUID *);
    HRESULT(__stdcall *EnableCache)(struct Loader *, const GUID *, BOOL);
};
struct Loader {
    struct LoaderVtbl *v;
};
struct Performance;
struct PerformanceVtbl {
    void *QueryInterface, *AddRef;
    ULONG(__stdcall *Release)(struct Performance *);
    HRESULT(__stdcall *Init)(struct Performance *, struct Music **, void *, HWND);
    HRESULT(__stdcall *PlaySegment)(struct Performance *, struct Segment *, DWORD, __int64, void **);
    HRESULT(__stdcall *Stop)(struct Performance *, struct Segment *, void *, int, DWORD);
    BYTE pad_18[0x60 - 0x18];
    HRESULT(__stdcall *AddPort)(struct Performance *, struct Port *);
    void *RemovePort;
    HRESULT(__stdcall *AssignPChannelBlock)(struct Performance *, DWORD, struct Port *, DWORD);
    BYTE pad_6c[0x98 - 0x6c];
    HRESULT(__stdcall *CloseDown)(struct Performance *);
};
struct Performance {
    struct PerformanceVtbl *v;
};

static int failures;

static void Check(const char *what, HRESULT hr) {
    if (FAILED(hr)) {
        printf("FAIL %s: %08lx\n", what, (unsigned long)hr);
        failures++;
    }
}

int main(int argc, char **argv) {
    LPDIRECTSOUND ds;
    LPDIRECTSOUNDBUFFER buffer = NULL;
    struct Performance *perf;
    struct Music *music = NULL;
    struct Port *port = NULL;
    struct Loader *loader;
    struct Segment *seg = NULL;
    struct ObjectDesc desc;
    struct PortParams params = {sizeof(params), 0, 24, 1, 2, 22050, 0, 0};
    DSBUFFERDESC bd;
    WAVEFORMATEX format[4];
    DWORD format_size = sizeof(format), buffer_size = 0, cap = 0;
    WCHAR folder[MAX_PATH];
    int i, loud = 0, polls = 0;

    if (argc < 2) {
        printf("usage: test_dmusic <data folder>\n");
        return 2;
    }
    MultiByteToWideChar(CP_ACP, 0, argv[1], -1, folder, MAX_PATH);
    wcscat(folder, L"\\imusic");
    if (MiniwinDirectSoundCreate(NULL, &ds, NULL) != DS_OK) {
        printf("no audio device: nothing to test\n");
        return 0;
    }
    IDirectSound_SetCooperativeLevel(ds, GetDesktopWindow(), DSSCL_NORMAL);
    CoInitialize(NULL);
    Check("create performance", CoCreateInstance(&CLSID_Performance, NULL, CLSCTX_INPROC, &IID_Performance, (void **)&perf));
    if (failures) {
        return 1;
    }
    Check("performance Init with miniwin's DirectSound", perf->v->Init(perf, &music, ds, NULL));
    if (failures) {
        return 1;
    }
    Check("CreatePort", music->v->CreatePort(music, &NullGuid, &params, &port, NULL));
    Check("GetFormat", port->v->GetFormat(port, format, &format_size, &buffer_size));
    printf("port format: %d ch, %ld Hz, %d bits; buffer %lu bytes\n", format[0].nChannels, format[0].nSamplesPerSec,
        format[0].wBitsPerSample, buffer_size);
    memset(&bd, 0, sizeof(bd));
    bd.dwSize = sizeof(bd);
    bd.dwFlags = DSBCAPS_CTRLVOLUME;
    bd.dwBufferBytes = buffer_size;
    bd.lpwfxFormat = format;
    Check("CreateSoundBuffer", IDirectSound_CreateSoundBuffer(ds, &bd, &buffer, NULL));
    IDirectSoundBuffer_SetVolume(buffer, DSBVOLUME_MIN); /* silent: miniwin applies it when mixing */
    Check("SetDirectSound", port->v->SetDirectSound(port, ds, buffer));
    Check("Activate", port->v->Activate(port, TRUE));
    Check("AddPort", perf->v->AddPort(perf, port));
    Check("AssignPChannelBlock", perf->v->AssignPChannelBlock(perf, 0, port, 1));
    Check("create loader", CoCreateInstance(&CLSID_Loader, NULL, CLSCTX_INPROC, &IID_Loader, (void **)&loader));
    Check("SetSearchDirectory", loader->v->SetSearchDirectory(loader, &GUID_AllTypes, folder, TRUE));
    loader->v->EnableCache(loader, &GUID_AllTypes, TRUE);
    memset(&desc, 0, sizeof(desc));
    desc.dwSize = sizeof(desc);
    desc.guidClass = CLSID_Segment;
    desc.dwValidData = 0x10 | 0x2; /* DMUS_OBJ_FILENAME | DMUS_OBJ_CLASS */
    wcscpy(desc.wszFileName, L"segtheme2.sgt");
    Check("load segtheme2.sgt", loader->v->GetObject(loader, &desc, &IID_Segment, (void **)&seg));
    if (failures) {
        return 1;
    }
    Check("download", seg->v->SetParam(seg, &GUID_Download, 0xffffffff, 0, 0, perf));
    Check("PlaySegment", perf->v->PlaySegment(perf, seg, 0, 0, NULL));
    /* five seconds: watch the buffer fill with music */
    for (i = 0; i < 50; i++) {
        DWORD play, write, n1, n2;
        void *p1, *p2;

        Sleep(100);
        if (IDirectSoundBuffer_GetCurrentPosition(buffer, &play, &write) == DS_OK &&
            IDirectSoundBuffer_Lock(buffer, 0, buffer_size, &p1, &n1, &p2, &n2, DSBLOCK_ENTIREBUFFER) == DS_OK) {
            const short *s = (const short *)p1;
            DWORD k;

            for (k = 0; k < n1 / 2; k++) {
                if (s[k] > 1000 || s[k] < -1000) {
                    loud++;
                    break;
                }
            }
            IDirectSoundBuffer_Unlock(buffer, p1, n1, p2, n2);
            polls++;
        }
    }
    IDirectSoundBuffer_GetStatus(buffer, &cap);
    printf("buffer status %lx; music heard in %d of %d looks\n", cap, loud, polls);
    if (loud < polls / 2) {
        printf("FAIL: little or no music in miniwin's buffer\n");
        failures++;
    }
    perf->v->Stop(perf, NULL, NULL, 0, 0);
    perf->v->CloseDown(perf);
    perf->v->Release(perf);
    printf("%s\n", failures ? "FAILED" : "OK");
    return failures != 0;
}
