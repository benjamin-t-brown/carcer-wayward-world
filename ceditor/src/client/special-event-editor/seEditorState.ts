import {
  GameEvent,
  GameEventChildChoice,
  GameEventChildEnd,
  GameEventChildExec,
  GameEventChildSwitch,
  GameEventChildComment,
  GameEventChildType,
  SENode,
} from '../types/assets';
import { EditorNode } from './cmpts/EditorNode';
import {
  getNodeParents,
  screenCoordsToCanvasCoords,
  screenToWorldCoords,
} from './nodeHelpers';
import { EditorNodeExec } from './cmpts/ExecNodeComponent';
import { EditorNodeSwitch } from './cmpts/SwitchNodeComponent';
import { Connector } from './cmpts/Connector';
import { EditorNodeChoice } from './cmpts/ChoiceNodeComponent';
import { EditorNodeEnd } from './cmpts/EndNodeComponent';
import { EditorNodeComment } from './cmpts/CommentNodeComponent';
import { randomId } from '../utils/mathUtils';

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
  // baseGameEvent: GameEvent | undefined = undefined;
  editorNodes: EditorNode[] = [];
  selectedChildId = '';
  zoneWidth = 8000;
  zoneHeight = 8000;
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
  // Copy/paste state
  copiedNodes: Array<{
    seNode: SENode;
    offsetX: number;
    offsetY: number;
  }> | null = null;
  shouldValidate: boolean = false;
  runnerErrors: {
    message: string;
    nodeId: string;
  }[] = [];
}
const editorStateSE = new EditorStateSE();

export const getEditorState = () => editorStateSE;
export const updateEditorState = (state: Partial<EditorStateSE>, validate: boolean = true) => {
  Object.assign(editorStateSE, { ...getEditorState(), ...state });
  // const gameEvent = editorStateSE.gameEventId;
  // if (gameEvent) {
  //   syncGameEventFromEditorState(gameEvent, editorStateSE);
  // }
  const renderFunc = (window as any).reRenderSpecialEventEditor;
  if (renderFunc) {
    renderFunc();
  }
  if (validate) {
    notifyStateUpdated();
  }
};
export const updateEditorStateNoReRender = (state: Partial<EditorStateSE>) => {
  Object.assign(editorStateSE, { ...getEditorState(), ...state });
  // const gameEvent = editorStateSE.baseGameEvent;
  // if (gameEvent) {
  //   syncGameEventFromEditorState(gameEvent, editorStateSE);
  // }
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
  console.log('initEditorStateForGameEvent', gameEvent, gameEvent.id);

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
  const ctx = canvas.getContext('2d')!;
  return gameEvent.children.map((child) =>
    seNodeToEditorNode(child, editorState, ctx)
  );
};

export const seNodeToEditorNode = (
  child: SENode,
  editorState: EditorStateSE,
  ctx: CanvasRenderingContext2D
) => {
  if (child.eventChildType === GameEventChildType.EXEC) {
    const node = new EditorNodeExec(child as GameEventChildExec, editorState);
    node.build(ctx);
    return node;
  }
  if (child.eventChildType === GameEventChildType.SWITCH) {
    const switchChild = child as GameEventChildSwitch;
    const node = new EditorNodeSwitch(switchChild, editorState);
    node.buildFromCases(switchChild.cases, ctx);
    return node;
  }
  if (child.eventChildType === GameEventChildType.CHOICE) {
    const choiceChild = child as GameEventChildChoice;
    const node = new EditorNodeChoice(choiceChild, editorState);
    node.buildFromChoices(choiceChild.choices, ctx);
    return node;
  }
  if (child.eventChildType === GameEventChildType.END) {
    const node = new EditorNodeEnd(child as GameEventChildEnd, editorState);
    node.calculateHeight(ctx);
    return node;
  }
  if (child.eventChildType === GameEventChildType.COMMENT) {
    const commentChild = child as GameEventChildComment;
    const node = new EditorNodeComment(commentChild, editorState);
    node.calculateHeight(ctx);
    return node;
  }
  throw new Error(`Unknown child type: ${child.eventChildType}`);
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
  nodeId: string,
  shouldFlash: boolean = false
) => {
  const editorState = getEditorState();

  const editorNode = editorState.editorNodes.find((node) => node.id === nodeId);
  if (!editorNode) {
    return;
  }

  if (shouldFlash) {
    editorNode.startFlash();
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
  if (gameEvent.id !== s.gameEventId) {
    console.error(
      "syncGameEventFromEditorState rejecting sync since ids don't match.",
      'attempted to sync event',
      `"${gameEvent.id}"`,
      'with current editor state id event',
      `"${s.gameEventId}"`
    );
    return;
  }
  gameEvent.children = s.editorNodes.map((node) => node.toSENode());
};

export const copySelectedNodes = (canvas: HTMLCanvasElement) => {
  const editorState = getEditorState();
  if (editorState.selectedNodeIds.size === 0) {
    return;
  }

  // Get all selected nodes
  const selectedNodes = editorState.editorNodes.filter((node) =>
    editorState.selectedNodeIds.has(node.id)
  );

  if (selectedNodes.length === 0) {
    return;
  }

  // Calculate center point of selection
  let minX = Infinity;
  let minY = Infinity;
  let maxX = -Infinity;
  let maxY = -Infinity;

  for (const node of selectedNodes) {
    const { width, height } = node.getBounds();
    minX = Math.min(minX, node.x);
    minY = Math.min(minY, node.y);
    maxX = Math.max(maxX, node.x + width);
    maxY = Math.max(maxY, node.y + height);
  }

  const centerX = (minX + maxX) / 2;
  const centerY = (minY + maxY) / 2;

  // Store nodes with their relative offsets from center
  const copiedNodes = selectedNodes.map((node) => {
    const seNode = node.toSENode();

    return {
      seNode,
      offsetX: node.x - centerX,
      offsetY: node.y - centerY,
    };
  });

  editorState.copiedNodes = copiedNodes;
};

export const pasteNodes = (canvas: HTMLCanvasElement) => {
  const editorState = getEditorState();
  if (!editorState.copiedNodes || editorState.copiedNodes.length === 0) {
    return;
  }

  const ctx = canvas.getContext('2d')!;
  const [worldX, worldY] = screenToWorldCoords(
    editorState.mouseX,
    editorState.mouseY,
    canvas,
    editorState.zoneWidth,
    editorState.zoneHeight
  );

  // Clear current selection
  editorState.selectedNodeIds.clear();

  // Create new nodes from copied data
  const newNodes: EditorNode[] = [];
  const oldIdToNewId = new Map<string, string>();

  for (const copiedNode of editorState.copiedNodes) {
    const seNode = copiedNode.seNode;
    const newId = randomId();
    oldIdToNewId.set(seNode.id, newId);
    const newNode = seNodeToEditorNode(
      {
        ...seNode,
        id: newId,
        x: worldX + copiedNode.offsetX,
        y: worldY + copiedNode.offsetY,
      },
      editorState,
      ctx
    );
    newNodes.push(newNode);
    editorState.selectedNodeIds.add(newId);
  }

  for (const node of newNodes) {
    for (const exit of node.exits) {
      if (exit.toNodeId) {
        exit.toNodeId = oldIdToNewId.get(exit.toNodeId) ?? '';
      }
    }

    if (node.type === GameEventChildType.SWITCH) {
      const switchNode = node as EditorNodeSwitch;
      switchNode.defaultNext = oldIdToNewId.get(switchNode.defaultNext) ?? '';
    }
  }


  // Add all new nodes to editorNodes
  editorState.editorNodes.push(...newNodes);

  updateEditorState({});
};

export const notifyStateUpdated = () => {
  const editorState = getEditorState();
  editorState.shouldValidate = true;
};
