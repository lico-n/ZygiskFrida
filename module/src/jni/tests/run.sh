#!/usr/bin/env bash
# Compile and run the host-side ZygiskFrida config unit tests.
# No NDK/device needed: <android/log.h> is stubbed under tests/stub.
set -euo pipefail

HERE="$(cd "$(dirname "$0")" && pwd)"
JNI="$(cd "$HERE/.." && pwd)"
BIN="$(mktemp -d)/config_test"

"${CXX:-clang++}" -std=c++17 -Wall -Wextra \
  -I "$HERE/stub" \
  -isystem "$JNI/include" \
  -I "$JNI" \
  "$HERE/config_test.cpp" \
  "$JNI/config.cpp" \
  -o "$BIN"

"$BIN"
