import { drawRect, drawText } from '../../utils/draw';

export const renderCloseButton = (
  nodeX: number,
  nodeY: number,
  nodeWidth: number,
  borderWidth: number,
  scale: number,
  ctx: CanvasRenderingContext2D,
  args: {
    isHovered: boolean;
    isActive: boolean;
  }
) => {
  const btnSize = 15;
  ctx.save();
  ctx.translate(nodeX, nodeY);
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
