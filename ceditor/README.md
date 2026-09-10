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
- `npm run preview` - Preview production build

## Features

- Load asset JSON files from `src/assets/db`
- Edit asset data
- Save changes back to files

## Asset Types

The managed database contains abilities, characters, items, map grids, maps,
special events, spells, status effects, and tilesets. Save All will be migrated
to commit those nine collections as one revision-checked transaction.

`feats.json` is not currently a game asset, and the legacy `tiles.json` is not
managed by the editor. Neither is part of Save All.

The prior `ceditor2` experiment remains in the repository as implementation
reference material.
