import { PaintActionType, PaintAction } from './paintTools';
import { FloorBrushData } from './renderState';

export interface EditorStateMap {
  selectedTileInd: number;
  hoveredTileIndex: number;
  hoveredTileData: {
    x: number;
    y: number;
    ind: number;
  };
  undoHistory: PaintAction[];
  undoIndex: number;
}

export interface EditorState {
  selectedMapName: string;
  selectedTileIndexInTileset: number;
  selectedTilesetName: string;
  currentPaintAction: PaintActionType;
  fillIndsFloor: number[];
  drawOverlayText: boolean;
  isSelectDragging: boolean;
  selectDragSourceTileIndex: number;
  rectSelectTileIndStart: number;
  rectSelectTileIndEnd: number;
  rectCloneBrushTiles: FloorBrushData[];
  maps: Record<string, EditorStateMap>;
}

const editorState: EditorState = {
  selectedMapName: '',
  selectedTileIndexInTileset: -1,
  selectedTilesetName: '',
  currentPaintAction: 'SELECT' as any,
  fillIndsFloor: [],
  drawOverlayText: false,
  isSelectDragging: false,
  selectDragSourceTileIndex: -1,
  rectSelectTileIndStart: -1,
  rectSelectTileIndEnd: -1,
  rectCloneBrushTiles: [],
  maps: {},
};
export const getEditorState = () => editorState;
export const updateEditorState = (state: Partial<EditorState>) => {
  Object.assign(editorState, { ...getEditorState(), ...state });
  (window as any).reRenderTileEditor();
};
export const updateEditorStateNoReRender = (state: Partial<EditorState>) => {
  Object.assign(editorState, { ...getEditorState(), ...state });
};
export const getEditorStateMap = (mapName: string): EditorStateMap | undefined => {
  const map = getEditorState().maps[mapName];
  return map;
};
export const updateEditorStateMap = (
  mapName: string,
  state: Partial<EditorStateMap>
) => {
  const map = getEditorStateMap(mapName);
  if (map) {
    console.log('updateEditorStateMap', mapName, state);
    Object.assign(map, { ...map, ...state });
    (window as any).reRenderTileEditor();
  }
};
export const updateEditorStateMapNoReRender = (
  mapName: string,
  state: Partial<EditorStateMap>
) => {
  const map = getEditorStateMap(mapName);
  if (map) {
    Object.assign(map, { ...map, ...state });
  }
};

export const findEditorStateMap = (
  editorState: EditorState,
  mapName: string
) => {
  const map = editorState.maps[mapName];
  if (!map) {
    throw new Error(`Map ${mapName} not found when finding editor state`);
  }
  return map;
};
(window as any).editorState = editorState;

export const createEditorStateMap = (mapName: string) => {
  const map: EditorStateMap = {
    selectedTileInd: -1,
    hoveredTileIndex: -1,
    hoveredTileData: { x: -1, y: -1, ind: -1 },
    undoHistory: [],
    undoIndex: 0,
  };
  getEditorState().maps[mapName] = map;
  return map;
};

export const getCurrentSelectedTileId = () => {
  const selectedTileIndex = getEditorState().selectedTileIndexInTileset ?? -1;
  const selectedTilesetName = getEditorState().selectedTilesetName ?? '';

  if (!selectedTilesetName || selectedTileIndex === -1) {
    return 0;
  }

  return selectedTilesetName + '_' + selectedTileIndex;
};

export const setCurrentPaintAction = (paintAction: PaintActionType) => {
  updateEditorState({ currentPaintAction: paintAction });
};

export const getCurrentPaintAction = () => {
  return getEditorState().currentPaintAction;
};
