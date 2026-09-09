import { calculateHoveredTile } from './renderState';
import {
  getCurrentAction,
  onActionUpdate,
  onTileHoverIndChange,
} from './paintTools';
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
  findMapGridPlacement,
  getGridAdjacentMaps,
  getGridAdjacentSlots,
} from '../utils/mapGridIndex';
import { getMapGridSlotDimensions } from './gridMapNavigation';
import {
  EditorState,
  getEditorStateMap,
  updateEditorStateMap,
  updateEditorStateMapNoReRender,
  updateEditorStateNoReRender,
} from './editorState';
import {
  getTileList,
  getTransform,
  invalidateCanvasRectCache,
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
  ms: number
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
  const data = calculateHoveredTile(currentMap, mapDataInterface.getCanvas());
  if (
    data.ind !==
    getEditorStateMap(mapDataInterface.getEditorState().selectedMapName)
      ?.hoveredTileIndex
  ) {
    onTileHoverIndChange(
      currentMap,
      mapDataInterface.getEditorState(),
      mapDataInterface.getEditorState().currentPaintAction,
      getEditorStateMap(mapDataInterface.getEditorState().selectedMapName)
        ?.hoveredTileIndex ?? -1,
      data.ind
    );
  }
  updateEditorStateMapNoReRender(
    mapDataInterface.getEditorState().selectedMapName,
    {
      hoveredTileIndex: data.ind,
      hoveredTileData: {
        x: data.x,
        y: data.y,
        ind: data.ind,
      },
    }
  );

  updateMapCanvasCursor(
    mapDataInterface.getCanvas(),
    mapDataInterface.getEditorState().currentPaintAction as PaintActionType,
    data.ind,
    mapDataInterface.getEditorState().isSelectDragging,
    mapDataInterface.getEditorState().hoveredGridAdjacentSlot
  );

  const currentAction = getCurrentAction();
  if (currentAction && currentMap) {
    onActionUpdate(
      currentAction,
      currentMap,
      mapDataInterface.getEditorState(),
      mapDataInterface.getTilesets()
    );
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
    const { x, y, scale } = getTransform();
    const canvas = mapDataInterface.getCanvas();
    // Hoisted out of the per-tile loops: getAssets() allocates a fresh object
    // on every call, and getTileList() walks the layer cache on every call.
    const assets = mapDataInterface.getAssets();
    const editorState = mapDataInterface.getEditorState();
    const spriteMap = mapDataInterface.getSpriteMap();
    const tilesets = mapDataInterface.getTilesets();
    const mapTiles = getTileList(currentMap);
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
      let adjacentSlots: ReturnType<typeof getGridAdjacentSlots> = [];
      let slotWidth = currentMap.width * spriteWidth * newScale;
      let slotHeight = currentMap.height * spriteHeight * newScale;

      if (placement) {
        const mapsByName: Record<string, CarcerMapTemplate> = {};
        for (const map of assets.maps) {
          mapsByName[map.name] = map;
        }
        ({ slotWidth, slotHeight } = getMapGridSlotDimensions(
          placement,
          spriteWidth,
          spriteHeight,
          newScale,
        ));
        adjacentSlots = getGridAdjacentSlots(placement, mapsByName);
        const adjacentMaps = getGridAdjacentMaps(placement, mapsByName);
        for (const adjacent of adjacentMaps) {
          const offsetPixelX = adjacent.offsetX * slotWidth;
          const offsetPixelY = adjacent.offsetY * slotHeight;
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
              originX: originX + offsetPixelX,
              originY: originY + offsetPixelY,
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

        renderGridAdjacentNavigation({
          ctx,
          slots: adjacentSlots,
          slotWidth,
          slotHeight,
          hoveredOffset: editorState.hoveredGridAdjacentSlot,
        });
      }

      if (i === 0) {
        drawRect(
          0,
          0,
          currentMap.width * spriteWidth * newScale,
          currentMap.height * spriteHeight * newScale,
          'black',
          false,
          ctx
        );
      }

      const visibleRange = getVisibleTileRange({
        originX,
        originY,
        canvasWidth: canvas.width,
        canvasHeight: canvas.height,
        mapWidth: currentMap.width,
        mapHeight: currentMap.height,
        tileWidth: spriteWidth,
        tileHeight: spriteHeight,
        scale: newScale,
      });

      for (
        let y = visibleRange?.minY ?? 0;
        y <= (visibleRange?.maxY ?? -1);
        y++
      ) {
        for (
          let x = visibleRange?.minX ?? 0;
          x <= (visibleRange?.maxX ?? -1);
          x++
        ) {
          const tileIndex = y * currentMap.width + x;
          const refTile = mapTiles[tileIndex];
          renderTileAndExtras({
            refTile,
            x,
            y,
            ctx,
            newScale,
            spriteMap,
            mapSpriteWidth: spriteWidth,
            mapSpriteHeight: spriteHeight,
            tilesets,
            characters: assets.characters,
            items: assets.items,
            overlayTextEntries,
          });

          if (i === 0) {
            const x1 = x * spriteWidth * newScale;
            const y1 = y * spriteHeight * newScale;
            const x2 = x1 + spriteWidth * newScale;
            const y2 = y1 + spriteHeight * newScale;
            let color = 'rgba(255, 255, 255, 0.25)';
            if (tileIndex === hoveredMapTileIndex) {
              color = 'rgba(100, 100, 255, 0.5)';
              drawLine(x1, y1, x2, y1, color, 2, ctx);
              drawLine(x1, y1, x1, y2, color, 2, ctx);
              drawLine(x2, y2, x2, y1, color, 2, ctx);
              drawLine(x2, y2, x1, y2, color, 2, ctx);
            } else if (showGrid) {
              drawLine(x1, y1, x2, y1, color, 1, ctx);
              drawLine(x1, y1, x1, y2, color, 1, ctx);
            }
          }
        }
      }

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
  }
};
