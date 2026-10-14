/* instruments_dmusic: plays one note of each instrument the game's music uses, every 3 s, through Windows'
 * DirectMusic synthesizer (on miniwin's DirectSound, its instruments downloaded by the game's segments) and through
 * the portable synthesizer (dmusic's TinySoundFont conversion of gm.dls, with the gain rules of port/dmusic's Synth.c
 * and the engine's overall gain), and writes both to WAV files, to compare instrument by instrument
 * (tools/music/instruments.py). The WAVs are derived from gm.dls and the game's music: keep them out of the repository.
 *
 *   instruments_dmusic.exe <data folder> <windows.wav> <portable.wav> */
#include <windows.h>
#include <dsound.h>
#include <stdio.h>
#include <string.h>

#include "../music/dmi.h"
#include "_Internal.h"
#include "../../miniwin/src/dmreverb.h"
#include "miniwin/dmusic.h"
#include "miniwin/miniwin.h"
#ifdef WITH_FLUIDSYNTH
#include <fluidsynth.h>
#endif

#define SLOT_SECONDS 3
#define NOTE_START 0.25
#define NOTE_SECONDS 1.5
#define RATE 22050
#define GAIN 0.95f /* miniwin/src/dmengine.c's SYNTH_GAIN */

static const GUID GUID_DefaultGMCollection = {0xf17e8673, 0xc3b4, 0x11d1, {0x87, 0x0b, 0x00, 0x60, 0x08, 0x93, 0xb1, 0xbd}};
static const GUID CLSID_DMCollection = {0x480ff4b0, 0x28b2, 0x11d1, {0xbe, 0xf7, 0x00, 0xc0, 0x4f, 0xbf, 0x8f, 0xef}};
static const GUID IID_DMCollection = {0xd2ac287c, 0xb39b, 0x11d1, {0x87, 0x04, 0x00, 0x60, 0x08, 0x93, 0xb1, 0xbd}};
typedef HRESULT(__stdcall *GetInstrumentFn)(void *, DWORD, void **);
typedef HRESULT(__stdcall *DownloadInstrumentFn)(struct DMPerformance *, void *, DWORD, void **, void *, DWORD, void **,
    DWORD *, DWORD *);
static const GUID CLSID_DMStyle = {0xd2ac288a, 0xb39b, 0x11d1, {0x87, 0x04, 0x00, 0x60, 0x08, 0x93, 0xb1, 0xbd}};
static const GUID IID_DMStyle = {0xd2ac28bd, 0xb39b, 0x11d1, {0x87, 0x04, 0x00, 0x60, 0x08, 0x93, 0xb1, 0xbd}};

typedef struct Test {
    int program, msb, drum, note, velocity;
    const char *name;
} Test;

/* the first is the alignment reference: loud, sharp attack */
static const Test tests[] = {{0, 0, 0, 60, 127, "piano (reference)"}, {0, 0, 0, 60, 90, "piano"},
    {8, 0, 0, 72, 90, "celesta"}, {9, 0, 0, 72, 90, "glockenspiel"}, {11, 0, 0, 72, 90, "vibraphone"},
    {12, 0, 0, 72, 90, "marimba"}, {13, 0, 0, 72, 90, "xylophone"}, {19, 0, 0, 60, 90, "church organ"},
    {22, 0, 0, 72, 90, "harmonica"}, {24, 0, 0, 60, 90, "nylon guitar"}, {24, 8, 0, 60, 90, "ukulele (msb 8)"},
    {32, 0, 0, 40, 90, "acoustic bass"}, {34, 0, 0, 40, 90, "picked bass"}, {36, 0, 0, 40, 90, "slap bass"},
    {45, 0, 0, 60, 90, "pizzicato"}, {46, 0, 0, 60, 90, "harp"}, {48, 0, 0, 60, 90, "strings"},
    {49, 0, 0, 60, 90, "slow strings"}, {50, 8, 0, 60, 90, "syn strings 3 (msb 8)"}, {56, 0, 0, 72, 90, "trumpet"},
    {60, 0, 0, 60, 90, "french horn"}, {64, 0, 0, 72, 90, "soprano sax"}, {68, 0, 0, 72, 90, "oboe"},
    {69, 0, 0, 72, 90, "english horn"}, {70, 0, 0, 48, 90, "bassoon"}, {71, 0, 0, 72, 90, "clarinet"},
    {73, 0, 0, 72, 90, "flute"}, {74, 0, 0, 72, 90, "recorder"}, {75, 0, 0, 72, 90, "pan flute"},
    {78, 0, 0, 72, 90, "whistle"}, {104, 0, 0, 60, 90, "sitar"}, {105, 0, 0, 60, 90, "banjo"},
    {106, 0, 0, 60, 90, "shamisen"}, {108, 0, 0, 60, 90, "kalimba"}, {110, 0, 0, 60, 90, "fiddle"},
    {115, 0, 0, 60, 90, "woodblock"}, {0, 0, 1, 54, 90, "drum tambourine"}, {0, 0, 1, 82, 90, "drum shaker"},
    {0, 0, 1, 36, 90, "drum kick"}, {0, 0, 1, 70, 90, "drum maracas"}, {0, 0, 1, 42, 90, "drum closed hihat"},
    {0, 0, 1, 43, 90, "drum floor tom"}, {0, 0, 1, 60, 90, "drum hi bongo"}, {0, 0, 1, 38, 90, "drum snare"},
    {0, 0, 1, 64, 90, "drum low conga"}, {0, 0, 1, 81, 90, "drum open triangle"}, {0, 0, 1, 83, 90, "drum jingle bell"},
    {0, 0, 1, 49, 90, "drum crash"}, {0, 0, 1, 75, 90, "drum claves"}, {0, 0, 1, 60, 60, "drum hi bongo soft"},
    {48, 0, 0, 60, 60, "strings soft"}, {73, 0, 0, 84, 90, "flute high"}, {48, 0, 0, 36, 90, "strings low"}};
#define TESTS (sizeof(tests) / sizeof(tests[0]))

static const char *const game_segments[] = {"segtheme1.sgt", "segegypt1.sgt", "seginca1.sgt", "segmed1.sgt",
    "segwest1.sgt", "segtheme2.sgt", "segegypt2.sgt", "seginca2.sgt", "segmed2.sgt", "segwest2.sgt"};
#define SEGMENTS (sizeof(game_segments) / sizeof(game_segments[0]))

static void PutWord(FILE *f, unsigned long v, int bytes) {
    while (bytes-- > 0) {
        fputc((int)(v & 0xff), f);
        v >>= 8;
    }
}

static FILE *OpenWav(const char *path) {
    FILE *f = fopen(path, "wb");

    if (f == NULL) {
        return NULL;
    }
    fwrite("RIFF\0\0\0\0WAVEfmt ", 1, 16, f);
    PutWord(f, 16, 4);
    PutWord(f, 1, 2);
    PutWord(f, 2, 2);
    PutWord(f, RATE, 4);
    PutWord(f, RATE * 4, 4);
    PutWord(f, 4, 2);
    PutWord(f, 16, 2);
    fwrite("data\0\0\0\0", 1, 8, f);
    return f;
}

static void CloseWav(FILE *f, DWORD bytes) {
    fseek(f, 4, SEEK_SET);
    PutWord(f, 36 + bytes, 4);
    fseek(f, 40, SEEK_SET);
    PutWord(f, bytes, 4);
    fclose(f);
}

static struct DMSegment *Load(struct DMLoader *loader, const char *file) {
    char path[MAX_PATH];
    DMObjectDesc desc;
    struct DMSegment *seg = NULL;
    static BYTE bytes[1 << 20];
    FILE *f;
    long n;

    snprintf(path, sizeof(path), "imusic\\%s", file);
    if ((f = fopen(path, "rb")) == NULL) {
        return NULL;
    }
    n = (long)fread(bytes, 1, sizeof(bytes), f);
    fclose(f);
    memset(&desc, 0, sizeof(desc));
    desc.dwSize = sizeof(desc);
    desc.dwValidData = 0x2 | 0x4 | 0x400;
    desc.guidClass = CLSID_DMSegment;
    desc.llMemLength = n;
    desc.pbMemData = bytes;
    MultiByteToWideChar(CP_ACP, 0, file, -1, desc.wszName, 64);
    loader->v->GetObject(loader, &desc, &IID_DMSegment, (void **)&seg);
    return seg;
}

/* ---- Windows' DirectMusic ---- */

typedef HRESULT(__stdcall *AllocPMsgFn)(struct DMPerformance *, ULONG, DMPMsg **);
typedef HRESULT(__stdcall *SendPMsgFn)(struct DMPerformance *, DMPMsg *);

static int RenderWindows(const char *out_path) {
    LPDIRECTSOUND ds;
    LPDIRECTSOUNDBUFFER buffer = NULL;
    struct DMPerformance *perf;
    struct DMMusic *music = NULL;
    struct DMPort *port = NULL;
    struct DMLoader *loader;
    DMPortParams params = {sizeof(params), 0, 24, 1, 2, RATE, 0, 0};
    DSBUFFERDESC bd;
    WAVEFORMATEX format[4];
    DWORD format_size = sizeof(format), buffer_size = 0, last = 0, written = 0, total;
    AllocPMsgFn alloc;
    SendPMsgFn send;
    REFERENCE_TIME rt;
    MUSIC_TIME mt, t0;
    FILE *out;
    unsigned int i;
    int started = 0;

    MiniwinUsePortableMusic(0);
    if (MiniwinDirectSoundCreate(NULL, &ds, NULL) != DS_OK) {
        printf("no audio device\n");
        return 0;
    }
    IDirectSound_SetCooperativeLevel(ds, GetDesktopWindow(), DSSCL_NORMAL);
    MiniwinCoInitialize(NULL);
    if (MiniwinCoCreateInstance(&CLSID_DMPerformance, NULL, CLSCTX_INPROC, &IID_DMPerformance, (void **)&perf) != S_OK ||
        perf->v->Init(perf, &music, ds, NULL) != S_OK) {
        printf("no performance\n");
        return 0;
    }
    music->v->CreatePort(music, &GUID_NULL, &params, &port, NULL);
    port->v->GetFormat(port, format, &format_size, &buffer_size);
    memset(&bd, 0, sizeof(bd));
    bd.dwSize = sizeof(bd);
    bd.dwFlags = DSBCAPS_CTRLVOLUME;
    bd.dwBufferBytes = buffer_size;
    bd.lpwfxFormat = format;
    IDirectSound_CreateSoundBuffer(ds, &bd, &buffer, NULL);
    MiniwinSoundKeepPlayingInactive(buffer);
    port->v->SetDirectSound(port, ds, buffer);
    port->v->Activate(port, TRUE);
    perf->v->AddPort(perf, port);
    perf->v->AssignPChannelBlock(perf, 0, port, 1);
    IDirectSoundBuffer_SetVolume(buffer, DSBVOLUME_MIN);
    MiniwinCoCreateInstance(&CLSID_DMLoader, NULL, CLSCTX_INPROC, &IID_DMLoader, (void **)&loader);
    loader->v->SetSearchDirectory(loader, &GUID_DMAllTypes, L"imusic", TRUE);
    loader->v->EnableCache(loader, &GUID_DMAllTypes, TRUE);
    /* as the game: the styles first (the world themes' instruments are in their styles' bands) */
    loader->v->ScanDirectory(loader, &CLSID_DMSegment, L"sgt", NULL);
    loader->v->ScanDirectory(loader, &CLSID_DMStyle, L"sty", NULL);
    {
        static void *styles[64];
        DMObjectDesc desc;

        memset(&desc, 0, sizeof(desc));
        desc.dwSize = sizeof(desc);
        for (i = 0; i < 64 && loader->v->EnumObject(loader, &CLSID_DMStyle, i, &desc) == S_OK; i++) {
            loader->v->GetObject(loader, &desc, &IID_DMStyle, &styles[i]);
        }
    }
    for (i = 0; i < SEGMENTS; i++) {
        struct DMSegment *seg = Load(loader, game_segments[i]);

        if (seg != NULL) {
            seg->v->SetParam(seg, &GUID_DMDownload, 0xffffffff, 0, 0, perf);
        }
    }
    /* every test instrument from Windows' GM collection, on its channel (the world themes' bands use the default GM
     * set, which the segments' downloads leave out) */
    {
        DMObjectDesc desc;
        void *collection = NULL;

        memset(&desc, 0, sizeof(desc));
        desc.dwSize = sizeof(desc);
        desc.dwValidData = 0x1 | 0x2; /* DMUS_OBJ_OBJECT | DMUS_OBJ_CLASS */
        desc.guidObject = GUID_DefaultGMCollection;
        desc.guidClass = CLSID_DMCollection;
        if (loader->v->GetObject(loader, &desc, &IID_DMCollection, &collection) != S_OK) {
            printf("no GM collection\n");
        } else {
            GetInstrumentFn get = (GetInstrumentFn)(*(void ***)collection)[3];
            DownloadInstrumentFn download = (DownloadInstrumentFn)((void **)perf->v)[29];
            int failed = 0;

            for (i = 0; i < TESTS; i++) {
                const Test *t = &tests[i];
                DWORD ch = t->drum ? 9 : (DWORD)(i % 15 == 9 ? 15 : i % 15);
                DWORD patch = t->drum ? 0x80000000u : (DWORD)(t->msb << 16 | t->program);
                void *instrument = NULL, *downloaded = NULL, *dport = NULL;
                DWORD group, mchannel;

                if (get(collection, patch, &instrument) != S_OK ||
                    download(perf, instrument, ch, &downloaded, NULL, 0, &dport, &group, &mchannel) != S_OK) {
                    failed++;
                }
            }
            printf("GM collection: %d of %u instruments not downloaded\n", failed, (unsigned)TESTS);
        }
    }
    alloc = (AllocPMsgFn)((void **)perf->v)[16];
    send = (SendPMsgFn)((void **)perf->v)[11];
    perf->v->GetTime(perf, &rt, &mt);
    t0 = mt + 1536; /* a second from now, at the default 120 bpm (1536 ticks a second) */
    for (i = 0; i < TESTS; i++) {
        const Test *t = &tests[i];
        DWORD ch = t->drum ? 9 : (DWORD)(i % 15 == 9 ? 15 : i % 15);
        MUSIC_TIME at = t0 + (MUSIC_TIME)(i * SLOT_SECONDS * 1536);
        DMPatchPMsg *patch;
        DMNotePMsg *note;

        if (alloc(perf, sizeof(DMPatchPMsg), (DMPMsg **)&patch) == S_OK) {
            patch->head.mtTime = at;
            patch->head.dwFlags = 2; /* DMUS_PMSGF_MUSICTIME */
            patch->head.dwPChannel = ch;
            patch->head.dwType = DMUS_PMSGT_PATCH;
            patch->byInstrument = (BYTE)t->program;
            patch->byMSB = (BYTE)t->msb;
            patch->byLSB = 0;
            send(perf, &patch->head);
        }
        if (alloc(perf, sizeof(DMNotePMsg), (DMPMsg **)&note) == S_OK) {
            note->head.mtTime = at + (MUSIC_TIME)(NOTE_START * 1536);
            note->head.dwFlags = 2;
            note->head.dwPChannel = ch;
            note->head.dwType = DMUS_PMSGT_NOTE;
            note->mtDuration = (MUSIC_TIME)(NOTE_SECONDS * 1536);
            note->wMusicValue = (WORD)t->note;
            note->bMidiValue = (BYTE)t->note;
            note->bVelocity = (BYTE)t->velocity;
            note->bFlags = 1; /* DMUS_NOTEF_NOTEON */
            note->bPlayModeFlags = 0; /* DMUS_PLAYMODE_FIXED: the music value is the MIDI note */
            send(perf, &note->head);
        }
    }
    out = OpenWav(out_path);
    total = (DWORD)((TESTS * SLOT_SECONDS + 2) * RATE * 4);
    while (written < total) {
        DWORD play_pos, write_pos, n1, n2;
        void *p1, *p2;

        Sleep(5);
        if (IDirectSoundBuffer_GetCurrentPosition(buffer, &play_pos, &write_pos) != DS_OK) {
            continue;
        }
        if (!started) {
            last = write_pos;
            started = 1;
            continue;
        }
        if (IDirectSoundBuffer_Lock(buffer, 0, buffer_size, &p1, &n1, &p2, &n2, DSBLOCK_ENTIREBUFFER) == DS_OK) {
            const BYTE *b = (const BYTE *)p1;

            while (last != write_pos && written < total) {
                DWORD end = write_pos > last ? write_pos : buffer_size;

                fwrite(b + last, 1, end - last, out);
                written += end - last;
                last = end % buffer_size;
            }
            IDirectSoundBuffer_Unlock(buffer, p1, n1, p2, n2);
        }
    }
    CloseWav(out, written);
    perf->v->CloseDown(perf);
    return 1;
}

/* ---- the portable synthesizer ---- */

static void *Resolve(void *ctx, char const *name, size_t *len) {
    const char *paths[] = {"C:\\Windows\\System32\\drivers\\gm.dls", "C:\\Windows\\Sysnative\\drivers\\gm.dls"};
    int i;

    (void)ctx;
    if (_stricmp(name, "gm.dls") != 0) {
        return NULL;
    }
    for (i = 0; i < 2; i++) {
        FILE *f = fopen(paths[i], "rb");
        void *p;
        long n;

        if (f == NULL) {
            continue;
        }
        fseek(f, 0, SEEK_END);
        n = ftell(f);
        fseek(f, 0, SEEK_SET);
        p = malloc(n);
        fread(p, 1, n, f);
        fclose(f);
        *len = (size_t)n;
        return p;
    }
    return NULL;
}

static int RenderPortable(const char *out_path) {
    DmLoader *loader;
    DmReference ref;
    DmDls *dls = NULL;
    tsf *syn = NULL;
    static short pcm[RATE * 2];
    FILE *out;
    unsigned int i;
    DWORD written = 0;
    DmeReverb *reverb;

    DmLoader_create(&loader, DmLoader_DEFAULT);
    DmLoader_addResolver(loader, Resolve, NULL);
    memset(&ref, 0, sizeof(ref));
    ref.file = "gm.dls";
    if (DmLoader_getDownloadableSound(loader, &ref, &dls) != DmResult_SUCCESS ||
        DmSynth_createTsfForDls(dls, &syn) != DmResult_SUCCESS) {
        printf("can't load gm.dls\n");
        return 0;
    }
    tsf_set_output(syn, TSF_STEREO_INTERLEAVED, RATE, 0);
    tsf_set_volume(syn, GAIN);
    reverb = DmeReverb_create(RATE);
    out = OpenWav(out_path);
    /* the same lead as Windows' (a second), so the slots line up the same way */
    memset(pcm, 0, sizeof(pcm));
    fwrite(pcm, 4, RATE, out);
    written += RATE * 4;
    for (i = 0; i < TESTS; i++) {
        const Test *t = &tests[i];
        int ch = t->drum ? 9 : (i % 15 == 9 ? 15 : (int)(i % 15));
        float vol = 100.0f / 127.0f, vel = t->velocity / 127.0f;
        int on = (int)(NOTE_START * RATE), off = (int)((NOTE_START + NOTE_SECONDS) * RATE);
        int s;

        if (!tsf_channel_set_bank_preset(syn, ch, t->drum ? 128 : t->msb << 8, t->program) &&
            !tsf_channel_set_bank_preset(syn, ch, t->drum ? 128 : 0, t->drum ? 0 : t->program)) {
            printf("no preset for %s\n", t->name);
        }
        tsf_channel_set_volume(syn, ch, vol * vol);
        tsf_channel_set_pan(syn, ch, 0.5f);
        for (s = 0; s < SLOT_SECONDS; s++) {
            int k0 = s * RATE, k1 = (s + 1) * RATE, k = k0;

            /* render up to the note's on and off within this second */
            while (k < k1) {
                int until = k1;

                if (on >= k && on < until) {
                    until = on;
                }
                if (off >= k && off < until) {
                    until = off;
                }
                if (until > k) {
                    tsf_render_short(syn, pcm + (k - k0) * 2, until - k, 0, 1);
                }
                if (until == on) {
                    tsf_channel_note_on(syn, ch, t->note, vel * vel);
                    on = -1;
                }
                if (until == off) {
                    tsf_channel_note_off(syn, ch, t->note);
                    off = -1;
                }
                k = until;
            }
            DmeReverb_process(reverb, pcm, RATE);
            fwrite(pcm, 4, RATE, out);
            written += RATE * 4;
        }
    }
    memset(pcm, 0, sizeof(pcm));
    fwrite(pcm, 4, RATE, out);
    written += RATE * 4;
    CloseWav(out, written);
    return 1;
}

#ifdef WITH_FLUIDSYNTH
/* ---- FluidSynth on gm.dls (its own DLS loader) ---- */

static int RenderFluid(const char *out_path) {
    fluid_settings_t *settings = new_fluid_settings();
    fluid_synth_t *syn;
    static short pcm[RATE * 2];
    const char *gain = getenv("FLUID_GAIN"), *rev = getenv("FLUID_REVERB");
    int own_reverb = rev == NULL || strcmp(rev, "own") == 0;
    DmeReverb *reverb = own_reverb ? DmeReverb_create(RATE) : NULL;
    FILE *out;
    unsigned int i;
    DWORD written = 0;
    int id;

    fluid_settings_setnum(settings, "synth.sample-rate", RATE);
    fluid_settings_setnum(settings, "synth.gain", gain != NULL ? atof(gain) : 0.5);
    fluid_settings_setint(settings, "synth.reverb.active", rev != NULL && strcmp(rev, "fluid") == 0);
    fluid_settings_setint(settings, "synth.chorus.active", 0);
    fluid_settings_setint(settings, "synth.polyphony", 256);
    syn = new_fluid_synth(settings);
    id = fluid_synth_sfload(syn, "C:\\Windows\\System32\\drivers\\gm.dls", 0);
    if (id < 0) {
        printf("FluidSynth can't load gm.dls\n");
        return 0;
    }
    out = OpenWav(out_path);
    memset(pcm, 0, sizeof(pcm));
    fwrite(pcm, 4, RATE, out);
    written += RATE * 4;
    for (i = 0; i < TESTS; i++) {
        const Test *t = &tests[i];
        int ch = t->drum ? 9 : (i % 15 == 9 ? 15 : (int)(i % 15));
        int on = (int)(NOTE_START * RATE), off = (int)((NOTE_START + NOTE_SECONDS) * RATE);
        int s;

        if (fluid_synth_program_select(syn, ch, id, t->drum ? 128 : t->msb, t->program) != FLUID_OK &&
            fluid_synth_program_select(syn, ch, id, t->drum ? 128 : 0, t->drum ? 0 : t->program) != FLUID_OK) {
            printf("FluidSynth: no preset for %s\n", t->name);
        }
        fluid_synth_cc(syn, ch, 7, 100);
        fluid_synth_cc(syn, ch, 11, 127);
        fluid_synth_cc(syn, ch, 10, 64);
        for (s = 0; s < SLOT_SECONDS; s++) {
            int k0 = s * RATE, k1 = (s + 1) * RATE, k = k0;

            while (k < k1) {
                int until = k1;

                if (on >= k && on < until) {
                    until = on;
                }
                if (off >= k && off < until) {
                    until = off;
                }
                if (until > k) {
                    short *p = pcm + (k - k0) * 2;

                    fluid_synth_write_s16(syn, until - k, p, 0, 2, p, 1, 2);
                }
                if (until == on) {
                    fluid_synth_noteon(syn, ch, t->note, t->velocity);
                    on = -1;
                }
                if (until == off) {
                    fluid_synth_noteoff(syn, ch, t->note);
                    off = -1;
                }
                k = until;
            }
            DmeReverb_process(reverb, pcm, RATE);
            fwrite(pcm, 4, RATE, out);
            written += RATE * 4;
        }
    }
    memset(pcm, 0, sizeof(pcm));
    fwrite(pcm, 4, RATE, out);
    written += RATE * 4;
    CloseWav(out, written);
    delete_fluid_synth(syn);
    delete_fluid_settings(settings);
    DmeReverb_destroy(reverb);
    return 1;
}
#endif

int main(int argc, char **argv) {
    if (argc < 4) {
        printf("usage: instruments_dmusic <data folder> <windows.wav> <portable.wav> [<fluidsynth.wav>]\n");
        return 2;
    }
    if (!SetCurrentDirectoryA(argv[1])) {
        printf("can't open %s\n", argv[1]);
        return 2;
    }
#ifdef WITH_FLUIDSYNTH
    if (argc > 4 && !RenderFluid(argv[4])) {
        return 1;
    }
    if (getenv("FLUID_ONLY") != NULL) {
        return 0;
    }
#endif
    if (!RenderPortable(argv[3]) || !RenderWindows(argv[2])) {
        return 1;
    }
    printf("%u instruments\n", (unsigned)TESTS);
    return 0;
}
