#!/usr/bin/env bash

set -Eeuo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
source "$SCRIPT_DIR/../lib/common.sh"

export QT_QPA_PLATFORM="${QT_QPA_PLATFORM:-offscreen}"

log_dir="${ROOT_DIR}/.ci-logs"
mkdir -p "$log_dir"

timestamp="$(date +%Y%m%d-%H%M%S)"
log_file="${CI_LOG_FILE:-${log_dir}/build-test-nonkde-${timestamp}.log}"
junit_file="${JUNIT_OUTPUT:-${log_dir}/build-test-nonkde-${timestamp}.xml}"

if [[ -n "${GITHUB_OUTPUT:-}" ]]; then
  {
    echo "log_file=$log_file"
    echo "junit_file=$junit_file"
  } >>"$GITHUB_OUTPUT"
fi

log "Running non-KDE tests (log: $log_file)"

if JUNIT_OUTPUT="$junit_file" "$SCRIPT_DIR/../test.sh" nonkde >"$log_file" 2>&1; then
  log "Build/tests completed successfully."
else
  log "Build/tests failed. Last 200 log lines:"
  tail -n 200 "$log_file"
  exit 1
fi

echo "JUnit report: $junit_file" >>"$GITHUB_STEP_SUMMARY"
