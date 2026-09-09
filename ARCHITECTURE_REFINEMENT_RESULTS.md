# Architecture Refinement Results

Companion to `ARCHITECTURE_REFINEMENT_PLAN.md`. Updated after each phase.

## Baseline (Phase 0)

Reproduce with the commands in the "Reproduction" section below.

| Measurement | Value |
| --- | --- |
| Starting revision | `ffbe5a56e37c3491c49e4979ac7421fb143d9fbd` |
| Branch | `refactor/header-architecture` |
| Host | Darwin 25.6.0, x86_64 (macOS) |
| Compiler | Apple clang 21.0.0 (clang-2100.1.1.101) |
| CMake | 4.4.3 |
| Preset | `dev-debug` |
| Clean build (`CARCER`) | ~18.4 s wall (~108 s user+sys, ~590% CPU) |
| Incremental no-op build | ~0.31 s wall |
| Non-UI test executables | 39 sources; 40 CTest tests (39 unit + `HeaderSelfContainment`) |
| UI compile-only targets | 43 sources |
| Full `make -C src test` | ~78 s wall (`HeaderSelfContainment` ~45 s) |
| Full `make -C src ui` | ~15.8 s wall |
| Production `.cpp` in `src` | 226 |
| Production `.h`/`.hpp` in `src` | 265 |
| `CARCER` executable size | 4,699,656 bytes |

### Reproduction

```sh
# from repo root
./scripts/bootstrap-deps.sh --check
rm -rf build/cmake/dev-debug
cmake --preset dev-debug                                   # configure (~2.8 s)
time cmake --build --preset dev-debug --target CARCER      # clean build
time cmake --build --preset dev-debug --target CARCER      # no-op build
ls -l build/cmake/dev-debug/CARCER                         # executable size
ctest --preset dev-debug -N                                # test inventory
make -C src test                                           # full non-UI suite
make -C src ui                                             # compile all UI targets
```

All baseline gates pass (100% of 40 CTest tests; all 43 UI targets link).

## Phase 0 characterization coverage

Existing tests already cover much of the plan's characterization list:

- `src/__test__/model/TestLayerLifecycle.cpp` — pushing the first world layer;
  opening/suspending/closing/restoring an overlay; bringing an existing layer to
  the front without a duplicate; removing a non-front layer; suspended layers
  render while only the active layer updates and receives keys.
- `src/__test__/model/TestCameraFollow.cpp`, `TestWorldTravel.cpp`,
  `src/__test__/runner/TestSpecialEvent*.cpp` — headless `worldUpdate`, travel,
  and special-event/trigger processing.

Added in Phase 0:

- `src/__test__/model/TestSoundQueue.cpp` — `PlaySound` de-duplication and
  headless queue draining by `worldUpdate` (Phase 6 must preserve this).
- `src/__test__/model/TestLayerEventRouting.cpp` — mouse-down/up, wheel,
  key-down/up all reach the front `ON` layer and not the `SUSPENDED` layer
  beneath it; both `ON` and `SUSPENDED` layers render (Phase 5 must preserve
  routing to the active layer).

### Current behavior noted for later phases

- **Event propagation does not stop.** `LayerManager` input handlers iterate all
  layers front-to-back but only `ON` layers receive events, and handlers return
  `void`, so there is no "consumed" signal and no early break
  (`src/layers/LayerManager.cpp` handlers). In practice only one layer is `ON`
  at a time, so routing is effectively front-only today. Phase 5 introduces
  explicit consumption/stop; the routing test above pins the active-layer path.

### Known defect: headless damage-particle lifetime (deferred to Phase 6)

`updateDamageParticles` (`src/actions/world/WorldUpdater.cpp:59`) returns early
when `window == nullptr`, so a damage particle's `lifetime` never advances and
the particle is never removed during a headless `worldUpdate`. The test
framework is plain `main()`-returns-`0/1` with no expected-failure mechanism, so
per the plan the passing regression test is added in **Phase 6** rather than
committing a failing test now.

Intended Phase 6 regression (currently fails against baseline):

```cpp
// Seed one damage particle with a short lifetime, then:
state::worldUpdate(nullptr, stateManager, /*dt=*/1000);
// EXPECT state.world.activeMap.damageParticles.empty();  // fails today
```

Reproduce the defect against the baseline by adding that assertion to a headless
model test; it will fail because the particle survives without a window.

## Phase status

| Phase | Title | Status | Commit |
| --- | --- | --- | --- |
| 0 | Baseline and characterization tests | complete | `d55a967` |
| 1 | Remove dormant and misleading abstractions | complete | `983cc55` |
| 2 | Make action ownership explicit | complete | `fdfda72` |
| 3 | Make layer and UI ownership explicit | complete | `69810fe` |
| 4 | Replace static service locators with explicit dependencies | skipped | — |
| 5 | Give LayerManager one authoritative stack | complete | `726191c` |
| 6 | Clarify world simulation and platform-output boundaries | complete | `c5516dc` |
| 7 | Replace mechanical observers with reusable bindings | complete | (this commit) |
| 8 | Final verification and documentation | not started | — |

## Phase 1 notes

Removed as confirmed-dormant (no references via `rg`, absent from the CMake
manifest and test manifests):

- `src/game/combat/CombatRunner.{h,cpp}` and `src/lib/hiscore/hiscore.{h,cpp}` —
  both `.cpp` files were already silently outside the build graph.
- `src/actions/Command.hpp` — unused raw-pointer command wrapper.
- `src/actions/combat/ActionBase.hpp` — `CombatAction`, an empty marker base
  whose scheduling helpers had already moved to `AbstractAction`. Its 19
  subclasses now derive from `AbstractAction` directly; no dynamic-cast/marker
  use existed.
- `ui::StateInterface` and `UiElement::stateInterface` — declared and
  initialized to `std::nullopt`, never read, set, or dispatched.

Renamed `state::UiManager` → `state::UiStateUpdater` (`src/state/UiStateUpdater.{h,cpp}`)
to reflect that it advances floating-notification timers; timing logic is
unchanged. Manifest, include, member, and call site updated.

The only production `.cpp` outside `cmake/carcer_sources.cmake` are `src/main.cpp`
and `src/app/runCarcer.cpp`, both attached directly to the `CARCER` target in
`CMakeLists.txt`. No silently-excluded production pair remains.

## Phase 2 notes

Action ownership is now explicit end to end. The scheduling API no longer
accepts an owning `AbstractAction*`:

- `StateManager::enqueueAction/insertAction/parallelAction` each take a
  `bmin::UniquePtr<AbstractAction>` by value and operate on the manager's own
  `actionData` (the old `(ActionData&, AbstractAction*, int)` signatures and
  `pllAction` are gone). `pllAction` is renamed `parallelAction`.
- `AbstractAction::enqueueAction/insertAction` forward a `bmin::UniquePtr`; on a
  missing manager the handle is destroyed rather than leaked.
- Call sites allocate through `state::makeAction<Concrete>(...)`, the single
  allocation point. `bmin::UniquePtr` has no derived-to-base converting
  constructor and the pinned dependency must not change, so
  `bmin::makeUnique<Derived>()` cannot be passed to a `UniquePtr<AbstractAction>`
  parameter; `makeAction<T>` builds the owning base handle directly. Because
  perfect forwarding cannot deduce a braced designated-initializer, the three
  `CombatActionContext{...}` arguments now name their type explicitly.
- Pure timed delays keep reading as `insertAction(nullptr, ms)` via a
  `decltype(nullptr)` overload that schedules a null (non-owning) action; the
  update loop already skips null actions, so ordering is unchanged.

`explicit UniquePtr(T*)` is the safety net: any residual raw `new` in a
scheduling call fails to compile. `TestStateManagerActions` (sequential, insert,
nested insert, delayed-front, parallel, and mixed ordering) passes unchanged, so
destruction still happens exactly once on execution and cancellation.

## Phase 3 notes

Layer and UI ownership is now explicit; the transfer is visible in every adopter
API and no compatibility overload that adopts a raw pointer survives.

- **LayerManager owns the layers.** `layers` is now
  `bmin::DynArray<bmin::UniquePtr<Layer>>` — the sole owner. `addLayer` takes a
  `bmin::UniquePtr<Layer>` by value; `LayerFactory` and `createLayer` return
  `bmin::UniquePtr<Layer>`; the `createWorldLayer`/`createPickUpLayer`/
  `createInventoryLayer` helpers return owning handles. `layerEventsStack` stays
  `bmin::DynArray<Layer*>` — non-owning observers into the owning container.
- Lookup/event-order APIs (`getLayerAt`, `getLayerById`, `getLastActiveLayer`)
  expose only non-owning `Layer*`. The mutable owning-container accessors
  (`getLayers()` both overloads) were removed along with the now-unused
  `removeLayerAt`; no caller referenced them.
- All manual layer deletion is gone: `removeLayer` erases the owning entry (drop
  frees it), `reconcileRequests` lets a rejected freshly-created layer free
  itself when its handle goes out of scope, and the destructor's
  `clearLayers()` teardown loop was removed — the implicit destructor releases
  every `UniquePtr`. `rg 'delete '` finds no manual deletion of container-owned
  layers or UI in `src`.
- **UI adopters transfer ownership too.** `UiElement::addChild` and
  `UiElement::addEventObserver` take `bmin::UniquePtr` by value; every other
  adopter (`SectionScrollable`/`BorderDropShadow` `addChild` overrides,
  `VerticalList`/`HorizontalList::addListItem`, `ButtonGroup::addObserverToButtonAtIndex`,
  the modal/in-game `setTitleElement`, `UiLayer::addUiElement`) was converted in
  the same phase. Parent pointers and `getChildById`/lookup results remain
  explicitly non-owning. Dead `addListItems` helpers were removed.
- Call sites wrap the existing raw expression at the point of adoption
  (`addChild(bmin::UniquePtr<ui::UiElement>(expr))`), preserving the pervasive
  "create raw local, configure, adopt, keep using the local as a non-owning
  observer" pattern and add-order/timing. `explicit UniquePtr(T*)` makes any
  stray un-wrapped `new` at an adopter a compile error.

The bmin `UniquePtr` has no derived-to-base converting constructor and the
pinned dependency cannot change, so ownership is transferred by constructing the
base handle directly from the raw pointer (`UniquePtr<Base>(derivedPtr)` via the
`explicit UniquePtr(Base*)` ctor); `makeUnique<Derived>()` cannot be moved into a
`UniquePtr<Base>` parameter.

Gate: `make -C src test` (42 CTest, 100%), `make -C src ui` (all targets link),
`make -C src` (CARCER links), `git diff --check` clean. The build system exposes
no AddressSanitizer preset/option, so the "under ASan if the host supports it"
check was not run rather than adding untracked build configuration.

## Phase 4 notes

**Skipped by decision (not attempted).** Scoping showed Phase 4 touches ~150
files: ~70 `AbstractAction::act` overrides, ~20 UI observers, the four
`DatabaseInterface`-inheriting UI classes (`MapView`, `PageInventory`,
`PageCharacter`, `MinipagePickUp`), `Layer`/`LayerManager`/`UiElement`, plus
~230 `getStateManager`/`getDatabase` call sites, ~60 `execute()` call sites, and
~33 `setStateManager` sites across ~35 test files. The base-class and
`execute`/`act` signature changes are atomic, so the change is large and
cross-cutting. The static locators (`StateManagerInterface`,
`DatabaseInterface`) and `StateManager`'s self-registration remain in place.
Later phases proceed without depending on this removal.

## Phase 5 notes

`LayerManager` now owns one authoritative order and there is no persistent
state mirror of the stack.

- **One-shot command queue replaces the stack mirror.** `UiState::layerStack`
  (a persistent `DynArray<LayerRequest>` that the manager reconciled against
  every frame) is gone. `UiState::layerCommands` is a `DynArray<LayerCommand>`
  — `{LayerCommandType type; LayerRequest request;}` with `type` in
  `{Push, Remove}`. `pushLayerRequest`/`removeLayerRequest` keep their names and
  signatures but now enqueue one `Push`/`Remove` command; every existing action
  and UI call site is unchanged.
- **The owned layer list is the sole truth.** `LayerManager::layers`
  (`DynArray<UniquePtr<Layer>>`) is ordered so the last non-removed entry is the
  front/active layer. `update()` drains the command queue once via
  `applyLayerCommands()`: a `Push` refronts an existing layer (moved to the back
  without destroying it) or creates one and appends it, then activates it; a
  `Remove` marks the matching layer for deferred removal. A `Push` whose factory
  rejects the request (missing item/event) is dropped.
- **`layerEventsStack` and its machinery are removed.** `reconcileRequests`,
  `restoreFrontAfterClose`, `activateLayerNoPush`, `isLiveLayer`, `scrubFromStack`,
  and the parallel non-owning event stack no longer exist. Two focused helpers
  replace them: `focusLayer` (unconditionally activates a target so opening or
  re-opening always fires `onActivate`, matching the previous behavior) and
  `activateFront` (transition-guarded; reactivates the front exposed by a removal
  without spurious callbacks). `moveToFront` now refronts within `layers` and is
  retained for the one caller that opens a baseline layer directly.
- **Deferred removal is preserved and documented.** `applyRemove` only sets
  `removeFlag`; the erase happens in `update()` after the per-layer update pass,
  so callbacks that fire during removal cannot invalidate the layer being
  iterated. `LayerManager::closeLayer` had no callers (layers close themselves by
  enqueuing a `UiRemoveLayer` action) and was removed.
- **Layers query the manager, not state.** A non-owning `LayerManager*`
  back-pointer on `Layer` is set in `addLayer`. `LayerWorld::syncWorldActionModeHighlight`
  reads open/closed state through `LayerManager::containsLayer(LayerId)` instead
  of scanning the former `layerStack`.

Part B of the plan (position-based event-routing consumption / `bool` handlers
with early-out) was **descoped by decision**: layers already accept events based
on whether they are active, not on stack position, so a consumed/stop signal is
unnecessary. Event handlers stay `void` and the front-only routing pinned by
`TestLayerEventRouting` is unchanged.

Gate: `make -C src test` (42 CTest, 100%), `make -C src ui` (all targets link),
`make -C src` (CARCER links), `git diff --check` clean.

## Phase 6 notes

The world-simulation / platform-output boundary is now explicit for damage
particles, closing the deferred defect noted in Phase 0.

- **Lifetime is simulation; the animation is output.** `updateDamageParticles`
  (`src/actions/world/WorldUpdater.cpp`) previously returned early when
  `window == nullptr`, so a headless `worldUpdate` never advanced a particle's
  `lifetime` and expired particles leaked forever. The window/store is now used
  only to lazily create and update the `sdl2w::Animation` (platform output); the
  `lifetime` timer advances and expired particles are erased on every tick,
  windowed or headless. No frame is created without a store.
- **Regression test.** `src/__test__/model/TestDamageParticleLifetime.cpp` seeds
  a particle and runs a headless `worldUpdate(nullptr, …)`: an expired particle
  is removed, and an unexpired one survives with its `lifetime` advanced and no
  animation allocated. This fails against the pre-Phase-6 baseline (the particle
  survives because its lifetime never advances) and passes now. The non-UI suite
  is 43 CTest tests (was 42).

Other platform-output paths already tolerated a null window before this phase:
`worldUpdate` guards sound playback on `window != nullptr` while still draining
the one-shot `soundsToPlay` queue (pinned by `TestSoundQueue`), and
`updateProjectiles` operates purely on world state. No further changes were
needed to run the world simulation headless.

Gate: `make -C src test` (43 CTest, 100%), `make -C src ui` (all targets link),
`make -C src` (CARCER links), `git diff --check` clean.

## Phase 7 notes

Click observers whose only job was to forward one action are gone, replaced by a
single reusable binding.

- **One reusable observer.** `ui::ActionObserver` (`src/ui/observers/ActionObserver.hpp`)
  owns a `std::function<UniquePtr<AbstractAction>(StateManager&)>` factory and, on
  click, builds a fresh action and enqueues it. Returning an empty handle skips
  enqueue, so the factory expresses guards and live reads uniformly. The
  `ui::makeActionObserver<T>(args...)` template covers the common case —
  constructing `state::actions::T(args...)` anew on every click (arguments are
  captured by value, so repeated clicks produce distinct action objects). Because
  `getStateManager()` is a protected static, `ActionObserver` derives from
  `state::StateManagerInterface` just as the old observers did.
- **24 observer headers removed.** The 22 mechanical forwarders (`ObserverRemoveLayer`,
  `ObserverDropInventoryItem`, `ObserverReorderInventoryItem`,
  `ObserverSetCurrentPartyMember{,Inventory,Magic}`, `ObserverInventorySelectItem`,
  `ObserverPickUpItem`, `ObserverShowLayer{DropContext,GiveContext,InventoryContext,
  PickUpContext,SpellInfo,EquipRunes,PopupText}`, `ObserverSelectSpellCast`,
  `ObserverSpecialEvent{Choice,Continue}`, `ObserverCancelEquipRunes`,
  `ObserverCommitEquipRunes`, `ObserverAdjustEquippedRune`,
  `ObserverSetSelectedPartyMemberId`) were migrated to `makeActionObserver<T>` or,
  where a runtime guard exists, an inline `ActionObserver` factory. Two headers
  (`ObserverSetSpellReady`, `ObserverToggleManaSlotRune`) had no call sites and were
  simply deleted. `HeaderSelfContainment` globs headers, so no manifest changes were
  needed.
- **Kept as classes.** `ObserverWorldAction` and `ObserverCancelWorldActionMode`
  invoke world-action helpers rather than enqueueing an action;
  `ObserverGiveInventoryItem` (reads the give-popup quantity at click) and
  `ObserverUpdateCurrentPartyMember` (computes the wrap-around party index) carry
  real behavior beyond forwarding. These four remain.
- **Guards preserved.** Sites whose guard was already at the call site
  (`!characterPlayerId.empty()`, `!spell.id.empty()`, `!helpDescription.empty()`)
  migrated to the pure `makeActionObserver<T>` form with identical behavior. The one
  runtime guard not visible at the call site — locking party switching during combat
  in `LayerWorld` — became an inline factory that returns an empty handle when
  `combat.active`. `LayerId`-from-string sites (`LayerPopupText`, the pickup/spell-cast
  "Done" buttons) resolve the id with `layerIdFromString` and attach the observer only
  when it resolves, matching the former `!layerId` no-op.

Gate: `make -C src test` (43 CTest, 100%), `make -C src ui` (all targets link),
`make -C src` (CARCER links), `git diff --check` clean.

## Deferred / known items

- Cross-platform qualification (MSYS2/UCRT64, Emscripten) is deferred to Phase 8;
  those toolchains are unavailable on the implementation host.
</content>
</invoke>
