# Carcer C++ Modules v2 Implementation Plan

Status: Phase 4 complete; Gate 2 passes natively
Baseline commit: `95562b3` on `experiment/cpp-modules`  
Date: 2026-09-05

## Implementation status

- Phase 1 completed in `e0c0b83`: dependencies are pinned by `deps.lock`,
  bootstrapped explicitly into `.deps/`, and validated without build-time
  repository mutation.
- Phase 2 adds a parallel CMake/Ninja build using `CXX_MODULES` file sets and
  compiler dependency scanning for BMIN, SDL2W, and the unchanged Carcer module
  tree. Make remains available.
- Phase 3 replaces the model/template partition forest with `carcer.data` and
  `carcer.model`, each using a declarations-only interface and grouped
  implementation units. The generated Make graph remains buildable.
- Phase 4 removes the concrete
  `UiRemoveFloatingNotification` action from `carcer.state`. Notification
  expiry is now state maintenance, explicit dismissal remains owned by
  `carcer.actions.ui`, and UI synchronization uses a state revision rather
  than requiring the state module to construct an action-layer type.
- `ActiveMapOrchestrator` no longer inherits `StateManagerInterface` or
  `DatabaseInterface`. Its active map, map-instance store, and database are
  explicit constructor inputs; map visibility, pathfinding, pickup, enemy-AI,
  and persistence APIs now carry the same narrow dependencies through their
  call chains instead of recovering them from process-global service locators.
- `carcer.game.map` no longer imports `carcer.state`. Map persistence operates
  on explicit map-instance data, movement aging is driven by its callers, and
  tile-trigger rules return values that the action layer applies to state. The
  generated graph falls from 847 to 846 edges and from 30 to 29 critical
  levels.
- `carcer.game.combat` no longer imports `carcer.state`. New-round sequencing,
  movement counting, and map aging now live in `GoNextCombatTurn`, while combat
  exposes only rules over explicit model, map, and database inputs. Removing
  the inversion and the action's now-redundant combat import lowers the graph
  from 846 to 844 edges.
- The `carcer.actions.world_effects` and `carcer.world_updater` cycle-break
  modules have been folded into `carcer.actions.world`. Generic world effects
  remain available to combat implementation units, the updater remains action
  orchestration, and the world/combat primary interfaces are independent. The
  graph falls from 183 to 181 interfaces, 844 to 838 edges, 29 to 26 critical
  levels, and 174 to 172 maximum transitive dependents.
- `carcer.model` no longer imports `carcer.db` or exposes database-dependent
  operations. Inventory and equipment lookups now live in
  `carcer.game.inventory`, map-character construction lives in
  `carcer.game.map`, and combat-party population lives in
  `carcer.game.combat`.
- GCC 15.3 and Homebrew Clang 22.1.8 debug builds pass on the qualification
  host. Thirty-two current non-UI tests pass under both compilers, five stale
  tests compile but are disabled, three tests call already-disabled production
  APIs, and all 43 UI test programs compile and link with GCC.
- Emscripten presets are present, but the SDK is not installed on the
  qualification host; that platform remains unverified and must be closed
  during the Phase 3 pilot rather than deferred to final cleanup.

### Phase 2 qualification results

Qualification host: Darwin x86_64, 20 logical CPUs; eight build jobs; CMake
4.4.3; Ninja 1.13.2; GCC 15.3.0; Homebrew Clang 22.1.8; SDL2W
`e5415231257b4a25f25ad3af5cb4e01c125ada15`; BMIN
`e60f65b1d3a36def8221bd93c5702d323c714cea`.

| Check | Result |
|---|---:|
| Repeated clean GCC builds | 10/10 passed |
| Clean GCC build median | 143.5 s |
| Clean GCC build range | 139-147 s |
| GCC no-op build | 0.38 s |
| `Json.cpp` implementation touch | 4.03 s; one object plus relink |
| `Items.cppm` interface touch | 88.06 s; 236 build actions |
| Make `Items.cppm` interface touch | 277.02 s; build passed |
| GCC runtime tests | 30 passed, 5 explicitly disabled |
| Clang runtime tests | 30 passed, 5 explicitly disabled |
| GCC UI compile/link tests | 43/43 passed |
| GCC debug build directory after `CARCER` | approximately 1.1 GiB |

Gate 1 passes for the native compiler-scanned build: automatic ordering is
repeatable, independent BMIs compile in parallel, implementation-only edits do
not regenerate downstream BMIs, and both native compiler families build and
test. The measurements also confirm that build-system repair alone cannot meet
the final performance or artifact-size targets. The deep, high-fan-out module
graph remains the next bottleneck, so the coarse `data`/`model` pilot in Phase 3
is warranted.

### Phase 3 qualification results

The pilot replaces the 26-file `carcer.model.templates` and
`carcer.model.instances` partition forests with two declarations-only public
interfaces and six grouped implementation units. The corresponding header-era
subsystem used 38 files. Imports now name `carcer.data` and `carcer.model`, and
the latter's data, database, tile-field, and SDL dependencies are private rather
than accidental re-exports.

| Check | Result |
|---|---:|
| Repeated clean GCC builds | 10/10 passed |
| Clean GCC build median | 128 s |
| Clean GCC build range | 126-130 s |
| GCC no-op build | 0.33 s |
| `templates.cpp` implementation touch | 4.37 s; one object plus relink; no BMI rebuild |
| `characters.cpp` implementation touch | 3.70 s; one object plus relink; no BMI rebuild |
| `data.cppm` interface touch | 80.73 s; 234 build actions |
| `model.cppm` interface touch | 75.84 s; 214 build actions |
| Generated Make clean build | 283.49 s; passed |
| GCC runtime tests | 30 passed plus 2 import probes; 5 explicitly disabled |
| Clang runtime tests | 30 passed plus 2 import probes; 5 explicitly disabled |
| GCC UI compile/link tests | 43/43 passed |
| GCC debug build directory after `CARCER` | approximately 547 MiB |

The checked graph changed from 207 to 183 interface units, 921 to 847 edges,
a critical depth of 37 to 30, and a maximum transitive dependent count of 185
to 174. Production source files fell from 258 to 239. The pilot subsystem
itself fell from 26 module files to 8 grouped files.

Gate 2 passes as a native feasibility gate, not as final acceptance. Stable
implementation edits now have the required isolation, the cold build improved
by about 11%, graph depth fell about 19%, and the artifact directory is about
half its Phase 2 size. Public-interface edits remain expensive, and the
remaining partition forests still dominate the graph. Further migration must
keep declarations stable and apply the same coarse interface/grouped
implementation shape; returning to partition-per-class would erase the gain.

One platform follow-up remains after the ownership work in Phase 4:

- Emscripten is still unverified because no Emscripten SDK is installed on the
  qualification host.

## Summary

The first module experiment succeeded at removing most header/implementation
pairs and made domain dependencies more explicit, but it did not meet the
compile-time or bootstrap-simplicity goals. The v2 effort will preserve the
useful architectural work while replacing the current 207-interface-unit,
one-class-per-partition design with a small set of coarse, declarations-only
modules backed by grouped `.cpp` implementation units.

This is a gated redesign. Build-system feasibility and a representative
`data`/`model` conversion must meet measurable acceptance criteria before the
actions or UI trees are migrated. If the pilot fails, Carcer will return to a
header-based application architecture while retaining the dual header/module
support in SDL2W and BMIN.

## Goals

- Reduce production C++ files without turning implementation edits into public
  interface edits.
- Improve cold and incremental compile times relative to the first module
  experiment.
- Replace repository-specific module dependency generation with
  compiler-supported dependency scanning.
- Make module boundaries follow domain ownership rather than compiler
  partition limits.
- Make dependency acquisition explicit, pinned, local to the repository, and
  reproducible.
- Support native GCC, native Clang, and Emscripten before the redesign is
  considered complete.

## Measured baseline

Tests and copied SDL2W files are excluded from source-file counts.

| Metric | Header branch (`fe50a90`) | Current modules (`95562b3`) |
|---|---:|---:|
| Production C++ lines | 38,338 | 38,492 |
| Production C++ files | 384 | 258 |
| Matching interface/implementation pairs | 135 | 8 |
| Compiled project units | 136 | 257 |
| Cold build, eight jobs | 34.22 s | 318.28 s |
| Cold module build with experimental BMI parallelism | n/a | 102.11 s |
| Leaf edit rebuild | 3.03 s | 6.77 s |
| No-op build | not recorded | 0.58 s |
| Objects, archives, and BMI cache | about 217 MiB | about 637 MiB |

The current module graph has 207 interface units, 921 module edges, and a
critical path 37 units deep. Foundational partitions have as many as 185
transitive dependents. The current Makefile also forces the Carcer BMI phase to
run with `-j1`.

## Target architecture

Target approximately 12-20 public modules and 40-70 production C++ files:

```text
carcer.util
carcer.data
carcer.database
carcer.model
carcer.map
carcer.combat
carcer.inventory
carcer.events
carcer.state
carcer.actions
carcer.ui.core
carcer.ui.widgets
carcer.ui.screens
carcer.app
```

The exact number may vary within the acceptance limit, but new modules must
represent stable subsystem boundaries rather than individual classes.

The normal source shape is:

```text
world.cppm          # exported declarations and small stable inline code
world.cpp           # primary grouped implementation
world_pathing.cpp   # optional additional grouped implementation
detail/*.h          # private implementation sharing only when necessary
```

Architecture rules:

- Do not create one exported partition per class.
- Use partitions only for a demonstrated semantic or build boundary.
- Keep non-template implementation bodies in `.cpp` implementation units.
- Use `export import` only when re-exporting another module is part of the
  intended public API.
- Production code and tests import the narrow module they use.
- Do not provide a production umbrella module.
- Keep state primitives independent from concrete UI actions.
- Keep map, combat, and inventory rules independent from `StateManager` and
  service-locator inheritance.
- Pass model and database dependencies explicitly to rule functions.

The intended dependency direction is:

```text
BMIN / SDL2W
      |
      v
carcer.util     carcer.data
                     |
          +----------+----------+
          v                     v
   carcer.database         carcer.model
          |                     |
          +----------+----------+
                     v
       map / combat / inventory / events
                     |
                     v
               carcer.state
                     |
                     v
              carcer.actions
                     |
                     v
          UI core / widgets / screens
                     |
                     v
                 carcer.app
```

## Implementation phases

### Phase 0: Preserve the experiment and automate benchmarks

Create `experiment/cpp-modules-v2` from `95562b3`. Do not rewrite or delete the
existing experiment branch.

Add a repeatable benchmark command that records:

- Clean native build.
- No-op build.
- High-level implementation edit.
- Foundational implementation edit.
- Public-interface edit.
- Full test compilation and execution.
- Build artifact size.
- Module node, edge, critical-depth, and fan-out metrics.

Store machine-readable results under an ignored build directory and document
the host compiler, compiler version, CPU count, configuration, and dependency
commits with every run. Use the median of at least three runs for comparisons.

### Phase 1: Normalize dependency bootstrapping

Replace implicit sibling-repository management with an explicit project-local
layout:

```text
.deps/
  sdl2w/
  bmin/
deps.lock
scripts/bootstrap-deps.sh
```

Pin the initial dependency revisions:

```text
sdl2w e5415231257b4a25f25ad3af5cb4e01c125ada15
bmin  e60f65b1d3a36def8221bd93c5702d323c714cea
```

Requirements:

- Building never silently clones, fetches, switches, or updates a repository.
- The bootstrap command is explicit and idempotent.
- Existing dependency checkouts are validated against `deps.lock`.
- `clean` never changes files outside the Carcer repository.
- SDL2W and BMIN BMIs use the same compiler, target, standard library, and
  relevant flags as Carcer.
- Header-mode dependency builds remain available as a fallback.
- BMIs and dependency build artifacts remain untracked.

### Phase 2: Introduce a compiler-scanned build in parallel

Add CMake with Ninja alongside the Make build. Keep Make working until the v2
build passes every acceptance gate.

Use CMake `CXX_MODULES` file sets and compiler dependency scanning. Build BMIN,
SDL2W, and Carcer source targets in one configured graph rather than importing
BMIs produced by an unknown compiler configuration.

Provide presets for:

- Native GCC debug and release.
- Native Clang debug and release.
- Emscripten, introduced during the pilot rather than deferred to final cleanup.

First express the unchanged module tree in the new build and verify:

- Independent BMIs run in parallel.
- At least ten repeated clean builds complete without cache corruption or an
  internal compiler error.
- Representative leaf and foundational edits rebuild the correct targets.
- Tests produce the same behavioral result as the Make build.
- The checked-in generated dependency manifests are no longer needed by the
  new build.

#### Gate 1

Do not begin the architectural migration until compiler-driven parallel builds
are repeatably correct. A single successful parallel build is insufficient.

### Phase 3: Pilot coarse `data` and `model` modules

Replace the current model/template partition forest with declarations-only
interfaces and grouped implementations:

```text
src/data/data.cppm
src/data/templates.cpp
src/data/stats.cpp

src/model/model.cppm
src/model/characters.cpp
src/model/maps.cpp
src/model/world.cpp
src/model/combat.cpp
```

During the pilot:

- Move database-dependent equipment, inventory, spell, and construction logic
  out of runtime model types and into the appropriate rules or service layer.
- Keep enums, value types, exported declarations, and required template bodies
  in interfaces.
- Move ordinary function and method bodies to implementation units.
- Replace cross-module re-exports with private imports unless downstream
  visibility is explicitly part of the API.
- Delete old partitions only after their replacement builds and tests pass.
- Add a minimal external-import probe for each new module.
- Measure cold, leaf-implementation, and public-interface rebuilds after every
  consolidation step.

#### Gate 2

Continue only if all of the following hold:

- An implementation-only edit recompiles its implementation unit and relinks
  without regenerating downstream BMIs.
- Foundational graph depth and fan-out decrease materially.
- Ten repeated parallel clean builds complete without compiler corruption.
- The pilot uses fewer files than both the current module layout and the
  corresponding header-era subsystem.
- Projected full-build performance can meet the final acceptance criteria.

If Gate 2 fails, stop the v2 migration and restore Carcer to headers while
retaining SDL2W/BMIN dual-mode support.

### Phase 4: Separate rules, state, and orchestration

Establish these ownership rules:

- `carcer.state` owns state data, `StateManager`, the action queue interface,
  triggers, and UI request data. It owns no concrete UI actions.
- `carcer.map`, `carcer.combat`, and `carcer.inventory` contain rules operating
  on explicit model and database inputs. They do not import `carcer.state`.
- `carcer.actions` owns state mutation, timing, sequencing, and calls into rule
  modules.

Refactor existing inversions by:

- Moving `UiRemoveFloatingNotification` out of the state kernel.
- Removing `StateManagerInterface` and `DatabaseInterface` inheritance from
  `ActiveMapOrchestrator`.
- Separating map persistence and trigger mutation from pure map calculations.
- Moving state-mutating combat-round behavior into orchestration.
- Removing cycle-break modules once the corrected dependency direction makes
  them unnecessary.

### Phase 5: Consolidate the action API

Replace exported concrete action classes with a small command API. Timed
internal actions may continue deriving from `AbstractAction`, but their types
remain private.

The public surface should resemble:

```cpp
export module carcer.actions;

export namespace state::actions {
void movePlayer(StateManager&, int dx, int dy);
void travel(StateManager&, TravelRequest);
void performMeleeAttack(StateManager&, AttackRequest);
void showInventory(StateManager&, CharacterId);
}
```

Group implementations by behavior:

```text
actions.cppm
world_actions.cpp
combat_actions.cpp
inventory_actions.cpp
ui_actions.cpp
scheduled_effects.cpp
```

Preserve queue ordering, delay behavior, and observable state transitions with
behavioral tests before deleting each concrete exported action.

### Phase 6: Redesign UI exposure

Consolidate UI into three public boundaries:

```text
carcer.ui.core
carcer.ui.widgets
carcer.ui.screens
```

Requirements:

- Export only types consumed outside their owning UI subsystem.
- Keep concrete screens and layers private where possible.
- Expose screen construction or registration instead of every `LayerX` type.
- Use grouped implementation units rather than per-class partitions.
- Permit a small number of private headers when implementations share internal
  declarations.
- Convert white-box UI tests to public behavioral tests where practical.
- Replace every test import of the current `carcer` umbrella with the narrowest
  relevant module.
- Run external-import probes during each UI consolidation to detect GCC issues
  before connecting the module to the full graph.

### Phase 7: Complete platform and compatibility coverage

Before removing the old build, verify:

- Native GCC debug and release builds.
- Native Clang debug and release builds.
- Emscripten build.
- SDL2W/BMIN module mode.
- SDL2W/BMIN header fallback mode.
- Full runtime test suite.
- Full UI compile suite.
- Ten repeated parallel clean builds for each native compiler.
- Correct clean rebuild after switching compiler, target, or configuration.

Segregate BMI caches by compiler identity, compiler version, target platform,
build configuration, relevant flags, and dependency revision.

### Phase 8: Remove experimental machinery

After all gates pass:

- Remove the generated BMI Make graph and manifests.
- Remove migration-only scripts.
- Delete superseded module partitions.
- Delete the umbrella module.
- Remove sibling-checkout assumptions.
- Replace the current module document with a concise architecture and build
  contract.
- Retain the benchmark and architecture-validation commands in CI.
- Keep the old Make build until the CMake/Ninja build passes on every supported
  platform, then remove it in a dedicated commit.

## Public API changes

- Concrete action classes stop being public; callers use action functions or
  compact request values.
- Runtime model APIs stop accepting or locating the concrete database where the
  behavior belongs in a rules module.
- Map and combat rules stop accepting whole `state::State` objects unless the
  operation is explicitly orchestration.
- UI exposes subsystem entry points and reusable widgets rather than every
  concrete layer or screen class.
- The `carcer` umbrella import is removed; callers import named subsystems.

Compatibility adapters may exist during migration, but each adapter must be
removed in the same phase that migrates its final caller.

## Test plan

For every phase:

- Run the existing runtime and UI compilation suites.
- Add focused tests before moving behavior across module boundaries.
- Compare Make and CMake/Ninja outputs while both builds exist.
- Run external-import probes for newly consolidated modules.
- Verify implementation-only edits do not rebuild downstream BMIs.
- Verify public-interface edits rebuild all and only actual dependents.
- Run `git diff --check` and ensure generated/build artifacts remain ignored.

Before final adoption:

- Resolve or explicitly retire the existing 11 stale/disabled tests.
- Run ten repeated clean parallel builds per native compiler.
- Run native behavioral tests with SDL2W's header and module APIs.
- Build the Emscripten target from a clean dependency state.
- Record final benchmark results alongside the baseline in this document.

## Final acceptance criteria

| Requirement | Target |
|---|---:|
| Public module interface units | 30 or fewer |
| Preferred public modules | 12-20 |
| Critical dependency depth | 12 or less |
| Cold native build | 60 seconds or less |
| Leaf implementation rebuild | 4 seconds or less |
| No-op build | 1 second or less |
| Objects, archives, and BMI cache | 300 MiB or less |
| Checked-in generated dependency manifests | 0 |
| Repeated parallel clean builds | 10/10 successful |
| Native GCC | Supported |
| Native Clang | Supported |
| Emscripten | Supported |
| Dependency revisions | Pinned and validated |
| `clean` modifies external repositories | Never |

Failure to satisfy the build-time, correctness, or compiler-stability criteria
after the `data`/`model` pilot ends the module migration. The fallback is the
header architecture for Carcer with SDL2W and BMIN retaining their tested dual
header/module distributions.

## Commit and rollout strategy

- Keep every phase buildable and reviewable.
- Land benchmark and dependency work before source restructuring.
- Migrate one domain at a time with compatibility adapters only when necessary.
- Record benchmark changes in each domain-consolidation commit.
- Do not combine build-system removal with the final domain migration.
- Do not rewrite the original experiment branch; preserve it for comparison and
  for recovering architectural changes.
