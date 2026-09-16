"""OpenAI Codex: verify Unix beta manifests and corresponding sources before publishing."""
from pathlib import Path
import sys,json,zipfile,tarfile,hashlib
folder=Path(sys.argv[1]);expected=sys.argv[2];results=[];commits=set();assets=[]
for platform,artifact in [('linux-x86_64','ezQuake-linux-x86_64'),('macos-arm64','ezQuake-macOS-arm64'),('macos-x64','ezQuake-macOS-x64')]:
 d=folder/artifact;prefix='ezQuake-Vulkan-0.3.0-beta.1-'+platform
 for sidecar in d.glob('*.sha256'):
  f=sidecar.with_suffix('');digest=hashlib.sha256(f.read_bytes()).hexdigest()
  assert digest==sidecar.read_text().split()[0],f
  assets += [{'path':str(x),'name':x.name,'bytes':x.stat().st_size,'sha256':hashlib.sha256(x.read_bytes()).hexdigest()} for x in (f,sidecar)]
 if platform.startswith('linux'):
  with tarfile.open(d/(prefix+'.tar.gz')) as z:
   manifest=json.load(z.extractfile(prefix+'/manifest.json'))
   for name,digest in manifest['files'].items():assert hashlib.sha256(z.extractfile(prefix+'/'+name).read()).hexdigest()==digest,name
 else:
  with zipfile.ZipFile(d/(prefix+'.zip')) as z:
   manifest=json.loads(z.read(prefix+'/manifest.json'))
   for name,digest in manifest['files'].items():assert hashlib.sha256(z.read(prefix+'/'+name)).hexdigest()==digest,name
   for component in ['MoltenVK-LICENSE.txt','Vulkan-Loader-LICENSE.txt.txt']:assert len(z.read(prefix+'/engine/notices/'+component))>1000
   for name in ['libMoltenVK.dylib','libvulkan.1.dylib']:assert any(n.endswith('/Frameworks/'+name) for n in z.namelist())
 commits.add(manifest['sourceCommit'])
 assert not any(Path(n).suffix.lower() in ['.pak','.pk3','.mvd','.qwd','.dem'] or Path(n).name=='friends.identity' for n in manifest['files'])
 with zipfile.ZipFile(d/(prefix+'-source.zip')) as z:
  assert manifest['sourceCommit'] in z.read('BUILD.txt').decode()
  assert any(n.startswith('source/src/qwprot/') for n in z.namelist())
  assert any(n.startswith('source/vcpkg/ports/') for n in z.namelist())
  if platform.startswith('macos'):
   for required in ['openssl','libjuice','ixwebsocket','sdl','libsndfile']:
    assert any(n.startswith('vcpkg-downloads/') and required in n.lower() for n in z.namelist()),required
  else:assert any(n.startswith('ubuntu-dependency-sources/') for n in z.namelist())
 results.append({'platform':platform,'manifestFileHashesVerified':len(manifest['files']),'sourceCommit':manifest['sourceCommit'],'matchingDependencySources':True,'commercialDataBundled':False})
assert commits=={expected} and len(assets)==16
result={'pass':True,'sourceCommit':commits.pop(),'platforms':results,'assets':assets}
(folder/'verified-assets.json').write_text(json.dumps(result,indent=2)+'\n');print(json.dumps({k:v for k,v in result.items() if k!='assets'},indent=2))
