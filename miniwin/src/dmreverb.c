/* The reverb DirectMusic's software synthesizer adds by default, for the portable music: see dmreverb.h. */
#include "dmreverb.h"

#include <math.h>
#include <stdlib.h>

#define COMBS 8
#define ALLPASSES 4
#define REVERB_SECONDS 1.0 /* DirectMusic's default: the tail falls 60 dB in a second */
#define INPUT_GAIN 0.015f /* Freeverb's fixed input scale */
#define WET 1.0f /* fitted: tools/music/README.md */
#define DAMP 0.2f /* fitted */
#define ALLPASS_FEEDBACK 0.5f

/* Freeverb's delays at 44.1 kHz; the right side's are longer by the stereo spread */
static const int comb_lengths[COMBS] = {1116, 1188, 1277, 1356, 1422, 1491, 1557, 1617};
static const int allpass_lengths[ALLPASSES] = {556, 441, 341, 225};
#define SPREAD 23

typedef struct Delay {
    float *buf;
    int size, pos;
    float feedback, store;
} Delay;

struct DmeReverb {
    Delay comb[2][COMBS];
    Delay allpass[2][ALLPASSES];
};

static int InitDelay(Delay *d, int size, float feedback) {
    d->buf = (float *)calloc((size_t)size, sizeof(float));
    d->size = size;
    d->pos = 0;
    d->feedback = feedback;
    d->store = 0;
    return d->buf != NULL;
}

DmeReverb *DmeReverb_create(unsigned int rate) {
    DmeReverb *r = (DmeReverb *)calloc(1, sizeof(*r));
    double scale = rate / 44100.0;
    int side, i, ok = 1;

    if (r == NULL) {
        return NULL;
    }
    for (side = 0; side < 2; side++) {
        for (i = 0; i < COMBS; i++) {
            int n = (int)((comb_lengths[i] + side * SPREAD) * scale + 0.5);

            /* the feedback that makes this comb fall 60 dB in the reverb time */
            ok &= InitDelay(&r->comb[side][i], n, (float)pow(10.0, -3.0 * n / (rate * REVERB_SECONDS)));
        }
        for (i = 0; i < ALLPASSES; i++) {
            ok &= InitDelay(&r->allpass[side][i], (int)((allpass_lengths[i] + side * SPREAD) * scale + 0.5),
                ALLPASS_FEEDBACK);
        }
    }
    if (!ok) {
        DmeReverb_destroy(r);
        return NULL;
    }
    return r;
}

void DmeReverb_destroy(DmeReverb *r) {
    int side, i;

    if (r == NULL) {
        return;
    }
    for (side = 0; side < 2; side++) {
        for (i = 0; i < COMBS; i++) {
            free(r->comb[side][i].buf);
        }
        for (i = 0; i < ALLPASSES; i++) {
            free(r->allpass[side][i].buf);
        }
    }
    free(r);
}

void DmeReverb_process(DmeReverb *r, short *frames, int count) {
    int n, side, i;

    if (r == NULL) {
        return;
    }
    for (n = 0; n < count; n++) {
        float in = (frames[n * 2] + frames[n * 2 + 1]) * 0.5f * INPUT_GAIN;

        for (side = 0; side < 2; side++) {
            float acc = 0;
            float v;

            for (i = 0; i < COMBS; i++) {
                Delay *c = &r->comb[side][i];
                float out = c->buf[c->pos];

                c->store = out * (1 - DAMP) + c->store * DAMP;
                c->buf[c->pos] = in + c->store * c->feedback;
                c->pos = c->pos + 1 == c->size ? 0 : c->pos + 1;
                acc += out;
            }
            for (i = 0; i < ALLPASSES; i++) {
                Delay *a = &r->allpass[side][i];
                float out = a->buf[a->pos];

                a->buf[a->pos] = acc + out * a->feedback;
                a->pos = a->pos + 1 == a->size ? 0 : a->pos + 1;
                acc = out - acc;
            }
            v = frames[n * 2 + side] + acc * WET;
            frames[n * 2 + side] = (short)(v > 32767 ? 32767 : v < -32768 ? -32768 : v);
        }
    }
}
