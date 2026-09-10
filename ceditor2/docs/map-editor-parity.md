# CEditor2 map editor parity and performance baseline

> Implementation disposition: see
> [`phase-6-summary.md`](./phase-6-summary.md) and
> [`legacy-differences.md`](./legacy-differences.md). This Phase 0 inventory
> intentionally remains unchanged so unchecked legacy interactions are not
> mistaken for retained requirements.

## Purpose

This document is the Phase 0 behavioral inventory for the existing CEditor map
and map-grid editors. It is a migration checklist, not a prescription to copy
their React structure. CEditor2 should preserve useful behavior and stored data
while replacing the implementation with explicit TypeScript controllers,
Canvas 2D rendering, native DOM controls, and editor-local CSS.

The audit covers:

- `ceditor/src/client/pages/Maps.tsx`
- `ceditor/src/client/pages/MapGrids.tsx`
- `ceditor/src/client/tile-editor/**`
- the map, grid, tile, and terrain dialogs under
  `ceditor/src/client/components/`
- `ceditor/src/client/utils/mapIndex.ts`, `mapGridIndex.ts`,
  `mapTabsStorage.ts`, and `mapPreview.ts`
- the current assets in `src/assets/db/maps.json`, `map-grids.json`, and
  `tilesets.json`

Checkboxes are acceptance items for CEditor2. Notes beginning with **Legacy
behavior** describe observable behavior that should be consciously accepted,
improved, or rejected rather than accidentally lost.

## Stored data contract

The current `maps.json` format stores compact data rather than one full object
per tile:

- `name`, `label`, `type`, `width`, `height`, `spriteWidth`, and
  `spriteHeight`
- a map-local `tilesets` name dictionary whose index 0 is the empty string
- numeric `layers`
- one dense, row-major graphic array per layer, stored as repeated
  `[tilesetIndex, tileId]` pairs
- sparse placement arrays for characters, items, markers, event triggers,
  travel triggers, tile overrides, and light sources; each placement has a
  layer `l` and row-major tile index `i`

The editor materializes that representation into `CarcerMapTileTemplate[]` for
interactive editing, then writes the graphic and sparse placement arrays back.
CEditor2 must preserve this storage format unless the game runtime is migrated
at the same time.

`map-grids.json` stores named rectangular grids, required per-map tile
dimensions, and a row-major two-dimensional array of map names. Empty strings
are unassigned cells.

## Feature parity checklist

### Map lifecycle

- [ ] List/open maps by name and show label/type/dimensions where useful.
- [ ] Create a map with required, trimmed unique name; required trimmed label;
      `TOWN` or `OUTDOOR` type; and positive integer width and height.
- [ ] Initialize a new map with layer 0, a correctly sized dense graphic
      array, an empty-string tileset dictionary entry, and empty sparse
      placement lists.
- [ ] Edit name, label, type, width, and height in a map-properties dialog.
- [ ] Resize every layer while preserving cells whose old coordinates remain
      in bounds and dropping out-of-bounds sparse placements.
- [ ] Duplicate a map by deep copy, place the copy after the source, generate a
      unique `_copy`, `_copy2`, ... name, append `(Copy)` to its label, leave it
      outside any grid, open it, and allow properties to be edited before save.
- [ ] Delete a map only after confirmation and close/update any UI tab that
      points to it.
- [ ] Keep create/edit/delete/duplicate changes in the database session until
      Save All, except where a cross-file transaction explicitly saves the
      complete database.
- [ ] Reject missing name, missing label, invalid type, non-positive
      dimensions, and duplicate map names before persistence.
- [ ] Preserve sprite dimensions and unknown supported fields through
      create/edit/resize/duplicate operations.
- [ ] Provide an empty state when no map is open.

**Legacy behavior:** standalone map creation checks duplicate names only when
`existingMapNames` is passed. The ordinary `+ New Map` call does not currently
pass it, so duplicates can be created and are caught only by Save. CEditor2
should reject duplicates at creation and again at Save All.

**Legacy behavior:** renaming a map updates map-grid cells, but does not update
travel-trigger destination map names. Deleting a map does not clear grid cells
or travel destinations. CEditor2 should treat both as cross-database reference
operations (see References).

### Navigation, tabs, and map grids

- [ ] Open a map from a one-shot searchable/select control.
- [ ] Open a grid from a one-shot control, choosing its first existing assigned
      map; report a useful error when the grid has no valid assigned maps.
- [ ] Use one tab per grid, with the tab's active map changing as the user moves
      through that grid; gridless maps each get their own tab.
- [ ] Show both the grid identity (or “no grid”) and active map name on a tab.
- [ ] Activate and close tabs without losing in-memory map changes.
- [ ] Persist open tabs and the active tab locally, restore them before the
      editor first paints, discard missing maps, rebind maps whose grid changed,
      collapse duplicate grid tabs, and tolerate malformed/private-mode
      storage.
- [ ] Accept a direct URL containing a map name, open/select it, then normalize
      the URL so refresh does not repeatedly consume the request.
- [ ] Preserve a separate pan/zoom viewport per map during ordinary tab
      switching.
- [ ] Draw nearby assigned maps in their stitched grid positions around the
      focused map.
- [ ] Draw immediate empty grid slots as `+` targets and assigned slots as
      labeled `Open` targets.
- [ ] Clicking an assigned slot navigates to that map without a visible camera
      jump; clicking an immediate empty slot begins constrained map creation.
- [ ] Constrain grid-created maps to the grid's required dimensions, suggest a
      unique name/label derived from the active map, seed the union of layers
      used by the grid, assign the new map to the requested cell, and focus it.
- [ ] Support a configurable render radius for surrounding context and an
      editable-neighbor radius; maps outside the editable radius are dimmed and
      read-only.
- [ ] Provide an “Edit whole grid” switch. When enabled, allow draw, terrain,
      brush, and selection interactions on editable neighboring blocks without
      first changing maps.
- [ ] Permit a draw/terrain/rectangle brush stroke to cross adjacent assigned
      map boundaries and ignore cells outside the grid or in empty/missing
      slots.
- [ ] Maintain grid-wide undo order so undo reverses the latest stroke even
      when it affected a neighboring map.
- [ ] Link from a map to every grid containing it and open the selected grid in
      its own browser tab.

Map-grid management:

- [ ] Search grids by case-insensitive name or label.
- [ ] Create, clone, delete with confirmation, and edit grids.
- [ ] Require unique non-empty grid names; trim strings, normalize positive
      integer dimensions, and sort grids by name when saving.
- [ ] Edit grid name/label, number of slots wide/tall, and required map width
      and height.
- [ ] Resize the cell matrix while preserving the upper-left overlapping area;
      make truncation clear to the user when assignments will be lost.
- [ ] Shift all assignments by integer X/Y cell offsets, dropping assignments
      that leave bounds.
- [ ] Display assigned/total slot counts and a preview for each assigned map.
- [ ] Assign cells through a map picker filtered to exactly matching map
      dimensions, with town/outdoor filtering available.
- [ ] Mark references to missing maps, and allow those cells to be cleared.
- [ ] Open an assigned map in the map editor from its grid cell.
- [ ] Clear an assignment without deleting the map.
- [ ] Restore the selected grid from local selection state or a direct
      `mapGrid` URL parameter.

**Legacy behavior:** a map can appear in more than one grid, but most map
editor behavior uses only the first placement. CEditor2 must either enforce one
grid per map or make the active grid explicit; it must not silently depend on
array order.

### Canvas rendering and interaction

- [ ] Resize the canvas to its container/window, keep pixel-art smoothing off,
      and render sprites with pixel-aligned pan offsets.
- [ ] Run one straightforward `requestAnimationFrame` loop while the map app is
      mounted. Continuous rendering is an accepted CEditor2 design choice;
      dirty-frame scheduling is not a parity requirement.
- [ ] Catch/report a frame failure without permanently stopping subsequent
      frames.
- [ ] Render only the current numeric layer for the focused map and surrounding
      grid maps.
- [ ] Cull tile drawing to the visible tile rectangle for every rendered map
      block.
- [ ] Render, in order, base tile graphic, character sprites, item sprites (or
      one container indicator), metadata control indicators, and configured
      event/travel overlay sprites.
- [ ] Draw a distinct hover outline, selected-tile outline, active tool preview,
      and optional tile grid.
- [ ] Show contextual character names, marker names, and event IDs while Tab is
      held, without permanently altering the document.
- [ ] Show the correct cursor for pan, select-drag, clone, and grid-slot
      navigation.
- [ ] Middle-button drag pans from any point on the canvas.
- [ ] Mouse wheel/trackpad zooms around the pointer, normalizes browser delta
      modes, and clamps scale to 0.5–10.
- [ ] Preserve pointer-to-tile accuracy after zoom, pan, resize, scroll, and map
      switching.
- [ ] Suppress the browser context menu over the canvas so right-click tools
      work.
- [ ] Select a tile after a completed paint stroke and display its index,
      `(x, y)` coordinate, sprite identifier, and preview.
- [ ] Center the viewport on a located tile and change to its layer.
- [ ] Show a useful empty/missing-sprite state instead of crashing when a
      graphic or reference cannot be rendered.

### Tools and undo

- [ ] Select tool: click/drag from a source tile to a destination tile, move all
      metadata, merge list metadata into the destination, keep destination
      singleton metadata on collision, and clear moved metadata from source.
- [ ] Draw tool: continuously paint the selected tile graphic; when a copied
      rectangular brush exists, paint its entire footprint and cross grid
      boundaries where valid.
- [ ] Fill tool: four-way flood fill the contiguous region whose tileset name
      and tile ID match the starting tile, replacing its graphics.
- [ ] Clone tool: click/drag source to destination and copy metadata while
      leaving source metadata intact; merge lists and do not overwrite existing
      destination singleton metadata.
- [ ] Terrain tool: continuously paint the selected terrain type and update its
      eight neighbors through terrain autotiling.
- [ ] Erase tool: replace each visited cell with the default empty tile,
      removing both graphic and metadata.
- [ ] Erase metadata tool: preserve the graphic while clearing characters,
      items, markers, overrides, light source, event trigger, and travel trigger.
- [ ] Delete-fill tool: flood the matching region and replace every cell with
      the default empty tile.
- [ ] Right-click a single painted cell while draw/fill/delete-fill is active to
      pick its graphic; right-click a blank graphic switches to Erase.
- [ ] Right-drag a rectangle while draw/fill/delete-fill is active to create a
      reusable rectangular graphic/metadata brush with stable relative offsets.
- [ ] Right-click while Select/Clone is active to select a tile without moving
      or copying it.
- [ ] Preview fill regions, erase targets, terrain neighbor changes, move/clone
      targets, and rectangular brush footprints before committing.
- [ ] Store enough before-state to undo all cells in a stroke, including cells
      on multiple grid maps.
- [ ] Cap undo memory/history (legacy cap: 100 actions per map and 400 entries
      in grid ordering) or introduce a measured byte-based cap.
- [ ] `Ctrl/Cmd+Z` undoes when no input, textarea, or contenteditable field is
      focused. Redo is not current behavior and is not required for parity.
- [ ] Structural layer deletion remains explicitly non-undoable unless
      CEditor2 intentionally improves it.

**Legacy behavior:** Select/Clone dragging operates only within the focused
map, whereas draw/terrain/right-drag brushes can address neighbor blocks.
CEditor2 should document whether cross-map move/clone is supported before
claiming broader grid-edit parity.

### Layers

- [ ] List numeric layers in descending order and clearly indicate the active
      layer.
- [ ] Change layers directly from the list.
- [ ] Add one layer above the current highest layer and one below the current
      lowest layer, initializing a correctly sized empty graphic array.
- [ ] Delete any nonzero layer after confirmation, remove all of that layer's
      sparse placements, and select a remaining layer.
- [ ] Never allow deletion of layer 0.
- [ ] Use Up/Down arrows to select the adjacent numeric layer in sorted order,
      without assuming layers are contiguous.
- [ ] Keep layer selection valid after switching to a map with a different
      layer set; the implementation should not rely on every grid map having an
      identical layer stack.
- [ ] Commit materialized tile edits into the correct current layer before
      changing documents or saving.

### Terrain

- [ ] Use the `terrain_borders` tileset and its per-tile four-corner
      `tileTerrainBorderMeta` as the authoritative autotile lookup.
- [ ] List only terrain tags that have a complete same-tag base tile and show
      each as a sprite plus human-readable label.
- [ ] Paint the base tile under the pointer and resolve new variants for all
      eight neighboring tiles from their existing corner metadata.
- [ ] Preserve the current compatibility mapping for `NONE` edges: grass/void
      resolves through grass/dirt variants, while dirt or water/void resolves
      through that terrain/grass variants.
- [ ] Preview every tile that the terrain stroke will change.
- [ ] Track each stroke center once, record every changed tile once, and undo
      the complete neighbor update.
- [ ] Continue a terrain stroke across editable grid maps; preserve the current
      behavior that autotiling is resolved independently inside each map at a
      seam unless a deliberate cross-seam algorithm is designed and tested.
- [ ] Report a clear configuration error when `terrain_borders` is missing,
      when the selected tag has no base tile, or when required variants are
      absent.
- [ ] Invalidate/rebuild terrain lookup data when tileset metadata changes.

**Legacy behavior:** `buildTerrainLookup` uses one module-global cache that is
not keyed by tileset revision, so live tileset edits can leave stale results.
Do not port that cache behavior.

### Per-tile metadata

- [ ] Materialize and commit metadata independently for every layer.
- [ ] Three-state tile overrides (`inherit`/`true`/`false`) for walkable,
      see-through, and container behavior; allow creating/removing the override
      object as a unit.
- [ ] Preserve light-source placements (`angle`, `intensity`, `radius`) through
      every edit, move, clone, resize, layer, and save path, even though the
      current selected-tile panel does not expose a light editor.
- [ ] Search and add characters by full database name/label, reject duplicates
      on a tile, display type, link to that character editor, and remove them.
- [ ] Search and add items by full database name/label, reject duplicate
      non-stackable entries, increment stackable entries, clamp quantities to
      positive integers, preserve item ordering, reorder up/down, link to the
      item editor, and remove entries.
- [ ] Accept legacy item strings/missing quantities and normalize them to
      `{name, quantity: 1}` without data loss.
- [ ] Add unique, trimmed marker names by button or Enter and remove markers.
- [ ] For each marker, show all travel triggers from all maps that target it;
      sort references by source label, layer, and coordinate; and locate the
      source tile locally or by opening/switching maps.
- [ ] Create/remove a travel trigger. Default a new trigger to the current map,
      tile coordinates, and layer with `requiresAction: true` and hidden
      overlay.
- [ ] Edit travel destination map through full-database search, destination
      marker through markers on that map, X/Y, an actually existing destination
      layer, `requiresAction`, and overlay visibility.
- [ ] Clear an incompatible marker and normalize layer when destination map
      changes.
- [ ] Open a travel destination and select/center its marker or coordinates on
      the requested layer.
- [ ] Assign/remove only `MODAL` special events as event triggers through
      full-database search.
- [ ] Default event triggers based on effective walkability: non-walkable cells
      require Look; all new triggers require non-combat and default to hidden
      overlay.
- [ ] Display missing or non-modal event references as errors and link an
      existing event to its editor.
- [ ] Edit `requiresNonCombat`, `requiresLook`, and event overlay visibility.
- [ ] Quick-create a sign event with unique required ID and required contents,
      a title derived from the first 60 characters of the first line, a preview,
      and a forced Look requirement on the assigned trigger.
- [ ] Warn when a tile contains both event and travel triggers because runtime
      executes the event and ignores travel.
- [ ] Support overlay values `HIDDEN`, `SHOW_EVENT_ON_TILE`,
      `SHOW_TRAVEL_UP`, and `SHOW_TRAVEL_DOWN`, and render their in-game sprites.
- [ ] Show metadata indicators on the canvas for container contents, overrides,
      events, travel, and markers.

**Legacy behavior:** quick-created sign events are written to
`special-events.json` immediately, while most map edits wait for map Save. In
CEditor2 this must be one dirty database session and one Save All transaction.

**Legacy behavior:** missing character/item sprites are logged once per render
attempt and can flood the console under continuous rendering. CEditor2 should
deduplicate diagnostics and show reference validation in the UI.

### References

- [ ] Build indexes for map name, map-grid placement, marker, character, item,
      special-event, tileset, and travel destinations from the complete
      database snapshot.
- [ ] Preview every reference affected by a map rename, then update map-grid
      cells, all travel-trigger destination map names, tabs/editor state, and
      any other registered map-name fields in one database change.
- [ ] Before map deletion, show inbound grid and travel references and require
      the user to clear, retarget, or explicitly accept them; commit the chosen
      operation atomically with deletion.
- [ ] Detect missing tilesets/tile IDs, characters, items, events, maps,
      markers, and layers without discarding the original stored value.
- [ ] Keep links between independent CEditor2 apps URL-based; the map app must
      not import UI/controller code from character, item, event, tileset, or
      map-grid apps.
- [ ] Preserve marker inbound-reference discovery and locate actions across all
      maps and layers.
- [ ] Define behavior for marker rename/removal when travel triggers reference
      it; do not silently produce dangling references.
- [ ] Define and validate the one-grid-versus-many-grids rule for maps.

### Persistence and Save All

- [ ] Load and normalize legacy map shape, compact current map shape, missing
      sparse arrays, missing layers, and legacy item entries.
- [ ] Keep compact graphics and sparse metadata semantically identical after a
      no-edit load/save round trip.
- [ ] Preserve unknown fields that the editor does not understand.
- [ ] Mark all changed maps, grids, tilesets, and special events dirty in one
      shared database session.
- [ ] Save All validates and commits the entire registered database, not just
      `maps.json` or the currently visible map.
- [ ] Flush all edited materialized layer buffers, including neighbor maps,
      before serializing.
- [ ] Trim user-entered strings without changing intentional free-form text,
      and use deterministic JSON formatting/order rules.
- [ ] Detect stale database revision conflicts rather than overwriting an
      external edit or another browser tab.
- [ ] Stage, validate, replace, and recover all database files as one
      multi-file commit.
- [ ] Keep dirty state and show an actionable error after any failed validation
      or write.
- [ ] Warn on browser close/navigation with unsaved database changes.
- [ ] After a successful Save All, update the in-memory snapshot/revision and
      report success without losing the selected map, tab, layer, or viewport.
- [ ] Test interrupted/failed writes, malformed server responses, read-only
      files, concurrent edits, and serialization failures.

**Legacy behavior:** Maps Save validates and writes only `maps.json`; Map Grids
Save writes only `map-grids.json`; tile-template edits save `tilesets.json` when
their modal closes; grid-cell map creation saves grids immediately; sign quick
create saves events immediately. These timing differences are not parity goals.

### Shortcuts and input rules

- [ ] `B`: Draw.
- [ ] `F`: Fill.
- [ ] `S`: Select (but never intercept `Ctrl/Cmd+S`).
- [ ] `C`: Clone (but never intercept modified clipboard shortcuts).
- [ ] `T`: Terrain.
- [ ] `E`: Erase (but do not intercept `Ctrl+E`).
- [ ] `G`: toggle grid visibility.
- [ ] Up/Down arrows: adjacent map layer.
- [ ] Hold Tab: show contextual overlay text; release Tab: hide it.
- [ ] Escape: cancel an in-progress select drag and/or clear selected tiles.
- [ ] `Ctrl/Cmd+Z`: undo latest eligible stroke when focus is not in a text
      entry control.
- [ ] `Ctrl/Cmd+S`: prevent browser Save and run database-wide Save All.
- [ ] Middle-button drag: pan.
- [ ] Wheel/trackpad: pointer-centered zoom.
- [ ] Left click/drag: active tool or grid-slot action.
- [ ] Right click/drag: pick one tile or copy a rectangular brush depending on
      active tool.
- [ ] Never invoke canvas shortcuts while an input, textarea, select-like text
      search, or contenteditable control is accepting keyboard input.
- [ ] Make shortcut matching case-insensitive or explicitly document the legacy
      lowercase-only behavior.

No legacy direct shortcut exists for Erase Metadata or Delete Fill; toolbar
access is sufficient for parity.

### Error and edge cases

- [ ] No maps, no tilesets, no selected map, and no valid map in a selected
      grid all produce usable empty/error states.
- [ ] Missing tileset image or sprite does not stop input or future frames.
- [ ] A frame exception is surfaced and later frames continue.
- [ ] Invalid tile coordinates, layers, indices, dimensions, and grid cells are
      bounds-checked before access or mutation.
- [ ] Flood fill from outside the map is rejected rather than reading an
      undefined starting tile.
- [ ] Blank/malformed dense graphic arrays are normalized to exactly
      `width * height * 2` entries per layer.
- [ ] A current layer absent from a neighboring map renders as empty and cannot
      corrupt another layer.
- [ ] Empty/missing grid cells are never paint targets; brush cells outside
      assigned maps are skipped.
- [ ] A failed constrained grid-map creation does not leave a map created but
      unassigned, or a grid assignment saved without its map.
- [ ] Duplicate map/grid names, a map assigned more than once, and incompatible
      grid dimensions produce explicit validation.
- [ ] Missing map/event/item/character/marker references remain visible and
      repairable.
- [ ] Event plus travel precedence warning is retained.
- [ ] Invalid/zero item quantity normalizes to 1; non-stackable duplicates are
      not created.
- [ ] Removing/resizing a layer/map reports sparse metadata and references that
      will be dropped.
- [ ] Browser local-storage parse/quota/private-mode failures do not prevent the
      map app from starting.
- [ ] Multiple open editor tabs cannot silently overwrite one another.
- [ ] Canvas event listeners and animation frames are removed on app teardown;
      reopening the map app does not duplicate input handling.

## Reusable pure logic inventory

The following logic is worth porting behind tests. “Port” means preserve the
contract and simplify the implementation; it does not mean retain module-global
state, React hooks, `window` escape hatches, or cache ownership.

| Area                       | Existing source                                                        | Candidate pure contract/tests                                                                                                                                                       |
| -------------------------- | ---------------------------------------------------------------------- | ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| Compact map indexing       | `utils/mapIndex.ts`                                                    | `tileIndex`, `tileXY`, layer keys, get/set graphic pairs, normalize/migrate legacy maps, materialize/commit sparse placements, sorted/adjacent layer, add/delete layer, resize map. |
| Grid math                  | `utils/mapGridIndex.ts`                                                | Find placements, adjacent slots/maps, shared layer union, cell assignment, map rename, cross-cell brush resolution, slot editability.                                               |
| Grid data transforms       | `types/assets.ts`                                                      | Create/resize/shift cell matrices and normalize positive integer dimensions.                                                                                                        |
| Coordinate math            | `tile-editor/editorEvents.ts`, `gridMapNavigation.ts`, `utils/draw.ts` | Screen/canvas/map/grid coordinates, bounding rectangles, pointer-centered zoom, stitched viewport offset, pixel-art pan snapping, visible tile range, slot hotspots.                |
| Fill                       | `tile-editor/fill.ts`                                                  | Four-neighbor iterative flood fill keyed by tileset plus tile ID, with invalid-start protection added.                                                                              |
| Tool mutations             | `tile-editor/tools/index.ts`, `paintTools.ts`                          | Draw, erase, erase-metadata, fill/delete-fill, move/clone metadata merge rules, brush footprints, before/after patches, cross-map undo.                                             |
| Terrain                    | `tile-editor/terrainTool.ts`                                           | Metadata lookup, paintable tags, adjacent indices, neighbor variant calculation, fallback compatibility. Cache must be keyed/owned by database revision.                            |
| Map item entries           | `tile-editor/mapTileItems.ts`                                          | Legacy coercion, quantity clamp, add/increment, remove, reorder, and quantity update.                                                                                               |
| Walkability                | `tile-editor/mapTileWalkability.ts`                                    | Tile override then tileset metadata fallback.                                                                                                                                       |
| Location/reference queries | `tile-editor/mapLocate.ts`                                             | Collect/find markers and characters, inbound marker travel references, stable sorting. Keep DOM centering outside these queries.                                                    |
| Tab restoration            | `utils/mapTabsStorage.ts`                                              | Parse/validate/migrate persisted tabs, rebind grids, remove stale entries, deduplicate. Keep localStorage access behind an adapter.                                                 |
| Map preview sizing         | `utils/mapPreview.ts`                                                  | Pixel/draw-size calculations and selected-layer data lookup. Keep Canvas drawing in the map app.                                                                                    |
| Sign event creation        | `SelectedTileInfo/createSignGameEvent.ts`                              | ID prefix suggestion and deterministic event-node construction.                                                                                                                     |

Logic not to port as shared core:

- singleton `editorState` and `mapEditorEventState`
- `window.editorState` and `window.reRenderTileEditor`
- React refs/revision counters used to synchronize the canvas with forms
- global terrain/materialized-layer caches without document/revision ownership
- the current dirty-frame/heartbeat scheduler (continuous rendering is the
  agreed CEditor2 default)
- app-specific map/event/tileset UI and CSS

## Reproducible legacy performance baseline

### Current fixture inventory (2026-09-09)

The baseline must use committed database names, not generated maps, so another
developer can repeat it at the same revision.

| Fixture                   | Why it is included                                               | Current size                                                                                                                    |
| ------------------------- | ---------------------------------------------------------------- | ------------------------------------------------------------------------------------------------------------------------------- |
| `Alinea1`                 | Largest single map by tile/layer work                            | 40×40, 2 layers, 3,200 tile-layer cells, 6,400 graphic numbers, 5 dictionary entries                                            |
| `alinea_outsideAlinea1`   | Largest metadata-rich grid map and a terrain/tool representative | 30×30, 3 layers, 2 characters, 4 items, 3 markers, 3 events, 3 travel triggers, 5 dictionary entries                            |
| `AlineaTest`              | Smaller isolated multi-layer metadata regression fixture         | 25×20, layers `1, 0, -1`, 1 character, 5 items, 4 markers, 3 events, 2 travel triggers                                          |
| Grid `Alinea`             | Worst committed stitched-grid view                               | 6×3, all 18 cells assigned, each 30×30; 16,200 cells per layer across all blocks and 30,600 tile-layer cells in the source maps |
| Tileset `terrain_borders` | Terrain/picker stress fixture                                    | 448×512 image, 28×32 tiles, 256 tile metadata entries                                                                           |

The current database contains 25 maps. `maps.json` is about 912 KB,
`tilesets.json` about 208 KB, and `map-grids.json` about 1 KB. Current sparse
map data contains 3 character, 9 item, 20 marker, 7 event-trigger, and 10
travel-trigger placements; it contains no top-level tile override or light
source placements. Add synthetic correctness fixtures for those two metadata
types, but do not substitute them for the committed performance baseline.

### Environment record

For every baseline run, record:

- git commit hash and whether the worktree is dirty
- OS, browser name/version, CPU model, memory, display resolution/refresh rate,
  and device-pixel ratio
- Node/npm versions and whether browser DevTools is open
- browser zoom at 100%, one normal (non-private) window, extensions disabled,
  cache enabled after one warm-up load, and no CPU/network throttling
- viewport fixed at 1600×1000 CSS pixels (or record the exact available size if
  the machine cannot provide it)
- whether grid visibility and “Edit whole grid” are enabled; use the legacy
  defaults of grid on, grid edit on, render radius 2, edit radius 1

Run from repository root:

```sh
git rev-parse HEAD
git status --short
node --version
npm --version
cd ceditor
npm ci
npm run dev
```

Use the URL printed by Vite (normally `http://localhost:3000`) and close other
CEditor tabs. Make one unrecorded warm-up pass through every named fixture so
JSON, images, and decoded sprites are cached. Reload before each recorded case.

### Measurements

Use Chromium DevTools Performance with screenshots and memory enabled. Record
three runs per scenario, report each raw value and the median, and save profiles
with names containing commit, fixture, scenario, and run number. A run is
invalid if the browser tab loses focus or a server rebuild occurs.

Collect:

1. **Startup:** hard reload the maps URL, immediately choose the named map, and
   stop when the complete canvas plus tile picker is visible. Record navigation
   to first usable canvas, long tasks, total scripting time, peak JS heap, and
   console errors.
2. **Steady render:** with the whole map fitted in view, record 10 seconds with
   the pointer stationary. Record median/p95 frame time, dropped/long frames,
   FPS, main-thread utilization, and JS heap start/end. Because CEditor2 will
   intentionally render continuously, this is a draw-cost comparison, not an
   idle-zero-CPU requirement.
3. **Pan/zoom:** record 15 seconds: five seconds of continuous middle-button
   diagonal pan, five seconds of alternating wheel zoom around the center, then
   five seconds of pan. Record median/p95 frame time, FPS, input latency, long
   tasks, and heap growth.
4. **Draw stroke:** on layer 0 choose `terrain0` tile 1 (or record the exact
   nonblank tile used), select Draw, and drag a zigzag covering approximately
   200 distinct cells for 10 seconds. Undo once after the trace ends. Record
   frame time, event-to-paint latency, action completion duration, undo
   duration, long tasks, and retained heap. Reload without saving afterward.
5. **Fill:** on a large visually contiguous region, perform one Fill and one
   `Ctrl/Cmd+Z`. Record affected cell count, fill duration, commit duration,
   undo duration, and peak heap. Record starting coordinate and chosen tile so
   the run can be repeated; reload without saving afterward.
6. **Terrain:** on `alinea_outsideAlinea1` layer 0 select Terrain and a recorded
   available tag, paint a 100-cell serpentine stroke, then undo. Record frame
   time, per-update/complete duration if instrumented, long tasks, and peak and
   retained heap. Reload without saving afterward.
7. **Grid:** open grid `Alinea`, navigate to `alinea_outsideAlinea1`, fit enough
   view to include all radius-2 context, and record 10 seconds steady plus 15
   seconds pan/zoom. Then draw a stroke from the focused map across one editable
   neighbor and undo it. Record rendered block count, visible cell count,
   median/p95 frame time, FPS, input latency, action/undo duration, and heap.
8. **Layer/tab churn:** on `Alinea1`, switch layers 20 times using arrow keys;
   alternate between `Alinea1`, `AlineaTest`, and the `Alinea` grid 20 times.
   Record total duration, worst interaction delay, long tasks, and retained
   heap after a forced GC when DevTools permits it.
9. **Save:** make one reversible map graphic edit and use `Ctrl/Cmd+S`. Record
   click/key-to-success duration, request duration/bytes, serialization time if
   instrumented, and UI-blocking long tasks. Immediately restore the file from
   the test commit/worktree workflow; never benchmark Save against irreplaceable
   uncommitted data.

For frame timing, use DevTools' Frames track and Performance Monitor. If precise
event-to-paint/action timing is needed, add temporary local `performance.mark`
instrumentation but do not commit it. Use identical marks in CEditor2. Do not
compare React render counts—the replacement acceptance metric is observable
latency/frame cost, not framework internals.

### Baseline result sheet

Fill one row with the median of three runs and retain links/paths to raw profiles.

| Fixture/scenario                     | First usable (ms) | Median frame (ms) | p95 frame (ms) | FPS | Worst input delay (ms) | Long tasks | Peak heap (MB) | Retained delta (MB) | Operation/undo (ms) | Profile |
| ------------------------------------ | ----------------: | ----------------: | -------------: | --: | ---------------------: | ---------: | -------------: | ------------------: | ------------------: | ------- |
| `Alinea1` startup                    |                   |                   |                |     |                        |            |                |                     |                     |         |
| `Alinea1` steady                     |                   |                   |                |     |                        |            |                |                     |                     |         |
| `Alinea1` pan/zoom                   |                   |                   |                |     |                        |            |                |                     |                     |         |
| `Alinea1` draw/undo                  |                   |                   |                |     |                        |            |                |                     |                     |         |
| `Alinea1` fill/undo                  |                   |                   |                |     |                        |            |                |                     |                     |         |
| `alinea_outsideAlinea1` terrain/undo |                   |                   |                |     |                        |            |                |                     |                     |         |
| `Alinea` grid steady                 |                   |                   |                |     |                        |            |                |                     |                     |         |
| `Alinea` grid pan/zoom               |                   |                   |                |     |                        |            |                |                     |                     |         |
| `Alinea` cross-map draw/undo         |                   |                   |                |     |                        |            |                |                     |                     |         |
| Tab/layer churn                      |                   |                   |                |     |                        |            |                |                     |                     |         |
| Save                                 |                   |                   |                |     |                        |            |                |                     |                     |         |

### CEditor2 performance acceptance

Run the same protocol on the same commit/database, browser, machine, viewport,
and continuous-rendering policy. CEditor2 passes the first performance gate
when:

- no primary interaction scenario has a median or p95 frame time more than 10%
  worse than the legacy median without a documented reason
- pointer-to-visible-paint latency stays within the next animation frame during
  draw and terrain strokes under the representative fixtures
- no tested interaction produces repeated long tasks above 50 ms
- undo of the specified draw/fill/terrain cases completes without a visible
  multi-frame stall or data mismatch
- repeated tab/layer churn has no monotonic retained-heap growth attributable
  to abandoned controllers, listeners, images, or materialized layers
- the radius-2 `Alinea` grid view remains interactive and cross-map edits plus
  undo are correct
- Save All may take longer than legacy's single-file save because it commits the
  full database, but the UI remains responsive, progress/error state is clear,
  and the committed snapshot is complete and recoverable

These are initial gates, not permanent numeric budgets. Once the first legacy
profiles are captured, replace qualitative clauses with the measured limits and
keep the raw profiles alongside the Phase 0 report or release artifacts.
