#!/bin/sh
# OpenAI Codex. Installs to a fresh user-owned directory; never requires sudo.
set -eu
base=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
if [ -d "$base/engine/ezQuake.app" ]; then
    "$base/engine/ezQuake.app/Contents/MacOS/ezv-install" --package "$base" "$@"
else
    "$base/engine/ezv-install" --package "$base" "$@"
fi
