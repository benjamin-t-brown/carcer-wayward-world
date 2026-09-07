# Header Architecture Restoration Plan

Status: Implemented through Phase 8 locally; supported-host UCRT64 and Emscripten qualification remains open
Plan source branch: `experiment/cpp-modules` at `d85a0e1`  
Header-layout reference: `origin/main` at `fe50a90`  
Date: 2026-09-07

## Objective

Return Carcer itself to a conventional C++23 header/source architecture while
preserving the useful results of the module experiment:

- pinned, explicit SDL2W/BMIN acquisition under `.deps/`;
- the corrected dependency direction between data, model, rules, state,
  actions, UI, layers, and application bootstrap;
- explicit dependencies instead of service-locator recovery;
- stable action events instead of RTTI/downcasts;
- native, Windows, and Emscripten build knowledge;
- focused behavioral tests and automated architecture checks.

The result must restore human-scale files, keep layers outside UI, make every
test runner useful again, and remove Carcer's module-scanning/BMI machinery.

## Starting evidence

The header reference contains 249 tracked headers, 217 `.cpp` files, and no
`.cppm` files. The current experiment contains 20 `.cppm` interfaces and only
two tracked headers. Its consolidation produced implementation units including
`actions/actions.cpp` at 3,746 lines, `ui/layers.cpp` at 2,834 lines,
`ui/widgets/controls.cpp` at 2,391 lines, and `ui/screens/pages.cpp` at 1,850
lines. The final measured module build was 69 seconds on the qualification host
versus the historical 34.22-second header build. Phase 0 must remeasure the
header baseline on the implementation host rather than assuming the historical
number remains comparable.

## Decisions fixed by this plan

An implementing agent must follow these decisions unless the user explicitly
revises this document before that phase begins.

1. Create a new branch named `refactor/header-architecture`; do not rewrite or
   delete `experiment/cpp-modules`.
2. Start the new branch from the known header revision `fe50a90`, not by
   mechanically rewriting the combined module files in place. Treat `d85a0e1`
   as the source of current behavior and architectural improvements.
3. Use `fe50a90` only as a file-layout and older-behavior reference. Never
   restore a whole directory from it over newer work without comparing every
   affected type and method with `d85a0e1`.
4. Keep CMake/Ninja as the single final build system, but replace the current
   module build with ordinary libraries, executables, and test targets.
5. Keep the old Make build only as a temporary parity oracle. Remove it in a
   dedicated commit as soon as the conventional CMake build passes the same
   supported configurations and test-runner contract.
6. Carcer production code uses self-contained headers. It does not `import`
   Carcer, SDL2W, or BMIN modules.
7. Keep the module-capable SDL2W/BMIN revisions and their pinned bootstrap.
   Carcer consumes their classic-header artifacts because its public headers
   expose SDL2W/BMIN types. A separate opt-in dependency module probe may
   continue validating upstream module support, but named modules must not be
   part of Carcer's default build graph.
8. Restore one primary class/concept per header/source pair for UI and layers.
   Restore one action per `.hpp`, matching the preferred header-only action
   style. Do not recreate facade, umbrella, barrel, or one-class module files.
9. `layers` is a top-level orchestration subsystem. UI must never own or import
   layers. A concrete layer may contain and include UI components.
10. Preserve behavior unless a test proves the module branch accidentally
    changed it or the user explicitly approves a behavior change.

## Why dependency modules are not the application path

A normal Carcer header must be compilable by itself. Many Carcer declarations
contain `bmin` containers/strings or SDL2W types by value, inheritance, or
template argument. Such headers need the corresponding dependency declarations
before they can be parsed. Importing the named module in implementation files
after those headers would mix the classic and module definitions in one
translation unit; placing imports inside project headers would retain module
scanning and make the headers toolchain-dependent. PIMPL wrappers could avoid
that, but would be a larger and less readable redesign.

Therefore this plan preserves the new dependency versions, pinning, bootstrap,
dual-mode upstream capability, and optional module validation while using the
header API in Carcer itself.

This boundary was explicitly accepted on 2026-09-07. The module-capable
SDL2W/BMIN revisions remain the pinned integration source in anticipation of
their upstream merge; choosing Carcer's header API is a consumer-mode decision,
not a rollback of those dependency improvements.

## Target architecture

Dependencies point downward in this table.

| Level | Paths | May depend on |
|---|---|---|
| Foundation | `src/lib/`, dependency headers | standard library, SDL2W/BMIN where owned |
| Data | `src/data/` and template/value definitions | foundation |
| Model | `src/model/` | foundation, data; never database/state/actions/UI/layers |
| Database | `src/db/` | foundation, data, model |
| Rules | `src/game/`, `src/in3/` | foundation, data, model, database; never state/actions/UI/layers |
| State | `src/state/` | lower levels; never actions/UI/layers |
| Actions | `src/actions/` | lower levels and state; never UI/layers |
| UI | `src/ui/` | lower levels, state, and action APIs; never layers |
| Layers | `src/layers/` | all lower levels including UI; owns loop segmentation and event routing |
| Application | `src/main.cpp` and optional `src/app/` | layers/bootstrap only where practical |

The permanent architecture checker must enforce at least these forbidden
directions:

- model → database, state, actions, UI, or layers;
- rules → state, actions, UI, or layers;
- state → actions, UI, or layers;
- actions → UI or layers;
- UI → layers.

Any unavoidable exception must be a specific checked-in `source -> target`
edge with a written reason. Wildcard exceptions are forbidden, and the
exception count may not increase during the migration.

## Target source layout

Use the `fe50a90` tree as the initial path manifest. Expected examples include:

```text
src/actions/{combat,general,navigation,world}/*.hpp
src/layers/Layer.h
src/layers/Layer.cpp
src/layers/LayerManager.h
src/layers/LayerManager.cpp
src/layers/ui/LayerInventory.h
src/layers/ui/LayerInventory.cpp
src/ui/elements/Quad.h
src/ui/elements/Quad.cpp
src/ui/components/ConfirmModal.h
src/ui/components/ConfirmModal.cpp
src/ui/layouts/InGameLayout.h
src/ui/layouts/InGameLayout.cpp
src/ui/minipages/MinipageEvent.h
src/ui/minipages/MinipageEvent.cpp
src/ui/pages/PageInventory.h
src/ui/pages/PageInventory.cpp
src/ui/popups/PopupInventoryItem.h
src/ui/popups/PopupInventoryItem.cpp
```

The old `src/layers/ui/` name means “layers that construct UI,” not “layers are
UI.” Renaming that leaf directory to `game/` or `visual/` is out of scope for
this restoration and can be considered after parity.

File rules:

- One primary public class per `.h`/`.cpp` pair.
- Put that class's property structs and tightly coupled enums in its header.
- Put substantial function bodies in `.cpp`; avoid new header-only UI code.
- Actions are the exception: use one `.hpp` per action, as requested.
- A class-specific observer may remain beside its owner. A reusable observer
  gets its own `.hpp` under `src/ui/observers/`.
- Do not create aggregate headers that include an entire subsystem.
- No restored production `.cpp` should exceed 1,000 lines without an explicit
  note in the migration manifest and user approval.

## Reference commits to mine for behavior

Do not cherry-pick these commits wholesale; most contain module syntax or
generated build changes. Read their diffs and port the semantic changes into
headers:

| Commit | Behavior/architecture to preserve |
|---|---|
| `e0c0b83` | pinned dependency bootstrap and validation |
| `d60964c` | notification expiry no longer constructs an action from state |
| `58b4cc9` | explicit active-map dependencies |
| `9e9617e` | map rules separated from state |
| `eebd21f` | combat round sequencing owned by actions |
| `e25ff69` | final rules/orchestration ownership split |
| `ba41411` | stable action event values and consolidated action semantics |
| `5030c80` | layers above screens in dependency direction |
| `a34939a` | narrow application/bootstrap responsibility |
| `aae59e4` | final forbidden-edge checks |
| `d85a0e1` | MSYS2 SDL entry-point handling and Windows build corrections |

Pure consolidation commits are references for locating current declarations
and bodies, not designs to preserve.

## Agent execution protocol

At the beginning of every phase, the implementing agent must:

1. Read this entire plan and `HEADER_RESTORATION_MANIFEST.md`.
2. Confirm the branch and a clean worktree with `git branch --show-current` and
   `git status --short`.
3. Inspect both versions of every file being ported:
   `git show fe50a90:<path>` for layout and `git show d85a0e1:<path>` or the
   relevant combined module file for current behavior.
4. Update the manifest before deleting or replacing any source implementation.
5. Make only the current phase's changes.
6. Run the phase gate, `git diff --check`, and inspect `git status --short`.
7. Commit only after the gate passes. Use the commit subject specified below.
8. Add the resulting commit hash and verification result to the manifest.

Never use `git reset --hard`, replace a whole current directory from the old
revision, delete the module branch, or hide a regression by disabling a test.
If an old and current behavior conflict and tests do not resolve the intent,
stop and ask the user one focused question.

## Phase 0: establish the restoration branch and parity manifest

The plan should first be committed on `experiment/cpp-modules`. Save that hash
as `<plan-commit>`, then:

```sh
git switch -c refactor/header-architecture fe50a90
git cherry-pick <plan-commit>
```

Create `HEADER_RESTORATION_MANIFEST.md` containing:

- fixed reference hashes (`fe50a90` and `d85a0e1`);
- baseline compiler/tool versions and host;
- every production header/source path from `fe50a90` grouped by subsystem;
- every public type/function found in the 20 interfaces at `d85a0e1`;
- the current implementation file containing each symbol;
- the target header/source path;
- status: `pending`, `ported`, `verified`, or `intentionally retired`;
- relevant test target(s);
- a section for semantic commits listed above;
- a section for known stale/disabled tests.

Run and record the unmodified header baseline:

```sh
make -C src -j8
bash test-runners/runner/TestJson.sh
bash test-runners/model/TestCharacterEquip.sh
bash test-runners/ui/TestConfirmModal.sh --build-only
```

Also record a clean eight-job build time and the number of production headers,
sources, and tests. A baseline failure is recorded as pre-existing; it must not
be silently fixed in this phase.

Gate:

- restoration branch points at `fe50a90` plus the plan/manifest only;
- manifest accounts for every current exported symbol;
- baseline build/test results are recorded;
- no production behavior has changed.

Commit: `Record the header restoration baseline`

## Phase 1: add pinned dependencies and conventional CMake

Port the durable dependency/build pieces from `d85a0e1`:

- `deps.lock`;
- `scripts/bootstrap-deps.sh`;
- the header-consumer portion of `scripts/build-deps.sh`;
- pinned-checkout changes to `scripts/anims.sh` and
  `scripts/update-translations.sh`;
- `scripts/Invoke-Ucrt64.ps1`;
- `cmake/win_sdl_main_ready.cpp` and the associated Windows SDL link behavior.

Add a conventional CMake build. Requirements:

- C++23, Ninja presets, and `CMAKE_EXPORT_COMPILE_COMMANDS=ON`;
- no `CXX_MODULES`, `CMAKE_CXX_SCAN_FOR_MODULES`, `-fmodules-ts`,
  `clang-scan-deps`, `.cppm`, BMI, or dyndep-specific logic;
- a `carcer_lib` static library built from an explicit, domain-grouped source
  list in `cmake/carcer_sources.cmake`;
- one `CARCER` executable linked to `carcer_lib`;
- imported SDL2W/BMIN header-mode libraries staged under ignored
  `build/compat/`, keyed by compiler/target/flags/lock identity;
- dependency validation at configure and build without cloning or switching;
- SDL discovery through target usage requirements, not global flags;
- preservation of Emscripten port, preload, memory, and exported-function
  options;
- preservation of the Windows `SDL_SetMainReady` solution from `d85a0e1`;
- debug/release presets for GCC, Clang, UCRT64, and Emscripten;
- compiler selection overridable through CMake rather than hard-coded into
  production logic.

Keep `src/Makefile` working during this phase. It may be minimally updated to
use the pinned header bundle, but do not add new capabilities to it.

Add an opt-in dependency-module compatibility script only if SDL2W/BMIN do not
already expose an upstream command that proves their modules. It must not be an
`ALL` target and must not add module flags to Carcer.

Gate:

- Make and CMake build the same header application;
- GCC debug/release and Clang debug/release configure and build;
- dependency bootstrap check fails clearly when a checkout is absent or at the
  wrong revision;
- a second no-op CMake build performs no C++ compilation;
- UCRT64 and Emscripten commands are documented; if unavailable locally, this
  phase remains open until the user or CI records those results.

Commit: `Add the conventional pinned-dependency CMake build`

## Phase 2: restore the test-runner contract and retire Make

Add ordinary CMake test targets for every source under:

- `src/__test__/runner/`;
- `src/__test__/db/`;
- `src/__test__/model/`;
- `src/__test__/ui/`.

Each existing shell wrapper must retain its old user-facing contract:

- configure/build only the requested test and required library work;
- run non-UI tests by default;
- honor `--build-only`;
- pass remaining arguments to an interactive UI executable;
- work from any current directory;
- propagate the compiler/build/run exit status;
- avoid shared `src/TestUi` executables so parallel invocations cannot race.

The helper may use Node or a shell script, but there must be one implementation
of target-name/path selection. Select the preset with
`CARCER_CMAKE_PRESET`, defaulting to `gcc-debug` and to `ucrt64-debug` inside
MSYS2 UCRT64.

Add aggregate commands for all non-UI tests and compile-only UI tests. Do not
automatically execute SDL UI tests.

Run every non-UI wrapper, not merely CTest, and run every UI wrapper with
`--build-only`. Fix stale tests against the intended current API or explicitly
retire them with user approval; do not omit CMake targets merely to obtain a
green build.

After GCC, Clang, UCRT64, and Emscripten parity from Phase 1 and runner parity
are recorded, delete `src/Makefile` in its own commit. Update all docs, VS Code,
Cursor rules, package scripts, and CI commands in that same commit so no live
instruction invokes it.

Gate:

- every retained test wrapper resolves to an existing target;
- aggregate CTest and UI compile-only commands pass;
- wrapper exit codes are correct for both success and an intentionally induced
  compile failure;
- repository search finds no live Carcer `make` build command;
- upstream Make invocations used internally by SDL2W/BMIN tooling are allowed.

Commit: `Restore CMake-backed test runners`  
Dedicated follow-up commit: `Retire the transitional Make build`

## Phase 3: port foundation, data, model, and database behavior

Start with leaf declarations so later phases can include stable headers.

Port the current behavior from these module interfaces and implementations:

- `carcer.lib.Json` and `carcer.lib.StringUtil`;
- `carcer.data`;
- `carcer.model`;
- `carcer.db`;
- `carcer.game.map.TileFields`.

Restore the class/concept file boundaries represented in `fe50a90`, including
model template, instance, stats, database, and loader files. Where the current
module branch added a type or combined an obsolete distinction, choose the
smallest owning header and record the mapping in the manifest. Do not recreate
one `Data.h` or `Model.h` umbrella.

Preserve these architectural results:

- model APIs do not locate or accept the concrete database for behavior that
  belongs in rules;
- inventory/equipment lookup behavior stays in inventory rules;
- map-character construction stays with map rules;
- combat-party population stays with combat rules;
- data/value types remain below runtime model and database.

Add `scripts/check-include-architecture.py` (or an equivalently deterministic
tool) now. It must normalize project-relative quoted includes, enforce the
forbidden matrix, print complete `source -> target` violations, and be
registered as a CTest.

Add header self-containment compile checks for the headers touched in this
phase. A public header passes only when a generated temporary translation unit
can include it first and compile with the project's normal include paths.

Gate:

- all Phase 3 manifest rows are `verified` or approved `intentionally retired`;
- architecture check has no unapproved forbidden edges;
- touched headers pass self-containment;
- GCC and Clang builds and all data/model/database tests pass.

Commit: `Restore header boundaries for data and model`

## Phase 4: port rules and state ownership cleanup

Restore individual headers/sources for `src/game/`, `src/in3/`, and
`src/state/`, using current behavior rather than blindly copying the baseline.

Port and test the semantic changes from `d60964c`, `58b4cc9`, `9e9617e`,
`eebd21f`, and `e25ff69`:

- notification expiry is state maintenance; explicit dismissal remains an
  action responsibility;
- active map, map-instance storage, and database are explicit constructor or
  call inputs rather than recovered from process-global interfaces;
- map persistence consumes explicit map-instance data;
- movement aging is driven by its orchestration caller;
- tile-trigger rules return values that the caller applies to state;
- combat rules do not import/include state;
- next-turn sequencing, movement counting, and map aging remain action/orchestration
  responsibilities;
- model remains independent of database.

Do not restore the removed `LayerManagerOps` abstraction or other dead adapters
merely because they exist in `fe50a90`.

Gate:

- architecture checker proves rules do not include state/actions/UI/layers;
- state does not include actions/UI/layers;
- map, combat, inventory, special-event, persistence, and state behavioral
  tests pass under GCC and Clang;
- touched headers pass self-containment.

Commit: `Port rules and state ownership to headers`

## Phase 5: restore one-action-per-header organization

Replace the combined action implementation with header-owned actions under
`src/actions/`. Use the action names from `fe50a90` as an inventory and the
factory/behavior list in `d85a0e1:src/actions/_actions.cppm` and
`src/actions/actions.cpp` as the source of truth.

Required shape:

```text
src/actions/Command.hpp
src/state/ActionEvent.h
src/actions/combat/*.hpp
src/actions/general/*.hpp
src/actions/navigation/*.hpp
src/actions/world/*.hpp
```

“Navigation” actions may request layer changes only by writing the neutral
`state::LayerRequest`/`LayerId` representation. They must not include a layer
or UI header. Avoid the misleading `actions/ui` name for commands that do not
own UI.

Preserve:

- move-only ownership and sequential dispatch semantics;
- stable semantic `ActionEvent` delivery and payloads;
- no RTTI or concrete-action downcasts in subscribers;
- combat sequencing and world-update ownership from Phase 4;
- all current action functions/types unless the manifest proves one obsolete.

Header-only action definitions must be `inline` where required by the ODR. Put
shared algorithms in lower rule `.cpp` files instead of copying large bodies
between action headers.

Gate:

- one manifest row exists for every current action factory and old action type;
- no `actions.cpp` mega-file remains;
- actions include neither `src/ui/` nor `src/layers/`;
- focused tests prove command ownership, ordering, events, payloads, combat
  sequencing, and navigation requests;
- GCC and Clang full tests pass.

Commit: `Restore one action per header`

## Phase 6: restore UI class-level files

Split the current UI declarations and bodies back into class-level files. Work
in this order, committing only once the complete phase is green:

1. core utilities and `UiElement`;
2. `elements/` and `elements/buttons/`;
3. reusable `components/`, `components/borders/`, and `components/lists/`;
4. `layouts/`;
5. `minipages/`;
6. `pages/`;
7. `popups/`;
8. observers and helpers.

Use these current combined files as behavior sources:

- `_core.cppm`, `UiElement.cpp`, and `FontScale.cpp`;
- `_widget_foundation.cppm`, `_widget_views.cppm`, and
  `_widget_composites.cppm`;
- `widgets/primitives.cpp`, `controls.cpp`, `foundation_views.cpp`,
  `views.cpp`, and `composites.cpp`;
- `_screens.cppm`;
- `screens/runtime.cpp`, `layouts.cpp`, `overlays.cpp`, and `pages.cpp`.

For every exported UI class/struct/enum in those interfaces, the manifest must
name exactly one target header. For every out-of-line method in the combined
implementations, it must name exactly one target `.cpp`. Remove a combined file
only after all of its rows are verified.

Preserve current fixes and SDL2W behavior, including text-cache/lazy-font use,
asset validation, deterministic window lifetime, double delta time, controller
opt-in, and event/audio behavior where Carcer touches them.

Gate:

- `elements`, `components`, `layouts`, `minipages`, `pages`, and `popups` all
  contain their class-level sources again;
- no UI declaration/implementation mega-file remains;
- no UI file includes a layer header;
- every UI wrapper compiles with `--build-only` under GCC and Clang;
- focused non-visual UI tests pass;
- touched headers pass self-containment.

Commit: `Restore class-level UI headers and sources`

## Phase 7: restore layers as top-level orchestration

Restore `Layer`, `LayerManager`, and each concrete layer under `src/layers/`.
Split the declarations in `d85a0e1:src/ui/_layers.cppm` and the methods in
`src/ui/layers.cpp`, `src/layers/Layer.cpp`, and
`src/layers/LayerManager.cpp` using the old `src/layers/` tree as the path
reference.

Required design:

- `Layer` is not a `UiElement` and its base header includes no UI header;
- `Layer` represents an activatable/suspendable loop segment with lifecycle,
  event, update, and render hooks; rendering may be a no-op;
- `LayerManager` owns activation, suspension, ordering, event dispatch, update,
  and rendering policy;
- concrete visual/game layers may own UI components and include UI headers;
- UI communicates navigation intent through neutral state/action requests and
  never constructs, removes, or imports concrete layers;
- layer factories/registration live in the layer subsystem;
- the application bootstrap creates the manager and initial layer set.

Add tests with at least one non-visual layer proving activation, suspension,
event/update dispatch, and absence of a rendering requirement. Retain the
existing visual layer compile tests.

Gate:

- no production layer declaration remains under `src/ui/`;
- architecture checker proves UI → layers is zero;
- non-visual layer lifecycle test passes;
- layer manager ordering and request-consumption tests pass;
- every visual layer UI test compiles under GCC and Clang;
- full native build/tests pass.

Commit: `Restore layers as top-level loop orchestration`

## Phase 8: remove module machinery and qualify the result

Because the restoration branch started at `fe50a90`, most Carcer module files
should never have entered it. Remove any transitional remnants that were added
while porting:

- all production `.cppm` files and `import carcer`, `import sdl2w`, or
  `import bmin` statements;
- module import probes and BMI/module benchmark scripts;
- module-specific clangd flags and include paths;
- module graph/manifests, scanner workarounds, and module-only CMake options;
- empty migration adapters and aggregate/facade files;
- root module implementation plans after their lasting results are summarized.

Write `MODULE_EXPERIMENT_RETROSPECTIVE.md` summarizing:

- measured wins: file-count reduction and cleaner dependency direction;
- measured costs: cold build, serial/dependency-scan work, artifact size, build
  complexity, and editing costs of combined files;
- architectural changes retained;
- reasons Carcer returned to headers;
- why dependencies remain pinned/dual-capable but Carcer uses header mode;
- final before/after build and source metrics.

Rewrite `DEVELOPMENT.md`, README, VS Code tasks/settings, Cursor rules, and CI
for the conventional CMake workflow. Keep `compile_commands.json` generation
through CMake.

Run final qualification from clean build directories:

```sh
./scripts/bootstrap-deps.sh --check
cmake --preset gcc-debug
cmake --build --preset gcc-debug
ctest --preset gcc-debug
cmake --build --preset gcc-debug --target carcer_ui_tests
cmake --preset gcc-release
cmake --build --preset gcc-release
ctest --preset gcc-release
cmake --preset clang-debug
cmake --build --preset clang-debug
ctest --preset clang-debug
cmake --build --preset clang-debug --target carcer_ui_tests
cmake --preset clang-release
cmake --build --preset clang-release
ctest --preset clang-release
```

Also run UCRT64 debug/release and Emscripten debug/release on their supported
hosts. Run every test wrapper once, with UI wrappers using `--build-only`.
Measure three genuinely fresh GCC debug builds, a no-op build, and a leaf `.cpp`
edit. Compare them with the Phase 0 header baseline and the recorded module
result.

Gate:

- zero production `.cppm` files;
- zero production named-module imports;
- zero module scanning/BMI configuration in the default build;
- zero forbidden include edges;
- every retained public header passes self-containment;
- all retained tests have working wrapper targets;
- all UI tests compile under GCC and Clang;
- GCC, Clang, UCRT64, and Emscripten debug/release builds pass;
- no-op build performs no compilation;
- leaf implementation edit rebuilds only its object and required links;
- fresh header build is no slower than 120% of the Phase 0 same-host baseline;
- no unapproved production `.cpp` exceeds 1,000 lines;
- worktree contains no tracked/generated build artifacts;
- retrospective and developer documentation match actual commands.

Commit: `Complete the return to header architecture`

## Final acceptance checklist

- [x] New work lives on `refactor/header-architecture`; module branch preserved.
- [x] Pinned SDL2W/BMIN bootstrap retained.
- [x] Carcer consumes self-contained dependency headers without named imports.
- [x] Conventional CMake is the only Carcer build graph.
- [x] Layers are top-level and UI never includes them.
- [x] Actions are individually readable `.hpp` files.
- [x] UI classes are restored to class-level header/source pairs.
- [x] Include direction is enforced automatically.
- [x] Explicit map/combat/state ownership improvements are retained.
- [x] Stable action events are retained.
- [x] Every test runner works with its old command-line behavior.
- [ ] Native, UCRT64, and Emscripten matrices pass.
- [x] Header build-time target passes.
- [x] Module experiment results are captured in the retrospective.

## Stop conditions

Stop and ask the user before proceeding when any of the following occurs:

- preserving dependency named-module consumption would require non-self-contained
  Carcer headers or PIMPL conversion;
- current and baseline behavior conflict without a test defining intent;
- a phase would require disabling or deleting a previously working test;
- the include-direction checker requires a new exception;
- Make/CMake output differs before the Make retirement checkpoint;
- UCRT64 or Emscripten cannot be qualified before removing the fallback build;
- a combined implementation cannot be split without changing public behavior;
- unrelated user changes overlap a file being restored.
