#!/bin/sh
# Run Felucca's host tests with the project-local macOS dependencies.
set -eu

ROOT=$(CDPATH= cd -- "$(dirname "$0")/.." && pwd)

export PATH="$ROOT/.venv/bin:$PATH"
export AC79_SDK="$ROOT/.toolchains/fw-AC79_AIoT_SDK"

cd "$ROOT"
exec sh tests/run_tests.sh "$@"
