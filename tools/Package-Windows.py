"""DIST-001. Build an allowlisted Windows raster package from an exact source commit."""
from pathlib import Path
import argparse,hashlib,json,shutil,subprocess,zipfile
p=argparse.ArgumentParser()
p.add_argument('--binary',type=Path,required=True)
p.add_argument('--dependency-share',type=Path,required=True)
p.add_argument('--output',type=Path,required=True)
p.add_argument('--version',required=True)
p.add_argument('--git',default='git')
a=p.parse_args();repo=Path(__file__).resolve().parent.parent
def git(*args):return subprocess.check_output([a.git,'-C',str(repo),*args]).decode().strip()
def sha(path):return hashlib.sha256(path.read_bytes()).hexdigest()
commit=git('rev-parse','HEAD')
name='ezQuake-Vulkan-'+a.version+'-windows-x64'
folder=a.output/name
folder.mkdir(parents=True,exist_ok=False)
shutil.copyfile(a.binary,folder/'ezquake.exe')
shutil.copytree(repo/'dist/windows/portable',folder,dirs_exist_ok=True)
shutil.copytree(repo/'profiles',folder/'profiles')
shutil.copyfile(repo/'LICENSE',folder/'LICENSE')
shutil.copytree(repo/'docs',folder/'docs')
shutil.copytree(repo/'provenance',folder/'provenance')
(folder/'notices').mkdir()
for source,dest in [('docs/ATTRIBUTION.md','ATTRIBUTION.md'),('docs/history/BUGS.md','BUGS.md'),('docs/VALIDATION.md','VALIDATION.md'),('docs/history/UNIFIED-MENUS.md','UNIFIED-MENUS.md'),('docs/CONTROLS.md','CONTROLS.md')]:
 shutil.copyfile(repo/source,folder/'docs'/dest)
for notice in sorted(a.dependency_share.glob('*/copyright')):
 shutil.copyfile(notice,folder/'notices'/(notice.parent.name+'-copyright.txt'))
assert len(list((folder/'notices').iterdir()))>=15
bootstrap='// DIST-001: lowest-priority first-run defaults, before user config and autoexec.\ncfg_save_onquit 1\n'
bootstrap+=(repo/'profiles/ezquake/presets/graphics/builtin/Balanced.cfg').read_text(encoding='utf-8')+'\n'
bootstrap+=(repo/'profiles/qw/ezv-wasd.cfg').read_text(encoding='utf-8')+'\n'
(folder/'ezv-dist-first-run.cfg').write_text(bootstrap,encoding='utf-8')
files={p.relative_to(folder).as_posix():sha(p) for p in sorted(folder.rglob('*')) if p.is_file()}
assert not any(Path(n).suffix.lower() in {'.pak','.pk3','.pcx','.wav','.mvd','.qwd','.dem','.pdb','.dll'} for n in files)
approved=json.loads((repo/'provenance/crosshairs.json').read_text())['files']
assert {n for n in files if Path(n).suffix.lower()=='.png'} == {'profiles/'+n for n in approved}
assert all(files['profiles/'+n]==v['sha256'] for n,v in approved.items())
assert not any(Path(n).name.lower() in {'config.cfg','autoexec.cfg'} for n in files)
manifest=dict(schema=1,version=a.version,platform='Windows x64',renderer='Vulkan raster',rayTracing=False,
 sourceCommit=commit,sourceURL='https://github.com/awkinnunen/ezQuake-Vulkan/tree/'+commit,
 sourceArchive='ezQuake-Vulkan-'+a.version+'-source.zip',binarySHA256=sha(a.binary),files=files,
 build='MSVC 2022 Release, static dependencies, Vulkan on, RTGL1 off, LTO off',author='OpenAI Codex')
(folder/'manifest.json').write_text(json.dumps(manifest,indent=2)+'\n',encoding='utf-8')
archive=a.output/(name+'.zip')
with zipfile.ZipFile(archive,'w',zipfile.ZIP_DEFLATED,compresslevel=9) as z:
 for f in sorted(folder.rglob('*')):
  if f.is_file():z.write(f,Path(name)/f.relative_to(folder))
source=a.output/manifest['sourceArchive']
subprocess.run([a.git,'-C',str(repo),'archive','--format=zip','--prefix=ezQuake-Vulkan-source/','-o',str(source.resolve()),commit],check=True)
for path in [archive,source]:(path.with_suffix(path.suffix+'.sha256')).write_text(sha(path)+'  '+path.name+'\n')
print(json.dumps(dict(binary=str(archive),source=str(source),sha256=sha(archive),commit=commit)))
