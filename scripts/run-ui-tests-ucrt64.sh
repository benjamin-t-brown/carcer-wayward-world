#!/usr/bin/env bash
# Build and run every interactive UI test in sequence.
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
PRESET="${CARCER_CMAKE_PRESET:-ucrt64-debug}"
BUILD_DIR="$ROOT/build/cmake/$PRESET"
EXE_SUFFIX=""
if [[ "${OS:-}" == "Windows_NT" ]]; then
  EXE_SUFFIX=".exe"
fi

cd "$ROOT"
cmake --preset "$PRESET"
cmake --build --preset "$PRESET" --target carcer_ui_tests

passed=0
failed=0
while IFS= read -r source; do
  name="$(basename "${source%.cpp}")"
  executable="$BUILD_DIR/carcer_test_${name}${EXE_SUFFIX}"
  echo "========== $name =========="
  if (cd "$ROOT/src" && "$executable"); then
    passed=$((passed + 1))
  else
    failed=$((failed + 1))
  fi
done < <(find "$ROOT/src/__test__/ui" -name '*.cpp' | sort)

echo "UI tests: $passed passed, $failed failed"
[[ "$failed" -eq 0 ]]
