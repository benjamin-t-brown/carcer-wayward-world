#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
DEPS_ROOT="${CARCER_DEPS_ROOT:-${ROOT}/.deps}"
SDL2W_ROOT="${DEPS_ROOT}/sdl2w"
BMIN_ROOT="${DEPS_ROOT}/bmin"
ANIMS="${SDL2W_ROOT}/src/build/tools/Anims"

CARCER_DEPS_ROOT="${DEPS_ROOT}" "${ROOT}/scripts/bootstrap-deps.sh" --check

if [[ ! -x "${ANIMS}" && ! -x "${ANIMS}.exe" ]]; then
  echo "Building Anims..."
  make -C "${SDL2W_ROOT}/src" tools BMIN_REPO="${BMIN_ROOT}"
fi
if [[ -x "${ANIMS}.exe" ]]; then
  ANIMS="${ANIMS}.exe"
fi

"${ANIMS}" --assets-dir "${ROOT}/src/assets" \
  --asset-file "${ROOT}/src/assets/assets.game.txt"
