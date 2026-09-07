# Carcer Module/UI Finalization Plan

Status: Phase 6 complete; Phase 8 remains blocked by the cold-build gate
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

Pilot observation (2026-09-06):

- A single 2,330-line declaration interface repeatedly produced GCC `Bad file
  data` diagnostics when imported by the external widget probe.
- Three compiler-sized declaration interfaces behind the six-line public
  `carcer.ui.widgets` facade import successfully. Shared declarations live in
  `foundation`; `views` and `composites` are siblings rather than a serial
  chain.
- The resulting graph has 25 interfaces, 98 edges, critical depth 12, and a
  maximum transitive fan-out of 20.
- Fresh GCC debug measurements were 71 and 72 seconds before redundant
  implementation imports were removed, then 70 seconds afterward. The result
  improves on the 77.05-second baseline but misses the pilot gate by one
  second (or 0.95 second relative to the improvement criterion).
- The measured widget implementation edit rebuilt its object, the Carcer
  archive, and the executable but no BMI. Its five-second end-to-end result is
  dominated by archiving and relinking, not downstream module recompilation.
- The project owner explicitly accepted the approximately one-second pilot
  variance on 2026-09-06. GCC and Clang debug builds, all 43 UI compile/link
  programs, all seven import probes, all 37 enabled runtime tests, and legacy
  Make pass. Phase 3 may proceed without weakening the final 60-second gate.

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

Completion record:

- `carcer.ui.screens` is now a real declaration interface. Runtime helpers,
  layouts, overlays, pages, and minipages have ordinary `.cpp` implementation
  units, and the four dotted screen interfaces are gone.
- The screen dependency chain collapsed from four build-organisation levels to
  one. The graph fell from 25 interfaces, 98 edges, and depth 12 to 21
  interfaces, 70 edges, and depth 9; maximum transitive fan-out fell to 16.
- Three fresh GCC debug app builds measured 65, 66, and 66 seconds (median 66).
  No-op builds were sub-second. Screen implementation edits measured four to
  six seconds end-to-end and rebuilt no downstream BMI.
- The structural, compiler-stability, and rebuild-isolation criteria pass. The
  interim 60-second performance criterion does not yet pass and remains a hard
  final qualification target for Phases 4–6.
- GCC and Clang debug builds, all 43 UI compile/link programs, all seven import
  probes, all 37 enabled runtime tests, the strict architecture check, and
  legacy Make pass.

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

Completion record:

- `carcer` now exports only `runCarcer(int, char**)`; it no longer re-exports
  project subsystems. A dedicated implementation unit owns SDL2W setup, asset
  and font initialization, and shutdown.
- `main.cpp` imports only `carcer` and delegates directly to `runCarcer`.
- `LayerManager` now registers and unregisters itself deterministically, binds
  mouse and keyboard routing, advances the state/layers, renders the stack,
  and owns `Window::startRenderLoop`. The current splash-only application
  behavior remains unchanged while the controller is ready for layer startup.
- Private interface imports make the terminal dependency direction explicit:
  `carcer.ui.screens -> carcer.ui.layers -> carcer`. The graph has 21
  interfaces, 71 edges, critical depth 10, and maximum fan-out 16.
- A narrow `ImportCarcer` probe was added. GCC and Clang debug builds, all 43 UI
  programs, all eight import probes, all 38 enabled runtime/probe tests, the
  architecture check, and legacy Make pass.
- One fresh GCC debug measurement was 68 seconds with a three-second
  LayerManager implementation rebuild. The 60-second final target remains for
  Phase 6 rather than being relaxed.

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

Completion record:

- The unused standalone `carcer.lib.hiscore.hiscore` boundary was folded into
  `carcer.data`, preserving its namespace and API while reducing the graph to
  20 interfaces, 70 edges, critical depth 10, and maximum fan-out 16.
- Every remaining `export import` was reviewed. Re-exports now represent types
  deliberately exposed by domain declarations or the three compiler-sized
  build units behind `carcer.ui.widgets`; `carcer` itself re-exports nothing.
- The permanent architecture check now rejects upward UI edges, new consumers
  of the application root, direct consumers of internal widget units, changes
  to the widget facade set, and a root module that re-exports subsystems. It is
  registered with CTest.
- The regenerated legacy Make graph contains no migration adapter for the
  removed hiscore interface. A hiscore implementation edit rebuilds its object
  and required links without rebuilding a public BMI.

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

Completion record (2026-09-07):

| Requirement | Final result | Status |
|---|---:|---|
| Carcer interfaces | 20 | Pass |
| Import edges | 70 | Reduced from 117 |
| Critical dependency depth | 10 | Pass |
| Fresh GCC debug `CARCER` | 68, 69, 69 s; median 69 s | **Fail** |
| Leaf implementation rebuild | 3, 3, 3 s; no BMI rebuilt | Pass |
| No-op build | 1, 0, 0 s | Pass |
| Objects + archives + BMIs | 277,056 KiB (about 270.6 MiB) | Pass |
| Scan preprocessing data | median 204,932 KiB | Reported separately |
| Entire fresh build directory | median 488,816 KiB | Reported separately |
| Repeated GCC debug clean builds | 10/10 | Pass |
| Repeated Clang debug clean builds | 10/10 | Pass |
| Forbidden architecture imports | zero | Pass |

- GCC 15.3 and Homebrew Clang 22.1.8 debug/release configurations build the
  application, all enabled runtime/import/architecture tests, and all 43 UI
  compile/link programs. Each CTest configuration reports 39 enabled tests
  passed and five documented stale tests disabled.
- Emscripten 6.0.9 debug and release builds were cleaned and rebuilt from the
  pinned BMIN/SDL2W module sources. Both produce `CARCER.js`, `CARCER.wasm`,
  and `CARCER.data`.
- The independent classic-header dependency probes pass with GCC 15.3 and
  Homebrew Clang 22.1.8. The clean legacy GCC Make build also passes after
  rebuilding the pinned named-module bundle and generated Carcer BMI graph.
- The 69-second final median is an eight-second improvement over the 77.05-
  second corrected baseline, but it misses the non-negotiable 60-second target
  by nine seconds. Phase 8 is therefore not authorized. The next decision is
  between one narrowly measured build-time follow-up and the documented
  header-architecture fallback; the fallback/migration machinery stays intact.

Narrow build-time follow-up (2026-09-07):

- Ninja timing data showed approximately 82 aggregate seconds in dependency
  scanning, 267 aggregate seconds in object compilation, and only about 0.5
  seconds creating the Carcer archive. There was no archive/link shortcut
  capable of closing the nine-second wall-time gap.
- Disabling GCC assignment-level variable tracking while retaining `-Og`, `-g`,
  and frame pointers produced fresh builds of 67, 68, and 69 seconds (median
  68). No-op builds remained at zero or one second and leaf builds at three
  seconds.
- The one-second median improvement is within host variance and trades away
  optimized-local variable-location precision. The option was therefore
  reverted rather than weakening the debug configuration for a result that
  still misses the target by eight seconds.
- This exhausts the single narrowly measured follow-up authorized after Phase
  6. Phase 8 remains blocked. The remaining decision is to adopt the documented
  header architecture or explicitly revise the acceptance target in a new
  plan; another unbounded module micro-optimization pass is not justified.

## Commit discipline

- One checkpoint commit for Phase 7 and its qualification tooling.
- One buildable commit per phase of this plan.
- Do not mix behavioral changes with mechanical module extraction.
- Record graph and timing deltas in this file before committing each phase.
- Preserve the last green commit before each facade/interface conversion.
- Do not remove the legacy/fallback machinery until the final qualification
  commit has passed and Phase 8 is explicitly authorized.
