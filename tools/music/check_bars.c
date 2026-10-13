/* check_bars: level 2 of the music check (tools/music/README.md). For every bar of recordings of Windows'
 * DirectMusic (record_dmusic.exe), works out from the segment and its styles (as dmusic loads them) which pattern
 * plays there and, for each of its parts, the notes of every variation; the notes DirectMusic played on that part's
 * channel must be exactly one variation's (within the notes' random time, duration and velocity ranges). Writes the
 * variation found for each bar and part to <recording>.choices, for the statistics.
 *
 *   check_bars.exe <data folder> <recording>...      (a summary, the first misses; exits 1 on any) */
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "_Internal.h"

#define MAX_NOTES 20000
#define MEASURE 3072 /* 4/4 at 768 ticks a beat: all of the game's music */

typedef struct Note {
    long t; /* from the segment's start */
    long g; /* the start of the bar its pattern played in (from DirectMusic's own measure, beat and grid) */
    int midi, vel;
    long dur;
    int time_range, dur_range, vel_range;
    int ch;
    int used;
    int grid_start, offset; /* the style's own values, for DETAIL */
} Note;

static char music_dir[1024];
static Note rec[MAX_NOTES];
static int debug_matches;
static int rec_count;

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

/* the latest message of a type at or before t */
static DmMessage *LatestAt(DmSegment *seg, DmMessageType type, long t) {
    DmMessage *best = NULL;
    size_t i;

    for (i = 0; i < seg->messages.length; i++) {
        DmMessage *m = &seg->messages.data[i];
        long mt = m->time == 0xffffffff ? 0 : (long)m->time;

        if (m->type == type && mt <= t && (best == NULL || mt >= (best->time == 0xffffffff ? 0 : (long)best->time))) {
            best = m;
        }
    }
    return best;
}

static struct DmSubChord DefaultChord(void) {
    struct DmSubChord c = {0x91, 0xab5ab5, 0, 0xffffffff, 12, 0}; /* DirectMusic's chord without a chord track */
    return c;
}

static struct DmSubChord ChordAt(DmSegment *seg, long t, int level) {
    DmMessage *m = LatestAt(seg, DmMessage_CHORD, t);
    size_t k;

    if (m == NULL) {
        return DefaultChord();
    }
    for (k = 0; k < m->chord.subchord_count; k++) {
        if (m->chord.subchords[k].levels & (1u << level)) {
            return m->chord.subchords[k];
        }
    }
    return m->chord.subchords[0];
}

static int ByTime(const void *a, const void *b) {
    const Note *x = (const Note *)a, *y = (const Note *)b;

    if (x->t != y->t) {
        return x->t < y->t ? -1 : 1;
    }
    return x->midi - y->midi;
}

/* Do the expected notes (exp, n) match the recorded notes of channel ch in [t0, t0+len)? */
static int Matches(Note *exp, int n, int ch, long t0, long len) {
    Note got[512];
    int g = 0, i, j;

    for (i = 0; i < rec_count; i++) {
        if (rec[i].ch == ch && rec[i].g >= t0 && rec[i].g < t0 + len && g < 512) {
            got[g++] = rec[i];
        }
    }
    if (g != n) {
        if (debug_matches) {
            printf("      [match: %d played, %d expected]\n", g, n);
        }
        return 0;
    }
    /* each expected note pairs with a recorded one (within ranges) */
    for (i = 0; i < g; i++) {
        got[i].used = 0;
    }
    for (i = 0; i < n; i++) {
        int found = 0;

        for (j = 0; j < g && !found; j++) {
            if (!got[j].used && got[j].midi == exp[i].midi && labs(got[j].t - exp[i].t) <= exp[i].time_range &&
                labs(got[j].dur - exp[i].dur) <= exp[i].dur_range && abs(got[j].vel - exp[i].vel) <= exp[i].vel_range) {
                got[j].used = 1;
                found = 1;
            }
        }
        if (!found) {
            if (debug_matches) {
                printf("      [match: expected t %ld m %d v %d d %ld not played]\n", exp[i].t, exp[i].midi, exp[i].vel,
                    exp[i].dur);
            }
            return 0;
        }
    }
    return 1;
}

/* The notes variation v of a part plays in the bar starting at t0. */
static int Expected(DmSegment *seg, DmPart *part, DmPartReference *pref, int v, long t0, Note *exp, int room) {
    int n = 0;
    uint32_t k;

    for (k = 0; k < part->note_count && n < room; k++) {
        DmNote *note = &part->notes[k];
        DmPlayModeFlags pm = note->play_mode_flags == DmPlayMode_NONE ? part->play_mode_flags : note->play_mode_flags;
        long grid_t = (long)Dm_getTimeOffset(note->grid_start, 0, part->time_signature);

        if (!(note->variation & (1u << v))) {
            continue;
        }
        if (getenv("KEEP_LATE") == NULL && grid_t >= (long)Dm_getMeasureLength(part->time_signature) * part->length_measures) {
            continue; /* a note placed at or past the part's end isn't played */
        }
        exp[n].midi = pm == DmPlayMode_FIXED ? note->music_value
                                             : DmPerformance_musicValueToMidi(
                                                   ChordAt(seg, t0 + grid_t, pref->subchord_level), pm, note->music_value);
        exp[n].t = t0 + grid_t + note->time_offset;
        exp[n].g = t0 + grid_t;
        exp[n].vel = note->velocity;
        exp[n].dur = note->duration;
        exp[n].time_range = (int)note->time_range;
        exp[n].dur_range = (int)note->duration_range;
        exp[n].vel_range = note->velocity_range;
        exp[n].ch = pref->logical_part_id;
        exp[n].grid_start = (int)note->grid_start;
        exp[n].offset = note->time_offset;
        n++;
    }
    return n;
}

/* DETAIL=<segment>:<bar>:<channel>: what was played there, and what each variation of each part would play. */
static void Detail(const char *name, long b, int ch, DmSegment *seg, DmPart **gpart, DmPartReference **group, int ng,
    long t0) {
    const char *want = getenv("DETAIL");
    char key[300];
    int i, g, v;

    snprintf(key, sizeof(key), "%s:%ld:%d", name, b, ch);
    if (want == NULL || strcmp(want, key) != 0) {
        return;
    }
    printf("    played:\n");
    for (i = 0; i < rec_count; i++) {
        if (rec[i].ch == ch && rec[i].g == t0) {
            printf("      t %ld (grid %ld) midi %d vel %d dur %ld\n", rec[i].t, rec[i].g, rec[i].midi, rec[i].vel,
                rec[i].dur);
        }
    }
    for (g = 0; g < ng; g++) {
        printf("    part %d of %d: '%s', lock %u, mode %d, sub %u, %u notes, part time signature %u/%u grids %u, "
               "choices:",
            g + 1, ng, gpart[g]->info.unam, group[g]->variation_lock_id, group[g]->random_variation,
            group[g]->subchord_level, gpart[g]->note_count, gpart[g]->time_signature.beats_per_measure,
            gpart[g]->time_signature.beat, gpart[g]->time_signature.grids_per_beat);
        for (v = 0; v < 32; v++) {
            if (gpart[g]->variation_choices[v] & 0x0FFFFFFF) {
                printf(" %d:%08x", v, gpart[g]->variation_choices[v]);
            }
        }
        printf("\n");
        for (v = 0; v < 32; v++) {
            Note exp[512];
            int n;

            if ((gpart[g]->variation_choices[v] & 0x0FFFFFFF) == 0) {
                continue;
            }
            n = Expected(seg, gpart[g], group[g], v, t0, exp, 512);
            printf("      variation %d:", v);
            for (i = 0; i < n; i++) {
                printf(" [t %ld m %d v %d d %ld r %d/%d/%d grid %d off %d]", exp[i].t, exp[i].midi, exp[i].vel,
                    exp[i].dur, exp[i].time_range, exp[i].dur_range, exp[i].vel_range, exp[i].grid_start, exp[i].offset);
            }
            printf("\n");
        }
    }
}

/* One channel of a pattern in one bar: its part references and the variation combinations that match. */
typedef struct Result {
    int ch, ng, nfound, lock, mode;
    int candidates; /* the variation combinations allowed */
    DmPartReference *group[8];
    DmPart *gpart[8];
    char found[400];
} Result;

static const char *cur_name;

static int CheckGroup(DmSegment *seg, DmStyle *sty, DmPattern *pttn, size_t r, long t0, Result *x) {
    int valid[8][32], nvalid[8], idx[8], assign[8], g0;
    size_t q;
    char key[300];

    x->ch = pttn->parts.data[r].logical_part_id;
    snprintf(key, sizeof(key), "%s:%ld:%d", cur_name, t0 / MEASURE, x->ch);
    debug_matches = getenv("DETAIL") != NULL && strcmp(getenv("DETAIL"), key) == 0;
    x->ng = 0;
    x->nfound = 0;
    x->found[0] = 0;
    for (q = r; q < pttn->parts.length && x->ng < 8; q++) {
        if (pttn->parts.data[q].logical_part_id == x->ch) {
            DmPart *part = DmStyle_findPart(sty, &pttn->parts.data[q]);

            if (part != NULL) {
                x->group[x->ng] = &pttn->parts.data[q];
                x->gpart[x->ng] = part;
                x->ng++;
            }
        }
    }
    if (x->ng == 0) {
        return 0;
    }
    x->lock = x->group[0]->variation_lock_id;
    x->mode = x->group[0]->random_variation;
    x->candidates = 0;
    /* every combination of valid variations, those sharing a lock id choosing the same one */
    for (g0 = 0; g0 < x->ng; g0++) {
        int v;

        nvalid[g0] = 0;
        /* a locked part plays the variation its lock group chose: the first part of the group (in the pattern's
         * order) chooses among its own enabled variations, and the others follow, even where they disable it */
        DmPart *chooser = x->gpart[g0];
        size_t o;

        for (o = 0; o < pttn->parts.length && x->group[g0]->variation_lock_id != 0; o++) {
            if (pttn->parts.data[o].variation_lock_id == x->group[g0]->variation_lock_id) {
                DmPart *op = DmStyle_findPart(sty, &pttn->parts.data[o]);

                if (op != NULL) {
                    chooser = op;
                }
                break;
            }
        }
        for (v = 0; v < 32; v++) {
            if ((chooser->variation_choices[v] & 0x0FFFFFFF) != 0) {
                valid[g0][nvalid[g0]++] = v;
            }
        }
        if (nvalid[g0] == 0) {
            valid[g0][nvalid[g0]++] = 0;
        }
        idx[g0] = 0;
    }
    for (;;) {
        int g, ok = 1, n = 0;
        Note exp[1024];

        for (g = 0; g < x->ng; g++) {
            assign[g] = valid[g][idx[g]];
        }
        for (g = 0; g < x->ng && ok; g++) {
            int h;

            for (h = 0; h < g && ok; h++) {
                if (x->group[g]->variation_lock_id != 0 &&
                    x->group[g]->variation_lock_id == x->group[h]->variation_lock_id && assign[g] != assign[h]) {
                    ok = 0;
                }
            }
        }
        if (ok) {
            x->candidates++;
            for (g = 0; g < x->ng; g++) {
                n += Expected(seg, x->gpart[g], x->group[g], assign[g], t0, exp + n, 1024 - n);
            }
            if (Matches(exp, n, x->ch, t0, MEASURE * pttn->length_measures)) {
                if (x->nfound < 8) {
                    for (g = 0; g < x->ng; g++) {
                        sprintf(x->found + strlen(x->found), "%s%d", g ? "+" : (x->nfound ? "," : ""), assign[g]);
                    }
                }
                x->nfound++;
            }
        }
        for (g = 0; g < x->ng; g++) {
            if (++idx[g] < nvalid[g]) {
                break;
            }
            idx[g] = 0;
        }
        if (g == x->ng) {
            break;
        }
    }
    return 1;
}

/* All channels of a pattern in one bar (res has room for 16); returns how many. */
static int CheckPattern(DmSegment *seg, DmStyle *sty, DmPattern *pttn, long t0, Result *res) {
    int n = 0;
    size_t r, q;

    for (r = 0; r < pttn->parts.length && n < 16; r++) {
        for (q = 0; q < r; q++) {
            if (pttn->parts.data[q].logical_part_id == pttn->parts.data[r].logical_part_id) {
                break;
            }
        }
        if (q == r && CheckGroup(seg, sty, pttn, r, t0, &res[n])) {
            n++;
        }
    }
    return n;
}

/* Are there notes in the bar on a channel the pattern has no part for? */
static int ExtraChannels(DmPattern *pttn, long t0) {
    long len = MEASURE * (long)pttn->length_measures;
    int i;
    size_t r;

    for (i = 0; i < rec_count; i++) {
        int known = 0;

        if (rec[i].g < t0 || rec[i].g >= t0 + len) {
            continue;
        }
        for (r = 0; r < pttn->parts.length && !known; r++) {
            known = pttn->parts.data[r].logical_part_id == rec[i].ch;
        }
        if (!known) {
            return 1;
        }
    }
    return 0;
}

int main(int argc, char **argv) {
    DmLoader *loader;
    long bars = 0, parts = 0, missed = 0, shown = 0, ambiguous = 0;
    int i;

    if (argc < 3) {
        printf("usage: check_bars <data folder> <recording>...\n");
        return 2;
    }
    snprintf(music_dir, sizeof(music_dir), "%s\\imusic", argv[1]);
    Dm_setLogger(DmLogLevel_ERROR, Quiet, NULL);
    DmLoader_create(&loader, DmLoader_DEFAULT);
    DmLoader_addResolver(loader, Resolve, NULL);
    {
        /* every style in the folder, as the game's ScanDirectory makes DirectMusic's loader know them: some
         * segments name their styles only by GUID */
        WIN32_FIND_DATAA fd;
        char pattern[1100];
        HANDLE find;

        snprintf(pattern, sizeof(pattern), "%s\\*.sty", music_dir);
        find = FindFirstFileA(pattern, &fd);
        if (find != INVALID_HANDLE_VALUE) {
            do {
                DmReference ref;
                DmStyle *sty = NULL;

                memset(&ref, 0, sizeof(ref));
                ref.file = fd.cFileName;
                ref.guid.data[0] = 0xff; /* matches no cached style: load it */
                if (DmLoader_getStyle(loader, &ref, &sty) == DmResult_SUCCESS) {
                    DmStyle_release(sty); /* the cache keeps it */
                }
            } while (FindNextFileA(find, &fd));
            FindClose(find);
        }
    }
    for (i = 2; i < argc; i++) {
        FILE *f = fopen(argv[i], "r"), *choices;
        char line[512], name[256] = "", cpath[1100];
        DmSegment *seg = NULL;
        long start = -1, b, nbars;
        int groove_offset = 0;

        if (f == NULL) {
            continue;
        }
        rec_count = 0;
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

            if (seg == NULL && strncmp(line, "segment ", 8) == 0 &&
                sscanf(line, "segment %255s groove %d", name, &groove_offset) == 2) {
                if (DmLoader_getSegment(loader, name, &seg) != DmResult_SUCCESS) {
                    printf("FAIL %s: dmusic can't load %s\n", argv[i], name);
                    missed++;
                    break;
                }
            } else if (sscanf(line,
                           "note %ld ch %u midi %u vel %u dur %ld mv %x meas %u beat %u grid %u off %d flags %x pm %x "
                           "sub %x tr %d ranges %u %u %u",
                           &mt, &ch, &midi, &vel, &dur, &mv, &meas, &beat, &grid, &off, &flags, &pm, &sub, &tr,
                           &tr_r, &dur_r, &vel_r) == 17 &&
                start >= 0 && rec_count < MAX_NOTES) {
                Note *n = &rec[rec_count++];

                n->t = mt - start;
                /* the bar of the note's grid position (its time without the offset); DirectMusic doesn't play notes
                 * placed on a pattern's very end, so there is no ambiguity there */
                n->g = (mt - start - off) / MEASURE * MEASURE;
                (void)meas;
                (void)beat;
                (void)grid;
                n->midi = (int)midi;
                n->vel = (int)vel;
                n->dur = dur;
                n->ch = (int)ch;
            }
        }
        fclose(f);
        if (seg == NULL) {
            continue;
        }
        cur_name = name;
        snprintf(cpath, sizeof(cpath), "%s.choices", argv[i]);
        choices = fopen(cpath, "w");
        nbars = (long)(seg->length / MEASURE);
        {
            /* the styles, as a download would resolve them (without the instruments, which aren't needed here) */
            size_t m;

            for (m = 0; m < seg->messages.length; m++) {
                DmMessage *x = &seg->messages.data[m];

                if (x->type == DmMessage_STYLE && x->style.style == NULL && x->time < seg->length &&
                    DmLoader_getStyle(loader, &x->style.reference, &x->style.style) != DmResult_SUCCESS) {
                    char guid[64];

                    DmGuid_toString(&x->style.reference.guid, guid, sizeof(guid));
                    printf("FAIL %s: style %s (file %s, %s) at %u doesn't load\n", name, x->style.reference.name,
                        x->style.reference.file, guid, x->time);
                    missed++;
                }
            }
        }
        if (getenv("DEBUGSEG") != NULL) {
            size_t m;

            printf("%s: length %u, %u messages, %ld bars\n", name, seg->length, (unsigned)seg->messages.length, nbars);
            for (m = 0; m < seg->messages.length; m++) {
                DmMessage *x = &seg->messages.data[m];

                if (x->type == DmMessage_STYLE) {
                    char guid[64] = "";
                    size_t p, q;

                    if (x->style.style != NULL) {
                        DmGuid_toString(&x->style.style->guid, guid, sizeof(guid));
                    }
                    printf("  style at %u: ref %s file %s -> loaded '%s' %s:", x->time, x->style.reference.name,
                        x->style.reference.file, x->style.style ? x->style.style->info.unam : "-", guid);
                    for (p = 0; x->style.style != NULL && p < x->style.style->patterns.length; p++) {
                        DmPattern *pt = &x->style.style->patterns.data[p];

                        printf(" [pattern %s groove %u-%u parts", pt->info.unam, pt->groove_bottom, pt->groove_top);
                        for (q = 0; q < pt->parts.length; q++) {
                            printf(" %u", pt->parts.data[q].logical_part_id);
                        }
                        printf("]");
                    }
                    printf("\n");
                } else if (x->type == DmMessage_COMMAND) {
                    printf("  command at %u: %d groove %u\n", x->time, x->command.command, x->command.groove_level);
                }
            }
        }
        long pattern_end = 0;

        for (b = 0; b < nbars; b++) {
            long t0 = b * MEASURE;
            DmMessage *sm = LatestAt(seg, DmMessage_STYLE, t0);
            DmMessage *cm = LatestAt(seg, DmMessage_COMMAND, t0);
            DmStyle *sty = sm != NULL ? sm->style.style : NULL;
            int groove = (cm != NULL ? cm->command.groove_level : 1) + groove_offset;
            size_t p;

            if (sty == NULL || t0 < pattern_end) {
                continue; /* no style, or the pattern picked earlier is still playing */
            }
            groove = groove < 1 ? 1 : groove > 100 ? 100
                                                   : groove;
            {
                /* DirectMusic picks one of the style's patterns for the groove level: the bar must be one of them,
                 * every channel of it one of its variations, and the channels it doesn't use silent */
                Result res[16][16];
                int nres[16], good[16], ngood = 0, best = -1, best_ok = -1, np = 0;
                DmPattern *cand[16];

                /* the patterns whose groove range holds the level or, when none does, comes closest to it */
                int nearest = 1000;

                for (p = 0; p < sty->patterns.length; p++) {
                    DmPattern *pttn = &sty->patterns.data[p];
                    int d = groove < pttn->groove_bottom ? pttn->groove_bottom - groove
                        : groove > pttn->groove_top      ? groove - pttn->groove_top
                                                         : 0;

                    if (pttn->embellishment == 0 && d < nearest) {
                        nearest = d;
                    }
                }
                for (p = 0; p < sty->patterns.length && np < 16; p++) {
                    DmPattern *pttn = &sty->patterns.data[p];
                    int ok = 0, c;
                    int d = groove < pttn->groove_bottom ? pttn->groove_bottom - groove
                        : groove > pttn->groove_top      ? groove - pttn->groove_top
                                                         : 0;

                    if (d != nearest || pttn->embellishment != 0) {
                        continue;
                    }
                    cand[np] = pttn;
                    nres[np] = CheckPattern(seg, sty, pttn, t0, res[np]);
                    for (c = 0; c < nres[np]; c++) {
                        ok += res[np][c].nfound > 0;
                    }
                    good[np] = ok == nres[np] && !ExtraChannels(pttn, t0);
                    ngood += good[np];
                    if (good[np] && best_ok != 1000) {
                        best = np;
                        best_ok = 1000;
                    } else if (ok > best_ok) {
                        best = np;
                        best_ok = ok;
                    }
                    np++;
                }
                if (np == 0) {
                    continue;
                }
                bars++;
                if (ngood > 1) {
                    ambiguous++;
                }
                if (best >= 0) {
                    int c;

                    pattern_end = t0 + MEASURE * (long)cand[best]->length_measures;

                    for (c = 0; c < nres[best]; c++) {
                        Result *x = &res[best][c];

                        parts++;
                        if (choices != NULL) {
                            fprintf(choices, "%s bar %ld style %s pattern %s of %d part %d parts %d lock %u mode %d "
                                             "choice %s of %d\n",
                                name, b, sty->info.unam, cand[best]->info.unam, np, x->ch, x->ng, x->lock, x->mode,
                                x->nfound ? x->found : "none", x->candidates);
                        }
                        if (x->nfound == 0) {
                            missed++;
                            if (shown++ < (getenv("MAXSHOW") ? atoi(getenv("MAXSHOW")) : 20)) {
                                printf("  %s bar %ld style %s pattern %s (of %d, %u bars): channel %d (%d parts) matches no "
                                       "variation\n",
                                    name, b, sty->info.unam, cand[best]->info.unam, np, cand[best]->length_measures, x->ch, x->ng);
                            }
                            Detail(name, b, x->ch, seg, x->gpart, x->group, x->ng, t0);
                        }
                    }
                    if (good[best] == 0 && ExtraChannels(cand[best], t0)) {
                        missed++;
                        if (shown++ < (getenv("MAXSHOW") ? atoi(getenv("MAXSHOW")) : 20)) {
                            printf("  %s bar %ld style %s pattern %s: notes on a channel the pattern doesn't use\n",
                                name, b, sty->info.unam, cand[best]->info.unam);
                        }
                    }
                }
            }
        }
        if (choices != NULL) {
            fclose(choices);
        }
        DmSegment_release(seg);
    }
    printf("%ld bars, %ld parts: %ld matched no variation, %ld matched several\n", bars, parts, missed, ambiguous);
    return missed != 0;
}
