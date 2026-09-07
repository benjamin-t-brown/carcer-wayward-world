#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
PRESET="${CARCER_CMAKE_PRESET:-emscripten-release}"
BUILD_DIR="$ROOT/build/cmake/$PRESET"

cd "$ROOT"
cmake --preset "$PRESET"
cmake --build --preset "$PRESET" --target CARCER
cmake -E copy_if_different "$BUILD_DIR/CARCER.js" "$ROOT/web/CARCER.js"
cmake -E copy_if_different "$BUILD_DIR/CARCER.wasm" "$ROOT/web/CARCER.wasm"
cmake -E copy_if_different "$BUILD_DIR/CARCER.data" "$ROOT/web/CARCER.data"
