# Carcer C++ modules

Carcer ships as C++23 named modules (`carcer.*`). This document describes the
current module boundaries, import policy, and build graph.

Migration status (2026-09): phases 1–6 of `MODULES_V2_PLAN.md` are complete.
The original class-per-module experiment has been reduced to 26 interfaces and
117 import edges. Data, model, actions, and UI now expose domain-sized APIs;
concrete action and layer implementations are private.

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
- Import the narrowest owning domain. The `carcer` umbrella exists for the
  entry point during migration, not as a default dependency.
- Dependency edges point from orchestration toward rules and data, never from
  lower-level state or rules back toward actions or UI.

## 2. Supported boundaries and layering

Dependencies point down this table.

| Layer | Supported modules | Responsibility |
|---|---|---|
| Foundations | `carcer.lib.Json`, `carcer.lib.StringUtil`, `carcer.lib.hiscore.hiscore`, external `sdl2w` and `bmin` | Generic utilities and platform wrappers. |
| Static data | `carcer.data`, `carcer.game.map.TileFields` | Immutable definitions and leaf tile-field types. |
| Data access | `carcer.db` | Template loading and lookup registries. |
| Runtime model | `carcer.model` | Mutable characters, maps, items, world, and combat state. |
| Rules | `carcer.game.map`, `carcer.game.combat`, `carcer.game.inventory`, `carcer.in3` | Logic over explicit model/database inputs. |
| State kernel | `carcer.state` | Store, action queue/bus, interfaces, and UI request data. |
| Commands | `carcer.actions` | Public command factories over private action implementations and frame orchestration. |
| UI foundation | `carcer.ui.core` | Scaling, pixels, colors, styles, `UiElement`, and general UI utilities. |
| UI widgets | `carcer.ui.widgets` | Primitives, controls, views, and game-aware composites. |
| Screens | `carcer.ui.screens` | Layouts, overlays, pages, screen runtime, and the layer stack. |
| Entry | `carcer` | Temporary umbrella consumed by `main.cpp`. |

The frame flow is input → `LayerManager` → active `Layer` → widget callback →
`state::actions::Command` → `StateManager` → rules/model mutation →
`WorldUpdater` → layer render.

Actions request screen changes through `state::LayerRequest`; they do not
import UI. `LayerManager` consumes those requests from the screen side of the
boundary.

## 3. UI organisation

The supported UI surface is exactly three facades:

```cpp
import carcer.ui.core;
import carcer.ui.widgets;
import carcer.ui.screens;
```

`carcer.ui.widgets` reexports four cohesive implementation modules:

```text
carcer.ui.widgets.primitives
carcer.ui.widgets.controls
carcer.ui.widgets.views
carcer.ui.widgets.composites
```

`carcer.ui.screens` reexports five:

```text
carcer.ui.screens.runtime
carcer.ui.screens.layouts
carcer.ui.screens.overlays
carcer.ui.screens.pages
carcer.ui.screens.layers
```

These dotted modules are build-organisation units. Application code and tests
should use the facades unless they are themselves implementing the UI
subsystem. They are standalone modules rather than standard partitions because
GCC 15 produced corrupt external BMI data when the entire UI was represented
as one very large interface/partition set. Dedicated external-import probes
protect the facade shape on GCC and Clang.

Concrete `LayerX` classes in `carcer.ui.screens.layers` have module linkage.
Code outside the implementation uses `Layer`, `LayerManager`, and the exported
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
src/ui/widgets/{primitives,controls,views,composites}.cppm
src/ui/_screens.cppm
src/ui/screens/{runtime,layouts,overlays,pages,layers}.cppm
```

`UiElement.cpp`, `FontScale.cpp`, `KeyboardHeldScroll.cpp`, helper `.cpp`
files, `LayerManager.cpp`, and `ChCompactInfo.cpp` remain implementation units
where out-of-line code is useful or avoids a known GCC GCM issue.

Do not infer that a new class needs a new module interface. Add it to the
cohesive owning interface and move substantial bodies to an implementation
unit when practical.

## 5. Import and source rules

- Production and tests import the narrowest supported domain they use. No test
  should import `carcer`; `main.cpp` is its only intended consumer while the
  umbrella remains.
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

## 6. Build graph

CMake/Ninja is the primary compiler-scanned build:

```sh
cmake --preset gcc-debug
cmake --build --preset gcc-debug -j 8
ctest --preset gcc-debug

cmake --preset clang-debug
cmake --build --preset clang-debug -j 8
ctest --preset clang-debug
```

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
current graph has 26 interfaces, 117 edges, critical depth 14, and maximum
transitive fan-out 21.

SDL2W and BMIN revisions are pinned in the repository-level `deps.lock` and
materialized into `.deps/`/the consumer bundle by the bootstrap scripts. CMake
and Make validate those revisions before building. On native Make builds,
`sdl2-config --cflags` supplies the platform SDL include flags needed by UI
interfaces that mention SDL types.

## 7. Tests and boundary checks

The public interfaces have standalone import probes in
`src/__test__/modules/`. The UI probes are:

- `ImportUiCore.cpp`
- `ImportUiWidgets.cpp`
- `ImportUiScreens.cpp`

Keep a probe minimal: import the supported facade, instantiate or reference a
small representative API, and link it outside the owning module target. A
successful in-tree build alone is insufficient to catch corrupted or
incomplete exported BMI data.

The regular CTest suite covers non-visual behavior. The `carcer_ui_tests`
CMake target compiles and links all 43 UI programs even though those programs
are not registered as headless runtime tests.

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
