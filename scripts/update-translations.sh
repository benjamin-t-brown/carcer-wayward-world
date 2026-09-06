#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
DEPS_ROOT="${CARCER_DEPS_ROOT:-${ROOT}/.deps}"
SDL2W_ROOT="${DEPS_ROOT}/sdl2w"
BMIN_ROOT="${DEPS_ROOT}/bmin"
SCANNER="${SDL2W_ROOT}/src/build/tools/L10nScanner"

CARCER_DEPS_ROOT="${DEPS_ROOT}" "${ROOT}/scripts/bootstrap-deps.sh" --check

if [[ ! -x "${SCANNER}" && ! -x "${SCANNER}.exe" ]]; then
  echo "Building L10nScanner..."
  make -C "${SDL2W_ROOT}/src" tools BMIN_REPO="${BMIN_ROOT}"
fi

if [[ -x "${SCANNER}.exe" ]]; then
  SCANNER="${SCANNER}.exe"
fi

echo "Running L10nScanner"
"${SCANNER}" --input-dir "${ROOT}/src" --output-dir "${ROOT}/src/assets" en la
