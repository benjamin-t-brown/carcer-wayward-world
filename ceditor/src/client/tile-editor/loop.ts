import { calculateHoveredTile } from './renderState';
import {
  getCurrentAction,
  onActionUpdate,
  onTileHoverIndChange,
} from './paintTools';
import { drawLine, drawRect } from '../utils/draw';
import {
  CarcerMapTemplate,
  TilesetTemplate,
  CharacterTemplate,
  ItemTemplate,
  GameEvent,
} from '../types/assets';
import {
  EditorState,
  getEditorStateMap,
  updateEditorStateMap,
  updateEditorStateMapNoReRender,
  updateEditorStateNoReRender,
} from './editorState';
import { getTransform } from './editorEvents';
import { Sprite } from '../utils/assetLoader';
import { renderTileAndExtras, renderToolUi } from './renderUi';

// let currentMap: MapResponse | null = null;
// let isLooping = false;
const getColors = () => {
  return {
    BACKGROUND2: 'orange',
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

  const currentAction = getCurrentAction();
  if (currentAction && currentMap) {
    onActionUpdate(
      currentAction,
      currentMap,
      mapDataInterface.getEditorState()
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

    //background rect
    ctx.save();
    ctx.translate(x, y);
    ctx.scale(scale, scale);
    ctx.translate(
      mapDataInterface.getCanvas().width / 2,
      mapDataInterface.getCanvas().height / 2
    );
    ctx.translate(
      -(currentMap.width * mapDataInterface.getMapData().spriteWidth) / 2,
      -(currentMap.height * mapDataInterface.getMapData().spriteHeight) / 2
    );
    drawRect(
      0,
      0,
      currentMap.width * mapDataInterface.getMapData().spriteWidth,
      currentMap.height * mapDataInterface.getMapData().spriteHeight,
      'black',
      false,
      ctx
    );
    ctx.restore();

    // layers
    for (let i = 0; i < 1; i++) {
      const newScale = scale * (1 + i * 0.04);

      const focalX = mapDataInterface.getCanvas().width / 2;
      const focalY = mapDataInterface.getCanvas().height / 2;

      const offsetX = focalX - (newScale / scale) * (focalX - x);
      const offsetY = focalY - (newScale / scale) * (focalY - y);

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

      for (let y = 0; y < currentMap.height; y++) {
        for (let x = 0; x < currentMap.width; x++) {
          const tileIndex = y * currentMap.width + x;
          const refTile = currentMap.tiles[tileIndex];
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
            // grid
            const x1 = x * mapDataInterface.getMapData().spriteWidth * scale;
            const y1 = y * mapDataInterface.getMapData().spriteHeight * scale;
            const x2 = x1 + mapDataInterface.getMapData().spriteWidth * scale;
            const y2 = y1 + mapDataInterface.getMapData().spriteHeight * scale;
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
            } else {
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
