import {
  graphicsEqual,
  type CellPatchCommand,
  type GraphicCell,
  type GraphicCellAccess,
  type TileGraphic,
} from '../history/cellPatches.js';
import { PaintGesture } from './paintGesture.js';

export interface WorldTilePoint {
  readonly x: number;
  readonly y: number;
}

/** Resolves continuous workspace tile coordinates to backing map cells. */
export type WorldCellResolver = (
  point: WorldTilePoint,
  layer: number,
) => GraphicCell | undefined;

export type GraphicProvider = (cell: GraphicCell) => TileGraphic | undefined;

export type RegionMatcher = (
  cell: GraphicCell,
  graphic: TileGraphic,
  startCell: GraphicCell,
  startGraphic: TileGraphic,
) => boolean;

export interface GraphicBrushCell {
  readonly offsetX: number;
  readonly offsetY: number;
  readonly graphic: TileGraphic;
}

export interface GraphicBrush {
  readonly width: number;
  readonly height: number;
  readonly cells: readonly GraphicBrushCell[];
}

export function paintRectangle(
  access: GraphicCellAccess,
  resolve: WorldCellResolver,
  layer: number,
  start: WorldTilePoint,
  end: WorldTilePoint,
  graphic: TileGraphic | GraphicProvider,
): CellPatchCommand {
  const gesture = new PaintGesture(access, [0, 0], 'Rectangle');
  forEachRectangle(start, end, (point) => {
    const cell = resolve(point, layer);
    if (!cell) return;
    const next = typeof graphic === 'function' ? graphic(cell) : graphic;
    if (next) gesture.visit(cell, next);
  });
  return gesture.finish();
}

export function captureGraphicBrush(
  access: GraphicCellAccess,
  resolve: WorldCellResolver,
  layer: number,
  start: WorldTilePoint,
  end: WorldTilePoint,
): GraphicBrush {
  const minX = Math.min(start.x, end.x);
  const minY = Math.min(start.y, end.y);
  const cells: GraphicBrushCell[] = [];
  forEachRectangle(start, end, (point) => {
    const cell = resolve(point, layer);
    if (!cell) return;
    cells.push({
      offsetX: point.x - minX,
      offsetY: point.y - minY,
      graphic: access.getGraphic(cell),
    });
  });
  return {
    width: Math.abs(end.x - start.x) + 1,
    height: Math.abs(end.y - start.y) + 1,
    cells,
  };
}

export function stampGraphicBrush(
  access: GraphicCellAccess,
  resolve: WorldCellResolver,
  layer: number,
  origin: WorldTilePoint,
  brush: GraphicBrush,
): CellPatchCommand {
  const gesture = new PaintGesture(access, [0, 0], 'Brush');
  for (const brushCell of brush.cells) {
    const cell = resolve(
      { x: origin.x + brushCell.offsetX, y: origin.y + brushCell.offsetY },
      layer,
    );
    if (cell) gesture.visit(cell, brushCell.graphic);
  }
  return gesture.finish();
}

/** Four-way fill over the continuous workspace, bounded by resolvable cells. */
export function floodFill(
  access: GraphicCellAccess,
  resolve: WorldCellResolver,
  layer: number,
  start: WorldTilePoint,
  replacement: TileGraphic | GraphicProvider,
  maxCells = 1_000_000,
  matches: RegionMatcher = (_cell, graphic, _startCell, startGraphic) =>
    graphicsEqual(graphic, startGraphic),
): CellPatchCommand {
  const startCell = resolve(start, layer);
  const gesture = new PaintGesture(access, [0, 0], 'Fill');
  if (!startCell) return gesture.finish();
  const target = access.getGraphic(startCell);
  const startReplacement =
    typeof replacement === 'function' ? replacement(startCell) : replacement;
  if (!startReplacement || graphicsEqual(target, startReplacement)) {
    return gesture.finish();
  }
  if (!Number.isSafeInteger(maxCells) || maxCells < 1) {
    throw new RangeError('maxCells must be a positive safe integer');
  }

  const queue: WorldTilePoint[] = [{ x: start.x, y: start.y }];
  const visited = new Set<string>();
  for (let cursor = 0; cursor < queue.length; cursor += 1) {
    const point = queue[cursor]!;
    const key = `${point.x},${point.y}`;
    if (visited.has(key)) continue;
    if (visited.size >= maxCells) {
      gesture.cancel();
      throw new RangeError(`Fill exceeded the ${maxCells}-cell safety limit`);
    }
    visited.add(key);
    const cell = resolve(point, layer);
    if (!cell) continue;
    const current = access.getGraphic(cell);
    if (!matches(cell, current, startCell, target)) continue;
    const next =
      typeof replacement === 'function' ? replacement(cell) : replacement;
    if (!next) continue;
    gesture.visit(cell, next);
    queue.push(
      { x: point.x - 1, y: point.y },
      { x: point.x + 1, y: point.y },
      { x: point.x, y: point.y - 1 },
      { x: point.x, y: point.y + 1 },
    );
  }
  return gesture.finish();
}

function forEachRectangle(
  start: WorldTilePoint,
  end: WorldTilePoint,
  visit: (point: WorldTilePoint) => void,
): void {
  assertPoint(start);
  assertPoint(end);
  const minX = Math.min(start.x, end.x);
  const maxX = Math.max(start.x, end.x);
  const minY = Math.min(start.y, end.y);
  const maxY = Math.max(start.y, end.y);
  for (let y = minY; y <= maxY; y += 1) {
    for (let x = minX; x <= maxX; x += 1) visit({ x, y });
  }
}

function assertPoint(point: WorldTilePoint): void {
  if (!Number.isSafeInteger(point.x) || !Number.isSafeInteger(point.y)) {
    throw new RangeError('World tile coordinates must be safe integers');
  }
}
