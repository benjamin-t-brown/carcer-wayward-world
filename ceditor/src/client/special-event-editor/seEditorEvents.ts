import {
  getExitAnchorFromWorldCoords,
  getConnectorFromLineAtPosition,
  getNodeFromWorldCoords,
  screenToWorldCoords,
} from './nodeHelpers';
import {
  centerPanzoomOnNode,
  copySelectedNodes,
  EditorStateSE,
  enterLinkingMode,
  notifyStateUpdated,
  pasteNodes,
  resetSelectedNodes,
  showDeleteNodeConfirm,
  showDeleteSelectedNodesConfirm,
  updateDraggedNodePosition,
  updateDraggedNodePositionsMulti,
  updateEditorState,
  updateSelectionRectangle,
  zoomPanzoom,
} from './seEditorState';
import { SpecialEventEditorController } from './SpecialEventEditorController';

const DOUBLE_CLICK_DELAY = 300; // milliseconds
const WHEEL_THROTTLE_DELAY = 125; // milliseconds
const LINKING_CANCEL_DRAG_THRESHOLD_SQ = 5 * 5; // pixels squared

const isEventWithCanvasTarget = (
  ev: MouseEvent,
  panzoomCanvas: HTMLCanvasElement | undefined,
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

export const initPanzoom = (
  controller: SpecialEventEditorController,
  specialEventEditorInterface: {
    getCanvas: () => HTMLCanvasElement;
    getEditorFuncs: () => {
      onNodeDoubleClick: (nodeId: string) => void;
      onContextMenu?: (event: MouseEvent) => void;
    };
  },
) => {
  const handleKeyDown = (ev: KeyboardEvent) => {
    if (shouldPreventDefault(ev)) {
      ev.preventDefault();
    }
    // if a modal window is open, return early
    if (controller.hasOpenModal()) {
      return;
    }

    // Clear selection on ESC
    if (ev.key === 'Escape') {
      const editorState = controller.getState();
      // Exit linking mode if active
      if (editorState.linking.isLinking) {
        editorState.linking.isLinking = false;
        editorState.linking.sourceNodeId = '';
        editorState.linking.exitIndex = 0;
        controller.linkingEmptyClickPending = false;
      }
      resetSelectedNodes(controller);
      updateEditorState(controller, {}, false);
    }
    // Delete selected nodes on Delete key
    if (ev.key === 'Delete') {
      const editorState = controller.getState();
      if (editorState.selectedNodeIds.size > 0) {
        ev.preventDefault();
        showDeleteSelectedNodesConfirm(
          controller,
          specialEventEditorInterface.getCanvas().getContext('2d')!,
        );
      }
    }
    // Copy selected nodes on Ctrl+C
    if ((ev.ctrlKey || ev.metaKey) && ev.key === 'c') {
      const editorState = controller.getState();
      if (editorState.selectedNodeIds.size > 0) {
        ev.preventDefault();
        copySelectedNodes(controller, specialEventEditorInterface.getCanvas());
      }
    }
    // Paste nodes on Ctrl+V
    if ((ev.ctrlKey || ev.metaKey) && ev.key === 'v') {
      const editorState = controller.getState();
      if (editorState.copiedNodes && editorState.copiedNodes.length > 0) {
        ev.preventDefault();
        pasteNodes(controller, specialEventEditorInterface.getCanvas());
      }
    }
  };
  const handleKeyUp = (ev: KeyboardEvent) => {};
  const handleMouseDown = (ev: MouseEvent) => {
    const editorState = controller.getState();
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
      const editorState = controller.getState();

      // Check if Ctrl is held
      if (ev.ctrlKey || ev.metaKey) {
        // First check if clicking on a node - if so, toggle selection
        // Otherwise start rectangle selection
        const nodeClicked = checkLeftMouseClickEvents({
          controller,
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
            editorState.zoneHeight,
            editorState,
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
          controller,
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
  };
  const handleMouseMove = (ev: MouseEvent) => {
    const editorState = controller.getState();
    editorState.mouseX = ev.clientX;
    editorState.mouseY = ev.clientY;

    if (editorState.isSelecting && editorState.selectionRect) {
      // Update selection rectangle
      updateSelectionRectangle(
        controller,
        ev.clientX,
        ev.clientY,
        specialEventEditorInterface.getCanvas(),
      );
      updateEditorState(controller, {}, false);
    } else if (editorState.isDraggingNode) {
      // Dragging node(s)
      // const gameEvent = editorState.gameEvent;
      const draggedNode = editorState.editorNodes.find(
        (node) => node.id === editorState.draggedNodeId,
      );

      if (draggedNode) {
        if (editorState.selectedNodeIds.size > 0 && editorState.draggedNodeId) {
          // Moving multiple selected nodes
          updateDraggedNodePositionsMulti(
            editorState,
            ev.clientX,
            ev.clientY,
            specialEventEditorInterface.getCanvas(),
          );
        } else if (editorState.draggedNodeId) {
          // Moving single node
          updateDraggedNodePosition(
            editorState,
            ev.clientX,
            ev.clientY,
            specialEventEditorInterface.getCanvas(),
          );
        }
      }
    } else if (editorState.isDragging) {
      // Pan dragging
      if (controller.linkingEmptyClickPending) {
        const dx = ev.clientX - editorState.lastClickX;
        const dy = ev.clientY - editorState.lastClickY;
        if (dx * dx + dy * dy > LINKING_CANCEL_DRAG_THRESHOLD_SQ) {
          controller.linkingEmptyClickPending = false;
        }
      }
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
      const editorState = controller.getState();
      checkMouseMoveHoverEvents({ ev, canvas, editorState });
    }
  };
  const handleMouseUp = (ev: MouseEvent) => {
    const editorState = controller.getState();

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
      updateEditorState(controller, {}, false);
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

      if (
        controller.linkingEmptyClickPending &&
        editorState.linking.isLinking
      ) {
        editorState.linking.isLinking = false;
        editorState.linking.sourceNodeId = '';
        editorState.linking.exitIndex = 0;
        updateEditorState(controller, {});
      }
      controller.linkingEmptyClickPending = false;
    }
  };
  const handleContextMenu = (ev: MouseEvent) => {
    if (isEventWithCanvasTarget(ev, specialEventEditorInterface.getCanvas())) {
      ev.preventDefault();
      specialEventEditorInterface.getEditorFuncs().onContextMenu?.(ev);
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
    const currentTime = Date.now();
    // Throttle: only process if at least 50ms have passed since last wheel event
    if (currentTime - controller.lastWheelTime < WHEEL_THROTTLE_DELAY) {
      return;
    }
    controller.lastWheelTime = currentTime;

    const mouseX = ev.clientX;
    const mouseY = ev.clientY;
    const wheelDelta = ev.deltaY;
    if (isEventWithCanvasTarget(ev, specialEventEditorInterface.getCanvas())) {
      zoomPanzoom(
        controller,
        mouseX,
        mouseY,
        wheelDelta,
        specialEventEditorInterface.getCanvas(),
      );
    }
  };
  controller.attach({
    canvas: specialEventEditorInterface.getCanvas(),
    handlers: {
      keydown: handleKeyDown,
      keyup: handleKeyUp,
      pointerdown: handleMouseDown,
      pointermove: handleMouseMove,
      pointerup: handleMouseUp,
      pointercancel: () => controller.abortInteraction(),
      contextmenu: handleContextMenu,
      wheel: handleWheel,
      dblclick: handleDoubleClick,
    },
  });
};

export const unInitPanzoom = (controller: SpecialEventEditorController) =>
  controller.detach();

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
    editorState.zoneHeight,
    editorState,
  );

  // let hoveredExitAnchor: Connector | undefined = undefined;
  const hoveredExitAnchor = getExitAnchorFromWorldCoords(
    worldX,
    worldY,
    editorState.editorNodes,
  );
  if (hoveredExitAnchor) {
    editorState.hoveredExitAnchor = hoveredExitAnchor;
  } else {
    editorState.hoveredExitAnchor = undefined;
  }

  const hoveredNode = getNodeFromWorldCoords(
    worldX,
    worldY,
    editorState.editorNodes,
  );
  if (hoveredNode) {
    editorState.hoveredNodeId = hoveredNode.id;
    editorState.hoveredCloseButtonNodeId = undefined;
    if (hoveredNode.isPointInCloseButtonBounds(worldX, worldY)) {
      editorState.hoveredCloseButtonNodeId = hoveredNode.id;
    }
  } else {
    editorState.hoveredNodeId = undefined;
    editorState.hoveredCloseButtonNodeId = undefined;
  }
};

const checkLeftMouseClickEvents = (args: {
  controller: SpecialEventEditorController;
  ev: MouseEvent;
  canvas: HTMLCanvasElement;
  editorState: EditorStateSE;
  onNodeDoubleClick?: (nodeId: string) => void;
  isCtrlClick?: boolean;
}): boolean => {
  // Returns true if a node or line was clicked, false otherwise
  const { controller, ev, canvas, editorState, onNodeDoubleClick } = args;

  const [worldX, worldY] = screenToWorldCoords(
    ev.clientX,
    ev.clientY,
    canvas,
    editorState.zoneWidth,
    editorState.zoneHeight,
    editorState,
  );

  const clickedExitAnchor = getExitAnchorFromWorldCoords(
    worldX,
    worldY,
    editorState.editorNodes,
  );

  if (clickedExitAnchor) {
    enterLinkingMode(
      editorState,
      clickedExitAnchor.fromNodeId,
      clickedExitAnchor.exitIndex,
    );
    return true;
  }

  const clickedNode = getNodeFromWorldCoords(
    worldX,
    worldY,
    editorState.editorNodes,
  );

  const clickedExitAnchorLine = getConnectorFromLineAtPosition(
    worldX,
    worldY,
    editorState.editorNodes,
  );

  if (!clickedNode && clickedExitAnchorLine && clickedExitAnchorLine.toNodeId) {
    console.log('clicked anchor line');
    centerPanzoomOnNode(
      controller,
      canvas,
      clickedExitAnchorLine.toNodeId,
      true,
    );
    return true;
  }

  if (clickedNode) {
    console.log('click node', clickedNode.id);
    // Handle linking mode
    if (editorState.linking.isLinking && editorState.linking.sourceNodeId) {
      if (clickedNode.disableEntrance) {
        return false;
      }

      // Check if clicking on a node
      const parentNode = editorState.editorNodes.find(
        (node) => node.id === editorState.linking.sourceNodeId,
      );

      if (
        parentNode &&
        clickedNode &&
        clickedNode.id !== editorState.linking.sourceNodeId
      ) {
        parentNode.updateExitLink(
          clickedNode.id,
          editorState.linking.exitIndex,
        );
      }

      // Exit linking mode (whether we linked or not)
      editorState.linking.isLinking = false;
      editorState.linking.sourceNodeId = '';
      editorState.linking.exitIndex = 0;
      controller.linkingEmptyClickPending = false;
      updateEditorState(controller, {});
      ev.preventDefault();
      return true;
    }

    // Check for double-click
    const currentTime = Date.now();
    if (currentTime - controller.lastClickTime < DOUBLE_CLICK_DELAY) {
      // Double-click detected - open edit modal
      if (onNodeDoubleClick) {
        console.log('double click', clickedNode.id);
        onNodeDoubleClick(clickedNode.id);
      }
      controller.lastClickTime = 0;
      controller.lastClickNodeId = null;
      ev.preventDefault();
      return true;
    }
    controller.lastClickTime = currentTime;
    controller.lastClickNodeId = clickedNode.id;

    const isCloseButtonClicked = clickedNode.isPointInCloseButtonBounds(
      worldX,
      worldY,
    );
    if (isCloseButtonClicked) {
      console.log('click close button', clickedNode.id);
      const nodeCount = editorState.selectedNodeIds.size;
      if (nodeCount > 0) {
        showDeleteSelectedNodesConfirm(controller, canvas.getContext('2d')!);
      } else {
        showDeleteNodeConfirm(
          controller,
          clickedNode.id,
          canvas.getContext('2d')!,
        );
      }
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
      updateEditorState(controller, {});
      ev.preventDefault();
      return true; // Indicate we handled a node click
    }

    if (!isNodeSelected && !args.isCtrlClick) {
      editorState.selectedNodeIds.clear();
    }

    editorState.selectedNodesInitialPositions.clear();
    for (const nodeId of editorState.selectedNodeIds) {
      const node = editorState.editorNodes.find((c) => c.id === nodeId);
      if (node) {
        editorState.selectedNodesInitialPositions.set(nodeId, {
          x: node.x,
          y: node.y,
        });
      }
    }

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

  if (editorState.linking.isLinking) {
    controller.linkingEmptyClickPending = true;
  }

  // Clicked outside any node - reset double-click tracking
  controller.lastClickTime = 0;
  controller.lastClickNodeId = null;

  return false; // No node was clicked
};

/**
 * Check if right-click is on a connection line and delete it if so
 * Returns true if a line was clicked and deleted, false otherwise
 */
export const checkRightClickLineEvents = (args: {
  controller: SpecialEventEditorController;
  ev: MouseEvent;
  canvas: HTMLCanvasElement;
  editorState: EditorStateSE;
}): boolean => {
  const { controller, ev, canvas, editorState } = args;

  const [worldX, worldY] = screenToWorldCoords(
    ev.clientX,
    ev.clientY,
    canvas,
    editorState.zoneWidth,
    editorState.zoneHeight,
    editorState,
  );

  const clickedExitAnchorLine = getConnectorFromLineAtPosition(
    worldX,
    worldY,
    editorState.editorNodes,
  );

  const clickedNode = getNodeFromWorldCoords(
    worldX,
    worldY,
    editorState.editorNodes,
  );
  if (clickedNode) {
    return false;
  }

  if (clickedExitAnchorLine) {
    // centerPanzoomOnNode(canvas, clickedExitAnchorLine.toNodeId);
    clickedExitAnchorLine.toNodeId = '';
    notifyStateUpdated(controller);
    return true;
  }

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
