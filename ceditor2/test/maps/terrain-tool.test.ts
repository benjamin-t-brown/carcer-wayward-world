import assert from 'node:assert/strict';
import { readFile } from 'node:fs/promises';
import test from 'node:test';

import { MapWorkspace } from '../../src/apps/maps/MapWorkspace.js';
import type {
  GraphicCell,
  GraphicCellAccess,
  TileGraphic,
} from '../../src/apps/maps/history/cellPatches.js';
import {
  TerrainConfigurationError,
  TerrainMetadataLookup,
  TerrainPaintStroke,
  applyTerrainPaintPlan,
  buildTerrainMetadataLookup,
  computeTerrainPaintPlan,
  paintTerrainAt,
  terrainTagLabel,
} from '../../src/apps/maps/tools/terrainTool.js';
import { MapGridTopology } from '../../src/core/domain/mapGrids/index.js';
import { MapDocument } from '../../src/core/domain/maps/index.js';
import {
  parseTilesetCollection,
  type TilesetRecord,
} from '../../src/core/domain/tilesets/index.js';

class DocumentGraphics implements GraphicCellAccess {
  constructor(readonly documents: ReadonlyMap<string, MapDocument>) {}

  getGraphic(cell: GraphicCell): TileGraphic {
    const graphic = this.documents
      .get(cell.documentId)
      ?.readCell(cell.layer, cell.index);
    if (!graphic) throw new RangeError(`Unknown cell ${cell.documentId}`);
    return [graphic.tilesetIndex, graphic.tileIndex];
  }

  setGraphic(cell: GraphicCell, graphic: TileGraphic): void {
    const written = this.documents
      .get(cell.documentId)
      ?.writeCell(cell.layer, cell.index, {
        tilesetIndex: graphic[0],
        tileIndex: graphic[1],
      });
    if (!written) throw new RangeError(`Unknown cell ${cell.documentId}`);
  }
}

test('indexes the real terrain metadata, paintable tags, and NONE aliases', async () => {
  const lookup = await realLookup();

  assert.deepEqual(lookup.paintableTags, [
    'GRASS',
    'DIRT',
    'WATER',
    'CAVE_FLOOR',
    'CAVE_WALL',
    'CAVE_WATER',
    'CLIFF',
  ]);
  assert.equal(
    lookup.tileIdFor({ nw: 'GRASS', ne: 'GRASS', sw: 'GRASS', se: 'GRASS' }),
    0,
  );
  assert.equal(
    lookup.tileIdFor({
      nw: 'CAVE_WATER',
      ne: 'CAVE_WATER',
      sw: 'CAVE_WATER',
      se: 'CAVE_WATER',
    }),
    93,
  );
  // Grass/void uses the recorded grass/dirt variant.
  assert.equal(
    lookup.tileIdFor({ nw: 'GRASS', ne: 'NONE', sw: 'NONE', se: 'NONE' }),
    27,
  );
  // Dirt and water/void use their corresponding terrain/grass variants.
  assert.equal(
    lookup.tileIdFor({ nw: 'DIRT', ne: 'NONE', sw: 'NONE', se: 'NONE' }),
    22,
  );
  assert.equal(
    lookup.tileIdFor({ nw: 'WATER', ne: 'NONE', sw: 'NONE', se: 'NONE' }),
    6,
  );
  assert.equal(terrainTagLabel('CAVE_FLOOR'), 'Cave Floor');
});

test('lookup ownership follows tileset revisions rather than a global cache', () => {
  const first = new TerrainMetadataLookup(uniformTileset(4));
  const second = new TerrainMetadataLookup(uniformTileset(19));
  const grass = { nw: 'GRASS', ne: 'GRASS', sw: 'GRASS', se: 'GRASS' };

  assert.equal(first.tileIdFor(grass), 4);
  assert.equal(second.tileIdFor(grass), 19);
});

test('computes and applies a nine-cell terrain update across map partitions', async () => {
  const lookup = await realLookup();
  const west = map('west', ['', 'terrain0', 'terrain_borders']);
  const east = map('east', ['', 'terrain_borders', 'terrain0']);
  const documents = new Map([
    ['west', west],
    ['east', east],
  ]);
  const access = new DocumentGraphics(documents);
  const workspace = gridWorkspace(west, documents);

  const plan = computeTerrainPaintPlan(
    access,
    workspace,
    lookup,
    0,
    { x: 1, y: 0 },
    'GRASS',
  );

  assert.equal(plan.changes.length, 6);
  assert.deepEqual(plan.issues, []);
  assert.deepEqual(west.readCell(0, 1), { tilesetIndex: 2, tileIndex: 13 });
  assert.ok(plan.changes.some(({ cell }) => cell.documentId === 'east'));
  assert.ok(
    plan.changes
      .filter(({ cell }) => cell.documentId === 'east')
      .every(({ graphic }) => graphic[0] === 1),
  );

  const command = applyTerrainPaintPlan(access, plan);
  assert.equal(command.patches.length, 6);
  assert.deepEqual(west.readCell(0, 1), { tilesetIndex: 2, tileIndex: 0 });
  assert.deepEqual(west.readCell(0, 3), { tilesetIndex: 2, tileIndex: 2 });
  assert.deepEqual(east.readCell(0, 0), { tilesetIndex: 1, tileIndex: 3 });
  assert.deepEqual(east.readCell(0, 2), { tilesetIndex: 1, tileIndex: 11 });

  command.undo(access);
  for (const document of [west, east]) {
    for (let index = 0; index < 4; index += 1) {
      assert.equal(document.readCell(0, index)?.tileIndex, 13);
    }
  }
  command.redo(access);
  assert.equal(east.readCell(0, 0)?.tileIndex, 3);
});

test('continuous strokes visit each center once and coalesce shared neighbors', async () => {
  const lookup = await realLookup();
  const west = map('west', ['', 'terrain_borders']);
  const east = map('east', ['', 'terrain0', 'terrain_borders']);
  const documents = new Map([
    ['west', west],
    ['east', east],
  ]);
  const access = new DocumentGraphics(documents);
  const stroke = new TerrainPaintStroke(
    access,
    gridWorkspace(west, documents),
    lookup,
    0,
    'GRASS',
  );

  stroke.paint({ x: 1, y: 0 });
  stroke.paint({ x: 2, y: 0 });
  assert.deepEqual(stroke.paint({ x: 1, y: 0 }), {
    changes: [],
    issues: [],
  });
  const command = stroke.finish();

  assert.equal(
    new Set(
      command.patches.map(
        ({ cell }) => `${cell.documentId}/${cell.layer}/${cell.index}`,
      ),
    ).size,
    command.patches.length,
  );
  assert.ok(command.patches.some(({ cell }) => cell.documentId === 'west'));
  assert.ok(command.patches.some(({ cell }) => cell.documentId === 'east'));
  command.undo(access);
  assert.ok(
    command.patches.every(({ cell }) => access.getGraphic(cell)[1] === 13),
  );
});

test('missing neighbor variants are reported and left untouched', () => {
  const lookup = new TerrainMetadataLookup(uniformTileset(7));
  const solo = map('solo', ['', 'terrain_borders'], [0, 0]);
  const documents = new Map([['solo', solo]]);
  const access = new DocumentGraphics(documents);
  const workspace = new MapWorkspace(solo, documents, []);

  const { command, issues } = paintTerrainAt(
    access,
    workspace,
    lookup,
    0,
    { x: 0, y: 0 },
    'GRASS',
  );

  assert.equal(command.patches.length, 1);
  assert.equal(issues.length, 3);
  assert.ok(issues.every(({ reason }) => reason === 'missing-variant'));
  assert.deepEqual(solo.readCell(0, 0), { tilesetIndex: 1, tileIndex: 7 });
  assert.deepEqual(solo.readCell(0, 1), { tilesetIndex: 0, tileIndex: 0 });
});

test('configuration failures happen before any map mutation', async () => {
  const lookup = await realLookup();
  const noTerrain = map('plain', ['', 'terrain0'], [1, 13]);
  const documents = new Map([['plain', noTerrain]]);
  const access = new DocumentGraphics(documents);
  const workspace = new MapWorkspace(noTerrain, documents, []);

  assert.throws(
    () => paintTerrainAt(access, workspace, lookup, 0, { x: 0, y: 0 }, 'GRASS'),
    (error: unknown) =>
      error instanceof TerrainConfigurationError &&
      /does not reference/.test(error.message),
  );
  assert.deepEqual(noTerrain.readCell(0, 0), {
    tilesetIndex: 1,
    tileIndex: 13,
  });

  assert.throws(
    () => buildTerrainMetadataLookup([]),
    /terrain_borders.*not found/,
  );
  assert.throws(
    () => new TerrainPaintStroke(access, workspace, lookup, 0, 'SNOW'),
    /Base terrain tile not found/,
  );
});

async function realLookup(): Promise<TerrainMetadataLookup> {
  const source = JSON.parse(
    await readFile('../src/assets/db/tilesets.json', 'utf8'),
  ) as unknown;
  return buildTerrainMetadataLookup(parseTilesetCollection(source));
}

function uniformTileset(tileId: number): TilesetRecord {
  return {
    name: 'terrain_borders',
    tiles: [
      {
        id: tileId,
        tileTerrainBorderMeta: {
          nw: 'GRASS',
          ne: 'GRASS',
          sw: 'GRASS',
          se: 'GRASS',
        },
      },
    ],
  };
}

function map(
  name: string,
  tilesets: string[],
  graphic: TileGraphic = [tilesets.indexOf('terrain_borders'), 13],
): MapDocument {
  return MapDocument.from({
    name,
    width: 2,
    height: 2,
    spriteWidth: 28,
    spriteHeight: 32,
    tilesets,
    layers: [0],
    tiles: {
      '0': Array.from({ length: 4 }, () => [...graphic]).flat(),
    },
  });
}

function gridWorkspace(
  focus: MapDocument,
  documents: ReadonlyMap<string, MapDocument>,
): MapWorkspace {
  return new MapWorkspace(focus, documents, [
    MapGridTopology.from({
      name: 'world',
      gridWidth: 2,
      gridHeight: 1,
      mapWidth: 2,
      mapHeight: 2,
      cells: [['west', 'east']],
    }),
  ]);
}
