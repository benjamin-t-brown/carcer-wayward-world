import { calculateHoveredTile } from './renderState';
import {
  getCurrentAction,
  onActionUpdate,
  onTileHoverIndChange,
} from './paintTools';
import { disableCanvasSmoothing } from '../utils/spriteUtils';
import { drawRect, scaledTileRect, snapPixelArtPanOffset } from '../utils/draw';
import {
  CarcerMapTemplate,
  TilesetTemplate,
  CharacterTemplate,
  ItemTemplate,
  GameEvent,
  MapGridTemplate,
} from '../types/assets';
import {
  findMapGridPlacement,
  getGridAdjacentSlots,
  getGridMapsWithinRadius,
} from '../utils/mapGridIndex';
import { getMapGridSlotDimensions } from './gridMapNavigation';
import {
  EditorState,
  ensureEditorStateMap,
  getEditorStateMap,
  updateEditorStateMapNoReRender,
  updateEditorStateNoReRender,
} from './editorState';
import {
  ensurePixelArtScale,
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
  getVisibleTileRange,
  OverlayTextEntry,
  renderGridAdjacentNavigation,
  renderMapTilesAtOffset,
  renderTileAndExtras,
  renderToolUi,
} from './renderUi';

// let currentMap: MapResponse | null = null;
// let isLooping = false;
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
  map: CarcerMapTemplate;
  ctx: CanvasRenderingContext2D;
  scale: number;
  /** Canvas-space top-left of THIS block, for culling. */
  originX: number;
  originY: number;
  canvasWidth: number;
  canvasHeight: number;
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
    originX,
    originY,
    canvasWidth,
    canvasHeight,
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
  const mapTiles = getTileList(map, layer);

  const mapPixelW = Math.round(map.width * spriteWidth * scale);
  const mapPixelH = Math.round(map.height * spriteHeight * scale);
  drawRect(0, 0, mapPixelW, mapPixelH, 'black', false, ctx);

  const visibleRange = getVisibleTileRange({
    originX,
    originY,
    canvasWidth,
    canvasHeight,
    mapWidth: map.width,
    mapHeight: map.height,
    tileWidth: spriteWidth,
    tileHeight: spriteHeight,
    scale,
  });

  for (let y = visibleRange?.minY ?? 0; y <= (visibleRange?.maxY ?? -1); y++) {
    for (let x = visibleRange?.minX ?? 0; x <= (visibleRange?.maxX ?? -1); x++) {
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
      });

      const tileRect = scaledTileRect(x, y, spriteWidth, spriteHeight, scale);
      if (tileIndex === hoveredTileIndex) {
        ctx.fillStyle = 'rgba(100, 100, 255, 0.5)';
        ctx.fillRect(tileRect.x, tileRect.y, tileRect.w, 2);
        ctx.fillRect(tileRect.x, tileRect.y, 2, tileRect.h);
        ctx.fillRect(tileRect.x, tileRect.y + tileRect.h - 2, tileRect.w, 2);
        ctx.fillRect(tileRect.x + tileRect.w - 2, tileRect.y, 2, tileRect.h);
      } else if (showGrid) {
        ctx.fillStyle = 'rgba(255, 255, 255, 0.25)';
        ctx.fillRect(tileRect.x, tileRect.y, tileRect.w, 1);
        ctx.fillRect(tileRect.x, tileRect.y, 1, tileRect.h);
      }
    }
  }
};

export const loop = (
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
  _ms: number
) => {
  // const appState: AppState = (window as any).appState;
  // if (!appState) {
  //   return;
  // }

  // The canvas can only move between frames, so one measurement per frame is
  // enough; every mouse event in between reuses it instead of forcing a layout.
  invalidateCanvasRectCache();

  const ctx = mapDataInterface.getCanvas().getContext('2d');
  if (!ctx) {
    return;
  }

  if (!mapDataInterface.getEditorState().selectedMapName) {
    return;
  }

  const currentMap = mapDataInterface.getMapData();
  if (currentMap) {
    ensurePixelArtScale(currentMap.spriteWidth);
  }
  const es = mapDataInterface.getEditorState();
  const canvasEl = mapDataInterface.getCanvas();
  const hoverAssets = mapDataInterface.getAssets();

  // Which grid block is the pointer over? Only when grid editing is on and the
  // focused map is in a grid; otherwise fall back to the focused map only.
  let gridHit: GridCellHit | null = null;
  if (es.gridEditEnabled && currentMap) {
    const [screenX, screenY] = getScreenMouseCoords();
    gridHit = screenCoordsToGridCell(
      screenX,
      screenY,
      currentMap,
      canvasEl,
      hoverAssets.mapGrids,
      hoverAssets.maps,
      Math.min(es.gridEditRadius ?? 1, es.gridRenderRadius ?? 2)
    );
  }
  const pointerOnNeighbour =
    !!gridHit &&
    !!gridHit.map &&
    gridHit.tileIndex >= 0 &&
    !(gridHit.cellOffsetX === 0 && gridHit.cellOffsetY === 0);

  let hoverMapName = es.selectedMapName;
  let hoverMap: CarcerMapTemplate | undefined = currentMap;
  let hoverInd: number;
  let hoverX: number;
  let hoverY: number;
  if (pointerOnNeighbour && gridHit && gridHit.map) {
    hoverMapName = gridHit.mapName;
    hoverMap = gridHit.map;
    hoverInd = gridHit.tileIndex;
    hoverX = gridHit.tileX;
    hoverY = gridHit.tileY;
    ensureEditorStateMap(hoverMapName);
  } else {
    const data = calculateHoveredTile(currentMap, canvasEl);
    hoverInd = data.ind;
    // data.x/y are raw floored coords; keep them only when actually on the map.
    hoverX = data.ind >= 0 ? data.x : -1;
    hoverY = data.ind >= 0 ? data.y : -1;
  }

  // Clear the block we were hovering last frame if it changed.
  const prevHoverMapName = es.hoveredGridMapName || es.selectedMapName;
  if (prevHoverMapName !== hoverMapName) {
    updateEditorStateMapNoReRender(prevHoverMapName, {
      hoveredTileIndex: -1,
      hoveredTileData: { x: -1, y: -1, ind: -1 },
    });
  }
  updateEditorStateNoReRender({
    hoveredGridMapName: hoverMapName === es.selectedMapName ? '' : hoverMapName,
  });

  const prevHoverInd = getEditorStateMap(hoverMapName)?.hoveredTileIndex ?? -1;
  if (hoverInd !== prevHoverInd && hoverMap) {
    onTileHoverIndChange(
      hoverMap,
      es,
      es.currentPaintAction,
      prevHoverInd,
      hoverInd
    );
  }
  updateEditorStateMapNoReRender(hoverMapName, {
    hoveredTileIndex: hoverInd,
    hoveredTileData: { x: hoverX, y: hoverY, ind: hoverInd },
  });

  updateMapCanvasCursor(
    canvasEl,
    es.currentPaintAction as PaintActionType,
    hoverInd,
    es.isSelectDragging,
    es.hoveredGridAdjacentSlot
  );

  const currentAction = getCurrentAction();
  if (currentAction) {
    // Route stroke updates to whichever block the stroke started on.
    const paintMap = es.activePaintMapName
      ? hoverAssets.maps.find((m) => m.name === es.activePaintMapName) ??
        currentMap
      : currentMap;
    if (paintMap) {
      onActionUpdate(
        currentAction,
        paintMap,
        es,
        mapDataInterface.getTilesets()
      );
    }
  }

  ctx.clearRect(
    0,
    0,
    mapDataInterface.getCanvas().width,
    mapDataInterface.getCanvas().height
  );
  drawRect(
    0,
    0,
    mapDataInterface.getCanvas().width,
    mapDataInterface.getCanvas().height,
    getColors().BACKGROUND2,
    false,
    ctx
  );

  if (currentMap) {
    const canvas = mapDataInterface.getCanvas();
    const { x, y, scale } = getTransform();
    // Hoisted out of the per-tile loops: getAssets() allocates a fresh object
    // on every call, and getTileList() walks the layer cache on every call.
    const assets = mapDataInterface.getAssets();
    const editorState = mapDataInterface.getEditorState();
    const spriteMap = mapDataInterface.getSpriteMap();
    const tilesets = mapDataInterface.getTilesets();
    const hoveredMapTileIndex =
      getEditorStateMap(editorState.selectedMapName)?.hoveredTileIndex ?? -1;
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
      spriteHeight
    );

    disableCanvasSmoothing(ctx);

    // Editable grid neighbours to run renderToolUi for, once the block loop has
    // laid their tiles down. Populated inside the (single-iteration) layer loop.
    const editableNeighbourToolPasses: {
      map: CarcerMapTemplate;
      offsetPixelX: number;
      offsetPixelY: number;
    }[] = [];

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
        (canvas.height * newScale) / 2
      );
      ctx.translate(
        -(currentMap.width * spriteWidth * newScale) / 2,
        -(currentMap.height * spriteHeight * newScale) / 2
      );

      const overlayTextEntries: OverlayTextEntry[] | undefined =
        editorState.drawOverlayText ? [] : undefined;
      const placement = findMapGridPlacement(currentMap.name, assets.mapGrids);

      if (placement) {
        const mapsByName: Record<string, CarcerMapTemplate> = {};
        for (const map of assets.maps) {
          mapsByName[map.name] = map;
        }
        const { slotWidth, slotHeight } = getMapGridSlotDimensions(
          placement,
          spriteWidth,
          spriteHeight,
          newScale,
        );
        const gridRenderRadius = editorState.gridRenderRadius ?? 2;
        const gridEditRadius = editorState.gridEditEnabled
          ? Math.min(editorState.gridEditRadius ?? 1, gridRenderRadius)
          : 0;
        const adjacentSlots = getGridAdjacentSlots(
          placement,
          mapsByName,
          gridRenderRadius,
        );
        const adjacentMaps = getGridMapsWithinRadius(
          placement,
          mapsByName,
          gridRenderRadius,
        );
        for (const adjacent of adjacentMaps) {
          const offsetPixelX = adjacent.offsetX * slotWidth;
          const offsetPixelY = adjacent.offsetY * slotHeight;
          const chebyshev = Math.max(
            Math.abs(adjacent.offsetX),
            Math.abs(adjacent.offsetY),
          );
          const blockOriginX = originX + offsetPixelX;
          const blockOriginY = originY + offsetPixelY;

          if (chebyshev <= gridEditRadius) {
            // Editable neighbour: same render path as the focused map.
            ctx.save();
            ctx.translate(offsetPixelX, offsetPixelY);
            renderMapBlockTiles({
              map: adjacent.map,
              ctx,
              scale: newScale,
              originX: blockOriginX,
              originY: blockOriginY,
              canvasWidth: canvas.width,
              canvasHeight: canvas.height,
              spriteMap,
              tilesets,
              characters: assets.characters,
              items: assets.items,
              layer: editorState.currentLevel,
              showGrid,
              hoveredTileIndex:
                getEditorStateMap(adjacent.map.name)?.hoveredTileIndex ?? -1,
            });
            ctx.restore();
            editableNeighbourToolPasses.push({
              map: adjacent.map,
              offsetPixelX,
              offsetPixelY,
            });
          } else {
            // Context-only neighbour: dimmed, read-only.
            renderMapTilesAtOffset({
              map: adjacent.map,
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
              visibleRange: getVisibleTileRange({
                originX: blockOriginX,
                originY: blockOriginY,
                canvasWidth: canvas.width,
                canvasHeight: canvas.height,
                mapWidth: adjacent.map.width,
                mapHeight: adjacent.map.height,
                tileWidth: adjacent.map.spriteWidth,
                tileHeight: adjacent.map.spriteHeight,
                scale: newScale,
              }),
            });
          }
        }

        renderGridAdjacentNavigation({
          ctx,
          slots: adjacentSlots,
          slotWidth,
          slotHeight,
          hoveredOffset: editorState.hoveredGridAdjacentSlot,
        });
      }

      renderMapBlockTiles({
        map: currentMap,
        ctx,
        scale: newScale,
        originX,
        originY,
        canvasWidth: canvas.width,
        canvasHeight: canvas.height,
        spriteMap,
        tilesets,
        characters: assets.characters,
        items: assets.items,
        layer: editorState.currentLevel,
        showGrid,
        hoveredTileIndex: hoveredMapTileIndex,
        overlayTextEntries,
      });

      if (overlayTextEntries?.length) {
        drawOverlayTextEntries(ctx, overlayTextEntries);
      }

      ctx.restore();
    }

    renderToolUi(
      editorState,
      currentMap,
      ctx,
      spriteMap,
      tilesets,
      assets.characters,
      assets.items
    );

    // Tool preview (hover box fill, brush ghost, fill outline) for the editable
    // neighbour the pointer / active stroke is on. Others no-op: only the block
    // with a hoveredTileIndex >= 0 draws anything.
    for (const pass of editableNeighbourToolPasses) {
      renderToolUi(
        editorState,
        pass.map,
        ctx,
        spriteMap,
        tilesets,
        assets.characters,
        assets.items,
        pass.map.name,
        pass.offsetPixelX,
        pass.offsetPixelY
      );
    }
  }
};
