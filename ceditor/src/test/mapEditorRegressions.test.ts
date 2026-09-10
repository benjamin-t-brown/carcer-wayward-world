import assert from 'node:assert/strict';
import test, { before } from 'node:test';

import type {
  CarcerMapTemplate,
  CarcerMapTileTemplate,
  MapGridTemplate,
} from '../client/types/assets';
import type { EditorState } from '../client/tile-editor/editorState';
import type { MapDocumentIndex } from '../client/tile-editor/mapDocumentIndex';

type Modules = {
  MapEditorController: typeof import('../client/tile-editor/MapEditorController').MapEditorController;
  PaintActionType: typeof import('../client/tile-editor/paintTools').PaintActionType;
  buildMapDocumentIndex: typeof import('../client/tile-editor/mapDocumentIndex').buildMapDocumentIndex;
  createPaintAction: typeof import('../client/tile-editor/paintTools').createPaintAction;
  getTileList: typeof import('../client/tile-editor/editorEvents').getTileList;
  onActionComplete: typeof import('../client/tile-editor/paintTools').onActionComplete;
  onActionUpdate: typeof import('../client/tile-editor/paintTools').onActionUpdate;
  setGridNavigationHandlers: typeof import('../client/tile-editor/editorEvents').setGridNavigationHandlers;
  undo: typeof import('../client/tile-editor/paintTools').undo;
};

let modules: Modules;

before(async () => {
  Object.defineProperty(globalThis, 'window', {
    configurable: true,
    value: { reRenderTileEditor: () => {} },
  });

  const paint = await import('../client/tile-editor/paintTools');
  const events = await import('../client/tile-editor/editorEvents');
  const controller = await import('../client/tile-editor/MapEditorController');
  const documentIndex = await import('../client/tile-editor/mapDocumentIndex');

  modules = {
    MapEditorController: controller.MapEditorController,
    PaintActionType: paint.PaintActionType,
    buildMapDocumentIndex: documentIndex.buildMapDocumentIndex,
    createPaintAction: paint.createPaintAction,
    getTileList: events.getTileList,
    onActionComplete: paint.onActionComplete,
    onActionUpdate: paint.onActionUpdate,
    setGridNavigationHandlers: events.setGridNavigationHandlers,
    undo: paint.undo,
  };
});

function map(name: string, graphic: [number, number] = [0, 0]) {
  return {
    name,
    label: name,
    type: 'TOWN',
    width: 1,
    height: 1,
    spriteWidth: 16,
    spriteHeight: 16,
    tilesets: ['', 'ground', 'paint'],
    layers: [0],
    tiles: { '0': graphic },
    characters: [],
    items: [],
    markers: [],
    eventTriggers: [],
    travelTriggers: [],
    tileOverrides: [],
    lightSources: [],
  } satisfies CarcerMapTemplate;
}

function focus(
  controller: InstanceType<Modules['MapEditorController']>,
  mapName: string,
): EditorState {
  const state = controller.getState();
  state.selectedMapName = mapName;
  state.currentLevel = 0;
  controller.ensureMap(mapName).hoveredTileIndex = -1;
  return state;
}

function blankTile(): CarcerMapTileTemplate {
  return {
    tilesetName: '',
    tileId: 0,
    characters: [],
    items: [],
    markers: [],
  };
}

test('fill actions completed outside a valid tile are safe no-ops', () => {
  for (const type of [
    modules.PaintActionType.FILL,
    modules.PaintActionType.DELETE_FILL,
  ]) {
    const controller = new modules.MapEditorController();
    const documentMap = map(`invalid-${type.toLowerCase()}`, [2, 9]);
    const state = focus(controller, documentMap.name);
    const action = modules.createPaintAction(type);
    controller.setCurrentAction(action);

    assert.equal(
      modules.onActionComplete(controller, action, documentMap, state),
      false,
    );
    assert.deepEqual(documentMap.tiles['0'], [2, 9]);
    assert.deepEqual(controller.ensureMap(documentMap.name).undoHistory, []);
    assert.equal(controller.getCurrentAction(), null);
  }
});

test('one undo restores every partition in a stroke wider than the layer cache', () => {
  const controller = new modules.MapEditorController();
  const maps = Array.from({ length: 120 }, (_, index) =>
    map(`partition-${index}`, [2, 9]),
  );
  const state = focus(controller, maps[0].name);
  const action = modules.createPaintAction(modules.PaintActionType.DRAW);
  action.data.blockWrites = maps.map((documentMap) => ({
    mapName: documentMap.name,
    ind: 0,
    prev: blankTile(),
  }));

  const ownerState = controller.ensureMap(maps[0].name);
  ownerState.undoHistory = [structuredClone(action)];
  ownerState.undoIndex = 0;
  modules.setGridNavigationHandlers(controller, {
    getMaps: () => maps,
    getMapGrids: () => [],
    onNavigateToGridMap: () => {},
    onCreateGridMap: () => {},
  });

  assert.equal(modules.undo(controller, maps[0], state, maps[0].name), true);
  for (const documentMap of maps) {
    assert.deepEqual(
      documentMap.tiles['0'],
      [0, 0],
      `${documentMap.name} was not committed after undo`,
    );
  }

  modules.setGridNavigationHandlers(controller, null);
});

test('an indexed known-absent placement never falls back to scanning grids', () => {
  const controller = new modules.MapEditorController();
  const standalone = map('indexed-standalone');
  const state = focus(controller, standalone.name);
  controller.ensureMap(standalone.name).hoveredTileIndex = 0;

  const index: MapDocumentIndex = modules.buildMapDocumentIndex('test', {
    maps: [standalone],
    mapGrids: [],
    tilesets: [],
    characters: [],
    items: [],
    gameEvents: [],
  });
  const poisonGrid = {
    name: 'must-not-scan',
    label: 'Must not scan',
    gridWidth: 1,
    gridHeight: 1,
    mapWidth: 1,
    mapHeight: 1,
  } as MapGridTemplate;
  Object.defineProperty(poisonGrid, 'cells', {
    get() {
      throw new Error(
        'indexed placement unexpectedly fell back to a grid scan',
      );
    },
  });
  modules.setGridNavigationHandlers(controller, {
    getMaps: () => [standalone],
    getMapGrids: () => [poisonGrid],
    getDocumentIndex: () => index,
    onNavigateToGridMap: () => {},
    onCreateGridMap: () => {},
  });

  const action = modules.createPaintAction(modules.PaintActionType.DRAW);
  action.data.floorDrawBrush = [
    { xOffset: 0, yOffset: 0, originalTile: { ref: blankTile() } },
    { xOffset: 1, yOffset: 0, originalTile: { ref: blankTile() } },
  ];

  assert.doesNotThrow(() =>
    modules.onActionUpdate(controller, action, standalone, state),
  );
  assert.deepEqual(
    action.data.blockWrites?.map(({ mapName, ind }) => [mapName, ind]),
    [[standalone.name, 0]],
  );

  modules.setGridNavigationHandlers(controller, null);
});
