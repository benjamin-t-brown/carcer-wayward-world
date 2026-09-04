# Carcer C++ modules

Carcer ships as C++23 named modules (`carcer.*`). This doc is the contract for
how the tree is organised, how to import across it, and how the build graph works.

> **Migration status (2026-09):** the ~185-module-per-class tree has been
> consolidated to **32 top-level modules** (30 domain modules + the `carcer`
> umbrella + the `carcer.game.map.TileFields` leaf). One holdout remains:
> `carcer.ui.pages` is still 5 separate per-widget modules, blocked by a
> precisely-diagnosed but unresolved GCC bug — see §6a. `scripts/modules/
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
| 8 Screen stack | `carcer.layers`, `carcer.layers.screens`, `carcer.layers.LayerSpecialEvent`, `carcer.layers.LayerWorld` | `LayerManager` (in `carcer.layers`) owns the stack; each `Layer*` binds a UI page + input + its state slice. Split across 4 modules — see §6a. |
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
  - `carcer.layers` vs `carcer.layers.screens` vs `carcer.layers.LayerSpecialEvent`
    vs `carcer.layers.LayerWorld`: conceptually one thing (the screen stack),
    split 4 ways purely because GCC can't compile the merged form — see §6a.
    Revisit as one module if that's ever fixed.
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
`carcer.ui.layouts`, `carcer.ui.minipages`, `carcer.ui.popups`.

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
shape), `carcer.layers`, `carcer.layers.screens`,
`carcer.layers.LayerSpecialEvent`, `carcer.layers.LayerWorld`. This shape was
adopted (rather than per-class partitions) where a folder's class count blew
past GCC's GCM-corruption ceiling (§6a) — merging into fewer, larger
translation units sidesteps that, at the cost of coarser rebuild
invalidation within the module.

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
   too-large/complex module** — independent of the module's own partition
   *count*; driven by total single-TU size/complexity or (for the umbrella)
   the closure shape reached through a specific module. Two confirmed
   instances:
   - `carcer.layers` (all 15 `LayerX` classes physically merged into one TU)
     compiled standalone but segfaulted `cc1plus` on **any external import** —
     confirmed with a one-line probe file (`import carcer.layers;`). Fixed by
     splitting into 4 modules by weight (§2, §3).
   - `carcer.ui.pages` (5 classes) crashes `cc1plus` specifically when merged
     *and* `carcer.layers.LayerSpecialEvent` is reachable from the same
     umbrella — reproduced at the exact same relative point
     (`export import carcer.layers.LayerSpecialEvent;`) across three retests,
     including after the umbrella shrank from ~185 lines to 43. This
     **disproves "total umbrella size"** as the cause; it's a specific closure
     shape through `LayerSpecialEvent`. **Still open** — `pages` stays 5
     separate modules. Next steps if revisited: bisect `LayerSpecialEvent`'s
     own import list/types (candidates: `carcer.in3`,
     `carcer.model.templates`), or file the GCC bug with a minimal repro
     (`-freport-bug`), or try `-fmodules` (non-TS) / a newer GCC.

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
- **New class in a merged-interface module** (`carcer.actions`, `carcer.layers*`)
  → add its declaration to the appropriate `export { … }` block and its body
  either inline after `} // export` or in the module's `.cpp` impl unit if it's
  heavy. No new file needed. Regenerate the graph if new cross-module `import`s
  were added.
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
