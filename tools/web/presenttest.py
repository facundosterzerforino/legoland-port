#!/usr/bin/env python3
"""Runs the miniwin tests built for a browser (tools/test_miniwin/build_web.sh) in a headless browser, with Playwright:

    uv run -q --no-project --with playwright==1.48.0 python tools/web/presenttest.py build-web/tests [--webkit]

test_present: the picture on the screen, and the input, as the game's page has them (the game on a worker's thread:
port/web/README.md). The test picture's four colours must be on the canvas where they were drawn (screenshots of it),
also after the page changed its size (a phone or a tablet turned); then a click, a key and a character from the page
(Module._MiniwinTypeChar, as the game page's text box sends them) must reach the test; it reports the time a frame
takes to show. --webkit: WebKit (Safari's engine) instead of Chromium, as pagetest.py's (the latest Playwright's; see
its notes)."""
import functools
import http.server
import os
import struct
import sys
import threading
import time
import zlib

from playwright.sync_api import sync_playwright


class Handler(http.server.SimpleHTTPRequestHandler):
    """the files, with the headers that make the page cross-origin isolated (threads need it)"""

    def end_headers(self):
        self.send_header('Cross-Origin-Opener-Policy', 'same-origin')
        self.send_header('Cross-Origin-Embedder-Policy', 'require-corp')
        super().end_headers()

    def log_message(self, *args):
        pass


def png_pixels(data):
    """(width, height, rows of (r, g, b)) of an 8-bit RGB or RGBA PNG, as Playwright's screenshots are"""
    pos = 8
    width = height = channels = 0
    packed = b''
    while pos < len(data):
        size, kind = struct.unpack('>I4s', data[pos:pos + 8])
        body = data[pos + 8:pos + 8 + size]
        if kind == b'IHDR':
            width, height, depth, colour = struct.unpack('>IIBB', body[:10])
            if depth != 8 or colour not in (2, 6) or body[12] != 0:
                raise ValueError('not an 8-bit RGB(A) PNG without interlacing')
            channels = 3 if colour == 2 else 4
        elif kind == b'IDAT':
            packed += body
        pos += 12 + size
    raw = zlib.decompress(packed)
    stride = width * channels
    rows = []
    previous = bytearray(stride)
    for y in range(height):
        kind = raw[y * (stride + 1)]
        line = bytearray(raw[y * (stride + 1) + 1:(y + 1) * (stride + 1)])
        for i in range(stride):
            left = line[i - channels] if i >= channels else 0
            up = previous[i]
            corner = previous[i - channels] if i >= channels else 0
            if kind == 1:
                line[i] = (line[i] + left) & 255
            elif kind == 2:
                line[i] = (line[i] + up) & 255
            elif kind == 3:
                line[i] = (line[i] + (left + up) // 2) & 255
            elif kind == 4:
                p = left + up - corner
                pa, pb, pc = abs(p - left), abs(p - up), abs(p - corner)
                line[i] = (line[i] + (left if pa <= pb and pa <= pc else up if pb <= pc else corner)) & 255
        rows.append([tuple(line[x * channels:x * channels + 3]) for x in range(width)])
        previous = line
    return width, height, rows


def check_picture(page, label):
    """the four quarters of the test picture on the canvas: the number of wrong ones"""
    width, height, rows = png_pixels(page.locator('#canvas').screenshot())
    wrong = 0
    for name, fx, fy, want in (('red', .25, .25, (255, 0, 0)), ('green', .75, .25, (0, 255, 0)),
                               ('blue', .25, .75, (0, 0, 255)), ('white', .75, .75, (255, 255, 255))):
        got = rows[int(height * fy)][int(width * fx)]
        ok = all(abs(a - b) <= 24 for a, b in zip(got, want))
        print('%s: %s quarter %s %s' % (label, name, got, 'OK' if ok else 'WRONG (want %s)' % (want,)))
        wrong += 0 if ok else 1
    return wrong


def main():
    args = sys.argv[1:]
    webkit = '--webkit' in args
    args = [a for a in args if a != '--webkit']
    folder = os.path.abspath(args[0])
    server = http.server.ThreadingHTTPServer(('127.0.0.1', 0), functools.partial(Handler, directory=folder))
    threading.Thread(target=server.serve_forever, daemon=True).start()
    url = 'http://127.0.0.1:%d/test_present.html' % server.server_address[1]
    problems = 0
    lines = []
    with sync_playwright() as p:
        browser = p.webkit.launch(headless=True) if webkit else p.chromium.launch(headless=True)
        print('browser:', browser.version, 'WebKit' if webkit else 'Chromium')
        page = browser.new_page(viewport={'width': 800, 'height': 600})
        page.on('console', lambda m: lines.append(m.text))
        page.on('pageerror', lambda e: lines.append('page error: %s' % e))

        def wait_for(text, seconds):
            start = time.time()
            while not any(text in line or 'FAILED' in line for line in lines) and time.time() - start < seconds:
                page.wait_for_timeout(100)  # (the page's events arrive only during a Playwright call)
            return any(text in line for line in lines)

        page.goto(url)
        if not wait_for('present: showing', 120):
            print('the test picture was never shown')
            problems += 1
        else:
            page.wait_for_timeout(1000)
            problems += check_picture(page, '800x600')
            page.set_viewport_size({'width': 1000, 'height': 500})
            page.wait_for_timeout(1500)
            problems += check_picture(page, '1000x500')
        if wait_for('present: waiting for input', 30):
            page.mouse.move(200, 150)
            page.mouse.down()
            page.wait_for_timeout(300)
            page.mouse.up()
            page.keyboard.down('a')
            page.wait_for_timeout(300)
            page.keyboard.up('a')
            page.evaluate('Module._MiniwinTypeChar(122)')  # z
            wait_for('present: done', 30)
            for want in ('present: now 1000x500 pixels', 'input: click at 200,150', 'input: key A', 'input: typed key'):
                ok = any(line.startswith(want) for line in lines)
                print('%s: %s' % (want, 'OK' if ok else 'MISSING'))
                problems += 0 if ok else 1
        else:
            print('the test never waited for the input')
            problems += 1
        for line in lines:
            if line.startswith('present:') or line.startswith('input:') or 'error' in line.lower():
                print('  ' + line)
        try:
            page.wait_for_function('window.testExit !== undefined', timeout=30000)
            code = page.evaluate('window.testExit')
            print('the test ended:', code)
            problems += 0 if code == 0 else 1
        except Exception as e:  # noqa: BLE001
            print('the test didn\'t end: %s' % str(e).splitlines()[0])
            problems += 1
        browser.close()
    server.shutdown()
    print('OK' if problems == 0 else '%d problem(s)' % problems)
    return 0 if problems == 0 else 1


if __name__ == '__main__':
    sys.exit(main())
