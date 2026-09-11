export interface WorldRect {
  minX: number;
  minY: number;
  maxX: number;
  maxY: number;
}

export interface EventViewportTransform {
  translateX: number;
  translateY: number;
  scale: number;
}

export interface EventViewportNode<TConnector = EventViewportConnector> {
  x: number;
  y: number;
  width: number;
  height: number;
  exits: readonly TConnector[];
}

export interface EventViewportConnector {
  fromNodeId: string;
  toNodeId: string;
  segments: readonly { x: number; y: number }[];
}

export interface SpecialEventViewportPlan<TNode, TConnector> {
  visibleWorldRect: WorldRect;
  nodes: TNode[];
  connectors: TConnector[];
}

/**
 * Node anchors extend 14 world pixels and connector strokes extend 5 world
 * pixels beyond their geometry. A little extra margin avoids edge flicker.
 */
export const EVENT_VIEWPORT_OVERSCAN = 16;

export const calculateVisibleEventWorldRect = (args: {
  canvasWidth: number;
  canvasHeight: number;
  zoneWidth: number;
  zoneHeight: number;
  transform: EventViewportTransform;
}): WorldRect => {
  const { canvasWidth, canvasHeight, zoneWidth, zoneHeight, transform } = args;
  const { translateX, translateY, scale } = transform;

  if (!Number.isFinite(scale) || scale <= 0) {
    throw new Error('Event editor viewport scale must be greater than zero.');
  }

  const minX =
    (-translateX - (canvasWidth * scale) / 2 + (zoneWidth * scale) / 2) / scale;
  const minY =
    (-translateY - (canvasHeight * scale) / 2 + (zoneHeight * scale) / 2) /
    scale;

  return {
    minX,
    minY,
    maxX: minX + canvasWidth / scale,
    maxY: minY + canvasHeight / scale,
  };
};

export const expandWorldRect = (
  rect: WorldRect,
  amount: number,
): WorldRect => ({
  minX: rect.minX - amount,
  minY: rect.minY - amount,
  maxX: rect.maxX + amount,
  maxY: rect.maxY + amount,
});

export const worldRectsIntersect = (a: WorldRect, b: WorldRect) =>
  a.minX <= b.maxX && a.maxX >= b.minX && a.minY <= b.maxY && a.maxY >= b.minY;

const pointIsInWorldRect = (point: { x: number; y: number }, rect: WorldRect) =>
  point.x >= rect.minX &&
  point.x <= rect.maxX &&
  point.y >= rect.minY &&
  point.y <= rect.maxY;

const orientation = (
  a: { x: number; y: number },
  b: { x: number; y: number },
  c: { x: number; y: number },
) => (b.x - a.x) * (c.y - a.y) - (b.y - a.y) * (c.x - a.x);

const segmentIntersectsSegment = (
  a: { x: number; y: number },
  b: { x: number; y: number },
  c: { x: number; y: number },
  d: { x: number; y: number },
) => {
  const abC = orientation(a, b, c);
  const abD = orientation(a, b, d);
  const cdA = orientation(c, d, a);
  const cdB = orientation(c, d, b);

  return abC * abD <= 0 && cdA * cdB <= 0;
};

const segmentIntersectsWorldRect = (
  start: { x: number; y: number },
  end: { x: number; y: number },
  rect: WorldRect,
) => {
  if (pointIsInWorldRect(start, rect) || pointIsInWorldRect(end, rect)) {
    return true;
  }

  if (
    Math.max(start.x, end.x) < rect.minX ||
    Math.min(start.x, end.x) > rect.maxX ||
    Math.max(start.y, end.y) < rect.minY ||
    Math.min(start.y, end.y) > rect.maxY
  ) {
    return false;
  }

  const topLeft = { x: rect.minX, y: rect.minY };
  const topRight = { x: rect.maxX, y: rect.minY };
  const bottomRight = { x: rect.maxX, y: rect.maxY };
  const bottomLeft = { x: rect.minX, y: rect.maxY };

  return (
    segmentIntersectsSegment(start, end, topLeft, topRight) ||
    segmentIntersectsSegment(start, end, topRight, bottomRight) ||
    segmentIntersectsSegment(start, end, bottomRight, bottomLeft) ||
    segmentIntersectsSegment(start, end, bottomLeft, topLeft)
  );
};

export const connectorIntersectsWorldRect = (
  connector: EventViewportConnector,
  rect: WorldRect,
) => {
  if (!connector.fromNodeId || !connector.toNodeId) {
    return false;
  }

  for (let index = 0; index < connector.segments.length - 1; index++) {
    if (
      segmentIntersectsWorldRect(
        connector.segments[index],
        connector.segments[index + 1],
        rect,
      )
    ) {
      return true;
    }
  }

  return false;
};

export const planSpecialEventViewport = <
  TConnector extends EventViewportConnector,
  TNode extends EventViewportNode<TConnector>,
>(args: {
  canvasWidth: number;
  canvasHeight: number;
  zoneWidth: number;
  zoneHeight: number;
  transform: EventViewportTransform;
  nodes: readonly TNode[];
  overscan?: number;
}): SpecialEventViewportPlan<TNode, TConnector> => {
  const visibleWorldRect = calculateVisibleEventWorldRect(args);
  const cullingRect = expandWorldRect(
    visibleWorldRect,
    args.overscan ?? EVENT_VIEWPORT_OVERSCAN,
  );
  const nodes: TNode[] = [];
  const connectors: TConnector[] = [];

  for (const node of args.nodes) {
    if (
      worldRectsIntersect(cullingRect, {
        minX: node.x,
        minY: node.y,
        maxX: node.x + node.width,
        maxY: node.y + node.height,
      })
    ) {
      nodes.push(node);
    }

    for (const connector of node.exits) {
      if (connectorIntersectsWorldRect(connector, cullingRect)) {
        connectors.push(connector);
      }
    }
  }

  return { visibleWorldRect, nodes, connectors };
};
