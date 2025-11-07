# Ceditor

Web-based editor for managing game asset JSON files.

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

- Item Templates (`items.json`)
- Character Templates (`characters.json`)
- Special Events (`special-events.json`)

