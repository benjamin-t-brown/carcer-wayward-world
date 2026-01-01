// import {
//   FloorBrushData,
//   getFillIndsFloor,
//   getHoveredTileInd,
//   getTileFloorBrush,
// } from './renderState';
import { calculateFillIndsFloor } from './fill';
import { CarcerMapTemplate, CarcerMapTileTemplate } from '../types/assets';
import { createDefaultCarcerMapTile } from '../components/MapTemplateForm';
import { FloorBrushData } from './renderState';
import {
  EditorState,
  getEditorStateMap,
  updateEditorStateMapNoReRender,
  updateEditorStateNoReRender,
} from './editorState';
import { getIsDraggingRight } from './editorEvents';

export enum PaintActionType {
  NONE = '',
  DRAW = 'DRAW',
  ERASE = 'ERASE',
  ERASE_META = 'ERASE_META',
  FILL = 'FILL',
  SELECT = 'SELECT',
  CLONE = 'CLONE',
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
}

export interface PaintAction {
  type: PaintActionType;
  data: PaintActionData;
}

const actionList: PaintAction[] = [];

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
  actionList.push(paintAction);
  return paintAction;
};

export const applyAction = (
  action: PaintAction,
  mapData: CarcerMapTemplate,
  editorState: EditorState
) => {
  switch (action.type) {
    case PaintActionType.DRAW:
      break;
    case PaintActionType.ERASE:
      break;
    case PaintActionType.ERASE_META:
      // Erase metadata is handled in applyActionUpdate
      break;
    case PaintActionType.FILL: {
      const ind =
        getEditorStateMap(editorState.selectedMapName)?.hoveredTileIndex ?? -1;
      const fillIndsFloor = calculateFillIndsFloor(ind, mapData);
      // Store the tile indices and ensure previous data is stored for undo
      action.data.tileInds = fillIndsFloor;
      for (let i = 0; i < fillIndsFloor.length; i++) {
        const tileInd = fillIndsFloor[i];
        // Store previous state if not already stored
        if (i >= action.data.prevRefData.length) {
          action.data.prevRefData.push(structuredClone(mapData.tiles[tileInd]));
        }
        mapData.tiles[tileInd] = Object.assign(
          mapData.tiles[tileInd],
          action.data.paintTileRef
        );
      }
      break;
    }
    case PaintActionType.SELECT: {
      // Move tile data (characters, items, overrides) from source to destination
      if (
        action.data.tileInds.length >= 2 &&
        action.data.prevRefData.length >= 2
      ) {
        const sourceTileIndex = action.data.startInd;
        const destTileIndex = action.data.endInd;

        if (
          sourceTileIndex >= 0 &&
          destTileIndex >= 0 &&
          sourceTileIndex !== destTileIndex
        ) {
          const sourceTile = mapData.tiles[sourceTileIndex];
          const destTile = mapData.tiles[destTileIndex];

          // Move characters, items, and overrides from source to destination
          // Merge with existing data on destination
          destTile.characters = [
            ...(destTile.characters || []),
            ...(sourceTile.characters || []),
          ];
          destTile.items = [
            ...(destTile.items || []),
            ...(sourceTile.items || []),
          ];
          destTile.markers = [
            ...(destTile.markers || []),
            ...(sourceTile.markers || []),
          ];

          // Move overrides (destination takes precedence if both exist)
          if (sourceTile.tileOverrides) {
            destTile.tileOverrides = {
              ...sourceTile.tileOverrides,
              ...(destTile.tileOverrides || {}),
            };
          }

          // Move light source (destination takes precedence)
          if (sourceTile.lightSource && !destTile.lightSource) {
            destTile.lightSource = sourceTile.lightSource;
          }

          // Move event trigger (destination takes precedence)
          if (sourceTile.eventTrigger && !destTile.eventTrigger) {
            destTile.eventTrigger = sourceTile.eventTrigger;
          }

          // Move travel trigger (destination takes precedence)
          if (sourceTile.travelTrigger && !destTile.travelTrigger) {
            destTile.travelTrigger = structuredClone(sourceTile.travelTrigger);
          }

          // Clear the source tile's movable data (keep base tile appearance)
          sourceTile.characters = [];
          sourceTile.items = [];
          sourceTile.markers = [];
          delete sourceTile.tileOverrides;
          delete sourceTile.lightSource;
          delete sourceTile.eventTrigger;
          delete sourceTile.travelTrigger;
        }
      }
      break;
    }
    case PaintActionType.CLONE: {
      // Clone tile data (characters, items, overrides) from source to destination
      if (
        action.data.tileInds.length >= 2 &&
        action.data.prevRefData.length >= 2
      ) {
        const sourceTileIndex = action.data.startInd;
        const destTileIndex = action.data.endInd;

        if (
          sourceTileIndex >= 0 &&
          destTileIndex >= 0 &&
          sourceTileIndex !== destTileIndex
        ) {
          const sourceTile = mapData.tiles[sourceTileIndex];
          const destTile = mapData.tiles[destTileIndex];

          // Clone characters, items, and overrides from source to destination
          // Merge with existing data on destination
          destTile.characters = [
            ...(destTile.characters || []),
            ...(sourceTile.characters || []),
          ];
          destTile.items = [
            ...(destTile.items || []),
            ...(sourceTile.items || []),
          ];
          destTile.markers = [
            ...(destTile.markers || []),
            ...(sourceTile.markers || []),
          ];

          // Clone overrides (destination takes precedence if both exist)
          if (sourceTile.tileOverrides) {
            destTile.tileOverrides = {
              ...sourceTile.tileOverrides,
              ...(destTile.tileOverrides || {}),
            };
          }

          // Clone light source (destination takes precedence)
          if (sourceTile.lightSource && !destTile.lightSource) {
            destTile.lightSource = structuredClone(sourceTile.lightSource);
          }

          // Clone event trigger (destination takes precedence)
          if (sourceTile.eventTrigger && !destTile.eventTrigger) {
            destTile.eventTrigger = structuredClone(sourceTile.eventTrigger);
          }

          // Clone travel trigger (destination takes precedence)
          if (sourceTile.travelTrigger && !destTile.travelTrigger) {
            destTile.travelTrigger = structuredClone(sourceTile.travelTrigger);
          }

          // Note: Source tile keeps its data (unlike SELECT which clears it)
        }
      }
      break;
    }
    // case PaintActionType.MOVE:
    //   break;
    // case PaintActionType.COPY:
    //   break;
    // case PaintActionType.PASTE:
    case PaintActionType.REF_CHANGE:
      break;
  }
};

export const applyActionUpdate = (
  action: PaintAction,
  mapData: CarcerMapTemplate,
  editorState: EditorState
) => {
  switch (action.type) {
    case PaintActionType.DRAW:
      for (const ind of action.data.tileInds) {
        const startX = ind % mapData.width;
        const startY = Math.floor(ind / mapData.width);
        const floorDrawBrush = action.data.floorDrawBrush;
        if (floorDrawBrush?.length) {
          for (const bt of floorDrawBrush) {
            const newX = startX + bt.xOffset;
            const newY = startY + bt.yOffset;
            const newInd = newY * mapData.width + newX;

            if (
              newX < 0 ||
              newX >= mapData.width ||
              newY < 0 ||
              newY >= mapData.height
            ) {
              continue;
            }

            Object.assign(mapData.tiles[newInd], bt.originalTile.ref);
          }
        } else {
          Object.assign(mapData.tiles[ind], action.data.paintTileRef);
        }
      }
      break;
    case PaintActionType.ERASE:
      for (const ind of action.data.tileInds) {
        mapData.tiles[ind] = createDefaultCarcerMapTile();
      }

      break;
    case PaintActionType.ERASE_META:
      for (const ind of action.data.tileInds) {
        const tile = mapData.tiles[ind];
        // Clear metadata but keep base tile properties
        tile.characters = [];
        tile.items = [];
        tile.markers = [];
        delete tile.tileOverrides;
        delete tile.lightSource;
        delete tile.eventTrigger;
        delete tile.travelTrigger;
      }
      break;
    case PaintActionType.FILL:
      break;
    case PaintActionType.SELECT:
      break;
    case PaintActionType.CLONE:
      break;
    case PaintActionType.ERASE_META:
      break;
    // case PaintActionType.MOVE:
    //   break;
    // case PaintActionType.COPY:
    //   break;
    // case PaintActionType.PASTE:
    case PaintActionType.REF_CHANGE:
      break;
  }
};

export const undoAction = (mapData: CarcerMapTemplate, action: PaintAction) => {
  switch (action.type) {
    case PaintActionType.DRAW:
      // Restore previous tile data for all affected tiles
      for (let i = 0; i < action.data.tileInds.length; i++) {
        const ind = action.data.tileInds[i];
        if (i < action.data.prevRefData.length) {
          mapData.tiles[ind] = structuredClone(action.data.prevRefData[i]);
        }
      }
      for (let i = 0; i < action.data.extraTileInds.length; i++) {
        const ind = action.data.extraTileInds[i];
        if (i < action.data.extraPrevRefData.length) {
          mapData.tiles[ind] = structuredClone(action.data.extraPrevRefData[i]);
        }
      }
      break;
    case PaintActionType.ERASE:
      // Restore previous tile data for all affected tiles
      for (let i = 0; i < action.data.tileInds.length; i++) {
        const ind = action.data.tileInds[i];
        if (i < action.data.prevRefData.length) {
          mapData.tiles[ind] = structuredClone(action.data.prevRefData[i]);
        }
      }
      break;
    case PaintActionType.ERASE_META:
      // Restore previous tile metadata for all affected tiles
      for (let i = 0; i < action.data.tileInds.length; i++) {
        const ind = action.data.tileInds[i];
        if (i < action.data.prevRefData.length) {
          const prevTile = action.data.prevRefData[i];
          const currentTile = mapData.tiles[ind];
          // Restore metadata but keep current base properties (tilesetName, tileId, x, y)
          currentTile.characters = structuredClone(prevTile.characters);
          currentTile.items = structuredClone(prevTile.items);
          if (prevTile.tileOverrides) {
            currentTile.tileOverrides = structuredClone(prevTile.tileOverrides);
          } else {
            delete currentTile.tileOverrides;
          }
          if (prevTile.lightSource) {
            currentTile.lightSource = structuredClone(prevTile.lightSource);
          } else {
            delete currentTile.lightSource;
          }
          if (prevTile.eventTrigger) {
            currentTile.eventTrigger = structuredClone(prevTile.eventTrigger);
          } else {
            delete currentTile.eventTrigger;
          }
          if (prevTile.travelTrigger) {
            currentTile.travelTrigger = structuredClone(prevTile.travelTrigger);
          } else {
            delete currentTile.travelTrigger;
          }
        }
      }
      break;
    case PaintActionType.FILL: {
      // Restore previous tile data for all affected tiles
      for (let i = 0; i < action.data.tileInds.length; i++) {
        const tileInd = action.data.tileInds[i];
        if (i < action.data.prevRefData.length) {
          mapData.tiles[tileInd] = structuredClone(action.data.prevRefData[i]);
        }
      }
      break;
    }
    case PaintActionType.SELECT: {
      // Restore both source and destination tiles to their previous state
      if (
        action.data.tileInds.length >= 2 &&
        action.data.prevRefData.length >= 2
      ) {
        const sourceTileIndex = action.data.startInd;
        const destTileIndex = action.data.endInd;

        if (sourceTileIndex >= 0 && destTileIndex >= 0) {
          // Restore source tile (index 0 in prevRefData)
          if (0 < action.data.prevRefData.length) {
            mapData.tiles[sourceTileIndex] = structuredClone(
              action.data.prevRefData[0]
            );
          }
          // Restore destination tile (index 1 in prevRefData)
          if (1 < action.data.prevRefData.length) {
            mapData.tiles[destTileIndex] = structuredClone(
              action.data.prevRefData[1]
            );
          }
        }
      }
      break;
    }
    case PaintActionType.CLONE: {
      // Restore destination tile to its previous state (source tile unchanged)
      if (
        action.data.tileInds.length >= 2 &&
        action.data.prevRefData.length >= 2
      ) {
        const destTileIndex = action.data.endInd;

        if (destTileIndex >= 0) {
          // Restore destination tile (index 1 in prevRefData)
          // Source tile (index 0) is not modified by CLONE, so we don't restore it
          if (1 < action.data.prevRefData.length) {
            mapData.tiles[destTileIndex] = structuredClone(
              action.data.prevRefData[1]
            );
          }
        }
      }
      break;
    }
    // case PaintActionType.MOVE:
    //   break;
    // case PaintActionType.COPY:
    //   break;
    // case PaintActionType.PASTE:
    case PaintActionType.REF_CHANGE:
      break;
  }
};

export const onActionUpdate = (
  action: PaintAction,
  mapData: CarcerMapTemplate,
  editorState: EditorState
) => {
  const ind =
    getEditorStateMap(editorState.selectedMapName)?.hoveredTileIndex ?? -1;

  if (ind === -1) {
    return;
  }

  if (action.data.startInd === -1) {
    action.data.startInd = ind;
  }
  action.data.endInd = ind;

  if (!action.data.tileInds.includes(ind)) {
    action.data.tileInds.push(ind);
    action.data.prevRefData.push(structuredClone(mapData.tiles[ind]));

    // ensure every tile affected by brush is also in tileInds array
    if (action.type == PaintActionType.DRAW && action.data.floorDrawBrush) {
      for (const ind of action.data.tileInds) {
        const startX = ind % mapData.width;
        const startY = Math.floor(ind / mapData.width);
        const floorDrawBrush = action.data.floorDrawBrush;
        if (floorDrawBrush?.length) {
          for (const bt of floorDrawBrush) {
            const newX = startX + bt.xOffset;
            const newY = startY + bt.yOffset;
            const newInd = newY * mapData.width + newX;

            if (
              newX < 0 ||
              newX >= mapData.width ||
              newY < 0 ||
              newY >= mapData.height
            ) {
              continue;
            }

            if (!action.data.extraTileInds.includes(newInd)) {
              action.data.extraTileInds.push(newInd);
              action.data.extraPrevRefData.push(
                structuredClone(mapData.tiles[newInd])
              );
            }
          }
        }
      }
    }

    applyActionUpdate(action, mapData, editorState);
  }
};

export const onActionComplete = (
  action: PaintAction,
  mapData: CarcerMapTemplate,
  editorState: EditorState
) => {
  currentAction = null;
  console.log('ACTION COMPLETE', action);
  applyAction(action, mapData, editorState);

  // Add action to undo history
  const newUndoHistory = [
    ...(getEditorStateMap(editorState.selectedMapName)?.undoHistory ?? []),
  ];
  const undoIndex =
    getEditorStateMap(editorState.selectedMapName)?.undoIndex ?? 0;

  // If we're not at the end of the history, slice off everything after the current index
  if (undoIndex < newUndoHistory.length - 1) {
    newUndoHistory.splice(undoIndex + 1);
  }

  // Add the new action to history
  newUndoHistory.push(structuredClone(action));
  const newUndoIndex = newUndoHistory.length - 1;

  updateEditorStateMapNoReRender(editorState.selectedMapName, {
    undoHistory: newUndoHistory,
    undoIndex: newUndoIndex,
  });
};

export const undo = (
  mapData: CarcerMapTemplate,
  editorState: EditorState
): boolean => {
  const { undoHistory, undoIndex } = getEditorStateMap(
    editorState.selectedMapName
  ) ?? { undoHistory: [], undoIndex: 0 };

  // Check if we can undo
  if (undoIndex < 0 || undoIndex >= undoHistory.length) {
    return false;
  }

  // Get the action to undo
  const actionToUndo = undoHistory[undoIndex];

  // Perform the undo
  undoAction(mapData, actionToUndo);

  // Update undo index and trigger re-render
  const newUndoIndex = undoIndex - 1;
  updateEditorStateMapNoReRender(editorState.selectedMapName, {
    undoIndex: newUndoIndex,
  });

  // Trigger re-render to show the undo
  (window as any).reRenderTileEditor?.();

  return true;
};

export const onTileHoverIndChange = (
  mapData: CarcerMapTemplate,
  currentPaintAction: PaintActionType,
  prevHoverInd: number,
  nextHoverInd: number
) => {
  if (nextHoverInd !== -1) {
    if (mapData && currentPaintAction === PaintActionType.FILL) {
      updateEditorStateNoReRender({
        fillIndsFloor: calculateFillIndsFloor(nextHoverInd, mapData),
      });
    } else {
      updateEditorStateNoReRender({
        fillIndsFloor: [],
      });
    }
  } else {
    updateEditorStateNoReRender({
      fillIndsFloor: [],
    });
  }

  if (getIsDraggingRight()) {
    updateEditorStateNoReRender({
      rectSelectTileIndEnd: nextHoverInd,
    });
  }
};
