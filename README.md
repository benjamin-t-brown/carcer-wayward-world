# Carcer: Wayward World

This is a 2d, grid based adventure rpg featuring a robust story with consequential actions, complex dialog trees, dnd-like combat system, and a persistent open world.

## Quick Start

Install these first. `setup-dev.sh` checks for them and stops if any are missing.

- git, CMake 3.28 or newer, Ninja, Make, pkg-config, and clangd
- a C++23 compiler (GCC or Clang)
- SDL2, SDL2_image, SDL2_ttf, SDL2_mixer, and SDL2_gfx

MSYS2 UCRT64:

```sh
pacman -S \
  git \
  mingw-w64-ucrt-x86_64-gcc \
  mingw-w64-ucrt-x86_64-cmake \
  mingw-w64-ucrt-x86_64-ninja \
  mingw-w64-ucrt-x86_64-make \
  mingw-w64-ucrt-x86_64-pkgconf \
  mingw-w64-ucrt-x86_64-clang \
  mingw-w64-ucrt-x86_64-clang-tools-extra \
  mingw-w64-ucrt-x86_64-SDL2 \
  mingw-w64-ucrt-x86_64-SDL2_image \
  mingw-w64-ucrt-x86_64-SDL2_ttf \
  mingw-w64-ucrt-x86_64-SDL2_mixer \
  mingw-w64-ucrt-x86_64-SDL2_gfx
```

Debian and Ubuntu:

```sh
sudo apt-get install build-essential cmake ninja-build pkg-config clangd \
  libsdl2-dev libsdl2-image-dev libsdl2-ttf-dev libsdl2-mixer-dev libsdl2-gfx-dev
```

macOS:

```sh
brew install cmake ninja llvm pkg-config sdl2 sdl2_image sdl2_ttf sdl2_mixer sdl2_gfx
```

The web build also needs an activated Emscripten SDK (`EMSDK` set).

On Windows, open an MSYS2 UCRT64 terminal first. That shell already has the toolchain on `PATH`, so the commands below are the whole setup.

```sh
./scripts/setup-dev.sh
cd src
make
```

Carcer is a conventional C++23 header/source project. It relies on two external libs built from source on github both published by myself.
  - SDL2W is a wrapper for SDL2 that helps with some windowing/sprite drawing/animation game dev stuff.  It also handles asset loading
  - BMIN is a cpp std library replacement, built to improve upon the standard library for these simple game development purposes.  It's primary function is to compile faster than stl equivalents but provide the same, if better functionality (opinionated).  SDL2W depends on this lib.

SDL2W and BMIN are pinned to their module-capable experimental revisions, but this project deliberately consumes their classic header API. See [DEVELOPMENT.md](DEVELOPMENT.md) for other compilers and targets.

From `src/`, use `make run` to build and run the game, `make test` to build and
run the unit suite, `make ui` to compile every interactive UI test without
opening windows, and `make js` to build the Emscripten release.
