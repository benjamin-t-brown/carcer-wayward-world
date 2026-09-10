import {
  connectionsFromNode,
  type EventNode,
} from '../../core/domain/events/index.js';
import {
  EVENT_NODE_WIDTH,
  nodeHeight,
  type EventViewport,
  type Point,
} from './graphGeometry.js';

export interface EventRenderState {
  readonly nodes: readonly Readonly<EventNode>[];
  readonly selectedIndexes: ReadonlySet<number>;
  readonly viewport: Readonly<EventViewport>;
  readonly selectionStart?: Point;
  readonly selectionEnd?: Point;
}

const TYPE_COLORS: Readonly<Record<string, string>> = {
  EXEC: '#55c7ad',
  CHOICE: '#78b9e8',
  SWITCH: '#e4bc65',
  END: '#e07171',
  COMMENT: '#aaaab3',
  KEYWORD: '#b795e8',
};

function roundedRect(
  context: CanvasRenderingContext2D,
  x: number,
  y: number,
  width: number,
  height: number,
  radius: number,
): void {
  context.beginPath();
  context.roundRect(x, y, width, height, radius);
}

function nodeCenter(node: Readonly<EventNode>): Point {
  return {
    x: (node.x ?? 0) + EVENT_NODE_WIDTH / 2,
    y: (node.y ?? 0) + nodeHeight(node) / 2,
  };
}

function drawGrid(
  context: CanvasRenderingContext2D,
  viewport: Readonly<EventViewport>,
  width: number,
  height: number,
): void {
  const step = 40 * viewport.scale;
  if (step < 8) return;
  context.strokeStyle = '#2d2d33';
  context.lineWidth = 1;
  context.beginPath();
  for (let x = ((viewport.x % step) + step) % step; x < width; x += step) {
    context.moveTo(Math.round(x) + 0.5, 0);
    context.lineTo(Math.round(x) + 0.5, height);
  }
  for (let y = ((viewport.y % step) + step) % step; y < height; y += step) {
    context.moveTo(0, Math.round(y) + 0.5);
    context.lineTo(width, Math.round(y) + 0.5);
  }
  context.stroke();
}

function truncate(text: string, maximum: number): string {
  return text.length <= maximum ? text : `${text.slice(0, maximum - 1)}…`;
}

export class EventRenderer {
  draw(context: CanvasRenderingContext2D, state: EventRenderState): void {
    const width = context.canvas.width;
    const height = context.canvas.height;
    context.save();
    context.setTransform(1, 0, 0, 1, 0, 0);
    context.fillStyle = '#18181b';
    context.fillRect(0, 0, width, height);
    drawGrid(context, state.viewport, width, height);

    context.translate(state.viewport.x, state.viewport.y);
    context.scale(state.viewport.scale, state.viewport.scale);
    this.drawConnections(context, state.nodes);
    state.nodes.forEach((node, index) =>
      this.drawNode(context, node, state.selectedIndexes.has(index)),
    );
    if (state.selectionStart && state.selectionEnd) {
      const left = Math.min(state.selectionStart.x, state.selectionEnd.x);
      const top = Math.min(state.selectionStart.y, state.selectionEnd.y);
      const boxWidth = Math.abs(state.selectionEnd.x - state.selectionStart.x);
      const boxHeight = Math.abs(state.selectionEnd.y - state.selectionStart.y);
      context.fillStyle = 'rgba(85, 199, 173, .12)';
      context.strokeStyle = '#72ddc3';
      context.lineWidth = 1 / state.viewport.scale;
      context.fillRect(left, top, boxWidth, boxHeight);
      context.strokeRect(left, top, boxWidth, boxHeight);
    }
    context.restore();
  }

  private drawConnections(
    context: CanvasRenderingContext2D,
    nodes: readonly Readonly<EventNode>[],
  ): void {
    const firstById = new Map<string, Readonly<EventNode>>();
    for (const node of nodes) {
      if (!firstById.has(node.id)) firstById.set(node.id, node);
    }
    context.lineWidth = 2;
    for (const source of nodes) {
      const start = nodeCenter(source);
      for (const connection of connectionsFromNode(source as EventNode)) {
        const target = firstById.get(connection.targetId);
        if (!target) continue;
        const end = nodeCenter(target);
        const bend = Math.max(35, Math.abs(end.x - start.x) * 0.35);
        context.beginPath();
        context.moveTo(start.x, start.y);
        context.bezierCurveTo(
          start.x + bend,
          start.y,
          end.x - bend,
          end.y,
          end.x,
          end.y,
        );
        context.strokeStyle =
          connection.kind === 'defaultNext' ? '#e4bc65' : '#7b7b86';
        context.stroke();
      }
    }
  }

  private drawNode(
    context: CanvasRenderingContext2D,
    node: Readonly<EventNode>,
    selected: boolean,
  ): void {
    const x = node.x ?? 0;
    const y = node.y ?? 0;
    const height = nodeHeight(node);
    const color = TYPE_COLORS[node.eventChildType] ?? '#d38ad7';
    roundedRect(context, x, y, EVENT_NODE_WIDTH, height, 7);
    context.fillStyle = '#29292f';
    context.fill();
    context.strokeStyle = selected ? '#ffffff' : color;
    context.lineWidth = selected ? 3 : 2;
    context.stroke();

    context.fillStyle = color;
    context.fillRect(x, y, EVENT_NODE_WIDTH, 25);
    context.fillStyle = '#111114';
    context.font = '700 12px system-ui, sans-serif';
    context.fillText(node.eventChildType, x + 8, y + 17);
    context.fillStyle = '#ededf0';
    context.font = '12px ui-monospace, monospace';
    context.fillText(truncate(node.id, 25), x + 8, y + 43);
    const detail =
      node.eventChildType === 'EXEC'
        ? String(node.p ?? node.execStr ?? '')
        : node.eventChildType === 'CHOICE'
          ? `${Array.isArray(node.choices) ? node.choices.length : 0} choices`
          : node.eventChildType === 'SWITCH'
            ? `${Array.isArray(node.cases) ? node.cases.length : 0} cases`
            : node.eventChildType === 'COMMENT'
              ? String(node.comment ?? '')
              : '';
    if (detail) {
      context.fillStyle = '#aaaab3';
      context.font = '11px system-ui, sans-serif';
      context.fillText(
        truncate(detail.replace(/\s+/g, ' '), 32),
        x + 8,
        y + 59,
      );
    }
  }
}
