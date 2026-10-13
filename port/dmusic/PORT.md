# dmusic in the port

[GothicKit/dmusic](https://github.com/GothicKit/dmusic) (MIT, see `LICENSE.md`; TinySoundFont in `vendor/` is
MIT too), a re-implementation of DirectMusic, vendored at upstream commit `b73b270b94714b8ec8d9d0b2a9f3a5c445da52d1` (the first commit adding this
folder is that commit's files unchanged: `README.md`, `CMakeLists.txt`, `LICENSE.md`, `include/`, `src/`,
`vendor/`, `support/BuildSupport.cmake`).

[library:music] It is the portable replacement for Windows' DirectMusic (ROADMAP.md phase 6, step 3). The port's
changes to it are separate commits on top, each one checked against recordings of what Windows' DirectMusic plays
(`tools/music/`).
