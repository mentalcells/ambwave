#!/bin/sh
# Compatibility alias for earlier development instructions.
set -eu
ROOT=$(CDPATH= cd -- "$(dirname "$0")/.." && pwd)
exec "$ROOT/tools/build-ambwave-macos.sh" "$@"
