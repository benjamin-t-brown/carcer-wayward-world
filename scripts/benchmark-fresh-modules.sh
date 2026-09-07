#!/usr/bin/env bash
# Benchmark an uncontaminated native module build in a newly-created directory.
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
PRESET="${1:-gcc-debug}"
JOBS="${CARCER_BUILD_JOBS:-8}"
LEAF_SOURCE="${CARCER_BENCHMARK_LEAF:-${ROOT}/src/lib/Json.cpp}"

case "${PRESET}" in
  gcc-debug|gcc-release|clang-debug|clang-release) ;;
  *)
    echo "usage: $0 {gcc-debug|gcc-release|clang-debug|clang-release}" >&2
    exit 2
    ;;
esac

if ! [[ "${JOBS}" =~ ^[1-9][0-9]*$ ]]; then
  echo "error: CARCER_BUILD_JOBS must be a positive integer" >&2
  exit 2
fi
if [[ ! -f "${LEAF_SOURCE}" ]]; then
  echo "error: benchmark leaf source does not exist: ${LEAF_SOURCE}" >&2
  exit 2
fi

benchmark_parent="${ROOT}/build/benchmarks"
mkdir -p "${benchmark_parent}"
build_root="$(mktemp -d "${benchmark_parent}/${PRESET}.XXXXXX")"
configure_log="${build_root}/configure.log"
build_log="${build_root}/clean-build.log"
noop_log="${build_root}/noop-build.log"
leaf_log="${build_root}/leaf-build.log"
results="${build_root}/results.tsv"

echo "Configuring fresh benchmark directory: ${build_root}"
if ! cmake --preset "${PRESET}" -B "${build_root}" \
    -DBUILD_TESTING=OFF \
    -DCARCER_BUILD_TESTS=OFF \
    -DCARCER_BUILD_UI_TESTS=OFF >"${configure_log}" 2>&1; then
  tail -n 80 "${configure_log}" >&2
  exit 1
fi

SECONDS=0
if ! cmake --build "${build_root}" --target CARCER -j "${JOBS}" \
    >"${build_log}" 2>&1; then
  tail -n 80 "${build_log}" >&2
  exit 1
fi
cold_seconds="${SECONDS}"

SECONDS=0
if ! cmake --build "${build_root}" --target CARCER -j "${JOBS}" \
    >"${noop_log}" 2>&1; then
  tail -n 80 "${noop_log}" >&2
  exit 1
fi
noop_seconds="${SECONDS}"

mtime_reference="${build_root}/leaf-source.mtime"
touch -r "${LEAF_SOURCE}" "${mtime_reference}"
restore_leaf_mtime() {
  touch -r "${mtime_reference}" "${LEAF_SOURCE}"
}
trap restore_leaf_mtime EXIT
touch "${LEAF_SOURCE}"
SECONDS=0
if ! cmake --build "${build_root}" --target CARCER -j "${JOBS}" \
    >"${leaf_log}" 2>&1; then
  tail -n 80 "${leaf_log}" >&2
  exit 1
fi
leaf_seconds="${SECONDS}"
restore_leaf_mtime
trap - EXIT

sum_kib() {
  find "${build_root}" -type f "$@" -exec du -k {} + |
    awk '{ total += $1 } END { print total + 0 }'
}

artifact_kib="$(sum_kib \( -name '*.o' -o -name '*.a' -o -name '*.gcm' \
  -o -name '*.bmi' -o -name '*.pcm' \))"
scan_kib="$(sum_kib -name '*.ddi.i')"
total_kib="$(du -sk "${build_root}" | awk '{print $1}')"

printf 'metric\tvalue\n' >"${results}"
printf 'preset\t%s\n' "${PRESET}" >>"${results}"
printf 'jobs\t%s\n' "${JOBS}" >>"${results}"
printf 'cold_seconds\t%s\n' "${cold_seconds}" >>"${results}"
printf 'noop_seconds\t%s\n' "${noop_seconds}" >>"${results}"
printf 'leaf_seconds\t%s\n' "${leaf_seconds}" >>"${results}"
printf 'artifact_kib\t%s\n' "${artifact_kib}" >>"${results}"
printf 'scan_preprocessed_kib\t%s\n' "${scan_kib}" >>"${results}"
printf 'total_build_kib\t%s\n' "${total_kib}" >>"${results}"
printf 'leaf_source\t%s\n' "${LEAF_SOURCE}" >>"${results}"

cat "${results}"
echo "Logs and results: ${build_root}"
