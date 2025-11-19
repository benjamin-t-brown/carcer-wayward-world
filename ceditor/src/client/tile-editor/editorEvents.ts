// import { isAnyModalVisible } from 'components/NotifyModal';
import {
  PaintActionType,
  createPaintAction,
  getCurrentAction,
  onActionComplete,
  setCurrentAction,
} from './paintTools';
// import {
//   getCurrentMapData,
//   getCurrentPaintAction,
//   getCurrentSelectedTileColor,
//   getCurrentSelectedTileId,
//   setCurrentPaintAction,
//   setSelectedMapTileIndex,
//   setSelectedTileFromTileId,
// } from 'stateHelpers';
// import { getIndsOfBoundingRect } from 'utils';
import {
  getHoveredTileInd,
  getRectSelectTileInds,
  getTileFloorBrush,
  setFillIndsFloor,
  setRectSelectTileIndEnd,
  setRectSelectTileIndStart,
  setTileFloorBrush,
} from './renderState';
import { calculateFillIndsFloor } from './fill';
import { CarcerMapTemplate } from '../components/MapTemplateForm';
import { getCurrentPaintAction, getCurrentSelectedTileId } from './editorState';
// import { saveSettingsToLs } from 'state';

class MapEditorEventState {
  isDragging = false;
  isPainting = false;
  isDraggingRight = false;
  lastClickX = 0;
  lastClickY = 0;
  lastTranslateX = 0;
  lastTranslateY = 0;
  translateX = 0;
  translateY = 0;
  scale = 1;
  mouseX = 0;
  mouseY = 0;
}
const mapEditorEventState = new MapEditorEventState();

let isPanZoomInitialized = false;
const panZoomEvents: {
  keydown: (ev: KeyboardEvent) => void;
  mousedown: (ev: MouseEvent) => void;
  mousemove: (ev: MouseEvent) => void;
  mouseup: (ev: MouseEvent) => void;
  contextmenu: (ev: MouseEvent) => void;
  wheel: (ev: WheelEvent) => void;
} = {
  keydown: () => {},
  mousedown: () => {},
  mousemove: () => {},
  mouseup: () => {},
  contextmenu: () => {},
  wheel: () => {},
};

(window as any).mapEditorEventState = mapEditorEventState;
// let panzoomCanvas: HTMLCanvasElement | null = null;

const isEventWithCanvasTarget = (
  ev: MouseEvent,
  panzoomCanvas: HTMLCanvasElement | undefined
) => {
  const targetId = (ev.target as unknown as HTMLElement)?.id;
  return (
    (Boolean(panzoomCanvas) && ev.target === panzoomCanvas) ||
    targetId === 'map-canvas'
  );
};

const isEditorActive = (ev: KeyboardEvent) => {
  return true; // TODO check if any modals are open
};

const shouldPreventDefault = (ev: KeyboardEvent) => {
  return ev.ctrlKey && (ev.key === 's' || ev.key === 'e');
};

export const onTileHoverIndChange = (
  mapData: CarcerMapTemplate,
  prevHoverInd: number,
  nextHoverInd: number
) => {
  if (nextHoverInd !== -1) {
    const currentPaintAction = getCurrentPaintAction();
    if (mapData && currentPaintAction === PaintActionType.FILL) {
      setFillIndsFloor(calculateFillIndsFloor(nextHoverInd, mapData));
    } else {
      setFillIndsFloor([]);
    }
  } else {
    setFillIndsFloor([]);
  }

  if (getIsDraggingRight()) {
    setRectSelectTileIndEnd(nextHoverInd);
  }
};

export const initPanzoom = (mapDataInterface: {
  getCanvas: () => HTMLCanvasElement;
  getMapData: () => CarcerMapTemplate;
}) => {
  // if (panzoomCanvas !== null || isPanZoomInitialized) {
  //   return;
  // }
  // panzoomCanvas = canvas;
  const handleKeyDown = (ev: KeyboardEvent) => {
    if (shouldPreventDefault(ev)) {
      ev.preventDefault();
    }

    // keyboard shortcuts
    // if (isEditorActive(ev)) {
    //   if (ev.key === 'b') {
    //     setCurrentPaintAction(PaintActionType.DRAW);
    //   } else if (ev.key === 'f') {
    //     setCurrentPaintAction(PaintActionType.FILL);
    //     // need to wait for a state update or currentPaintAction is not set
    //     setTimeout(() => {
    //       const ind = getHoveredTileInd();
    //       if (ind > -1) {
    //         onTileHoverIndChange(ind, ind);
    //       }
    //     }, 33);
    //   } else if (ev.key === 'e') {
    //     if (!ev.ctrlKey) {
    //       setCurrentPaintAction(PaintActionType.ERASE);
    //     }
    //   } else if (ev.key === 's') {
    //     if (!ev.ctrlKey) {
    //       setCurrentPaintAction(PaintActionType.SELECT);
    //     }
    //   }
    // }
  };
  const handleMouseDown = (ev: MouseEvent) => {
    if (
      ev.button === 1 &&
      isEventWithCanvasTarget(ev, mapDataInterface.getCanvas())
    ) {
      mapEditorEventState.lastClickX = ev.clientX;
      mapEditorEventState.lastClickY = ev.clientY;
      mapEditorEventState.lastTranslateX = mapEditorEventState.translateX;
      mapEditorEventState.lastTranslateY = mapEditorEventState.translateY;
      mapEditorEventState.isDragging = true;
    }
    if (
      ev.button === 0 &&
      isEventWithCanvasTarget(ev, mapDataInterface.getCanvas())
    ) {
      // TODO is this for painting?
    //   mapEditorEventState.isPainting = true;
    //   const currentPaintAction = getCurrentPaintAction();
    //   const action = createPaintAction(currentPaintAction);
    //   // action.data.paintTileRef = {
    //   //   arr: [getCurrentSelectedTileId()],
    //   //   opacity: [100],
    //   //   // color: [getCurrentSelectedTileColor()],
    //   // };
    // action.
    //   const floorBrush = getTileFloorBrush();
    //   if (floorBrush.length) {
    //     action.data.floorDrawBrush = floorBrush.slice();
    //   }
    //   setCurrentAction(action);
    }
    if (
      ev.button === 2 &&
      isEventWithCanvasTarget(ev, mapDataInterface.getCanvas()) &&
      (getCurrentPaintAction() === PaintActionType.DRAW ||
        getCurrentPaintAction() === PaintActionType.FILL)
    ) {
      mapEditorEventState.isDraggingRight = true;

      setRectSelectTileIndStart(getHoveredTileInd());
      setRectSelectTileIndEnd(getHoveredTileInd());
      // setSelectedMapTileIndex(getHoveredTileInd());
    }
  };
  const handleMouseMove = (ev: MouseEvent) => {
    mapEditorEventState.mouseX = ev.clientX;
    mapEditorEventState.mouseY = ev.clientY;

    if (mapEditorEventState.isDragging) {
      mapEditorEventState.translateX =
        mapEditorEventState.lastTranslateX +
        ev.clientX -
        mapEditorEventState.lastClickX;
      mapEditorEventState.translateY =
        mapEditorEventState.lastTranslateY +
        ev.clientY -
        mapEditorEventState.lastClickY;
    }
  };
  const handleMouseUp = (ev: MouseEvent) => {
    if (mapEditorEventState.isDragging) {
      mapEditorEventState.translateX =
        mapEditorEventState.lastTranslateX +
        ev.clientX -
        mapEditorEventState.lastClickX;
      mapEditorEventState.translateY =
        mapEditorEventState.lastTranslateY +
        ev.clientY -
        mapEditorEventState.lastClickY;
      mapEditorEventState.isDragging = false;
    }
    if (mapEditorEventState.isPainting) {
      // mapEditorEventState.isPainting = false;
      // const currentAction = getCurrentAction();
      // const mapData = getCurrentMapData();
      // if (currentAction && mapData) {
      //   onActionComplete(currentAction, mapData);
      // }
      // setSelectedMapTileIndex(getHoveredTileInd());
    }
    if (mapEditorEventState.isDraggingRight) {
      // mapEditorEventState.isDraggingRight = false;
      // const mapData = getCurrentMapData();
      // if (!mapData) {
      //   setTileFloorBrush([]);
      //   return;
      // }
      // const [ind0, ind1] = getRectSelectTileInds();
      // const dragSelectedInds = getIndsOfBoundingRect(
      //   ind0,
      //   ind1,
      //   mapData?.width ?? 0
      // );
      // if (dragSelectedInds.length === 0) {
      //   setTileFloorBrush([]);
      //   setSelectedMapTileIndex(getHoveredTileInd());
      //   return;
      // }
      // const nextRef = mapData.ref[dragSelectedInds[0]];
      // if (dragSelectedInds.length === 1 && nextRef) {
      //   setTileFloorBrush([]);
      //   setSelectedTileFromTileId(
      //     mapData.ref[dragSelectedInds[0]]?.arr?.[0] ?? 0
      //   );
      // }
      // const [topLeftX, topLeftY] = [
      //   ind0 % mapData.width,
      //   Math.floor(ind0 / mapData.width),
      // ];
      // const brush = dragSelectedInds.map((ind) => {
      //   const [x, y] = [ind % mapData.width, Math.floor(ind / mapData.width)];
      //   return {
      //     xOffset: x - topLeftX,
      //     yOffset: y - topLeftY,
      //     originalTile: {
      //       ref: mapData.ref[ind],
      //     },
      //   };
      // });
      // setTileFloorBrush(brush);
      // setSelectedMapTileIndex(getHoveredTileInd());
    }
  };
  const handleContextMenu = (ev: MouseEvent) => {
    if (isEventWithCanvasTarget(ev, mapDataInterface.getCanvas())) {
      ev.preventDefault();
    }
  };
  const handleWheel = (ev: WheelEvent) => {
    if (isEventWithCanvasTarget(ev, mapDataInterface.getCanvas())) {
      const [focalX, focalY] = screenCoordsToCanvasCoords(
        ev.clientX,
        ev.clientY,
        mapDataInterface.getCanvas()
      );

      let nextScale = mapEditorEventState.scale;
      const scaleStep = 0.5;

      if (ev.deltaY > 0) {
        // zoom out
        nextScale -= scaleStep;
      } else {
        // zoom in
        nextScale += scaleStep;
      }

      if (nextScale > 10) {
        nextScale = 10;
      } else if (nextScale < 0.5) {
        nextScale = 0.5;
      }

      const offsetX =
        focalX -
        (nextScale / mapEditorEventState.scale) *
          (focalX - mapEditorEventState.translateX);
      const offsetY =
        focalY -
        (nextScale / mapEditorEventState.scale) *
          (focalY - mapEditorEventState.translateY);

      mapEditorEventState.translateX = offsetX;
      mapEditorEventState.translateY = offsetY;
      mapEditorEventState.scale = nextScale;
    }
  };
  window.addEventListener('keydown', handleKeyDown);
  window.addEventListener('mousedown', handleMouseDown);
  window.addEventListener('mousemove', handleMouseMove);
  window.addEventListener('mouseup', handleMouseUp);
  window.addEventListener('contextmenu', handleContextMenu);
  window.addEventListener('wheel', handleWheel);

  isPanZoomInitialized = true;
  panZoomEvents.keydown = handleKeyDown;
  panZoomEvents.mousedown = handleMouseDown;
  panZoomEvents.mousemove = handleMouseMove;
  panZoomEvents.mouseup = handleMouseUp;
  panZoomEvents.contextmenu = handleContextMenu;
  panZoomEvents.wheel = handleWheel;
};

export const unInitPanzoom = () => {
  if (!isPanZoomInitialized || !panZoomEvents) {
    return;
  }
  window.removeEventListener('keydown', panZoomEvents.keydown);
  window.removeEventListener('mousedown', panZoomEvents.mousedown);
  window.removeEventListener('mousemove', panZoomEvents.mousemove);
  window.removeEventListener('mouseup', panZoomEvents.mouseup);
  window.removeEventListener('contextmenu', panZoomEvents.contextmenu);
  window.removeEventListener('wheel', panZoomEvents.wheel);
  isPanZoomInitialized = false;
};

export const getTransform = () => {
  return {
    x: mapEditorEventState.translateX,
    y: mapEditorEventState.translateY,
    scale: mapEditorEventState.scale,
  };
};

export const resetPanzoom = () => {
  mapEditorEventState.translateX = 0;
  mapEditorEventState.translateY = 0;
  mapEditorEventState.scale = 1;
};

export const screenCoordsToCanvasCoords = (
  x: number,
  y: number,
  panzoomCanvas: HTMLCanvasElement
) => {
  const { left, top } = panzoomCanvas?.getBoundingClientRect() || {
    left: 0,
    top: 0,
    width: 0,
    height: 0,
  };

  const canvasX = x - left;
  const canvasY = y - top;

  return [canvasX, canvasY];
};

export const screenCoordsToMapCoords = (
  x: number,
  y: number,
  mapData: CarcerMapTemplate,
  panzoomCanvas: HTMLCanvasElement
) => {
  if (!panzoomCanvas) {
    return [0, 0];
  }
  const [canvasX, canvasY] = screenCoordsToCanvasCoords(x, y, panzoomCanvas);
  const canvas = panzoomCanvas;

  const canvasW = canvas.width;
  const canvasH = canvas.height;

  const mapX =
    canvasX -
    mapEditorEventState.translateX -
    mapEditorEventState.scale *
      (canvasW / 2 - (mapData.width * mapData.spriteWidth) / 2);
  const mapY =
    canvasY -
    mapEditorEventState.translateY -
    mapEditorEventState.scale *
      (canvasH / 2 - (mapData.height * mapData.spriteHeight) / 2);

  return [mapX / mapEditorEventState.scale, mapY / mapEditorEventState.scale];
};

export const screenCoordsToTileIndex = (
  x: number,
  y: number,
  mapData: CarcerMapTemplate,
  panzoomCanvas: HTMLCanvasElement
): [number, number, number, number, number] => {
  const [mapX, mapY] = screenCoordsToMapCoords(x, y, mapData, panzoomCanvas);
  const tileX = Math.floor(mapX / mapData.spriteWidth);
  const tileY = Math.floor(mapY / mapData.spriteHeight);
  if (
    mapX > mapData.width * mapData.spriteWidth ||
    mapY > mapData.height * mapData.spriteHeight ||
    mapX < 0 ||
    mapY < 0
  ) {
    return [-1, tileX, tileY, mapX, mapY];
  }

  return [tileY * mapData.width + tileX, tileX, tileY, mapX, mapY];
};

export const getScreenMouseCoords = () => {
  return [mapEditorEventState.mouseX, mapEditorEventState.mouseY];
};

export const getIsDraggingRight = () => {
  return mapEditorEventState.isDraggingRight;
};
