/* capture_dmusic: sets the music up as the game's music thread does (MusicThreadProc: scan the segments and styles,
 * load every style and segment from memory, download the world themes and transitions), plays segments one after the
 * other, as the game does on each segment's end, through Windows' DirectMusic or (with "portable") miniwin's, and
 * writes what reaches miniwin's music buffer to a WAV file, to compare the two. Silent: the buffer's volume is at its
 * minimum. The WAV is derived from the game's music: keep it out of the repository.
 *
 *   capture_dmusic.exe <data folder> windows|portable <out.wav> <seconds> <segment.sgt>... */
#include <windows.h>
#include <dsound.h>
#include <stdio.h>
#include <string.h>

#include "../music/dmi.h"
#include "miniwin/dmusic.h"
#include "miniwin/miniwin.h"

static const GUID CLSID_DMStyle = {0xd2ac288a, 0xb39b, 0x11d1, {0x87, 0x04, 0x00, 0x60, 0x08, 0x93, 0xb1, 0xbd}};
static const GUID IID_DMStyle = {0xd2ac28bd, 0xb39b, 0x11d1, {0x87, 0x04, 0x00, 0x60, 0x08, 0x93, 0xb1, 0xbd}};

/* the game's segments, in its loading order; the main theme's two are never downloaded */
static const char *const game_segments[] = {"segtheme1.sgt", "segegypt1.sgt", "seginca1.sgt", "segmed1.sgt",
    "segwest1.sgt", "segtheme2.sgt", "segegypt2.sgt", "seginca2.sgt", "segmed2.sgt", "segwest2.sgt", "letran2.sgt",
    "litran2.sgt", "lmtran2.sgt", "lwtran2.sgt", "eltran2.sgt", "ietran2.sgt", "emtran2.sgt", "ewtran2.sgt",
    "iltran2.sgt", "imtran2.sgt", "iwtran2.sgt", "mltran2.sgt", "metran2.sgt", "mitran2.sgt", "mwtran2.sgt",
    "wltran2.sgt", "wetran2.sgt", "witran2.sgt", "wmtran2.sgt"};
#define SEGMENTS (sizeof(game_segments) / sizeof(game_segments[0]))

static struct DMSegment *Load(struct DMLoader *loader, const char *file) {
    char path[MAX_PATH];
    FILE *f;
    DMObjectDesc desc;
    struct DMSegment *seg = NULL;
    BYTE *bytes = (BYTE *)malloc(1 << 20);
    long n;

    snprintf(path, sizeof(path), "imusic\\%s", file);
    f = fopen(path, "rb");
    if (f == NULL) {
        printf("can't read %s\n", path);
        return NULL;
    }
    n = (long)fread(bytes, 1, 1 << 20, f);
    fclose(f);
    memset(&desc, 0, sizeof(desc));
    desc.dwSize = sizeof(desc);
    desc.dwValidData = 0x2 | 0x4 | 0x400; /* DMUS_OBJ_CLASS | DMUS_OBJ_NAME | DMUS_OBJ_MEMORY */
    desc.guidClass = CLSID_DMSegment;
    desc.llMemLength = n;
    desc.pbMemData = bytes;
    MultiByteToWideChar(CP_ACP, 0, file, -1, desc.wszName, 64);
    if (loader->v->GetObject(loader, &desc, &IID_DMSegment, (void **)&seg) != S_OK) {
        printf("can't load %s\n", file);
    }
    free(bytes); /* as the game: the loader keeps its own copy */
    return seg;
}

static void PutWord(FILE *f, unsigned long v, int bytes) {
    while (bytes-- > 0) {
        fputc((int)(v & 0xff), f);
        v >>= 8;
    }
}

int main(int argc, char **argv) {
    LPDIRECTSOUND ds;
    LPDIRECTSOUNDBUFFER buffer = NULL;
    struct DMPerformance *perf;
    struct DMMusic *music = NULL;
    struct DMPort *port = NULL;
    struct DMLoader *loader;
    struct DMSegment *segs[SEGMENTS];
    struct DMSegment *play[32];
    DMPortParams params = {sizeof(params), 0, 24, 1, 2, 22050, 0, 0};
    DMObjectDesc desc;
    DSBUFFERDESC bd;
    WAVEFORMATEX format[4];
    DWORD format_size = sizeof(format), buffer_size = 0, last = 0, written = 0, total;
    HANDLE event = CreateEventA(NULL, TRUE, FALSE, NULL);
    DMNotificationPMsg *msg;
    void *styles[64];
    unsigned int i, k, nplay = 0, next = 0;
    int started = 0;
    FILE *out;

    if (argc < 6) {
        printf("usage: capture_dmusic <data folder> windows|portable <out.wav> <seconds> <segment.sgt>...\n");
        return 2;
    }
    MiniwinUsePortableMusic(strcmp(argv[2], "portable") == 0);
    out = fopen(argv[3], "wb");
    if (out == NULL || !SetCurrentDirectoryA(argv[1])) {
        printf("can't write %s or open %s\n", argv[3], argv[1]);
        return 2;
    }
    if (MiniwinDirectSoundCreate(NULL, &ds, NULL) != DS_OK) {
        printf("no audio device\n");
        return 1;
    }
    IDirectSound_SetCooperativeLevel(ds, GetDesktopWindow(), DSSCL_NORMAL);
    MiniwinCoInitialize(NULL);
    if (MiniwinCoCreateInstance(&CLSID_DMPerformance, NULL, CLSCTX_INPROC, &IID_DMPerformance, (void **)&perf) != S_OK ||
        perf->v->Init(perf, &music, ds, NULL) != S_OK) {
        printf("no performance\n");
        return 1;
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
    port->v->Release(port);
    MiniwinCoCreateInstance(&CLSID_DMLoader, NULL, CLSCTX_INPROC, &IID_DMLoader, (void **)&loader);
    IDirectSoundBuffer_SetVolume(buffer, DSBVOLUME_MIN); /* silent: miniwin applies it when mixing */
    loader->v->SetSearchDirectory(loader, &GUID_DMAllTypes, L"imusic", TRUE);
    loader->v->EnableCache(loader, &GUID_DMAllTypes, TRUE);
    loader->v->ScanDirectory(loader, &CLSID_DMSegment, L"sgt", NULL);
    loader->v->ScanDirectory(loader, &CLSID_DMStyle, L"sty", NULL);
    memset(&desc, 0, sizeof(desc));
    desc.dwSize = sizeof(desc);
    for (i = 0; i < 64 && loader->v->EnumObject(loader, &CLSID_DMStyle, i, &desc) == S_OK; i++) {
        loader->v->GetObject(loader, &desc, &IID_DMStyle, &styles[i]);
    }
    printf("%u styles\n", i);
    for (i = 0; i < SEGMENTS; i++) {
        segs[i] = Load(loader, game_segments[i]);
    }
    perf->v->SetNotificationHandle(perf, event, 0);
    perf->v->AddNotificationType(perf, &GUID_DMNotifyMeasureBeat);
    perf->v->AddNotificationType(perf, &GUID_DMNotifySegment);
    for (i = 0; i < SEGMENTS; i++) {
        if (segs[i] != NULL && strncmp(game_segments[i], "segtheme", 8) != 0) {
            segs[i]->v->SetParam(segs[i], &GUID_DMDownload, 0xffffffff, 0, 0, perf);
            segs[i]->v->SetRepeats(segs[i], 0);
        }
    }
    for (k = 5; k < (unsigned)argc && nplay < 32; k++) {
        for (i = 0; i < SEGMENTS; i++) {
            if (_stricmp(argv[k], game_segments[i]) == 0 && segs[i] != NULL) {
                play[nplay++] = segs[i];
            }
        }
    }
    if (nplay == 0) {
        printf("no segment to play\n");
        return 1;
    }
    /* the WAV header; the sizes are written at the end */
    fwrite("RIFF\0\0\0\0WAVEfmt ", 1, 16, out);
    PutWord(out, 16, 4);
    PutWord(out, format[0].wFormatTag, 2);
    PutWord(out, format[0].nChannels, 2);
    PutWord(out, format[0].nSamplesPerSec, 4);
    PutWord(out, format[0].nAvgBytesPerSec, 4);
    PutWord(out, format[0].nBlockAlign, 2);
    PutWord(out, format[0].wBitsPerSample, 2);
    fwrite("data\0\0\0\0", 1, 8, out);
    total = (DWORD)atoi(argv[4]) * format[0].nAvgBytesPerSec;
    perf->v->PlaySegment(perf, play[next++], 0x2000, 0, NULL);
    while (written < total) {
        DWORD play_pos, write_pos;
        void *p1, *p2;
        DWORD n1, n2;

        WaitForSingleObject(event, 5);
        ResetEvent(event);
        while (perf->v->GetNotificationPMsg(perf, &msg) == S_OK) {
            /* as the game's music thread: on "almost end", the next segment at the next measure (the last one again
             * and again, as the game loops a theme) */
            if (IsEqualGUID(&msg->guidNotificationType, &GUID_DMNotifySegment) && msg->dwNotificationOption == 2) {
                perf->v->PlaySegment(perf, play[next < nplay ? next : nplay - 1], 0x2000, 0, NULL);
                next++;
            }
            perf->v->FreePMsg(perf, &msg->head);
        }
        /* the final part of the buffer: up to the write cursor for Windows' DirectMusic (which writes ahead of it and
         * clears what has played), up to the play cursor for miniwin's (which fills just ahead of it as it plays) */
        if (IDirectSoundBuffer_GetCurrentPosition(buffer, &play_pos, &write_pos) != DS_OK) {
            continue;
        }
        if (!MiniwinPortableMusic()) {
            play_pos = write_pos;
        }
        if (!started) {
            last = play_pos;
            started = 1;
            continue;
        }
        if (IDirectSoundBuffer_Lock(buffer, 0, buffer_size, &p1, &n1, &p2, &n2, DSBLOCK_ENTIREBUFFER) == DS_OK) {
            const BYTE *b = (const BYTE *)p1;

            while (last != play_pos && written < total) {
                DWORD end = play_pos > last ? play_pos : buffer_size;

                fwrite(b + last, 1, end - last, out);
                written += end - last;
                last = end % buffer_size;
            }
            IDirectSoundBuffer_Unlock(buffer, p1, n1, p2, n2);
        }
    }
    fseek(out, 4, SEEK_SET);
    PutWord(out, 36 + written, 4);
    fseek(out, 40, SEEK_SET);
    PutWord(out, written, 4);
    fclose(out);
    printf("%lu bytes\n", (unsigned long)written);
    perf->v->Stop(perf, NULL, NULL, 0, 0);
    perf->v->CloseDown(perf);
    return 0;
}
