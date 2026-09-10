# Phase 5 Summary: Form Editors

Status: native editor vertical slices complete; browser parity review remains
part of Phase 8

## Delivered

- Abilities: identity, cost, targeting, depiction, attacks, status effects,
  restores, direct damage, dice, and optional saves
- Spells: identity, ability reference, icon, description, and ordered rune
  requirements
- Items: loader-aligned type transitions, usability, rune/weapon fields,
  ability and event references, status effects, and damage/restore overrides
- Characters/NPCs: identity, type, sprite, stats, talk, behavior, combat,
  combat behavior, sound, statuses, and vision
- Tilesets: image/tile dimensions, sprite base, metadata reconciliation, flags,
  step sound, and optional terrain corners
- Map grids: scalable search and metadata, direct partition inspection, safe
  deletion that retains standalone maps, and atomic grid creation with one
  randomly named blank map per cell
- Sound/media browser: sorted name/path search and play/stop preview over the
  read-only SDL2W catalog

Status effects remain the first completed form slice from Phase 3. Feats are
not retained because there is no managed feat database file or game-loader
contract in the current asset registry.

## Architecture

Every editor is an independent Vite entry and mounts the shared database page
shell. Form pages use native DOM controls and the shared form stylesheet. Only
the tileset, map-grid, and sound pages add local presentation CSS. No editor
imports another editor and no runtime UI framework was added.

Each domain parser returns a detached record while preserving unknown fields.
Create and clone helpers generate loader-compatible records, and each page
replaces only its owned collection in `DatabaseSession`. Save All still
validates and submits all nine managed database files in one revision-checked
transaction.

Reference controls read options from the full session snapshot. The existing
cross-database validator remains authoritative for unresolved references;
cross-file rename/delete previews continue into the hardening/reference pass.

## Verification

- 70 focused form, grid, and media tests pass.
- The real ability, spell, item, character, tileset, and map-grid collections
  parse without normalization.
- Character and item fixture edits are covered by whole-database Save All
  tests.
- Targeted ESLint and Prettier checks pass for all Phase 5 paths.
- Each completed vertical slice has passed strict TypeScript checking and a
  production Vite build during implementation.

Phase 8 owns the interactive browser walkthrough, keyboard/accessibility pass,
reference-consequence acceptance, and final checklist reconciliation.
