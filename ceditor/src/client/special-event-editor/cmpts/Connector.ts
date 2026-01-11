import { distanceToLineSegment } from '../nodeHelpers';

const START_COLOR = 'rgba(100, 255, 255, 0.3)';
const END_COLOR = 'rgba(255, 255, 100, 0.3)';

export class Connector {
  private startX = 0;
  private startY = 0;
  private endX = 0;
  private endY = 0;
  toNodeId = '';
  fromNodeId = '';
  exitIndex = 0;
  segments: {
    x: number;
    y: number;
    color: string;
  }[] = [];
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

  updatePosition(startX: number, startY: number, endX: number, endY: number) {
    this.startX = startX;
    this.startY = startY;
    this.endX = endX;
    this.endY = endY;
    this.segments = this.calculateSegments();
  }

  getStartPos() {
    return { x: this.startX, y: this.startY };
  }

  getEndPos() {
    return { x: this.endX, y: this.endY };
  }

  calculateSegments() {
    const xDistance = this.endX - this.startX;
    const yDistance = this.endY - this.startY;

    const points: { x: number; y: number; color: string }[] = [];

    if (xDistance < 36) {
      const endpointLength = 36;
      const xDistanceMiddleSegment = xDistance - endpointLength * 2;
      points.push({ x: this.startX, y: this.startY, color: START_COLOR });
      points.push({
        x: this.startX + endpointLength,
        y: this.startY,
        color: START_COLOR,
      });

      points.push({
        x: this.startX + endpointLength,
        y: this.startY + yDistance / 2,
        color: '',
      });
      points.push({
        x: this.startX + endpointLength + xDistanceMiddleSegment,
        y: this.startY + yDistance / 2,
        color: END_COLOR,
      });
      points.push({
        x: this.startX + endpointLength + xDistanceMiddleSegment,
        y: this.startY + yDistance,
        color: END_COLOR,
      });

      points.push({
        x: this.endX - endpointLength,
        y: this.endY,
        color: END_COLOR,
      });
      points.push({ x: this.endX, y: this.endY, color: END_COLOR });
    } else {
      points.push({ x: this.startX, y: this.startY, color: START_COLOR });
      points.push({
        x: this.startX + xDistance / 2,
        y: this.startY,
        color: '',
      });
      points.push({ x: this.startX + xDistance / 2, y: this.endY, color: END_COLOR });
      points.push({ x: this.endX, y: this.endY, color: END_COLOR });
    }

    return points;
  }

  isLineColliding(x: number, y: number) {
    for (let i = 0; i < this.segments.length - 1; i++) {
      const segment = this.segments[i];
      const nextSegment = this.segments[i + 1];
      if (
        distanceToLineSegment(
          x,
          y,
          segment.x,
          segment.y,
          nextSegment.x,
          nextSegment.y
        ) <= 10
      ) {
        return true;
      }
    }
    return false;
  }

  update() {}

  render(ctx: CanvasRenderingContext2D, scale: number) {
    if (!this.toNodeId || !this.fromNodeId) {
      return;
    }

    ctx.save();

    ctx.lineWidth = 10 * scale;
    ctx.beginPath();

    // ctx.moveTo(this.startX * scale, this.startY * scale);
    // ctx.lineTo(this.endX * scale, this.endY * scale);

    const points = this.segments;
    for (let i = 0; i < points.length; i++) {
      ctx.strokeStyle = 'rgba(255, 255, 255, 0.3)';
      const point = points[i];
      ctx.lineTo(point.x * scale, point.y * scale);
    }
    ctx.stroke();

    // const startSegment =

    // const points = this.segments;
    for (let i = 0; i < points.length - 1; i++) {
      const firstPoint = points[i];
      const secondPoint = points[i + 1];
      ctx.lineWidth = 5 * scale;
      ctx.beginPath();
      const color = points[i].color;
      if (color) {
        ctx.strokeStyle = color;
      } else {
        continue;
      }
      ctx.moveTo(firstPoint.x * scale, firstPoint.y * scale);
      ctx.lineTo(secondPoint.x * scale, secondPoint.y * scale);
      ctx.stroke();
    }

    ctx.restore();
  }
}
