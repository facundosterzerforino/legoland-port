"""tools/music/fetch_soundfont.py: GeneralUser GS 2.0.3 (S. Christian Collins; free to use and redistribute, its
license: soundfont/LICENSE.txt), the General MIDI SoundFont the Linux, Android and web releases bring for the music
where there's no Windows gm.dls. Downloaded once into soundfont/ (not committed: 32 MB), from the author's GitHub
repository at a pinned commit, and checked by its SHA-256. tools/release/make_release.py and tools/android/build_apk.sh
use it.

    python3 tools/music/fetch_soundfont.py
"""
import hashlib
import os
import sys
import urllib.request

ROOT = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
COMMIT = '97049183643d5fc5a9322a69c5b09efb667c6c3a'
BASE = 'https://raw.githubusercontent.com/mrbumpy409/GeneralUser-GS/' + COMMIT + '/'
FILES = {
    'GeneralUser-GS.sf2': ('GeneralUser-GS.sf2', '9575028c7a1f589f5770fccc8cff2734566af40cd26ed836944e9a5152688cfe'),
    'LICENSE.txt': ('documentation/LICENSE.txt', None),
}


def path():
    return os.path.join(ROOT, 'soundfont', 'GeneralUser-GS.sf2')


def fetch():
    folder = os.path.join(ROOT, 'soundfont')
    os.makedirs(folder, exist_ok=True)
    for name, (remote, sha) in FILES.items():
        target = os.path.join(folder, name)
        if os.path.exists(target) and (sha is None or hashlib.sha256(open(target, 'rb').read()).hexdigest() == sha):
            continue
        data = urllib.request.urlopen(BASE + remote).read()
        if sha is not None and hashlib.sha256(data).hexdigest() != sha:
            sys.exit('%s: not the expected file (sha256 %s)' % (name, hashlib.sha256(data).hexdigest()))
        open(target, 'wb').write(data)
    return path()


if __name__ == '__main__':
    print(fetch())
