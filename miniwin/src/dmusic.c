/* [library:music] miniwin's DirectMusic: the DirectX 6 interfaces the game's music thread uses (MusicThreadProc in
 * sound_sfx.c), on miniwin's style engine (miniwin/src/dmengine.c) and GothicKit dmusic (port/dmusic). See
 * miniwin/dmusic.h.
 *
 * The objects carry the vtables at DirectX 6's layout (dmusici.h, dmusicc.h). The game calls only some methods; the
 * others answer E_NOTIMPL with their real signatures, so a call to one can't unbalance the stack. */
#include <windows.h>
#include <dsound.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "_Internal.h"
#include "miniwin/dmengine.h"
#include "miniwin/dmusic.h"
#include "miniwin/files.h"
#include "miniwin/miniwin.h"

typedef long MUSIC_TIME;
typedef LONGLONG REFERENCE_TIME;

static const GUID CLSID_Performance = {0xd2ac2881, 0xb39b, 0x11d1, {0x87, 0x04, 0x00, 0x60, 0x08, 0x93, 0xb1, 0xbd}};
static const GUID CLSID_Loader = {0xd2ac2892, 0xb39b, 0x11d1, {0x87, 0x04, 0x00, 0x60, 0x08, 0x93, 0xb1, 0xbd}};
static const GUID CLSID_Composer = {0xd2ac2890, 0xb39b, 0x11d1, {0x87, 0x04, 0x00, 0x60, 0x08, 0x93, 0xb1, 0xbd}};
static const GUID CLSID_Segment = {0xd2ac2882, 0xb39b, 0x11d1, {0x87, 0x04, 0x00, 0x60, 0x08, 0x93, 0xb1, 0xbd}};
static const GUID CLSID_Style = {0xd2ac288a, 0xb39b, 0x11d1, {0x87, 0x04, 0x00, 0x60, 0x08, 0x93, 0xb1, 0xbd}};
static const GUID GUID_Download = {0xd2ac28a7, 0xb39b, 0x11d1, {0x87, 0x04, 0x00, 0x60, 0x08, 0x93, 0xb1, 0xbd}};
static const GUID GUID_MasterGroove = {0xd2ac28b2, 0xb39b, 0x11d1, {0x87, 0x04, 0x00, 0x60, 0x08, 0x93, 0xb1, 0xbd}};
static const GUID GUID_NotifySegment = {0xd2ac2899, 0xb39b, 0x11d1, {0x87, 0x04, 0x00, 0x60, 0x08, 0x93, 0xb1, 0xbd}};
static const GUID GUID_NotifyMeasureBeat = {0xd2ac289a, 0xb39b, 0x11d1,
    {0x87, 0x04, 0x00, 0x60, 0x08, 0x93, 0xb1, 0xbd}};

#define DMUS_OBJ_CLASS 0x2
#define DMUS_OBJ_NAME 0x4
#define DMUS_OBJ_FILENAME 0x10
#define DMUS_OBJ_MEMORY 0x400
#define DMUS_SEGF_GRID 0x800
#define DMUS_SEGF_BEAT 0x1000
#define DMUS_SEGF_MEASURE 0x2000
#define DMUS_S_FALSE ((HRESULT)0x00000001) /* S_FALSE */
#define DMUS_E_LOADER_FAILEDOPEN ((HRESULT)0x88780170)
#define DMUS_E_LOADER_FAILEDCREATE ((HRESULT)0x88780171)

static int portable = 1; /* the default (the maintainer's choice); -windows-music turns it off */

void MiniwinUsePortableMusic(int on) {
    portable = on;
}

int MiniwinPortableMusic(void) {
    return portable;
}

/* ---- the files: one dmusic loader for the game's music folder, and the General MIDI collection ---- */

static DmLoader *dm_loader;
static char search_dir[MAX_PATH] = "imusic";

static void *ReadAll(const char *path, size_t *len) {
    FILE *f = MiniwinFopen(path, "rb");
    void *p = NULL;
    long n;

    if (f == NULL) {
        return NULL;
    }
    fseek(f, 0, SEEK_END);
    n = ftell(f);
    fseek(f, 0, SEEK_SET);
    if (n > 0 && (p = malloc((size_t)n)) != NULL && fread(p, 1, (size_t)n, f) != (size_t)n) {
        free(p);
        p = NULL;
    }
    fclose(f);
    *len = (size_t)n;
    return p;
}

/* The General MIDI instruments: a gm.dls in the data folder, else Windows' own (DirectMusic's). */
static int GmDlsPath(char *path, size_t size) {
    char win[MAX_PATH];
    FILE *f;

    snprintf(path, size, "gm.dls");
    if ((f = MiniwinFopen(path, "rb")) != NULL) {
        fclose(f);
        return 1;
    }
    if (GetSystemWindowsDirectoryA(win, MAX_PATH) != 0) {
        snprintf(path, size, "%s\\System32\\drivers\\gm.dls", win);
        if ((f = fopen(path, "rb")) != NULL) {
            fclose(f);
            return 1;
        }
        snprintf(path, size, "%s\\Sysnative\\drivers\\gm.dls", win); /* from a 32-bit process */
        if ((f = fopen(path, "rb")) != NULL) {
            fclose(f);
            return 1;
        }
    }
    return 0;
}

/* The synthesizer's instruments: gm.dls (above), else a General MIDI SoundFont: one in the data folder (gm.sf2, or
 * the first *.sf2), else (Linux) the distribution's. Windows always has gm.dls; elsewhere it can be copied from a
 * Windows PC into the data folder, which sounds as the original. */
static int SynthFontPath(char *path, size_t size) {
    static const char *const system_fonts[] = {"/usr/share/sounds/sf2/FluidR3_GM.sf2",
        "/usr/share/sounds/sf2/default-GM.sf2", "/usr/share/soundfonts/default.sf2", "/usr/share/soundfonts/FluidR3_GM.sf2",
        "/usr/share/sounds/sf2/TimGM6mb.sf2"};
    WIN32_FIND_DATAA found;
    HANDLE search;
    FILE *f;
    size_t i;

    if (GmDlsPath(path, size)) {
        return 1;
    }
    snprintf(path, size, "gm.sf2");
    if ((f = MiniwinFopen(path, "rb")) != NULL) {
        fclose(f);
        return 1;
    }
    search = FindFirstFileA("*.sf2", &found);
    if (search != INVALID_HANDLE_VALUE) {
        snprintf(path, size, "%s", found.cFileName);
        FindClose(search);
        return 1;
    }
    for (i = 0; i < sizeof(system_fonts) / sizeof(system_fonts[0]); i++) {
        if ((f = fopen(system_fonts[i], "rb")) != NULL) {
            fclose(f);
            snprintf(path, size, "%s", system_fonts[i]);
            return 1;
        }
    }
    return 0;
}

static void *Resolve(void *ctx, char const *name, size_t *len) {
    char path[MAX_PATH * 2];
    void *p;

    (void)ctx;
    snprintf(path, sizeof(path), "%s\\%s", search_dir, name);
    if ((p = ReadAll(path, len)) != NULL) {
        return p;
    }
    if (_stricmp(name, "gm.dls") == 0 && GmDlsPath(path, sizeof(path))) {
        return ReadAll(path, len);
    }
    return NULL;
}

static void Log(void *ctx, DmLogLevel lvl, char const *msg) {
    (void)ctx;
    if (lvl <= DmLogLevel_ERROR) {
        OutputDebugStringA("dmusic: ");
        OutputDebugStringA(msg);
        OutputDebugStringA("\n");
    }
}

static DmLoader *Loader(void) {
    if (dm_loader == NULL) {
        Dm_setLogger(DmLogLevel_ERROR, Log, NULL);
        if (DmLoader_create(&dm_loader, DmLoader_DEFAULT) == DmResult_SUCCESS) {
            DmLoader_addResolver(dm_loader, Resolve, NULL);
        }
    }
    return dm_loader;
}

static HRESULT NotImpl(void) {
    return E_NOTIMPL;
}

#define UNUSED_METHOD(name, ...) \
    static HRESULT __stdcall name(void *self, __VA_ARGS__) { \
        (void)self; \
        return NotImpl(); \
    }

/* ---- IDirectMusicStyle: handed out by the loader, which the game keeps but doesn't use ---- */

typedef struct Style {
    const void *vtbl;
    LONG refs;
    DmStyle *style;
} Style;

static HRESULT __stdcall Style_QueryInterface(Style *self, REFIID riid, void **out) {
    (void)riid;
    InterlockedIncrement(&self->refs);
    *out = self;
    return S_OK;
}

static ULONG __stdcall Style_AddRef(Style *self) {
    return (ULONG)InterlockedIncrement(&self->refs);
}

static ULONG __stdcall Style_Release(Style *self) {
    LONG refs = InterlockedDecrement(&self->refs);

    if (refs == 0) {
        DmStyle_release(self->style);
        free(self);
    }
    return (ULONG)refs;
}

static const void *style_vtbl[3] = {(void *)Style_QueryInterface, (void *)Style_AddRef, (void *)Style_Release};

/* ---- IDirectMusicSegment ---- */

typedef struct Segment {
    const void *vtbl;
    LONG refs;
    DmSegment *seg;
    DWORD repeats;
} Segment;

typedef struct Performance Performance;
static DmeEngine *PerformanceEngine(Performance *p);

static HRESULT __stdcall Segment_QueryInterface(Segment *self, REFIID riid, void **out) {
    (void)riid;
    InterlockedIncrement(&self->refs);
    *out = self;
    return S_OK;
}

static ULONG __stdcall Segment_AddRef(Segment *self) {
    return (ULONG)InterlockedIncrement(&self->refs);
}

static ULONG __stdcall Segment_Release(Segment *self) {
    LONG refs = InterlockedDecrement(&self->refs);

    if (refs == 0) {
        DmSegment_release(self->seg);
        free(self);
    }
    return (ULONG)refs;
}

static HRESULT __stdcall Segment_GetLength(Segment *self, MUSIC_TIME *length) {
    *length = (MUSIC_TIME)self->seg->length;
    return S_OK;
}

UNUSED_METHOD(Segment_SetLength, MUSIC_TIME length)

static HRESULT __stdcall Segment_GetRepeats(Segment *self, DWORD *repeats) {
    *repeats = self->repeats;
    return S_OK;
}

static HRESULT __stdcall Segment_SetRepeats(Segment *self, DWORD repeats) {
    self->repeats = repeats;
    return S_OK;
}

UNUSED_METHOD(Segment_GetDefaultResolution, DWORD *resolution)
UNUSED_METHOD(Segment_SetDefaultResolution, DWORD resolution)
UNUSED_METHOD(Segment_GetTrack, REFGUID type, DWORD group, DWORD index, void **track)
UNUSED_METHOD(Segment_GetTrackGroup, void *track, DWORD *group)
UNUSED_METHOD(Segment_InsertTrack, void *track, DWORD group)
UNUSED_METHOD(Segment_RemoveTrack, void *track)
UNUSED_METHOD(Segment_InitPlay, void **state, void *performance, DWORD flags)
UNUSED_METHOD(Segment_GetGraph, void **graph)
UNUSED_METHOD(Segment_SetGraph, void *graph)
UNUSED_METHOD(Segment_AddNotificationType, REFGUID type)
UNUSED_METHOD(Segment_RemoveNotificationType, REFGUID type)
UNUSED_METHOD(Segment_GetParam, REFGUID type, DWORD group, DWORD index, MUSIC_TIME time, MUSIC_TIME *next,
    void *param)

static HRESULT __stdcall Segment_SetParam(Segment *self, REFGUID type, DWORD group, DWORD index, MUSIC_TIME time,
    void *param) {
    (void)group;
    (void)index;
    (void)time;
    if (IsEqualGUID(type, &GUID_Download)) {
        /* the instruments, onto the performance given */
        Dme_download(PerformanceEngine((Performance *)param), self->seg);
    }
    return S_OK;
}

UNUSED_METHOD(Segment_Clone, MUSIC_TIME start, MUSIC_TIME end, void **segment)
UNUSED_METHOD(Segment_SetStartPoint, MUSIC_TIME start)
UNUSED_METHOD(Segment_GetStartPoint, MUSIC_TIME *start)
UNUSED_METHOD(Segment_SetLoopPoints, MUSIC_TIME start, MUSIC_TIME end)
UNUSED_METHOD(Segment_GetLoopPoints, MUSIC_TIME *start, MUSIC_TIME *end)
UNUSED_METHOD(Segment_SetPChannelsUsed, DWORD count, DWORD *channels)

static const void *segment_vtbl[26] = {(void *)Segment_QueryInterface, (void *)Segment_AddRef,
    (void *)Segment_Release, (void *)Segment_GetLength, (void *)Segment_SetLength, (void *)Segment_GetRepeats,
    (void *)Segment_SetRepeats, (void *)Segment_GetDefaultResolution, (void *)Segment_SetDefaultResolution,
    (void *)Segment_GetTrack, (void *)Segment_GetTrackGroup, (void *)Segment_InsertTrack, (void *)Segment_RemoveTrack,
    (void *)Segment_InitPlay, (void *)Segment_GetGraph, (void *)Segment_SetGraph,
    (void *)Segment_AddNotificationType, (void *)Segment_RemoveNotificationType, (void *)Segment_GetParam,
    (void *)Segment_SetParam, (void *)Segment_Clone, (void *)Segment_SetStartPoint, (void *)Segment_GetStartPoint,
    (void *)Segment_SetLoopPoints, (void *)Segment_GetLoopPoints, (void *)Segment_SetPChannelsUsed};

/* ---- IDirectMusicLoader ---- */

typedef struct DmObjectDesc {
    DWORD dwSize, dwValidData;
    GUID guidObject, guidClass;
    FILETIME ftDate;
    DWORD vVersion[2];
    WCHAR wszName[64], wszCategory[64], wszFileName[MAX_PATH];
    __int64 llMemLength;
    BYTE *pbMemData;
} DmObjectDesc;

#define MAX_STYLES 512

typedef struct LoaderObj {
    const void *vtbl;
    LONG refs;
    char styles[MAX_STYLES][MAX_PATH]; /* the folder's styles, from ScanDirectory */
    int nstyles;
} LoaderObj;

static HRESULT __stdcall Loader_QueryInterface(LoaderObj *self, REFIID riid, void **out) {
    (void)riid;
    InterlockedIncrement(&self->refs);
    *out = self;
    return S_OK;
}

static ULONG __stdcall Loader_AddRef(LoaderObj *self) {
    return (ULONG)InterlockedIncrement(&self->refs);
}

static ULONG __stdcall Loader_Release(LoaderObj *self) {
    LONG refs = InterlockedDecrement(&self->refs);

    if (refs == 0) {
        free(self);
    }
    return (ULONG)refs;
}

static DmStyle *LoadStyle(const char *file) {
    DmReference ref;
    DmStyle *sty = NULL;

    memset(&ref, 0, sizeof(ref));
    ref.file = file;
    ref.guid.data[0] = 0xff; /* matches no style loaded already: load this file */
    return DmLoader_getStyle(Loader(), &ref, &sty) == DmResult_SUCCESS ? sty : NULL;
}

static HRESULT __stdcall Loader_GetObject(LoaderObj *self, DmObjectDesc *desc, REFIID riid, void **out) {
    char name[MAX_PATH];

    (void)riid;
    *out = NULL;
    if (Loader() == NULL) {
        return E_OUTOFMEMORY;
    }
    if (IsEqualGUID(&desc->guidClass, &CLSID_Segment)) {
        Segment *s;
        DmSegment *seg = NULL;
        void *bytes;
        size_t len;

        if (desc->dwValidData & DMUS_OBJ_MEMORY) {
            /* the game reads the files itself and hands over their bytes */
            len = (size_t)desc->llMemLength;
            bytes = Dm_alloc(len ? len : 1);
            if (bytes == NULL) {
                return E_OUTOFMEMORY;
            }
            memcpy(bytes, desc->pbMemData, len);
        } else if (desc->dwValidData & DMUS_OBJ_FILENAME) {
            char path[MAX_PATH * 2];

            WideCharToMultiByte(CP_ACP, 0, desc->wszFileName, -1, name, MAX_PATH, NULL, NULL);
            snprintf(path, sizeof(path), "%s\\%s", search_dir, name);
            if ((bytes = ReadAll(path, &len)) == NULL) {
                return DMUS_E_LOADER_FAILEDOPEN;
            }
        } else {
            return E_INVALIDARG;
        }
        if (DmSegment_create(&seg) != DmResult_SUCCESS) {
            Dm_free(bytes);
            return E_OUTOFMEMORY;
        }
        if (DmSegment_parse(seg, bytes, len) != DmResult_SUCCESS) {
            DmSegment_release(seg);
            return DMUS_E_LOADER_FAILEDCREATE;
        }
        s = (Segment *)calloc(1, sizeof(*s));
        if (s == NULL) {
            DmSegment_release(seg);
            return E_OUTOFMEMORY;
        }
        s->vtbl = segment_vtbl;
        s->refs = 1;
        s->seg = seg;
        s->repeats = seg->repeats;
        *out = s;
        return S_OK;
    }
    if (IsEqualGUID(&desc->guidClass, &CLSID_Style)) {
        Style *st;
        DmStyle *sty;

        WideCharToMultiByte(CP_ACP, 0, desc->wszFileName, -1, name, MAX_PATH, NULL, NULL);
        if ((sty = LoadStyle(name)) == NULL) {
            return DMUS_E_LOADER_FAILEDOPEN;
        }
        st = (Style *)calloc(1, sizeof(*st));
        if (st == NULL) {
            DmStyle_release(sty);
            return E_OUTOFMEMORY;
        }
        st->vtbl = style_vtbl;
        st->refs = 1;
        st->style = sty;
        *out = st;
        return S_OK;
    }
    (void)self;
    return DMUS_E_LOADER_FAILEDCREATE;
}

UNUSED_METHOD(Loader_SetObject, DmObjectDesc *desc)

static HRESULT __stdcall Loader_SetSearchDirectory(LoaderObj *self, REFGUID cls, const WCHAR *path, BOOL clear) {
    (void)self;
    (void)cls;
    (void)clear;
    WideCharToMultiByte(CP_ACP, 0, path, -1, search_dir, MAX_PATH, NULL, NULL);
    return S_OK;
}

static HRESULT __stdcall Loader_ScanDirectory(LoaderObj *self, REFGUID cls, const WCHAR *ext, const WCHAR *cache) {
    char pattern[MAX_PATH * 2], e[16];
    struct _finddata_t fd;
    intptr_t find;
    int found = 0;

    (void)cache;
    WideCharToMultiByte(CP_ACP, 0, ext, -1, e, sizeof(e), NULL, NULL);
    snprintf(pattern, sizeof(pattern), "%s\\*.%s", search_dir, e);
    find = MiniwinFindFirst(pattern, &fd);
    if (find == -1) {
        return DMUS_S_FALSE;
    }
    do {
        found++;
        /* the styles are loaded now, as DirectMusic's scan makes them known: some segments name theirs by GUID */
        if (IsEqualGUID(cls, &CLSID_Style) && self->nstyles < MAX_STYLES) {
            DmStyle *sty = LoadStyle(fd.name);

            if (sty != NULL) {
                strncpy(self->styles[self->nstyles], fd.name, MAX_PATH - 1);
                self->nstyles++;
                DmStyle_release(sty); /* the loader's cache keeps it */
            }
        }
    } while (_findnext(find, &fd) == 0);
    _findclose(find);
    return found ? S_OK : DMUS_S_FALSE;
}

UNUSED_METHOD(Loader_CacheObject, void *object)
UNUSED_METHOD(Loader_ReleaseObject, void *object)

static HRESULT __stdcall Loader_ClearCache(LoaderObj *self, REFGUID cls) {
    (void)self;
    (void)cls;
    return S_OK;
}

static HRESULT __stdcall Loader_EnableCache(LoaderObj *self, REFGUID cls, BOOL enable) {
    (void)self;
    (void)cls;
    (void)enable;
    return S_OK;
}

static HRESULT __stdcall Loader_EnumObject(LoaderObj *self, REFGUID cls, DWORD index, DmObjectDesc *desc) {
    if (!IsEqualGUID(cls, &CLSID_Style) || index >= (DWORD)self->nstyles) {
        return DMUS_S_FALSE;
    }
    memset(&desc->dwValidData, 0, sizeof(*desc) - sizeof(desc->dwSize));
    desc->dwValidData = DMUS_OBJ_CLASS | DMUS_OBJ_FILENAME;
    desc->guidClass = CLSID_Style;
    MultiByteToWideChar(CP_ACP, 0, self->styles[index], -1, desc->wszFileName, MAX_PATH);
    return S_OK;
}

static const void *loader_vtbl[12] = {(void *)Loader_QueryInterface, (void *)Loader_AddRef, (void *)Loader_Release,
    (void *)Loader_GetObject, (void *)Loader_SetObject, (void *)Loader_SetSearchDirectory,
    (void *)Loader_ScanDirectory, (void *)Loader_CacheObject, (void *)Loader_ReleaseObject, (void *)Loader_ClearCache,
    (void *)Loader_EnableCache, (void *)Loader_EnumObject};

/* ---- IDirectMusicPort: the synthesizer's output, into the DirectSound buffer the game gives it ---- */

typedef struct Port {
    const void *vtbl;
    LONG refs;
    DWORD rate;
    Performance *performance;
    LPDIRECTSOUNDBUFFER buffer;
} Port;

struct Performance {
    const void *vtbl;
    LONG refs;
    DmeEngine *engine;
    HANDLE event;
    unsigned int types;
    LPDIRECTSOUNDBUFFER buffer;
};

static DmeEngine *PerformanceEngine(Performance *p) {
    return p != NULL ? p->engine : NULL;
}

static void Fill(void *ctx, void *bytes, DWORD size) {
    Dme_render((DmeEngine *)ctx, (short *)bytes, (int)(size / 4));
}

static HRESULT __stdcall Port_QueryInterface(Port *self, REFIID riid, void **out) {
    (void)riid;
    InterlockedIncrement(&self->refs);
    *out = self;
    return S_OK;
}

static ULONG __stdcall Port_AddRef(Port *self) {
    return (ULONG)InterlockedIncrement(&self->refs);
}

static ULONG __stdcall Port_Release(Port *self) {
    LONG refs = InterlockedDecrement(&self->refs);

    if (refs == 0) {
        free(self);
    }
    return (ULONG)refs;
}

UNUSED_METHOD(Port_PlayBuffer, void *buffer)
UNUSED_METHOD(Port_SetReadNotificationHandle, HANDLE event)
UNUSED_METHOD(Port_Read, void *buffer)
UNUSED_METHOD(Port_DownloadInstrument, void *instrument, void **downloaded, void *ranges, DWORD count)
UNUSED_METHOD(Port_UnloadInstrument, void *downloaded)
UNUSED_METHOD(Port_GetLatencyClock, void **clock)
UNUSED_METHOD(Port_GetRunningStats, void *stats)

static HRESULT __stdcall Port_Compact(Port *self) {
    (void)self;
    return S_OK;
}

UNUSED_METHOD(Port_GetCaps, void *caps)
UNUSED_METHOD(Port_DeviceIoControl, DWORD code, void *in, DWORD in_size, void *out, DWORD out_size, DWORD *returned,
    void *overlapped)
UNUSED_METHOD(Port_SetNumChannelGroups, DWORD groups)
UNUSED_METHOD(Port_GetNumChannelGroups, DWORD *groups)

static HRESULT __stdcall Port_Activate(Port *self, BOOL active) {
    if (self->buffer != NULL) {
        if (active) {
            IDirectSoundBuffer_Play(self->buffer, 0, 0, DSBPLAY_LOOPING);
        } else {
            IDirectSoundBuffer_Stop(self->buffer);
        }
    }
    return S_OK;
}

UNUSED_METHOD(Port_SetChannelPriority, DWORD group, DWORD channel, DWORD priority)
UNUSED_METHOD(Port_GetChannelPriority, DWORD group, DWORD channel, DWORD *priority)

static HRESULT __stdcall Port_SetDirectSound(Port *self, void *dsound, void *buffer) {
    (void)dsound;
    self->buffer = (LPDIRECTSOUNDBUFFER)buffer;
    if (self->buffer != NULL && self->performance != NULL && self->performance->engine != NULL) {
        self->performance->buffer = self->buffer;
        MiniwinSoundBufferSetSource(self->buffer, Fill, self->performance->engine);
    }
    return S_OK;
}

static HRESULT __stdcall Port_GetFormat(Port *self, WAVEFORMATEX *format, DWORD *format_size, DWORD *buffer_size) {
    if (format != NULL && format_size != NULL && *format_size >= sizeof(WAVEFORMATEX)) {
        memset(format, 0, sizeof(*format));
        format->wFormatTag = WAVE_FORMAT_PCM;
        format->nChannels = 2;
        format->nSamplesPerSec = self->rate;
        format->wBitsPerSample = 16;
        format->nBlockAlign = 4;
        format->nAvgBytesPerSec = self->rate * 4;
    }
    if (format_size != NULL) {
        *format_size = sizeof(WAVEFORMATEX);
    }
    if (buffer_size != NULL) {
        *buffer_size = self->rate * 4 * 2; /* two seconds, as DirectMusic's */
    }
    return S_OK;
}

static const void *port_vtbl[20] = {(void *)Port_QueryInterface, (void *)Port_AddRef, (void *)Port_Release,
    (void *)Port_PlayBuffer, (void *)Port_SetReadNotificationHandle, (void *)Port_Read,
    (void *)Port_DownloadInstrument, (void *)Port_UnloadInstrument, (void *)Port_GetLatencyClock,
    (void *)Port_GetRunningStats, (void *)Port_Compact, (void *)Port_GetCaps, (void *)Port_DeviceIoControl,
    (void *)Port_SetNumChannelGroups, (void *)Port_GetNumChannelGroups, (void *)Port_Activate,
    (void *)Port_SetChannelPriority, (void *)Port_GetChannelPriority, (void *)Port_SetDirectSound,
    (void *)Port_GetFormat};

/* ---- IDirectMusic ---- */

typedef struct DmPortParams {
    DWORD dwSize, dwValidParams, dwVoices, dwChannelGroups, dwAudioChannels, dwSampleRate, dwEffectFlags;
    BOOL fShare;
} DmPortParams;

typedef struct Music {
    const void *vtbl;
    LONG refs;
    Performance *performance;
} Music;

static HRESULT __stdcall Music_QueryInterface(Music *self, REFIID riid, void **out) {
    (void)riid;
    InterlockedIncrement(&self->refs);
    *out = self;
    return S_OK;
}

static ULONG __stdcall Music_AddRef(Music *self) {
    return (ULONG)InterlockedIncrement(&self->refs);
}

static ULONG __stdcall Music_Release(Music *self) {
    LONG refs = InterlockedDecrement(&self->refs);

    if (refs == 0) {
        free(self);
    }
    return (ULONG)refs;
}

UNUSED_METHOD(Music_EnumPort, DWORD index, void *caps)
UNUSED_METHOD(Music_CreateMusicBuffer, void *desc, void **buffer, void *outer)

static HRESULT __stdcall Music_CreatePort(Music *self, REFCLSID cls, DmPortParams *params, Port **out, void *outer) {
    Port *p = (Port *)calloc(1, sizeof(*p));
    Performance *perf = self->performance;

    (void)cls;
    (void)outer;
    *out = NULL;
    if (p == NULL) {
        return E_OUTOFMEMORY;
    }
    p->vtbl = port_vtbl;
    p->refs = 1;
    p->rate = params != NULL && params->dwSampleRate != 0 ? params->dwSampleRate : 22050; /* the game's: 22050 */
    p->performance = perf;
    if (perf != NULL && perf->engine == NULL && Loader() != NULL) {
        char gm_dls[MAX_PATH * 2];

        perf->engine = Dme_create(Loader(), p->rate);
        Dme_setNotificationTypes(perf->engine, perf->types);
        /* the synthesizer: FluidSynth on gm.dls, which sounds like DirectMusic's (tools/music/README.md), or a
         * SoundFont where there's no gm.dls */
        if (SynthFontPath(gm_dls, sizeof(gm_dls))) {
            Dme_useFluidSynth(perf->engine, gm_dls);
        }
    }
    *out = p;
    return S_OK;
}

UNUSED_METHOD(Music_EnumMasterClock, DWORD index, void *info)
UNUSED_METHOD(Music_GetMasterClock, GUID *guid, void **clock)
UNUSED_METHOD(Music_SetMasterClock, REFGUID guid)
UNUSED_METHOD(Music_Activate, BOOL enable)
UNUSED_METHOD(Music_GetDefaultPort, GUID *port)
UNUSED_METHOD(Music_SetDirectSound, void *dsound, HWND hwnd)

static const void *music_vtbl[12] = {(void *)Music_QueryInterface, (void *)Music_AddRef, (void *)Music_Release,
    (void *)Music_EnumPort, (void *)Music_CreateMusicBuffer, (void *)Music_CreatePort, (void *)Music_EnumMasterClock,
    (void *)Music_GetMasterClock, (void *)Music_SetMasterClock, (void *)Music_Activate, (void *)Music_GetDefaultPort,
    (void *)Music_SetDirectSound};

/* ---- IDirectMusicPerformance ---- */

/* DMUS_NOTIFICATION_PMSG */
typedef struct NotificationMsg {
    DWORD dwSize;
    REFERENCE_TIME rtTime;
    MUSIC_TIME mtTime;
    DWORD dwFlags, dwPChannel, dwVirtualTrackID;
    void *pTool, *pGraph;
    DWORD dwType, dwVoiceID, dwGroupID;
    void *punkUser;
    GUID guidNotificationType;
    DWORD dwNotificationOption, dwField1, dwField2;
} NotificationMsg;

static HRESULT __stdcall Perf_QueryInterface(Performance *self, REFIID riid, void **out) {
    (void)riid;
    InterlockedIncrement(&self->refs);
    *out = self;
    return S_OK;
}

static ULONG __stdcall Perf_AddRef(Performance *self) {
    return (ULONG)InterlockedIncrement(&self->refs);
}

static void Signal(void *ctx) {
    Performance *p = (Performance *)ctx;

    if (p->event != NULL) {
        SetEvent(p->event);
    }
}

static HRESULT __stdcall Perf_CloseDown(Performance *self) {
    if (self->buffer != NULL) {
        MiniwinSoundBufferSetSource(self->buffer, NULL, NULL);
        self->buffer = NULL;
    }
    if (self->engine != NULL) {
        Dme_destroy(self->engine);
        self->engine = NULL;
    }
    return S_OK;
}

static ULONG __stdcall Perf_Release(Performance *self) {
    LONG refs = InterlockedDecrement(&self->refs);

    if (refs == 0) {
        Perf_CloseDown(self);
        free(self);
    }
    return (ULONG)refs;
}

static HRESULT __stdcall Perf_Init(Performance *self, Music **music, void *dsound, HWND hwnd) {
    (void)dsound;
    (void)hwnd;
    if (music != NULL) {
        Music *m = (Music *)calloc(1, sizeof(*m));

        if (m == NULL) {
            return E_OUTOFMEMORY;
        }
        m->vtbl = music_vtbl;
        m->refs = 1;
        m->performance = self;
        *music = m;
    }
    return S_OK;
}

static HRESULT __stdcall Perf_PlaySegment(Performance *self, Segment *seg, DWORD flags, __int64 start, void **state) {
    int timing = (flags & DMUS_SEGF_MEASURE) ? DME_AT_MEASURE
        : (flags & DMUS_SEGF_BEAT)           ? DME_AT_BEAT
        : (flags & DMUS_SEGF_GRID)           ? DME_AT_GRID
                                             : DME_AT_ONCE;

    (void)start;
    if (state != NULL) {
        *state = NULL;
    }
    if (seg == NULL || self->engine == NULL) {
        return E_POINTER;
    }
    Dme_play(self->engine, seg->seg, seg->repeats, timing);
    return S_OK;
}

static HRESULT __stdcall Perf_Stop(Performance *self, Segment *seg, void *state, MUSIC_TIME time, DWORD flags) {
    (void)seg;
    (void)state;
    (void)time;
    (void)flags;
    Dme_stop(self->engine);
    return S_OK;
}

UNUSED_METHOD(Perf_GetSegmentState, void **state, MUSIC_TIME time)
UNUSED_METHOD(Perf_SetPrepareTime, DWORD ms)
UNUSED_METHOD(Perf_GetPrepareTime, DWORD *ms)
UNUSED_METHOD(Perf_SetBumperLength, DWORD ms)
UNUSED_METHOD(Perf_GetBumperLength, DWORD *ms)
UNUSED_METHOD(Perf_SendPMsg, void *msg)
UNUSED_METHOD(Perf_MusicToReferenceTime, MUSIC_TIME mt, REFERENCE_TIME *rt)
UNUSED_METHOD(Perf_ReferenceToMusicTime, REFERENCE_TIME rt, MUSIC_TIME *mt)
UNUSED_METHOD(Perf_IsPlaying, void *segment, void *state)

static HRESULT __stdcall Perf_GetTime(Performance *self, REFERENCE_TIME *rt, MUSIC_TIME *mt) {
    if (rt != NULL) {
        *rt = 0;
    }
    if (mt != NULL) {
        *mt = Dme_time(self->engine);
    }
    return S_OK;
}

UNUSED_METHOD(Perf_AllocPMsg, ULONG size, void **msg)

static HRESULT __stdcall Perf_FreePMsg(Performance *self, NotificationMsg *msg) {
    (void)self;
    free(msg);
    return S_OK;
}

UNUSED_METHOD(Perf_GetGraph, void **graph)
UNUSED_METHOD(Perf_SetGraph, void *graph)

static HRESULT __stdcall Perf_SetNotificationHandle(Performance *self, HANDLE event, REFERENCE_TIME minimum) {
    (void)minimum;
    self->event = event;
    return S_OK;
}

static HRESULT __stdcall Perf_GetNotificationPMsg(Performance *self, NotificationMsg **out) {
    DmeNotification n;
    NotificationMsg *m;

    *out = NULL;
    if (self->engine == NULL || !Dme_takeNotification(self->engine, &n)) {
        return DMUS_S_FALSE;
    }
    m = (NotificationMsg *)calloc(1, sizeof(*m));
    if (m == NULL) {
        return E_OUTOFMEMORY;
    }
    m->dwSize = sizeof(*m);
    m->mtTime = n.time;
    m->dwType = 3; /* DMUS_PMSGT_NOTIFICATION */
    m->guidNotificationType = n.type == DME_SEGMENT ? GUID_NotifySegment : GUID_NotifyMeasureBeat;
    m->dwNotificationOption = n.option;
    m->dwField1 = n.field1;
    m->dwField2 = n.field2;
    *out = m;
    return S_OK;
}

static HRESULT __stdcall Perf_AddNotificationType(Performance *self, REFGUID type) {
    if (IsEqualGUID(type, &GUID_NotifySegment)) {
        self->types |= 1u << DME_SEGMENT;
    } else if (IsEqualGUID(type, &GUID_NotifyMeasureBeat)) {
        self->types |= 1u << DME_MEASURE_AND_BEAT;
    }
    if (self->engine != NULL) {
        Dme_setNotificationTypes(self->engine, self->types);
        Dme_setNotificationCallback(self->engine, Signal, self);
    }
    return S_OK;
}

UNUSED_METHOD(Perf_RemoveNotificationType, REFGUID type)

static HRESULT __stdcall Perf_AddPort(Performance *self, Port *port) {
    (void)port;
    if (self->engine != NULL) {
        Dme_setNotificationCallback(self->engine, Signal, self);
    }
    return S_OK;
}

UNUSED_METHOD(Perf_RemovePort, void *port)

static HRESULT __stdcall Perf_AssignPChannelBlock(Performance *self, DWORD block, Port *port, DWORD group) {
    (void)self;
    (void)block;
    (void)port;
    (void)group;
    return S_OK;
}

UNUSED_METHOD(Perf_AssignPChannel, DWORD pchannel, void *port, DWORD group, DWORD channel)
UNUSED_METHOD(Perf_PChannelInfo, DWORD pchannel, void **port, DWORD *group, DWORD *channel)
UNUSED_METHOD(Perf_DownloadInstrument, void *instrument, DWORD pchannel, void **downloaded, void *ranges,
    DWORD count, void **port, DWORD *group, DWORD *channel)
UNUSED_METHOD(Perf_Invalidate, MUSIC_TIME time, DWORD flags)
UNUSED_METHOD(Perf_GetParam, REFGUID type, DWORD group, DWORD index, MUSIC_TIME time, MUSIC_TIME *next, void *param)
UNUSED_METHOD(Perf_SetParam, REFGUID type, DWORD group, DWORD index, MUSIC_TIME time, void *param)
UNUSED_METHOD(Perf_GetGlobalParam, REFGUID type, void *param, DWORD size)

static HRESULT __stdcall Perf_SetGlobalParam(Performance *self, REFGUID type, void *param, DWORD size) {
    if (IsEqualGUID(type, &GUID_MasterGroove) && param != NULL && size >= 1) {
        Dme_setMasterGroove(self->engine, *(signed char *)param);
    }
    return S_OK;
}

UNUSED_METHOD(Perf_GetLatencyTime, REFERENCE_TIME *time)
UNUSED_METHOD(Perf_GetQueueTime, REFERENCE_TIME *time)
UNUSED_METHOD(Perf_AdjustTime, REFERENCE_TIME amount)
UNUSED_METHOD(Perf_GetResolvedTime, REFERENCE_TIME time, REFERENCE_TIME *resolved, DWORD flags)
UNUSED_METHOD(Perf_MIDIToMusic, BYTE midi, void *chord, BYTE mode, BYTE level, WORD *value)
UNUSED_METHOD(Perf_MusicToMIDI, WORD value, void *chord, BYTE mode, BYTE level, BYTE *midi)
UNUSED_METHOD(Perf_TimeToRhythm, MUSIC_TIME time, void *signature, WORD *measure, BYTE *beat, BYTE *grid,
    short *offset)
UNUSED_METHOD(Perf_RhythmToTime, WORD measure, BYTE beat, BYTE grid, short offset, void *signature,
    MUSIC_TIME *time)

static const void *perf_vtbl[44] = {(void *)Perf_QueryInterface, (void *)Perf_AddRef, (void *)Perf_Release,
    (void *)Perf_Init, (void *)Perf_PlaySegment, (void *)Perf_Stop, (void *)Perf_GetSegmentState,
    (void *)Perf_SetPrepareTime, (void *)Perf_GetPrepareTime, (void *)Perf_SetBumperLength,
    (void *)Perf_GetBumperLength, (void *)Perf_SendPMsg, (void *)Perf_MusicToReferenceTime,
    (void *)Perf_ReferenceToMusicTime, (void *)Perf_IsPlaying, (void *)Perf_GetTime, (void *)Perf_AllocPMsg,
    (void *)Perf_FreePMsg, (void *)Perf_GetGraph, (void *)Perf_SetGraph, (void *)Perf_SetNotificationHandle,
    (void *)Perf_GetNotificationPMsg, (void *)Perf_AddNotificationType, (void *)Perf_RemoveNotificationType,
    (void *)Perf_AddPort, (void *)Perf_RemovePort, (void *)Perf_AssignPChannelBlock, (void *)Perf_AssignPChannel,
    (void *)Perf_PChannelInfo, (void *)Perf_DownloadInstrument, (void *)Perf_Invalidate, (void *)Perf_GetParam,
    (void *)Perf_SetParam, (void *)Perf_GetGlobalParam, (void *)Perf_SetGlobalParam, (void *)Perf_GetLatencyTime,
    (void *)Perf_GetQueueTime, (void *)Perf_AdjustTime, (void *)Perf_CloseDown, (void *)Perf_GetResolvedTime,
    (void *)Perf_MIDIToMusic, (void *)Perf_MusicToMIDI, (void *)Perf_TimeToRhythm, (void *)Perf_RhythmToTime};

/* ---- IDirectMusicComposer: created by the game, never used ---- */

typedef struct Composer {
    const void *vtbl;
    LONG refs;
} Composer;

static HRESULT __stdcall Composer_QueryInterface(Composer *self, REFIID riid, void **out) {
    (void)riid;
    InterlockedIncrement(&self->refs);
    *out = self;
    return S_OK;
}

static ULONG __stdcall Composer_AddRef(Composer *self) {
    return (ULONG)InterlockedIncrement(&self->refs);
}

static ULONG __stdcall Composer_Release(Composer *self) {
    LONG refs = InterlockedDecrement(&self->refs);

    if (refs == 0) {
        free(self);
    }
    return (ULONG)refs;
}

UNUSED_METHOD(Composer_FromTemplate, void *style, void *tmpl, WORD activity, void *chordmap, void **segment)
UNUSED_METHOD(Composer_FromShape, void *style, WORD measures, WORD shape, WORD activity, BOOL intro, BOOL end,
    void *chordmap, void **segment)
UNUSED_METHOD(Composer_Transition, void *from, void *to, MUSIC_TIME time, WORD command, DWORD flags,
    void *chordmap, void **segment)
UNUSED_METHOD(Composer_AutoTransition, void *performance, void *to, WORD command, DWORD flags, void *chordmap,
    void **segment, void **to_state, void **state)
UNUSED_METHOD(Composer_TemplateFromShape, WORD measures, WORD shape, BOOL intro, BOOL end, WORD end_length,
    void **tmpl)
UNUSED_METHOD(Composer_ChangeChordMap, void *segment, BOOL scale, void *chordmap)

static const void *composer_vtbl[9] = {(void *)Composer_QueryInterface, (void *)Composer_AddRef,
    (void *)Composer_Release, (void *)Composer_FromTemplate, (void *)Composer_FromShape, (void *)Composer_Transition,
    (void *)Composer_AutoTransition, (void *)Composer_TemplateFromShape, (void *)Composer_ChangeChordMap};

/* ---- COM ---- */

HRESULT WINAPI MiniwinCoInitialize(LPVOID reserved) {
    return portable ? S_OK : CoInitialize(reserved);
}

HRESULT WINAPI MiniwinCoCreateInstance(REFCLSID cls, LPUNKNOWN outer, DWORD context, REFIID riid, LPVOID *out) {
    if (!portable) {
        return CoCreateInstance(cls, outer, context, riid, out);
    }
    *out = NULL;
    if (IsEqualGUID(cls, &CLSID_Performance)) {
        Performance *p = (Performance *)calloc(1, sizeof(*p));

        if (p == NULL) {
            return E_OUTOFMEMORY;
        }
        p->vtbl = perf_vtbl;
        p->refs = 1;
        *out = p;
        return S_OK;
    }
    if (IsEqualGUID(cls, &CLSID_Loader)) {
        LoaderObj *l = (LoaderObj *)calloc(1, sizeof(*l));

        if (l == NULL || Loader() == NULL) {
            free(l);
            return E_OUTOFMEMORY;
        }
        l->vtbl = loader_vtbl;
        l->refs = 1;
        *out = l;
        return S_OK;
    }
    if (IsEqualGUID(cls, &CLSID_Composer)) {
        Composer *c = (Composer *)calloc(1, sizeof(*c));

        if (c == NULL) {
            return E_OUTOFMEMORY;
        }
        c->vtbl = composer_vtbl;
        c->refs = 1;
        *out = c;
        return S_OK;
    }
    return REGDB_E_CLASSNOTREG;
}
