"""Package a built probe plus matching adapter/dependency source, without game data.

New output directory required. Downloads are pinned and hash-verified against the
specified vcpkg baseline's port files. Does not publish anything or modify installs.
"""
from pathlib import Path
import argparse, hashlib, json, re, shutil, urllib.request, zipfile

p=argparse.ArgumentParser()
p.add_argument('--exe',type=Path,required=True)
p.add_argument('--installed',type=Path,required=True,help='vcpkg triplet installation')
p.add_argument('--vcpkg',type=Path,required=True)
p.add_argument('--output',type=Path,required=True)
a=p.parse_args()
root=Path(__file__).resolve().parent
a.output.mkdir(parents=True,exist_ok=False)
binary=a.output/'friends-probe';binary.mkdir()
source=a.output/'source';source.mkdir()
def fetch(url):
    with urllib.request.urlopen(urllib.request.Request(url,headers={'User-Agent':'ezQuake-Vulkan-probe-packager'}),timeout=60) as f:return f.read()
def sha(path):return hashlib.sha256(path.read_bytes()).hexdigest()
for name in ['FriendsProbe.exe']:
    shutil.copy2(a.exe,binary/name)
for name in ['Host.cmd','Join.cmd','Join.ps1','README.md']:
    shutil.copy2(root/name,binary/name)
files=['CMakeLists.txt','vcpkg.json','Build.ps1','Package.py','Test-LivePair.py','Host.cmd','Join.cmd','Join.ps1','README.md','main.cpp','dtls.h','broker.cpp','broker.h','protocol.cpp','protocol.h','.gitignore']
for name in files:shutil.copy2(root/name,source/name)
notices=binary/'licenses';notices.mkdir()
shutil.copy2(root.parents[1]/'LICENSE',notices/'GPL-2.0.txt')
(notices/'GPL-3.0.txt').write_bytes(fetch('https://www.gnu.org/licenses/gpl-3.0.txt'))
deps=source/'dependencies';deps.mkdir()
records=[]
for name,version,url in [
    ('libjuice','1.7.0','https://github.com/paullouisageneau/libjuice/archive/v1.7.0.tar.gz'),
    ('openssl','3.6.0','https://github.com/openssl/openssl/archive/openssl-3.6.0.tar.gz')]:
    port=a.vcpkg/'ports'/name
    expected=re.search(r'SHA512\s+([a-f0-9]{128})', (port/'portfile.cmake').read_text(encoding='utf-8'))
    if expected is None:raise RuntimeError('Missing pinned dependency source hash: '+name)
    data=fetch(url)
    if hashlib.sha512(data).hexdigest()!=expected[1]:raise RuntimeError('Dependency source hash mismatch: '+name)
    archive=deps/(name+'-'+version+'.tar.gz');archive.write_bytes(data)
    shutil.copytree(port,deps/(name+'-vcpkg-port'))
    shutil.copy2(a.installed/'share'/name/'copyright',notices/(name+'.txt'))
    records.append({'name':name,'version':version,'url':url,'sha512':expected[1],'sha256':sha(archive)})
shutil.copytree(notices,source/'licenses')
manifest={'id':'FRIENDS-002','version':json.loads((root/'vcpkg.json').read_text(encoding='utf-8'))['version-string'],'kind':'Standalone transport probe, not gameplay','vcpkgBaseline':'66c0373dc7fca549e5803087b9487edfe3aca0a1','exeSha256':sha(binary/'FriendsProbe.exe'),'sources':{name:sha(source/name) for name in files},'dependencies':records}
(source/'source-manifest.json').write_text(json.dumps(manifest,indent=2)+'\n',encoding='utf-8')
(binary/'source-manifest.json').write_text(json.dumps(manifest,indent=2)+'\n',encoding='utf-8')
for folder,name in [(binary,'ezQuake-Vulkan-friends-probe-windows-x64.zip'),(source,'ezQuake-Vulkan-friends-probe-source.zip')]:
    target=a.output/name
    with zipfile.ZipFile(target,'w',zipfile.ZIP_DEFLATED) as z:
        for file in sorted(folder.rglob('*')):
            if file.is_file():z.write(file,file.relative_to(folder))
    (a.output/(name+'.sha256')).write_text(sha(target)+'  '+name+'\n',encoding='ascii')
    print(name,sha(target))
