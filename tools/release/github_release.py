"""Publishes a release made by make_release.py as a GitHub Release of legoland-port, with its files attached:

    GITHUB_USER=... GITHUB_TOKEN=... python3 tools/release/github_release.py [dist]

The files in dist/ must all be one version (<date>-<commit>, from `make_release.py all`); the commit must already be
pushed. The release's tag is that version, set on that commit; its notes list the commits since the previous release
(by the commit in its tag). A version that already has a release is refused. No game data is in the files.
"""
import json
import os
import re
import subprocess
import sys
import urllib.error
import urllib.parse
import urllib.request

ROOT = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
REPO = 'legoland-port'
PAGE = 'https://facundosterzerforino.github.io/legoland-port/'
NAME = re.compile(r'^LEGOLAND-port-(?:windows|linux-x86_64|android|web)-(\d{4}-\d\d-\d\d-[0-9a-f]+)(?:-symbols|-README)?'
                  r'\.(?:zip|tar\.gz|apk|txt)$')
TYPES = {'.zip': 'application/zip', '.gz': 'application/gzip', '.apk': 'application/vnd.android.package-archive',
         '.txt': 'text/plain'}


def git(*args):
    return subprocess.run(['git', '-C', ROOT] + list(args), capture_output=True, text=True, check=True).stdout.strip()


def api(method, url, body=None, data=None, content_type='application/json'):
    if not url.startswith('https://'):
        url = 'https://api.github.com/repos/%s/%s%s' % (os.environ['GITHUB_USER'], REPO, url)
    if body is not None:
        data = json.dumps(body).encode()
    req = urllib.request.Request(url, data=data, method=method, headers={
        'Authorization': 'Bearer ' + os.environ['GITHUB_TOKEN'], 'Accept': 'application/vnd.github+json',
        'X-GitHub-Api-Version': '2022-11-28', 'Content-Type': content_type})
    try:
        with urllib.request.urlopen(req) as r:
            return r.status, json.loads(r.read() or b'null')
    except urllib.error.HTTPError as e:
        return e.code, json.loads(e.read() or b'null')


def notes(version, commit, previous):
    lines = ['Built from commit %s. **You need your own copy of LEGOLAND** (the CD or its image); no game data is '
             'included. The first start installs the game from it.' % commit, '',
             '| Platform | File |', '| - | - |',
             '| Web | no download: [play in your browser](%s) |' % PAGE,
             '| Windows 10/11 | `LEGOLAND-port-windows-%s.zip`: unzip, start `LEGOLAND-port.exe` |' % version,
             '| Linux x86_64 (glibc 2.31+) | `LEGOLAND-port-linux-x86_64-%s.tar.gz`: unpack, start `legoland` |' % version,
             '| Android 7.0+ | `LEGOLAND-port-android-%s.apk`: install, then choose the CD image |' % version, '',
             'The `-symbols` files are only for reading crash reports.', '']
    if previous:
        log = git('log', '--no-merges', '--format=- %s', '%s..%s' % (previous['commit'], commit))
        lines += ['## Changes since %s' % previous['tag'], '', log or '- (no new commits)']
    else:
        lines += ['## Changes', '', 'The first release published here; the earlier ones went out by hand.']
    return '\n'.join(lines) + '\n'


def main():
    dist = os.path.abspath(sys.argv[1] if len(sys.argv) > 1 else os.path.join(ROOT, 'dist'))
    files = sorted(f for f in os.listdir(dist) if NAME.match(f))
    versions = {NAME.match(f).group(1) for f in files}
    if len(versions) != 1:
        sys.exit('%s: need the files of exactly one version, found %s' % (dist, sorted(versions) or 'none'))
    version = versions.pop()
    platforms = {p for p in ('windows', 'linux', 'android', 'web') for f in files if f.startswith('LEGOLAND-port-' + p)}
    if len(platforms) != 4:
        sys.exit('missing platforms: %s (run make_release.py all)' % sorted({'windows', 'linux', 'android', 'web'} - platforms))
    short = version.rsplit('-', 1)[1]
    commit = git('rev-parse', short + '^{commit}')
    status, _ = api('GET', '/commits/' + commit)
    if status != 200:
        sys.exit('commit %s is not on GitHub yet (HTTP %d): push it first (~/push-port.sh)' % (short, status))
    status, _ = api('GET', '/releases/tags/' + version)
    if status == 200:
        sys.exit('release %s already exists' % version)
    status, latest = api('GET', '/releases/latest')
    previous = None
    if status == 200:
        tag = latest['tag_name']
        try:
            previous = {'tag': tag, 'commit': git('rev-parse', tag.rsplit('-', 1)[1] + '^{commit}')}
        except subprocess.CalledProcessError:
            print('warning: no commit for the previous release %s; its changes are not listed' % tag)
    date, _ = version.rsplit('-', 1)
    status, release = api('POST', '/releases', {
        'tag_name': version, 'target_commitish': commit, 'name': 'LEGOLAND port %s (%s)' % (date, short),
        'body': notes(version, short, previous), 'draft': False, 'prerelease': False, 'make_latest': 'true'})
    if status != 201:
        sys.exit('creating the release failed (HTTP %d): %s' % (status, release))
    upload = release['upload_url'].split('{')[0]
    for f in files:
        ext = os.path.splitext(f)[1]
        with open(os.path.join(dist, f), 'rb') as fh:
            status, asset = api('POST', upload + '?name=' + urllib.parse.quote(f), data=fh.read(),
                                content_type=TYPES.get(ext, 'application/octet-stream'))
        if status != 201:
            sys.exit('uploading %s failed (HTTP %d): %s' % (f, status, asset))
        print('uploaded %s  %.1f MB' % (f, asset['size'] / 2**20))
    print(release['html_url'])
    return 0


if __name__ == '__main__':
    sys.exit(main())
