import { GameEvent } from '../types/assets';
import {
  getNodeBounds,
  screenCoordsToCanvasCoords,
  screenToWorldCoords,
} from './nodeHelpers';

interface TransformState {
  translateX: number;
  translateY: number;
  scale: number;
}

export class EditorStateSE {
  gameEvent: GameEvent | undefined = undefined;
  selectedChildId = '';
  zoneWidth = 750;
  zoneHeight = 750;
  isDragging = false; // Pan dragging
  isDraggingNode = false; // Node dragging
  draggedNodeId: string | null = null;
  hoveredNodeId: string | null = null;
  hoveredCloseButtonNodeId: string | null = null;
  nodeDragOffsetX = 0; // Offset from mouse click to node position
  nodeDragOffsetY = 0;
  lastClickX = 0;
  lastClickY = 0;
  lastTranslateX = 0;
  lastTranslateY = 0;
  translateX = 0;
  translateY = 0;
  scale = 1;
  mouseX = 0;
  mouseY = 0;
  // Store transform state per game event ID
  gameEventTransforms: Map<string, TransformState> = new Map();
  // Multi-select state
  selectedNodeIds: Set<string> = new Set();
  isSelecting: boolean = false; // Rectangle selection mode
  selectionRect: {
    startX: number;
    startY: number;
    endX: number;
    endY: number;
  } | null = null;
  // Store initial positions when starting to drag multiple nodes
  selectedNodesInitialPositions: Map<string, { x: number; y: number }> =
    new Map();
}
const editorStateSE = new EditorStateSE();

export const getEditorState = () => editorStateSE;
export const updateEditorState = (state: Partial<EditorStateSE>) => {
  Object.assign(editorStateSE, { ...getEditorState(), ...state });
  (window as any).reRenderSpecialEventEditor();
};
export const updateEditorStateNoReRender = (state: Partial<EditorStateSE>) => {
  Object.assign(editorStateSE, { ...getEditorState(), ...state });
};
(window as any).editorStateSE = editorStateSE;

export const deleteNode = (nodeId: string) => {
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

export const saveTransformForGameEvent = (gameEventId: string) => {
  const editorState = getEditorState();
  editorState.gameEventTransforms.set(gameEventId, {
    translateX: editorState.translateX,
    translateY: editorState.translateY,
    scale: editorState.scale,
  });
};

export const restoreTransformForGameEvent = (gameEventId: string): boolean => {
  const editorState = getEditorState();
  const savedTransform = editorState.gameEventTransforms.get(gameEventId);
  if (savedTransform) {
    editorState.translateX = savedTransform.translateX;
    editorState.translateY = savedTransform.translateY;
    editorState.scale = savedTransform.scale;
    return true;
  }
  return false;
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

export const resetSelectedNodes = () => {
  const editorState = getEditorState();
  editorState.selectedNodeIds.clear();
  editorState.selectionRect = null;
};

export const showDeleteSelectedNodesConfirm = () => {
  const editorState = getEditorState();
  const nodeCount = editorState.selectedNodeIds.size;
  const message = `Are you sure you want to delete ${nodeCount} node${
    nodeCount > 1 ? 's' : ''
  }?`;
  if (confirm(message)) {
    const gameEvent = editorState.gameEvent;
    if (gameEvent && gameEvent.children) {
      // Delete all selected nodes
      const nodeIdsToDelete = Array.from(editorState.selectedNodeIds);
      nodeIdsToDelete.forEach((nodeId) => {
        const index = gameEvent.children.findIndex(
          (child) => child.id === nodeId
        );
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
};

export const updateSelectionRectangle = (
  xMouse: number,
  yMouse: number,
  canvas: HTMLCanvasElement
) => {
  const editorState = getEditorState();
  const [worldX, worldY] = screenToWorldCoords(
    xMouse,
    yMouse,
    canvas,
    editorState.zoneWidth,
    editorState.zoneHeight
  );
  if (editorState.selectionRect) {
    editorState.selectionRect.endX = worldX;
    editorState.selectionRect.endY = worldY;
  }
};

export const updateDraggedNodePosition = (
  gameEvent: GameEvent,
  xMouse: number,
  yMouse: number,
  canvas: HTMLCanvasElement
) => {
  const editorState = getEditorState();
  const [worldX, worldY] = screenToWorldCoords(
    xMouse,
    yMouse,
    canvas,
    editorState.zoneWidth,
    editorState.zoneHeight
  );

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
};

export const updateDraggedNodePositionsMulti = (
  gameEvent: GameEvent,
  xMouse: number,
  yMouse: number,
  canvas: HTMLCanvasElement
) => {
  const editorState = getEditorState();
  if (!editorState.draggedNodeId) {
    return;
  }
  const [worldX, worldY] = screenToWorldCoords(
    xMouse,
    yMouse,
    canvas,
    editorState.zoneWidth,
    editorState.zoneHeight
  );
  const initialPos = editorState.selectedNodesInitialPositions.get(
    editorState.draggedNodeId
  );
  if (initialPos) {
    const deltaX = worldX - editorState.nodeDragOffsetX - initialPos.x;
    const deltaY = worldY - editorState.nodeDragOffsetY - initialPos.y;

    // Move all selected nodes by the same delta
    editorState.selectedNodeIds.forEach((nodeId) => {
      const node = gameEvent.children.find((c) => c.id === nodeId);
      const nodeInitialPos =
        editorState.selectedNodesInitialPositions.get(nodeId);
      if (node && nodeInitialPos) {
        node.x = nodeInitialPos.x + deltaX;
        node.y = nodeInitialPos.y + deltaY;
      }
    });
  }
};

export const zoomPanzoom = (
  mouseX: number,
  mouseY: number,
  wheelDelta: number,
  canvas: HTMLCanvasElement
) => {
  const editorState = getEditorState();
  const [focalX, focalY] = screenCoordsToCanvasCoords(mouseX, mouseY, canvas);

  let nextScale = editorState.scale;
  const scaleStep = 0.5;

  if (wheelDelta > 0) {
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
};
