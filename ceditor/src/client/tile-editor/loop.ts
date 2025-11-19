import {
  FloorBrushData,
  calculateHoveredTile,
  getFillIndsFloor,
  getHoveredTileInd,
  getRectSelectTileInds,
  getTileFloorBrush,
  setHoveredTileData,
  setHoveredTileInd,
} from './renderState';
import {
  PaintActionType,
  getCurrentAction,
  onActionUpdate,
} from './paintTools';
import { drawLine, drawRect } from './draw';
import { CarcerMapTemplate } from '../components/MapTemplateForm';
import { EditorState } from './editorState';
import { getTransform } from './editorEvents';

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
  tileSize: number,
  scale: number,
  ctx: CanvasRenderingContext2D
) => {
  drawRect(
    tileX,
    tileY,
    tileSize * scale,
    tileSize * scale,
    'rgba(67, 67, 255, 0.44)',
    false,
    ctx
  );
};

// const drawHighlightTile = (
//   spriteName: string,
//   tileX: number,
//   tileY: number,
//   scale: number,
//   color: string,
//   ctx: CanvasRenderingContext2D
// ) => {
//   ctx.save();
//   ctx.globalAlpha = 0.65;
//   drawSprite(spriteName, tileX, tileY, scale, color, ctx);
//   ctx.restore();
// };

const findBrushTile = (
  startX: number,
  startY: number,
  x: number,
  y: number,
  width: number,
  height: number,
  floorDrawBrush: FloorBrushData[]
) => {
  return floorDrawBrush.find((bt) => {
    const newX = startX + bt.xOffset;
    const newY = startY + bt.yOffset;
    if (newX < 0 || newX >= width || newY < 0 || newY >= height) {
      return false;
    }
    if (newX === x && newY === y) {
      return true;
    } else {
      return false;
    }
  });
};

// const renderToolUi = (
//   appState: AppState,
//   mapData: MapResponse,
//   ctx: CanvasRenderingContext2D
// ) => {
//   const currentMapPath = appState.currentMapPath;
//   const hoveredTileInd = getHoveredTileInd();
//   const data = calculateHoveredTile(currentMapPath);
//   const hoveredTileX = data.x;
//   const hoveredTileY = data.y;
//   const currentAction = getCurrentAction();
//   if (currentAction && currentMap) {
//     onActionUpdate(currentAction, currentMap);
//   }

//   const tileSize = getSpriteSize();
//   const currentPaintAction = getCurrentPaintAction();
//   const selectedTileId = getCurrentSelectedTileId();
//   const selectedTileSprite = tileIdToSpriteName(selectedTileId);
//   const [ind0, ind1] = getRectSelectTileInds();
//   const dragSelectedInds = getIndsOfBoundingRect(
//     ind0,
//     ind1,
//     currentMap?.width ?? 0
//   );
//   const floorDrawBrush = getTileFloorBrush();

//   const { x: transformX, y: transformY, scale } = getTransform();
//   ctx.save();
//   ctx.translate(transformX, transformY);
//   ctx.translate(
//     (ctx.canvas.width * scale) / 2,
//     (ctx.canvas.height * scale) / 2
//   );
//   ctx.translate(
//     -(mapData.width * tileSize * scale) / 2,
//     -(mapData.height * tileSize * scale) / 2
//   );

//   if (currentPaintAction === PaintActionType.FILL) {
//     for (const ind of getFillIndsFloor()) {
//       if (selectedTileSprite) {
//         const tileX = (ind % mapData.width) * tileSize * scale;
//         const tileY = Math.floor(ind / mapData.width) * tileSize * scale;
//         drawHighlightTile(
//           selectedTileSprite,
//           tileX,
//           tileY,
//           scale,
//           getCurrentSelectedTileColor(),
//           ctx
//         );
//         drawHighlightRect(tileX, tileY, tileSize, scale, ctx);
//       }
//     }
//   } else if (currentPaintAction === PaintActionType.ERASE) {
//     if (hoveredTileInd > -1) {
//       const tileX = (hoveredTileInd % mapData.width) * tileSize * scale;
//       const tileY =
//         Math.floor(hoveredTileInd / mapData.width) * tileSize * scale;
//       drawHighlightRect(tileX, tileY, tileSize, scale, ctx);
//     }
//   } else if (currentPaintAction === PaintActionType.DRAW) {
//     if (getIsDraggingRight()) {
//       for (const tileIndex of dragSelectedInds) {
//         const tileX = (tileIndex % mapData.width) * tileSize * scale;
//         const tileY = Math.floor(tileIndex / mapData.width) * tileSize * scale;
//         drawHighlightRect(tileX, tileY, tileSize, scale, ctx);
//       }
//     } else {
//       if (floorDrawBrush.length) {
//         for (const bt of floorDrawBrush) {
//           const newX = hoveredTileX + bt.xOffset;
//           const newY = hoveredTileY + bt.yOffset;
//           if (
//             newX < 0 ||
//             newX >= mapData.width ||
//             newY < 0 ||
//             newY >= mapData.height
//           ) {
//             continue;
//           }
//           const spriteName = tileIdToSpriteName(
//             bt.originalTile.ref.arr[0] ?? 0
//           );
//           const tileX = newX * tileSize * scale;
//           const tileY = newY * tileSize * scale;
//           drawHighlightRect(tileX, tileY, tileSize, scale, ctx);
//           if (spriteName) {
//             drawHighlightTile(
//               spriteName,
//               tileX,
//               tileY,
//               scale,
//               bt.originalTile.ref.color[0],
//               ctx
//             );
//           }
//         }
//       } else {
//         if (selectedTileSprite) {
//           const tileX = hoveredTileX * tileSize * scale;
//           const tileY = hoveredTileY * tileSize * scale;
//           drawHighlightRect(tileX, tileY, tileSize, scale, ctx);
//           drawHighlightTile(
//             selectedTileSprite,
//             tileX,
//             tileY,
//             scale,
//             getCurrentSelectedTileColor(),
//             ctx
//           );
//         }
//       }
//     }
//   }
//   ctx.restore();
// };

export const loop = (
  mapDataInterface: {
    getCanvas: () => HTMLCanvasElement;
    getMapData: () => CarcerMapTemplate;
    getEditorState: () => EditorState;
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
  setHoveredTileInd(currentMap, data.ind);
  setHoveredTileData({
    x: data.x,
    y: data.y,
    ind: data.ind,
  });
  const currentAction = getCurrentAction();
  if (currentAction && currentMap) {
    onActionUpdate(currentAction, currentMap);
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
      undefined,
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
          const tileId = refTile.tileId ?? 0;
          if (tileId) {
            const tileX =
              x * mapDataInterface.getMapData().spriteWidth * newScale;
            const tileY =
              y * mapDataInterface.getMapData().spriteHeight * newScale;
          }

          if (i === 0) {
            // grid
            const x1 = x * mapDataInterface.getMapData().spriteWidth * scale;
            const y1 = y * mapDataInterface.getMapData().spriteHeight * scale;
            const x2 = x1 + mapDataInterface.getMapData().spriteWidth * scale;
            const y2 = y1 + mapDataInterface.getMapData().spriteHeight * scale;
            const selectedMapTileIndex =
              mapDataInterface.getEditorState().selectedTileIndex;
            let color = 'rgba(255, 255, 255, 0.25)';
            if (tileIndex === selectedMapTileIndex) {
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

    // renderToolUi(appState, currentMap, ctx);
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
