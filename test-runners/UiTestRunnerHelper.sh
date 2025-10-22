#!/bin/bash

# Generic UI Test Runner Helper
# Usage: ./UiTestRunnerHelper.sh <test_file_name> [--build-only]
#
# Arguments:
#   test_file_name: Name of the test file (without .cpp extension)
#   --build-only: Optional flag to only build the test without running it

if [ -z "$1" ]; then
  echo "Error: Test file name is required"
  echo "Usage: $0 <test_file_name> [--build-only]"
  exit 1
fi

TEST_FILE_NAME=$1
TEST_EXE_NAME=TestUi
SCRIPT_DIR=$(dirname "$0")

TOP_LEVEL_DIR=$(dirname $0)/..
SRC_DIR=$TOP_LEVEL_DIR/src

echo "SCRIPT DIR: $SCRIPT_DIR"

rm -rf $SRC_DIR/$TEST_EXE_NAME.*

echo "compile objects"
(cd $SRC_DIR  && make object_files -j8)

echo "compile test"
COMPILER_ARGS=$(cd $SRC_DIR && make compiler_args)
COMPILE_TEST="g++ test/ui/$TEST_FILE_NAME.cpp $COMPILER_ARGS -o $TEST_EXE_NAME"
echo $COMPILE_TEST
(cd $SRC_DIR  && $COMPILE_TEST)

if [ "$2" != "--build-only" ]; then
  echo "run test"
  (cd $SRC_DIR && ./$TEST_EXE_NAME)
fi

