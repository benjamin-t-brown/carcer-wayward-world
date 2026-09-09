import {
  CarcerMapTemplate,
  CarcerMapTileTemplate,
  TilesetTemplate,
  TileTerrainBorderTag,
} from '../types/assets';
import { PaintActionType, PaintAction } from './paintTools';
import { FloorBrushData } from './renderState';
import { TerrainPaintTileChange } from './terrainTool';

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
  /**
   * Bumped whenever this map's underlying data changes (a committed paint
   * stroke, a structural op, a tile-edit modal write). The materialized-layer
   * cache is keyed on it, so an ordinary React `{ ...map }` spread no longer
   * silently invalidates the cache — only an explicit bump does.
   */
  dataRevision: number;
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
  /**
   * How many grid cells out from the current map the editor paints surrounding
   * maps for. 1 = the eight immediate neighbours; higher shows more context at
   * a rendering cost. Navigation is unaffected. Tweak live via
   * `editorState.gridRenderRadius`.
   */
  gridRenderRadius: number;
  /**
   * When true, maps within `gridEditRadius` of the focused map are painted in
   * place and can be edited without switching tabs. Outside that radius (up to
   * `gridRenderRadius`) maps are still drawn as dimmed, read-only context.
   */
  gridEditEnabled: boolean;
  /** Chebyshev radius of editable neighbours; must be <= gridRenderRadius. */
  gridEditRadius: number;
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

const editorState: EditorState = {
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
  gridRenderRadius: 2,
  gridEditEnabled: true,
  gridEditRadius: 1,
  activePaintMapName: '',
  hoveredGridMapName: '',
  gridUndoOrder: [],
};
export const getEditorState = () => editorState;

/**
 * The map canvas render loop repaints only when this is set. Input handlers and
 * deliberate state changes set it; the loop clears it after a frame. Keeps the
 * editor from pinning a CPU core (and starving other browser tabs) while idle.
 */
let renderDirty = true;
export const markRenderDirty = () => {
  renderDirty = true;
};
export const isRenderDirty = () => renderDirty;
export const clearRenderDirty = () => {
  renderDirty = false;
};

export const updateEditorState = (state: Partial<EditorState>) => {
  Object.assign(editorState, { ...getEditorState(), ...state });
  markRenderDirty();
  (window as any).reRenderTileEditor();
};
export const updateEditorStateNoReRender = (state: Partial<EditorState>) => {
  Object.assign(editorState, { ...getEditorState(), ...state });
};
export const getEditorStateMap = (
  mapName: string
): EditorStateMap | undefined => {
  const map = getEditorState().maps[mapName];
  return map;
};
export const updateEditorStateMap = (
  mapName: string,
  state: Partial<EditorStateMap>
) => {
  const map = getEditorStateMap(mapName);
  if (map) {
    // console.log('updateEditorStateMap', mapName, state);
    Object.assign(map, { ...map, ...state });
    markRenderDirty();
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
    dataRevision: 0,
  };
  getEditorState().maps[mapName] = map;
  return map;
};

/** Per-map editor state for `mapName`, creating it if it does not exist yet. */
export const ensureEditorStateMap = (mapName: string): EditorStateMap => {
  return getEditorState().maps[mapName] ?? createEditorStateMap(mapName);
};

/**
 * The map paint state should be read from / written to right now: the stroke's
 * target block while a cross-block stroke is in flight, otherwise the focused
 * map. Everything in paintTools keys per-map state on this.
 */
export const getPaintMapName = (state: EditorState): string =>
  state.activePaintMapName || state.selectedMapName;

/** Record that a stroke completed on `mapName` for grid-wide undo ordering. */
export const pushGridUndo = (mapName: string): void => {
  if (!mapName) {
    return;
  }
  const order = getEditorState().gridUndoOrder;
  order.push(mapName);
  const MAX = 400;
  if (order.length > MAX) {
    order.splice(0, order.length - MAX);
  }
};

/**
 * Current data revision for a map. Returns 0 for a map with no editor-state
 * entry (e.g. a read-only neighbour rendered through renderMapTilesAtOffset).
 */
export const getMapDataRevision = (mapName: string): number =>
  getEditorStateMap(mapName)?.dataRevision ?? 0;

/**
 * Advance a map's data revision, invalidating every cached materialized layer
 * for it. No-op for a map with no editor-state entry. Call only after a commit
 * or a structural change — never mid-stroke, or the in-flight working buffer is
 * swapped out and the edit is lost.
 */
export const bumpMapDataRevision = (mapName: string): void => {
  const current = getEditorStateMap(mapName)?.dataRevision;
  if (current === undefined) {
    return;
  }
  updateEditorStateMapNoReRender(mapName, { dataRevision: current + 1 });
  markRenderDirty();
};

export const renameEditorStateMap = (oldName: string, newName: string) => {
  const trimmedOld = oldName.trim();
  const trimmedNew = newName.trim();
  if (!trimmedOld || !trimmedNew || trimmedOld === trimmedNew) {
    return;
  }

  const state = getEditorState();
  const existing = state.maps[trimmedOld];
  if (!existing) {
    return;
  }

  state.maps[trimmedNew] = existing;
  delete state.maps[trimmedOld];
  if (state.selectedMapName === trimmedOld) {
    state.selectedMapName = trimmedNew;
  }
  (window as any).reRenderTileEditor();
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

export { createTilesForLayer } from '../utils/mapIndex';
