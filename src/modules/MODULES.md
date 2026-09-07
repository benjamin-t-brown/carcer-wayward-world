# Carcer C++ modules

Carcer ships as C++23 named modules (`carcer.*`). This document describes the
current module boundaries, import policy, and build graph.

Migration status (2026-09): phases 1–7 of `MODULES_V2_PLAN.md` and Phases 0–6
of `MODULES_UI_FINALIZATION_PLAN.md` are complete.
The original class-per-module experiment has been reduced to 20 interfaces and
70 import edges. Data, model, actions, and UI now expose domain-sized APIs;
concrete action and layer implementations are private. Platform qualification
and the graph, stability, compatibility, rebuild-isolation, and literal
artifact-size gates pass. The final 69-second GCC cold-build median misses the
60-second target, so Phase 8 is not authorized yet.

## 1. Design goals

Modules provide isolation only when the exported surface is intentional, and
compile-time leverage only when the dependency graph is shallow. The original
185-micro-module tree recreated the header dependency graph as serialized BMI
work and made `import carcer;` invalidate almost everything.

The current rules are:

- A supported module is a cohesive domain API, not one class or one source
  file.
- Interface files primarily contain declarations. Put bodies in grouped
  implementation units when the compiler supports that shape reliably.
- Implementation classes use module linkage unless another domain genuinely
  needs their type.
- Import the narrowest owning domain. `carcer` is the narrow application entry
  API and does not re-export subsystem declarations.
- Dependency edges point from orchestration toward rules and data, never from
  lower-level state or rules back toward actions or UI.

## 2. Supported boundaries and layering

Dependencies point down this table.

| Layer | Supported modules | Responsibility |
|---|---|---|
| Foundations | `carcer.lib.Json`, `carcer.lib.StringUtil`, external `sdl2w` and `bmin` | Generic utilities and platform wrappers. |
| Static data | `carcer.data`, `carcer.game.map.TileFields` | Immutable definitions and leaf tile-field types. |
| Data access | `carcer.db` | Template loading and lookup registries. |
| Runtime model | `carcer.model` | Mutable characters, maps, items, world, and combat state. |
| Rules | `carcer.game.map`, `carcer.game.combat`, `carcer.game.inventory`, `carcer.in3` | Logic over explicit model/database inputs. |
| State kernel | `carcer.state` | Store, action queue/bus, interfaces, and UI request data. |
| Commands | `carcer.actions` | Public command factories over private action implementations and frame orchestration. |
| UI foundation | `carcer.ui.core` | Scaling, pixels, colors, styles, `UiElement`, and general UI utilities. |
| UI widgets | `carcer.ui.widgets` | Primitives, controls, views, and game-aware composites. |
| Screens | `carcer.ui.screens` | Passive layouts, overlays, pages, and screen runtime. |
| UI controllers | `carcer.ui.layers` | Layers, layer stack, lifecycle, input routing, and interactive orchestration. |
| Entry | `carcer` | Narrow application/bootstrap boundary consumed by `main.cpp`; its implementation starts the layer controller. |

The frame flow is input → `LayerManager` → active `Layer` → widget callback →
`state::actions::Command` → `StateManager` → rules/model mutation →
`WorldUpdater` → layer render.

Actions request screen changes through `state::LayerRequest`; they do not
import UI. `LayerManager` consumes those requests from the layer/controller
side of the boundary.

## 3. UI organisation

The supported UI surface has four boundaries:

```cpp
import carcer.ui.core;
import carcer.ui.widgets;
import carcer.ui.screens;
import carcer.ui.layers;
```

`carcer.ui.widgets` is a six-line public facade over three compiler-sized,
declaration-only build units:

```text
carcer.ui.widgets.foundation
carcer.ui.widgets.views
carcer.ui.widgets.composites
```

`carcer.ui.screens` is one declaration interface with grouped ordinary
implementation units:

```text
ui/screens/runtime.cpp
ui/screens/layouts.cpp
ui/screens/overlays.cpp
ui/screens/pages.cpp
```

The widget method bodies live in ordinary implementation units rather than
BMIs. The dotted modules are build-organisation units. Application code and tests
should use the facades unless they are themselves implementing the UI
subsystem. They are standalone modules rather than standard partitions because
GCC 15 produced corrupt external BMI data when the entire UI was represented
as one very large interface/partition set. Dedicated external-import probes
protect the facade shape on GCC and Clang.

`carcer.ui.layers` is a declarations-only public interface above screens.
Concrete `LayerX` classes have module linkage in its implementation. Code
outside that implementation uses `Layer`, `LayerManager`, and the exported
construction functions:

```cpp
layers::createWorldLayer(window, mapScale);
layers::createInventoryLayer(window);
layers::createPickUpLayer(window);
```

Add another factory or registration seam when a new external caller truly
needs to create a screen. Do not export the concrete layer merely for a test.

## 4. File layout

Broad domains use a small public interface and ordinary implementation units:

```text
src/actions/_actions.cppm      export module carcer.actions;
src/actions/actions.cpp        module carcer.actions;
src/actions/world/WorldUpdater.cpp
```

The same pattern is used by `carcer.data`, `carcer.model`, and most rule/state
domains. An underscore on a facade filename only keeps the entry file easy to
find; it is not part of the module name.

The consolidated UI currently has this shape:

```text
src/ui/_core.cppm
src/ui/_widgets.cppm
src/ui/_widget_{foundation,views,composites}.cppm
src/ui/widgets/{primitives,controls,foundation_views,views,composites}.cpp
src/ui/_screens.cppm
src/ui/screens/{runtime,layouts,overlays,pages}.cpp
src/ui/_layers.cppm
src/ui/layers.cpp
src/layers/{Layer,LayerManager}.cpp
```

`UiElement.cpp`, `FontScale.cpp`, `KeyboardHeldScroll.cpp`, helper `.cpp`
files, `LayerManager.cpp`, and `ChCompactInfo.cpp` are also implementation
units where out-of-line code is useful or avoids a known GCC GCM issue.

Do not infer that a new class needs a new module interface. Add it to the
cohesive owning interface and move substantial bodies to an implementation
unit when practical.

## 5. Import and source rules

- Production and tests import the narrowest supported domain they use. Only
  `main.cpp` and the dedicated application import probe import `carcer`.
- Internal UI implementation may import a grouped dotted module to avoid
  making GCC traverse a facade back into its own implementation graph.
- Never mix classic BMIN/SDL2W headers with their named modules in the same
  translation unit.
- Put standard-library includes in a module interface's global module fragment
  (`module;` before `export module`). In ordinary `.cpp` files, put textual
  includes before imports.
- `macros.h` is the deliberate exception: it only defines project macros and
  remains after module imports. `TRANSLATE` must stay a macro because the
  localization scanner finds it textually.
- Namespaces and module ownership are separate. Existing `ui::`, `state::`,
  and `model::` namespaces do not imply matching micro-modules.

### Intentional public re-exports

Every remaining `export import` is part of a supported domain contract:

- Domain interfaces re-export BMIN containers, and selected adjacent Carcer
  domains, where those named types occur throughout their exported fields,
  bases, return values, or templates. This preserves standalone imports rather
  than forcing consumers to reconstruct an interface's implementation graph.
- `carcer.ui.core`, `carcer.ui.screens`, and `carcer.ui.layers` re-export the
  lower public UI/domain types used directly by their declarations. Screens do
  not re-export layers, and layers privately import screens.
- `carcer.ui.widgets` deliberately re-exports exactly `foundation`, `views`,
  and `composites`. Those three declaration-only modules are compiler-sized
  implementation of one supported facade, not consumer-facing boundaries.
- `carcer` re-exports nothing. It is a narrow application entry contract.

The architecture test rejects new root-module consumers, imports of internal
widget units, screens-to-layers edges, lower-domain-to-UI edges, changes to the
widget facade set, and subsystem re-exports from `carcer`.

## 6. Build graph

CMake/Ninja is the primary compiler-scanned build:

```sh
cmake --preset gcc-debug
cmake --build --preset gcc-debug -j 8
ctest --preset gcc-debug

cmake --preset clang-debug
cmake --build --preset clang-debug -j 8
ctest --preset clang-debug

cmake --preset gcc-release
cmake --build --preset gcc-release -j 8
ctest --preset gcc-release

cmake --preset clang-release
cmake --build --preset clang-release -j 8
ctest --preset clang-release
```

After activating an Emscripten SDK, build both web configurations with:

```sh
source "$EMSDK/emsdk_env.sh"
cmake --preset emscripten-debug
cmake --build --preset emscripten-debug -j 8
cmake --preset emscripten-release
cmake --build --preset emscripten-release -j 8
```

The build prepares Emscripten's SDL ports serially before module scanning.
Successful builds produce `CARCER.js`, `CARCER.wasm`, and `CARCER.data` in the
corresponding preset directory.

The legacy GCC Make path remains available from `src/`:

```sh
make -j8
```

The Make BMI graph is generated from module declarations and imports:

```sh
python3 scripts/modules/gen_bmi_makefile.py
```

This updates:

- `src/modules/make/build-bmi.mk`
- `src/modules/cpp_bmi_deps.mk`
- `src/modules/cppm_sources.list`
- `src/modules/bmi_objs.list`
- `src/modules/module_order.txt`

Regenerate after adding/removing an interface or changing an import edge. The
current graph has 20 interfaces, 70 edges, critical depth 10, and maximum
transitive fan-out 16.

SDL2W and BMIN revisions are pinned in the repository-level `deps.lock` and
materialized into `.deps/`/the consumer bundle by the bootstrap scripts. CMake
and Make validate those revisions before building. On native Make builds,
`sdl2-config --cflags` supplies the platform SDL include flags needed by UI
interfaces that mention SDL types.

Preset build directories segregate BMIs by compiler, target, and configuration.
Dependency bundles are additionally keyed by compiler path/version, target,
mode, flags, and `deps.lock`; `build-deps.sh` cleans shared upstream Make
outputs when that identity changes. Do not manually copy BMIs or dependency
objects between preset directories. The legacy Make `gcm.cache` is not keyed,
so clean it before changing the compiler while that build remains available.

## 7. Tests and boundary checks

The public interfaces have standalone import probes in
`src/__test__/modules/`. The UI probes are:

- `ImportUiCore.cpp`
- `ImportUiWidgets.cpp`
- `ImportUiScreens.cpp`
- `ImportUiLayers.cpp`

Keep a probe minimal: import the supported facade, instantiate or reference a
small representative API, and link it outside the owning module target. A
successful in-tree build alone is insufficient to catch corrupted or
incomplete exported BMI data.

The regular CTest suite covers non-visual behavior. The `carcer_ui_tests`
CMake target compiles and links all 43 UI programs even though those programs
are not registered as headless runtime tests.

The compatibility and repeatability checks are:

```sh
scripts/validate-dependency-headers.sh g++-15
scripts/validate-dependency-headers.sh clang++
scripts/validate-native-repeatability.sh gcc-debug 10
scripts/validate-native-repeatability.sh clang-debug 10
```

The header probe uses a separately keyed consumer bundle and never mixes
classic SDL2W/BMIN headers with named-module imports. Repeatability logs are
written below the ignored `build/validation/` directory.

The Phase 6 qualification matrix passes GCC 15.3 and Homebrew Clang 22.1.8 in
debug and release, all 43 UI compile/link programs in every native
configuration, 10/10 clean debug builds per native compiler, Emscripten 6.0.9
debug and release, both dependency API modes, and the clean legacy Make build.
Three fresh GCC debug measurements were 68, 69, and 69 seconds (median 69),
with 0-second median no-op work, three-second leaf rebuilds, and 277,056 KiB of
objects/archives/BMIs. Only the 60-second cold-build target remains unmet.

## 8. Adding or changing code

1. Choose the lowest cohesive domain that owns the behavior.
2. Add declarations only when another domain needs them; keep implementation
   types private.
3. Import the narrow supported boundary, not `carcer`.
4. Regenerate the Make graph when interfaces or imports change.
5. Run the relevant public import probe before wiring a changed facade into
   wider consumers.
6. Build and test with both native compiler presets. During compatibility work,
   also run the legacy Make build.

`scripts/modules/consolidate_module.py` remains a migration helper for
mechanically combining old interfaces. Review its output for ownership and
exports; it cannot decide the correct public API. The generated graph script is
permanent build tooling and must remain after the migration scripts are
eventually removed in Phase 8.
