param(
    [Parameter(Mandatory=$true)][string]$VcpkgRoot,
    [string]$BuildRoot = "$PSScriptRoot/../../build-friends-probe",
    [ValidateSet('Debug','Release')][string]$Configuration = 'Release'
)
# FRIENDS-002, OpenAI Codex. Run in an x64 Visual Studio developer shell.
$ErrorActionPreference = 'Stop'
$toolchain = Join-Path $VcpkgRoot 'scripts/buildsystems/vcpkg.cmake'
if (!(Test-Path -LiteralPath $toolchain)) { throw 'VcpkgRoot must contain the vcpkg CMake toolchain.' }
& cmake -S $PSScriptRoot -B $BuildRoot -G 'Visual Studio 17 2022' -A x64 "-DCMAKE_TOOLCHAIN_FILE=$toolchain" -DVCPKG_TARGET_TRIPLET=x64-windows-static
if ($LASTEXITCODE) { throw 'Friends probe configuration failed.' }
& cmake --build $BuildRoot --config $Configuration --parallel 4
if ($LASTEXITCODE) { throw 'Friends probe build failed.' }
& ctest --test-dir $BuildRoot -C $Configuration --output-on-failure
if ($LASTEXITCODE) { throw 'Friends probe tests failed.' }
