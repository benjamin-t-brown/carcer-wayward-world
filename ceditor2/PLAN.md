# CEditor2 Project Plan

## 1. Purpose

`ceditor2` will replace the existing React-based `ceditor` with a simpler,
modular game-database editor built primarily with TypeScript, native browser
APIs, Canvas 2D, and Vite.

The rewrite is not intended to redesign the game database or change its JSON
formats. Its purpose is to make the editor easier to understand, maintain, and
extend while preserving or improving its editing capabilities.

The existing `ceditor` will remain usable throughout the rewrite. It will not
be removed until `ceditor2` has passed feature-parity and real-world editing
checks.

## 2. Agreed principles

### 2.1 Keep the application light

The browser application will not use React or another general UI framework.
It will use:

- TypeScript modules
- Native HTML forms and controls
- Direct DOM event handling
- Canvas 2D for the map and special-event editors
- Small, explicit controllers for stateful pages

Build and maintenance tooling does not count as application bloat. The project
may use:

- Vite
- TypeScript
- TSX
- Concurrently
- ESLint
- Prettier
- Express and its TypeScript declarations for the local filesystem API

Additional production dependencies require a concrete need. In particular,
the initial implementation will not add a client-side router, state-management
library, component framework, form framework, canvas framework, CSS framework,
or animation library.

### 2.2 Editors are independent applications

Each editor will be a separate Vite HTML entry point and browser application,
not a route inside one long-lived single-page application.

An editor may import shared database, domain, media, validation, and UI modules.
It must not import code from another editor application.

For example:

```text
apps/items  ------> core
apps/maps   ------> core
apps/events ------> core

apps/maps   --X--> apps/events
apps/items  --X--> apps/maps
```

Navigating from one editor to another may perform a normal page navigation and
create a fresh application session. This is intentional.

### 2.3 Every editor knows about the complete database

Each editor loads a complete database snapshot. This lets an item editor offer
ability and event selectors, lets the map editor resolve items, characters,
events, tilesets, maps, and grids, and lets rename/delete operations find every
affected reference.

Only the editor's own presentation and interaction code is isolated. The data
it can inspect and update is not artificially isolated.

### 2.4 Save All means the complete database

`Save All` will no longer mean only the collection visible on the current
page. It will validate and save a coherent snapshot of every managed JSON
database file.

Cross-file operations, such as renaming a special event and updating map,
character, and item references, will be part of the same save operation.

### 2.5 CSS follows ownership

Shared form-based editors are expected to share substantial styling. Map and
special-event editors are expected to have substantial custom styling.

- Base typography, color tokens, form controls, buttons, list layouts, common
  dialogs, validation states, and notifications live under `core/ui`.
- Reusable UI components own their own small CSS files when appropriate.
- App-specific CSS lives within that app.
- Map CSS does not accumulate in shared form CSS.
- Special-event CSS does not accumulate in map or shared CSS.
- App-specific selectors are scoped under an app root such as
  `[data-app="maps"]`.
- A style moves into shared CSS only after it is genuinely shared.
- Inline styles are reserved for calculated values such as canvas dimensions
  or dynamically positioned overlays, not ordinary visual styling.

### 2.6 Canvas rendering stays straightforward

The map and special-event canvases may render continuously with one
`requestAnimationFrame` loop.

- Start the loop when the editor mounts.
- Cancel it when the editor unmounts.
- Read the controller's current state each frame.
- Do not introduce dirty-render flags, idle heartbeats, or conditional frame
  scheduling unless profiling demonstrates a real need.
- Improve performance through visible-region drawing, stable data structures,
  cached images/lookups, and avoiding unnecessary allocation in hot paths.

The browser will naturally throttle `requestAnimationFrame` in background
tabs.

## 3. Scope

### 3.1 Managed applications

The initial application list is:

1. Home/database overview
2. Items
3. Abilities
4. Spells
5. Status effects
6. Feats, if retained as a database type
7. Characters/NPCs
8. Tilesets
9. Maps/tile editor
10. Map grids
11. Special events/in3 events
12. Sound/media browser

The asset inventory phase will resolve the current inconsistency where
`tiles.json` exists but is not in the editor registry, while `feats.json` is
registered but does not currently exist.

### 3.2 Non-goals

The initial rewrite will not:

- Change game-facing JSON schemas without a separately approved migration.
- Add a remotely hosted or multi-user service.
- Add real-time collaboration.
- Replace Canvas 2D with WebGL.
- Build a generic schema-driven form platform.
- Preserve accidental implementation details of React state and rerenders.
- Reproduce existing bugs solely for bug-for-bug parity.
- Optimize canvas scheduling without measured evidence.

## 4. Target project structure

The exact filenames may evolve, but dependencies should follow this shape:

```text
ceditor2/
├── README.md
├── PLAN.md
├── package.json
├── package-lock.json
├── tsconfig.json
├── eslint.config.js
├── .prettierrc.json
├── vite.config.ts
├── index.html
├── pages/
│   ├── items/index.html
│   ├── abilities/index.html
│   ├── spells/index.html
│   ├── status-effects/index.html
│   ├── feats/index.html
│   ├── characters/index.html
│   ├── tilesets/index.html
│   ├── maps/index.html
│   ├── map-grids/index.html
│   ├── events/index.html
│   └── sounds/index.html
├── src/
│   ├── core/
│   │   ├── database/
│   │   │   ├── assetRegistry.ts
│   │   │   ├── DatabaseClient.ts
│   │   │   ├── DatabaseSession.ts
│   │   │   ├── serializers.ts
│   │   │   └── types.ts
│   │   ├── domain/
│   │   │   ├── abilities.ts
│   │   │   ├── characters.ts
│   │   │   ├── events.ts
│   │   │   ├── items.ts
│   │   │   ├── maps.ts
│   │   │   ├── spells.ts
│   │   │   └── tilesets.ts
│   │   ├── media/
│   │   │   ├── assetFileParser.ts
│   │   │   ├── imageCache.ts
│   │   │   └── mediaCatalog.ts
│   │   ├── references/
│   │   │   ├── referenceIndex.ts
│   │   │   └── renameCommands.ts
│   │   ├── validation/
│   │   │   ├── databaseValidation.ts
│   │   │   └── validationTypes.ts
│   │   └── ui/
│   │       ├── base.css
│   │       ├── forms.css
│   │       ├── dom.ts
│   │       └── components/
│   ├── apps/
│   │   ├── home/
│   │   ├── items/
│   │   ├── abilities/
│   │   ├── spells/
│   │   ├── status-effects/
│   │   ├── feats/
│   │   ├── characters/
│   │   ├── tilesets/
│   │   ├── maps/
│   │   ├── map-grids/
│   │   ├── events/
│   │   └── sounds/
│   └── server/
│       ├── index.ts
│       ├── databaseRepository.ts
│       ├── saveTransaction.ts
│       └── staticAssets.ts
└── test/
    ├── fixtures/
    ├── database/
    ├── maps/
    └── events/
```

Larger applications may divide their local CSS by responsibility. For
example, the map app may contain `maps.css`, `tile-picker.css`, and
`selected-tile.css`. Those files remain owned and imported by the map app.

## 5. Core architecture

### 5.1 Database session

Each page owns one `DatabaseSession`. It contains:

- The full loaded database snapshot
- The disk revision from which it was loaded
- A set of dirty asset-file IDs
- Lookup indexes for names and IDs
- Validation results
- Explicit methods for replacing or mutating collections

This is an ordinary TypeScript object, not a global state framework. The page
controller decides which DOM regions need updating after a change.

The session should expose narrow operations such as:

```ts
session.getItem(name);
session.replaceItem(previousName, nextItem);
session.renameEvent(oldId, newId);
session.getReferences({ type: 'event', id: eventId });
session.validate();
session.saveAll();
```

Cross-domain behavior belongs here or in `core/references`, not in individual
form or canvas widgets.

### 5.2 Record preservation

The editor must not silently discard data it does not currently understand.

- Loaded records remain the source objects for edits.
- Forms replace known fields while preserving unknown fields.
- Normalization and migration code is explicit and tested.
- A no-edit load/save must be semantically identical.
- Formatting changes must be deterministic.

### 5.3 Reference index

Create one shared reference index over the database. It should locate at least:

- Item references to abilities and special events
- Character references to items, abilities, maps, and events
- Map placements of items and characters
- Map event and travel triggers
- Map-grid references to maps
- Event imports and event-node references
- Spell references to abilities and status effects
- Tileset references from maps

Rename and delete commands should return a change preview before applying
cross-file modifications.

### 5.4 Native UI approach

Most pages will use a small controller with direct DOM ownership:

```ts
class ItemEditorApp {
  constructor(
    private readonly root: HTMLElement,
    private readonly session: DatabaseSession,
  ) {}

  start(): void;
  stop(): void;
}
```

Use native event delegation for lists and repeated controls. Do not rerender an
entire form on every keystroke. Ordinary input elements already maintain their
own live state; the controller only needs to synchronize meaningful changes.

A shared list-editor shell may be extracted after two editors demonstrate the
same behavior. It should accept typed callbacks for list, select, create,
clone, and delete operations. It should not attempt to describe every domain
form through configuration.

## 6. Save All design

### 6.1 Load request

`GET /api/database` returns:

```ts
interface DatabaseEnvelope {
  revision: string;
  assets: DatabaseSnapshot;
  media: MediaCatalogSource;
}
```

The revision is derived from the managed source files. It detects changes made
by another editor tab, another program, or a source-control operation after the
page loaded.

### 6.2 Save request

`PUT /api/database` receives:

```ts
interface SaveDatabaseRequest {
  baseRevision: string;
  assets: DatabaseSnapshot;
}
```

The save sequence is:

1. Flush active form or canvas edits into the session.
2. Normalize only fields whose normalization is part of the data contract.
3. Run client-side collection and cross-reference validation.
4. Send the complete database snapshot and base revision.
5. Recompute the current disk revision on the server.
6. Return HTTP 409 without writing if the revision has changed.
7. Repeat structural and cross-reference validation on the server.
8. Serialize every managed asset using deterministic formatting.
9. Stage and parse-check every output file.
10. Back up current targets within the transaction workspace.
11. Replace changed files.
12. Roll back replaced files if any replacement fails.
13. Return the new revision and changed-file list.
14. Update the session baseline and clear dirty state.

Only changed bytes need to replace files, but the complete snapshot must be
validated and represented by the transaction.

### 6.3 Save behavior in the UI

Every app will provide the same save behavior:

- A clearly labeled `Save All` button
- Ctrl+S/Cmd+S
- Dirty-file indication
- Validation error summary
- Success message listing changed files
- Conflict message listing files changed on disk
- Navigation/unload warning while changes are dirty

The first release will not write timed autosaves to the game database. Draft
recovery through `localStorage` can be added later if it is shown to be useful,
but recovery data must never be confused with a successful database save.

## 7. Map editor architecture

### 7.1 State ownership

One `MapEditorController` owns the map editor's lifetime and transient state.
There will be no module-global editor state and no functions attached to
`window` to force UI rerenders.

The controller coordinates:

- Current map and open map tabs
- Current map grid and neighboring maps
- Selected tile and tileset tile
- Current level
- Active tool
- Pan and zoom
- Pointer gestures
- Undo and redo
- Map mutations in `DatabaseSession`
- Updates to native HTML panels

### 7.2 Suggested internal modules

```text
apps/maps/
├── index.ts
├── MapEditorController.ts
├── document/
│   ├── MapDocument.ts
│   ├── mapLayers.ts
│   └── mapPlacements.ts
├── canvas/
│   ├── MapRenderer.ts
│   ├── Viewport.ts
│   ├── InputController.ts
│   └── coordinates.ts
├── tools/
│   ├── Tool.ts
│   ├── PencilTool.ts
│   ├── EraseTool.ts
│   ├── FillTool.ts
│   ├── RectangleTool.ts
│   ├── CloneTool.ts
│   └── TerrainTool.ts
├── history/
│   ├── UndoStack.ts
│   └── MapPatch.ts
├── panels/
│   ├── LayersPanel.ts
│   ├── ToolsPanel.ts
│   ├── TilePicker.ts
│   └── SelectedTilePanel.ts
└── styles/
```

### 7.3 Map document mutations

Continue using the current flat, dense tile-graphics representation unless the
baseline proves it is a problem. Painting should mutate the working map
document deliberately rather than cloning the entire map to trigger UI work.

One completed gesture produces one undo command. A command stores compact
before/after patches for affected cells or placement entries, rather than full
map copies.

Pure and useful existing algorithms may be ported after tests are added,
including map indexing, grid coordinate resolution, materialization of tile
metadata, terrain selection, fill behavior, and sprite-coordinate math. React
components, React hooks, singleton editor state, and rerender bridges should
not be ported.

### 7.4 Rendering

`MapRenderer` uses a continuous, uncomplicated animation-frame loop. Its hot
path should:

- Draw only visible map cells and visible neighboring-map regions
- Reuse cached decoded images and sprite lookup data
- Avoid rebuilding reference indexes
- Avoid materializing every tile on every frame
- Avoid temporary arrays and object spreads inside tile loops
- Keep canvas drawing independent from HTML panel updates

Frame limiting, dirty flags, or worker-based rendering will be considered only
after a repeatable benchmark shows they are necessary.

## 8. Special-event editor architecture

One `EventEditorController` owns:

- The current `GameEvent`
- Canvas viewport state
- Node selection and multiselection
- Drag/link gestures
- Clipboard commands
- Dialog state
- Validation results
- Event-runner state

The stored `GameEvent.children` should remain the document model. Avoid keeping
a second authoritative graph that must later be synchronized back into the
event. Renderer-specific measurements and connector geometry may be cached as
derived view data.

Suggested modules:

```text
apps/events/
├── EventEditorController.ts
├── document/
├── canvas/
├── nodes/
├── dialogs/
├── runner/
├── validation/
└── styles/
```

The event canvas may also use a continuous animation-frame loop. Node editing
dialogs should use native `<dialog>` elements and event-specific CSS.

The event validator and event runner must depend on domain data and narrow
interfaces, not DOM nodes or editor-controller internals.

## 9. Tooling and commands

The initial scripts should provide:

```json
{
  "scripts": {
    "dev": "concurrently \"npm:dev:server\" \"npm:dev:client\"",
    "dev:server": "tsx watch src/server/index.ts",
    "dev:client": "vite",
    "build": "tsc --noEmit && vite build",
    "lint": "eslint .",
    "format": "prettier --write .",
    "format:check": "prettier --check .",
    "test": "node --import tsx --test",
    "check": "npm run lint && npm run format:check && npm run test && npm run build"
  }
}
```

Exact test globs may be added once the test directory is scaffolded.

Prettier owns formatting for TypeScript, JavaScript, HTML, CSS, JSON, and
Markdown. ESLint focuses on correctness and maintainability; it should not
duplicate Prettier's formatting rules.

The Vite client proxies `/api` to the Express server, so CORS middleware should
not be necessary for normal development.

## 10. Testing strategy

### 10.1 Fast tests

Use Node's built-in test runner, with TSX for TypeScript execution, for:

- Asset-file parsing
- Normalizers and serializers
- Map indexing and coordinate conversion
- Map placement materialization
- Paint tools and undo patches
- Terrain rules and fill behavior
- Reference indexing and rename/delete commands
- Event graph conversions
- Event validation and runner behavior
- Database-wide validation

### 10.2 Golden database tests

Maintain representative fixture files copied from or reduced from the real
database. Test that:

- Every current file loads.
- A no-edit round trip is semantically identical.
- Unknown fields survive known-field edits.
- Deterministic serialization is stable across repeated saves.
- Cross-file rename operations update all expected references and nothing else.

Tests must write only to temporary fixture directories, never directly to the
game database.

### 10.3 Save integration tests

Run the repository and transaction code against temporary directories. Cover:

- Successful complete save
- Unchanged save
- Invalid JSON shape
- Cross-reference validation failure
- Disk revision conflict
- Failure during staged commit
- Rollback after partial replacement
- Missing optional and required files
- Windows-compatible path and replacement behavior

### 10.4 Browser checks

Begin with a documented manual smoke checklist. Do not add a browser automation
framework until repeated regressions justify its maintenance cost.

Each editor's checklist should cover load, search, select, create, edit, clone,
delete, validation, Save All, refresh, deep link, and unsaved-navigation
behavior where applicable.

## 11. Performance baseline

Before replacing map code, capture repeatable measurements from the existing
editor using:

- The largest current map
- A representative multi-level map
- A map grid with neighboring-map rendering
- Large pencil and terrain strokes
- Fill, rectangle selection, clone, and undo
- Idle rendering

Record:

- Initial database load time
- Time until the first map frame
- Typical and worst frame time during interaction
- Input-to-visible-paint latency
- Memory after opening and switching several maps
- Save time for the complete current database

The initial goal is behavioral parity with no noticeable regression. Precise
optimization targets should be set from the baseline rather than guessed in
advance.

## 12. Phased implementation

### Phase 0: Inventory, contracts, and baseline

#### Work

- Enumerate every JSON database file and every SDL2W asset-definition file.
- Resolve whether `tiles.json` and `feats.json` are managed, legacy, optional,
  or future data.
- Map current TypeScript types to the C++ loaders and template structures.
- Document normalization performed by the current editor.
- Inventory references between every asset type.
- Create a feature-parity checklist for each current editor.
- Capture representative golden fixtures.
- Capture the map performance baseline.
- Document known existing defects separately from required parity.

#### Exit criteria

- The managed database registry is explicitly agreed upon.
- Every file has an owner, type, load rule, and save rule.
- No-edit round-trip expectations are documented.
- Map and event parity checklists exist.
- Baseline fixtures and measurements are reproducible.

### Phase 1: Project and tooling foundation

#### Work

- Create the package manifest and lockfile.
- Configure TypeScript, Vite MPA inputs, ESLint, and Prettier.
- Add development, build, check, and test scripts.
- Create the Express server and Vite proxy.
- Add the home HTML entry and at least two placeholder editor entries to prove
  independent builds.
- Establish `core`, `apps`, `server`, and `test` boundaries.
- Add shared design tokens, base CSS, and form CSS.
- Add a README with setup, development, test, and build instructions.

#### Exit criteria

- One command starts the client and server.
- Every placeholder entry is directly addressable and independently bundled.
- No client bundle contains React.
- One application cannot import another without violating the documented
  dependency policy.
- Lint, formatting check, tests, typecheck, and build run through `npm run
  check`.

### Phase 2: Database load, validation, and Save All

#### Work

- Implement the authoritative asset registry.
- Split domain types by asset family.
- Implement full snapshot loading.
- Port and test required normalizers.
- Parse SDL2W sprite, animation, picture, and sound definitions.
- Build media and reference indexes.
- Implement `DatabaseSession` and dirty-file tracking.
- Implement client and server validation.
- Implement revision hashing and conflict detection.
- Implement deterministic serializers.
- Implement staged database saves and rollback.
- Add Save All UI, keyboard shortcut, dirty state, and unload warning to the
  shared page shell.

#### Exit criteria

- The complete current database loads without changing source files.
- A no-edit round trip passes against fixtures.
- An edit to one file saves through the complete database transaction.
- Cross-reference errors can block a save with actionable messages.
- A stale browser session cannot overwrite newer disk content.
- Simulated commit failure restores the fixture database.

### Phase 3: First vertical editor

Use status effects as the initial editor because it is small while still
exercising the complete architecture.

#### Work

- Build search, selection, create, edit, clone, and delete behavior.
- Use native forms and shared form CSS.
- Preserve unknown record fields.
- Add validation messages and deep-link selection.
- Connect Save All and dirty tracking.
- Write unit and smoke tests.
- Review the implementation before extracting a generic list-editor shell.

#### Exit criteria

- The status-effect editor is usable without any legacy editor code at runtime.
- Editing and saving uses the full database transaction.
- The page has no dependency on another app.
- The native DOM approach is understandable enough to establish patterns for
  later form editors.

### Phase 4: Map-editor kernel

Address the highest-risk architecture early rather than leaving it until after
all simple forms are complete.

#### Work

- Port tested map-format and map-indexing utilities.
- Implement `MapDocument`, viewport conversion, and sprite/image caching.
- Implement `MapEditorController`, input lifecycle, and continuous renderer.
- Render one map and its layers.
- Add pan, zoom, grid overlay, hover, and selection.
- Add pencil and erase tools.
- Add compact undo/redo patches.
- Connect mutations to `DatabaseSession` and Save All.
- Create initial map-local CSS.
- Compare with baseline performance.

#### Exit criteria

- A current map opens and renders correctly.
- Pan, zoom, select, paint, erase, undo, redo, and save work.
- The canvas loop has no React or DOM-render dependency.
- Painting does not clone the entire map or rebuild form panels.
- Performance is no worse than the established baseline for this feature set.

### Phase 5: Form-based asset editors

Port editors in dependency-aware increments:

1. Abilities
2. Spells
3. Feats, if retained
4. Items
5. Characters/NPCs
6. Tilesets
7. Map grids
8. Sound/media browser

#### Work for each increment

- Port the relevant domain types and defaults.
- Implement the domain-specific form.
- Reuse the list-editor shell only where behavior is truly shared.
- Add reference pickers backed by the full database.
- Add create, clone, rename, and delete consequences.
- Add validation and cross-reference previews.
- Add app-local CSS only for unique presentation.
- Add fixture tests and update the parity checklist.

#### Exit criteria for each editor

- Current JSON records load and display correctly.
- All supported fields can be edited.
- Unknown fields survive edits.
- Create, clone, rename, delete, search, and selection behavior pass the
  relevant checklist.
- Cross-database references remain valid or produce explicit warnings.
- Save All and refresh reproduce the saved state.

### Phase 6: Complete map and tile editor

#### Work

- Implement fill, rectangle, clone, and terrain tools.
- Port terrain-border logic with focused tests.
- Implement layer creation, selection, and deletion.
- Implement the tileset and tile picker.
- Implement tile metadata editing for:
  - Characters
  - Items and quantities
  - Markers
  - Event triggers
  - Travel triggers
  - Walkability/visibility/container overrides
  - Light sources
- Implement map create, clone, edit, rename, and delete behavior.
- Implement open tabs and viewport persistence.
- Render neighboring grid maps.
- Implement grid navigation, adjacent-map creation, and cross-map painting.
- Implement grid-wide undo semantics.
- Complete app-local CSS and keyboard shortcuts.
- Run the full map parity and performance checklist.

#### Exit criteria

- Every retained legacy map-editor capability passes parity review.
- Map and grid renames/deletions correctly preview and update references.
- Existing maps save without schema drift.
- All tools work on representative large and multi-map fixtures.
- Interaction performance meets or exceeds the baseline.

### Phase 7: Special-event/in3 editor

#### Work

- Implement event list, filtering, recent selection, CRUD, and metadata form.
- Implement event canvas controller, viewport, and renderer.
- Implement each retained node type.
- Add create, edit, move, delete, link, and unlink behavior.
- Add multiselect, drag, copy, paste, and node-ID clipboard behavior.
- Implement variables and event imports.
- Port and isolate event validation.
- Port the event runner behind a domain-only interface.
- Add audio, item, map, and variable helpers backed by the full database.
- Add JSON preview/output if it remains useful.
- Add event-local CSS and native dialogs.
- Add event rename/delete reference previews and transaction support.

#### Exit criteria

- Current special events load without semantic conversion loss.
- Every retained node type can be created, edited, linked, validated, and
  saved.
- The event runner passes fixture scenarios.
- Event renames update all selected references through one Save All operation.
- The event app does not import map, item, or character application code.
- The event parity checklist passes.

### Phase 8: Hardening and cutover

#### Work

- Run complete golden-database round trips.
- Exercise failure recovery and stale-revision behavior.
- Complete accessibility and keyboard checks for native forms and dialogs.
- Review CSS ownership and remove leaked app-specific shared styles.
- Inspect production bundles for accidental cross-app inclusion.
- Remove dead abstractions and dependencies discovered during implementation.
- Run full map and event benchmarks.
- Conduct complete real-world editing sessions on a branch.
- Document differences from the old editor.
- Update repository development documentation and entry scripts.
- Mark the old editor read-only/deprecated before scheduling its removal.

#### Exit criteria

- `npm run check` passes from a clean installation.
- Every application passes its parity checklist.
- The real database can be loaded, edited across multiple asset types, saved,
  reloaded, and consumed by the game.
- Save conflicts and failures do not silently lose data.
- Map and event performance meet the agreed baseline.
- The user approves `ceditor2` as the default editor.

The old `ceditor` should be removed only in a later, explicit cleanup change.

## 13. Per-phase working rules

Every phase should follow these rules:

1. Work against fixtures for automated save tests.
2. Keep the old editor working.
3. Port behavior in vertical slices that can be manually exercised.
4. Add tests before porting complicated pure algorithms.
5. Run formatting, lint, tests, typecheck, and build before declaring the phase
   complete.
6. Update the parity checklist as behavior is accepted or intentionally
   changed.
7. Record newly discovered schema behavior in the domain contract.
8. Do not add an abstraction solely in anticipation of possible reuse.
9. Do not add rendering optimization without a reproducible measurement.
10. Do not write to the live game database from automated tests.

## 14. Project-wide completion criteria

`ceditor2` is ready to replace `ceditor` when:

- Every retained asset editor has its own HTML entry point and app directory.
- No app imports another app.
- Every app can inspect the complete database.
- Save All validates and commits a coherent complete database snapshot.
- Save All detects stale sessions and recovers from partial write failures.
- No-edit and ordinary-edit round trips preserve database semantics and unknown
  fields.
- Cross-domain rename/delete operations report and update all affected
  references.
- The map editor meets feature and performance parity without React rerender
  coordination.
- The special-event editor meets feature parity with one authoritative event
  document model.
- Shared form UI is cohesive while map and event CSS remain locally owned.
- The browser application contains no general UI, state, form, or canvas
  framework.
- Development tooling is documented and reliable.
- A complete editing session succeeds against the game database and the game
  loads the resulting assets correctly.

## 15. Recommended first implementation tranche

The first implementation tranche should cover Phases 0 through 3 only:

1. Establish the database contract and fixtures.
2. Scaffold the independent Vite applications and tooling.
3. Implement safe database-wide loading and saving.
4. Complete the status-effect editor as the architectural proof.

This produces a small but genuinely usable editor and validates the most
important architectural choices before committing to the map and special-event
rewrites.
