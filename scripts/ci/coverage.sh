#!/usr/bin/env bash

set -Eeuo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
source "$SCRIPT_DIR/../lib/common.sh"

export QT_QPA_PLATFORM=offscreen

cmake -S "$ROOT_DIR" -B "$ROOT_DIR/build" \
  -DCMAKE_BUILD_TYPE=Debug \
  -DKDE=OFF \
  -DENABLE_COVERAGE=ON

cmake --build "$ROOT_DIR/build" --target coverage --parallel "$(nproc_safe)"

if [ -f "$ROOT_DIR/Doxyfile" ]; then
  doxygen "$ROOT_DIR/Doxyfile"
fi
