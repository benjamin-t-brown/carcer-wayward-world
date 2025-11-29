import { SENode } from '../../types/assets';
import { drawRect, drawText } from '../../utils/draw';
import { getNodeDimensions } from '../nodeHelpers';

export const getCloseButtonBounds = (node: SENode) => {
  const btnSize = 15;
  const borderWidth = 2;
  const [nodeWidth] = getNodeDimensions(node);
  return {
    x: node.x + nodeWidth - btnSize - borderWidth * 2,
    y: node.y + borderWidth * 2,
    width: btnSize,
    height: btnSize,
  };
};

export const renderCloseButton = (
  node: SENode,
  scale: number,
  ctx: CanvasRenderingContext2D,
  args: {
    isHovered: boolean;
    isActive: boolean;
  }
) => {
  const btnSize = 15;
  const borderWidth = 2;
  const [nodeWidth] = getNodeDimensions(node);
  ctx.save();
  ctx.translate(node.x, node.y);
  ctx.translate(nodeWidth - btnSize * scale - borderWidth * 2, borderWidth * 2);
  ctx.scale(scale, scale);
  drawRect(
    0,
    0,
    btnSize,
    btnSize,
    args.isActive ? '#500' : args.isHovered ? '#a00' : '#800',
    false,
    ctx
  );
  drawText(
    'X',
    4,
    2,
    {
      color: '#fff',
      size: 12,
      font: 'courier',
      strokeColor: '',
      align: 'left',
      baseline: 'top',
    },
    ctx
  );
  ctx.restore();
};
