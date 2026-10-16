# The web page

[library:window] The WebAssembly build's page (`cmake --preset web`, ROADMAP.md phase 10): `shell.html` is
Emscripten's shell for legoland.js. The game's files live in the site's private storage in the browser (OPFS,
through Emscripten's WASMFS, mounted at /opfs: port/port_posix.c); the first time, the player chooses the CD's image
and the page copies it there, for the game's setup to install from (port/port_setup_sdl.c), which then removes it.

The game's threads are WebAssembly threads, which need the page to be *cross-origin isolated*: served with
`Cross-Origin-Opener-Policy: same-origin` and `Cross-Origin-Embedder-Policy: require-corp`. Where the host can't
send those headers (GitHub Pages, for one), `coi-serviceworker.js` adds them: a service worker that reloads the page
once, isolated. It is coi-serviceworker v0.1.7 by Guido Zuidhof and contributors, MIT license
(`coi-serviceworker.LICENSE`), from https://github.com/gzuidhof/coi-serviceworker commit 509a799daa6583b83b2a70b4dd1898c7e5a902dd, unchanged
(sha256 e97bbac6017322d48aa54a5bc7ce473c725a4b7376ff245002e0ba71c3dcdd7e).

`node-pre.js` starts the Node.js build (`cmake --preset web-node`), which runs `-port-selftest` on an installed
game's folder. `tools/web/pagetest.py` checks the page in headless Chromium, and runs `-port-selftest` in it.

To host the game: the files of build-web (legoland.html, legoland.js, legoland.wasm, coi-serviceworker.js) on any
static web server, over HTTPS (or localhost).
