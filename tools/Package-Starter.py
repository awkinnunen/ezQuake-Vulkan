"""DIST-002/INPUT-005. Package installer source and approved Quick crosshairs."""
from pathlib import Path
import argparse, hashlib, json, shutil, zipfile

parser = argparse.ArgumentParser()
parser.add_argument('--output', type=Path, required=True)
args = parser.parse_args()
repo = Path(__file__).resolve().parent.parent
source = repo / 'dist/windows/starter'
lock = json.loads((source / 'downloads.lock.json').read_text(encoding='utf-8-sig'))
name = 'ezQuake-Vulkan-Starter-' + lock['version']
args.output.mkdir(parents=True, exist_ok=True)
zip_path = args.output / (name + '.zip')
if zip_path.exists():
    raise SystemExit('Output already exists; use a new version or directory.')
allowed = {'Common.ps1', 'Install.ps1', 'Install.cmd', 'Launch.ps1', 'Start.cmd',
           'Diagnose.cmd', 'README.txt', 'downloads.lock.json'}
actual = {p.name for p in source.iterdir() if p.is_file()}
assert actual == allowed, (actual - allowed, allowed - actual)
files = {p.name: p.read_bytes() for p in source.iterdir() if p.is_file()}
notice = (repo / 'provenance/crosshairs.json').read_bytes()
images = json.loads(notice)['files']
profiles = ['qw/ezv-wasd.cfg', 'qw/ezv-sdfe.cfg', 'qw/ezv-crosshairs.cfg', *images]
payload = {n: (repo / 'profiles' / n).read_bytes() for n in profiles}
for filename, record in images.items():
    assert hashlib.sha256(payload[filename]).hexdigest() == record['sha256'], filename
files.update({'profiles/' + n: b for n, b in payload.items()})
files['crosshairs.json'] = notice
files['LICENSE'] = (repo / 'LICENSE').read_bytes()
files['SOURCE.txt'] = (
    'This archive contains the complete PowerShell installer source.\n'
    'Repository: https://github.com/awkinnunen/ezQuake-Vulkan\n'
    'Engine binary and matching source: https://github.com/awkinnunen/ezQuake-Vulkan/releases/tag/v0.1.0-beta.1\n'
    'Includes five user-authorized legacy crosshair PNGs and Quick keyboard profiles.\n'
    'Image attribution and hashes: crosshairs.json. No new image license is asserted.\n'
    'Engine binaries and base game data are downloaded separately.\n'
).encode()
manifest = {n: hashlib.sha256(b).hexdigest() for n, b in files.items()}
files['manifest.json'] = (json.dumps({'version': lock['version'], 'files': manifest}, indent=2) + '\n').encode()
with zipfile.ZipFile(zip_path, 'w', zipfile.ZIP_DEFLATED, compresslevel=9) as archive:
    for filename, data in sorted(files.items()):
        archive.writestr(name + '/' + filename, data)
with zipfile.ZipFile(zip_path) as archive:
    assert archive.testzip() is None
digest = hashlib.sha256(zip_path.read_bytes()).hexdigest()
zip_path.with_suffix('.zip.sha256').write_text(digest + '  ' + zip_path.name + '\n')
print(json.dumps({'file': str(zip_path), 'bytes': zip_path.stat().st_size, 'sha256': digest}))

# An existing installation can update just these namespaced files, without reinstalling.
update_name = 'ezQuake-Vulkan-Crosshairs-' + lock['version']
update_path = args.output / (update_name + '.zip')
assert not update_path.exists(), update_path
update = dict(payload)
update['crosshairs.json'] = notice
update['README-CROSSHAIRS.txt'] = (
    'Quick crosshair update ' + lock['version'] + '\n\n'
    'Close the game. Back up any existing same-named files before copying.\n'
    'Copy the qw and ezquake directories beside your installed id1 directory.\n'
    'Copy crosshairs.json alongside them to retain attribution and hashes.\n'
    'No config.cfg, autoexec.cfg or graphics presets are included.\n\n'
    'In the game console, run: exec ezv-crosshairs.cfg\n'
    'This restores only crosshair appearance and the weapon-change hook.\n'
    'To retain it, run: cfg_save\n'
    'Adjust size with crosshairsize (legacy default 2.5).\n'
    'Reloading Quick WASD/ESDF also loads the original crosshairs.\n'
    'Selecting a Quick layout intentionally replaces its keyboard bindings.\n'
    'The LG crosshair hides while firing through Quick aliases; axe is hidden.\n\n'
    'Images: user legacy installation; original authors not established.\n'
    'Inclusion explicitly requested by the user. Integration: OpenAI Codex.\n'
    'No new image license or authorship is asserted. See crosshairs.json.\n'
).encode()
update['manifest.json'] = (json.dumps({'version': lock['version'], 'files': {
    n: hashlib.sha256(b).hexdigest() for n, b in update.items()}}, indent=2) + '\n').encode()
with zipfile.ZipFile(update_path, 'w', zipfile.ZIP_DEFLATED, compresslevel=9) as archive:
    for filename, data in sorted(update.items()):
        archive.writestr(update_name + '/' + filename, data)
digest = hashlib.sha256(update_path.read_bytes()).hexdigest()
update_path.with_suffix('.zip.sha256').write_text(digest + '  ' + update_path.name + '\n')
print(json.dumps({'file': str(update_path), 'bytes': update_path.stat().st_size, 'sha256': digest}))
