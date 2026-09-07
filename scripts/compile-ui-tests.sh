#!/usr/bin/env bash
# Compile all UI tests without opening SDL windows.
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
PRESET="${CARCER_CMAKE_PRESET:-gcc-debug}"

cd "$ROOT"
cmake --preset "$PRESET"
cmake --build --preset "$PRESET" --target carcer_ui_tests
