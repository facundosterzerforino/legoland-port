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

## What the recordings showed (all 30 segments the game plays, 6 runs each at master groove 0 and +1)

Level 1: all 290510 notes have dmusic's pitch. Level 2: all 22868 channel-bars are one of the model's choices.
The model:

1. Without a chord track, the chord is root 12 (C), chord pattern 0x91 (major triad), scale 0xAB5AB5 (major).
2. The style track gives each bar's style. A pattern is picked among the style's patterns whose groove range holds
   the groove level (the command track's plus the master groove), with the groove embellishment; it plays to its
   end (some are two bars) before the next pick, from the style in force then.
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
