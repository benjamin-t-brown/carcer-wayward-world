# Carcer: Wayward World

This is a 2d, grid based adventure rpg featuring a robust story with consequential actions, complex dialog trees, dnd-like combat system, and a persistent open world.

## Quick Start

```sh
./scripts/setup-dev.sh
cd src
make
```

Carcer is a conventional C++23 header/source project. SDL2W and BMIN are pinned
to their module-capable experimental revisions, but this project deliberately
consumes their classic header API. See [DEVELOPMENT.md](DEVELOPMENT.md) for
other compilers and targets.

From `src/`, use `make run` to build and run the game, `make test` to build and
run the unit suite, `make ui` to compile every interactive UI test without
opening windows, and `make js` to build the Emscripten release.

## Game database editor

[`ceditor2`](ceditor2/README.md) is the lightweight TypeScript/Vite editor for
the JSON game database. It runs each asset editor as an independent native DOM
application and saves the complete database through one revision-checked
transaction. The older React editor under `ceditor` is retained only for
migration comparison.
