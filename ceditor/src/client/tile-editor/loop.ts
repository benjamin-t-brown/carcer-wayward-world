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
import {
  CarcerMapTemplate,
  CarcerMapTileTemplate,
} from '../components/MapTemplateForm';
import {
  EditorState,
  getEditorState,
  updateEditorStateNoReRender,
} from './editorState';
import {
  getIndsOfBoundingRect,
  getIsDraggingRight,
  getTransform,
} from './editorEvents';
import { Sprite } from '../utils/assetLoader';
import { TilesetTemplate } from '../components/TilesetTemplateForm';
import { calculateFillIndsFloor } from './fill';
import { CharacterTemplate } from '../components/CharacterTemplateForm';
import { ItemTemplate } from '../components/ItemTemplateForm';
import { GameEvent } from '../components/GameEventForm';

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

const drawSelectedTileRect = (
  tileX: number,
  tileY: number,
  tileWidth: number,
  tileHeight: number,
  scale: number,
  ctx: CanvasRenderingContext2D
) => {
  // Draw a thicker border to indicate selected tile
  const borderWidth = 2;
  drawRect(
    tileX,
    tileY,
    tileWidth * scale,
    tileHeight * scale,
    'rgba(255, 255, 255, 0.2)',
    false,
    ctx
  );
  // Draw inner border for better visibility
  drawRect(
    tileX + borderWidth,
    tileY + borderWidth,
    tileWidth * scale - borderWidth * 2,
    tileHeight * scale - borderWidth * 2,
    'rgba(0, 0, 255, 0.2)',
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
  tilesets: TilesetTemplate[],
  characters: CharacterTemplate[],
  items: ItemTemplate[]
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
  const selectedTileInd = editorState.selectedTileInd;

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

  // Draw selected tile indicator
  if (selectedTileInd >= 0 && selectedTileInd < mapData.tiles.length) {
    const selectedTile = mapData.tiles[selectedTileInd];
    const x = selectedTileInd % mapData.width;
    const y = Math.floor(selectedTileInd / mapData.width);
    const tileX = x * tileWidth * scale;
    const tileY = y * tileHeight * scale;
    drawSelectedTileRect(tileX, tileY, tileWidth, tileHeight, scale, ctx);
  }

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
  } else if (currentPaintAction === PaintActionType.SELECT) {
    // Draw the source tile at the destination when dragging
    if (editorState.isSelectDragging) {
      const sourceTileIndex = editorState.selectDragSourceTileIndex;
      const destTileIndex = editorState.hoveredTileIndex;

      if (
        sourceTileIndex >= 0 &&
        destTileIndex >= 0 &&
        sourceTileIndex !== destTileIndex
      ) {
        ctx.save()
        ctx.globalAlpha = 0.5;
        drawTileAndExtras({
          refTile: mapData.tiles[sourceTileIndex],
          x: editorState.hoveredTileData.x,
          y: editorState.hoveredTileData.y,
          ctx,
          newScale: scale,
          sprites,
          mapSpriteWidth: mapData.spriteWidth,
          mapSpriteHeight: mapData.spriteHeight,
          characters,
          items,
        });
        ctx.restore();
      }
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

const drawTileAndExtras = (args: {
  refTile: CarcerMapTileTemplate;
  x: number;
  y: number;
  ctx: CanvasRenderingContext2D;
  newScale: number;
  sprites: Sprite[];
  mapSpriteWidth: number;
  mapSpriteHeight: number;
  characters: CharacterTemplate[];
  items: ItemTemplate[];
}) => {
  const {
    refTile,
    x,
    y,
    ctx,
    newScale,
    sprites,
    mapSpriteWidth,
    mapSpriteHeight,
    characters,
    items,
  } = args;
  const controlSprites: string[] = [];
  const tileX = x * mapSpriteWidth * newScale;
  const tileY = y * mapSpriteHeight * newScale;
  const spriteName = getSpriteNameFromTile(refTile);
  const sprite = sprites.find((s) => s.name === spriteName);
  if (sprite) {
    drawSprite(sprite, tileX, tileY, newScale, ctx);
  }

  for (const characterName of refTile.characters) {
    const characterTemplate = characters.find((c) => c.name === characterName);
    if (characterTemplate) {
      const characterSpriteName = `${characterTemplate.spritesheet}_${characterTemplate.spriteOffset}`;
      const characterSprite = sprites.find(
        (s) => s.name === characterSpriteName
      );
      if (characterSprite) {
        drawSprite(characterSprite, tileX, tileY, newScale, ctx);
      } else {
        console.error(`Character sprite not found: ${characterSpriteName}`);
      }
    }
  }

  for (const itemName of refTile.items) {
    const itemTemplate = items.find((i) => i.name === itemName);
    if (itemTemplate) {
      const itemSpriteName = itemTemplate.icon;
      const itemSprite = sprites.find((s) => s.name === itemSpriteName);
      if (itemSprite) {
        const spriteWidth = itemSprite.width;
        const spriteHeight = itemSprite.height;
        const spriteX = tileX + ((mapSpriteWidth - spriteWidth) * newScale) / 2;
        const spriteY =
          tileY + ((mapSpriteHeight - spriteHeight) * newScale) / 2;
        drawSprite(itemSprite, spriteX, spriteY, newScale, ctx);
      } else {
        console.error(`Item sprite not found: ${itemSpriteName}`);
      }
    }
  }

  if (refTile.tileOverrides) {
    controlSprites.push('control_2');
  }

  let controlI = 0;
  for (const spriteName of controlSprites) {
    const sprite = sprites.find((s) => s.name === spriteName);
    if (sprite) {
      drawSprite(sprite, tileX, tileY + controlI * 8 * newScale, newScale, ctx);
    }
    controlI++;
  }
};

export const loop = (
  mapDataInterface: {
    getCanvas: () => HTMLCanvasElement;
    getMapData: () => CarcerMapTemplate;
    getEditorState: () => EditorState;
    getSprites: () => Sprite[];
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
          const tileControlSprites: string[] = [];
          const tileIndex = y * currentMap.width + x;
          const refTile = currentMap.tiles[tileIndex];
          const tileId = refTile.tileId ?? -1;
          drawTileAndExtras({
            refTile,
            x,
            y,
            ctx,
            newScale,
            sprites: mapDataInterface.getSprites(),
            mapSpriteWidth: mapDataInterface.getMapData().spriteWidth,
            mapSpriteHeight: mapDataInterface.getMapData().spriteHeight,
            characters: mapDataInterface.getAssets().characters,
            items: mapDataInterface.getAssets().items,
          });
          //   if (tileId > -1) {
          //     const tileX =
          //       x * mapDataInterface.getMapData().spriteWidth * newScale;
          //     const tileY =
          //       y * mapDataInterface.getMapData().spriteHeight * newScale;
          //     const spriteName = getSpriteNameFromTile(refTile);
          //     const sprite = mapDataInterface
          //       .getSprites()
          //       .find((s) => s.name === spriteName);
          //     if (sprite) {
          //       drawSprite(sprite, tileX, tileY, newScale, ctx);
          //     }

          //     for (const characterName of refTile.characters) {
          //       const characterTemplate = mapDataInterface
          //         .getAssets()
          //         .characters.find((c) => c.name === characterName);
          //       if (characterTemplate) {
          //         const characterSpriteName = `${characterTemplate.spritesheet}_${characterTemplate.spriteOffset}`;
          //         const characterSprite = mapDataInterface
          //           .getSprites()
          //           .find((s) => s.name === characterSpriteName);
          //         if (characterSprite) {
          //           drawSprite(characterSprite, tileX, tileY, newScale, ctx);
          //         } else {
          //           console.error(
          //             `Character sprite not found: ${characterSpriteName}`
          //           );
          //         }
          //       }
          //     }

          //     for (const itemName of refTile.items) {
          //       const itemTemplate = mapDataInterface
          //         .getAssets()
          //         .items.find((i) => i.name === itemName);
          //       if (itemTemplate) {
          //         const itemSpriteName = itemTemplate.icon;
          //         const itemSprite = mapDataInterface
          //           .getSprites()
          //           .find((s) => s.name === itemSpriteName);
          //         if (itemSprite) {
          //           const spriteWidth = itemSprite.width;
          //           const spriteHeight = itemSprite.height;
          //           const spriteX =
          //             tileX +
          //             ((mapDataInterface.getMapData().spriteWidth - spriteWidth) *
          //               newScale) /
          //               2;
          //           const spriteY =
          //             tileY +
          //             ((mapDataInterface.getMapData().spriteHeight -
          //               spriteHeight) *
          //               newScale) /
          //               2;
          //           drawSprite(itemSprite, spriteX, spriteY, newScale, ctx);
          //         } else {
          //           console.error(`Item sprite not found: ${itemSpriteName}`);
          //         }
          //       }
          //     }

          //     if (refTile.tileOverrides) {
          //       tileControlSprites.push('control_2');
          //     }

          //     let controlI = 0;
          //     for (const spriteName of tileControlSprites) {
          //       const sprite = mapDataInterface
          //         .getSprites()
          //         .find((s) => s.name === spriteName);
          //       if (sprite) {
          //         drawSprite(
          //           sprite,
          //           tileX,
          //           tileY + controlI * 8 * newScale,
          //           newScale,
          //           ctx
          //         );
          //       }
          //       controlI++;
          //     }
          //   }

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
      mapDataInterface.getTilesets(),
      mapDataInterface.getAssets().characters,
      mapDataInterface.getAssets().items
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
