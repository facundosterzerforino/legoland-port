"""Packs a release of the Windows port from the clang-cl build (ROADMAP.md phase 7):

    dist/LEGOLAND-port-<date>-<commit>.zip          LEGOLAND/LEGOLAND.exe, README.txt, licenses/
    dist/LEGOLAND-port-<date>-<commit>-symbols.zip  legoland.pdb, for reading crash reports

No game data goes in: the first start builds the data folder from the player's CD (port/port_setup.h).

    cmake --build build-clang-x86 && python3 tools/release/make_release.py
"""
import datetime
import os
import subprocess
import sys
import zipfile

ROOT = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
BUILD = os.path.join(ROOT, 'build-clang-x86')
REPO = 'https://github.com/facundosterzerforino/legoland-port'


def git(*args):
    return subprocess.run(['git', '-C', ROOT] + list(args), capture_output=True, text=True, check=True).stdout.strip()


def crlf(text):
    return text.replace('\r\n', '\n').replace('\n', '\r\n')


def main():
    exe = os.path.join(BUILD, 'legoland.exe')
    pdb = os.path.join(BUILD, 'legoland.pdb')
    if not os.path.exists(exe):
        print('no %s: build the clang-cl-x86 preset first' % exe)
        return 1
    commit = git('rev-parse', '--short', 'HEAD')
    dirty = git('status', '--porcelain', '--untracked-files=no')
    if dirty:
        print('warning: uncommitted changes; the release says commit %s but has them' % commit)
    newest = max(os.path.getmtime(os.path.join(ROOT, f)) for f in git('ls-files', 'src', 'port', 'miniwin',
                                                                       'extensions', 'CMakeLists.txt').split('
')
                 if os.path.exists(os.path.join(ROOT, f)))
    if os.path.getmtime(exe) < newest:
        print('warning: a source file is newer than the exe: rebuild first')
    version = '%s-%s' % (datetime.date.today().isoformat(), commit)
    readme = open(os.path.join(ROOT, 'tools', 'release', 'README.txt')).read().replace('@VERSION@', version)
    readme = readme.replace('@REPO@', REPO)

    fluid_tag = 'v2.6.1'
    licenses = {
        'licenses/THIRD-PARTY.txt': crlf('''Third-party code built into LEGOLAND.exe, each under its own license:

SDL3 3.4.18 (zlib license): https://www.libsdl.org/ -- SDL3.txt
FluidSynth %s (LGPL-2.1-or-later): https://github.com/FluidSynth/fluidsynth/tree/%s -- FluidSynth.txt
FFmpeg's Indeo 3 and Indeo 5 decoders (LGPL-2.1-or-later): in this port's source, port/ffmpeg/ -- FFmpeg.txt
GothicKit dmusic (MIT, with TinySoundFont, MIT): in this port's source, port/dmusic/ -- dmusic.txt
zlib's blast (zlib license, Mark Adler): in this port's source, port/blast/ -- blast.txt

The port's source, with everything needed to rebuild LEGOLAND.exe (including with other versions of the LGPL
libraries above): %s (commit %s).
''' % (fluid_tag, fluid_tag, REPO, commit)),
        'licenses/SDL3.txt': os.path.join(BUILD, '_deps', 'sdl3-src', 'LICENSE.txt'),
        'licenses/FluidSynth.txt': os.path.join(BUILD, '_deps', 'fluidsynth-src', 'LICENSE'),
        'licenses/FFmpeg.txt': os.path.join(ROOT, 'port', 'ffmpeg', 'COPYING.LGPLv2.1'),
        'licenses/dmusic.txt': os.path.join(ROOT, 'port', 'dmusic', 'LICENSE.md'),
    }
    blast = open(os.path.join(ROOT, 'port', 'blast', 'blast.h')).read()
    licenses['licenses/blast.txt'] = crlf(blast[:blast.index('*/') + 2])

    os.makedirs(os.path.join(ROOT, 'dist'), exist_ok=True)
    name = os.path.join(ROOT, 'dist', 'LEGOLAND-port-%s.zip' % version)
    with zipfile.ZipFile(name, 'w', zipfile.ZIP_DEFLATED, compresslevel=9) as z:
        z.write(exe, 'LEGOLAND/LEGOLAND.exe')
        z.writestr('LEGOLAND/README.txt', crlf(readme))
        for arc, src in sorted(licenses.items()):
            if os.path.exists(src):
                z.writestr('LEGOLAND/' + arc, crlf(open(src, encoding='utf-8', errors='replace').read()))
            else:
                z.writestr('LEGOLAND/' + arc, src)
    symbols = os.path.join(ROOT, 'dist', 'LEGOLAND-port-%s-symbols.zip' % version)
    with zipfile.ZipFile(symbols, 'w', zipfile.ZIP_DEFLATED) as z:
        z.write(pdb, 'legoland.pdb')
    for f in (name, symbols):
        print('%s  %.1f MB' % (os.path.relpath(f, ROOT), os.path.getsize(f) / 2**20))
    return 0


if __name__ == '__main__':
    sys.exit(main())
