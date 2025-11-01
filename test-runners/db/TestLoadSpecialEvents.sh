#!/bin/bash
SCRIPT_DIR=$(dirname "$0")
node "$SCRIPT_DIR/../TestRunnerHelper.js" db loaders TestLoadSpecialEvents "$@"
