#!/bin/sh
# AmbWave firmware: retain compatible Felucca device/package identifiers.
set -eu
ROOT=$(CDPATH= cd -- "$(dirname "$0")/.." && pwd)
export AMBIENT_POC=1
cd "$ROOT"
"$ROOT/tools/build-macos.sh" "$@"
"$ROOT/.venv/bin/python" tools/ambwave_release.py stamp firmware
