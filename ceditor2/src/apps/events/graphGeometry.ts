import type { EventNode } from '../../core/domain/events/index.js';

export const EVENT_NODE_WIDTH = 220;
export const EVENT_NODE_MIN_HEIGHT = 64;

export interface EventViewport {
  x: number;
  y: number;
  scale: number;
}

export interface Point {
  readonly x: number;
  readonly y: number;
}

export function nodeHeight(node: Readonly<EventNode>): number {
  return Math.max(EVENT_NODE_MIN_HEIGHT, node.h ?? EVENT_NODE_MIN_HEIGHT);
}

export function screenToWorld(
  point: Point,
  viewport: Readonly<EventViewport>,
): Point {
  return {
    x: (point.x - viewport.x) / viewport.scale,
    y: (point.y - viewport.y) / viewport.scale,
  };
}

export function worldToScreen(
  point: Point,
  viewport: Readonly<EventViewport>,
): Point {
  return {
    x: point.x * viewport.scale + viewport.x,
    y: point.y * viewport.scale + viewport.y,
  };
}

export function hitTestEventNode(
  nodes: readonly Readonly<EventNode>[],
  worldPoint: Point,
): number {
  for (let index = nodes.length - 1; index >= 0; index -= 1) {
    const node = nodes[index]!;
    const x = node.x ?? 0;
    const y = node.y ?? 0;
    if (
      worldPoint.x >= x &&
      worldPoint.x <= x + EVENT_NODE_WIDTH &&
      worldPoint.y >= y &&
      worldPoint.y <= y + nodeHeight(node)
    ) {
      return index;
    }
  }
  return -1;
}

export function indexesInWorldRectangle(
  nodes: readonly Readonly<EventNode>[],
  start: Point,
  end: Point,
): Set<number> {
  const left = Math.min(start.x, end.x);
  const top = Math.min(start.y, end.y);
  const right = Math.max(start.x, end.x);
  const bottom = Math.max(start.y, end.y);
  const selected = new Set<number>();
  nodes.forEach((node, index) => {
    const x = node.x ?? 0;
    const y = node.y ?? 0;
    if (
      x <= right &&
      x + EVENT_NODE_WIDTH >= left &&
      y <= bottom &&
      y + nodeHeight(node) >= top
    ) {
      selected.add(index);
    }
  });
  return selected;
}

export function fitEventViewport(
  nodes: readonly Readonly<EventNode>[],
  width: number,
  height: number,
  padding = 48,
): EventViewport {
  if (nodes.length === 0) return { x: padding, y: padding, scale: 1 };
  const left = Math.min(...nodes.map((node) => node.x ?? 0));
  const top = Math.min(...nodes.map((node) => node.y ?? 0));
  const right = Math.max(
    ...nodes.map((node) => (node.x ?? 0) + EVENT_NODE_WIDTH),
  );
  const bottom = Math.max(
    ...nodes.map((node) => (node.y ?? 0) + nodeHeight(node)),
  );
  const contentWidth = Math.max(1, right - left);
  const contentHeight = Math.max(1, bottom - top);
  const scale = Math.min(
    1.5,
    Math.max(
      0.15,
      Math.min(
        Math.max(1, width - padding * 2) / contentWidth,
        Math.max(1, height - padding * 2) / contentHeight,
      ),
    ),
  );
  return { x: padding - left * scale, y: padding - top * scale, scale };
}
