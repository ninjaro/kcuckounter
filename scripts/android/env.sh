#!/usr/bin/env bash

if [ "${BASH_SOURCE[0]}" = "$0" ]; then
  echo "this script is meant to be sourced, not executed."
  echo "use:"
  echo "  source ./scripts/android/env.sh"
  exit 1
fi

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
source "$SCRIPT_DIR/lib.sh"

sdk_root="$(detect_android_sdk_root)" || true
if [ -n "$sdk_root" ]; then
  export ANDROID_SDK_ROOT="$sdk_root"
else
  export ANDROID_SDK_ROOT="${ANDROID_SDK_ROOT:-$HOME/Android/Sdk}"
  if [ "${ANDROID_ENV_SILENT:-0}" != "1" ]; then
    echo "WARNING: could not auto-detect Android SDK root, using ANDROID_SDK_ROOT=$ANDROID_SDK_ROOT (may not exist)" >&2
  fi
fi

export ANDROID_NDK_VERSION="${ANDROID_NDK_VERSION:-27.2.12479018}"
export ANDROID_NDK_ROOT="${ANDROID_NDK_ROOT:-$ANDROID_SDK_ROOT/ndk/$ANDROID_NDK_VERSION}"

export ANDROID_PLATFORM="${ANDROID_PLATFORM:-android-24}"
export ANDROID_BUILD_TOOLS_VERSION="${ANDROID_BUILD_TOOLS_VERSION:-34.0.0}"

export QT_DIR="${QT_DIR:-$HOME/Qt}"
export QT_VER="${QT_VER:-6.8.2}"
export QT_HOST_PATH="${QT_HOST_PATH:-$QT_DIR/$QT_VER/gcc_64}"

export ANDROID_QT_ARCH="${ANDROID_QT_ARCH:-android_x86_64}"

export ANDROID_CMAKE_BIN="${ANDROID_CMAKE_BIN:-$QT_DIR/$QT_VER/$ANDROID_QT_ARCH/bin/qt-cmake}"

emu_bin="$(detect_android_emulator)" || true
if [ -n "$emu_bin" ]; then
  export ANDROID_EMULATOR_BIN="$emu_bin"
else
  export ANDROID_EMULATOR_BIN="${ANDROID_EMULATOR_BIN:-$ANDROID_SDK_ROOT/emulator/emulator}"
  if [ "${ANDROID_ENV_SILENT:-0}" != "1" ]; then
    echo "WARNING: Android emulator binary not found; ANDROID_EMULATOR_BIN=$ANDROID_EMULATOR_BIN (may not exist)" >&2
  fi
fi

export ANDROID_AVD_NAME="${ANDROID_AVD_NAME:-api35_x86_64}"

case ":$PATH:" in
  *":$ANDROID_SDK_ROOT/platform-tools:"*) ;;
  *) PATH="$ANDROID_SDK_ROOT/platform-tools:$PATH" ;;
esac

case ":$PATH:" in
  *":$ANDROID_SDK_ROOT/emulator:"*) ;;
  *) PATH="$ANDROID_SDK_ROOT/emulator:$PATH" ;;
esac

export PATH

if [ "${ANDROID_ENV_SILENT:-0}" = "1" ]; then
  return 0
fi

if [ -x "$ANDROID_SDK_ROOT/platform-tools/adb" ]; then
  adb_path="$ANDROID_SDK_ROOT/platform-tools/adb"
elif command -v adb >/dev/null 2>&1; then
  adb_path="$(command -v adb)"
else
  adb_path="(not found)"
fi

echo "ANDROID_SDK_ROOT=$ANDROID_SDK_ROOT"
echo "ANDROID_NDK_ROOT=$ANDROID_NDK_ROOT"
echo "ANDROID_PLATFORM=$ANDROID_PLATFORM"
echo "ANDROID_BUILD_TOOLS_VERSION=$ANDROID_BUILD_TOOLS_VERSION"
echo "QT_DIR=$QT_DIR"
echo "QT_VER=$QT_VER"
echo "QT_HOST_PATH=$QT_HOST_PATH"
echo "ANDROID_QT_ARCH=$ANDROID_QT_ARCH"
echo "ANDROID_CMAKE_BIN=$ANDROID_CMAKE_BIN"
echo "ANDROID_EMULATOR_BIN=$ANDROID_EMULATOR_BIN"
echo "ANDROID_AVD_NAME=$ANDROID_AVD_NAME"
echo "adb=$adb_path"

if [ "$adb_path" = "(not found)" ]; then
  echo
  echo "WARNING: adb not found. If SDK is installed via sdkmanager, you probably need:"
  echo "  sdkmanager --sdk_root=\"$ANDROID_SDK_ROOT\" \"platform-tools\""
fi
