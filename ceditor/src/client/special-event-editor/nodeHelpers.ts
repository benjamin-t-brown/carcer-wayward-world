import { GameEvent } from '../types/assets';
import { Connector } from './cmpts/Connector';
import { EditorNode } from './EditorNode';
import { getTransform } from './seEditorState';

// export const getAnchorCoordinates = (node: SENode) => {
//   const [nodeWidth] = getNodeDimensions(node);
//   const baseCoordinates = {
//     entrance: {
//       x: node.x,
//       y: node.y + node.h / 2,
//     },
//     exits: [] as Array<{ x: number; y: number }>,
//   };

//   if (node.eventChildType === GameEventChildType.EXEC) {
//     baseCoordinates.exits.push({
//       x: node.x + nodeWidth,
//       y: node.y + node.h / 2,
//     });
//   } else if (node.eventChildType === GameEventChildType.SWITCH) {
//     const switchNode = node as GameEventChildSwitch;
//     const minHeight = getSwitchNodeMinHeight();
//     const lineHeight = getSwitchNodeLineHeight();
//     const totalCaseHeight = (switchNode.cases.length + 1) * lineHeight;
//     node.h = totalCaseHeight + minHeight;
//     for (let i = 0; i < switchNode.cases.length; i++) {
//       baseCoordinates.exits.push({
//         x: node.x + nodeWidth,
//         y: node.y + minHeight + i * lineHeight,
//       });
//     }
//     baseCoordinates.exits.push({
//       x: node.x + nodeWidth,
//       y: node.y + minHeight + switchNode.cases.length * lineHeight,
//     });
//   }

//   return baseCoordinates;
// };

// export const getNodeChildren = (node: SENode) => {
//   if (node.eventChildType === GameEventChildType.EXEC) {
//     return getExecNodeChildren(node as GameEventChildExec);
//   }
//   if (node.eventChildType === GameEventChildType.SWITCH) {
//     return getSwitchNodeChildren(node as GameEventChildSwitch);
//   }
//   return [];
// };

// export const renderNode = (
//   node: SENode,
//   scale: number,
//   ctx: CanvasRenderingContext2D,
//   args: RenderNodeArgs
// ) => {
//   if (node.eventChildType === GameEventChildType.EXEC) {
//     renderExecNode(
//       node as GameEventChildExec,
//       node.x,
//       node.y,
//       scale,
//       ctx,
//       args
//     );
//   } else if (node.eventChildType === GameEventChildType.SWITCH) {
//     renderSwitchNode(
//       node as GameEventChildSwitch,
//       node.x,
//       node.y,
//       scale,
//       ctx,
//       args
//     );
//   }
// };

export const breakTextIntoLines = (
  text: string,
  availableWidth: number,
  fontSize: number,
  fontFamily: string,
  ctx: CanvasRenderingContext2D
): string[] => {
  const lines: string[] = [];

  ctx.font = `${fontSize}px ${fontFamily}`;
  ctx.textAlign = 'left';
  ctx.textBaseline = 'top';

  // Split by explicit newlines first
  const paragraphs = text.split('\n');

  for (const paragraph of paragraphs) {
    if (paragraph.trim() === '') {
      lines.push('');
      continue;
    }

    // Word wrap within the paragraph
    const words = paragraph.split(' ');
    let currentLine = '';

    for (const word of words) {
      const testLine = currentLine ? `${currentLine} ${word}` : word;
      const metrics = ctx.measureText(testLine);

      if (metrics.width > availableWidth && currentLine) {
        // Current line is full, start a new one
        lines.push(currentLine);
        currentLine = word;
      } else {
        currentLine = testLine;
      }
    }

    if (currentLine) {
      lines.push(currentLine);
    }
  }

  return lines;
};

export const calculateHeightFromText = (
  lines: string[],
  fontSize: number,
  fontFamily: string,
  lineHeight: number,
  lineSpacing: number,
  ctx: CanvasRenderingContext2D
): number => {
  if (lines.length === 0) {
    return 0;
  }

  // Set font to measure text
  ctx.font = `${fontSize}px ${fontFamily}`;
  ctx.textAlign = 'left';
  ctx.textBaseline = 'top';

  let lineCount = 0;

  for (let i = 0; i < lines.length; i++) {
    lineCount++;
  }

  const totalHeight =
    lineCount * lineHeight - (lineCount > 0 ? lineSpacing : 0);

  return Math.max(totalHeight, 0);
};

/**
 * Calculate the distance from a point to a line segment
 */
export const distanceToLineSegment = (
  px: number,
  py: number,
  x1: number,
  y1: number,
  x2: number,
  y2: number
): number => {
  const dx = x2 - x1;
  const dy = y2 - y1;
  const lengthSquared = dx * dx + dy * dy;

  if (lengthSquared === 0) {
    const distX = px - x1;
    const distY = py - y1;
    return Math.sqrt(distX * distX + distY * distY);
  }

  const t = Math.max(
    0,
    Math.min(1, ((px - x1) * dx + (py - y1) * dy) / lengthSquared)
  );
  const closestX = x1 + t * dx;
  const closestY = y1 + t * dy;
  const distX = px - closestX;
  const distY = py - closestY;
  return Math.sqrt(distX * distX + distY * distY);
};

interface AccessibleVariable {
  key: string;
  value: string;
  source: string;
}

export const getVarsFromNode = (
  gameEvent: GameEvent,
  gameEvents: GameEvent[]
) => {
  const usedNodes: string[] = [];

  const innerHelper = (gameEvent: GameEvent) => {
    const vars: AccessibleVariable[] = [];
    // infinite loop protection
    if (usedNodes.includes(gameEvent.id)) {
      return [];
    }
    usedNodes.push(gameEvent.id);

    for (const variable of gameEvent.vars) {
      if (variable.importFrom === '') {
        vars.push({
          key: variable.key,
          value: variable.value,
          source: gameEvent.id,
        });
      }
    }

    for (const variable of gameEvent.vars) {
      if (variable.importFrom !== '') {
        const sourceEvent = gameEvents.find(
          (event) => event.id === variable.importFrom
        );
        if (sourceEvent) {
          vars.push(...innerHelper(sourceEvent));
        }
      }
    }
    return vars;
  };

  return innerHelper(gameEvent);
};

/**
 * Convert screen coordinates to world/canvas coordinates accounting for pan/zoom
 */
export const screenToWorldCoords = (
  screenX: number,
  screenY: number,
  canvas: HTMLCanvasElement,
  zoneWidth: number,
  zoneHeight: number
): [number, number] => {
  const { left, top } = canvas.getBoundingClientRect();
  const canvasX = screenX - left;
  const canvasY = screenY - top;

  const { x: translateX, y: translateY, scale } = getTransform();

  // Reverse the transform applied in seLoop.ts
  // The transform in seLoop is:
  // 1. translate(offsetX, offsetY) where offsetX = translateX, offsetY = translateY (since newScale === scale)
  // 2. translate(canvas.width * scale / 2, canvas.height * scale / 2)
  // 3. translate(-zoneWidth * scale / 2, -zoneHeight * scale / 2)

  // Reverse the transforms step by step
  // Start with screen coordinates relative to canvas
  let worldX = canvasX;
  let worldY = canvasY;

  // Reverse step 1: subtract translate offset
  worldX -= translateX;
  worldY -= translateY;

  // Reverse step 2: subtract canvas center translation
  worldX -= (canvas.width * scale) / 2;
  worldY -= (canvas.height * scale) / 2;

  // Reverse step 3: add zone center translation
  worldX += (zoneWidth * scale) / 2;
  worldY += (zoneHeight * scale) / 2;

  // Divide by scale to get world coordinates
  worldX /= scale;
  worldY /= scale;

  return [worldX, worldY];
};

export const screenCoordsToCanvasCoords = (
  x: number,
  y: number,
  panzoomCanvas: HTMLCanvasElement
) => {
  const { left, top } = panzoomCanvas?.getBoundingClientRect() || {
    left: 0,
    top: 0,
    width: 0,
    height: 0,
  };

  const canvasX = x - left;
  const canvasY = y - top;

  return [canvasX, canvasY];
};

export const getNodeFromWorldCoords = (
  worldX: number,
  worldY: number,
  nodes: EditorNode[]
): EditorNode | undefined => {
  let clickedNode: EditorNode | undefined = undefined;
  for (let i = nodes.length - 1; i >= 0; i--) {
    const node = nodes[i];
    if (node.isPointInBounds(worldX, worldY)) {
      clickedNode = node;
      break;
    }
  }
  return clickedNode;
};

export const getExitAnchorFromWorldCoords = (
  worldX: number,
  worldY: number,
  nodes: EditorNode[]
): Connector | undefined => {
  let clickedExitAnchor: Connector | undefined = undefined;
  for (let i = nodes.length - 1; i >= 0; i--) {
    const node = nodes[i];
    const exitAnchor = node.getAnchorCollidingWithPoint(worldX, worldY);
    if (exitAnchor) {
      clickedExitAnchor = exitAnchor;
      break;
    }
  }
  return clickedExitAnchor;
};

export const getExitAnchorFromWorldCoordsLine = (
  worldX: number,
  worldY: number,
  nodes: EditorNode[]
): Connector | undefined => {
  let clickedExitAnchor: Connector | undefined = undefined;
  for (let i = nodes.length - 1; i >= 0; i--) {
    const node = nodes[i];
    const exitAnchor = node.getConnectorLineCollidingWithPoint(worldX, worldY);
    if (exitAnchor) {
      clickedExitAnchor = exitAnchor;
      break;
    }
  }
  return clickedExitAnchor;
};

export const getNodeParents = (
  nodeId: string,
  nodes: EditorNode[]
): EditorNode[] => {
  return nodes.filter((node) =>
    node.exits.some((exit) => exit.toNodeId === nodeId)
  );
};

// export const isPointInNode = (
//   child: SENode,
//   worldX: number,
//   worldY: number
// ) => {
//   const [nodeWidth] = getNodeDimensions(child);

//   // Check if point is within node bounds
//   if (
//     worldX >= child.x &&
//     worldX <= child.x + nodeWidth &&
//     worldY >= child.y &&
//     worldY <= child.y + (child.h || 0)
//   ) {
//     return true;
//   }

//   return false;
// };

// export const isPointInCloseButton = (
//   child: SENode,
//   worldX: number,
//   worldY: number
// ) => {
//   const closeButtonBounds = getCloseButtonBounds(child);
//   return (
//     worldX >= closeButtonBounds.x &&
//     worldX <= closeButtonBounds.x + closeButtonBounds.width &&
//     worldY >= closeButtonBounds.y &&
//     worldY <= closeButtonBounds.y + closeButtonBounds.height
//   );
// };

// export const getIndexOfClickedLineForChildren = (
//   child: SENode,
//   worldX: number,
//   worldY: number,
//   gameEvent: GameEvent
// ) => {
//   const LINE_CLICK_THRESHOLD = 15;
//   const { exits } = getAnchorCoordinates(child);
//   const nextChildren = getNodeChildren(child);
//   for (let i = 0; i < exits.length; i++) {
//     const nextChildId = nextChildren[i];
//     const startX = exits[i].x;
//     const startY = exits[i].y;
//     const nextChild = gameEvent.children.find((c) => c.id === nextChildId);
//     if (nextChild) {
//       const { entrance } = getAnchorCoordinates(nextChild);
//       const endX = entrance.x;
//       const endY = entrance.y;
//       const distance = distanceToLineSegment(
//         worldX,
//         worldY,
//         startX,
//         startY,
//         endX,
//         endY
//       );
//       if (distance <= LINE_CLICK_THRESHOLD) {
//         return i;
//       }
//     }
//   }
//   return -1;
// };

// export const setNextNodeForChild = (
//   child: SENode,
//   index: number,
//   nextNodeId: string
// ) => {
//   if (child.eventChildType === GameEventChildType.EXEC) {
//     const execNode = child as GameEventChildExec;
//     execNode.next = nextNodeId;
//   } else if (child.eventChildType === GameEventChildType.SWITCH) {
//     const switchNode = child as GameEventChildSwitch;
//     if (index < switchNode.cases.length) {
//       // Set next for a specific case
//       switchNode.cases[index].next = nextNodeId;
//     } else if (index === switchNode.cases.length) {
//       // Set next for default case
//       switchNode.defaultNext = nextNodeId;
//     }
//   }
// };

/**
 * Check if a point is on an exit anchor for a switch node
 * Returns the exit index if clicked, -1 otherwise
 */
// export const getIndexOfClickedExitAnchor = (
//   child: SENode,
//   worldX: number,
//   worldY: number
// ): number => {
//   if (child.eventChildType !== GameEventChildType.SWITCH) {
//     return -1;
//   }

//   const { exits } = getAnchorCoordinates(child);
//   const EXIT_ANCHOR_RADIUS = 10; // 10px radius

//   for (let i = 0; i < exits.length; i++) {
//     const exit = exits[i];
//     const dx = worldX - exit.x;
//     const dy = worldY - exit.y;
//     const distance = Math.sqrt(dx * dx + dy * dy);

//     if (distance <= EXIT_ANCHOR_RADIUS) {
//       return i;
//     }
//   }

//   return -1;
// };
