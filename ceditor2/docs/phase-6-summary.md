# Phase 6 Summary: Map and Tile Editor

Status: core map workflow complete; final browser profiling remains in Phase 8

## Delivered

- One continuous canvas workspace for a standalone map or every compatible
  partition in its map grid. World-tile lookup is direct and visible scene
  enumeration is bounded by the viewport plus one partition of overscan.
- Pencil, erase, four-way semantic fill, rectangle, and terrain tools work
  across partition boundaries. Each gesture is represented by compact cell
  patches and enters one bounded, grid-wide undo/redo history.
- Terrain painting uses the real `terrain_borders` corner metadata, updates the
  center and eight neighbors, translates each map's local tileset dictionary,
  and reports unavailable border variants.
- Map create, clone, rename, metadata edit, resize, delete, layer add, and layer
  delete operations preserve unknown fields. Rename and delete update map-grid
  cells and travel-trigger map references together in the database session.
- Selected-tile metadata can read and replace character, item, marker, event,
  travel, override, and light-source placements without losing unrelated or
  unknown map data.
- Sparse metadata overlays are culled per visible block. Holding Tab reveals
  labels, while the normal view retains compact indicators.
- Right-click samples a graphic from any editable partition and translates its
  map-local tileset index into the focused map's picker.
- Canvas rendering remains a simple continuous `requestAnimationFrame` loop.
  It has logical-pixel/DPR-safe coordinates, visible-tile culling, sprite
  caching, independent map viewports, and frame-error recovery.

The map-grid editor delivered in Phase 5 creates every backing map atomically
with a random collision-safe API name. The game-facing map and map-grid JSON
formats are unchanged.

## Deliberate simplifications

CEditor2 does not reproduce the legacy tab/local-storage subsystem. A native
map selector and URL deep link choose the workspace, while viewport state is
retained per map for the current session. A grid is inferred from the selected
map and presented as one canvas; users do not manage a second grid-edit mode or
an editable-neighbor radius.

The selected-tile inspector exposes the complete sparse placement bundle as
JSON instead of rebuilding seven deeply coupled modal editors. This keeps the
storage contract explicit and makes all metadata editable now; friendly
domain-specific controls can be added independently when they demonstrate a
clear usability benefit.

Legacy metadata move/clone drag modes, fill previews, and brush previews are
not retained in this phase. The reusable region/brush primitives are isolated
and tested, but the default toolbar favors the smaller set of common editing
operations. Structural map and layer changes are intentionally not added to
cell undo history.

## Verification

- 81 focused map tests pass, including the real Alinea grid and real terrain
  metadata fixtures.
- A synthetic 1,000 by 1,000 grid inspects only the visible partition and its
  overscan neighbors.
- Cross-partition fill, rectangle, pencil, erase, and terrain commands are
  covered with undo-safe tests.
- Map lifecycle, reference updates, sparse metadata preservation, renderer
  culling, overlay alignment, high-DPI coordinates, and malformed input paths
  have focused coverage.
- Strict TypeScript checking passes after the complete Phase 6 integration.

Interactive browser timing and the destructive Save All walkthrough are kept
in the Phase 8 verification protocol so automated work never modifies the live
game database.
