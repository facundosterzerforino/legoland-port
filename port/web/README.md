# The web page

[library:window] The WebAssembly build's page (`cmake --preset web`, ROADMAP.md phase 10): `shell.html` is
Emscripten's shell for legoland.js. The game's files live in the site's private storage in the browser (OPFS,
through Emscripten's WASMFS, mounted at /opfs: port/port_posix.c); the first time, the player chooses the CD's image
and the page copies it there, for the game's setup to install from (port/port_setup_sdl.c), which then removes it.

The game runs on a thread of its own, a worker's (Emscripten's PROXY_TO_PTHREAD: main is started on one), and the
page's own thread is left to the page: the drawing, the input, the sound and the storage. The game waits the way it
does on Windows (it blocks: a file, a lock, the next frame), which the page's thread mustn't: WebKit (Safari, and
every browser on an iPhone or an iPad) does every thread's requests to the site's storage on the page's thread, so a
game waiting there for its files waited for itself, and the page hung at its start. So:
- the drawing: SDL's WebGL calls from the game's thread are made on the page's thread (OFFSCREEN_FRAMEBUFFER), into a
  picture of its own that miniwin copies to the canvas after each frame (`emscripten_webgl_commit_frame`, in
  miniwin/src/window.c); `offscreen_resize.js` gives that picture a new size the way WebKit takes it;
- the input: SDL's mouse, finger and pen handlers run on the page's thread, its keyboard, focus and size handlers on
  the game's (they wait in its queue until MiniwinPumpEvents, every frame); the page's typed characters
  (`Module._MiniwinTypeChar`) are passed to the game's thread too; the page keeps the keys from the browser (Tab,
  the arrows) while the game plays, as SDL did;
- the sound: SDL's audio callback, miniwin's mixer, runs on the page's thread; it never waits for the music thread
  (miniwin/src/dmengine.c, Dme_render). The page makes the audio context in the tap on Play, which iPhones and iPads
  need to play any sound (SDL takes it);
- the page's memory may grow to 1 GB, not more: iOS doesn't start a page with threads that may grow further.

The game's threads are WebAssembly threads, which need the page to be *cross-origin isolated*: served with
`Cross-Origin-Opener-Policy: same-origin` and `Cross-Origin-Embedder-Policy: require-corp`. Where the host can't
send those headers (GitHub Pages, for one), `coi-serviceworker.js` adds them: a service worker that reloads the page
once, isolated. It is coi-serviceworker v0.1.7 by Guido Zuidhof and contributors, MIT license
(`coi-serviceworker.LICENSE`), from https://github.com/gzuidhof/coi-serviceworker commit 509a799daa6583b83b2a70b4dd1898c7e5a902dd, unchanged
(sha256 e97bbac6017322d48aa54a5bc7ce473c725a4b7376ff245002e0ba71c3dcdd7e).

`node-pre.js` starts the Node.js build (`cmake --preset web-node`), which runs `-port-selftest` on an installed
game's folder. `tools/web/pagetest.py` checks the page in headless Chromium (or WebKit, `--webkit`), and runs
`-port-selftest` in it; `tools/web/presenttest.py` runs the miniwin tests built for the browser
(`tools/test_miniwin/build_web.sh`): the picture and the input. `manifest.webmanifest` lets a phone or a tablet add
the page to its home screen as an app, full screen (iPhones can't make a page full screen otherwise).

To host the game: the files of build-web (legoland.html, legoland.js, legoland.wasm, coi-serviceworker.js,
manifest.webmanifest) on any static web server, over HTTPS (or localhost).
