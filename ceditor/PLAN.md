# CEditor Incremental Simplification Plan

## Purpose

Improve the established React CEditor in place instead of replacing it. The
work preserves its mature editing workflows and game-facing JSON formats while
making persistence safer, modules easier to understand, and the canvas editors
less globally coupled.

`ceditor2` remains in the repository as a reference implementation and source
of tested persistence/domain ideas. It is not the active product direction.

## Principles

1. Keep React. Replacing the UI framework is not an objective.
2. Add no runtime library unless a measured need cannot be met by React,
   TypeScript, Canvas 2D, and browser APIs already in use.
3. Preserve user-visible behavior before redesigning it.
4. Prefer small domain components and controllers over universal frameworks.
5. Every editor may read the complete database, but editor-specific UI and
   transient state stay within that editor.
6. **Save All** means one validated, revision-checked transaction containing
   the complete managed database.
7. Unknown JSON fields and legacy values remain lossless unless an explicit
   migration says otherwise.
8. Map and event canvases may render continuously. Performance comes from
   viewport culling, stable indexes, image caches, and allocation control—not
   complicated frame scheduling.
9. Shared CSS covers genuinely shared UI. Map, event, and specialized form CSS
   stays with its owner.

## Database ownership

The first atomic-save contract manages exactly the nine JSON collections the
game currently loads:

- `abilities.json`
- `characters.json`
- `items.json`
- `map-grids.json`
- `maps.json`
- `special-events.json`
- `spells.json`
- `status-effects.json`
- `tilesets.json`

It explicitly excludes:

- `feats.json`, which does not exist and has no current game loader.
- `tiles.json`, which exists but is not owned by either the editor or current
  game database loader.

No phase may silently add either file to Save All.

## Phase 0 — Re-baseline the active editor

Status: complete on 2026-09-10.

### Work

- Document `ceditor` as the active editor and `ceditor2` as a retained
  experiment/reference.
- Make the declared npm requirement usable by the repository's supported npm
  versions; do not require a global npm upgrade for optional install policy.
- Install from the lockfile and capture the current TypeScript/build baseline.
- Record the managed-file contract above.
- Confirm that no asset JSON changes during baseline setup.

### Gate

- `npm ci` succeeds on the supported local toolchain.
- The production build runs or its pre-existing failures are recorded.
- Git shows no changes under `src/assets/db`.

## Phase 1 — Tooling and characterization tests

Status: complete on 2026-09-10. Prettier enforcement begins with configuration,
tests, and newly extracted pure modules; legacy files join it in controlled
mechanical batches as their owning phase touches them.

### Work

- Add ESLint flat configuration, Prettier, and Node's built-in test runner via
  TSX. Do not add a browser test framework yet.
- Add `typecheck`, `lint`, `format`, `format:check`, `test`, and `check` scripts.
- Add pure tests for the shared asset registry, filtered-list selection,
  routing, JSON preparation, map geometry, and event geometry.
- Fix known filtered-index bugs in Items, Characters, and Tilesets by tracking
  stable IDs or unambiguous full-collection indexes.
- Remove the dead Feats home route until a game schema/file exists.
- Pass the registered status-effect deep-link parameter through its route.
- Capture deterministic counters for representative map and event rendering;
  avoid fragile pixel snapshots as the primary performance contract.

### Gate

- One `npm run check` runs lint, formatting verification, tests, typecheck, and
  the production build.
- Filtering cannot redirect edits or post-save selection to another record.
- Every visible Home card opens a valid editor route.
- No intended UI or serialized-data change.

## Phase 2 — Coherent database server

Status: complete on 2026-09-10. The coherent API now runs alongside the
temporary per-file endpoints so the existing client remains usable until the
Phase 3 session migration.

### Work

- Split `server/index.ts` into a small listener and a testable Express app.
- Add a repository for loading all nine managed arrays and computing a revision
  from their raw contents.
- Add `GET /api/database` returning `{ revision, assets }`.
- Add `PUT /api/database` accepting `{ baseRevision, assets }` and returning
  `{ revision, changedFiles }`.
- Validate the complete request before writing.
- Serialize concurrent saves; return HTTP 409 on stale revisions.
- Stage candidate files, parse-check them, replace changed files only, and roll
  back already replaced files after ordinary I/O failure.
- Preserve original bytes and mtimes for semantically unchanged collections.
- Retain old per-file endpoints only while the client migration needs them.

### Gate

- Incomplete, malformed, or stale requests perform zero writes.
- Injected replacement failure restores every prior target.
- No-op saves preserve bytes and mtimes.
- `tiles.json`, media definitions, and scratch files are untouched.

## Phase 3 — Client database session

Status: complete on 2026-09-10. Bootstrap now loads one coherent database
envelope, while normalized editor views remain separate from the raw clean
baseline until a collection is actually edited.

### Work

- Introduce a plain TypeScript database client/session owning the full snapshot,
  base revision, mutation versions, and dirty collections.
- Change bootstrap from ten unrelated requests to one coherent database load.
- Adapt `AssetsContext` to the session while initially preserving its familiar
  typed collection/setter API for editor components.
- Preserve existing normalization only where proven compatible with a no-edit
  round trip.
- Make an edit occurring during an in-flight save remain dirty afterward.
- Expose saving, validation, conflict, and dirty-file state to shared UI.

### Gate

- Every save payload contains all nine managed collections.
- Success advances the revision and clears only changes included in that save.
- Failure/conflict preserves local state and never overwrites newer disk data.
- A real database load/no-edit/save round trip is semantically identical.

## Phase 4 — Make Save All truthful everywhere

### Work

- Centralize the Save All button and Ctrl/Cmd+S command.
- Extract pure preparation functions for each collection's current trim, sort,
  and validation behavior.
- Add an explicit draft-flush boundary for canvas/editor state outside React.
- Migrate in three lanes: form editors, Maps/Map Grids, then Special Events.
- Stage cross-collection rename/delete changes in memory and persist them once.
- Convert the Special Events five-minute disk autosave into local draft recovery
  or remove it; it must not silently commit unrelated database changes.
- Remove per-file client saves and legacy endpoints only after all callers move.

### Gate

- Save All has exactly one meaning and produces one transaction on every page.
- Active map/event edits are included.
- Cross-file operations cannot partially persist.
- Navigation warns when committed session state is dirty.

## Phase 5 — Modular shell and CSS ownership

### Work

- Split bootstrap into API, load/normalize, root-provider, and route modules.
- Replace eager editor imports with a typed `React.lazy` route table; do not add
  a router dependency.
- Keep the complete database loaded while code for inactive editors stays out
  of the active route chunk.
- Mechanically extract the 1,691-line inline stylesheet into shared base,
  editor-shell, forms, and media styles plus map/event-owned CSS.
- Convert inline style objects incrementally after the mechanical move.
- Gate debug globals behind development mode or remove them.
- Move eager decoding of every sprite out of the critical startup path, backed
  by a promise/image cache and focused visual verification.

### Gate

- Direct hash navigation and deep links still work.
- Each major editor produces its own lazy-loaded chunk.
- Screenshot comparison shows no unintended visual change.
- Initial UI does not wait for every sprite to be decoded.

## Phase 6 — Form editor simplification and media polish

### Work

- Strengthen `TemplateEditorPage` only for common list/form behavior: stable
  keys/selection, pre-save validation, and card media slots.
- Migrate Items first. Keep Abilities, Characters, and Tilesets custom where
  their cascade/modal behavior differs.
- Share small notification, save-shortcut, and selection hooks rather than a
  schema-driven form engine.
- Replace the Tilesets timed remount workaround with a keyed form.
- Split large Ability and Character forms into presentational domain sections;
  parent forms retain state ownership.
- Consolidate the existing sprite/picture/animation pickers around one simple
  modal/grid shell while retaining sound playback and unknown legacy values.
- Add searchable animated previews with one clock per open animation modal.
- Remove arbitrary sprite truncation using filtering and incremental rendering.
- Co-locate editor-specific form styles.

### Gate

- Search, create, clone, delete, edit, validation, media preview, and selection
  restoration retain feature parity.
- Sprites/pictures are visually searchable, animations visibly play, and sounds
  play/stop correctly.
- Load/save preserves unknown fields and values.

## Phase 7 — Map editor controller and massive-grid performance

### Work

1. Characterize coordinate transforms, selection, paint/fill/terrain behavior,
   grid-seam editing, undo, and lifecycle.
2. Replace `window.reRenderTileEditor` with a tiny subscribed external store
   observed through `useSyncExternalStore`.
3. Convert module singleton state and input state into one instance-owned
   `MapEditorController` with idempotent attach/start/stop/destroy methods.
4. Scope pointer events to the canvas/root and use pointer capture. Retain only
   necessary keyboard/resize listeners outside it.
5. Move continuous `requestAnimationFrame` ownership into the controller and
   delete dirty-frame/heartbeat scheduling.
6. Preserve the existing paint, terrain, fill, grid-seam, undo, and materialized
   layer algorithms while passing controller-owned state explicitly.
7. Add a revision-keyed document index for maps, grids, placements, tilesets,
   tile definitions, characters, items, and events.
8. Replace fixed-radius grid traversal with a pure viewport-to-grid render plan
   that visits only intersecting partitions and visible tiles, with overscan.
9. Preserve standalone maps through the same one-partition planner.
10. Add coarse overview rendering only if benchmarks show it is needed.

### Gate

- Frame work scales with visible partitions/tiles, not total grid size.
- No per-tile linear searches through complete asset arrays.
- Offscreen grid maps are not materialized merely because they belong to a
  large grid.
- Continuous idle rendering meets the recorded baseline budget.
- Destroying a controller leaves no listener or animation-frame leak.
- Two controller instances share no selection, viewport, clipboard, or undo.

## Phase 8 — Special-event controller and viewport scaling

### Work

- Repeat the store/global-removal pattern with a separate
  `SpecialEventEditorController`; do not create a generic canvas framework.
- Make state, transform, input timing, linking, and clipboard instance-owned.
- Scope pointer/context-menu handling and expose semantic callbacks to React.
- Deep-clone or serialize saved per-event editor state to prevent aliasing.
- Make controller serialization the single source for switch/save/validate/run.
- Cull nodes and connectors against a pure visible-world rectangle while
  preserving update timing and z-order hit behavior.
- Add spatial hit-test indexing only if graph benchmarks justify it.

### Gate

- Every node subtype round-trips through serialization.
- Switching, saving, validation, and the runner see identical current content.
- Render work scales with visible nodes/connectors.
- Controller teardown is complete and two instances are isolated.

## Phase 9 — Hardening and cleanup

### Work

- Run real-database golden and failure-path integration tests.
- Verify map-plus-grid and event cross-reference transactions.
- Remove compatibility wrappers, obsolete global state, and legacy endpoints.
- Update contributor documentation and operational recovery notes.
- Reassess dependencies and delete only those made genuinely unused.
- Keep `ceditor2` until a separate explicit archival/removal decision.

### Gate

- Full check and browser smoke suite pass.
- A no-op real-database save changes no bytes.
- All failure tests leave disk unchanged.
- The working architecture matches the documented ownership boundaries.

## Swarm plan and sequencing

The following gates are sequential:

```text
managed-file contract
  -> server API contract
  -> client session integration
  -> editor save migration
  -> legacy persistence removal
```

After Phase 1 establishes tests, these lanes can be swarmed safely:

- Server repository/transaction and pure client session implementation, after
  sharing only the finalized API types.
- Mechanical CSS extraction and bootstrap/routing split.
- Map characterization tests and event characterization tests.
- Ability-form and Character-form section extraction.
- Map controller work and event controller work after agreeing on the small
  subscription lifecycle interface.
- Map viewport indexing and event viewport culling after controller ownership.

Avoid parallel edits when two lanes touch `AssetsContext`, `main.tsx`, the
shared route table, `TemplateEditorPage`, or the same CSS selectors. Integrate
at each phase gate and run the complete check before starting dependent work.

## Delivery discipline

- Keep phases in reviewable commits; separate mechanical formatting/moves from
  behavior changes.
- Never change game JSON schemas as an incidental refactor.
- Do not write live asset files in automated tests; use temporary copies.
- Record benchmark fixtures and counts so later optimizations are evidence-led.
- Update this plan when evidence changes sequencing or scope.
