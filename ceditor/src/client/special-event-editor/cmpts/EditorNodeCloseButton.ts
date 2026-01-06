import { drawRect, drawText } from '../../utils/draw';
import { EditorNode } from './EditorNode';

export interface RenderCloseButtonArgs {
  isHovered: boolean;
  isActive: boolean;
}

export class EditorNodeCloseButton {
  btnSize: number = 15;
  x: number = 0;
  y: number = 0;
  parentNode: EditorNode;

  constructor(parentNode: EditorNode) {
    this.parentNode = parentNode;
  }

  isColliding(x: number, y: number) {
    // const radius = 15;
    // const startX = this.x + this.btnSize / 2;
    // const startY = this.y + this.btnSize / 2;
    // const dist = Math.sqrt((x - startX) ** 2 + (y - startY) ** 2);
    // return dist <= radius;
    return (
      x >= this.x &&
      x <= this.x + this.btnSize &&
      y >= this.y &&
      y <= this.y + this.btnSize
    );
  }

  update() {
    const bounds = this.parentNode.getBounds();
    this.x =
      bounds.x + bounds.width - this.btnSize - this.parentNode.borderSize - 1;
    this.y = bounds.y + this.parentNode.borderSize + 1;
  }

  render(
    ctx: CanvasRenderingContext2D,
    scale: number,
    args: RenderCloseButtonArgs
  ) {
    ctx.save();
    ctx.translate(this.x * scale, this.y * scale);
    ctx.scale(scale, scale);
    drawRect(
      0,
      0,
      this.btnSize,
      this.btnSize,
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
  }
}
