import {
  PaintActionType,
  createPaintAction,
  onActionComplete,
  onTileHoverIndChange,
} from './paintTools';
import { TOOL_LIST } from './tools';
import {
  CarcerMapTemplate,
  MapGridTemplate,
  TilesetTemplate,
} from '../types/assets';
import {
  commitMaterializedLayer,
  getAdjacentLayer,
  getMaterializedLayer,
  getTileGraphic,
} from '../utils/mapIndex';
import {
  clearAllSelectedTiles,
  EditorState,
  ensureEditorStateMap,
  getEditorStateMap,
  setCurrentPaintAction,
  setSoleSelectedTile,
  updateEditorState,
  updateEditorStateMap,
  updateEditorStateMapNoReRender,
  updateEditorStateNoReRender,
} from './editorState';
import {
  findAdjacentGridSlotAtCanvasPoint,
  GridSlotHit,
} from './gridMapNavigation';
import {
  findMapGridPlacement,
  isGridSlotEditable,
  resolveGridBrushCell,
} from '../utils/mapGridIndex';
import type { FloorBrushData } from './renderState';
import { MapEditorController } from './MapEditorController';
import type { MapDocumentIndex } from './mapDocumentIndex';
import type { MapGridPlacement } from '../utils/mapGridIndex';

const MOUSE_BUTTON_LEFT = 0;
const MOUSE_BUTTON_RIGHT = 2;
const MOUSE_BUTTON_MIDDLE = 1;

const isEventWithCanvasTarget = (
  ev: MouseEvent,
  canvas: HTMLCanvasElement | undefined,
) => Boolean(canvas) && (ev.target === canvas || ev.currentTarget === canvas);

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

export interface GridNavigateStitchOffset {
  /** Grid cell offset from the current map (may span more than one cell). */
  offsetX: number;
  offsetY: number;
}

export interface GridNavigationHandlers {
  getMaps: () => CarcerMapTemplate[];
  getMapGrids: () => MapGridTemplate[];
  getDocumentIndex?: () => MapDocumentIndex;
  onNavigateToGridMap: (
    mapName: string,
    stitchOffset: GridNavigateStitchOffset,
  ) => void;
  onCreateGridMap: (request: GridSlotCreateRequest) => void;
}

export const setGridNavigationHandlers = (
  controller: MapEditorController,
  handlers: GridNavigationHandlers | null,
) => {
  controller.setGridNavigationHandlers(handlers);
};

const GRID_SLOT_CLICK_DRAG_THRESHOLD = 6;

/**
 * Maps + grids for grid-aware tools (cross-block brush stamping and its
 * preview). Backed by the same handlers the canvas navigation uses; null before
 * they are registered or outside the map editor.
 */
export const getGridPaintContext = (
  controller: MapEditorController,
): {
  maps: CarcerMapTemplate[];
  mapGrids: MapGridTemplate[];
  mapsByName: ReadonlyMap<string, CarcerMapTemplate>;
  placementsByMapName: MapDocumentIndex['placementsByMapName'];
} | null => {
  const handlers = controller.getGridNavigationHandlers();
  if (!handlers) {
    return null;
  }
  const maps = handlers.getMaps();
  const index = handlers.getDocumentIndex?.();
  return {
    maps,
    mapGrids: handlers.getMapGrids(),
    mapsByName:
      index?.mapsByName ?? new Map(maps.map((map) => [map.name, map])),
    placementsByMapName: index?.placementsByMapName ?? new Map(),
  };
};

const findGridSlotAtScreen = (
  controller: MapEditorController,
  clientX: number,
  clientY: number,
  mapDataInterface: {
    getCanvas: () => HTMLCanvasElement;
    getMapData: () => CarcerMapTemplate;
  },
): GridSlotHit | null => {
  const handlers = controller.getGridNavigationHandlers();
  const currentMap = mapDataInterface.getMapData();
  const canvas = mapDataInterface.getCanvas();
  if (!handlers || !currentMap || !canvas) {
    return null;
  }

  const [canvasX, canvasY] = screenCoordsToCanvasCoords(
    controller,
    clientX,
    clientY,
    canvas,
  );

  const index = handlers.getDocumentIndex?.();
  const hit = findAdjacentGridSlotAtCanvasPoint({
    canvasX,
    canvasY,
    canvas,
    map: currentMap,
    mapGrids: handlers.getMapGrids(),
    maps: handlers.getMaps(),
    translateX: controller.input.translateX,
    translateY: controller.input.translateY,
    scale: controller.input.scale,
    mapsByName: index?.mapsByName,
    placement: index
      ? (index.placementsByMapName.get(currentMap.name)?.[0] ?? null)
      : undefined,
  });
  if (
    hit?.slot.map &&
    controller.getState().gridEditEnabled &&
    hit.slot.map.width === hit.placement.grid.mapWidth &&
    hit.slot.map.height === hit.placement.grid.mapHeight &&
    hit.slot.map.spriteWidth === currentMap.spriteWidth &&
    hit.slot.map.spriteHeight === currentMap.spriteHeight
  ) {
    return null;
  }
  return hit;
};

type PaintTargetInterface = {
  controller: MapEditorController;
  getCanvas: () => HTMLCanvasElement;
  getMapData: () => CarcerMapTemplate;
  getEditorState: () => EditorState;
};

const mapsByNameOf = (
  maps: CarcerMapTemplate[],
): Record<string, CarcerMapTemplate> => {
  const byName: Record<string, CarcerMapTemplate> = {};
  for (const m of maps) {
    byName[m.name] = m;
  }
  return byName;
};

/**
 * Name of the grid block the pointer is over, when grid editing is on and it is
 * a neighbour of the focused map (not the focused map itself, and a real tile).
 * Empty string means "paint the focused map as usual".
 */
const resolvePaintTargetMapName = (
  clientX: number,
  clientY: number,
  mapDataInterface: PaintTargetInterface,
): string => {
  const es = mapDataInterface.controller.getState();
  const { controller } = mapDataInterface;
  const handlers = controller.getGridNavigationHandlers();
  const focusedMap = mapDataInterface.getMapData();
  const canvas = mapDataInterface.getCanvas();
  if (!es.gridEditEnabled || !handlers || !focusedMap || !canvas) {
    return '';
  }
  const index = handlers.getDocumentIndex?.();
  const placement = index
    ? (index.placementsByMapName.get(focusedMap.name)?.[0] ?? null)
    : findMapGridPlacement(focusedMap.name, handlers.getMapGrids());
  const hit = screenCoordsToGridCell(
    controller,
    clientX,
    clientY,
    focusedMap,
    canvas,
    placement,
    index?.mapsByName ??
      new Map(handlers.getMaps().map((map) => [map.name, map])),
  );
  if (
    !hit ||
    !hit.map ||
    !hit.editable ||
    hit.tileIndex < 0 ||
    (hit.cellOffsetX === 0 && hit.cellOffsetY === 0)
  ) {
    return '';
  }
  return hit.mapName;
};

/**
 * Block + tile a right-click (pick / brush-copy) should act on: a grid
 * neighbour when the pointer is over one, otherwise the focused map. `mapName`
 * is '' for the focused map. `tileIndex` is -1 when there is nothing to pick.
 */
const resolveRightPickTarget = (
  clientX: number,
  clientY: number,
  mapDataInterface: PaintTargetInterface,
): { mapName: string; tileIndex: number } => {
  const es = mapDataInterface.controller.getState();
  const { controller } = mapDataInterface;
  const handlers = controller.getGridNavigationHandlers();
  const focusedMap = mapDataInterface.getMapData();
  const canvas = mapDataInterface.getCanvas();
  if (es.gridEditEnabled && handlers && focusedMap && canvas) {
    const index = handlers.getDocumentIndex?.();
    const placement = index
      ? (index.placementsByMapName.get(focusedMap.name)?.[0] ?? null)
      : findMapGridPlacement(focusedMap.name, handlers.getMapGrids());
    const hit = screenCoordsToGridCell(
      controller,
      clientX,
      clientY,
      focusedMap,
      canvas,
      placement,
      index?.mapsByName ??
        new Map(handlers.getMaps().map((map) => [map.name, map])),
    );
    if (
      hit &&
      hit.map &&
      hit.editable &&
      hit.tileIndex >= 0 &&
      !(hit.cellOffsetX === 0 && hit.cellOffsetY === 0)
    ) {
      return { mapName: hit.mapName, tileIndex: hit.tileIndex };
    }
  }
  return {
    mapName: '',
    tileIndex:
      getEditorStateMap(controller, es.selectedMapName)?.hoveredTileIndex ?? -1,
  };
};

/**
 * Finish a right-drag rect select that was tracked in focused-map tile space:
 * resolve every cell of the rect to its real grid block and either pick a
 * single tile (or switch to erase for a blank one) or build a clone brush whose
 * cells carry the tiles from whichever blocks they landed in.
 */
const completeGridRightDrag = (mapDataInterface: PaintTargetInterface) => {
  const { controller } = mapDataInterface;
  const es = mapDataInterface.controller.getState();
  const focusedMap = mapDataInterface.getMapData();
  const handlers = controller.getGridNavigationHandlers();
  if (!focusedMap || !handlers) {
    return;
  }
  const grids = handlers.getMapGrids();
  const index = handlers.getDocumentIndex?.();
  const mapsByName = index?.mapsByName ?? mapsByNameOf(handlers.getMaps());
  const placement = index
    ? (index.placementsByMapName.get(focusedMap.name)?.[0] ?? null)
    : undefined;

  const gx0 = Math.min(
    controller.input.rightDragStartGX,
    controller.input.rightDragEndGX,
  );
  const gx1 = Math.max(
    controller.input.rightDragStartGX,
    controller.input.rightDragEndGX,
  );
  const gy0 = Math.min(
    controller.input.rightDragStartGY,
    controller.input.rightDragEndGY,
  );
  const gy1 = Math.max(
    controller.input.rightDragStartGY,
    controller.input.rightDragEndGY,
  );

  if (gx0 === gx1 && gy0 === gy1) {
    const target = resolveGridBrushCell(
      focusedMap,
      gx0,
      gy0,
      grids,
      mapsByName,
      placement,
    );
    if (!target) {
      return;
    }
    const { tilesetIndex, tileId } = getTileGraphic(
      target.map,
      es.currentLevel,
      target.tileIndex,
    );
    if (tilesetIndex === 0 && tileId === 0) {
      setCurrentPaintAction(controller, PaintActionType.ERASE);
    } else {
      const ref = getTileList(controller, target.map)[target.tileIndex];
      updateEditorStateNoReRender(controller, {
        rectCloneBrushTiles: [],
        selectedTileIndexInTileset: ref.tileId,
        selectedTilesetName: ref.tilesetName,
      });
    }
    setSoleSelectedTile(controller, target.map.name, target.tileIndex);
    return;
  }

  const brush: FloorBrushData[] = [];
  for (let gy = gy0; gy <= gy1; gy++) {
    for (let gx = gx0; gx <= gx1; gx++) {
      const target = resolveGridBrushCell(
        focusedMap,
        gx,
        gy,
        grids,
        mapsByName,
        placement,
      );
      if (!target) {
        continue;
      }
      brush.push({
        xOffset: gx - gx0,
        yOffset: gy - gy0,
        originalTile: {
          ref: structuredClone(
            getTileList(controller, target.map)[target.tileIndex],
          ),
        },
      });
    }
  }
  if (brush.length === 0) {
    return;
  }
  updateEditorStateNoReRender(controller, { rectCloneBrushTiles: brush });

  const anchor = resolveGridBrushCell(
    focusedMap,
    gx0,
    gy0,
    grids,
    mapsByName,
    placement,
  );
  if (anchor) {
    setSoleSelectedTile(controller, anchor.map.name, anchor.tileIndex);
  }
};

/** The map a stroke is currently writing to (activePaintMapName, or focused). */
const getPaintTargetMap = (
  mapDataInterface: PaintTargetInterface,
): CarcerMapTemplate => {
  const focused = mapDataInterface.getMapData();
  const name = mapDataInterface.controller.getState().activePaintMapName;
  if (!name || name === focused?.name) {
    return focused;
  }
  const handlers = mapDataInterface.controller.getGridNavigationHandlers();
  return (
    handlers?.getDocumentIndex?.().mapsByName.get(name) ??
    handlers?.getMaps().find((map) => map.name === name) ??
    focused
  );
};

export const initPanzoom = (
  controller: MapEditorController,
  mapDataInterface: {
    controller: MapEditorController;
    getCanvas: () => HTMLCanvasElement;
    getMapData: () => CarcerMapTemplate;
    getTilesets: () => TilesetTemplate[];
    getEditorState: () => EditorState;
    onUndo?: () => void;
    onMapUpdate?: (map: CarcerMapTemplate) => void;
  },
) => {
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

    if (
      !isInputFocused &&
      (ev.ctrlKey || ev.metaKey) &&
      ev.key.toLowerCase() === 'z' &&
      !ev.shiftKey
    ) {
      ev.preventDefault();
      mapDataInterface.onUndo?.();
      return;
    }

    // keyboard shortcuts - only process if not typing in an input
    if (isEditorActive(ev) && !isInputFocused) {
      const toolForKey = TOOL_LIST.find(
        (tool) => tool.shortcut !== undefined && tool.shortcut === ev.key,
      );
      if (toolForKey) {
        if (!(toolForKey.shortcutBlockedByCtrl && ev.ctrlKey)) {
          setCurrentPaintAction(controller, toolForKey.id as PaintActionType);
          if (toolForKey.id === PaintActionType.FILL) {
            const state = controller.getState();
            const ind =
              getEditorStateMap(controller, state.selectedMapName)
                ?.hoveredTileIndex ?? -1;
            if (ind > -1) {
              onTileHoverIndChange(
                controller,
                mapDataInterface.getMapData(),
                state,
                state.currentPaintAction,
                -1,
                ind,
              );
            }
          }
        }
      } else if (ev.key === 'g' && !ev.ctrlKey) {
        const editorState = controller.getState();
        updateEditorState(controller, { showGrid: !editorState.showGrid });
        ev.preventDefault();
      } else if (ev.key === 'Escape') {
        const editorState = mapDataInterface.controller.getState();
        const hasSelection = Object.values(editorState.maps).some(
          (m) => (m?.selectedTileInd ?? -1) >= 0,
        );
        if (hasSelection || editorState.isSelectDragging) {
          ev.preventDefault();
          if (editorState.isSelectDragging) {
            updateEditorState(controller, {
              isSelectDragging: false,
              selectDragSourceTileIndex: -1,
              activePaintMapName: '',
            });
          }
          if (hasSelection) {
            clearAllSelectedTiles(controller);
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
            controller.getState().currentLevel,
            direction,
          );
          if (nextLevel !== null) {
            updateEditorState(controller, { currentLevel: nextLevel });
            ev.preventDefault();
          }
        }
      }
    }

    if (!isInputFocused && ev.key === 'Tab') {
      updateEditorStateNoReRender(controller, {
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
      updateEditorStateNoReRender(controller, {
        drawOverlayText: false,
      });
      ev.preventDefault();
    }
  };
  const handleMouseDown = (ev: PointerEvent) => {
    if (
      ev.button === MOUSE_BUTTON_MIDDLE &&
      isEventWithCanvasTarget(ev, mapDataInterface.getCanvas())
    ) {
      controller.input.lastClickX = ev.clientX;
      controller.input.lastClickY = ev.clientY;
      controller.input.lastTranslateX = controller.input.translateX;
      controller.input.lastTranslateY = controller.input.translateY;
      controller.input.isDragging = true;
    }
    if (
      ev.button === MOUSE_BUTTON_LEFT &&
      isEventWithCanvasTarget(ev, mapDataInterface.getCanvas())
    ) {
      const gridSlotHit = findGridSlotAtScreen(
        controller,
        ev.clientX,
        ev.clientY,
        mapDataInterface,
      );
      if (gridSlotHit) {
        controller.input.pendingGridSlotClick = gridSlotHit;
        controller.input.gridSlotClickStartX = ev.clientX;
        controller.input.gridSlotClickStartY = ev.clientY;
        return;
      }

      const currentPaintAction =
        mapDataInterface.controller.getState().currentPaintAction;
      if (currentPaintAction === PaintActionType.NONE) {
        return;
      }

      // SELECT action: start dragging to move tile data
      if (currentPaintAction === PaintActionType.SELECT) {
        const state = controller.getState();
        const selectionMapName =
          state.hoveredGridMapName || state.selectedMapName;
        const hoveredTileIndex =
          getEditorStateMap(controller, selectionMapName)?.hoveredTileIndex ??
          -1;
        if (hoveredTileIndex >= 0) {
          updateEditorState(controller, {
            selectDragSourceTileIndex: hoveredTileIndex,
            isSelectDragging: true,
            activePaintMapName:
              selectionMapName === state.selectedMapName
                ? ''
                : selectionMapName,
          });
          updateEditorStateMapNoReRender(controller, selectionMapName, {
            selectedTileInd: hoveredTileIndex,
          });
        }
        return;
      }

      // CLONE action: start dragging to clone tile data
      if (currentPaintAction === PaintActionType.CLONE) {
        const state = controller.getState();
        const selectionMapName =
          state.hoveredGridMapName || state.selectedMapName;
        const hoveredTileIndex =
          getEditorStateMap(controller, selectionMapName)?.hoveredTileIndex ??
          -1;
        if (hoveredTileIndex >= 0) {
          updateEditorState(controller, {
            selectDragSourceTileIndex: hoveredTileIndex,
            isSelectDragging: true,
            activePaintMapName:
              selectionMapName === state.selectedMapName
                ? ''
                : selectionMapName,
          });
          updateEditorStateMapNoReRender(controller, selectionMapName, {
            selectedTileInd: hoveredTileIndex,
          });
        }
        return;
      }

      controller.input.isPainting = true;
      // Route the stroke to whichever grid block the pointer landed on. Empty
      // means the focused map; anything else overrides the per-map paint state
      // for the duration of the stroke (cleared on mouseup).
      const paintTargetMapName = resolvePaintTargetMapName(
        ev.clientX,
        ev.clientY,
        mapDataInterface,
      );
      if (paintTargetMapName) {
        ensureEditorStateMap(controller, paintTargetMapName);
      }
      updateEditorStateNoReRender(controller, {
        activePaintMapName: paintTargetMapName,
      });
      const action = createPaintAction(currentPaintAction);
      action.data.paintTileRef = {
        tilesetName: mapDataInterface.controller.getState().selectedTilesetName,
        tileId:
          mapDataInterface.controller.getState().selectedTileIndexInTileset,
      };
      const floorBrush =
        mapDataInterface.controller.getState().rectCloneBrushTiles;
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
      controller.setCurrentAction(action);
    }
    if (
      ev.button === MOUSE_BUTTON_RIGHT &&
      isEventWithCanvasTarget(ev, mapDataInterface.getCanvas()) &&
      (controller.getState().currentPaintAction === PaintActionType.DRAW ||
        controller.getState().currentPaintAction === PaintActionType.FILL ||
        controller.getState().currentPaintAction ===
          PaintActionType.DELETE_FILL)
    ) {
      controller.input.rightDragGridActive = false;
      controller.input.rightDragMapName = '';

      // Grid maps: track the drag in focused-map tile space so it can span
      // blocks. resolveGridBrushCell confirms the start is on a real tile.
      const es = mapDataInterface.controller.getState();
      const focusedMap = mapDataInterface.getMapData();
      const canvas = mapDataInterface.getCanvas();
      const handlers = controller.getGridNavigationHandlers();
      if (es.gridEditEnabled && focusedMap && canvas && handlers) {
        const index = handlers.getDocumentIndex?.();
        const placement = index
          ? (index.placementsByMapName.get(focusedMap.name)?.[0] ?? null)
          : findMapGridPlacement(focusedMap.name, handlers.getMapGrids());
        const g = screenCoordsToGridTile(
          controller,
          ev.clientX,
          ev.clientY,
          focusedMap,
          canvas,
          handlers.getMapGrids(),
          placement,
        );
        if (
          g &&
          resolveGridBrushCell(
            focusedMap,
            g.gx,
            g.gy,
            handlers.getMapGrids(),
            index?.mapsByName ?? mapsByNameOf(handlers.getMaps()),
            placement,
          )
        ) {
          controller.input.isDraggingRight = true;
          controller.input.rightDragGridActive = true;
          controller.input.rightDragStartGX = g.gx;
          controller.input.rightDragStartGY = g.gy;
          controller.input.rightDragEndGX = g.gx;
          controller.input.rightDragEndGY = g.gy;
          return;
        }
      }

      // Non-grid map: single-block rect select.
      const pick = resolveRightPickTarget(
        ev.clientX,
        ev.clientY,
        mapDataInterface,
      );
      if (pick.tileIndex < 0) {
        return;
      }
      controller.input.isDraggingRight = true;
      controller.input.rightDragMapName = pick.mapName;
      updateEditorStateNoReRender(controller, {
        rectSelectTileIndStart: pick.tileIndex,
        rectSelectTileIndEnd: pick.tileIndex,
      });
    } else if (
      ev.button === MOUSE_BUTTON_RIGHT &&
      isEventWithCanvasTarget(ev, mapDataInterface.getCanvas()) &&
      [PaintActionType.CLONE, PaintActionType.SELECT].includes(
        controller.getState().currentPaintAction,
      )
    ) {
      const pick = resolveRightPickTarget(
        ev.clientX,
        ev.clientY,
        mapDataInterface,
      );
      if (pick.tileIndex < 0) {
        return;
      }
      setSoleSelectedTile(
        controller,
        pick.mapName || mapDataInterface.controller.getState().selectedMapName,
        pick.tileIndex,
      );
    }
  };
  const clearHoveredGridSlot = () => {
    if (mapDataInterface.controller.getState().hoveredGridAdjacentSlot) {
      updateEditorStateNoReRender(controller, {
        hoveredGridAdjacentSlot: null,
      });
    }
  };

  const refreshHoveredGridSlot = (ev: MouseEvent) => {
    if (!isEventWithCanvasTarget(ev, mapDataInterface.getCanvas())) {
      clearHoveredGridSlot();
      return;
    }
    const gridSlotHit = findGridSlotAtScreen(
      controller,
      ev.clientX,
      ev.clientY,
      mapDataInterface,
    );
    const nextHovered = gridSlotHit
      ? {
          offsetX: gridSlotHit.slot.offsetX,
          offsetY: gridSlotHit.slot.offsetY,
        }
      : null;
    const prevHovered =
      mapDataInterface.controller.getState().hoveredGridAdjacentSlot;
    if (
      nextHovered?.offsetX !== prevHovered?.offsetX ||
      nextHovered?.offsetY !== prevHovered?.offsetY
    ) {
      updateEditorStateNoReRender(controller, {
        hoveredGridAdjacentSlot: nextHovered,
      });
    }
  };

  const handleMouseMove = (ev: PointerEvent) => {
    controller.input.mouseX = ev.clientX;
    controller.input.mouseY = ev.clientY;

    if (controller.input.rightDragGridActive) {
      const focusedMap = mapDataInterface.getMapData();
      const canvas = mapDataInterface.getCanvas();
      const handlers = controller.getGridNavigationHandlers();
      if (focusedMap && canvas && handlers) {
        const index = handlers.getDocumentIndex?.();
        const g = screenCoordsToGridTile(
          controller,
          ev.clientX,
          ev.clientY,
          focusedMap,
          canvas,
          handlers.getMapGrids(),
          index?.placementsByMapName.get(focusedMap.name)?.[0] ?? null,
        );
        if (g) {
          controller.input.rightDragEndGX = g.gx;
          controller.input.rightDragEndGY = g.gy;
        }
      }
    }

    // While panning the slot hotspots move with the view, so a hit test against
    // the pre-move transform is stale anyway, and mousemove outruns the frame
    // rate. Skip it here and refresh once on mouseup.
    if (controller.input.isDragging) {
      clearHoveredGridSlot();
    } else {
      refreshHoveredGridSlot(ev);
    }

    if (controller.input.isDragging) {
      controller.input.translateX =
        controller.input.lastTranslateX +
        ev.clientX -
        controller.input.lastClickX;
      controller.input.translateY =
        controller.input.lastTranslateY +
        ev.clientY -
        controller.input.lastClickY;
    }
  };
  const handleMouseUp = (ev: PointerEvent) => {
    if (
      controller.input.pendingGridSlotClick &&
      ev.button === MOUSE_BUTTON_LEFT
    ) {
      const pending = controller.input.pendingGridSlotClick;
      controller.input.pendingGridSlotClick = null;
      const dragDistance = Math.hypot(
        ev.clientX - controller.input.gridSlotClickStartX,
        ev.clientY - controller.input.gridSlotClickStartY,
      );
      if (dragDistance <= GRID_SLOT_CLICK_DRAG_THRESHOLD) {
        const handlers = controller.getGridNavigationHandlers();
        if (handlers) {
          if (isGridSlotEditable(pending.slot)) {
            handlers.onNavigateToGridMap(pending.slot.mapName, {
              offsetX: pending.slot.offsetX,
              offsetY: pending.slot.offsetY,
            });
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

    if (controller.input.isDragging) {
      controller.input.translateX =
        controller.input.lastTranslateX +
        ev.clientX -
        controller.input.lastClickX;
      controller.input.translateY =
        controller.input.lastTranslateY +
        ev.clientY -
        controller.input.lastClickY;
      controller.input.isDragging = false;
      refreshHoveredGridSlot(ev);
    }
    if (controller.input.isPainting) {
      controller.input.isPainting = false;
      const currentAction = controller.getCurrentAction();
      const paintMapName =
        mapDataInterface.controller.getState().activePaintMapName ||
        mapDataInterface.controller.getState().selectedMapName;
      const mapData = getPaintTargetMap(mapDataInterface);
      if (currentAction && mapData) {
        // onActionComplete keys per-map state off activePaintMapName, so leave
        // it set until after this call.
        if (
          onActionComplete(
            controller,
            currentAction,
            mapData,
            mapDataInterface.controller.getState(),
          )
        ) {
          mapDataInterface.onMapUpdate?.({ ...mapData });
        }
      }
      setSoleSelectedTile(
        controller,
        paintMapName,
        getEditorStateMap(controller, paintMapName)?.hoveredTileIndex ?? -1,
      );
      updateEditorStateNoReRender(controller, { activePaintMapName: '' });
    }
    // Handle SELECT/CLONE drag completion
    const editorState = mapDataInterface.controller.getState();
    if (editorState.isSelectDragging && ev.button === 0) {
      const selectionMapName =
        editorState.activePaintMapName || editorState.selectedMapName;
      const sourceTileIndex = editorState.selectDragSourceTileIndex;
      const destTileIndex =
        getEditorStateMap(controller, selectionMapName)?.hoveredTileIndex ?? -1;
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

        const mapData = getPaintTargetMap(mapDataInterface);
        if (mapData) {
          const mapTiles = getTileList(controller, mapData);
          // Store previous state of both tiles for undo
          action.data.prevRefData.push(
            structuredClone(mapTiles[sourceTileIndex]),
          );
          action.data.prevRefData.push(
            structuredClone(mapTiles[destTileIndex]),
          );

          if (onActionComplete(controller, action, mapData, editorState)) {
            mapDataInterface.onMapUpdate?.({ ...mapData });
          }
        }
      }

      updateEditorState(controller, {
        isSelectDragging: false,
        selectDragSourceTileIndex: -1,
        activePaintMapName: '',
      });
      setSoleSelectedTile(
        controller,
        selectionMapName,
        destTileIndex >= 0
          ? destTileIndex
          : (getEditorStateMap(controller, selectionMapName)?.selectedTileInd ??
              -1),
      );
    }
    if (controller.input.isDraggingRight) {
      controller.input.isDraggingRight = false;
      const dragMapName = controller.input.rightDragMapName;
      controller.input.rightDragMapName = '';
      const wasGridDrag = controller.input.rightDragGridActive;
      controller.input.rightDragGridActive = false;
      const es = mapDataInterface.controller.getState();

      if (wasGridDrag) {
        completeGridRightDrag(mapDataInterface);
        return;
      }
      // A right pick / brush-copy can target any grid block, not just the
      // focused one.
      const dragMap =
        (dragMapName
          ? controller
              .getGridNavigationHandlers()
              ?.getMaps()
              .find((m) => m.name === dragMapName)
          : undefined) ?? mapDataInterface.getMapData();
      const pickKey = dragMapName || es.selectedMapName;

      const ind0 = es.rectSelectTileIndStart;
      const ind1 = es.rectSelectTileIndEnd;
      const dragSelectedInds = getIndsOfBoundingRect(
        ind0,
        ind1,
        dragMap.width ?? 0,
      );
      if (dragSelectedInds.length === 0) {
        return;
      }
      const mapTiles = getTileList(controller, dragMap);
      const nextRef = mapTiles[dragSelectedInds[0]];
      if (dragSelectedInds.length === 1 && nextRef) {
        // An unpainted cell has graphic (0, 0); picking it up just yields the
        // first tileset's tile 0. Right-clicking a blank cell means "erase",
        // so switch to the erase tool instead of selecting that tile.
        const { tilesetIndex, tileId } = getTileGraphic(
          dragMap,
          es.currentLevel,
          dragSelectedInds[0],
        );
        if (tilesetIndex === 0 && tileId === 0) {
          setCurrentPaintAction(controller, PaintActionType.ERASE);
        } else {
          updateEditorStateNoReRender(controller, {
            rectCloneBrushTiles: [],
            selectedTileIndexInTileset: nextRef.tileId,
            selectedTilesetName: nextRef.tilesetName,
          });
        }
        setSoleSelectedTile(controller, pickKey, dragSelectedInds[0]);
        return;
      }
      const mapWidth = dragMap.width ?? 0;
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
      updateEditorStateNoReRender(controller, {
        rectCloneBrushTiles: brush,
      });
      setSoleSelectedTile(controller, pickKey, dragSelectedInds[0]);
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
      controller,
      ev.clientX,
      ev.clientY,
      mapDataInterface.getCanvas(),
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
    let nextScale = controller.input.scale * zoomFactor;
    if (nextScale > 10) {
      nextScale = 10;
    } else if (nextScale < 0.5) {
      nextScale = 0.5;
    }

    if (nextScale === controller.input.scale) {
      return;
    }

    const offsetX =
      focalX -
      (nextScale / controller.input.scale) *
        (focalX - controller.input.translateX);
    const offsetY =
      focalY -
      (nextScale / controller.input.scale) *
        (focalY - controller.input.translateY);

    controller.input.translateX = offsetX;
    controller.input.translateY = offsetY;
    controller.input.scale = nextScale;
  };
  // The canvas rect is cached and only refreshed per rendered frame; when the
  // loop is idle a scroll/resize can move it, so drop the cache and force one
  // frame here too.
  const handleViewportChange = () => {
    invalidateCanvasRectCache(controller);
  };

  controller.lastAppliedCursor = null;
  invalidateCanvasRectCache(controller);

  controller.attach({
    canvas: mapDataInterface.getCanvas(),
    handlers: {
      keydown: handleKeyDown,
      keyup: handleKeyUp,
      pointerdown: handleMouseDown,
      pointermove: handleMouseMove,
      pointerup: handleMouseUp,
      pointercancel: handleMouseUp,
      contextmenu: handleContextMenu,
      wheel: handleWheel,
      resize: handleViewportChange,
    },
  });
};

export const unInitPanzoom = (controller: MapEditorController) =>
  controller.detach();

export const getTransform = (controller: MapEditorController) => {
  return {
    x: controller.input.translateX,
    y: controller.input.translateY,
    scale: controller.input.scale,
  };
};

export const updateMapCanvasCursor = (
  controller: MapEditorController,
  canvas: HTMLCanvasElement | null,
  paintAction: PaintActionType,
  hoveredTileIndex: number,
  isSelectDragging = false,
  hoveredGridAdjacentSlot: { offsetX: number; offsetY: number } | null = null,
) => {
  if (!canvas) {
    return;
  }

  let cursor = '';
  if (controller.input.isDragging || isSelectDragging) {
    cursor = 'grabbing';
  } else if (hoveredGridAdjacentSlot) {
    cursor = 'pointer';
  } else if (paintAction === PaintActionType.SELECT && hoveredTileIndex >= 0) {
    cursor = 'pointer';
  } else if (paintAction === PaintActionType.CLONE) {
    cursor = 'grab';
  }

  // This runs every frame; writing the style unconditionally dirties layout and
  // makes the next getBoundingClientRect() a forced reflow.
  if (cursor !== controller.lastAppliedCursor) {
    canvas.style.cursor = cursor;
    controller.lastAppliedCursor = cursor;
  }
};

export const resetPanzoom = (controller: MapEditorController) => {
  controller.input.translateX = 0;
  controller.input.translateY = 0;
  controller.input.scale = 1;
};

export const saveViewportForMap = (
  controller: MapEditorController,
  mapName: string,
) => {
  if (!mapName) {
    return;
  }
  updateEditorStateMapNoReRender(controller, mapName, {
    viewport: {
      translateX: controller.input.translateX,
      translateY: controller.input.translateY,
      scale: controller.input.scale,
    },
  });
};

export const restoreViewportForMap = (
  controller: MapEditorController,
  mapName: string,
) => {
  if (!mapName) {
    resetPanzoom(controller);
    return;
  }
  const viewport = getEditorStateMap(controller, mapName)?.viewport;
  if (viewport) {
    controller.input.translateX = viewport.translateX;
    controller.input.translateY = viewport.translateY;
    controller.input.scale = viewport.scale;
    return;
  }
  resetPanzoom(controller);
};

export const switchMapViewport = (
  controller: MapEditorController,
  fromMapName: string,
  toMapName: string,
) => {
  if (fromMapName) {
    saveViewportForMap(controller, fromMapName);
  }
  restoreViewportForMap(controller, toMapName);
};

/**
 * Switch maps while keeping the stitched world under the camera fixed.
 * Adjacent maps are drawn at offset * (mapPixelSize * scale); adjusting pan
 * by that amount makes the neighbor become current without a visual jump.
 */
export const switchMapViewportPreservingStitch = (
  controller: MapEditorController,
  fromMapName: string,
  toMapName: string,
  stitchOffset: GridNavigateStitchOffset,
  mapPixelWidth: number,
  mapPixelHeight: number,
) => {
  if (fromMapName) {
    saveViewportForMap(controller, fromMapName);
  }
  const scale = controller.input.scale;
  controller.input.translateX += stitchOffset.offsetX * mapPixelWidth * scale;
  controller.input.translateY += stitchOffset.offsetY * mapPixelHeight * scale;
  if (toMapName) {
    saveViewportForMap(controller, toMapName);
  }
};

/** Pan the map view so the tile center is at the canvas center. */
export const centerViewOnTile = (
  controller: MapEditorController,
  canvas: HTMLCanvasElement,
  mapData: CarcerMapTemplate,
  tileIndex: number,
) => {
  const tileX = tileIndex % mapData.width;
  const tileY = Math.floor(tileIndex / mapData.width);
  const tileCenterMapX = (tileX + 0.5) * mapData.spriteWidth;
  const tileCenterMapY = (tileY + 0.5) * mapData.spriteHeight;
  const mapWidth = mapData.width * mapData.spriteWidth;
  const mapHeight = mapData.height * mapData.spriteHeight;
  const scale = controller.input.scale;
  const canvasW = canvas.width;
  const canvasH = canvas.height;

  controller.input.translateX =
    (canvasW / 2) * (1 - scale) + scale * (mapWidth / 2 - tileCenterMapX);
  controller.input.translateY =
    (canvasH / 2) * (1 - scale) + scale * (mapHeight / 2 - tileCenterMapY);
};

export const getIndsOfBoundingRect = (
  ind0: number,
  ind1: number,
  width: number,
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

/**
 * getBoundingClientRect() forces a synchronous layout. Mouse events fire far
 * more often than frames (up to 1000Hz on a high-polling mouse), and the render
 * loop writes canvas.style.cursor each frame, so measuring per event thrashes
 * layout while panning. Measure at most once per frame instead.
 */
export const invalidateCanvasRectCache = (controller: MapEditorController) => {
  controller.canvasRect = null;
};

const getCanvasOffset = (
  controller: MapEditorController,
  panzoomCanvas: HTMLCanvasElement,
) => {
  if (controller.canvasRect && controller.canvasRect.canvas === panzoomCanvas) {
    return controller.canvasRect;
  }
  const { left, top } = panzoomCanvas?.getBoundingClientRect() ?? {
    left: 0,
    top: 0,
  };
  controller.canvasRect = { canvas: panzoomCanvas, left, top };
  return controller.canvasRect;
};

export const screenCoordsToCanvasCoords = (
  controller: MapEditorController,
  x: number,
  y: number,
  panzoomCanvas: HTMLCanvasElement,
) => {
  const { left, top } = getCanvasOffset(controller, panzoomCanvas);

  const canvasX = x - left;
  const canvasY = y - top;

  return [canvasX, canvasY];
};

export const screenCoordsToMapCoords = (
  controller: MapEditorController,
  x: number,
  y: number,
  mapData: CarcerMapTemplate,
  panzoomCanvas: HTMLCanvasElement,
) => {
  if (!panzoomCanvas) {
    return [0, 0];
  }
  const [canvasX, canvasY] = screenCoordsToCanvasCoords(
    controller,
    x,
    y,
    panzoomCanvas,
  );
  const canvas = panzoomCanvas;

  const canvasW = canvas.width;
  const canvasH = canvas.height;

  const mapX =
    canvasX -
    controller.input.translateX -
    controller.input.scale *
      (canvasW / 2 - (mapData.width * mapData.spriteWidth) / 2);
  const mapY =
    canvasY -
    controller.input.translateY -
    controller.input.scale *
      (canvasH / 2 - (mapData.height * mapData.spriteHeight) / 2);

  return [mapX / controller.input.scale, mapY / controller.input.scale];
};

export const screenCoordsToTileIndex = (
  controller: MapEditorController,
  x: number,
  y: number,
  mapData: CarcerMapTemplate,
  panzoomCanvas: HTMLCanvasElement,
): [number, number, number, number, number] => {
  const [mapX, mapY] = screenCoordsToMapCoords(
    controller,
    x,
    y,
    mapData,
    panzoomCanvas,
  );
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

/**
 * Tile coords for a screen point relative to `focusedMap`'s top-left, allowed to
 * run negative or past the map's edges — a point over a neighbouring grid cell
 * yields the continuation of the focused map's tile grid. Null when the focused
 * map is not in a grid. Pair with resolveGridBrushCell to land on a real block.
 */
export const screenCoordsToGridTile = (
  controller: MapEditorController,
  x: number,
  y: number,
  focusedMap: CarcerMapTemplate,
  panzoomCanvas: HTMLCanvasElement,
  mapGrids: MapGridTemplate[],
  placement?: MapGridPlacement | null,
): { gx: number; gy: number } | null => {
  if (!panzoomCanvas || !focusedMap) {
    return null;
  }
  const resolvedPlacement =
    placement === undefined
      ? findMapGridPlacement(focusedMap.name, mapGrids)
      : placement;
  if (!resolvedPlacement) {
    return null;
  }
  const [fx, fy] = screenCoordsToMapCoords(
    controller,
    x,
    y,
    focusedMap,
    panzoomCanvas,
  );
  return {
    gx: Math.floor(fx / focusedMap.spriteWidth),
    gy: Math.floor(fy / focusedMap.spriteHeight),
  };
};

/** The in-progress right-drag rect select, in focused-map tile space, or null. */
export const getRightDragGridRect = (
  controller: MapEditorController,
): {
  gx0: number;
  gy0: number;
  gx1: number;
  gy1: number;
} | null => {
  if (
    !controller.input.isDraggingRight ||
    !controller.input.rightDragGridActive
  ) {
    return null;
  }
  return {
    gx0: Math.min(
      controller.input.rightDragStartGX,
      controller.input.rightDragEndGX,
    ),
    gy0: Math.min(
      controller.input.rightDragStartGY,
      controller.input.rightDragEndGY,
    ),
    gx1: Math.max(
      controller.input.rightDragStartGX,
      controller.input.rightDragEndGX,
    ),
    gy1: Math.max(
      controller.input.rightDragStartGY,
      controller.input.rightDragEndGY,
    ),
  };
};

export interface GridCellHit {
  mapName: string;
  map: CarcerMapTemplate | null;
  /** Grid cell offset from the focused map. */
  cellOffsetX: number;
  cellOffsetY: number;
  /** Local tile coords within the cell's map (-1 when there is no map there). */
  tileX: number;
  tileY: number;
  /** tileY * map.width + tileX, or -1 when no map / outside that map's bounds. */
  tileIndex: number;
  /** Whether this partition shares the grid/focused-map editing geometry. */
  editable: boolean;
}

/**
 * Resolve which grid cell (and tile within it) the pointer is over, expressed
 * relative to `focusedMap`'s placement. For cell offset (0, 0) this reproduces
 * `screenCoordsToTileIndex` exactly; neighbours reuse the same transform shifted
 * by whole slots. Returns null when the focused map is not in a grid, or the
 * pointer is off the grid.
 */
export const screenCoordsToGridCell = (
  controller: MapEditorController,
  x: number,
  y: number,
  focusedMap: CarcerMapTemplate,
  panzoomCanvas: HTMLCanvasElement,
  placement: MapGridPlacement | null,
  mapsByName: ReadonlyMap<string, CarcerMapTemplate>,
): GridCellHit | null => {
  if (!panzoomCanvas || !focusedMap) {
    return null;
  }
  if (!placement) {
    return null;
  }

  const [fx, fy] = screenCoordsToMapCoords(
    controller,
    x,
    y,
    focusedMap,
    panzoomCanvas,
  );
  const slotW = placement.grid.mapWidth * focusedMap.spriteWidth;
  const slotH = placement.grid.mapHeight * focusedMap.spriteHeight;
  if (slotW <= 0 || slotH <= 0) {
    return null;
  }

  const cellOffsetX = Math.floor(fx / slotW);
  const cellOffsetY = Math.floor(fy / slotH);
  const cellX = placement.cellX + cellOffsetX;
  const cellY = placement.cellY + cellOffsetY;
  if (
    cellY < 0 ||
    cellY >= placement.grid.gridHeight ||
    cellX < 0 ||
    cellX >= placement.grid.gridWidth
  ) {
    return null;
  }

  const mapName = placement.grid.cells[cellY]?.[cellX]?.trim() ?? '';
  const map = mapName ? (mapsByName.get(mapName) ?? null) : null;
  const editable = Boolean(
    map &&
    (cellOffsetX === 0 && cellOffsetY === 0
      ? map.name === focusedMap.name
      : map.width === placement.grid.mapWidth &&
        map.height === placement.grid.mapHeight &&
        map.spriteWidth === focusedMap.spriteWidth &&
        map.spriteHeight === focusedMap.spriteHeight),
  );

  let tileX = -1;
  let tileY = -1;
  let tileIndex = -1;
  if (map) {
    const localX = fx - cellOffsetX * slotW;
    const localY = fy - cellOffsetY * slotH;
    const tx = Math.floor(localX / map.spriteWidth);
    const ty = Math.floor(localY / map.spriteHeight);
    if (tx >= 0 && tx < map.width && ty >= 0 && ty < map.height) {
      tileX = tx;
      tileY = ty;
      tileIndex = ty * map.width + tx;
    }
  }

  return {
    mapName,
    map,
    cellOffsetX,
    cellOffsetY,
    tileX,
    tileY,
    tileIndex,
    editable,
  };
};

export const getScreenMouseCoords = (controller: MapEditorController) => {
  return [controller.input.mouseX, controller.input.mouseY];
};

export const getIsDraggingRight = (controller: MapEditorController) => {
  return controller.input.isDraggingRight;
};

/** Any pointer interaction that needs the canvas repainted every frame. */
export const isCanvasInteracting = (controller: MapEditorController) => {
  return (
    controller.input.isDragging ||
    controller.input.isDraggingRight ||
    controller.input.isPainting ||
    controller.input.pendingGridSlotClick !== null ||
    controller.getState().isSelectDragging
  );
};

export const getTileList = (
  controller: MapEditorController,
  mapData: CarcerMapTemplate,
  level?: number,
) => {
  return getMaterializedLayer(
    mapData,
    level ?? controller.getState().currentLevel,
    controller,
  );
};

export const commitCurrentLayer = (
  controller: MapEditorController,
  mapData: CarcerMapTemplate,
  level?: number,
) => {
  commitMaterializedLayer(
    mapData,
    level ?? controller.getState().currentLevel,
    controller,
  );
};
