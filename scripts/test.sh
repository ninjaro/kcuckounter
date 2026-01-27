#!/usr/bin/env bash

set -Eeuo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
source "$SCRIPT_DIR/lib/common.sh"

mode="${1:-all}"
build_type="${BUILD_TYPE:-Debug}"
install_prefix="${INSTALL_PREFIX:-$HOME/kde/usr}"
parallel="${PARALLEL:-$(nproc_safe)}"

run_tests() {
  local name="$1"
  local kde_flag="$2"
  local build_dir="$ROOT_DIR/build-$name"
  local junit_output="${JUNIT_OUTPUT:-}"

  cmake -S "$ROOT_DIR" -B "$build_dir" \
    -DCMAKE_INSTALL_PREFIX="$install_prefix" \
    -DKDE="$kde_flag" \
    -DBUILD_UNIT_TESTS=ON \
    -DCMAKE_BUILD_TYPE="$build_type"

  cmake --build "$build_dir" --parallel "$parallel"
  if [[ -n "$junit_output" ]]; then
    ctest --test-dir "$build_dir" --output-on-failure --parallel "$parallel" \
      --output-junit "$junit_output"
  else
    ctest --test-dir "$build_dir" --output-on-failure --parallel "$parallel"
  fi
}

case "$mode" in
  kde)
    run_tests "kde" "ON"
    ;;
  nonkde|nokde)
    run_tests "nokde" "OFF"
    ;;
  all)
    run_tests "kde" "ON"
    run_tests "nokde" "OFF"
    ;;
  *)
    die "usage: $0 {kde|nonkde|all}"
    ;;
esac
