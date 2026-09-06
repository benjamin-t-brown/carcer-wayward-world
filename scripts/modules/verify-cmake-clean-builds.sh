#!/usr/bin/env bash

set -uo pipefail

PRESET="${1:-gcc-debug}"
RUNS="${2:-10}"
TARGET="${3:-CARCER}"
JOBS="${4:-8}"

if ! [[ "$RUNS" =~ ^[1-9][0-9]*$ ]]; then
  echo "Run count must be a positive integer: $RUNS" >&2
  exit 2
fi
if ! [[ "$JOBS" =~ ^[1-9][0-9]*$ ]]; then
  echo "Job count must be a positive integer: $JOBS" >&2
  exit 2
fi

ROOT="$(cd "$(dirname "$0")/../.." && pwd)"
BUILD_DIR="$ROOT/build/cmake/$PRESET"
STAMP="$(date -u +%Y%m%dT%H%M%SZ)"
RESULT_DIR="$ROOT/build/benchmarks/${STAMP}-${PRESET}-clean"
RESULTS="$RESULT_DIR/results.csv"
METADATA="$RESULT_DIR/metadata.csv"

mkdir -p "$RESULT_DIR"

if ! cmake --preset "$PRESET" >"$RESULT_DIR/configure.log" 2>&1; then
  echo "Configure failed; see $RESULT_DIR/configure.log" >&2
  exit 1
fi

CXX="$(sed -n 's/^CMAKE_CXX_COMPILER:[^=]*=//p' "$BUILD_DIR/CMakeCache.txt" | head -1)"
if [ -z "$CXX" ]; then
  CXX="unknown"
fi

printf '%s\n' 'key,value' >"$METADATA"
printf 'timestamp_utc,%s\n' "$STAMP" >>"$METADATA"
printf 'preset,%s\n' "$PRESET" >>"$METADATA"
printf 'target,%s\n' "$TARGET" >>"$METADATA"
printf 'runs,%s\n' "$RUNS" >>"$METADATA"
printf 'jobs,%s\n' "$JOBS" >>"$METADATA"
printf 'os,%s\n' "$(uname -s)" >>"$METADATA"
printf 'architecture,%s\n' "$(uname -m)" >>"$METADATA"
printf 'logical_cpus,%s\n' "$(getconf _NPROCESSORS_ONLN 2>/dev/null || echo unknown)" >>"$METADATA"
printf 'cmake,%s\n' "$(cmake --version | sed -n '1s/^cmake version //p')" >>"$METADATA"
printf 'ninja,%s\n' "$(ninja --version)" >>"$METADATA"
printf 'compiler_path,%s\n' "$CXX" >>"$METADATA"
if [ "$CXX" != unknown ]; then
  printf 'compiler_version,%s\n' "$($CXX --version | head -1 | tr ',' ';')" >>"$METADATA"
fi
printf 'carcer_commit,%s\n' "$(git -C "$ROOT" rev-parse HEAD)" >>"$METADATA"
printf 'carcer_branch,%s\n' "$(git -C "$ROOT" branch --show-current)" >>"$METADATA"
if [ -n "$(git -C "$ROOT" status --porcelain)" ]; then
  printf '%s\n' 'carcer_worktree,dirty' >>"$METADATA"
else
  printf '%s\n' 'carcer_worktree,clean' >>"$METADATA"
fi
printf 'sdl2w_commit,%s\n' "$(git -C "$ROOT/.deps/sdl2w" rev-parse HEAD)" >>"$METADATA"
printf 'bmin_commit,%s\n' "$(git -C "$ROOT/.deps/bmin" rev-parse HEAD)" >>"$METADATA"

printf '%s\n' 'run,seconds,status' >"$RESULTS"
FAILURES=0

for ((run = 1; run <= RUNS; run++)); do
  CLEAN_LOG="$RESULT_DIR/run-${run}-clean.log"
  BUILD_LOG="$RESULT_DIR/run-${run}-build.log"

  if ! cmake --build "$BUILD_DIR" --target clean >"$CLEAN_LOG" 2>&1; then
    printf '%s,%s,%s\n' "$run" 0 clean-failed >>"$RESULTS"
    echo "[$run/$RUNS] clean failed"
    FAILURES=$((FAILURES + 1))
    continue
  fi

  START="$(date +%s)"
  if cmake --build --preset "$PRESET" --target "$TARGET" --parallel "$JOBS" \
      >"$BUILD_LOG" 2>&1; then
    STATUS=passed
  else
    STATUS=failed
    FAILURES=$((FAILURES + 1))
  fi
  END="$(date +%s)"
  ELAPSED=$((END - START))
  printf '%s,%s,%s\n' "$run" "$ELAPSED" "$STATUS" >>"$RESULTS"
  echo "[$run/$RUNS] $STATUS in ${ELAPSED}s"
done

echo "Results: $RESULT_DIR"
if [ "$FAILURES" -ne 0 ]; then
  echo "$FAILURES of $RUNS clean builds failed" >&2
  exit 1
fi

echo "$RUNS of $RUNS clean builds passed"
