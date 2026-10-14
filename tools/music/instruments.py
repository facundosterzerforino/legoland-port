"""Compares the instruments of tools/test_miniwin/instruments_dmusic.exe's two recordings (Windows' synthesizer and
the portable one): per instrument, the level difference, the decay (time to fall 20 dB from the peak), the pitch
(strongest partial), the brightness (spectral centroid) and how alike the envelopes are; at the end, the mean envelope
difference over all of them (0.1 s steps, down to 60 dB below the peak). Needs numpy.

    python3 tools/music/instruments.py <windows.wav> <portable.wav>
"""
import re
import sys

import numpy as np

RATE = 22050
SLOT = 3 * RATE


def load(p):
    b = open(p, 'rb').read()
    a = np.frombuffer(b[44:44 + (len(b) - 44) // 4 * 4], dtype='<i2').astype(np.float64).reshape(-1, 2)
    return a.mean(axis=1)


def onset(x):
    thr = np.max(np.abs(x[:5 * RATE])) * 0.05
    return int(np.argmax(np.abs(x) > thr))


def env(x, hop=441):
    n = len(x) // hop
    return np.sqrt(np.mean(x[:n * hop].reshape(n, hop) ** 2, axis=1) + 1e-9)


def features(x):
    e = env(x)
    peak = int(np.argmax(e))
    db = 20 * np.log10(e / e[peak])
    after = np.where(db[peak:] < -20)[0]
    decay = after[0] * 0.02 if len(after) else 99
    seg = x[int(0.1 * RATE):int(0.9 * RATE)]
    mag = np.abs(np.fft.rfft(seg * np.hanning(len(seg))))
    f = np.fft.rfftfreq(len(seg), 1 / RATE)
    ok = f > 30
    pitch = f[ok][np.argmax(mag[ok])]
    centroid = np.sum(f * mag ** 2) / np.sum(mag ** 2)
    rms = np.sqrt(np.mean(x[:int(1.75 * RATE)] ** 2))
    return rms, decay, pitch, centroid, e


w = load(sys.argv[1])
p = load(sys.argv[2])
# the instrument names, from the tool's table
here = __file__.replace('\\', '/').rsplit('/', 1)[0]
table = open(here + '/../test_miniwin/instruments_dmusic.c').read()
table = table[table.index('tests[] = {'):table.index('#define TESTS')]
names = re.findall(r'"([^"]+)"\}', table)
ow, op = onset(w), onset(p)
start = int(0.25 * RATE)
w, p = w[ow - start:], p[op - start:]
print('%-24s %6s  %11s  %15s  %13s  %s' % ('instrument', 'dB', 'decay s w/p', 'pitch Hz w/p', 'bright w/p', 'env corr'))
for i, name in enumerate(names):
    a = w[i * SLOT + start:(i + 1) * SLOT]
    b = p[i * SLOT + start:(i + 1) * SLOT]
    if len(a) < SLOT // 2 or len(b) < SLOT // 2:
        break
    rw, dw, pw, cw, ew = features(a)
    rp, dp, pp, cp, ep = features(b)
    n = min(len(ew), len(ep))
    lw, lp = np.log(ew[:n]), np.log(ep[:n])
    corr = np.corrcoef(lw, lp)[0, 1]
    print('%-24s %+6.1f  %5.2f/%5.2f  %7.1f/%7.1f  %6.0f/%6.0f  %5.2f' % (name, 20 * np.log10(rp / rw), dw, dp, pw,
                                                                           pp, cw, cp, corr))

# the score: mean envelope difference, where Windows played the instrument
errors = []
for i in range(len(names)):
    a = w[i * SLOT + start:(i + 1) * SLOT]
    b = p[i * SLOT + start:(i + 1) * SLOT]
    if len(a) < SLOT // 2 or len(b) < SLOT // 2:
        break
    ea, eb = 20 * np.log10(env(a, RATE // 10)), 20 * np.log10(env(b, RATE // 10))
    m = min(len(ea), len(eb))
    ea, eb = ea[:m], eb[:m]
    if ea.max() < 20 * np.log10(300):
        continue
    top = max(ea.max(), eb.max())
    keep = (ea > top - 60) | (eb > top - 60)
    errors.append(np.mean(np.abs(np.maximum(ea, top - 60) - np.maximum(eb, top - 60))[keep]))
print('mean envelope difference: %.2f dB over %d instruments' % (np.mean(errors), len(errors)))
