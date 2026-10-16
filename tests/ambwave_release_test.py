#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
"""Verify actual release downloads, matching source, notices and local links."""
import hashlib
import html.parser
import json
import subprocess
import sys
import tarfile
import urllib.parse
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
folder = Path(sys.argv[1]).resolve()
manifest = json.loads((folder / 'manifest.json').read_text())
sha = lambda p: hashlib.sha256(p.read_bytes()).hexdigest()
assert manifest['product'] == 'AmbWave'
assert manifest['maintainer'] == 'mentalcells'
assert manifest['package_identity'] == 'FM-1_900'
assert not manifest['dirty_snapshot'] or manifest['candidate']
for base in (folder, folder / 'site/downloads'):
    for line in (base / 'SHA256SUMS').read_text().splitlines():
        expected, name = line.split('  ', 1)
        assert sha(base / name) == expected, name
    for name in ('LICENSE', 'LICENSING.md', 'ATTRIBUTION.txt'):
        assert (base / name).is_file(), name
    for path in (ROOT / 'LICENSES').glob('*.txt'):
        assert (base / 'LICENSES' / path.name).read_bytes() == path.read_bytes()
source = folder / manifest['source_archive']
assert sha(source) == manifest['source_archive_sha256']
with tarfile.open(source) as tar:
    names = tar.getnames()
    for name in names:
        parts = Path(name).parts[1:]
        assert not any(p in ('.git', '.venv', '.toolchains', 'build', 'backups') for p in parts), name
        assert not any(p.startswith('.env') and p != '.env.example' for p in parts), name
    for name in ('firmware/src/ambient.c', 'firmware/src/fm6_core.c',
                 'firmware/src/core.h', 'tools/ambwave_release.py', 'LICENSE', 'LICENSING.md'):
        member = f'ambwave-{manifest["version"]}/{name}'
        assert tar.extractfile(member).read() == (ROOT / name).read_bytes(), member
assert sha(folder / f'ambwave-{manifest["version"]}.fwsc') == manifest['builds']['firmware']['artifact_sha256']
site = folder / 'site'
assert sha(site / 'try/felucca.wasm') == manifest['builds']['browser']['artifact_sha256']
assert 'ambwave-emu-flash-v1' in (site / 'try/index.html').read_text()
assert 'AmbWave' in (site / 'try/index.html').read_text()
installer = (site / 'install/index.html').read_text()
assert '/*META*/' not in installer
assert manifest['builds']['firmware']['artifact_sha256'] in installer
assert f'../downloads/ambwave-{manifest["version"]}.fwsc' in installer
assert 'confirm(' in installer and 'requestMIDIAccess({ sysex: true })' in installer
for name in ('fm1pkg.js', 'fm1ota.js'):
    assert (site / 'install' / name).read_bytes() == (ROOT / 'web' / name).read_bytes()

class Links(html.parser.HTMLParser):
    def __init__(self):
        super().__init__(); self.links = []
    def handle_starttag(self, tag, attrs):
        self.links.extend(value for key, value in attrs if key in ('href', 'src') and value)

count = 0
for path in site.rglob('*.html'):
    parser = Links(); parser.feed(path.read_text())
    for value in parser.links:
        url = urllib.parse.urlsplit(value)
        if url.scheme or url.netloc or not url.path:
            continue
        assert not url.path.startswith('/'), f'Root-relative URL breaks project Pages: {value}'
        target = (path.parent / urllib.parse.unquote(url.path)).resolve()
        assert target.is_relative_to(site), value
        if target.is_dir(): target /= 'index.html'
        assert target.is_file(), f'{path.name}: {value}'
        count += 1
assert (site / 'try/fonts/DotGothic16-subset.woff').is_file()
assert (site / 'try/fonts/OFL.txt').is_file()
print(f'AmbWave release: source, firmware/browser provenance, checksums, notices and {count} local links passed')
