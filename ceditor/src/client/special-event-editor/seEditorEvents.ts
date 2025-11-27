import {
  GameEvent,
  GameEventChildExec,
  GameEventChildType,
} from '../types/assets';
import { getNodeBounds } from './nodeRendering/nodeHelpers';
import { getExecNodeDimensions } from './nodeRendering/renderExecNode';
import {
  deleteNode,
  EditorStateSE,
  getEditorState,
  updateEditorState,
} from './seEditorState';

let isPanZoomInitialized = false;
// Track double-click state
let lastClickTime = 0;
let lastClickNodeId: string | null = null;
const DOUBLE_CLICK_DELAY = 150; // milliseconds

const panZoomEvents: {
  keydown: (ev: KeyboardEvent) => void;
  keyup: (ev: KeyboardEvent) => void;
  mousedown: (ev: MouseEvent) => void;
  mousemove: (ev: MouseEvent) => void;
  mouseup: (ev: MouseEvent) => void;
  contextmenu: (ev: MouseEvent) => void;
  wheel: (ev: WheelEvent) => void;
  dblclick: (ev: MouseEvent) => void;
} = {
  keydown: () => {},
  keyup: () => {},
  mousedown: () => {},
  mousemove: () => {},
  mouseup: () => {},
  contextmenu: () => {},
  wheel: () => {},
  dblclick: () => {},
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
  getEditorFuncs: () => {
    onNodeDoubleClick: (nodeId: string) => void;
  };
}) => {
  const handleKeyDown = (ev: KeyboardEvent) => {
    if (shouldPreventDefault(ev)) {
      ev.preventDefault();
    }
    // Clear selection on ESC
    if (ev.key === 'Escape') {
      const editorState = specialEventEditorInterface.getEditorState();
      editorState.selectedNodeIds.clear();
      editorState.selectionRect = null;
      updateEditorState({});
    }
    // Delete selected nodes on Delete key
    if (ev.key === 'Delete') {
      const editorState = specialEventEditorInterface.getEditorState();
      if (editorState.selectedNodeIds.size > 0) {
        ev.preventDefault();
        const nodeCount = editorState.selectedNodeIds.size;
        console.log('TO DELETE', editorState);
        const message = `Are you sure you want to delete ${nodeCount} node${nodeCount > 1 ? 's' : ''}?`;
        if (confirm(message)) {
          const gameEvent = editorState.gameEvent;
          if (gameEvent && gameEvent.children) {
            // Delete all selected nodes
            const nodeIdsToDelete = Array.from(editorState.selectedNodeIds);
            nodeIdsToDelete.forEach((nodeId) => {
              const index = gameEvent.children.findIndex((child) => child.id === nodeId);
              if (index !== -1) {
                gameEvent.children.splice(index, 1);
              }
            });
            // Clear selection after deletion
            editorState.selectedNodeIds.clear();
            editorState.selectionRect = null;
            updateEditorState({ gameEvent: gameEvent });
          }
        }
      }
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
      
      // Check if Ctrl is held
      if (ev.ctrlKey || ev.metaKey) {
        // First check if clicking on a node - if so, toggle selection
        // Otherwise start rectangle selection
        const nodeClicked = checkLeftMouseClickEvents({
          ev,
          canvas,
          editorState,
          onNodeDoubleClick:
            specialEventEditorInterface.getEditorFuncs().onNodeDoubleClick,
          isCtrlClick: true,
        });
        
        // If we didn't click on a node, start rectangle selection
        if (!nodeClicked) {
          const [worldX, worldY] = screenToWorldCoords(
            ev.clientX,
            ev.clientY,
            canvas,
            editorState.zoneWidth,
            editorState.zoneHeight
          );
          editorState.isSelecting = true;
          editorState.selectionRect = {
            startX: worldX,
            startY: worldY,
            endX: worldX,
            endY: worldY,
          };
          ev.preventDefault();
        }
      } else {
        // Normal click handling
        checkLeftMouseClickEvents({
          ev,
          canvas,
          editorState,
          onNodeDoubleClick:
            specialEventEditorInterface.getEditorFuncs().onNodeDoubleClick,
          isCtrlClick: false,
        });
      }
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

    if (editorState.isSelecting && editorState.selectionRect) {
      // Update selection rectangle
      const canvas = specialEventEditorInterface.getCanvas();
      const [worldX, worldY] = screenToWorldCoords(
        ev.clientX,
        ev.clientY,
        canvas,
        editorState.zoneWidth,
        editorState.zoneHeight
      );
      editorState.selectionRect.endX = worldX;
      editorState.selectionRect.endY = worldY;
      updateEditorState({});
    } else if (editorState.isDraggingNode) {
      // Dragging node(s)
      const canvas = specialEventEditorInterface.getCanvas();
      const gameEvent = editorState.gameEvent;

      if (gameEvent) {
        const [worldX, worldY] = screenToWorldCoords(
          ev.clientX,
          ev.clientY,
          canvas,
          editorState.zoneWidth,
          editorState.zoneHeight
        );

        if (editorState.selectedNodeIds.size > 0 && editorState.draggedNodeId) {
          // Moving multiple selected nodes
          const draggedNode = gameEvent.children.find(
            (c) => c.id === editorState.draggedNodeId
          );
          if (draggedNode) {
            const initialPos = editorState.selectedNodesInitialPositions.get(
              editorState.draggedNodeId
            );
            if (initialPos) {
              const deltaX = worldX - editorState.nodeDragOffsetX - initialPos.x;
              const deltaY = worldY - editorState.nodeDragOffsetY - initialPos.y;

              // Move all selected nodes by the same delta
              editorState.selectedNodeIds.forEach((nodeId) => {
                const node = gameEvent.children.find((c) => c.id === nodeId);
                const nodeInitialPos = editorState.selectedNodesInitialPositions.get(
                  nodeId
                );
                if (node && nodeInitialPos) {
                  node.x = nodeInitialPos.x + deltaX;
                  node.y = nodeInitialPos.y + deltaY;
                }
              });
            }
          }
        } else if (editorState.draggedNodeId) {
          // Moving single node
          const newX = worldX - editorState.nodeDragOffsetX;
          const newY = worldY - editorState.nodeDragOffsetY;

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

    // Detect hover over nodes (only when not dragging and not selecting)
    if (
      !editorState.isDragging &&
      !editorState.isDraggingNode &&
      !editorState.isSelecting &&
      isEventWithCanvasTarget(ev, specialEventEditorInterface.getCanvas())
    ) {
      const canvas = specialEventEditorInterface.getCanvas();
      const editorState = specialEventEditorInterface.getEditorState();
      checkMouseMoveHoverEvents({ ev, canvas, editorState });
    }
  };
  const handleMouseUp = (ev: MouseEvent) => {
    const editorState = specialEventEditorInterface.getEditorState();
    
    if (editorState.isSelecting && editorState.selectionRect) {
      const canvas = specialEventEditorInterface.getCanvas();
      // Finalize selection - find nodes in rectangle
      const gameEvent = editorState.gameEvent;
      if (gameEvent) {
        const rect = editorState.selectionRect;
        const minX = Math.min(rect.startX, rect.endX);
        const maxX = Math.max(rect.startX, rect.endX);
        const minY = Math.min(rect.startY, rect.endY);
        const maxY = Math.max(rect.startY, rect.endY);

        // Check which nodes are in the selection rectangle
        gameEvent.children.forEach((child) => {
          const [nodeWidth, nodeHeight] = getNodeBounds(child);
          const nodeRight = child.x + nodeWidth;
          const nodeBottom = child.y + (child.h || nodeHeight);

          // Check if node overlaps with selection rectangle
          if (
            child.x < maxX &&
            nodeRight > minX &&
            child.y < maxY &&
            nodeBottom > minY
          ) {
            editorState.selectedNodeIds.add(child.id);
          }
        });
      }
      editorState.isSelecting = false;
      editorState.selectionRect = null;
      updateEditorState({});
    } else if (editorState.isDraggingNode) {
      // Stop dragging node(s)
      editorState.isDraggingNode = false;
      editorState.draggedNodeId = null;
      editorState.nodeDragOffsetX = 0;
      editorState.nodeDragOffsetY = 0;
      editorState.selectedNodesInitialPositions.clear();
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
  const handleDoubleClick = (ev: MouseEvent) => {
    if (
      ev.button === 0 &&
      isEventWithCanvasTarget(ev, specialEventEditorInterface.getCanvas())
    ) {
      // Double-click is already handled in checkLeftMouseClickEvents
      // This is just to prevent default browser behavior
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
  window.addEventListener('dblclick', handleDoubleClick);

  isPanZoomInitialized = true;
  panZoomEvents.keydown = handleKeyDown;
  panZoomEvents.keyup = handleKeyUp;
  panZoomEvents.mousedown = handleMouseDown;
  panZoomEvents.mousemove = handleMouseMove;
  panZoomEvents.mouseup = handleMouseUp;
  panZoomEvents.contextmenu = handleContextMenu;
  panZoomEvents.wheel = handleWheel;
  panZoomEvents.dblclick = handleDoubleClick;
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
  window.removeEventListener('dblclick', panZoomEvents.dblclick);
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
        const [nodeWidth] = getNodeBounds(child);

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
  onNodeDoubleClick?: (nodeId: string) => void;
  isCtrlClick?: boolean;
}): boolean => {
  // Returns true if a node was clicked, false otherwise
  const { ev, canvas, editorState, onNodeDoubleClick } = args;
  const gameEvent = editorState.gameEvent;
  if (!gameEvent) {
    return false;
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
        console.log('click node', child.id);
        // Check for double-click
        const currentTime = Date.now();
        if (currentTime - lastClickTime < DOUBLE_CLICK_DELAY) {
          // Double-click detected - open edit modal
          if (onNodeDoubleClick) {
            console.log('double click', child.id);
            onNodeDoubleClick(child.id);
          }
          lastClickTime = 0;
          lastClickNodeId = null;
          ev.preventDefault();
          return true;
        }

        // Update last click info for double-click detection
        lastClickTime = currentTime;
        lastClickNodeId = child.id;

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
            lastClickTime = 0;
            lastClickNodeId = null;
            ev.preventDefault();
            return true; // Indicate we handled a node click
          }
        }

        // Check if this node is selected
        const isNodeSelected = editorState.selectedNodeIds.has(child.id);
        
        // If Ctrl+click, toggle selection without starting drag
        if (args.isCtrlClick) {
          if (isNodeSelected) {
            editorState.selectedNodeIds.delete(child.id);
          } else {
            editorState.selectedNodeIds.add(child.id);
          }
          updateEditorState({});
          ev.preventDefault();
          return true; // Indicate we handled a node click
        }
        
        // Normal click behavior: If clicking on a selected node, drag all selected nodes
        // If clicking on an unselected node, select only this node and drag it
        if (!isNodeSelected && !args.isCtrlClick) {
          editorState.selectedNodeIds.clear();
          // editorState.selectedNodeIds.add(child.id);
        }
        
        // Store initial positions of all selected nodes
        editorState.selectedNodesInitialPositions.clear();
        editorState.selectedNodeIds.forEach((nodeId) => {
          const node = gameEvent.children.find((c) => c.id === nodeId);
          if (node) {
            editorState.selectedNodesInitialPositions.set(nodeId, {
              x: node.x,
              y: node.y,
            });
          }
        });
        
        // Start dragging this node (and all selected nodes)
        editorState.isDraggingNode = true;
        editorState.draggedNodeId = child.id;
        editorState.nodeDragOffsetX = worldX - child.x;
        editorState.nodeDragOffsetY = worldY - child.y;
        editorState.lastClickX = ev.clientX;
        editorState.lastClickY = ev.clientY;
        ev.preventDefault();
        return true; // Indicate we handled a node click
      }
    }
  }

  // Clicked outside any node - reset double-click tracking
  lastClickTime = 0;
  lastClickNodeId = null;
  return false; // No node was clicked
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

export const centerPanzoomOnNode = (
  canvas: HTMLCanvasElement,
  nodeId: string
) => {
  const editorState = getEditorState();
  const gameEvent = editorState.gameEvent;

  if (!gameEvent || !gameEvent.children) {
    return;
  }

  const node = gameEvent.children.find((child) => child.id === nodeId);
  if (!node) {
    return;
  }

  const [nodeWidth, nodeHeight] = getNodeBounds(node);

  // const currentZoomLevel = editorState.scale;
  editorState.scale = 1;
  const scale = 1;

  // const scale = editorState.scale;
  const canvasWidth = canvas.width;
  const canvasHeight = canvas.height;
  const zoneWidth = editorState.zoneWidth;
  const zoneHeight = editorState.zoneHeight;

  const translateX =
    canvasWidth / 2 -
    (canvasWidth * scale) / 2 +
    (zoneWidth * scale) / 2 -
    (node.x + nodeWidth / 2) * scale;
  const translateY =
    canvasHeight / 2 -
    (canvasHeight * scale) / 2 +
    (zoneHeight * scale) / 2 -
    (node.y + nodeHeight / 2) * scale;

  editorState.translateX = translateX;
  editorState.translateY = translateY;
  editorState.scale = scale;
};
