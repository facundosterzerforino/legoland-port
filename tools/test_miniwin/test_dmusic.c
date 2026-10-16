/* test_dmusic: plays the game's music the way the game's music thread sets it up (MusicThreadProc), without
 * starting the game, through Windows' DirectMusic or (with "portable") miniwin's (miniwin/src/dmusic.c), both on
 * miniwin's DirectSound. It checks every step succeeds, that music reaches miniwin's buffer, and that the segment and
 * beat notifications the game waits for arrive. Silent: the buffer's volume is at its minimum.
 *
 *   tools/test_miniwin/build.sh && tools/test_miniwin/test_dmusic.exe <data folder> [portable]
 *   (FAIL lines; exits 1 on any). Off Windows (tools/test_miniwin/build_linux.sh), always miniwin's, with the
 *   instruments from gm.dls or a SoundFont (miniwin/src/dmusic.c). */
#include <windows.h>
#include <dsound.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

#include "../music/dmi.h"
#include "miniwin/dmusic.h"
#include "miniwin/files.h"
#include "miniwin/miniwin.h"

static const GUID CLSID_DMStyle = {0xd2ac288a, 0xb39b, 0x11d1, {0x87, 0x04, 0x00, 0x60, 0x08, 0x93, 0xb1, 0xbd}};
static int failures;

static void Check(const char *what, HRESULT hr) {
    if (FAILED(hr)) {
        printf("FAIL %s: %08lx\n", what, (unsigned long)hr);
        failures++;
    }
}

static struct DMSegment *Load(struct DMLoader *loader, const char *data, const char *file, const WCHAR *name) {
    /* as the game: the file's bytes, handed over with a name */
    char path[MAX_PATH];
    static BYTE bytes[8][300000];
    static int used;
    FILE *f;
    DMObjectDesc desc;
    struct DMSegment *seg = NULL;
    long n;

    snprintf(path, sizeof(path), "%s\\imusic\\%s", data, file);
    f = MiniwinFopen(path, "rb"); /* a game path: backslashes, any case */
    if (f == NULL || used == 8) {
        printf("FAIL: can't read %s\n", path);
        failures++;
        return NULL;
    }
    n = (long)fread(bytes[used], 1, sizeof(bytes[0]), f);
    fclose(f);
    memset(&desc, 0, sizeof(desc));
    desc.dwSize = sizeof(desc);
    desc.dwValidData = 0x2 | 0x4 | 0x400; /* DMUS_OBJ_CLASS | DMUS_OBJ_NAME | DMUS_OBJ_MEMORY */
    desc.guidClass = CLSID_DMSegment;
    desc.llMemLength = n;
    desc.pbMemData = bytes[used++];
    wcscpy(desc.wszName, name);
    Check(file, loader->v->GetObject(loader, &desc, &IID_DMSegment, (void **)&seg));
    return seg;
}

int main(int argc, char **argv) {
    LPDIRECTSOUND ds;
    LPDIRECTSOUNDBUFFER buffer = NULL;
    struct DMPerformance *perf;
    struct DMMusic *music = NULL;
    struct DMPort *port = NULL;
    struct DMLoader *loader;
    struct DMSegment *seg;
    DMPortParams params = {sizeof(params), 0, 24, 1, 2, 22050, 0, 0};
    DSBUFFERDESC bd;
    WAVEFORMATEX format[4];
    DWORD format_size = sizeof(format), buffer_size = 0;
    WCHAR folder[MAX_PATH];
    HANDLE event = CreateEventA(NULL, TRUE, FALSE, NULL);
    DMNotificationPMsg *msg;
    int i, loud = 0, polls = 0, starts = 0, beats = 0;
    double energy = 0, samples = 0;
    DWORD last = 0;

    if (argc < 2) {
        printf("usage: test_dmusic <data folder> [portable]\n");
        return 2;
    }
    MiniwinUsePortableMusic(argc > 2 && strcmp(argv[2], "portable") == 0);
    printf("%s DirectMusic\n", MiniwinPortableMusic() ? "miniwin's" : "Windows'");
    MultiByteToWideChar(CP_ACP, 0, argv[1], -1, folder, MAX_PATH);
    wcscat(folder, L"\\imusic");
    if (MiniwinDirectSoundCreate(NULL, &ds, NULL) != DS_OK) {
        printf("no audio device: nothing to test\n");
        return 0;
    }
    IDirectSound_SetCooperativeLevel(ds, GetDesktopWindow(), DSSCL_NORMAL);
    MiniwinCoInitialize(NULL);
    Check("create performance",
        MiniwinCoCreateInstance(&CLSID_DMPerformance, NULL, CLSCTX_INPROC, &IID_DMPerformance, (void **)&perf));
    if (failures) {
        return 1;
    }
    Check("performance Init", perf->v->Init(perf, &music, ds, NULL));
    Check("CreatePort", music->v->CreatePort(music, &GUID_NULL, &params, &port, NULL));
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
    Check("create loader",
        MiniwinCoCreateInstance(&CLSID_DMLoader, NULL, CLSCTX_INPROC, &IID_DMLoader, (void **)&loader));
    if (failures) {
        return 1;
    }
    Check("SetSearchDirectory", loader->v->SetSearchDirectory(loader, &GUID_DMAllTypes, folder, TRUE));
    loader->v->EnableCache(loader, &GUID_DMAllTypes, TRUE);
    Check("scan styles", loader->v->ScanDirectory(loader, &CLSID_DMStyle, L"sty", NULL));
    seg = Load(loader, argv[1], "segtheme2.sgt", L"theme");
    if (seg == NULL) {
        return 1;
    }
    Check("download", seg->v->SetParam(seg, &GUID_DMDownload, 0xffffffff, 0, 0, perf));
    perf->v->SetNotificationHandle(perf, event, 0);
    perf->v->AddNotificationType(perf, &GUID_DMNotifyMeasureBeat);
    perf->v->AddNotificationType(perf, &GUID_DMNotifySegment);
    Check("PlaySegment", perf->v->PlaySegment(perf, seg, 0x2000, 0, NULL));
    /* five seconds: music in the buffer, and the notifications */
    for (i = 0; i < 500; i++) {
        DWORD play, write, n1, n2;
        void *p1, *p2;

        WaitForSingleObject(event, 10);
        ResetEvent(event);
        while (perf->v->GetNotificationPMsg(perf, &msg) == S_OK) {
            if (IsEqualGUID(&msg->guidNotificationType, &GUID_DMNotifySegment)) {
                starts += msg->dwNotificationOption == 0;
            } else {
                beats++;
            }
            perf->v->FreePMsg(perf, &msg->head);
        }
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
            /* the level of what is final: up to the write cursor for Windows' DirectMusic (it writes ahead of it and
             * clears what has played), up to the play cursor for miniwin's (it fills just ahead of it) */
            if (!MiniwinPortableMusic()) {
                play = write;
            }
            for (k = last / 2; i > 0 && k != play / 2; k = (k + 1) % (n1 / 2)) {
                energy += (double)s[k] * s[k];
                samples++;
            }
            last = play;
            IDirectSoundBuffer_Unlock(buffer, p1, n1, p2, n2);
            polls++;
        }
    }
    printf("music heard in %d of %d looks; %d segment starts, %d beats; level %.0f RMS\n", loud, polls, starts, beats,
        samples > 0 ? sqrt(energy / samples) : 0.0);
    if (loud < polls / 2 && polls > 0) {
        printf("FAIL: little or no music in miniwin's buffer\n");
        failures++;
    }
    if (starts != 1 || beats < 3) {
        printf("FAIL: expected a segment start and a beat a second (60 bpm)\n");
        failures++;
    }
    perf->v->Stop(perf, NULL, NULL, 0, 0);
    perf->v->CloseDown(perf);
    perf->v->Release(perf);
    printf("%s\n", failures ? "FAILED" : "OK");
    return failures != 0;
}
