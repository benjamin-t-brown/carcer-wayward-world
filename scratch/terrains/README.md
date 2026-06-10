# Terrain tileset generator

Open `index.html` in a browser (or serve this folder locally if file:// blocks image loads).

## Setup

Place source tiles in this folder (28×32 PNG):

- `grass.png`
- `dirt.png`
- `ocean.png`
- `cave_floor.png`
- `cave_wall.png`
- `cave_water.png`
- `snow.png`

You can also pick files with the file inputs if they are named differently.

## Usage

1. Choose **primary** (interior fill) and **secondary** (edge overlay) terrains.
2. Set **overlapSize** (default 10) — width/height of border strips composited onto the base.
3. The strip of 13 tiles is drawn at 1:1 on the top canvas. **Right-click → Copy image** and paste into Paint.NET, Aseprite, etc.
4. Use the legend for `tileTerrainBorderMeta` corner tags when configuring `terrain_borders` in ceditor.

Tile indices 0–12 match the first row of the `terrain_borders` tileset layout.
