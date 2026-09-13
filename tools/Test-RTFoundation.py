"""RT-00/01 boundary tests, OpenAI Codex. Does not certify ray-traced pixels."""
from pathlib import Path
import ctypes, json, shutil, subprocess, tempfile

root=Path(__file__).resolve().parent.parent
(root/'cache').mkdir(exist_ok=True)
package=root/'runtime/rt-test-release'
exe=package/'ezquake-rt-harness.exe'
runroot=Path(tempfile.mkdtemp(prefix='rt-foundation-',dir=root/'cache'))
results=[]
def run(name,args,code,contains):
    done=subprocess.run([str(exe),*map(str,args)],capture_output=True,text=True,timeout=60)
    (runroot/(name+'.log')).write_text(done.stdout+done.stderr)
    assert done.returncode==code,(name,done.returncode,done.stdout,done.stderr)
    assert contains in done.stdout,(name,done.stdout)
    results.append(dict(test=name,exit=code,status='passed'))
runtime=package/'rt-runtime'
run('valid-package',[runtime,'--check-package'],0,'26 required exports verified')
run('wrong-device',[runtime,'--reject-device'],0,'missing selected UUID rejected 16 times')
base=json.loads((runtime/'manifest.json').read_text())
fixture=runroot/'runtime'
shutil.copytree(runtime,fixture)
manifest=fixture/'manifest.json'

bad=dict(base,headerSHA256='0'*64)
manifest.write_text(json.dumps(bad))
run('wrong-header',[fixture,'--check-package'],3,'header/ABI mismatch')
bad=dict(base,extensionVersion=999)
manifest.write_text(json.dumps(bad))
run('wrong-extension',[fixture,'--check-package'],3,'extension version mismatch')
bad=dict(base,schema=999)
manifest.write_text(json.dumps(bad))
run('wrong-schema',[fixture,'--check-package'],3,'manifest schema')
bad=dict(base,files=dict(base['files']))
del bad['files'][next(k for k in bad['files'] if k.startswith('shaders/'))]
manifest.write_text(json.dumps(bad))
run('omitted-shader',[fixture,'--check-package'],3,'manifest lacks required shader')
manifest.write_text(json.dumps(base))
shader=next((fixture/'shaders').glob('*.spv'))
original=shader.read_bytes()
shader.write_bytes(original[:-4]+b'FAIL')
run('corrupt-shader',[fixture,'--check-package'],3,'SHA-256 mismatch')
shader.unlink()
run('missing-shader',[fixture,'--check-package'],3,'runtime file missing')
shader.write_bytes(original)
bad=dict(base,files=dict(base['files']))
bad['files']['../outside.dll']='0'*64
manifest.write_text(json.dumps(bad))
run('invalid-file-path',[fixture,'--check-package'],3,'runtime file missing')
manifest.write_text('{ broken json')
run('invalid-manifest',[fixture,'--check-package'],3,'manifest missing or invalid')
manifest.unlink()
run('missing-manifest',[fixture,'--check-package'],3,'manifest missing or invalid')

library=ctypes.CDLL(str(runtime/'RayTracedGL1.dll'))
library.rgEzqGetApiVersion.restype=ctypes.c_uint32
library.rgEzqSelectDevice.argtypes=[ctypes.c_void_p]
library.rgEzqSelectDevice.restype=ctypes.c_uint32
assert library.rgEzqGetApiVersion()==1 and library.rgEzqSelectDevice(None)==0
results.append(dict(test='extension-null-selection',status='passed'))
library.rgCreateInstance.argtypes=[ctypes.c_void_p,ctypes.c_void_p]
library.rgCreateInstance.restype=ctypes.c_int
output=ctypes.c_void_p(1234)
assert library.rgCreateInstance(None,ctypes.byref(output))==3 and output.value is None
assert library.rgCreateInstance(None,None)==3
results.append(dict(test='null-create-arguments',status='passed'))
report=dict(author='OpenAI Codex',scope='RT-00/01 package, ABI and rejection boundaries',tests=results,
    rayTracedFramesVerified=False,engineIntegrationVerified=False,evidenceDirectory=runroot.relative_to(root).as_posix())
(root/'cache/rt-foundation-results.json').write_text(json.dumps(report,indent=2)+'\n')
print(f'PASS: {len(results)} boundary tests; actual RT image still requires the RTX 3060.')
