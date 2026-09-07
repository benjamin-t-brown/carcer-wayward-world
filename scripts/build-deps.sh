#!/usr/bin/env bash
# Build a compiler-compatible SDL2W/BMIN consumer bundle from pinned sources.
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
LOCK_FILE="${CARCER_DEPS_LOCK:-${ROOT}/deps.lock}"
DEPS_ROOT="${CARCER_DEPS_ROOT:-${ROOT}/.deps}"
SDL2W_ROOT="${DEPS_ROOT}/sdl2w"
BMIN_ROOT="${DEPS_ROOT}/bmin"
BUNDLE_ROOT="${CARCER_SDL2W_BUNDLE:-${ROOT}/src/lib/sdl2w}"
CXX_COMMAND="${CARCER_DEPS_CXX:-${CXX:-c++}}"
TARGET="${CARCER_DEPS_TARGET:-native}"
MODE="${1:-dual}"
FORCE="${CARCER_DEPS_FORCE:-0}"
MODULE_CXXFLAGS="${CARCER_DEPS_MODULE_CXXFLAGS:-}"
MODULE_INTERFACE_FLAGS="${CARCER_DEPS_MODULE_INTERFACE_FLAGS:-}"

if [[ "${MODE}" != "dual" && "${MODE}" != "headers" ]]; then
  echo "error: dependency build mode must be 'dual' or 'headers'" >&2
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
  printf '%s\n' "mode=${MODE}" "target=${TARGET}" "compiler=${compiler_path}" \
    "version=${compiler_version}" "module_flags=${MODULE_CXXFLAGS}" \
    "interface_flags=${MODULE_INTERFACE_FLAGS}"
} | hash_stream)"
key_file="${BUNDLE_ROOT}/.carcer-${MODE}-${TARGET}-build-key"
source_key_file="${ROOT}/build/dependency-source-build.key"

bundle_is_ready() {
  [[ -f "${key_file}" ]] || return 1
  [[ "$(<"${key_file}")" == "${key}" ]] || return 1
  [[ -f "${BUNDLE_ROOT}/lib/libsdl2w.a" ]] || return 1
  [[ -f "${BUNDLE_ROOT}/lib/libbmin.a" ]] || return 1
  [[ -d "${BUNDLE_ROOT}/include/bmin" ]] || return 1
  if [[ "${MODE}" == "dual" ]]; then
    [[ -f "${BUNDLE_ROOT}/lib/libsdl2w_modules.a" ]] || return 1
    [[ -f "${BUNDLE_ROOT}/lib/libbmin_modules.a" ]] || return 1
    [[ -f "${BUNDLE_ROOT}/modules/make/use.mk" ]] || return 1
    [[ -f "${BUNDLE_ROOT}/modules/bmin/make/use.mk" ]] || return 1
  fi
  if [[ "${TARGET}" == "wasm" ]]; then
    [[ -d "${BUNDLE_ROOT}/lib/wasm" ]] || return 1
  fi
}

if [[ "${FORCE}" != "1" ]] && bundle_is_ready; then
  echo "Using pinned ${MODE} SDL2W/BMIN bundle for ${TARGET} (${key:0:12})"
  exit 0
fi

make_args=(
  -C "${SDL2W_ROOT}/src"
  "BMIN_REPO=${BMIN_ROOT}"
  "CXX=${CXX_COMMAND}"
)
if [[ -n "${MODULE_CXXFLAGS}" ]]; then
  make_args+=("SDL2W_MODULE_CXXFLAGS=${MODULE_CXXFLAGS}")
fi
if [[ -n "${MODULE_INTERFACE_FLAGS}" ]]; then
  make_args+=("SDL2W_MODULE_INTERFACE_FLAGS=${MODULE_INTERFACE_FLAGS}")
fi

# SDL2W and BMIN's Make outputs live in their source checkouts. Make does not
# consider a compiler or flag change when deciding whether those objects are
# current, so clean the dependency outputs before changing build identity.
# Consumer bundles remain separately keyed and can still be reused directly.
if [[ ! -f "${source_key_file}" ]] || [[ "$(<"${source_key_file}")" != "${key}" ]]; then
  echo "Dependency build identity changed; cleaning shared source outputs"
  make "${make_args[@]}" clean TARGET="${TARGET}"
fi

if [[ "${MODE}" == "headers" ]]; then
  echo "Building pinned SDL2W/BMIN header artifacts with ${CXX_COMMAND}"
  make -C "${BMIN_ROOT}/src" install-headers \
    "CXX=${CXX_COMMAND}" TARGET="${TARGET}"

  # SDL2W's upstream install-headers target stages BMIN through a dual-mode
  # prerequisite. Supply an already-complete header-only staging directory so
  # a classic consumer does not need to compile BMIN modules (notably, the GCC
  # -fmodules-ts Make path is invalid for Clang).
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
  mkdir -p "${BUNDLE_ROOT}/lib" "${BUNDLE_ROOT}/include"
  cp -f "${SDL2W_ROOT}/sdl2w/lib/libsdl2w.a" "${BUNDLE_ROOT}/lib/"
  cp -f "${SDL2W_ROOT}/sdl2w/lib/libbmin.a" "${BUNDLE_ROOT}/lib/"
  cp -R "${SDL2W_ROOT}/sdl2w/include/." "${BUNDLE_ROOT}/include/"
else
  echo "Building pinned SDL2W/BMIN dual bundle with ${CXX_COMMAND} for ${TARGET}"
  if [[ "${TARGET}" == "wasm" ]]; then
    make "${make_args[@]}" wasm
  else
    make "${make_args[@]}" native TARGET="${TARGET}"
  fi
  "${SDL2W_ROOT}/copy-sdl2w-artifacts.sh" "${BUNDLE_ROOT}"
  if [[ "${TARGET}" == "wasm" && -d "${SDL2W_ROOT}/sdl2w/lib/wasm" ]]; then
    mkdir -p "${BUNDLE_ROOT}/lib/wasm"
    cp -R "${SDL2W_ROOT}/sdl2w/lib/wasm/." "${BUNDLE_ROOT}/lib/wasm/"
  fi
fi

mkdir -p "${BUNDLE_ROOT}"
tmp_key="$(mktemp "${BUNDLE_ROOT}/.build-key.XXXXXX")"
printf '%s\n' "${key}" >"${tmp_key}"
mv -f "${tmp_key}" "${key_file}"
mkdir -p "$(dirname "${source_key_file}")"
tmp_source_key="$(mktemp "${source_key_file}.XXXXXX")"
printf '%s\n' "${key}" >"${tmp_source_key}"
mv -f "${tmp_source_key}" "${source_key_file}"
echo "Prepared ${MODE} SDL2W/BMIN bundle for ${TARGET} (${key:0:12})"
