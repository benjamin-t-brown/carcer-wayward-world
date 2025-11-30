import {
  GameEvent,
  GameEventChildExec,
  GameEventChildSwitch,
  GameEventChildType,
} from '../types/assets';
import { EditorNode } from './EditorNode';
import {
  getNodeParents,
  screenCoordsToCanvasCoords,
  screenToWorldCoords,
} from './nodeHelpers';
import { EditorNodeExec } from './cmpts/ExecNodeComponent';
import { EditorNodeSwitch } from './cmpts/SwitchNodeComponent';
import { Connector } from './cmpts/Connector';

interface TransformState {
  translateX: number;
  translateY: number;
  scale: number;
}

interface EditorSaveState {
  editorNodes: EditorNode[];
  gameEventTransform: TransformState;
}

export class EditorStateSE {
  // gameEvent: GameEvent | undefined = undefined;
  gameEventId: string | undefined = undefined;
  baseGameEvent: GameEvent | undefined = undefined;
  editorNodes: EditorNode[] = [];
  selectedChildId = '';
  zoneWidth = 3000;
  zoneHeight = 3000;
  isDragging = false; // Pan dragging
  isDraggingNode = false; // Node dragging
  draggedNodeId: string | null = null;
  hoveredNodeId: string | undefined = undefined;
  hoveredCloseButtonNodeId: string | undefined = undefined;
  hoveredExitAnchor: Connector | undefined = undefined;
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
  editorSaveStates: Map<string, EditorSaveState> = new Map();

  // Store transform state per game event ID
  // gameEventTransforms: Map<string, TransformState> = new Map();
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
  linking: {
    isLinking: boolean;
    sourceNodeId: string;
    exitIndex: number;
  } = {
    isLinking: false,
    sourceNodeId: '',
    exitIndex: 0,
  };
  // Copy feedback state
  showCopyFeedback: boolean = false;
  copyFeedbackTimeout: number | null = null;
}
const editorStateSE = new EditorStateSE();

export const getEditorState = () => editorStateSE;
export const updateEditorState = (state: Partial<EditorStateSE>) => {
  Object.assign(editorStateSE, { ...getEditorState(), ...state });
  const gameEvent = editorStateSE.baseGameEvent;
  if (gameEvent) {
    syncGameEventFromEditorState(gameEvent, editorStateSE);
  }
  (window as any).reRenderSpecialEventEditor();
};
export const updateEditorStateNoReRender = (state: Partial<EditorStateSE>) => {
  Object.assign(editorStateSE, { ...getEditorState(), ...state });
  const gameEvent = editorStateSE.baseGameEvent;
  if (gameEvent) {
    syncGameEventFromEditorState(gameEvent, editorStateSE);
  }
};
(window as any).editorStateSE = editorStateSE;

// called when a new game event is selected
export const initEditorStateForGameEvent = (
  gameEvent: GameEvent,
  canvas: HTMLCanvasElement
) => {
  const editorState = getEditorState();
  editorState.selectedNodeIds.clear();
  editorState.selectionRect = null;

  const hasSavedEditorState = restoreEditorStateForGameEvent(gameEvent.id);
  if (!hasSavedEditorState) {
    editorState.editorNodes = createEditorNodesForGameEvent(gameEvent, canvas);

    // No saved transform, center on first node
    const firstNode = gameEvent?.children?.[0];
    if (firstNode) {
      console.log('centering on node', firstNode.id);
      centerPanzoomOnNode(canvas, firstNode.id);
    }
  }
};

export const deleteNode = (nodeId: string, ctx: CanvasRenderingContext2D) => {
  const editorState = getEditorState();
  const node = editorState.editorNodes.find((node) => node.id === nodeId);
  if (node) {
    editorState.editorNodes.splice(editorState.editorNodes.indexOf(node), 1);
    const parents = getNodeParents(nodeId, editorState.editorNodes);
    for (const parent of parents) {
      for (const exit of parent.exits) {
        if (exit.toNodeId === nodeId) {
          exit.toNodeId = '';
        }
      }
      parent.calculateHeight(ctx);
    }
  }
};

export const saveEditorStateForGameEvent = (gameEventId: string) => {
  const editorState = getEditorState();
  editorState.editorSaveStates.set(gameEventId, {
    editorNodes: [...editorState.editorNodes],
    gameEventTransform: {
      translateX: editorState.translateX,
      translateY: editorState.translateY,
      scale: editorState.scale,
    },
  });
};

export const restoreEditorStateForGameEvent = (gameEventId: string) => {
  const editorState = getEditorState();
  const savedState = editorState.editorSaveStates.get(gameEventId);
  if (savedState) {
    editorState.editorNodes = savedState.editorNodes;
    editorState.translateX = savedState.gameEventTransform.translateX;
    editorState.translateY = savedState.gameEventTransform.translateY;
    editorState.scale = savedState.gameEventTransform.scale;
    return true;
  }
  return false;
};

export const createEditorNodesForGameEvent = (
  gameEvent: GameEvent,
  canvas: HTMLCanvasElement
) => {
  const editorState = getEditorState();
  return gameEvent.children
    .map((child) => {
      if (child.eventChildType === GameEventChildType.EXEC) {
        return new EditorNodeExec(child as GameEventChildExec, editorState);
      }
      if (child.eventChildType === GameEventChildType.SWITCH) {
        return new EditorNodeSwitch(child as GameEventChildSwitch, editorState);
      }
      throw new Error(`Unknown child type: ${child.eventChildType}`);
    })
    .map((node) => {
      node.calculateHeight(canvas.getContext('2d')!);
      return node;
    });
};

export const getTransform = (editorState?: EditorStateSE) => {
  const s = editorState || getEditorState();
  return {
    x: s.translateX,
    y: s.translateY,
    scale: s.scale,
  };
};

export const resetPanzoom = (editorState?: EditorStateSE) => {
  const s = editorState || getEditorState();
  s.translateX = 0;
  s.translateY = 0;
  s.scale = 1;
};

export const centerPanzoomOnNode = (
  canvas: HTMLCanvasElement,
  nodeId: string
) => {
  const editorState = getEditorState();

  const editorNode = editorState.editorNodes.find((node) => node.id === nodeId);
  if (!editorNode) {
    return;
  }

  const { width, height } = editorNode.getBounds();

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
    (editorNode.x + width / 2) * scale;
  const translateY =
    canvasHeight / 2 -
    (canvasHeight * scale) / 2 +
    (zoneHeight * scale) / 2 -
    (editorNode.y + height / 2) * scale;

  editorState.translateX = translateX;
  editorState.translateY = translateY;
  editorState.scale = scale;
};

export const resetSelectedNodes = () => {
  const editorState = getEditorState();
  editorState.selectedNodeIds.clear();
  editorState.selectionRect = null;
};

export const showDeleteNodeConfirm = (
  nodeId: string,
  ctx: CanvasRenderingContext2D
) => {
  const editorState = getEditorState();
  const node = editorState.editorNodes.find((node) => node.id === nodeId);
  if (node) {
    const message = `Are you sure you want to delete this node?`;
    if (confirm(message)) {
      deleteNode(nodeId, ctx);
      updateEditorState({});
    }
  }
};

export const showDeleteSelectedNodesConfirm = (
  ctx: CanvasRenderingContext2D
) => {
  const editorState = getEditorState();
  const nodeCount = editorState.selectedNodeIds.size;
  const message = `Are you sure you want to delete ${nodeCount} node${
    nodeCount > 1 ? 's' : ''
  }?`;
  if (confirm(message)) {
    const nodeIdsToDelete = Array.from(editorState.selectedNodeIds);
    nodeIdsToDelete.forEach((nodeId) => {
      deleteNode(nodeId, ctx);
    });
    editorState.selectedNodeIds.clear();
    editorState.selectionRect = null;
    updateEditorState({});
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
  editorState: EditorStateSE,
  xMouse: number,
  yMouse: number,
  canvas: HTMLCanvasElement
) => {
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

  const child = editorState.editorNodes.find(
    (c) => c.id === editorState.draggedNodeId
  );
  if (child) {
    child.x = newX;
    child.y = newY;
  }
};

export const updateDraggedNodePositionsMulti = (
  // gameEvent: GameEvent,
  editorState: EditorStateSE,
  xMouse: number,
  yMouse: number,
  canvas: HTMLCanvasElement
) => {
  // const editorState = getEditorState();
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
      const node = editorState.editorNodes.find((c) => c.id === nodeId);
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

export const enterLinkingMode = (
  editorState: EditorStateSE,
  sourceNodeId: string,
  exitIndex: number = 0
) => {
  editorState.linking.isLinking = true;
  editorState.linking.sourceNodeId = sourceNodeId;
  editorState.linking.exitIndex = exitIndex;
};

export const syncGameEventFromEditorState = (
  gameEvent: GameEvent,
  editorState?: EditorStateSE
) => {
  const s = editorState || getEditorState();
  gameEvent.children = s.editorNodes.map((node) => node.toSENode());
};
