/* dmreverb: the reverb DirectMusic's software synthesizer adds by default (its port's Waves reverb: 1 s reverb time,
 * the highs damped), for the portable music (miniwin/src/dmengine.c). A Freeverb-style network (Jezar's public domain
 * design: eight damped comb filters and four allpasses a side), its level and damping fitted to recordings of Windows'
 * synthesizer, instrument by instrument (tools/music/README.md). */
#ifndef MINIWIN_DMREVERB_H
#define MINIWIN_DMREVERB_H

typedef struct DmeReverb DmeReverb;

DmeReverb *DmeReverb_create(unsigned int rate);
void DmeReverb_destroy(DmeReverb *r);
/* Adds the reverb of the stereo 16-bit frames to them. */
void DmeReverb_process(DmeReverb *r, short *frames, int count);

#endif
