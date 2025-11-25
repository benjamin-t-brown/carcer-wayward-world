import { drawRect } from '../utils/draw';
import { getTransform } from './seEditorEvents';
import { SpecialEventEditorState } from './seEditorState';
import {
  GameEvent,
  GameEventChildExec,
  GameEventChildType,
} from '../types/assets';
import { renderExecNode } from './nodeRendering/renderExecNode';

const getColors = () => {
  return {
    BACKGROUND1: 'black',
    BACKGROUND2: '#243F72',
    TEXT: 'white',
  };
};

export const loop = (
  dataInterface: {
    getCanvas: () => HTMLCanvasElement;
    getEditorState: () => SpecialEventEditorState;
    getGameEvent: () => GameEvent | undefined;
  },
  ms: number
) => {
  const ctx = dataInterface.getCanvas().getContext('2d');
  if (!ctx) {
    return;
  }

  if (!dataInterface.getEditorState().selectedEventId) {
    return;
  }

  ctx.clearRect(
    0,
    0,
    dataInterface.getCanvas().width,
    dataInterface.getCanvas().height
  );
  drawRect(
    0,
    0,
    dataInterface.getCanvas().width,
    dataInterface.getCanvas().height,
    getColors().BACKGROUND2,
    false,
    ctx
  );

  const { x, y, scale } = getTransform();

  //background rect
  ctx.save();
  ctx.translate(x, y);
  ctx.scale(scale, scale);
  ctx.translate(
    dataInterface.getCanvas().width / 2,
    dataInterface.getCanvas().height / 2
  );
  ctx.translate(
    -dataInterface.getEditorState().zoneWidth / 2,
    -dataInterface.getEditorState().zoneHeight / 2
  );
  drawRect(
    0,
    0,
    dataInterface.getEditorState().zoneWidth,
    dataInterface.getEditorState().zoneHeight,
    getColors().BACKGROUND1,
    false,
    ctx
  );
  ctx.restore();

  // const nodeZone = document.getElementById('se-nodes');
  // if (nodeZone) {
  //   const offsetX =
  //     x +
  //     dataInterface.getCanvas().width / 2 -
  //     dataInterface.getEditorState().zoneWidth / 2;
  //   const offsetY =
  //     y +
  //     dataInterface.getCanvas().height / 2 -
  //     dataInterface.getEditorState().zoneHeight / 2;
  //   nodeZone.style.left = `${offsetX}px`;
  //   nodeZone.style.top = `${offsetY}px`;
  //   // nodeZone.style.width = `${dataInterface.getEditorState().zoneWidth}px`;
  //   // nodeZone.style.height = `${dataInterface.getEditorState().zoneHeight}px`;
  // }

  const newScale = scale;

  const focalX = dataInterface.getCanvas().width / 2;
  const focalY = dataInterface.getCanvas().height / 2;

  const offsetX = focalX - (newScale / scale) * (focalX - x);
  const offsetY = focalY - (newScale / scale) * (focalY - y);

  ctx.save();
  ctx.translate(offsetX, offsetY);
  ctx.translate(
    (dataInterface.getCanvas().width * newScale) / 2,
    (dataInterface.getCanvas().height * newScale) / 2
  );
  ctx.translate(
    -(dataInterface.getEditorState().zoneWidth * newScale) / 2,
    -(dataInterface.getEditorState().zoneHeight * newScale) / 2
  );

  // Render nodes
  const gameEvent = dataInterface.getGameEvent();
  if (gameEvent && gameEvent.children) {
    for (const child of gameEvent.children) {
      if (child.eventChildType === GameEventChildType.EXEC) {
        renderExecNode(
          child as GameEventChildExec,
          child.x,
          child.y,
          newScale,
          ctx
        );
      }
      // TODO: Add rendering for other node types
    }
  }

  ctx.restore();
};
