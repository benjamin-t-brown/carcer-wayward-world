import type { SpriteDefinition } from '../../../core/media/index.js';
import { ImageCache } from './ImageCache.js';
import { type TileBounds, Viewport } from './Viewport.js';

/**
 * Narrow read interface for the future editable MapDocument. Implementations
 * should return stable sprite metadata and must not allocate per call.
 */
export interface RenderMapDocument {
  readonly width: number;
  readonly height: number;
  readonly tileWidth: number;
  readonly tileHeight: number;
  /** `null` is an intentionally blank tile; `undefined` is unresolved. */
  spriteAt(
    layer: number,
    tileIndex: number,
  ): SpriteDefinition | null | undefined;
}

export interface DrawMapOptions {
  document: RenderMapDocument;
  layer: number;
  viewport: Viewport;
  originX?: number;
  originY?: number;
  opacity?: number;
  background?: string;
  /** Extra cells for sprites that extend beyond their logical tile. */
  marginTiles?: number;
}

export interface MapRenderStats {
  mapBlocks: number;
  visitedTiles: number;
  drawnTiles: number;
  blankTiles: number;
  unresolvedSprites: number;
  pendingImages: number;
}

/** Canvas-only renderer. It has no knowledge of panels, tools, or DOM state. */
export class MapRenderer {
  readonly stats: MapRenderStats = {
    mapBlocks: 0,
    visitedTiles: 0,
    drawnTiles: 0,
    blankTiles: 0,
    unresolvedSprites: 0,
    pendingImages: 0,
  };

  private readonly visibleBounds: TileBounds = {
    minX: 0,
    maxX: -1,
    minY: 0,
    maxY: -1,
  };
  private canvasWidth = 0;
  private canvasHeight = 0;

  constructor(readonly images = new ImageCache()) {}

  beginFrame(
    context: CanvasRenderingContext2D,
    canvasWidth: number,
    canvasHeight: number,
    background = '#18181b',
  ): void {
    this.resetStats();
    this.canvasWidth = canvasWidth;
    this.canvasHeight = canvasHeight;
    context.imageSmoothingEnabled = false;
    context.clearRect(0, 0, canvasWidth, canvasHeight);
    context.fillStyle = background;
    context.fillRect(0, 0, canvasWidth, canvasHeight);
  }

  drawMap(context: CanvasRenderingContext2D, options: DrawMapOptions): boolean {
    const document = options.document;
    const viewport = options.viewport;
    const originX = options.originX ?? 0;
    const originY = options.originY ?? 0;

    const visible = viewport.writeVisibleTileBounds(this.visibleBounds, {
      originX,
      originY,
      mapWidth: document.width,
      mapHeight: document.height,
      tileWidth: document.tileWidth,
      tileHeight: document.tileHeight,
      canvasWidth: this.canvasWidth || context.canvas.width,
      canvasHeight: this.canvasHeight || context.canvas.height,
      marginTiles: options.marginTiles,
    });
    if (!visible) {
      return false;
    }

    this.stats.mapBlocks += 1;
    const opacity = clampOpacity(options.opacity ?? 1);
    if (opacity !== 1) {
      context.save();
      context.globalAlpha *= opacity;
    }

    try {
      if (options.background) {
        context.fillStyle = options.background;
        context.fillRect(
          Math.round(viewport.worldToScreenX(originX)),
          Math.round(viewport.worldToScreenY(originY)),
          Math.round(document.width * document.tileWidth * viewport.scale),
          Math.round(document.height * document.tileHeight * viewport.scale),
        );
      }

      const minX = this.visibleBounds.minX;
      const maxX = this.visibleBounds.maxX;
      const minY = this.visibleBounds.minY;
      const maxY = this.visibleBounds.maxY;
      const tileWidth = document.tileWidth;
      const tileHeight = document.tileHeight;

      for (let tileY = minY; tileY <= maxY; tileY += 1) {
        const worldTop = originY + tileY * tileHeight;
        const drawY = Math.round(viewport.worldToScreenY(worldTop));
        const drawBottom = Math.round(
          viewport.worldToScreenY(worldTop + tileHeight),
        );
        const drawHeight = drawBottom - drawY;
        let tileIndex = tileY * document.width + minX;

        for (let tileX = minX; tileX <= maxX; tileX += 1, tileIndex += 1) {
          this.stats.visitedTiles += 1;
          const sprite = document.spriteAt(options.layer, tileIndex);
          if (sprite === null) {
            this.stats.blankTiles += 1;
            continue;
          }
          if (sprite === undefined || !validSprite(sprite)) {
            this.stats.unresolvedSprites += 1;
            continue;
          }

          const image = this.images.getOrLoad(sprite.picturePath);
          if (!image) {
            if (this.images.error(sprite.picturePath)) {
              this.stats.unresolvedSprites += 1;
            } else {
              this.stats.pendingImages += 1;
            }
            continue;
          }

          const spritesWide = Math.floor(image.width / sprite.width);
          const spritesHigh = Math.floor(image.height / sprite.height);
          if (
            spritesWide <= 0 ||
            spritesHigh <= 0 ||
            sprite.index >= spritesWide * spritesHigh
          ) {
            this.stats.unresolvedSprites += 1;
            continue;
          }

          const sourceX = (sprite.index % spritesWide) * sprite.width;
          const sourceY =
            Math.floor(sprite.index / spritesWide) * sprite.height;
          const worldLeft = originX + tileX * tileWidth;
          const drawX = Math.round(viewport.worldToScreenX(worldLeft));
          const drawRight = Math.round(
            viewport.worldToScreenX(worldLeft + tileWidth),
          );

          context.drawImage(
            image.source,
            sourceX,
            sourceY,
            sprite.width,
            sprite.height,
            drawX,
            drawY,
            drawRight - drawX,
            drawHeight,
          );
          this.stats.drawnTiles += 1;
        }
      }
    } finally {
      if (opacity !== 1) {
        context.restore();
      }
    }
    return true;
  }

  private resetStats(): void {
    this.stats.mapBlocks = 0;
    this.stats.visitedTiles = 0;
    this.stats.drawnTiles = 0;
    this.stats.blankTiles = 0;
    this.stats.unresolvedSprites = 0;
    this.stats.pendingImages = 0;
  }
}

function validSprite(sprite: SpriteDefinition): boolean {
  return (
    sprite.picturePath.length > 0 &&
    Number.isInteger(sprite.index) &&
    sprite.index >= 0 &&
    sprite.width > 0 &&
    sprite.height > 0
  );
}

function clampOpacity(opacity: number): number {
  if (!Number.isFinite(opacity)) {
    return 1;
  }
  return Math.min(1, Math.max(0, opacity));
}
