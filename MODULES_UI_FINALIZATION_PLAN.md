# Carcer Module/UI Finalization Plan

Status: Phase 1 complete; ready for Phase 2
Pre-plan application commit: `0b661c3` (`Consolidate the UI module architecture`)
Date: 2026-09-06

## Purpose

Phases 1–7 of `MODULES_V2_PLAN.md` made the module build reproducible,
portable, and substantially easier to understand. The remaining design still
misses the cold-build and dependency-depth gates because the consolidated UI
is implemented as a serial chain of large interface units.

This plan is a focused finalization pass. It preserves the architectural gains,
keeps `carcer` as the eventual application entry module, and makes layers the
top UI/controller boundary. It does not remove the fallback build machinery;
that remains Phase 8 work and is unlocked only after the final gates pass.

## Corrected baseline

Qualification host: Darwin x86_64, 20 logical CPUs; eight build jobs; CMake
4.4.3; Ninja 1.13.2; GCC 15.3.0; Homebrew Clang 22.1.8; Emscripten 6.0.9.

| Metric | Current result | Final target |
|---|---:|---:|
| Carcer interfaces | 26 | 20 or fewer |
| Import edges | 117 | record; must decrease |
| Critical dependency depth | 14 | 12 or fewer |
| Fresh GCC debug `CARCER` build | 77.05 s | 60 s or less |
| GCC no-op build | to be remeasured | 1 s or less |
| Leaf implementation rebuild | to be remeasured | 4 s or less |
| Objects + archives + BMIs | approximately 260 MiB | 300 MiB or less |
| Entire fresh GCC build directory | approximately 438 MiB | report separately |

The old 1.1 GiB size reading is not a valid clean baseline. The reused build
directory contained outputs for interfaces deleted during earlier phases;
Ninja could not remove outputs that were no longer present in its graph. The
fresh directory contains about 172 MiB of retained CMake/GCC dependency-scan
preprocessing data in addition to the acceptance metric above.

The current longest application chain is effectively:

```text
carcer.data
  -> carcer.model
  -> carcer.state
  -> carcer.ui.core
  -> carcer.ui.widgets.primitives
  -> carcer.ui.widgets.controls
  -> carcer.ui.widgets.views
  -> carcer.ui.widgets.composites
  -> carcer.ui.screens.runtime
  -> carcer.ui.screens.layouts
  -> carcer.ui.screens.pages/overlays
  -> carcer.ui.screens.layers
  -> carcer.ui.screens
  -> carcer
```

## Target architecture

Layers own lifecycle, input routing, interactive orchestration, screen
transitions, and update/render-loop participation. Screens are passive view
compositions. The application module owns process-level construction and
starts the layer controller.

```text
data / database / model / rules
              |
              v
         state / actions
              |              sdl2w / bmin
              |                    |
              +----------+---------+
                         v
                 carcer.ui.core
                         |
                         v
                carcer.ui.widgets
                         |
                         v
                carcer.ui.screens
                         |
                         v
                 carcer.ui.layers
                         |
                         v
                       carcer
```

The arrows show build dependencies, not ownership of runtime data. Layers may
depend on state, actions, rules, screens, and SDL2W because layers are the UI
orchestration boundary. Screens must never import layers. Lower-level state,
rules, and actions must never import UI modules.

### Permanent public boundaries

- `carcer.ui.core`: stable UI primitives such as scaling, geometry, styling,
  and the base element contract.
- `carcer.ui.widgets`: declarations for reusable controls and views.
- `carcer.ui.screens`: passive screen/view composition contracts and factories.
- `carcer.ui.layers`: `Layer`, `LayerManager`, layer factories, lifecycle,
  interaction routing, and loop coordination.
- `carcer`: the narrow application/bootstrap API consumed by `main.cpp`.

### Source-shape rules

- Public `.cppm` files contain declarations and genuinely small stable inline
  functions only.
- Non-template method bodies live in ordinary module implementation `.cpp`
  files.
- Several grouped implementation files may belong to the same named module;
  implementation grouping does not create another public module.
- Private headers are allowed for declarations shared only among implementation
  units. They must not become a second public API.
- `export import` is used only when the imported API is deliberately part of
  the public contract. Implementation convenience is not a reason to re-export.
- Screens expose callbacks or neutral interaction intents. Layers translate
  those interactions into `carcer.actions` commands.
- Navigation requested by actions continues to use neutral state values such as
  `LayerRequest`; actions do not acquire a dependency on the layer module.
- `carcer` is retained, but it stops re-exporting every subsystem. It exports a
  small application entry API and privately imports what its implementation
  needs.

## Phase 0: checkpoint and trustworthy measurements

1. Commit the completed Phase 7 compatibility work separately from this UI
   redesign.
2. Add a fresh-build benchmark command that always configures a new ignored
   directory instead of relying on `ninja clean`.
3. Record wall time, the current critical chain, the literal acceptance
   artifact categories, total build-directory size, no-op time, and a leaf
   implementation rebuild.
4. Add or preserve narrow import probes for every public UI boundary.
5. Add an architecture check that rejects imports from screens to layers and
   imports from state/actions/rules to any UI module.

Exit criteria:

- The baseline is reproducible without stale artifacts.
- Phase 7 changes and the new plan are reviewable independently from source
  migration.
- All existing native tests remain green.

Completion record:

- Phase 7 compatibility work is checkpointed in `8c61ef5`; this plan is
  checkpointed separately in `795b421`.
- `scripts/benchmark-fresh-modules.sh` creates a unique ignored directory and
  records cold, no-op, leaf-edit, categorized-artifact, scan-intermediate, and
  total-directory measurements.
- Its first full GCC debug run records an 81-second cold build, a sub-second
  no-op build, a 3-second `Json.cpp` implementation rebuild, 264,924 KiB of
  objects/archives/BMIs, 186,164 KiB of retained scan preprocessing data, and
  457,384 KiB total. The cold result is consistent with the manually timed
  77.05-second baseline and shows normal host variance.
- `scripts/modules/check_ui_architecture.sh` passes all permanent lower-layer
  checks. Its temporary Phase 0 mode permits only the known
  `carcer.ui.screens -> carcer.ui.screens.layers` facade edge; strict mode
  already fails on that edge and will become mandatory in Phase 1.

## Phase 1: put layers above screens

1. Introduce the public module name `carcer.ui.layers`.
2. Move `Layer`, `LayerManager`, concrete layer factories, and layer-stack API
   ownership to that boundary.
3. Stop re-exporting layers from `carcer.ui.screens`.
4. Make layers import the screen modules they compose.
5. Update production imports, tests, probes, CMake sources, and the legacy Make
   graph without changing behavior.
6. Keep the existing large implementation temporarily; this phase corrects
   direction and naming before method extraction.

Exit criteria:

- No screen module imports or re-exports `carcer.ui.layers`.
- `carcer` depends on layers as its top UI boundary.
- Critical depth falls by at least one level.
- GCC and Clang debug builds, runtime tests, UI compile tests, import probes,
  and legacy Make all pass.

Completion record:

- `carcer.ui.layers` is now a standalone declarations-only public boundary.
  `Layer`, `LayerManager`, and the public layer factories are owned there.
- Concrete layer bodies, the base `Layer` body, and `LayerManager` are ordinary
  implementation units of `carcer.ui.layers`.
- `carcer.ui.screens` no longer imports or re-exports layers. `carcer` imports
  screens and layers as siblings, with layers as the final UI/controller
  dependency.
- The strict architecture check passes with no exception. A dedicated external
  `ImportUiLayers` probe passes under GCC and Clang.
- The graph remains at 26 interfaces but falls from 117 to 102 edges and from
  depth 14 to 13. The maximum transitive dependent count remains 21.
- All 37 enabled native runtime/probe tests pass under GCC and Clang; five stale
  tests remain disabled. All 43 UI programs compile and link under both debug
  compilers, and the corrected legacy Make source/archive list builds.

## Phase 2: declaration-only widgets pilot

1. Replace the re-export facade with one real declaration interface:
   `carcer.ui.widgets`.
2. Move non-template bodies from primitives, controls, views, and composites
   into grouped `.cpp` implementation units of `carcer.ui.widgets`.
3. Remove the four dotted widget interfaces after their final import is gone.
4. Replace accidental re-exports with the smallest private imports required by
   the interface or implementation.
5. Keep the external `ImportUiWidgets` probe and behavioral UI compile suite.

Pilot gate:

- A fresh GCC debug `CARCER` build must improve by at least 8 seconds from the
  77.05-second baseline, or finish in 69 seconds or less.
- The widget portion of the critical chain must collapse to one public
  interface level.
- An implementation-only widget edit must not rebuild a downstream BMI.
- GCC and Clang must both import the resulting interface externally without
  corrupt BMI diagnostics.

If the pilot misses the timing gate or repeats the GCC large-facade failure,
stop before migrating screens. First try two or three sibling declaration-only
widget interfaces with no serial dependencies between them. If that also
fails, choose the header fallback rather than rebuilding a micro-module tree.

## Phase 3: passive declaration-only screens

1. Make `carcer.ui.screens` a real declaration interface rather than a
   re-export facade.
2. Move runtime, layout, overlay, and page bodies into grouped ordinary
   implementation units of that module.
3. Keep concrete screen types private unless a non-UI consumer demonstrates a
   stable public need.
4. Expose factories, view models, callbacks, or neutral interaction intents
   instead of controller ownership.
5. Remove the `runtime -> layouts -> pages/overlays` interface chain and delete
   the superseded dotted screen interfaces.

Exit criteria:

- Screens import widgets but never layers.
- Screen code does not own the layer stack, application loop, or action
  dispatch policy.
- Critical dependency depth is 10 or less before the application boundary.
- Fresh GCC debug `CARCER` build is 60 seconds or less, measured three times
  with the median reported.
- GCC/Clang import probes and all UI compile tests pass.

## Phase 4: finalize layers and the application root

1. Make `carcer.ui.layers` declaration-only and move concrete layer and manager
   bodies into grouped implementation units.
2. Put input routing, screen transitions, layer-stack mutation, and update/
   render-loop coordination in layers.
3. Convert `carcer` from a broad umbrella into a narrow application module,
   for example an exported `runCarcer(argc, argv)` entry point.
4. Move process/application bootstrapping out of `main.cpp` into a `carcer`
   implementation unit. `main.cpp` should import only `carcer` for project
   behavior.
5. Privately import layers, state, database, and SDL2W from the application
   implementation rather than re-exporting their APIs.

Exit criteria:

- The permanent ending is `screens -> layers -> carcer`.
- `main.cpp` is a thin entry point and `carcer` is ready to own the full game
  startup as the remaining refactor is completed.
- No production consumer uses `carcer` as a convenience umbrella.
- Critical dependency depth, including `carcer`, is 12 or less.
- Application behavior and all tests remain unchanged.

## Phase 5: import hygiene and enforcement

1. Audit every remaining `export import` and document the intentional public
   re-exports.
2. Replace implementation-only public imports with private imports, callbacks,
   value types, or private implementation declarations as appropriate.
3. Add automated checks for the permanent dependency direction.
4. Regenerate and verify the legacy Make graph while it remains supported.
5. Remove migration adapters introduced by Phases 1–4.

Exit criteria:

- The checked graph has 20 or fewer Carcer interfaces, critical depth 12 or
  less, and fewer edges than the Phase 7 graph.
- No forbidden upward import exists.
- Touching a stable implementation rebuilds only its object and necessary
  links, not public BMIs.

## Phase 6: final qualification and Phase 8 decision

Run from uncontaminated or correctly keyed build roots:

- Native GCC debug and release builds and runtime tests.
- Native Clang debug and release builds and runtime tests.
- All 43 UI compile/link tests under all native configurations.
- Ten repeated clean parallel debug builds for each native compiler.
- Emscripten debug and release builds from a clean target cache.
- SDL2W/BMIN named-module builds and GCC/Clang classic-header probes.
- Legacy Make build.
- Fresh-build, no-op, leaf-implementation, graph, and artifact measurements.

Final gates:

| Requirement | Target |
|---|---:|
| Carcer interfaces | 20 or fewer |
| Critical dependency depth | 12 or less |
| Fresh GCC debug `CARCER` median | 60 seconds or less |
| Leaf implementation rebuild | 4 seconds or less |
| No-op build | 1 second or less |
| Objects + archives + BMIs | 300 MiB or less |
| Repeated native clean builds | 10/10 per compiler |
| GCC, Clang, Emscripten | all supported |
| Header and module dependency modes | both supported |
| Forbidden architecture imports | zero |

Passing every gate unlocks Phase 8 of `MODULES_V2_PLAN.md`. Failure of the
widgets pilot or the final cold-build gate triggers an explicit choice between
one narrowly measured follow-up and the existing header-architecture fallback;
it does not justify recreating per-class module partitions.

## Commit discipline

- One checkpoint commit for Phase 7 and its qualification tooling.
- One buildable commit per phase of this plan.
- Do not mix behavioral changes with mechanical module extraction.
- Record graph and timing deltas in this file before committing each phase.
- Preserve the last green commit before each facade/interface conversion.
- Do not remove the legacy/fallback machinery until the final qualification
  commit has passed and Phase 8 is explicitly authorized.
