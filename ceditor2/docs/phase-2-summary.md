# Phase 2 Summary

Status: Complete

## Delivered

- One authoritative registry for the nine database files loaded by the game
- A full-snapshot `DatabaseClient` and isolated `DatabaseSession`
- Dirty tracking by asset collection, including edits made during an active save
- Structural, cross-reference, map, map-grid, event-graph, and status-effect
  validation
- A queryable cross-reference index with inbound, outbound, and unresolved views
- Runtime-compatible status-effect types, parsing, defaults, and clone helpers
- SDL2W picture, sprite, animation, and sound parsing with lookup indexes
- SHA-256 disk revisions and stale-session conflict rejection
- Deterministic two-space JSON serialization with final newlines
- A staged Save All transaction that replaces only changed files and rolls back
  already-replaced files when a later replacement fails
- A shared native page controller for load state, validation, Save All,
  Ctrl/Cmd+S, dirty labels, conflict feedback, and unload warnings

The database and media APIs are intentionally separate. `GET /api/database`
returns only the revisioned database snapshot, while `GET /api/media-sources`
returns the read-only SDL2W definition files. Media definitions are not part of
the writable database transaction.

## Managed database boundary

Save All manages exactly:

1. `status-effects.json`
2. `abilities.json`
3. `items.json`
4. `spells.json`
5. `characters.json`
6. `maps.json`
7. `map-grids.json`
8. `tilesets.json`
9. `special-events.json`

Legacy `tiles.json` is neither loaded nor written. Automated coverage verifies
that it remains byte-for-byte unchanged through a successful Save All.

## Live database verification

The complete current database loads without mutation under a stable 64-character
revision. It contains:

| Collection     | Records |
| -------------- | ------: |
| Status effects |       1 |
| Abilities      |       8 |
| Items          |      18 |
| Spells         |       3 |
| Characters     |       9 |
| Maps           |      25 |
| Map grids      |       2 |
| Tilesets       |       4 |
| Special events |      33 |

Validation reports zero blocking errors and seven previously audited warnings:
five negative map layers that the current C++ loader ignores, the missing
`HEAL_SELF` ability reference, and the missing `exampleTownsperson_2` character
reference. Warnings are surfaced but do not prevent preserving and saving the
existing database.

## Transaction guarantees

The server validates the complete submitted snapshot and checks its base
revision before writing. It serializes and parse-checks all nine managed files
inside a hidden transaction directory, backs up every changed target, then
replaces changed files. A failed replacement restores every earlier replacement.
A cleanup failure after a successful commit is logged without falsely reporting
that the already-committed save failed.

This is a recoverable multi-file transaction, not a claim of filesystem-level
atomicity across nine paths. The revision check prevents one editor session from
silently overwriting changes made on disk after it loaded.

## Phase 2 exit gate

- [x] The complete current database loads without changing source files.
- [x] A no-edit fixture round trip is byte-preserving.
- [x] A one-file edit saves through the complete database transaction.
- [x] Cross-reference errors block saves with the failing JSON path.
- [x] A stale session receives HTTP 409 without writing.
- [x] A simulated second-file commit failure restores the first file.
- [x] Save All UI and keyboard behavior are shared without coupling apps.
- [x] Dirty state and unload protection are shared across editor pages.
- [x] Media definitions load without becoming writable database state.
- [x] The full lint, format, test, typecheck, and production-build check passes.

Domain schemas and normalizers will continue to be added one asset family at a
time. Keeping those rules beside the editor being migrated avoids copying the
legacy application's all-domain type bundle into the new shared core.

## Next phase

Phase 3 completes the native status-effect editor as the first end-to-end
vertical slice and uses it to evaluate which list-and-form patterns are truly
shared before extracting any broader editor abstraction.
