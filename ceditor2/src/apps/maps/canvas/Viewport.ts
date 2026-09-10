export const DEFAULT_MIN_SCALE = 0.5;
export const DEFAULT_MAX_SCALE = 10;

export interface Point {
  x: number;
  y: number;
}

export interface TileBounds {
  minX: number;
  maxX: number;
  minY: number;
  maxY: number;
}

export interface VisibleTileOptions {
  originX: number;
  originY: number;
  mapWidth: number;
  mapHeight: number;
  tileWidth: number;
  tileHeight: number;
  canvasWidth: number;
  canvasHeight: number;
  marginTiles?: number;
}

export interface ViewportOptions {
  x?: number;
  y?: number;
  scale?: number;
  minScale?: number;
  maxScale?: number;
}

/**
 * A small, canvas-independent world-to-screen transform.
 * `x` and `y` are screen-pixel translations; world units are unscaled pixels.
 */
export class Viewport {
  readonly minScale: number;
  readonly maxScale: number;

  private translateX: number;
  private translateY: number;
  private zoomScale: number;

  constructor(options: ViewportOptions = {}) {
    const minScale = positiveFinite(options.minScale, DEFAULT_MIN_SCALE);
    const maxScale = positiveFinite(options.maxScale, DEFAULT_MAX_SCALE);
    if (minScale > maxScale) {
      throw new RangeError('Viewport minScale cannot exceed maxScale');
    }

    this.minScale = minScale;
    this.maxScale = maxScale;
    this.translateX = finite(options.x, 0);
    this.translateY = finite(options.y, 0);
    this.zoomScale = this.clampScale(
      positiveFinite(options.scale, Math.max(1, minScale)),
    );
  }

  get x(): number {
    return this.translateX;
  }

  get y(): number {
    return this.translateY;
  }

  get scale(): number {
    return this.zoomScale;
  }

  setTransform(x: number, y: number, scale: number): void {
    this.translateX = finite(x, this.translateX);
    this.translateY = finite(y, this.translateY);
    this.zoomScale = this.clampScale(positiveFinite(scale, this.zoomScale));
  }

  panBy(screenDx: number, screenDy: number): void {
    this.translateX += finite(screenDx, 0);
    this.translateY += finite(screenDy, 0);
  }

  /** Zoom while keeping the world point beneath the screen anchor fixed. */
  zoomAt(screenX: number, screenY: number, factor: number): number {
    if (!Number.isFinite(factor) || factor <= 0) {
      return this.zoomScale;
    }

    const anchorX = finite(screenX, 0);
    const anchorY = finite(screenY, 0);
    const worldX = this.screenToWorldX(anchorX);
    const worldY = this.screenToWorldY(anchorY);
    const nextScale = this.clampScale(this.zoomScale * factor);

    this.zoomScale = nextScale;
    this.translateX = anchorX - worldX * nextScale;
    this.translateY = anchorY - worldY * nextScale;
    return nextScale;
  }

  centerOn(
    worldX: number,
    worldY: number,
    canvasWidth: number,
    canvasHeight: number,
  ): void {
    this.translateX = finite(canvasWidth, 0) / 2 - worldX * this.zoomScale;
    this.translateY = finite(canvasHeight, 0) / 2 - worldY * this.zoomScale;
  }

  worldToScreenX(worldX: number): number {
    return worldX * this.zoomScale + this.translateX;
  }

  worldToScreenY(worldY: number): number {
    return worldY * this.zoomScale + this.translateY;
  }

  screenToWorldX(screenX: number): number {
    return (screenX - this.translateX) / this.zoomScale;
  }

  screenToWorldY(screenY: number): number {
    return (screenY - this.translateY) / this.zoomScale;
  }

  worldToScreen(worldX: number, worldY: number, out: Point): Point {
    out.x = this.worldToScreenX(worldX);
    out.y = this.worldToScreenY(worldY);
    return out;
  }

  screenToWorld(screenX: number, screenY: number, out: Point): Point {
    out.x = this.screenToWorldX(screenX);
    out.y = this.screenToWorldY(screenY);
    return out;
  }

  /**
   * Writes inclusive visible tile coordinates into `out` and returns false
   * when the map block does not intersect the canvas.
   */
  writeVisibleTileBounds(
    out: TileBounds,
    options: VisibleTileOptions,
  ): boolean {
    const {
      originX,
      originY,
      mapWidth,
      mapHeight,
      tileWidth,
      tileHeight,
      canvasWidth,
      canvasHeight,
      marginTiles = 0,
    } = options;

    if (
      mapWidth <= 0 ||
      mapHeight <= 0 ||
      tileWidth <= 0 ||
      tileHeight <= 0 ||
      canvasWidth <= 0 ||
      canvasHeight <= 0
    ) {
      setEmptyBounds(out);
      return false;
    }

    const margin = Math.max(0, Math.trunc(marginTiles));
    const worldLeft = this.screenToWorldX(0) - originX;
    const worldRight = this.screenToWorldX(canvasWidth) - originX;
    const worldTop = this.screenToWorldY(0) - originY;
    const worldBottom = this.screenToWorldY(canvasHeight) - originY;

    out.minX = Math.max(0, Math.floor(worldLeft / tileWidth) - margin);
    out.maxX = Math.min(
      mapWidth - 1,
      Math.ceil(worldRight / tileWidth) - 1 + margin,
    );
    out.minY = Math.max(0, Math.floor(worldTop / tileHeight) - margin);
    out.maxY = Math.min(
      mapHeight - 1,
      Math.ceil(worldBottom / tileHeight) - 1 + margin,
    );

    if (out.minX > out.maxX || out.minY > out.maxY) {
      setEmptyBounds(out);
      return false;
    }
    return true;
  }

  private clampScale(scale: number): number {
    return Math.min(this.maxScale, Math.max(this.minScale, scale));
  }
}

/** Convert WheelEvent deltas to CSS pixels without needing a WheelEvent. */
export function normalizeWheelDelta(
  deltaY: number,
  deltaMode: number,
  pageHeight: number,
): number {
  if (!Number.isFinite(deltaY)) {
    return 0;
  }
  if (deltaMode === 1) {
    return deltaY * 16;
  }
  if (deltaMode === 2) {
    return deltaY * Math.max(1, finite(pageHeight, 1));
  }
  return deltaY;
}

/** Exponential wheel scaling behaves consistently across small trackpad deltas. */
export function wheelZoomFactor(pixelDeltaY: number): number {
  return Math.exp(-finite(pixelDeltaY, 0) * 0.0015);
}

function finite(value: number | undefined, fallback: number): number {
  return typeof value === 'number' && Number.isFinite(value) ? value : fallback;
}

function positiveFinite(value: number | undefined, fallback: number): number {
  return typeof value === 'number' && Number.isFinite(value) && value > 0
    ? value
    : fallback;
}

function setEmptyBounds(out: TileBounds): void {
  out.minX = 0;
  out.maxX = -1;
  out.minY = 0;
  out.maxY = -1;
}
