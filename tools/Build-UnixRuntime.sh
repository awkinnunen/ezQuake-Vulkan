#!/usr/bin/env bash
# OpenAI Codex. Ubuntu 24.04 baseline; pinned SDL3/OpenSSL for native Friends.
set -euo pipefail
prefix=${1:?prefix required}
root=$(cd "$(dirname "$0")/.." && pwd)
work=${EZV_DEPS_WORK:-/tmp/ezv-friends-deps}
mkdir -p "$prefix" "$work"
fetch() {
 local name=$1 url=$2 hash=$3
 test -f "$work/$name.tar.gz" || curl -fL --retry 3 "$url" -o "$work/$name.tar.gz"
 echo "$hash  $work/$name.tar.gz" | sha512sum -c -
 mkdir -p "$work/$name" "$prefix/share/ezv-sources/$name"
 tar -xzf "$work/$name.tar.gz" --strip-components=1 -C "$work/$name"
 cp "$work/$name.tar.gz" "$prefix/share/ezv-sources/$name/"
 find "$work/$name" -maxdepth 1 -iname '*license*' -exec cp '{}' "$prefix/share/ezv-sources/$name/" \;
}
hash=$(awk '/SHA512/{gsub(/\r/, "", $2); print $2}' "$root/vcpkg/ports/openssl/portfile.cmake")
fetch openssl-3.6.0 https://github.com/openssl/openssl/archive/openssl-3.6.0.tar.gz "$hash"
if [ ! -f "$prefix/lib/libssl.so.3" ]; then
 (cd "$work/openssl-3.6.0"; ./Configure --prefix="$prefix" --libdir=lib --openssldir=/usr/lib/ssl shared no-tests no-docs; make -j4; make install_sw)
fi
hash=$(awk '/SHA512/{gsub(/\r/, "", $2); print $2}' "$root/vcpkg/ports/sdl3/portfile.cmake")
fetch SDL-3.4.0 https://github.com/libsdl-org/SDL/archive/release-3.4.0.tar.gz "$hash"
cmake -S "$work/SDL-3.4.0" -B "$work/SDL-3.4.0-build" -G Ninja -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX="$prefix" -DSDL_SHARED=ON -DSDL_STATIC=OFF -DSDL_TESTS=OFF -DSDL_TEST_LIBRARY=OFF -DSDL_UNIX_CONSOLE_BUILD=ON -DSDL_X11_XTEST=OFF
cmake --build "$work/SDL-3.4.0-build" --parallel 4
cmake --install "$work/SDL-3.4.0-build"
export CMAKE_PREFIX_PATH="$prefix"
export PKG_CONFIG_PATH="$prefix/lib/pkgconfig:${PKG_CONFIG_PATH:-}"
bash "$root/tools/Build-FriendsDeps.sh" "$prefix"
