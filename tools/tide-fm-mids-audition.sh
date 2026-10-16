#!/bin/sh
# Compare the positively heard FM patch with one second-harmonic level change.
# Uses the existing FM6 engine; no firmware/device write.
set -eu
cd "$(dirname "$0")/.."
output=build/tide_fm_mids
mkdir -p "$output"
cc -O2 -w -Ibuild/gen -Ifirmware/src -o "$output/render" tools/tide-fm-audition.c -lm
"$output/render" 1 "$output/current.wav"
"$output/render" 2 "$output/revised.wav"
"$output/render" 1 "$output/current-single.wav" single
"$output/render" 2 "$output/revised-single.wav" single
"${PYTHON:-python3}" tools/tide_fm_match.py "$output" current revised
