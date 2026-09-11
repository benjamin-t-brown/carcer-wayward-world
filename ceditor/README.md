# CEditor

The active web-based editor for managing Carcer game asset JSON files. CEditor
uses React, TypeScript, Vite, and a small local Express filesystem API. It is
being simplified incrementally rather than replaced; see [PLAN.md](./PLAN.md).

## Setup

1. Install dependencies:

```bash
npm install
```

2. Run the development server:

```bash
npm run dev
```

This will start:

- Express backend server on http://localhost:3001
- Vite dev server on http://localhost:3000

Open http://localhost:3000 in your browser to use the editor.

## Scripts

- `npm run dev` - Start both client and server in development mode
- `npm run dev:client` - Start only the Vite dev server
- `npm run dev:server` - Start only the Express server
- `npm run build` - Build for production
- `npm run typecheck` - Check the TypeScript project without emitting files
- `npm run test` - Run the editor's Node test suite
- `npm run check` - Run lint, formatting, tests, typecheck, and a production build
- `npm run preview` - Preview production build

## Architecture

- The client loads the complete managed database once. `DatabaseSession` owns
  its revision and stages complete collection replacements from every editor.
- Form editors keep ordinary React form state. The map and special-event canvas
  editors each use a page-owned controller; do not add module-global editor
  state or `window` rerender callbacks.
- Shared asset types live in `src/client/types`, while database transport types
  and the managed collection registry live in `src/shared`. Import asset types
  from those modules rather than re-exporting them through form components.
- Pure document, indexing, and viewport modules are the preferred boundary for
  editor logic that does not need React or browser events.

React and ReactDOM render the client; Express and CORS provide the local file
server. Vite, TypeScript/tsx, concurrently, ESLint, and Prettier are the small
build and development toolchain. All are directly used; avoid adding runtime
dependencies for behavior that the platform APIs already provide.

## Database API

The database transport is intentionally small:

- `GET /api/database` loads all managed collections plus their revision.
- `PUT /api/database` submits a revision and all managed collections. The
  server validates and stages the full snapshot, writes only changed files,
  and rolls ordinary partial-write failures back.
- `GET /api/sdl2w-assets` loads the generated media manifests.
- `GET /api/assets/img/*` and `GET /api/assets/snd/*` serve picker previews.

The former per-collection `/api/assets/:id` read/write endpoints no longer
exist. New editor saves must be staged through `DatabaseSession` so Save All
always has a coherent view of the database.

## Asset Types

The managed database contains abilities, characters, items, map grids, maps,
special events, spells, status effects, and tilesets. Save All submits those
nine collections as one revision-checked transaction.

`feats.json` is not currently a game asset, and the legacy `tiles.json` is not
managed by the editor. Neither is part of Save All.

The prior `ceditor2` experiment remains in the repository as implementation
reference material.

## Recovery

- **Database unavailable:** confirm both processes from `npm run dev` are
  running, then request `http://localhost:3001/api/database`. A valid response
  contains `revision` and `assets`; server-side parse/read failures name the
  managed file in the server log.
- **Revision conflict:** another process changed a managed file after this
  browser loaded it. The failed save leaves the browser draft dirty and does
  not overwrite disk. Preserve any values you need, reload to obtain the new
  revision, reapply the draft, and save again.
- **Write or rollback failure:** stop editing and inspect
  `git -C .. diff -- src/assets/db` before changing files manually. An ordinary
  write failure is rolled back automatically. If the server specifically
  reports an incomplete rollback, restore the affected files from a known-good
  version-control state before restarting CEditor.

Run `npm run check` after editor or persistence changes. Database transaction
tests use temporary directories; they must never point at the real asset
database.
