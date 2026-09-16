"""DIST-001. Build an allowlisted Windows raster package from an exact source commit."""
from pathlib import Path
import argparse,hashlib,json,shutil,subprocess,zipfile,io
p=argparse.ArgumentParser()
p.add_argument('--binary',type=Path,required=True)
p.add_argument('--dependency-share',type=Path,required=True)
p.add_argument('--extra-dependency-share',type=Path,action='append',default=[],help='Additional vcpkg share directory, e.g. the Friends dependency build')
p.add_argument('--friends-dependency-source',type=Path,help='Verified probe source/dependencies directory containing matching tarballs and vcpkg ports')
p.add_argument('--output',type=Path,required=True)
p.add_argument('--version',required=True)
p.add_argument('--git',default='git')
a=p.parse_args();repo=Path(__file__).resolve().parent.parent
def git(*args):return subprocess.check_output([a.git,'-C',str(repo),*args]).decode().strip()
def sha(path):return hashlib.sha256(path.read_bytes()).hexdigest()
commit=git('rev-parse','HEAD')
assert not git('status','--porcelain'), 'Commit source changes before packaging an exact revision'
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
for share in [a.dependency_share,*a.extra_dependency_share]:
 for notice in sorted(share.glob('*/copyright')):
  shutil.copyfile(notice,folder/'notices'/(notice.parent.name+'-copyright.txt'))
friends_enabled=b'EZVG1 AUTH ' in a.binary.read_bytes()
if friends_enabled:
 assert a.friends_dependency_source, 'Matching Friends dependency sources are required'
 for dependency_name in ['libjuice','openssl']:
  assert (folder/'notices'/(dependency_name+'-copyright.txt')).is_file(), 'Missing Friends dependency notice: '+dependency_name
 shutil.copyfile(repo/'LICENSE',folder/'notices/GPL-2.0.txt')
 shutil.copyfile(repo/'licenses/GPL-3.0.txt',folder/'LICENSE')
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
 friends=friends_enabled,combinedBinaryLicense='GPL-3.0-or-later' if friends_enabled else 'GPL-2.0-or-later',
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
with zipfile.ZipFile(source,'a',zipfile.ZIP_DEFLATED,compresslevel=9) as z:
 # git archive omits submodule contents; include the exact checked-out protocol source.
 qwprot=repo/'src/qwprot'
 expected=git('rev-parse','HEAD:src/qwprot')
 actual=subprocess.check_output([a.git,'-C',str(qwprot),'rev-parse','HEAD']).decode().strip()
 assert actual==expected, 'qwprot source does not match the engine commit'
 blob=subprocess.check_output([a.git,'-C',str(qwprot),'archive','--format=zip',actual])
 with zipfile.ZipFile(io.BytesIO(blob)) as sub:
  for entry in sub.infolist():
   if not entry.is_dir():z.writestr('ezQuake-Vulkan-source/src/qwprot/'+entry.filename,sub.read(entry))
 if friends_enabled:
  records=json.loads((a.friends_dependency_source.parent/'source-manifest.json').read_text())['dependencies']
  for record in records:
   tarball=a.friends_dependency_source/(record['name']+'-'+record['version']+'.tar.gz')
   assert sha(tarball)==record['sha256'], 'Dependency source hash mismatch'
  for f in sorted(a.friends_dependency_source.rglob('*')):
   if f.is_file():z.write(f,'ezQuake-Vulkan-source/dependency-sources/'+f.relative_to(a.friends_dependency_source).as_posix())
  z.writestr('ezQuake-Vulkan-source/dependency-sources/manifest.json',json.dumps(records,indent=2)+'\n')
 z.writestr('ezQuake-Vulkan-source/BUILD-ARCHIVE.txt',
  'Exact engine commit: '+commit+'\nProtocol submodule sources are included.\n'
  'Before using CMake presets, clone https://github.com/microsoft/vcpkg.git into vcpkg\n'
  'and check out '+git('rev-parse','HEAD:vcpkg')+'. Bootstrap vcpkg as described in README.md.\n'
  'The manifest pins the dependency recipe baseline. Friends dependency sources and port files are included.\n')
for path in [archive,source]:(path.with_suffix(path.suffix+'.sha256')).write_text(sha(path)+'  '+path.name+'\n')
print(json.dumps(dict(binary=str(archive),source=str(source),sha256=sha(archive),commit=commit)))
