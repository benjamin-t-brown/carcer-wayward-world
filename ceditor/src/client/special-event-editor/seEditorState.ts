import { GameEvent } from '../types/assets';

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
