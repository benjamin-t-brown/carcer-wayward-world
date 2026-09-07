#!/usr/bin/env bash
# Build and link a small SDL2W/BMIN consumer through their classic headers.
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
CXX_COMMAND="${1:-${CXX:-c++}}"
TARGET="${CARCER_DEPS_TARGET:-native}"

read -r -a cxx_parts <<<"${CXX_COMMAND}"
compiler_bin="${cxx_parts[0]}"
if ! command -v "${compiler_bin}" >/dev/null 2>&1; then
  echo "error: dependency compiler not found: ${compiler_bin}" >&2
  exit 1
fi

compiler_path="$(command -v "${compiler_bin}")"
compiler_version="$("${compiler_bin}" --version | head -n 1)"

hash_stream() {
  if command -v shasum >/dev/null 2>&1; then
    shasum -a 256 | awk '{print $1}'
  elif command -v sha256sum >/dev/null 2>&1; then
    sha256sum | awk '{print $1}'
  else
    echo "error: shasum or sha256sum is required to key probe artifacts" >&2
    return 1
  fi
}

identity="$({
  cat "${ROOT}/deps.lock"
  printf '%s\n' "compiler=${compiler_path}" "version=${compiler_version}" \
    "target=${TARGET}" "flags=${CARCER_HEADER_PROBE_CXXFLAGS:-}"
} | hash_stream)"
compiler_name="$(basename "${compiler_path}" | tr -c '[:alnum:]_.-' '_')"
output_root="${ROOT}/build/compat/dependency-headers/${compiler_name}-${identity:0:12}"
bundle_root="${output_root}/bundle"
probe_exe="${output_root}/HeaderMode"

CARCER_SDL2W_BUNDLE="${bundle_root}" \
CARCER_DEPS_CXX="${CXX_COMMAND}" \
CARCER_DEPS_TARGET="${TARGET}" \
  "${ROOT}/scripts/build-deps.sh" headers

if command -v pkg-config >/dev/null 2>&1; then
  read -r -a sdl_cflags <<<"$(pkg-config --cflags sdl2 SDL2_image SDL2_ttf SDL2_mixer SDL2_gfx)"
  read -r -a sdl_libs <<<"$(pkg-config --libs sdl2 SDL2_image SDL2_ttf SDL2_mixer SDL2_gfx)"
else
  read -r -a sdl_cflags <<<"$(sdl2-config --cflags)"
  read -r -a sdl_libs <<<"$(sdl2-config --libs) -lSDL2_image -lSDL2_ttf -lSDL2_mixer -lSDL2_gfx"
fi
mkdir -p "${output_root}"
# Intentional word splitting lets callers provide an ordinary flag string.
"${cxx_parts[@]}" -std=c++23 ${CARCER_HEADER_PROBE_CXXFLAGS:-} "${sdl_cflags[@]}" \
  -I"${bundle_root}/include" \
  "${ROOT}/src/__test__/deps/HeaderMode.cpp" \
  "${bundle_root}/lib/libsdl2w.a" "${bundle_root}/lib/libbmin.a" \
  "${sdl_libs[@]}" -o "${probe_exe}"
"${probe_exe}"

echo "Validated SDL2W/BMIN header mode with ${compiler_version}"
