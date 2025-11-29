import { SENode } from '../../types/assets';
import { drawRect, drawText } from '../../utils/draw';
import { getNodeDimensions } from '../nodeHelpers';

export const getCloseButtonBounds = (node: SENode, scale: number = 1) => {
  const btnSize = 15;
  const borderWidth = 2;
  const [nodeWidth] = getNodeDimensions(node);
  return {
    x: node.x * scale + nodeWidth * scale - btnSize * scale - borderWidth * 2,
    y: node.y * scale + borderWidth * 2,
    width: btnSize * scale,
    height: btnSize * scale,
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
  const bounds = getCloseButtonBounds(node, scale);
  ctx.save();
  ctx.translate(bounds.x, bounds.y);
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
