class SpecialEventEditorEventState {
  isDragging = false;
  isDraggingRight = false;
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
const specialEventEditorEventState = new SpecialEventEditorEventState();

let isPanZoomInitialized = false;
const panZoomEvents: {
  keydown: (ev: KeyboardEvent) => void;
  keyup: (ev: KeyboardEvent) => void;
  mousedown: (ev: MouseEvent) => void;
  mousemove: (ev: MouseEvent) => void;
  mouseup: (ev: MouseEvent) => void;
  contextmenu: (ev: MouseEvent) => void;
  wheel: (ev: WheelEvent) => void;
} = {
  keydown: () => {},
  keyup: () => {},
  mousedown: () => {},
  mousemove: () => {},
  mouseup: () => {},
  contextmenu: () => {},
  wheel: () => {},
};

(window as any).specialEventEditorEventState = specialEventEditorEventState;
// let panzoomCanvas: HTMLCanvasElement | null = null;

const isEventWithCanvasTarget = (
  ev: MouseEvent,
  panzoomCanvas: HTMLCanvasElement | undefined
) => {
  const targetId = (ev.target as unknown as HTMLElement)?.id;
  return (
    (Boolean(panzoomCanvas) && ev.target === panzoomCanvas) ||
    targetId === 'special-event-editor-canvas'
  );
};

const isEditorActive = (ev: KeyboardEvent) => {
  return true; // TODO check if any modals are open
};

const shouldPreventDefault = (ev: KeyboardEvent) => {
  return ev.ctrlKey && (ev.key === 's' || ev.key === 'e');
};

export const initPanzoom = (specialEventEditorInterface: {
  getCanvas: () => HTMLCanvasElement;
}) => {
  const handleKeyDown = (ev: KeyboardEvent) => {
    if (shouldPreventDefault(ev)) {
      ev.preventDefault();
    }
  };
  const handleKeyUp = (ev: KeyboardEvent) => {};
  const handleMouseDown = (ev: MouseEvent) => {
    if (
      ev.button === 1 &&
      isEventWithCanvasTarget(ev, specialEventEditorInterface.getCanvas())
    ) {
      specialEventEditorEventState.lastClickX = ev.clientX;
      specialEventEditorEventState.lastClickY = ev.clientY;
      specialEventEditorEventState.lastTranslateX =
        specialEventEditorEventState.translateX;
      specialEventEditorEventState.lastTranslateY =
        specialEventEditorEventState.translateY;
      specialEventEditorEventState.isDragging = true;
    }
    if (
      ev.button === 0 &&
      isEventWithCanvasTarget(ev, specialEventEditorInterface.getCanvas())
    ) {
    }
    if (
      ev.button === 2 &&
      isEventWithCanvasTarget(ev, specialEventEditorInterface.getCanvas())
    ) {
    }
  };
  const handleMouseMove = (ev: MouseEvent) => {
    specialEventEditorEventState.mouseX = ev.clientX;
    specialEventEditorEventState.mouseY = ev.clientY;

    if (specialEventEditorEventState.isDragging) {
      specialEventEditorEventState.translateX =
        specialEventEditorEventState.lastTranslateX +
        ev.clientX -
        specialEventEditorEventState.lastClickX;
      specialEventEditorEventState.translateY =
        specialEventEditorEventState.lastTranslateY +
        ev.clientY -
        specialEventEditorEventState.lastClickY;
    }
  };
  const handleMouseUp = (ev: MouseEvent) => {
    if (specialEventEditorEventState.isDragging) {
      specialEventEditorEventState.translateX =
        specialEventEditorEventState.lastTranslateX +
        ev.clientX -
        specialEventEditorEventState.lastClickX;
      specialEventEditorEventState.translateY =
        specialEventEditorEventState.lastTranslateY +
        ev.clientY -
        specialEventEditorEventState.lastClickY;
      specialEventEditorEventState.isDragging = false;
    }
  };
  const handleContextMenu = (ev: MouseEvent) => {
    if (isEventWithCanvasTarget(ev, specialEventEditorInterface.getCanvas())) {
      ev.preventDefault();
    }
  };
  const handleWheel = (ev: WheelEvent) => {
    if (isEventWithCanvasTarget(ev, specialEventEditorInterface.getCanvas())) {
      const [focalX, focalY] = screenCoordsToCanvasCoords(
        ev.clientX,
        ev.clientY,
        specialEventEditorInterface.getCanvas()
      );

      let nextScale = specialEventEditorEventState.scale;
      const scaleStep = 0.5;

      if (ev.deltaY > 0) {
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
        (nextScale / specialEventEditorEventState.scale) *
          (focalX - specialEventEditorEventState.translateX);
      const offsetY =
        focalY -
        (nextScale / specialEventEditorEventState.scale) *
          (focalY - specialEventEditorEventState.translateY);

      specialEventEditorEventState.translateX = offsetX;
      specialEventEditorEventState.translateY = offsetY;
      specialEventEditorEventState.scale = nextScale;
    }
  };
  window.addEventListener('keydown', handleKeyDown);
  window.addEventListener('keyup', handleKeyUp);
  window.addEventListener('mousedown', handleMouseDown);
  window.addEventListener('mousemove', handleMouseMove);
  window.addEventListener('mouseup', handleMouseUp);
  window.addEventListener('contextmenu', handleContextMenu);
  window.addEventListener('wheel', handleWheel);

  isPanZoomInitialized = true;
  panZoomEvents.keydown = handleKeyDown;
  panZoomEvents.keyup = handleKeyUp;
  panZoomEvents.mousedown = handleMouseDown;
  panZoomEvents.mousemove = handleMouseMove;
  panZoomEvents.mouseup = handleMouseUp;
  panZoomEvents.contextmenu = handleContextMenu;
  panZoomEvents.wheel = handleWheel;
};

export const unInitPanzoom = () => {
  if (!isPanZoomInitialized || !panZoomEvents) {
    return;
  }
  window.removeEventListener('keydown', panZoomEvents.keydown);
  window.removeEventListener('keyup', panZoomEvents.keyup);
  window.removeEventListener('mousedown', panZoomEvents.mousedown);
  window.removeEventListener('mousemove', panZoomEvents.mousemove);
  window.removeEventListener('mouseup', panZoomEvents.mouseup);
  window.removeEventListener('contextmenu', panZoomEvents.contextmenu);
  window.removeEventListener('wheel', panZoomEvents.wheel);
  isPanZoomInitialized = false;
};

export const getTransform = () => {
  return {
    x: specialEventEditorEventState.translateX,
    y: specialEventEditorEventState.translateY,
    scale: specialEventEditorEventState.scale,
  };
};

export const resetPanzoom = () => {
  specialEventEditorEventState.translateX = 0;
  specialEventEditorEventState.translateY = 0;
  specialEventEditorEventState.scale = 1;
};

export const screenCoordsToCanvasCoords = (
  x: number,
  y: number,
  panzoomCanvas: HTMLCanvasElement
) => {
  const { left, top } = panzoomCanvas?.getBoundingClientRect() || {
    left: 0,
    top: 0,
    width: 0,
    height: 0,
  };

  const canvasX = x - left;
  const canvasY = y - top;

  return [canvasX, canvasY];
};

export const getScreenMouseCoords = () => {
  return [
    specialEventEditorEventState.mouseX,
    specialEventEditorEventState.mouseY,
  ];
};

export const getIsDraggingRight = () => {
  return specialEventEditorEventState.isDraggingRight;
};
