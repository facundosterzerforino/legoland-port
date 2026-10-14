#ifndef MINIWIN_DMENGINE_H
#define MINIWIN_DMENGINE_H

/* [library:music] The style engine behind miniwin's DirectMusic (miniwin/src/dmusic.c): plays the game's segments
 * the way Windows' DirectMusic composes them, as measured against recordings of it (tools/music/README.md, "the
 * model"), with GothicKit dmusic (port/dmusic) for the files and the synthesizer. Times are music ticks (768 a beat).
 * Thread-safe: the game's music thread plays, stops and reads notifications while the mixer renders. */

#include <stdio.h>

#include <dmusic.h>

typedef struct DmeEngine DmeEngine;

/* DirectMusic's notifications, as the game reads them (DMUS_NOTIFICATION_PMSG) */
enum {
    DME_SEGMENT = 1, /* GUID_NOTIFICATION_SEGMENT, option DME_SEG_* */
    DME_MEASURE_AND_BEAT, /* GUID_NOTIFICATION_MEASUREANDBEAT: field1 the beat, field2 the measure */
};
enum {
    DME_SEG_START = 0,
    DME_SEG_END = 1,
    DME_SEG_ALMOST_END = 2,
    DME_SEG_LOOP = 3,
    DME_SEG_ABORT = 4,
};

typedef struct DmeNotification {
    int type;
    unsigned int option, field1, field2;
    long time;
} DmeNotification;

/* The timing of DirectMusic's PlaySegment flags. */
enum {
    DME_AT_ONCE,
    DME_AT_GRID,
    DME_AT_BEAT,
    DME_AT_MEASURE,
};

/* rate: the synthesizer's (the port's) sample rate. */
DmeEngine *Dme_create(DmLoader *loader, unsigned int rate);
/* Plays through FluidSynth on this gm.dls (DLS read by FluidSynth itself, by Microsoft's rules) instead of dmusic's
 * synthesizer, when built with it (DME_FLUIDSYNTH); 0 when it can't (then dmusic's plays). */
int Dme_useFluidSynth(DmeEngine *e, const char *gm_dls);
void Dme_destroy(DmeEngine *e);

/* Loads the segment's instruments and styles (DirectMusic's GUID_Download). */
int Dme_download(DmeEngine *e, DmSegment *seg);

/* Plays seg as the primary segment from the next boundary of that kind (as PlaySegment). repeats: how often it
 * repeats after its first time. */
void Dme_play(DmeEngine *e, DmSegment *seg, unsigned int repeats, int timing);
/* Stops at once (as Stop(NULL, NULL, 0, 0)): the playing segment is aborted. */
void Dme_stop(DmeEngine *e);
/* GUID_PerfMasterGrooveLevel: added to the command track's groove level when a pattern is picked. */
void Dme_setMasterGroove(DmeEngine *e, int offset);

/* Renders frames of 16-bit stereo at the engine's rate, advancing the music. */
void Dme_render(DmeEngine *e, short *out, int frames);

/* The next notification of a type the caller asked for (types: bit 1 << DME_*), 0 if none. */
void Dme_setNotificationTypes(DmeEngine *e, unsigned int types);
int Dme_takeNotification(DmeEngine *e, DmeNotification *out);
/* Called (from the rendering thread) when notifications arrive. */
void Dme_setNotificationCallback(DmeEngine *e, void (*callback)(void *ctx), void *ctx);

/* Writes every note and notification to log, in record_dmusic's format (tools/music), for the checkers. */
void Dme_setLog(DmeEngine *e, FILE *log);

/* The performance's time, in ticks. */
long Dme_time(DmeEngine *e);

#endif
