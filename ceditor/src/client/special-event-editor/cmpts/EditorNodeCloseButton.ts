import { drawRect, drawText } from '../../utils/draw';
import { EditorNode } from '../EditorNode';

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
    return (
      x >= this.x &&
      x <= this.x + this.btnSize &&
      y >= this.y &&
      y <= this.y + this.btnSize
    );
  }

  update() {
    this.x =
      this.parentNode.getBounds().x +
      this.parentNode.getBounds().width -
      this.btnSize -
      this.parentNode.borderSize -
      this.parentNode.padding;
    this.y =
      this.parentNode.getBounds().y +
      this.parentNode.borderSize +
      this.parentNode.padding;
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
