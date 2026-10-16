#!/bin/sh
# Reproducible local macOS wrapper for the Felucca build.
set -eu

ROOT=$(CDPATH= cd -- "$(dirname "$0")/.." && pwd)

export PYTHON="$ROOT/.venv/bin/python"
export JIELI_TOOLCHAIN="$ROOT/.toolchains/jieli/toolchain"
export AC79_SDK="$ROOT/.toolchains/fw-AC79_AIoT_SDK"

exec "$ROOT/build.sh" "$@"
