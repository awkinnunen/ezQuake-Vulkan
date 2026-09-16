#!/usr/bin/env bash
# OpenAI Codex. Build pinned native Unix dependencies; no service is deployed.
set -euo pipefail
prefix=${1:?installation prefix required}
root=$(cd "$(dirname "$0")/.." && pwd)
work=${EZV_DEPS_WORK:-/tmp/ezv-friends-deps}
mkdir -p "$work" "$prefix"
build_dependency() {
    local name=$1 url=$2 checksum=$3
    shift 3
    if [ ! -f "$work/$name.tar.gz" ]; then curl --fail --location --retry 3 "$url" -o "$work/$name.tar.gz"; fi
    echo "$checksum  $work/$name.tar.gz" | sha512sum --check -
    mkdir -p "$work/$name"
    tar -xzf "$work/$name.tar.gz" --strip-components=1 -C "$work/$name"
    cmake -S "$work/$name" -B "$work/$name-build" -G Ninja -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX="$prefix" -DBUILD_SHARED_LIBS=OFF -DCMAKE_PREFIX_PATH="$prefix" "$@"
    cmake --build "$work/$name-build" --parallel 4
    cmake --install "$work/$name-build"
    mkdir -p "$prefix/share/ezv-sources/$name"
    cp "$work/$name.tar.gz" "$prefix/share/ezv-sources/$name/"
    find "$work/$name" -maxdepth 1 -iname '*license*' -exec cp '{}' "$prefix/share/ezv-sources/$name/" \;
}
# Hashes are the source hashes from the repository's pinned vcpkg baseline.
juice_hash=$(awk '/SHA512/{gsub(/\r/, "", $2); print $2}' "$root/vcpkg/ports/libjuice/portfile.cmake")
build_dependency libjuice-1.7.0 https://github.com/paullouisageneau/libjuice/archive/v1.7.0.tar.gz "$juice_hash" -DNO_TESTS=ON -DNO_SERVER=ON
build_dependency ixwebsocket-11.4.6 https://github.com/machinezone/IXWebSocket/archive/v11.4.6.tar.gz de43c240282e34b905444f84eb5825f55e6f4d68dc9c3937318233a3916d1bb6934fb5bbbe9485c6e181e14c968189559b6837447d8f66ccd73d24634f7749d1 -DUSE_TLS=ON -DUSE_OPEN_SSL=ON -DUSE_ZLIB=OFF
