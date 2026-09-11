import assert from 'node:assert/strict';
import test, { before } from 'node:test';

import type {
  CarcerMapTemplate,
  CarcerMapTileTemplate,
  MapGridTemplate,
  TileTerrainBorderTag,
  TilesetTemplate,
} from '../client/types/assets';
import type { EditorState } from '../client/tile-editor/editorState';

type MapEditorModules = {
  MapEditorController: typeof import('../client/tile-editor/MapEditorController').MapEditorController;
  PaintActionType: typeof import('../client/tile-editor/paintTools').PaintActionType;
  buildTerrainLookup: typeof import('../client/tile-editor/terrainTool').buildTerrainLookup;
  calculateFillIndsFloor: typeof import('../client/tile-editor/fill').calculateFillIndsFloor;
  createPaintAction: typeof import('../client/tile-editor/paintTools').createPaintAction;
  getTileChangesForPaintingTerrainAt: typeof import('../client/tile-editor/terrainTool').getTileChangesForPaintingTerrainAt;
  getTileList: typeof import('../client/tile-editor/editorEvents').getTileList;
  onActionUpdate: typeof import('../client/tile-editor/paintTools').onActionUpdate;
  setGridNavigationHandlers: typeof import('../client/tile-editor/editorEvents').setGridNavigationHandlers;
  terrainMetaKey: typeof import('../client/tile-editor/terrainTool').terrainMetaKey;
  undoAction: typeof import('../client/tile-editor/paintTools').undoAction;
};

let modules: MapEditorModules;

before(async () => {
  const paint = await import('../client/tile-editor/paintTools');
  const fill = await import('../client/tile-editor/fill');
  const events = await import('../client/tile-editor/editorEvents');
  const terrain = await import('../client/tile-editor/terrainTool');
  const controller = await import('../client/tile-editor/MapEditorController');

  modules = {
    MapEditorController: controller.MapEditorController,
    PaintActionType: paint.PaintActionType,
    buildTerrainLookup: terrain.buildTerrainLookup,
    calculateFillIndsFloor: fill.calculateFillIndsFloor,
    createPaintAction: paint.createPaintAction,
    getTileChangesForPaintingTerrainAt:
      terrain.getTileChangesForPaintingTerrainAt,
    getTileList: events.getTileList,
    onActionUpdate: paint.onActionUpdate,
    setGridNavigationHandlers: events.setGridNavigationHandlers,
    terrainMetaKey: terrain.terrainMetaKey,
    undoAction: paint.undoAction,
  };
});

function map(
  name: string,
  width: number,
  height: number,
  graphics?: number[],
): CarcerMapTemplate {
  return {
    name,
    label: name,
    type: 'TOWN',
    width,
    height,
    spriteWidth: 16,
    spriteHeight: 16,
    tilesets: ['', 'ground', 'paint'],
    layers: [0, 1],
    tiles: {
      '0':
        graphics ?? Array.from({ length: width * height }, () => [0, 0]).flat(),
      '1': Array.from({ length: width * height }, () => [0, 0]).flat(),
    },
    characters: [],
    items: [],
    markers: [],
    eventTriggers: [],
    travelTriggers: [],
    tileOverrides: [],
    lightSources: [],
  };
}

function grid(cells: string[][], mapWidth: number, mapHeight: number) {
  return {
    name: 'behavior-grid',
    label: 'Behavior grid',
    gridWidth: cells[0]?.length ?? 0,
    gridHeight: cells.length,
    mapWidth,
    mapHeight,
    cells,
  } satisfies MapGridTemplate;
}

function setFocusedMap(
  controller: InstanceType<MapEditorModules['MapEditorController']>,
  name: string,
  level = 0,
): EditorState {
  const state = controller.getState();
  Object.assign(state, {
    selectedMapName: name,
    activePaintMapName: '',
    hoveredGridMapName: '',
    currentLevel: level,
  });
  const mapState = controller.ensureMap(name);
  mapState.hoveredTileIndex = -1;
  return state;
}

function tileRef(
  tilesetName: string,
  tileId: number,
): Partial<CarcerMapTileTemplate> {
  return { tilesetName, tileId };
}

test('fill is four-way contiguous, graphic-specific, and layer-specific', () => {
  const controller = new modules.MapEditorController();
  // G G X G
  // G X G G
  // G G G X
  const G = [1, 7];
  const X = [2, 7];
  const documentMap = map(
    'behavior-fill',
    4,
    3,
    [G, G, X, G, G, X, G, G, G, G, G, X].flat(),
  );
  setFocusedMap(controller, documentMap.name);

  const filled = modules
    .calculateFillIndsFloor(controller, 0, documentMap, 0)
    .sort((a, b) => a - b);
  assert.deepEqual(filled, [0, 1, 3, 4, 6, 7, 8, 9, 10]);
  assert.deepEqual(
    modules.calculateFillIndsFloor(controller, 2, documentMap, 0),
    [2],
  );

  // The same coordinate on another level is unrelated to the floor graphic.
  assert.equal(
    modules.calculateFillIndsFloor(controller, 0, documentMap, 1).length,
    documentMap.width * documentMap.height,
  );
});

test('draw records the prior tile once and undo restores it', () => {
  const controller = new modules.MapEditorController();
  const documentMap = map('behavior-draw', 3, 1);
  const state = setFocusedMap(controller, documentMap.name);
  controller.ensureMap(documentMap.name).hoveredTileIndex = 1;
  const action = modules.createPaintAction(modules.PaintActionType.DRAW);
  action.data.paintTileRef = tileRef('paint', 9);

  modules.onActionUpdate(controller, action, documentMap, state);
  modules.onActionUpdate(controller, action, documentMap, state);

  assert.deepEqual(
    modules.getTileList(controller, documentMap, 0)[1],
    expectTile('paint', 9),
  );
  assert.equal(action.data.blockWrites?.length, 1);
  assert.equal(action.data.blockWrites?.[0]?.mapName, documentMap.name);

  modules.undoAction(controller, documentMap, action);
  assert.deepEqual(
    modules.getTileList(controller, documentMap, 0)[1],
    expectTile('', 0),
  );
});

test('materialized tile buffers are isolated between controller instances', () => {
  const first = new modules.MapEditorController();
  const second = new modules.MapEditorController();
  const firstMap = map('behavior-shared-name', 2, 1);
  const secondMap = map('behavior-shared-name', 2, 1);
  setFocusedMap(first, firstMap.name);
  setFocusedMap(second, secondMap.name);

  const firstTiles = modules.getTileList(first, firstMap, 0);
  const secondTiles = modules.getTileList(second, secondMap, 0);
  assert.notEqual(firstTiles, secondTiles);

  firstTiles[0]!.tileId = 42;
  assert.equal(secondTiles[0]?.tileId, 0);
});

test('one draw stroke crosses a grid seam and one undo restores both maps', () => {
  const controller = new modules.MapEditorController();
  const west = map('behavior-west', 2, 1);
  const east = map('behavior-east', 2, 1);
  const documentGrid = grid([[west.name, east.name]], 2, 1);
  const state = setFocusedMap(controller, west.name);
  const westState = controller.ensureMap(west.name);
  const eastState = controller.ensureMap(east.name);
  modules.setGridNavigationHandlers(controller, {
    getMaps: () => [west, east],
    getMapGrids: () => [documentGrid],
    onNavigateToGridMap: () => {},
    onCreateGridMap: () => {},
  });

  const action = modules.createPaintAction(modules.PaintActionType.DRAW);
  action.data.paintTileRef = tileRef('paint', 11);
  westState.hoveredTileIndex = 1;
  modules.onActionUpdate(controller, action, west, state);

  state.hoveredGridMapName = east.name;
  eastState.hoveredTileIndex = 0;
  modules.onActionUpdate(controller, action, west, state);

  assert.equal(modules.getTileList(controller, west, 0)[1]?.tileId, 11);
  assert.equal(modules.getTileList(controller, east, 0)[0]?.tileId, 11);
  assert.deepEqual(
    action.data.blockWrites?.map(({ mapName, ind }) => [mapName, ind]),
    [
      [west.name, 1],
      [east.name, 0],
    ],
  );

  modules.undoAction(controller, west, action);
  assert.deepEqual(
    modules.getTileList(controller, west, 0)[1],
    expectTile('', 0),
  );
  assert.deepEqual(
    modules.getTileList(controller, east, 0)[0],
    expectTile('', 0),
  );
  modules.setGridNavigationHandlers(controller, null);
});

test('standalone drawing never resolves an out-of-map brush cell', async () => {
  const { resolveGridBrushCell } = await import('../client/utils/mapGridIndex');
  const standalone = map('behavior-standalone', 2, 2);
  const byName = { [standalone.name]: standalone };

  assert.deepEqual(resolveGridBrushCell(standalone, 1, 1, [], byName), {
    map: standalone,
    tileIndex: 3,
  });
  assert.equal(resolveGridBrushCell(standalone, 2, 1, [], byName), null);
  assert.equal(resolveGridBrushCell(standalone, -1, 1, [], byName), null);
});

test('terrain painting updates the center and only its in-bounds neighbors', async () => {
  const controller = new modules.MapEditorController();
  const { TileTerrainBorderTag } = await import('../client/types/assets');
  const documentMap = map('behavior-terrain', 3, 3);
  const state = setFocusedMap(controller, documentMap.name);
  const mapState = controller.ensureMap(documentMap.name);
  const lookup = terrainLookup(
    TileTerrainBorderTag.NONE,
    TileTerrainBorderTag.GRASS,
  );
  const tileset = terrainTileset();

  const center = modules.getTileChangesForPaintingTerrainAt(
    controller,
    documentMap,
    state,
    mapState,
    tileset,
    lookup,
    1,
    1,
    TileTerrainBorderTag.GRASS,
  );
  assert.equal(center.length, 9);
  assert.deepEqual(
    center.map(({ ind }) => ind).sort((a, b) => a - b),
    [0, 1, 2, 3, 4, 5, 6, 7, 8],
  );

  const corner = modules.getTileChangesForPaintingTerrainAt(
    controller,
    documentMap,
    state,
    mapState,
    tileset,
    lookup,
    0,
    0,
    TileTerrainBorderTag.GRASS,
  );
  assert.deepEqual(
    corner.map(({ ind }) => ind).sort((a, b) => a - b),
    [0, 1, 3, 4],
  );
  assert.deepEqual(
    modules.getTileChangesForPaintingTerrainAt(
      controller,
      documentMap,
      state,
      mapState,
      tileset,
      lookup,
      -1,
      0,
      TileTerrainBorderTag.GRASS,
    ),
    [],
  );
});

test('terrain lookup caching follows the current tiles array', async () => {
  const { TileTerrainBorderTag } = await import('../client/types/assets');
  const metadata = {
    nw: TileTerrainBorderTag.NONE,
    ne: TileTerrainBorderTag.NONE,
    sw: TileTerrainBorderTag.NONE,
    se: TileTerrainBorderTag.NONE,
  };
  const first = terrainTileset();
  const second = terrainTileset();
  first.tiles = [{ id: 101, tileTerrainBorderMeta: metadata }];
  second.tiles = [{ id: 202, tileTerrainBorderMeta: metadata }];
  const previousConsoleError = console.error;
  console.error = () => {};

  try {
    const firstLookup = modules.buildTerrainLookup(first);
    const secondLookup = modules.buildTerrainLookup(second);
    const key = modules.terrainMetaKey(metadata);

    assert.equal(firstLookup.get(key), 101);
    assert.equal(secondLookup.get(key), 202);
    assert.equal(modules.buildTerrainLookup(first), firstLookup);
  } finally {
    console.error = previousConsoleError;
  }
});

function expectTile(
  tilesetName: string,
  tileId: number,
): CarcerMapTileTemplate {
  return {
    tilesetName,
    tileId,
    characters: [],
    items: [],
    markers: [],
  };
}

function terrainLookup(
  none: TileTerrainBorderTag,
  grass: TileTerrainBorderTag,
): Map<string, number> {
  const lookup = new Map<string, number>();
  for (let mask = 0; mask < 16; mask++) {
    lookup.set(
      modules.terrainMetaKey({
        nw: mask & 8 ? grass : none,
        ne: mask & 4 ? grass : none,
        sw: mask & 2 ? grass : none,
        se: mask & 1 ? grass : none,
      }),
      mask,
    );
  }
  return lookup;
}

function terrainTileset(): TilesetTemplate {
  return {
    name: 'terrain_borders',
    spriteBase: 'terrain',
    imageWidth: 16,
    imageHeight: 16,
    tileWidth: 16,
    tileHeight: 16,
    tiles: [],
  };
}
