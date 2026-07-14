import {
  PaintActionType,
  createPaintAction,
  getCurrentAction,
  onActionComplete,
  onTileHoverIndChange,
  setCurrentAction,
} from './paintTools';
import { CarcerMapTemplate, MapGridTemplate, TilesetTemplate } from '../types/assets';
import {
  commitMaterializedLayer,
  getAdjacentLayer,
  getMaterializedLayer,
} from '../utils/mapIndex';
import {
  EditorState,
  getCurrentPaintAction,
  getEditorState,
  getEditorStateMap,
  setCurrentPaintAction,
  updateEditorState,
  updateEditorStateMap,
  updateEditorStateMapNoReRender,
  updateEditorStateNoReRender,
} from './editorState';
import {
  findAdjacentGridSlotAtCanvasPoint,
  GridSlotHit,
} from './gridMapNavigation';
import { isGridSlotEditable } from '../utils/mapGridIndex';

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
  pendingGridSlotClick: GridSlotHit | null = null;
  gridSlotClickStartX = 0;
  gridSlotClickStartY = 0;
}
const mapEditorEventState = new MapEditorEventState();

let isPanZoomInitialized = false;
const panZoomEvents: {
  keydown: (ev: KeyboardEvent) => void;
  keyup: (ev: KeyboardEvent) => void;
  mousedown: (ev: MouseEvent) => void;
  mousemove: (ev: MouseEvent) => void;
  mouseup: (ev: MouseEvent) => void;
  contextmenu: (ev: MouseEvent) => void;
  wheel: (ev: WheelEvent) => void;
} = {
  keydown: () => {},
  keyup: () => {},
  mousedown: () => {},
  mousemove: () => {},
  mouseup: () => {},
  contextmenu: () => {},
  wheel: () => {},
};

const MOUSE_BUTTON_LEFT = 0;
const MOUSE_BUTTON_RIGHT = 2;
const MOUSE_BUTTON_MIDDLE = 1;
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

export interface GridSlotCreateRequest {
  gridName: string;
  cellX: number;
  cellY: number;
  mapWidth: number;
  mapHeight: number;
}

export interface GridNavigationHandlers {
  getMaps: () => CarcerMapTemplate[];
  getMapGrids: () => MapGridTemplate[];
  onNavigateToGridMap: (mapName: string) => void;
  onCreateGridMap: (request: GridSlotCreateRequest) => void;
}

let gridNavigationHandlers: GridNavigationHandlers | null = null;

export const setGridNavigationHandlers = (
  handlers: GridNavigationHandlers | null
) => {
  gridNavigationHandlers = handlers;
};

const GRID_SLOT_CLICK_DRAG_THRESHOLD = 6;

const findGridSlotAtScreen = (
  clientX: number,
  clientY: number,
  mapDataInterface: {
    getCanvas: () => HTMLCanvasElement;
    getMapData: () => CarcerMapTemplate;
  }
): GridSlotHit | null => {
  const handlers = gridNavigationHandlers;
  const currentMap = mapDataInterface.getMapData();
  const canvas = mapDataInterface.getCanvas();
  if (!handlers || !currentMap || !canvas) {
    return null;
  }

  const [canvasX, canvasY] = screenCoordsToCanvasCoords(
    clientX,
    clientY,
    canvas
  );

  return findAdjacentGridSlotAtCanvasPoint({
    canvasX,
    canvasY,
    canvas,
    map: currentMap,
    mapGrids: handlers.getMapGrids(),
    maps: handlers.getMaps(),
    translateX: mapEditorEventState.translateX,
    translateY: mapEditorEventState.translateY,
    scale: mapEditorEventState.scale,
  });
};

export const initPanzoom = (mapDataInterface: {
  getCanvas: () => HTMLCanvasElement;
  getMapData: () => CarcerMapTemplate;
  getTilesets: () => TilesetTemplate[];
  getEditorState: () => EditorState;
}) => {
  // if (panzoomCanvas !== null || isPanZoomInitialized) {
  //   return;
  // }
  // panzoomCanvas = canvas;
  const handleKeyDown = (ev: KeyboardEvent) => {
    if (shouldPreventDefault(ev)) {
      ev.preventDefault();
    }

    // Check if text cursor is focused on an input element
    const activeElement = document.activeElement;
    const isInputFocused =
      activeElement &&
      (activeElement.tagName === 'INPUT' ||
        activeElement.tagName === 'TEXTAREA' ||
        activeElement.getAttribute('contenteditable') === 'true');

    // keyboard shortcuts - only process if not typing in an input
    if (isEditorActive(ev) && !isInputFocused) {
      if (ev.key === 'b') {
        setCurrentPaintAction(PaintActionType.DRAW);
      } else if (ev.key === 'f') {
        setCurrentPaintAction(PaintActionType.FILL);
        // need to wait for a state update or currentPaintAction is not set
        setTimeout(() => {
          const ind =
            getEditorStateMap(mapDataInterface.getEditorState().selectedMapName)
              ?.hoveredTileIndex ?? -1;
          if (ind > -1) {
            onTileHoverIndChange(
              mapDataInterface.getMapData(),
              mapDataInterface.getEditorState(),
              mapDataInterface.getEditorState().currentPaintAction,
              -1,
              ind
            );
          }
        }, 33);
      } else if (ev.key === 'e') {
        if (!ev.ctrlKey) {
          setCurrentPaintAction(PaintActionType.ERASE);
        }
      } else if (ev.key === 's') {
        if (!ev.ctrlKey) {
          setCurrentPaintAction(PaintActionType.SELECT);
        }
      } else if (ev.key === 'c') {
        if (!ev.ctrlKey) {
          setCurrentPaintAction(PaintActionType.CLONE);
        }
      } else if (ev.key === 't') {
        if (!ev.ctrlKey) {
          setCurrentPaintAction(PaintActionType.TERRAIN);
        }
      } else if (ev.key === 'g' && !ev.ctrlKey) {
        const editorState = getEditorState();
        updateEditorState({ showGrid: !editorState.showGrid });
        ev.preventDefault();
      } else if (ev.key === 'Escape') {
        const mapName = mapDataInterface.getEditorState().selectedMapName;
        const editorState = mapDataInterface.getEditorState();
        const mapState = mapName ? getEditorStateMap(mapName) : undefined;
        const hasSelection = (mapState?.selectedTileInd ?? -1) >= 0;
        if (hasSelection || editorState.isSelectDragging) {
          ev.preventDefault();
          if (editorState.isSelectDragging) {
            updateEditorState({
              isSelectDragging: false,
              selectDragSourceTileIndex: -1,
            });
          }
          if (hasSelection && mapName) {
            updateEditorStateMap(mapName, { selectedTileInd: -1 });
          }
        }
      }
      if (
        (ev.key === 'ArrowUp' || ev.key === 'ArrowDown') &&
        !ev.ctrlKey &&
        !ev.metaKey &&
        !ev.shiftKey
      ) {
        const currentMap = mapDataInterface.getMapData();
        if (currentMap) {
          const direction = ev.key === 'ArrowUp' ? 'up' : 'down';
          const nextLevel = getAdjacentLayer(
            currentMap,
            getEditorState().currentLevel,
            direction,
          );
          if (nextLevel !== null) {
            updateEditorState({ currentLevel: nextLevel });
            ev.preventDefault();
          }
        }
      }
    }

    if (!isInputFocused && ev.key === 'Tab') {
      updateEditorStateNoReRender({
        drawOverlayText: true,
      });
      ev.preventDefault();
    }
  };
  const handleKeyUp = (ev: KeyboardEvent) => {
    const activeElement = document.activeElement;
    const isInputFocused =
      activeElement &&
      (activeElement.tagName === 'INPUT' ||
        activeElement.tagName === 'TEXTAREA' ||
        activeElement.getAttribute('contenteditable') === 'true');
    if (!isInputFocused && ev.key === 'Tab') {
      updateEditorStateNoReRender({
        drawOverlayText: false,
      });
      ev.preventDefault();
    }
  };
  const handleMouseDown = (ev: MouseEvent) => {
    if (
      ev.button === MOUSE_BUTTON_MIDDLE &&
      isEventWithCanvasTarget(ev, mapDataInterface.getCanvas())
    ) {
      mapEditorEventState.lastClickX = ev.clientX;
      mapEditorEventState.lastClickY = ev.clientY;
      mapEditorEventState.lastTranslateX = mapEditorEventState.translateX;
      mapEditorEventState.lastTranslateY = mapEditorEventState.translateY;
      mapEditorEventState.isDragging = true;
    }
    if (
      ev.button === MOUSE_BUTTON_LEFT &&
      isEventWithCanvasTarget(ev, mapDataInterface.getCanvas())
    ) {
      const gridSlotHit = findGridSlotAtScreen(ev.clientX, ev.clientY, mapDataInterface);
      if (gridSlotHit) {
        mapEditorEventState.pendingGridSlotClick = gridSlotHit;
        mapEditorEventState.gridSlotClickStartX = ev.clientX;
        mapEditorEventState.gridSlotClickStartY = ev.clientY;
        return;
      }

      const currentPaintAction =
        mapDataInterface.getEditorState().currentPaintAction;
      if (currentPaintAction === PaintActionType.NONE) {
        return;
      }

      // SELECT action: start dragging to move tile data
      if (currentPaintAction === PaintActionType.SELECT) {
        const hoveredTileIndex =
          getEditorStateMap(mapDataInterface.getEditorState().selectedMapName)
            ?.hoveredTileIndex ?? -1;
        if (hoveredTileIndex >= 0) {
          updateEditorState({
            selectDragSourceTileIndex: hoveredTileIndex,
            isSelectDragging: true,
          });
          updateEditorStateMapNoReRender(
            mapDataInterface.getEditorState().selectedMapName,
            {
              selectedTileInd: hoveredTileIndex,
            }
          );
        }
        return;
      }

      // CLONE action: start dragging to clone tile data
      if (currentPaintAction === PaintActionType.CLONE) {
        const hoveredTileIndex =
          getEditorStateMap(mapDataInterface.getEditorState().selectedMapName)
            ?.hoveredTileIndex ?? -1;
        if (hoveredTileIndex >= 0) {
          updateEditorState({
            selectDragSourceTileIndex: hoveredTileIndex,
            isSelectDragging: true,
          });
          updateEditorStateMapNoReRender(
            mapDataInterface.getEditorState().selectedMapName,
            {
              selectedTileInd: hoveredTileIndex,
            }
          );
        }
        return;
      }

      mapEditorEventState.isPainting = true;
      const action = createPaintAction(currentPaintAction);
      action.data.paintTileRef = {
        tilesetName: mapDataInterface.getEditorState().selectedTilesetName,
        tileId: mapDataInterface.getEditorState().selectedTileIndexInTileset,
      };
      const floorBrush = mapDataInterface.getEditorState().rectCloneBrushTiles;
      if (floorBrush.length) {
        action.data.floorDrawBrush = floorBrush.map((ft) => {
          return {
            xOffset: ft.xOffset,
            yOffset: ft.yOffset,
            originalTile: {
              ref: structuredClone(ft.originalTile.ref),
            },
          };
        });
      }
      setCurrentAction(action);
    }
    if (
      ev.button === MOUSE_BUTTON_RIGHT &&
      isEventWithCanvasTarget(ev, mapDataInterface.getCanvas()) &&
      (getCurrentPaintAction() === PaintActionType.DRAW ||
        getCurrentPaintAction() === PaintActionType.FILL ||
        getCurrentPaintAction() === PaintActionType.DELETE_FILL)
    ) {
      const hoveredTileIndex =
        getEditorStateMap(mapDataInterface.getEditorState().selectedMapName)
          ?.hoveredTileIndex ?? -1;
      if (hoveredTileIndex < 0) {
        return;
      }

      mapEditorEventState.isDraggingRight = true;

      updateEditorStateNoReRender({
        rectSelectTileIndStart: hoveredTileIndex,
        rectSelectTileIndEnd: hoveredTileIndex,
      });

      // setSelectedMapTileIndex(getHoveredTileInd());
    } else if (
      ev.button === MOUSE_BUTTON_RIGHT &&
      isEventWithCanvasTarget(ev, mapDataInterface.getCanvas()) &&
      [PaintActionType.CLONE, PaintActionType.SELECT].includes(
        getCurrentPaintAction()
      )
    ) {
      const hoveredTileIndex =
        getEditorStateMap(mapDataInterface.getEditorState().selectedMapName)
          ?.hoveredTileIndex ?? -1;
      if (hoveredTileIndex < 0) {
        return;
      }
      updateEditorStateMap(mapDataInterface.getEditorState().selectedMapName, {
        selectedTileInd: hoveredTileIndex,
      });
    }
  };
  const handleMouseMove = (ev: MouseEvent) => {
    mapEditorEventState.mouseX = ev.clientX;
    mapEditorEventState.mouseY = ev.clientY;

    const canvas = mapDataInterface.getCanvas();
    if (isEventWithCanvasTarget(ev, canvas)) {
      const gridSlotHit = findGridSlotAtScreen(ev.clientX, ev.clientY, mapDataInterface);
      const nextHovered = gridSlotHit
        ? {
            offsetX: gridSlotHit.slot.offsetX,
            offsetY: gridSlotHit.slot.offsetY,
          }
        : null;
      const prevHovered =
        mapDataInterface.getEditorState().hoveredGridAdjacentSlot;
      if (
        nextHovered?.offsetX !== prevHovered?.offsetX ||
        nextHovered?.offsetY !== prevHovered?.offsetY
      ) {
        updateEditorStateNoReRender({
          hoveredGridAdjacentSlot: nextHovered,
        });
      }
    } else if (mapDataInterface.getEditorState().hoveredGridAdjacentSlot) {
      updateEditorStateNoReRender({ hoveredGridAdjacentSlot: null });
    }

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
    if (mapEditorEventState.pendingGridSlotClick && ev.button === MOUSE_BUTTON_LEFT) {
      const pending = mapEditorEventState.pendingGridSlotClick;
      mapEditorEventState.pendingGridSlotClick = null;
      const dragDistance = Math.hypot(
        ev.clientX - mapEditorEventState.gridSlotClickStartX,
        ev.clientY - mapEditorEventState.gridSlotClickStartY
      );
      if (dragDistance <= GRID_SLOT_CLICK_DRAG_THRESHOLD) {
        const handlers = gridNavigationHandlers;
        if (handlers) {
          if (isGridSlotEditable(pending.slot)) {
            handlers.onNavigateToGridMap(pending.slot.mapName);
          } else {
            handlers.onCreateGridMap({
              gridName: pending.placement.grid.name,
              cellX: pending.slot.cellX,
              cellY: pending.slot.cellY,
              mapWidth: pending.placement.grid.mapWidth,
              mapHeight: pending.placement.grid.mapHeight,
            });
          }
        }
      }
    }

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
      mapEditorEventState.isPainting = false;
      const currentAction = getCurrentAction();
      const mapData = mapDataInterface.getMapData();
      if (currentAction && mapData) {
        onActionComplete(
          currentAction,
          mapData,
          mapDataInterface.getEditorState()
        );
      }
      updateEditorStateMap(mapDataInterface.getEditorState().selectedMapName, {
        selectedTileInd:
          getEditorStateMap(mapDataInterface.getEditorState().selectedMapName)
            ?.hoveredTileIndex ?? -1,
      });
    }
    // Handle SELECT/CLONE drag completion
    const editorState = mapDataInterface.getEditorState();
    if (editorState.isSelectDragging && ev.button === 0) {
      const sourceTileIndex = editorState.selectDragSourceTileIndex;
      const destTileIndex =
        getEditorStateMap(editorState.selectedMapName)?.hoveredTileIndex ?? -1;
      const currentPaintAction = editorState.currentPaintAction;

      // Only create action if dragging to a different tile
      if (
        sourceTileIndex >= 0 &&
        destTileIndex >= 0 &&
        sourceTileIndex !== destTileIndex
      ) {
        const actionType =
          currentPaintAction === PaintActionType.CLONE
            ? PaintActionType.CLONE
            : PaintActionType.SELECT;
        const action = createPaintAction(actionType);
        action.data.startInd = sourceTileIndex;
        action.data.endInd = destTileIndex;
        action.data.tileInds = [sourceTileIndex, destTileIndex];

        const mapData = mapDataInterface.getMapData();
        if (mapData) {
          const mapTiles = getTileList(mapData);
          // Store previous state of both tiles for undo
          action.data.prevRefData.push(
            structuredClone(mapTiles[sourceTileIndex])
          );
          action.data.prevRefData.push(
            structuredClone(mapTiles[destTileIndex])
          );

          onActionComplete(action, mapData, editorState);
        }
      }

      updateEditorState({
        isSelectDragging: false,
        selectDragSourceTileIndex: -1,
      });
      updateEditorStateMapNoReRender(editorState.selectedMapName, {
        selectedTileInd:
          destTileIndex >= 0
            ? destTileIndex
            : getEditorStateMap(editorState.selectedMapName)?.selectedTileInd ??
              -1,
      });
    }
    if (mapEditorEventState.isDraggingRight) {
      mapEditorEventState.isDraggingRight = false;
      const ind0 = mapDataInterface.getEditorState().rectSelectTileIndStart;
      const ind1 = mapDataInterface.getEditorState().rectSelectTileIndEnd;
      const dragSelectedInds = getIndsOfBoundingRect(
        ind0,
        ind1,
        mapDataInterface.getMapData().width ?? 0
      );
      if (dragSelectedInds.length === 0) {
        return;
      }
      const mapTiles = getTileList(mapDataInterface.getMapData());
      const nextRef = mapTiles[dragSelectedInds[0]];
      if (dragSelectedInds.length === 1 && nextRef) {
        updateEditorStateNoReRender({
          rectCloneBrushTiles: [],
          selectedTileIndexInTileset: nextRef.tileId,
          selectedTilesetName: nextRef.tilesetName,
        });
        updateEditorStateMap(
          mapDataInterface.getEditorState().selectedMapName,
          {
            selectedTileInd:
              getEditorStateMap(
                mapDataInterface.getEditorState().selectedMapName
              )?.hoveredTileIndex ?? -1,
          }
        );
        return;
      }
      const mapWidth = mapDataInterface.getMapData().width ?? 0;
      const mapHeight = mapDataInterface.getMapData().height ?? 0;
      const [topLeftX, topLeftY] = [
        ind0 % mapWidth,
        Math.floor(ind0 / mapWidth),
      ];
      const brush = dragSelectedInds.map((ind) => {
        const [x, y] = [ind % mapWidth, Math.floor(ind / mapWidth)];
        return {
          xOffset: x - topLeftX,
          yOffset: y - topLeftY,
          originalTile: {
            ref: structuredClone(mapTiles[ind]),
          },
        };
      });
      const selectedMapName = mapDataInterface.getEditorState().selectedMapName;
      updateEditorStateNoReRender({
        rectCloneBrushTiles: brush,
      });
      updateEditorStateMap(selectedMapName, {
        selectedTileInd:
          getEditorStateMap(selectedMapName)?.hoveredTileIndex ?? -1,
      });
    }
  };
  const handleContextMenu = (ev: MouseEvent) => {
    if (isEventWithCanvasTarget(ev, mapDataInterface.getCanvas())) {
      ev.preventDefault();
    }
  };
  const handleWheel = (ev: WheelEvent) => {
    if (!isEventWithCanvasTarget(ev, mapDataInterface.getCanvas())) {
      return;
    }
    ev.preventDefault();

    const [focalX, focalY] = screenCoordsToCanvasCoords(
      ev.clientX,
      ev.clientY,
      mapDataInterface.getCanvas()
    );

    // Normalize delta across mice (lines) and trackpads (pixels).
    let delta = ev.deltaY;
    if (ev.deltaMode === 1) {
      delta *= 16;
    } else if (ev.deltaMode === 2) {
      delta *= 100;
    }

    // Multiplicative zoom tracks continuous scroll; ~15% per 100px of delta.
    const zoomFactor = Math.exp(-delta * 0.0015);
    let nextScale = mapEditorEventState.scale * zoomFactor;
    if (nextScale > 10) {
      nextScale = 10;
    } else if (nextScale < 0.5) {
      nextScale = 0.5;
    }

    if (nextScale === mapEditorEventState.scale) {
      return;
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
  };
  window.addEventListener('keydown', handleKeyDown);
  window.addEventListener('keyup', handleKeyUp);
  window.addEventListener('mousedown', handleMouseDown);
  window.addEventListener('mousemove', handleMouseMove);
  window.addEventListener('mouseup', handleMouseUp);
  window.addEventListener('contextmenu', handleContextMenu);
  window.addEventListener('wheel', handleWheel, { passive: false });

  isPanZoomInitialized = true;
  panZoomEvents.keydown = handleKeyDown;
  panZoomEvents.keyup = handleKeyUp;
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
  window.removeEventListener('keyup', panZoomEvents.keyup);
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

export const updateMapCanvasCursor = (
  canvas: HTMLCanvasElement | null,
  paintAction: PaintActionType,
  hoveredTileIndex: number,
  isSelectDragging = false,
  hoveredGridAdjacentSlot: { offsetX: number; offsetY: number } | null = null
) => {
  if (!canvas) {
    return;
  }
  if (mapEditorEventState.isDragging || isSelectDragging) {
    canvas.style.cursor = 'grabbing';
    return;
  }
  if (hoveredGridAdjacentSlot) {
    canvas.style.cursor = 'pointer';
    return;
  }
  if (paintAction === PaintActionType.SELECT && hoveredTileIndex >= 0) {
    canvas.style.cursor = 'pointer';
    return;
  }
  if (paintAction === PaintActionType.CLONE) {
    canvas.style.cursor = 'grab';
    return;
  }
  canvas.style.cursor = '';
};

export const resetPanzoom = () => {
  mapEditorEventState.translateX = 0;
  mapEditorEventState.translateY = 0;
  mapEditorEventState.scale = 1;
};

export const saveViewportForMap = (mapName: string) => {
  if (!mapName) {
    return;
  }
  updateEditorStateMapNoReRender(mapName, {
    viewport: {
      translateX: mapEditorEventState.translateX,
      translateY: mapEditorEventState.translateY,
      scale: mapEditorEventState.scale,
    },
  });
};

export const restoreViewportForMap = (mapName: string) => {
  if (!mapName) {
    resetPanzoom();
    return;
  }
  const viewport = getEditorStateMap(mapName)?.viewport;
  if (viewport) {
    mapEditorEventState.translateX = viewport.translateX;
    mapEditorEventState.translateY = viewport.translateY;
    mapEditorEventState.scale = viewport.scale;
    return;
  }
  resetPanzoom();
};

export const switchMapViewport = (fromMapName: string, toMapName: string) => {
  if (fromMapName) {
    saveViewportForMap(fromMapName);
  }
  restoreViewportForMap(toMapName);
};

/** Pan the map view so the tile center is at the canvas center. */
export const centerViewOnTile = (
  canvas: HTMLCanvasElement,
  mapData: CarcerMapTemplate,
  tileIndex: number
) => {
  const tileX = tileIndex % mapData.width;
  const tileY = Math.floor(tileIndex / mapData.width);
  const tileCenterMapX = (tileX + 0.5) * mapData.spriteWidth;
  const tileCenterMapY = (tileY + 0.5) * mapData.spriteHeight;
  const mapWidth = mapData.width * mapData.spriteWidth;
  const mapHeight = mapData.height * mapData.spriteHeight;
  const scale = mapEditorEventState.scale;
  const canvasW = canvas.width;
  const canvasH = canvas.height;

  mapEditorEventState.translateX =
    (canvasW / 2) * (1 - scale) + scale * (mapWidth / 2 - tileCenterMapX);
  mapEditorEventState.translateY =
    (canvasH / 2) * (1 - scale) + scale * (mapHeight / 2 - tileCenterMapY);
};

export const getIndsOfBoundingRect = (
  ind0: number,
  ind1: number,
  width: number
): number[] => {
  if (ind0 === -1) {
    return [];
  }
  if (ind1 === -1) {
    return [];
  }

  const x0 = ind0 % width;
  const y0 = Math.floor(ind0 / width);
  const x1 = ind1 % width;
  const y1 = Math.floor(ind1 / width);

  const minX = Math.min(x0, x1);
  const maxX = Math.max(x0, x1);
  const minY = Math.min(y0, y1);
  const maxY = Math.max(y0, y1);

  const inds: number[] = [];
  for (let y = minY; y <= maxY; y++) {
    for (let x = minX; x <= maxX; x++) {
      inds.push(y * width + x);
    }
  }

  return inds;
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

export const getTileList = (mapData: CarcerMapTemplate, level?: number) => {
  return getMaterializedLayer(mapData, level ?? getEditorState().currentLevel);
};

export const commitCurrentLayer = (
  mapData: CarcerMapTemplate,
  level?: number
) => {
  commitMaterializedLayer(mapData, level ?? getEditorState().currentLevel);
};
