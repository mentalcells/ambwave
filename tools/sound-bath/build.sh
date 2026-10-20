#!/bin/sh
# SPDX-License-Identifier: GPL-3.0-only
set -eu
cd "$(dirname "$0")/../.."
mkdir -p build/sound-bath
cc -O2 -ffp-contract=off -std=gnu99 -w -Ibuild/gen -Ifirmware/src tools/sound-bath/integrated-adapter.c -lm -o build/sound-bath/integrated
emcc -O2 -ffp-contract=off -std=gnu99 -w -Ibuild/gen -Ifirmware/src tools/sound-bath/integrated-adapter.c --no-entry -sSTANDALONE_WASM -sSTACK_SIZE=1048576 -sINITIAL_MEMORY=33554432 -sFILESYSTEM=0 -o build/sound-bath/integrated.wasm
