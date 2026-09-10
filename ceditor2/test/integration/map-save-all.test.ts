import assert from 'node:assert/strict';
import { mkdtemp, readFile, rm, writeFile } from 'node:fs/promises';
import { tmpdir } from 'node:os';
import { join } from 'node:path';
import test from 'node:test';

import {
  ASSET_REGISTRY,
  DatabaseSession,
  type DatabaseEnvelope,
  type DatabaseSnapshot,
  type DatabaseTransport,
  type JsonArray,
  type SaveDatabaseRequest,
  type SaveDatabaseResponse,
} from '../../src/core/database/index.js';
import {
  MapDocument,
  parseMapCollection,
  type MapRecord,
} from '../../src/core/domain/maps/index.js';
import { MapGridTopology } from '../../src/core/domain/mapGrids/index.js';
import {
  buildMapScene,
  hitTestMapScene,
} from '../../src/apps/maps/MapScene.js';
import { BoundedHistory } from '../../src/apps/maps/history/BoundedHistory.js';
import type {
  GraphicCell,
  GraphicCellAccess,
  TileGraphic,
} from '../../src/apps/maps/history/cellPatches.js';
import { createPencilGesture } from '../../src/apps/maps/tools/paintGesture.js';
import { translateTileGraphic } from '../../src/apps/maps/tools/tileGraphic.js';
import {
  DatabaseRepository,
  type DatabaseRepositoryContract,
} from '../../src/server/databaseRepository.js';

const UNMANAGED_TILES = '[{"legacy":"must remain byte-identical"}]\n';

class RepositoryTransport implements DatabaseTransport {
  constructor(private readonly repository: DatabaseRepositoryContract) {}

  loadDatabase(): Promise<DatabaseEnvelope> {
    return this.repository.load();
  }

  saveDatabase(request: SaveDatabaseRequest): Promise<SaveDatabaseResponse> {
    return this.repository.save(request);
  }
}

class MapGraphicAccess implements GraphicCellAccess {
  constructor(private readonly documents: ReadonlyMap<string, MapDocument>) {}

  getGraphic(cell: GraphicCell): TileGraphic {
    const graphic = this.documents
      .get(cell.documentId)
      ?.readCell(cell.layer, cell.index);
    if (!graphic) {
      throw new RangeError(`Invalid map cell: ${JSON.stringify(cell)}`);
    }
    return [graphic.tilesetIndex, graphic.tileIndex];
  }

  setGraphic(cell: GraphicCell, graphic: TileGraphic): void {
    const changed = this.documents
      .get(cell.documentId)
      ?.writeCell(cell.layer, cell.index, {
        tilesetIndex: graphic[0],
        tileIndex: graphic[1],
      });
    if (!changed) {
      throw new RangeError(`Invalid map cell: ${JSON.stringify(cell)}`);
    }
  }
}

test('map paint survives full Save All and reload without collateral writes', async () => {
  const databasePath = await mkdtemp(join(tmpdir(), 'ceditor2-map-save-all-'));

  try {
    const initialSnapshot = createFixtureSnapshot();
    await writeFixtureDatabase(databasePath, initialSnapshot);
    const before = await readAllFiles(databasePath);

    const transport = new RepositoryTransport(
      new DatabaseRepository(databasePath),
    );
    const session = await DatabaseSession.load(transport);
    const initialRevision = session.baseRevision;
    const documents = parseMapCollection(session.collection('maps')).map(
      (record, index) => MapDocument.from(record, `maps[${index}]`),
    );
    const documentByName = new Map(
      documents.map((document) => [document.name, document]),
    );
    const access = new MapGraphicAccess(documentByName);
    const history = new BoundedHistory<GraphicCellAccess>();
    const target: GraphicCell = {
      documentId: 'SAVE_TEST_MAP',
      layer: 0,
      index: 2,
    };

    const gesture = createPencilGesture(access, [2, 99]);
    assert.equal(gesture.visit(target), true);
    assert.equal(history.recordApplied(gesture.finish()), true);
    assert.deepEqual(access.getGraphic(target), [2, 99]);

    assert.equal(history.undo(access)?.label, 'Pencil stroke');
    assert.deepEqual(access.getGraphic(target), [1, 12]);
    assert.equal(history.redo(access)?.label, 'Pencil stroke');
    assert.deepEqual(access.getGraphic(target), [2, 99]);

    session.replaceCollection(
      'maps',
      documents.map((document) => document.snapshot()) as JsonArray,
    );
    assert.deepEqual([...session.dirtyAssetIds], ['maps']);

    const save = await session.saveAll();
    assert.deepEqual(save.changedFiles, ['maps.json']);
    assert.notEqual(save.revision, initialRevision);
    assert.equal(session.baseRevision, save.revision);
    assert.equal(session.isDirty, false);

    const after = await readAllFiles(databasePath);
    for (const { fileName } of ASSET_REGISTRY) {
      if (fileName === 'maps.json') {
        assert.notEqual(after.get(fileName), before.get(fileName));
      } else {
        assert.equal(after.get(fileName), before.get(fileName));
      }
    }
    assert.equal(after.get('tiles.json'), UNMANAGED_TILES);
    assert.equal(after.get('tiles.json'), before.get('tiles.json'));

    const reloadedSession = await DatabaseSession.load(transport);
    assert.equal(reloadedSession.baseRevision, save.revision);
    const [reloadedMap] = parseMapCollection(
      reloadedSession.collection('maps'),
    );
    assert.ok(reloadedMap);
    const reloadedDocument = MapDocument.from(reloadedMap, 'maps[0]');
    assert.deepEqual(reloadedDocument.readCell(0, 2), {
      tilesetIndex: 2,
      tileIndex: 99,
    });
    assert.deepEqual(reloadedMap.layers, [0, -1]);
    assert.deepEqual(reloadedMap.tiles['-1'], [2, 20, 2, 21, 2, 22, 2, 23]);
    assert.deepEqual(reloadedMap.futureMapData, {
      preserve: true,
      version: 3,
    });
    assert.equal(
      reloadedMap.markers?.[0]?.futurePlacementData,
      'also preserved',
    );
  } finally {
    await rm(databasePath, { recursive: true, force: true });
  }
});

test('one cross-grid gesture saves and reloads both map documents atomically', async () => {
  const databasePath = await mkdtemp(join(tmpdir(), 'ceditor2-grid-save-all-'));

  try {
    await writeFixtureDatabase(databasePath, createFixtureSnapshot());
    const before = await readAllFiles(databasePath);
    const transport = new RepositoryTransport(
      new DatabaseRepository(databasePath),
    );
    const session = await DatabaseSession.load(transport);
    const documents = parseMapCollection(session.collection('maps')).map(
      (record, index) => MapDocument.from(record, `maps[${index}]`),
    );
    const documentsByName = new Map(
      documents.map((document) => [document.name, document]),
    );
    const focus = documentsByName.get('SAVE_TEST_MAP');
    const neighbor = documentsByName.get('SAVE_TEST_NEIGHBOR');
    assert.ok(focus);
    assert.ok(neighbor);
    const [gridValue] = session.collection('mapGrids');
    const grid = MapGridTopology.from(gridValue, 'mapGrids[0]');
    const scene = buildMapScene(focus, documentsByName, [grid], {
      worldBounds: { left: 0, top: 0, right: 112, bottom: 64 },
      overscanCells: 0,
    });
    const focusHit = hitTestMapScene(scene, 1, 1, 0);
    const neighborHit = hitTestMapScene(scene, 57, 1, 0);
    assert.ok(focusHit?.block.editable);
    assert.ok(neighborHit?.block.editable);

    const access = new MapGraphicAccess(documentsByName);
    const focusGraphic = translateTileGraphic(focus, focus, 1, 77);
    const neighborGraphic = translateTileGraphic(focus, neighbor, 1, 77);
    assert.ok(focusGraphic);
    assert.ok(neighborGraphic);
    assert.deepEqual(focusGraphic, [1, 77]);
    assert.deepEqual(neighborGraphic, [2, 77]);
    const gesture = createPencilGesture(access, focusGraphic);
    assert.equal(gesture.visit(focusHit.cell), true);
    assert.equal(gesture.visit(neighborHit.cell, neighborGraphic), true);
    const history = new BoundedHistory<GraphicCellAccess>();
    assert.equal(history.recordApplied(gesture.finish()), true);
    assert.equal(history.undoDepth, 1);
    assert.equal(history.undo(access)?.label, 'Pencil stroke');
    assert.deepEqual(focus.readCell(0, 0), {
      tilesetIndex: 1,
      tileIndex: 10,
    });
    assert.deepEqual(neighbor.readCell(0, 0), {
      tilesetIndex: 2,
      tileIndex: 30,
    });
    assert.equal(history.redo(access)?.label, 'Pencil stroke');
    assert.deepEqual(focus.readCell(0, 0), {
      tilesetIndex: 1,
      tileIndex: 77,
    });
    assert.deepEqual(neighbor.readCell(0, 0), {
      tilesetIndex: 2,
      tileIndex: 77,
    });

    session.replaceCollection(
      'maps',
      documents.map((document) => document.snapshot()) as JsonArray,
    );
    const save = await session.saveAll();
    assert.deepEqual(save.changedFiles, ['maps.json']);

    const after = await readAllFiles(databasePath);
    for (const { fileName } of ASSET_REGISTRY) {
      if (fileName !== 'maps.json')
        assert.equal(after.get(fileName), before.get(fileName), fileName);
    }
    assert.equal(after.get('tiles.json'), before.get('tiles.json'));

    const reloaded = await DatabaseSession.load(transport);
    const reloadedByName = new Map(
      parseMapCollection(reloaded.collection('maps')).map((record, index) => {
        const document = MapDocument.from(record, `maps[${index}]`);
        return [document.name, document] as const;
      }),
    );
    assert.deepEqual(reloadedByName.get('SAVE_TEST_MAP')?.readCell(0, 0), {
      tilesetIndex: 1,
      tileIndex: 77,
    });
    assert.deepEqual(reloadedByName.get('SAVE_TEST_NEIGHBOR')?.readCell(0, 0), {
      tilesetIndex: 2,
      tileIndex: 77,
    });
  } finally {
    await rm(databasePath, { recursive: true, force: true });
  }
});

function createFixtureSnapshot(): DatabaseSnapshot {
  const snapshot = {} as DatabaseSnapshot;
  for (const { id } of ASSET_REGISTRY) {
    snapshot[id] = [];
  }
  snapshot.maps = [createMapFixture(), createNeighborFixture()];
  snapshot.mapGrids = [
    {
      name: 'SAVE_TEST_GRID',
      label: 'Save Test Grid',
      gridWidth: 2,
      gridHeight: 1,
      mapWidth: 2,
      mapHeight: 2,
      cells: [['SAVE_TEST_MAP', 'SAVE_TEST_NEIGHBOR']],
      futureGridData: 'preserved',
    },
  ];
  snapshot.tilesets = [{ name: 'terrain0' }, { name: 'terrain_borders' }];
  return snapshot;
}

function createNeighborFixture(): MapRecord {
  return {
    ...createMapFixture(),
    name: 'SAVE_TEST_NEIGHBOR',
    label: 'Save Test Neighbor',
    tilesets: ['', 'terrain_borders', 'terrain0'],
    tiles: {
      '0': [2, 30, 2, 31, 2, 32, 2, 33],
      '-1': [1, 40, 1, 41, 1, 42, 1, 43],
    },
    futureMapData: { preserve: true, neighbor: true },
  };
}

function createMapFixture(): MapRecord {
  return {
    name: 'SAVE_TEST_MAP',
    label: 'Save Test Map',
    type: 'TOWN',
    width: 2,
    height: 2,
    spriteWidth: 28,
    spriteHeight: 32,
    tilesets: ['', 'terrain0', 'terrain_borders'],
    layers: [0, -1],
    tiles: {
      '0': [1, 10, 1, 11, 1, 12, 1, 13],
      '-1': [2, 20, 2, 21, 2, 22, 2, 23],
    },
    characters: [],
    items: [],
    markers: [
      {
        l: -1,
        i: 3,
        name: 'BASEMENT_EXIT',
        futurePlacementData: 'also preserved',
      },
    ],
    eventTriggers: [],
    travelTriggers: [],
    tileOverrides: [],
    lightSources: [],
    futureMapData: { preserve: true, version: 3 },
  };
}

async function writeFixtureDatabase(
  databasePath: string,
  snapshot: DatabaseSnapshot,
): Promise<void> {
  for (const { id, fileName } of ASSET_REGISTRY) {
    await writeFile(
      join(databasePath, fileName),
      `${JSON.stringify(snapshot[id], null, 2)}\n`,
      'utf8',
    );
  }
  await writeFile(join(databasePath, 'tiles.json'), UNMANAGED_TILES, 'utf8');
}

async function readAllFiles(
  databasePath: string,
): Promise<Map<string, string>> {
  const fileNames = [
    ...ASSET_REGISTRY.map(({ fileName }) => fileName),
    'tiles.json',
  ];
  return new Map(
    await Promise.all(
      fileNames.map(
        async (fileName) =>
          [
            fileName,
            await readFile(join(databasePath, fileName), 'utf8'),
          ] as const,
      ),
    ),
  );
}
