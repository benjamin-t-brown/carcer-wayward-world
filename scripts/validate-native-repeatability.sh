#!/usr/bin/env bash
# Repeatedly clean and rebuild one configured native CMake preset.
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
PRESET="${1:-}"
RUNS="${2:-10}"
JOBS="${CARCER_BUILD_JOBS:-8}"

case "${PRESET}" in
  gcc-debug|gcc-release|clang-debug|clang-release) ;;
  *)
    echo "usage: $0 {gcc-debug|gcc-release|clang-debug|clang-release} [runs]" >&2
    exit 2
    ;;
esac
if ! [[ "${RUNS}" =~ ^[1-9][0-9]*$ ]]; then
  echo "error: runs must be a positive integer" >&2
  exit 2
fi

result_root="${ROOT}/build/validation/${PRESET}"
summary="${result_root}/repeated-clean.tsv"
mkdir -p "${result_root}"
printf 'run\tseconds\n' >"${summary}"

for ((run = 1; run <= RUNS; ++run)); do
  log="${result_root}/run-${run}.log"
  cmake --build --preset "${PRESET}" --target clean >"${log}" 2>&1
  started="$(date +%s)"
  if ! cmake --build --preset "${PRESET}" -j "${JOBS}" >>"${log}" 2>&1; then
    echo "${PRESET} clean build ${run}/${RUNS} failed; tail follows" >&2
    tail -n 80 "${log}" >&2
    exit 1
  fi
  elapsed="$(( $(date +%s) - started ))"
  printf '%s\t%s\n' "${run}" "${elapsed}" >>"${summary}"
  echo "${PRESET} clean build ${run}/${RUNS} passed in ${elapsed}s"
done

echo "Wrote ${summary}"
