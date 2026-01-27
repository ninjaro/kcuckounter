#!/usr/bin/env bash

set -Eeuo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
source "$SCRIPT_DIR/lib/common.sh"

rg --files -g '*.cpp' -g '*.hpp' -g '*.cc' -g '*.cxx' "$ROOT_DIR" \
  | xargs -r clang-format -style=file -i
