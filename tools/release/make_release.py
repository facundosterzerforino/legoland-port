"""Packs releases of the port (ROADMAP.md phases 7-10), from the builds:

    windows   dist/LEGOLAND-port-windows-<date>-<commit>.zip       LEGOLAND/LEGOLAND-port.exe, README.txt, licenses/
              dist/LEGOLAND-port-windows-<date>-<commit>-symbols.zip  legoland.pdb, for reading crash reports
              (build-clang-x86)
    linux     dist/LEGOLAND-port-linux-x86_64-<date>-<commit>.tar.gz  LEGOLAND/legoland, README.txt, licenses/
              dist/LEGOLAND-port-linux-x86_64-<date>-<commit>-symbols.tar.gz  its debug information
              (build-linux-portable: glibc 2.31 and later)
    android   dist/LEGOLAND-port-android-<date>-<commit>.apk, and its README (tools/android/build_apk.sh)
    web       dist/LEGOLAND-port-web-<date>-<commit>.zip           the site: index.html, legoland.js/.wasm, ... (build-web)

No game data goes in: the first start builds the data folder from the player's CD (port/port_setup.h).

    python3 tools/release/make_release.py [windows|linux|android|web|all]
"""
import datetime
import io
import os
import shutil
import subprocess
import sys
import tarfile
import zipfile

sys.path.insert(0, os.path.join(os.path.dirname(os.path.dirname(os.path.abspath(__file__))), 'music'))
import fetch_soundfont  # noqa: E402

ROOT = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
REPO = 'https://github.com/facundosterzerforino/legoland-port'
FLUID_TAG = 'v2.6.1'


def git(*args):
    return subprocess.run(['git', '-C', ROOT] + list(args), capture_output=True, text=True, check=True).stdout.strip()


def crlf(text):
    return text.replace('\r\n', '\n').replace('\n', '\r\n')


def check_fresh(binary, commit):
    if git('status', '--porcelain', '--untracked-files=no'):
        print('warning: uncommitted changes; the release says commit %s but has them' % commit)
    sources = git('ls-files', 'src', 'port', 'miniwin', 'extensions', 'CMakeLists.txt').splitlines()
    newest = max(os.path.getmtime(os.path.join(ROOT, f)) for f in sources if os.path.exists(os.path.join(ROOT, f)))
    if os.path.getmtime(binary) < newest:
        print('warning: a source file is newer than %s: rebuild first' % os.path.relpath(binary, ROOT))


def readme(platform, version):
    name = 'README.txt' if platform == 'windows' else 'README-%s.txt' % platform
    text = open(os.path.join(ROOT, 'tools', 'release', name)).read()
    return text.replace('@VERSION@', version).replace('@REPO@', REPO)


def licenses(build, platform, commit):
    """the third-party licenses of what's built into that platform's program"""
    lines = ['SDL3 3.4.18 (zlib license): https://www.libsdl.org/ -- SDL3.txt',
             'FluidSynth %s (LGPL-2.1-or-later): https://github.com/FluidSynth/fluidsynth/tree/%s -- FluidSynth.txt'
             % (FLUID_TAG, FLUID_TAG),
             'FFmpeg\'s Indeo 3, Indeo 5 and Cinepak decoders (LGPL-2.1-or-later): in this port\'s source, port/ffmpeg/ '
             '-- FFmpeg.txt',
             'GothicKit dmusic (MIT, with TinySoundFont, MIT): in this port\'s source, port/dmusic/ -- dmusic.txt',
             'zlib\'s blast (zlib license, Mark Adler): in this port\'s source, port/blast/ -- blast.txt',
             'zlib\'s puff (zlib license, Mark Adler): in this port\'s source, port/puff/ -- puff.txt']
    files = {
        'licenses/SDL3.txt': os.path.join(build, '_deps', 'sdl3-src', 'LICENSE.txt'),
        'licenses/FluidSynth.txt': os.path.join(build, '_deps', 'fluidsynth-src', 'LICENSE'),
        'licenses/FFmpeg.txt': os.path.join(ROOT, 'port', 'ffmpeg', 'COPYING.LGPLv2.1'),
        'licenses/dmusic.txt': os.path.join(ROOT, 'port', 'dmusic', 'LICENSE.md'),
    }
    text = {}
    blast = open(os.path.join(ROOT, 'port', 'blast', 'blast.h')).read()
    text['licenses/blast.txt'] = blast[:blast.index('*/') + 2]
    puff = open(os.path.join(ROOT, 'port', 'puff', 'puff.h')).read()
    text['licenses/puff.txt'] = puff[:puff.index('*/') + 2]
    if platform != 'windows':
        lines.append('stb_truetype 1.26 (public domain, or MIT): in this port\'s source, port/stb/ -- stb_truetype.txt')
        stb = open(os.path.join(ROOT, 'port', 'stb', 'stb_truetype.h')).read()
        at = stb.rindex('This software is available under 2 licenses')
        text['licenses/stb_truetype.txt'] = stb[stb.rindex('/*', 0, at):]
    if platform != 'windows':
        lines.append('GeneralUser GS 2.0.3, the music\'s instruments (S. Christian Collins, free to use and to '
                     'redistribute): https://www.schristiancollins.com/generaluser -- GeneralUser-GS.txt')
        fetch_soundfont.fetch()
        files['licenses/GeneralUser-GS.txt'] = os.path.join(ROOT, 'soundfont', 'LICENSE.txt')
    if platform == 'web':
        lines.append('coi-serviceworker 0.1.7 (MIT): in this port\'s source, port/web/ -- coi-serviceworker.txt')
        files['licenses/coi-serviceworker.txt'] = os.path.join(ROOT, 'port', 'web', 'coi-serviceworker.LICENSE')
    program = {'windows': 'LEGOLAND-port.exe', 'linux': 'legoland', 'android': 'the app', 'web': 'legoland.wasm'}[platform]
    text['licenses/THIRD-PARTY.txt'] = ('Third-party code built into %s, each under its own license:\n\n%s\n\n'
                                        'The port\'s source, with everything needed to rebuild %s (including with other '
                                        'versions of the LGPL libraries above): %s (commit %s).\n'
                                        % (program, '\n'.join(lines), program, REPO, commit))
    for arc, src in files.items():
        text[arc] = open(src, encoding='utf-8', errors='replace').read()
    return text


def windows(version, commit):
    build = os.path.join(ROOT, 'build-clang-x86')
    exe = os.path.join(build, 'legoland.exe')
    pdb = os.path.join(build, 'legoland.pdb')
    if not os.path.exists(exe):
        print('no %s: build the clang-cl-x86 preset first' % exe)
        return []
    check_fresh(exe, commit)
    name = os.path.join(ROOT, 'dist', 'LEGOLAND-port-windows-%s.zip' % version)
    with zipfile.ZipFile(name, 'w', zipfile.ZIP_DEFLATED, compresslevel=9) as z:
        # not LEGOLAND.exe: Windows' compatibility database knows the 1999 game by that name next to its files
        # (Legoland.icm, Lego.TTF, speech\\balloonz.wav) and applies its old fixes, which crash the port at load
        z.write(exe, 'LEGOLAND/LEGOLAND-port.exe')
        z.writestr('LEGOLAND/README.txt', crlf(readme('windows', version)))
        for arc, text in sorted(licenses(build, 'windows', commit).items()):
            z.writestr('LEGOLAND/' + arc, crlf(text))
    symbols = os.path.join(ROOT, 'dist', 'LEGOLAND-port-windows-%s-symbols.zip' % version)
    with zipfile.ZipFile(symbols, 'w', zipfile.ZIP_DEFLATED) as z:
        z.write(pdb, 'legoland.pdb')
    return [name, symbols]


def add_text(tar, arcname, text, mode=0o644):
    data = text.encode('utf-8')
    info = tarfile.TarInfo(arcname)
    info.size = len(data)
    info.mode = mode
    info.mtime = int(datetime.datetime.now().timestamp())
    tar.addfile(info, io.BytesIO(data))


def linux(version, commit):
    build = os.path.join(ROOT, 'build-linux-portable')
    binary = os.path.join(build, 'legoland')
    if not os.path.exists(binary):
        print('no %s: build the linux-portable preset first' % binary)
        return []
    check_fresh(binary, commit)
    work = os.path.join(ROOT, 'dist', 'linux-work')
    os.makedirs(work, exist_ok=True)
    stripped = os.path.join(work, 'legoland')
    debug = os.path.join(work, 'legoland.debug')
    subprocess.run(['objcopy', '--only-keep-debug', binary, debug], check=True)
    subprocess.run(['objcopy', '--strip-debug', '--add-gnu-debuglink=' + debug, binary, stripped], check=True)
    name = os.path.join(ROOT, 'dist', 'LEGOLAND-port-linux-x86_64-%s.tar.gz' % version)
    with tarfile.open(name, 'w:gz') as t:
        t.add(stripped, 'LEGOLAND/legoland')
        t.add(fetch_soundfont.fetch(), 'LEGOLAND/GeneralUser-GS.sf2')
        add_text(t, 'LEGOLAND/README.txt', readme('linux', version))
        for arc, text in sorted(licenses(build, 'linux', commit).items()):
            add_text(t, 'LEGOLAND/' + arc, text)
    symbols = os.path.join(ROOT, 'dist', 'LEGOLAND-port-linux-x86_64-%s-symbols.tar.gz' % version)
    with tarfile.open(symbols, 'w:gz') as t:
        t.add(debug, 'legoland.debug')
    shutil.rmtree(work)
    return [name, symbols]


def android(version, commit):
    apk = os.path.join(ROOT, 'build-android', 'LEGOLAND-port.apk')
    if not os.path.exists(apk):
        print('no %s: run tools/android/build_apk.sh first' % apk)
        return []
    check_fresh(apk, commit)
    name = os.path.join(ROOT, 'dist', 'LEGOLAND-port-android-%s.apk' % version)
    shutil.copyfile(apk, name)
    text = os.path.join(ROOT, 'dist', 'LEGOLAND-port-android-%s-README.txt' % version)
    lic = licenses(os.path.join(ROOT, 'build-android'), 'android', commit)
    open(text, 'w').write(readme('android', version) + '\n\n' + lic['licenses/THIRD-PARTY.txt'])
    return [name, text]


def web(version, commit):
    build = os.path.join(ROOT, 'build-web')
    page = os.path.join(build, 'legoland.html')
    if not os.path.exists(page):
        print('no %s: build the web preset first' % page)
        return []
    check_fresh(os.path.join(build, 'legoland.wasm'), commit)
    name = os.path.join(ROOT, 'dist', 'LEGOLAND-port-web-%s.zip' % version)
    with zipfile.ZipFile(name, 'w', zipfile.ZIP_DEFLATED, compresslevel=9) as z:
        z.write(page, 'LEGOLAND/index.html')
        for f in ('legoland.js', 'legoland.wasm', 'coi-serviceworker.js'):
            z.write(os.path.join(build, f), 'LEGOLAND/' + f)
        z.write(os.path.join(ROOT, 'port', 'web', 'manifest.webmanifest'), 'LEGOLAND/manifest.webmanifest')
        z.write(fetch_soundfont.fetch(), 'LEGOLAND/GeneralUser-GS.sf2')
        # the page's icons: the original exe's window icon, made from external/legoland.exe (not committed), as for
        # the Android app (tools/android/build_apk.sh)
        exe = os.path.join(ROOT, 'external', 'legoland.exe')
        if os.path.exists(exe):
            for icon, size in (('favicon.png', 64), ('apple-touch-icon.png', 180)):
                out = os.path.join(build, icon)
                subprocess.run([sys.executable, os.path.join(ROOT, 'tools', 'android', 'exe_icon.py'), exe, out,
                                str(size)], check=True)
                z.write(out, 'LEGOLAND/' + icon)
        z.writestr('LEGOLAND/README.txt', readme('web', version))
        for arc, text in sorted(licenses(build, 'web', commit).items()):
            z.writestr('LEGOLAND/' + arc, text)
    return [name]


def main():
    which = sys.argv[1] if len(sys.argv) > 1 else 'windows'
    commit = git('rev-parse', '--short', 'HEAD')
    version = '%s-%s' % (datetime.date.today().isoformat(), commit)
    os.makedirs(os.path.join(ROOT, 'dist'), exist_ok=True)
    made = []
    for platform, make in (('windows', windows), ('linux', linux), ('android', android), ('web', web)):
        if which in (platform, 'all'):
            made += make(version, commit)
    for f in made:
        print('%s  %.1f MB' % (os.path.relpath(f, ROOT), os.path.getsize(f) / 2**20))
    return 0 if made else 1


if __name__ == '__main__':
    sys.exit(main())
