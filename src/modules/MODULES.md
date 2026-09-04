# Carcer C++ modules (`carcer.*`)

Carcer ships as C++23 named modules. Only `main.cpp` and `src/__test__/` may use the umbrella:

```cpp
import carcer;
import sdl2w;
import bmin.string_interop;  // bmin::toStringView
#include "macros.h"  // TRANSLATE / LOG after import sdl2w
```

Production implementation units (`.cpp` under `src/`, not tests) must `import` the domain modules they use (`carcer.layers`, `carcer.model.stats.CharacterStats`, …). Do **not** `import carcer;` there — the umbrella `export import`s every domain, so it flattens isolation. Re-narrow with `python scripts/modules/narrow_imports.py`. Do **not** mix classic `#include "bmin/…"` / `#include "sdl2w/…"` headers with module imports in the same binary.

## Layout

Implementation units (`.cpp`) keep unique names next to a **domain** interface (`.cppm`). Example after consolidating layers:

```
src/layers/layers.cppm              # export module carcer.layers
src/layers/Layer.cpp                # module carcer.layers;
src/layers/LayerManager.cpp
src/layers/ui/LayerInventory.cpp
```

Do not recreate a `LayerInventory.cppm` pair. Remaining one-type modules are mostly under `ui/` (layouts, pages, popups). Fold those folder-sized, not all of `carcer.ui.*` at once. `ui/elements` and most of `ui/components` keep method bodies in the `.cppm` after `} // export`. Keep a sibling `.cpp` for `ChCompactInfo` (folding it corrupts GCC GCM when lists nest `DynArray<ChCompactInfoProps>`) and `ListMagicSpells` (observer `onClick` enqueues actions whose `act()` imports `layers`).

`src/modules/` keeps the umbrella and build graph:

| Path | Role |
|---|---|
| `modules/carcer.cppm` | umbrella (`export import` of domains) |
| `modules/macros.h` | TRANSLATE / LOG (`-Imodules`) |
| `modules/make/build-bmi.mk` | ordered BMI compile |
| `modules/bmi_objs.list` | objects linked into `libcarcer.a` |
| `modules/cppm_sources.list` | stamp deps for `gcm.cache/.carcer-ready` |

`carcer.state.core` (StateManager + ActionBus + …) lives at `src/state/core.cppm`. Thin `export import` aliases (`carcer.state.StateManager`, etc.) sit beside it.

## Graph (BMI compile order is bottom-up)

Ordered BMI compile is driven by `src/modules/make/build-bmi.mk` (generated from `export import` edges). Typical domains:

| Module prefix | Responsibility |
|---|---|
| `carcer.lib.*` | Json, StringUtil, helpers |
| `carcer.model.*` | templates, instances, stats, combat model |
| `carcer.db` | Database + loaders (consolidated) |
| `carcer.game.map` | map helpers (consolidated; `TileFields` stays a leaf so `MapInstance` can import it) |
| `carcer.game.combat` | combat helpers (consolidated) |
| `carcer.runner` | special-event scripting (consolidated) |
| `carcer.state.*` | StateManager, ActionBus, WorldUpdater |
| `carcer.actions.*` | one `.cppm` per action (PlaySound shape: class + `act()` in `export { }`). Do not barrel them — UI widgets import only the action they enqueue. |
| `carcer.ui.helpers` | modal layout, keyboard shortcuts, world-action helpers (consolidated; bodies in `.cpp`) |
| `carcer.ui.ObserverRemoveLayer` | shared close-layer observer (`import carcer.actions.ui.UiRemoveLayer`) |
| `carcer.ui.*` | elements, components, layouts, pages (observers live on the owner module) |
| `carcer.layers` | Layer, LayerManager, Layer* (consolidated) |
| `carcer` | umbrella (`export import` of the above) |

Some deep UI **module names** are **shortened** (e.g. `carcer.ui.ListMagicSpells` instead of `carcer.ui.components.lists.ListMagicSpells`) to avoid GCC BMI issues with very deep dotted names. The **files** still live at the deep path (`ui/components/lists/ListMagicSpells.cppm`).

Namespaces (`ui::`, `state::`, `layers::`, …) are unchanged; modules are the shipping boundary.

## Consolidating a domain

Merge remaining one-type modules with:

```bash
python scripts/modules/consolidate_module.py \
    --name carcer.game.map \
    --out src/game/map/map.cppm \
    src/game/map
python scripts/modules/gen_bmi_makefile.py
```

Keep domain BMIs moderate (folder-sized, like sdl2w’s `sdl2w.window` / `sdl2w.draw`). Do not fold all of `carcer.ui.*` into one module in a single step — GCC GCM size is the limiter.

## Deps (sdl2w + bundled bmin)

Build/copy via Makefile:

1. `make -C ../../sdl2w/src native` (ref: `experiment/cpp-modules`)
2. `../../sdl2w/copy-sdl2w-artifacts.sh lib/sdl2w`
3. `include lib/sdl2w/modules/make/use.mk`
4. Local BMI stamps: `sdl2w-bmi` then `carcer-bmi` → `gcm.cache/.carcer-ready`

Link with `$(SDL2W_LDLIBS)` (`libsdl2w_modules.a` + `libbmin_modules.a`), plus `modules/bmi_objs.list` object files.

## Caches

```
gcm.cache/   # GCC module BMIs (native)
pcm.cache/   # em++ PCMs when TARGET=wasm
.bmin-bmi/ .sdl2w-bmi/ .carcer-bmi/
```

`make clean` removes these. Never share `gcm.cache` between GCC and Clang/em++. After wiping caches, rebuild BMIs with `make carcer-bmi` (`-j1` for BMI ordering).

## GCC BMI pitfalls

- Nested `bmin::DynArray` inside a struct that itself lives in a `DynArray` on a `UiElement`-derived type can corrupt GCMs (`Bad file data`). Prefer `std::vector` for that nested leaf, or flatten the shape.
- Keep global module fragments minimal (only headers that TU needs).

## Regenerating the graph

After adding/removing an interface unit, or changing `import` edges:

```bash
python scripts/modules/gen_bmi_makefile.py
```

Do **not** re-run `convert_to_modules.py` for day-to-day work.
