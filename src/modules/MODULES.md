# Carcer C++ modules

Carcer ships as C++23 named modules (`carcer.*`). This doc is the contract for
how the tree is organised, how to import across it, and how the build graph works.

> **Migration status (2026-09):** the ~185-module-per-class tree has been
> consolidated to **30 top-level modules** (28 domain modules + the `carcer`
> umbrella + the `carcer.game.map.TileFields` leaf). `carcer.layers` is now
> one module, 15 partitions, no exceptions. One holdout remains:
> `carcer.ui.pages` is still 5 separate per-widget modules, blocked by the
> same GCC bug that used to force `carcer.layers` apart — see §6a for the
> full diagnosis and how `carcer.layers` got fixed for real (not worked
> around). `scripts/modules/
> partitionize_folder.py` and `consolidate_module.py` do the mechanical
> conversion when revisiting `pages` or splitting a module later;
> `scripts/modules/gen_bmi_makefile.py` regenerates the build graph and is a
> permanent tool (see §6), not a migration one-shot.

## 1. Why modules here

Modules replaced headers for two payoffs:

- **Isolation.** No include-order fragility, no macro leakage, and names that
  aren't `export`ed have module linkage — genuinely invisible outside, so no ODR
  landmines.
- **Build speed.** An interface is parsed once into a BMI; importers load the
  BMI instead of re-parsing text.

Both payoffs need a **shallow, wide** dependency graph. 185 micro-modules in a
deep chain gave neither (the graph was as deep as the old `#include` graph, and
`import carcer;` everywhere dragged the whole closure). Hence **folder-sized
modules** — 30 of them, plus the umbrella and one intentional leaf — like
`sdl2w.window` / `sdl2w.draw`.

## 2. The layering

Dependencies point **down** this list. An arrow pointing up is a design smell —
fix it by moving the shared type down (usually into `carcer.model.templates` or
`carcer.ui.core`) or by routing through an interface seam:
`StateManagerInterface`, `DatabaseInterface`, `LayerManagerInterface`.

| Layer | Modules | Role |
|---|---|---|
| 0 Foundations | `carcer.lib.Json`, `carcer.lib.StringUtil`, `carcer.lib.hiscore.hiscore` (+ external `sdl2w`, `bmin`) | pure utilities: json, strings, hiscore. No game knowledge. |
| 1 Static data | `carcer.model.templates`, `carcer.game.map.TileFields` (leaf) | immutable definitions mirrored from `assets/db`. |
| 2 Data access | `carcer.db` | loads templates, owns lookup registries. |
| 3 Runtime model | `carcer.model.instances` | the mutable store shape: live characters, maps, items, world, combat state. |
| 4 Rules | `carcer.game.map`, `carcer.game.combat`, `carcer.in3` | pure-ish logic over the model; compute results, don't own state. Independent siblings. |
| 5 State kernel | `carcer.state` | store + `ActionBus` + `AbstractAction` base + `WorldUpdater` + interface seams + `LayerRequest`/`layerStack`. Small, stable, universally depended on. |
| 6 Actions | `carcer.actions` (partitions `:combat` / `:world` / `:ui`) | one command class per state transition; `act()` mutates state, calls rules, enqueues timed follow-ups. Pushes `LayerRequest`s onto `state` rather than calling layers directly. |
| 7 UI widgets | `carcer.ui.core` → `carcer.ui.elements` → `carcer.ui.components` → `carcer.ui.layouts` → `carcer.ui.{minipages,popups,pages.*}` (+ `carcer.ui.helpers`, `carcer.ui.lists`, `carcer.ui.KeyboardHeldScroll`, `carcer.ui.ObserverRemoveLayer`) | framework → primitives → game-aware composites → screens. Read model/state to render; enqueue actions on interaction. |
| 8 Screen stack | `carcer.layers` | `LayerManager` owns the stack; each `Layer*` binds a UI page + input + its state slice. One module, 15 partitions (`Layer`, `LayerManager`, and 13 `LayerX` screens), one file per class, no exceptions — see §6a for why that took two attempts. |
| 9 Entry | `carcer` umbrella, `main.cpp` | umbrella used only by `main` + tests. |

**Frame flow:** input → `LayerManager` → active `Layer` → widget `onClick`
constructs an action and enqueues it on `ActionBus` → `StateManager` drains the
queue, each `act()` mutates `State` / calls rules / enqueues follow-ups →
`WorldUpdater` ticks time-based logic → `LayerManager.render` → widgets read the
new state and draw.

`carcer.in3` (special-event / dialogue-trigger scripting, named for the
*Imagine Nation* lineage) holds event state such as "awaiting a dialogue choice".
UI reads that state to render dialogue / signs / modals and, on interaction,
enqueues an action that calls back into `in3` to advance it. `in3` therefore
**never imports `carcer.actions` in its interface**.

**Layers as a function of state.** `carcer.actions` never imports
`carcer.layers` (there'd be no acyclic order — `layers` sits above `actions`).
Instead an action that wants to open/close a screen pushes/removes a
`state::LayerRequest` on `State::uiState.layerStack`
(`pushLayerRequest()`/`removeLayerRequest()` in `carcer.state`).
`LayerManager::update()` is meant to reconcile its live `Layer*` stack against
`layerStack` each frame (construct/destroy to match, resolving a request's
`itemId`/`eventId` fields back to real objects via `state`/`db` at that point).
**That reconciler body, and `main.cpp`'s `StateManager`/`LayerManager`
construction + per-frame wiring, are not implemented yet** — both are
TODO-marked at their integration points; this is real game-loop feature work,
out of scope for the modules conversion. `__test__/ui/layers/TestLayerWorld.cpp`
shows the intended per-frame shape (`layerManager->update(dt);
stateManager.update(dt); … layerManager->render(dt);`).

## 3. What is (and isn't) a module

- **A module is a directory of cohesive code** — roughly, something you could
  ship as a static lib with a documented surface. 30 domain modules total.
- Two pieces of code go in **separate modules** when they differ in *change
  frequency* or *dependency footprint*, even if related — a module is the unit
  of rebuild invalidation. They share a module (as partitions, or as one merged
  interface unit) when they change together and share downstream deps.
  - `carcer.state` vs `carcer.actions`: the kernel is small, stable, and
    depended on by everything; actions are numerous, churny, and drag in
    `game` + `model`. Separate.
  - `carcer.in3` vs `carcer.game.*`: narrower footprint (`lib` + `model` only),
    different cadence (a scripting VM, not spatial/combat mechanics). Separate.
  - `carcer.layers`: the screen stack is genuinely one module — every
    `LayerX` (15 of them, `Layer` and `LayerManager` included) is a
    partition, each in its own file. A UI-observer helper that needed to
    call back into `LayerSpecialEvent` briefly forced `Layer` and
    `LayerSpecialEvent` out into their own modules to work around a GCC bug
    (§6a); the actual fix was relocating that helper to `carcer.ui.*` (it
    belonged there anyway — see `carcer.ui.ObserverRemoveLayer` in the
    layering table above, the existing precedent for a UI observer), which
    let `carcer.layers` go back to being one module.
- If two folders turn out to be mutually dependent at folder granularity (e.g.
  an element imports a button while a button imports an element), that's **one
  module**, not two with a forced layer. (`carcer.ui.elements` absorbs its
  `buttons/` subfolder for exactly this reason.)

## 4. File layout & partition mechanics

Two equally-valid shapes are in use, depending on the module:

**(a) Partitions** — one file per class, declaration + inline bodies:

```
src/<folder>/<folder>.cppm     export module carcer.<folder>;          (primary interface unit / aggregator)
                               export import :Thing;
                               export import :Other;
src/<folder>/Thing.cppm        export module carcer.<folder>:Thing;    (partition: declaration + inline bodies)
                               import :Other;                          (sibling partition)
                               import carcer.model.instances;          (cross-module: full name)
src/<folder>/Thing.cpp         module carcer.<folder>;                 (impl unit: bodies for any partition)
```

Used by `carcer.model.templates`, `carcer.model.instances`, `carcer.ui.core`,
`carcer.ui.elements`, `carcer.ui.components`, `carcer.ui.lists`,
`carcer.ui.layouts`, `carcer.ui.minipages`, `carcer.ui.popups`, `carcer.layers`
(15 partitions: `Layer`, `LayerManager`, and 13 `LayerX` screens — each
partition file still declares its class in `export { … }` and defines the
body out-of-line after `} // export`, same convention as shape (b) below;
that's a per-file style choice, independent of whether the file is a
partition or its own module).

**(b) One merged interface unit per module**, declarations for every class in
one `export { … }` block, method bodies defined out-of-line *after* the
`} // export` closer (still non-exported — module-linkage-only, just not in a
separate `.cpp`), with a handful of specific classes' bodies pulled into a
sibling `.cpp` impl unit when they're the exception (see below):

```
src/<folder>/<folder>.cppm     module;
                               #include <...>                         (global module fragment)
                               export module carcer.<folder>;
                               import ...;
                               export {
                                 class A { ... };                     (declarations, all classes)
                                 class B { ... };
                               } // export
                               A::method() { ... }                    (bodies, non-exported, same TU)
                               B::method() { ... }
src/<folder>/hot.cpp           module carcer.<folder>;                (impl unit: body for one specific class)
```

Used by `carcer.actions` (4 partitions `:combat`/`:world`/`:ui`, each this
shape, multiple classes per file). This shape was adopted (rather than
per-class partitions) where a folder's class count blew past GCC's
GCM-corruption ceiling (§6a) — merging into fewer, larger translation units
sidesteps that, at the cost of coarser rebuild invalidation within the
module.

Both shapes:

- **Partition names are flat** — `carcer.ui.elements:Quad`, never dotted after
  the `:` (GCC BMI stability).
- The primary unit of a partitioned module only `export import`s partitions —
  no code of its own beyond what's needed to curate the surface.
- **Bodies out-of-line, in a `.cpp` impl unit, when:** a file is a build
  hotspot, its bodies pull heavy deps you don't want in the interface, or (in
  `carcer.actions`) the body itself would create an import cycle between
  partitions — see the `EndCombat`/`PerformMeleeAttack`/`PerformSpellCast`/
  `SetActiveCombatCharacter`/`WorldExamineAt`/`UiSelectSpellCast` bodies in
  `actions/{combat,world,ui}/{combat,world,ui}.cpp`. An implementation unit
  implicitly imports the primary interface unit, so it sees every partition
  without importing them.
- Two bodies **must** stay in `.cpp` regardless of shape — `ChCompactInfo`,
  `ListMagicSpells` — their nested `bmin::DynArray` shapes corrupt GCC GCMs
  when inline.
- Namespaces (`ui::`, `state::`, …) are unchanged by any of this. Modules are the
  shipping boundary; namespaces are the naming one.

## 5. Import rules

- **Production `.cpp` / `.cppm` import the specific domain modules they use**
  (`import carcer.model.instances;`, `import carcer.ui.elements;`). Do **not**
  `import carcer;` outside `main.cpp` and `src/__test__/` — the umbrella
  `export import`s every domain and flattens all isolation. (Verified
  2026-09: no production `.cpp` currently does this — the rule is a guardrail
  for new code, not a pending cleanup.)
- `main.cpp` and tests may `import carcer;` for convenience.
- **Never mix** classic `#include "bmin/…"` / `#include "sdl2w/…"` headers with
  `import bmin.*` / `import sdl2w;` in the same TU.
- **Include ordering.** Textually `#include <std-header>` *after* an `import`
  whose reachable code already included that header makes GCC choke
  (`redefinition of 'std::__is_constant_evaluated'` and similar). So:
  - in a `.cppm`, put std `#include`s in the global module fragment
    (`module;` … before `export module`);
  - in a `.cpp` / `main.cpp`, put all `#include`s at the very top, before any
    `import`.
  - `#include "macros.h"` is the deliberate exception — it only `#define`s
    (`TRANSLATE`, `LOG`, `THROW_RUNTIME_ERROR`), so it goes *after*
    `import sdl2w;`. It's found via `-Imodules`. `TRANSLATE` must stay a macro
    (L10nScanner greps sources for it).
- External deps: `sdl2w` + bundled `bmin` are consumed as prebuilt modules via
  `lib/sdl2w/modules/make/use.mk` (populated by `copy-sdl2w-artifacts.sh`).
  Flags and BMIs come from `use.mk` — don't hand-roll include paths to them.

## 6. Build graph

Two toolchains, two module models:

| | native | wasm |
|---|---|---|
| compiler | `g++` (UCRT64) | `em++` / Clang |
| flag | `-std=c++23 -fmodules-ts` | `-std=c++23` (Clang rejects `-fmodules-ts`) |
| BMI | `gcm.cache/` (auto-discovered) | `pcm.cache/*.pcm` via `--precompile` + `-fprebuilt-module-path` |
| partition BMI file | automatic | `pcm.cache/carcer.<mod>-<Part>.pcm` (`:` → `-`) |
| driven by | `src/modules/make/build-bmi.mk` | `src/modules/make/build-bmi-em.mk` (not yet built for `carcer` — wasm is deferred, see below) |

Both are generated by `scripts/modules/gen_bmi_makefile.py` from the
`export module` / `import` / `import :part` edges in the `.cppm` files. Ordering
rules: a module's partitions compile before its primary interface unit; a module
compiles before anything that imports it; `carcer` (umbrella) is last. The BMI
step runs `-j1`; `.cpp` implementation units and the final link are parallel.

**`gen_bmi_makefile.py` is a permanent tool, not a migration one-shot.** The
original plan assumed it'd be replaced by a hand-written makefile once the
module count dropped to ~20; at 30 domain modules with a mix of partitions and
merged-interface modules, regenerating from the actual `export module`/`import`
graph is still less error-prone than hand-maintaining ~30 module→module
prerequisite edges, especially since that graph still shifts as `pages` gets
revisited or `layers` gets re-merged. Rerun it after any change to
cross-module `import` edges.

- Stamp contract: `gcm.cache/.carcer-ready` gates the `%.o: %.cpp` rule in the
  top `Makefile`.
- `src/modules/{bmi_objs.list,cppm_sources.list,module_order.txt}` are
  regenerated alongside the makefile; the top `Makefile` reads the first two.
- Caches: `gcm.cache/` (GCC BMIs), `pcm.cache/` (Clang PCMs), `.carcer-bmi/`
  `.sdl2w-bmi/` `.bmin-bmi/` (objects). Never share `gcm.cache` between
  compilers. `make clean` wipes them; the next build rebuilds sdl2w BMIs then
  carcer BMIs before any `.cpp`.
- **wasm for `carcer`'s new module graph is deferred.** `lib/sdl2w`'s
  `build-bmi-em.mk` is the existing reference for the two-phase
  `--precompile` → `.pcm` → `.o` shape; `carcer`'s wasm build graph needs the
  same treatment once wasm work resumes.

### 6a. Known GCC 16 `-fmodules-ts` limits (and how to work around them)

Two distinct failure modes were hit and diagnosed while consolidating. Both are
GCC bugs/limits, not modeling mistakes — expect to hit them again when merging
further, and use the same bisection technique.

1. **GCM corruption ("Bad file data") from too many partitions in one module.**
   Folding ~22 small classes into one partitioned module corrupted the merged
   BMI. Rough ceiling: **~17–20 partitions per module** for this GCC. Fix:
   split the module (e.g. `carcer.ui.components` (17) + `carcer.ui.lists` (5)).
2. **`cc1plus` internal compiler error (segfault) importing an externally
   too-large/complex module** — **not** about the module's own partition
   *count*, physical-merge-vs-partition shape, or total size. Pinned down (via
   the probe-and-bisect technique) to a specific culprit: `LayerSpecialEvent`.
   - First observed as `carcer.layers` (all 15 `LayerX` classes physically
     merged into one TU) compiling standalone but segfaulting `cc1plus` on
     **any external import**, confirmed with a one-line probe file
     (`import carcer.layers;`).
   - Retried as **partitions** instead of a physical merge (one file per
     class, `carcer.layers:LayerX`, 15 partitions of one module) — same
     crash, on the same minimal probe. This ruled out "single giant TU" as
     the cause.
   - Bisected by adding partitions to the primary one group at a time and
     re-running the probe: 13 partitions (everything except
     `LayerSpecialEvent`/`LayerWorld`) import fine; adding `LayerWorld` (14
     partitions) still imports fine; adding `LayerSpecialEvent` instead (14
     partitions, no `LayerWorld`) crashes every time. **`LayerSpecialEvent`
     is the actual cause**, confirmed in isolation, independent of partition
     count or which other classes are present.
   - **Root cause, isolated down to one language construct.** Subtractively
     bisected `LayerSpecialEvent`'s own content (not just which module it
     sits in) by copying it into the crashing partition set and stripping
     pieces, rebuilding + re-probing after each cut. Every "obviously heavy"
     candidate tested clean in isolation — a by-value `in3::SpecialEventRunner`
     member, a `bmin::Map<bmin::String, model::GameEvent>` parameter (itself
     nesting a `bmin::DynArray<std::variant<...>>`), importing *two*
     `carcer.ui.pages.*` modules at once — none of them reproduced the crash
     on their own. What did: `LayerSpecialEvent` is the **only** `LayerX`
     class that declares anything in `namespace ui { ... }` — two small
     observer classes (`ObserverSpecialEventChoice`,
     `ObserverSpecialEventContinue`), reopening `ui::` from inside a
     `carcer.layers` partition, alongside its own `namespace layers { ... }`
     declaration in the same file. Deleting just those two classes (with
     their two call sites) made the crash disappear; replacing them with a
     single trivial `namespace ui { struct Stub {}; }` — nothing else,
     dropping the real members/imports entirely — **reproduced the crash on
     its own**. So the trigger is exactly: *a partition of `carcer.layers`
     contributing a declaration to `namespace ui`*, a namespace that's
     already independently populated by the many separate `carcer.ui.*`
     modules `carcer.layers` imports.
   - This isn't simply "reopening a foreign namespace is unsafe," though —
     the identical trivial stub, in an isolated one-file module unrelated to
     `carcer.layers` (`import carcer.ui.core;` then
     `export namespace ui { struct Stub {}; }`), imports externally with no
     problem at all. The bug needs *both*: a large, many-partition, deeply
     `carcer.ui.*`-connected module (`carcer.layers`, 13+ partitions) *and*
     one of its partitions independently reopening `ui::`. Read as a GCC
     internal limit: merging a namespace's member list across partitions
     *and* across every imported module that also populates it apparently
     has a complexity/recursion cost that only shows up at `carcer.layers`'s
     scale — not a simple "never reopen a namespace from another module"
     rule.
   - This is the same root cause already suspected for the `carcer.ui.pages`
     blocker below (both crash at the same relative point,
     `export import carcer.layers.LayerSpecialEvent;`) — `carcer.ui.pages` is
     itself part of the `ui.*` family already crowding `namespace ui`, so
     it's a plausible second data point for the same mechanism.
   - **Real fix, not a workaround: the two observer classes didn't belong in
     `carcer.layers` in the first place.** `ObserverSpecialEventChoice` /
     `ObserverSpecialEventContinue` are UI event-observer classes (`ui::`),
     and every other UI-domain observer in the codebase already lives in its
     own `carcer.ui.*` module and reaches back into game state only through
     the action bus, never by holding a raw pointer into a concrete `Layer`
     — see `carcer.ui.ObserverRemoveLayer` for the existing precedent. The
     two special-event observers were the one exception: they held a
     `layers::LayerSpecialEvent*` back-pointer and called its methods
     directly, which is both the layering smell that put a `ui::` namespace
     declaration inside `carcer.layers` in the first place *and*, it turned
     out, the literal trigger for the GCC bug above. Moved them to their own
     module, `carcer.ui.ObserverSpecialEvent` (same shape as
     `ObserverRemoveLayer`); they now enqueue `state::actions::
     UiSelectSpecialEventChoice` / `UiContinueSpecialEvent` (broadcast-only
     actions — no `act()` override, `LayerSpecialEvent` is the sole
     subscriber via its existing `subscribeAction<>()`) instead of calling
     back into the layer.
   - With that done, `carcer.layers` no longer has *any* partition
     contributing to `namespace ui` — and the crash is gone for real.
     `carcer.layers` is genuinely one module, **15 partitions, no
     exceptions**: `Layer` and `LayerSpecialEvent` are ordinary partitions
     again, same as every other `LayerX`. Verified with a full
     `make clean && make -j8` and the complete test suite (identical 26/12/43
     baseline).
   - `carcer.ui.pages` (5 classes) still crashes `cc1plus` when merged *and*
     `carcer.layers.LayerSpecialEvent` was reachable from the same umbrella
     — reproduced at the exact same relative point across three retests
     before this fix landed. **Still open** — `pages` stays 5 separate
     modules; hasn't been retried since `LayerSpecialEvent` stopped being a
     separate module (it's just a `carcer.layers` partition now, so the old
     repro shape doesn't directly apply — worth a fresh probe). Given the
     confirmed mechanism above, the concrete next step if revisited: check
     whether any of the 5 `carcer.ui.pages.*` classes (or their would-be
     merged module) declares into a namespace already populated elsewhere in
     its closure, the same way the two special-event observers did.
     Otherwise, file the GCC bug with a minimal repro (`-freport-bug`; the
     trivial `namespace ui { struct Stub {}; }` reproduction above is a good
     starting point for one), or try `-fmodules` (non-TS) / a newer GCC.

**Bisection technique** (fast — seconds, not the 3-4 min full rebuild):
targeted single-object builds via
`make -f modules/make/build-bmi.mk CXX=g++ -j1 .carcer-bmi/<target>.o`, and
for testing whether a *candidate merge* is externally importable before
wiring it into the real tree, a minimal one-line probe `.cppm`
(`import carcer.<candidate>;`) built the same way. Established discipline:
**probe first, wire second** — every module split in Phase 4/5 was
probe-tested for external importability before touching the umbrella or real
consumers, after an earlier costly mistake of wiring first and bisecting a
crash after the fact.

## 7. clangd

Repo-root `.clangd` adds `-Isrc/modules` and the sdl2w/bmin module dirs.
`--experimental-modules-support` belongs in `clangd.arguments`
(`.vscode/settings.json`) **only**, never in `.clangd` `CompileFlags` (clang++
rejects it and module scanning breaks). Regenerate `compile_commands.json` with
`scripts/compile-commands.sh` after module changes, then restart the clangd
server. First open of a module-heavy TU may index for a minute or two.

## 8. Adding / changing code

- **New class in an existing partitioned module** → add a partition
  `export module carcer.<folder>:NewThing;` and one `export import :NewThing;`
  line in `<folder>.cppm`. No new module. Regenerate the graph.
- **New class in a merged-interface module** (`carcer.actions`) → add its
  declaration to the appropriate `export { … }` block and its body either
  inline after `} // export` or in the module's `.cpp` impl unit if it's
  heavy. No new file needed. Regenerate the graph if new cross-module `import`s
  were added.
- **New `LayerX` screen** → add a partition of `carcer.layers`, same as any
  other partitioned module (previous bullet) — own file, own
  `export module carcer.layers:LayerX;`.
- **A UI element/observer needs to react to something a specific `Layer`
  does** → don't hold a pointer to the concrete `Layer` from `ui::` code.
  Enqueue an action (broadcast-only is fine — no `act()` override needed)
  and have the `Layer` `subscribeAction<>()` it, same as
  `carcer.ui.ObserverRemoveLayer` / `carcer.ui.ObserverSpecialEvent`. Besides
  being the established decoupling pattern, a `ui::`-namespace declaration
  living inside a `carcer.layers` partition is exactly what triggered the
  GCC bug in §6a — keep observer classes in `carcer.ui.*`.
- **New folder / subsystem** → new module. Place it in the layering (§2), add
  its cross-module prerequisites, and add one `export import carcer.<new>;` to
  the umbrella.
- **Before merging classes into an existing module, or adding a new
  `export import` to the umbrella**, if the module is already large (~15+
  classes, or heavy classes like the `LayerX` screens): probe-test external
  importability first with a minimal one-line probe file (§6a) — cheaper than
  bisecting a `cc1plus` segfault after wiring it in for real.
- **Changed cross-module `import` edges** → rerun
  `scripts/modules/gen_bmi_makefile.py`.
- **Don't** create a new top-level module for a single type — that's what
  partitions and the `carcer.*.core` modules are for.
- **Converting a folder** to the partition shape:
  `python scripts/modules/partitionize_folder.py --module carcer.<folder> --dir src/<folder>`
  then `python scripts/modules/gen_bmi_makefile.py`, then build native + wasm.
- **Merging a folder's classes into one interface unit** (shape (b), §4) when
  partition count would exceed GCC's ceiling:
  `scripts/modules/consolidate_module.py` — but check every source file for
  **trailing non-exported content** (bodies defined after `} // export`)
  first. The tool's per-file strip logic silently no-ops when a file has that
  shape, producing a corrupted multi-closer merge (caught, at small and large
  scale, by `grep -c "^} // export$" <merged file>` not equaling 1). For a
  folder where every file has that shape (as with `layers/`), hand-roll the
  merge instead: read each source at its git HEAD, locate its own
  `export {`/`} // export` boundary explicitly, and reassemble declarations
  into one block with bodies concatenated after.
