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
  ensureEditorStateMap,
  getEditorStateMap,
  getPaintMapName,
  pushGridUndo,
  updateEditorStateMapNoReRender,
  updateEditorStateNoReRender,
} from './editorState';
import { resolveGridBrushCell } from '../utils/mapGridIndex';
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
   * Every tile a draw / clone-brush / terrain stroke wrote, tagged with its
   * block. Lets one stroke span grid maps and one undo restore every block it
   * touched.
   */
  blockWrites?: { mapName: string; ind: number; prev: CarcerMapTileTemplate }[];
  /** `${blockName}:${tileIndex}` cells a terrain stroke has already painted at. */
  terrainCenters?: string[];
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
  startMap: CarcerMapTemplate,
  editorState: EditorState,
  tilesets: TilesetTemplate[],
) {
  // Paint terrain in whichever grid block the pointer is over, so a stroke
  // continues across block borders. Border auto-tiling is still resolved per
  // block (each block sees only its own tiles at the seam).
  const hoveredBlockName =
    editorState.hoveredGridMapName || editorState.selectedMapName;
  const ind = getEditorStateMap(hoveredBlockName)?.hoveredTileIndex ?? -1;
  if (ind === -1) {
    return;
  }

  const ctx = getGridPaintContext();
  const byName: Record<string, CarcerMapTemplate> = {
    [startMap.name]: startMap,
  };
  if (ctx) {
    for (const m of ctx.maps) {
      byName[m.name] = m;
    }
  }
  const targetMap = byName[hoveredBlockName] ?? startMap;

  const centerKey = `${hoveredBlockName}:${ind}`;
  const centers = (action.data.terrainCenters ??= []);
  if (centers.includes(centerKey)) {
    return;
  }
  centers.push(centerKey);

  const terrainTileset = getTerrainTileset(tilesets);
  const mapState = ensureEditorStateMap(hoveredBlockName);
  const x = ind % targetMap.width;
  const y = Math.floor(ind / targetMap.width);
  const lookup = buildTerrainLookup(terrainTileset);

  const tileChanges = getTileChangesForPaintingTerrainAt(
    targetMap,
    editorState,
    mapState,
    terrainTileset,
    lookup,
    x,
    y,
    editorState.selectedTerrainTag,
  );

  const targetTiles = getTileList(targetMap);
  const writes = (action.data.blockWrites ??= []);
  const seen = new Set(writes.map((w) => `${w.mapName}:${w.ind}`));
  for (const tileChange of tileChanges) {
    const key = `${hoveredBlockName}:${tileChange.ind}`;
    if (!seen.has(key)) {
      seen.add(key);
      writes.push({
        mapName: hoveredBlockName,
        ind: tileChange.ind,
        prev: structuredClone(targetTiles[tileChange.ind]),
      });
    }
  }
  for (const tileChange of tileChanges) {
    targetTiles[tileChange.ind].tileId = tileChange.tileId;
    targetTiles[tileChange.ind].tilesetName = TERRAIN_TILESET_NAME;
  }
}

/**
 * Draw / clone-brush stroke frame: paint the tile (and brush footprint) under
 * the pointer in whatever grid block it is currently over, so a drag continues
 * seamlessly across block borders. Every written tile is recorded in
 * action.data.blockWrites for a single multi-block undo.
 */
const applyDrawUpdate = (
  action: PaintAction,
  startMap: CarcerMapTemplate,
  editorState: EditorState,
) => {
  const hoveredBlockName =
    editorState.hoveredGridMapName || editorState.selectedMapName;
  const hoveredInd =
    getEditorStateMap(hoveredBlockName)?.hoveredTileIndex ?? -1;
  if (hoveredInd === -1) {
    return;
  }

  const ctx = getGridPaintContext();
  const mapsByName: Record<string, CarcerMapTemplate> = {
    [startMap.name]: startMap,
  };
  if (ctx) {
    for (const m of ctx.maps) {
      mapsByName[m.name] = m;
    }
  }
  const grids = ctx?.mapGrids ?? [];
  const anchorMap = mapsByName[hoveredBlockName] ?? startMap;

  if (action.data.startInd === -1) {
    action.data.startInd = hoveredInd;
  }
  action.data.endInd = hoveredInd;

  const anchorX = hoveredInd % anchorMap.width;
  const anchorY = Math.floor(hoveredInd / anchorMap.width);
  const brush = action.data.floorDrawBrush;
  const cells: { dx: number; dy: number; ref: Partial<CarcerMapTileTemplate> }[] =
    brush?.length
      ? brush.map((bt) => ({
          dx: bt.xOffset,
          dy: bt.yOffset,
          ref: bt.originalTile.ref,
        }))
      : [{ dx: 0, dy: 0, ref: action.data.paintTileRef }];

  const writes = (action.data.blockWrites ??= []);
  const seen = new Set(writes.map((w) => `${w.mapName}:${w.ind}`));

  for (const cell of cells) {
    const target = resolveGridBrushCell(
      anchorMap,
      anchorX + cell.dx,
      anchorY + cell.dy,
      grids,
      mapsByName,
    );
    if (!target) {
      continue;
    }
    const key = `${target.map.name}:${target.tileIndex}`;
    if (seen.has(key)) {
      continue;
    }
    seen.add(key);
    if (target.map.name !== startMap.name) {
      ensureEditorStateMap(target.map.name);
    }
    const tiles = getTileList(target.map);
    writes.push({
      mapName: target.map.name,
      ind: target.tileIndex,
      prev: structuredClone(tiles[target.tileIndex]),
    });
    Object.assign(tiles[target.tileIndex], cell.ref);
  }
};

export const onActionUpdate = (
  action: PaintAction,
  mapData: CarcerMapTemplate,
  editorState: EditorState,
  tilesets?: TilesetTemplate[],
) => {
  // DRAW and TERRAIN follow the pointer across grid blocks, so they must run
  // even when the start block's own hovered tile is now -1.
  if (action.type === PaintActionType.DRAW) {
    applyDrawUpdate(action, mapData, editorState);
    return;
  }
  if (action.type === PaintActionType.TERRAIN && tilesets) {
    applyTerrainPaintUpdate(action, mapData, editorState, tilesets);
    return;
  }

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

  if (!action.data.tileInds.includes(ind)) {
    action.data.tileInds.push(ind);
    action.data.prevRefData.push(structuredClone(mapTiles[ind]));
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
