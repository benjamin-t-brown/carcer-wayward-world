import { calculateHoveredTile } from './renderState';
import { onActionUpdate, onTileHoverIndChange } from './paintTools';
import { disableCanvasSmoothing } from '../utils/spriteUtils';
import { drawLine, drawRect, snapPixelArtPanOffset } from '../utils/draw';
import {
  CarcerMapTemplate,
  TilesetTemplate,
  CharacterTemplate,
  ItemTemplate,
  GameEvent,
  MapGridTemplate,
} from '../types/assets';
import {
  EditorState,
  ensureEditorStateMap,
  getEditorStateMap,
  updateEditorStateMapNoReRender,
  updateEditorStateNoReRender,
} from './editorState';
import {
  getScreenMouseCoords,
  getTileList,
  getTransform,
  GridCellHit,
  invalidateCanvasRectCache,
  screenCoordsToGridCell,
  updateMapCanvasCursor,
} from './editorEvents';
import { PaintActionType } from './paintTools';
import { Sprite } from '../utils/assetLoader';
import {
  drawOverlayTextEntries,
  OverlayTextEntry,
  renderGridAdjacentNavigation,
  renderMapTilesAtOffset,
  renderTileAndExtras,
  renderToolUi,
} from './renderUi';
import { MapEditorController } from './MapEditorController';
import { buildMapRenderPlan } from './mapRenderPlan';
import type { MapRenderLookups } from './mapDocumentIndex';

const getColors = () => {
  return {
    BACKGROUND2: '#421',
    TEXT: 'white',
  };
};

/**
 * Paint one map block's tiles (plus its grid lines and hover box) into the
 * already-translated context. Shared by the focused map and each editable grid
 * neighbour so they render identically; `hoveredTileIndex` is that block's own.
 */
const renderMapBlockTiles = (args: {
  controller: MapEditorController;
  map: CarcerMapTemplate;
  ctx: CanvasRenderingContext2D;
  scale: number;
  visibleRange: import('./viewport').VisibleTileRange;
  renderLookups: MapRenderLookups;
  spriteMap: Record<string, Sprite>;
  tilesets: TilesetTemplate[];
  characters: CharacterTemplate[];
  items: ItemTemplate[];
  layer: number;
  showGrid: boolean;
  hoveredTileIndex: number;
  overlayTextEntries?: OverlayTextEntry[];
}) => {
  const {
    map,
    ctx,
    scale,
    visibleRange,
    renderLookups,
    spriteMap,
    tilesets,
    characters,
    items,
    layer,
    showGrid,
    hoveredTileIndex,
    overlayTextEntries,
  } = args;

  const spriteWidth = map.spriteWidth;
  const spriteHeight = map.spriteHeight;
  const mapTiles = getTileList(args.controller, map, layer);

  drawRect(
    0,
    0,
    map.width * spriteWidth * scale,
    map.height * spriteHeight * scale,
    'black',
    false,
    ctx,
  );

  for (let y = visibleRange.minY; y <= visibleRange.maxY; y++) {
    for (let x = visibleRange.minX; x <= visibleRange.maxX; x++) {
      const tileIndex = y * map.width + x;
      renderTileAndExtras({
        refTile: mapTiles[tileIndex],
        x,
        y,
        ctx,
        newScale: scale,
        spriteMap,
        mapSpriteWidth: spriteWidth,
        mapSpriteHeight: spriteHeight,
        tilesets,
        characters,
        items,
        overlayTextEntries,
        renderLookups,
      });

      const x1 = x * spriteWidth * scale;
      const y1 = y * spriteHeight * scale;
      const x2 = x1 + spriteWidth * scale;
      const y2 = y1 + spriteHeight * scale;
      if (tileIndex === hoveredTileIndex) {
        const color = 'rgba(100, 100, 255, 0.5)';
        drawLine(x1, y1, x2, y1, color, 2, ctx);
        drawLine(x1, y1, x1, y2, color, 2, ctx);
        drawLine(x2, y2, x2, y1, color, 2, ctx);
        drawLine(x2, y2, x1, y2, color, 2, ctx);
      } else if (showGrid) {
        const color = 'rgba(255, 255, 255, 0.25)';
        drawLine(x1, y1, x2, y1, color, 1, ctx);
        drawLine(x1, y1, x1, y2, color, 1, ctx);
      }
    }
  }
};

export const loop = (
  controller: MapEditorController,
  mapDataInterface: {
    getCanvas: () => HTMLCanvasElement;
    getMapData: () => CarcerMapTemplate;
    getEditorState: () => EditorState;
    getSprites: () => Sprite[];
    getSpriteMap: () => Record<string, Sprite>;
    getTilesets: () => TilesetTemplate[];
    getAssets: () => {
      characters: CharacterTemplate[];
      items: ItemTemplate[];
      tilesets: TilesetTemplate[];
      gameEvents: GameEvent[];
      maps: CarcerMapTemplate[];
      mapGrids: MapGridTemplate[];
    };
  },
  _ms: number,
) => {
  // The canvas can only move between frames, so one measurement per frame is
  // enough; every mouse event in between reuses it instead of forcing a layout.
  invalidateCanvasRectCache(controller);

  const ctx = mapDataInterface.getCanvas().getContext('2d');
  if (!ctx) {
    return;
  }

  if (!mapDataInterface.getEditorState().selectedMapName) {
    return;
  }

  const currentMap = mapDataInterface.getMapData();
  const es = mapDataInterface.getEditorState();
  const canvasEl = mapDataInterface.getCanvas();
  const assets = mapDataInterface.getAssets();
  const documentIndex = controller.getDocumentIndex(assets);

  // Which grid block is the pointer over? Only when grid editing is on and the
  // focused map is in a grid; otherwise fall back to the focused map only.
  let gridHit: GridCellHit | null = null;
  if (es.gridEditEnabled && currentMap) {
    const [screenX, screenY] = getScreenMouseCoords(controller);
    gridHit = screenCoordsToGridCell(
      controller,
      screenX,
      screenY,
      currentMap,
      canvasEl,
      documentIndex.placementsByMapName.get(currentMap.name)?.[0] ?? null,
      documentIndex.mapsByName,
    );
  }
  const pointerOnNeighbour =
    !!gridHit &&
    !!gridHit.map &&
    gridHit.editable &&
    gridHit.tileIndex >= 0 &&
    !(gridHit.cellOffsetX === 0 && gridHit.cellOffsetY === 0);

  let hoverMapName = es.selectedMapName;
  let hoverMap: CarcerMapTemplate | undefined = currentMap;
  let hoverInd = -1;
  let hoverX = -1;
  let hoverY = -1;
  if (pointerOnNeighbour && gridHit && gridHit.map) {
    hoverMapName = gridHit.mapName;
    hoverMap = gridHit.map;
    hoverInd = gridHit.tileIndex;
    hoverX = gridHit.tileX;
    hoverY = gridHit.tileY;
    ensureEditorStateMap(controller, hoverMapName);
  } else {
    const data = calculateHoveredTile(controller, currentMap, canvasEl);
    hoverInd = data.ind;
    // data.x/y are raw floored coords; keep them only when actually on the map.
    hoverX = data.ind >= 0 ? data.x : -1;
    hoverY = data.ind >= 0 ? data.y : -1;
  }

  // Clear the block we were hovering last frame if it changed.
  const prevHoverMapName = es.hoveredGridMapName || es.selectedMapName;
  if (prevHoverMapName !== hoverMapName) {
    updateEditorStateMapNoReRender(controller, prevHoverMapName, {
      hoveredTileIndex: -1,
      hoveredTileData: { x: -1, y: -1, ind: -1 },
    });
  }
  updateEditorStateNoReRender(controller, {
    hoveredGridMapName: hoverMapName === es.selectedMapName ? '' : hoverMapName,
  });

  const prevHoverInd =
    getEditorStateMap(controller, hoverMapName)?.hoveredTileIndex ?? -1;
  if (hoverInd !== prevHoverInd && hoverMap) {
    onTileHoverIndChange(
      controller,
      hoverMap,
      es,
      es.currentPaintAction,
      prevHoverInd,
      hoverInd,
    );
  }
  updateEditorStateMapNoReRender(controller, hoverMapName, {
    hoveredTileIndex: hoverInd,
    hoveredTileData: { x: hoverX, y: hoverY, ind: hoverInd },
  });

  updateMapCanvasCursor(
    controller,
    canvasEl,
    es.currentPaintAction as PaintActionType,
    hoverInd,
    es.isSelectDragging,
    es.hoveredGridAdjacentSlot,
  );

  const currentAction = controller.getCurrentAction();
  if (currentAction) {
    // Route stroke updates to whichever block the stroke started on.
    const paintMap = es.activePaintMapName
      ? (documentIndex.mapsByName.get(es.activePaintMapName) ?? currentMap)
      : currentMap;
    if (paintMap) {
      onActionUpdate(
        controller,
        currentAction,
        paintMap,
        es,
        mapDataInterface.getTilesets(),
      );
    }
  }

  ctx.clearRect(
    0,
    0,
    mapDataInterface.getCanvas().width,
    mapDataInterface.getCanvas().height,
  );
  drawRect(
    0,
    0,
    mapDataInterface.getCanvas().width,
    mapDataInterface.getCanvas().height,
    getColors().BACKGROUND2,
    false,
    ctx,
  );

  if (currentMap) {
    const { x, y, scale } = getTransform(controller);
    const canvas = mapDataInterface.getCanvas();
    const editorState = mapDataInterface.getEditorState();
    const spriteMap = mapDataInterface.getSpriteMap();
    const tilesets = mapDataInterface.getTilesets();
    const showGrid = editorState.showGrid;
    const spriteWidth = currentMap.spriteWidth;
    const spriteHeight = currentMap.spriteHeight;
    const { x: panX, y: panY } = snapPixelArtPanOffset(
      x,
      y,
      scale,
      canvas.width,
      canvas.height,
      currentMap.width,
      currentMap.height,
      spriteWidth,
      spriteHeight,
    );

    disableCanvasSmoothing(ctx);

    // Editable grid neighbours to run renderToolUi for, once the block loop has
    // laid their tiles down. Populated inside the (single-iteration) layer loop.
    const editableNeighbourToolPasses: {
      map: CarcerMapTemplate;
      offsetPixelX: number;
      offsetPixelY: number;
    }[] = [];
    let focusedMapIsVisible = false;

    // layers
    for (let i = 0; i < 1; i++) {
      const newScale = scale * (1 + i * 0.04);

      const focalX = canvas.width / 2;
      const focalY = canvas.height / 2;

      const offsetX = focalX - (newScale / scale) * (focalX - panX);
      const offsetY = focalY - (newScale / scale) * (focalY - panY);

      // Canvas-space position of this map's top-left tile, used for culling.
      const originX =
        offsetX +
        (canvas.width * newScale) / 2 -
        (currentMap.width * spriteWidth * newScale) / 2;
      const originY =
        offsetY +
        (canvas.height * newScale) / 2 -
        (currentMap.height * spriteHeight * newScale) / 2;

      ctx.save();
      ctx.translate(offsetX, offsetY);
      ctx.translate(
        (canvas.width * newScale) / 2,
        (canvas.height * newScale) / 2,
      );
      ctx.translate(
        -(currentMap.width * spriteWidth * newScale) / 2,
        -(currentMap.height * spriteHeight * newScale) / 2,
      );

      const overlayTextEntries: OverlayTextEntry[] | undefined =
        editorState.drawOverlayText ? [] : undefined;
      const placement =
        documentIndex.placementsByMapName.get(currentMap.name)?.[0] ?? null;
      const renderPlan = buildMapRenderPlan({
        focusedMap: currentMap,
        canvasWidth: canvas.width,
        canvasHeight: canvas.height,
        focusOriginX: originX,
        focusOriginY: originY,
        scale: newScale,
        mapsByName: documentIndex.mapsByName,
        placement,
      });

      for (const partition of renderPlan.partitions) {
        if (!partition.visibleTiles) continue;

        const offsetPixelX = partition.originX - originX;
        const offsetPixelY = partition.originY - originY;
        const editable =
          partition.focused ||
          (editorState.gridEditEnabled && partition.editable);

        if (editable) {
          ctx.save();
          ctx.translate(offsetPixelX, offsetPixelY);
          renderMapBlockTiles({
            controller,
            map: partition.map,
            ctx,
            scale: newScale,
            visibleRange: partition.visibleTiles,
            renderLookups: documentIndex,
            spriteMap,
            tilesets,
            characters: assets.characters,
            items: assets.items,
            layer: editorState.currentLevel,
            showGrid,
            hoveredTileIndex:
              getEditorStateMap(controller, partition.mapName)
                ?.hoveredTileIndex ?? -1,
            overlayTextEntries: partition.focused
              ? overlayTextEntries
              : undefined,
          });
          ctx.restore();

          if (partition.focused) {
            focusedMapIsVisible = true;
          } else {
            editableNeighbourToolPasses.push({
              map: partition.map,
              offsetPixelX,
              offsetPixelY,
            });
          }
        } else {
          renderMapTilesAtOffset({
            controller,
            map: partition.map,
            ctx,
            scale: newScale,
            offsetPixelX,
            offsetPixelY,
            opacity: 0.5,
            spriteMap,
            tilesets,
            characters: assets.characters,
            items: assets.items,
            layer: editorState.currentLevel,
            visibleRange: partition.visibleTiles,
            renderLookups: documentIndex,
          });
        }
      }

      if (renderPlan.grid) {
        const seamlessMapNames = new Set(
          renderPlan.partitions
            .filter((partition) => partition.editable)
            .map((partition) => partition.mapName),
        );
        renderGridAdjacentNavigation({
          ctx,
          slots: renderPlan.cells
            .filter(
              (cell) =>
                cell.navigable &&
                (!editorState.gridEditEnabled ||
                  !cell.map ||
                  !seamlessMapNames.has(cell.mapName)),
            )
            .map((cell) => ({
              offsetX: cell.offsetX,
              offsetY: cell.offsetY,
              cellX: cell.cellX,
              cellY: cell.cellY,
              mapName: cell.mapName,
              map: cell.map,
            })),
          slotWidth: renderPlan.grid.mapWidth * spriteWidth * newScale,
          slotHeight: renderPlan.grid.mapHeight * spriteHeight * newScale,
          hoveredOffset: editorState.hoveredGridAdjacentSlot,
        });
      }

      if (overlayTextEntries?.length) {
        drawOverlayTextEntries(ctx, overlayTextEntries);
      }

      ctx.restore();
    }

    if (focusedMapIsVisible) {
      renderToolUi(
        controller,
        editorState,
        currentMap,
        ctx,
        spriteMap,
        tilesets,
        assets.characters,
        assets.items,
        documentIndex,
        undefined,
        undefined,
        undefined,
      );
    }

    // Tool preview (hover box fill, brush ghost, fill outline) for the editable
    // neighbour the pointer / active stroke is on. Others no-op: only the block
    // with a hoveredTileIndex >= 0 draws anything.
    for (const pass of editableNeighbourToolPasses) {
      renderToolUi(
        controller,
        editorState,
        pass.map,
        ctx,
        spriteMap,
        tilesets,
        assets.characters,
        assets.items,
        documentIndex,
        pass.map.name,
        pass.offsetPixelX,
        pass.offsetPixelY,
      );
    }
  }
};
