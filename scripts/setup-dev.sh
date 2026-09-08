#!/usr/bin/env bash
# Prepare a current-platform native development build and clangd database.
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
PRESET="dev-debug"

print_install_hint() {
  case "$(uname -s)" in
    Darwin)
      echo "Install with: brew install cmake ninja llvm pkg-config sdl2 sdl2_image sdl2_ttf sdl2_mixer sdl2_gfx" >&2
      ;;
    MINGW*|MSYS*)
      echo "Install the UCRT64 toolchain, CMake, Ninja, Make, pkg-config, clang-tools-extra, and matching SDL2 packages with pacman." >&2
      ;;
    *)
      if command -v apt-get >/dev/null 2>&1; then
        echo "Install with: sudo apt-get install build-essential cmake ninja-build pkg-config clangd libsdl2-dev libsdl2-image-dev libsdl2-ttf-dev libsdl2-mixer-dev libsdl2-gfx-dev" >&2
      else
        echo "Install CMake, Ninja, Make, pkg-config, clangd, a C++23 compiler, and the SDL2 development packages for your distribution." >&2
      fi
      ;;
  esac
}

missing=()
for command_name in git cmake ninja pkg-config make clangd; do
  if ! command -v "${command_name}" >/dev/null 2>&1; then
    missing+=("${command_name}")
  fi
done

compiler="${CXX:-c++}"
if ! command -v "${compiler}" >/dev/null 2>&1; then
  missing+=("${compiler} (C++ compiler)")
fi

if ((${#missing[@]})); then
  printf 'error: missing development tools:' >&2
  printf ' %s' "${missing[@]}" >&2
  printf '\n' >&2
  print_install_hint
  exit 1
fi

cmake_version="$(cmake --version | awk 'NR == 1 { print $3 }')"
if ! awk -v version="${cmake_version}" 'BEGIN {
  split(version, parts, ".")
  exit !((parts[1] > 3) || (parts[1] == 3 && parts[2] >= 28))
}'; then
  echo "error: CMake 3.28 or newer is required; found ${cmake_version}" >&2
  exit 1
fi

sdl_packages=(sdl2 SDL2_image SDL2_ttf SDL2_mixer SDL2_gfx)
if ! pkg-config --exists "${sdl_packages[@]}"; then
  echo "error: native SDL2 development packages are missing." >&2
  echo "Required pkg-config packages: ${sdl_packages[*]}" >&2
  print_install_hint
  exit 1
fi

cd "${ROOT}"
./scripts/bootstrap-deps.sh
cmake --preset "${PRESET}"
cmake --build --preset "${PRESET}" --target CARCER

executable="${ROOT}/build/cmake/${PRESET}/CARCER"
if [[ "$(uname -s)" == MINGW* || "$(uname -s)" == MSYS* ]]; then
  executable="${executable}.exe"
fi

echo
echo "Development setup complete."
echo "  executable: ${executable}"
echo "  clangd database: ${ROOT}/build/cmake/${PRESET}/compile_commands.json"
echo "  from src/: make | make run | make test | make js"
