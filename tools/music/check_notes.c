/* check_notes: level 1 of the music check (tools/music/README.md). For every note in recordings of Windows'
 * DirectMusic (record_dmusic.exe), works out the pitch dmusic (port/dmusic) gives the same music value under the
 * same play mode and chord, and compares it with the pitch DirectMusic played. The chord is the segment's own
 * (its chord track), so this doesn't depend on which variations either side picked.
 *
 *   check_notes.exe <data folder> <recording>...      (a summary, the first mismatches; exits 1 on any) */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "_Internal.h"

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
        return NULL;
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
    if (lvl <= DmLogLevel_ERROR) {
        fprintf(stderr, "dmusic: %s\n", msg);
    }
}

/* the chord in force at time t (from the segment's start) */
static DmMessage_Chord *ChordAt(DmSegment *seg, long t) {
    DmMessage_Chord *best = NULL;
    size_t i;

    for (i = 0; i < seg->messages.length; i++) {
        DmMessage *m = &seg->messages.data[i];

        if (m->type == DmMessage_CHORD && (long)m->time <= t && (best == NULL || m->time >= best->time)) {
            best = &m->chord;
        }
    }
    return best;
}

int main(int argc, char **argv) {
    DmLoader *loader;
    long total = 0, wrong = 0, shown = 0, nochord = 0;
    int i;

    if (argc < 3) {
        printf("usage: check_notes <data folder> <recording>...\n");
        return 2;
    }
    snprintf(music_dir, sizeof(music_dir), "%s\\imusic", argv[1]);
    Dm_setLogger(DmLogLevel_ERROR, Quiet, NULL);
    DmLoader_create(&loader, DmLoader_DEFAULT);
    DmLoader_addResolver(loader, Resolve, NULL);
    for (i = 2; i < argc; i++) {
        FILE *f = fopen(argv[i], "r");
        char line[512], name[256] = "";
        DmSegment *seg = NULL;
        long start = -1;
        long file_total = 0, file_wrong = 0;

        if (f == NULL) {
            printf("FAIL: can't read %s\n", argv[i]);
            wrong++;
            continue;
        }
        /* the notes are written as they are made, ahead of time; the segment's start comes later in the file */
        while (fgets(line, sizeof(line), f)) {
            long mt;

            if (sscanf(line, "segment-start %ld", &mt) == 1) {
                start = mt;
            }
        }
        rewind(f);
        while (fgets(line, sizeof(line), f)) {
            long mt, dur;
            unsigned ch, midi, vel, mv, meas, beat, grid, flags, pm, sub, tr_r, dur_r, vel_r;
            int off, tr;

            if (seg == NULL && strncmp(line, "segment ", 8) == 0 && sscanf(line, "segment %255s", name) == 1) {
                if (DmLoader_getSegment(loader, name, &seg) != DmResult_SUCCESS) {
                    printf("FAIL %s: dmusic can't load %s\n", argv[i], name);
                    wrong++;
                    break;
                }
            } else if (sscanf(line,
                           "note %ld ch %u midi %u vel %u dur %ld mv %x meas %u beat %u grid %u off %d flags %x pm %x "
                           "sub %x tr %d ranges %u %u %u",
                           &mt, &ch, &midi, &vel, &dur, &mv, &meas, &beat, &grid, &off, &flags, &pm, &sub, &tr,
                           &tr_r, &dur_r, &vel_r) == 17) {
                DmMessage_Chord *chord;
                struct DmSubChord level;
                int got;
                size_t k;

                if (start < 0 || seg == NULL) {
                    continue;
                }
                chord = ChordAt(seg, mt - start - off);
                file_total++;
                if (chord == NULL) {
                    /* no chord track: which default chord explains DirectMusic's pitches? */
                    static const struct DmSubChord candidates[] = {
                        {0, 0, 0, 0xffffffff, 0, 0},
                        {0x91, 0xab5ab5, 0, 0xffffffff, 0, 0},
                        {0x91, 0xab5ab5, 0, 0xffffffff, 12, 0},
                        {0x91, 0xab5ab5, 0, 0xffffffff, 24, 0},
                        {0x91, 0xab5ab5, 0, 0xffffffff, 48, 0},
                        {0x1, 0xab5ab5, 0, 0xffffffff, 0, 0},
                    };
                    static long fits[6];
                    size_t c;

                    nochord++;
                    for (c = 0; c < 6; c++) {
                        int g = pm == 0 ? (int)mv
                                        : DmPerformance_musicValueToMidi(candidates[c], (DmPlayModeFlags)pm, (uint16_t)mv);

                        fits[c] += g == (int)midi;
                    }
                    if (nochord % 500 == 0) {
                        printf("  without a chord, %ld notes: fits %ld %ld %ld %ld %ld %ld\n", nochord, fits[0], fits[1],
                            fits[2], fits[3], fits[4], fits[5]);
                    }
                    continue;
                }
                level = chord->subchords[0];
                for (k = 0; k < chord->subchord_count; k++) {
                    if (chord->subchords[k].levels & (1u << sub)) {
                        level = chord->subchords[k];
                        break;
                    }
                }
                got = pm == DmPlayMode_FIXED ? (int)mv : DmPerformance_musicValueToMidi(level, (DmPlayModeFlags)pm, (uint16_t)mv);
                if (got != (int)midi) {
                    file_wrong++;
                    if (shown++ < 25) {
                        printf("  %s t=%ld ch %u: mv %04x pm %x sub %u chord '%s' root %u pattern %06x scale %06x/%u: "
                               "DirectMusic %u, dmusic %d\n",
                            name, mt - start, ch, mv, pm, sub, chord->name, level.chord_root, level.chord_pattern,
                            level.scale_pattern, level.scale_root, midi, got);
                    }
                }
            }
        }
        fclose(f);
        total += file_total;
        wrong += file_wrong;
        if (seg != NULL) {
            DmSegment_release(seg);
        }
    }
    printf("%ld notes, %ld with a different pitch, %ld without a chord\n", total, wrong, nochord);
    return wrong != 0;
}
