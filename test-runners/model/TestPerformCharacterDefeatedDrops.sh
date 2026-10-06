#!/usr/bin/env bash
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
node "$SCRIPT_DIR/../TestRunnerHelper.js" model . TestPerformCharacterDefeatedDrops "$@"
