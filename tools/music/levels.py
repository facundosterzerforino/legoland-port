"""Compares the levels of recordings of the music through Windows' DirectMusic and the portable one
(tools/test_miniwin/capture_dmusic.exe): overall RMS and per octave band, as the portable one's difference in dB.

    python3 tools/music/levels.py <windows.wav> <portable.wav> [<windows.wav> <portable.wav>]...
"""
import array
import math
import sys

CENTERS = [62, 125, 250, 500, 1000, 2000, 4000, 8000]


def load(path):
    """16-bit stereo PCM after a 44-byte header, as capture_dmusic writes it: (rate, mono samples)."""
    data = open(path, 'rb').read()
    rate = int.from_bytes(data[24:28], 'little')
    a = array.array('h')
    a.frombytes(data[44:44 + (len(data) - 44) // 4 * 4])
    return rate, [(a[i] + a[i + 1]) * 0.5 for i in range(0, len(a), 2)]


def rms(x):
    return math.sqrt(sum(v * v for v in x) / len(x))


def band(x, rate, f0):
    """RMS through an octave-wide band-pass (RBJ biquad, Q 1.414)."""
    w = 2 * math.pi * f0 / rate
    alpha = math.sin(w) / (2 * 1.414)
    a0 = 1 + alpha
    b0, b2, a1, a2 = alpha / a0, -alpha / a0, -2 * math.cos(w) / a0, (1 - alpha) / a0
    x1 = x2 = y1 = y2 = 0.0
    acc = 0.0
    for v in x:
        y = b0 * v + b2 * x2 - a1 * y1 - a2 * y2
        x2, x1, y2, y1 = x1, v, y1, y
        acc += y * y
    return math.sqrt(acc / len(x))


def main(args):
    if len(args) < 2 or len(args) % 2:
        print(__doc__)
        return 2
    print('%-24s %7s %7s %6s  %s' % ('', 'Windows', 'port', 'dB', ' '.join('%5d' % f for f in CENTERS)))
    for i in range(0, len(args), 2):
        rate, w = load(args[i])
        _, p = load(args[i + 1])
        bands = []
        for f in CENTERS:
            bands.append(20 * math.log10(band(p, rate, f) / band(w, rate, f)) if f < rate / 2 else 0.0)
        print('%-24s %7.0f %7.0f %+6.1f  %s' % (args[i].replace('\\', '/').split('/')[-1], rms(w), rms(p),
                                                20 * math.log10(rms(p) / rms(w)), ' '.join('%+5.1f' % b for b in bands)))
    return 0


if __name__ == '__main__':
    sys.exit(main(sys.argv[1:]))
