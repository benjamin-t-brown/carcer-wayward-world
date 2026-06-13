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
} from '../utils/mapGridIndex';
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
  updateMapCanvasCursor,
} from './editorEvents';
import { PaintActionType } from './paintTools';
import { Sprite } from '../utils/assetLoader';
import {
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
    mapDataInterface.getEditorState().isSelectDragging
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
    const spriteWidth = mapDataInterface.getMapData().spriteWidth;
    const spriteHeight = mapDataInterface.getMapData().spriteHeight;
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

      const focalX = mapDataInterface.getCanvas().width / 2;
      const focalY = mapDataInterface.getCanvas().height / 2;

      const offsetX = focalX - (newScale / scale) * (focalX - panX);
      const offsetY = focalY - (newScale / scale) * (focalY - panY);

      ctx.save();
      ctx.translate(offsetX, offsetY);
      ctx.translate(
        (mapDataInterface.getCanvas().width * newScale) / 2,
        (mapDataInterface.getCanvas().height * newScale) / 2
      );
      ctx.translate(
        -(
          currentMap.width *
          mapDataInterface.getMapData().spriteWidth *
          newScale
        ) / 2,
        -(
          currentMap.height *
          mapDataInterface.getMapData().spriteHeight *
          newScale
        ) / 2
      );

      const assets = mapDataInterface.getAssets();
      const editorState = mapDataInterface.getEditorState();
      const placement = findMapGridPlacement(currentMap.name, assets.mapGrids);
      if (placement) {
        const mapsByName: Record<string, CarcerMapTemplate> = {};
        for (const map of assets.maps) {
          mapsByName[map.name] = map;
        }
        const slotWidth = currentMap.width * spriteWidth * newScale;
        const slotHeight = currentMap.height * spriteHeight * newScale;
        const adjacentMaps = getGridAdjacentMaps(placement, mapsByName);
        for (const adjacent of adjacentMaps) {
          renderMapTilesAtOffset({
            map: adjacent.map,
            ctx,
            scale: newScale,
            offsetPixelX: adjacent.offsetX * slotWidth,
            offsetPixelY: adjacent.offsetY * slotHeight,
            opacity: 0.5,
            spriteMap: mapDataInterface.getSpriteMap(),
            tilesets: mapDataInterface.getTilesets(),
            characters: assets.characters,
            items: assets.items,
            layer: editorState.currentLevel,
            drawOverlayText: false,
          });
        }
      }

      if (i === 0) {
        drawRect(
          0,
          0,
          currentMap.width * spriteWidth,
          currentMap.height * spriteHeight,
          'black',
          false,
          ctx
        );
      }

      for (let y = 0; y < currentMap.height; y++) {
        for (let x = 0; x < currentMap.width; x++) {
          const tileIndex = y * currentMap.width + x;
          const mapTiles = getTileList(currentMap);
          const refTile = mapTiles[tileIndex];
          renderTileAndExtras({
            refTile,
            x,
            y,
            ctx,
            newScale,
            spriteMap: mapDataInterface.getSpriteMap(),
            mapSpriteWidth: mapDataInterface.getMapData().spriteWidth,
            mapSpriteHeight: mapDataInterface.getMapData().spriteHeight,
            tilesets: mapDataInterface.getTilesets(),
            characters: mapDataInterface.getAssets().characters,
            items: mapDataInterface.getAssets().items,
            drawOverlayText: mapDataInterface.getEditorState().drawOverlayText,
          });

          if (i === 0) {
            const x1 = x * spriteWidth * newScale;
            const y1 = y * spriteHeight * newScale;
            const x2 = x1 + spriteWidth * newScale;
            const y2 = y1 + spriteHeight * newScale;
            const hoveredMapTileIndex = getEditorStateMap(
              mapDataInterface.getEditorState().selectedMapName
            )?.hoveredTileIndex;
            let color = 'rgba(255, 255, 255, 0.25)';
            if (tileIndex === hoveredMapTileIndex) {
              color = 'rgba(100, 100, 255, 0.5)';
              drawLine(x1, y1, x2, y1, color, 2, ctx);
              drawLine(x1, y1, x1, y2, color, 2, ctx);
              drawLine(x2, y2, x2, y1, color, 2, ctx);
              drawLine(x2, y2, x1, y2, color, 2, ctx);
            } else if (mapDataInterface.getEditorState().showGrid) {
              drawLine(x1, y1, x2, y1, color, 1, ctx);
              drawLine(x1, y1, x1, y2, color, 1, ctx);
            }
          }
        }
      }
      ctx.restore();
    }

    renderToolUi(
      mapDataInterface.getEditorState(),
      currentMap,
      ctx,
      mapDataInterface.getSpriteMap(),
      mapDataInterface.getTilesets(),
      mapDataInterface.getAssets().characters,
      mapDataInterface.getAssets().items
    );
  }
};
