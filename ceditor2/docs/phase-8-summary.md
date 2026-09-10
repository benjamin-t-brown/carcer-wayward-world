# Phase 8 Summary: Hardening and Cutover Readiness

Status: automated hardening complete; interactive acceptance and default-editor
approval remain open

## Verification result

`npm run check` passes end to end:

- ESLint: pass
- Prettier check: pass
- Node test suite: 224/224 pass
- strict TypeScript check: pass
- production Vite build: pass, 120 modules transformed

The test suite includes local HTTP API behavior, stale-revision rejection,
database validation, failed multi-file rollback, dirty edits made during an
active save, application import boundaries, independent HTML entries, and the
absence of browser runtime frameworks.

## Real-database qualification

The complete checked-in nine-file database passes validation and a golden
JSON round trip. A temporary byte-for-byte copy also passes this sequence:

1. load the complete database;
2. Save All without edits and retain every original file byte;
3. edit an item, map, and event in one snapshot;
4. atomically save exactly those three files;
5. reload an equal complete snapshot and validate it again.

The live game database is never written by tests. `tiles.json` and media source
files remain unmanaged.

An important hardening fix came from this test: the transaction now reuses the
original bytes for every semantically unchanged collection. Therefore legacy
formatting does not cause a no-edit Save All to rewrite all files.

## Size and dependency audit

Express 5 is the only production dependency, and it is server-only. The client
has no React or other UI/state/form/router/canvas framework. Build-only tooling
is TypeScript, Vite, TSX, concurrently, ESLint, and Prettier.

The production build is approximately 266 KB uncompressed across all emitted
HTML, CSS, and JavaScript before shared-browser caching. The largest independent
application chunks are:

| Entry  | Raw JS | Gzip JS |
| ------ | -----: | ------: |
| Maps   |  57 KB |   17 KB |
| Events |  36 KB |   11 KB |

Shared CSS is limited to the page shell and form system. Custom map, event,
tileset, map-grid, and sound rules remain in their editor directories.

## Map performance

The deterministic render benchmark retains its visual hashes and reports:

| Scenario                    | Visited tiles |       p50 |       p95 |
| --------------------------- | ------------: | --------: | --------: |
| 24×18 full map              |           432 | 0.0140 ms | 0.0450 ms |
| 128×128 representative view |           840 | 0.0291 ms | 0.0737 ms |
| 512×512 zoomed out          |         9,940 | 0.3345 ms | 0.3855 ms |

A separate 1,000×1,000 map-grid test proves that scene enumeration inspects
only nine cells for one focused partition plus one-cell overscan. Total grid
area is not part of the render or pointer-hit hot path.

These microbenchmarks validate the core canvas traversal. Browser frame/input
profiles from the documented Phase 4 protocol remain a manual acceptance item.

## Cutover disposition

Repository documentation now points new database-editor work to CEditor2 and
marks `ceditor` as a deprecated comparison implementation. It has not been
deleted, disabled, or redirected. Making CEditor2 the sole/default editor and
later removing CEditor require explicit user approval after real browser
editing sessions.

Before that approval, perform the browser walkthrough in
[`phase-4-browser-verification.md`](./phase-4-browser-verification.md), with
special attention to:

- high-DPI canvas pointer alignment and long continuous grid strokes;
- keyboard/focus behavior and readable labels at narrow widths;
- representative form edits across every asset page;
- event graph multiselect, copy/paste, and reference-selection dialogs;
- a Save All on a disposable database branch followed by loading the result in
  the C++ game.

The event audit also found pre-existing authored/runtime DSL mismatches
(`IS_NOT`, `OPEN_SHOP`, and `ADD_ITEM_TO_PLAYER`). They are documented in the
Phase 7 summary and intentionally not rewritten during editor cutover.
