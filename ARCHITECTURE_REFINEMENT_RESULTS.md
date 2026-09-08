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
| 2 | Make action ownership explicit | complete | (this commit) |
| 3 | Make layer and UI ownership explicit | not started | — |
| 4 | Replace static service locators with explicit dependencies | not started | — |
| 5 | Give LayerManager one authoritative stack | not started | — |
| 6 | Clarify world simulation and platform-output boundaries | not started | — |
| 7 | Replace mechanical observers with reusable bindings | not started | — |
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

## Deferred / known items

- Cross-platform qualification (MSYS2/UCRT64, Emscripten) is deferred to Phase 8;
  those toolchains are unavailable on the implementation host.
</content>
</invoke>
