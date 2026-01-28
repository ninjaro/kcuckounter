#!/usr/bin/env bash

set -Eeuo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
source "$SCRIPT_DIR/lib.sh"

ANDROID_ENV_SILENT=1
source "$SCRIPT_DIR/env.sh"

mode="${1:-emulator}"

case "$mode" in
  emulator|device|build) ;;
  *)
    die "usage: $0 {emulator|device|build}"
    ;;
esac

declare -a missing
declare -a missing_pkgs

check_path() {
  local label="$1"
  local rel_path="$2"
  local pkg="$3"

  if [ ! -e "$ANDROID_SDK_ROOT/$rel_path" ]; then
    missing+=("$label ($pkg)")
    missing_pkgs+=("$pkg")
  fi
}

check_path "platform-tools" "platform-tools/adb" "platform-tools"
check_path "cmdline-tools" "cmdline-tools/latest/bin/sdkmanager" "cmdline-tools;latest"
check_path "build-tools" "build-tools/$ANDROID_BUILD_TOOLS_VERSION" "build-tools;$ANDROID_BUILD_TOOLS_VERSION"
check_path "platforms" "platforms/$ANDROID_PLATFORM" "platforms;$ANDROID_PLATFORM"
check_path "ndk" "ndk/$ANDROID_NDK_VERSION" "ndk;$ANDROID_NDK_VERSION"

if [ "$mode" = "emulator" ]; then
  emulator_image="${ANDROID_SYSTEM_IMAGE:-system-images;android-35;google_apis;x86_64}"
  emulator_image_path="${emulator_image//;/\/}"
  check_path "emulator" "emulator/emulator" "emulator"
  check_path "system-image" "$emulator_image_path" "$emulator_image"
fi

if [ "${#missing[@]}" -eq 0 ]; then
  log "All required Android SDK components are present for mode: $mode"
  exit 0
fi

log "Missing Android SDK components for mode: $mode"
for item in "${missing[@]}"; do
  log "  - $item"
done
log
log "Install with sdkmanager (example):"
log "  sdkmanager --sdk_root=\"$ANDROID_SDK_ROOT\" ${missing_pkgs[*]}"
