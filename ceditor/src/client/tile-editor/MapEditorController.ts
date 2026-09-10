import type { PaintAction } from './paintTools';
import {
  createInitialEditorState,
  type EditorState,
  type EditorStateMap,
} from './editorState';
import type { GridSlotHit } from './gridMapNavigation';
import type { GridNavigationHandlers } from './editorEvents';
import type { CarcerMapTileTemplate } from '../types/assets';
import {
  MapDocumentIndexCache,
  type MapDocumentIndex,
  type MapDocumentIndexAssets,
} from './mapDocumentIndex';

export interface MapEditorInputState {
  isDragging: boolean;
  isPainting: boolean;
  isDraggingRight: boolean;
  lastClickX: number;
  lastClickY: number;
  lastTranslateX: number;
  lastTranslateY: number;
  translateX: number;
  translateY: number;
  scale: number;
  mouseX: number;
  mouseY: number;
  pendingGridSlotClick: GridSlotHit | null;
  gridSlotClickStartX: number;
  gridSlotClickStartY: number;
  rightDragMapName: string;
  rightDragGridActive: boolean;
  rightDragStartGX: number;
  rightDragStartGY: number;
  rightDragEndGX: number;
  rightDragEndGY: number;
}

export const createMapEditorInputState = (): MapEditorInputState => ({
  isDragging: false,
  isPainting: false,
  isDraggingRight: false,
  lastClickX: 0,
  lastClickY: 0,
  lastTranslateX: 0,
  lastTranslateY: 0,
  translateX: 0,
  translateY: 0,
  scale: 1,
  mouseX: 0,
  mouseY: 0,
  pendingGridSlotClick: null,
  gridSlotClickStartX: 0,
  gridSlotClickStartY: 0,
  rightDragMapName: '',
  rightDragGridActive: false,
  rightDragStartGX: 0,
  rightDragStartGY: 0,
  rightDragEndGX: 0,
  rightDragEndGY: 0,
});

type FrameRequest = (callback: FrameRequestCallback) => number;
type FrameCancel = (id: number) => void;

export interface MapEditorControllerEnvironment {
  keyboardTarget?: EventTarget;
  resizeTarget?: EventTarget;
  requestAnimationFrame?: FrameRequest;
  cancelAnimationFrame?: FrameCancel;
}

export interface MapEditorControllerHandlers {
  pointerdown?: (event: PointerEvent) => void;
  pointermove?: (event: PointerEvent) => void;
  pointerup?: (event: PointerEvent) => void;
  pointercancel?: (event: PointerEvent) => void;
  contextmenu?: (event: MouseEvent) => void;
  wheel?: (event: WheelEvent) => void;
  keydown?: (event: KeyboardEvent) => void;
  keyup?: (event: KeyboardEvent) => void;
  resize?: (event: Event) => void;
}

export interface MapEditorAttachment {
  canvas: HTMLCanvasElement;
  handlers: MapEditorControllerHandlers;
}

export type MapEditorFrame = (timestamp: number, elapsedMs: number) => void;

/**
 * Owns one map editor session's mutable state and browser lifecycle.
 *
 * Domain algorithms remain ordinary functions; they receive this controller
 * (or its state) explicitly. The controller only coordinates mutation,
 * subscriptions, input listeners, and the continuous canvas frame loop.
 */
export class MapEditorController {
  readonly state: EditorState;
  readonly input: MapEditorInputState;

  private readonly subscribers = new Set<() => void>();
  private readonly mapRevisions = new Map<string, number>();
  private readonly layerViews = new Map<string, CarcerMapTileTemplate[]>();
  private readonly documentIndexCache = new MapDocumentIndexCache();
  private readonly keyboardTarget?: EventTarget;
  private readonly resizeTarget?: EventTarget;
  private readonly requestFrame?: FrameRequest;
  private readonly cancelFrame?: FrameCancel;
  private attachment: MapEditorAttachment | null = null;
  private detachListeners: (() => void) | null = null;
  private frameCallback: MapEditorFrame | null = null;
  private frameId: number | null = null;
  private previousFrameTimestamp: number | null = null;
  private keyboardActive = false;
  private snapshot = 0;
  private layerViewPinCount = 0;
  private destroyed = false;
  private action: PaintAction | null = null;
  private gridNavigationHandlers: GridNavigationHandlers | null = null;
  private readonly capturedPointers = new Set<number>();
  canvasRect: {
    canvas: HTMLCanvasElement;
    left: number;
    top: number;
  } | null = null;
  lastAppliedCursor: string | null = null;

  constructor(environment: MapEditorControllerEnvironment = {}) {
    this.state = createInitialEditorState();
    this.input = createMapEditorInputState();
    this.keyboardTarget =
      environment.keyboardTarget ??
      (typeof window === 'undefined' ? undefined : window);
    this.resizeTarget =
      environment.resizeTarget ??
      (typeof window === 'undefined' ? undefined : window);
    this.requestFrame =
      environment.requestAnimationFrame ??
      (typeof requestAnimationFrame === 'undefined'
        ? undefined
        : requestAnimationFrame.bind(globalThis));
    this.cancelFrame =
      environment.cancelAnimationFrame ??
      (typeof cancelAnimationFrame === 'undefined'
        ? undefined
        : cancelAnimationFrame.bind(globalThis));
  }

  get isAttached(): boolean {
    return this.attachment !== null;
  }

  get isRunning(): boolean {
    return this.frameId !== null;
  }

  get isDestroyed(): boolean {
    return this.destroyed;
  }

  getState = (): EditorState => this.state;

  getSnapshot = (): number => this.snapshot;

  subscribe = (listener: () => void): (() => void) => {
    if (this.destroyed) {
      return () => {};
    }
    this.subscribers.add(listener);
    return () => this.subscribers.delete(listener);
  };

  notify = (): void => {
    if (this.destroyed) {
      return;
    }
    this.snapshot++;
    for (const listener of this.subscribers) {
      listener();
    }
  };

  update = (next: Partial<EditorState>, notify = true): void => {
    Object.assign(this.state, next);
    if (notify) {
      this.notify();
    }
  };

  getMapState = (mapName: string): EditorStateMap | undefined =>
    this.state.maps[mapName];

  ensureMap = (mapName: string): EditorStateMap => {
    const existing = this.state.maps[mapName];
    if (existing) {
      return existing;
    }
    const created: EditorStateMap = {
      selectedTileInd: -1,
      hoveredTileIndex: -1,
      hoveredTileData: { x: -1, y: -1, ind: -1 },
      undoHistory: [],
      undoIndex: 0,
    };
    this.state.maps[mapName] = created;
    this.mapRevisions.set(mapName, 0);
    return created;
  };

  updateMap = (
    mapName: string,
    next: Partial<EditorStateMap>,
    notify = true,
  ): void => {
    const mapState = this.state.maps[mapName];
    if (!mapState) {
      return;
    }
    Object.assign(mapState, next);
    if (notify) {
      this.notify();
    }
  };

  renameMap = (oldName: string, newName: string): void => {
    const trimmedOld = oldName.trim();
    const trimmedNew = newName.trim();
    if (!trimmedOld || !trimmedNew || trimmedOld === trimmedNew) {
      return;
    }
    const existing = this.state.maps[trimmedOld];
    if (!existing) {
      return;
    }
    this.state.maps[trimmedNew] = existing;
    delete this.state.maps[trimmedOld];
    const revision = this.mapRevisions.get(trimmedOld);
    this.deleteLayerViewsForMap(trimmedOld);
    if (revision !== undefined) {
      this.mapRevisions.set(trimmedNew, revision);
      this.mapRevisions.delete(trimmedOld);
    }
    if (this.state.selectedMapName === trimmedOld) {
      this.state.selectedMapName = trimmedNew;
    }
    this.notify();
  };

  getMapRevision = (mapName: string): number =>
    this.mapRevisions.get(mapName) ?? 0;

  bumpMapRevision = (mapName: string): void => {
    const revision = this.mapRevisions.get(mapName);
    if (revision === undefined) {
      return;
    }
    this.mapRevisions.set(mapName, revision + 1);
    this.deleteLayerViewsForMap(mapName);
    this.notify();
  };

  getLayerView = (
    mapName: string,
    level: number,
  ): CarcerMapTileTemplate[] | undefined =>
    this.layerViews.get(this.layerViewKey(mapName, level));

  setLayerView = (
    mapName: string,
    level: number,
    tiles: CarcerMapTileTemplate[],
  ): void => {
    this.layerViews.set(this.layerViewKey(mapName, level), tiles);
    if (!this.action && this.layerViewPinCount === 0) this.trimLayerViews();
  };

  getDocumentIndex = (
    assets: MapDocumentIndexAssets,
    revision = 'working',
  ): MapDocumentIndex => this.documentIndexCache.get(revision, assets);

  getCurrentAction = (): PaintAction | null => this.action;

  setCurrentAction = (action: PaintAction | null): void => {
    this.action = action;
    if (!action && this.layerViewPinCount === 0) this.trimLayerViews();
  };

  pinLayerViews = (): (() => void) => {
    this.layerViewPinCount++;
    let active = true;
    return () => {
      if (!active) return;
      active = false;
      this.layerViewPinCount = Math.max(0, this.layerViewPinCount - 1);
      if (this.layerViewPinCount === 0 && !this.action) this.trimLayerViews();
    };
  };

  removeMap = (mapName: string): void => {
    delete this.state.maps[mapName];
    this.mapRevisions.delete(mapName);
    this.deleteLayerViewsForMap(mapName);
    this.state.gridUndoOrder = this.state.gridUndoOrder.filter(
      (name) => name !== mapName,
    );
    if (this.state.selectedMapName === mapName) {
      this.state.selectedMapName = '';
    }
    if (this.state.activePaintMapName === mapName) {
      this.state.activePaintMapName = '';
    }
    if (this.state.hoveredGridMapName === mapName) {
      this.state.hoveredGridMapName = '';
    }
    this.notify();
  };

  setGridNavigationHandlers = (
    handlers: GridNavigationHandlers | null,
  ): void => {
    this.gridNavigationHandlers = handlers;
  };

  getGridNavigationHandlers = (): GridNavigationHandlers | null =>
    this.gridNavigationHandlers;

  attach = (attachment: MapEditorAttachment): void => {
    this.assertAlive();
    if (this.attachment === attachment) {
      return;
    }
    this.detach();

    const { canvas, handlers } = attachment;
    const pointerDown = (event: Event) => {
      const pointerEvent = event as PointerEvent;
      this.keyboardActive = true;
      canvas.focus();
      if (canvas.setPointerCapture && pointerEvent.pointerId !== undefined) {
        canvas.setPointerCapture(pointerEvent.pointerId);
        this.capturedPointers.add(pointerEvent.pointerId);
      }
      handlers.pointerdown?.(pointerEvent);
    };
    const pointerMove = (event: Event) =>
      handlers.pointermove?.(event as PointerEvent);
    const finishPointer = (event: Event, cancelled: boolean) => {
      const pointerEvent = event as PointerEvent;
      if (
        canvas.releasePointerCapture &&
        pointerEvent.pointerId !== undefined &&
        (!canvas.hasPointerCapture ||
          canvas.hasPointerCapture(pointerEvent.pointerId))
      ) {
        canvas.releasePointerCapture(pointerEvent.pointerId);
      }
      this.capturedPointers.delete(pointerEvent.pointerId);
      if (cancelled) {
        handlers.pointercancel?.(pointerEvent);
      } else {
        handlers.pointerup?.(pointerEvent);
      }
    };
    const pointerUp = (event: Event) => finishPointer(event, false);
    const pointerCancel = (event: Event) => finishPointer(event, true);
    const contextMenu = (event: Event) =>
      handlers.contextmenu?.(event as MouseEvent);
    const wheel = (event: Event) => handlers.wheel?.(event as WheelEvent);
    const focus = () => {
      this.keyboardActive = true;
    };
    const blur = () => {
      this.keyboardActive = false;
    };
    const ownsKeyboardFocus = () =>
      typeof document === 'undefined'
        ? this.keyboardActive
        : document.activeElement === canvas;
    const keyDown = (event: Event) => {
      if (ownsKeyboardFocus()) {
        handlers.keydown?.(event as KeyboardEvent);
      }
    };
    const keyUp = (event: Event) => {
      if (ownsKeyboardFocus()) {
        handlers.keyup?.(event as KeyboardEvent);
      }
    };
    const resize = (event: Event) => handlers.resize?.(event);

    if (!canvas.hasAttribute('tabindex')) {
      canvas.tabIndex = 0;
    }
    canvas.addEventListener('pointerdown', pointerDown);
    canvas.addEventListener('pointermove', pointerMove);
    canvas.addEventListener('pointerup', pointerUp);
    canvas.addEventListener('pointercancel', pointerCancel);
    canvas.addEventListener('contextmenu', contextMenu);
    canvas.addEventListener('wheel', wheel, { passive: false });
    canvas.addEventListener('focus', focus);
    canvas.addEventListener('blur', blur);
    this.keyboardTarget?.addEventListener('keydown', keyDown);
    this.keyboardTarget?.addEventListener('keyup', keyUp);
    this.resizeTarget?.addEventListener('resize', resize);

    this.attachment = attachment;
    this.detachListeners = () => {
      for (const pointerId of this.capturedPointers) {
        if (
          canvas.releasePointerCapture &&
          (!canvas.hasPointerCapture || canvas.hasPointerCapture(pointerId))
        ) {
          canvas.releasePointerCapture(pointerId);
        }
      }
      this.capturedPointers.clear();
      canvas.removeEventListener('pointerdown', pointerDown);
      canvas.removeEventListener('pointermove', pointerMove);
      canvas.removeEventListener('pointerup', pointerUp);
      canvas.removeEventListener('pointercancel', pointerCancel);
      canvas.removeEventListener('contextmenu', contextMenu);
      canvas.removeEventListener('wheel', wheel);
      canvas.removeEventListener('focus', focus);
      canvas.removeEventListener('blur', blur);
      this.keyboardTarget?.removeEventListener('keydown', keyDown);
      this.keyboardTarget?.removeEventListener('keyup', keyUp);
      this.resizeTarget?.removeEventListener('resize', resize);
    };
  };

  detach = (): void => {
    this.abortInteraction();
    this.detachListeners?.();
    this.detachListeners = null;
    this.attachment = null;
    this.keyboardActive = false;
  };

  start = (frame: MapEditorFrame): void => {
    this.assertAlive();
    if (this.frameId !== null || !this.requestFrame) {
      return;
    }
    this.frameCallback = frame;
    const tick = (timestamp: number) => {
      if (this.frameId === null || this.destroyed) {
        return;
      }
      const previous = this.previousFrameTimestamp ?? timestamp;
      this.previousFrameTimestamp = timestamp;
      try {
        this.frameCallback?.(timestamp, timestamp - previous);
      } catch (error) {
        console.error('map editor frame failed', error);
      }
      if (this.frameId !== null && !this.destroyed) {
        this.frameId = this.requestFrame?.(tick) ?? null;
      }
    };
    this.frameId = this.requestFrame(tick);
  };

  stop = (): void => {
    if (this.frameId !== null) {
      this.cancelFrame?.(this.frameId);
      this.frameId = null;
    }
    this.previousFrameTimestamp = null;
  };

  destroy = (): void => {
    if (this.destroyed) {
      return;
    }
    this.stop();
    this.detach();
    this.subscribers.clear();
    this.action = null;
    this.gridNavigationHandlers = null;
    this.layerViews.clear();
    this.layerViewPinCount = 0;
    this.documentIndexCache.clear();
    this.canvasRect = null;
    this.lastAppliedCursor = null;
    this.destroyed = true;
  };

  private assertAlive(): void {
    if (this.destroyed) {
      throw new Error('MapEditorController has been destroyed');
    }
  }

  private layerViewKey(mapName: string, level: number): string {
    return `${mapName}|${this.getMapRevision(mapName)}|${level}`;
  }

  private trimLayerViews(): void {
    while (this.layerViews.size > 96) {
      const oldest = this.layerViews.keys().next().value;
      if (oldest === undefined) break;
      this.layerViews.delete(oldest);
    }
  }

  private deleteLayerViewsForMap(mapName: string): void {
    const prefix = `${mapName}|`;
    for (const key of this.layerViews.keys()) {
      if (key.startsWith(prefix)) this.layerViews.delete(key);
    }
  }

  private abortInteraction(): void {
    const hadUncommittedAction = this.action !== null || this.input.isPainting;
    if (hadUncommittedAction) this.layerViews.clear();
    this.action = null;
    this.input.isDragging = false;
    this.input.isPainting = false;
    this.input.isDraggingRight = false;
    this.input.pendingGridSlotClick = null;
    this.input.rightDragMapName = '';
    this.input.rightDragGridActive = false;
    this.state.isSelectDragging = false;
    this.state.selectDragSourceTileIndex = -1;
    this.state.activePaintMapName = '';
    this.state.hoveredGridMapName = '';
  }
}
