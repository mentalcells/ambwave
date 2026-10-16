#!/bin/sh
# Rebuild the dry A/B from the installed pre-experiment source and current tree.
# Requires generated build/gen headers from tools/build-hasake-macos.sh.
set -eu
cd "$(dirname "$0")/.."
reference=${1:-86d2167}
output=${2:-build/tide_fm_comparison}
source=build/tide_dry_reference
mkdir -p "$source" "$output/before" "$output/after" build/host
git archive "$reference" firmware/src tests | tar -x -C "$source"
cp tests/tide_dry_test.c "$source/tests/tide_dry_test.c"
cc -O2 -w -Ibuild/gen -Ifirmware/src -o build/host/tide_dry_before "$source/tests/tide_dry_test.c" -lm
cc -O2 -w -Ibuild/gen -Ifirmware/src -o build/host/tide_dry_test tests/tide_dry_test.c -lm
build/host/tide_dry_before "$output/before" reference
build/host/tide_dry_test "$output/after"
"${PYTHON:-python3}" tools/tide_dry_compare.py "$output"
