#!/usr/bin/env bash
# UCRT64 native acceptance: game, non-UI tests, and compile-only UI tests.
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
PRESET="${CARCER_CMAKE_PRESET:-ucrt64-debug}"

cd "$ROOT"
cmake --preset "$PRESET"
cmake --build --preset "$PRESET" --target CARCER carcer_non_ui_tests
ctest --preset "$PRESET"
cmake --build --preset "$PRESET" --target carcer_ui_tests

echo "Acceptance passed. UI executables were compiled but not opened."
