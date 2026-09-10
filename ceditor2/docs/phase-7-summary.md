# Phase 7 Summary: Special-Event Editor

Status: native event editor and isolated runtime model complete; final browser
walkthrough remains in Phase 8

## Delivered

- A lossless event parser/document model preserves unknown event, node,
  variable, branch, audio, and forward-compatible fields.
- The independent native event page supports search, deep links, recent-event
  restoration, create, clone, rename, delete, and a raw JSON preview.
- Event rename/delete actions preview structured references in maps,
  characters, items, and other event imports, then stage the chosen updates in
  the same database session for Save All.
- One continuously rendered canvas supports fit, pan, zoom, reverse-order hit
  testing, box/shift multiselect, group drag, node create/delete, and
  graph-aware copy/paste with remapped internal links.
- Focused fields cover EXEC, CHOICE, SWITCH, END, COMMENT, and KEYWORD data.
  Raw per-node JSON keeps every field and unknown node type editable without a
  speculative component hierarchy.
- Graph analysis reports duplicate IDs, broken targets, missing roots,
  unreachable nodes, and runtime-unsupported types. Diagnostics can locate a
  referenced node on the canvas.
- Variables and imports are editable and validated independently of the UI.
- The domain-only event runner follows the C++ execution model for EXEC,
  CHOICE, SWITCH, and END nodes. Evaluation, execution, and once-key storage
  are injected callbacks, keeping game-state mutation out of the editor.

The browser intentionally does not simulate game state or execute event
commands. It validates graph structure while the runner remains a reusable,
tested domain boundary. This avoids maintaining a second partial game runtime
inside CEditor2.

## Real-database audit

All 33 checked-in special events and 554 nodes parse and round-trip exactly.
They contain 387 EXEC, 72 CHOICE, 42 END, 38 SWITCH, and 15 editor-only COMMENT
nodes. Graph validation reports no errors and variable import resolution
reports no issues. With deterministic true and false callback policies, every
event reaches its first continue/choice/end stop without a runner error or step
limit.

The audit found existing authored/runtime DSL mismatches which CEditor2 does
not rewrite:

- `IS_NOT(...)` appears once, while the C++ evaluator exposes `ISNOT`.
- `OPEN_SHOP` and `ADD_ITEM_TO_PLAYER` each appear four times; corresponding
  evaluator helpers exist in C++, but the current dispatcher does not route
  those command names.

These are game-runtime/data issues, not graph-format issues. They should be
resolved with the evaluator and authored content together.

## Verification

- 24 event domain, lifecycle, editor-model, Save All, geometry, and runner
  tests pass.
- The real database has permanent exact-round-trip, graph/import, and
  deterministic runner probes.
- Strict TypeScript, ESLint, and Prettier checks pass for the complete event
  slice.
- The architecture test prevents the event app from importing map, item,
  character, or any other application code.
