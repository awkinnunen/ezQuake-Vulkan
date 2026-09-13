"""RT-01, OpenAI Codex. Package only redistributable RT test files; no game data."""
from pathlib import Path
import argparse, hashlib, json, shutil

p=argparse.ArgumentParser()
p.add_argument('--configuration',choices=['Debug','Release'],default='Release')
p.add_argument('--library-source',type=Path)
p.add_argument('--library-binary',type=Path)
p.add_argument('--shader-directory',type=Path)
p.add_argument('--output',type=Path)
args=p.parse_args()
root=Path(__file__).resolve().parent.parent
host=root if (root/'src/rt').is_dir() else root/'ezquake-vulkan'
library=args.library_source or root/'rtgl1'
library_binary=args.library_binary or library/'build'/('x64-'+args.configuration)/'RayTracedGL1.dll'
shader_directory=args.shader_directory or root/'build/rt-shaders'
destination=args.output or root/'runtime'/('rt-test-'+args.configuration.lower())
runtime=destination/'rt-runtime'
runtime.mkdir(parents=True,exist_ok=True)
files={'RayTracedGL1.dll':library_binary,
       'BlueNoise_LDR_RGBA_128.ktx2':library/'Tools/BlueNoise_LDR_RGBA_128.ktx2'}
shaders=sorted(shader_directory.glob('*.spv'))
assert len(shaders)==51, 'Pinned shader set must be rebuilt and SPIR-V validated first'
files.update({'shaders/'+s.name:s for s in shaders})
for name,source in files.items():
    target=runtime/name
    target.parent.mkdir(parents=True,exist_ok=True)
    shutil.copyfile(source,target)
config=runtime/'RayTracedGL1.txt'
config.write_text('VulkanValidation\n' if args.configuration=='Debug' else '')
files['RayTracedGL1.txt']=config
for source in library.rglob('*'):
    if source.is_file() and (source.name.lower() in ('license','license.txt','license.md','license.adoc','copying','copying.txt') or 'LICENSES' in source.parts):
        relative=source.relative_to(library)
        if 'build' in relative.parts or '.git' in relative.parts: continue
        target=runtime/'notices'/relative
        target.parent.mkdir(parents=True,exist_ok=True)
        shutil.copyfile(source,target)
        files['notices/'+relative.as_posix()]=target
for name,source in {
    'RTGL1-Tools-README.md':library/'Tools/README.md',
    'ezQuake-LICENSE':host/'LICENSE',
    'SDL3-copyright':host/'build-msvc-x64/vcpkg_installed/x64-windows-static/share/sdl3/copyright',
    'Jansson-copyright':host/'build-msvc-x64/vcpkg_installed/x64-windows-static/share/jansson/copyright',
}.items():
    target=runtime/'notices'/name
    shutil.copyfile(source,target)
    files['notices/'+name]=target
manifest=dict(schema=1,extensionVersion=1,rtgl1Commit='9efa82daf963e1192daaf2a8596446656f113cd2',
    adapterPatch='RT-00-device-uuid-and-RT-02-initialization-cleanup',configuration=args.configuration,dlss=False,
    headerSHA256=hashlib.sha256((host/'src/rt/RTGL1.h').read_bytes()).hexdigest(),
    waterNormal='Uses the pinned library built-in neutral fallback when WaterNormal_n.ktx2 is absent.',
    files={name:hashlib.sha256((runtime/name).read_bytes()).hexdigest() for name in sorted(files)})
(runtime/'manifest.json').write_text(json.dumps(manifest,indent=2)+'\n')
shutil.copyfile(host/'build-msvc-x64'/args.configuration/'ezquake-rt-harness.exe',destination/'ezquake-rt-harness.exe')
(destination/'Test-RTX.cmd').write_text('@echo off\ncd /d "%~dp0"\nezquake-rt-harness.exe "%~dp0rt-runtime" 600 > RTX-test.log 2>&1\nset "rt_result=%errorlevel%"\ntype RTX-test.log\necho.\necho Exit code: %rt_result% (0=submitted frames, 2=unsupported GPU, 3=package error)\necho Please inspect the triangle and send RTX-test.log with your observations.\npause\nexit /b %rt_result%\n')
(destination/'README.txt').write_text('ezQuake RT-01 hardware test\n\nRun Test-RTX.cmd. No Quake assets or Vulkan SDK are needed. An up-to-date\nVulkan-capable RT driver and the Microsoft Visual C++ x64 runtime are required.\nExpected: an orange triangle lit against a blue-grey background. Try resizing\nthe window; Escape closes it. The test normally submits 600 frames.\n\nReturn RTX-test.log and describe what appeared, any flicker, and resizing behavior.\nSuccessful API calls do not establish that the picture is correct. This is a\nstandalone dependency test; ezQuake RT gameplay is not part of this executable.\n\nNo DLSS or replacement material pack is included. Blue noise comes from the\npinned RTGL1 Tools directory; component notices are under rt-runtime/notices.\n')
print(destination)
with (destination/'README.txt').open('a') as readme:
    readme.write('\nSource and build instructions: https://github.com/awkinnunen/ezQuake-Vulkan\nRTGL1 source revision: 9efa82daf963e1192daaf2a8596446656f113cd2\nApply the RTGL1 Windows and ezQuake foundation patches from that repository.\n')
