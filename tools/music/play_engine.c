/* play_engine: plays segments through miniwin's style engine (miniwin/src/dmengine.c) offline, without instruments,
 * and writes what it plays in record_dmusic's format, so that check_notes and check_bars judge it the same way as
 * Windows' DirectMusic (tools/music/README.md).
 *
 *   play_engine.exe <data folder> <out folder> <runs> <master groove> <segment.sgt>... */
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "_Internal.h"
#include "miniwin/dmengine.h"

static char music_dir[1024];

static void *Resolve(void *ctx, char const *name, size_t *len) {
    char path[1200];
    FILE *f;
    void *p;
    long n;

    (void)ctx;
    snprintf(path, sizeof(path), "%s\\%s", music_dir, name);
    f = fopen(path, "rb");
    if (f == NULL) {
        return NULL; /* gm.dls too: no instruments, nothing synthesized */
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

static void Quiet(void *ctx, DmLogLevel lvl, char const *msg) {
    (void)ctx;
    (void)lvl;
    (void)msg;
}

int main(int argc, char **argv) {
    DmLoader *loader;
    WIN32_FIND_DATAA fd;
    char pattern[1100];
    HANDLE find;
    int runs, groove, i, r;
    static short pcm[4096 * 2];

    if (argc < 6) {
        printf("usage: play_engine <data folder> <out folder> <runs> <master groove> <segment>...\n");
        return 2;
    }
    snprintf(music_dir, sizeof(music_dir), "%s\\imusic", argv[1]);
    runs = atoi(argv[3]);
    groove = atoi(argv[4]);
    Dm_setLogger(DmLogLevel_FATAL, Quiet, NULL);
    DmLoader_create(&loader, DmLoader_DEFAULT);
    DmLoader_addResolver(loader, Resolve, NULL);
    /* the styles, as the game's directory scan loads them */
    snprintf(pattern, sizeof(pattern), "%s\\*.sty", music_dir);
    find = FindFirstFileA(pattern, &fd);
    if (find != INVALID_HANDLE_VALUE) {
        do {
            DmReference ref;
            DmStyle *sty = NULL;

            memset(&ref, 0, sizeof(ref));
            ref.file = fd.cFileName;
            ref.guid.data[0] = 0xff;
            if (DmLoader_getStyle(loader, &ref, &sty) == DmResult_SUCCESS) {
                DmStyle_release(sty);
            }
        } while (FindNextFileA(find, &fd));
        FindClose(find);
    }
    for (i = 5; i < argc; i++) {
        for (r = 0; r < runs; r++) {
            DmSegment *seg = NULL;
            DmeEngine *e = Dme_create(loader, 22050);
            DmeNotification n;
            char path[1200], base[256], *dot;
            FILE *log;
            int ended = 0;
            long guard = 0;

            strncpy(base, argv[i], sizeof(base) - 1);
            base[sizeof(base) - 1] = 0;
            dot = strrchr(base, '.');
            if (dot != NULL) {
                *dot = 0;
            }
            snprintf(path, sizeof(path), "%s\\%s.g%d.r%d.txt", argv[2], base, groove, r);
            if (DmLoader_getSegment(loader, argv[i], &seg) != DmResult_SUCCESS) {
                printf("FAIL: can't load %s\n", argv[i]);
                Dme_destroy(e);
                continue;
            }
            log = fopen(path, "w");
            fprintf(log, "segment %s groove %d\n", argv[i], groove);
            Dme_setLog(e, log);
            Dme_setMasterGroove(e, groove);
            Dme_setNotificationTypes(e, (1u << DME_SEGMENT) | (1u << DME_MEASURE_AND_BEAT));
            Dme_play(e, seg, 0, DME_AT_ONCE);
            while (!ended && guard++ < 1000000) {
                Dme_render(e, pcm, 4096);
                while (Dme_takeNotification(e, &n)) {
                    if (n.type == DME_SEGMENT && (n.option == DME_SEG_END || n.option == DME_SEG_ABORT)) {
                        ended = 1;
                    }
                }
            }
            fclose(log);
            Dme_destroy(e);
            DmSegment_release(seg);
        }
        printf("%s: %d runs\n", argv[i], runs);
        fflush(stdout);
    }
    return 0;
}
