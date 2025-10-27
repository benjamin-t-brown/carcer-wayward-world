#!/bin/bash

if [ ! -f "../src/Anims" ] && [ ! -f "../src/Anims.exe" ]; then
  echo "go to sdl2w src folder"
  cd ../sdl2w/src
  echo "Building Anims..."
  make clean
  make -j8
  make tools
  echo "cp Anims to top level src folder"
  cp build/tools/Anims ../../src
  cd ../../scripts
fi

echo "cd to top level src folder"
cd ../src

ls
./Anims --assets-dir assets --asset-file assets/assets.txt