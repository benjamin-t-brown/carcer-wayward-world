#!/usr/bin/env bash
# Run all non-UI C++ tests; compile UI tests only (no execution).
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
cd "$ROOT/src"

echo "Building object_files..."
make object_files -j8

CXX=$(make print_cxx)
ARGS=$(make compiler_args)

FAILED=0
PASSED=0
COMPILED=0
COMPILE_FAILED=0

run_cpp() {
  local cpp="$1"
  local run_it="$2"
  echo ""
  echo "========== $cpp =========="
  if ! $CXX "$cpp" $ARGS -o TestUi; then
    echo "COMPILE FAILED: $cpp"
    if [ "$run_it" = "run" ]; then
      FAILED=$((FAILED + 1))
    else
      COMPILE_FAILED=$((COMPILE_FAILED + 1))
    fi
    return
  fi
  if [ "$run_it" = "run" ]; then
    if ./TestUi; then
      PASSED=$((PASSED + 1))
    else
      echo "RUN FAILED: $cpp"
      FAILED=$((FAILED + 1))
    fi
  else
    echo "compile-only OK"
    COMPILED=$((COMPILED + 1))
  fi
}

echo ""
echo "===== RUNNER TESTS ====="
for cpp in __test__/runner/*.cpp; do
  [ -f "$cpp" ] || continue
  run_cpp "$cpp" run
done

echo ""
echo "===== DB TESTS ====="
for cpp in __test__/db/loaders/*.cpp; do
  [ -f "$cpp" ] || continue
  run_cpp "$cpp" run
done

echo ""
echo "===== MODEL TESTS ====="
for cpp in __test__/model/*.cpp; do
  [ -f "$cpp" ] || continue
  run_cpp "$cpp" run
done

echo ""
echo "===== UI TESTS (compile only) ====="
while IFS= read -r -d '' cpp; do
  run_cpp "$cpp" compile
done < <(find __test__/ui -name '*.cpp' -print0 | sort -z)

echo ""
echo "Summary: ran $PASSED passed, $FAILED failed; UI compiled $COMPILED, compile failures $COMPILE_FAILED"
if [ "$FAILED" -ne 0 ] || [ "$COMPILE_FAILED" -ne 0 ]; then
  exit 1
fi

echo "All tests passed."
