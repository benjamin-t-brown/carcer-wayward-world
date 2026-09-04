# Carcer C++ modules

Carcer ships as C++23 named modules (`carcer.*`). This doc is the contract for
how the tree is organised, how to import across it, and how the build graph works.

> **Migration status (2026-09):** consolidating ~185 one-class modules down to
> ~20 folder modules. Done: `carcer.ui.elements` (pilot). The rest still follow
> the old one-module-per-file shape until their folder is converted. The rules
> below describe the target; `scripts/modules/partitionize_folder.py` does the
> mechanical conversion, `scripts/modules/gen_bmi_makefile.py` regenerates the
> build graph.

## 1. Why modules here

Modules replaced headers for two payoffs:

- **Isolation.** No include-order fragility, no macro leakage, and names that
  aren't `export`ed have module linkage — genuinely invisible outside, so no ODR
  landmines.
- **Build speed.** An interface is parsed once into a BMI; importers load the
  BMI instead of re-parsing text.

Both payoffs need a **shallow, wide** dependency graph. 185 micro-modules in a
deep chain give neither (the graph is as deep as the old `#include` graph, and
`import carcer;` everywhere drags the whole closure). Hence: **folder-sized
modules**, ~20 of them, like `sdl2w.window` / `sdl2w.draw`.

## 2. The layering

Dependencies point **down** this list. An arrow pointing up is a design smell —
fix it by moving the shared type down (usually into `carcer.model.templates` or
`carcer.ui.core`) or by routing through an interface seam:
`StateManagerInterface`, `DatabaseInterface`, `LayerManagerInterface`.

| Layer | Modules | Role |
|---|---|---|
| 0 Foundations | `carcer.lib.*` (+ external `sdl2w`, `bmin`) | pure utilities: json, strings, hiscore. No game knowledge. |
| 1 Static data | `carcer.model.templates`, `carcer.game.map.TileFields` (leaf) | immutable definitions mirrored from `assets/db`. |
| 2 Data access | `carcer.db` | loads templates, owns lookup registries. |
| 3 Runtime model | `carcer.model.instances` (+ `Combat`) | the mutable store shape: live characters, maps, items, world, combat state. |
| 4 Rules | `carcer.game.map`, `carcer.game.combat`, `carcer.in3` | pure-ish logic over the model; compute results, don't own state. Independent siblings. |
| 5 State kernel | `carcer.state` | store + `ActionBus` + `AbstractAction` base + `WorldUpdater` + interface seams. Small, stable, universally depended on. |
| 6 Actions | `carcer.actions` | one command class per state transition; `act()` mutates state, calls rules, enqueues timed follow-ups. |
| 7 UI widgets | `carcer.ui.core` → `carcer.ui.elements` → `carcer.ui.components` → `carcer.ui.layouts` → `carcer.ui.{minipages,popups,pages}` (+ `carcer.ui.helpers`) | framework → primitives → game-aware composites → screens. Read model/state to render; enqueue actions on interaction. |
| 8 Screen stack | `carcer.layers` | `LayerManager` owns the stack; each `Layer*` binds a UI page + input + its state slice. |
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

## 3. What is (and isn't) a module

- **A module is a directory of cohesive code** — roughly, something you could
  ship as a static lib with a documented surface. Target ~20 total.
- Two pieces of code go in **separate modules** when they differ in *change
  frequency* or *dependency footprint*, even if related — a module is the unit
  of rebuild invalidation. They share a module (as partitions) when they change
  together and share downstream deps.
  - `carcer.state` vs `carcer.actions`: the kernel is small, stable, and
    depended on by everything; actions are numerous, churny, and drag in
    `game` + `model`. Separate.
  - `carcer.in3` vs `carcer.game.*`: narrower footprint (`lib` + `model` only),
    different cadence (a scripting VM, not spatial/combat mechanics). Separate.
- If two folders turn out to be mutually dependent at folder granularity (e.g.
  an element imports a button while a button imports an element), that's **one
  module**, not two with a forced layer. (`carcer.ui.elements` absorbs its
  `buttons/` subfolder for exactly this reason.)

## 4. File layout & partition mechanics

```
src/<folder>/<folder>.cppm     export module carcer.<folder>;          (primary interface unit / aggregator)
                               export import :Thing;
                               export import :Other;
src/<folder>/Thing.cppm        export module carcer.<folder>:Thing;    (partition: declaration + inline bodies)
                               import :Other;                          (sibling partition)
                               import carcer.model.instances;          (cross-module: full name)
src/<folder>/Thing.cpp         module carcer.<folder>;                 (impl unit: bodies for any partition)
```

- **Partition names are flat** — `carcer.ui.elements:Quad`, never dotted after
  the `:` (GCC BMI stability). The name is the last dotted segment of the file's
  old module name, so `carcer.ui.ButtonClose` → partition `:ButtonClose`.
- The primary unit only `export import`s partitions — no code of its own beyond
  what's needed to curate the surface.
- **Bodies:** inline in the partition `.cppm` (nicer to author, but every edit
  invalidates the module BMI) **or** in a sibling `.cpp` implementation unit
  (`module carcer.<folder>;` — a body edit costs one object file, nothing
  downstream). Use the `.cpp` when a file is a build hotspot or its bodies pull
  heavy deps you don't want in the interface. An implementation unit implicitly
  imports the primary interface unit, so it sees every partition without
  importing them.
- Two bodies **must** stay in `.cpp` — `ChCompactInfo`, `ListMagicSpells` —
  their nested `bmin::DynArray` shapes corrupt GCC GCMs when inline.
- Namespaces (`ui::`, `state::`, …) are unchanged by any of this. Modules are the
  shipping boundary; namespaces are the naming one.

## 5. Import rules

- **Production `.cpp` / `.cppm` import the specific domain modules they use**
  (`import carcer.model.instances;`, `import carcer.ui.elements;`). Do **not**
  `import carcer;` outside `main.cpp` and `src/__test__/` — the umbrella
  `export import`s every domain and flattens all isolation.
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
| driven by | `src/modules/make/build-bmi.mk` | `src/modules/make/build-bmi-em.mk` |

Both are generated by `scripts/modules/gen_bmi_makefile.py` from the
`export module` / `import` / `import :part` edges in the `.cppm` files. Ordering
rules: a module's partitions compile before its primary interface unit; a module
compiles before anything that imports it; `carcer` (umbrella) is last. The BMI
step runs `-j1`; `.cpp` implementation units and the final link are parallel.

- Stamp contract: `gcm.cache/.carcer-ready` gates the `%.o: %.cpp` rule in the
  top `Makefile`.
- `src/modules/{bmi_objs.list,cppm_sources.list,module_order.txt}` are
  regenerated alongside the makefile; the top `Makefile` reads the first two.
- Caches: `gcm.cache/` (GCC BMIs), `pcm.cache/` (Clang PCMs), `.carcer-bmi/`
  `.sdl2w-bmi/` `.bmin-bmi/` (objects). Never share `gcm.cache` between
  compilers. `make clean` wipes them; the next build rebuilds sdl2w BMIs then
  carcer BMIs before any `.cpp`.

## 7. clangd

Repo-root `.clangd` adds `-Isrc/modules` and the sdl2w/bmin module dirs.
`--experimental-modules-support` belongs in `clangd.arguments`
(`.vscode/settings.json`) **only**, never in `.clangd` `CompileFlags` (clang++
rejects it and module scanning breaks). Regenerate `compile_commands.json` with
`scripts/compile-commands.sh` after module changes, then restart the clangd
server. First open of a module-heavy TU may index for a minute or two.

## 8. Adding / changing code

- **New class in an existing folder** → add a partition
  `export module carcer.<folder>:NewThing;` and one `export import :NewThing;`
  line in `<folder>.cppm`. No new module. Regenerate the graph.
- **New folder / subsystem** → new module. Place it in the layering (§2), add
  its cross-module prerequisites, and add one `export import carcer.<new>;` to
  the umbrella.
- **Changed cross-module `import` edges** → rerun
  `scripts/modules/gen_bmi_makefile.py`.
- **Don't** create a new top-level module for a single type — that's what
  partitions and the `carcer.*.core` modules are for.
- **Converting a folder** to the partition shape:
  `python scripts/modules/partitionize_folder.py --module carcer.<folder> --dir src/<folder>`
  then `python scripts/modules/gen_bmi_makefile.py`, then build native + wasm.
