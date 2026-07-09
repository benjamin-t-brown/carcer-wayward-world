#!/usr/bin/env bash
# Compile all UI tests (no execution). Requires Node — see DEVELOPMENT.md.
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
FAIL=0
PASS=0

for script in "$ROOT"/test-runners/ui/*.sh; do
  name=$(basename "$script")
  echo ""
  echo "========== $name =========="
  if bash "$script" --build-only; then
    PASS=$((PASS + 1))
  else
    echo "FAILED: $name"
    FAIL=$((FAIL + 1))
  fi
done

echo ""
echo "UI tests compiled: $PASS passed, $FAIL failed"
[ "$FAIL" -eq 0 ]
