"""Reads the game's .res volumes the way resource.c does: a directory tree of (child, sibling, is_dir, size, base,
name) records; files are found by directory path + name, case-insensitively."""
import os
import struct


class Volumes:
    def __init__(self, installed):
        self.files = {}  # 'dir\\name' lowercased -> (volume path, base, size)
        vdir = os.path.join(installed, 'volumes')
        for v in sorted(os.listdir(vdir)):
            if v.lower().endswith('.res'):
                self._load(os.path.join(vdir, v))

    def _load(self, path):
        with open(path, 'rb') as f:
            data = f.read()
        dir_offset = struct.unpack_from('<I', data, 0)[0]
        d = data[dir_offset:]

        def walk(off, prefix):
            while True:
                child, sibling, is_dir, size, base = struct.unpack_from('<iiiii', d, off)
                end = d.index(b'\0', off + 20)
                name = d[off + 20:end].decode('latin-1')
                if not is_dir:
                    self.files.setdefault((prefix + name).lower(), (path, base, size))
                if child != -1:
                    walk(child, prefix + name + '\\')
                if sibling == -1:
                    return
                off = sibling

        walk(0, '')

    def lookup(self, path):
        p = path.replace('/', '\\')
        while p.startswith('.\\'):
            p = p[2:]
        return self.files.get(p.lower())

    def read(self, path):
        hit = self.lookup(path)
        if hit is None:
            return None
        vol, base, size = hit
        with open(vol, 'rb') as f:
            f.seek(base)
            return f.read(size)


if __name__ == '__main__':
    import sys
    v = Volumes(sys.argv[1])
    print(len(v.files), 'files')
    for k in sorted(v.files):
        if any(s in k for s in sys.argv[2:]):
            print(k, v.files[k][2])
