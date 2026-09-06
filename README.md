# Carcer: Wayward World

This is a 2d, grid based adventure rpg featuring a robust story with consequential actions, complex dialog trees, dnd-like combat system, and a persistent open world.

## Quick Start

```
./scripts/bootstrap-deps.sh
cmake --preset gcc-debug
cmake --build --preset gcc-debug
ctest --preset gcc-debug
```

The experimental C++ module build uses CMake's compiler dependency scanner and
Ninja. The original Make build remains available with `make -C src -j8` while
the module redesign is evaluated. See [DEVELOPMENT.md](DEVELOPMENT.md) for
compiler requirements, other presets, UI test compilation, and known stale
tests.
