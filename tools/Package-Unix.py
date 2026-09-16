"""OpenAI Codex: self-contained Unix engine + native Starter packages, exact revision."""
from pathlib import Path
import argparse, hashlib, json, os, plistlib, re, shutil, subprocess, tarfile, zipfile, io
p=argparse.ArgumentParser();p.add_argument('--platform',choices=['linux','macos'],required=True);p.add_argument('--arch',required=True);p.add_argument('--binary',type=Path,required=True);p.add_argument('--installer',type=Path,required=True);p.add_argument('--output',type=Path,required=True);p.add_argument('--version',required=True);p.add_argument('--dependency-share',type=Path);p.add_argument('--dependency-sources',type=Path);p.add_argument('--sdk',type=Path);a=p.parse_args()
repo=Path(__file__).resolve().parent.parent
run=lambda *args:subprocess.check_output([str(x) for x in args],text=True).strip()
sha=lambda f:hashlib.sha256(f.read_bytes()).hexdigest()
commit=run('git','-C',repo,'rev-parse','HEAD')
binary_file=a.binary/'Contents/MacOS/ezQuake' if a.platform=='macos' else a.binary
assert commit[:8].encode() in binary_file.read_bytes(),'Engine was not built from this source revision'
name=f'ezQuake-Vulkan-{a.version}-{a.platform}-{a.arch}'
folder=a.output/name;folder.mkdir(parents=True,exist_ok=False);engine=folder/'engine';engine.mkdir()
for n in ['Install.command','Start.command','README.txt']:
 (folder/n).write_text((repo/'dist/unix'/n).read_text(encoding='utf-8-sig'),encoding='utf-8',newline='\n')
 if n.endswith('.command'):(folder/n).chmod(0o755)
shutil.copytree(repo/'profiles',folder/'profiles');shutil.copytree(repo/'docs',engine/'docs');shutil.copytree(repo/'provenance',engine/'provenance')
shutil.copyfile(repo/'dist/windows/starter/downloads.lock.json',folder/'downloads.lock.json')
shutil.copyfile(repo/'licenses/GPL-3.0.txt',engine/'LICENSE');notices=engine/'notices';notices.mkdir();shutil.copyfile(repo/'LICENSE',notices/'GPL-2.0.txt')
if a.dependency_share:
 for f in a.dependency_share.glob('*/copyright'):shutil.copyfile(f,notices/(f.parent.name+'-copyright.txt'))
if a.dependency_sources:
 for f in a.dependency_sources.rglob('*'):
  if f.is_file() and 'license' in f.name.lower():shutil.copyfile(f,notices/(f.parent.name+'-'+f.name))
if a.platform=='macos':
 assert a.sdk
 app=engine/'ezQuake.app';shutil.copytree(a.binary,app,symlinks=True)
 executable=app/'Contents/MacOS/ezQuake';assert executable.is_file()
 setup=executable.with_name('ezv-install');shutil.copyfile(a.installer,setup);setup.chmod(0o755)
 frameworks=app/'Contents/Frameworks';frameworks.mkdir(exist_ok=True)
 # Copy every non-system dynamic dependency, and rewrite only package copies.
 resolved={};pending=[(a.binary/'Contents/MacOS/ezQuake',executable),(a.installer,setup)]
 molten=a.sdk/'lib/libMoltenVK.dylib';assert molten.is_file();shutil.copyfile(molten,frameworks/molten.name);pending.append((molten,frameworks/molten.name))
 def deps(f):return [line.strip().split(' (',1)[0] for line in run('otool','-L',f).splitlines()[1:]]
 def resolve(dep,source):
  candidates=[Path(dep),source.parent/Path(dep).name,a.sdk/'lib'/Path(dep).name]
  if a.dependency_share:candidates.append(a.dependency_share.parent/'lib'/Path(dep).name)
  if dep.startswith('@loader_path/'):candidates.insert(0,source.parent/dep[len('@loader_path/'):])
  for c in candidates:
   if c.is_file():return c.resolve()
  raise RuntimeError('Unresolved packaged library: '+dep)
 while pending:
  original,copy=pending.pop()
  # Existing Xcode ad-hoc signatures are invalid once load commands change.
  subprocess.run(['codesign','--remove-signature',str(copy)],capture_output=True)
  for dep in deps(original):
   if dep.startswith(('/usr/lib/','/System/Library/')):continue
   if dep==str(original) or (copy.suffix=='.dylib' and Path(dep).name==copy.name):continue
   source=resolve(dep,original);library_name=Path(dep).name;target=frameworks/library_name
   if library_name not in resolved:
    resolved[library_name]=source
    if not target.exists():shutil.copyfile(source,target)
    pending.append((source,target))
   else:assert sha(resolved[library_name])==sha(source),'Library basename collision'
   run('install_name_tool','-change',dep,'@rpath/'+library_name,copy)
  if copy.suffix=='.dylib':run('install_name_tool','-id','@rpath/'+copy.name,copy)
  commands=run('otool','-l',copy)
  rpath='@loader_path' if copy.suffix=='.dylib' else '@executable_path/../Frameworks'
  if 'path '+rpath+' ' not in commands:run('install_name_tool','-add_rpath',rpath,copy)
 resources=app/'Contents/Resources/vulkan/icd.d';resources.mkdir(parents=True,exist_ok=True)
 icd=json.loads((a.sdk/'share/vulkan/icd.d/MoltenVK_icd.json').read_text());icd['ICD']['library_path']='../../../Frameworks/libMoltenVK.dylib'
 (resources/'MoltenVK_icd.json').write_text(json.dumps(icd,indent=2)+'\n')
 info=app/'Contents/Info.plist';pl=plistlib.loads(info.read_bytes());pl.update(CFBundleIdentifier='org.ezquake.vulkan',CFBundleExecutable='ezQuake',CFBundleName='ezQuake Vulkan',LSMinimumSystemVersion='11.0')
 pl.setdefault('LSEnvironment',{})['MVK_CONFIG_USE_METAL_ARGUMENT_BUFFERS']='1';info.write_bytes(plistlib.dumps(pl))
 for f in [*frameworks.glob('*.dylib'),setup,executable]:run('codesign','--force','--sign','-',f)
 run('codesign','--force','--sign','-',app);run('codesign','--verify','--deep','--strict',app)
 # SDK license texts accompany the loader and MoltenVK. Preserve source manifest too.
 for f in (a.sdk/'share').rglob('*'):
  if f.is_file() and ('license' in f.name.lower() or 'copyright' in f.name.lower()):
   target=notices/'vulkan-sdk'/f.relative_to(a.sdk/'share');target.parent.mkdir(parents=True,exist_ok=True);shutil.copyfile(f,target)
 (notices/'VULKAN-SDK.txt').write_text('Bundled from LunarG Vulkan SDK '+str(a.sdk.parent.name)+'\nMoltenVK: https://github.com/KhronosGroup/MoltenVK\nLoader: https://github.com/KhronosGroup/Vulkan-Loader\nSDK sources: https://vulkan.lunarg.com/sdk/home\n')
else:
 binaries=[engine/'ezquake',engine/'ezv-install']
 for source,dest in zip([a.binary,a.installer],binaries):shutil.copyfile(source,dest);dest.chmod(0o755)
 libs=engine/'lib';libs.mkdir();pending=list(binaries);copied=set();packages=set()
 system=re.compile(r'^(ld-linux|lib(c|m|pthread|dl|rt|util|resolv|nss_[^.]+)\.so|lib(GL|EGL|GLX|GLdispatch|drm|gbm))')
 while pending:
  f=pending.pop()
  for line in run('ldd',f).splitlines():
   match=re.search(r'^\s*(\S+) => (/\S+) ',line)
   if not match:continue
   soname,src=match.groups()
   if system.match(soname) or soname in copied:continue
   copied.add(soname);target=libs/soname;shutil.copyfile(src,target);pending.append(target)
   owner=subprocess.run(['dpkg-query','-S',str(Path(src).resolve())],capture_output=True,text=True)
   if owner.returncode:owner=subprocess.run(['dpkg-query','-S',src],capture_output=True,text=True)
   if owner.returncode==0:
    package=owner.stdout.split(': ',1)[0].split(':')[0];packages.add(package)
    notice=Path('/usr/share/doc')/package/'copyright'
    if notice.exists():shutil.copyfile(notice,notices/(package+'-copyright.txt'))
  run('patchelf','--set-rpath','$ORIGIN' if f.parent==libs else '$ORIGIN/lib',f)
 # Keep corresponding Ubuntu source archives alongside bundled shared libraries.
 source_dir=a.output/(name+'-dependency-sources');source_dir.mkdir()
 source_packages={}
 for n in sorted(packages):
  package,version=run('dpkg-query','-W','-f=${source:Package} ${source:Version}',n).split()
  source_packages[package]=version
 for package,version in source_packages.items():
  subprocess.run(['apt-get','source','--download-only',package+'='+version],cwd=source_dir,check=True,stdout=subprocess.DEVNULL)
 (notices/'linux-packages.json').write_text(json.dumps({n:run('dpkg-query','-W','-f=${Version}',n) for n in sorted(packages)},indent=2)+'\n')
# No private game files, identities or commercial resources enter the archive.
assert not any(f.suffix.lower() in {'.pak','.pk3','.mvd','.qwd','.dem'} or f.name=='friends.identity' for f in folder.rglob('*'))
manifest={'schema':1,'version':a.version,'platform':a.platform,'architecture':a.arch,'sourceCommit':commit,'friendsProtocol':1,'renderer':'Vulkan raster','rayTracing':False,'combinedBinaryLicense':'GPL-3.0-or-later','author':'OpenAI Codex','files':{f.relative_to(folder).as_posix():sha(f) for f in sorted(folder.rglob('*')) if f.is_file()}}
(folder/'manifest.json').write_text(json.dumps(manifest,indent=2)+'\n')
a.output.mkdir(parents=True,exist_ok=True)
if a.platform=='macos':
 archive=a.output/(name+'.zip');run('ditto','-c','-k','--keepParent',folder,archive)
 dmg=a.output/(name+'.dmg');run('hdiutil','create','-volname','ezQuake Vulkan Test','-srcfolder',folder,'-format','UDZO',dmg)
 artifacts=[archive,dmg]
else:
 archive=a.output/(name+'.tar.gz')
 with tarfile.open(archive,'w:gz') as tar:tar.add(folder,arcname=name)
 artifacts=[archive]
# Corresponding engine source plus protocol submodule and dependency recipes.
source=a.output/(name+'-source.zip')
blob=subprocess.check_output(['git','-C',str(repo),'archive','--format=zip',commit])
with zipfile.ZipFile(source,'w',zipfile.ZIP_DEFLATED) as out:
 with zipfile.ZipFile(io.BytesIO(blob)) as z:
  for item in z.infolist():out.writestr('source/'+item.filename,z.read(item))
 for sub in ['src/qwprot','vcpkg']:
  revision=run('git','-C',repo,'rev-parse','HEAD:'+sub)
  blob=subprocess.check_output(['git','-C',str(repo/sub),'archive','--format=zip',revision])
  with zipfile.ZipFile(io.BytesIO(blob)) as z:
   for item in z.infolist():out.writestr('source/'+sub+'/'+item.filename,z.read(item))
 if a.dependency_sources:
  for f in a.dependency_sources.rglob('*'):
   if f.is_file():out.write(f,'dependency-sources/'+f.relative_to(a.dependency_sources).as_posix())
 if a.platform=='linux':
  for f in source_dir.iterdir():
   if f.is_file():out.write(f,'ubuntu-dependency-sources/'+f.name)
 # vcpkg retains hash-verified upstream source archives; include these for static Mac dependencies.
 if a.platform=='macos':
  for f in (repo/'vcpkg/downloads').glob('*'):
   if f.is_file() and f.name.endswith(('.tar.gz','.tar.xz','.tar.bz2','.tgz','.zip')):out.write(f,'vcpkg-downloads/'+f.name)
 out.writestr('BUILD.txt',f'Exact commit: {commit}\nSee source/docs/UNIX-PORT.md and source/.github/workflows/main.yml.\nPinned vcpkg and qwprot sources are included.\n')
artifacts.append(source)
for f in artifacts:f.with_name(f.name+'.sha256').write_text(sha(f)+'  '+f.name+'\n')
print(json.dumps({'commit':commit,'artifacts':[str(f) for f in artifacts]}))
