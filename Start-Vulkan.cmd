@echo off
setlocal
if "%~1"=="" goto usage
set "ezv_config=Release"
set "ezv_debug="
if /I "%~2"=="Debug" set "ezv_config=Debug"
if /I "%~2"=="Debug" set "ezv_debug=-dev"
set "ezv_exe=%~dp0build-msvc-x64\%ezv_config%\ezquake.exe"
if not exist "%ezv_exe%" (
  echo Build %ezv_config% first. See README.md.
  exit /b 1
)
if not exist "%~1\id1" (
  echo The game-data directory must contain id1.
  exit /b 1
)
pushd "%~1" || exit /b 1
start "" "%ezv_exe%" %ezv_debug% -nohome -basedir . -fullscreen +set vid_renderer 2
popd
exit /b 0
:usage
echo Usage: Start-Vulkan.cmd "C:\Games\nQuake" [Debug]
exit /b 1
