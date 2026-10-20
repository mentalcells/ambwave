#!/bin/sh
# Offline P0b auditions only; no firmware, device or user-data writes.
set -eu
cd "$(dirname "$0")/.."
mkdir -p build/p0b
baseline=build/p0b/baseline-source
mkdir -p "$baseline/tools"
git archive 828ceaa firmware tests | tar -x -C "$baseline"
cp tools/tide-breath-render.c "$baseline/tools/"
cc -O2 -ffp-contract=off -w -Ibuild/gen -I"$baseline/firmware/src" "$baseline/tools/tide-breath-render.c" -lm -o build/p0b/baseline-render
cc -O2 -ffp-contract=off -w -Ibuild/gen -Ifirmware/src tools/tide-voicing-render.c -lm -o build/p0b/render
cc -O2 -ffp-contract=off -w -Ibuild/gen -Ifirmware/src tests/tide_voicing_test.c -lm -o build/p0b/test
build/p0b/test
exec "${PYTHON:-python3}" tools/tide_voicing_compare.py "${1:-build/p0b/listening}" "${2:-ABC}"
