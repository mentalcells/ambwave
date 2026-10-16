#!/bin/sh
# Build and stamp the product browser, never the generic Felucca default.
set -eu
ROOT=$(CDPATH= cd -- "$(dirname "$0")/.." && pwd)
cd "$ROOT"
export AMBIENT_POC=1
export EMU_PREVIEW=${EMU_PREVIEW:-0}
sh web/emu/build.sh
mkdir -p build/ambwave-browser/fonts
cp build/emu/index.html build/emu/worklet.js build/emu/felucca.wasm build/ambwave-browser/
cp web/emu/fonts/DotGothic16-subset.woff web/emu/fonts/OFL.txt build/ambwave-browser/fonts/
"${PYTHON:-python3}" tools/ambwave_release.py stamp browser
