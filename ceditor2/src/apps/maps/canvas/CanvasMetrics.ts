export const MAX_CANVAS_PIXEL_RATIO = 4;

export interface CanvasMetrics {
  logicalWidth: number;
  logicalHeight: number;
  backingWidth: number;
  backingHeight: number;
  pixelRatio: number;
}

export interface CanvasClientRect {
  left: number;
  top: number;
  width: number;
  height: number;
}

export interface LogicalCanvasPoint {
  x: number;
  y: number;
}

interface CanvasSizeTarget {
  width: number;
  height: number;
}

interface CanvasTransformTarget {
  setTransform(
    a: number,
    b: number,
    c: number,
    d: number,
    e: number,
    f: number,
  ): void;
}

export function createCanvasMetrics(): CanvasMetrics {
  return {
    logicalWidth: 0,
    logicalHeight: 0,
    backingWidth: 1,
    backingHeight: 1,
    pixelRatio: 1,
  };
}

/**
 * Derives logical and backing dimensions into a reusable object. Returns true
 * when any metric changed, which lets callers react to CSS resizes cheaply.
 */
export function writeCanvasMetrics(
  out: CanvasMetrics,
  cssWidth: number,
  cssHeight: number,
  devicePixelRatio: number,
  maxPixelRatio = MAX_CANVAS_PIXEL_RATIO,
): boolean {
  const logicalWidth = nonNegativeFinite(cssWidth);
  const logicalHeight = nonNegativeFinite(cssHeight);
  const pixelRatio = boundedPixelRatio(devicePixelRatio, maxPixelRatio);
  const backingWidth = Math.max(1, Math.round(logicalWidth * pixelRatio));
  const backingHeight = Math.max(1, Math.round(logicalHeight * pixelRatio));
  const changed =
    out.logicalWidth !== logicalWidth ||
    out.logicalHeight !== logicalHeight ||
    out.backingWidth !== backingWidth ||
    out.backingHeight !== backingHeight ||
    out.pixelRatio !== pixelRatio;

  out.logicalWidth = logicalWidth;
  out.logicalHeight = logicalHeight;
  out.backingWidth = backingWidth;
  out.backingHeight = backingHeight;
  out.pixelRatio = pixelRatio;
  return changed;
}

/**
 * Updates the backing store only when needed, then restores logical CSS-pixel
 * drawing coordinates. Assigning canvas width/height clears context state.
 */
export function configureCanvasBackingStore(
  canvas: CanvasSizeTarget,
  context: CanvasTransformTarget,
  metrics: Readonly<CanvasMetrics>,
): boolean {
  let resized = false;
  if (canvas.width !== metrics.backingWidth) {
    canvas.width = metrics.backingWidth;
    resized = true;
  }
  if (canvas.height !== metrics.backingHeight) {
    canvas.height = metrics.backingHeight;
    resized = true;
  }
  context.setTransform(metrics.pixelRatio, 0, 0, metrics.pixelRatio, 0, 0);
  return resized;
}

/**
 * Converts a client coordinate to the logical coordinate system used by the
 * viewport. Backing-store pixels and DPR never leak into editor interaction.
 */
export function clientToLogicalCanvasPoint(
  out: LogicalCanvasPoint,
  clientX: number,
  clientY: number,
  rect: Readonly<CanvasClientRect>,
  metrics: Readonly<CanvasMetrics>,
): LogicalCanvasPoint {
  if (
    rect.width <= 0 ||
    rect.height <= 0 ||
    metrics.logicalWidth <= 0 ||
    metrics.logicalHeight <= 0
  ) {
    out.x = 0;
    out.y = 0;
    return out;
  }

  out.x =
    (finite(clientX) - finite(rect.left)) * (metrics.logicalWidth / rect.width);
  out.y =
    (finite(clientY) - finite(rect.top)) *
    (metrics.logicalHeight / rect.height);
  return out;
}

export function boundedPixelRatio(
  devicePixelRatio: number,
  maxPixelRatio = MAX_CANVAS_PIXEL_RATIO,
): number {
  const upperBound = Math.max(1, finite(maxPixelRatio, MAX_CANVAS_PIXEL_RATIO));
  return Math.min(upperBound, Math.max(1, finite(devicePixelRatio, 1)));
}

function nonNegativeFinite(value: number): number {
  return Math.max(0, finite(value, 0));
}

function finite(value: number, fallback = 0): number {
  return Number.isFinite(value) ? value : fallback;
}
