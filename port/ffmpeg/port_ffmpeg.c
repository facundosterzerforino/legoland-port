/*
 * [library:movie] The port's entry points into FFmpeg's Indeo 3 and Indeo 5 decoders, plus the pieces of FFmpeg
 * they need that aren't in ffmpeg_compat.h: the VLC table builder (the single-level case of libavcodec/vlc.c)
 * and the zigzag scan table (libavcodec/mathtables.c).
 *
 * This file is part of the port's copy of FFmpeg's Indeo decoders and, like them, is licensed under the
 * GNU Lesser General Public License, version 2.1 or later (see COPYING.LGPLv2.1).
 */

#include "ffmpeg_compat.h"
#include "ivi.h"
#include "port_ffmpeg.h"

int ff_indeo5_decode_init(AVCodecContext *avctx);

extern const int ff_indeo3_priv_data_size;
int ff_indeo3_decode_init(AVCodecContext *avctx);
int ff_indeo3_decode_frame(AVCodecContext *avctx, AVFrame *frame, int *got_frame, AVPacket *avpkt);
int ff_indeo3_decode_close(AVCodecContext *avctx);

const uint8_t ff_zigzag_direct[64] = {
    0,  1,  8,  16, 9,  2,  3,  10, 17, 24, 32, 25, 18, 11, 4,  5,  12, 19, 26, 33, 40, 48,
    41, 34, 27, 20, 13, 6,  7,  14, 21, 28, 35, 42, 49, 56, 57, 50, 43, 36, 29, 22, 15, 23,
    30, 37, 44, 51, 58, 59, 52, 45, 38, 31, 39, 46, 53, 60, 61, 54, 47, 55, 62, 63,
};

/* Builds a table indexed by the next nb_bits bits of the stream. With VLC_INIT_OUTPUT_LE (always the case
 * here) the stream is read least significant bit first, so each code goes in bit-reversed, repeated for every
 * value of the bits after it. Entries no code reaches get len 0 and sym -1, as in FFmpeg. */
int ivi_vlc_init(VLC *vlc, int nb_bits, int nb_codes, const uint8_t *bits, const uint16_t *codes, int flags) {
    int size = 1 << nb_bits;
    int i;

    if (!(flags & VLC_INIT_USE_STATIC)) {
        vlc->table = (VLCElem *)calloc(size, sizeof(VLCElem));
        if (!vlc->table) {
            return AVERROR(ENOMEM);
        }
        vlc->table_allocated = size;
    } else if (vlc->table_allocated < size) {
        return AVERROR(EINVAL);
    }
    memset(vlc->table, 0, size * sizeof(VLCElem));
    vlc->bits = nb_bits;
    vlc->table_size = size;

    for (i = 0; i < nb_codes; i++) {
        int n = bits[i];
        unsigned code = codes[i];
        unsigned reversed = 0;
        int k;

        if (n == 0) {
            continue;
        }
        if (n > nb_bits || code >= (1u << n)) {
            if (!(flags & VLC_INIT_USE_STATIC)) {
                ff_vlc_free(vlc);
            }
            return AVERROR_INVALIDDATA;
        }
        for (k = 0; k < n; k++) {
            reversed |= ((code >> k) & 1) << (n - 1 - k);
        }
        for (k = reversed; k < size; k += 1 << n) {
            if (vlc->table[k].len && (vlc->table[k].len != n || vlc->table[k].sym != i)) {
                if (!(flags & VLC_INIT_USE_STATIC)) {
                    ff_vlc_free(vlc);
                }
                return AVERROR_INVALIDDATA; /* "incorrect codes" */
            }
            vlc->table[k].len = (int16_t)n;
            vlc->table[k].sym = (int16_t)i;
        }
    }
    for (i = 0; i < size; i++) {
        if (vlc->table[i].len == 0) {
            vlc->table[i].sym = -1;
        }
    }
    return 0;
}

void ff_vlc_free(VLC *vlc) {
    free(vlc->table);
    vlc->table = NULL;
    vlc->table_size = vlc->table_allocated = 0;
}

struct PortVideoDecoder {
    AVCodecContext avctx;
    int (*decode)(AVCodecContext *avctx, AVFrame *frame, int *got_frame, AVPacket *avpkt);
    int (*close)(AVCodecContext *avctx);
    AVFrame frame;
};

static unsigned int UpperFourcc(unsigned int fourcc) {
    unsigned int result = 0;
    int i;

    for (i = 0; i < 4; i++) {
        unsigned int c = (fourcc >> (i * 8)) & 0xff;
        if (c >= 'a' && c <= 'z') {
            c -= 'a' - 'A';
        }
        result |= c << (i * 8);
    }
    return result;
}

#define FOURCC(a, b, c, d) ((unsigned int)(a) | (unsigned int)(b) << 8 | (unsigned int)(c) << 16 | (unsigned int)(d) << 24)

PortVideoDecoder *PortVideoDecoder_Open(unsigned int fourcc, int width, int height) {
    PortVideoDecoder *decoder;
    int (*init)(AVCodecContext *avctx);
    size_t priv_size;
    int codec_id;

    fourcc = UpperFourcc(fourcc);
    if (fourcc == FOURCC('I', 'V', '5', '0')) {
        codec_id = AV_CODEC_ID_INDEO5;
        priv_size = sizeof(IVI45DecContext);
        init = ff_indeo5_decode_init;
    } else if (fourcc == FOURCC('I', 'V', '3', '1') || fourcc == FOURCC('I', 'V', '3', '2')) {
        codec_id = AV_CODEC_ID_INDEO3;
        priv_size = (size_t)ff_indeo3_priv_data_size;
        init = ff_indeo3_decode_init;
    } else {
        return NULL;
    }
    decoder = (PortVideoDecoder *)calloc(1, sizeof(PortVideoDecoder));
    if (decoder == NULL) {
        return NULL;
    }
    decoder->avctx.priv_data = calloc(1, priv_size);
    if (decoder->avctx.priv_data == NULL) {
        free(decoder);
        return NULL;
    }
    decoder->avctx.width = width;
    decoder->avctx.height = height;
    decoder->avctx.codec_id = codec_id;
    if (codec_id == AV_CODEC_ID_INDEO5) {
        decoder->decode = ff_ivi_decode_frame;
        decoder->close = ff_ivi_decode_close;
    } else {
        decoder->decode = ff_indeo3_decode_frame;
        decoder->close = ff_indeo3_decode_close;
    }
    if (init(&decoder->avctx) < 0) {
        PortVideoDecoder_Close(decoder); /* FF_CODEC_CAP_INIT_CLEANUP: close cleans up a failed init */
        return NULL;
    }
    return decoder;
}

int PortVideoDecoder_Decode(PortVideoDecoder *decoder, const unsigned char *data, int size,
    const unsigned char *planes[3], int linesize[3]) {
    AVPacket packet;
    int got_frame = 0;
    int result;
    int i;

    packet.data = data;
    packet.size = size;
    result = decoder->decode(&decoder->avctx, &decoder->frame, &got_frame, &packet);
    if (result < 0) {
        return result;
    }
    if (!got_frame) {
        return 0;
    }
    for (i = 0; i < 3; i++) {
        planes[i] = decoder->frame.data[i];
        linesize[i] = decoder->frame.linesize[i];
    }
    return 1;
}

void PortVideoDecoder_Close(PortVideoDecoder *decoder) {
    if (decoder != NULL) {
        decoder->close(&decoder->avctx);
        av_frame_unref(&decoder->frame);
        free(decoder->avctx.priv_data);
        free(decoder);
    }
}
