import {
  TileTerrainBorderTag,
  type CarcerMapTemplate,
  type CarcerMapTileTemplate,
  type TilesetTemplate,
} from '../types/assets';
import type { PaintActionType, PaintAction } from './paintTools';
import type { FloorBrushData } from './renderState';
import type { TerrainPaintTileChange } from './terrainTool';

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
  /** Per-level vertex terrain grids; length (width+1)*(height+1) each. */
  terrainVertexGridsByLevel?: Record<number, TileTerrainBorderTag[]>;
  viewport?: {
    translateX: number;
    translateY: number;
    scale: number;
  };
}

export interface EditorState {
  selectedMapName: string;
  selectedTileIndexInTileset: number;
  selectedTilesetName: string;
  currentPaintAction: PaintActionType;
  fillIndsFloor: number[];
  terrainPaintChanges: TerrainPaintTileChange[];
  drawOverlayText: boolean;
  showGrid: boolean;
  isSelectDragging: boolean;
  selectDragSourceTileIndex: number;
  rectSelectTileIndStart: number;
  rectSelectTileIndEnd: number;
  rectCloneBrushTiles: FloorBrushData[];
  currentLevel: number;
  selectedTerrainTag: TileTerrainBorderTag;
  maps: Record<string, EditorStateMap>;
  tilesets: TilesetTemplate[];
  hoveredGridAdjacentSlot: { offsetX: number; offsetY: number } | null;
  /** When true, every loaded grid partition in view can be edited in place. */
  gridEditEnabled: boolean;
  /**
   * The map a paint stroke is currently writing to. Empty except between
   * mousedown and mouseup of a stroke that landed on a neighbour block; while
   * set it overrides `selectedMapName` for per-map paint state lookups.
   */
  activePaintMapName: string;
  /** Grid map the pointer is currently over ('' = the focused map or none). */
  hoveredGridMapName: string;
  /**
   * Map names in the order their strokes completed, across every block in the
   * grid. Ctrl+Z pops the last and undoes it on that map. Capped like the
   * per-map histories.
   */
  gridUndoOrder: string[];
}

export const createInitialEditorState = (): EditorState => ({
  selectedMapName: '',
  selectedTileIndexInTileset: -1,
  selectedTilesetName: '',
  currentPaintAction: 'SELECT' as any,
  fillIndsFloor: [],
  terrainPaintChanges: [],
  drawOverlayText: false,
  showGrid: true,
  isSelectDragging: false,
  selectDragSourceTileIndex: -1,
  rectSelectTileIndStart: -1,
  rectSelectTileIndEnd: -1,
  rectCloneBrushTiles: [],
  currentLevel: 0,
  selectedTerrainTag: TileTerrainBorderTag.GRASS,
  maps: {},
  tilesets: [],
  hoveredGridAdjacentSlot: null,
  gridEditEnabled: true,
  activePaintMapName: '',
  hoveredGridMapName: '',
  gridUndoOrder: [],
});

export interface EditorStateController {
  getState(): EditorState;
  notify(): void;
  update(state: Partial<EditorState>, notify?: boolean): void;
  getMapState(mapName: string): EditorStateMap | undefined;
  ensureMap(mapName: string): EditorStateMap;
  updateMap(
    mapName: string,
    state: Partial<EditorStateMap>,
    notify?: boolean,
  ): void;
  renameMap(oldName: string, newName: string): void;
}

export const getEditorState = (controller: EditorStateController) =>
  controller.getState();

export const updateEditorState = (
  controller: EditorStateController,
  state: Partial<EditorState>,
) => controller.update(state);

export const updateEditorStateNoReRender = (
  controller: EditorStateController,
  state: Partial<EditorState>,
) => controller.update(state, false);

export const getEditorStateMap = (
  controller: EditorStateController,
  mapName: string,
): EditorStateMap | undefined => {
  return controller.getMapState(mapName);
};
export const updateEditorStateMap = (
  controller: EditorStateController,
  mapName: string,
  state: Partial<EditorStateMap>,
) => controller.updateMap(mapName, state);
export const updateEditorStateMapNoReRender = (
  controller: EditorStateController,
  mapName: string,
  state: Partial<EditorStateMap>,
) => controller.updateMap(mapName, state, false);

export const findEditorStateMap = (
  editorState: EditorState,
  mapName: string,
) => {
  const map = editorState.maps[mapName];
  if (!map) {
    throw new Error(`Map ${mapName} not found when finding editor state`);
  }
  return map;
};

export const createEditorStateMap = (
  controller: EditorStateController,
  mapName: string,
) => controller.ensureMap(mapName);

/** Per-map editor state for `mapName`, creating it if it does not exist yet. */
export const ensureEditorStateMap = (
  controller: EditorStateController,
  mapName: string,
): EditorStateMap => controller.ensureMap(mapName);

/**
 * The map paint state should be read from / written to right now: the stroke's
 * target block while a cross-block stroke is in flight, otherwise the focused
 * map. Everything in paintTools keys per-map state on this.
 */
export const getPaintMapName = (state: EditorState): string =>
  state.activePaintMapName || state.selectedMapName;

/** Record that a stroke completed on `mapName` for grid-wide undo ordering. */
export const pushGridUndo = (
  controller: EditorStateController,
  mapName: string,
): void => {
  if (!mapName) {
    return;
  }
  const order = controller.getState().gridUndoOrder;
  order.push(mapName);
  const MAX = 400;
  if (order.length > MAX) {
    order.splice(0, order.length - MAX);
  }
};

export const renameEditorStateMap = (
  controller: EditorStateController,
  oldName: string,
  newName: string,
) => controller.renameMap(oldName, newName);

/**
 * There is one selected tile across the whole grid. Set it on `mapName` and
 * clear it on every other block, so a right-click / paint on a neighbour block
 * doesn't leave a second, un-clearable selection rect behind.
 */
export const setSoleSelectedTile = (
  controller: EditorStateController,
  mapName: string,
  tileInd: number,
): void => {
  const maps = controller.getState().maps;
  for (const name of Object.keys(maps)) {
    if (name !== mapName && maps[name].selectedTileInd !== -1) {
      maps[name].selectedTileInd = -1;
    }
  }
  controller.ensureMap(mapName).selectedTileInd = tileInd;
  controller.notify();
};

/** Clear the selected-tile rect on every block (Escape). */
export const clearAllSelectedTiles = (
  controller: EditorStateController,
): void => {
  const maps = controller.getState().maps;
  for (const name of Object.keys(maps)) {
    if (maps[name].selectedTileInd !== -1) {
      maps[name].selectedTileInd = -1;
    }
  }
  controller.notify();
};

export const getCurrentSelectedTileId = (controller: EditorStateController) => {
  const selectedTileIndex =
    controller.getState().selectedTileIndexInTileset ?? -1;
  const selectedTilesetName = controller.getState().selectedTilesetName ?? '';

  if (!selectedTilesetName || selectedTileIndex === -1) {
    return 0;
  }

  return selectedTilesetName + '_' + selectedTileIndex;
};

export const setCurrentPaintAction = (
  controller: EditorStateController,
  paintAction: PaintActionType,
) => {
  controller.update({ currentPaintAction: paintAction });
};

export const getCurrentPaintAction = (controller: EditorStateController) => {
  return controller.getState().currentPaintAction;
};
