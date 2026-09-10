# Differences from the legacy CEditor

CEditor2 preserves the C++ game loader's JSON contracts, but intentionally does
not reproduce the React application's internal structure or every interaction.

## Application structure

- Every asset editor is a separate HTML entry and native TypeScript
  application. Editors share database, validation, reference, DOM, and form
  primitives but cannot import another editor.
- There is no React, client router, state library, form library, CSS framework,
  canvas framework, or transition system. Express is the only production
  dependency and serves the local database/media API.
- Shared CSS contains page-shell and form rules. Map, event, tileset, map-grid,
  and sound presentation remains in those applications.

## Saving

- Every editor works against the same complete in-memory snapshot. Save All
  validates and revision-checks all nine managed arrays, stages all output,
  rolls back partial replacement failures, and leaves dirty state intact after
  a failed request.
- A collection with no semantic changes keeps its exact bytes. This prevents a
  no-edit Save All from reformatting the checked-in database.
- `tiles.json` and media catalog files remain unmanaged and untouched.

## Maps and grids

- Selecting a map that belongs to a grid opens one continuous world canvas.
  The game still receives separate map records and the same grid record; the
  editor alone stitches partitions together.
- Rendering and hit-testing enumerate the viewport plus one partition of
  overscan, so workspace cost depends on visible partitions rather than total
  grid size. The canvas intentionally renders continuously.
- New grids eagerly stage a blank backing map for every cell with a random,
  stable, collision-safe API name. Those maps remain normal standalone map
  records.
- The legacy tabs, local-storage restoration, focus radii, and “edit whole
  grid” toggle are replaced by a map selector, URL deep link, per-map in-session
  viewport, and one consistent workspace.
- Common graphic tools and terrain autotiling cross compatible partition
  seams and share one compact undo/redo history. Legacy metadata move/clone
  drag modes and elaborate tool previews are not retained.
- Complete per-tile sparse metadata is editable through an explicit JSON
  bundle. This replaces seven coupled modal flows while preserving unknown
  fields and leaving room for smaller domain-specific controls later.

## Special events

- One lossless event document owns both graph and inspector edits. A simple
  continuous canvas supports pan, zoom, fit, box/multiple selection, group
  movement, and graph-aware copy/paste.
- Runtime node types have focused native controls; COMMENT, KEYWORD, unknown
  fields, and forward-compatible node types remain available through raw JSON.
- Graph/import diagnostics and the event runner are isolated domain code and
  tested against all checked-in events. The editor does not simulate game
  state or execute authored commands in the browser.

## Retained legacy editor

The `ceditor` directory is deprecated but not deleted or disabled. It remains
available for comparison until real editing sessions are accepted and the user
explicitly chooses CEditor2 as the default. Removal is a later cleanup change.
