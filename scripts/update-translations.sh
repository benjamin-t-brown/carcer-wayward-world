#!/bin/bash

cd ../sdl2w/src

if [ ! -f "build/tools/L10nScanner" ] && [ ! -f "build/tools/L10nScanner.exe" ]; then
  echo "Building L10nScanner..."
  make clean
  make -j8
  make tools
fi

L10N_SCANNER_COMMAND="build/tools/L10nScanner --input-dir ../../src --output-dir ../../src/assets en la"
echo "Running: $L10N_SCANNER_COMMAND"
$L10N_SCANNER_COMMAND