# Form and Special-Event Editor Parity Audit

## Purpose and scope

This document records the user-visible behavior of the current `ceditor` form
editors and special-event editor before they are replaced. It is a parity
checklist, not a direction to reproduce the current React implementation or its
bugs.

The map/tile canvas and map lifecycle are covered by the map-editor audit. This
document does include map grids, tilesets, and cross-references into maps because
they are required by non-map workflows.

Checkboxes below describe behavior to deliberately retain or re-evaluate in
`ceditor2`. “Questionable behavior” sections identify behavior that must not be
accepted as a compatibility requirement without a product decision.

## Shared list-and-form baseline

The item, ability, spell, feat, status-effect, character, tileset, and map-grid
pages use variations of the same two-column editor pattern.

- [ ] Show a page header, back navigation, and a prominent Save All action.
- [ ] Show a searchable entity list beside the selected entity's form.
- [ ] Search case-insensitively across the entity's ID/name and useful display
      fields; keep the selected entity stable when filtering where possible.
- [ ] Provide a clear empty state when no entity is selected or no search result
      matches.
- [ ] Create a valid domain-specific default entity, select it, and clear the
      search so it remains visible.
- [ ] Deep-clone an entity, give the clone an `_copy` ID/name suffix, insert it
      near the source (or consistently at the end), and select it.
- [ ] Confirm deletion and keep selection valid when list indices change.
- [ ] Update the in-memory database immediately as form controls change.
- [ ] Trim string values recursively and apply deterministic collection sorting
      as part of the database save pipeline.
- [ ] Preserve selection by stable ID/name after sorting, renaming, or reloading.
- [ ] Support `Ctrl+S` and `Cmd+S` without triggering the browser save dialog.
- [ ] Report save success, validation failures, and server failures visibly.
- [ ] Distinguish unsaved in-memory edits from data committed to disk.
- [ ] Keep all form controls keyboard accessible and associate labels with
      controls.
- [ ] Preserve unknown JSON fields unless an explicit schema migration removes
      them.

### Shared behavior worth extracting

These are useful shared primitives, not a request for a generic schema-driven
form framework.

- Editor shell: header, sidebar, main form region, empty state, and app root.
- Entity list: selection, case-insensitive search, clone/delete actions, optional
  badge/preview content, and a stable ID rather than array-index identity.
- Basic controls: text, textarea, number, checkbox, enum select, searchable
  select, sprite/picture picker, sound preview/search, and labeled field groups.
- Dialogs: confirm/cancel lifecycle, destructive confirmation, Escape handling,
  and focus restoration.
- Repeatable fields: add/remove/reorder rows and compact remove buttons.
- Feedback: transient notifications plus persistent validation/dirty state near
  Save All.
- Cross-reference select: searchable label/ID display, tolerate and flag a
  currently missing target, and link to the target editor in a new page/tab.
- Collection command helpers: create, clone, delete, rename, sort, restore
  selection, and preview/apply reference consequences.
- Media previews: sprite, picture, animation, and sound lookup with a clear
  missing-media state.

Shared controls should own behavior and shared form CSS. Domain-specific
sections, validation, conditional fields, and mutations remain inside their
individual applications.

## Per-editor parity checklists

### Items

- [ ] Search by name, label, and item type; show the item sprite and type in the
      result card.
- [ ] Create, clone, select, and delete items.
- [ ] Edit required item type, name, label, weight, value, icon, and description.
- [ ] Validate required fields, numeric fields, and duplicate names before save.
- [ ] Edit `stackable` and the storage-spelled `indestructable` flag without
      silently changing the game-facing JSON key.
- [ ] When type is `RUNE`, edit rune type and provide/derive the corresponding
      rune icon; remove rune-only data when changing to another item type.
- [ ] Edit where an item is usable and enforce the existing exclusive choice
      between `useAbility` and `useSpecialEvent`.
- [ ] Select a use ability through full-database search and provide a deep link
      to that ability.
- [ ] For use abilities, edit per-attack damage overrides and per-restore
      overrides based on the selected ability.
- [ ] Select a use special event through full-database search and provide a deep
      link to that event.
- [ ] For weapon item types, select a base ability and edit per-attack damage
      overrides.
- [ ] Seed and normalize override arrays when the selected ability changes while
      preserving intentional overrides where their corresponding attack/restore
      still exists.
- [ ] Add, change, and remove status effects applied while equipped.
- [ ] Retain compatibility normalization for legacy `itemUsabilityArgs`, legacy
      weapon damage fields, and legacy attack-index storage until the database
      has been explicitly migrated.

Cross-reference consequences: items refer to abilities from both `weapon` and
`useAbility`, special events from `useSpecialEvent`, and status effects from the
equipped-status list. Ability attack structure changes can require remapping item
override arrays.

### Abilities

- [ ] Search by name, label, and ability type; show type in the result card.
- [ ] Provide three create presets: generic ability, melee ability, and spell
      ability.
- [ ] Create, clone, select, and delete abilities.
- [ ] Edit identity (name/ID, label, icon, type), AP cost, auxiliary cost type
      and value, and description.
- [ ] Edit target selection: target type, allegiance, number of targets, zone
      width/height, and range.
- [ ] Edit depiction: projectile enable/type/path, damage animation, start sound,
      and damage sound, with animation and sound previews/search.
- [ ] Add, edit, and remove ordered attacks, including class, damage type, dice,
      damage bonus/stat/multiplier, attack bonus where applicable, and save
      fields.
- [ ] Add, edit, and remove applied statuses, including status-effect reference,
      duration override, and duration bonus.
- [ ] Add, edit, and remove restores, including current stat, dice, bonus, source
      stat, and multiplier.
- [ ] Add, edit, and remove direct damages, including damage type, dice, bonus,
      source stat, and multiplier.
- [ ] When deleting an attack, preview affected weapon/use-ability item overrides
      and remap them only after confirmation.
- [ ] When deleting an ability, preview and update item weapon references, item
      use-ability references, and triggered-ability references in status effects.
- [ ] Flag spells whose `abilityName` no longer resolves.
- [ ] Normalize deprecated/invalid depiction values against current media
      catalogs deliberately, with a visible indication if loading the form
      changes data.

### Spells

- [ ] Search by name and label; show the icon sprite in the result card.
- [ ] Create, clone, select, and delete spells.
- [ ] Edit name/ID, label, icon, and description.
- [ ] Search/select the backing ability and link to the ability editor.
- [ ] Preserve a missing current ability value so it can be diagnosed rather
      than silently cleared.
- [ ] Edit required rune counts for every rune type with increment/decrement
      controls and sprite previews; omit or normalize zero/invalid counts using
      the established JSON contract.
- [ ] Validate backing ability references as part of pre-save whole-database
      validation.

### Status effects

- [ ] Search by name and description.
- [ ] Create, clone, select, and delete status effects.
- [ ] Edit name/ID, base duration, and description.
- [ ] Optionally enable duration scaling and edit source stat and multiplier.
- [ ] Optionally enable passive generic-stat bonuses (`STR`, `MND`, `CON`,
      `AGI`, `LCK`).
- [ ] Optionally enable current-stat changes (`HP`, `AP`, `MANA`, `AC`).
- [ ] Add, edit, and remove resistance modifiers by damage type.
- [ ] Add, edit, and remove triggered-ability actions, including target, ability,
      and one or more trigger event/condition pairs.
- [ ] Search abilities from the complete database and preserve/flag unresolved
      references.

Cross-reference consequences: status-effect names are referenced by abilities
and items; status-effect actions reference abilities. The current editor only
handles ability deletion consequences, so `ceditor2` needs a complete reference
index before defining status-effect rename/delete behavior.

### Feats

The current live form exposes only a small subset of the apparent schema.

- [ ] If feats remain a managed asset, provide create, clone, select, delete,
      search by ID/label, deterministic sorting, and Save All integration.
- [ ] Edit ID, label, implementation (`DATA` or `CUSTOM`), and description.
- [ ] Resolve whether `requiresFeats`, `effects`, and `excludesGroup` are active
      game schema before implementing them; their current UI is commented out
      and is not a parity requirement.
- [ ] Add an actual feat route/page entry if the database type is retained.

### Characters / NPCs

- [ ] Search by name, label, and character type; show sprite and type in the
      result card.
- [ ] Create, clone, select, and delete characters.
- [ ] Edit type, name/ID, label, and sprite sheet/offset through the sprite picker.
- [ ] Edit generic combat stats (`STR`, `MND`, `CON`, `AGI`, `LCK`).
- [ ] Edit weapon mastery (edged, pole, blunt, range, unarmed).
- [ ] Edit magic mastery (mana, ability power, attunement, faith, lore).
- [ ] Edit body mastery (physical/magical resistance, healing effectiveness,
      damage reduction, armor training).
- [ ] Edit skills (trickery, stealth, social, magic-item use, cooking,
      acrobatics, survival, focus, conditioning).
- [ ] Edit optional talk event and portrait sprite; limit normal selection to
      TALK events while preserving and flagging a currently unresolved or
      non-TALK value.
- [ ] Deep-link from a selected talk event to the special-event editor.
- [ ] Edit map behavior and town/combat behavior modes.
- [ ] Edit combat HP, MP, drop-table ID, death sound, weapon sound, and vision
      radius.
- [ ] Add, edit, and remove initial statuses.
- [ ] Validate required type/name/label/sprite information and duplicate names.
- [ ] When deleting a character, preview placements across maps by map, level,
      and coordinate, then remove those placements after confirmation.

### Tilesets

- [ ] Search by name/sprite base and show useful image/tile metadata in the list.
- [ ] Create, clone, select, and delete tilesets.
- [ ] Choose a source picture, capture its pixel dimensions, populate the
      tileset name/sprite base, and create row-major default metadata for every
      tile.
- [ ] Display tile width/height and derived grid dimensions. Preserve the current
      database's fixed tile-size assumption unless a deliberate migration makes
      these editable.
- [ ] Overlay selectable tile cells on the source picture with single selection,
      `Ctrl`/`Cmd` toggle selection, and drag-rectangle selection.
- [ ] Edit one tile or apply supported metadata changes to multiple selected
      tiles.
- [ ] Support keyboard navigation between single-tile edit dialogs with
      `Alt+Left` and `Alt+Right` if this remains useful after usability review.
- [ ] Edit all tile metadata currently exposed by the tile edit modal, including
      description, step sound, walkability, visibility, door/container flags,
      and terrain-border metadata.
- [ ] Select a terrain strip, primary/secondary terrain, and starting tile; apply
      and validate the terrain-strip layout.
- [ ] Validate name, image/sprite base, image dimensions, tile dimensions, tile
      metadata presence, and duplicate names before save.

Cross-reference consequences: maps address tilesets by name and tile index.
Renaming a tileset or selecting a different source picture can invalidate a
large amount of map data and must use a reference preview rather than the
current unguarded behavior.

### Map grids

- [ ] Search by name and label; create, clone, select, and delete map grids.
- [ ] Edit name/ID and label.
- [ ] Edit grid width/height as positive integers and resize the rectangular
      cells array while retaining in-bounds assignments.
- [ ] Edit required map width/height as positive integers.
- [ ] Show assigned/total slot counts.
- [ ] Render each cell with a map preview, label/name, coordinate, missing-map
      indication, and clear action.
- [ ] Pick only maps matching the required tile dimensions, with town/outdoor
      filtering supplied by the map picker.
- [ ] Link from an assigned cell to the map editor.
- [ ] Move all assignments by an integer x/y offset and drop cells moved outside
      the grid only after clear confirmation/preview.
- [ ] Normalize malformed/non-rectangular cell arrays and validate non-empty,
      unique grid names before save.

Cross-reference consequences: grid cells refer to maps by name, while maps may
also declare grid membership/position. `ceditor2` must define one invariant and
validate both directions; map/grid rename and deletion cannot be local-only.

### Sound/media browser

- [ ] Load the complete sound catalog from generated/native asset metadata.
- [ ] Search case-insensitively by sound name or path and sort by name.
- [ ] Show name and path and provide play/stop preview controls.
- [ ] Show an empty state for no results and a useful missing/unloadable-audio
      error.
- [ ] Keep this browser read-only unless media authoring is explicitly added to
      scope.

## Special-event editor parity

### Event library and document operations

- [ ] Search case-insensitively by ID, title, or event type.
- [ ] Filter independently by `MODAL`, `TALK`, and `TRAVEL`, grouped in the
      current TALK/MODAL/TRAVEL display order or a newly documented order.
- [ ] Create an event with unique ID, title, type, icon, empty variables, and a
      root exec node.
- [ ] Clone an event deeply with an `_copy` ID and select it.
- [ ] Edit ID, title, type, and icon without rebuilding or losing graph data.
- [ ] Confirm deletion and report events that import variables from the target.
- [ ] Keep a session recent-events list (currently at most ten shown) and allow
      entries to be selected and removed.
- [ ] Expose a read-only, current JSON view and copy it to the clipboard.
- [ ] Find a node by exact ID, center it, and visibly flash it.
- [ ] Validate the current graph and make each error navigate to its node.
- [ ] Run the current unsaved graph in the event runner.

### Canvas navigation and selection

- [ ] Render node cards and directed connectors continuously on Canvas 2D.
- [ ] Resize the canvas with its container and preserve usable pan/zoom state.
- [ ] Pan with left-drag on empty space and middle-drag; zoom around the pointer
      with the wheel within reasonable bounds.
- [ ] Select a node by clicking, toggle selection with `Ctrl`/`Cmd` click, and
      select intersecting nodes with a `Ctrl`/`Cmd` drag rectangle.
- [ ] Drag one node or all currently selected nodes while retaining relative
      positions.
- [ ] Clear selection and cancel link mode with Escape.
- [ ] Delete the selected nodes with the Delete key after confirmation; clear
      all parent exits that pointed at deleted nodes.
- [ ] Copy selected nodes with `Ctrl`/`Cmd+C` and paste near the pointer with
      `Ctrl`/`Cmd+V`.
- [ ] Give pasted nodes new IDs, preserve relative layout, preserve links within
      the copied set, and clear links to nodes outside the copied set.
- [ ] Double-click a node to edit it.
- [ ] Right-click empty canvas or a node to create/link/copy ID; right-click a
      connector line to remove that connection.
- [ ] Enter link mode from a source exit and complete it by selecting a target;
      allow empty-canvas click or Escape to cancel.
- [ ] Retain graph position and pan/zoom while switching among events during the
      same page session.

There is no working undo/redo in the current event editor. It should be treated
as a desirable new feature, not as existing parity.

### Node types and editors

- [ ] `EXEC`: edit display text (`p`), executable statements (`execStr`),
      auto-advance, optional audio, and one `next` exit.
- [ ] `CHOICE`: edit introductory text and optional audio; add/remove/reorder
      choices; for each choice edit condition, visible text, evaluated command,
      prefix text, conditional alternate texts, and `next` exit.
- [ ] `SWITCH`: add/remove/reorder condition cases, link each case, and link a
      default exit.
- [ ] `END`: edit its end-node data and render it as a terminal node. Confirm
      whether the schema's `next` field has any supported meaning.
- [ ] `COMMENT`: edit free-form comment text; keep it out of runner execution.
- [ ] Preserve unknown/legacy node types in JSON and report them. `KEYWORD`
      remains in TypeScript types but has no current creation or runner path, so
      it must not be silently deleted.
- [ ] Recalculate node dimensions and connector anchor positions when modal
      edits change text or repeatable exits.
- [ ] Provide searchable/copyable variable names, item IDs, executor functions,
      and audio names in the relevant node dialogs.

### Variables and imports

- [ ] Add, edit, remove, and reorder local variables (`id`, `key`, `value`).
- [ ] Add and remove imports from other events using `importFrom` records.
- [ ] Prevent obvious duplicate/self imports and expose missing import targets.
- [ ] Offer the existing utility-variable preset only if its exact generated
      values are covered by fixtures.
- [ ] Resolve imported variables when displaying/running an event and preserve
      local ordering where it is game-significant.
- [ ] On event rename, update all `importFrom` references in the same database
      transaction.
- [ ] On event deletion, preview every import that will be removed; do not
      mutate importing events without making the consequence explicit.

### Runner behavior

- [ ] Start at node ID `root` when present, otherwise at the first child; report
      an empty event rather than throwing.
- [ ] Execute semicolon- or newline-delimited statements, except delimiters
      nested inside parentheses.
- [ ] Replace `@variable` tokens using local/imported event variables.
- [ ] Traverse switch cases in order, taking the first true case, otherwise the
      default exit.
- [ ] Filter choices by condition; resolve the first matching alternate
      `switchText`; render prefix plus choice text; execute a selected choice's
      command before following its exit.
- [ ] Implement `ONCE` as a pending condition effect committed only when its
      branch/choice is actually taken.
- [ ] Pause on a non-auto-advancing exec node with display text, pause for a
      choice, show terminal storage at an end node, and recursively skip empty or
      auto-advancing exec nodes.
- [ ] Maintain a transcript of displayed text and selected choices; visually dim
      history and auto-scroll to the newest entry.
- [ ] Clicking a transcript line centers the graph on its source node; runner
      advancement also follows the active node.
- [ ] Disable advancement after an evaluation error and transfer runner errors
      to the graph validation/error display when closed.
- [ ] Support and test the current condition vocabulary: `IS`, `ISNOT`, `EQ`,
      `NEQ`, `GT`, `GTE`, `LT`, `LTE`, `ALL`, `ANY`, `HAS_ITEM`, `ONCE`,
      `QUEST_IS_STARTED`, `QUEST_IS_COMPLETE`, and `QUEST_STEP_EQ`.
- [ ] Support and test the current executor vocabulary: `GET`, `SET_BOOL`,
      `SET_NUM`, `MOD_NUM`, `SET_STR`, `SETUP_DISPOSITION`, `START_QUEST`,
      `COMPLETE_QUEST_STEP`, `COMPLETE_QUEST`, `SPAWN_CH`, `DESPAWN_CH`,
      `CHANGE_TILE_AT`, `TELEPORT_TO`, `ADD_ITEM_AT`, `REMOVE_ITEM_AT`,
      `ADD_ITEM_TO_PLAYER`, `REMOVE_ITEM_FROM_PLAYER`, and `OPEN_SHOP`.
- [ ] Clearly identify simulator no-ops versus functions that mutate simulated
      storage; never imply the editor runner fully simulates game world effects.
- [ ] Make parser/evaluator errors actionable with node ID and source statement.

### Validation baseline

Whole-collection validation currently checks required event ID/title/type/icon,
duplicate event IDs, missing child IDs, and duplicate child IDs. Per-graph
validation checks exec `next` plus executor syntax, switch default/case exits,
and nonempty/link-complete choices.

`ceditor2` should retain those checks and add:

- [ ] Root/entry-node existence and reachability.
- [ ] Every connector target resolves to a child in the same event.
- [ ] Duplicate IDs are checked after trimming, with a documented case policy.
- [ ] Variable IDs/keys and imports are valid and non-conflicting.
- [ ] Event references from maps, characters, items, and imports resolve.
- [ ] Parser validation covers conditions and choice eval strings, not only exec
      nodes.
- [ ] Cycles consisting only of auto-advancing nodes cannot lock the runner via
      unbounded recursion.
- [ ] Unknown node/function/schema values are preserved and surfaced.

## Deep links, local persistence, autosave, and saving

### Current behavior matrix

| Area                  | Current behavior                                                                                                                                                                                                       | CEditor2 parity decision                                                                                                                           |
| --------------------- | ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- | -------------------------------------------------------------------------------------------------------------------------------------------------- |
| Deep links            | Hash query parameters select items, abilities, spells, characters, tilesets, events, maps, and map grids. Status effects have a registered parameter but the route does not pass it through. Feats have no live route. | Give each independent app a stable URL query parameter and test reload/direct-open behavior for every retained editor.                             |
| Selected entity       | One `localStorage` object (`ceditor.editorSelection`) stores the last selected ID per editor. URL selection takes precedence.                                                                                          | Retain last selection as a convenience; URL always wins; missing IDs show a useful state and do not erase data.                                    |
| Event graph view      | Node objects and pan/zoom are cached per event only in a module-global in-memory map.                                                                                                                                  | Retain within-session switching behavior. Persist view state locally only if it remains simple; graph coordinates themselves remain database data. |
| Recent events         | Kept only in React memory, newest-first, with ten displayed.                                                                                                                                                           | Session-only is sufficient unless user research asks for persistence.                                                                              |
| Form edits            | Immediately mutate the page's in-memory collection. No general dirty indicator or unload warning.                                                                                                                      | Track dirty database files, show dirty state, and warn before abandoning unsaved work.                                                             |
| Save buttons / hotkey | “Save All” and `Ctrl`/`Cmd+S` save only the current collection.                                                                                                                                                        | Save the complete validated database snapshot, as required by the CEditor2 plan.                                                                   |
| Event autosave        | A five-minute interval syncs the selected graph and writes the entire special-events collection directly to disk, without visible success/failure UI.                                                                  | Do not reproduce disk autosave by default. Consider local draft recovery separately; explicit Save All remains the commit boundary.                |
| Event rename          | Referenced maps, characters, and items are written sequentially before the updated events are saved later; imports change only in memory until an event save.                                                          | Perform rename plus every reference update in one previewed, revision-checked database transaction.                                                |

## Cross-reference consequences to cover centrally

The reference index should be complete and independent of whichever app is
open. At minimum it must cover:

| Target                              | Known incoming references                                                                                  | Required mutation behavior                                                                                            |
| ----------------------------------- | ---------------------------------------------------------------------------------------------------------- | --------------------------------------------------------------------------------------------------------------------- |
| Ability name                        | Spell `abilityName`; item weapon/use ability; status-effect triggered action                               | Preview rename/delete; update or intentionally clear every reference and remap positional attack/restore overrides.   |
| Ability attack index/order          | Item weapon/use-ability damage overrides                                                                   | Treat reorder/delete as a schema-aware command; positional arrays cannot be blindly spliced without a preview.        |
| Ability restore index/order         | Item use-ability restore overrides                                                                         | Remap corresponding overrides with the same care as attacks.                                                          |
| Status-effect name                  | Ability status applications; item equipped statuses                                                        | Preview and transactionally update rename/delete.                                                                     |
| Character name                      | Map character placements                                                                                   | Preview map/level/coordinate impacts and remove/update transactionally.                                               |
| Special-event ID                    | Map tile event triggers; character talk event; item use event; event variable imports                      | Update all four categories atomically on rename; preview all consequences on delete.                                  |
| Tileset name and tile index         | Map tiles/layers                                                                                           | Prevent or migrate breaking rename, source-image, tile-count, and tile-order changes.                                 |
| Map name                            | Map-grid cells, map travel links, and any other map references found by the Phase 0 database inventory     | Update atomically or block the operation with precise diagnostics.                                                    |
| Map-grid name/position              | Map grid-link metadata and neighboring-map behavior                                                        | Define and validate bidirectional ownership before allowing rename/delete/move.                                       |
| Sprite/picture/sound/animation name | Icons, portraits, tilesets, ability depictions, character sounds, event node audio, and other media fields | Flag missing media. Media renames are outside the current editor but must not be silently “fixed” by clearing values. |

## Known questionable behavior: do not copy automatically

- “Save All” is collection-local in every current page. This conflicts with the
  explicit `ceditor2` database-wide Save All contract.
- Current saves write individual JSON files directly and cross-file event rename
  writes are sequential. Partial failure can leave disk data inconsistent.
- The event editor's five-minute autosave writes to disk without checking dirty
  state, whole-database validity, revision conflicts, or clearly notifying the
  user.
- Spell-to-ability validation and ability-to-spell validation happen after the
  relevant collection has already been saved, so an error toast can describe
  invalid data that is already on disk.
- Several pages mix filtered-list indices with full-array indices during update
  or save-selection restoration (notably character, item, and tileset paths).
  This can update the wrong record or lose selection while a search filter is
  active. CEditor2 must use stable IDs for identity.
- Event editor state is a module-global singleton exposed on `window`, with a
  global rerender callback. Per-event caches hold mutable node instances and are
  not durable drafts.
- The event page installs its save key listener on every render (no dependency
  array), relying on effect cleanup to avoid overlap.
- Event runner parsing is intentionally small but permissive/inconsistent:
  string executor arguments use a naive comma split, equality mixes storage and
  literals in surprising ways, and values are coerced to strings. Lock semantics
  with fixtures before porting, then fix deliberately rather than incidentally.
- Event runner world-effect functions such as spawn, teleport, tile mutation,
  and shop opening are no-ops, but the UI does not strongly distinguish them
  from simulated storage mutations.
- Auto-advance uses recursive calls and has no cycle/step guard; a cycle can
  overflow or hang.
- Runner construction assumes at least one child when no `root` node exists.
- `replaceVariables(..., true)` replaces plain values before trying to add
  highlight markup, so highlighting is likely ineffective. Runner UI strips
  markup from choice labels anyway.
- Graph validation does not currently verify missing targets, reachability,
  condition syntax in all locations, imports, or auto-advance cycles.
- Deleting an event silently removes importing-variable records but does not
  clean map triggers, character talk references, or item use-event references.
- Event rename can be confirmed with reference updates, but changes to event
  imports are not written in the same sequence as maps/characters/items, and
  the new event document itself is not committed until a later Save.
- Event node deletion severs incoming exits, but copy/paste intentionally clears
  outgoing links to nodes outside the copied set. Confirm that this is the
  desired UX and explain it in copy feedback.
- There is no event undo/redo despite commented scaffolding.
- Centering on a node forcibly resets zoom to `1`; preserve only if users prefer
  it.
- Wheel zoom is throttled to 125 ms and jumps in `0.5` scale increments. This is
  implementation behavior, not a parity requirement.
- Tileset tile width, height, and name are disabled; choosing a new picture
  recreates the entire tile metadata array. This can destroy annotations and
  invalidate map tile IDs without a migration preview.
- Tileset selection logic contains edge cases (multi-selected tiles are only
  assembled when a single selected index is also set) and forces form remounts
  when switching tilesets. Preserve the workflow, not those mechanics.
- Adding one “resistance” in the status-effect form appends a full nine-damage-
  type block. Confirm whether the action means “add standard resistance set” or
  should add one row.
- Ability and item forms normalize/seed data inside selection-triggered effects.
  Merely opening an entity can therefore make silent edits. CEditor2 should
  separate read compatibility, explicit migration, and user edits.
- Feat prerequisite/effect/exclusion UI is commented out, the feat page is not
  routed by `App.tsx`, and the registered `feats.json` may be absent. Do not
  build speculative feat features before the asset contract is resolved.
- The current sound browser has no persistence or editing behavior and should
  not be generalized into a media-management system without an explicit need.

## Phase exit criteria for this audit area

Phase 0 is complete for form/special-event scope when:

- Every retained editor has an accepted checklist derived from this document.
- Fixtures cover one representative and one legacy/edge-case record per asset
  type, including every special-event node type actually present in the
  database.
- The reference inventory has been compared against real database occurrences,
  not just TypeScript types.
- Product decisions are recorded for feats, event disk autosave, unknown
  `KEYWORD` nodes, event simulation fidelity, tileset source-image replacement,
  and map-grid bidirectional ownership.
- Current questionable behaviors are represented by regression tests only when
  they are explicitly accepted semantics; otherwise tests assert the corrected
  behavior.
