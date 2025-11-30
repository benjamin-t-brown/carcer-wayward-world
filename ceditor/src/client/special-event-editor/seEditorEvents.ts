import { Connector } from './cmpts/Connector';
import { EditorNode } from './EditorNode';
import { screenToWorldCoords } from './nodeHelpers';
import {
  centerPanzoomOnNode,
  deleteNode,
  EditorStateSE,
  resetSelectedNodes,
  showDeleteSelectedNodesConfirm,
  updateDraggedNodePosition,
  updateDraggedNodePositionsMulti,
  updateEditorState,
  updateSelectionRectangle,
  zoomPanzoom,
} from './seEditorState';

let isPanZoomInitialized = false;
// Track double-click state
let lastClickTime = 0;
let lastClickNodeId: string | null = null;
const DOUBLE_CLICK_DELAY = 300; // milliseconds

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
      // Exit linking mode if active
      if (editorState.isLinking) {
        editorState.isLinking = false;
        editorState.linkingSourceNodeId = null;
        editorState.linkingExitIndex = null;
      }
      resetSelectedNodes();
      updateEditorState({});
    }
    // Delete selected nodes on Delete key
    if (ev.key === 'Delete') {
      const editorState = specialEventEditorInterface.getEditorState();
      if (editorState.selectedNodeIds.size > 0) {
        ev.preventDefault();
        showDeleteSelectedNodesConfirm();
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
        const nodeClicked = checkLeftMouseClickEvents({
          ev,
          canvas,
          editorState,
          onNodeDoubleClick:
            specialEventEditorInterface.getEditorFuncs().onNodeDoubleClick,
          isCtrlClick: false,
        });

        if (!nodeClicked) {
          editorState.lastClickX = ev.clientX;
          editorState.lastClickY = ev.clientY;
          editorState.lastTranslateX = editorState.translateX;
          editorState.lastTranslateY = editorState.translateY;
          editorState.isDragging = true;
        }
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
      updateSelectionRectangle(
        ev.clientX,
        ev.clientY,
        specialEventEditorInterface.getCanvas()
      );
      updateEditorState({});
    } else if (editorState.isDraggingNode) {
      // Dragging node(s)
      // const gameEvent = editorState.gameEvent;
      const draggedNode = editorState.editorNodes.find(
        (node) => node.id === editorState.draggedNodeId
      );

      if (draggedNode) {
        if (editorState.selectedNodeIds.size > 0 && editorState.draggedNodeId) {
          // Moving multiple selected nodes
          updateDraggedNodePositionsMulti(
            editorState,
            ev.clientX,
            ev.clientY,
            specialEventEditorInterface.getCanvas()
          );
        } else if (editorState.draggedNodeId) {
          // Moving single node
          updateDraggedNodePosition(
            editorState,
            ev.clientX,
            ev.clientY,
            specialEventEditorInterface.getCanvas()
          );
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
      // Finalize selection - find nodes in rectangle
      const rect = editorState.selectionRect;
      const minX = Math.min(rect.startX, rect.endX);
      const maxX = Math.max(rect.startX, rect.endX);
      const minY = Math.min(rect.startY, rect.endY);
      const maxY = Math.max(rect.startY, rect.endY);

      const nodesAddedToSelection: string[] = [];

      // Check which nodes are in the selection rectangle
      for (const editorNode of editorState.editorNodes) {
        const { width, height } = editorNode.getBounds();
        const nodeRight = editorNode.x + width;
        const nodeBottom = editorNode.y + height;

        // Check if node overlaps with selection rectangle
        if (
          editorNode.x < maxX &&
          nodeRight > minX &&
          editorNode.y < maxY &&
          nodeBottom > minY
        ) {
          if (!nodesAddedToSelection.includes(editorNode.id)) {
            editorState.selectedNodeIds.add(editorNode.id);
            nodesAddedToSelection.push(editorNode.id);
          }
        }
      }
      if (nodesAddedToSelection.length === 0) {
        editorState.selectedNodeIds.clear();
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
    const mouseX = ev.clientX;
    const mouseY = ev.clientY;
    const wheelDelta = ev.deltaY;
    if (isEventWithCanvasTarget(ev, specialEventEditorInterface.getCanvas())) {
      zoomPanzoom(
        mouseX,
        mouseY,
        wheelDelta,
        specialEventEditorInterface.getCanvas()
      );
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

export const checkMouseMoveHoverEvents = (args: {
  ev: MouseEvent;
  canvas: HTMLCanvasElement;
  editorState: EditorStateSE;
}) => {
  const { ev, canvas, editorState } = args;

  const [worldX, worldY] = screenToWorldCoords(
    ev.clientX,
    ev.clientY,
    canvas,
    editorState.zoneWidth,
    editorState.zoneHeight
  );

  // let hoveredExitAnchor: Connector | undefined = undefined;
  let didFindHoveredExitAnchor = false;
  for (const node of editorState.editorNodes) {
    const anchor = node.getAnchorCollidingWithPoint(worldX, worldY);
    if (anchor) {
      editorState.hoveredExitAnchor = anchor;
      didFindHoveredExitAnchor = true;
      break;
    }
  }
  if (!didFindHoveredExitAnchor) {
    editorState.hoveredExitAnchor = undefined;
  }

  // Check which node is hovered
  // Iterate in reverse to check topmost nodes first (nodes drawn later are on top)
  let didFindHoveredNode = false;
  for (const node of editorState.editorNodes) {
    if (node.isPointInBounds(worldX, worldY)) {
      editorState.hoveredNodeId = node.id;
      didFindHoveredNode = true;
      editorState.hoveredCloseButtonNodeId = undefined;
      if (node.isPointInCloseButtonBounds(worldX, worldY)) {
        editorState.hoveredCloseButtonNodeId = node.id;
        didFindHoveredNode = true;
      }
      break;
    }
  }
  if (!didFindHoveredNode) {
    editorState.hoveredNodeId = undefined;
    editorState.hoveredCloseButtonNodeId = undefined;
  }
};

const checkLeftMouseClickEvents = (args: {
  ev: MouseEvent;
  canvas: HTMLCanvasElement;
  editorState: EditorStateSE;
  onNodeDoubleClick?: (nodeId: string) => void;
  isCtrlClick?: boolean;
}): boolean => {
  // Returns true if a node or line was clicked, false otherwise
  const { ev, canvas, editorState, onNodeDoubleClick } = args;

  const [worldX, worldY] = screenToWorldCoords(
    ev.clientX,
    ev.clientY,
    canvas,
    editorState.zoneWidth,
    editorState.zoneHeight
  );

  let clickedNode: EditorNode | undefined = undefined;
  for (let i = editorState.editorNodes.length - 1; i >= 0; i--) {
    const node = editorState.editorNodes[i];
    if (node.isPointInBounds(worldX, worldY)) {
      clickedNode = node;
      break;
    }
  }

  if (clickedNode) {
    console.log('click node', clickedNode.id);

    // Handle linking mode
    if (editorState.isLinking && editorState.linkingSourceNodeId) {
      // Check if clicking on a node
      const parentNode = editorState.editorNodes.find(
        (node) => node.id === editorState.linkingSourceNodeId
      );

      if (
        parentNode &&
        clickedNode &&
        clickedNode.id !== editorState.linkingSourceNodeId
      ) {
        parentNode.updateExitLink(
          clickedNode.id,
          editorState.linkingExitIndex ?? 0
        );
      }

      // Exit linking mode (whether we linked or not)
      editorState.isLinking = false;
      editorState.linkingSourceNodeId = null;
      editorState.linkingExitIndex = null;
      updateEditorState({});
      ev.preventDefault();
      return true;
    }

    // Check for double-click
    const currentTime = Date.now();
    if (currentTime - lastClickTime < DOUBLE_CLICK_DELAY) {
      // Double-click detected - open edit modal
      if (onNodeDoubleClick) {
        console.log('double click', clickedNode.id);
        onNodeDoubleClick(clickedNode.id);
      }
      lastClickTime = 0;
      lastClickNodeId = null;
      ev.preventDefault();
      return true;
    }
    lastClickTime = currentTime;
    lastClickNodeId = clickedNode.id;

    const isCloseButtonClicked = clickedNode.isPointInCloseButtonBounds(
      worldX,
      worldY
    );
    if (isCloseButtonClicked) {
      // Clicked on close button - delete the node
      deleteNode(clickedNode.id);
      ev.preventDefault();
      return true;
    }

    const isNodeSelected = editorState.selectedNodeIds.has(clickedNode.id);

    // If Ctrl+click, toggle selection without starting drag
    if (args.isCtrlClick) {
      if (isNodeSelected) {
        editorState.selectedNodeIds.delete(clickedNode.id);
      } else {
        editorState.selectedNodeIds.add(clickedNode.id);
      }
      updateEditorState({});
      ev.preventDefault();
      return true; // Indicate we handled a node click
    }

    if (!isNodeSelected && !args.isCtrlClick) {
      editorState.selectedNodeIds.clear();
      // editorState.selectedNodeIds.add(child.id);
    }

    editorState.selectedNodesInitialPositions.clear();
    editorState.selectedNodeIds.forEach((nodeId) => {
      const node = editorState.editorNodes.find((c) => c.id === nodeId);
      if (node) {
        editorState.selectedNodesInitialPositions.set(nodeId, {
          x: node.x,
          y: node.y,
        });
      }
    });

    // Start dragging this node (and all selected nodes)
    editorState.isDraggingNode = true;
    editorState.draggedNodeId = clickedNode.id;
    editorState.nodeDragOffsetX = worldX - clickedNode.x;
    editorState.nodeDragOffsetY = worldY - clickedNode.y;
    editorState.lastClickX = ev.clientX;
    editorState.lastClickY = ev.clientY;
    ev.preventDefault();
    return true; // Indicate we handled a node click
  }

  // if you didn't click a node... the following happens

  // TODO handle clicking on connectors, etc.

  if (editorState.isLinking) {
    editorState.isLinking = false;
    editorState.linkingSourceNodeId = null;
    editorState.linkingExitIndex = null;
    updateEditorState({});
  }

  // Clicked outside any node - reset double-click tracking
  lastClickTime = 0;
  lastClickNodeId = null;

  return false; // No node was clicked
};

/**
 * Check if right-click is on a connection line and delete it if so
 * Returns true if a line was clicked and deleted, false otherwise
 */
export const checkRightClickLineEvents = (args: {
  ev: MouseEvent;
  canvas: HTMLCanvasElement;
  editorState: EditorStateSE;
}): boolean => {
  // const { ev, canvas, editorState } = args;

  // const [worldX, worldY] = screenToWorldCoords(
  //   ev.clientX,
  //   ev.clientY,
  //   canvas,
  //   editorState.zoneWidth,
  //   editorState.zoneHeight
  // );

  // if (gameEvent.children) {
  //   for (const child of gameEvent.children) {
  //     const indexOfClickedLine = getIndexOfClickedLineForChildren(
  //       child,
  //       worldX,
  //       worldY,
  //       gameEvent
  //     );
  //     if (indexOfClickedLine !== -1) {
  //       // Right-clicked on line - delete the connection
  //       setNextNodeForChild(child, 0, '');
  //       updateEditorState({});
  //       return true;
  //     }
  //   }
  // }

  return false;
};
