/* [library:audio] miniwin's Audio Compression Manager: the stream calls the game makes, decoding MS ADPCM, IMA
 * ADPCM and PCM to 16-bit PCM without Windows' ACM. See miniwin/acm.h. tools/test_miniwin/test_acm.exe compares
 * it with Windows' ACM: results, sizes, header fields and every decoded byte. */
#include <windows.h>
#include <mmreg.h>
#include <msacm.h>
#include <stdlib.h>
#include <string.h>

#include "miniwin/acm.h"

#define MAGIC 0x6d636120 /* " acm" */
#define MAX_COEFS 32
#define MAX_FRAMES 8192 /* sample frames per block (a 16 KB stereo block) */

struct Stream {
    int magic;
    int tag; /* WAVE_FORMAT_PCM, WAVE_FORMAT_ADPCM or WAVE_FORMAT_IMA_ADPCM */
    int channels;
    int src_align; /* bytes per source block (per sample frame for PCM) */
    int src_bits; /* PCM only: 8 or 16 */
    int dst_bits; /* 8 or 16 */
    int frames; /* ADPCM: sample frames per block */
    int coefs; /* MS ADPCM: the predictor pairs from the format */
    short coef[MAX_COEFS][2];
};

/* MS ADPCM: how the step size follows each nibble. */
static const int ms_adapt[16] = {230, 230, 230, 230, 307, 409, 512, 614, 768, 614, 512, 409, 307, 230, 230, 230};

/* IMA ADPCM: the step sizes, and how the step index follows each nibble. */
static const int ima_steps[89] = {7, 8, 9, 10, 11, 12, 13, 14, 16, 17, 19, 21, 23, 25, 28, 31, 34, 37, 41, 45, 50,
    55, 60, 66, 73, 80, 88, 97, 107, 118, 130, 143, 157, 173, 190, 209, 230, 253, 279, 307, 337, 371, 408, 449, 494,
    544, 598, 658, 724, 796, 876, 963, 1060, 1166, 1282, 1411, 1552, 1707, 1878, 2066, 2272, 2499, 2749, 3024, 3327,
    3660, 4026, 4428, 4871, 5358, 5894, 6484, 7132, 7845, 8630, 9493, 10442, 11487, 12635, 13899, 15289, 16818, 18500,
    20350, 22385, 24623, 27086, 29794, 32767};
static const int ima_index[16] = {-1, -1, -1, -1, 2, 4, 6, 8, -1, -1, -1, -1, 2, 4, 6, 8};

static short Clamp(long v) {
    return (short)(v < -32768 ? -32768 : v > 32767 ? 32767
                                                   : v);
}

static int Word(const BYTE *p) {
    return (short)(p[0] | p[1] << 8);
}

static struct Stream *Valid(void *handle) {
    struct Stream *s = (struct Stream *)handle;

    return s != NULL && s->magic == MAGIC ? s : NULL;
}

/* Bytes of output for one source block. */
static int OutBlock(const struct Stream *s) {
    return (s->tag == WAVE_FORMAT_PCM ? 1 : s->frames) * s->channels * s->dst_bits / 8;
}

/* The decoders write a block's first frames sample frames; they return 0 for a block ACM refuses (and then the whole
 * conversion produces nothing). */
static int DecodeMs(const struct Stream *s, const BYTE *in, short *out, int frames) {
    int ch = s->channels;
    int pred[2], delta[2], s1[2], s2[2];
    int c, k, total;
    const BYTE *p = in;

    for (c = 0; c < ch; c++, p++) {
        if (*p >= s->coefs) {
            return 0;
        }
        pred[c] = *p;
    }
    for (c = 0; c < ch; c++, p += 2) {
        delta[c] = Word(p);
    }
    for (c = 0; c < ch; c++, p += 2) {
        s1[c] = Word(p);
    }
    for (c = 0; c < ch; c++, p += 2) {
        s2[c] = Word(p);
    }
    for (c = 0; c < ch; c++) {
        out[c] = (short)s2[c];
        out[ch + c] = (short)s1[c];
    }
    out += 2 * ch;
    total = (frames - 2) * ch;
    for (k = 0; k < total; k++) {
        int nibble = k & 1 ? p[k >> 1] & 15 : p[k >> 1] >> 4;
        int signed_nibble = nibble & 8 ? nibble - 16 : nibble;
        long prediction;
        short v;

        c = k % ch;
        prediction = ((long)s1[c] * s->coef[pred[c]][0] + (long)s2[c] * s->coef[pred[c]][1]) >> 8;
        v = Clamp(prediction + (long)signed_nibble * delta[c]);
        s2[c] = s1[c];
        s1[c] = v;
        out[k] = v;
        delta[c] = ms_adapt[nibble] * delta[c] >> 8;
        if (delta[c] < 16) {
            delta[c] = 16;
        }
    }
    return 1;
}

static int DecodeIma(const struct Stream *s, const BYTE *in, short *out) {
    int ch = s->channels;
    int sample[2], index[2];
    int c, n, j;
    const BYTE *p = in;

    for (c = 0; c < ch; c++, p += 4) {
        if (p[2] > 88) {
            return 0;
        }
        sample[c] = Word(p);
        index[c] = p[2]; /* p[3] is reserved: ignored */
        out[c] = (short)sample[c];
    }
    /* then, per channel in turn, 4 bytes holding its next 8 samples (low nibble first) */
    for (n = 1; n < s->frames; n += 8) {
        for (c = 0; c < ch; c++, p += 4) {
            for (j = 0; j < 8 && n + j < s->frames; j++) {
                int nibble = p[j >> 1] >> ((j & 1) * 4) & 15;
                int step = ima_steps[index[c]];
                int diff = step >> 3;

                if (nibble & 4) {
                    diff += step;
                }
                if (nibble & 2) {
                    diff += step >> 1;
                }
                if (nibble & 1) {
                    diff += step >> 2;
                }
                sample[c] = Clamp(nibble & 8 ? (long)sample[c] - diff : (long)sample[c] + diff);
                index[c] += ima_index[nibble];
                index[c] = index[c] < 0 ? 0 : index[c] > 88 ? 88
                                                            : index[c];
                out[(n + j) * ch + c] = (short)sample[c];
            }
        }
    }
    return 1;
}

static int DecodePcm(const struct Stream *s, const BYTE *in, short *out) {
    int c;

    for (c = 0; c < s->channels; c++) {
        out[c] = s->src_bits == 8 ? (short)((in[c] - 128) << 8) : (short)Word(in + c * 2);
    }
    return 1;
}

static int Decode(const struct Stream *s, const BYTE *in, short *out, int frames) {
    if (s->tag == WAVE_FORMAT_ADPCM) {
        return DecodeMs(s, in, out, frames);
    }
    if (s->tag == WAVE_FORMAT_IMA_ADPCM) {
        return DecodeIma(s, in, out);
    }
    return DecodePcm(s, in, out);
}

/* Writes the first bytes of a decoded block in the output's sample size. */
static void Store(const struct Stream *s, const short *in, BYTE *out, unsigned int bytes) {
    unsigned int i;

    if (s->dst_bits == 16) {
        memcpy(out, in, bytes);
        return;
    }
    for (i = 0; i < bytes; i++) {
        out[i] = (BYTE)((in[i] >> 8) + 128);
    }
}

int __stdcall MiniwinAcmStreamOpen(void **stream, void *driver, void *src_format, void *dst_format, void *filter,
    unsigned int callback, unsigned int instance, unsigned int flags) {
    const WAVEFORMATEX *src = (const WAVEFORMATEX *)src_format;
    const WAVEFORMATEX *dst = (const WAVEFORMATEX *)dst_format;
    struct Stream s;
    int frames = 0;

    (void)driver;
    (void)callback;
    (void)instance;
    (void)flags;
    if (stream != NULL) {
        *stream = NULL;
    }
    if (src == NULL || dst == NULL || filter != NULL) {
        return MMSYSERR_INVALPARAM;
    }
    if (dst->wFormatTag != WAVE_FORMAT_PCM || (dst->wBitsPerSample != 8 && dst->wBitsPerSample != 16) ||
        dst->nChannels != src->nChannels || dst->nSamplesPerSec != src->nSamplesPerSec || src->nChannels < 1 ||
        src->nChannels > 2 || dst->nBlockAlign != dst->nChannels * dst->wBitsPerSample / 8) {
        return ACMERR_NOTPOSSIBLE;
    }
    memset(&s, 0, sizeof(s));
    s.magic = MAGIC;
    s.tag = src->wFormatTag;
    s.channels = src->nChannels;
    s.src_align = src->nBlockAlign;
    s.dst_bits = dst->wBitsPerSample;
    switch (src->wFormatTag) {
    case WAVE_FORMAT_PCM:
        if ((src->wBitsPerSample != 8 && src->wBitsPerSample != 16) ||
            src->nBlockAlign != src->nChannels * src->wBitsPerSample / 8) {
            return ACMERR_NOTPOSSIBLE;
        }
        s.src_bits = src->wBitsPerSample;
        break;
    case WAVE_FORMAT_ADPCM: {
        const ADPCMWAVEFORMAT *ms = (const ADPCMWAVEFORMAT *)src;
        int i;

        if (src->wBitsPerSample != 4 || src->cbSize < 4 || ms->wNumCoef < 1 || ms->wNumCoef > MAX_COEFS ||
            src->cbSize < 4 + 4 * ms->wNumCoef) {
            return ACMERR_NOTPOSSIBLE;
        }
        frames = ms->wSamplesPerBlock;
        if (frames < 2 || frames > MAX_FRAMES || src->nBlockAlign < 7 * src->nChannels + ((frames - 2) * src->nChannels + 1) / 2) {
            return ACMERR_NOTPOSSIBLE;
        }
        s.coefs = ms->wNumCoef;
        for (i = 0; i < s.coefs; i++) {
            s.coef[i][0] = ms->aCoef[i].iCoef1;
            s.coef[i][1] = ms->aCoef[i].iCoef2;
        }
        break;
    }
    case WAVE_FORMAT_IMA_ADPCM:
        if (src->wBitsPerSample != 4 || src->nBlockAlign < 4 * src->nChannels ||
            src->nBlockAlign % (4 * src->nChannels) != 0) {
            return ACMERR_NOTPOSSIBLE;
        }
        frames = (src->nBlockAlign - 4 * src->nChannels) * 2 / src->nChannels + 1;
        if (frames > MAX_FRAMES) {
            return ACMERR_NOTPOSSIBLE;
        }
        break;
    default:
        return ACMERR_NOTPOSSIBLE;
    }
    s.frames = frames;
    if (stream == NULL) {
        return MMSYSERR_NOERROR; /* ACM_STREAMOPENF_QUERY-style: possible */
    }
    *stream = malloc(sizeof(s));
    if (*stream == NULL) {
        return MMSYSERR_NOMEM;
    }
    memcpy(*stream, &s, sizeof(s));
    return MMSYSERR_NOERROR;
}

int __stdcall MiniwinAcmStreamSize(void *stream, unsigned int input_size, unsigned int *output_size,
    unsigned int flags) {
    struct Stream *s = Valid(stream);
    unsigned int blocks;

    if (s == NULL) {
        return MMSYSERR_INVALHANDLE;
    }
    if (output_size == NULL) {
        return MMSYSERR_INVALPARAM;
    }
    if (input_size == 0) {
        return MMSYSERR_INVALPARAM;
    }
    switch (flags & ACM_STREAMSIZEF_QUERYMASK) {
    case ACM_STREAMSIZEF_SOURCE:
        /* less than a block can't be converted; beyond that an ADPCM part block counts in full */
        if (input_size < (unsigned int)s->src_align) {
            return ACMERR_NOTPOSSIBLE;
        }
        blocks = input_size / (unsigned int)s->src_align;
        if (s->tag != WAVE_FORMAT_PCM && input_size % (unsigned int)s->src_align != 0) {
            blocks++;
        }
        *output_size = blocks * (unsigned int)OutBlock(s);
        break;
    case ACM_STREAMSIZEF_DESTINATION:
        blocks = input_size / (unsigned int)OutBlock(s);
        *output_size = blocks * (unsigned int)s->src_align;
        break;
    default:
        return MMSYSERR_INVALFLAG;
    }
    return *output_size != 0 ? MMSYSERR_NOERROR : ACMERR_NOTPOSSIBLE;
}

void __stdcall MiniwinAcmStreamClose(void *stream, unsigned int flags) {
    struct Stream *s = Valid(stream);

    (void)flags;
    if (s != NULL) {
        s->magic = 0;
        free(s);
    }
}

int __stdcall MiniwinAcmStreamPrepareHeader(void *stream, void *header, unsigned int flags) {
    ACMSTREAMHEADER *h = (ACMSTREAMHEADER *)header;
    unsigned int size;
    int result;

    (void)flags;
    if (Valid(stream) == NULL) {
        return MMSYSERR_INVALHANDLE;
    }
    if (h == NULL || h->cbStruct < sizeof(ACMSTREAMHEADER)) {
        return MMSYSERR_INVALPARAM;
    }
    /* the source must hold at least a block, as for acmStreamSize */
    result = MiniwinAcmStreamSize(stream, h->cbSrcLength, &size, ACM_STREAMSIZEF_SOURCE);
    if (result != MMSYSERR_NOERROR) {
        return result;
    }
    h->fdwStatus |= ACMSTREAMHEADER_STATUSF_PREPARED;
    return MMSYSERR_NOERROR;
}

int __stdcall MiniwinAcmStreamConvert(void *stream, void *header, unsigned int flags) {
    struct Stream *s = Valid(stream);
    ACMSTREAMHEADER *h = (ACMSTREAMHEADER *)header;
    static short pcm[MAX_FRAMES * 2];
    unsigned int blocks, rest, frame, room, produced, i;
    int part_frames = 0;

    if (s == NULL) {
        return MMSYSERR_INVALHANDLE;
    }
    if (h == NULL || h->cbStruct < sizeof(ACMSTREAMHEADER)) {
        return MMSYSERR_INVALPARAM;
    }
    if (!(h->fdwStatus & ACMSTREAMHEADER_STATUSF_PREPARED)) {
        return ACMERR_UNPREPARED;
    }
    h->fdwStatus &= ~ACMSTREAMHEADER_STATUSF_DONE;
    /* The source's whole blocks go into as many whole sample frames as the destination has room for (the last block
     * may fit only in part); for ADPCM all of them count as used even so, for PCM only the frames that fit. Without
     * ACM_STREAMCONVERTF_BLOCKALIGN, an ADPCM part block at the end counts as used too, and MS ADPCM decodes what it
     * holds. A block ACM refuses ends the conversion, and then nothing counts as produced, though the blocks before
     * it were written. */
    blocks = h->cbSrcLength / (unsigned int)s->src_align;
    rest = h->cbSrcLength % (unsigned int)s->src_align;
    h->cbSrcLengthUsed = blocks * s->src_align;
    if (rest != 0 && !(flags & ACM_STREAMCONVERTF_BLOCKALIGN) && s->tag != WAVE_FORMAT_PCM) {
        h->cbSrcLengthUsed = h->cbSrcLength;
        if (s->tag == WAVE_FORMAT_ADPCM && rest >= 7u * s->channels) {
            part_frames = 2 + (rest - 7 * s->channels) * 2 / s->channels;
        }
    }
    frame = s->channels * s->dst_bits / 8;
    room = h->cbDstLength / frame * frame;
    produced = 0;
    for (i = 0; i < blocks + (part_frames != 0) && produced < room; i++) {
        int frames = i < blocks ? (s->tag == WAVE_FORMAT_PCM ? 1 : s->frames) : part_frames;
        unsigned int n = frames * frame;

        if (!Decode(s, h->pbSrc + i * s->src_align, pcm, frames)) {
            produced = 0;
            break;
        }
        if (n > room - produced) {
            n = room - produced;
        }
        Store(s, pcm, h->pbDst + produced, n);
        produced += n;
    }
    if (s->tag == WAVE_FORMAT_PCM) {
        h->cbSrcLengthUsed = produced / frame * s->src_align;
    }
    h->cbDstLengthUsed = produced;
    h->fdwStatus |= ACMSTREAMHEADER_STATUSF_DONE;
    return MMSYSERR_NOERROR;
}

void __stdcall MiniwinAcmStreamUnprepareHeader(void *stream, void *header, unsigned int flags) {
    ACMSTREAMHEADER *h = (ACMSTREAMHEADER *)header;

    (void)flags;
    if (Valid(stream) != NULL && h != NULL) {
        h->fdwStatus &= ~ACMSTREAMHEADER_STATUSF_PREPARED;
    }
}
