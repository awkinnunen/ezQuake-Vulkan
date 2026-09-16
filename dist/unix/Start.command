#!/bin/sh
# OpenAI Codex. Installed game launcher; arguments remain separate argv entries.
set -eu
base=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
cd "$base"
if [ -d "$base/engine/ezQuake.app" ]; then
    exec "$base/engine/ezQuake.app/Contents/MacOS/ezQuake" -nohome -basedir "$base" +set vid_renderer 2 "$@"
else
    exec "$base/engine/ezquake" -nohome -basedir "$base" +set vid_renderer 2 "$@"
fi
