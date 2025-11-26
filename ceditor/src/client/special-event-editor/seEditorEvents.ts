import {
  GameEvent,
  GameEventChildExec,
  GameEventChildType,
} from '../types/assets';
import { getExecNodeDimensions } from './nodeRendering/renderExecNode';
import {
  EditorStateSE,
  getEditorState,
  updateEditorState,
} from './seEditorState';

let isPanZoomInitialized = false;
const panZoomEvents: {
  keydown: (ev: KeyboardEvent) => void;
  keyup: (ev: KeyboardEvent) => void;
  mousedown: (ev: MouseEvent) => void;
  mousemove: (ev: MouseEvent) => void;
  mouseup: (ev: MouseEvent) => void;
  contextmenu: (ev: MouseEvent) => void;
  wheel: (ev: WheelEvent) => void;
} = {
  keydown: () => {},
  keyup: () => {},
  mousedown: () => {},
  mousemove: () => {},
  mouseup: () => {},
  contextmenu: () => {},
  wheel: () => {},
};

const isEventWithCanvasTarget = (
  ev: MouseEvent,
  panzoomCanvas: HTMLCanvasElement | undefined
) => {
  const targetId = (ev.target as unknown as HTMLElement)?.id;
  return (
    (Boolean(panzoomCanvas) && ev.target === panzoomCanvas) ||
    targetId === 'special-event-editor-canvas'
  );
};

const isEditorActive = (ev: KeyboardEvent) => {
  return true; // TODO check if any modals are open
};

const shouldPreventDefault = (ev: KeyboardEvent) => {
  return ev.ctrlKey && (ev.key === 's' || ev.key === 'e');
};

export const initPanzoom = (specialEventEditorInterface: {
  getCanvas: () => HTMLCanvasElement;
  getEditorState: () => EditorStateSE;
}) => {
  const handleKeyDown = (ev: KeyboardEvent) => {
    if (shouldPreventDefault(ev)) {
      ev.preventDefault();
    }
  };
  const handleKeyUp = (ev: KeyboardEvent) => {};
  const handleMouseDown = (ev: MouseEvent) => {
    const editorState = specialEventEditorInterface.getEditorState();
    if (
      ev.button === 1 &&
      isEventWithCanvasTarget(ev, specialEventEditorInterface.getCanvas())
    ) {
      editorState.lastClickX = ev.clientX;
      editorState.lastClickY = ev.clientY;
      editorState.lastTranslateX = editorState.translateX;
      editorState.lastTranslateY = editorState.translateY;
      editorState.isDragging = true;
    }
    if (
      ev.button === 0 &&
      isEventWithCanvasTarget(ev, specialEventEditorInterface.getCanvas())
    ) {
      const canvas = specialEventEditorInterface.getCanvas();
      const editorState = specialEventEditorInterface.getEditorState();
      checkLeftMouseClickEvents({ ev, canvas, editorState });
    }
    if (
      ev.button === 2 &&
      isEventWithCanvasTarget(ev, specialEventEditorInterface.getCanvas())
    ) {
    }
  };
  const handleMouseMove = (ev: MouseEvent) => {
    const editorState = specialEventEditorInterface.getEditorState();
    editorState.mouseX = ev.clientX;
    editorState.mouseY = ev.clientY;

    if (editorState.isDraggingNode) {
      // Dragging a node
      if (editorState.draggedNodeId) {
        const canvas = specialEventEditorInterface.getCanvas();
        const editorState = specialEventEditorInterface.getEditorState();
        const gameEvent = editorState.gameEvent;

        if (gameEvent) {
          const [worldX, worldY] = screenToWorldCoords(
            ev.clientX,
            ev.clientY,
            canvas,
            editorState.zoneWidth,
            editorState.zoneHeight
          );

          // Calculate new node position (subtract the offset to maintain relative position)
          const newX = worldX - editorState.nodeDragOffsetX;
          const newY = worldY - editorState.nodeDragOffsetY;

          // Update the node position
          const child = gameEvent.children.find(
            (c) => c.id === editorState.draggedNodeId
          );
          if (child) {
            child.x = newX;
            child.y = newY;
          }
        }
      }
    } else if (editorState.isDragging) {
      // Pan dragging
      editorState.translateX =
        editorState.lastTranslateX + ev.clientX - editorState.lastClickX;
      editorState.translateY =
        editorState.lastTranslateY + ev.clientY - editorState.lastClickY;
    }

    // Detect hover over nodes (only when not dragging)
    if (!editorState.isDragging && !editorState.isDraggingNode) {
      const canvas = specialEventEditorInterface.getCanvas();
      const editorState = specialEventEditorInterface.getEditorState();
      checkMouseMoveHoverEvents({ ev, canvas, editorState });
    }
  };
  const handleMouseUp = (ev: MouseEvent) => {
    const editorState = specialEventEditorInterface.getEditorState();
    if (editorState.isDraggingNode) {
      // Stop dragging node
      editorState.isDraggingNode = false;
      editorState.draggedNodeId = null;
      editorState.nodeDragOffsetX = 0;
      editorState.nodeDragOffsetY = 0;
    } else if (editorState.isDragging) {
      // Stop pan dragging
      editorState.translateX =
        editorState.lastTranslateX + ev.clientX - editorState.lastClickX;
      editorState.translateY =
        editorState.lastTranslateY + ev.clientY - editorState.lastClickY;
      editorState.isDragging = false;
    }
  };
  const handleContextMenu = (ev: MouseEvent) => {
    if (isEventWithCanvasTarget(ev, specialEventEditorInterface.getCanvas())) {
      ev.preventDefault();
    }
  };
  const handleWheel = (ev: WheelEvent) => {
    const editorState = specialEventEditorInterface.getEditorState();
    if (isEventWithCanvasTarget(ev, specialEventEditorInterface.getCanvas())) {
      const [focalX, focalY] = screenCoordsToCanvasCoords(
        ev.clientX,
        ev.clientY,
        specialEventEditorInterface.getCanvas()
      );

      let nextScale = editorState.scale;
      const scaleStep = 0.5;

      if (ev.deltaY > 0) {
        // zoom out
        nextScale -= scaleStep;
      } else {
        // zoom in
        nextScale += scaleStep;
      }

      if (nextScale > 10) {
        nextScale = 10;
      } else if (nextScale < 0.5) {
        nextScale = 0.5;
      }

      const offsetX =
        focalX -
        (nextScale / editorState.scale) * (focalX - editorState.translateX);
      const offsetY =
        focalY -
        (nextScale / editorState.scale) * (focalY - editorState.translateY);

      editorState.translateX = offsetX;
      editorState.translateY = offsetY;
      editorState.scale = nextScale;
    }
  };
  window.addEventListener('keydown', handleKeyDown);
  window.addEventListener('keyup', handleKeyUp);
  window.addEventListener('mousedown', handleMouseDown);
  window.addEventListener('mousemove', handleMouseMove);
  window.addEventListener('mouseup', handleMouseUp);
  window.addEventListener('contextmenu', handleContextMenu);
  window.addEventListener('wheel', handleWheel);

  isPanZoomInitialized = true;
  panZoomEvents.keydown = handleKeyDown;
  panZoomEvents.keyup = handleKeyUp;
  panZoomEvents.mousedown = handleMouseDown;
  panZoomEvents.mousemove = handleMouseMove;
  panZoomEvents.mouseup = handleMouseUp;
  panZoomEvents.contextmenu = handleContextMenu;
  panZoomEvents.wheel = handleWheel;
};

export const unInitPanzoom = () => {
  if (!isPanZoomInitialized || !panZoomEvents) {
    return;
  }
  window.removeEventListener('keydown', panZoomEvents.keydown);
  window.removeEventListener('keyup', panZoomEvents.keyup);
  window.removeEventListener('mousedown', panZoomEvents.mousedown);
  window.removeEventListener('mousemove', panZoomEvents.mousemove);
  window.removeEventListener('mouseup', panZoomEvents.mouseup);
  window.removeEventListener('contextmenu', panZoomEvents.contextmenu);
  window.removeEventListener('wheel', panZoomEvents.wheel);
  isPanZoomInitialized = false;
};

export const getTransform = () => {
  return {
    x: getEditorState().translateX,
    y: getEditorState().translateY,
    scale: getEditorState().scale,
  };
};

export const resetPanzoom = () => {
  getEditorState().translateX = 0;
  getEditorState().translateY = 0;
  getEditorState().scale = 1;
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

export const checkMouseMoveHoverEvents = (args: {
  ev: MouseEvent;
  canvas: HTMLCanvasElement;
  editorState: EditorStateSE;
}) => {
  const { ev, canvas, editorState } = args;
  const gameEvent = editorState.gameEvent;
  if (!gameEvent) {
    return;
  }

  if (gameEvent) {
    const [worldX, worldY] = screenToWorldCoords(
      ev.clientX,
      ev.clientY,
      canvas,
      editorState.zoneWidth,
      editorState.zoneHeight
    );

    // Check which node is hovered
    // Iterate in reverse to check topmost nodes first (nodes drawn later are on top)
    let hoveredNodeId: string | null = null;
    let hoveredCloseButtonNodeId: string | null = null;
    if (gameEvent.children) {
      for (let i = gameEvent.children.length - 1; i >= 0; i--) {
        const child = gameEvent.children[i];
        let nodeWidth = 0;
        if (child.eventChildType === GameEventChildType.EXEC) {
          const [NODE_WIDTH] = getExecNodeDimensions(
            child as GameEventChildExec
          );
          nodeWidth = NODE_WIDTH;
        }

        // Check if point is within node bounds
        if (
          worldX >= child.x &&
          worldX <= child.x + nodeWidth &&
          worldY >= child.y &&
          worldY <= child.y + (child.h || 0)
        ) {
          hoveredNodeId = child.id;

          // Check if hovering over close button
          if (child.eventChildType === GameEventChildType.EXEC) {
            const btnSize = 15;
            const BORDER_WIDTH = 2;
            const closeButtonX =
              child.x + nodeWidth - btnSize - BORDER_WIDTH * 2;
            const closeButtonY = child.y + BORDER_WIDTH * 2;

            if (
              worldX >= closeButtonX &&
              worldX <= closeButtonX + btnSize &&
              worldY >= closeButtonY &&
              worldY <= closeButtonY + btnSize
            ) {
              hoveredCloseButtonNodeId = child.id;
            }
          }

          break; // Use topmost matching node
        }
      }
    }

    // Update hover state if it changed
    if (editorState.hoveredNodeId !== hoveredNodeId) {
      editorState.hoveredNodeId = hoveredNodeId;
    }
    if (editorState.hoveredCloseButtonNodeId !== hoveredCloseButtonNodeId) {
      editorState.hoveredCloseButtonNodeId = hoveredCloseButtonNodeId;
    }
  }
};

const checkLeftMouseClickEvents = (args: {
  ev: MouseEvent;
  canvas: HTMLCanvasElement;
  editorState: EditorStateSE;
}) => {
  const { ev, canvas, editorState } = args;
  const gameEvent = editorState.gameEvent;
  if (!gameEvent) {
    return;
  }

  const [worldX, worldY] = screenToWorldCoords(
    ev.clientX,
    ev.clientY,
    canvas,
    editorState.zoneWidth,
    editorState.zoneHeight
  );

  // Check which node was clicked (check in reverse order for topmost)
  if (gameEvent.children) {
    for (let i = gameEvent.children.length - 1; i >= 0; i--) {
      const child = gameEvent.children[i];
      let nodeWidth = 0;
      if (child.eventChildType === GameEventChildType.EXEC) {
        const [NODE_WIDTH] = getExecNodeDimensions(child as GameEventChildExec);
        nodeWidth = NODE_WIDTH;
      }

      // Check if point is within node bounds
      if (
        worldX >= child.x &&
        worldX <= child.x + nodeWidth &&
        worldY >= child.y &&
        worldY <= child.y + (child.h || 0)
      ) {
        // Check if clicking on close button first
        if (child.eventChildType === GameEventChildType.EXEC) {
          const btnSize = 15;
          const BORDER_WIDTH = 2;
          const nodeX = child.x;
          const nodeY = child.y;

          // Close button position in world coordinates
          const closeButtonX = nodeX + nodeWidth - btnSize - BORDER_WIDTH * 2;
          const closeButtonY = nodeY + BORDER_WIDTH * 2;

          // Check if click is on close button (in world coordinates)
          if (
            worldX >= closeButtonX &&
            worldX <= closeButtonX + btnSize &&
            worldY >= closeButtonY &&
            worldY <= closeButtonY + btnSize
          ) {
            // Clicked on close button - delete the node
            deleteNode(child.id);
            ev.preventDefault();
            return;
          }
        }

        // Start dragging this node
        editorState.isDraggingNode = true;
        editorState.draggedNodeId = child.id;
        editorState.nodeDragOffsetX = worldX - child.x;
        editorState.nodeDragOffsetY = worldY - child.y;
        editorState.lastClickX = ev.clientX;
        editorState.lastClickY = ev.clientY;
        ev.preventDefault();
        return;
      }
    }
  }
};

const deleteNode = (nodeId: string) => {
  const gameEvent = getEditorState().gameEvent;
  if (!gameEvent) {
    return;
  }
  if (confirm('Are you sure you want to delete this node?')) {
    const ind = gameEvent.children?.findIndex((child) => child.id === nodeId);
    if (ind !== undefined) {
      gameEvent.children.splice(ind, 1);
    }
    updateEditorState({ gameEvent: gameEvent });
  }
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
