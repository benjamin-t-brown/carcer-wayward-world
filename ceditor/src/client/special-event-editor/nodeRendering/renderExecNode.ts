import { GameEventChildExec } from '../../types/assets';
import { drawRect, drawText } from '../../utils/draw';
import { renderCloseButton } from './closeButton';
import { breakTextIntoLines, calculateHeightFromText } from './nodeHelpers';

const NODE_COLOR = '#808080';
const BORDER_COLOR = '#aaa';
const BORDER_HOVER_COLOR = '#fff';
const TEXT_COLOR = '#FFF';
const FONT_FAMILY = 'arial';
const NODE_WIDTH = 300;
const NODE_TITLE_HEIGHT = 20;
const BORDER_WIDTH = 2;
const FONT_SIZE = 12;
const PADDING = 6;
const LINE_SPACING = 4;
const LINE_HEIGHT = 14;

export const getExecNodeDimensions = (node: GameEventChildExec) => {
  return [NODE_WIDTH, node.h];
};

export const renderExecNode = (
  node: GameEventChildExec,
  x: number,
  y: number,
  scale: number,
  ctx: CanvasRenderingContext2D,
  args: {
    isHovered: boolean;
    isCloseButtonHovered?: boolean;
  }
) => {
  const { p, execStr } = node;

  const availableWidth = NODE_WIDTH - PADDING * 2 - BORDER_WIDTH * 2;
  const allText = p + '\n\n' + execStr;
  const lines = breakTextIntoLines(
    allText,
    availableWidth,
    FONT_SIZE,
    FONT_FAMILY,
    ctx
  );
  const calculatedHeight = calculateHeightFromText(
    lines,
    FONT_SIZE,
    FONT_FAMILY,
    LINE_HEIGHT,
    LINE_SPACING,
    ctx
  );
  node.h =
    calculatedHeight + NODE_TITLE_HEIGHT + PADDING * 2 + BORDER_WIDTH * 2;

  const nodeX = x * scale;
  const nodeY = y * scale;
  const nodeWidth = NODE_WIDTH * scale;
  const nodeHeight = node.h * scale;

  drawRect(
    nodeX,
    nodeY,
    nodeWidth,
    nodeHeight,
    args.isHovered ? BORDER_HOVER_COLOR : BORDER_COLOR,
    false,
    ctx
  );
  drawRect(
    nodeX + BORDER_WIDTH,
    nodeY + BORDER_WIDTH,
    nodeWidth - BORDER_WIDTH * 2,
    nodeHeight - BORDER_WIDTH * 2,
    NODE_COLOR,
    false,
    ctx
  );

  renderCloseButton(nodeX, nodeY, nodeWidth, BORDER_WIDTH, scale, ctx, {
    isHovered: args.isCloseButtonHovered || false,
    isActive: false,
  });

  // title
  ctx.save();
  ctx.translate(nodeX, nodeY);
  ctx.scale(scale, scale);
  ctx.translate(PADDING + BORDER_WIDTH, PADDING + BORDER_WIDTH);
  drawText(
    node.id,
    0,
    0,
    {
      color: '#ddd',
      size: FONT_SIZE,
      font: FONT_FAMILY,
      strokeColor: '',
      align: 'left',
      baseline: 'top',
    },
    ctx
  );
  ctx.restore();

  for (let i = 0; i < lines.length; i++) {
    ctx.save();
    ctx.translate(nodeX, nodeY);
    ctx.scale(scale, scale);
    ctx.translate(
      PADDING + BORDER_WIDTH,
      PADDING + BORDER_WIDTH + NODE_TITLE_HEIGHT
    );
    drawText(
      lines[i],
      0,
      i * LINE_HEIGHT,
      {
        color: TEXT_COLOR,
        size: FONT_SIZE,
        font: FONT_FAMILY,
        strokeColor: '',
        align: 'left',
        baseline: 'top',
      },
      ctx
    );
    ctx.restore();
  }
};
