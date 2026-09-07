#!/usr/bin/env bash
# Run the CMake/CTest acceptance suite and compile every UI test.
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
exec "$ROOT/scripts/run-all-tests-ucrt64.sh"
