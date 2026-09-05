# Carcer C++ modules

Carcer ships as C++23 named modules (`carcer.*`). This doc is the contract for
how the tree is organised, how to import across it, and how the build graph works.

> **Migration status (2026-09):** the ~185-module-per-class tree has been
> consolidated to **33 top-level modules**. Every domain is one-class-per-file
> now — `carcer.actions` was the last holdout (used to be 4 partitions each
> cramming 16–33 unrelated classes into one file) and is now a thin re-export
> of 5 real modules (`carcer.actions.{combat,world,general,ui,ui.layers}`),
> each with one partition per class; see §3 and §4. `carcer.layers` and
> `carcer.ui.pages` are fully unified with no exceptions; see §6a for the two
> GCC bugs that blocked them and how each was actually resolved (not worked
> around). `scripts/modules/partitionize_folder.py` and
> `consolidate_module.py` do the mechanical conversion for any future folder
> consolidation; `scripts/modules/gen_bmi_makefile.py` regenerates the build
> graph and is a permanent tool (see §6), not a migration one-shot.

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
modules** — 30 of them, plus two narrow cycle-breaking modules and the
umbrella — like
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
| 5 State kernel | `carcer.state` | store + `ActionBus` + `AbstractAction` base + interface seams + `LayerRequest`/`layerStack`. Small, stable, universally depended on. |
| 6 Actions / orchestration | `carcer.actions` (pure re-export of `carcer.actions.{combat,world,general,ui,ui.layers}`), internal `carcer.actions.world_effects`, and `carcer.world_updater` | one command class per state transition, one file per class; `act()` mutates state, calls rules, enqueues timed follow-ups. The two narrow modules keep shared deferred world effects and the frame updater above `state` but below their consumers, avoiding reverse imports into a module's own purview. |
| 7 UI widgets | `carcer.ui.core` → `carcer.ui.elements` → `carcer.ui.components` → `carcer.ui.layouts` → `carcer.ui.{minipages,popups,pages}` (+ `carcer.ui.helpers`, `carcer.ui.lists`, `carcer.ui.KeyboardHeldScroll`, `carcer.ui.ObserverRemoveLayer`, `carcer.ui.ObserverSpecialEvent`) | framework → primitives → game-aware composites → screens. Read model/state to render; enqueue actions on interaction. |
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
  ship as a static lib with a documented surface. 29 domain modules total.
- Two pieces of code go in **separate modules** when they differ in *change
  frequency* or *dependency footprint*, even if related — a module is the unit
  of rebuild invalidation. They share a module (as partitions) when they change
  together and share downstream deps.
  - `carcer.state` vs `carcer.actions`: the kernel is small, stable, and
    depended on by everything; actions are numerous, churny, and drag in
    `game` + `model`. Separate.
  - `carcer.in3` vs `carcer.game.*`: narrower footprint (`lib` + `model` only),
    different cadence (a scripting VM, not spatial/combat mechanics). Separate.
  - `carcer.actions.combat` / `.world` / `.general` / `.ui` / `.ui.layers`:
    conceptually one thing (`carcer.actions`, which is what every consumer
    still imports — it's a pure re-export of these 5). Split into 5 real
    modules purely because 71 classes as one module's partitions would blow
    well past the ~17–20 GCM-corruption ceiling (§6a); the split follows real
    seams anyway (`ui.layers`'s classes are pure `pushLayerRequest` calls,
    a genuinely different shape from `ui`'s state-mutating ones) rather than
    being arbitrary. `world` and `combat` are otherwise as separate as two
    related domains can be: neither needs the other at the *declaration*
    level (a shared `CombatAction` base that used to force this was removed
    — see §6b, it added no actual behavior over `AbstractAction`). The one
    remaining link is narrow: both domains use the lower-level
    `carcer.actions.world_effects` module for queueable particles,
    projectiles, and action-mode changes. `PerformTownMeleeAttack` (`world`)
    still imports one `combat` sprite-effect partition, but combat no longer
    imports world, so the graph stays acyclic (see §4).
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

Every module in the tree is **partitions — one file per class**, declaration
plus (usually) inline bodies:

```
src/<folder>/_<folder>.cppm    export module carcer.<folder>;          (primary interface unit / aggregator — the "barrel" file)
                               export import :Thing;
                               export import :Other;
src/<folder>/Thing.cppm        export module carcer.<folder>:Thing;    (partition: declaration + inline bodies)
                               import :Other;                          (sibling partition)
                               import carcer.model.instances;          (cross-module: full name)
src/<folder>/Thing.cpp         module carcer.<folder>;                 (impl unit: bodies for any partition)
```

**Every "look here first for this folder" file is prefixed `_`** — not just
partition-aggregating barrels (`_elements.cppm`, `_layers.cppm`,
`modules/_carcer.cppm`) but any single `.cppm` that holds a whole module's
content directly, no partitions at all (`state/_State.cppm`, `in3/_in3.cppm`,
`game/combat/_combat.cppm`, `game/map/_map.cppm`, `db/_db.cppm`,
`ui/helpers/_helpers.cppm`, `lib/hiscore/_hiscore.cppm`,
`actions/general/_general.cppm`). The test: does this file's name just
repeat its containing folder's name, such that in a listing of several
files you'd have to know that convention to find it? If yes, prefix it.
The leading underscore has no meaning to the compiler and isn't part of the
module name (the file is still `export module carcer.ui.elements;` inside,
unrelated to what it's called on disk) — it's purely so the file sorts
first and is easy to spot. `partitionize_folder.py` defaults new primaries
to this name; pass `--primary <name>` to override.

**Not every root-level `.cppm` needs it.** A handful of modules are
standalone leaves that happen to share a directory with other, unrelated
modules — `game/map/TileFields.cppm` (sibling to `_map.cppm`, its own
module), `lib/Json.cppm` / `lib/StringUtil.cppm` (no unifying `carcer.lib`
folder-module to be "the" file for), `ui/KeyboardHeldScroll.cppm` /
`ui/ObserverRemoveLayer.cppm` / `ui/ObserverSpecialEvent.cppm` (standalone
modules living directly under `ui/`, distinct from its real barrel,
`_core.cppm`). These already have specific, self-explanatory names — there's
no "which file is the one for this folder" ambiguity to resolve, so leave
them alone.

This is universal now — `carcer.model.templates`, `carcer.model.instances`,
`carcer.ui.core`, `carcer.ui.elements`, `carcer.ui.components`,
`carcer.ui.lists`, `carcer.ui.layouts`, `carcer.ui.minipages`,
`carcer.ui.popups`, `carcer.ui.pages`, `carcer.layers`, and all 5
`carcer.actions.*` modules. A folder that used to be one module with 20–70
classes crammed into a single merged `export { … }` block (`carcer.actions`
before it was split, `carcer.layers`'s old `screens.cppm`) is always a sign
that folder needs splitting into more partitions or, past the GCM ceiling
(§6a), more modules — not a shape to reach for on purpose.

A partition's body can be:
- **Inline**, in the same file as the declaration (the common case).
- **Out-of-line but still in the same file, after `} // export`** (still
  non-exported / module-linkage-only) — used where a class's declaration and
  definition are naturally kept apart for readability (e.g. every `LayerX` in
  `carcer.layers` — see the file-format note in §2/§6a).
- **In a sibling `.cpp` implementation unit**, when a body needs something its
  own partition's declaration-time position in the build order can't see. An
  implementation unit implicitly imports its module's primary interface unit,
  so it sees every sibling partition regardless of declared order. The whole
  module graph must still be acyclic: GCC rejects importing a module that
  depends on the implementation unit's own module as "cannot import module in
  its own purview." Put genuinely shared deferred types in a lower-level module
  instead; `carcer.actions.world_effects` is the concrete example used by both
  combat and world actions. Most classes never need an implementation unit.
- Two bodies **must** stay in a `.cpp` regardless of anything else —
  `ChCompactInfo`, `ListMagicSpells` — their nested `bmin::DynArray` shapes
  corrupt GCC GCMs when inline.

Partition names are **flat** — `carcer.ui.elements:Quad`, never dotted after
the `:` (GCC BMI stability). The primary unit of a partitioned module only
`export import`s partitions — no code of its own beyond what's needed to
curate the surface.
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
module count dropped to ~20; at 29 domain modules, all partitioned,
regenerating from the actual `export module`/`import` graph is still less
error-prone than hand-maintaining ~29 module→module prerequisite edges by
hand, especially since partition-to-partition ordering (e.g.
`carcer.ui.pages:PageModalEvent` needing `:PageTalkChoice` built first, or
`carcer.actions.combat`'s `DoCombatAction` needing five sibling partitions built first)
is exactly the kind of edge that's easy to get wrong manually. Rerun it after
any change to cross-module `import` edges.

- Stamp contract: `gcm.cache/.carcer-ready` (`$(CARCER_BMI_STAMP)`) gates
  whether the BMI submake needs to run at all; it does **not** gate individual
  `.cpp` recompiles directly (see below).
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

### 6c. Per-file `.cpp` → BMI dependencies, and a real GNU Make scheduling gap

Every `.cpp` implementation unit used to depend on the single coarse
`$(CARCER_BMI_STAMP)` file, so touching *any one* `.cppm` anywhere forced
*every* `.cpp` impl unit to recompile — correct, but far coarser than
necessary. `gen_bmi_makefile.py` now also emits
`src/modules/cpp_bmi_deps.mk`: for each `.cpp`, the specific
`.carcer-bmi/*.o` objects it (or its own module's primary, which covers every
sibling partition) actually imports, as prerequisite-only lines with no
recipe — GNU Make unions these with the top Makefile's `%.o: %.cpp` pattern
rule. `%.o: %.cpp` itself now takes `carcer-bmi` only as an *order-only*
prerequisite (a `.carcer-bmi/%.o: | carcer-bmi` placeholder rule gives the top
Makefile something to point at, since the real files are only ever produced by
the recursive `build-bmi.mk` submake). Net effect: touching one leaf module
now recompiles only the `.cpp` files that actually import it, not all of them.

**Real bug found and fixed while wiring this up, not just a design nuance:**
with a parallel (`-j>1`) build, a single `make` process can "consider" a
`.cpp` target's `.carcer-bmi/*.o` prerequisite *before* a sibling target's
dependency chain finishes rebuilding that same file as a recursive-submake
side effect, and then use the pre-rebuild mtime for the first target's
freshness check — confirmed directly with `make --debug=v` (a target's own
"Finished prerequisites" trace reported a `.carcer-bmi/*.o` file as "older"
immediately after a `--debug=v` line elsewhere in the *same run* showed that
exact file being "Successfully remade"). This is a real GNU Make scheduling
gap around a file that one target's order-only prerequisite recipe produces
as a side effect while a *different* target references it as a normal
prerequisite — not a misreading of the order-only docs, and not fixable by
rearranging prerequisite order within one `make` process. The fix: `all` and
`libcarcer`/`object_files` (the only real entry points — `object_files` is
what every `run-*-tests-ucrt64.sh` script calls) now run `carcer-bmi` as its
**own, already-exited `$(MAKE)` invocation** before recursing into a second,
completely fresh `$(MAKE)` process to compile/link. The second process stats
every BMI object for the first time (already final, never changing again
during its own lifetime), so the race has no window to occur in. Verified
stable across repeated leaf-touch rebuilds at both `-j8` and `-j16`, through
both entry points.

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
   - This was also the root cause behind the `carcer.ui.pages` blocker
     described below — both crashed at the same relative point,
     `export import carcer.layers.LayerSpecialEvent;`, and both were
     resolved by the same fix (confirmed once `LayerSpecialEvent` stopped
     reopening `namespace ui`).
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
   - **`carcer.ui.pages` was never the real problem — resolved.** It used to
     crash `cc1plus` specifically when merged *and*
     `carcer.layers.LayerSpecialEvent` was reachable from the same umbrella,
     reproduced at the same relative point across three retests. Once
     `LayerSpecialEvent` stopped independently reopening `namespace ui`
     (previous bullet), that repro shape no longer applied — retried merging
     `pages` into partitions (`carcer.ui.pages`, 5 partitions, one file per
     class) with the probe-first discipline, and it built and imported
     externally clean on the first try. This confirms `pages` itself was
     never the culprit; it just happened to sit next to `LayerSpecialEvent`
     in the umbrella's failure mode.
   - Wiring it into the full tree *did* surface one real, separate bug: a
     dead import in `MinipageEquipRunes.cppm`
     (`import carcer.ui.pages.PageCharacter;`, confirmed unused — zero
     `ui::Page*` references in the file, first flagged as dead back when
     `pages` was still 5 modules and never cleaned up). `partitionize_folder.py`'s
     repo-wide rewrite mechanically widened it to `import carcer.ui.pages;`
     (the new merged module name), and pulling in the *entire* merged pages
     closure from `carcer.ui.minipages`, itself imported deep inside the
     large `carcer.layers` (via `LayerEquipRunes`), was enough on its own to
     retrigger a `cc1plus` segfault — a third data point for the same class
     of GCC limit: an unnecessarily wide import between two already-large
     modules. Deleting the dead import (it cost nothing — it was never used)
     fixed it. **Lesson:** a dead import that's cheap while pointing at a
     single small module can become a real problem once the module on the
     other end gets consolidated — worth clearing dead imports *before*
     merging the module they point at, not just when they're first noticed.
   - Verified: `make clean && make -j8` green, full test suite identical
     26/12/43. `carcer.ui.pages` is now one module, 5 partitions, no
     exceptions — every page has its own file, matching every other UI
     domain.

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

### 6b. `carcer.actions`: telling fake cross-domain coupling from real

`carcer.actions` used to be 4 partitions (`:combat`/`:world`/`:ui`/`:general`)
each merging 16–33 unrelated classes into one file, specifically because 6 of
those 71 classes' bodies needed a class from a different domain that their
own partition's position in the build order couldn't see yet — the standard
fix for that is a `.cpp` implementation unit (§4), but 6 classes needing one
was also the reason the domains couldn't be split into their own top-level
modules or given per-class partitions without exceeding the GCM ceiling
(§6a). Splitting those 71 classes one-per-file (into
`carcer.actions.{combat,world,general,ui,ui.layers}`) meant looking hard at
each of the 6 first, because most of them turned out not to need the
cross-domain reference at all:

- **4 were fake coupling.** `EndCombat`, `SetActiveCombatCharacter`
  (`combat`), `WorldExamineAt` (`world`), and `UiSelectSpellCast` (`ui`) each
  constructed a sibling-domain action object purely to call `.execute(state)`
  on it once, synchronously, right there — no deferral, no polymorphism
  actually exercised. Two of the four target actions
  (`WorldSetCamera`, the `UiShowLayerPickUp`-style `LayerRequest` push) were
  trivial one-line state setters and got inlined directly, dropping the
  cross-domain reference entirely. The third, `WorldSetActionMode`, has real
  ~40 lines of branching logic (aim-tile targeting via
  `game::findPartyAvatarOnActiveMap`) that two call sites needed without
  duplicating it — extracted to a free function, `game::resolveWorldActionMode()`,
  in `carcer.game.map` (a layer below `carcer.actions` that both `world` and
  `ui` already import), so both call it directly with no module dependency
  between them at all.
- **2 are genuine.** `PerformMeleeAttack` and `PerformSpellCast` (`combat`)
  use `insertAction(new WorldSpawnDamageParticle(...), delayMs)` — real
  deferred, timed follow-ups queued through the same polymorphic `ActionBus`
  dispatch everything else uses. That fundamentally needs the concrete
  action class to exist as a heap-allocated, queueable object; there is no way
  to defer a timed effect without it. The shared `WorldSpawnDamageParticle`,
  `WorldSpawnProjectile`, and `WorldSetActionMode` types therefore live in
  `carcer.actions.world_effects`, below both action domains. `combat.cpp`
  imports that module, while the original world partitions re-export it to
  preserve the public `carcer.actions.world` API.

**The test to apply when a body seems to need a cross-domain type:** does it
construct that type and call `.execute()` on it immediately, in the same
statement or nearly so? If yes, look at what that type's own `act()` actually
does — if it's a small, self-contained mutation, it's very likely cheaper and
clearer to inline that mutation (or, if two+ places need the exact same
logic, extract a free function in whatever layer already sits below both)
than to carry the cross-domain reference. Reach for a real `.cpp` impl unit
only when the need is genuinely deferred/queued, or otherwise can't be
satisfied by reading — not just constructing and immediately calling — the
other domain's public surface.

**A base class can be fake coupling too, not just a body reference.**
`carcer.actions.world`'s `WorldMovePlayer`, `TownEnemyAiAfterPlayerMove`,
`TownEnemySeekAndMelee`, and `PerformTownMeleeAttack` all derived from a
`CombatAction` class declared in `carcer.actions.combat` — the *entire*
reason `world`'s interface needed `combat`'s interface. `CombatAction`
turned out to provide zero actual behavior: its only members
(`insertAction()`/`enqueueAction()` wrappers) were commented out and already
duplicated on `AbstractAction` itself, and nothing anywhere used
`CombatAction` polymorphically (no `dynamic_cast`, no `CombatAction*`
container, no `typeid` check) — it was a pure compile-time tag, a fossil
from before those two methods got hoisted onto the shared base. Deleted it;
every former subclass now derives from `AbstractAction` directly. Same
underlying lesson as the `.execute()` case above, one level up the
hierarchy: before treating a shared base class as a reason two domains must
depend on each other, check whether it actually contributes anything a
plain `AbstractAction` doesn't.

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
  line in `_<folder>.cppm` (the barrel). No new module. Regenerate the graph.
- **New action class** → decide which of `carcer.actions.{combat,world,general,ui,ui.layers}`
  it belongs to by what its `act()` does (a screen open/close request →
  `ui.layers`; anything else → its domain), then add a partition there, same
  as the first bullet. `carcer.actions` (what everything else imports)
  re-exports all 5 already — no umbrella edit needed. If a body needs a class
  from a *different* `carcer.actions.*` domain and it's a real, deferred/
  queued need (not just "call one setter and return" — inline that instead,
  see §6a), put that one body in the module's `.cpp` impl unit.
- **New `LayerX` screen** → add a partition of `carcer.layers`, same as any
  other partitioned module (first bullet) — own file, own
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
- **Before merging module `B` into a folder that other code already imports
  narrowly from** — e.g. `import carcer.B.SomeLeaf;` — grep for dead imports
  of `B`'s pieces first (`grep -rn "import carcer.B\." src`, then check each
  hit is actually used). `partitionize_folder.py`'s repo-wide rewrite
  mechanically widens `import carcer.B.SomeLeaf;` to `import carcer.B;` once
  merged; a dead import that cost nothing pointed at a single small leaf can
  become a real `cc1plus`-crash risk once it points at the whole merged
  module instead (§6a, the `MinipageEquipRunes` case).
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
