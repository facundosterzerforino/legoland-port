"""tools/web/pagetest.py <build-web folder> [<CD image>] [--selftest <installed game> <reports>] [--webkit]: the web
build's page in headless Chromium (Playwright; --webkit: in its WebKit, Safari's engine, for iPhones and iPads),
served locally with the headers that make it cross-origin isolated (as coi-serviceworker.js does elsewhere): it
must load without script errors, be isolated, start the WebAssembly runtime (its thread pool) and ask for the CD's
image; given one, the page's own file input takes it and the copy into the site's storage (OPFS) must finish, the
page then offering to install and play. With --selftest, an installed game is copied into the site's storage
instead and the build's -port-selftest runs in the page; its reports are copied out. The game itself isn't started
(nothing presses Play).

uv run --with playwright==1.48.0 python tools/web/pagetest.py build-web ["LEGOLAND english.iso"]"""
import functools
import http.server
import os
import sys
import tempfile
import threading
import time

from playwright.sync_api import sync_playwright


GAME = None  # the installed game served at /game/ (--selftest)


class Handler(http.server.SimpleHTTPRequestHandler):
    def do_GET(self):
        if GAME is not None and self.path == '/game/__list.json':
            import json
            names = []
            for d, _, files in os.walk(GAME):
                for f in files:
                    names.append(os.path.relpath(os.path.join(d, f), GAME).replace(os.sep, '/'))
            body = json.dumps(sorted(names)).encode()
            self.send_response(200)
            self.send_header('Content-Type', 'application/json')
            self.send_header('Content-Length', str(len(body)))
            self.end_headers()
            self.wfile.write(body)
            return
        if GAME is not None and self.path.startswith('/game/'):
            from urllib.parse import unquote
            path = os.path.join(GAME, unquote(self.path[6:]))
            if os.path.isfile(path):
                data = open(path, 'rb').read()
                self.send_response(200)
                self.send_header('Content-Type', 'application/octet-stream')
                self.send_header('Content-Length', str(len(data)))
                self.end_headers()
                self.wfile.write(data)
                return
        super().do_GET()

    def end_headers(self):
        self.send_header('Cross-Origin-Opener-Policy', 'same-origin')
        self.send_header('Cross-Origin-Embedder-Policy', 'require-corp')
        self.send_header('Cache-Control', 'no-store')
        super().end_headers()

    def log_message(self, *args):
        pass


# WebKit's headless browser here (Linux, WSL) has no sound output: an audio context's clock stands still, and its
# resume() never returns (the page hangs with it). Safari on a Mac, an iPhone or an iPad has one. With --webkit the page
# gets this stand-in for the Web Audio calls SDL makes (SDL_emscriptenaudio.c): its script processor's callback is
# called on the page's thread as the sound's time passes, as a real one is, and the sound goes nowhere.
FAKE_AUDIO = """(() => {
  class FakeBuffer {
    constructor(channels, length, rate) {
      this.numberOfChannels = channels; this.length = length; this.sampleRate = rate; this.duration = length / rate;
      this.data = Array.from({ length: channels }, () => new Float32Array(length));
    }
    getChannelData(c) { return this.data[c]; }
    copyToChannel(source, c, at) { this.data[c].set(source, at || 0); }
  }
  class FakeProcessor {
    constructor(context, size, channels) {
      this.context = context; this.bufferSize = size || 4096; this.channels = channels || 2; this.done = 0;
      this.onaudioprocess = null;
    }
    connect() { this.context.nodes.add(this); }
    disconnect() { this.context.nodes.delete(this); }
  }
  class FakeContext {
    constructor() {
      this.sampleRate = 48000; this.state = 'suspended'; this.destination = {}; this.nodes = new Set();
      this.frames = 0; this.started = 0; this.timer = null; this.onstatechange = null;
    }
    get currentTime() { return this.frames / this.sampleRate; }
    createBuffer(channels, length, rate) { return new FakeBuffer(channels, length, rate); }
    createScriptProcessor(size, inputs, outputs) { return new FakeProcessor(this, size, outputs); }
    resume() {
      if (this.state === 'suspended') {
        this.state = 'running';
        this.started = performance.now() - this.frames * 1000 / this.sampleRate;
        this.timer = setInterval(() => this.tick(), 10);
      }
      return Promise.resolve();
    }
    suspend() { if (this.state === 'running') { clearInterval(this.timer); this.state = 'suspended'; } return Promise.resolve(); }
    close() { clearInterval(this.timer); this.state = 'closed'; return Promise.resolve(); }
    tick() {
      this.frames = Math.floor((performance.now() - this.started) * this.sampleRate / 1000);
      for (const node of Array.from(this.nodes)) {
        while (node.done + node.bufferSize <= this.frames && this.state === 'running') {
          const buffer = new FakeBuffer(node.channels, node.bufferSize, this.sampleRate);
          if (node.onaudioprocess) node.onaudioprocess({ outputBuffer: buffer, playbackTime: node.done / this.sampleRate });
          node.done += node.bufferSize;
        }
      }
    }
    addEventListener() {}
    removeEventListener() {}
  }
  window.AudioContext = FakeContext;
  window.webkitAudioContext = FakeContext;
})();"""


def main():
    global GAME
    args = sys.argv[1:]
    reports = None
    webkit = '--webkit' in args
    args = [a for a in args if a != '--webkit']
    if '--selftest' in args:
        k = args.index('--selftest')
        GAME = os.path.abspath(args[k + 1])
        reports = args[k + 2]
        args = args[:k] + args[k + 3:]
    folder = os.path.abspath(args[0])
    image = args[1] if len(args) > 1 else None
    server = http.server.ThreadingHTTPServer(('127.0.0.1', 0), functools.partial(Handler, directory=folder))
    threading.Thread(target=server.serve_forever, daemon=True).start()
    url = 'http://127.0.0.1:%d/legoland.html' % server.server_address[1]
    problems = 0
    errors = []
    with sync_playwright() as p, tempfile.TemporaryDirectory() as profile:
        if webkit:
            browser = p.webkit.launch_persistent_context(profile, headless=True)
        else:
            browser = p.chromium.launch_persistent_context(profile, headless=True,
                                                           args=['--autoplay-policy=no-user-gesture-required'])
        print('browser:', browser.browser.version if browser.browser else '', 'WebKit' if webkit else 'Chromium')
        if webkit:
            browser.add_init_script(FAKE_AUDIO)
            print('(WebKit here has no sound output: a stand-in audio device, FAKE_AUDIO)')
        page = browser.new_page()
        page.on('pageerror', lambda e: errors.append('page error: %s' % e))
        page.on('console', lambda m: errors.append('console %s: %s' % (m.type, m.text)) if m.type == 'error' else None)
        if os.environ.get('PAGETEST_CONSOLE'):
            started = time.time()
            page.on('console', lambda m: print('%7.2f console %s: %s' % (time.time() - started, m.type, m.text[:300]), flush=True))
        page.goto(url)
        page.wait_for_function("document.getElementById('status').textContent.indexOf('Loading') < 0 || "
                               "!document.getElementById('choose').classList.contains('hidden')", timeout=120000)
        isolated = page.evaluate('window.crossOriginIsolated')
        print('cross-origin isolated:', isolated)
        if not isolated:
            problems += 1
        page.wait_for_selector('#choose:not(.hidden)', timeout=120000)
        print('the page asks for the CD image')
        if image:
            page.set_input_files('#iso', image)
            page.wait_for_selector('#ready:not(.hidden)', timeout=600000)
            print('copied into the site\'s storage; the page offers: %s' % page.inner_text('#play'))
            size = page.evaluate('''async () => {
                const root = await navigator.storage.getDirectory();
                const folder = await root.getDirectoryHandle('LEGOLAND');
                for await (const [name, handle] of folder.entries()) {
                    if (/\\.(iso|bin)$/i.test(name)) return (await handle.getFile()).size;
                }
                return -1; }''')
            print('the copy: %d bytes (the image: %d)' % (size, os.path.getsize(image)))
            if size != os.path.getsize(image):
                problems += 1
        if GAME is not None:
            # the installed game into the site's storage, then the selftest in the page
            count = page.evaluate('''async () => {
                const names = await (await fetch('/game/__list.json')).json();
                const root = await navigator.storage.getDirectory();
                const top = await root.getDirectoryHandle('LEGOLAND', { create: true });
                for (const name of names) {
                    const parts = name.split('/');
                    let dir = top;
                    for (const part of parts.slice(0, -1)) dir = await dir.getDirectoryHandle(part, { create: true });
                    const handle = await dir.getFileHandle(parts[parts.length - 1], { create: true });
                    const out = await handle.createWritable();
                    await out.write(await (await fetch('/game/' + encodeURI(name))).arrayBuffer());
                    await out.close();
                }
                return names.length; }''')
            print("the installed game copied into the site's storage: %d files" % count)
            # started by a click, as Play is: browsers play a page's sound only after one (WebKit's always)
            page.evaluate("window.selftestDone = false; Module.onExit = () => { window.selftestDone = true; }; "
                          "const b = document.createElement('button'); b.id = 'pagetest-start'; b.textContent = 'start'; "
                          "b.style.cssText = 'position:fixed;left:0;top:0;z-index:99999'; "
                          "b.onclick = () => { b.remove(); window.startGame(['-port-selftest']); }; "
                          "document.body.append(b)")
            page.click('#pagetest-start')
            page.wait_for_function('window.selftestDone', timeout=900000)
            files = page.evaluate('''async () => {
                const root = await navigator.storage.getDirectory();
                const top = await root.getDirectoryHandle('LEGOLAND');
                const out = {};
                for await (const [name, handle] of top.entries()) {
                    if (name.startsWith('port-selftest')) out[name] = await (await handle.getFile()).text();
                }
                return out; }''')
            os.makedirs(reports, exist_ok=True)
            for name, text in files.items():
                open(os.path.join(reports, name), 'w').write(text)
            print('reports: %s' % ' '.join(sorted(files)))
            if 'port-selftest.txt' not in files or 'OK: 0 problem(s)' not in files['port-selftest.txt']:
                problems += 1
        browser.close()
    server.shutdown()
    for e in errors:
        print(e)
    problems += len([e for e in errors if e.startswith('page error')])
    print('OK' if problems == 0 else '%d problem(s)' % problems)
    sys.exit(1 if problems else 0)


main()
