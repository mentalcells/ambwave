#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
"""Local release audit/provenance/packaging. Never publishes or flashes."""
import argparse
import gzip
import hashlib
import html
import io
import json
import re
import shutil
import subprocess
import tarfile
import time
import zipfile
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
BUILD = ROOT / 'build'
PATTERNS = {
    'private-key': rb'-----BEGIN (?:RSA |EC |OPENSSH |DSA )?PRIVATE KEY-----',
    'github-token': rb'\b(?:gh[pousr]_[A-Za-z0-9]{36,}|github_pat_[A-Za-z0-9_]{60,})\b',
    'aws-access-key': rb'\b(?:AKIA|ASIA)[A-Z0-9]{16}\b',
    'openai-key': rb'\bsk-(?:proj-|svcacct-)?[A-Za-z0-9_-]{40,}\b',
}
PRIVATE_NAME = re.compile(r'(^|/)(?:\.env(?:\.(?!example$)[^/]*)?|id_rsa|id_ed25519|credentials\.json)$|\.(?:p12|pfx|pem|key)$')


def git(*args):
    return subprocess.check_output(['git', *args], cwd=ROOT)


def digest(data):
    return hashlib.sha256(data).hexdigest()


def files():
    names = git('ls-files', '--cached', '--others', '--exclude-standard', '-z').decode().split('\0')
    return sorted({n for n in names if n and (ROOT / n).is_file() and not n.endswith('.fwsc')})


def version():
    return re.search(r'^#define AMBWAVE_VERSION "([^"]+)"', (ROOT / 'firmware/src/core.h').read_text(), re.M)[1]


def fingerprint():
    # Include every compiler/generator input and the emulator. Documentation and
    # validation reports may be completed after the build without recompilation.
    return digest(b''.join(n.encode() + b'\0' + hashlib.sha256((ROOT / n).read_bytes()).digest()
                          for n in files() if n.startswith(('firmware/', 'assets/', 'tools/', 'web/'))
                          or n in ('build.sh', 'requirements-dev.txt')))


def artifact(kind):
    return BUILD / ('felucca.fwsc' if kind == 'firmware' else 'ambwave-browser/felucca.wasm')


def stamp(kind):
    obj = {'product': 'AmbWave', 'version': version(), 'ambient_poc': 1,
           'source_inputs_sha256': fingerprint(), 'artifact_sha256': digest(artifact(kind).read_bytes()),
           'created_utc': time.strftime('%Y-%m-%dT%H:%M:%SZ', time.gmtime())}
    (BUILD / f'ambwave-{kind}.json').write_text(json.dumps(obj, indent=2) + '\n')
    print(f'Stamped AmbWave {kind} {obj["artifact_sha256"]}')


def findings(data):
    return [label for label, pattern in PATTERNS.items() if re.search(pattern, data)]


def audit(history=False):
    hits = []
    names = files()
    for name in names:
        if PRIVATE_NAME.search(name):
            hits.append({'location': name, 'kind': 'credential-like filename'})
        hits.extend({'location': name, 'kind': k} for k in findings((ROOT / name).read_bytes()))
    blobs = 0
    if history:
        objects = git('rev-list', '--objects', '--all').decode().splitlines()
        # Batch protocol keeps binary blobs out of text parsing and never prints
        # their contents. Findings identify the object/path and pattern only.
        proc = subprocess.Popen(['git', 'cat-file', '--batch'], cwd=ROOT,
                                stdin=subprocess.PIPE, stdout=subprocess.PIPE)
        try:
            for entry in objects:
                oid, _, name = entry.partition(' ')
                proc.stdin.write((oid + '\n').encode()); proc.stdin.flush()
                header = proc.stdout.readline().decode().split()
                size = int(header[2]); data = proc.stdout.read(size); proc.stdout.read(1)
                if header[1] != 'blob':
                    continue
                blobs += 1
                if PRIVATE_NAME.search(name):
                    hits.append({'location': f'{oid[:12]}:{name}', 'kind': 'credential-like filename'})
                hits.extend({'location': f'{oid[:12]}:{name}', 'kind': k} for k in findings(data))
        finally:
            proc.stdin.close(); proc.stdout.close(); proc.wait()
    report = {'files_scanned': len(names), 'historical_blobs_scanned': blobs, 'findings': hits,
              'limits': 'Pattern scan of local reachable refs, not proof of no sensitive data. Remote attachments and refs require review.'}
    BUILD.mkdir(exist_ok=True)
    (BUILD / ('ambwave-public-audit.json' if history else 'ambwave-working-audit.json')).write_text(json.dumps(report, indent=2) + '\n')
    print(json.dumps(report, indent=2))
    if hits:
        raise SystemExit('Review audit findings before packaging; no values have been printed.')


def source_tar(names, release_version):
    raw = io.BytesIO()
    with tarfile.open(fileobj=raw, mode='w') as tar:
        for name in names:
            path = ROOT / name
            if path.is_symlink():
                raise SystemExit(f'Review symlink before release: {name}')
            data = path.read_bytes()
            info = tarfile.TarInfo(f'ambwave-{release_version}/{name}')
            info.size = len(data); info.mode = 0o755 if path.stat().st_mode & 0o111 else 0o644
            tar.addfile(info, io.BytesIO(data))
    return gzip.compress(raw.getvalue(), mtime=0)


def render_markdown(text):
    """Small renderer for the bundled guides; raw HTML is always escaped."""
    def inline(value):
        value = html.escape(value)
        def link(match):
            label, url = match.groups()
            if url == 'LICENSE' or url.startswith('LICENSES/'):
                url = 'downloads/' + url
            elif url in ('QUICK_START.md', 'docs/QUICK_START.md'):
                url = 'guide.html'
            elif url == 'LICENSING.md':
                url = 'credits.html'
            elif not re.match(r'https?://', url):
                url = 'https://github.com/mentalcells/ambwave/blob/ambient-prototype/' + url
            return f'<a href="{url}">{label}</a>'
        value = re.sub(r'\[([^\]]+)\]\(([^)]+)\)', link, value)
        value = re.sub(r'&lt;(https?://[^&]+)&gt;', r'<a href="\1">\1</a>', value)
        value = re.sub(r'`([^`]+)`', r'<code>\1</code>', value)
        return re.sub(r'\*\*([^*]+)\*\*', r'<strong>\1</strong>', value)
    blocks, paragraph, code = [], [], None
    def flush():
        if paragraph:
            blocks.append('<p>' + inline(' '.join(paragraph)) + '</p>'); paragraph.clear()
    lines = iter(text.splitlines())
    table = False
    for line in lines:
        if line.startswith('```'):
            flush()
            if code is None: code = []
            else:
                blocks.append('<pre><code>' + html.escape('\n'.join(code)) + '</code></pre>'); code = None
            continue
        if code is not None:
            code.append(line); continue
        if line.startswith('|'):
            flush()
            if not table: blocks.append('<table>'); table = True
            cells = line.strip('|').split('|')
            if not all(re.fullmatch(r'\s*:?-+:?\s*', cell) for cell in cells):
                blocks.append('<tr>' + ''.join('<td>' + inline(cell.strip()) + '</td>' for cell in cells) + '</tr>')
            continue
        if table: blocks.append('</table>'); table = False
        heading = re.match(r'^(#{1,6}) (.+)', line)
        if heading:
            flush(); n = len(heading[1]); blocks.append(f'<h{n}>' + inline(heading[2]) + f'</h{n}>')
        elif re.match(r'^(?:- |\d+\. )', line):
            flush(); blocks.append('<p>' + inline(line) + '</p>')
        elif not line.strip(): flush()
        else: paragraph.append(line.strip())
    flush()
    if table: blocks.append('</table>')
    return '\n'.join(blocks)


def page(title, body):
    return f'''<!doctype html><html lang="en"><meta charset="utf-8">
<meta name="viewport" content="width=device-width,initial-scale=1"><title>{html.escape(title)} · AmbWave</title>
<style>body{{margin:3rem auto;padding:0 1.5rem;max-width:56rem;background:#101b21;color:#eee9df;font:18px/1.65 system-ui}}a{{color:#a7e5d1}}h1{{font-size:3rem;line-height:1.1}}nav{{display:flex;gap:1.5rem;flex-wrap:wrap}}pre{{white-space:pre-wrap;overflow-wrap:anywhere;background:#1d3039;padding:1rem}}table{{border-collapse:collapse;font-size:.85em}}td{{padding:.4rem;border-bottom:1px solid #48616d}}small{{color:#c2c8c6}}.button{{display:inline-block;border:1px solid;padding:.6rem 1rem;border-radius:.5rem}}code{{font-size:.9em}}</style>
<nav><a href="./">AmbWave</a><a href="try/">Play</a><a href="guide.html">Quick start</a><a href="install.html">Install / recover</a><a href="credits.html">Credits</a></nav>
<main>{body}</main><footer><small>Independent ambient firmware for M-VAVE FM-1, based on Felucca by Leo Kuroshita / Hügelton Instruments. GPL-3.0-only; third-party notices included. No affiliation with M-VAVE.</small></footer></html>'''


def package(candidate):
    names = files()
    dirty = bool(git('status', '--porcelain').strip())
    if dirty and not candidate:
        raise SystemExit('Commit/review the source first, or use --candidate for a clearly marked local snapshot.')
    audit()
    stamps = {}
    for kind in ('firmware', 'browser'):
        stamp_path = BUILD / f'ambwave-{kind}.json'
        if not stamp_path.exists():
            raise SystemExit(f'Missing {kind} provenance; run its AmbWave build wrapper first.')
        obj = json.loads(stamp_path.read_text())
        if obj['source_inputs_sha256'] != fingerprint() or obj['artifact_sha256'] != digest(artifact(kind).read_bytes()):
            raise SystemExit(f'Stale {kind} build; rebuild with the AmbWave wrapper.')
        stamps[kind] = obj
    v = version(); label = v + ('-candidate' if candidate else '')
    out = BUILD / f'ambwave-{label}'
    if out.exists():
        raise SystemExit(f'{out.relative_to(ROOT)} exists; preserve it or move it before repackaging.')
    out.mkdir()
    for name in ('LICENSE', 'LICENSING.md', 'CHANGELOG.md'):
        shutil.copy(ROOT / name, out / name)
    shutil.copytree(ROOT / 'LICENSES', out / 'LICENSES')
    shutil.copy(ROOT / 'assets/samples-cc0/ATTRIBUTION.txt', out / 'ATTRIBUTION.txt')
    shutil.copy(ROOT / 'docs/INSTALLING.md', out / 'INSTALLING.md')
    shutil.copy(ROOT / 'docs/QUICK_START.md', out / 'QUICK_START.md')
    fw_name = f'ambwave-{v}.fwsc'; source_name = f'ambwave-{v}-source.tar.gz'
    shutil.copy(artifact('firmware'), out / fw_name)
    (out / source_name).write_bytes(source_tar(names, v))
    manifest = {'product': 'AmbWave', 'maintainer': 'mentalcells', 'version': v, 'candidate': candidate, 'dirty_snapshot': dirty,
                'git_base': git('rev-parse', 'HEAD').decode().strip(), 'source_archive': source_name,
                'source_archive_sha256': digest((out / source_name).read_bytes()),
                'package_identity': 'FM-1_900', 'builds': stamps,
                'source_url': 'https://github.com/mentalcells/ambwave',
                'publication': 'Prepared locally; no repository visibility or hosting changes made.'}
    (out / 'manifest.json').write_text(json.dumps(manifest, indent=2) + '\n')
    site = out / 'site'; site.mkdir(); (site / 'try').mkdir()
    # Copy an explicit runtime asset list: never ship old emulator experiments.
    for name in ('index.html', 'worklet.js', 'felucca.wasm'):
        shutil.copy(BUILD / 'ambwave-browser' / name, site / 'try' / name)
    demo_html = site / 'try/index.html'
    demo_html.write_text(demo_html.read_text().replace('<nav>', '<nav><a href="../">AmbWave</a><a href="../guide.html">Quick start</a><a href="../credits.html">Local notices</a>'))
    shutil.copytree(ROOT / 'web/emu/fonts', site / 'try/fonts')
    (site / '.nojekyll').touch()
    for target, source in [('guide', ROOT / 'docs/QUICK_START.md'), ('install', ROOT / 'docs/INSTALLING.md'), ('credits', ROOT / 'LICENSING.md')]:
        (site / f'{target}.html').write_text(page(target.title(), render_markdown(source.read_text())))
    # Use the existing Felucca package parser and updater unchanged. Never link
    # the upstream installer as if it installed this distribution.
    import sys
    sys.path.insert(0, str(ROOT / 'web'))
    from make_site import product_of
    raw = (out / fw_name).read_bytes()
    if product_of(raw) != manifest['package_identity'] or b'FELUCCA-LOADER-1' not in raw:
        raise SystemExit('Installer requires the expected FM-1 identity and existing Felucca loader.')
    installer = site / 'install'; installer.mkdir()
    template = (ROOT / 'web/ambwave_install.html').read_text()
    if template.count('/*META*/') != 1:
        raise SystemExit('Installer template needs exactly one metadata marker.')
    metadata = {'version': v, 'candidate': candidate, 'product': manifest['package_identity'],
                'pkg': '../downloads/' + fw_name, 'sha256': digest(raw)}
    (installer / 'index.html').write_text(template.replace('/*META*/', json.dumps(metadata)))
    for name in ('fm1pkg.js', 'fm1ota.js'):
        shutil.copy(ROOT / 'web' / name, installer / name)
    editor = site / 'editor'; editor.mkdir()
    for source, target in [('editor.html', 'index.html'), ('fm1backup.js', 'fm1backup.js'),
                           ('fukiai.ttf', 'fukiai.ttf'), ('FUKIAI-LICENSE.txt', 'FUKIAI-LICENSE.txt')]:
        shutil.copy(ROOT / 'web' / source, editor / target)
    editor_html = editor / 'index.html'
    editor_html.write_text(editor_html.read_text().replace('../installer/', '../install/'))
    # Every download carries notices and corresponding source, including when a
    # user downloads only the firmware from this same directory.
    downloads = site / 'downloads'; downloads.mkdir()
    for name in (fw_name, source_name, 'LICENSE', 'LICENSING.md', 'ATTRIBUTION.txt', 'manifest.json'):
        shutil.copy(out / name, downloads / name)
    shutil.copytree(out / 'LICENSES', downloads / 'LICENSES')
    for base in (out, downloads):
        entries = sorted(p for p in base.rglob('*') if p.is_file() and 'site' not in p.relative_to(base).parts and p.name != 'SHA256SUMS')
        (base / 'SHA256SUMS').write_text(''.join(f'{digest(p.read_bytes())}  {p.relative_to(base).as_posix()}\n' for p in entries))
    body = f'''<h1>AmbWave</h1><p>An ambient instrument for the M-VAVE FM-1. Hold a chord, shape its movement, add space.</p>
<p>{'Local release candidate · ' if candidate else ''}Version {v} · Based on Felucca</p>
<p><a class="button" href="try/">Play in your browser</a> <a class="button" href="guide.html">Start playing</a></p>
<h2>Four tracks. Four controls.</h2><p>TIDE, TEXTURE, CLOUD and scattered plucks. SHAPE, MOTION, DENSITY and SPACE. Direct mixing, held chords and deliberate phrases.</p>
<p>By mentalcells · Try TIDE with Space 0 to hear the dry FM source. Browser audio starts only after you click Switch on. Demo saves stay in this browser.</p>
<h2>Firmware and source</h2><p><a href="downloads/{fw_name}">FM-1 firmware</a> · <a href="downloads/{source_name}">Matching source</a> · <a href="downloads/SHA256SUMS">Checksums</a> · <a href="credits.html">Licences and credits</a></p>
<p><a class="button" href="install/">Install AmbWave in your browser</a></p>
<p>Read <a href="install.html">backup, installation and recovery instructions</a> before installing. Trying the browser demo never flashes a device.</p>
<h2>MVP limits</h2><p>Eight shared voices, shared effects and no complete scene save/recall. Dense layering can steal voices. The browser does not emulate hardware USB, TRS MIDI or user sample slots.</p>
<p><a href="{manifest['source_url']}">Repository</a> · <a href="{manifest['source_url']}/issues">Report a problem</a></p>'''
    (site / 'index.html').write_text(page('Ambient instrument', body))
    with zipfile.ZipFile(BUILD / f'ambwave-{label}.zip', 'w', zipfile.ZIP_DEFLATED) as z:
        for path in sorted(out.rglob('*')):
            if path.is_file():
                z.write(path, f'ambwave-{label}/{path.relative_to(out)}')
    print(f'Prepared {out.relative_to(ROOT)} and build/ambwave-{label}.zip; no publication performed.')


def main():
    ap = argparse.ArgumentParser(description=__doc__); sub = ap.add_subparsers(dest='cmd', required=True)
    p = sub.add_parser('stamp'); p.add_argument('kind', choices=['firmware', 'browser'])
    p = sub.add_parser('audit'); p.add_argument('--history', action='store_true')
    p = sub.add_parser('package'); p.add_argument('--candidate', action='store_true')
    a = ap.parse_args()
    if a.cmd == 'stamp': stamp(a.kind)
    elif a.cmd == 'audit': audit(a.history)
    else: package(a.candidate)


if __name__ == '__main__':
    main()
