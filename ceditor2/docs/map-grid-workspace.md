# Map-grid workspace design

Status: accepted for the CEditor2 map editor.

## User model

The editor has two map workspaces:

- **Standalone map** edits one finite map.
- **Map grid** presents all of a grid's finite maps as one continuous world.

Partition boundaries are an implementation detail while painting. The map
inspector may expose the partition under the pointer and direct map metadata,
but users do not have to open neighboring maps to edit across a boundary.

This does not add a merged-map asset. `maps.json` and `map-grids.json` remain
the authoritative files consumed by the game.

## Coordinate conversion

Grid-wide tile coordinates resolve without searching every map:

1. Divide by `mapWidth` and `mapHeight` to find the grid cell.
2. Read the map API name from `cells[cellY][cellX]`.
3. Subtract the cell origin to find map-local tile coordinates.
4. Convert the local coordinate to the map's existing dense-array index.

Canvas world units are pixels. A grid cell at `(cellX, cellY)` begins at:

```text
x = (cellX - anchorCellX) * mapWidth  * spriteWidth
y = (cellY - anchorCellY) * mapHeight * spriteHeight
```

This conversion is constant time and does not change saved data.

## Rendering and memory

The renderer continuously redraws. Before each frame, it converts the canvas
viewport into a grid-cell rectangle, adds one partition of overscan, clamps the
rectangle to grid bounds, and enumerates only those cells. The tile renderer
then performs its existing per-map visible-tile culling.

Work per frame therefore follows the number of visible partitions and visible
tiles, not the total grid dimensions. There is no radius centered on a
"current" map and no merged dense tile array.

Save All still requires the complete JSON database snapshot. Map documents
retain compact dense numeric arrays and sparse metadata; the editor must not
materialize a per-tile object graph or duplicate an entire grid for rendering.

## Editing and history

Every visible, structurally compatible partition is editable. Hit testing
returns both the backing map and its local dense index. A drag may collect
patches for several maps, but it produces one undo command. Undo, redo, and Save
All therefore match the continuous-world interaction while still mutating the
original map records.

A partition is context-only when it is missing, malformed, uses incompatible
tile dimensions, lacks the selected layer, or conflicts with another placement.
The UI must state the reason rather than silently writing the wrong map.

## Grid creation

Creating a grid eagerly creates one blank map per cell. Each backing map gets a
random API-safe name generated once, checked against all existing map names,
and stored in the grid's `cells` matrix. Coordinates and display labels may
change later without changing those API names.

The creation result contains the new grid and every new map. The editor stages
both collections in the database session and Save All commits them together.
Standalone-map creation continues to use the same blank-map factory without a
grid record.

The creation dialog should show the partition count and estimated dense tile
pair count before confirming a very large grid. This is an explicit content
cost, not a rendering cost.

## Metadata

The inspector separates:

- grid metadata: name, label, dimensions, partition dimensions;
- partition metadata: backing map API name, label, type, layers, tilesets, and
  sparse placements.

Ordinary canvas work stays in grid coordinates. Direct partition metadata is
the deliberate escape hatch for data that the C++ runtime still owns per map.

## Compatibility invariants

- No C++ loader changes are required.
- No new runtime asset is introduced.
- Grid cell order remains row-major `cells[y][x]`.
- Map tile arrays remain dense map-local arrays.
- Unknown JSON fields survive untouched edits.
- Save All remains one revision-checked transaction for the entire database.
