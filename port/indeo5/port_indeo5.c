/*
 * [library:movie] The port's entry points into FFmpeg's Indeo 5 decoder, plus the pieces of FFmpeg it needs
 * that aren't in ivi_compat.h: the VLC table builder (the single-level case of libavcodec/vlc.c) and the
 * zigzag scan table (libavcodec/mathtables.c).
 *
 * This file is part of the port's copy of FFmpeg's Indeo 5 decoder and, like it, is licensed under the
 * GNU Lesser General Public License, version 2.1 or later (see COPYING.LGPLv2.1).
 */

#include "ivi_compat.h"
#include "ivi.h"
#include "port_indeo5.h"

int ff_indeo5_decode_init(AVCodecContext *avctx);

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

struct PortIndeo5 {
    AVCodecContext avctx;
    IVI45DecContext ctx;
    AVFrame frame;
};

PortIndeo5 *PortIndeo5_Open(int width, int height) {
    PortIndeo5 *decoder = (PortIndeo5 *)calloc(1, sizeof(PortIndeo5));

    if (decoder == NULL) {
        return NULL;
    }
    decoder->avctx.priv_data = &decoder->ctx;
    decoder->avctx.width = width;
    decoder->avctx.height = height;
    decoder->avctx.codec_id = AV_CODEC_ID_INDEO5;
    if (ff_indeo5_decode_init(&decoder->avctx) < 0) {
        PortIndeo5_Close(decoder); /* FF_CODEC_CAP_INIT_CLEANUP: close cleans up a failed init */
        return NULL;
    }
    return decoder;
}

int PortIndeo5_Decode(PortIndeo5 *decoder, const unsigned char *data, int size, const unsigned char *planes[3],
    int linesize[3]) {
    AVPacket packet;
    int got_frame = 0;
    int result;
    int i;

    packet.data = data;
    packet.size = size;
    result = ff_ivi_decode_frame(&decoder->avctx, &decoder->frame, &got_frame, &packet);
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

void PortIndeo5_Close(PortIndeo5 *decoder) {
    if (decoder != NULL) {
        ff_ivi_decode_close(&decoder->avctx);
        av_frame_unref(&decoder->frame);
        free(decoder);
    }
}
