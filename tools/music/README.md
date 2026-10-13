# Music tools

[library:music] Tools for replacing Windows' DirectMusic with the portable one in `port/dmusic/`
(ROADMAP.md phase 6, step 3). DirectMusic composes the game's music while it plays: for every bar the style engine
picks one of a pattern's variations at random and maps its notes through the chord track, and a synthesizer plays
the result. Recordings of the sound can't be compared, since two runs of the original already differ; the messages
the performance sends can.

- `record_dmusic.exe <data folder> <out folder> <runs> <master groove> <tempo factor> <segment.sgt>...` plays
  segments through Windows' DirectMusic (on miniwin's DirectSound, silently) with a tool in the performance's graph
  that writes down every message: notes with their final MIDI pitch, curves, patch and controller changes, tempo,
  time signatures, and the segment and beat notifications. Times are in music ticks (768 a beat), so a tempo factor
  above 1 (DirectMusic's master tempo, up to 100) records faster without changing the notes; the notifications that
  depend on real time ("almost end", about a second before the end) need a factor of 1.

  The recordings are derived from the game's music: keep them out of the repository.

Build with `tools/music/build.sh` (after the `build-clang-x86` preset, for SDL3).
