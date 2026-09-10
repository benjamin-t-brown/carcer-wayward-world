# Phase 4 Progress: Map Kernel

Status: Implementation and automated exit criteria complete; interactive
browser acceptance remains open

## Delivered

- A lossless `MapDocument` over the compact dense graphics stored in `maps.json`
- Runtime-faithful map parsing with exact paths and unknown-field preservation
- Bounded coordinate conversion and two-integer cell reads/writes
- A canvas-independent viewport with pointer-anchored zoom and visible bounds
- A sprite-sheet image cache that coalesces loads by path
- A renderer that traverses only the visible tile rectangle
- Lossless map-grid topology with deterministic placements, neighbor slots, and
  tile-space origins
- Viewport-driven grid composition with one-partition overscan and no fixed
  radius around the focused map
- Seamless editing across every visible compatible grid partition
- Cross-block hit testing and map-local tileset dictionary translation
- High-DPI backing-store sizing with logical CSS-pixel drawing and pointer
  coordinates
- Continuous `requestAnimationFrame` rendering with per-frame error recovery
- Independent per-map viewport state
- Map, numeric-layer, tileset, and tile selection controls
- Hover and selected-cell outlines plus an optional grid overlay
- Middle-button pan and normalized wheel/trackpad zoom
- Select, pencil, and erase tools
- One compact deduplicated patch command per pointer gesture
- One undo/redo command for a gesture spanning multiple map documents
- A bounded 100-command undo/redo history
- A dependency-free, headless map-render benchmark with deterministic small,
  medium, and large compact-map scenarios
- Ctrl/Cmd+Z, Ctrl/Cmd+Shift+Z, and Ctrl/Cmd+Y shortcuts outside form controls
- Full-session Save All integration, including flushing an active gesture before
  a keyboard save
- An isolated paint, undo, redo, Save All, and reload integration test over all
  nine managed database files
- An isolated cross-grid Save All test that reloads edits in both maps while
  leaving every other managed file byte-identical
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

The scene writer converts the viewport to a clamped grid-cell rectangle each
frame and reuses its block storage. Enumeration is proportional to visible plus
overscan partitions, even for a massive grid. Each block still culls its own
visible tile rectangle, and offscreen grid maps do not traverse their dense
tile storage.

## Live integration checks

- All 25 current maps parse, including five maps with preserved negative layers.
- Both current map grids parse losslessly. The populated Alinea grid resolves
  every assigned map without missing or duplicate references.
- A focused-map-sized viewport over `alinea_outsideAlinea1` plus one-partition
  overscan produces nine editable blocks with exact 30×30-map seams.
- A synthetic 1,000×1,000 grid inspects exactly nine grid cells for a
  one-partition viewport plus overscan; it does not scan the million-cell grid
  during scene enumeration.
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
- [x] Neighboring maps render from grid topology without coupling the renderer
      to database or grid-domain code.
- [x] Pencil and erase gestures cross visible map boundaries and remain one
      compact undo/redo command.
- [x] Cross-map painting translates tileset names to each map's local compact
      dictionary index and refuses unavailable tilesets/layers.
- [x] Map-specific styling remains local to the map app.
- [x] DPR-aware sizing preserves logical viewport and pointer coordinates.
- [x] An isolated paint/save/reload cycle changes only `maps.json` and preserves
      unknown map data.
- [x] Unit, lint, type, build, and real-asset HTTP checks pass.
- [x] Headless frame timings, visible-cell counts, and visual command hashes are
      captured for representative compact maps.

## Remaining acceptance work

The code-level Phase 4 exit criteria are complete. Before visually accepting the
kernel for later parity work:

1. Perform an interactive browser visual comparison and browser-profiler run
   on representative real maps using
   [phase-4-browser-verification.md](phase-4-browser-verification.md). The
   headless JavaScript baseline and automated functional checks are complete.

Rectangle/clone brushes, fill, terrain autotiling, map/grid lifecycle, layer
mutation, metadata panels, references, and workspace navigation remain Phase 6
work. Those features should build on this kernel rather than expanding the
controller into a second global application state. The accepted continuous
workspace design is recorded in
[map-grid-workspace.md](map-grid-workspace.md).
