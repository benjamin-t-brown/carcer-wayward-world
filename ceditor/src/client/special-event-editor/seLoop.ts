import { drawRect } from '../utils/draw';
import { getTransform } from './seEditorEvents';
import { SpecialEventEditorState } from './seEditorState';

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

  ctx.restore();
};
