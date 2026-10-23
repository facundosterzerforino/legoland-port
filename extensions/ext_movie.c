/* Pause movies: see ext_movie.h. */
#include <windows.h>
#include <SDL3/SDL.h>

#include "../src/legoland/globals.h"
#include "../src/legoland/imports.h"
#include "ext_movie.h"
#include "miniwin/acm.h"
#include "miniwin/avifile.h"
#include "miniwin/miniwin.h"

int ExtPauseMovies = 1;

static int waiting_now;
static Uint64 wait_started;
static Uint64 waited;

void ExtPauseMoviesApply(void) {
    MiniwinSoundHoldWhenInactive(ExtPauseMovies);
}

void ExtPauseMoviesWaiting(int waiting) {
    if (waiting && !waiting_now) {
        waiting_now = 1;
        wait_started = SDL_GetTicks();
    } else if (!waiting && waiting_now) {
        waiting_now = 0;
        waited += SDL_GetTicks() - wait_started;
    }
}

int ExtPauseMoviesTake(void) {
    int ms = ExtPauseMovies ? (int)waited : 0;

    waited = 0;
    return ms;
}

int ExtPauseMoviesResync(unsigned int frame, int play_pos) {
    unsigned int per_block, target, skip;
    int bytes, samples;

    if (!ExtPauseMovies) {
        return play_pos;
    }
    /* SetCurrentPosition takes bytes: the frame's slot in the sound buffer */
    play_pos = (int)(AviSoundBytesPerFrame * AviSoundBufferSlot);
    if (AviAudioNeedsAcm == 0) {
        return play_pos; /* PCM: AVI positions are sample frames, as the game assumes */
    }
    /* Compressed: AVI positions count blocks. Normal playback hands out the decoded sound AviSoundBytesPerFrame
     * bytes per movie frame, so the frame's sound starts that many bytes times frame into the decoded stream:
     * decode the block holding it, and leave the rest of that block for the refill to use first. */
    AviAcmDstBytes = 0;
    AviAcmDstOffset = 0;
    if (MiniwinAcmStreamSize(AviAcmStream, AviAcmStreamHeader.cbSrcLength, &per_block, 0 /* ACM_STREAMSIZEF_SOURCE */) != 0 ||
        per_block == 0) {
        AviAudioSamplePos = DAT_00668f88; /* silence, as before */
        return play_pos;
    }
    target = AviSoundBytesPerFrame * frame;
    skip = target % per_block;
    AviAudioSamplePos = (int)(target / per_block);
    if (AviAudioSamplePos >= DAT_00668f88) {
        return play_pos;
    }
    MiniwinAVIStreamRead(AviAudioStream, AviAudioSamplePos, 0x100, AviAcmStreamHeader.pbSrc, AviSoundBytesPerFrame >> 2,
        &bytes, &samples);
    AviAcmStreamHeader.pbDst = AviAcmDstBase;
    MiniwinAcmStreamPrepareHeader(AviAcmStream, &AviAcmStreamHeader, 0);
    MiniwinAcmStreamConvert(AviAcmStream, &AviAcmStreamHeader, 0x10 /* ACM_STREAMCONVERTF_BLOCKALIGN */);
    MiniwinAcmStreamUnprepareHeader(AviAcmStream, &AviAcmStreamHeader, 0);
    AviAudioSamplePos++;
    if (AviAcmStreamHeader.cbDstLengthUsed > skip) {
        AviAcmDstBytes = AviAcmStreamHeader.cbDstLengthUsed - skip;
        AviAcmDstOffset = (int)skip;
    }
    return play_pos;
}
