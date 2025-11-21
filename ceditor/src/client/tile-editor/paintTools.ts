// import {
//   FloorBrushData,
//   getFillIndsFloor,
//   getHoveredTileInd,
//   getTileFloorBrush,
// } from './renderState';
import { calculateFillIndsFloor } from './fill';
import {
  CarcerMapTemplate,
  CarcerMapTileTemplate,
  createDefaultCarcerMapTile,
} from '../components/MapTemplateForm';
import { FloorBrushData } from './renderState';
import { EditorState, updateEditorStateNoReRender } from './editorState';
import { getIsDraggingRight } from './editorEvents';

export enum PaintActionType {
  NONE = '',
  DRAW = 'DRAW',
  ERASE = 'ERASE',
  FILL = 'FILL',
  SELECT = 'SELECT',
  MOVE = 'MOVE',
  COPY = 'COPY',
  PASTE = 'PASTE',
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
    case PaintActionType.FILL: {
      const ind = editorState.hoveredTileIndex;
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
    case PaintActionType.SELECT:
      break;
    case PaintActionType.MOVE:
      break;
    case PaintActionType.COPY:
      break;
    case PaintActionType.PASTE:
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
    case PaintActionType.FILL:
      break;
    case PaintActionType.SELECT:
      break;
    case PaintActionType.MOVE:
      break;
    case PaintActionType.COPY:
      break;
    case PaintActionType.PASTE:
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
    case PaintActionType.SELECT:
      break;
    case PaintActionType.MOVE:
      break;
    case PaintActionType.COPY:
      break;
    case PaintActionType.PASTE:
    case PaintActionType.REF_CHANGE:
      break;
  }
};

export const onActionUpdate = (
  action: PaintAction,
  mapData: CarcerMapTemplate,
  editorState: EditorState
) => {
  const ind = editorState.hoveredTileIndex;

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
  const newUndoHistory = [...editorState.undoHistory];
  const undoIndex = editorState.undoIndex;

  // If we're not at the end of the history, slice off everything after the current index
  if (undoIndex < newUndoHistory.length - 1) {
    newUndoHistory.splice(undoIndex + 1);
  }

  // Add the new action to history
  newUndoHistory.push(structuredClone(action));
  const newUndoIndex = newUndoHistory.length - 1;

  updateEditorStateNoReRender({
    undoHistory: newUndoHistory,
    undoIndex: newUndoIndex,
  });
};

export const undo = (
  mapData: CarcerMapTemplate,
  editorState: EditorState
): boolean => {
  const { undoHistory, undoIndex } = editorState;

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
  updateEditorStateNoReRender({
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
