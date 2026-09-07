# Carcer module architecture

Carcer is a C++23 named-module application built with CMake/Ninja dependency
scanning. The production graph has 20 interface units, 70 import edges, and a
critical depth of 10.

## Boundaries

Dependencies point down this table; orchestration may depend on rules and data,
but lower layers must not depend back on actions or UI.

| Layer | Modules | Responsibility |
|---|---|---|
| Foundation | `carcer.lib.Json`, `carcer.lib.StringUtil`, `sdl2w`, `bmin` | Generic utilities and platform wrappers |
| Data | `carcer.data`, `carcer.db`, `carcer.game.map.TileFields` | Definitions, lookup registries, and leaf tile types |
| Model | `carcer.model` | Mutable game state values |
| Rules | `carcer.game.map`, `carcer.game.combat`, `carcer.game.inventory`, `carcer.in3` | Logic over explicit model/database inputs |
| State | `carcer.state` | Store, action queue/bus, service interfaces, and UI requests |
| Commands | `carcer.actions` | Public command factories and frame orchestration |
| UI core | `carcer.ui.core` | UI primitives, scaling, style, and `UiElement` |
| Widgets | `carcer.ui.widgets` | Controls, reusable views, and composites |
| Screens | `carcer.ui.screens` | Passive layouts, pages, overlays, and screen runtime |
| Controllers | `carcer.ui.layers` | Layer stack, lifecycle, input routing, and interaction |
| Application | `carcer` | Narrow bootstrap API used by `main.cpp` |

The runtime flow is input → `LayerManager` → active `Layer` → widget callback
→ command → `StateManager` → rules/model mutation → `WorldUpdater` → render.
Actions request screen changes through `state::LayerRequest`; they never import
UI. Layers own screen composition and consume those requests.

`carcer` is intentionally retained as the narrow application boundary because
it will eventually contain the complete game bootstrap. It is not an umbrella:
it re-exports no subsystem, and only `main.cpp` plus its import probe may import
it.

## Source shape

A public module represents a cohesive domain, not one class. Interfaces should
mostly contain declarations; substantial method bodies belong in grouped
implementation units. Implementation-only types have module linkage unless a
different domain genuinely needs the type.

The widget facade re-exports three compiler-sized declaration units:

```text
carcer.ui.widgets.foundation
carcer.ui.widgets.views
carcer.ui.widgets.composites
```

These are build-organization units, not consumer APIs. Application code and
tests import `carcer.ui.widgets`. This shape avoids a GCC 15 corruption issue
seen with one very large UI interface while preserving one supported boundary.

`carcer.ui.screens` is a declaration interface backed by grouped implementation
files. `carcer.ui.layers` sits above it and exposes `Layer`, `LayerManager`, and
construction functions; concrete `LayerX` types remain private.

## Import contract

- Import the narrowest supported owning domain. Do not use `carcer` as a
  convenience import.
- Screens never import layers. Layers import screens and own loop management
  and interactivity.
- State, actions, rules, model, database, and data never import UI.
- Do not import internal `carcer.ui.widgets.*` units outside the UI
  implementation.
- Never mix classic SDL2W/BMIN headers with their named modules in one
  translation unit.
- Put standard-library includes in an interface's global module fragment
  (`module;` before `export module`). In implementation files, textual includes
  precede imports.
- `src/modules/macros.h` is the deliberate exception: localization keeps
  `TRANSLATE` textual for the scanner.
- A namespace does not imply a separate module. Add declarations to the owning
  cohesive interface instead of creating a module per class.

Intentional `export import` declarations are limited to types that form part of
a domain contract and to the three-unit widget facade. The application module
must never re-export a subsystem.

## Build and test contract

Dependencies are pinned by `deps.lock` and explicitly bootstrapped into
`.deps/`. Configure and build validate the revisions without mutating them.

```sh
./scripts/bootstrap-deps.sh
cmake --preset gcc-debug
cmake --build --preset gcc-debug
ctest --preset gcc-debug
cmake --build --preset gcc-debug --target carcer_ui_tests
```

The equivalent native presets are `gcc-release`, `clang-debug`, and
`clang-release`. Emscripten uses `emscripten-debug` and `emscripten-release`.
MSYS2 UCRT64 uses `ucrt64-debug` and `ucrt64-release` through
`scripts/Invoke-Ucrt64.ps1`.

CMake discovers module order with the compiler scanner. Do not add checked-in
BMI manifests, hand-maintained ordering, or copied dependency trees. Every
preset has its own build directory so compiler, target, and configuration
artifacts cannot mix.

Public interfaces have standalone probes under `src/__test__/modules/`. The
regular CTest suite covers non-visual behavior; `carcer_ui_tests` compiles and
links all 43 interactive UI programs without running them.

Permanent architecture and performance checks are:

```sh
./scripts/modules/check_ui_architecture.sh
./scripts/benchmark-fresh-modules.sh gcc-debug
./scripts/validate-native-repeatability.sh gcc-debug 10
./scripts/validate-dependency-headers.sh g++-15
```

The architecture check is registered with CTest. Benchmark and repeatability
results are written below ignored `build/` directories.

## Qualification record

The adopted graph passes GCC 15.3 and Clang 22.1.8 debug/release builds, 10/10
repeated clean builds for each native compiler, 39 enabled CTests, all 43 UI
compile/link tests, Emscripten 6.0.9 debug/release builds, and both dependency
API modes. Five known-stale runtime tests remain explicitly disabled and three
sources targeting disabled APIs remain excluded in `CMakeLists.txt`.

Fresh GCC debug application builds measured 68, 69, and 69 seconds (median 69),
with a zero-second median no-op build, three-second leaf rebuilds, and 277,056
KiB of objects, archives, and BMIs. The original 60-second cold target was not
met; after one bounded optimization pass yielded no meaningful improvement,
the 69-second result was explicitly accepted on 2026-09-07 to complete module
adoption.

When changing a boundary, run its import probe, the architecture check, the
relevant behavioral tests, and both native compiler builds. Re-run the fresh
benchmark when an interface or high-fan-out import changes.
