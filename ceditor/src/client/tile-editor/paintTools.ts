import { calculateFillIndsFloor } from './fill';
import {
  CarcerMapTemplate,
  CarcerMapTileTemplate,
  TilesetTemplate,
} from '../types/assets';
import { createDefaultCarcerMapTile } from '../components/MapTemplateForm';
import { FloorBrushData } from './renderState';
import { TOOLS } from './tools';
import {
  EditorState,
  findEditorStateMap,
  getEditorStateMap,
  getPaintMapName,
  pushGridUndo,
  updateEditorStateMapNoReRender,
  updateEditorStateNoReRender,
} from './editorState';
import {
  commitCurrentLayer,
  getGridPaintContext,
  getIsDraggingRight,
  getTileList,
} from './editorEvents';
// import {
//   buildTerrainLookup,
//   collectAffectedTileIndices,
//   getTerrainTileset,
//   paintTerrainAt,
//   TERRAIN_TILESET_NAME,
// } from './terrainTool';
import {
  buildTerrainLookup,
  getAdjacentTileInds,
  getTerrainTileset,
  getTileChangesForPaintingTerrainAt,
  TERRAIN_TILESET_NAME,
} from './terrainTool';

export enum PaintActionType {
  NONE = '',
  DRAW = 'DRAW',
  ERASE = 'ERASE',
  ERASE_META = 'ERASE_META',
  DELETE_FILL = 'DELETE_FILL',
  FILL = 'FILL',
  SELECT = 'SELECT',
  CLONE = 'CLONE',
  TERRAIN = 'TERRAIN',
  // MOVE = 'MOVE',
  // COPY = 'COPY',
  // PASTE = 'PASTE',
  REF_CHANGE = 'REF_CHANGE',
}

interface PaintActionData {
  layer: 'floor';
  paintTileRef: Partial<CarcerMapTileTemplate>;
  floorDrawBrush?: FloorBrushData[];
  startInd: number;
  endInd: number;
  tileInds: number[];
  extraTileInds: number[];
  extraPrevRefData: CarcerMapTileTemplate[];

  prevRefData: CarcerMapTileTemplate[];

  /**
   * Every tile a clone-brush stroke wrote, tagged with its block. Lets one
   * stroke span grid maps and one undo restore every block it touched.
   */
  blockWrites?: { mapName: string; ind: number; prev: CarcerMapTileTemplate }[];
}

export interface PaintAction {
  type: PaintActionType;
  data: PaintActionData;
}

/** Cap per-map undo depth; each entry deep-clones every tile it touched. */
const MAX_UNDO_HISTORY = 100;

let currentAction: PaintAction | null = null;
export const setCurrentAction = (action: PaintAction) => {
  currentAction = action;
};
export const getCurrentAction = () => currentAction;

export const createPaintAction = (type: PaintActionType) => {
  const paintAction: PaintAction = {
    type,
    data: {
      layer: 'floor',
      paintTileRef: createDefaultCarcerMapTile(),
      startInd: -1,
      endInd: -1,
      tileInds: [],
      extraTileInds: [],
      extraPrevRefData: [],
      prevRefData: [],
      // prevObjectList: [],
    },
  };
  return paintAction;
};

export const applyAction = (
  action: PaintAction,
  mapData: CarcerMapTemplate,
  editorState: EditorState,
) => {
  TOOLS[action.type]?.apply(action, mapData, editorState);
};

export const applyActionUpdate = (
  action: PaintAction,
  mapData: CarcerMapTemplate,
  editorState: EditorState,
) => {
  TOOLS[action.type]?.update?.(action, mapData, editorState);
};

export const undoAction = (mapData: CarcerMapTemplate, action: PaintAction) => {
  TOOLS[action.type]?.undo(action, mapData);
};

function applyTerrainPaintUpdate(
  action: PaintAction,
  mapData: CarcerMapTemplate,
  editorState: EditorState,
  tilesets: TilesetTemplate[],
) {
  const mapTiles = getTileList(mapData);
  const paintMapName = getPaintMapName(editorState);
  const ind = getEditorStateMap(paintMapName)?.hoveredTileIndex ?? -1;
  if (ind === -1) {
    return;
  }

  if (action.data.extraTileInds.includes(ind)) {
    return;
  }
  action.data.extraTileInds.push(ind);

  const terrainTileset = getTerrainTileset(tilesets);

  const mapState = findEditorStateMap(editorState, paintMapName);
  const x = ind % mapData.width;
  const y = Math.floor(ind / mapData.width);
  const lookup = buildTerrainLookup(terrainTileset);

  const tileChanges = getTileChangesForPaintingTerrainAt(
    mapData,
    editorState,
    mapState,
    terrainTileset,
    lookup,
    x,
    y,
    editorState.selectedTerrainTag,
  );

  for (const tileChange of tileChanges) {
    if (!action.data.tileInds.includes(tileChange.ind)) {
      action.data.tileInds.push(tileChange.ind);
      action.data.prevRefData.push(structuredClone(mapTiles[tileChange.ind]));
    }
  }
  for (const tileChange of tileChanges) {
    mapTiles[tileChange.ind].tileId = tileChange.tileId;
    mapTiles[tileChange.ind].tilesetName = TERRAIN_TILESET_NAME;
  }
}

export const onActionUpdate = (
  action: PaintAction,
  mapData: CarcerMapTemplate,
  editorState: EditorState,
  tilesets?: TilesetTemplate[],
) => {
  const mapTiles = getTileList(mapData);
  const ind =
    getEditorStateMap(getPaintMapName(editorState))?.hoveredTileIndex ?? -1;

  if (ind === -1) {
    return;
  }

  if (action.data.startInd === -1) {
    action.data.startInd = ind;
  }
  action.data.endInd = ind;

  if (action.type === PaintActionType.TERRAIN && tilesets) {
    applyTerrainPaintUpdate(action, mapData, editorState, tilesets);
    return;
  }

  if (!action.data.tileInds.includes(ind)) {
    action.data.tileInds.push(ind);
    action.data.prevRefData.push(structuredClone(mapTiles[ind]));
    // Clone-brush strokes capture their own per-tile undo state (per block) in
    // DRAW.update via action.data.blockWrites, so no pre-clone pass here.
    applyActionUpdate(action, mapData, editorState);
  }
};

/**
 * Commit the materialized layer of every block a clone-brush stroke touched
 * besides `alreadyCommitted`, so writes that landed in neighbouring grid maps
 * are flushed and their caches invalidated.
 */
const commitBlockWriteMaps = (
  action: PaintAction,
  alreadyCommitted: string,
  level: number,
) => {
  const writes = action.data.blockWrites;
  if (!writes || writes.length === 0) {
    return;
  }
  const ctx = getGridPaintContext();
  if (!ctx) {
    return;
  }
  const byName: Record<string, CarcerMapTemplate> = {};
  for (const m of ctx.maps) {
    byName[m.name] = m;
  }
  const done = new Set([alreadyCommitted]);
  for (const write of writes) {
    if (done.has(write.mapName)) {
      continue;
    }
    done.add(write.mapName);
    const map = byName[write.mapName];
    if (map) {
      commitCurrentLayer(map, level);
    }
  }
};

export const onActionComplete = (
  action: PaintAction,
  mapData: CarcerMapTemplate,
  editorState: EditorState,
) => {
  currentAction = null;
  applyAction(action, mapData, editorState);

  const paintMapName = getPaintMapName(editorState);

  // Add action to undo history
  const newUndoHistory = [
    ...(getEditorStateMap(paintMapName)?.undoHistory ?? []),
  ];
  const undoIndex = getEditorStateMap(paintMapName)?.undoIndex ?? 0;

  // If we're not at the end of the history, slice off everything after the current index
  if (undoIndex < newUndoHistory.length - 1) {
    newUndoHistory.splice(undoIndex + 1);
  }

  // Add the new action to history
  newUndoHistory.push(structuredClone(action));
  // Drop the oldest entries rather than letting history grow for the session.
  if (newUndoHistory.length > MAX_UNDO_HISTORY) {
    newUndoHistory.splice(0, newUndoHistory.length - MAX_UNDO_HISTORY);
  }
  const newUndoIndex = newUndoHistory.length - 1;

  updateEditorStateMapNoReRender(paintMapName, {
    undoHistory: newUndoHistory,
    undoIndex: newUndoIndex,
  });
  // Grid-wide undo ordering across every block edited this session.
  pushGridUndo(paintMapName);
  commitCurrentLayer(mapData, editorState.currentLevel);
  commitBlockWriteMaps(action, mapData.name, editorState.currentLevel);
};

export const undo = (
  mapData: CarcerMapTemplate,
  editorState: EditorState,
  mapName?: string,
): boolean => {
  const key = mapName || getPaintMapName(editorState);
  const { undoHistory, undoIndex } = getEditorStateMap(key) ?? {
    undoHistory: [],
    undoIndex: 0,
  };

  // Check if we can undo
  if (undoIndex < 0 || undoIndex >= undoHistory.length) {
    return false;
  }

  // Get the action to undo
  const actionToUndo = undoHistory[undoIndex];

  // Perform the undo
  undoAction(mapData, actionToUndo);
  commitCurrentLayer(mapData, editorState.currentLevel);
  commitBlockWriteMaps(actionToUndo, mapData.name, editorState.currentLevel);

  // Update undo index and trigger re-render
  const newUndoIndex = undoIndex - 1;
  updateEditorStateMapNoReRender(key, {
    undoIndex: newUndoIndex,
  });

  // Trigger re-render to show the undo
  (window as any).reRenderTileEditor?.();

  return true;
};

export const onTileHoverIndChange = (
  mapData: CarcerMapTemplate,
  editorState: EditorState,
  currentPaintAction: PaintActionType,
  prevHoverInd: number,
  nextHoverInd: number,
) => {
  // `mapData` is whichever block the pointer is over; read its state, not the
  // focused map's, so fill/terrain previews land on the right block.
  const mapState = getEditorStateMap(
    editorState.hoveredGridMapName || editorState.selectedMapName,
  );
  if (nextHoverInd !== -1) {
    if (
      mapData &&
      (currentPaintAction === PaintActionType.FILL ||
        currentPaintAction === PaintActionType.DELETE_FILL)
    ) {
      updateEditorStateNoReRender({
        fillIndsFloor: calculateFillIndsFloor(
          nextHoverInd,
          mapData,
          editorState.currentLevel,
        ),
      });
    } else {
      updateEditorStateNoReRender({
        fillIndsFloor: [],
      });
    }

    if (mapData && mapState && currentPaintAction === PaintActionType.TERRAIN) {
      const terrainTileset = getTerrainTileset(editorState.tilesets);
      const lookup = buildTerrainLookup(terrainTileset);
      const x = nextHoverInd % mapData.width;
      const y = Math.floor(nextHoverInd / mapData.width);
      const changes = getTileChangesForPaintingTerrainAt(
        mapData,
        editorState,
        mapState,
        terrainTileset,
        lookup,
        x,
        y,
        editorState.selectedTerrainTag,
      );
      updateEditorStateNoReRender({
        terrainPaintChanges: changes,
      });
    } else {
      updateEditorStateNoReRender({
        terrainPaintChanges: [],
      });
    }
  } else {
    updateEditorStateNoReRender({
      fillIndsFloor: [],
      terrainPaintChanges: [],
    });
  }

  if (getIsDraggingRight()) {
    updateEditorStateNoReRender({
      rectSelectTileIndEnd: nextHoverInd,
    });
  }
};
