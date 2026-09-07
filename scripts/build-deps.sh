#!/usr/bin/env bash
# Build a compiler-compatible classic-header SDL2W/BMIN consumer bundle.
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
LOCK_FILE="${CARCER_DEPS_LOCK:-${ROOT}/deps.lock}"
DEPS_ROOT="${CARCER_DEPS_ROOT:-${ROOT}/.deps}"
SDL2W_ROOT="${DEPS_ROOT}/sdl2w"
BMIN_ROOT="${DEPS_ROOT}/bmin"
BUNDLE_ROOT="${CARCER_SDL2W_BUNDLE:-${ROOT}/build/compat/dependency-headers}"
CXX_COMMAND="${CARCER_DEPS_CXX:-${CXX:-c++}}"
TARGET="${CARCER_DEPS_TARGET:-native}"
MODE="${1:-headers}"
FORCE="${CARCER_DEPS_FORCE:-0}"
CONSUMER_FLAGS="${CARCER_DEPS_CONSUMER_FLAGS:-}"

if [[ "${MODE}" != "headers" ]]; then
  echo "error: Carcer only stages the dependencies' classic header API" >&2
  echo "Use the upstream SDL2W/BMIN commands to validate their optional modules." >&2
  exit 2
fi

check_args=(--check)
if [[ "${CARCER_ALLOW_DIRTY_DEPS:-0}" == "1" ]]; then
  check_args+=(--allow-dirty)
fi
CARCER_DEPS_LOCK="${LOCK_FILE}" CARCER_DEPS_ROOT="${DEPS_ROOT}" \
  "${ROOT}/scripts/bootstrap-deps.sh" "${check_args[@]}"

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
    echo "error: shasum or sha256sum is required to key dependency artifacts" >&2
    return 1
  fi
}

key="$({
  cat "${LOCK_FILE}"
  printf '%s\n' "mode=headers" "target=${TARGET}" "compiler=${compiler_path}" \
    "version=${compiler_version}" "consumer_flags=${CONSUMER_FLAGS}"
} | hash_stream)"
key_file="${BUNDLE_ROOT}/.carcer-headers-${TARGET}-build-key"
source_key_file="${ROOT}/build/compat/dependency-source-build.key"

bundle_is_ready() {
  [[ -f "${key_file}" ]] || return 1
  [[ "$(<"${key_file}")" == "${key}" ]] || return 1
  [[ -f "${BUNDLE_ROOT}/lib/libsdl2w.a" ]] || return 1
  [[ -f "${BUNDLE_ROOT}/lib/libbmin.a" ]] || return 1
  [[ -f "${BUNDLE_ROOT}/include/sdl2w/Window.h" ]] || return 1
  [[ -f "${BUNDLE_ROOT}/include/bmin/String.h" ]] || return 1
}

if [[ "${FORCE}" != "1" ]] && bundle_is_ready; then
  echo "Using pinned SDL2W/BMIN header bundle for ${TARGET} (${key:0:12})"
  exit 0
fi

make_args=(-C "${SDL2W_ROOT}/src" "BMIN_REPO=${BMIN_ROOT}" "CXX=${CXX_COMMAND}")
if [[ ! -f "${source_key_file}" ]] || [[ "$(<"${source_key_file}")" != "${key}" ]]; then
  echo "Dependency build identity changed; cleaning shared source outputs"
  make "${make_args[@]}" clean TARGET="${TARGET}"
fi

echo "Building pinned SDL2W/BMIN header artifacts with ${CXX_COMMAND}"
make -C "${BMIN_ROOT}/src" install-headers "CXX=${CXX_COMMAND}" TARGET="${TARGET}"

# SDL2W's upstream install-headers target has a dual-mode BMIN prerequisite.
# Seed that staging directory with the already-built classic archive so Carcer
# does not compile dependency modules merely to consume headers.
header_stage="${SDL2W_ROOT}/src/bmin"
mkdir -p "${header_stage}"
cp -R "${BMIN_ROOT}/bmin/include/." "${header_stage}/"
cp -f "${BMIN_ROOT}/bmin/lib/libbmin.a" "${header_stage}/libbmin.a"
cp -f "${BMIN_ROOT}/bmin/lib/libbmin.a" "${header_stage}/libbmin_modules.a"
header_stamp="${header_stage}/.artifacts-ready"
touch "${header_stamp}"
make "${make_args[@]}" install-headers TARGET="${TARGET}" \
  "DEPS_BMIN_DIR=${header_stage}" \
  "DEPS_BMIN_STAMP=${header_stamp}" \
  "DEPS_BMIN_HEADER_LIB=${header_stage}/libbmin.a" \
  "BMIN_BUILT_MODULE_LIB=${header_stage}/libbmin_modules.a" \
  BMIN_MODULE_INPUTS=

mkdir -p "${BUNDLE_ROOT}/lib" "${BUNDLE_ROOT}/include/sdl2w" \
  "${BUNDLE_ROOT}/include/bmin"
cp -f "${SDL2W_ROOT}/sdl2w/lib/libsdl2w.a" "${BUNDLE_ROOT}/lib/"
cp -f "${SDL2W_ROOT}/sdl2w/lib/libbmin.a" "${BUNDLE_ROOT}/lib/"
cp -f "${SDL2W_ROOT}/sdl2w/include/"*.h "${BUNDLE_ROOT}/include/sdl2w/"
cp -R "${SDL2W_ROOT}/sdl2w/include/bmin/." "${BUNDLE_ROOT}/include/bmin/"

mkdir -p "${BUNDLE_ROOT}"
tmp_key="$(mktemp "${BUNDLE_ROOT}/.build-key.XXXXXX")"
printf '%s\n' "${key}" >"${tmp_key}"
mv -f "${tmp_key}" "${key_file}"
mkdir -p "$(dirname "${source_key_file}")"
tmp_source_key="$(mktemp "${source_key_file}.XXXXXX")"
printf '%s\n' "${key}" >"${tmp_source_key}"
mv -f "${tmp_source_key}" "${source_key_file}"
echo "Prepared SDL2W/BMIN header bundle for ${TARGET} (${key:0:12})"
