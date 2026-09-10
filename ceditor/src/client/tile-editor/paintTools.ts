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
import { MapEditorController } from './MapEditorController';

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
  controller: MapEditorController,
  action: PaintAction,
  mapData: CarcerMapTemplate,
  editorState: EditorState,
) => {
  TOOLS[action.type]?.apply(controller, action, mapData, editorState);
};

export const applyActionUpdate = (
  controller: MapEditorController,
  action: PaintAction,
  mapData: CarcerMapTemplate,
  editorState: EditorState,
) => {
  TOOLS[action.type]?.update?.(controller, action, mapData, editorState);
};

export const undoAction = (
  controller: MapEditorController,
  mapData: CarcerMapTemplate,
  action: PaintAction,
) => {
  TOOLS[action.type]?.undo(controller, action, mapData);
};

function applyTerrainPaintUpdate(
  controller: MapEditorController,
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
  const ind =
    getEditorStateMap(controller, hoveredBlockName)?.hoveredTileIndex ?? -1;
  if (ind === -1) {
    return;
  }

  const ctx = getGridPaintContext(controller);
  const targetMap = ctx?.mapsByName.get(hoveredBlockName) ?? startMap;

  const centerKey = `${hoveredBlockName}:${ind}`;
  const centers = (action.data.terrainCenters ??= []);
  if (centers.includes(centerKey)) {
    return;
  }
  centers.push(centerKey);

  const terrainTileset = getTerrainTileset(tilesets);
  const mapState = ensureEditorStateMap(controller, hoveredBlockName);
  const x = ind % targetMap.width;
  const y = Math.floor(ind / targetMap.width);
  const lookup = buildTerrainLookup(terrainTileset);

  const tileChanges = getTileChangesForPaintingTerrainAt(
    controller,
    targetMap,
    editorState,
    mapState,
    terrainTileset,
    lookup,
    x,
    y,
    editorState.selectedTerrainTag,
  );

  const targetTiles = getTileList(controller, targetMap);
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
  controller: MapEditorController,
  action: PaintAction,
  startMap: CarcerMapTemplate,
  editorState: EditorState,
) => {
  const hoveredBlockName =
    editorState.hoveredGridMapName || editorState.selectedMapName;
  const hoveredInd =
    getEditorStateMap(controller, hoveredBlockName)?.hoveredTileIndex ?? -1;
  if (hoveredInd === -1) {
    return;
  }

  const ctx = getGridPaintContext(controller);
  const mapsByName =
    ctx?.mapsByName ?? new Map([[startMap.name, startMap] as const]);
  const grids = ctx?.mapGrids ?? [];
  const anchorMap = mapsByName.get(hoveredBlockName) ?? startMap;
  const placement = ctx
    ? (ctx.placementsByMapName.get(anchorMap.name)?.[0] ?? null)
    : undefined;

  if (action.data.startInd === -1) {
    action.data.startInd = hoveredInd;
  }
  action.data.endInd = hoveredInd;

  const anchorX = hoveredInd % anchorMap.width;
  const anchorY = Math.floor(hoveredInd / anchorMap.width);
  const brush = action.data.floorDrawBrush;
  const cells: {
    dx: number;
    dy: number;
    ref: Partial<CarcerMapTileTemplate>;
  }[] = brush?.length
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
      placement,
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
      ensureEditorStateMap(controller, target.map.name);
    }
    const tiles = getTileList(controller, target.map);
    writes.push({
      mapName: target.map.name,
      ind: target.tileIndex,
      prev: structuredClone(tiles[target.tileIndex]),
    });
    Object.assign(tiles[target.tileIndex], cell.ref);
  }
};

export const onActionUpdate = (
  controller: MapEditorController,
  action: PaintAction,
  mapData: CarcerMapTemplate,
  editorState: EditorState,
  tilesets?: TilesetTemplate[],
) => {
  // DRAW and TERRAIN follow the pointer across grid blocks, so they must run
  // even when the start block's own hovered tile is now -1.
  if (action.type === PaintActionType.DRAW) {
    applyDrawUpdate(controller, action, mapData, editorState);
    return;
  }
  if (action.type === PaintActionType.TERRAIN && tilesets) {
    applyTerrainPaintUpdate(controller, action, mapData, editorState, tilesets);
    return;
  }

  const mapTiles = getTileList(controller, mapData);
  const ind =
    getEditorStateMap(controller, getPaintMapName(editorState))
      ?.hoveredTileIndex ?? -1;

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
    applyActionUpdate(controller, action, mapData, editorState);
  }
};

/**
 * Commit the materialized layer of every block a clone-brush stroke touched
 * besides `alreadyCommitted`, so writes that landed in neighbouring grid maps
 * are flushed and their caches invalidated.
 */
const commitBlockWriteMaps = (
  controller: MapEditorController,
  action: PaintAction,
  alreadyCommitted: string,
  level: number,
) => {
  const writes = action.data.blockWrites;
  if (!writes || writes.length === 0) {
    return;
  }
  const ctx = getGridPaintContext(controller);
  if (!ctx) {
    return;
  }
  const done = new Set([alreadyCommitted]);
  for (const write of writes) {
    if (done.has(write.mapName)) {
      continue;
    }
    done.add(write.mapName);
    const map = ctx.mapsByName.get(write.mapName);
    if (map) {
      commitCurrentLayer(controller, map, level);
    }
  }
};

export const onActionComplete = (
  controller: MapEditorController,
  action: PaintAction,
  mapData: CarcerMapTemplate,
  editorState: EditorState,
): boolean => {
  applyAction(controller, action, mapData, editorState);

  const hasChanges = Boolean(
    action.data.tileInds.length || action.data.blockWrites?.length,
  );
  if (!hasChanges) {
    controller.setCurrentAction(null);
    return false;
  }

  const paintMapName = getPaintMapName(editorState);

  // Add action to undo history
  const newUndoHistory = [
    ...(getEditorStateMap(controller, paintMapName)?.undoHistory ?? []),
  ];
  const undoIndex = getEditorStateMap(controller, paintMapName)?.undoIndex ?? 0;

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

  updateEditorStateMapNoReRender(controller, paintMapName, {
    undoHistory: newUndoHistory,
    undoIndex: newUndoIndex,
  });
  // Grid-wide undo ordering across every block edited this session.
  pushGridUndo(controller, paintMapName);
  commitCurrentLayer(controller, mapData, editorState.currentLevel);
  commitBlockWriteMaps(
    controller,
    action,
    mapData.name,
    editorState.currentLevel,
  );
  controller.setCurrentAction(null);
  return true;
};

export const undo = (
  controller: MapEditorController,
  mapData: CarcerMapTemplate,
  editorState: EditorState,
  mapName?: string,
): boolean => {
  const key = mapName || getPaintMapName(editorState);
  const { undoHistory, undoIndex } = getEditorStateMap(controller, key) ?? {
    undoHistory: [],
    undoIndex: 0,
  };

  // Check if we can undo
  if (undoIndex < 0 || undoIndex >= undoHistory.length) {
    return false;
  }

  // Get the action to undo
  const actionToUndo = undoHistory[undoIndex];

  // Restoring a grid-wide action can materialize more blocks than the normal
  // cache limit. Keep every restored buffer alive until all of them commit.
  const releaseLayerViews = controller.pinLayerViews();
  try {
    undoAction(controller, mapData, actionToUndo);
    commitCurrentLayer(controller, mapData, editorState.currentLevel);
    commitBlockWriteMaps(
      controller,
      actionToUndo,
      mapData.name,
      editorState.currentLevel,
    );
  } finally {
    releaseLayerViews();
  }

  // Update undo index and trigger re-render
  const newUndoIndex = undoIndex - 1;
  updateEditorStateMapNoReRender(controller, key, {
    undoIndex: newUndoIndex,
  });

  // Trigger re-render to show the undo
  controller.notify();

  return true;
};

export const onTileHoverIndChange = (
  controller: MapEditorController,
  mapData: CarcerMapTemplate,
  editorState: EditorState,
  currentPaintAction: PaintActionType,
  prevHoverInd: number,
  nextHoverInd: number,
) => {
  // `mapData` is whichever block the pointer is over; read its state, not the
  // focused map's, so fill/terrain previews land on the right block.
  const mapState = getEditorStateMap(
    controller,
    editorState.hoveredGridMapName || editorState.selectedMapName,
  );
  if (nextHoverInd !== -1) {
    if (
      mapData &&
      (currentPaintAction === PaintActionType.FILL ||
        currentPaintAction === PaintActionType.DELETE_FILL)
    ) {
      updateEditorStateNoReRender(controller, {
        fillIndsFloor: calculateFillIndsFloor(
          controller,
          nextHoverInd,
          mapData,
          editorState.currentLevel,
        ),
      });
    } else {
      updateEditorStateNoReRender(controller, {
        fillIndsFloor: [],
      });
    }

    if (mapData && mapState && currentPaintAction === PaintActionType.TERRAIN) {
      const terrainTileset = getTerrainTileset(editorState.tilesets);
      const lookup = buildTerrainLookup(terrainTileset);
      const x = nextHoverInd % mapData.width;
      const y = Math.floor(nextHoverInd / mapData.width);
      const changes = getTileChangesForPaintingTerrainAt(
        controller,
        mapData,
        editorState,
        mapState,
        terrainTileset,
        lookup,
        x,
        y,
        editorState.selectedTerrainTag,
      );
      updateEditorStateNoReRender(controller, {
        terrainPaintChanges: changes,
      });
    } else {
      updateEditorStateNoReRender(controller, {
        terrainPaintChanges: [],
      });
    }
  } else {
    updateEditorStateNoReRender(controller, {
      fillIndsFloor: [],
      terrainPaintChanges: [],
    });
  }

  if (getIsDraggingRight(controller)) {
    updateEditorStateNoReRender(controller, {
      rectSelectTileIndEnd: nextHoverInd,
    });
  }
};
