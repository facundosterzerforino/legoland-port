/* [library:music] miniwin's style engine: see miniwin/dmengine.h. The rules it follows, and the recordings of
 * Windows' DirectMusic they were checked against, are in tools/music/README.md ("the model"); the numbers below refer
 * to it. dmusic (port/dmusic) parses the files and synthesizes the sound. */
#include <limits.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "_Internal.h"
#include "dmreverb.h"
#include "miniwin/dmengine.h"
#ifdef DME_FLUIDSYNTH
#include <fluidsynth.h>
#endif

#define BEAT 768 /* ticks a quarter note */
#define LOOKAHEAD BEAT /* patterns are composed this far ahead: notes may start early (negative offsets) */
#define ALMOST_END_SECONDS 0.906 /* "almost end" comes this long before the end (measured: 696 ticks at 60 bpm) */
#define CURVE_STEP 12 /* ticks between the steps of a controller curve */
#define MAX_SEQ 512
/* The synthesizer's overall level, to play as loud as Windows' DirectMusic does into the same buffer: measured with
 * tools/test_miniwin/capture_dmusic.exe and tools/music/levels.py on the main theme and the four world themes (40 s
 * each, the game's setup): all within 0.6 dB of Windows', and every octave band within 1 dB. */
#define SYNTH_GAIN 0.95f
/* FluidSynth's (synth.gain), measured the same way: the five themes within 0.8 dB of Windows' DirectMusic */
#define FLUID_GAIN 0.8

enum {
    EV_NOTE_OFF,
    EV_CONTROL,
    EV_BEND,
    EV_NOTE_ON
}; /* the order at the same time */

typedef struct Event {
    long time;
    int type, ch, note, vel;
    float value;
    unsigned long order;
} Event;

/* a part's state for the sequential variation modes */
typedef struct SeqState {
    const DmPart *part;
    int last;
    uint32_t played; /* RANDOM_ROW: played since the row began */
} SeqState;

struct DmeEngine {
    mtx_t lock;
    DmLoader *loader;
    DmSynth synth;
    DmeReverb *reverb; /* DirectMusic's synthesizer adds one by default */
#ifdef DME_FLUIDSYNTH
    /* FluidSynth on gm.dls (its own DLS loader, Microsoft's rules) instead of dmusic's synthesizer: Dme_useFluidSynth */
    fluid_settings_t *fluid_settings;
    fluid_synth_t *fluid;
    int fluid_font;
    int fluid_transpose[64];
#endif
    unsigned int rate;
    double now; /* ticks */
    double tempo;
    int master_groove;
    uint32_t rng;

    /* the primary segment */
    DmSegment *seg;
    DmMessage *msgs; /* its messages by time */
    size_t nmsgs, next_msg;
    long seg_start, seg_end;
    unsigned int repeats_left;
    long pattern_end; /* when the next pattern is picked */
    long next_beat, beat_index;
    int almost_sent;

    /* the segment waiting for its boundary */
    DmSegment *queued;
    long queued_start;
    unsigned int queued_repeats;

    Event *ev;
    size_t nev, evcap;
    unsigned long order;

    DmeNotification *nq;
    size_t nn, ncap;
    unsigned int types;
    int notified;
    void (*callback)(void *ctx);
    void *callback_ctx;

    SeqState seq[MAX_SEQ];
    int nseq;
    FILE *log;
};

/* ---- small helpers ---- */

static uint32_t Rand(DmeEngine *e) {
    /* xorshift32 */
    e->rng ^= e->rng << 13;
    e->rng ^= e->rng >> 17;
    e->rng ^= e->rng << 5;
    return e->rng;
}

/* a random value in [-range/2, range/2] */
static int Spread(DmeEngine *e, int range) {
    return range <= 0 ? 0 : (int)(Rand(e) % (uint32_t)(range + 1)) - range / 2;
}

static long MsgTime(const DmMessage *m) {
    return m->time == 0xffffffff ? 0 : (long)m->time;
}

static int ByTime(const void *a, const void *b) {
    long x = MsgTime((const DmMessage *)a), y = MsgTime((const DmMessage *)b);

    return x < y ? -1 : x > y;
}

static void Notify(DmeEngine *e, int type, unsigned int option, unsigned int field1, unsigned int field2, long time) {
    if (e->log != NULL) {
        static const char *const kinds[] = {"start", "end", "almostend", "loop", "abort"};

        if (type == DME_SEGMENT) {
            fprintf(e->log, "segment-%s %ld\n", option < 5 ? kinds[option] : "?", time);
        } else {
            fprintf(e->log, "beat %ld %u %u\n", time, field1, field2);
        }
    }
    if (!(e->types & (1u << type))) {
        return;
    }
    if (e->nn == e->ncap) {
        size_t cap = e->ncap ? e->ncap * 2 : 32;
        DmeNotification *grown = (DmeNotification *)realloc(e->nq, cap * sizeof(*grown));

        if (grown == NULL) {
            return;
        }
        e->nq = grown;
        e->ncap = cap;
    }
    e->nq[e->nn].type = type;
    e->nq[e->nn].option = option;
    e->nq[e->nn].field1 = field1;
    e->nq[e->nn].field2 = field2;
    e->nq[e->nn].time = time;
    e->nn++;
    e->notified = 1;
}

static int EventBefore(const Event *a, const Event *b) {
    if (a->time != b->time) {
        return a->time < b->time;
    }
    if (a->type != b->type) {
        return a->type < b->type;
    }
    return a->order < b->order;
}

static void Push(DmeEngine *e, long time, int type, int ch, int note, int vel, float value) {
    Event ev;
    size_t lo = 0, hi;

    if (e->nev == e->evcap) {
        size_t cap = e->evcap ? e->evcap * 2 : 1024;
        Event *grown = (Event *)realloc(e->ev, cap * sizeof(*grown));

        if (grown == NULL) {
            return;
        }
        e->ev = grown;
        e->evcap = cap;
    }
    ev.time = time;
    ev.type = type;
    ev.ch = ch;
    ev.note = note;
    ev.vel = vel;
    ev.value = value;
    ev.order = e->order++;
    hi = e->nev;
    while (lo < hi) {
        size_t mid = (lo + hi) / 2;

        if (EventBefore(&e->ev[mid], &ev)) {
            lo = mid + 1;
        } else {
            hi = mid;
        }
    }
    memmove(&e->ev[lo + 1], &e->ev[lo], (e->nev - lo) * sizeof(Event));
    e->ev[lo] = ev;
    e->nev++;
}

/* Drops what hasn't started by time: a new primary segment cuts the old one's notes (those sounding end as
 * written). */
static void Invalidate(DmeEngine *e, long time) {
    size_t i, j = 0;

    for (i = 0; i < e->nev; i++) {
        if (e->ev[i].time >= time && e->ev[i].type != EV_NOTE_OFF) {
            continue;
        }
        e->ev[j++] = e->ev[i];
    }
    e->nev = j;
}

/* ---- the segment's tracks ---- */

static const DmMessage *LatestAt(const DmeEngine *e, DmMessageType type, long rel) {
    const DmMessage *best = NULL;
    size_t i;

    for (i = 0; i < e->nmsgs && MsgTime(&e->msgs[i]) <= rel; i++) {
        if (e->msgs[i].type == type && (type != DmMessage_STYLE || e->msgs[i].style.style != NULL)) {
            best = &e->msgs[i];
        }
    }
    return best;
}

/* (1) the chord in force: DirectMusic's default without a chord track */
static struct DmSubChord ChordAt(const DmeEngine *e, long rel, int level) {
    const DmMessage *m = LatestAt(e, DmMessage_CHORD, rel);
    struct DmSubChord c = {0x91, 0xab5ab5, 0, 0xffffffff, 12, 0};
    uint32_t k;

    if (m == NULL || m->chord.subchord_count == 0) {
        return c;
    }
    for (k = 0; k < m->chord.subchord_count; k++) {
        if (m->chord.subchords[k].levels & (1u << level)) {
            return m->chord.subchords[k];
        }
    }
    return m->chord.subchords[0];
}

static DmTimeSignature StyleSignature(const DmeEngine *e, long rel) {
    const DmMessage *m = LatestAt(e, DmMessage_STYLE, rel);
    DmTimeSignature four = {4, 4, 8};

    return m != NULL ? m->style.style->time_signature : four;
}

/* ---- composing ---- */

static int Enabled(const DmPart *part, int v) {
    return (part->variation_choices[v] & 0x0FFFFFFF) != 0;
}

static SeqState *Seq(DmeEngine *e, const DmPart *part) {
    int i;

    for (i = 0; i < e->nseq; i++) {
        if (e->seq[i].part == part) {
            return &e->seq[i];
        }
    }
    if (e->nseq == MAX_SEQ) {
        e->nseq = 0;
    }
    e->seq[e->nseq].part = part;
    e->seq[e->nseq].last = -1;
    e->seq[e->nseq].played = 0;
    return &e->seq[e->nseq++];
}

/* (3, 7) a part's variation, by the part reference's mode (DMUS_VARIATIONT_*) */
static int ChooseVariation(DmeEngine *e, const DmPart *part, const DmPartReference *pref) {
    int enabled[32], n = 0, v, pick;
    SeqState *s = Seq(e, part);

    for (v = 0; v < 32; v++) {
        if (Enabled(part, v)) {
            enabled[n++] = v;
        }
    }
    if (n == 0) {
        return 0;
    }
    switch ((int)pref->random_variation) {
    case 0: /* sequential */
    case 2: /* random start, then sequential */
        if (s->last < 0) {
            pick = pref->random_variation == 2 ? (int)(Rand(e) % (uint32_t)n) : 0;
        } else {
            for (pick = 0; pick < n && enabled[pick] <= s->last; pick++) {
            }
            pick %= n;
        }
        break;
    case 3: /* no repeat */
        do {
            pick = (int)(Rand(e) % (uint32_t)n);
        } while (n > 1 && enabled[pick] == s->last);
        break;
    case 4: { /* random row: all once before any again */
        int left[32], nl = 0, i;

        for (i = 0; i < n; i++) {
            if (!(s->played & (1u << enabled[i]))) {
                left[nl++] = i;
            }
        }
        if (nl == 0) {
            s->played = 0;
            for (i = 0; i < n; i++) {
                left[nl++] = i;
            }
        }
        pick = left[Rand(e) % (uint32_t)nl];
        break;
    }
    default: /* random */
        pick = (int)(Rand(e) % (uint32_t)n);
        break;
    }
    s->last = enabled[pick];
    s->played |= 1u << enabled[pick];
    return enabled[pick];
}

static float CurveAt(const DmCurve *c, float phase) {
    float a = c->start_value, b = c->end_value;

    switch (c->curve_shape) {
    case DmCurveShape_INSTANT:
        return b;
    case DmCurveShape_EXP:
        phase = powf(phase, 4);
        break;
    case DmCurveShape_LOG:
        phase = sqrtf(phase);
        break;
    case DmCurveShape_SINE:
        phase = (sinf((phase - 0.5f) * 3.14159265f) + 1) * 0.5f;
        break;
    default:
        break;
    }
    return a + (b - a) * phase;
}

static void Curve(DmeEngine *e, const DmCurve *c, long start, int ch) {
    int type = c->event_type == DmCurveType_PITCH_BEND ? EV_BEND : EV_CONTROL;
    long k;
    float prev = -100000.0f;

    if (c->event_type != DmCurveType_PITCH_BEND && c->event_type != DmCurveType_CONTROL_CHANGE) {
        return; /* aftertouch: the synthesizer has none */
    }
    if (type == EV_CONTROL && (c->start_value < 0 || c->start_value > 127 || c->end_value < 0 || c->end_value > 127)) {
        return;
    }
    for (k = 0; k <= (long)c->duration; k += CURVE_STEP) {
        float v = c->duration ? CurveAt(c, (float)k / (float)c->duration) : (float)c->end_value;

        if (k + CURVE_STEP > (long)c->duration) {
            v = c->end_value;
        }
        if ((int)v != (int)prev) {
            Push(e, start + k, type, ch, c->cc_data, 0, type == EV_CONTROL ? v / 127.0f : v);
            prev = v;
        }
    }
    if (c->flags & DmCurveFlags_RESET) {
        float r = c->reset_value;

        Push(e, start + (long)c->duration + (long)c->reset_duration, type, ch, c->cc_data, 0,
            type == EV_CONTROL ? r / 127.0f : r);
    }
    if (e->log != NULL) {
        fprintf(e->log, "curve %ld ch %d type %u cc %u shape %u start %d end %d dur %u reset %d/%u flags %x\n", start,
            ch, c->event_type, c->cc_data, c->curve_shape, c->start_value, c->end_value, c->duration, c->reset_value,
            c->reset_duration, c->flags);
    }
}

static int GrooveDistance(const DmPattern *p, int groove) {
    return groove < p->groove_bottom ? p->groove_bottom - groove : groove > p->groove_top ? groove - p->groove_top
                                                                                          : 0;
}

/* (2-5) the pattern for the bar starting at t, its variations and its notes */
static void PickPattern(DmeEngine *e, long t) {
    long rel = t - e->seg_start;
    const DmMessage *sm = LatestAt(e, DmMessage_STYLE, rel), *cm = LatestAt(e, DmMessage_COMMAND, rel);
    DmStyle *sty = sm != NULL ? sm->style.style : NULL;
    int groove = (cm != NULL ? cm->command.groove_level : 1) + e->master_groove;
    DmPattern *cand[64], *p;
    int ncand = 0, lockv[256], nearest = 1000;
    size_t i, r;

    e->pattern_end = t + (long)Dm_getMeasureLength(StyleSignature(e, rel));
    if (sty == NULL) {
        return;
    }
    groove = groove < 1 ? 1 : groove > 100 ? 100
                                           : groove;
    /* (2) the patterns whose groove range holds the level or, when none does, comes closest to it (the
     * transitions have only level 1 patterns, and play them at master groove +1 too) */
    for (i = 0; i < sty->patterns.length; i++) {
        int d = GrooveDistance(&sty->patterns.data[i], groove);

        if (sty->patterns.data[i].embellishment == 0 && d < nearest) {
            nearest = d;
        }
    }
    for (i = 0; i < sty->patterns.length && ncand < 64; i++) {
        p = &sty->patterns.data[i];
        if (p->embellishment == 0 && GrooveDistance(p, groove) == nearest) {
            cand[ncand++] = p;
        }
    }
    if (ncand == 0) {
        return;
    }
    p = cand[Rand(e) % (uint32_t)ncand];
    e->pattern_end = t + (long)Dm_getMeasureLength(p->time_signature) * (p->length_measures ? p->length_measures : 1);
    memset(lockv, -1, sizeof(lockv));
    for (r = 0; r < p->parts.length; r++) {
        DmPartReference *pref = &p->parts.data[r];
        DmPart *part = DmStyle_findPart(sty, pref);
        int v, ch = pref->logical_part_id;
        long part_len;
        uint32_t k;

        if (part == NULL) {
            continue;
        }
        if (pref->variation_lock_id != 0 && lockv[pref->variation_lock_id] >= 0) {
            v = lockv[pref->variation_lock_id];
        } else {
            v = ChooseVariation(e, part, pref);
            if (pref->variation_lock_id != 0) {
                lockv[pref->variation_lock_id] = v;
            }
        }
        part_len = (long)Dm_getMeasureLength(part->time_signature) *
            (part->length_measures ? part->length_measures : (p->length_measures ? p->length_measures : 1));
        for (k = 0; k < part->note_count; k++) {
            const DmNote *note = &part->notes[k];
            DmPlayModeFlags pm = note->play_mode_flags == DmPlayMode_NONE ? part->play_mode_flags
                                                                          : note->play_mode_flags;
            long grid_t, at;
            int midi, vel, dur;

            if (!(note->variation & (1u << v))) {
                continue;
            }
            grid_t = (long)Dm_getTimeOffset(note->grid_start, 0, part->time_signature);
            if (grid_t >= part_len) {
                continue; /* (5) not at or past the part's end */
            }
            midi = pm == DmPlayMode_FIXED ? note->music_value
                                          : DmPerformance_musicValueToMidi(
                                                ChordAt(e, rel + grid_t, pref->subchord_level), pm, note->music_value);
            if (midi < 0 || midi > 127) {
                continue;
            }
            at = t + grid_t + note->time_offset + Spread(e, (int)note->time_range);
            dur = (int)note->duration + Spread(e, (int)note->duration_range);
            vel = note->velocity + Spread(e, note->velocity_range);
            dur = dur < 1 ? 1 : dur;
            vel = vel < 1 ? 1 : vel > 127 ? 127
                                          : vel;
            if (at < (long)ceil(e->now)) {
                at = (long)ceil(e->now);
            }
            Push(e, at, EV_NOTE_ON, ch, midi, vel, 0);
            Push(e, at + dur, EV_NOTE_OFF, ch, midi, 0, 0);
            if (e->log != NULL) {
                fprintf(e->log,
                    "note %ld ch %d midi %d vel %d dur %d mv %04x meas 0 beat 0 grid 0 off %d flags 0 pm %x sub %u tr 0 "
                    "ranges %u %u %u\n",
                    at, ch, midi, vel, dur, note->music_value, (int)(at - (t + grid_t)), pm, pref->subchord_level,
                    note->time_range, note->duration_range, note->velocity_range);
            }
        }
        for (k = 0; k < part->curve_count; k++) {
            const DmCurve *c = &part->curves[k];

            if (c->variation & (1u << v)) {
                Curve(e, c, t + (long)Dm_getTimeOffset(c->grid_start, c->time_offset, part->time_signature), ch);
            }
        }
    }
}

/* ---- the primary segment ---- */

static void Start(DmeEngine *e, DmSegment *seg, long t, unsigned int repeats) {
    DmSegment_release(e->seg);
    e->seg = seg; /* the queue's reference */
    free(e->msgs);
    e->nmsgs = seg->messages.length;
    e->msgs = (DmMessage *)malloc((e->nmsgs ? e->nmsgs : 1) * sizeof(DmMessage));
    if (e->msgs != NULL) {
        memcpy(e->msgs, seg->messages.data, e->nmsgs * sizeof(DmMessage));
        qsort(e->msgs, e->nmsgs, sizeof(DmMessage), ByTime);
    } else {
        e->nmsgs = 0;
    }
    e->next_msg = 0;
    e->seg_start = t;
    e->seg_end = t + (long)seg->length;
    e->repeats_left = repeats;
    e->pattern_end = t;
    e->next_beat = t;
    e->beat_index = 0;
    e->almost_sent = 0;
    Notify(e, DME_SEGMENT, DME_SEG_START, 0, 0, t);
}

#ifdef DME_FLUIDSYNTH
static void FluidBand(DmeEngine *e, const DmBand *band) {
    size_t i;

    for (i = 0; i < band->instruments_len; i++) {
        const DmInstrument *ins = &band->instruments[i];
        int ch = (int)ins->channel;

        if (ch >= 64) {
            continue;
        }
        if (ins->options & DmInstrument_VALID_PATCH) {
            int drum = (ins->patch & DmDls_DRUM_KIT) != 0;
            int msb = (int)(ins->patch >> 16) & 0x7f, program = (int)ins->patch & 0x7f;

            fluid_synth_set_channel_type(e->fluid, ch, drum ? CHANNEL_TYPE_DRUM : CHANNEL_TYPE_MELODIC);
            if (fluid_synth_program_select(e->fluid, ch, e->fluid_font, drum ? 128 : msb, program) != FLUID_OK) {
                fluid_synth_program_select(e->fluid, ch, e->fluid_font, drum ? 128 : 0, drum ? 0 : program);
            }
        }
        fluid_synth_cc(e->fluid, ch, 11, 127);
        if (ins->options & DmInstrument_VALID_VOLUME) {
            fluid_synth_cc(e->fluid, ch, 7, (int)ins->volume);
        }
        if (ins->options & DmInstrument_VALID_PAN) {
            fluid_synth_cc(e->fluid, ch, 10, (int)ins->pan);
        }
        if (ins->options & DmInstrument_VALID_TRANSPOSE) {
            e->fluid_transpose[ch] = ins->transpose;
        }
    }
}
#endif

static void Message(DmeEngine *e, const DmMessage *m) {
    if (m->type == DmMessage_TEMPO) {
        e->tempo = m->tempo.tempo > 0 ? m->tempo.tempo : e->tempo;
        if (e->log != NULL) {
            fprintf(e->log, "tempo %ld %.4f\n", e->seg_start + MsgTime(m), e->tempo);
        }
    } else if (m->type == DmMessage_BAND && m->band.band != NULL) {
#ifdef DME_FLUIDSYNTH
        if (e->fluid != NULL) {
            FluidBand(e, m->band.band);
        } else
#endif
            DmSynth_sendBandUpdate(&e->synth, m->band.band);
        if (e->log != NULL) {
            size_t i;

            for (i = 0; i < m->band.band->instruments_len; i++) {
                const DmInstrument *ins = &m->band.band->instruments[i];

                fprintf(e->log, "patch %ld ch %u program %u msb %u lsb %u\n", e->seg_start + MsgTime(m),
                    ins->channel, ins->patch & 0x7f, (ins->patch >> 16) & 0x7f, (ins->patch >> 8) & 0x7f);
            }
        }
    }
}

static void Apply(DmeEngine *e, const Event *ev) {
#ifdef DME_FLUIDSYNTH
    if (e->fluid != NULL) {
        int ch = ev->ch & 63, key = ev->note + e->fluid_transpose[ev->ch & 63];

        switch (ev->type) {
        case EV_NOTE_ON:
            if (key >= 0 && key < 128) {
                fluid_synth_noteon(e->fluid, ch, key, ev->vel);
            }
            break;
        case EV_NOTE_OFF:
            if (key >= 0 && key < 128) {
                fluid_synth_noteoff(e->fluid, ch, key);
            }
            break;
        case EV_CONTROL:
            fluid_synth_cc(e->fluid, ch, ev->note, (int)(ev->value * 127.0f + 0.5f));
            break;
        case EV_BEND:
            fluid_synth_pitch_bend(e->fluid, ch, (int)ev->value);
            break;
        }
        return;
    }
#endif
    switch (ev->type) {
    case EV_NOTE_ON:
        DmSynth_sendNoteOn(&e->synth, (uint32_t)ev->ch, (uint8_t)ev->note, (uint8_t)ev->vel);
        break;
    case EV_NOTE_OFF:
        DmSynth_sendNoteOff(&e->synth, (uint32_t)ev->ch, (uint8_t)ev->note);
        break;
    case EV_CONTROL:
        DmSynth_sendControl(&e->synth, (uint32_t)ev->ch, (uint8_t)ev->note, ev->value);
        break;
    case EV_BEND:
        DmSynth_sendPitchBend(&e->synth, (uint32_t)ev->ch, (int)ev->value);
        break;
    }
}

/* Does everything due at or before time t, in time order. */
static void Process(DmeEngine *e, long t) {
    for (;;) {
        long best = LONG_MAX, almost = LONG_MAX;
        int what = 0;
        long per_beat = BEAT;

        /* at the same time: the old segment's end, the new one's start, its messages, "almost end", the beat, the
         * next pattern, then the notes */
        if (e->seg != NULL && e->seg_end <= t && e->seg_end < best) {
            best = e->seg_end, what = 1;
        }
        if (e->queued != NULL && e->queued_start <= t && e->queued_start < best) {
            best = e->queued_start, what = 2;
        }
        if (e->seg != NULL && e->next_msg < e->nmsgs) {
            long mt = e->seg_start + MsgTime(&e->msgs[e->next_msg]);

            if (mt <= t && mt < best) {
                best = mt, what = 3;
            }
        }
        if (e->seg != NULL && !e->almost_sent && e->repeats_left == 0) {
            almost = e->seg_end - (long)(ALMOST_END_SECONDS * e->tempo / 60.0 * BEAT);
            if (almost <= t && almost < best) {
                best = almost, what = 4;
            }
        }
        if (e->seg != NULL && e->next_beat < e->seg_end && e->next_beat <= t && e->next_beat < best) {
            best = e->next_beat, what = 5;
        }
        if (e->seg != NULL && e->pattern_end < e->seg_end && e->pattern_end - LOOKAHEAD <= t &&
            e->pattern_end - LOOKAHEAD < best &&
            !(e->queued != NULL && e->queued_start <= e->pattern_end)) {
            best = e->pattern_end - LOOKAHEAD, what = 6;
        }
        if (e->nev > 0 && e->ev[0].time <= t && e->ev[0].time < best) {
            best = e->ev[0].time, what = 7;
        }
        switch (what) {
        case 0:
            return;
        case 1: /* the end, or a repeat */
            if (e->repeats_left > 0) {
                long len = e->seg_end - e->seg_start;

                e->repeats_left--;
                e->seg_start = e->seg_end;
                e->seg_end += len;
                e->next_msg = 0;
                e->pattern_end = e->seg_start;
                e->next_beat = e->seg_start;
                e->beat_index = 0;
                Notify(e, DME_SEGMENT, DME_SEG_LOOP, 0, 0, e->seg_start);
            } else {
                Notify(e, DME_SEGMENT, DME_SEG_END, 0, 0, e->seg_end);
                DmSegment_release(e->seg);
                e->seg = NULL;
            }
            break;
        case 2: /* a new primary segment */
            if (e->seg != NULL) {
                Notify(e, DME_SEGMENT, DME_SEG_ABORT, 0, 0, e->queued_start);
            }
            Invalidate(e, e->queued_start);
            Start(e, e->queued, e->queued_start, e->queued_repeats);
            e->queued = NULL;
            break;
        case 3:
            Message(e, &e->msgs[e->next_msg++]);
            break;
        case 4:
            e->almost_sent = 1;
            Notify(e, DME_SEGMENT, DME_SEG_ALMOST_END, 0, 0, almost);
            break;
        case 5: {
            DmTimeSignature sig = StyleSignature(e, e->next_beat - e->seg_start);
            int beats = sig.beats_per_measure ? sig.beats_per_measure : 4;

            per_beat = (long)Dm_getBeatLength(sig);
            Notify(e, DME_MEASURE_AND_BEAT, 0, (unsigned int)(e->beat_index % beats),
                (unsigned int)(e->beat_index / beats), e->next_beat);
            e->beat_index++;
            e->next_beat += per_beat;
            break;
        }
        case 6:
            PickPattern(e, e->pattern_end);
            break;
        case 7: {
            Event ev = e->ev[0];

            memmove(&e->ev[0], &e->ev[1], (e->nev - 1) * sizeof(Event));
            e->nev--;
            Apply(e, &ev);
            break;
        }
        }
    }
}

/* The next time something is due after the present. */
static long NextDue(const DmeEngine *e) {
    long n = LONG_MAX;

    if (e->seg != NULL) {
        n = e->seg_end < n ? e->seg_end : n;
        n = e->next_beat < n ? e->next_beat : n;
        n = e->pattern_end - LOOKAHEAD < n ? e->pattern_end - LOOKAHEAD : n;
        if (e->next_msg < e->nmsgs && e->seg_start + MsgTime(&e->msgs[e->next_msg]) < n) {
            n = e->seg_start + MsgTime(&e->msgs[e->next_msg]);
        }
        if (!e->almost_sent && e->repeats_left == 0) {
            long a = e->seg_end - (long)(ALMOST_END_SECONDS * e->tempo / 60.0 * BEAT);

            n = a < n ? a : n;
        }
    }
    if (e->queued != NULL && e->queued_start < n) {
        n = e->queued_start;
    }
    if (e->nev > 0 && e->ev[0].time < n) {
        n = e->ev[0].time;
    }
    return n;
}

/* ---- the interface ---- */

DmeEngine *Dme_create(DmLoader *loader, unsigned int rate) {
    DmeEngine *e = (DmeEngine *)calloc(1, sizeof(*e));

    if (e == NULL) {
        return NULL;
    }
    if (mtx_init(&e->lock, mtx_plain) != thrd_success) {
        free(e);
        return NULL;
    }
    e->loader = DmLoader_retain(loader);
    e->rate = rate ? rate : 22050;
    e->tempo = 120.0;
    {
        static uint32_t engines; /* engines made in the same second still differ */

        e->rng = ((uint32_t)time(NULL) ^ (uint32_t)clock() * 40503u ^ ++engines * 2246822519u) * 2654435761u | 1u;
        Rand(e);
    }
    DmSynth_init(&e->synth, e->rate);
    DmSynth_setVolume(&e->synth, SYNTH_GAIN);
    e->reverb = DmeReverb_create(e->rate);
    return e;
}

int Dme_useFluidSynth(DmeEngine *e, const char *gm_dls) {
#ifdef DME_FLUIDSYNTH
    fluid_synth_t *fluid;
    int font;

    if (e == NULL || gm_dls == NULL) {
        return 0;
    }
    if (e->fluid_settings == NULL) {
        e->fluid_settings = new_fluid_settings();
        fluid_settings_setnum(e->fluid_settings, "synth.sample-rate", e->rate);
        fluid_settings_setnum(e->fluid_settings, "synth.gain", FLUID_GAIN);
        /* the engine adds DirectMusic's reverb (dmreverb.c); gm.dls sets no reverb or chorus send anyway */
        fluid_settings_setint(e->fluid_settings, "synth.reverb.active", 0);
        fluid_settings_setint(e->fluid_settings, "synth.chorus.active", 0);
        fluid_settings_setint(e->fluid_settings, "synth.midi-channels", 64);
        fluid_settings_setint(e->fluid_settings, "synth.polyphony", 256);
    }
    fluid = new_fluid_synth(e->fluid_settings);
    font = fluid != NULL ? fluid_synth_sfload(fluid, gm_dls, 0) : -1;
    if (font < 0) {
        delete_fluid_synth(fluid);
        return 0;
    }
    mtx_lock(&e->lock);
    e->fluid = fluid;
    e->fluid_font = font;
    mtx_unlock(&e->lock);
    return 1;
#else
    (void)e;
    (void)gm_dls;
    return 0;
#endif
}

/* milliseconds, for the selftest's timings (Android's C library has no timespec_get) */
static double SelfTestMs(void) {
    struct timespec ts;

#ifdef _WIN32
    timespec_get(&ts, TIME_UTC);
#else
    clock_gettime(CLOCK_MONOTONIC, &ts);
#endif
    return ts.tv_sec * 1000.0 + ts.tv_nsec / 1000000.0;
}

double Dme_timeFluidSynthLoad(const char *gm_dls) {
#ifdef DME_FLUIDSYNTH
    double t0;
    double t1;
    fluid_settings_t *settings = new_fluid_settings();
    fluid_synth_t *fluid;
    int font;

    t0 = SelfTestMs();
    fluid = settings != NULL ? new_fluid_synth(settings) : NULL;
    font = fluid != NULL ? fluid_synth_sfload(fluid, gm_dls, 0) : -1;
    t1 = SelfTestMs();
    if (fluid != NULL) {
        delete_fluid_synth(fluid);
    }
    if (settings != NULL) {
        delete_fluid_settings(settings);
    }
    return font < 0 ? -1.0 : t1 - t0;
#else
    (void)gm_dls;
    return -1.0;
#endif
}

void Dme_destroy(DmeEngine *e) {
    if (e == NULL) {
        return;
    }
    DmSegment_release(e->seg);
    DmSegment_release(e->queued);
    DmSynth_free(&e->synth);
    DmeReverb_destroy(e->reverb);
#ifdef DME_FLUIDSYNTH
    if (e->fluid != NULL) {
        delete_fluid_synth(e->fluid);
    }
    if (e->fluid_settings != NULL) {
        delete_fluid_settings(e->fluid_settings);
    }
#endif
    DmLoader_release(e->loader);
    free(e->msgs);
    free(e->ev);
    free(e->nq);
    mtx_destroy(&e->lock);
    free(e);
}

int Dme_download(DmeEngine *e, DmSegment *seg) {
    size_t i;
    int ok = 1;

    if (e == NULL || seg == NULL) {
        return 0;
    }
    if (seg->downloaded) {
        return 1;
    }
    for (i = 0; i < seg->messages.length; i++) {
        DmMessage *m = &seg->messages.data[i];

        if (m->type == DmMessage_BAND && m->band.band != NULL) {
            ok &= DmBand_download(m->band.band, e->loader) == DmResult_SUCCESS;
        } else if (m->type == DmMessage_STYLE && MsgTime(m) < (long)seg->length) {
            /* (6) a style named by GUID only is found among the loaded ones; empty references past the end
             * (left over in some segments) are skipped */
            if (m->style.style == NULL &&
                DmLoader_getStyle(e->loader, &m->style.reference, &m->style.style) != DmResult_SUCCESS) {
                ok = 0;
            }
        }
    }
    seg->downloaded = true;
    return ok;
}

void Dme_play(DmeEngine *e, DmSegment *seg, unsigned int repeats, int timing) {
    long now, start;

    if (e == NULL || seg == NULL) {
        return;
    }
    Dme_download(e, seg); /* the bands' instruments: DirectMusic plays undownloaded ones with the port's */
    mtx_lock(&e->lock);
    now = (long)ceil(e->now);
    start = now;
    if (e->seg != NULL && timing != DME_AT_ONCE) {
        DmTimeSignature sig = StyleSignature(e, now - e->seg_start);
        long unit = timing == DME_AT_MEASURE ? (long)Dm_getMeasureLength(sig)
            : timing == DME_AT_BEAT          ? (long)Dm_getBeatLength(sig)
                                             : (long)Dm_getBeatLength(sig) / (sig.grids_per_beat ? sig.grids_per_beat : 1);

        unit = unit > 0 ? unit : 1;
        start = e->seg_start + ((now - e->seg_start) / unit + 1) * unit;
    }
    DmSegment_release(e->queued);
    e->queued = DmSegment_retain(seg);
    e->queued_start = start;
    e->queued_repeats = repeats;
    mtx_unlock(&e->lock);
}

void Dme_stop(DmeEngine *e) {
    if (e == NULL) {
        return;
    }
    mtx_lock(&e->lock);
    if (e->seg != NULL) {
        Notify(e, DME_SEGMENT, DME_SEG_ABORT, 0, 0, (long)ceil(e->now));
        DmSegment_release(e->seg);
        e->seg = NULL;
    }
    DmSegment_release(e->queued);
    e->queued = NULL;
    e->nev = 0;
    DmSynth_sendNoteOffEverything(&e->synth);
#ifdef DME_FLUIDSYNTH
    if (e->fluid != NULL) {
        fluid_synth_all_notes_off(e->fluid, -1);
    }
#endif
    mtx_unlock(&e->lock);
}

void Dme_setMasterGroove(DmeEngine *e, int offset) {
    if (e != NULL) {
        mtx_lock(&e->lock);
        e->master_groove = offset;
        mtx_unlock(&e->lock);
    }
}

void Dme_render(DmeEngine *e, short *out, int frames) {
    int done = 0;
    void (*callback)(void *) = NULL;
    void *ctx = NULL;

    memset(out, 0, (size_t)frames * 4);
    if (e == NULL) {
        return;
    }
    mtx_lock(&e->lock);
    while (done < frames) {
        double per_frame = e->tempo * BEAT / 60.0 / e->rate;
        long next;
        int n = frames - done;

        Process(e, (long)floor(e->now));
        next = NextDue(e);
        if (next != LONG_MAX) {
            double until = ((double)next - e->now) / per_frame;
            int k = until < 1.0 ? 1 : (int)ceil(until);

            n = k < n ? k : n;
        }
#ifdef DME_FLUIDSYNTH
        if (e->fluid != NULL) {
            short *p = out + done * 2;

            fluid_synth_write_s16(e->fluid, n, p, 0, 2, p, 1, 2);
        } else
#endif
            DmSynth_render(&e->synth, out + done * 2, (size_t)n * 2, DmRender_SHORT | DmRender_STEREO);
        DmeReverb_process(e->reverb, out + done * 2, n);
        e->now += n * per_frame;
        done += n;
    }
    if (e->notified) {
        e->notified = 0;
        callback = e->callback;
        ctx = e->callback_ctx;
    }
    mtx_unlock(&e->lock);
    if (callback != NULL) {
        callback(ctx);
    }
}

void Dme_setNotificationTypes(DmeEngine *e, unsigned int types) {
    if (e != NULL) {
        mtx_lock(&e->lock);
        e->types = types;
        mtx_unlock(&e->lock);
    }
}

int Dme_takeNotification(DmeEngine *e, DmeNotification *out) {
    int got = 0;

    if (e == NULL) {
        return 0;
    }
    mtx_lock(&e->lock);
    if (e->nn > 0) {
        *out = e->nq[0];
        memmove(&e->nq[0], &e->nq[1], (e->nn - 1) * sizeof(DmeNotification));
        e->nn--;
        got = 1;
    }
    mtx_unlock(&e->lock);
    return got;
}

void Dme_setNotificationCallback(DmeEngine *e, void (*callback)(void *ctx), void *ctx) {
    if (e != NULL) {
        mtx_lock(&e->lock);
        e->callback = callback;
        e->callback_ctx = ctx;
        mtx_unlock(&e->lock);
    }
}

void Dme_setLog(DmeEngine *e, FILE *log) {
    if (e != NULL) {
        mtx_lock(&e->lock);
        e->log = log;
        mtx_unlock(&e->lock);
    }
}

long Dme_time(DmeEngine *e) {
    long t;

    if (e == NULL) {
        return 0;
    }
    mtx_lock(&e->lock);
    t = (long)floor(e->now);
    mtx_unlock(&e->lock);
    return t;
}
