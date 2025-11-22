import {
  PaintActionType,
} from './paintTools';
import {
  drawRect,
  drawSprite,
  getSpriteNameFromTile,
  getSpriteNameFromTileMetadata,
} from './draw';
import {
  CarcerMapTemplate,
  CarcerMapTileTemplate,
  TilesetTemplate,
  CharacterTemplate,
  ItemTemplate,
} from '../types/assets';
import {
  EditorState,
} from './editorState';
import {
  getIndsOfBoundingRect,
  getIsDraggingRight,
  getTransform,
} from './editorEvents';
import { Sprite } from '../utils/assetLoader';

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
    'rgba(0, 255, 0, 0.1)',
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

export const renderToolUi = (
  editorState: EditorState,
  mapData: CarcerMapTemplate,
  ctx: CanvasRenderingContext2D,
  spriteMap: Record<string, Sprite>,
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
    ? spriteMap[
        getSpriteNameFromTileMetadata(
          selectedTileset?.spriteBase ?? '',
          paintTile
        )
      ]
    : undefined;
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
  } else if (currentPaintAction === PaintActionType.ERASE_META) {
    const hoveredTileInd = editorState.hoveredTileIndex;
    if (hoveredTileInd > -1) {
      const tileX = (hoveredTileInd % mapData.width) * tileWidth * scale;
      const tileY =
        Math.floor(hoveredTileInd / mapData.width) * tileHeight * scale;
      // Use a different color to distinguish from regular erase
      drawRect(
        tileX,
        tileY,
        tileWidth * scale,
        tileHeight * scale,
        'rgba(255, 165, 0, 0.44)',
        false,
        ctx
      );
    }
  } else if (
    currentPaintAction === PaintActionType.SELECT ||
    currentPaintAction === PaintActionType.CLONE
  ) {
    // Draw the source tile at the destination when dragging
    if (editorState.isSelectDragging) {
      const sourceTileIndex = editorState.selectDragSourceTileIndex;
      const destTileIndex = editorState.hoveredTileIndex;

      if (
        sourceTileIndex >= 0 &&
        destTileIndex >= 0 &&
        sourceTileIndex !== destTileIndex
      ) {
        ctx.save();
        ctx.globalAlpha = 0.5;
        renderTileAndExtras({
          refTile: mapData.tiles[sourceTileIndex],
          x: editorState.hoveredTileData.x,
          y: editorState.hoveredTileData.y,
          ctx,
          newScale: scale,
          spriteMap,
          tilesets,
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
        const spr = spriteMap[spriteName];
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

export const renderTileAndExtras = (args: {
  refTile: CarcerMapTileTemplate;
  x: number;
  y: number;
  ctx: CanvasRenderingContext2D;
  newScale: number;
  spriteMap: Record<string, Sprite>;
  mapSpriteWidth: number;
  mapSpriteHeight: number;
  tilesets: TilesetTemplate[];
  characters: CharacterTemplate[];
  items: ItemTemplate[];
}) => {
  const {
    refTile,
    x,
    y,
    ctx,
    newScale,
    spriteMap,
    mapSpriteWidth,
    mapSpriteHeight,
    tilesets,
    characters,
    items,
  } = args;
  const controlSprites: string[] = [];
  const tileX = x * mapSpriteWidth * newScale;
  const tileY = y * mapSpriteHeight * newScale;
  const spriteName = getSpriteNameFromTile(refTile);
  const sprite = spriteMap[spriteName];
  if (sprite) {
    drawSprite(sprite, tileX, tileY, newScale, ctx);
  }

  const tileTemplate = tilesets
    .find((t) => t.name === refTile.tilesetName)
    ?.tiles.find((t) => t.id === refTile.tileId);

  for (const characterName of refTile.characters) {
    const characterTemplate = characters.find((c) => c.name === characterName);
    if (characterTemplate) {
      const characterSpriteName = `${characterTemplate.spritesheet}_${characterTemplate.spriteOffset}`;
      const characterSprite = spriteMap[characterSpriteName];
      if (characterSprite) {
        drawSprite(characterSprite, tileX, tileY, newScale, ctx);
      } else {
        console.error(`Character sprite not found: ${characterSpriteName}`);
      }
    }
  }

  const tileIsContainerWithItems =
    (refTile.items.length &&
      tileTemplate?.isContainer &&
      (refTile.tileOverrides?.isContainerOverride === true ||
        refTile.tileOverrides?.isContainerOverride === undefined)) ||
    refTile.tileOverrides?.isContainerOverride;

  if (tileIsContainerWithItems) {
    controlSprites.push('control_1');
  } else {
    for (const itemName of refTile.items) {
      const itemTemplate = items.find((i) => i.name === itemName);
      if (itemTemplate) {
        const itemSpriteName = itemTemplate.icon;
        const itemSprite = spriteMap[itemSpriteName];
        if (itemSprite) {
          const spriteWidth = itemSprite.width;
          const spriteHeight = itemSprite.height;
          const spriteX =
            tileX + ((mapSpriteWidth - spriteWidth) * newScale) / 2;
          const spriteY =
            tileY + ((mapSpriteHeight - spriteHeight) * newScale) / 2;
          drawSprite(itemSprite, spriteX, spriteY, newScale, ctx);
        } else {
          console.error(`Item sprite not found: ${itemSpriteName}`);
        }
      }
    }
  }

  if (refTile.tileOverrides) {
    controlSprites.push('control_2');
  }

  if (refTile.eventTrigger) {
    controlSprites.push('control_0');
  }

  if (refTile.travelTrigger) {
    controlSprites.push('control_3');
  }

  let controlI = 0;
  for (const spriteName of controlSprites) {
    const sprite = spriteMap[spriteName];
    if (sprite) {
      drawSprite(sprite, tileX, tileY + controlI * 8 * newScale, newScale, ctx);
    }
    controlI++;
  }
};
