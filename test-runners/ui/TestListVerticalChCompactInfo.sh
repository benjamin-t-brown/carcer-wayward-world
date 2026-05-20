#!/bin/bash
SCRIPT_DIR=$(dirname "$0")
node "$SCRIPT_DIR/../UiTestRunnerHelper.js" components/lists TestListVerticalChCompactInfo "$@"
