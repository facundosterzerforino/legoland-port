"""compare_gdi_text.py <windows out> <linux out>: test_gdi_text's two reports and pictures side by side: the
measurements line by line, then per picture how many pixels have text on one side only, or differ by more than a
quarter of the text's brightness."""
import glob
import sys


def lum(v):
    r = (v >> 11) & 31
    g = (v >> 5) & 63
    b = v & 31
    return (r * 255 // 31 + g * 255 // 63 + b * 255 // 31) // 3


def main():
    a, b = sys.argv[1], sys.argv[2]
    la = open(a + '.txt').read().splitlines()
    lb = open(b + '.txt').read().splitlines()
    same = sum(1 for x, y in zip(la, lb) if x == y)
    print('measurements: %d of %d lines the same' % (same, max(len(la), len(lb))))
    measure_diffs = [(x, y) for x, y in zip(la, lb) if x.split(', ink')[0] != y.split(', ink')[0]]
    print('  of which the returned height or rectangle differs: %d' % len(measure_diffs))
    for x, y in measure_diffs[:15]:
        print('  W ' + x)
        print('  L ' + y)
    ink_diffs = [(x, y) for x, y in zip(la, lb) if x != y and (x, y) not in measure_diffs]
    print('  only the ink bounds differ: %d' % len(ink_diffs))
    for x, y in ink_diffs[:10]:
        print('  W ' + x)
        print('  L ' + y)
    # the characters: widths and ink offsets
    import re
    cw = {}
    for side, lines in (('W', la), ('L', lb)):
        for line in lines:
            m = re.match(r'font (\d+) char (\d+): width (-?\d+), ink (-?\d+) (-?\d+) (-?\d+) (-?\d+)', line)
            if m:
                cw.setdefault((int(m.group(1)), int(m.group(2))), {})[side] = tuple(int(g) for g in m.groups()[2:])
    wd = sum(1 for v in cw.values() if 'W' in v and 'L' in v and v['W'][0] != v['L'][0])
    print('characters: widths differ for %d of %d' % (wd, len(cw)))
    for key in sorted(cw)[:400]:
        v = cw[key]
        if 'W' in v and 'L' in v and v['W'] != v['L'] and key[1] < 128:
            print('  font %d char %r: W width %d ink %s | L width %d ink %s' % (key[0], chr(key[1]), v['W'][0], v['W'][1:],
                                                                         v['L'][0], v['L'][1:]))
    hist = {}
    for pa in sorted(glob.glob(a + '.*.raw'))[:4]:
        da = open(pa, 'rb').read()
        for i in range(0, len(da), 2):
            v = lum(da[i] | da[i + 1] << 8)
            if v:
                hist[v // 32] = hist.get(v // 32, 0) + 1
    print('Windows text pixel brightness, in eighths:', sorted(hist.items()))
    total_ink = 0
    only = 0
    far = 0
    for pa in sorted(glob.glob(a + '.*.raw')):
        pb = b + pa[len(a):]
        da = open(pa, 'rb').read()
        db = open(pb, 'rb').read()
        for i in range(0, len(da), 2):
            va = lum(da[i] | da[i + 1] << 8)
            vb = lum(db[i] | db[i + 1] << 8)
            if va or vb:
                total_ink += 1
                if (va == 0) != (vb == 0):
                    only += 1
                if abs(va - vb) > 64:
                    far += 1
    print('pictures: %d text pixels; on one side only %d (%.1f%%), differing by more than 64 of 255: %d (%.1f%%)' % (
        total_ink, only, 100.0 * only / max(total_ink, 1), far, 100.0 * far / max(total_ink, 1)))


main()
