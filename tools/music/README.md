# Music tools

[library:music] Tools for replacing Windows' DirectMusic with the portable one in `port/dmusic/`
(ROADMAP.md phase 6, step 3). DirectMusic composes the game's music while it plays: for every bar the style engine
picks a pattern and, for each of its parts, one of the part's variations at random, maps the notes through the chord
track, and a synthesizer plays the result. Recordings of the sound can't be compared, since two runs of the original
already differ; the messages the performance sends can.

Build with `tools/music/build.sh` (after the `build-clang-x86` preset, for SDL3). The recordings are derived from
the game's music: keep them out of the repository.

## The tools

- `record_dmusic.exe <data folder> <out folder> <runs> <master groove> <tempo factor> <segment.sgt>...` plays
  segments through Windows' DirectMusic (on miniwin's DirectSound, silently) with a tool in the performance's graph
  that writes down every message: notes with their final MIDI pitch, curves, patch and controller changes, tempo,
  time signatures, and the segment and beat notifications. Times are in music ticks (768 a beat), so a tempo factor
  above 1 (DirectMusic's master tempo, up to 100) records faster without changing the notes; the notifications that
  depend on real time ("almost end", about a second before the end) need a factor of 1. The notes are written as
  they are made, ahead of time, so they come before the segment's start notification in the file.
- `check_notes.exe <data folder> <recording>...` (level 1): for every recorded note, the pitch dmusic gives its
  music value under its play mode and the segment's chord must be the one DirectMusic played.
- `check_bars.exe <data folder> <recording>...` (level 2): for every bar, works out from the segment and its styles
  which patterns can play there and, for each of their parts, every variation's notes; DirectMusic's notes on each
  channel must be one of them exactly. Writes the choices found to `<recording>.choices` (level 3, the statistics).
  `DETAIL=<segment>:<bar>:<channel>` prints what was played there and what each variation would play.
- `play_engine.exe <data folder> <out folder> <runs> <master groove> <segment.sgt>...` plays segments through
  miniwin's style engine (`miniwin/src/dmengine.c`, the portable DirectMusic) offline and writes the same format,
  for the same checks.

## What the recordings showed (all 30 segments the game plays, 6 runs each at master groove 0 and +1)

Level 1: all 290510 notes have dmusic's pitch. Level 2: all 24608 channel-bars (3456 bars) are one of the model's
choices.
The model:

1. Without a chord track, the chord is root 12 (C), chord pattern 0x91 (major triad), scale 0xAB5AB5 (major).
2. The style track gives each bar's style. A pattern is picked among the style's patterns whose groove range holds
   the groove level (the command track's plus the master groove), or when none does, comes closest to it (the
   transitions have level 1 patterns only, and play them at master groove +1), with the groove embellishment; it
   plays to its end (some are two bars) before the next pick, from the style in force then.
3. Each part reference picks one of its part's variations. Parts with the same variation lock id play the same
   one: the first part of the group, in the pattern's order, picks among its own enabled variations, and the others
   follow even where they disable it.
4. Several part references can play on one channel (their notes together).
5. A note's time is its grid position (in the part's own time signature: one style has 6 grids a beat) plus its
   offset. A note placed at or past the end of its part isn't played.
6. A style can be named by GUID only (the Inca segments): it is found among the styles the game's directory scan
   loaded.
7. Random picks are uniform: variations among the candidates (2: 49.5/50.5%, 3: 35.7/29.2/35.1%, 5: about 20%
   each), and patterns when two fit (300/264).

## The engine

`miniwin/src/dmengine.c` follows the model, with dmusic for the files and the synthesizer, and
`miniwin/src/dmusic.c` gives the game the DirectMusic objects on top (with `-portable-music`). Checked with
`play_engine.exe` and the same tools, 6 runs of every segment at both master grooves: all 290681 notes have the
right pitch, all 3456 bars are the model's, and the random picks spread like DirectMusic's (2 candidates
46.1/53.9%, 4 candidates 19-20% each and 10-12% for the two rarer; patterns 294/270).

The synthesizer is TinySoundFont on Windows' gm.dls, so the sound differs from Microsoft's: that part is judged by
ear, and by level. `tools/test_miniwin/capture_dmusic.exe <data folder> windows|portable <out.wav> <seconds>
<segment.sgt>...` sets the music up as the game does, plays the segments one after the other and records what reaches
miniwin's music buffer; `levels.py <windows.wav> <portable.wav>...` (pairs) compares the levels, overall and per
octave. The recordings are derived from the game's music: keep them out of the repository.

dmusic follows DLS where TinySoundFont differs: a channel's volume and expression are each squared, and so is the
velocity (DLS's default 40 log10 curves); drum kits and GS variation banks (MSB 8) select their own presets (dmusic
used the LSB alone, so the drums played the piano). With the engine's overall gain, the main theme and the four world
themes (60 s each) are all within 0.8 dB of Windows' DirectMusic, every octave band within about 1.4 dB.

Instrument by instrument: `tools/test_miniwin/instruments_dmusic.exe <data folder> <windows.wav> <portable.wav>` plays
one note of each of the 53 instruments and drum sounds the music uses, every 3 s, through Windows' synthesizer (each
downloaded from its GM collection) and the portable one (gm.dls through dmusic's conversion, the engine's gain and
reverb); `instruments.py <windows.wav> <portable.wav>` (numpy) compares level, decay, pitch, brightness and envelope
per instrument and gives the mean envelope difference. Two differences found that way:

- gm.dls's melodic instruments decay faster the higher the note (a "key number to decay time" connection). dmusic made
  it a modulator, which TinySoundFont ignores, so every note decayed at the instrument's slowest rate: bells, mallets
  and plucked notes rang 2-4 times too long and piled up. It is now SoundFont's per-key generator.
- DirectMusic's synthesizer adds a reverb by default (its port's Waves reverb: 1 s, the highs damped, the tail about
  15 dB under the sound); TinySoundFont has none, and notes with short releases stopped dead.
  `miniwin/src/dmreverb.c` (Freeverb's design) has its level and damping fitted to the recordings.

Mean envelope difference over the 53: 7.5 dB without the reverb (after the decay fix), 3.1 dB with it; every
instrument's level within about 3 dB, the same pitch, envelope correlation 0.8 to 1.0.

## FluidSynth (experiment)

FluidSynth (LGPL) reads DLS itself, following Microsoft's rules (no conversion to a SoundFont).
`tools/test_miniwin/build_fluid.sh` (with `FLUIDSYNTH_DIR`, an unpacked 32-bit release) builds the instrument tool with
FluidSynth as a third synthesizer (a fourth argument, its WAV; `FLUID_GAIN`, `FLUID_REVERB=own|fluid|none`) and the
capture tool with the engine on FluidSynth (`DME_FLUIDSYNTH=1`, `DME_FLUIDSYNTH_GAIN`; `miniwin/src/dmengine.c`
built with `DME_FLUIDSYNTH`). At gain 0.8 with the engine's reverb: mean envelope difference 2.7 dB (TinySoundFont
3.0), envelope correlation 0.97 to 1.0 on almost every instrument, the five themes within 0.8 dB of Windows' level.
The sustained winds come out a few dB louder than with Windows' synthesizer (as with TinySoundFont, a little more).
FluidSynth's own reverb does nothing on gm.dls (no reverb send). By ear (the maintainer, comparing the recordings):
FluidSynth sounds right.
