#!/usr/bin/env bash

set -Eeuo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
source "$SCRIPT_DIR/lib.sh"

ANDROID_ENV_SILENT=1
source "$SCRIPT_DIR/env.sh"

build_dir="${ANDROID_BUILD_DIR:-$ROOT_DIR/build-android-debug}"
cmake_bin="${ANDROID_CMAKE_BIN:-}"

if [ -z "$cmake_bin" ] || [ ! -x "$cmake_bin" ]; then
  if [ "${ANDROID_OPTIONAL:-0}" = "1" ]; then
    log "ANDROID_CMAKE_BIN is not set or not executable; skipping android build."
    log "set it to your qt-cmake, for example:"
    log "  export ANDROID_CMAKE_BIN=\"$HOME/Qt/6.8.2/android_x86_64/bin/qt-cmake\""
    exit 0
  fi
  die "ANDROID_CMAKE_BIN is not set or not executable. Set it to your qt-cmake binary."
fi

"$cmake_bin" -S "$ROOT_DIR" -B "$build_dir" \
  -DQT_HOST_PATH="$QT_HOST_PATH" \
  -DANDROID_ABI="${ANDROID_ABI:-x86_64}" \
  -DANDROID_PLATFORM="$ANDROID_PLATFORM" \
  -DANDROID_SDK_ROOT="$ANDROID_SDK_ROOT" \
  -DANDROID_NDK="$ANDROID_NDK_ROOT" \
  -DCMAKE_BUILD_TYPE="${ANDROID_BUILD_TYPE:-Debug}" \
  -DKDE=OFF

cmake --build "$build_dir" --parallel "$(nproc_safe)"
