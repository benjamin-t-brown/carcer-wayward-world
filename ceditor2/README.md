# CEditor2

CEditor2 is the lightweight, modular replacement for `ceditor`. Each editor is
an independent Vite entry point built with TypeScript and native browser APIs.
It is ready for verification on real editing workflows; the existing editor
remains available as a deprecated migration reference until default-editor
approval.

See [PLAN.md](./PLAN.md) for the phased implementation plan and
[docs/phase-8-summary.md](./docs/phase-8-summary.md) for implementation and
verification status.

## Requirements

- Node.js 20.19 or newer
- npm 10 or newer

## Commands

```sh
npm install
npm run dev
```

The Vite client listens on `http://127.0.0.1:3000`. The local Express API
listens on `http://127.0.0.1:3001` and is reached through Vite's proxy.

```sh
npm run format       # format source and documentation
npm run lint         # run correctness linting
npm test             # run TypeScript tests with Node's test runner
npm run build        # typecheck and build every HTML entry
npm run check        # run every verification step
npm run preview      # preview the production build with the local API
```

Open `/pages/maps/` for standalone and continuous map-grid editing. The other
entries under `/pages/` independently manage abilities, spells, items,
characters, status effects, tilesets, map grids, special events, and sounds.
Every page sees one shared in-memory database snapshot; **Save All** validates
and commits all managed JSON assets together.

## Dependency rule

Applications under `src/apps` may import modules under `src/core`. They must
not import another application. Shared code must be application-neutral.

The browser client intentionally has no UI framework, router, state manager,
form framework, canvas framework, or CSS framework. Build and server tooling is
allowed when it makes development and maintenance clearer.
