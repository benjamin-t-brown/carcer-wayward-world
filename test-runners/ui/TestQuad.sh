TEST_EXE_NAME=TestUi
TEST_FILE_NAME=TestQuad
SRC_DIR=../../src

rm -rf $SRC_DIR/$TEST_EXE_NAME.*

echo "compile objects"
(cd $SRC_DIR  && make object_files -j8)

echo "compile test"
COMPILER_ARGS=$(cd $SRC_DIR && make compiler_args)
COMPILE_TEST="g++ test/ui/$TEST_FILE_NAME.cpp $COMPILER_ARGS -o $TEST_EXE_NAME"
echo $COMPILE_TEST
(cd $SRC_DIR  && $COMPILE_TEST)

echo "run test"
(cd $SRC_DIR && ./$TEST_EXE_NAME)