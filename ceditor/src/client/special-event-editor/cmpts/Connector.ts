import { distanceToLineSegment } from '../nodeHelpers';

export class Connector {
  startX = 0;
  startY = 0;
  endX = 0;
  endY = 0;
  toNodeId = '';
  fromNodeId = '';
  exitIndex = 0;

  static create(
    fromNodeId: string,
    toNodeId: string,
    startX: number,
    startY: number,
    exitIndex: number
  ) {
    const c = new Connector();
    c.fromNodeId = fromNodeId;
    c.toNodeId = toNodeId;
    c.startX = startX;
    c.startY = startY;
    c.exitIndex = exitIndex;
    return c;
  }

  isLineColliding(x: number, y: number) {
    return (
      distanceToLineSegment(
        x,
        y,
        this.startX,
        this.startY,
        this.endX,
        this.endY
      ) <= 10
    );
  }

  update() {}

  render(ctx: CanvasRenderingContext2D, scale: number) {
    if (!this.toNodeId || !this.fromNodeId) {
      return;
    }

    ctx.save();
    ctx.strokeStyle = 'rgba(255, 255, 255, 0.3)';
    ctx.lineWidth = 10 * scale;
    ctx.beginPath();

    ctx.moveTo(this.startX * scale, this.startY * scale);
    ctx.lineTo(this.endX * scale, this.endY * scale);
    ctx.stroke();
    ctx.restore();
  }
}
