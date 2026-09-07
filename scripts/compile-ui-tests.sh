#!/usr/bin/env bash
# Compile all UI tests without executing interactive SDL programs.
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
DEFAULT_PRESET=gcc-debug
if [[ "${MSYSTEM:-}" == "UCRT64" ]]; then
  DEFAULT_PRESET=ucrt64-debug
fi
PRESET="${CARCER_CMAKE_PRESET:-$DEFAULT_PRESET}"

cmake --preset "$PRESET" -S "$ROOT"
cmake --build --preset "$PRESET" --target carcer_ui_tests
