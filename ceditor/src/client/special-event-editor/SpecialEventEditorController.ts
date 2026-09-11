import { EditorStateSE } from './seEditorState';

export interface SpecialEventEditorControllerEnvironment {
  keyboardTarget?: EventTarget;
  documentTarget?: Pick<Document, 'activeElement' | 'querySelector'>;
}

export interface SpecialEventEditorInputHandlers {
  pointerdown?: (event: PointerEvent) => void;
  pointermove?: (event: PointerEvent) => void;
  pointerup?: (event: PointerEvent) => void;
  pointercancel?: (event: PointerEvent) => void;
  contextmenu?: (event: MouseEvent) => void;
  wheel?: (event: WheelEvent) => void;
  dblclick?: (event: MouseEvent) => void;
  keydown?: (event: KeyboardEvent) => void;
  keyup?: (event: KeyboardEvent) => void;
}

export interface SpecialEventEditorAttachment {
  canvas: HTMLCanvasElement;
  handlers: SpecialEventEditorInputHandlers;
}

/**
 * Owns one special-event editor session's state and browser input lifecycle.
 *
 * Node and serialization operations stay as ordinary functions. They receive
 * this controller (or its state) explicitly, so separate editor instances do
 * not share selections, transforms, clipboard state, or input timing.
 */
export class SpecialEventEditorController {
  readonly state = new EditorStateSE();

  lastClickTime = 0;
  lastClickNodeId: string | null = null;
  lastWheelTime = 0;
  linkingEmptyClickPending = false;

  private readonly subscribers = new Set<() => void>();
  private readonly keyboardTarget?: EventTarget;
  private readonly documentTarget?: Pick<
    Document,
    'activeElement' | 'querySelector'
  >;
  private attachment: SpecialEventEditorAttachment | null = null;
  private detachListeners: (() => void) | null = null;
  private readonly capturedPointers = new Set<number>();
  private keyboardActive = false;
  private snapshot = 0;
  private destroyed = false;

  constructor(environment: SpecialEventEditorControllerEnvironment = {}) {
    this.keyboardTarget =
      environment.keyboardTarget ??
      (typeof window === 'undefined' ? undefined : window);
    this.documentTarget =
      environment.documentTarget ??
      (typeof document === 'undefined' ? undefined : document);
  }

  get isAttached(): boolean {
    return this.attachment !== null;
  }

  get isDestroyed(): boolean {
    return this.destroyed;
  }

  getState = (): EditorStateSE => this.state;

  getSnapshot = (): number => this.snapshot;

  subscribe = (listener: () => void): (() => void) => {
    if (this.destroyed) return () => {};
    this.subscribers.add(listener);
    return () => this.subscribers.delete(listener);
  };

  notify = (validate = true): void => {
    if (this.destroyed) return;
    if (validate) this.state.shouldValidate = true;
    this.snapshot++;
    for (const listener of this.subscribers) listener();
  };

  update = (next: Partial<EditorStateSE>, validate = true): void => {
    Object.assign(this.state, next);
    this.notify(validate);
  };

  updateSilent = (next: Partial<EditorStateSE>): void => {
    if (this.destroyed) return;
    Object.assign(this.state, next);
  };

  markValidationRequired = (): void => {
    if (this.destroyed) return;
    this.notify(true);
  };

  hasOpenModal = (): boolean =>
    Boolean(this.documentTarget?.querySelector('.generic-modal'));

  attach = (attachment: SpecialEventEditorAttachment): void => {
    this.assertAlive();
    if (this.attachment === attachment) return;
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
      if (cancelled) handlers.pointercancel?.(pointerEvent);
      else handlers.pointerup?.(pointerEvent);
    };
    const pointerUp = (event: Event) => finishPointer(event, false);
    const pointerCancel = (event: Event) => finishPointer(event, true);
    const contextMenu = (event: Event) =>
      handlers.contextmenu?.(event as MouseEvent);
    const wheel = (event: Event) => handlers.wheel?.(event as WheelEvent);
    const doubleClick = (event: Event) =>
      handlers.dblclick?.(event as MouseEvent);
    const focus = () => {
      this.keyboardActive = true;
    };
    const blur = () => {
      this.keyboardActive = false;
    };
    const ownsKeyboardFocus = () =>
      this.documentTarget
        ? this.documentTarget.activeElement === canvas
        : this.keyboardActive;
    const keyDown = (event: Event) => {
      if (ownsKeyboardFocus()) handlers.keydown?.(event as KeyboardEvent);
    };
    const keyUp = (event: Event) => {
      if (ownsKeyboardFocus()) handlers.keyup?.(event as KeyboardEvent);
    };

    if (!canvas.hasAttribute('tabindex')) canvas.tabIndex = 0;
    canvas.addEventListener('pointerdown', pointerDown);
    canvas.addEventListener('pointermove', pointerMove);
    canvas.addEventListener('pointerup', pointerUp);
    canvas.addEventListener('pointercancel', pointerCancel);
    canvas.addEventListener('contextmenu', contextMenu);
    canvas.addEventListener('wheel', wheel, { passive: false });
    canvas.addEventListener('dblclick', doubleClick);
    canvas.addEventListener('focus', focus);
    canvas.addEventListener('blur', blur);
    this.keyboardTarget?.addEventListener('keydown', keyDown);
    this.keyboardTarget?.addEventListener('keyup', keyUp);

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
      canvas.removeEventListener('dblclick', doubleClick);
      canvas.removeEventListener('focus', focus);
      canvas.removeEventListener('blur', blur);
      this.keyboardTarget?.removeEventListener('keydown', keyDown);
      this.keyboardTarget?.removeEventListener('keyup', keyUp);
    };
  };

  detach = (): void => {
    this.abortInteraction();
    this.detachListeners?.();
    this.detachListeners = null;
    this.attachment = null;
    this.keyboardActive = false;
  };

  destroy = (): void => {
    if (this.destroyed) return;
    this.detach();
    if (this.state.copyFeedbackTimeout !== null) {
      clearTimeout(this.state.copyFeedbackTimeout);
      this.state.copyFeedbackTimeout = null;
    }
    this.subscribers.clear();
    this.state.editorSaveStates.clear();
    this.state.editorNodes = [];
    this.state.copiedNodes = null;
    this.destroyed = true;
  };

  abortInteraction = (): void => {
    this.state.isDragging = false;
    this.state.isDraggingNode = false;
    this.state.draggedNodeId = null;
    this.state.nodeDragOffsetX = 0;
    this.state.nodeDragOffsetY = 0;
    this.state.isSelecting = false;
    this.state.selectionRect = null;
    this.state.selectedNodesInitialPositions.clear();
    this.linkingEmptyClickPending = false;
  };

  resetDocumentInteraction = (): void => {
    this.abortInteraction();
    this.state.selectedNodeIds.clear();
    this.state.hoveredNodeId = undefined;
    this.state.hoveredCloseButtonNodeId = undefined;
    this.state.hoveredExitAnchor = undefined;
    this.state.linking = {
      isLinking: false,
      sourceNodeId: '',
      exitIndex: 0,
    };
    this.state.runnerErrors = [];
    this.lastClickTime = 0;
    this.lastClickNodeId = null;
  };

  private assertAlive(): void {
    if (this.destroyed) {
      throw new Error('SpecialEventEditorController has been destroyed');
    }
  }
}
