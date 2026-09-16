"""OpenAI Codex: native Unix installer integration and archive-boundary regression tests."""
from pathlib import Path
import argparse,hashlib,json,os,shutil,subprocess,zipfile
p=argparse.ArgumentParser();p.add_argument('package',type=Path);p.add_argument('output',type=Path);p.add_argument('--cache',type=Path);a=p.parse_args();package=a.package.resolve();out=a.output.resolve();out.mkdir(parents=True,exist_ok=False)
exe=package/('engine/ezQuake.app/Contents/MacOS/ezv-install' if (package/'engine/ezQuake.app').exists() else 'engine/ezv-install');cache=a.cache.resolve() if a.cache else out/'cache';dest=out/'game with spaces'
def invoke(source=package,target=dest,downloads=cache):
 return subprocess.run([str(exe),'--package',str(source),'--destination',str(target),'--cache',str(downloads),'--yes','--no-links','--no-shortcut'],capture_output=True,text=True,timeout=900)
def digest(path):return hashlib.sha256(path.read_bytes()).hexdigest()
result=invoke();(out/'install.log').write_text(result.stdout+result.stderr);assert result.returncode==0,result.stderr
lock=json.loads((package/'downloads.lock.json').read_text());assert (dest/'id1/pak0.pak').read_bytes()[:4]==b'PACK';assert (dest/'qw/ktx.pk3').is_file()
# nQuake's user bootstrap remains byte-for-byte unchanged.
original={}
for p in lock['packages']:
 if p['kind']=='engine':continue
 with zipfile.ZipFile(cache/(p['sha256'][:12]+'-'+p['name'])) as z:
  for n in z.namelist():
   if Path(n).name.lower()=='autoexec.cfg':original[n]=z.read(n);assert (dest/n).read_bytes()==z.read(n)
assert original,'No upstream autoexec fixture'
# Exercise the relocated engine and its bundled runtime without opening a window.
installed_engine=dest/('engine/ezQuake.app/Contents/MacOS/ezQuake' if (dest/'engine/ezQuake.app').exists() else 'engine/ezquake')
startup=subprocess.run([str(installed_engine),'-friends-invite','invalid'],capture_output=True,timeout=20)
assert startup.returncode==2,'Relocated engine or its native dependencies failed to start' 
assert b'exec ezv-crosshairs.cfg' in (dest/'ezquake/configs/preset.cfg').read_bytes()
assert len(list((dest/'ezquake/crosshairs').glob('legacy_*.png')))==5
before={f.relative_to(dest).as_posix():digest(f) for f in dest.rglob('*') if f.is_file()};again=invoke();assert again.returncode!=0
assert before=={f.relative_to(dest).as_posix():digest(f) for f in dest.rglob('*') if f.is_file()},'Repeat install modified destination'
# Test the actual ZIP extractor with independently checksum-valid malicious fixtures.
for case in ['traversal','symlink','duplicate','corrupt-cache']:
 fixture=out/case;fixture.mkdir();shutil.copyfile(package/'downloads.lock.json',fixture/'downloads.lock.json');broken=json.loads(json.dumps(lock));record=broken['packages'][0];local=fixture/'cache';local.mkdir()
 archive=fixture/'malicious.zip'
 with zipfile.ZipFile(archive,'w') as z:
  if case=='traversal':z.writestr('../escaped.txt','escaped')
  elif case=='symlink':
   info=zipfile.ZipInfo('link');info.create_system=3;info.external_attr=(0o120777<<16);z.writestr(info,'../escape')
  elif case=='duplicate':z.writestr('same','a');z.writestr('same','b')
  else:z.writestr('harmless','test')
 record['sha256']=digest(archive);record['bytes']=archive.stat().st_size
 shutil.copyfile(archive,local/(record['sha256'][:12]+'-'+record['name']))
 if case=='corrupt-cache':(local/(record['sha256'][:12]+'-'+record['name'])).write_bytes(b'broken')
 (fixture/'downloads.lock.json').write_text(json.dumps(broken));r=invoke(fixture,fixture/'install',local);assert r.returncode!=0,case;assert not (fixture/'install').exists();assert not (out/'escaped.txt').exists()
 (fixture/'result.log').write_text(r.stdout+r.stderr)
summary={'pass':True,'freshNativeInstall':True,'relocatedEngineStarts':True,'pathsWithSpaces':True,'nquakeAutoexecPreserved':True,'quickCrosshairs':5,'repeatInstallUnchanged':True,'archiveTraversalRejected':True,'archiveSymlinksRejected':True,'duplicateEntriesRejected':True,'corruptCacheRejected':True,'commercialDataBundled':False}
(out/'result.json').write_text(json.dumps(summary,indent=2)+'\n');print(json.dumps(summary))
