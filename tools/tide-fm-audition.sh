#!/bin/sh
# Listening-only FM patch audition. Requires generated firmware headers.
set -eu
cd "$(dirname "$0")/.."
output=build/tide_fm_audition
source=build/tide_dry_reference
mkdir -p "$output" "$source/tools"
git archive 86d2167 firmware/src tests | tar -x -C "$source"
cp tools/tide-fm-audition.c "$source/tools/"
cc -O2 -w -Ibuild/gen -Ifirmware/src -o "$output/original" "$source/tools/tide-fm-audition.c" -lm
cc -O2 -w -Ibuild/gen -Ifirmware/src -o "$output/candidate" tools/tide-fm-audition.c -lm
"$output/original" 0 "$output/original.wav"
"$output/candidate" 1 "$output/candidate.wav"
"$output/original" 0 "$output/original-single.wav" single
"$output/candidate" 1 "$output/candidate-single.wav" single
"${PYTHON:-python3}" tools/tide_fm_match.py
