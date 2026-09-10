# Phase 4 Progress: Initial Map Kernel

Status: Initial milestone complete; full Phase 4 exit gate remains open

## Delivered

- A lossless `MapDocument` over the compact dense graphics stored in `maps.json`
- Runtime-faithful map parsing with exact paths and unknown-field preservation
- Bounded coordinate conversion and two-integer cell reads/writes
- A canvas-independent viewport with pointer-anchored zoom and visible bounds
- A sprite-sheet image cache that coalesces loads by path
- A renderer that traverses only the visible tile rectangle
- High-DPI backing-store sizing with logical CSS-pixel drawing and pointer
  coordinates
- Continuous `requestAnimationFrame` rendering with per-frame error recovery
- Independent per-map viewport state
- Map, numeric-layer, tileset, and tile selection controls
- Hover and selected-cell outlines plus an optional grid overlay
- Middle-button pan and normalized wheel/trackpad zoom
- Select, pencil, and erase tools
- One compact deduplicated patch command per pointer gesture
- A bounded 100-command undo/redo history
- A dependency-free, headless map-render benchmark with deterministic small,
  medium, and large compact-map scenarios
- Ctrl/Cmd+Z, Ctrl/Cmd+Shift+Z, and Ctrl/Cmd+Y shortcuts outside form controls
- Full-session Save All integration, including flushing an active gesture before
  a keyboard save
- An isolated paint, undo, redo, Save All, and reload integration test over all
  nine managed database files
- Map-owned responsive CSS with no changes to other editor layouts

## Performance shape

The animation loop is intentionally continuous. Its render path reads the
current dense layer directly and does not materialize per-tile objects. Visible
bounds are written into a reused object, sprite metadata is indexed before the
loop, images are cached by sheet path, and the renderer draws directly from
sprite sheets. The renderer test demonstrates nine visits for a 25×25 viewport
over a 10,000-cell map rather than traversing all 10,000 cells.

The repeatable benchmark in [map-performance-baseline.md](map-performance-baseline.md)
separates deterministic culling/Canvas2D-command counts from wall-clock timing.
Its 512×512 scenario visits 9,940 of 262,144 cells per frame. The fake Canvas2D
context measures JavaScript traversal and command dispatch, not browser
rasterization, GPU compositing, image decoding, or DOM layout.

Painting changes the active `MapDocument` immediately so the next frame sees
the result. A gesture stores only the first before pair and final after pair for
each visited cell. The complete maps collection is copied into
`DatabaseSession` once when the gesture ends, is undone/redone, or must be
flushed before Save All; it is not cloned for each painted cell or animation
frame.

## Live integration checks

- All 25 current maps parse, including five maps with preserved negative layers.
- Full-database validation remains at zero errors and the same seven known
  warnings.
- The real database API returns 25 maps and four tilesets under a 64-character
  revision.
- Both SDL2W definition files load through the media endpoint.
- A real sprite sheet resolves through the new static URL and returns HTTP 200.

## Current milestone checklist

- [x] Map document reads and writes the existing compact format losslessly.
- [x] Current maps and negative layers load without normalization.
- [x] Only visible cells are traversed by the renderer.
- [x] The canvas runs continuously without React or DOM-render dependencies.
- [x] Pan, zoom, layer change, hover, and selection are implemented.
- [x] Pencil and erase gestures are compact and undoable.
- [x] Redo and bounded history are implemented.
- [x] Active gestures flush into the database session before Save All.
- [x] Map-specific styling remains local to the map app.
- [x] DPR-aware sizing preserves logical viewport and pointer coordinates.
- [x] An isolated paint/save/reload cycle changes only `maps.json` and preserves
      unknown map data.
- [x] Unit, lint, type, build, and real-asset HTTP checks pass.
- [x] Headless frame timings, visible-cell counts, and visual command hashes are
      captured for representative compact maps.

## Remaining Phase 4 work

Before declaring the full Phase 4 gate complete:

1. Perform an interactive browser visual comparison and browser-profiler run
   on representative real maps; the headless JavaScript baseline is complete.
2. Add neighboring grid-map rendering and cross-block editing.
3. Add the first rectangular brush if it belongs in the kernel milestone.

Later map phases still own fill, terrain autotiling, map/grid lifecycle, layers,
metadata panels, references, tabs, and grid navigation. Those features should
build on this kernel rather than expanding the controller into a second global
application state.
