#!/usr/bin/env bash

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
source "$SCRIPT_DIR/../lib/common.sh"

detect_android_sdk_root() {
  if [ -n "${ANDROID_SDK_ROOT:-}" ] && [ -d "$ANDROID_SDK_ROOT" ]; then
    echo "$ANDROID_SDK_ROOT"
    return 0
  fi

  for dir in "$HOME/Android/Sdk" "/opt/android-sdk"; do
    if [ -d "$dir" ]; then
      echo "$dir"
      return 0
    fi
  done

  if command -v adb >/dev/null 2>&1; then
    local adb_path sdk_from_adb
    adb_path="$(command -v adb)"
    sdk_from_adb="$(cd "$(dirname "$adb_path")/.." 2>/dev/null && pwd)"
    if [ -n "$sdk_from_adb" ]; then
      echo "$sdk_from_adb"
      return 0
    fi
  fi

  if command -v emulator >/dev/null 2>&1; then
    local emu_path sdk_from_emu
    emu_path="$(command -v emulator)"
    sdk_from_emu="$(cd "$(dirname "$emu_path")/.." 2>/dev/null && pwd)"
    if [ -n "$sdk_from_emu" ]; then
      echo "$sdk_from_emu"
      return 0
    fi
  fi

  return 1
}

detect_android_emulator() {
  if [ -n "${ANDROID_EMULATOR_BIN:-}" ] && [ -x "$ANDROID_EMULATOR_BIN" ]; then
    echo "$ANDROID_EMULATOR_BIN"
    return 0
  fi

  if [ -n "${ANDROID_SDK_ROOT:-}" ] && [ -x "$ANDROID_SDK_ROOT/emulator/emulator" ]; then
    echo "$ANDROID_SDK_ROOT/emulator/emulator"
    return 0
  fi

  if command -v emulator >/dev/null 2>&1; then
    command -v emulator
    return 0
  fi

  return 1
}

android_adb_path() {
  local sdk_root adb_bin
  sdk_root="${ANDROID_SDK_ROOT:-${ANDROID_HOME:-$HOME/Android/Sdk}}"
  adb_bin="${ADB_BIN:-$sdk_root/platform-tools/adb}"

  if [ -x "$adb_bin" ]; then
    echo "$adb_bin"
    return 0
  fi

  if command -v adb >/dev/null 2>&1; then
    command -v adb
    return 0
  fi

  return 1
}

android_find_apk() {
  local build_dir apk_path
  build_dir="$1"
  apk_path="$(find "$build_dir" -path '*outputs/apk/*/*-debug.apk' | head -n 1 || true)"
  if [ -n "$apk_path" ]; then
    echo "$apk_path"
    return 0
  fi
  return 1
}

android_first_emulator_serial() {
  local adb_bin="$1"
  "$adb_bin" devices -l | awk 'NR>1 && $1 ~ /^emulator-/ {print $1; exit}'
}

android_first_device_serial() {
  local adb_bin="$1"
  "$adb_bin" devices -l | awk 'NR>1 && $1 !~ /^emulator-/ {print $1; exit}'
}

android_wait_for_boot() {
  local adb_bin="$1"
  local serial="$2"

  "$adb_bin" -s "$serial" wait-for-device
  while { [ "$("$adb_bin" -s "$serial" shell getprop sys.boot_completed | tr -d '\r')" != "1" ]; }; do
    sleep 1
  done
}
