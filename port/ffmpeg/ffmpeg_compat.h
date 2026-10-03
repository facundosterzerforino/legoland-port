/*
 * The few FFmpeg (libavutil/libavcodec) definitions that FFmpeg's Indeo 5 decoder (indeo5.c, ivi.c, ivi_dsp.c)
 * and Indeo 3 decoder (indeo3.c) need, so they build on their own inside the LEGOLAND port. Written for the
 * port; the bit reader and VLC tables behave like FFmpeg's get_bits.h (safe mode; little-endian when the file
 * defines BITSTREAM_READER_LE first, as the Indeo 5 files do) and vlc.c for the single-level tables these files
 * build.
 *
 * This file is part of the port's copy of FFmpeg's Indeo decoders and, like them, is licensed under the
 * GNU Lesser General Public License, version 2.1 or later (see COPYING.LGPLv2.1).
 */

#ifndef PORT_IVI_COMPAT_H
#define PORT_IVI_COMPAT_H

#include <errno.h>
#include <inttypes.h>
#include <limits.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

/* attributes and helpers (libavutil/attributes.h, common.h, macros.h) */
#define av_cold
#define av_always_inline inline
#define av_fallthrough ((void)0)

#define FFMIN(a, b) ((a) > (b) ? (b) : (a))
#define FFMAX(a, b) ((a) > (b) ? (a) : (b))
#define FFSWAP(type, a, b) \
    do {                   \
        type SWAP_tmp = b; \
        b = a;             \
        a = SWAP_tmp;      \
    } while (0)
#define FFALIGN(x, a) (((x) + (a)-1) & ~((a)-1))
#define FFSIGN(a) ((a) > 0 ? 1 : -1)
#define FFMIN3(a, b, c) FFMIN(FFMIN(a, b), c)
#define FFMAX3(a, b, c) FFMAX(FFMAX(a, b), c)
#define MKBETAG(a, b, c, d) ((unsigned)(d) | ((unsigned)(c) << 8) | ((unsigned)(b) << 16) | ((unsigned)(a) << 24))
#define HAVE_BIGENDIAN 0

static inline int av_clip(int a, int amin, int amax) { return a < amin ? amin : (a > amax ? amax : a); }
static inline uint8_t av_clip_uint8(int a) { return (uint8_t)(a < 0 ? 0 : (a > 255 ? 255 : a)); }
static inline unsigned av_clip_uintp2(int a, int p) {
    if (a & ~((1 << p) - 1)) {
        return (~a) >> 31 & ((1 << p) - 1);
    }
    return a;
}

/* errors and logging (libavutil/error.h, log.h, avassert.h) */
#define AVERROR(e) (-(e))
#define AVERROR_INVALIDDATA (-0x41444e49)
#define AVERROR_PATCHWELCOME (-0x45424150)
#define AV_LOG_ERROR 16
#define AV_LOG_DEBUG 48
static inline void av_log(void *avcl, int level, const char *fmt, ...) {
    (void)avcl;
    (void)level;
    (void)fmt;
}
#define ff_dlog(ctx, ...) ((void)0)
#define avpriv_report_missing_feature(avc, ...) ((void)0)
#define avpriv_request_sample(avc, ...) ((void)0)
#define av_assert2(cond) ((void)0)
#define av_assert0(cond) \
    do {                 \
        if (!(cond)) {   \
            abort();     \
        }                \
    } while (0)

/* memory (libavutil/mem.h) */
static inline void *av_malloc(size_t size) { return malloc(size ? size : 1); }
static inline void *av_mallocz(size_t size) { return calloc(1, size ? size : 1); }
static inline void *av_calloc(size_t nmemb, size_t size) { return calloc(nmemb ? nmemb : 1, size ? size : 1); }
static inline void av_free(void *ptr) { free(ptr); }
static inline void av_freep(void *arg) {
    void *ptr;
    memcpy(&ptr, arg, sizeof(ptr));
    free(ptr);
    ptr = NULL;
    memcpy(arg, &ptr, sizeof(ptr));
}

/* one-time initialisation (libavutil/thread.h); the port decodes on one thread */
typedef int AVOnce;
#define AV_ONCE_INIT 0
static inline int ff_thread_once(AVOnce *once, void (*routine)(void)) {
    if (!*once) {
        *once = 1;
        routine();
    }
    return 0;
}

/* codec, frame and packet (libavcodec/avcodec.h, decode.h, libavutil/frame.h, imgutils.h) */
enum { AV_PIX_FMT_YUV410P = 1 };
enum { AV_CODEC_ID_INDEO3 = 1, AV_CODEC_ID_INDEO4 = 2, AV_CODEC_ID_INDEO5 = 3 };
enum { AVDISCARD_DEFAULT = 0, AVDISCARD_NONREF = 8, AVDISCARD_NONKEY = 32 };

typedef struct AVCodecContext {
    void *priv_data;
    int width;
    int height;
    int codec_id;
    int pix_fmt;
    int64_t max_pixels;
    int flags;
    int skip_frame;
} AVCodecContext;

/* YUV 4:1:0 planes, owned by the frame */
typedef struct AVFrame {
    uint8_t *data[3];
    int linesize[3];
    int width;
    int height;
} AVFrame;

typedef struct AVPacket {
    const uint8_t *data;
    int size;
} AVPacket;

static inline int av_image_check_size2(unsigned w, unsigned h, int64_t max_pixels, int pix_fmt, int log_offset,
    void *log_ctx) {
    (void)max_pixels;
    (void)pix_fmt;
    (void)log_offset;
    (void)log_ctx;
    return (w > 0 && h > 0 && w <= 4096 && h <= 4096) ? 0 : AVERROR(EINVAL);
}

static inline int av_image_check_size(unsigned w, unsigned h, int log_offset, void *log_ctx) {
    return av_image_check_size2(w, h, 0, 0, log_offset, log_ctx);
}

static inline int ff_set_dimensions(AVCodecContext *avctx, int width, int height) {
    if (av_image_check_size2(width, height, 0, 0, 0, NULL) < 0) {
        return AVERROR(EINVAL);
    }
    avctx->width = width;
    avctx->height = height;
    return 0;
}

static inline void av_frame_unref(AVFrame *frame) {
    int i;
    for (i = 0; i < 3; i++) {
        free(frame->data[i]);
    }
    memset(frame, 0, sizeof(*frame));
}

/* (Re)allocates the frame's planes for the codec's current size. */
static inline int ff_get_buffer(AVCodecContext *avctx, AVFrame *frame, int flags) {
    int i;
    (void)flags;
    if (frame->data[0] == NULL || frame->width != avctx->width || frame->height != avctx->height) {
        av_frame_unref(frame);
        frame->width = avctx->width;
        frame->height = avctx->height;
        frame->linesize[0] = FFALIGN(avctx->width, 16);
        frame->linesize[1] = frame->linesize[2] = FFALIGN((avctx->width + 3) >> 2, 16);
        frame->data[0] = (uint8_t *)calloc(1, (size_t)frame->linesize[0] * avctx->height);
        for (i = 1; i < 3; i++) {
            frame->data[i] = (uint8_t *)calloc(1, (size_t)frame->linesize[i] * ((avctx->height + 3) >> 2));
        }
        if (!frame->data[0] || !frame->data[1] || !frame->data[2]) {
            av_frame_unref(frame);
            return AVERROR(ENOMEM);
        }
    }
    return 0;
}

static inline void av_frame_move_ref(AVFrame *dst, AVFrame *src) {
    av_frame_unref(dst);
    *dst = *src;
    memset(src, 0, sizeof(*src));
}

static inline void av_frame_free(AVFrame **frame) {
    if (*frame) {
        av_frame_unref(*frame);
        free(*frame);
        *frame = NULL;
    }
}

extern const uint8_t ff_zigzag_direct[64];

/* unaligned and aligned loads and stores (libavutil/intreadwrite.h), little-endian host */
static inline uint16_t AV_RN16(const void *p) { uint16_t v; memcpy(&v, p, 2); return v; }
static inline uint32_t AV_RN32(const void *p) { uint32_t v; memcpy(&v, p, 4); return v; }
static inline uint64_t AV_RN64(const void *p) { uint64_t v; memcpy(&v, p, 8); return v; }
#define AV_RN32A(p) AV_RN32(p)
#define AV_WN16A(p, v) do { uint16_t w_ = (uint16_t)(v); memcpy((p), &w_, 2); } while (0)
#define AV_WN32A(p, v) do { uint32_t w_ = (uint32_t)(v); memcpy((p), &w_, 4); } while (0)
#define AV_WN64A(p, v) do { uint64_t w_ = (uint64_t)(v); memcpy((p), &w_, 8); } while (0)
#define AV_COPY32U(d, s) memmove((d), (s), 4)
#define AV_COPY64U(d, s) memmove((d), (s), 8)
static inline uint32_t AV_RL32(const uint8_t *p) {
    return (uint32_t)p[0] | (uint32_t)p[1] << 8 | (uint32_t)p[2] << 16 | (uint32_t)p[3] << 24;
}
static inline uint32_t AV_RB32(const uint8_t *p) {
    return (uint32_t)p[3] | (uint32_t)p[2] << 8 | (uint32_t)p[1] << 16 | (uint32_t)p[0] << 24;
}

/* byte reader (libavcodec/bytestream.h) */
typedef struct GetByteContext {
    const uint8_t *buffer;
    const uint8_t *buffer_end;
    const uint8_t *buffer_start;
} GetByteContext;

static inline void bytestream2_init(GetByteContext *g, const uint8_t *buf, int buf_size) {
    g->buffer = g->buffer_start = buf;
    g->buffer_end = buf + (buf_size > 0 ? buf_size : 0);
}
static inline void bytestream2_skip(GetByteContext *g, unsigned size) {
    g->buffer += FFMIN((size_t)(g->buffer_end - g->buffer), (size_t)size);
}
static inline unsigned bytestream2_get_byte(GetByteContext *g) {
    if (g->buffer_end - g->buffer < 1) {
        g->buffer = g->buffer_end;
        return 0;
    }
    return *g->buffer++;
}
static inline unsigned bytestream2_get_le16(GetByteContext *g) {
    unsigned v;
    if (g->buffer_end - g->buffer < 2) {
        g->buffer = g->buffer_end;
        return 0;
    }
    v = g->buffer[0] | g->buffer[1] << 8;
    g->buffer += 2;
    return v;
}
static inline unsigned bytestream2_get_le32(GetByteContext *g) {
    unsigned v;
    if (g->buffer_end - g->buffer < 4) {
        g->buffer = g->buffer_end;
        return 0;
    }
    v = AV_RL32(g->buffer);
    g->buffer += 4;
    return v;
}
static inline unsigned bytestream_get_byte(const uint8_t **p) { return *(*p)++; }
static inline unsigned bytestream_get_le32(const uint8_t **p) {
    unsigned v = AV_RL32(*p);
    *p += 4;
    return v;
}

/* block copies (libavcodec/copy_block.h, hpeldsp.h: only the full-pel put functions) */
static inline void copy_block4(uint8_t *dst, const uint8_t *src, ptrdiff_t dstStride, ptrdiff_t srcStride, int h) {
    int i;
    for (i = 0; i < h; i++) {
        AV_COPY32U(dst, src);
        dst += dstStride;
        src += srcStride;
    }
}

typedef void (*op_pixels_func)(uint8_t *block, const uint8_t *pixels, ptrdiff_t line_size, int h);
typedef struct HpelDSPContext {
    op_pixels_func put_pixels_tab[4][4];
} HpelDSPContext;

static inline void ffmpeg_put_pixels(uint8_t *block, const uint8_t *pixels, ptrdiff_t line_size, int h, int w) {
    int i;
    for (i = 0; i < h; i++) {
        memcpy(block, pixels, w);
        block += line_size;
        pixels += line_size;
    }
}
static void ffmpeg_put_pixels16(uint8_t *b, const uint8_t *p, ptrdiff_t l, int h) { ffmpeg_put_pixels(b, p, l, h, 16); }
static void ffmpeg_put_pixels8(uint8_t *b, const uint8_t *p, ptrdiff_t l, int h) { ffmpeg_put_pixels(b, p, l, h, 8); }
static void ffmpeg_put_pixels4(uint8_t *b, const uint8_t *p, ptrdiff_t l, int h) { ffmpeg_put_pixels(b, p, l, h, 4); }
static inline void ff_hpeldsp_init(HpelDSPContext *c, int flags) {
    (void)flags;
    memset(c, 0, sizeof(*c));
    c->put_pixels_tab[0][0] = ffmpeg_put_pixels16;
    c->put_pixels_tab[1][0] = ffmpeg_put_pixels8;
    c->put_pixels_tab[2][0] = ffmpeg_put_pixels4;
}

/* bit reader (libavcodec/get_bits.h, safe mode): little-endian if BITSTREAM_READER_LE is defined first */
typedef struct GetBitContext {
    const uint8_t *buffer;
    int index;
    int size_in_bits;
    int size_in_bits_plus8;
} GetBitContext;

/* The input must be followed by at least 8 readable bytes (the port pads it with PORT_FFMPEG_PADDING). */
static inline int init_get_bits(GetBitContext *s, const uint8_t *buffer, int bit_size) {
    if (bit_size < 0 || bit_size > INT_MAX - 64 || !buffer) {
        s->buffer = NULL;
        s->index = s->size_in_bits = s->size_in_bits_plus8 = 0;
        return AVERROR_INVALIDDATA;
    }
    s->buffer = buffer;
    s->index = 0;
    s->size_in_bits = bit_size;
    s->size_in_bits_plus8 = bit_size + 8;
    return 0;
}

static inline int init_get_bits8(GetBitContext *s, const uint8_t *buffer, int byte_size) {
    if (byte_size < 0 || byte_size > INT_MAX / 8 - 8) {
        return init_get_bits(s, NULL, 0);
    }
    return init_get_bits(s, buffer, byte_size * 8);
}

/* up to 25 bits */
static inline unsigned int show_bits(GetBitContext *s, int n) {
    if (!n) {
        return 0;
    }
#ifdef BITSTREAM_READER_LE
    return (AV_RL32(s->buffer + (s->index >> 3)) >> (s->index & 7)) & ((1u << n) - 1);
#else
    return (AV_RB32(s->buffer + (s->index >> 3)) << (s->index & 7)) >> (32 - n);
#endif
}

static inline void skip_bits(GetBitContext *s, int n) {
    int index = s->index + n;
    s->index = index < s->size_in_bits_plus8 ? index : s->size_in_bits_plus8;
}

static inline void skip_bits_long(GetBitContext *s, int n) {
    int index = s->index + n;
    if (index < 0) {
        index = 0;
    }
    s->index = index < s->size_in_bits_plus8 ? index : s->size_in_bits_plus8;
}

static inline unsigned int get_bits(GetBitContext *s, int n) {
    unsigned int value = show_bits(s, n);
    skip_bits(s, n);
    return value;
}

static inline unsigned int get_bits1(GetBitContext *s) { return get_bits(s, 1); }

/* up to 32 bits */
static inline unsigned int get_bits_long(GetBitContext *s, int n) {
    unsigned int first;
    if (n <= 25) {
        return get_bits(s, n);
    }
    first = get_bits(s, 16);
#ifdef BITSTREAM_READER_LE
    return first | get_bits(s, n - 16) << 16;
#else
    return first << (n - 16) | get_bits(s, n - 16);
#endif
}

static inline int get_bits_count(const GetBitContext *s) { return s->index; }
static inline int get_bits_left(GetBitContext *s) { return s->size_in_bits - s->index; }

static inline const uint8_t *align_get_bits(GetBitContext *s) {
    int n = -get_bits_count(s) & 7;
    if (n) {
        skip_bits(s, n);
    }
    return s->buffer + (s->index >> 3);
}

/* VLC tables (libavcodec/vlc.h, vlc.c): single-level only, codes no longer than the table's bits */
typedef struct VLCElem {
    int16_t sym;
    int16_t len;
} VLCElem;

typedef struct VLC {
    int bits;
    VLCElem *table;
    int table_size;
    int table_allocated;
} VLC;

#define VLC_INIT_USE_STATIC 1
#define VLC_INIT_OUTPUT_LE 8

int ivi_vlc_init(VLC *vlc, int nb_bits, int nb_codes, const uint8_t *bits, const uint16_t *codes, int flags);
#define vlc_init(vlc, nb_bits, nb_codes, bits, bits_wrap, bits_size, codes, codes_wrap, codes_size, flags) \
    ivi_vlc_init(vlc, nb_bits, nb_codes, bits, codes, flags)
void ff_vlc_free(VLC *vlc);

static inline int get_vlc2(GetBitContext *s, const VLCElem *table, int bits, int max_depth) {
    unsigned int index = show_bits(s, bits);
    int n = table[index].len;
    (void)max_depth;
    skip_bits(s, n);
    return table[index].sym;
}

#endif
