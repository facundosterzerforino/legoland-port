/* record_dmusic: plays the game's music segments through Windows' DirectMusic and writes down every message the
 * performance produces (notes with their final MIDI pitch, curves, patch changes, tempo, time signatures,
 * notifications), in music time, so that a replacement can be checked against the real thing note by note
 * (tools/music/README.md). Nothing is heard: the output buffer is silent.
 *
 *   record_dmusic.exe <data folder> <out folder> <runs> <master groove> <tempo factor> <segment.sgt>...
 *
 * Each run of each segment is written to <out folder>\<segment>.g<groove>.r<run>.txt. */
#include <windows.h>
#include <dsound.h>
#include <stdio.h>
#include <string.h>

#include "dmi.h"
#include "miniwin/miniwin.h"

static CRITICAL_SECTION log_lock;
static FILE *out;

static void Log(const char *fmt, ...) {
    va_list ap;

    EnterCriticalSection(&log_lock);
    if (out != NULL) {
        va_start(ap, fmt);
        vfprintf(out, fmt, ap);
        va_end(ap);
    }
    LeaveCriticalSection(&log_lock);
}

/* ---- the recording tool ---- */

static struct DMGraph *tool_graph;

static HRESULT __stdcall ToolQueryInterface(struct DMTool *self, REFIID riid, void **p) {
    if (IsEqualGUID(riid, &IID_IUnknown) || IsEqualGUID(riid, &IID_DMTool)) {
        *p = self;
        return S_OK;
    }
    *p = NULL;
    return E_NOINTERFACE;
}

static ULONG __stdcall ToolAddRef(struct DMTool *self) {
    (void)self;
    return 2;
}

static ULONG __stdcall ToolRelease(struct DMTool *self) {
    (void)self;
    return 1;
}

static HRESULT __stdcall ToolInit(struct DMTool *self, struct DMGraph *graph) {
    (void)self;
    tool_graph = graph;
    return S_OK;
}

static HRESULT __stdcall ToolGetMsgDeliveryType(struct DMTool *self, DWORD *type) {
    (void)self;
    *type = DMUS_PMSGF_TOOL_IMMEDIATE;
    return S_OK;
}

/* The musical messages only: notifications pass by (a tool that requeues them loses them). */
static const DWORD media_types[] = {DMUS_PMSGT_MIDI, DMUS_PMSGT_NOTE, DMUS_PMSGT_SYSEX, DMUS_PMSGT_TEMPO,
    DMUS_PMSGT_CURVE, DMUS_PMSGT_TIMESIG, DMUS_PMSGT_PATCH, DMUS_PMSGT_TRANSPOSE, DMUS_PMSGT_CHANNEL_PRIORITY};

static HRESULT __stdcall ToolGetMediaTypeArraySize(struct DMTool *self, DWORD *size) {
    (void)self;
    *size = sizeof(media_types) / sizeof(media_types[0]);
    return S_OK;
}

static HRESULT __stdcall ToolGetMediaTypes(struct DMTool *self, DWORD **types, DWORD size) {
    DWORD i;

    (void)self;
    for (i = 0; i < size && i < sizeof(media_types) / sizeof(media_types[0]); i++) {
        (*types)[i] = media_types[i];
    }
    return S_OK;
}

static HRESULT __stdcall ToolProcessPMsg(struct DMTool *self, struct DMPerformance *perf, DMPMsg *m) {
    (void)self;
    (void)perf;
    switch (m->dwType) {
    case DMUS_PMSGT_NOTE: {
        DMNotePMsg *n = (DMNotePMsg *)m;

        Log("note %ld ch %lu midi %u vel %u dur %ld mv %04x meas %u beat %u grid %u off %d flags %x pm %x sub %x "
            "tr %d ranges %u %u %u\n",
            m->mtTime, m->dwPChannel, n->bMidiValue, n->bVelocity, n->mtDuration, n->wMusicValue, n->wMeasure,
            n->bBeat, n->bGrid, n->nOffset, n->bFlags, n->bPlayModeFlags, n->bSubChordLevel, n->cTranspose,
            n->bTimeRange, n->bDurRange, n->bVelRange);
        break;
    }
    case DMUS_PMSGT_CURVE: {
        DMCurvePMsg *c = (DMCurvePMsg *)m;

        Log("curve %ld ch %lu type %u cc %u shape %u start %d end %d dur %ld reset %d/%ld flags %x\n", m->mtTime,
            m->dwPChannel, c->bType, c->bCCData, c->bCurveShape, c->nStartValue, c->nEndValue, c->mtDuration,
            c->nResetValue, c->mtResetDuration, c->bFlags);
        break;
    }
    case DMUS_PMSGT_MIDI: {
        DMMidiPMsg *d = (DMMidiPMsg *)m;

        Log("midi %ld ch %lu status %02x %u %u\n", m->mtTime, m->dwPChannel, d->bStatus, d->bByte1, d->bByte2);
        break;
    }
    case DMUS_PMSGT_PATCH: {
        DMPatchPMsg *p = (DMPatchPMsg *)m;

        Log("patch %ld ch %lu program %u msb %u lsb %u\n", m->mtTime, m->dwPChannel, p->byInstrument, p->byMSB,
            p->byLSB);
        break;
    }
    case DMUS_PMSGT_TEMPO:
        Log("tempo %ld %.4f\n", m->mtTime, ((DMTempoPMsg *)m)->dblTempo);
        break;
    case DMUS_PMSGT_TIMESIG: {
        DMTimeSigPMsg *t = (DMTimeSigPMsg *)m;

        Log("timesig %ld %u/%u grids %u\n", m->mtTime, t->bBeatsPerMeasure, t->bBeat, t->wGridsPerBeat);
        break;
    }
    case DMUS_PMSGT_TRANSPOSE:
        Log("transpose %ld ch %lu %d\n", m->mtTime, m->dwPChannel, ((DMTransposePMsg *)m)->nTranspose);
        break;
    case DMUS_PMSGT_NOTIFICATION:
    case DMUS_PMSGT_DIRTY:
        break;
    default:
        Log("other %ld ch %lu type %lu\n", m->mtTime, m->dwPChannel, m->dwType);
        break;
    }
    if (m->pGraph != NULL && SUCCEEDED(m->pGraph->v->StampPMsg(m->pGraph, m))) {
        return DMUS_S_REQUEUE;
    }
    return DMUS_S_FREE;
}

static HRESULT __stdcall ToolFlush(struct DMTool *self, struct DMPerformance *perf, DMPMsg *m, REFERENCE_TIME t) {
    (void)self;
    (void)perf;
    (void)m;
    (void)t;
    return S_OK;
}

static struct DMToolVtbl tool_vtbl = {ToolQueryInterface, ToolAddRef, ToolRelease, ToolInit, ToolGetMsgDeliveryType,
    ToolGetMediaTypeArraySize, ToolGetMediaTypes, ToolProcessPMsg, ToolFlush};
static struct DMTool tool = {&tool_vtbl};

/* ---- one run ---- */

static LPDIRECTSOUND ds;
static struct DMLoader *loader;

static int Fail(const char *what, HRESULT hr) {
    printf("FAIL %s: %08lx\n", what, (unsigned long)hr);
    return 0;
}

static int Record(const char *name, int groove, float tempo, const char *path) {
    struct DMPerformance *perf;
    struct DMMusic *music = NULL;
    struct DMPort *port = NULL;
    struct DMGraph *graph = NULL;
    struct DMSegment *seg = NULL;
    LPDIRECTSOUNDBUFFER buffer = NULL;
    DMPortParams params = {sizeof(params), 0, 24, 1, 2, 22050, 0, 0};
    DMObjectDesc desc;
    DSBUFFERDESC bd;
    WAVEFORMATEX format[4];
    DWORD format_size = sizeof(format), buffer_size = 0;
    HANDLE event = CreateEventA(NULL, FALSE, FALSE, NULL);
    DMNotificationPMsg *note;
    char groove_byte = (char)groove;
    int started = 0, ended = 0, waited = 0;
    HRESULT hr;

    if (FAILED(hr = CoCreateInstance(&CLSID_DMPerformance, NULL, CLSCTX_INPROC, &IID_DMPerformance, (void **)&perf))) {
        return Fail("create performance", hr);
    }
    if (FAILED(hr = perf->v->Init(perf, &music, ds, NULL))) {
        return Fail("Init", hr);
    }
    music->v->CreatePort(music, &GUID_NULL, &params, &port, NULL);
    port->v->GetFormat(port, format, &format_size, &buffer_size);
    memset(&bd, 0, sizeof(bd));
    bd.dwSize = sizeof(bd);
    bd.dwFlags = DSBCAPS_CTRLVOLUME;
    bd.dwBufferBytes = buffer_size;
    bd.lpwfxFormat = format;
    IDirectSound_CreateSoundBuffer(ds, &bd, &buffer, NULL);
    IDirectSoundBuffer_SetVolume(buffer, DSBVOLUME_MIN);
    port->v->SetDirectSound(port, ds, buffer);
    port->v->Activate(port, TRUE);
    perf->v->AddPort(perf, port);
    perf->v->AssignPChannelBlock(perf, 0, port, 1);
    port->v->Release(port);
    CoCreateInstance(&CLSID_DMGraph, NULL, CLSCTX_INPROC, &IID_DMGraph, (void **)&graph);
    if (getenv("NOTOOL") == NULL) {
        graph->v->InsertTool(graph, &tool, NULL, 0, 0);
        perf->v->SetGraph(perf, graph);
    }
    perf->v->SetGlobalParam(perf, &GUID_DMMasterTempo, &tempo, sizeof(tempo));
    perf->v->SetGlobalParam(perf, &GUID_DMMasterGrooveLevel, &groove_byte, 1);
    perf->v->SetNotificationHandle(perf, event, 0);
    perf->v->AddNotificationType(perf, &GUID_DMNotifySegment);
    perf->v->AddNotificationType(perf, &GUID_DMNotifyMeasureBeat);

    memset(&desc, 0, sizeof(desc));
    desc.dwSize = sizeof(desc);
    desc.guidClass = CLSID_DMSegment;
    desc.dwValidData = 0x10 | 0x2; /* DMUS_OBJ_FILENAME | DMUS_OBJ_CLASS */
    MultiByteToWideChar(CP_ACP, 0, name, -1, desc.wszFileName, MAX_PATH);
    if (FAILED(hr = loader->v->GetObject(loader, &desc, &IID_DMSegment, (void **)&seg))) {
        return Fail(name, hr);
    }
    seg->v->SetRepeats(seg, 0); /* as the game does */
    out = fopen(path, "w");
    Log("segment %s groove %d\n", name, groove);
    perf->v->PlaySegment(perf, seg, 0, 0, NULL);
    while (!ended && waited < 600000) {
        WaitForSingleObject(event, 50);
        waited += 50;
        /* the segment's end, should its notification not come */
        if (perf->v->IsPlaying(perf, seg, NULL) == S_OK) {
            started = 1;
        } else if (started || waited > 5000) {
            Log("segment-stopped\n");
            ended = 1;
        }
        while (perf->v->GetNotificationPMsg(perf, &note) == S_OK) {
            if (IsEqualGUID(&note->guidNotificationType, &GUID_DMNotifySegment)) {
                static const char *const kinds[] = {"start", "end", "almostend", "loop", "abort"};

                Log("segment-%s %ld\n", note->dwNotificationOption < 5 ? kinds[note->dwNotificationOption] : "?",
                    note->head.mtTime);
                started |= note->dwNotificationOption == 0;
                ended |= note->dwNotificationOption == 1 || note->dwNotificationOption == 4;
            } else {
                Log("beat %ld %lu %lu\n", note->head.mtTime, note->dwField1, note->dwField2);
            }
            perf->v->FreePMsg(perf, &note->head);
        }
    }
    Sleep(200);
    EnterCriticalSection(&log_lock);
    fclose(out);
    out = NULL;
    LeaveCriticalSection(&log_lock);
    perf->v->Stop(perf, NULL, NULL, 0, 0);
    perf->v->CloseDown(perf);
    perf->v->Release(perf);
    graph->v->Release(graph);
    seg->v->Release(seg);
    IDirectSoundBuffer_Release(buffer);
    CloseHandle(event);
    if (!ended) {
        printf("FAIL %s: no segment end after %d s\n", name, waited / 1000);
        return 0;
    }
    return 1;
}

int main(int argc, char **argv) {
    WCHAR folder[MAX_PATH];
    int runs, groove, i, r, failed = 0;
    float tempo;

    if (argc < 7) {
        printf("usage: record_dmusic <data folder> <out folder> <runs> <master groove> <tempo factor> <segment>...\n");
        return 2;
    }
    runs = atoi(argv[3]);
    groove = atoi(argv[4]);
    tempo = (float)atof(argv[5]);
    InitializeCriticalSection(&log_lock);
    MultiByteToWideChar(CP_ACP, 0, argv[1], -1, folder, MAX_PATH);
    wcscat(folder, L"\\imusic");
    if (MiniwinDirectSoundCreate(NULL, &ds, NULL) != DS_OK) {
        printf("FAIL: no audio device\n");
        return 1;
    }
    IDirectSound_SetCooperativeLevel(ds, GetDesktopWindow(), DSSCL_NORMAL);
    CoInitialize(NULL);
    if (FAILED(CoCreateInstance(&CLSID_DMLoader, NULL, CLSCTX_INPROC, &IID_DMLoader, (void **)&loader))) {
        printf("FAIL: no DirectMusic loader\n");
        return 1;
    }
    loader->v->SetSearchDirectory(loader, &GUID_DMAllTypes, folder, TRUE);
    loader->v->EnableCache(loader, &GUID_DMAllTypes, TRUE);
    for (i = 6; i < argc; i++) {
        for (r = 0; r < runs; r++) {
            char path[MAX_PATH], base[MAX_PATH], *dot;
            DWORD t0 = GetTickCount();

            strcpy(base, argv[i]);
            dot = strrchr(base, '.');
            if (dot != NULL) {
                *dot = 0;
            }
            sprintf(path, "%s\\%s.g%d.r%d.txt", argv[2], base, groove, r);
            failed += !Record(argv[i], groove, tempo, path);
            printf("%s run %d: %lu ms\n", argv[i], r, GetTickCount() - t0);
            fflush(stdout);
        }
    }
    return failed != 0;
}
