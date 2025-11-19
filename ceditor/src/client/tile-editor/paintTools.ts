import {
  FloorBrushData,
  getFillIndsFloor,
  getHoveredTileInd,
  getTileFloorBrush,
} from './renderState';
import { calculateFillIndsFloor } from './fill';
import {
  CarcerMapTemplate,
  CarcerMapTileTemplate,
  createDefaultCarcerMapTile,
} from '../components/MapTemplateForm';

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
  layer: 'floor' | 'top' | 'object';
  // paintTileId: number;
  paintTileRef: CarcerMapTileTemplate;
  floorDrawBrush?: FloorBrushData[];
  startInd: number;
  endInd: number;
  tileInds: number[];

  prevRefData: CarcerMapTileTemplate[];
  // prevObjectList: MapObject[];
}

interface PaintAction {
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

      prevRefData: [],
      // prevObjectList: [],
    },
  };
  actionList.push(paintAction);
  return paintAction;
};

export const applyAction = (
  action: PaintAction,
  mapData: CarcerMapTemplate
) => {
  switch (action.type) {
    case PaintActionType.DRAW:
      break;
    case PaintActionType.ERASE:
      break;
    case PaintActionType.FILL: {
      const ind = getHoveredTileInd();
      const fillIndsFloor = calculateFillIndsFloor(ind, mapData);
      for (const ind of fillIndsFloor) {
        mapData.tiles[ind] = structuredClone(action.data.paintTileRef);
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
  mapData: CarcerMapTemplate
) => {
  switch (action.type) {
    case PaintActionType.DRAW:
      for (const ind of action.data.tileInds) {
        if (action.data.layer === 'floor') {
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

              mapData.tiles[newInd] = structuredClone(bt.originalTile.ref);
            }
          } else {
            mapData.tiles[ind] = structuredClone(action.data.paintTileRef);
          }
        } else if (action.data.layer === 'object') {
          // mapData.objectList[ind] = {
          //   ...action.data.objectList[ind],
          // };
        }
      }
      break;
    case PaintActionType.ERASE:
      for (const ind of action.data.tileInds) {
        if (action.data.layer === 'floor') {
          mapData.tiles[ind] = createDefaultCarcerMapTile();
        } else if (action.data.layer === 'object') {
          // mapData.objectList[ind] = {
          //   ...action.data.objectList[ind],
          // };
        }
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
      // action.data.tileInds.forEach(ind => {
      //   mapData.floor[ind] = 0;
      // });
      break;
    case PaintActionType.ERASE:
      // action.data.tileInds.forEach(ind => {
      //   mapData.floor[ind] = 1;
      // });
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

export const onActionUpdate = (
  action: PaintAction,
  mapData: CarcerMapTemplate
) => {
  const ind = getHoveredTileInd();

  if (ind === -1) {
    return;
  }

  if (action.data.startInd === -1) {
    action.data.startInd = ind;
  }
  action.data.endInd = ind;

  if (!action.data.tileInds.includes(ind)) {
    action.data.tileInds.push(ind);
    if (action.data.layer === 'floor') {
      action.data.prevRefData.push(structuredClone(mapData.tiles[ind]));
    } else if (action.data.layer === 'object') {
      // action.data.prevObjectList.push({ ...mapData.objectList[ind] });
    }

    applyActionUpdate(action, mapData);
  }
};

export const onActionComplete = (
  action: PaintAction,
  mapData: CarcerMapTemplate
) => {
  currentAction = null;
  console.log('ACTION COMPLETE', action);
  applyAction(action, mapData);
};
