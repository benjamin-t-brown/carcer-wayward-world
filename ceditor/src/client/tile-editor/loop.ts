import { calculateHoveredTile } from './renderState';
import {
  PaintActionType,
  getCurrentAction,
  onActionUpdate,
  onTileHoverIndChange,
} from './paintTools';
import {
  drawLine,
  drawRect,
  drawSprite,
  getSpriteNameFromTile,
  getSpriteNameFromTileMetadata,
} from './draw';
import { CarcerMapTemplate } from '../components/MapTemplateForm';
import {
  EditorState,
  getEditorState,
  updateEditorStateNoReRender,
} from './editorState';
import { getIndsOfBoundingRect, getIsDraggingRight, getTransform } from './editorEvents';
import { Sprite } from '../utils/assetLoader';
import { TilesetTemplate } from '../components/TilesetTemplateForm';
import { calculateFillIndsFloor } from './fill';

// let currentMap: MapResponse | null = null;
// let isLooping = false;

const getColors = () => {
  return {
    BACKGROUND2: 'orange',
    TEXT: 'white',
  };
};

const drawHighlightRect = (
  tileX: number,
  tileY: number,
  tileWidth: number,
  tileHeight: number,
  scale: number,
  ctx: CanvasRenderingContext2D
) => {
  drawRect(
    tileX,
    tileY,
    tileWidth * scale,
    tileHeight * scale,
    'rgba(67, 67, 255, 0.44)',
    false,
    ctx
  );
};

const drawHighlightEraseRect = (
  tileX: number,
  tileY: number,
  tileWidth: number,
  tileHeight: number,
  scale: number,
  ctx: CanvasRenderingContext2D
) => {
  drawRect(
    tileX,
    tileY,
    tileWidth * scale,
    tileHeight * scale,
    'rgba(255, 67, 67, 0.44)',
    false,
    ctx
  );
};

const drawHighlightTile = (
  sprite: Sprite,
  tileX: number,
  tileY: number,
  scale: number,
  ctx: CanvasRenderingContext2D
) => {
  ctx.save();
  ctx.globalAlpha = 0.65;
  drawSprite(sprite, tileX, tileY, scale, ctx);
  ctx.restore();
};

// const findBrushTile = (
//   startX: number,
//   startY: number,
//   x: number,
//   y: number,
//   width: number,
//   height: number,
//   floorDrawBrush: FloorBrushData[]
// ) => {
//   return floorDrawBrush.find((bt) => {
//     const newX = startX + bt.xOffset;
//     const newY = startY + bt.yOffset;
//     if (newX < 0 || newX >= width || newY < 0 || newY >= height) {
//       return false;
//     }
//     if (newX === x && newY === y) {
//       return true;
//     } else {
//       return false;
//     }
//   });
// };

const renderToolUi = (
  editorState: EditorState,
  mapData: CarcerMapTemplate,
  ctx: CanvasRenderingContext2D,
  sprites: Sprite[],
  tilesets: TilesetTemplate[]
) => {
  const currentPaintAction = editorState.currentPaintAction;
  const paintTileIndexInTileset = editorState.selectedTileIndexInTileset;
  const selectedTileset = tilesets.find(
    (t) => t.name === editorState.selectedTilesetName
  );
  const paintTile = selectedTileset?.tiles[paintTileIndexInTileset];
  const paintTileSprite = paintTile
    ? sprites.find(
        (s) =>
          s.name ===
          getSpriteNameFromTileMetadata(
            selectedTileset?.spriteBase ?? '',
            paintTile
          )
      )
    : null;
  const tileWidth = mapData.spriteWidth;
  const tileHeight = mapData.spriteHeight;
  const rectCloneBrushTiles = editorState.rectCloneBrushTiles;

  const { x: transformX, y: transformY, scale } = getTransform();
  ctx.save();
  ctx.translate(transformX, transformY);
  ctx.translate(
    (ctx.canvas.width * scale) / 2,
    (ctx.canvas.height * scale) / 2
  );
  ctx.translate(
    -(mapData.width * tileWidth * scale) / 2,
    -(mapData.height * tileHeight * scale) / 2
  );

  if (currentPaintAction === PaintActionType.FILL) {
    for (const ind of editorState.fillIndsFloor) {
      if (paintTileSprite) {
        const tileX = (ind % mapData.width) * tileWidth * scale;
        const tileY = Math.floor(ind / mapData.width) * tileHeight * scale;
        drawHighlightTile(paintTileSprite, tileX, tileY, scale, ctx);
        drawHighlightRect(tileX, tileY, tileWidth, tileHeight, scale, ctx);
      }
    }
  } else if (currentPaintAction === PaintActionType.ERASE) {
    const hoveredTileInd = editorState.hoveredTileIndex;
    if (hoveredTileInd > -1 && paintTileSprite) {
      const tileX = (hoveredTileInd % mapData.width) * tileWidth * scale;
      const tileY =
        Math.floor(hoveredTileInd / mapData.width) * tileHeight * scale;
      drawHighlightEraseRect(tileX, tileY, tileWidth, tileHeight, scale, ctx);
    }
  } else if (currentPaintAction === PaintActionType.DRAW) {
    if (
      getIsDraggingRight() &&
      editorState.hoveredTileData.x > -1 &&
      editorState.hoveredTileData.y > -1
    ) {
      const ind0 = editorState.rectSelectTileIndStart;
      const ind1 = editorState.rectSelectTileIndEnd;
      const dragSelectedInds = getIndsOfBoundingRect(
        ind0,
        ind1,
        mapData.width ?? 0
      );
      for (const ind of dragSelectedInds) {
        const tile = mapData.tiles[ind];
        if (tile) {
          const tileX = (ind % mapData.width) * tileWidth * scale;
          const tileY = Math.floor(ind / mapData.width) * tileHeight * scale;
          drawHighlightRect(tileX, tileY, tileWidth, tileHeight, scale, ctx);
        }
      }

      // drawHighlightRect(tileX, tileY, tileWidth, tileHeight, scale, ctx);
    } else if (rectCloneBrushTiles.length) {
      for (const brush of rectCloneBrushTiles) {
        const newX = editorState.hoveredTileData.x + brush.xOffset;
        const newY = editorState.hoveredTileData.y + brush.yOffset;
        if (
          newX < 0 ||
          newX >= mapData.width ||
          newY < 0 ||
          newY >= mapData.height
        ) {
          continue;
        }

        const spriteName = getSpriteNameFromTile(brush.originalTile.ref);
        const spr = sprites.find((s) => s.name === spriteName);
        const tileX = newX * mapData.spriteWidth * scale;
        const tileY = newY * mapData.spriteHeight * scale;
        drawHighlightRect(tileX, tileY, tileWidth, tileHeight, scale, ctx);
        if (spr) {
          drawHighlightTile(spr, tileX, tileY, scale, ctx);
        }
      }
    } else {
      if (paintTileSprite) {
        const tileX = editorState.hoveredTileData.x * tileWidth * scale;
        const tileY = editorState.hoveredTileData.y * tileHeight * scale;
        drawHighlightRect(tileX, tileY, tileWidth, tileHeight, scale, ctx);
        drawHighlightTile(paintTileSprite, tileX, tileY, scale, ctx);
      }
    }
  }
  ctx.restore();
};

export const loop = (
  mapDataInterface: {
    getCanvas: () => HTMLCanvasElement;
    getMapData: () => CarcerMapTemplate;
    getEditorState: () => EditorState;
    getSprites: () => Sprite[];
    getTilesets: () => TilesetTemplate[];
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

  const currentMap = mapDataInterface.getMapData();
  const data = calculateHoveredTile(currentMap, mapDataInterface.getCanvas());
  if (data.ind !== mapDataInterface.getEditorState().hoveredTileIndex) {
    onTileHoverIndChange(
      currentMap,
      mapDataInterface.getEditorState().currentPaintAction,
      mapDataInterface.getEditorState().hoveredTileIndex,
      data.ind
    );
  }
  updateEditorStateNoReRender({
    hoveredTileIndex: data.ind,
    hoveredTileData: {
      x: data.x,
      y: data.y,
      ind: data.ind,
    },
  });

  const currentAction = getCurrentAction();
  if (currentAction && currentMap) {
    onActionUpdate(
      currentAction,
      currentMap,
      mapDataInterface.getEditorState()
    );
  }
  // const tileSize = getSpriteSize();

  // const hoveredTileInd = getHoveredTileInd();
  // const hoveredTileX = data.x;
  // const hoveredTileY = data.y;

  // const currentPaintAction = getCurrentPaintAction();
  // const selectedTileId = getCurrentSelectedTileId();
  // const selectedTileSprite = tileIdToSpriteName(selectedTileId);
  // const [ind0, ind1] = getRectSelectTileInds();
  // const dragSelectedInds = getIndsOfBoundingRect(
  //   ind0,
  //   ind1,
  //   currentMap?.width ?? 0
  // );
  // const floorDrawBrush = getTileFloorBrush();

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
          const tileId = refTile.tileId ?? -1;
          if (tileId > -1) {
            const tileX =
              x * mapDataInterface.getMapData().spriteWidth * newScale;
            const tileY =
              y * mapDataInterface.getMapData().spriteHeight * newScale;
            const spriteName = getSpriteNameFromTile(refTile);
            const sprite = mapDataInterface
              .getSprites()
              .find((s) => s.name === spriteName);
            if (sprite) {
              drawSprite(sprite, tileX, tileY, newScale, ctx);
            }
          }

          if (i === 0) {
            // grid
            const x1 = x * mapDataInterface.getMapData().spriteWidth * scale;
            const y1 = y * mapDataInterface.getMapData().spriteHeight * scale;
            const x2 = x1 + mapDataInterface.getMapData().spriteWidth * scale;
            const y2 = y1 + mapDataInterface.getMapData().spriteHeight * scale;
            const hoveredMapTileIndex =
              mapDataInterface.getEditorState().hoveredTileIndex;
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
      mapDataInterface.getSprites(),
      mapDataInterface.getTilesets()
    );
  }

  // drawRect(
  //   canvas.width / 2 - 2,
  //   canvas.height / 2 - 2,
  //   4,
  //   4,
  //   'red',
  //   false,
  //   ctx
  // );

  // for (const r of Object.values(getRenderables())) {
  //   r();
  // }

  // const currentOpenMapName =
};

// export const setLoopMap = async (mapPath: string) => {
//   currentMap = null;
//   if (!mapPath) {
//     return;
//   }
//   const mapData = await getMapData(mapPath);
//   currentMap = mapData;
//   console.log('set currentMap', mapPath, mapData);
// };

// export const startLoop = (canvas: HTMLCanvasElement) => {
//   if (isLooping) {
//     return;
//   }
//   const ctx = canvas.getContext('2d');
//   if (!ctx) {
//     return;
//   }

//   setCanvas(canvas);

//   isLooping = true;

//   const _loop = (ms: number) => {
//     ctx.imageSmoothingEnabled = false;
//     loop(canvas, ms);
//     window.requestAnimationFrame(_loop);
//   };
//   window.requestAnimationFrame(_loop);
//   // setInterval(_loop, 100);

//   initPanzoom(canvas);
// };
