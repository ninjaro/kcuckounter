#!/usr/bin/env bash

set -Eeuo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
source "$SCRIPT_DIR/../lib/common.sh"

export QT_QPA_PLATFORM="${QT_QPA_PLATFORM:-offscreen}"

log_dir="${ROOT_DIR}/.ci-logs"
mkdir -p "$log_dir"

timestamp="$(date +%Y%m%d-%H%M%S)"
log_file="${CI_LOG_FILE:-${log_dir}/build-test-sanitizer-${timestamp}.log}"

if [[ -n "${GITHUB_OUTPUT:-}" ]]; then
  echo "log_file=$log_file" >>"$GITHUB_OUTPUT"
fi

sanitizer_flags="-fsanitize=address,undefined -fno-omit-frame-pointer -fno-sanitize-recover=all"
build_dir="${ROOT_DIR}/build-sanitizers"

log "Building with sanitizers (log: $log_file)"

if (
  cmake -S "$ROOT_DIR" -B "$build_dir" \
    -DCMAKE_BUILD_TYPE=RelWithDebInfo \
    -DKDE=OFF \
    -DBUILD_UNIT_TESTS=OFF \
    -DCMAKE_C_FLAGS="$sanitizer_flags" \
    -DCMAKE_CXX_FLAGS="$sanitizer_flags" \
    -DCMAKE_EXE_LINKER_FLAGS="$sanitizer_flags" \
    -DCMAKE_SHARED_LINKER_FLAGS="$sanitizer_flags" \
    && cmake --build "$build_dir" --parallel "$(nproc_safe)"
) >"$log_file" 2>&1; then
  log "Sanitizer build completed successfully."
else
  log "Sanitizer build failed. Last 200 log lines:"
  tail -n 200 "$log_file"
  exit 1
fi

echo "Sanitizer build log: $log_file" >>"$GITHUB_STEP_SUMMARY"
