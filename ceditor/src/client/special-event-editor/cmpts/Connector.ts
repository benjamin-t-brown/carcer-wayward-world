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

  isColliding(x: number, y: number) {
    const radius = 15;
    const dist = Math.sqrt((x - this.startX) ** 2 + (y - this.startY) ** 2);
    return dist <= radius;
  }

  isLineColliding(x: number, y: number) {
    return false;
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

    // ctx.save();
    // TODO anchor
    // ctx.fillStyle = 'white';
    // const ANCHOR_RADIUS = 4;

    // ctx.beginPath();
    // ctx.arc(
    //   this.endX * scale,
    //   this.endY * scale,
    //   ANCHOR_RADIUS * scale,
    //   0,
    //   Math.PI * 2
    // );
    // ctx.fill();
    // ctx.restore();
  }
}
