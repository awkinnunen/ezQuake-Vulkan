"""DIST-002. Package installer source only; game data/binaries are downloaded separately."""
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
files['LICENSE'] = (repo / 'LICENSE').read_bytes()
files['SOURCE.txt'] = (
    'This archive contains the complete PowerShell installer source.\n'
    'Repository: https://github.com/awkinnunen/ezQuake-Vulkan\n'
    'Engine binary and matching source: https://github.com/awkinnunen/ezQuake-Vulkan/releases/tag/v0.1.0-beta.1\n'
    'No engine binaries or game resources are embedded in this Starter.\n'
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
