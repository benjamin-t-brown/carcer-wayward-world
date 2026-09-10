# Phase 0 Summary

Status: Audit complete; interactive legacy measurements remain to be captured
before the Phase 4 map comparison gate.

This document is the integration point for CEditor2 Phase 0. Detailed audits
are maintained in:

- `database-contract.md`
- `map-editor-parity.md`
- `editor-parity.md`

## Current static baseline

The baseline was collected from the existing `ceditor` without modifying game
assets.

| Measurement                         |   Current value |
| ----------------------------------- | --------------: |
| Client TypeScript/TSX/HTML          |    41,313 lines |
| Client TypeScript/TSX only          |    39,288 lines |
| Tile editor TypeScript/TSX          |     9,860 lines |
| Special-event editor TypeScript/TSX |     8,239 lines |
| Inline CSS in `index.html`          |     1,691 lines |
| Inline TSX `style` properties       | 659 occurrences |
| React hook calls/imports            | 443 occurrences |
| Current map count                   |              25 |
| Current special-event count         |              33 |

These numbers describe the starting point; they are not line-count targets for
the rewrite. CEditor2 will be judged by dependency direction, explicit state
ownership, behavior, safety, and maintainability rather than by minimizing
lines at any cost.

## Runtime database observation

`src/db/Database.cpp::Database::load()` currently loads these nine JSON files:

1. `status-effects.json`
2. `abilities.json`
3. `items.json`
4. `spells.json`
5. `characters.json`
6. `maps.json`
7. `map-grids.json`
8. `tilesets.json`
9. `special-events.json`

The repository also contains `tiles.json`, which is not loaded by the runtime
database, while the current editor registers `feats.json`, which does not exist
and is described in its TypeScript types as editor-only.

The database-contract audit must classify these as one of:

- Runtime-managed
- Optional runtime-managed
- Editor-only/planned
- Legacy/read-only
- Unmanaged

Save All will cover the authoritative managed set. It must not create or
rewrite legacy/planned files merely because the old editor registry mentioned
them.

## Representative map fixtures

Static inspection of the current maps identifies these useful baseline cases:

| Map                     | Dimensions | Layers | Purpose                                               |
| ----------------------- | ---------: | -----: | ----------------------------------------------------- |
| `Alinea1`               |      40x40 |      2 | Largest total editable cell count in the current data |
| `alinea_outsideAlinea1` |      30x30 |      3 | Outdoor, multi-layer map                              |
| `alinea_insideAlinea8`  |      30x30 |      3 | Indoor, multi-layer map                               |
| `AlineaTest`            |      25x20 |      3 | Smaller multi-layer test map                          |

The map audit will add grid membership, feature coverage, and repeatable
interaction scenarios before these become the accepted benchmark fixtures.

## Local environment observation

At the start of Phase 0:

- Node: `v24.13.1`
- npm: `11.8.0`
- Existing `ceditor/node_modules`: absent
- Existing `ceditor/.npmrc` requires npm `>=11.10.0` because of its
  `min-release-age` configuration

As a result, the existing editor build was not used as a clean-build baseline
in this pass. CEditor2 tooling should state its own supported Node/npm versions
and should not inherit a higher npm requirement accidentally.

## Phase 0 gate

- [x] Initial authoritative runtime asset registry identified
- [x] C++/JSON/TypeScript contracts mapped
- [x] Load normalization and serialization behavior documented
- [x] Cross-asset reference matrix complete
- [x] Form-editor parity checklist complete
- [x] Map-editor parity checklist complete
- [x] Event-editor and runner parity checklist complete
- [x] Known questionable behavior separated from required parity
- [x] Golden fixture candidates selected
- [x] Map benchmark procedure reproducible
- [x] Phase 1 provisional decisions and unresolved questions recorded
- [ ] Raw interactive legacy map measurements captured

## Initial decisions carried into Phase 0

- Browser UI: native TypeScript and DOM APIs; no React or replacement UI
  framework.
- Development tooling may include Vite, TSX, Concurrently, ESLint, and
  Prettier.
- Local API may use Express.
- Navigation: independent Vite HTML entry points.
- Data availability: every editor loads the complete database snapshot.
- Persistence: Save All validates and commits the complete managed database.
- Styling: shared form styles live in core; map and event styles stay in their
  owning apps.
- Canvas: a simple continuous animation-frame loop is the default.
- Optimization: add scheduling complexity only in response to measured need.

## Provisional foundation decisions

These defaults let Phase 1 proceed. They remain visible and reversible if game
design or runtime work supplies new evidence.

1. The initial Save All registry contains the nine files loaded by
   `Database::load()`.
2. `tiles.json` is preserved as unmanaged legacy data. CEditor2 will not load,
   rewrite, delete, or silently merge it into tilesets.
3. Feats are excluded from the initial application and Save All registry until
   a runtime schema and consumer exist. The prospective type is documented but
   does not justify creating `feats.json`.
4. Loading is non-mutating. Current ability, spell, item, map, and grid
   normalizations must be separated into derived editor defaults or explicit,
   tested migrations.
5. The existing event disk-autosave interval will not be copied. Explicit Save
   All is the disk commit boundary; local draft recovery may be evaluated
   independently.
6. Existing dangling references are diagnostics, not automatic fixes:
   `PotionHealing -> HEAL_SELF` and map placements of
   `exampleTownsperson_2` remain visible until content intent is confirmed.
7. Negative map layers are preserved. CEditor2 will report that the current C++
   loader ignores them, but will not destroy them during load or Save All.
8. `COMMENT` event nodes are preserved as intentional editor-only data.
   Unsupported/unknown node types are preserved and diagnosed; newly creating
   unsupported types is not allowed.
9. Structured references participate in rename commands. DSL references are
   never changed with blind string replacement; supported DSL arguments will
   be added through parser-aware analysis.
10. Map-grid membership must not depend silently on array order. Phase 2 will
    validate duplicate grid membership, and the map application will require
    an explicit grid context until a single-grid invariant is confirmed.
11. Runtime-supported character fields remain preserved even before every field
    has editor UI. Exposure decisions belong to the character-editor parity
    increment.
12. Output will be deterministic. The final-newline convention will be chosen
    when serializer golden tests are introduced, with the expected one-time
    formatting impact made explicit.

## Known corpus diagnostics

Phase 0 found these issues in checked-in content or runtime compatibility:

- `PotionHealing.useAbility.abilityName` refers to missing `HEAL_SELF`.
- Maps place missing character `exampleTownsperson_2`.
- Five maps declare layer `-1`, while the current C++ map loader only loads
  numeric layer keys greater than or equal to zero.
- Special events contain 15 `COMMENT` nodes that the runtime intentionally
  discards after retaining their editor layout in JSON.
- The event loader silently skips children that fail to parse.
- Several destructive renames/deletes in the legacy editor do not update every
  known reference and do not save cross-file changes transactionally.

The complete evidence and field-level details are in the three audit documents.
