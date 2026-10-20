#!/bin/sh
# P0a listening only: no device or storage writes. Requires cc and Python/NumPy.
set -eu
cd "$(dirname "$0")/.."
test -f build/gen/felucca_tables.h || { echo 'Build firmware first for generated headers.'; exit 1; }
source_dir=build/p0a/baseline-source
mkdir -p "$source_dir/tools"
git archive 828ceaa firmware tests | tar -x -C "$source_dir"
cp tools/tide-breath-render.c "$source_dir/tools/"
cc -O2 -ffp-contract=off -w -Ibuild/gen -Ifirmware/src tools/tide-breath-render.c -lm -o build/p0a/factory-render
cc -O2 -ffp-contract=off -w -DTIDE_BREATH_EXPERIMENT=1 -Ibuild/gen -Ifirmware/src tools/tide-breath-render.c -lm -o build/p0a/candidate-render
cc -O2 -ffp-contract=off -w -Ibuild/gen -I"$source_dir/firmware/src" "$source_dir/tools/tide-breath-render.c" -lm -o build/p0a/baseline-render
exec "${PYTHON:-python3}" tools/tide_breath_compare.py "${1:-build/p0a/listening}"
