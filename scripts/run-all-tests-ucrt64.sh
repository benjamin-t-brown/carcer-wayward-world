#!/usr/bin/env bash
# Run non-UI C++ tests and compile UI tests without executing them.
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
PRESET="${CARCER_CMAKE_PRESET:-ucrt64-debug}"

cd "$ROOT"
cmake --preset "$PRESET"
cmake --build --preset "$PRESET"
ctest --preset "$PRESET"
cmake --build --preset "$PRESET" --target carcer_ui_tests
