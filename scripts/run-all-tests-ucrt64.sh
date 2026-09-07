#!/usr/bin/env bash
# Run all non-UI tests and compile every UI test without opening windows.
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
PRESET="${CARCER_CMAKE_PRESET:-ucrt64-debug}"

cd "$ROOT"
cmake --preset "$PRESET"
cmake --build --preset "$PRESET" --target carcer_non_ui_tests
ctest --preset "$PRESET"
cmake --build --preset "$PRESET" --target carcer_ui_tests
