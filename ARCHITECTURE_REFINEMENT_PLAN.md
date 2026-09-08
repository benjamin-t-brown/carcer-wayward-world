# Architecture Refinement Plan

Status: Proposed
Starting branch: `refactor/header-architecture`
Starting revision: record `git rev-parse HEAD` when Phase 0 begins
Date: 2026-09-08

## Objective

Refine the restored header architecture without repeating the C++ modules
experiment or introducing another large framework. The work should:

- give `LayerManager` one authoritative layer stack and a defined event-routing
  policy;
- replace mechanical UI observer classes with reusable action/callback bindings;
- separate world simulation from platform output where the existing SDL2W API
  permits it;
- make ownership transfer explicit;
- replace process-global state/database service locators with explicit runtime
  dependencies;
- remove unused or misleading abstractions;
- preserve the current simple CMake graph, source-file conventions, developer
  commands, and supported platforms.

Every phase is independently testable and should be committed before the next
phase starts.

## Decisions fixed by this plan

An implementing agent must follow these decisions unless the user explicitly
changes them.

1. Carcer remains a conventional C++23 header/source project. Do not add Carcer
   modules, module scanning, BMI generation, or a second build system.
2. Keep one explicit `carcer_objects` production build graph. Do not create a
   CMake library for every architectural level merely to enforce boundaries.
3. Preserve the current file convention: `.h` has a corresponding `.cpp`;
   `.hpp` is header-only. Actions remain one action per `.hpp`.
4. `Layer` remains a top-level loop/interactivity concept outside `ui`. A layer
   may contain UI, but it is not itself a UI component.
5. `LayerManager` owns the authoritative ordered runtime layer collection.
   State/actions may issue layer commands, but must not own a second desired
   layer stack that requires reconciliation.
6. A handled layer input event stops propagating unless the handling layer
   explicitly reports that the event was not consumed.
7. Preserve each damage particle's independent animation playback. Do not cache
   damage animation playback by asset name and do not synchronize particles
   that happen to use the same animation.
8. For now, a particle may continue to contain its own `sdl2w::Animation`.
   SDL2W currently combines playback state with store-backed sprite resources;
   moving that object out of the particle would require an ID-indexed shadow
   state in the view and would not improve this design.
9. The current particle lifetime must advance without a window. Animation
   initialization may remain conditional on a live SDL2W store until SDL2W
   separates animation playback from immutable render resources.
10. Sound identifiers and animation identifiers are valid state data. Calling
    `Window::playSound`, loading sprites, and drawing are platform/view work.
11. Prefer concrete references and small context objects over interface trees.
    Do not introduce `IRenderer`, `IAudio`, ECS, or a general dependency-
    injection container as part of this work.
12. Preserve behavior except where this plan explicitly defines a correction:
    one authoritative layer stack, consumed event propagation, headless
    particle lifetime progression, and output queues serviced independently of
    whether `LayerWorld` is active.

## Architectural destination

Dependencies continue to point downward:

```text
model/data
    ^
game rules
    ^
state and action execution
    ^
UI components
    ^
layers and loop orchestration
    ^
application bootstrap / SDL platform output
```

The runtime flow should become:

```text
input -> active layer -> action queue -> StateManager update
                                      -> state changes
                                      -> pending layer/audio requests

LayerManager -> consumes layer commands -> updates active layer(s)
             -> flushes platform output -> renders visible layer(s)
```

There are two intentionally different kinds of context:

- Action execution receives state, action scheduling, and optional database
  access. It never receives `sdl2w::Window`.
- UI/layers receive the concrete window, state manager, and database references
  they require. Non-visual game rules receive none of these.

## Execution protocol

At the start of every phase:

1. Read this entire plan.
2. Run `git branch --show-current` and `git status --short`. Work only on
   `refactor/header-architecture` unless the user directs otherwise.
3. If the tree contains unrelated user edits, preserve them and avoid touching
   overlapping files.
4. Search all production and test call sites before changing a public API.
5. Change only the current phase.
6. Update `cmake/carcer_sources.cmake` for every paired `.cpp` added, moved, or
   removed.
7. Run the phase gate, `git diff --check`, and inspect `git status --short`.
8. Commit only when the phase gate passes. Record important behavior decisions
   in the commit body when needed.

Do not use destructive Git commands, regenerate source inventories with an
untracked script, hide a failure by disabling a test, or modify the pinned
dependency checkouts during these phases.

## Global verification contract

Unless a phase states otherwise, its gate is:

```sh
make -C src test
make -C src ui
make -C src
git diff --check
```

`make -C src test` must run all registered non-interactive tests, `make -C src
ui` must compile all interactive UI runners without opening windows, and
`make -C src` must build the game. Run focused test runners while iterating,
but do not substitute them for the complete phase gate.

MSYS2/UCRT64 and Emscripten qualification may be deferred until Phase 8 when
those toolchains are unavailable on the implementation host. Native tests must
not be deferred.

## Phase 0: Baseline and characterization tests

Record the starting revision, host, compiler, clean build duration, incremental
no-op build duration, test count, UI target count, and executable size in a new
`ARCHITECTURE_REFINEMENT_RESULTS.md`. This results document is updated after
each later phase.

Add or strengthen characterization tests for behavior that later phases will
change internally:

- pushing the first world layer;
- opening an overlay, suspending the previous layer, closing the overlay, and
  restoring the previous layer;
- bringing an existing layer to the front without creating a duplicate;
- removing a non-front layer;
- rendering suspended layers while only updating the active layer;
- current mouse, wheel, and keyboard routing order;
- independently timed damage particles using the same animation name;
- damage-particle lifetime and removal when `worldUpdate` receives no window;
- sound request de-duplication and draining;
- special-event and travel trigger processing;
- representative observer-to-action behavior.

If the headless particle-lifetime test exposes the known current defect, commit
it disabled only by an explicit expected-failure mechanism already supported by
the test framework. If there is no such mechanism, document the failing command
in the results file and add the passing regression test in Phase 6 instead.

Acceptance criteria:

- no production behavior changes;
- the baseline is reproducible from commands in the results document;
- later layer, output, and observer behavior has focused coverage.

Commit: `Record architecture refinement baseline`

## Phase 1: Remove dormant and misleading abstractions

Before removing each candidate, confirm with `rg`, the CMake source manifest,
and the test manifests that it has no live use.

Candidates identified during planning:

- `src/game/combat/CombatRunner.h/.cpp`;
- `src/lib/hiscore/hiscore.h/.cpp`;
- `src/actions/Command.hpp`;
- `ui::StateInterface` and `UiElement::stateInterface`;
- the empty `CombatAction` subclass, unless a real marker use is found.

Rename `UiManager.h/.cpp` to a responsibility-revealing paired name such as
`UiStateUpdater.h/.cpp`, because it currently advances floating-notification
timers rather than managing UI elements. Use the final name consistently in
types, includes, tests, and the CMake manifest.

Do not remove a candidate merely because its implementation is incomplete. If
a live include, test, documented extension point, or runtime construction is
found, record that evidence in the results file and leave it in place.

Ignored legacy outputs such as `src/CARCER`, `src/TestUi`, `src/TestUi.dSYM`,
`src/gcm.cache`, and `src/lib/libcarcer.a` may be deleted locally only after
`git check-ignore` confirms each exact target is ignored. This is workspace
hygiene, not part of the commit.

Acceptance criteria:

- no production-looking source pair remains silently outside the build unless
  its intentional exclusion is documented;
- no unused state-interface or raw-pointer command wrapper remains;
- UI notification timing behavior is unchanged;
- the global verification contract passes.

Commit: `Remove dormant architecture scaffolding`

## Phase 2: Make action ownership explicit

Change action scheduling APIs to accept owning smart pointers rather than raw
pointers that are silently adopted.

Target shape:

```cpp
void enqueueAction(bmin::UniquePtr<AbstractAction> action, int ms = 0);
void insertAction(bmin::UniquePtr<AbstractAction> action, int ms = 0);
void parallelAction(bmin::UniquePtr<AbstractAction> action, int ms = 0);
```

The methods should operate on the manager's own `ActionData`; callers should no
longer pass `getActionData()` back into the same manager. Rename `pllAction` to
`parallelAction` unless a compatibility wrapper is temporarily required inside
this phase.

Provide a small template convenience where it reduces repetitive allocation:

```cpp
template <typename T, typename... Args>
void enqueue(int ms, Args&&... args);
```

Do not require use of the helper; explicit `bmin::makeUnique<Action>(...)` is
acceptable. Convert all action, layer, UI, application, and test call sites.
`AbstractAction::enqueueAction` and `insertAction` must also accept/forward
`UniquePtr`.

Acceptance criteria:

- no scheduling API accepts an owning `AbstractAction*`;
- no action scheduling call contains `new`;
- sequential, inserted, delayed, and parallel ordering tests remain unchanged;
- action destruction remains exactly once on normal execution and cancellation;
- the global verification contract passes.

Commit: `Make action queue ownership explicit`

## Phase 3: Make layer and UI ownership explicit

Convert the remaining container-adopts-raw-pointer APIs.

Layer targets:

- `LayerFactory` returns `bmin::UniquePtr<Layer>`;
- layer creation helpers and `LayerManager::createLayer` return
  `bmin::UniquePtr<Layer>`;
- `LayerManager` owns `bmin::DynArray<bmin::UniquePtr<Layer>>`;
- `addLayer` accepts a `UniquePtr<Layer>`;
- lookup and event-order APIs expose non-owning `Layer*`/`const Layer*` only;
- remove manual `delete` calls and the custom destructor cleanup loop;
- remove mutable access to the owning layer container unless a demonstrated
  test seam requires it.

UI targets:

- `UiElement::addChild` accepts `bmin::UniquePtr<UiElement>`;
- `UiElement::addEventObserver` accepts
  `bmin::UniquePtr<UiEventObserver>`;
- every other API that adopts a UI element or observer, including button-group,
  title-element, modal, and layer helper APIs, must be inventoried and converted;
- add `emplaceChild`/`emplaceObserver` helpers only if they simplify call sites
  without obscuring the concrete type being constructed;
- parent pointers and lookup results remain explicitly non-owning.

Perform the layer and UI conversions in one phase so no compatibility API that
adopts raw pointers survives into later work.

Acceptance criteria:

- ownership transfer is visible in every action, layer, child, and observer API;
- `rg` finds no manual deletion of objects owned by those containers;
- layer/UI lifecycle tests pass under AddressSanitizer when the host toolchain
  supports it;
- the global verification contract passes.

Commit: `Make layer and UI ownership explicit`

## Phase 4: Replace static service locators with explicit dependencies

Remove the process-global pointers in `StateManagerInterface` and
`DatabaseInterface` incrementally, keeping the project buildable throughout the
phase.

### Action execution context

Add a small state-owned execution context containing:

- the current `State&`;
- the current `StateManager&` for scheduling follow-up actions;
- an explicit `db::Database*` when database access is optional.

Change `AbstractAction::execute`/`act` to receive this context. Header-only
actions may use context accessors but may not recover either service through a
static variable. `StateManager` is constructed with its database dependency
(an explicit `nullptr` is allowed in focused tests) and creates the execution
context when dispatching an action.

Actions must never receive an SDL window through this context. Constructors
whose `Window*` parameter is unused, including the current special-event layer
action, must drop that parameter.

### Layer and UI dependencies

Construct `LayerManager` with explicit window, state-manager, and database
dependencies. Pass those dependencies to layers explicitly. `Layer` may expose
protected non-owning accessors, but must not inherit either service-locator
class.

Add a small `ui::Context` containing the window plus explicit optional/required
state-manager and database pointers appropriate for component tests. Pass that
context to `UiElement` and through child construction. `UiElement` must not
inherit `StateManagerInterface`.

Use references for dependencies required for the object's entire lifetime and
pointers only where absence is a supported test/component mode. Context
objects do not own the referenced services.

Finally remove `StateManagerInterface.h/.cpp` and
`DatabaseInterface.h/.cpp`, their CMake entries, their inheritance, and all
`setStateManager`/`setDatabase` bootstrap and test cleanup calls.

Acceptance criteria:

- no mutable static state/database pointer exists;
- no production or test code calls `setStateManager` or `setDatabase`;
- two independent state-manager/context graphs can be constructed in one test
  and actions/layers in each observe only their own state;
- destruction order does not require clearing a global pointer;
- the global verification contract passes.

Commit: `Pass runtime dependencies explicitly`

## Phase 5: Give LayerManager one authoritative stack

Replace `UiState::layerStack` as a persistent desired-state mirror with a
one-shot command queue. The state/action layer may request layer changes without
depending upward on concrete layers.

Define explicit command data sufficient for existing behavior, for example:

```cpp
enum class LayerCommandType { Push, Remove, BringToFront };

struct LayerCommand {
  LayerCommandType type;
  LayerRequest request;
};
```

Use the smallest command set that expresses current behavior. Do not add
replace/pop operations without a current call site or test. Preserve the rule
that pushing an already existing layer brings it to the front and updates its
request parameters according to existing behavior.

`LayerManager` consumes the command queue once at a documented point in each
loop iteration. Its ordered owning layer collection becomes the sole runtime
truth for existence, order, active layer, and restoration after close. Remove:

- `layerEventsStack`;
- desired-stack reconciliation;
- layer code that edits the state mirror while being added or closed;
- duplicate `removeFlag`/command state where immediate safe removal can replace
  it. Retain deferred removal only if callbacks can invalidate active iteration,
  and document that invariant.

Provide read-only queries such as `containsLayer(LayerId)` and
`activeLayerId()` from `LayerManager`. Code such as
`LayerWorld::syncWorldActionModeHighlight` must query actual layer state through
an injected read-only layer navigator/query rather than inspecting a second
stack in `State`. Do not give UI components mutable access to the manager's
container.

### Event routing

Change layer input handlers to report whether they consumed an event. Route
mouse-down, mouse-up, wheel, key-down, and key-up from front to back, stopping
at the first consumed event. Default base-layer handlers return false.

`UiLayer` should return the result of UI hit-testing/dispatch. Specialized
layers return true for shortcuts or interactions they actually handle. A layer
that intentionally allows click-through returns false. Hover may remain a
broadcast/polled concern if its semantics are documented and covered by tests.

Acceptance criteria:

- exactly one container determines live layer order;
- state contains pending commands, not a mirrored desired layer stack;
- opening, re-fronting, closing, and restoring layers match characterization
  tests without duplicates;
- a consumed event reaches only the first handling layer;
- explicit pass-through reaches the next eligible layer;
- `Layer` remains outside `ui` and owns loop/interactivity behavior;
- the global verification contract passes.

Commit: `Make layer order and event routing authoritative`

## Phase 6: Clarify world simulation and platform-output boundaries

This phase deliberately preserves independent particle-owned animations.

### Audio output

Move `Window::playSound` calls out of `worldUpdate`. Add a small concrete
platform-output helper, owned or invoked by `LayerManager`, that drains pending
sound requests immediately after the state/action update each frame. It must
run even when `LayerWorld` is suspended or absent.

Keep request de-duplication unless tests establish that repeated simultaneous
sounds are required. Ensure a missing/disabled audio device does not retain an
ever-growing queue.

### Trigger processing

Remove `Window*` from `worldProcessPendingTriggers` and from action constructors
where it is unused. Convert special-event and travel results into normal queued
actions/layer commands rather than directly invoking presentation code. Preserve
the ordering of held-move cancellation, travel, map-change notification, and
special-event opening with focused tests.

### Particle and projectile simulation

Ensure particle lifetime, projectile travel, camera following, and CPU turn
scheduling advance without an SDL window. Particle expiration must not depend
on whether an animation could be initialized.

Keep one independent `sdl2w::Animation` per damage particle for now. Isolate
the store-dependent initialization and playback update in a clearly named
helper so the coupling is visible. `MapView` continues to perform only drawing
of the particle's current animation frame; it must not replace particles with
an animation-name cache.

The helper must preserve this exact progression policy:

- advance the particle lifetime on every world-simulation update;
- if the particle already has an animation, advance that independent animation
  even when no window is present;
- if the animation is absent and a live SDL2W store is available, create that
  particle's animation and advance it;
- if the animation is absent and no store is available, leave it uninitialized
  while still advancing/expiring the particle's simulation lifetime.

This means the simulation continues to control particle playback state and the
view only draws it. The temporary store parameter needed for lazy animation
creation remains an acknowledged SDL2W boundary until the upstream API can
separate a clip from its playback state.

Document this upstream SDL2W follow-up in the results file:

- immutable `AnimationDefinition`/clip and sprites owned by `Store`;
- lightweight independent `AnimationPlayback` containing elapsed time, current
  frame, loop, and finished state;
- drawing a clip using caller-owned playback state or elapsed time.

Do not modify `.deps/sdl2w`, `deps.lock`, or Carcer's particle representation to
assume that future API in this phase.

Acceptance criteria:

- sound playback is not performed by `WorldUpdater` or gated on
  `LayerWorld::update`;
- trigger processing has no window parameter;
- headless world update advances and removes particle data correctly;
- two particles with the same animation name retain independent playback;
- `MapView` draws but does not own a shadow cache of damage-particle playback;
- existing combat, camera, travel, trigger, and `LayerWorld` tests pass;
- the global verification contract passes.

Commit: `Separate world updates from platform output`

## Phase 7: Replace mechanical observers with reusable bindings

Introduce one header-only reusable click/callback observer under
`src/ui/observers/`. It should own a callable and invoke it on click. Add a
typed action-binding helper that creates a fresh `UniquePtr<AbstractAction>` and
enqueues it through the explicit UI context/state manager.

The action factory must create a fresh action for every activation so repeated
clicks work. Do not store a single action instance in a button.

Replace observer classes whose only behavior is:

1. store constructor arguments;
2. obtain the state manager;
3. construct one action with those arguments;
4. enqueue it.

Likely candidates include the remove-layer, layer-opening, party-selection,
spell-selection, rune-adjustment, reorder, pickup, commit, and cancel wrappers
under `src/ui/observers/`. Inspect every observer rather than deleting by name.

Keep named observers or local callbacks when they contain meaningful behavior,
including:

- reading live values from a popup at click time;
- coordinating multiple actions;
- invoking a non-action UI helper;
- section scrolling, slider behavior, and other widget-owned interaction;
- special-event logic that needs more than constructing one action.

Prefer a short binding at the construction site over creating another named
one-method class. Do not merge unrelated UI components or actions into large
files. Remove each obsolete observer header and include only after all call
sites have migrated.

Acceptance criteria:

- no observer class remains whose sole purpose is mechanical one-action
  forwarding;
- repeated clicks construct and enqueue distinct action objects;
- specialized observers retain their behavior and remain near their natural
  owner where appropriate;
- actions remain individually named `.hpp` files;
- UI test targets compile and representative click behavior tests run;
- the global verification contract passes.

Commit: `Replace mechanical observers with action bindings`

## Phase 8: Final verification and documentation

Run a clean native qualification from the documented developer workflow:

```sh
./scripts/setup-dev.sh
make -C src clean
make -C src
make -C src test
make -C src ui
```

Also verify, where available:

- GCC debug and release presets;
- Clang debug and release presets;
- MSYS2/UCRT64 configure, game build, non-UI tests, and UI compile targets;
- Emscripten `make -C src js` output;
- `compile_commands.json` remains usable by clangd from the repository root.

Repeat the Phase 0 clean/no-op timing and size measurements using the same host,
compiler, preset, and job count. The goal is no material regression; investigate
a clean-build regression greater than 10 percent before accepting it. Timing is
diagnostic, not permission to add modules, generated unity files, or complicated
source scanning.

Update `README.md` and `DEVELOPMENT.md` only where public construction,
extension, layer, or testing guidance changed. Mark every plan phase complete in
`ARCHITECTURE_REFINEMENT_RESULTS.md`, including commit hashes and any deferred
cross-platform qualification.

Acceptance criteria:

- native game, unit suite, and every UI target build from a clean checkout;
- supported cross-platform results are recorded honestly;
- dependency direction remains unchanged or cleaner;
- no static state/database service locator remains;
- no hidden raw-pointer ownership boundary remains in the changed subsystems;
- layer state has one source of truth and consumed input has deterministic
  propagation;
- particle playback semantics remain independent per particle;
- quick-start commands remain unchanged.

Commit: `Document refined runtime architecture`

## Stop conditions

Stop and ask the user before proceeding if:

- removing a dormant candidate would discard reachable behavior;
- layer characterization reveals intentional simultaneous active input layers
  that conflict with consumed top-down routing;
- a UI/layer context would require reversing the documented include direction;
- independent particle playback cannot be preserved without changing SDL2W;
- a phase requires changing a pinned dependency revision;
- native tests expose a behavior disagreement not resolved by existing tests or
  this plan;
- unrelated user changes overlap the phase's files.

## Explicitly deferred work

The following are not required to complete this plan:

- the SDL2W animation clip/playback API redesign;
- changing the pinned SDL2W or BMIN revisions;
- splitting Carcer into multiple CMake libraries;
- reintroducing an architecture-lint or migration script;
- renaming every namespace or directory solely for aesthetic consistency;
- converting the project to ECS, modules, or another UI framework;
- optimizing large source files solely because of line count.
